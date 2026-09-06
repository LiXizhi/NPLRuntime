#include "ParaScriptingMuJoCo.h"
#include "RobotCoordinateConverter.h"

#ifdef USE_MUJOCO

#include <mujoco/mujoco.h>
#include <cmath>
#include <sstream>
#include <vector>
#include <array>
#include <algorithm>

namespace {
using BlockBox = std::array<double, 12>;

// MuJoCo 3.10 world-body BVH stores local center/half-extents, child indices
// relative to body_bvhadr. Refit the existing topology after changing its leaves.
// No extra bodies, mocap state, mass or surface velocities are introduced.
bool RefitWorldNode(mjModel* m, int relative) {
	const int base = m->body_bvhadr[0], node = base + relative;
	mjtNum* bounds = m->bvh_aabb + 6*node;
	const int left=m->bvh_child[2*node], right=m->bvh_child[2*node+1];
	if (left < 0 && right < 0) {
		const int geom=m->bvh_nodeid[node];
		if (geom < 0 || !(m->geom_contype[geom] || m->geom_conaffinity[geom])) {
			mju_zero(bounds,6); bounds[0]=100000; return false;
		}
		mjtNum matrix[9]; mju_quat2Mat(matrix,m->geom_quat+4*geom);
		const mjtNum* aabb=m->geom_aabb+6*geom;
		for(int i=0;i<3;++i) {
			bounds[i]=m->geom_pos[3*geom+i]; bounds[3+i]=0;
			for(int j=0;j<3;++j) { bounds[i]+=matrix[3*i+j]*aabb[j]; bounds[3+i]+=std::abs(matrix[3*i+j])*aabb[3+j]; }
		}
		return true;
	}
	const bool a=left>=0 && RefitWorldNode(m,left), b=right>=0 && RefitWorldNode(m,right);
	const mjtNum* x=left>=0 ? m->bvh_aabb+6*(base+left) : NULL;
	const mjtNum* y=right>=0 ? m->bvh_aabb+6*(base+right) : NULL;
	if (!a && !b) { mju_zero(bounds,6); bounds[0]=100000; return false; }
	if (!a || !b) { mju_copy(bounds,a?x:y,6); return true; }
	for(int i=0;i<3;++i) {
		const double low=std::min(x[i]-x[i+3],y[i]-y[i+3]), high=std::max(x[i]+x[i+3],y[i]+y[i+3]);
		bounds[i]=(low+high)/2; bounds[i+3]=(high-low)/2;
	}
	return true;
}
}

struct ParaScripting::MuJoCoSimulation::Impl
{
	Impl() : model(NULL), data(NULL), source(NULL) {}
	~Impl()
	{
		mj_deleteData(data);
		mj_deleteModel(model);
		mj_deleteSpec(source);
	}

	mjModel* model;
	mjData* data;
	mjSpec* source;
	std::vector<int> poolGeoms;
	std::vector<int> poolPairs;
	std::vector<BlockBox> poolBoxes;
	double poolFriction=1.2, poolRolling=0.0003;
	int originalPairs=0;
	std::unique_ptr<mjData, decltype(&mj_deleteData)> scratch{NULL, mj_deleteData};
	std::string lastError;
	void ApplyPool(const std::vector<BlockBox>& boxes, double friction, double rolling);
};

ParaScripting::MuJoCoSimulation::MuJoCoSimulation() : m_impl(new Impl()) {}
ParaScripting::MuJoCoSimulation::~MuJoCoSimulation() {}

bool ParaScripting::MuJoCoSimulation::Load(const std::string& filename)
{
	char error[1024] = { 0 };
	std::unique_ptr<mjSpec, decltype(&mj_deleteSpec)> source(
		mj_parseXML(filename.c_str(), NULL, error, sizeof(error)), mj_deleteSpec);
	mjModel* model = source ? mj_compile(source.get(), NULL) : NULL;
	if (!model)
	{
		m_impl->lastError = source ? mjs_getError(source.get()) : error;
		return false;
	}

	mjData* data = mj_makeData(model);
	if (!data)
	{
		mj_deleteModel(model);
		m_impl->lastError = "mj_makeData failed";
		return false;
	}

	mj_deleteData(m_impl->data);
	mj_deleteModel(m_impl->model);
	m_impl->model = model;
	m_impl->data = data;
	mj_deleteSpec(m_impl->source);
	m_impl->source = source.release();
	m_impl->poolGeoms.clear(); m_impl->poolPairs.clear(); m_impl->poolBoxes.clear(); m_impl->scratch.reset();
	m_impl->originalPairs = model->npair;
	m_impl->lastError.clear();
	mj_forward(m_impl->model, m_impl->data);
	return true;
}

bool ParaScripting::MuJoCoSimulation::UpdateBlockCollision(const std::string& boxes, double friction, double rollingFriction)
{
	if (!IsValid() || !m_impl->source)
		return false;
	auto fail = [&](const std::string& reason) { m_impl->lastError = reason; return false; };
	if (!std::isfinite(friction) || friction < 0 || friction > 2 ||
		!std::isfinite(rollingFriction) || rollingFriction < 0 || rollingFriction > 0.01 || boxes.size() > 1024 * 1024)
		return fail("invalid block collision parameters");
	// A fresh copy of the original description prevents accumulated geoms/pairs.
	std::unique_ptr<mjSpec, decltype(&mj_deleteSpec)> candidate(mj_copySpec(m_impl->source), mj_deleteSpec);
	if (!candidate) return fail("cannot copy model specification");
	mjsBody* world = mjs_findBody(candidate.get(), "world");
	if (!world) return fail("world body not found");
	if (mjsElement* element = mjs_findElement(candidate.get(), mjOBJ_GEOM, "floor"))
	{
		mjsGeom* floor = mjs_asGeom(element);
		if (floor->type != mjGEOM_PLANE) return fail("named floor is not a plane");
		floor->contype = floor->conaffinity = 0; // Keep its name, not its hidden support.
	}
	const bool hasBall = mjs_findElement(candidate.get(), mjOBJ_GEOM, "ball_geom") != NULL;
	std::istringstream input(boxes);
	int count = 0;
	while (input >> std::ws && !input.eof())
	{
		double values[12]; // center, half extents, orthonormal X/Y axes (physical coordinates).
		for (double& value : values)
			if (!(input >> value) || !std::isfinite(value) || std::abs(value) > 100000)
				return fail("invalid block box record");
		if (++count > 2048) return fail("block collision box budget exceeded (2048)");
		for (int i = 3; i < 6; ++i)
			if (values[i] <= 0) return fail("block half size must be positive");
		double aa = 0, bb = 0, ab = 0;
		for (int i = 0; i < 3; ++i) { aa += values[6+i]*values[6+i]; bb += values[9+i]*values[9+i]; ab += values[6+i]*values[9+i]; }
		if (std::abs(aa-1) > 1e-6 || std::abs(bb-1) > 1e-6 || std::abs(ab) > 1e-6)
			return fail("block axes must be orthonormal");
		mjsGeom* geom = mjs_addGeom(world, NULL);
		const std::string name = "microduck_local_block_" + std::to_string(count);
		mjs_setName(geom->element, name.c_str());
		geom->type = mjGEOM_BOX;
		mjtNum matrix[9], third[3];
		mju_cross(third, values+6, values+9);
		for (int i = 0; i < 3; ++i)
		{
			geom->pos[i] = values[i]; geom->size[i] = values[i+3];
			matrix[3*i] = values[i+6]; matrix[3*i+1] = values[i+9]; matrix[3*i+2] = third[i];
		}
		mju_mat2Quat(geom->quat, matrix);
		geom->contype = geom->conaffinity = 1;
		geom->condim = 3;
		geom->friction[0] = friction; geom->friction[1] = 0.005; geom->friction[2] = 0.0001;
		if (hasBall && rollingFriction > 0)
		{
			mjsPair* pair = mjs_addPair(candidate.get(), NULL);
			mjs_setString(pair->geomname1, "ball_geom"); mjs_setString(pair->geomname2, name.c_str());
			pair->condim = 6;
			pair->friction[0] = pair->friction[1] = friction;
			pair->friction[2] = 0.005;
			pair->friction[3] = pair->friction[4] = rollingFriction;
		}
	}
	std::unique_ptr<mjModel, decltype(&mj_deleteModel)> model(mj_compile(candidate.get(), NULL), mj_deleteModel);
	if (!model) return fail(mjs_getError(candidate.get()));
	const mjModel* previous = m_impl->model;
	if (model->nq != previous->nq || model->nv != previous->nv || model->nu != previous->nu ||
		model->na != previous->na || model->nbody != previous->nbody || model->njnt != previous->njnt ||
		model->nsensor != previous->nsensor || model->nsensordata != previous->nsensordata ||
		model->nplugin != 0 || previous->nplugin != 0 || model->nmocap != previous->nmocap)
		return fail("block update changed dynamic model contract or uses unsupported plugins");
	// Bodies/joints/actuators/sensors must keep their indices. Geom indices may change.
	for (int type : {mjOBJ_BODY, mjOBJ_JOINT, mjOBJ_ACTUATOR, mjOBJ_SENSOR})
	{
		const int n = type == mjOBJ_BODY ? model->nbody : type == mjOBJ_JOINT ? model->njnt : type == mjOBJ_ACTUATOR ? model->nu : model->nsensor;
		for (int i = 0; i < n; ++i)
		{
			const char* a = mj_id2name(previous, type, i); const char* b = mj_id2name(model.get(), type, i);
			if (std::string(a ? a : "") != std::string(b ? b : "")) return fail("dynamic object indices changed");
		}
	}
	const int sig = mjSTATE_INTEGRATION;
	const int size = mj_stateSize(previous, sig);
	if (size != mj_stateSize(model.get(), sig)) return fail("integration state size changed");
	std::vector<mjtNum> state(size);
	mj_getState(previous, m_impl->data, state.data(), sig);
	std::unique_ptr<mjData, decltype(&mj_deleteData)> data(mj_makeData(model.get()), mj_deleteData);
	if (!data) return fail("cannot allocate candidate state");
	mj_setState(model.get(), data.get(), state.data(), sig);
	mj_forward(model.get(), data.get());
	for (int i = 0; i < data->ncon; ++i)
	{
		const mjContact& contact = data->contact[i];
		const char* a = mj_id2name(model.get(), mjOBJ_GEOM, contact.geom[0]);
		const char* b = mj_id2name(model.get(), mjOBJ_GEOM, contact.geom[1]);
		if (contact.dist < -0.01 && ((a && std::string(a).find("microduck_local_block_") == 0) ||
			(b && std::string(b).find("microduck_local_block_") == 0)))
			return fail("block overlaps robot or ball; remove block or reset before resuming");
	}
	// Forward recomputes contacts/derived state but may alter warm start; restore integration state exactly.
	mj_setState(model.get(), data.get(), state.data(), sig);
	mj_deleteData(m_impl->data); mj_deleteModel(m_impl->model);
	m_impl->model = model.release(); m_impl->data = data.release(); m_impl->lastError.clear();
	m_impl->poolGeoms.clear(); m_impl->poolPairs.clear(); m_impl->poolBoxes.clear(); m_impl->scratch.reset();
	return true;
}

void ParaScripting::MuJoCoSimulation::Impl::ApplyPool(const std::vector<BlockBox>& boxes, double friction, double rolling) {
	const int ball=mj_name2id(model,mjOBJ_GEOM,"ball_geom");
	for (size_t slot=0;slot<poolGeoms.size();++slot) {
		const int geom=poolGeoms[slot]; const bool active=slot<boxes.size();
		// Compiler marked parked identity rotations as BODYROT. After changing
		// orientation that shortcut would IGNORE geom_quat in mj_kinematics.
		model->geom_sameframe[geom]=mjSAMEFRAME_NONE;
		model->geom_contype[geom]=model->geom_conaffinity[geom]=active?1:0;
		mjtNum* pos=model->geom_pos+3*geom; mjtNum* size=model->geom_size+3*geom;
		mjtNum* quat=model->geom_quat+4*geom; mjtNum* aabb=model->geom_aabb+6*geom;
		mju_zero(aabb,6);
		if(active) {
			const auto& v=boxes[slot]; mjtNum third[3],matrix[9]; mju_cross(third,v.data()+6,v.data()+9);
			for(int i=0;i<3;++i) { pos[i]=v[i]; size[i]=aabb[3+i]=v[3+i]; matrix[3*i]=v[6+i]; matrix[3*i+1]=v[9+i]; matrix[3*i+2]=third[i]; }
			mju_mat2Quat(quat,matrix);
		} else { pos[0]=100000;pos[1]=pos[2]=0; mju_unit4(quat); for(int i=0;i<3;++i) size[i]=aabb[3+i]=0.001; }
		model->geom_rbound[geom]=mju_norm3(size);
		model->geom_friction[3*geom]=friction;
	}
	// Keep npair and name-map offsets immutable. MuJoCo 3.10 SphereBox returns
	// before contact generation when distance-radius > margin (distance >= 0).
	// An inactive pair's margin < -radius therefore disables it even if the
	// sphere is teleported inside the parked box. Global margin override is disallowed.
	for(size_t slot=0;slot<poolPairs.size();++slot) {
		const int i=poolPairs[slot];
		model->pair_margin[i]=slot<boxes.size()?0:-model->geom_size[3*ball]-1;
		model->pair_gap[i]=0;model->pair_dim[i]=rolling>0?6:3;
		model->pair_friction[5*i]=model->pair_friction[5*i+1]=friction;
		model->pair_friction[5*i+2]=0.005;
		model->pair_friction[5*i+3]=model->pair_friction[5*i+4]=rolling;
	}
	model->body_contype[0]=model->body_conaffinity[0]=0;
	for(int i=0;i<model->body_geomnum[0];++i) {
		const int geom=model->body_geomadr[0]+i;
		model->body_contype[0]|=model->geom_contype[geom]; model->body_conaffinity[0]|=model->geom_conaffinity[geom];
	}
	if(model->body_bvhadr[0]>=0) RefitWorldNode(model,0);
}

bool ParaScripting::MuJoCoSimulation::InitializeBlockCollisionPool(int capacity) {
	if(!IsValid() || capacity<1 || capacity>2048 || m_impl->originalPairs!=0 || mj_version()!=3010000 || (m_impl->model->opt.enableflags & mjENBL_OVERRIDE)) {
		m_impl->lastError="pool requires MuJoCo 3.10.0, 1..2048 slots and a source with no explicit pairs"; return false;
	}
	// Initialization is also transactional: retain the complete old model/pool
	// until every slot, pair and scratch allocation has been validated.
	std::unique_ptr<Impl> candidate(new Impl());
	candidate->model=mj_copyModel(NULL,m_impl->model);
	candidate->source=mj_copySpec(m_impl->source);
	if(candidate->model) candidate->data=mj_makeData(candidate->model);
	if(!candidate->model || !candidate->source || !candidate->data) {
		m_impl->lastError="cannot allocate pool initialization candidate";return false;
	}
	mj_copyData(candidate->data,candidate->model,m_impl->data);
	candidate->originalPairs=m_impl->originalPairs;
	struct Rollback {
		std::unique_ptr<Impl>& current; std::unique_ptr<Impl> previous; bool committed=false;
		~Rollback() { if(!committed) { previous->lastError=current->lastError;current=std::move(previous); } }
	} rollback{m_impl,std::move(m_impl)};
	m_impl=std::move(candidate);
	std::ostringstream boxes;
	for(int i=0;i<capacity;++i) boxes<<"10000 0 0 0.001 0.001 0.001 1 0 0 0 1 0 ";
	if(!UpdateBlockCollision(boxes.str(),1.2,0.0003)) return false;
	for(int i=0;i<capacity;++i) {
		const int geom=FindGeom("microduck_local_block_"+std::to_string(i+1));
		if(geom<0) { m_impl->lastError="pool geom not found"; return false; }
		m_impl->poolGeoms.push_back(geom);
	}
	m_impl->poolPairs.clear();
	const int ball=FindGeom("ball_geom");
	if(ball>=0) {
		if(m_impl->model->geom_type[ball]!=mjGEOM_SPHERE) { m_impl->lastError="pool ball must be a sphere"; return false; }
		for(int geom:m_impl->poolGeoms) {
			int found=-1;
			for(int i=0;i<m_impl->model->npair;++i)
				if((m_impl->model->pair_geom1[i]==ball && m_impl->model->pair_geom2[i]==geom) || (m_impl->model->pair_geom2[i]==ball && m_impl->model->pair_geom1[i]==geom)) { found=i;break; }
			if(found<0) { m_impl->lastError="pool ball pair not found";return false; }
			m_impl->poolPairs.push_back(found);
		}
	}
	m_impl->scratch.reset(mj_makeData(m_impl->model));
	if(!m_impl->scratch) { m_impl->lastError="cannot allocate pool scratch data"; return false; }
	std::vector<mjtNum> state(mj_stateSize(m_impl->model,mjSTATE_INTEGRATION));
	mj_getState(m_impl->model,m_impl->data,state.data(),mjSTATE_INTEGRATION);
	m_impl->ApplyPool({},1.2,0.0003); m_impl->poolBoxes.clear(); Forward();
	mj_setState(m_impl->model,m_impl->data,state.data(),mjSTATE_INTEGRATION);
	rollback.committed=true;
	return true;
}

bool ParaScripting::MuJoCoSimulation::UpdateBlockCollisionPool(const std::string& text, double friction, double rolling) {
	auto fail=[&](const std::string& message) { m_impl->lastError=message; return false; };
	if(!IsValid() || m_impl->poolGeoms.empty() || !m_impl->scratch) return fail("pool is not initialized");
	if(m_impl->model->opt.enableflags & mjENBL_OVERRIDE) return fail("global contact override is incompatible with pool deactivation");
	if(text.size()>1024*1024 || !std::isfinite(friction) || friction<0 || friction>2 || !std::isfinite(rolling) || rolling<0 || rolling>0.01) return fail("invalid pool parameters");
	std::vector<BlockBox> boxes; std::istringstream input(text);
	while(input>>std::ws && !input.eof()) {
		BlockBox v;
		for(double& value:v) if(!(input>>value) || !std::isfinite(value) || std::abs(value)>10000) return fail("invalid pool box record");
		for(int i=3;i<6;++i) if(v[i]<=0) return fail("pool box half size must be positive");
		double a=0,b=0,ab=0; for(int i=0;i<3;++i) { a+=v[6+i]*v[6+i];b+=v[9+i]*v[9+i];ab+=v[6+i]*v[9+i]; }
		if(std::abs(a-1)>1e-6 || std::abs(b-1)>1e-6 || std::abs(ab)>1e-6) return fail("pool axes must be orthonormal");
		boxes.push_back(v); if(boxes.size()>m_impl->poolGeoms.size()) return fail("collision pool capacity exceeded");
	}
	mjModel* m=m_impl->model; mjData* candidate=m_impl->scratch.get();
	mj_copyData(candidate,m,m_impl->data);
	m_impl->ApplyPool(boxes,friction,rolling);
	mj_forward(m,candidate);
	for(int i=0;i<candidate->ncon;++i) {
		const mjContact& contact=candidate->contact[i];
		if(contact.dist < -0.01 && (m->geom_bodyid[contact.geom[0]]==0 || m->geom_bodyid[contact.geom[1]]==0)) {
			std::ostringstream reason;
			reason<<"block overlap: "<<(mj_id2name(m,mjOBJ_GEOM,contact.geom[0])?mj_id2name(m,mjOBJ_GEOM,contact.geom[0]):"unnamed")
				<<" / "<<(mj_id2name(m,mjOBJ_GEOM,contact.geom[1])?mj_id2name(m,mjOBJ_GEOM,contact.geom[1]):"unnamed")<<" depth="<<-contact.dist;
			m_impl->ApplyPool(m_impl->poolBoxes,m_impl->poolFriction,m_impl->poolRolling);
			return fail(reason.str());
		}
	}
	std::vector<mjtNum> state(mj_stateSize(m,mjSTATE_INTEGRATION));
	mj_getState(m,m_impl->data,state.data(),mjSTATE_INTEGRATION);
	mj_setState(m,candidate,state.data(),mjSTATE_INTEGRATION);
	mjData* previous=m_impl->data; m_impl->data=m_impl->scratch.release();m_impl->scratch.reset(previous);
	m_impl->poolBoxes=std::move(boxes); m_impl->poolFriction=friction;m_impl->poolRolling=rolling;m_impl->lastError.clear();
	return true;
}

int ParaScripting::MuJoCoSimulation::GetGeomCount() const { return IsValid() ? m_impl->model->ngeom : 0; }
int ParaScripting::MuJoCoSimulation::GetContactDimension(int index) const
{ return IsValid() && index >= 0 && index < m_impl->data->ncon ? m_impl->data->contact[index].dim : 0; }
double ParaScripting::MuJoCoSimulation::GetContactFriction(int index, int component) const
{ return IsValid() && index >= 0 && index < m_impl->data->ncon && component >= 0 && component < 5 ? m_impl->data->contact[index].friction[component] : 0; }

void ParaScripting::MuJoCoSimulation::Reset()
{
	if (IsValid())
	{
		mj_resetData(m_impl->model, m_impl->data);
		mj_forward(m_impl->model, m_impl->data);
	}
}

void ParaScripting::MuJoCoSimulation::Forward()
{
	if (IsValid())
		mj_forward(m_impl->model, m_impl->data);
}

void ParaScripting::MuJoCoSimulation::Step(int count)
{
	if (!IsValid())
		return;
	for (int index = 0; index < count; ++index)
		mj_step(m_impl->model, m_impl->data);
}

bool ParaScripting::MuJoCoSimulation::IsValid() const { return m_impl->model && m_impl->data; }
const std::string& ParaScripting::MuJoCoSimulation::GetLastError() const { return m_impl->lastError; }
int ParaScripting::MuJoCoSimulation::GetQPosCount() const { return IsValid() ? static_cast<int>(m_impl->model->nq) : 0; }
int ParaScripting::MuJoCoSimulation::GetQVelCount() const { return IsValid() ? static_cast<int>(m_impl->model->nv) : 0; }
int ParaScripting::MuJoCoSimulation::GetControlCount() const { return IsValid() ? static_cast<int>(m_impl->model->nu) : 0; }
int ParaScripting::MuJoCoSimulation::GetBodyCount() const { return IsValid() ? static_cast<int>(m_impl->model->nbody) : 0; }
int ParaScripting::MuJoCoSimulation::GetJointCount() const { return IsValid() ? static_cast<int>(m_impl->model->njnt) : 0; }
int ParaScripting::MuJoCoSimulation::GetActuatorCount() const { return IsValid() ? static_cast<int>(m_impl->model->nu) : 0; }
int ParaScripting::MuJoCoSimulation::GetContactCount() const { return IsValid() ? static_cast<int>(m_impl->data->ncon) : 0; }
double ParaScripting::MuJoCoSimulation::GetTime() const { return IsValid() ? m_impl->data->time : 0.0; }

double ParaScripting::MuJoCoSimulation::GetQPos(int index) const
{
	return IsValid() && index >= 0 && index < m_impl->model->nq ? m_impl->data->qpos[index] : 0.0;
}

bool ParaScripting::MuJoCoSimulation::SetQPos(int index, double value)
{
	if (!IsValid() || index < 0 || index >= m_impl->model->nq)
		return false;
	m_impl->data->qpos[index] = value;
	return true;
}

double ParaScripting::MuJoCoSimulation::GetQVel(int index) const
{
	return IsValid() && index >= 0 && index < m_impl->model->nv ? m_impl->data->qvel[index] : 0.0;
}

bool ParaScripting::MuJoCoSimulation::SetQVel(int index, double value)
{
	if (!IsValid() || index < 0 || index >= m_impl->model->nv)
		return false;
	m_impl->data->qvel[index] = value;
	return true;
}

double ParaScripting::MuJoCoSimulation::GetControl(int index) const
{
	return IsValid() && index >= 0 && index < m_impl->model->nu ? m_impl->data->ctrl[index] : 0.0;
}

bool ParaScripting::MuJoCoSimulation::SetControl(int index, double value)
{
	if (!IsValid() || index < 0 || index >= m_impl->model->nu)
		return false;
	m_impl->data->ctrl[index] = value;
	return true;
}

int ParaScripting::MuJoCoSimulation::NameToId(int objectType, const std::string& name) const
{
	return IsValid() ? mj_name2id(m_impl->model, objectType, name.c_str()) : -1;
}

int ParaScripting::MuJoCoSimulation::FindBody(const std::string& name) const
{
	return NameToId(mjOBJ_BODY, name);
}

int ParaScripting::MuJoCoSimulation::FindJoint(const std::string& name) const
{
	return NameToId(mjOBJ_JOINT, name);
}

int ParaScripting::MuJoCoSimulation::FindActuator(const std::string& name) const
{
	return NameToId(mjOBJ_ACTUATOR, name);
}

int ParaScripting::MuJoCoSimulation::FindSensor(const std::string& name) const
{
	return NameToId(mjOBJ_SENSOR, name);
}

int ParaScripting::MuJoCoSimulation::GetSensorDim(int sensorId) const
{
	if (!IsValid() || sensorId < 0 || sensorId >= m_impl->model->nsensor)
		return 0;
	return static_cast<int>(m_impl->model->sensor_dim[sensorId]);
}

double ParaScripting::MuJoCoSimulation::GetSensorData(int sensorId, int component) const
{
	const int dimension = GetSensorDim(sensorId);
	if (component < 0 || component >= dimension)
		return 0.0;
	return m_impl->data->sensordata[m_impl->model->sensor_adr[sensorId] + component];
}

int ParaScripting::MuJoCoSimulation::GetJointQPosAdr(int jointId) const
{
	if (!IsValid() || jointId < 0 || jointId >= m_impl->model->njnt)
		return -1;
	return static_cast<int>(m_impl->model->jnt_qposadr[jointId]);
}

int ParaScripting::MuJoCoSimulation::GetJointDofAdr(int jointId) const
{
	if (!IsValid() || jointId < 0 || jointId >= m_impl->model->njnt)
		return -1;
	return static_cast<int>(m_impl->model->jnt_dofadr[jointId]);
}

double ParaScripting::MuJoCoSimulation::GetJointAxis(int jointId, int component) const
{
	if (!IsValid() || jointId < 0 || jointId >= m_impl->model->njnt || component < 0 || component >= 3)
		return 0.0;
	return m_impl->model->jnt_axis[3 * jointId + component];
}

double ParaScripting::MuJoCoSimulation::GetJointParaAxis(int jointId, int component) const
{
	if (!IsValid() || jointId < 0 || jointId >= m_impl->model->njnt || component < 0 || component >= 3)
		return 0.0;
	const mjtNum* axis = m_impl->data->xaxis + 3 * jointId;
	ParaEngine::RobotVector3 converted = ParaEngine::RobotCoordinateConverter::MuJoCoPositionToParaEngine(axis[0], axis[1], axis[2]);
	const double values[] = { converted.x, converted.y, converted.z };
	return values[component];
}

double ParaScripting::MuJoCoSimulation::GetJointParaAnchor(int jointId, int component) const
{
	if (!IsValid() || jointId < 0 || jointId >= m_impl->model->njnt || component < 0 || component >= 3)
		return 0.0;
	const mjtNum* anchor = m_impl->data->xanchor + 3 * jointId;
	ParaEngine::RobotVector3 converted = ParaEngine::RobotCoordinateConverter::MuJoCoPositionToParaEngine(anchor[0], anchor[1], anchor[2]);
	const double values[] = { converted.x, converted.y, converted.z };
	return values[component];
}

double ParaScripting::MuJoCoSimulation::GetBodyPosition(int bodyId, int component) const
{
	if (!IsValid() || bodyId < 0 || bodyId >= m_impl->model->nbody || component < 0 || component >= 3)
		return 0.0;
	return m_impl->data->xpos[3 * bodyId + component];
}

double ParaScripting::MuJoCoSimulation::GetBodyQuaternion(int bodyId, int component) const
{
	if (!IsValid() || bodyId < 0 || bodyId >= m_impl->model->nbody || component < 0 || component >= 4)
		return 0.0;
	return m_impl->data->xquat[4 * bodyId + component];
}

double ParaScripting::MuJoCoSimulation::GetBodyParaPosition(int bodyId, int component) const
{
	if (!IsValid() || bodyId < 0 || bodyId >= m_impl->model->nbody || component < 0 || component >= 3)
		return 0.0;
	const mjtNum* position = m_impl->data->xpos + 3 * bodyId;
	ParaEngine::RobotVector3 converted = ParaEngine::RobotCoordinateConverter::MuJoCoPositionToParaEngine(position[0], position[1], position[2]);
	const double values[] = { converted.x, converted.y, converted.z };
	return values[component];
}

double ParaScripting::MuJoCoSimulation::GetBodyParaQuaternion(int bodyId, int component) const
{
	if (!IsValid() || bodyId < 0 || bodyId >= m_impl->model->nbody || component < 0 || component >= 4)
		return 0.0;
	const mjtNum* quaternion = m_impl->data->xquat + 4 * bodyId;
	ParaEngine::RobotQuaternion converted = ParaEngine::RobotCoordinateConverter::MuJoCoQuaternionToParaEngine(quaternion[0], quaternion[1], quaternion[2], quaternion[3]);
	const double values[] = { converted.x, converted.y, converted.z, converted.w };
	return values[component];
}

double ParaScripting::MuJoCoSimulation::GetBodyLinearVelocity(int bodyId, int component) const
{
	// cvel layout: [wx, wy, wz, vx, vy, vz] per body (world frame about COM).
	if (!IsValid() || bodyId < 0 || bodyId >= m_impl->model->nbody || component < 0 || component >= 3)
		return 0.0;
	return m_impl->data->cvel[6 * bodyId + 3 + component];
}

double ParaScripting::MuJoCoSimulation::GetBodyAngularVelocity(int bodyId, int component) const
{
	if (!IsValid() || bodyId < 0 || bodyId >= m_impl->model->nbody || component < 0 || component >= 3)
		return 0.0;
	return m_impl->data->cvel[6 * bodyId + component];
}

double ParaScripting::MuJoCoSimulation::RaycastDown(double x, double y, double z) const
{
	if (!IsValid())
		return -1.0;
	const mjtNum origin[] = { x, y, z };
	const mjtNum direction[] = { 0.0, 0.0, -1.0 };
	const mjtByte geomGroup[] = { 1, 0, 0, 0, 0, 0 };
	int geomId = -1;
	return mj_ray(m_impl->model, m_impl->data, origin, direction, geomGroup, 1, -1, &geomId, NULL);
}

double ParaScripting::MuJoCoSimulation::GetBodyParaLinearVelocity(int bodyId, int component) const
{
	if (!IsValid() || bodyId < 0 || bodyId >= m_impl->model->nbody || component < 0 || component >= 3)
		return 0.0;
	const mjtNum* velocity = m_impl->data->cvel + 6 * bodyId + 3;
	ParaEngine::RobotVector3 converted = ParaEngine::RobotCoordinateConverter::MuJoCoPositionToParaEngine(velocity[0], velocity[1], velocity[2]);
	const double values[] = { converted.x, converted.y, converted.z };
	return values[component];
}

double ParaScripting::MuJoCoSimulation::GetBodyParaAngularVelocity(int bodyId, int component) const
{
	if (!IsValid() || bodyId < 0 || bodyId >= m_impl->model->nbody || component < 0 || component >= 3)
		return 0.0;
	const mjtNum* velocity = m_impl->data->cvel + 6 * bodyId;
	ParaEngine::RobotVector3 converted = ParaEngine::RobotCoordinateConverter::MuJoCoPositionToParaEngine(velocity[0], velocity[1], velocity[2]);
	const double values[] = { converted.x, converted.y, converted.z };
	return values[component];
}

int ParaScripting::MuJoCoSimulation::GetContactGeom1(int contactIndex) const
{
	if (!IsValid() || contactIndex < 0 || contactIndex >= m_impl->data->ncon)
		return -1;
	return m_impl->data->contact[contactIndex].geom1;
}

int ParaScripting::MuJoCoSimulation::GetContactGeom2(int contactIndex) const
{
	if (!IsValid() || contactIndex < 0 || contactIndex >= m_impl->data->ncon)
		return -1;
	return m_impl->data->contact[contactIndex].geom2;
}

int ParaScripting::MuJoCoSimulation::GetContactBody1(int contactIndex) const
{
	int geomId = GetContactGeom1(contactIndex);
	if (!IsValid() || geomId < 0 || geomId >= m_impl->model->ngeom)
		return -1;
	return m_impl->model->geom_bodyid[geomId];
}

int ParaScripting::MuJoCoSimulation::GetContactBody2(int contactIndex) const
{
	int geomId = GetContactGeom2(contactIndex);
	if (!IsValid() || geomId < 0 || geomId >= m_impl->model->ngeom)
		return -1;
	return m_impl->model->geom_bodyid[geomId];
}

double ParaScripting::MuJoCoSimulation::GetContactDist(int contactIndex) const
{
	if (!IsValid() || contactIndex < 0 || contactIndex >= m_impl->data->ncon)
		return 0.0;
	return m_impl->data->contact[contactIndex].dist;
}

double ParaScripting::MuJoCoSimulation::GetContactPosition(int contactIndex, int component) const
{
	if (!IsValid() || contactIndex < 0 || contactIndex >= m_impl->data->ncon || component < 0 || component >= 3)
		return 0.0;
	return m_impl->data->contact[contactIndex].pos[component];
}

double ParaScripting::MuJoCoSimulation::GetContactNormal(int contactIndex, int component) const
{
	if (!IsValid() || contactIndex < 0 || contactIndex >= m_impl->data->ncon || component < 0 || component >= 3)
		return 0.0;
	return m_impl->data->contact[contactIndex].frame[component];
}

double ParaScripting::MuJoCoSimulation::GetContactParaPosition(int contactIndex, int component) const
{
	if (!IsValid() || contactIndex < 0 || contactIndex >= m_impl->data->ncon || component < 0 || component >= 3)
		return 0.0;
	const mjtNum* position = m_impl->data->contact[contactIndex].pos;
	ParaEngine::RobotVector3 converted = ParaEngine::RobotCoordinateConverter::MuJoCoPositionToParaEngine(position[0], position[1], position[2]);
	const double values[] = { converted.x, converted.y, converted.z };
	return values[component];
}

double ParaScripting::MuJoCoSimulation::GetContactParaNormal(int contactIndex, int component) const
{
	if (!IsValid() || contactIndex < 0 || contactIndex >= m_impl->data->ncon || component < 0 || component >= 3)
		return 0.0;
	const mjtNum* normal = m_impl->data->contact[contactIndex].frame;
	ParaEngine::RobotVector3 converted = ParaEngine::RobotCoordinateConverter::MuJoCoPositionToParaEngine(normal[0], normal[1], normal[2]);
	const double values[] = { converted.x, converted.y, converted.z };
	return values[component];
}

double ParaScripting::MuJoCoSimulation::GetContactForce(int contactIndex, int component) const
{
	if (!IsValid() || contactIndex < 0 || contactIndex >= m_impl->data->ncon || component < 0 || component >= 3)
		return 0.0;
	mjtNum force[6] = { 0 };
	mj_contactForce(m_impl->model, m_impl->data, contactIndex, force);
	// Contact frame is row-major; first row is the normal. Rotate contact-frame force into world.
	const mjtNum* frame = m_impl->data->contact[contactIndex].frame;
	const double worldForce[3] = {
		frame[0] * force[0] + frame[3] * force[1] + frame[6] * force[2],
		frame[1] * force[0] + frame[4] * force[1] + frame[7] * force[2],
		frame[2] * force[0] + frame[5] * force[1] + frame[8] * force[2]
	};
	return worldForce[component];
}

double ParaScripting::MuJoCoSimulation::GetContactParaForce(int contactIndex, int component) const
{
	if (!IsValid() || contactIndex < 0 || contactIndex >= m_impl->data->ncon || component < 0 || component >= 3)
		return 0.0;
	const double forceX = GetContactForce(contactIndex, 0);
	const double forceY = GetContactForce(contactIndex, 1);
	const double forceZ = GetContactForce(contactIndex, 2);
	ParaEngine::RobotVector3 converted = ParaEngine::RobotCoordinateConverter::MuJoCoPositionToParaEngine(forceX, forceY, forceZ);
	const double values[] = { converted.x, converted.y, converted.z };
	return values[component];
}

int ParaScripting::MuJoCoSimulation::FindHField(const std::string& name) const
{
	return NameToId(mjOBJ_HFIELD, name);
}

int ParaScripting::MuJoCoSimulation::FindGeom(const std::string& name) const
{
	return NameToId(mjOBJ_GEOM, name);
}

int ParaScripting::MuJoCoSimulation::GetHFieldCount() const
{
	return IsValid() ? static_cast<int>(m_impl->model->nhfield) : 0;
}

int ParaScripting::MuJoCoSimulation::GetHFieldNRow(int hfieldId) const
{
	if (!IsValid() || hfieldId < 0 || hfieldId >= m_impl->model->nhfield)
		return 0;
	return m_impl->model->hfield_nrow[hfieldId];
}

int ParaScripting::MuJoCoSimulation::GetHFieldNCol(int hfieldId) const
{
	if (!IsValid() || hfieldId < 0 || hfieldId >= m_impl->model->nhfield)
		return 0;
	return m_impl->model->hfield_ncol[hfieldId];
}

double ParaScripting::MuJoCoSimulation::GetHFieldSize(int hfieldId, int component) const
{
	if (!IsValid() || hfieldId < 0 || hfieldId >= m_impl->model->nhfield || component < 0 || component >= 4)
		return 0.0;
	return m_impl->model->hfield_size[4 * hfieldId + component];
}

double ParaScripting::MuJoCoSimulation::GetHFieldElevation(int hfieldId, int row, int col) const
{
	if (!IsValid() || hfieldId < 0 || hfieldId >= m_impl->model->nhfield)
		return 0.0;
	const int nrow = m_impl->model->hfield_nrow[hfieldId];
	const int ncol = m_impl->model->hfield_ncol[hfieldId];
	if (row < 0 || row >= nrow || col < 0 || col >= ncol)
		return 0.0;
	return m_impl->model->hfield_data[m_impl->model->hfield_adr[hfieldId] + row * ncol + col];
}

bool ParaScripting::MuJoCoSimulation::SetHFieldElevation(int hfieldId, int row, int col, double value)
{
	if (!IsValid() || hfieldId < 0 || hfieldId >= m_impl->model->nhfield)
		return false;
	const int nrow = m_impl->model->hfield_nrow[hfieldId];
	const int ncol = m_impl->model->hfield_ncol[hfieldId];
	if (row < 0 || row >= nrow || col < 0 || col >= ncol)
		return false;
	if (value < 0.0)
		value = 0.0;
	if (value > 1.0)
		value = 1.0;
	m_impl->model->hfield_data[m_impl->model->hfield_adr[hfieldId] + row * ncol + col] = static_cast<float>(value);
	return true;
}

bool ParaScripting::MuJoCoSimulation::FillHFieldElevation(int hfieldId, double value)
{
	if (!IsValid() || hfieldId < 0 || hfieldId >= m_impl->model->nhfield)
		return false;
	if (value < 0.0)
		value = 0.0;
	if (value > 1.0)
		value = 1.0;
	const int nrow = m_impl->model->hfield_nrow[hfieldId];
	const int ncol = m_impl->model->hfield_ncol[hfieldId];
	float* data = m_impl->model->hfield_data + m_impl->model->hfield_adr[hfieldId];
	const float filled = static_cast<float>(value);
	for (int index = 0; index < nrow * ncol; ++index)
		data[index] = filled;
	return true;
}

double ParaScripting::MuJoCoSimulation::GetGeomPosition(int geomId, int component) const
{
	if (!IsValid() || geomId < 0 || geomId >= m_impl->model->ngeom || component < 0 || component >= 3)
		return 0.0;
	return m_impl->model->geom_pos[3 * geomId + component];
}

bool ParaScripting::MuJoCoSimulation::SetGeomPosition(int geomId, int component, double value)
{
	if (!IsValid() || geomId < 0 || geomId >= m_impl->model->ngeom || component < 0 || component >= 3)
		return false;
	m_impl->model->geom_pos[3 * geomId + component] = value;
	return true;
}

double ParaScripting::MuJoCoSimulation::GetGeomParaPosition(int geomId, int component) const
{
	if (!IsValid() || geomId < 0 || geomId >= m_impl->model->ngeom || component < 0 || component >= 3)
		return 0.0;
	const mjtNum* position = m_impl->model->geom_pos + 3 * geomId;
	ParaEngine::RobotVector3 converted = ParaEngine::RobotCoordinateConverter::MuJoCoPositionToParaEngine(position[0], position[1], position[2]);
	const double values[] = { converted.x, converted.y, converted.z };
	return values[component];
}

bool ParaScripting::MuJoCoSimulation::SetGeomParaPosition(int geomId, double x, double y, double z)
{
	if (!IsValid() || geomId < 0 || geomId >= m_impl->model->ngeom)
		return false;
	ParaEngine::RobotVector3 converted = ParaEngine::RobotCoordinateConverter::ParaEnginePositionToMuJoCo(x, y, z);
	m_impl->model->geom_pos[3 * geomId + 0] = converted.x;
	m_impl->model->geom_pos[3 * geomId + 1] = converted.y;
	m_impl->model->geom_pos[3 * geomId + 2] = converted.z;
	return true;
}

std::string ParaScripting::MuJoCoSimulation::GetBodyName(int bodyId) const
{
	if (!IsValid() || bodyId < 0 || bodyId >= m_impl->model->nbody)
		return std::string();
	const char* name = mj_id2name(m_impl->model, mjOBJ_BODY, bodyId);
	return name ? std::string(name) : std::string();
}

std::string ParaScripting::MuJoCoSimulation::GetJointName(int jointId) const
{
	if (!IsValid() || jointId < 0 || jointId >= m_impl->model->njnt)
		return std::string();
	const char* name = mj_id2name(m_impl->model, mjOBJ_JOINT, jointId);
	return name ? std::string(name) : std::string();
}

#endif

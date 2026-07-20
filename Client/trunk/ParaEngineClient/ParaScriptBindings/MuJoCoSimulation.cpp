#include "ParaScriptingMuJoCo.h"
#include "RobotCoordinateConverter.h"

#ifdef USE_MUJOCO

#include <mujoco/mujoco.h>

struct ParaScripting::MuJoCoSimulation::Impl
{
	Impl() : model(NULL), data(NULL) {}
	~Impl()
	{
		mj_deleteData(data);
		mj_deleteModel(model);
	}

	mjModel* model;
	mjData* data;
	std::string lastError;
};

ParaScripting::MuJoCoSimulation::MuJoCoSimulation() : m_impl(new Impl()) {}
ParaScripting::MuJoCoSimulation::~MuJoCoSimulation() {}

bool ParaScripting::MuJoCoSimulation::Load(const std::string& filename)
{
	char error[1024] = { 0 };
	mjModel* model = mj_loadXML(filename.c_str(), NULL, error, sizeof(error));
	if (!model)
	{
		m_impl->lastError = error;
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
	m_impl->lastError.clear();
	mj_forward(m_impl->model, m_impl->data);
	return true;
}

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
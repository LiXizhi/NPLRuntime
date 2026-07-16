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

#endif
#include "ParaEngine.h"

#ifdef USE_MUJOCO

#include "ParaScriptingMuJoCo.h"
#include "NPLScriptingState.h"

#include <luabind/luabind.hpp>
#include <map>
#include <mutex>

namespace
{
	typedef std::map<int, std::shared_ptr<ParaScripting::MuJoCoSimulation> > SimulationMap;

	SimulationMap& GetSimulations()
	{
		static SimulationMap simulations;
		return simulations;
	}

	std::mutex& GetSimulationsMutex()
	{
		static std::mutex simulationsMutex;
		return simulationsMutex;
	}

	std::string& GetLoadError()
	{
		static std::string loadError;
		return loadError;
	}

	int& GetNextHandle()
	{
		static int nextHandle = 1;
		return nextHandle;
	}

	std::shared_ptr<ParaScripting::MuJoCoSimulation> GetSimulation(int handle)
	{
		std::lock_guard<std::mutex> lock(GetSimulationsMutex());
		SimulationMap::iterator item = GetSimulations().find(handle);
		return item != GetSimulations().end() ? item->second : std::shared_ptr<ParaScripting::MuJoCoSimulation>();
	}
}

int ParaScripting::ParaMuJoCo::LoadModel(const std::string& filename)
{
	std::shared_ptr<MuJoCoSimulation> simulation(new MuJoCoSimulation());
	if (!simulation->Load(filename))
	{
		std::lock_guard<std::mutex> lock(GetSimulationsMutex());
		GetLoadError() = simulation->GetLastError();
		return 0;
	}
	std::lock_guard<std::mutex> lock(GetSimulationsMutex());
	int handle = GetNextHandle()++;
	GetSimulations()[handle] = simulation;
	GetLoadError().clear();
	return handle;
}

bool ParaScripting::ParaMuJoCo::DeleteModel(int handle) { std::lock_guard<std::mutex> lock(GetSimulationsMutex()); return GetSimulations().erase(handle) != 0; }
bool ParaScripting::ParaMuJoCo::IsValid(int handle) { std::shared_ptr<MuJoCoSimulation> simulation = GetSimulation(handle); return simulation && simulation->IsValid(); }
void ParaScripting::ParaMuJoCo::Reset(int handle) { std::shared_ptr<MuJoCoSimulation> simulation = GetSimulation(handle); if (simulation) simulation->Reset(); }
void ParaScripting::ParaMuJoCo::Forward(int handle) { std::shared_ptr<MuJoCoSimulation> simulation = GetSimulation(handle); if (simulation) simulation->Forward(); }
void ParaScripting::ParaMuJoCo::Step(int handle, int count) { std::shared_ptr<MuJoCoSimulation> simulation = GetSimulation(handle); if (simulation) simulation->Step(count); }
std::string ParaScripting::ParaMuJoCo::GetLastError(int handle) { std::shared_ptr<MuJoCoSimulation> simulation = GetSimulation(handle); if (simulation) return simulation->GetLastError(); std::lock_guard<std::mutex> lock(GetSimulationsMutex()); return handle == 0 ? GetLoadError() : "invalid MuJoCo handle"; }
int ParaScripting::ParaMuJoCo::GetQPosCount(int handle) { std::shared_ptr<MuJoCoSimulation> simulation = GetSimulation(handle); return simulation ? simulation->GetQPosCount() : 0; }
int ParaScripting::ParaMuJoCo::GetQVelCount(int handle) { std::shared_ptr<MuJoCoSimulation> simulation = GetSimulation(handle); return simulation ? simulation->GetQVelCount() : 0; }
int ParaScripting::ParaMuJoCo::GetControlCount(int handle) { std::shared_ptr<MuJoCoSimulation> simulation = GetSimulation(handle); return simulation ? simulation->GetControlCount() : 0; }
int ParaScripting::ParaMuJoCo::GetBodyCount(int handle) { std::shared_ptr<MuJoCoSimulation> simulation = GetSimulation(handle); return simulation ? simulation->GetBodyCount() : 0; }
int ParaScripting::ParaMuJoCo::GetJointCount(int handle) { std::shared_ptr<MuJoCoSimulation> simulation = GetSimulation(handle); return simulation ? simulation->GetJointCount() : 0; }
int ParaScripting::ParaMuJoCo::GetActuatorCount(int handle) { std::shared_ptr<MuJoCoSimulation> simulation = GetSimulation(handle); return simulation ? simulation->GetActuatorCount() : 0; }
int ParaScripting::ParaMuJoCo::GetContactCount(int handle) { std::shared_ptr<MuJoCoSimulation> simulation = GetSimulation(handle); return simulation ? simulation->GetContactCount() : 0; }
double ParaScripting::ParaMuJoCo::GetTime(int handle) { std::shared_ptr<MuJoCoSimulation> simulation = GetSimulation(handle); return simulation ? simulation->GetTime() : 0.0; }
double ParaScripting::ParaMuJoCo::GetQPos(int handle, int index) { std::shared_ptr<MuJoCoSimulation> simulation = GetSimulation(handle); return simulation ? simulation->GetQPos(index) : 0.0; }
bool ParaScripting::ParaMuJoCo::SetQPos(int handle, int index, double value) { std::shared_ptr<MuJoCoSimulation> simulation = GetSimulation(handle); return simulation && simulation->SetQPos(index, value); }
double ParaScripting::ParaMuJoCo::GetQVel(int handle, int index) { std::shared_ptr<MuJoCoSimulation> simulation = GetSimulation(handle); return simulation ? simulation->GetQVel(index) : 0.0; }
bool ParaScripting::ParaMuJoCo::SetQVel(int handle, int index, double value) { std::shared_ptr<MuJoCoSimulation> simulation = GetSimulation(handle); return simulation && simulation->SetQVel(index, value); }
double ParaScripting::ParaMuJoCo::GetControl(int handle, int index) { std::shared_ptr<MuJoCoSimulation> simulation = GetSimulation(handle); return simulation ? simulation->GetControl(index) : 0.0; }
bool ParaScripting::ParaMuJoCo::SetControl(int handle, int index, double value) { std::shared_ptr<MuJoCoSimulation> simulation = GetSimulation(handle); return simulation && simulation->SetControl(index, value); }
int ParaScripting::ParaMuJoCo::NameToId(int handle, int objectType, const std::string& name) { std::shared_ptr<MuJoCoSimulation> simulation = GetSimulation(handle); return simulation ? simulation->NameToId(objectType, name) : -1; }
int ParaScripting::ParaMuJoCo::FindBody(int handle, const std::string& name) { std::shared_ptr<MuJoCoSimulation> simulation = GetSimulation(handle); return simulation ? simulation->FindBody(name) : -1; }
int ParaScripting::ParaMuJoCo::FindJoint(int handle, const std::string& name) { std::shared_ptr<MuJoCoSimulation> simulation = GetSimulation(handle); return simulation ? simulation->FindJoint(name) : -1; }
int ParaScripting::ParaMuJoCo::FindActuator(int handle, const std::string& name) { std::shared_ptr<MuJoCoSimulation> simulation = GetSimulation(handle); return simulation ? simulation->FindActuator(name) : -1; }
double ParaScripting::ParaMuJoCo::GetBodyPosition(int handle, int bodyId, int component) { std::shared_ptr<MuJoCoSimulation> simulation = GetSimulation(handle); return simulation ? simulation->GetBodyPosition(bodyId, component) : 0.0; }
double ParaScripting::ParaMuJoCo::GetBodyQuaternion(int handle, int bodyId, int component) { std::shared_ptr<MuJoCoSimulation> simulation = GetSimulation(handle); return simulation ? simulation->GetBodyQuaternion(bodyId, component) : 0.0; }
double ParaScripting::ParaMuJoCo::GetBodyParaPosition(int handle, int bodyId, int component) { std::shared_ptr<MuJoCoSimulation> simulation = GetSimulation(handle); return simulation ? simulation->GetBodyParaPosition(bodyId, component) : 0.0; }
double ParaScripting::ParaMuJoCo::GetBodyParaQuaternion(int handle, int bodyId, int component) { std::shared_ptr<MuJoCoSimulation> simulation = GetSimulation(handle); return simulation ? simulation->GetBodyParaQuaternion(bodyId, component) : 0.0; }

void ParaScripting::CNPLScriptingState::LoadHAPI_MuJoCo()
{
	using namespace luabind;
	module(GetLuaState())
	[
		namespace_("ParaMuJoCo")
		[
			def("LoadModel", &ParaMuJoCo::LoadModel),
			def("DeleteModel", &ParaMuJoCo::DeleteModel),
			def("IsValid", &ParaMuJoCo::IsValid),
			def("Reset", &ParaMuJoCo::Reset),
			def("Forward", &ParaMuJoCo::Forward),
			def("Step", &ParaMuJoCo::Step),
			def("GetLastError", &ParaMuJoCo::GetLastError),
			def("GetQPosCount", &ParaMuJoCo::GetQPosCount),
			def("GetQVelCount", &ParaMuJoCo::GetQVelCount),
			def("GetControlCount", &ParaMuJoCo::GetControlCount),
			def("GetBodyCount", &ParaMuJoCo::GetBodyCount),
			def("GetJointCount", &ParaMuJoCo::GetJointCount),
			def("GetActuatorCount", &ParaMuJoCo::GetActuatorCount),
			def("GetContactCount", &ParaMuJoCo::GetContactCount),
			def("GetTime", &ParaMuJoCo::GetTime),
			def("GetQPos", &ParaMuJoCo::GetQPos),
			def("SetQPos", &ParaMuJoCo::SetQPos),
			def("GetQVel", &ParaMuJoCo::GetQVel),
			def("SetQVel", &ParaMuJoCo::SetQVel),
			def("GetControl", &ParaMuJoCo::GetControl),
			def("SetControl", &ParaMuJoCo::SetControl),
			def("NameToId", &ParaMuJoCo::NameToId),
			def("FindBody", &ParaMuJoCo::FindBody),
			def("FindJoint", &ParaMuJoCo::FindJoint),
			def("FindActuator", &ParaMuJoCo::FindActuator),
			def("GetBodyPosition", &ParaMuJoCo::GetBodyPosition),
			def("GetBodyQuaternion", &ParaMuJoCo::GetBodyQuaternion),
			def("GetBodyParaPosition", &ParaMuJoCo::GetBodyParaPosition),
			def("GetBodyParaQuaternion", &ParaMuJoCo::GetBodyParaQuaternion)
		]
	];
}

#endif
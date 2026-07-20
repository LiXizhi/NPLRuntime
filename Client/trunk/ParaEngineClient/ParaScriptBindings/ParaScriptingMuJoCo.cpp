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
int ParaScripting::ParaMuJoCo::GetJointQPosAdr(int handle, int jointId) { std::shared_ptr<MuJoCoSimulation> simulation = GetSimulation(handle); return simulation ? simulation->GetJointQPosAdr(jointId) : -1; }
int ParaScripting::ParaMuJoCo::GetJointDofAdr(int handle, int jointId) { std::shared_ptr<MuJoCoSimulation> simulation = GetSimulation(handle); return simulation ? simulation->GetJointDofAdr(jointId) : -1; }
double ParaScripting::ParaMuJoCo::GetBodyPosition(int handle, int bodyId, int component) { std::shared_ptr<MuJoCoSimulation> simulation = GetSimulation(handle); return simulation ? simulation->GetBodyPosition(bodyId, component) : 0.0; }
double ParaScripting::ParaMuJoCo::GetBodyQuaternion(int handle, int bodyId, int component) { std::shared_ptr<MuJoCoSimulation> simulation = GetSimulation(handle); return simulation ? simulation->GetBodyQuaternion(bodyId, component) : 0.0; }
double ParaScripting::ParaMuJoCo::GetBodyParaPosition(int handle, int bodyId, int component) { std::shared_ptr<MuJoCoSimulation> simulation = GetSimulation(handle); return simulation ? simulation->GetBodyParaPosition(bodyId, component) : 0.0; }
double ParaScripting::ParaMuJoCo::GetBodyParaQuaternion(int handle, int bodyId, int component) { std::shared_ptr<MuJoCoSimulation> simulation = GetSimulation(handle); return simulation ? simulation->GetBodyParaQuaternion(bodyId, component) : 0.0; }
double ParaScripting::ParaMuJoCo::GetBodyLinearVelocity(int handle, int bodyId, int component) { std::shared_ptr<MuJoCoSimulation> simulation = GetSimulation(handle); return simulation ? simulation->GetBodyLinearVelocity(bodyId, component) : 0.0; }
double ParaScripting::ParaMuJoCo::GetBodyAngularVelocity(int handle, int bodyId, int component) { std::shared_ptr<MuJoCoSimulation> simulation = GetSimulation(handle); return simulation ? simulation->GetBodyAngularVelocity(bodyId, component) : 0.0; }
double ParaScripting::ParaMuJoCo::GetBodyParaLinearVelocity(int handle, int bodyId, int component) { std::shared_ptr<MuJoCoSimulation> simulation = GetSimulation(handle); return simulation ? simulation->GetBodyParaLinearVelocity(bodyId, component) : 0.0; }
double ParaScripting::ParaMuJoCo::GetBodyParaAngularVelocity(int handle, int bodyId, int component) { std::shared_ptr<MuJoCoSimulation> simulation = GetSimulation(handle); return simulation ? simulation->GetBodyParaAngularVelocity(bodyId, component) : 0.0; }
int ParaScripting::ParaMuJoCo::GetContactGeom1(int handle, int contactIndex) { std::shared_ptr<MuJoCoSimulation> simulation = GetSimulation(handle); return simulation ? simulation->GetContactGeom1(contactIndex) : -1; }
int ParaScripting::ParaMuJoCo::GetContactGeom2(int handle, int contactIndex) { std::shared_ptr<MuJoCoSimulation> simulation = GetSimulation(handle); return simulation ? simulation->GetContactGeom2(contactIndex) : -1; }
int ParaScripting::ParaMuJoCo::GetContactBody1(int handle, int contactIndex) { std::shared_ptr<MuJoCoSimulation> simulation = GetSimulation(handle); return simulation ? simulation->GetContactBody1(contactIndex) : -1; }
int ParaScripting::ParaMuJoCo::GetContactBody2(int handle, int contactIndex) { std::shared_ptr<MuJoCoSimulation> simulation = GetSimulation(handle); return simulation ? simulation->GetContactBody2(contactIndex) : -1; }
double ParaScripting::ParaMuJoCo::GetContactDist(int handle, int contactIndex) { std::shared_ptr<MuJoCoSimulation> simulation = GetSimulation(handle); return simulation ? simulation->GetContactDist(contactIndex) : 0.0; }
double ParaScripting::ParaMuJoCo::GetContactPosition(int handle, int contactIndex, int component) { std::shared_ptr<MuJoCoSimulation> simulation = GetSimulation(handle); return simulation ? simulation->GetContactPosition(contactIndex, component) : 0.0; }
double ParaScripting::ParaMuJoCo::GetContactNormal(int handle, int contactIndex, int component) { std::shared_ptr<MuJoCoSimulation> simulation = GetSimulation(handle); return simulation ? simulation->GetContactNormal(contactIndex, component) : 0.0; }
double ParaScripting::ParaMuJoCo::GetContactParaPosition(int handle, int contactIndex, int component) { std::shared_ptr<MuJoCoSimulation> simulation = GetSimulation(handle); return simulation ? simulation->GetContactParaPosition(contactIndex, component) : 0.0; }
double ParaScripting::ParaMuJoCo::GetContactParaNormal(int handle, int contactIndex, int component) { std::shared_ptr<MuJoCoSimulation> simulation = GetSimulation(handle); return simulation ? simulation->GetContactParaNormal(contactIndex, component) : 0.0; }
double ParaScripting::ParaMuJoCo::GetContactForce(int handle, int contactIndex, int component) { std::shared_ptr<MuJoCoSimulation> simulation = GetSimulation(handle); return simulation ? simulation->GetContactForce(contactIndex, component) : 0.0; }
double ParaScripting::ParaMuJoCo::GetContactParaForce(int handle, int contactIndex, int component) { std::shared_ptr<MuJoCoSimulation> simulation = GetSimulation(handle); return simulation ? simulation->GetContactParaForce(contactIndex, component) : 0.0; }
int ParaScripting::ParaMuJoCo::FindHField(int handle, const std::string& name) { std::shared_ptr<MuJoCoSimulation> simulation = GetSimulation(handle); return simulation ? simulation->FindHField(name) : -1; }
int ParaScripting::ParaMuJoCo::FindGeom(int handle, const std::string& name) { std::shared_ptr<MuJoCoSimulation> simulation = GetSimulation(handle); return simulation ? simulation->FindGeom(name) : -1; }
int ParaScripting::ParaMuJoCo::GetHFieldCount(int handle) { std::shared_ptr<MuJoCoSimulation> simulation = GetSimulation(handle); return simulation ? simulation->GetHFieldCount() : 0; }
int ParaScripting::ParaMuJoCo::GetHFieldNRow(int handle, int hfieldId) { std::shared_ptr<MuJoCoSimulation> simulation = GetSimulation(handle); return simulation ? simulation->GetHFieldNRow(hfieldId) : 0; }
int ParaScripting::ParaMuJoCo::GetHFieldNCol(int handle, int hfieldId) { std::shared_ptr<MuJoCoSimulation> simulation = GetSimulation(handle); return simulation ? simulation->GetHFieldNCol(hfieldId) : 0; }
double ParaScripting::ParaMuJoCo::GetHFieldSize(int handle, int hfieldId, int component) { std::shared_ptr<MuJoCoSimulation> simulation = GetSimulation(handle); return simulation ? simulation->GetHFieldSize(hfieldId, component) : 0.0; }
double ParaScripting::ParaMuJoCo::GetHFieldElevation(int handle, int hfieldId, int row, int col) { std::shared_ptr<MuJoCoSimulation> simulation = GetSimulation(handle); return simulation ? simulation->GetHFieldElevation(hfieldId, row, col) : 0.0; }
bool ParaScripting::ParaMuJoCo::SetHFieldElevation(int handle, int hfieldId, int row, int col, double value) { std::shared_ptr<MuJoCoSimulation> simulation = GetSimulation(handle); return simulation && simulation->SetHFieldElevation(hfieldId, row, col, value); }
bool ParaScripting::ParaMuJoCo::FillHFieldElevation(int handle, int hfieldId, double value) { std::shared_ptr<MuJoCoSimulation> simulation = GetSimulation(handle); return simulation && simulation->FillHFieldElevation(hfieldId, value); }
double ParaScripting::ParaMuJoCo::GetGeomPosition(int handle, int geomId, int component) { std::shared_ptr<MuJoCoSimulation> simulation = GetSimulation(handle); return simulation ? simulation->GetGeomPosition(geomId, component) : 0.0; }
bool ParaScripting::ParaMuJoCo::SetGeomPosition(int handle, int geomId, int component, double value) { std::shared_ptr<MuJoCoSimulation> simulation = GetSimulation(handle); return simulation && simulation->SetGeomPosition(geomId, component, value); }
double ParaScripting::ParaMuJoCo::GetGeomParaPosition(int handle, int geomId, int component) { std::shared_ptr<MuJoCoSimulation> simulation = GetSimulation(handle); return simulation ? simulation->GetGeomParaPosition(geomId, component) : 0.0; }
bool ParaScripting::ParaMuJoCo::SetGeomParaPosition(int handle, int geomId, double x, double y, double z) { std::shared_ptr<MuJoCoSimulation> simulation = GetSimulation(handle); return simulation && simulation->SetGeomParaPosition(geomId, x, y, z); }

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
			def("GetJointQPosAdr", &ParaMuJoCo::GetJointQPosAdr),
			def("GetJointDofAdr", &ParaMuJoCo::GetJointDofAdr),
			def("GetBodyPosition", &ParaMuJoCo::GetBodyPosition),
			def("GetBodyQuaternion", &ParaMuJoCo::GetBodyQuaternion),
			def("GetBodyParaPosition", &ParaMuJoCo::GetBodyParaPosition),
			def("GetBodyParaQuaternion", &ParaMuJoCo::GetBodyParaQuaternion),
			def("GetBodyLinearVelocity", &ParaMuJoCo::GetBodyLinearVelocity),
			def("GetBodyAngularVelocity", &ParaMuJoCo::GetBodyAngularVelocity),
			def("GetBodyParaLinearVelocity", &ParaMuJoCo::GetBodyParaLinearVelocity),
			def("GetBodyParaAngularVelocity", &ParaMuJoCo::GetBodyParaAngularVelocity),
			def("GetContactGeom1", &ParaMuJoCo::GetContactGeom1),
			def("GetContactGeom2", &ParaMuJoCo::GetContactGeom2),
			def("GetContactBody1", &ParaMuJoCo::GetContactBody1),
			def("GetContactBody2", &ParaMuJoCo::GetContactBody2),
			def("GetContactDist", &ParaMuJoCo::GetContactDist),
			def("GetContactPosition", &ParaMuJoCo::GetContactPosition),
			def("GetContactNormal", &ParaMuJoCo::GetContactNormal),
			def("GetContactParaPosition", &ParaMuJoCo::GetContactParaPosition),
			def("GetContactParaNormal", &ParaMuJoCo::GetContactParaNormal),
			def("GetContactForce", &ParaMuJoCo::GetContactForce),
			def("GetContactParaForce", &ParaMuJoCo::GetContactParaForce),
			def("FindHField", &ParaMuJoCo::FindHField),
			def("FindGeom", &ParaMuJoCo::FindGeom),
			def("GetHFieldCount", &ParaMuJoCo::GetHFieldCount),
			def("GetHFieldNRow", &ParaMuJoCo::GetHFieldNRow),
			def("GetHFieldNCol", &ParaMuJoCo::GetHFieldNCol),
			def("GetHFieldSize", &ParaMuJoCo::GetHFieldSize),
			def("GetHFieldElevation", &ParaMuJoCo::GetHFieldElevation),
			def("SetHFieldElevation", &ParaMuJoCo::SetHFieldElevation),
			def("FillHFieldElevation", &ParaMuJoCo::FillHFieldElevation),
			def("GetGeomPosition", &ParaMuJoCo::GetGeomPosition),
			def("SetGeomPosition", &ParaMuJoCo::SetGeomPosition),
			def("GetGeomParaPosition", &ParaMuJoCo::GetGeomParaPosition),
			def("SetGeomParaPosition", &ParaMuJoCo::SetGeomParaPosition)
		]
	];
}

#endif
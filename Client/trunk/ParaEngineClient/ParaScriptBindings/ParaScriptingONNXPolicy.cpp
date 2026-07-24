#include "ParaEngine.h"

#ifdef USE_ONNXRUNTIME

#include "ONNXPolicySession.h"
#include "ParaScriptingONNXPolicy.h"
#include "NPLScriptingState.h"

#include <luabind/luabind.hpp>
#include <mutex>
#include <vector>

namespace
{
	ParaScripting::ONNXPolicySession& GetPolicySession()
	{
		static ParaScripting::ONNXPolicySession session;
		return session;
	}

	std::mutex& GetPolicyMutex()
	{
		static std::mutex policyMutex;
		return policyMutex;
	}
}

bool ParaScripting::ParaONNXPolicy::LoadModel(const std::string& filename, int inputCount, int outputCount)
{
	std::lock_guard<std::mutex> lock(GetPolicyMutex());
	return GetPolicySession().Load(filename, inputCount, outputCount);
}

void ParaScripting::ParaONNXPolicy::UnloadModel()
{
	std::lock_guard<std::mutex> lock(GetPolicyMutex());
	GetPolicySession().Unload();
}

void ParaScripting::ParaONNXPolicy::ResetState()
{
	std::lock_guard<std::mutex> lock(GetPolicyMutex());
	GetPolicySession().ResetState();
}

bool ParaScripting::ParaONNXPolicy::IsLoaded()
{
	std::lock_guard<std::mutex> lock(GetPolicyMutex());
	return GetPolicySession().IsLoaded();
}

bool ParaScripting::ParaONNXPolicy::Infer(const luabind::object& input, const luabind::object& output)
{
	if (luabind::type(input) != LUA_TTABLE || luabind::type(output) != LUA_TTABLE)
		return false;
	std::vector<float> inputValues;
	for (int index = 1; ; ++index)
	{
		luabind::object value = input[index];
		if (luabind::type(value) == LUA_TNIL)
			break;
		if (luabind::type(value) != LUA_TNUMBER)
			return false;
		inputValues.push_back(luabind::object_cast<float>(value));
	}
	std::vector<float> outputValues;
	std::lock_guard<std::mutex> lock(GetPolicyMutex());
	if (!GetPolicySession().Infer(inputValues, outputValues))
		return false;
	for (size_t index = 0; index < outputValues.size(); ++index)
		output[static_cast<int>(index + 1)] = outputValues[index];
	return true;
}

std::string ParaScripting::ParaONNXPolicy::GetLastError()
{
	std::lock_guard<std::mutex> lock(GetPolicyMutex());
	return GetPolicySession().GetLastError();
}

void ParaScripting::CNPLScriptingState::LoadHAPI_ONNXPolicy()
{
	using namespace luabind;
	module(GetLuaState())
	[
		namespace_("ParaONNXPolicy")
		[
			def("LoadModel", &ParaONNXPolicy::LoadModel),
			def("UnloadModel", &ParaONNXPolicy::UnloadModel),
			def("ResetState", &ParaONNXPolicy::ResetState),
			def("IsLoaded", &ParaONNXPolicy::IsLoaded),
			def("Infer", &ParaONNXPolicy::Infer),
			def("GetLastError", &ParaONNXPolicy::GetLastError)
		]
	];
}

#endif
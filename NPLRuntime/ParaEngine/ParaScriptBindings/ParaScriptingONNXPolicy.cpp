#include "ParaEngine.h"

#ifdef USE_ONNXRUNTIME

#include "ONNXPolicySession.h"
#include "ParaScriptingONNXPolicy.h"
#include "NPLScriptingState.h"

#include <luabind/luabind.hpp>
#include <limits>
#include <memory>
#include <mutex>
#include <unordered_map>
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

	struct PolicyEntry
	{
		ParaScripting::ONNXPolicySession session;
		std::mutex mutex;
	};

	std::unordered_map<int, std::shared_ptr<PolicyEntry>>& GetPolicyRegistry()
	{
		static std::unordered_map<int, std::shared_ptr<PolicyEntry>> registry;
		return registry;
	}

	std::mutex& GetPolicyRegistryMutex()
	{
		static std::mutex registryMutex;
		return registryMutex;
	}

	int& GetNextPolicyHandle()
	{
		static int nextHandle = 1;
		return nextHandle;
	}

	std::string& GetPolicyCreateError()
	{
		static std::string createError;
		return createError;
	}

	std::shared_ptr<PolicyEntry> FindPolicyEntry(int handle)
	{
		std::lock_guard<std::mutex> lock(GetPolicyRegistryMutex());
		auto iterator = GetPolicyRegistry().find(handle);
		return iterator == GetPolicyRegistry().end() ? nullptr : iterator->second;
	}

	bool ReadInputValues(const luabind::object& input, std::vector<float>& inputValues)
	{
		if (luabind::type(input) != LUA_TTABLE)
			return false;
		for (int index = 1; ; ++index)
		{
			luabind::object value = input[index];
			if (luabind::type(value) == LUA_TNIL)
				break;
			if (luabind::type(value) != LUA_TNUMBER)
				return false;
			inputValues.push_back(luabind::object_cast<float>(value));
		}
		return true;
	}

	void WriteOutputValues(const luabind::object& output, const std::vector<float>& outputValues)
	{
		for (size_t index = 0; index < outputValues.size(); ++index)
			output[static_cast<int>(index + 1)] = outputValues[index];
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
	if (luabind::type(output) != LUA_TTABLE)
		return false;
	std::vector<float> inputValues;
	if (!ReadInputValues(input, inputValues))
		return false;
	std::vector<float> outputValues;
	std::lock_guard<std::mutex> lock(GetPolicyMutex());
	if (!GetPolicySession().Infer(inputValues, outputValues))
		return false;
	WriteOutputValues(output, outputValues);
	return true;
}

std::string ParaScripting::ParaONNXPolicy::GetLastError()
{
	std::lock_guard<std::mutex> lock(GetPolicyMutex());
	return GetPolicySession().GetLastError();
}

int ParaScripting::ParaONNXPolicy::CreateModel(const std::string& filename, int inputCount, int outputCount)
{
	auto entry = std::make_shared<PolicyEntry>();
	if (!entry->session.Load(filename, inputCount, outputCount))
	{
		std::lock_guard<std::mutex> lock(GetPolicyRegistryMutex());
		GetPolicyCreateError() = entry->session.GetLastError();
		return 0;
	}
	std::lock_guard<std::mutex> lock(GetPolicyRegistryMutex());
	int& nextHandle = GetNextPolicyHandle();
	if (nextHandle <= 0 || nextHandle == std::numeric_limits<int>::max())
	{
		GetPolicyCreateError() = "ONNX policy handle space exhausted";
		return 0;
	}
	const int handle = nextHandle++;
	GetPolicyRegistry()[handle] = entry;
	GetPolicyCreateError().clear();
	return handle;
}

void ParaScripting::ParaONNXPolicy::DestroyModel(int handle)
{
	std::lock_guard<std::mutex> lock(GetPolicyRegistryMutex());
	GetPolicyRegistry().erase(handle);
}

void ParaScripting::ParaONNXPolicy::ResetModelState(int handle)
{
	auto entry = FindPolicyEntry(handle);
	if (!entry)
		return;
	std::lock_guard<std::mutex> lock(entry->mutex);
	entry->session.ResetState();
}

bool ParaScripting::ParaONNXPolicy::IsModelLoaded(int handle)
{
	auto entry = FindPolicyEntry(handle);
	if (!entry)
		return false;
	std::lock_guard<std::mutex> lock(entry->mutex);
	return entry->session.IsLoaded();
}

bool ParaScripting::ParaONNXPolicy::InferModel(int handle, const luabind::object& input, const luabind::object& output)
{
	if (luabind::type(output) != LUA_TTABLE)
		return false;
	std::vector<float> inputValues;
	if (!ReadInputValues(input, inputValues))
		return false;
	auto entry = FindPolicyEntry(handle);
	if (!entry)
		return false;
	std::vector<float> outputValues;
	std::lock_guard<std::mutex> lock(entry->mutex);
	if (!entry->session.Infer(inputValues, outputValues))
		return false;
	WriteOutputValues(output, outputValues);
	return true;
}

std::string ParaScripting::ParaONNXPolicy::GetModelLastError(int handle)
{
	auto entry = FindPolicyEntry(handle);
	if (!entry)
		return "ONNX policy handle is invalid";
	std::lock_guard<std::mutex> lock(entry->mutex);
	return entry->session.GetLastError();
}

std::string ParaScripting::ParaONNXPolicy::GetLastCreateError()
{
	std::lock_guard<std::mutex> lock(GetPolicyRegistryMutex());
	return GetPolicyCreateError();
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
			def("GetLastError", &ParaONNXPolicy::GetLastError),
			def("CreateModel", &ParaONNXPolicy::CreateModel),
			def("DestroyModel", &ParaONNXPolicy::DestroyModel),
			def("ResetModelState", &ParaONNXPolicy::ResetModelState),
			def("IsModelLoaded", &ParaONNXPolicy::IsModelLoaded),
			def("InferModel", &ParaONNXPolicy::InferModel),
			def("GetModelLastError", &ParaONNXPolicy::GetModelLastError),
			def("GetLastCreateError", &ParaONNXPolicy::GetLastCreateError)
		]
	];
}

#endif
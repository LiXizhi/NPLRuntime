#pragma once

#ifdef USE_ONNXRUNTIME

#include <luabind/object.hpp>
#include <string>

namespace ParaScripting
{
	class ParaONNXPolicy
	{
	public:
		static bool LoadModel(const std::string& filename, int inputCount, int outputCount);
		static void UnloadModel();
		static void ResetState();
		static bool IsLoaded();
		static bool Infer(const luabind::object& input, const luabind::object& output);
		static std::string GetLastError();
		static int CreateModel(const std::string& filename, int inputCount, int outputCount);
		static void DestroyModel(int handle);
		static void ResetModelState(int handle);
		static bool IsModelLoaded(int handle);
		static bool InferModel(int handle, const luabind::object& input, const luabind::object& output);
		static std::string GetModelLastError(int handle);
		static std::string GetLastCreateError();
	};
}

#endif
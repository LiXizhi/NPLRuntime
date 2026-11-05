#pragma once

#ifdef USE_ONNXRUNTIME

#include <memory>
#include <string>
#include <vector>

namespace ParaScripting
{
	class ONNXPolicySession
	{
	public:
		ONNXPolicySession();
		~ONNXPolicySession();

		bool Load(const std::string& filename, int inputCount, int outputCount);
		void Unload();
		void ResetState();
		bool IsLoaded() const;
		bool Infer(const std::vector<float>& input, std::vector<float>& output);
		const std::string& GetLastError() const;

	private:
		struct Impl;
		std::unique_ptr<Impl> m_impl;
	};
}

#endif
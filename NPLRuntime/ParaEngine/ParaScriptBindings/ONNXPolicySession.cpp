#include "ONNXPolicySession.h"

#ifdef USE_ONNXRUNTIME

#include <onnxruntime_cxx_api.h>
#include "RobotResourcePath.h"

#include <cmath>
#include <cstdint>
#include <limits>

namespace ParaScripting
{
	struct ONNXPolicySession::Impl
	{
		Impl() : env(ORT_LOGGING_LEVEL_WARNING, "ParaEnginePolicy"), inputCount(0), outputCount(0), recurrent(false) {}

		Ort::Env env;
		std::unique_ptr<Ort::Session> session;
		std::vector<std::string> inputNames;
		std::vector<std::string> outputNames;
		std::vector<float> hiddenState;
		std::vector<float> cellState;
		std::vector<int64_t> stateShape;
		std::string lastError;
		int inputCount;
		int outputCount;
		bool recurrent;
	};

	ONNXPolicySession::ONNXPolicySession() : m_impl(new Impl()) {}
	ONNXPolicySession::~ONNXPolicySession() {}

	bool ONNXPolicySession::Load(const std::string& filename, int inputCount, int outputCount)
	{
		Unload();
		try
		{
			Ort::SessionOptions options;
			options.SetExecutionMode(ExecutionMode::ORT_SEQUENTIAL);
			options.SetIntraOpNumThreads(1);
			options.SetInterOpNumThreads(1);
			options.SetGraphOptimizationLevel(GraphOptimizationLevel::ORT_ENABLE_ALL);
#ifdef _WIN32
			const std::wstring wideFilename = RobotWidePath(filename);
			m_impl->session.reset(new Ort::Session(m_impl->env, wideFilename.c_str(), options));
#else
			m_impl->session.reset(new Ort::Session(m_impl->env, filename.c_str(), options));
#endif

			const size_t modelInputCount = m_impl->session->GetInputCount();
			const size_t modelOutputCount = m_impl->session->GetOutputCount();
			if (!((modelInputCount == 1 && modelOutputCount == 1) || (modelInputCount == 3 && modelOutputCount == 3)))
				throw std::runtime_error("policy must be stateless 1-in/1-out or LSTM 3-in/3-out");
			m_impl->recurrent = modelInputCount == 3;

			Ort::TypeInfo inputType = m_impl->session->GetInputTypeInfo(0);
			Ort::TypeInfo outputType = m_impl->session->GetOutputTypeInfo(0);
			auto inputInfo = inputType.GetTensorTypeAndShapeInfo();
			auto outputInfo = outputType.GetTensorTypeAndShapeInfo();
			std::vector<int64_t> inputShape = inputInfo.GetShape();
			std::vector<int64_t> outputShape = outputInfo.GetShape();
			if (inputInfo.GetElementType() != ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT || inputShape.size() != 2 || inputShape[0] != 1 || inputShape[1] != inputCount)
				throw std::runtime_error("policy input must be float32 [1,inputCount]");
			if (outputInfo.GetElementType() != ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT || outputShape.size() != 2 || outputShape[0] != 1 || outputShape[1] != outputCount)
				throw std::runtime_error("policy output must be float32 [1,outputCount]");
			if (m_impl->recurrent)
			{
				std::vector<int64_t> expectedStateShape;
				for (size_t index = 1; index < 3; ++index)
				{
					Ort::TypeInfo stateInputType = m_impl->session->GetInputTypeInfo(index);
					Ort::TypeInfo stateOutputType = m_impl->session->GetOutputTypeInfo(index);
					auto stateInputInfo = stateInputType.GetTensorTypeAndShapeInfo();
					auto stateOutputInfo = stateOutputType.GetTensorTypeAndShapeInfo();
					std::vector<int64_t> stateInputShape = stateInputInfo.GetShape();
					std::vector<int64_t> stateOutputShape = stateOutputInfo.GetShape();
					if (stateInputInfo.GetElementType() != ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT || stateInputShape.size() != 3 ||
						stateInputShape[0] <= 0 || stateInputShape[1] <= 0 || stateInputShape[2] <= 0)
						throw std::runtime_error("LSTM state input must be a fixed float32 rank-3 tensor");
					if (stateOutputInfo.GetElementType() != ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT || stateOutputShape != stateInputShape)
						throw std::runtime_error("LSTM state output must match its input shape and type");
					if (index == 1)
						expectedStateShape = stateInputShape;
					else if (stateInputShape != expectedStateShape)
						throw std::runtime_error("LSTM hidden and cell state shapes must match");
				}
				m_impl->stateShape = expectedStateShape;
			}

			Ort::AllocatorWithDefaultOptions allocator;
			for (size_t index = 0; index < modelInputCount; ++index)
			{
				Ort::AllocatedStringPtr name = m_impl->session->GetInputNameAllocated(index, allocator);
				m_impl->inputNames.push_back(name.get());
			}
			for (size_t index = 0; index < modelOutputCount; ++index)
			{
				Ort::AllocatedStringPtr name = m_impl->session->GetOutputNameAllocated(index, allocator);
				m_impl->outputNames.push_back(name.get());
			}
			m_impl->inputCount = inputCount;
			m_impl->outputCount = outputCount;
			ResetState();
			return true;
		}
		catch (const std::exception& error)
		{
			m_impl->session.reset();
			m_impl->lastError = error.what();
			return false;
		}
	}

	void ONNXPolicySession::Unload()
	{
		m_impl->session.reset();
		m_impl->inputNames.clear();
		m_impl->outputNames.clear();
		m_impl->hiddenState.clear();
		m_impl->cellState.clear();
		m_impl->stateShape.clear();
		m_impl->lastError.clear();
		m_impl->inputCount = 0;
		m_impl->outputCount = 0;
		m_impl->recurrent = false;
	}

	void ONNXPolicySession::ResetState()
	{
		if (m_impl->recurrent)
		{
			size_t stateSize = 1;
			for (size_t index = 0; index < m_impl->stateShape.size(); ++index)
				stateSize *= static_cast<size_t>(m_impl->stateShape[index]);
			m_impl->hiddenState.assign(stateSize, 0.0f);
			m_impl->cellState.assign(stateSize, 0.0f);
		}
	}

	bool ONNXPolicySession::IsLoaded() const { return m_impl->session.get() != 0; }

	bool ONNXPolicySession::Infer(const std::vector<float>& input, std::vector<float>& output)
	{
		if (!m_impl->session)
		{
			m_impl->lastError = "no ONNX policy is loaded";
			return false;
		}
		if (static_cast<int>(input.size()) != m_impl->inputCount)
		{
			m_impl->lastError = "invalid policy input count";
			return false;
		}
		for (size_t i = 0; i < input.size(); ++i)
		{
			if (!std::isfinite(input[i]))
			{
				m_impl->lastError = "policy input contains NaN or infinity";
				return false;
			}
		}

		try
		{
			std::vector<float> inputBuffer(input);
			std::vector<int64_t> inputShape(2);
			inputShape[0] = 1;
			inputShape[1] = m_impl->inputCount;
			Ort::MemoryInfo memoryInfo = Ort::MemoryInfo::CreateCpu(OrtArenaAllocator, OrtMemTypeDefault);
			Ort::Value inputTensor = Ort::Value::CreateTensor<float>(memoryInfo, inputBuffer.data(), inputBuffer.size(), inputShape.data(), inputShape.size());
			std::vector<const char*> inputNames;
			std::vector<const char*> outputNames;
			for (size_t index = 0; index < m_impl->inputNames.size(); ++index)
				inputNames.push_back(m_impl->inputNames[index].c_str());
			for (size_t index = 0; index < m_impl->outputNames.size(); ++index)
				outputNames.push_back(m_impl->outputNames[index].c_str());
			std::vector<Ort::Value> inputs;
			inputs.push_back(std::move(inputTensor));
			if (m_impl->recurrent)
			{
				inputs.push_back(Ort::Value::CreateTensor<float>(memoryInfo, m_impl->hiddenState.data(), m_impl->hiddenState.size(), m_impl->stateShape.data(), m_impl->stateShape.size()));
				inputs.push_back(Ort::Value::CreateTensor<float>(memoryInfo, m_impl->cellState.data(), m_impl->cellState.size(), m_impl->stateShape.data(), m_impl->stateShape.size()));
			}
			std::vector<Ort::Value> outputs = m_impl->session->Run(Ort::RunOptions{ 0 }, inputNames.data(), inputs.data(), inputs.size(), outputNames.data(), outputNames.size());
			if (outputs.size() != outputNames.size() || !outputs[0].IsTensor())
				throw std::runtime_error("policy inference returned invalid outputs");
			auto outputInfo = outputs[0].GetTensorTypeAndShapeInfo();
			if (outputInfo.GetElementType() != ONNX_TENSOR_ELEMENT_DATA_TYPE_FLOAT || outputInfo.GetElementCount() != static_cast<size_t>(m_impl->outputCount))
				throw std::runtime_error("policy inference returned an invalid output tensor");
			float* values = outputs[0].GetTensorMutableData<float>();
			output.assign(values, values + m_impl->outputCount);
			if (m_impl->recurrent)
			{
				if (!outputs[1].IsTensor() || !outputs[2].IsTensor() ||
					outputs[1].GetTensorTypeAndShapeInfo().GetElementCount() != m_impl->hiddenState.size() ||
					outputs[2].GetTensorTypeAndShapeInfo().GetElementCount() != m_impl->cellState.size())
					throw std::runtime_error("policy inference returned invalid LSTM state tensors");
				float* hiddenValues = outputs[1].GetTensorMutableData<float>();
				float* cellValues = outputs[2].GetTensorMutableData<float>();
				m_impl->hiddenState.assign(hiddenValues, hiddenValues + m_impl->hiddenState.size());
				m_impl->cellState.assign(cellValues, cellValues + m_impl->cellState.size());
			}
			for (size_t i = 0; i < output.size(); ++i)
			{
				if (!std::isfinite(output[i]))
					throw std::runtime_error("policy output contains NaN or infinity");
			}
			m_impl->lastError.clear();
			return true;
		}
		catch (const std::exception& error)
		{
			m_impl->lastError = error.what();
			output.clear();
			return false;
		}
	}

	const std::string& ONNXPolicySession::GetLastError() const { return m_impl->lastError; }
}

#endif

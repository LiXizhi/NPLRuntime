#pragma once
#include "Framework/Interface/Render/IEffect.h"
#include <vector>
#include <unordered_map>
class DxEffectsTree;
namespace ParaEngine
{

	struct UniformInfoGL;
	struct UpdateParmeterCommand
	{
		IParaEngine::ParameterHandle handle;
		void* data;
		uint32_t size;
	};

	class EffectOpenGL : public IParaEngine::IEffect
	{

	public:
		EffectOpenGL();
		virtual ~EffectOpenGL();
		static std::shared_ptr<EffectOpenGL> Create(const std::string& src, IParaEngine::IEffectInclude* include, std::string& error);
		virtual bool GetDesc(IParaEngine::EffectDesc* pOutDesc) override;
		virtual IParaEngine::TechniqueHandle GetTechnique(uint32_t index) override;
		virtual bool GetTechniqueDesc(const IParaEngine::TechniqueHandle& handle, IParaEngine::TechniqueDesc* pOutDesc) override;
		virtual IParaEngine::ParameterHandle GetParameter(uint32_t index) override;
		virtual bool GetParameterDesc(const IParaEngine::ParameterHandle& handle, IParaEngine::ParameterDesc* pOutDesc) override;
		virtual bool SetTechnique(const IParaEngine::TechniqueHandle& handle) override;
		virtual bool SetMatrixArray(const IParaEngine::ParameterHandle& handle, const ParaEngine::DeviceMatrix* data, uint32_t count) override;
		virtual bool SetMatrix(const IParaEngine::ParameterHandle& handle, const ParaEngine::DeviceMatrix* data) override;
		virtual bool SetVectorArray(const IParaEngine::ParameterHandle& handle, const ParaEngine::DeviceVector4* data, uint32_t count) override;
		virtual bool SetFloatArray(const IParaEngine::ParameterHandle& handle, const float* data, uint32_t count) override;
		virtual bool SetValue(const IParaEngine::ParameterHandle& handle, const void* data, uint32_t size) override;
		virtual bool SetBool(const IParaEngine::ParameterHandle& handle, bool value) override;
		virtual bool SetInt(const IParaEngine::ParameterHandle& handle, int value) override;
		virtual bool SetFloat(const IParaEngine::ParameterHandle& handle, float value) override;
		virtual bool SetTexture(const IParaEngine::ParameterHandle& handle, ParaEngine::DeviceTexturePtr_type texture) override;
		virtual bool SetTexture(const char* name, ParaEngine::DeviceTexturePtr_type texture) override;
		virtual bool SetRawValue(const IParaEngine::ParameterHandle& handle, const void* data, uint32_t offset, uint32_t size) override;
		virtual bool SetRawValue(const char* name, const void* data, uint32_t offset, uint32_t size) override;
		virtual IParaEngine::ParameterHandle GetParameterByName(const char* name) override;
		virtual bool SetVector(const IParaEngine::ParameterHandle& handle, const ParaEngine::DeviceVector4* data) override;
		virtual void OnLostDevice() override;
		virtual void OnResetDevice() override;
		virtual bool Begin() override;
		virtual bool BeginPass(const uint8_t pass) override;
		virtual bool EndPass() override;
		virtual bool End() override;
		virtual IParaEngine::TechniqueHandle GetCurrentTechnique() override;
		virtual bool CommitChanges() override;
	private:
		DxEffectsTree* m_FxDesc;
		static const int MAX_TECHNIQUES = 16;
		static const int MAX_PASSES = 16;
		static const int MAX_UNIFORMS = 64;
		uint32_t m_ShaderPrograms[MAX_TECHNIQUES][MAX_PASSES];
		UpdateParmeterCommand m_ParameterCommands[MAX_UNIFORMS];
		std::vector<UniformInfoGL> m_Uniforms;
	// Runtime state
	private:
		IParaEngine::TechniqueHandle m_CurrentTechniqueHandle;

	};
}
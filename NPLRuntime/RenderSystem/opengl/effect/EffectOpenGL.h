#pragma once
#include "Framework/Interface/Render/IEffect.h"
#include <vector>
#include <unordered_map>
class DxEffectsTree;
class PassNode;
namespace ParaEngine
{

	struct UniformInfoGL;
	struct SamplerInitialzerInfo;

	struct ParmeterValueCache
	{
		IParaEngine::ParameterHandle handle;
		void* data;
		uint32_t size;
	};




	struct EffectRenderState
	{
		bool ZTest;
		bool ZWrite;
		ECullMode CullMode;
		bool AlphaBlend;

		EffectRenderState()
			: ZTest(true)
			, ZWrite(true)
			, CullMode(ECullMode::None)
			, AlphaBlend(true)
		{

		}
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
		virtual bool SetTexture(const IParaEngine::ParameterHandle& handle, IParaEngine::ITexture* texture) override;
		virtual bool SetTexture(const char* name, IParaEngine::ITexture* texture) override;
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
		const EffectRenderState GetPassRenderState(PassNode* pass);
		const EffectRenderState GetCurrentRenderState();
		void ApplyRenderState(const EffectRenderState& rs);

	// Runtime state
	private:
		IParaEngine::TechniqueHandle m_CurrentTechniqueHandle;
		bool m_IsBeginTechnique;
		bool m_IsBeginPass;
		uint32_t m_CurrentPass;
		EffectRenderState m_LastRenderState;

		DxEffectsTree* m_FxDesc;
		static const int MAX_TECHNIQUES = 16;
		static const int MAX_PASSES = 16;
		static const int MAX_UNIFORMS = 64;
		uint32_t m_ShaderPrograms[MAX_TECHNIQUES][MAX_PASSES];
		ParmeterValueCache m_ParametersValueCache[MAX_UNIFORMS];
		std::vector<UniformInfoGL> m_Uniforms;
		// Key = techinique*10000*pass*1000+uniformid
		std::unordered_map<uint32_t, int> m_UniformLocaltionCache;
		std::unordered_map<uint32_t, bool> m_UniformValueCacheDirty;

		std::unordered_map<std::string, uint8_t> m_TextureSlotMap;
		std::unordered_map<std::string, std::string> m_Texture2SamplerMap;
		std::unordered_map<std::string, SamplerInitialzerInfo> m_SamplersInfo;
		std::unordered_map<IParaEngine::ITexture*, SamplerInitialzerInfo> m_TextureOldSamplerState;
	};
}
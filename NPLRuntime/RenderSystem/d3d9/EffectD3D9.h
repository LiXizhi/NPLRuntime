#pragma once
#include "Framework/Interface/Render/IEffect.h"
#include "d3dx9effect.h"
#include <vector>
#include <unordered_map>
namespace ParaEngine
{
	class EffectD3D9 : public IParaEngine::IEffect
	{

	public:
		EffectD3D9(ID3DXEffect* pEffect);
		virtual ~EffectD3D9();
		virtual bool GetDesc(IParaEngine::EffectDesc* pOutDesc) override;
		virtual IParaEngine::TechniqueHandle GetTechnique(uint32_t index) override;
		virtual bool GetTechniqueDesc(const IParaEngine::TechniqueHandle& handle, IParaEngine::TechniqueDesc* pOutDesc) override;
		virtual IParaEngine::ParameterHandle GetParameter(uint32_t index) override;
		virtual bool GetParameterDesc(const IParaEngine::ParameterHandle& handle, IParaEngine::ParameterDesc* pOutDesc) override;
		ID3DXEffect* GetD3DEffect();
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

	private:
		ID3DXEffect * m_pEffect;
		std::vector<D3DXHANDLE> m_TechniqueHandles;
		std::vector<D3DXHANDLE> m_ParameterHandles;
		std::unordered_map<std::string, uint32_t> m_Name2HandleIdxMap;
	};
}
#pragma once
#include "Framework/Interface/Render/IEffect.h"
#include "d3dx9effect.h"
#include <vector>
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

	private:
		ID3DXEffect * m_pEffect;
		std::vector<D3DXHANDLE> m_TechniqueHandles;
		std::vector<D3DXHANDLE> m_ParameterHandles;
	};
}
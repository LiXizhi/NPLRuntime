#include "EffectD3D9.h"
using namespace ParaEngine;
using namespace IParaEngine;
EffectD3D9::~EffectD3D9()
{
	if (m_pEffect)
	{
		m_pEffect->Release();
		m_pEffect = nullptr;
	}
}
bool EffectD3D9::GetDesc(IParaEngine::EffectDesc* pOutDesc)
{
	if (pOutDesc)
	{
		D3DXEFFECT_DESC desc;
		if (m_pEffect->GetDesc(&desc) == S_OK)
		{
			pOutDesc->Parameters = m_ParameterHandles.size();
			pOutDesc->Techniques = m_TechniqueHandles.size();
			return true;
		}
	}

	return false;
}

ParaEngine::EffectD3D9::EffectD3D9(ID3DXEffect* pEffect):m_pEffect(pEffect)
{
	D3DXEFFECT_DESC eff_desc;
	D3DXHANDLE tech_handle = NULL;

	if (pEffect->GetDesc(&eff_desc) == S_OK)
	{
		// techniques
		while (pEffect->FindNextValidTechnique(tech_handle, &tech_handle) == S_OK)
		{
			m_TechniqueHandles.push_back(tech_handle);
		}

		// parameters
		for (int i = 0; i < eff_desc.Parameters; i++)
		{
			auto handle = m_pEffect->GetParameter(NULL,i);
			m_ParameterHandles.push_back(handle);
		}
	}
}

IParaEngine::TechniqueHandle ParaEngine::EffectD3D9::GetTechnique(uint32_t index)
{
	IParaEngine::TechniqueHandle handle;
	handle.idx = PARA_INVALID_HANDLE;

	if (index >= 0 && index < m_TechniqueHandles.size())
	{
		auto tech_handle = m_TechniqueHandles[index];
		if (tech_handle != NULL)
		{
			handle.idx = index;
		}
	}

	return handle;

}

bool EffectD3D9::GetTechniqueDesc(const IParaEngine::TechniqueHandle& handle, IParaEngine::TechniqueDesc* pOutDesc)
{
	if (!pOutDesc)return false;
	if (isValidHandle(handle))
	{
		if (handle.idx >= 0 && handle.idx < m_TechniqueHandles.size())
		{
			auto tech_handle = m_TechniqueHandles[handle.idx];
			D3DXTECHNIQUE_DESC tech_desc;
			if (m_pEffect->GetTechniqueDesc(tech_handle, &tech_desc) == S_OK)
			{
				pOutDesc->Name = tech_desc.Name;
				pOutDesc->Passes = tech_desc.Passes;
				return true;
			}
		}
	}
	return false;
}

IParaEngine::ParameterHandle ParaEngine::EffectD3D9::GetParameter(uint32_t index)
{
	IParaEngine::ParameterHandle handle;
	handle.idx = PARA_INVALID_HANDLE;
	if (index >= 0 && index < m_ParameterHandles.size())
	{
		auto parameter_handle = m_ParameterHandles[index];
		if (parameter_handle != NULL)
		{
			handle.idx = index;
		}
	}
	return handle;
}

bool ParaEngine::EffectD3D9::GetParameterDesc(const IParaEngine::ParameterHandle & handle, IParaEngine::ParameterDesc * pOutDesc)
{
	if (pOutDesc == NULL)return false;
	if (!isValidHandle(handle))return false;
	
	if (handle.idx >= 0 && handle.idx < m_ParameterHandles.size())
	{
		D3DXPARAMETER_DESC desc;
		D3DXHANDLE parameter_handle = m_ParameterHandles[handle.idx];
		if (m_pEffect->GetParameterDesc(parameter_handle, &desc) == S_OK)
		{
			
		}
	}



	return false;
}

ID3DXEffect* ParaEngine::EffectD3D9::GetD3DEffect()
{
	return m_pEffect;
}


bool EffectD3D9::SetTechnique(const TechniqueHandle& handle)
{
	if (isValidHandle(handle))
	{
		if (handle.idx >= 0 && handle.idx < m_TechniqueHandles.size())
		{
			auto tech_handle = m_TechniqueHandles[handle.idx];
			if (tech_handle == NULL) return false;
			if (m_pEffect->SetTechnique(tech_handle) == S_OK)
			{
				return true;
			}
		}
	}

	return false;
}


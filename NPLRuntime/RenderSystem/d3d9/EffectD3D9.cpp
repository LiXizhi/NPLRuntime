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
			pOutDesc->Parameters = (uint16_t)m_ParameterHandles.size();
			pOutDesc->Techniques = (uint16_t)m_TechniqueHandles.size();
			return true;
		}
	}

	return false;
}

ParaEngine::EffectD3D9::EffectD3D9(ID3DXEffect* pEffect):m_pEffect(pEffect)
{
	D3DXEFFECT_DESC eff_desc;
	D3DXPARAMETER_DESC paramter_desc;
	D3DXHANDLE tech_handle = NULL;

	if (pEffect->GetDesc(&eff_desc) == S_OK)
	{
		// techniques
		while (pEffect->FindNextValidTechnique(tech_handle, &tech_handle) == S_OK)
		{
			m_TechniqueHandles.push_back(tech_handle);
		}

		// parameters
		for (uint32_t i = 0; i < eff_desc.Parameters; i++)
		{
			auto handle = m_pEffect->GetParameter(NULL,i);
			m_ParameterHandles.push_back(handle);
			if (m_pEffect->GetParameterDesc(handle, &paramter_desc) == S_OK)
			{
				m_Name2HandleIdxMap[paramter_desc.Name] = i;
			}
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
			pOutDesc->Name = desc.Name;
			if(desc.Semantic!=NULL)
				pOutDesc->Semantic = desc.Semantic;
			pOutDesc->Type = EParameterType::PT_UNSUPPORTED;
			pOutDesc->Elements = desc.Elements;
			if (desc.Type == D3DXPT_FLOAT)
			{
				if (desc.Rows == 1)
				{
					if (desc.Columns == 1) pOutDesc->Type = EParameterType::PT_FLOAT;
					if (desc.Columns == 2) pOutDesc->Type = EParameterType::PT_FLOAT2;
					if (desc.Columns == 3) pOutDesc->Type = EParameterType::PT_FLOAT3;
					if (desc.Columns == 4) pOutDesc->Type = EParameterType::PT_FLOAT4;
				}
				else if (desc.Rows == 2)
				{
					if (desc.Columns == 2) pOutDesc->Type = EParameterType::PT_FLOAT2x2;
					if (desc.Columns == 3) pOutDesc->Type = EParameterType::PT_FLOAT2x3;
					if (desc.Columns == 4) pOutDesc->Type = EParameterType::PT_FLOAT2x4;

				}else if (desc.Rows == 3)
				{
					if (desc.Columns == 3)pOutDesc->Type = EParameterType::PT_FLOAT3x3;
					if (desc.Columns == 4)pOutDesc->Type = EParameterType::PT_FLOAT3x4;
				}
				else if (desc.Rows == 4)
				{
					if (desc.Columns == 2)pOutDesc->Type = EParameterType::PT_FLOAT4x2;
					if (desc.Columns == 3)pOutDesc->Type = EParameterType::PT_FLOAT4x3;
					if (desc.Columns == 4)pOutDesc->Type = EParameterType::PT_FLOAT4x4;
				}
			}
			else if (desc.Type == D3DXPT_BOOL)
			{
				pOutDesc->Type = EParameterType::PT_BOOL;
			}else if (desc.Type == D3DXPT_INT)
			{
				pOutDesc->Type = EParameterType::PT_INT;
			}
			else if (desc.Type == D3DXPT_VOID)
			{
				pOutDesc->Type = EParameterType::PT_VOID;
			}else if (desc.Type == D3DXPT_TEXTURE)
			{
				pOutDesc->Type = EParameterType::PT_TEXTURE;
			}else if (desc.Type == D3DXPT_TEXTURE1D)
			{
				pOutDesc->Type = EParameterType::PT_TEXTURE1D;
			}
			else if (desc.Type == D3DXPT_TEXTURE2D)
			{
				pOutDesc->Type = EParameterType::PT_TEXTURE2D;
			}
			else if (desc.Type == D3DXPT_TEXTURE3D)
			{
				pOutDesc->Type = EParameterType::PT_TEXTURE3D;
			}
			else if (desc.Type == D3DXPT_TEXTURECUBE)
			{
				pOutDesc->Type = EParameterType::PT_TEXTURECUBE;
			}
			else if (desc.Type == D3DXPT_SAMPLER)
			{
				pOutDesc->Type = EParameterType::PT_SAMPLER;
			}
			else if (desc.Type == D3DXPT_SAMPLER1D)
			{
				pOutDesc->Type = EParameterType::PT_SAMPLER1D;
			}
			else if (desc.Type == D3DXPT_SAMPLER2D)
			{
				pOutDesc->Type = EParameterType::PT_SAMPLER2D;
			}
			else if (desc.Type == D3DXPT_SAMPLER3D)
			{
				pOutDesc->Type = EParameterType::PT_SAMPLER3D;
			}
			else if (desc.Type == D3DXPT_SAMPLERCUBE)
			{
				pOutDesc->Type = EParameterType::PT_SAMPLERCUBE;
			}
			else if (desc.Class == D3DXPC_STRUCT)
			{
				pOutDesc->Type = EParameterType::PT_STRUCT;
			}
			return true;
		}
	}



	return false;
}

ID3DXEffect* ParaEngine::EffectD3D9::GetD3DEffect()
{
	return m_pEffect;
}



bool ParaEngine::EffectD3D9::SetMatrixArray(const ParameterHandle& handle, const ParaEngine::DeviceMatrix* data, uint32_t count)
{
	if (!isValidHandle(handle))return false;
	if (handle.idx < 0 || handle.idx >= m_ParameterHandles.size()) return false;

	auto h = m_ParameterHandles[handle.idx];
	return m_pEffect->SetMatrixArray(h, data, count) == S_OK;
}


bool ParaEngine::EffectD3D9::SetMatrix(const ParameterHandle& handle, const ParaEngine::DeviceMatrix* data)
{
	if (!isValidHandle(handle))return false;
	if (handle.idx < 0 || handle.idx >= m_ParameterHandles.size()) return false;
	auto h = m_ParameterHandles[handle.idx];
	return m_pEffect->SetMatrix(h, data) == S_OK;
}


bool ParaEngine::EffectD3D9::SetVectorArray(const ParameterHandle& handle, const ParaEngine::DeviceVector4* data, uint32_t count)
{
	if (!isValidHandle(handle))return false;
	if (handle.idx < 0 || handle.idx >= m_ParameterHandles.size()) return false;
	auto h = m_ParameterHandles[handle.idx];
	return m_pEffect->SetVectorArray(h,data, count) == S_OK;
}


bool ParaEngine::EffectD3D9::SetFloatArray(const ParameterHandle& handle, const float* data, uint32_t count)
{
	if (!isValidHandle(handle))return false;
	if (handle.idx < 0 || handle.idx >= m_ParameterHandles.size()) return false;
	auto h = m_ParameterHandles[handle.idx];
	return m_pEffect->SetFloatArray(h, data, count) == S_OK;
}


bool ParaEngine::EffectD3D9::SetValue(const ParameterHandle& handle, const void* data, uint32_t size)
{
	if (!isValidHandle(handle))return false;
	if (handle.idx < 0 || handle.idx >= m_ParameterHandles.size()) return false;
	auto h = m_ParameterHandles[handle.idx];
	return m_pEffect->SetValue(h, data, size) == S_OK;
}


bool ParaEngine::EffectD3D9::SetBool(const ParameterHandle& handle, bool value)
{
	if (!isValidHandle(handle))return false;
	if (handle.idx < 0 || handle.idx >= m_ParameterHandles.size()) return false;
	auto h = m_ParameterHandles[handle.idx];
	return m_pEffect->SetBool(h, value) == S_OK;
}


bool ParaEngine::EffectD3D9::SetInt(const ParameterHandle& handle, int value)
{
	if (!isValidHandle(handle))return false;
	if (handle.idx < 0 || handle.idx >= m_ParameterHandles.size()) return false;
	auto h = m_ParameterHandles[handle.idx];
	return m_pEffect->SetInt(h, value) == S_OK;
}


bool ParaEngine::EffectD3D9::SetFloat(const ParameterHandle& handle, float value)
{
	if (!isValidHandle(handle))return false;
	if (handle.idx < 0 || handle.idx >= m_ParameterHandles.size()) return false;
	auto h = m_ParameterHandles[handle.idx];
	return m_pEffect->SetFloat(h, value) == S_OK;
}


bool ParaEngine::EffectD3D9::SetTexture(const ParameterHandle& handle, ParaEngine::DeviceTexturePtr_type texture)
{
	if (!isValidHandle(handle))return false;
	if (handle.idx < 0 || handle.idx >= m_ParameterHandles.size()) return false;
	auto h = m_ParameterHandles[handle.idx];
	bool ret = m_pEffect->SetTexture(h, texture) == S_OK;
	return ret;
}


bool ParaEngine::EffectD3D9::SetRawValue(const ParameterHandle& handle, const void* data, uint32_t offset, uint32_t size)
{
	if (!isValidHandle(handle))return false;
	if (handle.idx < 0 || handle.idx >= m_ParameterHandles.size()) return false;
	auto h = m_ParameterHandles[handle.idx];
	return m_pEffect->SetRawValue(h, data, offset, size) == S_OK;
}


IParaEngine::ParameterHandle ParaEngine::EffectD3D9::GetParameterByName(const char* name)
{
	ParameterHandle handle;
	handle.idx = PARA_INVALID_HANDLE;
	auto it = m_Name2HandleIdxMap.find(name);
	if (it != m_Name2HandleIdxMap.end())
	{
		handle.idx = it->second;
	}
	return handle;
}


bool ParaEngine::EffectD3D9::SetVector(const ParameterHandle& handle, const ParaEngine::DeviceVector4* data)
{
	if (!isValidHandle(handle))return false;
	if (handle.idx < 0 || handle.idx >= m_ParameterHandles.size()) return false;
	auto h = m_ParameterHandles[handle.idx];
	return m_pEffect->SetVector(h,data) == S_OK;
}


void ParaEngine::EffectD3D9::OnLostDevice()
{
	m_pEffect->OnLostDevice();
}


void ParaEngine::EffectD3D9::OnResetDevice()
{
	m_pEffect->OnResetDevice();
}


bool ParaEngine::EffectD3D9::Begin()
{
	return m_pEffect->Begin(0, D3DXFX_DONOTSAVESTATE | D3DXFX_DONOTSAVESAMPLERSTATE | D3DXFX_DONOTSAVESHADERSTATE) == S_OK;
}


bool ParaEngine::EffectD3D9::BeginPass(const uint8_t pass)
{
	return m_pEffect->BeginPass(pass) == S_OK;
}
bool ParaEngine::EffectD3D9::EndPass()
{
	return m_pEffect->EndPass() == S_OK;
}


bool ParaEngine::EffectD3D9::End()
{
	return m_pEffect->End() == S_OK;
}


IParaEngine::TechniqueHandle ParaEngine::EffectD3D9::GetCurrentTechnique()
{
	TechniqueHandle handle;
	handle.idx = PARA_INVALID_HANDLE;

	auto hTech = m_pEffect->GetCurrentTechnique();
	if (hTech != NULL)
	{
		for (size_t i = 0; i < m_TechniqueHandles.size(); i++)
		{
			if (m_TechniqueHandles[i] == hTech)
			{
				handle.idx = i;
				return handle;
			}
		}
	}
	return handle;
}


bool ParaEngine::EffectD3D9::CommitChanges()
{
	return m_pEffect->CommitChanges() == S_OK;
}

bool ParaEngine::EffectD3D9::SetRawValue(const char* name, const void* data, uint32_t offset, uint32_t size)
{
	return m_pEffect->SetRawValue(name, data, offset, size) == S_OK;
}

bool ParaEngine::EffectD3D9::SetTexture(const char* name, ParaEngine::DeviceTexturePtr_type texture)
{
	return m_pEffect->SetTexture(name, texture) == S_OK;
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


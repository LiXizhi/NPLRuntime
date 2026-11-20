//-----------------------------------------------------------------------------
// Class:	Effect Manager
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2006.4.7
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "SceneObject.h"
#include "ParaWorldAsset.h"
#include "ShadowMap.h"
#include "GlowEffect.h"
#include "WaveEffect.h"
#include "OceanManager.h"
#include "SunLight.h"
#include "terrain/GlobalTerrain.h"
#include "terrain/Settings.h"
#include "ParaEngineSettings.h"
#include "EffectManager.h"
#include "2dengine/GUIRoot.h"
#include "BlockEngine/BlockCommon.h"
#include "BlockEngine/BlockWorldClient.h"
#include "VertexDeclaration.h"

using namespace ParaEngine;
/**@def define to disable all shaders. */
//#define TESTING_FIXED_FUNCTION

/**@def batch shader technique and passes. I do not know why but the Effect->BeginPass() will fail if stream is not set.  
however, the texture is restored on BeginPass, which make sharing states and textures across passes impossible. 
plus, BeginPass() is pretty expensive to call Only calling BeginPass when I switch effect instead of 
for each object was a massive improvement! So all single pass effect, we shall consider enable ENABLE_SHARE_MODE_SHADER. 
*/
#define ENABLE_SHARE_MODE_SHADER

/**@def define to enable shadow on mesh. */
// #define SHADOW_ON_MESH

/**@def  alpha testing reference at fixed function. used to be 0x18. But I changed it to 0x80 which is the same as the programmable pipeline 0.5*/
#define FIXED_FUNCTION_ALPHA_TESTING_REF	0x80

/**@def define maximum number of local lights per object. */
#define MAX_LOCAL_LIGHTS_NUM	4

namespace ParaEngine
{
	extern float g_flash;

	inline DWORD FtoDW( FLOAT f ) { return *((DWORD*)&f); }

	static VertexElement vertexdesc_pos[] =
	{
		// base data (stream 0)
		{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		D3DDECL_END()
	};

	static VertexElement vertexdesc_simple_mesh[]=
	{
		// base data (stream 0)
		{ 0, 0,	D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0,12,	D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		D3DDECL_END()
	};
	static VertexElement vertexdesc_single_color[] =
	{
		// base data (stream 0)
		{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 12, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
		D3DDECL_END()
	};

	static VertexElement vertex2desc_single_color[] =
	{
		// base data (stream 0)
		{ 0, 0, D3DDECLTYPE_FLOAT4, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 16, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
		D3DDECL_END()
	};

	static VertexElement vertexdesc_particle[]=
	{
		// base data (stream 0)
		{ 0, 0,	D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0,12,	D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
		{ 0,16,	D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		D3DDECL_END()
	};
	static VertexElement vertexdesc_bmax_model[] =
	{
		// base data (stream 0)
		{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
		{ 0, 24, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
		D3DDECL_END()
	};
	static VertexElement vertexdesc_bmax_model_skinned[] =
	{
		// base data (stream 0)
		{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
		{ 0, 24, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
		{ 0, 28, D3DDECLTYPE_UBYTE4N, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT, 0 },
		{ 0, 32, D3DDECLTYPE_UBYTE4N, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDINDICES, 0 },
		D3DDECL_END()
	};
	static VertexElement vertexdesc_simple_mesh_normal[]=
	{
		// base data (stream 0)
		{ 0, 0,	D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0,12,	D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
		{ 0,24,	D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		D3DDECL_END()
	};
	static VertexElement vertexdesc_character_skinned[] =
	{
		// base data (stream 0)
		{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
		{ 0, 24, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		{ 0, 32, D3DDECLTYPE_UBYTE4N, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDWEIGHT, 0 },
		{ 0, 36, D3DDECLTYPE_UBYTE4N, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_BLENDINDICES, 0 },
		D3DDECL_END()
	};

	static VertexElement vertexdesc_simple_mesh_normal_color[]=
	{
		// base data (stream 0)
		{ 0, 0,	D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0,12,	D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
		{ 0,24,	D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
		{ 0,28,	D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		D3DDECL_END()
	};

	static VertexElement vertexdesc_simple_mesh_normal_instanced[]=
	{
		// base data (stream 0)
		{ 0, 0,	D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0,12,	D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
		{ 0,24,	D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		// instance world matrix data (stream 1)
		{ 1, 0, D3DDECLTYPE_FLOAT4,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1},
		{ 1,16, D3DDECLTYPE_FLOAT4,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 2},
		{ 1,32, D3DDECLTYPE_FLOAT4,   D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 3},
		D3DDECL_END()
	};
	static VertexElement vertexdesc_ocean_fft[]=
	{
		// base data (stream 0)
		{ 0, 0, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },

		// anim data (stream 1)
		{ 1, 0, D3DDECLTYPE_FLOAT1, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 1 },
		{ 1, 4, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   0 },

		// anim data (stream 2)
		{ 2, 0, D3DDECLTYPE_FLOAT1, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 2 },
		{ 2, 4, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   1 },
		D3DDECL_END()
	};
	// with height field;
	static VertexElement vertexdesc_ocean_fft_hf[]=
	{
		// base data (stream 0)
		{ 0, 0, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },

		// anim data (stream 1)
		{ 1, 0, D3DDECLTYPE_FLOAT1, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 1 },
		{ 1, 4, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   0 },

		// anim data (stream 2)
		{ 2, 0, D3DDECLTYPE_FLOAT1, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 2 },
		{ 2, 4, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL,   1 },

		// terrain height field (stream 3)
		{ 3, 0, D3DDECLTYPE_FLOAT1, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		D3DDECL_END()
	};
	// with height field;
	static VertexElement vertexdesc_ocean_cloud[] =
	{
		// base data (stream 0)
		{ 0, 0, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		// terrain height field (stream 1)
		{ 1, 0, D3DDECLTYPE_FLOAT1, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		D3DDECL_END()
	};
	static VertexElement vertexdesc_terrain[]=
	{
		// base data (stream 0)
		{ 0, 0,	D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0,12,	D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		{ 0,20,	D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1 },
		D3DDECL_END()
	};
	static VertexElement vertexdesc_terrain_normal[]=
	{
		// base data (stream 0)
		{ 0, 0,	D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0,12,	D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
		{ 0,24,	D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		{ 0,32,	D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 1 },
		D3DDECL_END()
	};
	static VertexElement vertexdesc_block[] =
	{
		// base data (stream 0)
		{ 0, 0, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0, 12, D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
		{ 0, 24, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 0 },
		{ 0, 28, D3DDECLTYPE_D3DCOLOR, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_COLOR, 1 },
		{ 0, 32, D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
		D3DDECL_END()
	};

	HRESULT EffectManager::SetSamplerState( int nStage, ESamplerStateType dwType, DWORD dwValue, bool bForceSet /*= false*/ )
	{
		if(bForceSet)
		{
			auto pRenderDevice =CGlobals::GetRenderDevice();
			pRenderDevice->SetSamplerState(nStage, (ESamplerStateType)dwType, dwValue);
			m_lastSamplerStates[nStage][(int)dwType] = dwValue;
		}
		else
		{
			DWORD dwLastValue = m_lastSamplerStates[nStage][(int)dwType];
			if(dwLastValue != dwValue)
			{
				auto pRenderDevice =CGlobals::GetRenderDevice();
				pRenderDevice->SetSamplerState(nStage, (ESamplerStateType)dwType, dwValue);
				m_lastSamplerStates[nStage][(int)dwType] = dwValue;
			}
		}
		return S_OK;
	}

	HRESULT EffectManager::GetSamplerState( int nStage, ESamplerStateType dwType, DWORD* pValue, bool bForceGet /*= false*/ )
	{
		DWORD dwValue = 0;
		if (bForceGet && CGlobals::GetRenderDevice())
		{
			uint32_t val = 0;
			if (CGlobals::GetRenderDevice()->GetSamplerState(nStage, dwType, &val))
				dwValue = val;
			else
				dwValue = m_lastSamplerStates[nStage][(int)dwType];
		}
		else
		{
			dwValue = m_lastSamplerStates[nStage][(int)dwType];
		}
		if(pValue)
			*pValue = dwValue;
		return S_OK;
	}

	void EffectManager::SetDefaultSamplerState(int nStage, ESamplerStateType dwType, DWORD dwValue)
	{
		m_defaultSamplerStates[nStage][(int)dwType] = dwValue;
	}
	DWORD EffectManager::GetDefaultSamplerState(int nStage, ESamplerStateType dwType)
	{
		return m_defaultSamplerStates[nStage][(int)dwType];
	}
}
//////////////////////////////////////////////////////////////////////////
//
// EffectManager class
//
//////////////////////////////////////////////////////////////////////////

EffectManager::EffectManager()
:m_nCurrentEffect(0),m_pCurrentEffect(NULL),m_bUseFog(true),m_ClipPlaneState(ClipPlane_Disabled),m_bClipPlaneEnabled(false),
m_bDisableD3DAlphaTesting(false), m_bDisableD3DCulling(false), m_bEnableLocalLighting(true), m_bUsingShadowMap(false), m_bZEnable(true),
m_pShadowMap(NULL), m_pGlowEffect(NULL),
m_colorGlowness(1.0f, 1.0f, 1.0f, 1.0f), m_nGlowTechnique(0),m_bIsUsingFullScreenGlow(false),m_nMaxLocalLightsNum(4),
m_bEnableReflectionRendering(true),m_pScene(NULL), m_nEffectLevel(30), m_bEffectValid(true), m_bDisableZWrite(false),m_pWaveEffect(NULL),
m_enableScreenWaveEffect(false)
{
	memset(m_pVertexDeclarations, 0, sizeof(m_pVertexDeclarations));
	memset(m_pVertexDeclarations_status, 0, sizeof(m_pVertexDeclarations_status));
	memset(m_lastSamplerStates, 0, sizeof(m_lastSamplerStates));
	memset(m_defaultSamplerStates, 0, sizeof(m_defaultSamplerStates));
	
	SetDefaultSamplerState(0, ESamplerStateType::MINFILTER, D3DTEXF_LINEAR);
	SetDefaultSamplerState(0, ESamplerStateType::MAGFILTER, D3DTEXF_LINEAR);
	SetDefaultSamplerState(1, ESamplerStateType::MINFILTER, D3DTEXF_LINEAR);
	SetDefaultSamplerState(1, ESamplerStateType::MAGFILTER, D3DTEXF_LINEAR);
}
 
EffectManager::~EffectManager()
{

}

void EffectManager::Cleanup()
{
	m_pCurrentEffect = NULL;
	m_HandleMap.clear();
	AssetManager <CEffectFile>::Cleanup();
	SAFE_DELETE(m_pShadowMap);
	SAFE_DELETE(m_pGlowEffect);
	SAFE_DELETE(m_pWaveEffect);
}

void EffectManager::SetMaxLocalLightsNum(int nNum)
{
	if(nNum > MAX_LOCAL_LIGHTS_NUM)
		nNum = MAX_LOCAL_LIGHTS_NUM;
	m_nMaxLocalLightsNum = nNum;
}

int EffectManager::GetMaxLocalLightsNum()
{
	return m_nMaxLocalLightsNum;
}

CShadowMap* EffectManager::GetShadowMap()
{
	if (m_pShadowMap == 0)
	{
		m_pShadowMap = new CShadowMap();
		if(m_pShadowMap!=0)
		{
			m_pShadowMap->RestoreDeviceObjects();
		}
	}
	return m_pShadowMap;
}

bool EffectManager::IsUsingFullScreenGlow()
{
	return m_bIsUsingFullScreenGlow;
}

void EffectManager::EnableFullScreenGlow( bool bEnable)
{
	m_bIsUsingFullScreenGlow = bEnable;
	if (bEnable && m_pGlowEffect != 0)
	{
		m_pGlowEffect->InvalidateDeviceObjects();
		SAFE_DELETE(m_pGlowEffect);
	}
}

void EffectManager::SetGlowTechnique(int nTech)
{
	if(m_nGlowTechnique!=nTech)
	{
		m_nGlowTechnique = nTech;
		if(m_pGlowEffect!=0)
		{
			CEffectFile* pEffect = GetEffectByHandle(TECH_FULL_SCREEN_GLOW);
			if(pEffect!=0)
			{
				pEffect->SetTechniqueByIndex(nTech);
			}
		}
	}
}

int EffectManager::GetGlowTechnique()
{
	return m_nGlowTechnique;
}

void EffectManager::SetGlowness(const Vector4& Glowness)
{
	m_colorGlowness = Glowness;
}
const Vector4& EffectManager::GetGlowness()
{
	return m_colorGlowness;
}

CGlowEffect* EffectManager::GetGlowEffect()
{
	if (m_pGlowEffect == 0)
	{
		m_pGlowEffect = new CGlowEffect();
		if(m_pGlowEffect!=0)
			m_pGlowEffect->RestoreDeviceObjects();
	}
	return m_pGlowEffect;
}

bool EffectManager::IsUsingScreenWaveEffect()
{
	return m_enableScreenWaveEffect;
}

void EffectManager::EnableScreenWaveEffect(bool enable)
{
	m_enableScreenWaveEffect = enable;
	if(!enable && m_pWaveEffect != 0)
	{
		m_pWaveEffect->InvalidateDeviceObjects();
		SAFE_DELETE(m_pWaveEffect);
	}
}

void EffectManager::SetScreenWaveTechnique(int nTech)
{
	if(m_screenWaveTechnique !=nTech)
	{
		m_screenWaveTechnique = nTech;
		if(m_pWaveEffect!=0)
		{
			CEffectFile* pEffect = GetEffectByHandle(TECH_SCREEN_WAVE);
			if(pEffect!=0)
			{
				pEffect->SetTechniqueByIndex(nTech);
			}
		}
	}
}

int EffectManager::GetScreenWaveTechnique()
{
	return m_screenWaveTechnique;
}

WaveEffect* EffectManager::GetScreenWaveEffect()
{
	if(m_pWaveEffect == 0)
	{
		m_pWaveEffect = new WaveEffect();
		if(m_pWaveEffect != 0)
			m_pWaveEffect->RestoreDeviceObjects();
	}
	return m_pWaveEffect;
}

EffectManager::EffectTechniques EffectManager::GetCurrentEffectTechniqueType()
{
	if(m_pCurrentEffect == 0)
		return EFFECT_FIXED_FUNCTION;
	else
	{
		CEffectFile::TechniqueCategory nCat = m_pCurrentEffect->GetCurrentTechniqueDesc()->nCategory;
		switch(nCat)
		{
		case CEffectFile::TechCategory_GenShadowMap:
			return EFFECT_GEN_SHADOWMAP;
			break;
		default:
			return EFFECT_DEFAULT;
			break;
		}
	}
}

void EffectManager::RestoreDeviceObjects()
{
	if(m_pShadowMap!=0)
		m_pShadowMap->RestoreDeviceObjects();
	if(m_pGlowEffect!=0)
		m_pGlowEffect->RestoreDeviceObjects();
	if(m_pWaveEffect!=0)
		m_pWaveEffect->RestoreDeviceObjects();
	AssetManager <CEffectFile>::RestoreDeviceObjects();
}

void EffectManager::InvalidateDeviceObjects()
{
	if(m_pShadowMap!=0)
		m_pShadowMap->InvalidateDeviceObjects();
	if(m_pGlowEffect!=0)
		m_pGlowEffect->InvalidateDeviceObjects();
	if(m_pWaveEffect != 0)
		m_pWaveEffect->InvalidateDeviceObjects();
	AssetManager <CEffectFile>::InvalidateDeviceObjects();
}

void EffectManager::InitDeviceObjects()
{
	AssetManager <CEffectFile>::InitDeviceObjects();
}

void EffectManager::DeleteDeviceObjects()
{
	for (int i =0;i<EffectManager::MAX_DECLARATIONS_NUM; ++i)
	{
		SAFE_RELEASE(m_pVertexDeclarations[i]);
		m_pVertexDeclarations_status[i] = 0;
	}
	AssetManager <CEffectFile>::DeleteDeviceObjects();
}

void EffectManager::RendererRecreated()
{
	AssetManager<CEffectFile>::RendererRecreated();
}

bool EffectManager::DeleteEntity(AssetEntity* entity)
{
	if(AssetManager <CEffectFile>::DeleteEntity(entity))
	{
		for( map<int, CEffectFile*>::iterator itCurCP = m_HandleMap.begin(); itCurCP != m_HandleMap.end(); )
		{
			if((*itCurCP).second == entity)
			{
				itCurCP = m_HandleMap.erase(itCurCP);
				if(m_pCurrentEffect == entity)
				{
					m_pCurrentEffect = NULL;
				}
			}
			else
				++ itCurCP;
		}
		return true;
	}
	return false;
}
void EffectManager::EnableClipPlane(bool bEnable)
{
	if(m_bClipPlaneEnabled!=bEnable)
	{
		m_bClipPlaneEnabled = bEnable;
		if (!CGlobals::GetRenderDevice())
			return;
		if(!bEnable)
		{
			// disable it immediately
			EnableClipPlaneImmediate(ClipPlane_Disabled);
		}
		else
		{
			CGlobals::GetRenderDevice()->SetRenderState(ERenderState::CLIPPLANEENABLE, RSV_CLIPPLANE0);
		}
	}
}
void EffectManager::EnableUsingShadowMap(bool bEnable)
{
	m_bUsingShadowMap = bEnable;
}

bool EffectManager::IsUsingShadowMap()
{
	return m_bUsingShadowMap;
}

const Matrix4* EffectManager::GetTexViewProjMatrix()
{
	if(m_pShadowMap!=0)
	{
		return m_pShadowMap->GetTexViewProjMatrix();
	}
	return NULL;
}

void EffectManager::EnableClipPlaneImmediate(EffectManager::ClipPlaneState state, int nIndex)
{
	PE_ASSERT(nIndex<MaxClipPlanesNum);
	m_ClipPlaneState = state;
	auto* pDevice = CGlobals::GetRenderDevice();
	if (!pDevice)
		return;
	switch(state)
	{
	case ClipPlane_Enabled_ClipSpace:
		pDevice->SetClipPlane(nIndex, m_ClipPlaneClipSpace[nIndex]);
		break;
	case ClipPlane_Enabled_WorldSpace:
		pDevice->SetClipPlane(nIndex, m_ClipPlaneWorldSpace[nIndex]);
		break;
	default: // disable it
		pDevice->SetRenderState(ERenderState::CLIPPLANEENABLE, RSV_CLIPPLANE_NONE);
		break;
	}
}

bool EffectManager::SetD3DFogState()
{
	return true;
}
void EffectManager::EnableTextures(bool bEnable)
{
	CEffectFile::EnableTextures(bEnable);
}
bool EffectManager::AreTextureEnabled()
{
	return CEffectFile::AreTextureEnabled();
}
void EffectManager::DisableD3DAlphaTesting(bool bDisable)
{
	m_bDisableD3DAlphaTesting = bDisable;
}
bool EffectManager::IsD3DAlphaTestingDisabled()
{
	return m_bDisableD3DAlphaTesting;
}

void EffectManager::DisableD3DCulling(bool bDisable)
{
	m_bDisableD3DCulling = bDisable;
}

bool EffectManager::IsD3DCullingDisabled()
{
	return m_bDisableD3DCulling;
}

void EffectManager::SetCullingMode(DWORD dwCullMode)
{
	if(!IsD3DCullingDisabled())
		CGlobals::GetRenderDevice()->SetRenderState(ERenderState::CULLMODE, dwCullMode);
}
void EffectManager::SetCullingMode(bool bEnable)
{
	if(!IsD3DCullingDisabled())
	{
		if(bEnable)
			CGlobals::GetRenderDevice()->SetRenderState(ERenderState::CULLMODE, CGlobals::GetSceneState()->m_dwD3D_CULLMODE);
		else
			CGlobals::GetRenderDevice()->SetRenderState(ERenderState::CULLMODE, RSV_CULL_NONE);
	}
}

void EffectManager::DisableZWrite( bool bDisable )
{
	m_bDisableZWrite = bDisable;
}

bool EffectManager::IsZWriteDisabled()
{
	return m_bDisableZWrite;
}

void EffectManager::EnableZWrite( bool bZWriteEnabled )
{
	if(!IsZWriteDisabled())
	{
		CGlobals::GetRenderDevice()->SetRenderState(ERenderState::ZWRITEENABLE, bZWriteEnabled ? RSV_TRUE : RSV_FALSE);
	}
}

void EffectManager::EnableZTest(bool bEnable, bool bForceSet /*= false*/)
{
	if (m_bZEnable != bEnable || bForceSet)
	{
		m_bZEnable = bEnable;
		CGlobals::GetRenderDevice()->SetRenderState(ERenderState::ZENABLE, m_bZEnable ? RSV_TRUE : RSV_FALSE);
	}
}

bool EffectManager::IsZTestEnabled()
{
	return m_bZEnable;
}

bool EffectManager::IsClipPlaneEnabled()
{
	return m_bClipPlaneEnabled;
}

EffectManager::ClipPlaneState EffectManager::GetClipPlaneState()
{
	return m_ClipPlaneState;
}

void EffectManager::SetClipPlane(DWORD Index,	const float * pPlane, bool bClipSpace)
{
	PE_ASSERT(Index<MaxClipPlanesNum);
	if(bClipSpace)
		memcpy(m_ClipPlaneClipSpace[Index], pPlane, sizeof(Plane));
	else
		memcpy(m_ClipPlaneWorldSpace[Index], pPlane, sizeof(Plane));
}

HRESULT EffectManager::SetVertexDeclaration(int nIndex)
{
	VertexDeclarationPtr pDecl = GetVertexDeclaration(nIndex);
	if(pDecl != 0)
		CGlobals::GetRenderDevice()->SetVertexDeclaration(pDecl);
	else
		return E_FAIL;
	return S_OK;
}

VertexDeclarationPtr EffectManager::GetVertexDeclaration(int nIndex)
{
	if(m_pVertexDeclarations_status[nIndex]!=0)
		return m_pVertexDeclarations[nIndex];

	VertexDeclarationPtr pDecl = NULL;
	m_pVertexDeclarations_status[nIndex] = 1;

	/**
	* create vertex declaration if it has not been created before.
	*/
	auto pd3dDevice =CGlobals::GetRenderDevice();
	if (!pd3dDevice)
		return nullptr;
	switch(nIndex)
	{
	case S0_POS_TEX0:
		if(pd3dDevice->CreateVertexDeclaration(vertexdesc_simple_mesh, &pDecl) && pDecl)
		{
			m_pVertexDeclarations[S0_POS_TEX0] = pDecl;
		}
		else
		{
			OUTPUT_LOG("error: CreateVertexDeclaration failed for S0_POS_TEX0\n");
		}
		break;
	case S0_POS_NORM_TEX0:
		if(pd3dDevice->CreateVertexDeclaration(vertexdesc_simple_mesh_normal, &pDecl) && pDecl)
		{
			m_pVertexDeclarations[S0_POS_NORM_TEX0] = pDecl;
		}
		else
		{
			OUTPUT_LOG("error: CreateVertexDeclaration failed for S0_POS_NORM_TEX0\n");
		}
		break;
	case S0_POS_NORM_TEX0_COLOR:
		if(pd3dDevice->CreateVertexDeclaration(vertexdesc_simple_mesh_normal_color, &pDecl) && pDecl)
		{
			m_pVertexDeclarations[S0_POS_NORM_TEX0_COLOR] = pDecl;
		}
		else
		{
			OUTPUT_LOG("error: CreateVertexDeclaration failed for S0_POS_NORM_TEX0_COLOR\n");
		}
		break;
	case S0_POS_NORM_TEX0_INSTANCED:
		if(pd3dDevice->CreateVertexDeclaration(vertexdesc_simple_mesh_normal_instanced, &pDecl) && pDecl)
		{
			m_pVertexDeclarations[S0_POS_NORM_TEX0_INSTANCED] = pDecl;
		}
		else
		{
			OUTPUT_LOG("error: CreateVertexDeclaration failed for S0_POS_NORM_TEX0_INSTANCED\n");
		}
		break;
	case S0_POS_TEX0_COLOR:
		if(pd3dDevice->CreateVertexDeclaration(vertexdesc_particle, &pDecl) && pDecl)
		{
			m_pVertexDeclarations[S0_POS_TEX0_COLOR] = pDecl;
		}
		else
		{
			OUTPUT_LOG("error: CreateVertexDeclaration failed for S0_POS_TEX0_COLOR\n");
		}
		break;
	case S0_POS_COLOR:
		if (pd3dDevice->CreateVertexDeclaration(vertexdesc_single_color, &pDecl) && pDecl)
		{
			m_pVertexDeclarations[S0_POS_COLOR] = pDecl;
		}
		else
		{
			OUTPUT_LOG("error: CreateVertexDeclaration failed for S0_POS_COLOR\n");
		}
		break;
	
	case S0_POS2_COLOR:
		if (pd3dDevice->CreateVertexDeclaration(vertex2desc_single_color, &pDecl) && pDecl)
		{
			m_pVertexDeclarations[S0_POS2_COLOR] = pDecl;
		}
		else
		{
			OUTPUT_LOG("error: CreateVertexDeclaration failed for S0_POS2_COLOR\n");
		}
		break;


	case S0_POS:
		if (pd3dDevice->CreateVertexDeclaration(vertexdesc_pos, &pDecl) && pDecl)
		{
			m_pVertexDeclarations[S0_POS] = pDecl;
		}
		else
		{
			OUTPUT_LOG("error: CreateVertexDeclaration failed for S0_POS\n");
		}
		break;
	case S0_S1_OCEAN_CLOUD:
		if(pd3dDevice->CreateVertexDeclaration(vertexdesc_ocean_cloud, &pDecl) && pDecl)
		{
			m_pVertexDeclarations[S0_S1_OCEAN_CLOUD] = pDecl;
		}
		else
		{
			OUTPUT_LOG("error: CreateVertexDeclaration failed for S0_S1_OCEAN_CLOUD\n");
		}
		break;
	case S0_S1_S2_OCEAN_FFT:
		if(pd3dDevice->CreateVertexDeclaration(vertexdesc_ocean_fft, &pDecl) && pDecl)
		{
			m_pVertexDeclarations[S0_S1_S2_OCEAN_FFT] = pDecl;
		}
		else
		{
			OUTPUT_LOG("error: CreateVertexDeclaration failed for S0_S1_S2_OCEAN_FFT\n");
		}
		break;
	case S0_S1_S2_S3_OCEAN_FFT:
		if(pd3dDevice->CreateVertexDeclaration(vertexdesc_ocean_fft_hf, &pDecl) && pDecl)
		{
			m_pVertexDeclarations[S0_S1_S2_S3_OCEAN_FFT] = pDecl;
		}
		else
		{
			OUTPUT_LOG("error: CreateVertexDeclaration failed for S0_S1_S2_S3_OCEAN_FFT\n");
		}
		break;
	case S0_POS_NORM_TEX0_TEX1:
		if(pd3dDevice->CreateVertexDeclaration(vertexdesc_terrain_normal, &pDecl) && pDecl)
		{
			m_pVertexDeclarations[S0_POS_NORM_TEX0_TEX1] = pDecl;
		}
		else
		{
			OUTPUT_LOG("error: CreateVertexDeclaration failed for S0_POS_NORM_TEX0_TEX1\n");
		}
		break;
	case S0_POS_TEX0_TEX1:
		if(pd3dDevice->CreateVertexDeclaration(vertexdesc_terrain, &pDecl) && pDecl)
		{
			m_pVertexDeclarations[S0_POS_TEX0_TEX1] = pDecl;
		}
		else
		{
			OUTPUT_LOG("error: CreateVertexDeclaration failed for S0_POS_NORM_TEX0_TEX1\n");
		}
		break;	
	case S0_POS_NORM_COLOR:
		if (pd3dDevice->CreateVertexDeclaration(vertexdesc_bmax_model, &pDecl) && pDecl)
		{
			m_pVertexDeclarations[S0_POS_NORM_COLOR] = pDecl;
		}
		else
		{
			OUTPUT_LOG("error: CreateVertexDeclaration failed for S0_POS_NORM_COLOR\n");
		}
		break;
	case S0_POS_NORM_TEX0_COLOR0_COLOR1:
		if (pd3dDevice->CreateVertexDeclaration(vertexdesc_block, &pDecl) && pDecl)
		{
			m_pVertexDeclarations[S0_POS_NORM_TEX0_COLOR0_COLOR1] = pDecl;
		}
		else
		{
			OUTPUT_LOG("error: CreateVertexDeclaration failed for S0_POS_NORM_TEX0_COLOR0_COLOR1\n");
		}
		break;
	case S0_POS_NORM_TEX0_SKINNED:
		if (pd3dDevice->CreateVertexDeclaration(vertexdesc_character_skinned, &pDecl) && pDecl)
		{
			m_pVertexDeclarations[S0_POS_NORM_TEX0_SKINNED] = pDecl;
		}
		else
		{
			OUTPUT_LOG("error: CreateVertexDeclaration failed for S0_POS_NORM_TEX0_SKINNED\n");
		}
		break;
	case S0_POS_NORM_COLOR_SKINNED:
		if (pd3dDevice->CreateVertexDeclaration(vertexdesc_bmax_model_skinned, &pDecl) && pDecl)
		{
			m_pVertexDeclarations[S0_POS_NORM_COLOR_SKINNED] = pDecl;
		}
		else
		{
			OUTPUT_LOG("error: CreateVertexDeclaration failed for S0_POS_NORM_COLOR_SKINNED\n");
		}
		break;
	default:
		break;
	}

	return pDecl;
}

int EffectManager::GetEffectHandle( CEffectFile* effectFile )
{
	map<int, CEffectFile*>::iterator itCur, itEnd = m_HandleMap.end();
	for( itCur = m_HandleMap.begin(); itCur!=itEnd; ++itCur)
	{
		if(itCur->second == effectFile)
		{
			return itCur->first;
		}
	}
	return -1;
}

CEffectFile* EffectManager::MapHandleToEffect(int nHandle, CEffectFile* pNewEffect)
{
	CEffectFile* pOld = NULL;
	if(nHandle>0)
	{
		if(pNewEffect!=0)
			m_HandleMap[nHandle] = pNewEffect;
		else
		{
			map<int, CEffectFile*>::iterator it = m_HandleMap.find(nHandle);
			if(it!=m_HandleMap.end())
			{
				m_HandleMap.erase(it);
			}
		}
		if(m_pCurrentEffect == pOld)
		{
			m_pCurrentEffect = NULL;
		}
	}
	return pOld;
}

CEffectFile* EffectManager::GetEffectByHandle(int nHandle)
{
	map<int, CEffectFile*>::iterator it = m_HandleMap.find(nHandle);
	if(it!=m_HandleMap.end())
		return (*it).second;
	return NULL;
}
int EffectManager::GetCurrentTechHandle()
{
	return m_nCurrentEffect;
}
CEffectFile* EffectManager::GetCurrentEffectFile()
{
	return m_pCurrentEffect;
}

void EffectManager::EnableLocalLighting(bool bEnable)
{
	m_bEnableLocalLighting = bEnable;
}
bool EffectManager::IsLocalLightingEnabled()
{
	return m_bEnableLocalLighting;
}

void EffectManager::EnableGlobalLighting(bool bEnable)
{
	if(m_pCurrentEffect != 0)
		m_pCurrentEffect->EnableSunLight(bEnable);
}

bool EffectManager::IsGlobalLightingEnabled()
{
	return GetScene()->IsLightEnabled();
}
void EffectManager::EnableD3DAlphaBlending(bool bEnable)
{
	CGlobals::GetRenderDevice()->SetRenderState(ERenderState::ALPHABLENDENABLE, bEnable ? RSV_TRUE : RSV_FALSE);
}
void EffectManager::EnableFog(bool bEnabled)
{
	m_bUseFog = bEnabled;
}
const Matrix4& EffectManager::GetWorldTransform()
{
	return CGlobals::GetWorldMatrixStack().SafeGetTop();
}
const Matrix4& EffectManager::GetViewTransform()
{
	return CGlobals::GetViewMatrixStack().SafeGetTop();
}
const Matrix4& EffectManager::GetProjTransform()
{
	return CGlobals::GetProjectionMatrixStack().SafeGetTop();
}

void EffectManager::UpdateD3DPipelineTransform(bool pWorld, bool pView,bool pProjection)
{
	(void)pWorld;
	(void)pView;
	(void)pProjection;
}

HRESULT EffectManager::SetTransform(D3DTRANSFORMSTATETYPE State, const Matrix4 *pMatrix)
{
	// set the programmable pipeline
	switch(State)
	{
	case D3DTS_WORLD:
		CGlobals::GetWorldMatrixStack().SafeSetTop(*pMatrix);
		break;
	case D3DTS_VIEW:
		CGlobals::GetViewMatrixStack().SafeSetTop(*pMatrix);
		break;
	case D3DTS_PROJECTION:
		CGlobals::GetProjectionMatrixStack().SafeSetTop(*pMatrix);
		break;
	default:
		break;
	}
	return S_OK;
}

HRESULT EffectManager::GetTransform(D3DTRANSFORMSTATETYPE State,Matrix4 * pMatrix)
{
	// set the programmable pipeline
	switch(State)
	{
	case D3DTS_WORLD:
		*pMatrix = CGlobals::GetWorldMatrixStack().SafeGetTop();
		break;
	case D3DTS_VIEW:
		*pMatrix = CGlobals::GetViewMatrixStack().SafeGetTop();
		break;
	case D3DTS_PROJECTION:
		*pMatrix = CGlobals::GetProjectionMatrixStack().SafeGetTop();
		break;
	default:
		return E_FAIL;
		break;
	}
	return S_OK;
}

void EffectManager::applyFogParameters()
{
	if(m_pCurrentEffect!=0)
	{
		IScene * pScene = GetScene();
		Vector4 fogParam(pScene->GetFogStart(), pScene->GetFogEnd()-pScene->GetFogStart(), pScene->GetFogDensity(), 1.0);
		LinearColor fogColor = pScene->GetFogColor();
		m_pCurrentEffect->applyFogParameters(m_bUseFog, &fogParam, &fogColor);
	}
}

void EffectManager::applySurfaceMaterial(const ParaMaterial* pSurfaceMaterial, bool bUseGlobalAmbient)
{
	if(m_pCurrentEffect != 0)
		m_pCurrentEffect->applySurfaceMaterial(pSurfaceMaterial, bUseGlobalAmbient);
}

void EffectManager::applyLocalLightingData(const LightList* plights, int nLightNum)
{
	if(m_pCurrentEffect != 0)
		m_pCurrentEffect->applyLocalLightingData(plights, nLightNum);
}
void EffectManager::applyLocalLightingData()
{
	applyLocalLightingData(CGlobals::GetLightManager()->GetLastResult(), CGlobals::GetLightManager()->GetLastResultNum());
}

void EffectManager::applyObjectLocalLighting(CBaseObject* pObj)
{
	if (IsLocalLightingEnabled() && CGlobals::GetLightManager()->GetNumLights()>0)
	{
		//////////////////////////////////////////////////////////////////////////
		// Do local lighting: find nearby lights, and enable them.
		IViewClippingObject* pViewObj =  pObj->GetViewClippingObject();
		Vector3 vCenter = pViewObj->GetRenderOffset();
		vCenter.y += pViewObj->GetHeight()/2;
		CGlobals::GetLightManager()->GetBestLights(vCenter, pViewObj->GetRadius(),m_nMaxLocalLightsNum);
		applyLocalLightingData();
	}
	else
	{
		applyLocalLightingData(NULL, 0);
	}
}

bool EffectManager::BeginEffectFF(int nHandle)
{
	if(nHandle < 0)
		return false;

	auto pRenderDevice = CGlobals::GetRenderDevice();
	if (!pRenderDevice)
		return false;

	switch(nHandle)
	{
	case TECH_FULL_SCREEN_GLOW:
	case TECH_SCREEN_WAVE:
	case TECH_SIMPLE_MESH_NORMAL_INSTANCED:
	case TECH_CHARACTER_SKINNED:
	case TECH_BMAX_SKINNED:
	case TECH_SHADOWMAP_BLUR:
		return false;
	case TECH_OCEAN_UNDERWATER:
	{
		EnableLocalLighting(false);
		pRenderDevice->SetTexture(0, nullptr);
		EnableZTest(false);
		pRenderDevice->SetRenderState(ERenderState::ZWRITEENABLE, FALSE);
		pRenderDevice->SetRenderState(ERenderState::ALPHABLENDENABLE, TRUE);
		pRenderDevice->SetRenderState(ERenderState::ALPHATESTENABLE, FALSE);
		pRenderDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_SRCALPHA);
		pRenderDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_INVSRCALPHA);
		SetVertexDeclaration(S0_POS2_COLOR);
		break;
	}
	case TECH_GUI:
	case TECH_NONE:
	{
		EnableLocalLighting(false);
		pRenderDevice->SetTexture(0, nullptr);
		EnableZTest(false);
		pRenderDevice->SetRenderState(ERenderState::ZWRITEENABLE, FALSE);
		pRenderDevice->SetRenderState(ERenderState::ALPHABLENDENABLE, TRUE);
		pRenderDevice->SetRenderState(ERenderState::ALPHATESTENABLE, FALSE);
		pRenderDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_SRCALPHA);
		pRenderDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_INVSRCALPHA);
		SetVertexDeclaration(S0_POS_TEX0_COLOR);
		break;
	}
	default:
		SetSamplerState(0, ESamplerStateType::ADDRESSU, D3DTADDRESS_WRAP);
		SetSamplerState(0, ESamplerStateType::ADDRESSV, D3DTADDRESS_WRAP);
		EnableZTest(true);
		pRenderDevice->SetRenderState(ERenderState::ZWRITEENABLE, TRUE);
		pRenderDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_SRCALPHA);
		pRenderDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_INVSRCALPHA);
		SetVertexDeclaration(S0_POS_NORM_TEX0);
		break;
	}
	if(IsClipPlaneEnabled())
		EnableClipPlaneImmediate(ClipPlane_Enabled_WorldSpace);
	return true;
}

CEffectFile* EffectManager::CheckLoadEffect( int nHandle )
{
	CEffectFile* pEffect = GetEffectByHandle(nHandle);
	if( pEffect != 0  )
	{
		pEffect->LoadAsset();
		if(pEffect->IsValid())
		{
			return pEffect;
		}
	}
	return NULL;
}

bool EffectManager::BeginEffect(int nHandle, CEffectFile** pOutEffect)
{
	m_bEffectValid = BeginEffectShader(nHandle, pOutEffect);
	return m_bEffectValid;
}

bool EffectManager::BeginEffectShader(int nHandle, CEffectFile** pOutEffect)
{
	if(nHandle < 0)
	{
		return false;
	}

	auto pd3dDevice = CGlobals::GetRenderDevice();
	if (!pd3dDevice)
		return false;

	CEffectFile* pEffect = GetEffectByHandle(nHandle);
	if( pEffect != 0  )
	{
		pEffect->LoadAsset();
		if(!pEffect->IsValid())
		{
			//////////////////////////////////////////////////////////////////////////
			// handle effect fall back policy. Use a lower effect or fall back if there is none
			//////////////////////////////////////////////////////////////////////////
			const AssetKey& sEffectFileName = pEffect->GetKey();

			if(sEffectFileName==":IDR_FX_TERRAIN_NORMAL")
			{
				pEffect = 0;
				GetScene()->SetShadow(false);
			}
			else if(sEffectFileName==":IDR_FX_SIMPLE_MESH_NORMAL" || sEffectFileName==":IDR_FX_SIMPLE_MESH_SHADOW" || sEffectFileName == ":IDR_FX_SIMPLE_MESH_NORMAL_SPECIALEFFECTS")
			{
				pEffect = GetEffectByName("simple_mesh_normal_low");
				if(pEffect)
				{
					pEffect->LoadAsset();
					if(!pEffect->IsValid())
					{
						pEffect = 0;
					}
				}
			}
			else
			{
				pEffect = 0;
			}
			MapHandleToEffect(nHandle, pEffect);
		}
	}

	if(pOutEffect!=NULL)
		*pOutEffect = pEffect;
	if(	(pEffect == 0) ||
		((m_pCurrentEffect == pEffect) && (m_nCurrentEffect == nHandle)) ) 
	{
		if(m_nCurrentEffect!=nHandle)
			EndEffect();
		m_nCurrentEffect = nHandle;
		m_pCurrentEffect = pEffect;
		if(pEffect == 0)
		{
			return BeginEffectFF(nHandle);
		}
		return true;
	}

	int nLastEffectHandle = m_nCurrentEffect;
	if(m_nCurrentEffect!=nHandle)
	{
		EndEffect();
		m_nCurrentEffect = nHandle;
	}

	if(m_pCurrentEffect != pEffect)
	{
		m_pCurrentEffect = pEffect;
		m_pCurrentEffect->OnSwitchInShader();
	}

	const AssetKey& sEffectFileName = m_pCurrentEffect->GetKey();
	if(sEffectFileName == ":IDR_FX_SIMPLE_MESH_NORMAL_SPECIALEFFECTS")
	{
		if(m_nCurrentEffect == TECH_SIMPLE_MESH_NORMAL_TRANSPARENT)
		{
			m_pCurrentEffect->SetTechniqueByIndex(0);
		}
	}

	bool bEnableLight = GetScene()->IsLightEnabled();
	bool bEnableSunLight = GetScene()->IsSunLightEnabled();

	//////////////////////////////////////////////////////////////////////////
	// Programmable RHI pipeline setup across all backends
	//////////////////////////////////////////////////////////////////////////
	VertexDeclarationPtr pDecl=NULL;
	switch(nHandle)
	{
	case TECH_UNKNOWN:
	{
#ifndef __EMSCRIPTEN__
		pd3dDevice->SetVertexDeclaration(0);
#endif
		break;
	}
	case TECH_NONE:
	{
#ifndef __EMSCRIPTEN__
		pd3dDevice->SetIndices(0);
		pd3dDevice->SetStreamSource(0, 0, 0, 0);
#endif
		pd3dDevice->SetRenderState(ERenderState::CULLMODE, RSV_CULL_NONE);
		EnableZTest(false);
		pd3dDevice->SetRenderState(ERenderState::ZWRITEENABLE, TRUE);
		pd3dDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_SRCALPHA);
		pd3dDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_INVSRCALPHA);

		SetSamplerState(0, ESamplerStateType::MINFILTER, D3DTEXF_LINEAR, true);
		SetSamplerState(0, ESamplerStateType::MAGFILTER, D3DTEXF_LINEAR, true);
		break;
	}
	case TECH_GUI_TEXT:
	{
		if (nLastEffectHandle != TECH_GUI && nLastEffectHandle != TECH_GUI_TEXT)
		{
			pDecl = GetVertexDeclaration(S0_POS_TEX0_COLOR);
			if (pDecl == 0)
				return false;
			pd3dDevice->SetIndices(0);
			pd3dDevice->SetStreamSource(0, 0, 0, 0);
			pd3dDevice->SetVertexDeclaration(pDecl);
			pd3dDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_SRCALPHA);
			pd3dDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_INVSRCALPHA);
		}
		SetSamplerState(0, ESamplerStateType::MINFILTER, D3DTEXF_LINEAR, true);
		SetSamplerState(0, ESamplerStateType::MAGFILTER, D3DTEXF_LINEAR, true);
		break;
	}
	case TECH_GUI:
	{
		if (nLastEffectHandle != TECH_GUI && nLastEffectHandle != TECH_GUI_TEXT)
		{
			pDecl = GetVertexDeclaration(S0_POS_TEX0_COLOR);
			if (pDecl == 0)
				return false;
			pd3dDevice->SetIndices(0);
			pd3dDevice->SetStreamSource(0, 0, 0, 0);
			pd3dDevice->SetVertexDeclaration(pDecl);
			pd3dDevice->SetRenderState(ERenderState::CULLMODE, RSV_CULL_NONE);
			EnableZTest(false);
			pd3dDevice->SetRenderState(ERenderState::ZWRITEENABLE, TRUE);
			pd3dDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_SRCALPHA);
			pd3dDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_INVSRCALPHA);
			pd3dDevice->SetRenderState(ERenderState::ALPHABLENDENABLE, TRUE);
		}
		const DWORD dwGuiFilter = (CGUIRoot::GetInstance() && CGUIRoot::GetInstance()->GetUsePointTextureFiltering()) ? D3DTEXF_POINT : D3DTEXF_LINEAR;
		SetSamplerState(0, ESamplerStateType::MINFILTER, dwGuiFilter, true);
		SetSamplerState(0, ESamplerStateType::MAGFILTER, dwGuiFilter, true);
		break;
	}
	case TECH_SINGLE_COLOR:
	{
		pDecl = GetVertexDeclaration(S0_POS_COLOR);
		if (pDecl == 0)
			return false;
		pd3dDevice->SetIndices(0);
		pd3dDevice->SetStreamSource(0, 0, 0, 0);
		pd3dDevice->SetVertexDeclaration(pDecl);
		pd3dDevice->SetRenderState(ERenderState::CULLMODE, RSV_CULL_CCW);
		EnableZTest(true);
		pd3dDevice->SetRenderState(ERenderState::ZWRITEENABLE, TRUE);
		pd3dDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_SRCALPHA);
		pd3dDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_INVSRCALPHA);
		SetSamplerState(0, ESamplerStateType::MINFILTER, D3DTEXF_POINT, true);
		SetSamplerState(0, ESamplerStateType::MAGFILTER, D3DTEXF_POINT, true);
		break;
	}
	case TECH_OCCLUSION_TEST:
	{
		pDecl = GetVertexDeclaration(S0_POS_TEX0);
		if(pDecl == 0)
			return false;
		pd3dDevice->SetVertexDeclaration(pDecl);
		EnableZTest(true);

		EnableGlobalLighting(false);
		EnableLocalLighting(false);

		// using a simple shader instead of fixed programming pipeline for occlusion testing
		if(m_pCurrentEffect->begin(true, 0, false))
			m_pCurrentEffect->BeginPass(0);
		break;
	}
	case TECH_SIMPLE_MESH:
	{
		pDecl = GetVertexDeclaration(S0_POS_TEX0);
		if(pDecl == 0)
			return false;
		pd3dDevice->SetVertexDeclaration(pDecl);

		DisableD3DAlphaTesting(true);
		pEffect->EnableAlphaBlending(false);
		pEffect->EnableAlphaTesting(false);
		pEffect->applySurfaceMaterial(&CGlobals::GetSceneState()->GetGlobalMaterial());

		applyFogParameters();

		EnableZTest(true);
		pd3dDevice->SetRenderState(ERenderState::ALPHABLENDENABLE, TRUE);
		pd3dDevice->SetRenderState(ERenderState::ALPHATESTENABLE, FALSE);
		pd3dDevice->SetRenderState(ERenderState::ZWRITEENABLE, TRUE);
		pd3dDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_SRCALPHA);
		pd3dDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_INVSRCALPHA);
		SetCullingMode(true);

		pd3dDevice->SetSamplerState(0, ESamplerStateType::MIPFILTER, D3DTEXF_LINEAR);
		SetSamplerState(0, ESamplerStateType::MINFILTER, D3DTEXF_LINEAR, true);
		SetSamplerState(0, ESamplerStateType::MAGFILTER, D3DTEXF_LINEAR, true);
		pd3dDevice->SetSamplerState(0, ESamplerStateType::ADDRESSU, D3DTADDRESS_WRAP);
		pd3dDevice->SetSamplerState(0, ESamplerStateType::ADDRESSV, D3DTADDRESS_WRAP);
		break;
	}
	case TECH_SKY_DOME:
	{
		pDecl = GetVertexDeclaration(S0_POS_TEX0);
		if(pDecl == 0)
			return false;
		pd3dDevice->SetVertexDeclaration(pDecl);

		DisableD3DAlphaTesting(true);
		pEffect->EnableAlphaBlending(false);
		pEffect->EnableAlphaTesting(false);
		pEffect->applySurfaceMaterial(&CGlobals::GetSceneState()->GetGlobalMaterial());
		applyFogParameters();

		EnableZTest(true);
		pd3dDevice->SetRenderState(ERenderState::ZFUNC, D3DCMP_LESSEQUAL);
		pd3dDevice->SetRenderState(ERenderState::ALPHABLENDENABLE, TRUE);
		pd3dDevice->SetRenderState(ERenderState::ALPHATESTENABLE, FALSE);
		pd3dDevice->SetRenderState(ERenderState::ZWRITEENABLE, FALSE);
		pd3dDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_SRCALPHA);
		pd3dDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_INVSRCALPHA);
		SetCullingMode(false);
		DisableD3DCulling(true);

		pd3dDevice->SetSamplerState(0, ESamplerStateType::MIPFILTER, D3DTEXF_LINEAR);
		SetSamplerState(0, ESamplerStateType::MINFILTER, D3DTEXF_LINEAR, true);
		SetSamplerState(0, ESamplerStateType::MAGFILTER, D3DTEXF_LINEAR, true);
		pd3dDevice->SetSamplerState(0, ESamplerStateType::ADDRESSU, D3DTADDRESS_WRAP);
		pd3dDevice->SetSamplerState(0, ESamplerStateType::ADDRESSV, D3DTADDRESS_WRAP);
		pd3dDevice->SetSamplerState(1, ESamplerStateType::ADDRESSU, D3DTADDRESS_WRAP);
		pd3dDevice->SetSamplerState(1, ESamplerStateType::ADDRESSV, D3DTADDRESS_WRAP);
		break;
	}
	case TECH_SIMPLE_MESH_NORMAL:
	case TECH_SIMPLE_MESH_NORMAL_UNLIT:
	case TECH_CHARACTER:
	case TECH_CHARACTER_SKINNED:
	case TECH_SIMPLE_MESH_NORMAL_CTOR:
	case TECH_SIMPLE_MESH_NORMAL_VEGETATION:
	case TECH_SIMPLE_MESH_NORMAL_SELECTED:
	case TECH_SIMPLE_MESH_NORMAL_BORDER:
	case TECH_SIMPLE_MESH_NORMAL_TRANSPARENT:
	case TECH_SKY_MESH:
	case TECH_SIMPLE_MESH_NORMAL_SHADOW:
	case TECH_SIMPLE_MESH_NORMAL_TEX2:
	case TECH_SIMPLE_MESH_NORMAL_INSTANCED:
	{
		if(nHandle == TECH_SIMPLE_MESH_NORMAL_TEX2)
			pDecl = GetVertexDeclaration(S0_POS_NORM_TEX0_TEX1);
		else if(nHandle == TECH_SIMPLE_MESH_NORMAL_INSTANCED)
			pDecl = GetVertexDeclaration(S0_POS_NORM_TEX0_INSTANCED);
		else if(nHandle == TECH_CHARACTER_SKINNED)
			pDecl = GetVertexDeclaration(S0_POS_NORM_TEX0_SKINNED);
		else
			pDecl = GetVertexDeclaration(S0_POS_NORM_TEX0);

		if(pDecl == 0)
			return false;

		pd3dDevice->SetVertexDeclaration(pDecl);

		DisableD3DAlphaTesting(true);

		pEffect->EnableAlphaBlending(false);
		pEffect->EnableAlphaTesting(false);

		EnableZTest(true);
		pd3dDevice->SetRenderState(ERenderState::ALPHATESTENABLE, FALSE);
		pd3dDevice->SetRenderState(ERenderState::ALPHABLENDENABLE, TRUE); // force blending
		pd3dDevice->SetRenderState(ERenderState::ZWRITEENABLE, (nHandle == TECH_SKY_MESH) ? FALSE : TRUE);
		pd3dDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_SRCALPHA);
		pd3dDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_INVSRCALPHA);
		SetCullingMode(true);

		pd3dDevice->SetSamplerState(0, ESamplerStateType::MIPFILTER, D3DTEXF_LINEAR);
		SetSamplerState(0, ESamplerStateType::MINFILTER, D3DTEXF_LINEAR, true);
		SetSamplerState(0, ESamplerStateType::MAGFILTER, D3DTEXF_LINEAR, true);
		if(nHandle == TECH_SIMPLE_MESH_NORMAL || nHandle == TECH_CHARACTER || nHandle == TECH_CHARACTER_SKINNED || nHandle == TECH_SIMPLE_MESH_NORMAL_VEGETATION || nHandle == TECH_SIMPLE_MESH_NORMAL_SHADOW || nHandle == TECH_SIMPLE_MESH_NORMAL_INSTANCED || nHandle == TECH_SIMPLE_MESH_NORMAL_UNLIT)
		{
			pEffect->EnableNormalMap(false);
			pEffect->EnableReflectionMapping(false);
			pEffect->EnableEnvironmentMapping(false);

			EnableGlobalLighting(bEnableSunLight && bEnableLight);
			EnableLocalLighting(bEnableLight);

			pd3dDevice->SetSamplerState(0, ESamplerStateType::ADDRESSU, D3DTADDRESS_WRAP);
			pd3dDevice->SetSamplerState(0, ESamplerStateType::ADDRESSV, D3DTADDRESS_WRAP);

			applyFogParameters();

			if(nHandle == TECH_SIMPLE_MESH_NORMAL_SHADOW)
			{
				if(GetScene()->IsShadowMapEnabled())
				{
					CShadowMap* pShadowMap = GetShadowMap();
					if (pShadowMap != NULL)
					{
						EnableUsingShadowMap(true);
					}
					pEffect->SetShadowRadius(GetScene()->GetShadowRadius());
				}
			}
		}
		else if(nHandle == TECH_SIMPLE_MESH_NORMAL_TRANSPARENT)
		{
			EnableGlobalLighting(bEnableSunLight && bEnableLight);
			EnableLocalLighting(false);
			pEffect->applyLocalLightingData(NULL, 0);

			SetCullingMode(false);
			DisableD3DCulling(true);

			pd3dDevice->SetSamplerState(0, ESamplerStateType::ADDRESSU, D3DTADDRESS_WRAP);
			pd3dDevice->SetSamplerState(0, ESamplerStateType::ADDRESSV, D3DTADDRESS_WRAP);
		}
		else if(nHandle == TECH_SIMPLE_MESH_NORMAL_CTOR)
		{
			EnableGlobalLighting(bEnableSunLight && bEnableLight);
			EnableLocalLighting(false);
			pEffect->applyLocalLightingData(NULL, 0);

			SetCullingMode(false);
			DisableD3DCulling(true);

			pd3dDevice->SetSamplerState(0, ESamplerStateType::ADDRESSU, D3DTADDRESS_WRAP);
			pd3dDevice->SetSamplerState(0, ESamplerStateType::ADDRESSV, D3DTADDRESS_WRAP);
		}
		else if(nHandle == TECH_SIMPLE_MESH_NORMAL_SELECTED)
		{
			EnableGlobalLighting(bEnableSunLight && bEnableLight);
			EnableLocalLighting(bEnableLight);

			float fIntensity = 1.25f + g_flash;
			Vector4 fogParam(0, 1.f, 1.0f, 0.1f);
			LinearColor fogColor = CGlobals::GetSettings().GetSelectionColor();
			fogColor.r *= fIntensity;
			fogColor.g *= fIntensity;
			fogColor.b *= fIntensity;
			fogColor.a = 1.f;
			pEffect->applyFogParameters(true, &fogParam, &fogColor);

			pd3dDevice->SetSamplerState(0, ESamplerStateType::ADDRESSU, D3DTADDRESS_WRAP);
			pd3dDevice->SetSamplerState(0, ESamplerStateType::ADDRESSV, D3DTADDRESS_WRAP);
		}
		else if(nHandle == TECH_SIMPLE_MESH_NORMAL_TEX2)
		{
			EnableGlobalLighting(bEnableSunLight && bEnableLight);
			EnableLocalLighting(bEnableLight);
			applyFogParameters();

			pd3dDevice->SetSamplerState(0, ESamplerStateType::ADDRESSU, D3DTADDRESS_WRAP);
			pd3dDevice->SetSamplerState(0, ESamplerStateType::ADDRESSV, D3DTADDRESS_WRAP);
		}
		else if(nHandle == TECH_SKY_MESH)
		{
			pd3dDevice->SetSamplerState(0, ESamplerStateType::ADDRESSU, D3DTADDRESS_CLAMP);
			pd3dDevice->SetSamplerState(0, ESamplerStateType::ADDRESSV, D3DTADDRESS_CLAMP);
			applyFogParameters();
		}
		else if(nHandle == TECH_SIMPLE_MESH_NORMAL_BORDER)
		{
			DisableZWrite(true);
		}

#ifdef ENABLE_SHARE_MODE_SHADER
		pEffect->BeginSharePassMode();
#endif
		break;
	}
	case TECH_SIMPLE_CAD_MODEL:
	{
		pDecl = GetVertexDeclaration(S0_POS_NORM_TEX0);
		if(pDecl == 0)
			return false;
		pd3dDevice->SetVertexDeclaration(pDecl);

		DisableD3DAlphaTesting(true);

		pEffect->EnableAlphaBlending(false);
		pEffect->EnableAlphaTesting(false);

		EnableZTest(true);
		pd3dDevice->SetRenderState(ERenderState::ALPHATESTENABLE, FALSE);
		pd3dDevice->SetRenderState(ERenderState::ALPHABLENDENABLE, TRUE);
		pd3dDevice->SetRenderState(ERenderState::ZWRITEENABLE, TRUE);
		pd3dDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_SRCALPHA);
		pd3dDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_INVSRCALPHA);
		SetCullingMode(false);

		pd3dDevice->SetSamplerState(0, ESamplerStateType::MIPFILTER, D3DTEXF_LINEAR);
		SetSamplerState(0, ESamplerStateType::MINFILTER, D3DTEXF_LINEAR, true);
		SetSamplerState(0, ESamplerStateType::MAGFILTER, D3DTEXF_LINEAR, true);
		break;
	}
	case TECH_SIMPLE_DROPSHADOW:
	{
		pDecl = GetVertexDeclaration(S0_POS_TEX0);
		if(pDecl == 0)
			return false;
		pd3dDevice->SetVertexDeclaration(pDecl);
		DisableD3DAlphaTesting(true);
		EnableZTest(true);

		pEffect->EnableAlphaBlending(true);
		pd3dDevice->SetRenderState(ERenderState::ALPHABLENDENABLE, TRUE);
		break;
	}
	case TECH_OCEAN_SIMPLE:
	{
		pDecl = GetVertexDeclaration(S0_POS_TEX0);
		if(pDecl == 0)
			return false;
		pd3dDevice->SetVertexDeclaration(pDecl);
		EnableZTest(true);

		applyFogParameters();
		break;
	}
	case TECH_OCEAN_CLOUD:
	{
		pDecl = GetVertexDeclaration(S0_S1_OCEAN_CLOUD);
		if(pDecl == 0)
			return false;
		EnableZTest(true);
		applyFogParameters();
		pd3dDevice->SetVertexDeclaration(pDecl);
		break;
	}
	case TECH_OCEAN:
	{
		if(CGlobals::GetOceanManager()->IsDrawShoreLine())
			pDecl = GetVertexDeclaration(S0_S1_S2_S3_OCEAN_FFT);
		else
			pDecl = GetVertexDeclaration(S0_S1_S2_OCEAN_FFT);

		if(pDecl == 0)
			return false;
		EnableZTest(true);
		applyFogParameters();
		pd3dDevice->SetVertexDeclaration(pDecl);
		break;
	}
	case TECH_TERRAIN:
	{
		EnableGlobalLighting(bEnableSunLight && bEnableLight);
		EnableLocalLighting(bEnableLight);
		if(GetScene()->IsShadowMapEnabled())
		{
			CShadowMap* pShadowMap = GetShadowMap();
			if (pShadowMap != NULL)
			{
				EnableUsingShadowMap(true);
			}
			pEffect->SetShadowRadius(GetScene()->GetShadowRadius());
		}

		applyLocalLightingData();
		applyFogParameters();

		ParaMaterial matTerrain = CGlobals::GetSceneState()->GetGlobalMaterial();

		BlockWorldClient* pBlockWorldClient = BlockWorldClient::GetInstance();
		if(pBlockWorldClient && pBlockWorldClient->IsInBlockWorld())
		{
			float fLightness = max(1.f - fabs(CGlobals::GetScene()->GetSunLight().GetTimeOfDaySTD()), 0.25f);
			matTerrain.Ambient = LinearColor(fLightness*0.7f, fLightness*0.7f, fLightness*0.7f, 1.f);
			matTerrain.Diffuse = LinearColor(fLightness*0.4f, fLightness*0.4f, fLightness*0.4f, 1.f);
		}
		else
		{
			matTerrain.Diffuse = LinearColor(1.f, 1.f, 1.f, 1.f);
		}
		pEffect->applySurfaceMaterial(&matTerrain);

		SetCullingMode(true);
		EnableZTest(true);
		pd3dDevice->SetRenderState(ERenderState::ALPHABLENDENABLE, FALSE);
		pd3dDevice->SetRenderState(ERenderState::ALPHATESTENABLE, FALSE);
		pd3dDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_ONE);
		pd3dDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_ONE);
		pd3dDevice->SetRenderState(ERenderState::ZWRITEENABLE, TRUE);
		pd3dDevice->SetRenderState(ERenderState::ZFUNC, D3DCMP_LESSEQUAL);

		if(CGlobals::GetGlobalTerrain()->GetSettings()->UseNormals())
		{
			pDecl = GetVertexDeclaration(S0_POS_NORM_TEX0_TEX1);
		}
		else
		{
			pDecl = GetVertexDeclaration(S0_POS_TEX0_TEX1);
		}

		if(pDecl == 0)
			return false;
		pd3dDevice->SetVertexDeclaration(pDecl);
		break;
	}
	case TECH_FULL_SCREEN_GLOW:
	{
		pDecl = GetVertexDeclaration(S0_POS_TEX0);
		if(pDecl == 0)
			return false;
		pd3dDevice->SetVertexDeclaration(pDecl);
		break;
	}
	case TECH_SCREEN_WAVE:
	{
		pDecl = GetVertexDeclaration(S0_POS_TEX0);
		if(pDecl == 0)
			return false;
		pd3dDevice->SetVertexDeclaration(pDecl);
		break;
	}
	case TECH_SHADOWMAP_BLUR:
	{
		pDecl = GetVertexDeclaration(S0_POS_TEX0);
		if(pDecl == 0)
			return false;
		pd3dDevice->SetVertexDeclaration(pDecl);
		break;
	}
	case TECH_PARTICLES:
	case TECH_WATER_RIPPLE:
	{
		pDecl = GetVertexDeclaration(S0_POS_TEX0_COLOR);
		if(pDecl == 0)
			return false;
		pd3dDevice->SetVertexDeclaration(pDecl);

		EnableZTest(true);
		pd3dDevice->SetRenderState(ERenderState::ALPHATESTENABLE, FALSE);
		pd3dDevice->SetRenderState(ERenderState::ALPHABLENDENABLE, TRUE);
		pd3dDevice->SetRenderState(ERenderState::ZWRITEENABLE, FALSE);
		pd3dDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_SRCALPHA);
		pd3dDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_INVSRCALPHA);
		SetCullingMode(false);

		pd3dDevice->SetSamplerState(0, ESamplerStateType::MIPFILTER, D3DTEXF_LINEAR);
		SetSamplerState(0, ESamplerStateType::MINFILTER, D3DTEXF_LINEAR, true);
		SetSamplerState(0, ESamplerStateType::MAGFILTER, D3DTEXF_LINEAR, true);
		break;
	}
	case TECH_BMAX_MODEL:
	case TECH_BMAX_SKINNED:
	{
		pDecl = GetVertexDeclaration((nHandle == TECH_BMAX_SKINNED) ? S0_POS_NORM_COLOR_SKINNED : S0_POS_NORM_COLOR);
		if (pDecl == 0)
			return false;
		pd3dDevice->SetVertexDeclaration(pDecl);

		pEffect->EnableAlphaBlending(false);
		pEffect->EnableAlphaTesting(false);
		applyFogParameters();

		EnableZTest(true);
		pd3dDevice->SetRenderState(ERenderState::ALPHATESTENABLE, FALSE);
		pd3dDevice->SetRenderState(ERenderState::ZWRITEENABLE, TRUE);
		pd3dDevice->SetRenderState(ERenderState::ALPHABLENDENABLE, FALSE);
		SetCullingMode(true);
#ifdef ENABLE_SHARE_MODE_SHADER
		pEffect->BeginSharePassMode();
#endif
		break;
	}
	case TECH_LIGHT_DIRECTIONAL:
	case TECH_LIGHT_POINT:
	case TECH_LIGHT_SPOT:
	{
		pDecl = GetVertexDeclaration(S0_POS_NORM_COLOR);
		if (pDecl == 0)
			return false;
		pd3dDevice->SetVertexDeclaration(pDecl);

		pEffect->EnableAlphaBlending(true);
		pEffect->EnableAlphaTesting(false);

		applyFogParameters();

		EnableZTest(true);
		pd3dDevice->SetRenderState(ERenderState::ALPHATESTENABLE, FALSE);
		pd3dDevice->SetRenderState(ERenderState::ALPHABLENDENABLE, TRUE);
		pd3dDevice->SetRenderState(ERenderState::ZWRITEENABLE, TRUE);
		pd3dDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_SRCALPHA);
		pd3dDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_INVSRCALPHA);
		pd3dDevice->SetSamplerState(0, ESamplerStateType::ADDRESSU, D3DTADDRESS_WRAP);
		pd3dDevice->SetSamplerState(0, ESamplerStateType::ADDRESSV, D3DTADDRESS_WRAP);
		SetCullingMode(false);
		break;
	}
	case TECH_BLOCK_FANCY:
	case TECH_BLOCK:
	{
		pEffect->EnableAlphaBlending(false);
		pEffect->EnableAlphaTesting(false);

		applyFogParameters();

		EnableZTest(true);
		pd3dDevice->SetRenderState(ERenderState::ALPHATESTENABLE, FALSE);
		pd3dDevice->SetRenderState(ERenderState::ALPHABLENDENABLE, TRUE);
		pd3dDevice->SetRenderState(ERenderState::ZWRITEENABLE, TRUE);
		pd3dDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_SRCALPHA);
		pd3dDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_INVSRCALPHA);
		pd3dDevice->SetSamplerState(0, ESamplerStateType::ADDRESSU, D3DTADDRESS_WRAP);
		pd3dDevice->SetSamplerState(0, ESamplerStateType::ADDRESSV, D3DTADDRESS_WRAP);
		SetSamplerState(0, ESamplerStateType::MINFILTER, D3DTEXF_POINT, true);
		SetSamplerState(0, ESamplerStateType::MAGFILTER, D3DTEXF_POINT, true);
		SetCullingMode(true);
		break;
	}
	default:
	{
		if(nHandle > 1000)
		{
			pDecl = GetVertexDeclaration(S0_POS_NORM_TEX0);
			if(pDecl == 0)
				return false;

			pd3dDevice->SetVertexDeclaration(pDecl);
			DisableD3DAlphaTesting(true);
			pEffect->EnableAlphaBlending(false);
			pEffect->EnableAlphaTesting(false);
			EnableZTest(true);
			pd3dDevice->SetRenderState(ERenderState::ALPHATESTENABLE, FALSE);
			pd3dDevice->SetRenderState(ERenderState::ALPHABLENDENABLE, TRUE);
			pd3dDevice->SetRenderState(ERenderState::ZWRITEENABLE, TRUE);
			pd3dDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_SRCALPHA);
			pd3dDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_INVSRCALPHA);
			SetCullingMode(true);
			pd3dDevice->SetSamplerState(0, ESamplerStateType::MIPFILTER, D3DTEXF_LINEAR);
			SetSamplerState(0, ESamplerStateType::MINFILTER, D3DTEXF_LINEAR, true);
			SetSamplerState(0, ESamplerStateType::MAGFILTER, D3DTEXF_LINEAR, true);

			pEffect->EnableNormalMap(false);
			pEffect->EnableReflectionMapping(false);
			pEffect->EnableEnvironmentMapping(false);

			EnableGlobalLighting(bEnableSunLight && bEnableLight);
			EnableLocalLighting(bEnableLight);

			pd3dDevice->SetSamplerState(0, ESamplerStateType::ADDRESSU, D3DTADDRESS_WRAP);
			pd3dDevice->SetSamplerState(0, ESamplerStateType::ADDRESSV, D3DTADDRESS_WRAP);

			applyFogParameters();

			if(pEffect->GetParamBlock())
			{
				pEffect->GetParamBlock()->ApplyToEffect(pEffect);
			}
		}
		else if (pEffect->GetParamBlock())
		{
			pEffect->GetParamBlock()->ApplyToEffect(pEffect);
		}
		break;
	}
	}

	// Shared by all programmable pipeline shaders
	if(IsClipPlaneEnabled())
		EnableClipPlaneImmediate(ClipPlane_Enabled_ClipSpace);

	return true;
}

void EffectManager::EndEffect()
{
	auto pd3dDevice = CGlobals::GetRenderDevice();
	if (!pd3dDevice)
	{
		m_nCurrentEffect = 0;
		m_pCurrentEffect = NULL;
		return;
	}

	if((m_pCurrentEffect == 0) || (!m_pCurrentEffect->IsValid()))
	{
		switch(m_nCurrentEffect)
		{
		case TECH_OCEAN_UNDERWATER:
		case TECH_GUI:
		case TECH_GUI_TEXT:
		case TECH_NONE:
			EnableZTest(true);
			pd3dDevice->SetRenderState(ERenderState::ZWRITEENABLE, TRUE);
			SetSamplerState(0, ESamplerStateType::MINFILTER, D3DTEXF_LINEAR, true);
			SetSamplerState(0, ESamplerStateType::MAGFILTER, D3DTEXF_LINEAR, true);
			break;
		default:
			break;
		}
		EnableLocalLighting(false);
	}
	else
	{
		m_pCurrentEffect->OnSwitchOutShader();

		if(m_pCurrentEffect->IsInShareMode())
		{
			m_pCurrentEffect->EndSharePassMode();
		}

		switch(m_nCurrentEffect)
		{
		case TECH_OCCLUSION_TEST:
			m_pCurrentEffect->EndPass();
			m_pCurrentEffect->end();
			break;
		case TECH_SKY_MESH:
			pd3dDevice->SetSamplerState(0, ESamplerStateType::ADDRESSU, D3DTADDRESS_WRAP);
			pd3dDevice->SetSamplerState(0, ESamplerStateType::ADDRESSV, D3DTADDRESS_WRAP);
			pd3dDevice->SetRenderState(ERenderState::ZWRITEENABLE, TRUE);
			break;
		case TECH_SKY_DOME:
			pd3dDevice->SetSamplerState(0, ESamplerStateType::ADDRESSU, D3DTADDRESS_WRAP);
			pd3dDevice->SetSamplerState(0, ESamplerStateType::ADDRESSV, D3DTADDRESS_WRAP);
			pd3dDevice->SetRenderState(ERenderState::ZWRITEENABLE, TRUE);
			DisableD3DCulling(false);
			SetCullingMode(true);
			break;
		case TECH_SIMPLE_MESH_NORMAL:
		case TECH_SIMPLE_MESH_NORMAL_UNLIT:
		case TECH_SIMPLE_MESH_NORMAL_VEGETATION:
		case TECH_SIMPLE_MESH_NORMAL_SHADOW:
		case TECH_SIMPLE_MESH_NORMAL_TEX2:
		case TECH_SIMPLE_MESH_NORMAL_INSTANCED:
		case TECH_SIMPLE_MESH_NORMAL_TRANSPARENT:
		case TECH_SIMPLE_MESH_NORMAL_SELECTED:
		case TECH_SIMPLE_MESH_NORMAL_BORDER:
			if(m_nCurrentEffect == TECH_SIMPLE_MESH_NORMAL_SHADOW)
			{
				if(GetScene()->IsShadowMapEnabled())
				{
					CShadowMap* pShadowMap = GetShadowMap();
					if (pShadowMap != NULL)
					{
						EnableUsingShadowMap(false);
						m_pCurrentEffect->EnableShadowmap(0);
						pd3dDevice->SetTexture(2, nullptr);
					}
				}
			}
			else if(m_nCurrentEffect == TECH_SIMPLE_MESH_NORMAL_INSTANCED)
			{
				pd3dDevice->SetStreamSourceFreq(0, 1);
				pd3dDevice->SetStreamSourceFreq(1, 1);
			}
			else if(m_nCurrentEffect == TECH_SIMPLE_MESH_NORMAL_TRANSPARENT)
			{
				pd3dDevice->SetRenderState(ERenderState::ZWRITEENABLE, TRUE);
				DisableD3DCulling(false);
				SetCullingMode(true);
			}
			else if(m_nCurrentEffect == TECH_SIMPLE_MESH_NORMAL_BORDER)
			{
				DisableZWrite(false);
			}
			EnableLocalLighting(false);
			break;
		case TECH_CHARACTER:
		case TECH_CHARACTER_SKINNED:
			EnableLocalLighting(false);
			break;
		case TECH_SIMPLE_MESH_NORMAL_CTOR:
			pd3dDevice->SetRenderState(ERenderState::ALPHATESTENABLE, FALSE);
			DisableD3DCulling(false);
			SetCullingMode(true);
			break;
		case TECH_OCEAN_CLOUD:
		case TECH_OCEAN_SIMPLE:
		case TECH_OCEAN:
			SetCullingMode(true);
			break;
		case TECH_TERRAIN:
		{
			for(int i = 0; i < 8; ++i)
			{
				pd3dDevice->SetTexture(i, nullptr);
			}
			if(GetScene()->IsShadowMapEnabled())
			{
				CShadowMap* pShadowMap = GetShadowMap();
				if (pShadowMap != NULL)
				{
					EnableUsingShadowMap(false);
					m_pCurrentEffect->EnableShadowmap(0);
					pShadowMap->UnsetShadowTexture(2);
				}
			}
			EnableD3DAlphaBlending(true);
			pd3dDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_SRCALPHA);
			pd3dDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_INVSRCALPHA);
			pd3dDevice->SetSamplerState(0, ESamplerStateType::ADDRESSU, D3DTADDRESS_WRAP);
			pd3dDevice->SetSamplerState(0, ESamplerStateType::ADDRESSV, D3DTADDRESS_WRAP);
			break;
		}
		case TECH_PARTICLES:
		case TECH_WATER_RIPPLE:
			SetCullingMode(true);
			pd3dDevice->SetRenderState(ERenderState::ZWRITEENABLE, TRUE);
			break;
		case TECH_GUI:
		case TECH_GUI_TEXT:
		case TECH_NONE:
			EnableZTest(true);
			pd3dDevice->SetRenderState(ERenderState::ZWRITEENABLE, TRUE);
			SetSamplerState(0, ESamplerStateType::MINFILTER, D3DTEXF_LINEAR, true);
			SetSamplerState(0, ESamplerStateType::MAGFILTER, D3DTEXF_LINEAR, true);
			break;
		case TECH_BLOCK_FANCY:
		case TECH_BLOCK:
			SetCullingMode(true);
			pd3dDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_SRCALPHA);
			pd3dDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_INVSRCALPHA);
			SetSamplerState(0, ESamplerStateType::MINFILTER, D3DTEXF_LINEAR, true);
			SetSamplerState(0, ESamplerStateType::MAGFILTER, D3DTEXF_LINEAR, true);
			pd3dDevice->SetStreamSource(0, nullptr, 0, 0);
			pd3dDevice->SetIndices(nullptr);
			pd3dDevice->SetTexture(0, nullptr);
			pd3dDevice->SetTexture(1, nullptr);
			pd3dDevice->SetTexture(2, nullptr);
			pd3dDevice->SetTexture(3, nullptr);
			break;
		default:
			break;
		}
		DisableD3DAlphaTesting(false);
	}

	m_nCurrentEffect = 0;
	m_pCurrentEffect = NULL;
}

void EffectManager::SetAllEffectsTechnique(EffectTechniques nTech)
{
	EndEffect();
	switch(nTech)
	{
	case EFFECT_GEN_SHADOWMAP:
		{
			for (auto& item : m_items)
			{
				((CEffectFile*)(item.second))->SetFirstValidTechniqueByCategory(CEffectFile::TechCategory_GenShadowMap);
			}
			break;
		}
	default:
		{
			for (auto& item : m_items)
			{
				((CEffectFile*)(item.second))->SetFirstValidTechniqueByCategory(CEffectFile::TechCategory_default);
			}
		}
		break;
	}
}

void EffectManager::SetDefaultEffectMapping(int nLevel)
{
	m_nEffectLevel = nLevel;
#ifndef NPLRUNTIME_SERVER
#ifdef _DEBUG
#ifdef TESTING_FIXED_FUNCTION
	nLevel = 0;
	OUTPUT_LOG("all shaders are disabled for testing proposes\r\n");
#endif
#endif
	
	// clear current effect
	if(m_pCurrentEffect!=0)
		EndEffect();
	m_nCurrentEffect = 0;
	m_pCurrentEffect = NULL;

	CEffectFile* pEffect = NULL;

	// Ensure programmable pipeline floor and map SM 3/4/5 to the full shader mapping
	if (nLevel < 20 || nLevel > 30)
		nLevel = 30;

	if(nLevel<10)
	{
		m_HandleMap.clear();
		CGlobals::GetGlobalTerrain()->EnableLighting(false);
	}
	else if(nLevel>=10 && nLevel<=20)
	{
		//////////////////////////////////////////////////////////////////////////
		// 20, VS PS shader version 2
		m_HandleMap.clear();
		//CGlobals::GetGlobalTerrain()->EnableLighting(false);
		//CGlobals::GetOceanManager()->SetRenderTechnique(COceanManager::OCEAN_TECH_FULL);
		
		//pEffect = GetEffectByName("simple_mesh_normal");
		pEffect = GetEffectByName("simple_mesh_normal_low");
		MapHandleToEffect(TECH_SIMPLE_MESH_NORMAL, pEffect);
		MapHandleToEffect(TECH_CHARACTER, pEffect);

		pEffect = GetEffectByName("simple_mesh_normal_shadow");
		MapHandleToEffect(TECH_SIMPLE_MESH_NORMAL_SHADOW, pEffect);

		pEffect = GetEffectByName("simple_cad_model");
		MapHandleToEffect(TECH_SIMPLE_CAD_MODEL, pEffect);

		pEffect = GetEffectByName("dropShadow");
		MapHandleToEffect(TECH_SIMPLE_DROPSHADOW, pEffect);

		pEffect = GetEffectByName("simple_mesh_normal_specialeffects");
		MapHandleToEffect(TECH_SIMPLE_MESH_NORMAL_TRANSPARENT, pEffect);

		// try enable instancing anyway, even though it may not be supported by the card.
		pEffect = GetEffectByName("simple_mesh_normal_instanced");
		MapHandleToEffect(TECH_SIMPLE_MESH_NORMAL_INSTANCED, pEffect);

		pEffect = GetEffectByName("simple_mesh");
		MapHandleToEffect(TECH_SIMPLE_MESH, pEffect);

		pEffect = GetEffectByName("simple_mesh_normal_ctor");
		MapHandleToEffect(TECH_SIMPLE_MESH_NORMAL_CTOR, pEffect);

		pEffect = GetEffectByName("simple_mesh_normal_vegetation");
		MapHandleToEffect(TECH_SIMPLE_MESH_NORMAL_VEGETATION, pEffect);

		pEffect = GetEffectByName("simple_mesh_normal_unlit");
		MapHandleToEffect(TECH_SIMPLE_MESH_NORMAL_UNLIT, pEffect);


		pEffect = GetEffectByName("simple_mesh_normal_selected");
		MapHandleToEffect(TECH_SIMPLE_MESH_NORMAL_SELECTED, pEffect);

		pEffect = GetEffectByName("simple_mesh_normal_border");
		MapHandleToEffect(TECH_SIMPLE_MESH_NORMAL_BORDER, pEffect);

		pEffect = GetEffectByName("simple_mesh_normal_tex2");
		MapHandleToEffect(TECH_SIMPLE_MESH_NORMAL_TEX2, pEffect);


		pEffect = GetEffectByName("occlusion_test");
		MapHandleToEffect(TECH_OCCLUSION_TEST, pEffect);

#ifdef SIMPLE_OCEAN
		pEffect = GetEffectByName("ocean_fft");
#else
		pEffect = GetEffectByName("ocean_fft_high");
#endif
		MapHandleToEffect(TECH_OCEAN, pEffect);

		pEffect = GetEffectByName("ocean_simple");
		MapHandleToEffect(TECH_OCEAN_SIMPLE, pEffect);

		pEffect = GetEffectByName("ocean_cloud");
		MapHandleToEffect(TECH_OCEAN_CLOUD, pEffect);

		pEffect = GetEffectByName("sky");
		MapHandleToEffect(TECH_SKY_MESH, pEffect);

		pEffect = GetEffectByName("skydome");
		MapHandleToEffect(TECH_SKY_DOME, pEffect);

		pEffect = GetEffectByName("waterripple");
		MapHandleToEffect(TECH_WATER_RIPPLE, pEffect);

		pEffect = GetEffectByName("simple_particle");
		MapHandleToEffect(TECH_PARTICLES, pEffect);

		//////////////////////////////////////////////////////////////////////////
		// terrain related

		pEffect = GetEffectByName("shadowmap_blur");
		MapHandleToEffect(TECH_SHADOWMAP_BLUR, pEffect);

		// terrain actually uses shader model 3. 
		pEffect = GetEffectByName("terrain_normal");
		MapHandleToEffect(TECH_TERRAIN, pEffect);
		
		// terrain: using normals.
		CGlobals::GetGlobalTerrain()->EnableLighting(true);

		pEffect = GetEffectByName("screenWave");
		MapHandleToEffect(TECH_SCREEN_WAVE, pEffect);

		pEffect = GetEffectByName("block");
		MapHandleToEffect(TECH_BLOCK,pEffect);
	}
	else if(nLevel>20 && nLevel<=30)
	{
		//////////////////////////////////////////////////////////////////////////
		// 30, VS PS shader version 3
		m_HandleMap.clear();
		//CGlobals::GetOceanManager()->SetRenderTechnique(COceanManager::OCEAN_TECH_FULL);

		pEffect = GetEffectByName("simple_mesh_normal");
		MapHandleToEffect(TECH_SIMPLE_MESH_NORMAL, pEffect);
		MapHandleToEffect(TECH_CHARACTER, pEffect);
		

		pEffect = GetEffectByName("simple_mesh_normal_instanced");
		MapHandleToEffect(TECH_SIMPLE_MESH_NORMAL_INSTANCED, pEffect);
		
		pEffect = GetEffectByName("simple_mesh");
		MapHandleToEffect(TECH_SIMPLE_MESH, pEffect);
	
		pEffect = GetEffectByName("simple_mesh_normal_ctor");
		MapHandleToEffect(TECH_SIMPLE_MESH_NORMAL_CTOR, pEffect);

		pEffect = GetEffectByName("simple_mesh_normal_vegetation");
		MapHandleToEffect(TECH_SIMPLE_MESH_NORMAL_VEGETATION, pEffect);

		pEffect = GetEffectByName("simple_mesh_normal_unlit");
		MapHandleToEffect(TECH_SIMPLE_MESH_NORMAL_UNLIT, pEffect);


		pEffect = GetEffectByName("simple_mesh_normal_selected");
		MapHandleToEffect(TECH_SIMPLE_MESH_NORMAL_SELECTED, pEffect);

		pEffect = GetEffectByName("simple_mesh_normal_border");
		MapHandleToEffect(TECH_SIMPLE_MESH_NORMAL_BORDER, pEffect);
		
		pEffect = GetEffectByName("simple_mesh_normal_specialeffects");
		MapHandleToEffect(TECH_SIMPLE_MESH_NORMAL_TRANSPARENT, pEffect);


		pEffect = GetEffectByName("simple_mesh_normal_shadow");
		MapHandleToEffect(TECH_SIMPLE_MESH_NORMAL_SHADOW, pEffect);

		pEffect = GetEffectByName("simple_cad_model");
		MapHandleToEffect(TECH_SIMPLE_CAD_MODEL, pEffect);

		pEffect = GetEffectByName("dropShadow");
		MapHandleToEffect(TECH_SIMPLE_DROPSHADOW, pEffect);

		pEffect = GetEffectByName("simple_mesh_normal_tex2");
		MapHandleToEffect(TECH_SIMPLE_MESH_NORMAL_TEX2, pEffect);


		pEffect = GetEffectByName("occlusion_test");
		MapHandleToEffect(TECH_OCCLUSION_TEST, pEffect);

#ifdef SIMPLE_OCEAN
		pEffect = GetEffectByName("ocean_fft");
#else
		pEffect = GetEffectByName("ocean_fft_high");
#endif
		MapHandleToEffect(TECH_OCEAN, pEffect);

		pEffect = GetEffectByName("ocean_simple");
		MapHandleToEffect(TECH_OCEAN_SIMPLE, pEffect);

		pEffect = GetEffectByName("ocean_cloud");
		MapHandleToEffect(TECH_OCEAN_CLOUD, pEffect);

		pEffect = GetEffectByName("sky");
		MapHandleToEffect(TECH_SKY_MESH, pEffect);

		pEffect = GetEffectByName("skydome");
		MapHandleToEffect(TECH_SKY_DOME, pEffect);

		pEffect = GetEffectByName("waterripple");
		MapHandleToEffect(TECH_WATER_RIPPLE, pEffect);
		
		pEffect = GetEffectByName("simple_particle");
		MapHandleToEffect(TECH_PARTICLES, pEffect);

		pEffect = GetEffectByName("fullscreen_glow");
		MapHandleToEffect(TECH_FULL_SCREEN_GLOW, pEffect);

		pEffect = GetEffectByName("screenWave");
		MapHandleToEffect(TECH_SCREEN_WAVE, pEffect);

		pEffect = GetEffectByName("block");
		MapHandleToEffect(TECH_BLOCK,pEffect);
		pEffect = GetEffectByName("block_fancy");
		MapHandleToEffect(TECH_BLOCK_FANCY,pEffect);
		MapHandleToEffect(TECH_LIGHT_DIRECTIONAL, pEffect);
		MapHandleToEffect(TECH_LIGHT_SPOT, pEffect);
		MapHandleToEffect(TECH_LIGHT_POINT, pEffect);

		//////////////////////////////////////////////////////////////////////////
		// terrain related
		pEffect = GetEffectByName("shadowmap_blur");
		MapHandleToEffect(TECH_SHADOWMAP_BLUR, pEffect);
		
		// this actually uses shader model 3. why not design a simpler terrain shader for model 2.
		pEffect = GetEffectByName("terrain_normal");
		MapHandleToEffect(TECH_TERRAIN, pEffect);
		
		// terrain: using normals.
		CGlobals::GetGlobalTerrain()->EnableLighting(true);
	}
	else if(nLevel>30)
	{
		//////////////////////////////////////////////////////////////////////////
		// 40, VS PS shader version 4
		OUTPUT_LOG("shader model 4 not supported so far\r\n");
	}
	if(nLevel>=10)
	{
		pEffect = GetEffectByName("BMaxModel");
		MapHandleToEffect(TECH_BMAX_MODEL, pEffect);

		if (!CGlobals::GetSettings().IsLowEndRenderDevice())
		{
			pEffect = GetEffectByName("character_skinned");
#if PARA_TARGET_PLATFORM == PARA_PLATFORM_MAC
			auto* device = CGlobals::GetRenderDevice();
			if (nLevel == 20 && device && device->GetBackendType() == ERHIBackend::OpenGL &&
				device->GetCaps().maxVertexUniformVectors >= 384)
				pEffect = CGlobals::GetAssetManager()->LoadEffectFile("character_skinned_low", ":IDR_FX_CHARACTER_SKINNED_LOW");
#endif
			MapHandleToEffect(TECH_CHARACTER_SKINNED, pEffect);

			pEffect = GetEffectByName("bmax_model_skinned");
			MapHandleToEffect(TECH_BMAX_SKINNED, pEffect);
		}

		pEffect = GetEffectByName("singleColor");
		MapHandleToEffect(TECH_SINGLE_COLOR, pEffect);

		pEffect = GetEffectByName("guiEffect");
		MapHandleToEffect(TECH_GUI, pEffect);
		MapHandleToEffect(TECH_NONE, pEffect);
		MapHandleToEffect(TECH_UNKNOWN, pEffect);

		pEffect = GetEffectByName("guiTextEffect");
		MapHandleToEffect(TECH_GUI_TEXT, pEffect);

		// preload stereo shader
		GetEffectByName("RedBlueStereo");
	}
#endif
}

int EffectManager::GetDefaultEffectMapping()
{
	return m_nEffectLevel;
}

CEffectFile* EffectManager::GetEffectByName( const string& sName )
{
	CEffectFile* pEffect = NULL;

#ifndef NPLRUNTIME_SERVER
	if (sName == "simple_mesh_normal_low")
	{
		pEffect = GetByName("simple_mesh_normal_low");
		if(pEffect == 0)
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("simple_mesh_normal_low", ":IDR_FX_SIMPLE_MESH_NORMAL_LOW");
	}
	else if (sName == "simple_mesh_normal")
	{
		pEffect = GetByName("simple_mesh_normal");
		if(pEffect == 0)
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("simple_mesh_normal", ":IDR_FX_SIMPLE_MESH_NORMAL");
	}
	else if (sName == "simple_mesh_normal_vegetation")
	{
		pEffect = GetByName("simple_mesh_normal_vegetation");
		if(pEffect == 0)
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("simple_mesh_normal_vegetation", ":IDR_FX_SIMPLE_MESH_VEGETATION");
	}
	else if (sName == "simple_mesh_normal_instanced")
	{
		pEffect = GetByName("simple_mesh_normal_instanced");
		if(pEffect == 0)
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("simple_mesh_normal_instanced", ":IDR_FX_SIMPLE_MESH_NORMAL_INSTANCED");
	}
	else if (sName == "simple_mesh")
	{
		pEffect = GetByName("simple_mesh");
		if(pEffect == 0)
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("simple_mesh", ":IDR_FX_SIMPLE_MESH");
	}
	else if (sName == "simple_mesh_normal_ctor")
	{
		pEffect = GetByName("simple_mesh_normal_ctor");
		if(pEffect == 0)
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("simple_mesh_normal_ctor", ":IDR_FX_SMESH_CTOR");
	}
	else if (sName == "simple_mesh_normal_specialeffects")
	{
		pEffect = GetByName("simple_mesh_normal_specialeffects");
		if(pEffect == 0)
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("simple_mesh_normal_specialeffects", ":IDR_FX_SIMPLE_MESH_NORMAL_SPECIALEFFECTS");
	}
	else if (sName == "simple_mesh_normal_unlit")
	{
		pEffect = GetByName("simple_mesh_normal_unlit");
		if(pEffect == 0)
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("simple_mesh_normal_unlit", ":IDR_FX_SIMPLE_MESH_NORMAL_UNLIT");
	}
	else if (sName == "simple_mesh_normal_selected")
	{
		pEffect = GetByName("simple_mesh_normal_selected");
		if(pEffect == 0)
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("simple_mesh_normal_selected", ":IDR_FX_SIMPLE_MESH_NORMAL_SELECTED");
	}
	else if (sName == "simple_mesh_normal_border")
	{
		pEffect = GetByName("simple_mesh_normal_border");
		if(pEffect == 0)
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("simple_mesh_normal_border", ":IDR_FX_SIMPLE_MESH_NORMAL_BORDER");
	}
	else if (sName == "simple_mesh_normal_shadow")
	{
		pEffect = GetByName("simple_mesh_normal_shadow");
		if(pEffect == 0)
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("simple_mesh_normal_shadow", ":IDR_FX_SIMPLE_MESH_SHADOW");
	}
	else if (sName == "simple_mesh_normal_tex2")
	{
		pEffect = GetByName("simple_mesh_normal_tex2");
		if(pEffect == 0)
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("simple_mesh_normal_tex2", ":IDR_FX_SIMPLE_MESH_NORMAL_TEX2");
	}
	else if (sName == "occlusion_test")
	{
		pEffect = GetByName("occlusion_test");
		if(pEffect == 0)
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("occlusion_test", ":IDR_FX_OCCLUSION_TEST");
	}
	else if (sName == "ocean_fft")
	{
		pEffect = GetByName("ocean_fft");
		if(pEffect == 0)
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("ocean_fft", ":IDR_FX_OCEANWATER");
	}
	else if (sName == "ocean_fft_high")
	{
		pEffect = GetByName("ocean_fft_high");
		if(pEffect == 0)
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("ocean_fft_high", ":IDR_FX_OCEANWATER_HIGH");
	}
	else if (sName == "ocean_simple")
	{
		pEffect = GetByName("ocean_simple");
		if(pEffect == 0)
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("ocean_simple", ":IDR_FX_OCEANWATER_QUAD");
	}
	else if (sName == "ocean_cloud")
	{
		pEffect = GetByName("ocean_cloud");
		if(pEffect == 0)
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("ocean_cloud", ":IDR_FX_OCEANWATER_CLOUD");
	}
	else if (sName == "sky")
	{
		pEffect = GetByName("sky");
		if(pEffect == 0)
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("sky", ":IDR_FX_SKY");
	}
	else if (sName == "skydome")
	{
		pEffect = GetByName("skydome");
		if(pEffect == 0)
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("skydome", ":IDR_FX_SKYDOME");
	}
	else if (sName == "waterripple")
	{
		pEffect = GetByName("waterripple");
		if(pEffect == 0)
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("waterripple", ":IDR_FX_WATER_RIPPLE");
	}
	else if (sName == "fullscreen_glow")
	{
		pEffect = GetByName("fullscreen_glow");
		if(pEffect == 0)
		{
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("fullscreen_glow", ":IDR_FX_FULLSCREEN_GLOW");
			if (pEffect)
				m_nGlowTechnique = pEffect->GetCurrentTechniqueIndex();
		}
	}
	else if (sName == "shadowmap_blur")
	{
		pEffect = GetByName("shadowmap_blur");
		if(pEffect == 0)
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("shadowmap_blur", ":IDR_FX_SHADOWMAP_BLUR");
	}
	else if (sName == "terrain_normal")
	{
		pEffect = GetByName("terrain_normal");
		if(pEffect == 0)
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("terrain_normal", ":IDR_FX_TERRAIN_NORMAL");
	}
	else if (sName == "simple_particle")
	{
		pEffect = GetByName("simple_particle");
		if(pEffect == 0)
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("simple_particle", ":IDR_FX_SIMPLE_PARTICLE");
	}
	else if (sName == "simple_cad_model")
	{
		pEffect = GetByName("simple_cad_model");
		if(pEffect == 0)
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("simple_cad_model", ":IDR_FX_SIMPLE_CAD_MODEL");
	}
	else if (sName == "dropShadow")
	{
		pEffect = GetByName("dropShadow");
		if(pEffect == 0)
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("dropShadow", ":IDR_FX_DROPSHADOW");
	}
	else if(sName == "screenWave")
	{
		pEffect = GetByName("screenWave");
		if(pEffect == 0)
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("screenWave", ":IDR_FX_SCREENWAVE");
	}
	else if(sName == "block")
	{
		pEffect = GetByName("block");
		if(pEffect == 0)
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("block", ":IDR_FX_BLOCK");
	}
	else if (sName == "BMaxModel")
	{
		pEffect = GetByName("BMaxModel");
		if (pEffect == 0)
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("BMaxModel", ":IDR_FX_BMAXMODEL");
	}
	else if (sName == "character_skinned")
	{
		pEffect = GetByName("character_skinned");
		if (pEffect == 0)
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("character_skinned", ":IDR_FX_CHARACTER_SKINNED");
	}
	else if (sName == "bmax_model_skinned")
	{
		pEffect = GetByName("bmax_model_skinned");
		if (pEffect == 0)
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("bmax_model_skinned", ":IDR_FX_BMAX_MODEL_SKINNED");
	}
	else if (sName == "singleColor")
	{
		pEffect = GetByName("singleColor");
		if (pEffect == 0)
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("singleColor", ":IDR_FX_SINGLECOLOR");
	}
	else if (sName == "RedBlueStereo")
	{
		pEffect = GetByName("RedBlueStereo");
		if (pEffect == 0)
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("RedBlueStereo", ":IDR_FX_REDBLUESTEREO");
	}
	else if (sName == "guiEffect")
	{
		pEffect = GetByName("guiEffect");
		if (pEffect == 0)
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("guiEffect", ":IDR_FX_GUI");
	}
	else if (sName == "guiTextEffect")
	{
		pEffect = GetByName("guiTextEffect");
		if (pEffect == 0)
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("guiTextEffect", ":IDR_FX_GUI_TEXT");
	}
	else
	{
		pEffect = GetByName(sName);
	}
#endif

	return pEffect;
}


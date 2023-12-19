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
#ifdef USE_DIRECTX_RENDERER
#include "ShadowMap.h"
#include "GlowEffect.h"
#include "WaveEffect.h"
#include "RenderDeviceD3D9.h"
#include "D3DMapping.h"
#endif
#include "OceanManager.h"
#include "SunLight.h"
#include "terrain/GlobalTerrain.h"
#include "terrain/Settings.h"
#include "ParaEngineSettings.h"
#include "EffectManager.h"
#include "BlockEngine/BlockCommon.h"
#include "BlockEngine/BlockWorldClient.h"

#ifdef USE_OPENGL_RENDERER
#include "ShadowMap.h"
#include "RenderDeviceOpenGL.h"
#endif
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
	static VertexElement vertexdesc_simple_mesh_normal[]=
	{
		// base data (stream 0)
		{ 0, 0,	D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_POSITION, 0 },
		{ 0,12,	D3DDECLTYPE_FLOAT3, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_NORMAL, 0 },
		{ 0,24,	D3DDECLTYPE_FLOAT2, D3DDECLMETHOD_DEFAULT, D3DDECLUSAGE_TEXCOORD, 0 },
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
#ifdef USE_DIRECTX_RENDERER
		if(bForceGet)
		{
			auto pRenderDevice = GETD3D(CGlobals::GetRenderDevice());
			pRenderDevice->GetSamplerState(nStage, D3DMapping::toD3DSamplerSatetType(dwType), &dwValue);
		}
		else
			dwValue = m_lastSamplerStates[nStage][(int)dwType];
#endif
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
#ifdef USE_DIRECTX_RENDERER
m_pShadowMap(NULL), m_pGlowEffect(NULL), 
#elif defined(USE_OPENGL_RENDERER)
m_pShadowMap(NULL),
#endif
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
#if defined(USE_DIRECTX_RENDERER)||defined(USE_OPENGL_RENDERER)
	SAFE_DELETE(m_pShadowMap);
#endif
#ifdef USE_DIRECTX_RENDERER
	SAFE_DELETE(m_pGlowEffect);
	SAFE_DELETE(m_pWaveEffect);
#endif
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
#if defined(USE_DIRECTX_RENDERER)||defined(USE_OPENGL_RENDERER)
	if (m_pShadowMap == 0)
	{
		m_pShadowMap = new CShadowMap();
		if(m_pShadowMap!=0)
		{
			m_pShadowMap->RestoreDeviceObjects();
		}
	}
	return m_pShadowMap;
#else
	return NULL;
#endif
}

bool EffectManager::IsUsingFullScreenGlow()
{
	return m_bIsUsingFullScreenGlow;
}

void EffectManager::EnableFullScreenGlow( bool bEnable)
{
	m_bIsUsingFullScreenGlow = bEnable;
#ifdef USE_DIRECTX_RENDERER
	if (bEnable && m_pGlowEffect != 0)
	{
		m_pGlowEffect->InvalidateDeviceObjects();
		SAFE_DELETE(m_pGlowEffect);
	}
#endif
}

void EffectManager::SetGlowTechnique(int nTech)
{
	if(m_nGlowTechnique!=nTech)
	{
		m_nGlowTechnique = nTech;
#ifdef USE_DIRECTX_RENDERER
		if(m_pGlowEffect!=0)
		{
			CEffectFile* pEffect = GetEffectByHandle(TECH_FULL_SCREEN_GLOW);
			if(pEffect!=0)
			{
				pEffect->SetTechniqueByIndex(nTech);
			}
		}
#endif
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
#ifdef USE_DIRECTX_RENDERER
	if (m_pGlowEffect == 0)
	{
		m_pGlowEffect = new CGlowEffect();
		if(m_pGlowEffect!=0)
			m_pGlowEffect->RestoreDeviceObjects();
	}
	return m_pGlowEffect;
#else
	return NULL;
#endif
}

bool EffectManager::IsUsingScreenWaveEffect()
{
	return m_enableScreenWaveEffect;
}

void EffectManager::EnableScreenWaveEffect(bool enable)
{
	m_enableScreenWaveEffect = enable;
#ifdef USE_DIRECTX_RENDERER
	if(!enable && m_pWaveEffect != 0)
	{
		m_pWaveEffect->InvalidateDeviceObjects();
		SAFE_DELETE(m_pWaveEffect);
	}
#endif
}

void EffectManager::SetScreenWaveTechnique(int nTech)
{
	if(m_screenWaveTechnique !=nTech)
	{
		m_screenWaveTechnique = nTech;

#ifdef USE_DIRECTX_RENDERER
		if(m_pWaveEffect!=0)
		{
			CEffectFile* pEffect = GetEffectByHandle(TECH_SCREEN_WAVE);
			if(pEffect!=0)
			{
				pEffect->SetTechniqueByIndex(nTech);
			}
		}
#endif
	}
}

int EffectManager::GetScreenWaveTechnique()
{
	return m_screenWaveTechnique;
}

WaveEffect* EffectManager::GetScreenWaveEffect()
{
#ifdef USE_DIRECTX_RENDERER
	if(m_pWaveEffect == 0)
	{
		m_pWaveEffect = new WaveEffect();
		if(m_pWaveEffect != 0)
			m_pWaveEffect->RestoreDeviceObjects();
	}
	return m_pWaveEffect;
#else
	return NULL;
#endif
}

EffectManager::EffectTechniques EffectManager::GetCurrentEffectTechniqueType()
{
//#ifdef USE_DIRECTX_RENDERER
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
//#else
	//return EFFECT_DEFAULT;
//#endif
}

void EffectManager::RestoreDeviceObjects()
{
#if defined(USE_DIRECTX_RENDERER)||defined(USE_OPENGL_RENDERER)
	if(m_pShadowMap!=0)
		m_pShadowMap->RestoreDeviceObjects();
#endif
#ifdef USE_DIRECTX_RENDERER
	if(m_pGlowEffect!=0)
		m_pGlowEffect->RestoreDeviceObjects();
	if(m_pWaveEffect!=0)
		m_pWaveEffect->RestoreDeviceObjects();
#endif
	AssetManager <CEffectFile>::RestoreDeviceObjects();
}

void EffectManager::InvalidateDeviceObjects()
{
#if defined(USE_DIRECTX_RENDERER)||defined(USE_OPENGL_RENDERER)
	if(m_pShadowMap!=0)
		m_pShadowMap->InvalidateDeviceObjects();
#endif
#ifdef USE_DIRECTX_RENDERER
	if(m_pGlowEffect!=0)
		m_pGlowEffect->InvalidateDeviceObjects();
	if(m_pWaveEffect != 0)
		m_pWaveEffect->InvalidateDeviceObjects();
#endif
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
#ifdef USE_DIRECTX_RENDERER
		if(!bEnable)
		{
			// disable it immediately
			EnableClipPlaneImmediate(ClipPlane_Disabled);
		}
		else
		{
			CGlobals::GetRenderDevice()->SetRenderState(ERenderState::CLIPPLANEENABLE, RSV_CLIPPLANE0);
		}
#endif
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
#if defined(USE_DIRECTX_RENDERER)||defined(USE_OPENGL_RENDERER)
	if(m_pShadowMap!=0)
	{
		return m_pShadowMap->GetTexViewProjMatrix();
	}
#endif
	return NULL;
}

void EffectManager::EnableClipPlaneImmediate(EffectManager::ClipPlaneState state, int nIndex)
{
	PE_ASSERT(nIndex<MaxClipPlanesNum);
	// Fixed 2010.10.7: It looks like SetClipPlane() must be set per shader, otherwise it will not take effect. 
	//if(m_ClipPlaneState != state)
	{
		m_ClipPlaneState = state;
#ifdef USE_DIRECTX_RENDERER		
		HRESULT hr = S_OK;
		switch(state)
		{
		case ClipPlane_Enabled_ClipSpace:
			// Please note: the pipeline ensures that the clip plane is always enabled before this function is called, so there is no need to enable it here. 
			hr = CGlobals::GetRenderDevice()->SetClipPlane(nIndex, m_ClipPlaneClipSpace[nIndex]);
			break;
		case ClipPlane_Enabled_WorldSpace:
			// Please note: the pipeline ensures that the clip plane is always enabled before this function is called, so there is no need to enable it here. 
			hr = CGlobals::GetRenderDevice()->SetClipPlane(nIndex, m_ClipPlaneWorldSpace[nIndex]);
			break;
		default: // disable it
			hr = CGlobals::GetRenderDevice()->SetRenderState(ERenderState::CLIPPLANEENABLE, RSV_CLIPPLANE_NONE);
			break;
		}
		PE_ASSERT(hr == D3D_OK);
#endif
	}
}

bool EffectManager::SetD3DFogState()
{
#ifdef USE_DIRECTX_RENDERER

	auto pRenderDevice = CGlobals::GetRenderDevice();
	IScene * pScene = GetScene();
	pRenderDevice->SetRenderState( ERenderState::FOGENABLE,      pScene->IsFogEnabled());
	if(pScene->IsFogEnabled())
	{
		LinearColor color = pScene->GetFogColor();
		pRenderDevice->SetRenderState(ERenderState::FOGCOLOR,       color);
		pRenderDevice->SetRenderState(ERenderState::FOGTABLEMODE,   D3DFOG_NONE );
		pRenderDevice->SetRenderState(ERenderState::FOGVERTEXMODE,  D3DFOG_LINEAR );
		pRenderDevice->SetRenderState(ERenderState::RANGEFOGENABLE, FALSE );
		pRenderDevice->SetRenderState(ERenderState::FOGSTART,       FtoDW(pScene->GetFogStart()) );
		pRenderDevice->SetRenderState(ERenderState::FOGEND,         FtoDW(pScene->GetFogEnd()) );
	}
#endif
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
	switch(nIndex)
	{
	case S0_POS_TEX0:
		if(SUCCEEDED(pd3dDevice->CreateVertexDeclaration(vertexdesc_simple_mesh, &pDecl)))
		{
			m_pVertexDeclarations[S0_POS_TEX0] = pDecl;
		}
		else
		{
			OUTPUT_LOG("error: CreateVertexDeclaration failed for S0_POS_TEX0\n");
		}
		break;
	case S0_POS_NORM_TEX0:
		if(SUCCEEDED(pd3dDevice->CreateVertexDeclaration(vertexdesc_simple_mesh_normal, &pDecl)))
		{
			m_pVertexDeclarations[S0_POS_NORM_TEX0] = pDecl;
		}
		else
		{
			OUTPUT_LOG("error: CreateVertexDeclaration failed for S0_POS_NORM_TEX0\n");
		}
		break;
	case S0_POS_NORM_TEX0_COLOR:
		if(SUCCEEDED(pd3dDevice->CreateVertexDeclaration(vertexdesc_simple_mesh_normal_color, &pDecl)))
		{
			m_pVertexDeclarations[S0_POS_NORM_TEX0_COLOR] = pDecl;
		}
		else
		{
			OUTPUT_LOG("error: CreateVertexDeclaration failed for S0_POS_NORM_TEX0_COLOR\n");
		}
		break;
	case S0_POS_NORM_TEX0_INSTANCED:
		if(SUCCEEDED(pd3dDevice->CreateVertexDeclaration(vertexdesc_simple_mesh_normal_instanced, &pDecl)))
		{
			m_pVertexDeclarations[S0_POS_NORM_TEX0_INSTANCED] = pDecl;
		}
		else
		{
			OUTPUT_LOG("error: CreateVertexDeclaration failed for S0_POS_NORM_TEX0_INSTANCED\n");
		}
		break;
	case S0_POS_TEX0_COLOR:
		if(SUCCEEDED(pd3dDevice->CreateVertexDeclaration(vertexdesc_particle, &pDecl)))
		{
			m_pVertexDeclarations[S0_POS_TEX0_COLOR] = pDecl;
		}
		else
		{
			OUTPUT_LOG("error: CreateVertexDeclaration failed for S0_POS_TEX0_COLOR\n");
		}
		break;
	case S0_POS_COLOR:
		if (SUCCEEDED(pd3dDevice->CreateVertexDeclaration(vertexdesc_single_color, &pDecl)))
		{
			m_pVertexDeclarations[S0_POS_COLOR] = pDecl;
		}
		else
		{
			OUTPUT_LOG("error: CreateVertexDeclaration failed for S0_POS_COLOR\n");
		}
		break;
	
	case S0_POS2_COLOR:
		if (SUCCEEDED(pd3dDevice->CreateVertexDeclaration(vertex2desc_single_color, &pDecl)))
		{
			m_pVertexDeclarations[S0_POS2_COLOR] = pDecl;
		}
		else
		{
			OUTPUT_LOG("error: CreateVertexDeclaration failed for S0_POS2_COLOR\n");
		}
		break;


	case S0_POS:
		if (SUCCEEDED(pd3dDevice->CreateVertexDeclaration(vertexdesc_pos, &pDecl)))
		{
			m_pVertexDeclarations[S0_POS] = pDecl;
		}
		else
		{
			OUTPUT_LOG("error: CreateVertexDeclaration failed for S0_POS\n");
		}
		break;
	case S0_S1_OCEAN_CLOUD:
		if(SUCCEEDED(pd3dDevice->CreateVertexDeclaration(vertexdesc_ocean_cloud, &pDecl)))
		{
			m_pVertexDeclarations[S0_S1_OCEAN_CLOUD] = pDecl;
		}
		else
		{
			OUTPUT_LOG("error: CreateVertexDeclaration failed for S0_S1_OCEAN_CLOUD\n");
		}
		break;
	case S0_S1_S2_OCEAN_FFT:
		if(SUCCEEDED(pd3dDevice->CreateVertexDeclaration(vertexdesc_ocean_fft, &pDecl)))
		{
			m_pVertexDeclarations[S0_S1_S2_OCEAN_FFT] = pDecl;
		}
		else
		{
			OUTPUT_LOG("error: CreateVertexDeclaration failed for S0_S1_S2_OCEAN_FFT\n");
		}
		break;
	case S0_S1_S2_S3_OCEAN_FFT:
		if(SUCCEEDED(pd3dDevice->CreateVertexDeclaration(vertexdesc_ocean_fft_hf, &pDecl)))
		{
			m_pVertexDeclarations[S0_S1_S2_S3_OCEAN_FFT] = pDecl;
		}
		else
		{
			OUTPUT_LOG("error: CreateVertexDeclaration failed for S0_S1_S2_S3_OCEAN_FFT\n");
		}
		break;
	case S0_POS_NORM_TEX0_TEX1:
		if(SUCCEEDED(pd3dDevice->CreateVertexDeclaration(vertexdesc_terrain_normal, &pDecl)))
		{
			m_pVertexDeclarations[S0_POS_NORM_TEX0_TEX1] = pDecl;
		}
		else
		{
			OUTPUT_LOG("error: CreateVertexDeclaration failed for S0_POS_NORM_TEX0_TEX1\n");
		}
		break;
	case S0_POS_TEX0_TEX1:
		if(SUCCEEDED(pd3dDevice->CreateVertexDeclaration(vertexdesc_terrain, &pDecl)))
		{
			m_pVertexDeclarations[S0_POS_TEX0_TEX1] = pDecl;
		}
		else
		{
			OUTPUT_LOG("error: CreateVertexDeclaration failed for S0_POS_NORM_TEX0_TEX1\n");
		}
		break;	
	case S0_POS_NORM_COLOR:
		if (SUCCEEDED(pd3dDevice->CreateVertexDeclaration(vertexdesc_bmax_model, &pDecl)))
		{
			m_pVertexDeclarations[S0_POS_NORM_COLOR] = pDecl;
		}
		else
		{
			OUTPUT_LOG("error: CreateVertexDeclaration failed for S0_POS_NORM_COLOR\n");
		}
		break;
	case S0_POS_NORM_TEX0_COLOR0_COLOR1:
		if (SUCCEEDED(pd3dDevice->CreateVertexDeclaration(vertexdesc_block, &pDecl)))
		{
			m_pVertexDeclarations[S0_POS_NORM_TEX0_COLOR0_COLOR1] = pDecl;
		}
		else
		{
			OUTPUT_LOG("error: CreateVertexDeclaration failed for S0_POS_NORM_TEX0_COLOR0_COLOR1\n");
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
#ifdef USE_DIRECTX_RENDERER
	auto pd3dDevice = GETD3D(CGlobals::GetRenderDevice());
	if(pWorld)
		pd3dDevice->SetTransform( D3DTS_WORLD, GetWorldTransform().GetConstPointer());
	if(pView)
		pd3dDevice->SetTransform(D3DTS_VIEW, GetViewTransform().GetConstPointer());
	if(pProjection)
		pd3dDevice->SetTransform(D3DTS_PROJECTION, GetProjTransform().GetConstPointer());
#endif
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
	auto pd3dDevice = CGlobals::GetRenderDevice();
	if(m_pCurrentEffect != 0)
		m_pCurrentEffect->applySurfaceMaterial(pSurfaceMaterial,bUseGlobalAmbient);
	else
	{
#ifdef USE_DIRECTX_RENDERER
		if(bUseGlobalAmbient)
		{
			ParaMaterial mat = *pSurfaceMaterial;
			mat.Ambient = GetScene()->GetSunLight().GetSunAmbientHue();
			mat.Diffuse = GetScene()->GetSunLight().GetSunColor();
			// because the red component of Emissive light is used for reflectivity. we should manually set the Emissive light to 0.
			mat.Emissive = LinearColor(0,0,0,0);
			// turn off specular lighting at the moment
			mat.Specular = LinearColor(0,0,0,0);
			GETD3D(CGlobals::GetRenderDevice())->SetMaterial((D3DMATERIAL9*)&mat);
		}
		else
		{
			ParaMaterial mat = *pSurfaceMaterial;
			// because the red component of Emissive light is used for reflectivity. we should manually set the Emissive light to 0.
			mat.Emissive = LinearColor(0,0,0,0);
			// turn off specular lighting at the moment
			mat.Specular = LinearColor(0,0,0,0);
			GETD3D(CGlobals::GetRenderDevice())->SetMaterial((D3DMATERIAL9*)&mat);
		}
#endif
	}
}

void EffectManager::applyLocalLightingData(const LightList* plights, int nLightNum)
{
#ifdef USE_DIRECTX_RENDERER
	if(m_pCurrentEffect != 0)
		m_pCurrentEffect->applyLocalLightingData(plights, nLightNum);
	else
	{
		//////////////////////////////////////////////////////////////////////////
		// set d3d light for fixed function pipeline.
		auto pd3dDevice = GETD3D(CGlobals::GetRenderDevice());
		nLightNum = min(nLightNum, CEffectFile::MAX_EFFECT_LIGHT_NUM);

		if(plights!=0)
		{
			//////////////////////////////////////////////////////////////////////////
			// light 0 is reserved for global light, local lights begins with index 1
			const LightList& lights = *plights;
			for (int i=0;i<CEffectFile::MAX_EFFECT_LIGHT_NUM;++i)
			{
				bool bEnabled = i< nLightNum;
				if(bEnabled)
					pd3dDevice->SetLight(i+1, (const D3DLIGHT9*)lights[i]); 
				pd3dDevice->LightEnable(i+1, bEnabled);
			}
		}
		else if(nLightNum == 0)
		{
			for (int i=0;i<CEffectFile::MAX_EFFECT_LIGHT_NUM;++i)
			{
				pd3dDevice->LightEnable(i+1, false);
			}
		}
	}
#endif
}
void EffectManager::applyLocalLightingData()
{
#ifdef USE_DIRECTX_RENDERER
	applyLocalLightingData(CGlobals::GetLightManager()->GetLastResult(), CGlobals::GetLightManager()->GetLastResultNum());
#endif
}

void EffectManager::applyObjectLocalLighting(CBaseObject* pObj)
{
#ifdef USE_DIRECTX_RENDERER
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
#endif
}

bool EffectManager::BeginEffectFF(int nHandle)
{
#ifdef USE_DIRECTX_RENDERER
	if(nHandle < 0)
	{
		return false;
	}

	auto pRenderDevice = CGlobals::GetRenderDevice();
	bool bEnableLight = GetScene()->IsLightEnabled();
	switch(nHandle)
	{
	case TECH_OCCLUSION_TEST:
		EnableLocalLighting(false);
		pRenderDevice->SetRenderState( ERenderState::ZWRITEENABLE, FALSE );
		pRenderDevice->SetRenderState( ERenderState::ALPHABLENDENABLE, TRUE );
		pRenderDevice->SetRenderState( ERenderState::SRCBLEND,  D3DBLEND_ZERO );
		pRenderDevice->SetRenderState( ERenderState::DESTBLEND, D3DBLEND_ONE );
		GETD3D(CGlobals::GetRenderDevice())->SetTexture(0,NULL);
		GETD3D(CGlobals::GetRenderDevice())->SetFVF(OCCLUSION_VERTEX::FVF);
		GETD3D(CGlobals::GetRenderDevice())->SetTransform( D3DTS_WORLD, CGlobals::GetIdentityMatrix()->GetConstPointer());
		break;

	case TECH_SKY_DOME:
		EnableLocalLighting(false);
		GETD3D(CGlobals::GetRenderDevice())->SetFVF(mesh_vertex_normal::FVF);
		break;
	case TECH_SKY_MESH:
		EnableLocalLighting(false);
		GETD3D(CGlobals::GetRenderDevice())->SetSamplerState( 0, D3DMapping::toD3DSamplerSatetType(ESamplerStateType::ADDRESSU),  D3DTADDRESS_CLAMP );
		GETD3D(CGlobals::GetRenderDevice())->SetSamplerState( 0, D3DMapping::toD3DSamplerSatetType(ESamplerStateType::ADDRESSV),  D3DTADDRESS_CLAMP );
		GETD3D(CGlobals::GetRenderDevice())->SetFVF(mesh_vertex_normal::FVF);
		break;
	case TECH_TERRAIN:
		EnableLocalLighting(bEnableLight);
		
#ifndef ALLOW_LIGHT_IN_FIXED_FUNCTION_TERRAIN
		// force not using normal and lighting in fixed function, remove this line if one wants to enable lighting. 
		CGlobals::GetGlobalTerrain()->EnableLighting(false);
#endif

		// set states
		SetCullingMode(true);
		pRenderDevice->SetRenderState(ERenderState::ALPHABLENDENABLE, FALSE);
		pRenderDevice->SetRenderState(ERenderState::ALPHATESTENABLE, FALSE);
		pRenderDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_SRCALPHA);
		// 2009.10.13: I will use D3DBLEND_INVSRCALPHA instead of D3DBLEND_ONE. This makes fixed function looks almost identical to shader version.
		// pRenderDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_ONE);
		pRenderDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_INVSRCALPHA);

		// it should not be Less function. Because we have multi-texturing
		pRenderDevice->SetRenderState(ERenderState::ZFUNC, D3DCMP_LESSEQUAL);

		// clamping should be enabled for base texture and alpha texture, which are in texture stage 0.
		GETD3D(CGlobals::GetRenderDevice())->SetSamplerState( 0, D3DMapping::toD3DSamplerSatetType(ESamplerStateType::ADDRESSU),  D3DTADDRESS_CLAMP );
		GETD3D(CGlobals::GetRenderDevice())->SetSamplerState( 0, D3DMapping::toD3DSamplerSatetType(ESamplerStateType::ADDRESSV),  D3DTADDRESS_CLAMP );

		GETD3D(CGlobals::GetRenderDevice())->SetSamplerState( 1, D3DMapping::toD3DSamplerSatetType(ESamplerStateType::MIPFILTER), D3DTEXF_LINEAR );
		GETD3D(CGlobals::GetRenderDevice())->SetSamplerState( 1, D3DMapping::toD3DSamplerSatetType(ESamplerStateType::MINFILTER), D3DTEXF_LINEAR );
		GETD3D(CGlobals::GetRenderDevice())->SetSamplerState( 1, D3DMapping::toD3DSamplerSatetType(ESamplerStateType::MAGFILTER), D3DTEXF_LINEAR );

		// set vertex sources
		if(!CGlobals::GetGlobalTerrain()->GetSettings()->UseNormals())
		{
			GETD3D(CGlobals::GetRenderDevice())->SetFVF(terrain_vertex::FVF);
			pRenderDevice->SetRenderState( ERenderState::LIGHTING, FALSE );

		}
		else
		{
			GETD3D(CGlobals::GetRenderDevice())->SetFVF(terrain_vertex_normal::FVF);
			pRenderDevice->SetRenderState( ERenderState::LIGHTING, bEnableLight );
			ParaMaterial matTerrain = CGlobals::GetSceneState()->GetGlobalMaterial();
			// matTerrain.Ambient = LinearColor(0.6f,0.6f,0.6f,1.f );
			matTerrain.Diffuse = LinearColor(1.f,1.f,1.f,1.f);
			GETD3D(CGlobals::GetRenderDevice())->SetMaterial((D3DMATERIAL9*)&matTerrain);
		}
		break;
	case TECH_OCEAN_SIMPLE:
	case TECH_OCEAN_CLOUD:
	case TECH_OCEAN:
		pRenderDevice->SetRenderState( ERenderState::LIGHTING, FALSE );
		EnableLocalLighting(false);
		
		//CGlobals::GetOceanManager()->SetRenderTechnique(COceanManager::OCEAN_TECH_QUAD);
		// render ocean as a quad
		SetCullingMode(false);
		pRenderDevice->SetRenderState(ERenderState::ALPHABLENDENABLE, TRUE);
		pRenderDevice->SetRenderState(ERenderState::ALPHATESTENABLE, FALSE);
		pRenderDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_SRCALPHA);
		pRenderDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_INVSRCALPHA);
		GETD3D(CGlobals::GetRenderDevice())->SetSamplerState( 0, D3DMapping::toD3DSamplerSatetType(ESamplerStateType::ADDRESSU),  D3DTADDRESS_WRAP );
		GETD3D(CGlobals::GetRenderDevice())->SetSamplerState( 0, D3DMapping::toD3DSamplerSatetType(ESamplerStateType::ADDRESSV),  D3DTADDRESS_WRAP );
		pRenderDevice->SetRenderState( ERenderState::ZWRITEENABLE,          FALSE );
		pRenderDevice->SetRenderState( ERenderState::FOGENABLE,        FALSE );

		GETD3D(CGlobals::GetRenderDevice())->SetTransform(D3DTS_WORLD, CGlobals::GetIdentityMatrix()->GetConstPointer());
		GETD3D(CGlobals::GetRenderDevice())->SetFVF(SPRITEVERTEX::FVF);
		GETD3D(CGlobals::GetRenderDevice())->SetMaterial((D3DMATERIAL9*)&CGlobals::GetSceneState()->GetGlobalMaterial());

		break;
	case TECH_OCEAN_UNDERWATER:
		{
			EnableLocalLighting(false);
			pRenderDevice->SetRenderState( ERenderState::LIGHTING, FALSE);
			pRenderDevice->SetRenderState( ERenderState::FOGENABLE,      FALSE);
			GETD3D(CGlobals::GetRenderDevice())->SetTexture(0, NULL);

			pRenderDevice->SetRenderState(ERenderState::ZENABLE, FALSE); m_bZEnable = false;
			pRenderDevice->SetRenderState( ERenderState::ZWRITEENABLE,     FALSE );
			pRenderDevice->SetRenderState( ERenderState::ALPHABLENDENABLE, TRUE );
			pRenderDevice->SetRenderState(ERenderState::ALPHATESTENABLE, FALSE);
			pRenderDevice->SetRenderState( ERenderState::SRCBLEND,  D3DBLEND_SRCALPHA );
			pRenderDevice->SetRenderState( ERenderState::DESTBLEND, D3DBLEND_INVSRCALPHA );

			// Draw a big square
			GETD3D(CGlobals::GetRenderDevice())->SetFVF( UNDERWATER_VERTEX::FVF );
			break;
		}
	case TECH_CHARACTER:
		EnableLocalLighting(bEnableLight);
		pRenderDevice->SetRenderState( ERenderState::LIGHTING, bEnableLight );
		if(bEnableLight)
		{
			ParaMaterial mat = CGlobals::GetSceneState()->GetGlobalMaterial();
			// matTerrain.Ambient = LinearColor(0.6f,0.6f,0.6f,1.f );
			mat.Diffuse = LinearColor(1.f,1.f,1.f,1.f);
			GETD3D(CGlobals::GetRenderDevice())->SetMaterial((D3DMATERIAL9*)&mat);
		}
		pRenderDevice->SetRenderState( ERenderState::ZWRITEENABLE, TRUE );
		pRenderDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_SRCALPHA);
		pRenderDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_INVSRCALPHA);
		pRenderDevice->SetRenderState(ERenderState::ALPHAREF, (DWORD)0x0000000BE); // should be 0.3
		pRenderDevice->SetRenderState(ERenderState::ALPHAFUNC, D3DCMP_GREATER);
		GETD3D(CGlobals::GetRenderDevice())->SetSamplerState( 0, D3DMapping::toD3DSamplerSatetType(ESamplerStateType::ADDRESSU),  D3DTADDRESS_WRAP );
		GETD3D(CGlobals::GetRenderDevice())->SetSamplerState( 0, D3DMapping::toD3DSamplerSatetType(ESamplerStateType::ADDRESSV),  D3DTADDRESS_WRAP );
		SetCullingMode(true);

		GETD3D(CGlobals::GetRenderDevice())->SetFVF(mesh_vertex_normal::FVF);
		
		break;
	case TECH_PARTICLES:
		EnableLocalLighting(false);
		pRenderDevice->SetRenderState( ERenderState::LIGHTING, FALSE);
		SetCullingMode(false);
		pRenderDevice->SetRenderState( ERenderState::ZWRITEENABLE, FALSE );
		GETD3D(CGlobals::GetRenderDevice())->SetFVF(SPRITEVERTEX::FVF);
		break;
	case TECH_GUI:
		EnableLocalLighting(false);
		pRenderDevice->SetRenderState( ERenderState::LIGHTING, FALSE);
		pRenderDevice->SetRenderState( ERenderState::FOGENABLE,      FALSE);
		GETD3D(CGlobals::GetRenderDevice())->SetTexture(0, NULL);
		
		pRenderDevice->SetRenderState(ERenderState::ZENABLE, FALSE); m_bZEnable = false;
		pRenderDevice->SetRenderState( ERenderState::ZWRITEENABLE,     FALSE );
		pRenderDevice->SetRenderState( ERenderState::ALPHABLENDENABLE, TRUE );
		pRenderDevice->SetRenderState(ERenderState::ALPHATESTENABLE, FALSE);
		pRenderDevice->SetRenderState( ERenderState::SRCBLEND,  D3DBLEND_SRCALPHA );
		pRenderDevice->SetRenderState( ERenderState::DESTBLEND, D3DBLEND_INVSRCALPHA );

		GETD3D(CGlobals::GetRenderDevice())->SetFVF( DXUT_SCREEN_VERTEX::FVF );
		break;
	case TECH_FULL_SCREEN_GLOW:
		return false;
		break;
	case TECH_SCREEN_WAVE:
		return false;
		break;
	case TECH_BMAX_MODEL:
		pRenderDevice->SetRenderState(ERenderState::ZWRITEENABLE, TRUE);
		pRenderDevice->SetRenderState(ERenderState::LIGHTING, FALSE);
		pRenderDevice->SetRenderState(ERenderState::FOGENABLE, FALSE);
		pRenderDevice->SetRenderState(ERenderState::ALPHABLENDENABLE, FALSE);
		GETD3D(CGlobals::GetRenderDevice())->SetFVF(bmax_vertex::FVF);
		SetCullingMode(true);
		break;
	case TECH_LIGHT_POINT:
	case TECH_LIGHT_SPOT:
	case TECH_LIGHT_DIRECTIONAL:
		pRenderDevice->SetRenderState(ERenderState::ZWRITEENABLE, TRUE);
		pRenderDevice->SetRenderState(ERenderState::LIGHTING, FALSE);
		pRenderDevice->SetRenderState(ERenderState::FOGENABLE, FALSE);
	case TECH_BLOCK_FANCY:
	case TECH_BLOCK:
		EnableLocalLighting(bEnableLight);
		if(bEnableLight)
		{
			ParaMaterial mat = CGlobals::GetSceneState()->GetGlobalMaterial();
			// matTerrain.Ambient = LinearColor(0.6f,0.6f,0.6f,1.f );
			mat.Diffuse = LinearColor(1.f,1.f,1.f,1.f);
			GETD3D(CGlobals::GetRenderDevice())->SetMaterial((D3DMATERIAL9*)&mat);
		}
		pRenderDevice->SetRenderState( ERenderState::ZWRITEENABLE, TRUE );
		pRenderDevice->SetRenderState( ERenderState::LIGHTING, FALSE);
		pRenderDevice->SetRenderState( ERenderState::FOGENABLE, FALSE);

		pRenderDevice->SetRenderState( ERenderState::ALPHABLENDENABLE, FALSE);
		pRenderDevice->SetRenderState( ERenderState::SRCBLEND,  D3DBLEND_SRCALPHA );
		pRenderDevice->SetRenderState( ERenderState::DESTBLEND, D3DBLEND_INVSRCALPHA );

		GETD3D(CGlobals::GetRenderDevice())->SetSamplerState( 0, D3DMapping::toD3DSamplerSatetType(ESamplerStateType::ADDRESSU),  D3DTADDRESS_WRAP);
		GETD3D(CGlobals::GetRenderDevice())->SetSamplerState(0, D3DMapping::toD3DSamplerSatetType(ESamplerStateType::ADDRESSV), D3DTADDRESS_WRAP);
		GETD3D(CGlobals::GetRenderDevice())->SetSamplerState( 0, D3DMapping::toD3DSamplerSatetType(ESamplerStateType::MAGFILTER),  D3DTEXF_POINT );
		GETD3D(CGlobals::GetRenderDevice())->SetSamplerState( 0, D3DMapping::toD3DSamplerSatetType(ESamplerStateType::MINFILTER),  D3DTEXF_POINT );

		pRenderDevice->SetRenderState( ERenderState::ALPHAREF, 0);
		pRenderDevice->SetRenderState(ERenderState::ALPHAFUNC, D3DCMP_GREATER);

		GETD3D(CGlobals::GetRenderDevice())->SetFVF( mesh_vertex_normal_color::FVF );
		SetCullingMode(true);

		break;
	case TECH_SIMPLE_MESH_NORMAL_INSTANCED:
		return false;
		break;
	case TECH_SHADOWMAP_BLUR:
		return false;
		break;
	default: // automatically default to fixed function simple mesh for unknown or unsupported shader. 
	/*case TECH_SIMPLE_MESH:
	case TECH_SIMPLE_MESH_NORMAL:
	case TECH_SIMPLE_MESH_NORMAL_SHADOW:
	case TECH_SIMPLE_MESH_NORMAL_TEX2:
	case TECH_SIMPLE_MESH_NORMAL_UNLIT:
	case TECH_SIMPLE_MESH_NORMAL_SELECTED:
	case TECH_SIMPLE_MESH_NORMAL_TRANSPARENT:
	case TECH_SIMPLE_MESH_NORMAL_VEGETATION:
	case TECH_SIMPLE_MESH_NORMAL_CTOR:*/
		
		GETD3D(CGlobals::GetRenderDevice())->SetSamplerState( 0, D3DMapping::toD3DSamplerSatetType(ESamplerStateType::ADDRESSU),  D3DTADDRESS_WRAP );
		GETD3D(CGlobals::GetRenderDevice())->SetSamplerState( 0, D3DMapping::toD3DSamplerSatetType(ESamplerStateType::ADDRESSV),  D3DTADDRESS_WRAP );
		GETD3D(CGlobals::GetRenderDevice())->SetSamplerState( 0, D3DMapping::toD3DSamplerSatetType(ESamplerStateType::MIPFILTER), D3DTEXF_LINEAR ); // use this or not?
		pRenderDevice->SetRenderState(ERenderState::ALPHABLENDENABLE, FALSE);
		pRenderDevice->SetRenderState(ERenderState::ALPHATESTENABLE, FALSE);

		pRenderDevice->SetRenderState( ERenderState::ALPHAREF,         FIXED_FUNCTION_ALPHA_TESTING_REF );
		pRenderDevice->SetRenderState( ERenderState::ALPHAFUNC,  D3DCMP_GREATER );
		pRenderDevice->SetRenderState( ERenderState::ZWRITEENABLE, TRUE );
		pRenderDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_SRCALPHA);
		pRenderDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_INVSRCALPHA);

		if(nHandle == TECH_SIMPLE_MESH_NORMAL || nHandle == TECH_SIMPLE_MESH_NORMAL_CTOR || nHandle== TECH_SIMPLE_MESH_NORMAL_VEGETATION)
		{
			// use lighting 
			pRenderDevice->SetRenderState( ERenderState::LIGHTING, bEnableLight );
			EnableLocalLighting(bEnableLight);
		}

		if (nHandle == TECH_NONE)
			pRenderDevice->SetRenderState(ERenderState::LIGHTING, false);

		GETD3D(CGlobals::GetRenderDevice())->SetFVF(mesh_vertex_normal::FVF);

		break;
	}
	//pRenderDevice->SetSamplerState( 0, ESamplerStateType::MIPFILTER, D3DTEXF_NONE );
	GETD3D(CGlobals::GetRenderDevice())->SetVertexShader( NULL );
	GETD3D(CGlobals::GetRenderDevice())->SetPixelShader( NULL );
	if(IsClipPlaneEnabled())
		EnableClipPlaneImmediate(ClipPlane_Enabled_WorldSpace);
#endif
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
#ifdef USE_DIRECTX_RENDERER
	if(nHandle < 0)
	{
		return false;
	}

	CEffectFile* pEffect = GetEffectByHandle(nHandle);
	if( pEffect != 0  )
	{
		pEffect->LoadAsset();
		if(!pEffect->IsValid())
		{
			//////////////////////////////////////////////////////////////////////////
			//
			// handle effect fall back policy. Use a lower effect or fall back to fixed function if there is none
			//
			//////////////////////////////////////////////////////////////////////////

			const AssetKey& sEffectFileName = pEffect->GetKey();

			if(sEffectFileName==":IDR_FX_TERRAIN_NORMAL")
			{
				//////////////////////////////////////////////////////////////////////////
				// fall back to fixed function
				pEffect = 0;
				// disable shadow if the terrain shader is not valid.
				GetScene()->SetShadow(false);
			}
			else if(sEffectFileName==":IDR_FX_SIMPLE_MESH_NORMAL" || sEffectFileName==":IDR_FX_SIMPLE_MESH_SHADOW" || sEffectFileName == ":IDR_FX_SIMPLE_MESH_NORMAL_SPECIALEFFECTS")
			{
				//////////////////////////////////////////////////////////////////////////
				// for shader model 3 mesh with normal shader, fall back to simple_mesh_normal_low which is shader model 2.0.
				pEffect = GetEffectByName("simple_mesh_normal_low");
				if(pEffect)
				{
					pEffect->LoadAsset();
					if(!pEffect->IsValid())
					{
						//////////////////////////////////////////////////////////////////////////
						// fall back to fixed function
						pEffect = 0;
					}
				}
			}
			else
			{
				//////////////////////////////////////////////////////////////////////////
				// fall back to fixed function
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
		// use fixed function, if the effect is invalid. 

		if(m_nCurrentEffect!=nHandle)
			EndEffect();
		m_nCurrentEffect = nHandle;
		m_pCurrentEffect = pEffect;
		if(pEffect == 0)
		{
			//////////////////////////////////////////////////////////////////////////
			// if the effect file is not supported, we will try emulate using fixed programming pipeline
			return BeginEffectFF(nHandle);
		}
		return true;
	}

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

	auto pd3dDevice = CGlobals::GetRenderDevice();

	bool bEnableLight = GetScene()->IsLightEnabled();
	bool bEnableSunLight = GetScene()->IsSunLightEnabled();

	//////////////////////////////////////////////////////////////////////////
	//
	// mostly for programmable pipeline
	//
	//////////////////////////////////////////////////////////////////////////
	VertexDeclarationPtr pDecl=NULL;
	switch(nHandle)
	{
	case TECH_NONE:
	{
		return BeginEffectFF(nHandle);
		break;
	}
	case TECH_OCCLUSION_TEST:
	{
		pDecl = GetVertexDeclaration(S0_POS_TEX0);
		if(pDecl == 0)
			return false;
		GETD3D(CGlobals::GetRenderDevice())->SetVertexDeclaration(pDecl);

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
		GETD3D(CGlobals::GetRenderDevice())->SetVertexDeclaration(pDecl);

		DisableD3DAlphaTesting(true);
		pEffect->EnableAlphaBlending(false);
		pEffect->EnableAlphaTesting(false);
		pEffect->applySurfaceMaterial(&CGlobals::GetSceneState()->GetGlobalMaterial());

		applyFogParameters();

		pd3dDevice->SetRenderState(ERenderState::ALPHABLENDENABLE, TRUE);
		pd3dDevice->SetRenderState(ERenderState::ALPHATESTENABLE, FALSE);
		pd3dDevice->SetRenderState( ERenderState::ZWRITEENABLE, TRUE );
		pd3dDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_SRCALPHA);
		pd3dDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_INVSRCALPHA);
		SetCullingMode(true);

		GETD3D(CGlobals::GetRenderDevice())->SetSamplerState( 0, D3DMapping::toD3DSamplerSatetType(ESamplerStateType::MIPFILTER), D3DTEXF_LINEAR );
		SetSamplerState( 0, ESamplerStateType::MINFILTER, D3DTEXF_LINEAR , true);
		SetSamplerState( 0, ESamplerStateType::MAGFILTER, D3DTEXF_LINEAR ,true);
		GETD3D(CGlobals::GetRenderDevice())->SetSamplerState( 0, D3DMapping::toD3DSamplerSatetType(ESamplerStateType::ADDRESSU),  D3DTADDRESS_WRAP );
		GETD3D(CGlobals::GetRenderDevice())->SetSamplerState( 0, D3DMapping::toD3DSamplerSatetType(ESamplerStateType::ADDRESSV),  D3DTADDRESS_WRAP );
		break;
	}
	case TECH_SKY_DOME:
	{
		pDecl = GetVertexDeclaration(S0_POS_TEX0);
		if(pDecl == 0)
			return false;
		GETD3D(CGlobals::GetRenderDevice())->SetVertexDeclaration(pDecl);

		DisableD3DAlphaTesting(true);
		pEffect->EnableAlphaBlending(false);
		pEffect->EnableAlphaTesting(false);
		pEffect->applySurfaceMaterial(&CGlobals::GetSceneState()->GetGlobalMaterial());
		applyFogParameters();

		pd3dDevice->SetRenderState(ERenderState::ALPHABLENDENABLE, TRUE);
		pd3dDevice->SetRenderState(ERenderState::ALPHATESTENABLE, FALSE);
		pd3dDevice->SetRenderState( ERenderState::ZWRITEENABLE, TRUE );
		pd3dDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_SRCALPHA);
		pd3dDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_INVSRCALPHA);
		SetCullingMode(false);
		DisableD3DCulling(true);

		GETD3D(CGlobals::GetRenderDevice())->SetSamplerState( 0, D3DMapping::toD3DSamplerSatetType(ESamplerStateType::MIPFILTER), D3DTEXF_LINEAR );
		SetSamplerState( 0, ESamplerStateType::MINFILTER, D3DTEXF_LINEAR , true);
		SetSamplerState( 0, ESamplerStateType::MAGFILTER, D3DTEXF_LINEAR ,true);
		GETD3D(CGlobals::GetRenderDevice())->SetSamplerState( 0, D3DMapping::toD3DSamplerSatetType(ESamplerStateType::ADDRESSU),  D3DTADDRESS_CLAMP );
		GETD3D(CGlobals::GetRenderDevice())->SetSamplerState( 0, D3DMapping::toD3DSamplerSatetType(ESamplerStateType::ADDRESSV),  D3DTADDRESS_CLAMP );
		break;
	}
	case TECH_SIMPLE_MESH_NORMAL:
	case TECH_SIMPLE_MESH_NORMAL_UNLIT:
	case TECH_CHARACTER:
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
		else
			pDecl = GetVertexDeclaration(S0_POS_NORM_TEX0);

		if(pDecl == 0)
			return false;

		GETD3D(CGlobals::GetRenderDevice())->SetVertexDeclaration(pDecl);

		DisableD3DAlphaTesting(true);

		pEffect->EnableAlphaBlending(false);
		pEffect->EnableAlphaTesting(false);

		pd3dDevice->SetRenderState(ERenderState::ALPHATESTENABLE, FALSE);
		pd3dDevice->SetRenderState(ERenderState::ALPHABLENDENABLE, TRUE); // force blending
		pd3dDevice->SetRenderState( ERenderState::ZWRITEENABLE, TRUE );
		pd3dDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_SRCALPHA);
		pd3dDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_INVSRCALPHA);
		SetCullingMode(true);

		GETD3D(CGlobals::GetRenderDevice())->SetSamplerState( 0, D3DMapping::toD3DSamplerSatetType(ESamplerStateType::MIPFILTER), D3DTEXF_LINEAR );
		SetSamplerState( 0, ESamplerStateType::MINFILTER, D3DTEXF_LINEAR , true);
		SetSamplerState( 0, ESamplerStateType::MAGFILTER, D3DTEXF_LINEAR ,true);
		if(nHandle == TECH_SIMPLE_MESH_NORMAL || nHandle == TECH_CHARACTER|| nHandle == TECH_SIMPLE_MESH_NORMAL_VEGETATION ||nHandle == TECH_SIMPLE_MESH_NORMAL_SHADOW || nHandle == TECH_SIMPLE_MESH_NORMAL_INSTANCED || nHandle == TECH_SIMPLE_MESH_NORMAL_UNLIT)
		{
			pEffect->EnableNormalMap(false);
			pEffect->EnableReflectionMapping(false);
			pEffect->EnableEnvironmentMapping(false);

			EnableGlobalLighting(bEnableSunLight && bEnableLight);
			EnableLocalLighting(bEnableLight);

			GETD3D(CGlobals::GetRenderDevice())->SetSamplerState(0, D3DMapping::toD3DSamplerSatetType(ESamplerStateType::ADDRESSU), D3DTADDRESS_WRAP);
			GETD3D(CGlobals::GetRenderDevice())->SetSamplerState( 0, D3DMapping::toD3DSamplerSatetType(ESamplerStateType::ADDRESSV),  D3DTADDRESS_WRAP );

			applyFogParameters();

			if(nHandle == TECH_SIMPLE_MESH_NORMAL_SHADOW)
			{
				if(GetScene()->IsShadowMapEnabled())
				{
					CShadowMap* pShadowMap = GetShadowMap();
					if (pShadowMap !=NULL)
					{
						EnableUsingShadowMap(true);
						/* the following is set per mesh. 
						if(pShadowMap->UsingBlurredShadowMap())
						{
						m_pCurrentEffect->EnableShadowmap(pShadowMap->SupportsHWShadowMaps()?1:2);
						pShadowMap->SetShadowTexture(*m_pCurrentEffect, 2, 1);
						}
						else
						{
						m_pCurrentEffect->EnableShadowmap(pShadowMap->SupportsHWShadowMaps()?1:2);
						pShadowMap->SetShadowTexture(*m_pCurrentEffect, 2);
						}*/
					}
					pEffect->SetShadowRadius(GetScene()->GetShadowRadius());
				}
			}

		}
		else if(nHandle ==TECH_SIMPLE_MESH_NORMAL_TRANSPARENT)
		{
			EnableGlobalLighting(bEnableSunLight && bEnableLight);
			EnableLocalLighting(false);
			pEffect->applyLocalLightingData(NULL, 0);

			// force alpha testing
			SetCullingMode(false);
			DisableD3DCulling(true);

			GETD3D(CGlobals::GetRenderDevice())->SetSamplerState( 0, D3DMapping::toD3DSamplerSatetType(ESamplerStateType::ADDRESSU),  D3DTADDRESS_WRAP );
			GETD3D(CGlobals::GetRenderDevice())->SetSamplerState( 0, D3DMapping::toD3DSamplerSatetType(ESamplerStateType::ADDRESSV),  D3DTADDRESS_WRAP );
		}
		else if(nHandle == TECH_SIMPLE_MESH_NORMAL_CTOR)
		{
			EnableGlobalLighting(bEnableSunLight && bEnableLight);
			EnableLocalLighting(false);
			pEffect->applyLocalLightingData(NULL, 0);

			// force alpha testing
			SetCullingMode(false);
			DisableD3DCulling(true);

			GETD3D(CGlobals::GetRenderDevice())->SetSamplerState( 0, D3DMapping::toD3DSamplerSatetType(ESamplerStateType::ADDRESSU),  D3DTADDRESS_WRAP );
			GETD3D(CGlobals::GetRenderDevice())->SetSamplerState( 0, D3DMapping::toD3DSamplerSatetType(ESamplerStateType::ADDRESSV),  D3DTADDRESS_WRAP );

		}
		else if(nHandle == TECH_SIMPLE_MESH_NORMAL_SELECTED)
		{
			EnableGlobalLighting(bEnableSunLight && bEnableLight);
			EnableLocalLighting(bEnableLight);

			float fIntensity = 1.25f+g_flash;
			Vector4 fogParam(0, 1.f, 1.0f, 0.1f);
			LinearColor fogColor = CGlobals::GetSettings().GetSelectionColor();
			fogColor.r *= fIntensity;
			fogColor.g *= fIntensity;
			fogColor.b *= fIntensity;
			fogColor.a = 1.f;
			pEffect->applyFogParameters(true, &fogParam, &fogColor);

			GETD3D(CGlobals::GetRenderDevice())->SetSamplerState( 0, D3DMapping::toD3DSamplerSatetType(ESamplerStateType::ADDRESSU),  D3DTADDRESS_WRAP );
			GETD3D(CGlobals::GetRenderDevice())->SetSamplerState( 0, D3DMapping::toD3DSamplerSatetType(ESamplerStateType::ADDRESSV),  D3DTADDRESS_WRAP );
		}
		else if(nHandle == TECH_SIMPLE_MESH_NORMAL_TEX2)
		{
			EnableGlobalLighting(bEnableSunLight && bEnableLight);
			EnableLocalLighting(bEnableLight);
			applyFogParameters();

			GETD3D(CGlobals::GetRenderDevice())->SetSamplerState( 0, D3DMapping::toD3DSamplerSatetType(ESamplerStateType::ADDRESSU),  D3DTADDRESS_WRAP );
			GETD3D(CGlobals::GetRenderDevice())->SetSamplerState( 0, D3DMapping::toD3DSamplerSatetType(ESamplerStateType::ADDRESSV),  D3DTADDRESS_WRAP );
		}
		else if(nHandle == TECH_SKY_MESH)
		{
			GETD3D(CGlobals::GetRenderDevice())->SetSamplerState( 0, D3DMapping::toD3DSamplerSatetType(ESamplerStateType::ADDRESSU),  D3DTADDRESS_CLAMP );
			GETD3D(CGlobals::GetRenderDevice())->SetSamplerState( 0, D3DMapping::toD3DSamplerSatetType(ESamplerStateType::ADDRESSV),  D3DTADDRESS_CLAMP );
			//applyFogParameters();	
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
		GETD3D(CGlobals::GetRenderDevice())->SetVertexDeclaration(pDecl);

		DisableD3DAlphaTesting(true);

		pEffect->EnableAlphaBlending(false);
		pEffect->EnableAlphaTesting(false);

		pd3dDevice->SetRenderState(ERenderState::ALPHATESTENABLE, FALSE);
		pd3dDevice->SetRenderState(ERenderState::ALPHABLENDENABLE, TRUE); // force blending
		pd3dDevice->SetRenderState( ERenderState::ZWRITEENABLE, TRUE );
		pd3dDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_SRCALPHA);
		pd3dDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_INVSRCALPHA);
		SetCullingMode(false);

		GETD3D(CGlobals::GetRenderDevice())->SetSamplerState( 0, D3DMapping::toD3DSamplerSatetType(ESamplerStateType::MIPFILTER), D3DTEXF_LINEAR );
		SetSamplerState( 0, ESamplerStateType::MINFILTER, D3DTEXF_LINEAR , true);
		SetSamplerState( 0, ESamplerStateType::MAGFILTER, D3DTEXF_LINEAR ,true);

		break;
	}
	case TECH_SIMPLE_DROPSHADOW:
	{
		pDecl = GetVertexDeclaration(S0_POS_TEX0);
		if(pDecl == 0)
			return false;
		GETD3D(CGlobals::GetRenderDevice())->SetVertexDeclaration(pDecl);
		DisableD3DAlphaTesting(true);
		
		pEffect->EnableAlphaBlending(true);
		pd3dDevice->SetRenderState(ERenderState::ALPHABLENDENABLE, TRUE); // force blending
		break;
	}
	case TECH_OCEAN_SIMPLE:
	{
		pDecl = GetVertexDeclaration(S0_POS_TEX0);
		if(pDecl == 0)
			return false;
		GETD3D(CGlobals::GetRenderDevice())->SetVertexDeclaration(pDecl);

		applyFogParameters();
		break;
	}
	case TECH_OCEAN_CLOUD:
	{
		pDecl = GetVertexDeclaration(S0_S1_OCEAN_CLOUD);

		if(pDecl == 0)
			return false;
		applyFogParameters();
		GETD3D(CGlobals::GetRenderDevice())->SetVertexDeclaration(pDecl);
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
		applyFogParameters();
		GETD3D(CGlobals::GetRenderDevice())->SetVertexDeclaration(pDecl);
		break;
	}
	case TECH_TERRAIN:
	{
		EnableGlobalLighting(bEnableSunLight && bEnableLight);
		EnableLocalLighting(bEnableLight);
		if(GetScene()->IsShadowMapEnabled())
		{
			CShadowMap* pShadowMap = GetShadowMap();
			if (pShadowMap !=NULL)
			{
				EnableUsingShadowMap(true);
			}
			pEffect->SetShadowRadius(GetScene()->GetShadowRadius());
		}

		// Just for test, 
		applyLocalLightingData();
		applyFogParameters();

		ParaMaterial matTerrain = CGlobals::GetSceneState()->GetGlobalMaterial();

		BlockWorldClient* pBlockWorldClient = BlockWorldClient::GetInstance();
		if(pBlockWorldClient && pBlockWorldClient->IsInBlockWorld())
		{
			float fLightness = max( 1.f- fabs(CGlobals::GetScene()->GetSunLight().GetTimeOfDaySTD()), 0.25f);
			matTerrain.Ambient = LinearColor(fLightness*0.7f, fLightness*0.7f, fLightness*0.7f, 1.f);
			matTerrain.Diffuse = LinearColor(fLightness*0.4f, fLightness*0.4f, fLightness*0.4f, 1.f);
			// pEffect->setParameter(CEffectFile::k_ambientLight, &(matTerrain.Ambient));
		}
		else
		{
			matTerrain.Diffuse = LinearColor(1.f,1.f,1.f,1.f);
		}
		pEffect->applySurfaceMaterial(&matTerrain);

		SetCullingMode(true);
		pd3dDevice->SetRenderState(ERenderState::ALPHABLENDENABLE, FALSE); // this may be true in future
		pd3dDevice->SetRenderState(ERenderState::ALPHATESTENABLE, FALSE);
		pd3dDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_ONE);
		pd3dDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_ONE);
		pd3dDevice->SetRenderState(ERenderState::ZWRITEENABLE, TRUE);
		// the separate alpha channel setting is for full screen glow effect
		//if(IsUsingFullScreenGlow())
		//{
		//	//bool bSupportSeperateAlpha = (CGlobals::GetDirectXEngine().m_d3dCaps.PrimitiveMiscCaps&D3DPMISCCAPS_SEPARATEALPHABLEND)>0;
		//	pd3dDevice->SetRenderState(ERenderState::ALPHABLENDENABLE, TRUE);
		//	pd3dDevice->SetRenderState(ERenderState::SEPARATEALPHABLENDENABLE, true);
		//	pd3dDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_ZERO);
		//	pd3dDevice->SetRenderState(ERenderState::SRCBLENDALPHA, D3DBLEND_ZERO);
		//	pd3dDevice->SetRenderState(ERenderState::DESTBLENDALPHA, D3DBLEND_ZERO);
		//}

		// it should not be Less function. Because we have multi-texturing
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
		GETD3D(CGlobals::GetRenderDevice())->SetVertexDeclaration(pDecl);
		break;
	}
	case TECH_FULL_SCREEN_GLOW:
	{
		pDecl = GetVertexDeclaration(S0_POS_TEX0);
		if(pDecl == 0)
			return false;
		GETD3D(CGlobals::GetRenderDevice())->SetVertexDeclaration(pDecl);
		break;
	}
	case TECH_SCREEN_WAVE:
	{
		pDecl = GetVertexDeclaration(S0_POS_TEX0);
		if(pDecl == 0)
			return false;
		GETD3D(CGlobals::GetRenderDevice())->SetVertexDeclaration(pDecl);
		break;
	}
	case TECH_SHADOWMAP_BLUR:
	{
		pDecl = GetVertexDeclaration(S0_POS_TEX0);
		if(pDecl == 0)
			return false;
		GETD3D(CGlobals::GetRenderDevice())->SetVertexDeclaration(pDecl);
		break;
	}
	case TECH_PARTICLES:
	case TECH_WATER_RIPPLE:
	{
		pDecl = GetVertexDeclaration(S0_POS_TEX0_COLOR);
		if(pDecl == 0)
			return false;
		GETD3D(CGlobals::GetRenderDevice())->SetVertexDeclaration(pDecl);

		pd3dDevice->SetRenderState(ERenderState::ALPHATESTENABLE, FALSE);
		pd3dDevice->SetRenderState(ERenderState::ALPHABLENDENABLE, TRUE); // force blending
		pd3dDevice->SetRenderState( ERenderState::ZWRITEENABLE, FALSE);
		pd3dDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_SRCALPHA);
		pd3dDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_INVSRCALPHA);
		SetCullingMode(false);

		GETD3D(CGlobals::GetRenderDevice())->SetSamplerState( 0, D3DMapping::toD3DSamplerSatetType(ESamplerStateType::MIPFILTER), D3DTEXF_LINEAR );
		SetSamplerState( 0, ESamplerStateType::MINFILTER, D3DTEXF_LINEAR , true);
		SetSamplerState( 0, ESamplerStateType::MAGFILTER, D3DTEXF_LINEAR ,true);
		break;
	}
	case TECH_BMAX_MODEL:
	{
		pDecl = GetVertexDeclaration(S0_POS_NORM_COLOR);
		if (pDecl == 0)
			return false;
		GETD3D(CGlobals::GetRenderDevice())->SetVertexDeclaration(pDecl);

		pEffect->EnableAlphaBlending(false);
		pEffect->EnableAlphaTesting(false);
		applyFogParameters();

		pd3dDevice->SetRenderState(ERenderState::ALPHATESTENABLE, FALSE);
		pd3dDevice->SetRenderState(ERenderState::ZWRITEENABLE, TRUE);
		pd3dDevice->SetRenderState(ERenderState::ALPHABLENDENABLE, FALSE);
		SetCullingMode(true);
		break;
	}
	case TECH_BLOCK_FANCY:
	case TECH_BLOCK:
	{
		pEffect->EnableAlphaBlending(false);
		pEffect->EnableAlphaTesting(false);

		applyFogParameters();

		// pd3dDevice->SetRenderState(ERenderState::FOGENABLE,0);
		pd3dDevice->SetRenderState(ERenderState::ALPHATESTENABLE, FALSE);
		pd3dDevice->SetRenderState(ERenderState::ALPHABLENDENABLE, TRUE); // force blending
		pd3dDevice->SetRenderState( ERenderState::ZWRITEENABLE, TRUE );
		pd3dDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_SRCALPHA);
		pd3dDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_INVSRCALPHA);
		GETD3D(CGlobals::GetRenderDevice())->SetSamplerState(0, D3DMapping::toD3DSamplerSatetType(ESamplerStateType::ADDRESSU), D3DTADDRESS_WRAP);
		GETD3D(CGlobals::GetRenderDevice())->SetSamplerState(0, D3DMapping::toD3DSamplerSatetType(ESamplerStateType::ADDRESSV), D3DTADDRESS_WRAP);
		SetCullingMode(true);

		break;
	}

	default:
	{
		// handle user defined effect file
		/**
		* 100-200: using declaration: POSITION|Tex1
		* 1000+: using declaration: POSITION|NORMAL|Tex1
		* etc.
		*/
		if(nHandle>1000)
		{
			pDecl = GetVertexDeclaration(S0_POS_NORM_TEX0);

			if(pDecl == 0)
				return false;

			GETD3D(CGlobals::GetRenderDevice())->SetVertexDeclaration(pDecl);
			DisableD3DAlphaTesting(true);
			pEffect->EnableAlphaBlending(false);
			pEffect->EnableAlphaTesting(false);
			pd3dDevice->SetRenderState(ERenderState::ALPHATESTENABLE, FALSE);
			pd3dDevice->SetRenderState(ERenderState::ALPHABLENDENABLE, TRUE); // force blending
			pd3dDevice->SetRenderState( ERenderState::ZWRITEENABLE, TRUE );
			pd3dDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_SRCALPHA);
			pd3dDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_INVSRCALPHA);
			SetCullingMode(true);
			GETD3D(CGlobals::GetRenderDevice())->SetSamplerState( 0, D3DMapping::toD3DSamplerSatetType(ESamplerStateType::MIPFILTER), D3DTEXF_LINEAR );
			SetSamplerState( 0, ESamplerStateType::MINFILTER, D3DTEXF_LINEAR , true);
			SetSamplerState( 0, ESamplerStateType::MAGFILTER, D3DTEXF_LINEAR ,true);

			pEffect->EnableNormalMap(false);
			pEffect->EnableReflectionMapping(false);
			pEffect->EnableEnvironmentMapping(false);

			EnableGlobalLighting(bEnableSunLight && bEnableLight);
			EnableLocalLighting(bEnableLight);

			GETD3D(CGlobals::GetRenderDevice())->SetSamplerState( 0, D3DMapping::toD3DSamplerSatetType(ESamplerStateType::ADDRESSU),  D3DTADDRESS_WRAP );
			GETD3D(CGlobals::GetRenderDevice())->SetSamplerState( 0, D3DMapping::toD3DSamplerSatetType(ESamplerStateType::ADDRESSV),  D3DTADDRESS_WRAP );

			applyFogParameters();

			if(pEffect->GetParamBlock())
			{
				// apply per effect parameters. 
				pEffect->GetParamBlock()->ApplyToEffect(pEffect);
			}
		}
		else
		{

		}
	}
		break;
	}
	// this is shared by all programmable pipeline
	if(IsClipPlaneEnabled())
		EnableClipPlaneImmediate(ClipPlane_Enabled_ClipSpace);
#elif defined(USE_OPENGL_RENDERER)
	if (nHandle < 0)
	{
		return false;
	}

	CEffectFile* pEffect = GetEffectByHandle(nHandle);
	if (pEffect != 0)
	{
		pEffect->LoadAsset();
		if (!pEffect->IsValid())
		{
			return false;
		}
	}
	RenderDevicePtr pd3dDevice = CGlobals::GetRenderDevice();

	if (pOutEffect != NULL)
		*pOutEffect = pEffect;
	if ((pEffect == 0) ||
		((m_pCurrentEffect == pEffect) && (m_nCurrentEffect == nHandle)))
	{
		// use fixed function, if the effect is invalid. 
		if (m_nCurrentEffect != nHandle)
			EndEffect();
		m_nCurrentEffect = nHandle;
		m_pCurrentEffect = pEffect;
		if (pEffect == 0)
		{
			//////////////////////////////////////////////////////////////////////////
			// if the effect file is not supported, we will try emulate using fixed programming pipeline
			return BeginEffectFF(nHandle);
		}
		return true;
	}
	bool bEnableLight = GetScene()->IsLightEnabled();
	bool bEnableSunLight = GetScene()->IsSunLightEnabled();

	int nLastEffectHandle = m_nCurrentEffect;
	if (m_nCurrentEffect != nHandle)
	{
		EndEffect();
		m_nCurrentEffect = nHandle;
	}

	if (m_pCurrentEffect != pEffect)
	{
		m_pCurrentEffect = pEffect;
	}

	switch (nHandle)
	{
	case TECH_UNKNOWN:
	{
#ifndef EMSCRIPTEN
		CGlobals::GetRenderDevice()->SetVertexDeclaration(0);
#endif
		break;
	}
	case TECH_NONE:
	{
#ifndef EMSCRIPTEN
		CGlobals::GetRenderDevice()->SetIndices(0);
		CGlobals::GetRenderDevice()->SetStreamSource(0, 0, 0, 0);
#endif
		pd3dDevice->SetRenderState(ERenderState::CULLMODE, RSV_CULL_NONE);
		pd3dDevice->SetRenderState(ERenderState::ZENABLE, FALSE);m_bZEnable = false;
		// Note by Xizhi: always enable zwrite otherwise z-clear will not working when cocos clear the zbuffer in the outer loop. 
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
			VertexDeclarationPtr pDecl = GetVertexDeclaration(S0_POS_TEX0_COLOR);
			if (pDecl == 0)
				return false;
			CGlobals::GetRenderDevice()->SetIndices(0);
			CGlobals::GetRenderDevice()->SetStreamSource(0, 0, 0, 0);
			CGlobals::GetRenderDevice()->SetVertexDeclaration(pDecl);
			pd3dDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_SRCALPHA);
			pd3dDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_INVSRCALPHA);
			SetSamplerState(0, ESamplerStateType::MINFILTER, D3DTEXF_POINT, true);
			SetSamplerState(0, ESamplerStateType::MAGFILTER, D3DTEXF_POINT, true);
			// @note: do not set ERenderState::ZENABLE, ERenderState::ZWRITEENABLE
			// it will inherit last technique to enable text rendering in 3d object space. 
		}
		break;
	}
	case TECH_GUI:
	{
		if (nLastEffectHandle != TECH_GUI && nLastEffectHandle != TECH_GUI_TEXT)
		{
			VertexDeclarationPtr pDecl = GetVertexDeclaration(S0_POS_TEX0_COLOR);
			if (pDecl == 0)
				return false;
			CGlobals::GetRenderDevice()->SetIndices(0);
			CGlobals::GetRenderDevice()->SetStreamSource(0, 0, 0, 0);
			CGlobals::GetRenderDevice()->SetVertexDeclaration(pDecl);
			pd3dDevice->SetRenderState(ERenderState::CULLMODE, RSV_CULL_NONE);
			pd3dDevice->SetRenderState(ERenderState::ZENABLE, FALSE);m_bZEnable = false;
			// Note by Xizhi: always enable zwrite otherwise z-clear will not working when cocos clear the zbuffer in the outer loop. 
			pd3dDevice->SetRenderState(ERenderState::ZWRITEENABLE, TRUE);
			pd3dDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_SRCALPHA);
			pd3dDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_INVSRCALPHA);
			pd3dDevice->SetRenderState(ERenderState::ALPHABLENDENABLE, TRUE);
			SetSamplerState(0, ESamplerStateType::MINFILTER, D3DTEXF_POINT, true);
			SetSamplerState(0, ESamplerStateType::MAGFILTER, D3DTEXF_POINT, true);
		}
		break;
	}
	case TECH_SINGLE_COLOR:
	{
		VertexDeclarationPtr pDecl = GetVertexDeclaration(S0_POS_COLOR);
		if (pDecl == 0)
			return false;
		CGlobals::GetRenderDevice()->SetIndices(0);
		CGlobals::GetRenderDevice()->SetStreamSource(0,0,0,0);
		CGlobals::GetRenderDevice()->SetVertexDeclaration(pDecl);
		pd3dDevice->SetRenderState(ERenderState::CULLMODE, RSV_CULL_CCW);
		pd3dDevice->SetRenderState(ERenderState::ZENABLE, TRUE);m_bZEnable = true;
		pd3dDevice->SetRenderState(ERenderState::ZWRITEENABLE, TRUE);
		pd3dDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_SRCALPHA);
		pd3dDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_INVSRCALPHA);
		SetSamplerState(0, ESamplerStateType::MINFILTER, D3DTEXF_POINT, true);
		SetSamplerState(0, ESamplerStateType::MAGFILTER, D3DTEXF_POINT, true);
		break;
	}
	case TECH_BMAX_MODEL:
	{
		VertexDeclarationPtr pDecl = GetVertexDeclaration(S0_POS_NORM_COLOR);
		if (pDecl == 0)
			return false;

		CGlobals::GetRenderDevice()->SetIndices(0);
		CGlobals::GetRenderDevice()->SetStreamSource(0, 0, 0, 0);
		CGlobals::GetRenderDevice()->SetVertexDeclaration(pDecl);
		pEffect->EnableAlphaBlending(false);
		pEffect->EnableAlphaTesting(false);
		applyFogParameters();
		pd3dDevice->SetRenderState(ERenderState::ALPHATESTENABLE, FALSE);
		pd3dDevice->SetRenderState(ERenderState::ZWRITEENABLE, TRUE);
		pd3dDevice->SetRenderState(ERenderState::ALPHABLENDENABLE, FALSE);
		SetCullingMode(true);
		break;
	}
	case TECH_BLOCK:
	{
		applyFogParameters();
		pEffect->EnableAlphaBlending(false);
		pEffect->EnableAlphaTesting(false);
		pd3dDevice->SetSamplerState( 0, ESamplerStateType::ADDRESSU,  D3DTADDRESS_WRAP );
		pd3dDevice->SetSamplerState( 0, ESamplerStateType::ADDRESSV,  D3DTADDRESS_WRAP );
		SetSamplerState(0, ESamplerStateType::MINFILTER, D3DTEXF_POINT, true);
		SetSamplerState(0, ESamplerStateType::MAGFILTER, D3DTEXF_POINT, true);
		pd3dDevice->SetRenderState(ERenderState::CULLMODE, RSV_CULL_CCW);
		pd3dDevice->SetRenderState(ERenderState::ZENABLE, TRUE);m_bZEnable = true;
		pd3dDevice->SetRenderState(ERenderState::ZWRITEENABLE, TRUE);
		pd3dDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_SRCALPHA);
		pd3dDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_INVSRCALPHA);
		break;
	}
	case TECH_PARTICLES:
	{
		VertexDeclarationPtr pDecl = GetVertexDeclaration(S0_POS_TEX0_COLOR);
		if (pDecl == 0)
			return false;
		CGlobals::GetRenderDevice()->SetIndices(0);
		CGlobals::GetRenderDevice()->SetStreamSource(0, 0, 0, 0);
		CGlobals::GetRenderDevice()->SetVertexDeclaration(pDecl);
		pd3dDevice->SetRenderState(ERenderState::ALPHATESTENABLE, FALSE);
		pd3dDevice->SetRenderState(ERenderState::ALPHABLENDENABLE, TRUE); // force blending
		pd3dDevice->SetRenderState(ERenderState::ZWRITEENABLE, FALSE);
		pd3dDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_SRCALPHA);
		pd3dDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_INVSRCALPHA);
		SetCullingMode(false);
		pd3dDevice->SetSamplerState(0, ESamplerStateType::MIPFILTER, D3DTEXF_LINEAR);
		SetSamplerState(0, ESamplerStateType::MINFILTER, D3DTEXF_LINEAR, true);
		SetSamplerState(0, ESamplerStateType::MAGFILTER, D3DTEXF_LINEAR, true);
		break;
	}
	case TECH_SKY_DOME:
	{
		VertexDeclarationPtr pDecl = GetVertexDeclaration(S0_POS_TEX0);
		if (pDecl == 0)
			return false;
		CGlobals::GetRenderDevice()->SetIndices(0);
		CGlobals::GetRenderDevice()->SetStreamSource(0, 0, 0, 0);
		CGlobals::GetRenderDevice()->SetVertexDeclaration(pDecl);
		pd3dDevice->SetSamplerState(0, ESamplerStateType::ADDRESSU, D3DTADDRESS_WRAP);
		pd3dDevice->SetSamplerState(0, ESamplerStateType::ADDRESSV, D3DTADDRESS_WRAP);
		pd3dDevice->SetSamplerState(1, ESamplerStateType::ADDRESSU, D3DTADDRESS_WRAP);
		pd3dDevice->SetSamplerState(1, ESamplerStateType::ADDRESSV, D3DTADDRESS_WRAP);
		SetSamplerState(0, ESamplerStateType::MINFILTER, D3DTEXF_LINEAR, true);
		SetSamplerState(0, ESamplerStateType::MAGFILTER, D3DTEXF_LINEAR, true);
		pd3dDevice->SetRenderState(ERenderState::CULLMODE, RSV_CULL_NONE);
		pd3dDevice->SetRenderState(ERenderState::ZENABLE, TRUE);m_bZEnable = true;
		pd3dDevice->SetRenderState(ERenderState::ZFUNC, D3DCMP_LESSEQUAL);

		pd3dDevice->SetRenderState(ERenderState::ZWRITEENABLE, FALSE);
		pd3dDevice->SetRenderState(ERenderState::ALPHABLENDENABLE, TRUE);
		pd3dDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_SRCALPHA);
		pd3dDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_INVSRCALPHA);
		applyFogParameters();
		break;
	}
	case TECH_SKY_MESH:
	{
		VertexDeclarationPtr pDecl = GetVertexDeclaration(S0_POS_NORM_TEX0);
		if (pDecl == 0)
			return false;
		CGlobals::GetRenderDevice()->SetIndices(0);
		CGlobals::GetRenderDevice()->SetStreamSource(0, 0, 0, 0);
		CGlobals::GetRenderDevice()->SetVertexDeclaration(pDecl);
		pEffect->EnableAlphaBlending(false);
		pEffect->EnableAlphaTesting(false);
		pd3dDevice->SetSamplerState(0, ESamplerStateType::ADDRESSU, D3DTADDRESS_CLAMP);
		pd3dDevice->SetSamplerState(0, ESamplerStateType::ADDRESSV, D3DTADDRESS_CLAMP);
		// used to be D3DTEXF_LINEAR in PC, but we force using POINT in mobile version. 
		// since the sky texture is really low resolution. 
		//SetSamplerState(0, ESamplerStateType::MINFILTER, D3DTEXF_POINT, true);
		//SetSamplerState(0, ESamplerStateType::MAGFILTER, D3DTEXF_POINT, true);
		pd3dDevice->SetRenderState(ERenderState::CULLMODE, RSV_CULL_CCW);
		pd3dDevice->SetRenderState(ERenderState::ZENABLE, TRUE);m_bZEnable = true;
		pd3dDevice->SetRenderState(ERenderState::ZWRITEENABLE, FALSE);
		pd3dDevice->SetRenderState(ERenderState::ALPHABLENDENABLE, TRUE);
		pd3dDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_SRCALPHA);
		pd3dDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_INVSRCALPHA);
		applyFogParameters();
		break;
	}
	case TECH_CHARACTER:
	case TECH_SIMPLE_MESH_NORMAL:
	{
		if (nLastEffectHandle != TECH_CHARACTER && nLastEffectHandle != TECH_SIMPLE_MESH_NORMAL)
		{
			VertexDeclarationPtr pDecl = GetVertexDeclaration(S0_POS_NORM_TEX0);
			if (pDecl == 0)
				return false;
			CGlobals::GetRenderDevice()->SetIndices(0);
			CGlobals::GetRenderDevice()->SetStreamSource(0, 0, 0, 0);
			CGlobals::GetRenderDevice()->SetVertexDeclaration(pDecl);
			pEffect->EnableAlphaBlending(false);
			pEffect->EnableAlphaTesting(false);
			pd3dDevice->SetSamplerState(0, ESamplerStateType::ADDRESSU, D3DTADDRESS_WRAP);
			pd3dDevice->SetSamplerState(0, ESamplerStateType::ADDRESSV, D3DTADDRESS_WRAP);
			SetSamplerState(0, ESamplerStateType::MINFILTER, D3DTEXF_LINEAR, true);
			SetSamplerState(0, ESamplerStateType::MAGFILTER, D3DTEXF_LINEAR, true);
			pd3dDevice->SetRenderState(ERenderState::CULLMODE, RSV_CULL_CCW);
			pd3dDevice->SetRenderState(ERenderState::ZENABLE, TRUE);m_bZEnable = true;
			pd3dDevice->SetRenderState(ERenderState::ZWRITEENABLE, TRUE);
			pd3dDevice->SetRenderState(ERenderState::ALPHABLENDENABLE, TRUE);
			pd3dDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_SRCALPHA);
			pd3dDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_INVSRCALPHA);

			applyFogParameters();
		}
		break;
	}
	case TECH_TERRAIN:
	{
		auto pDecl = GetVertexDeclaration(S0_POS_NORM_TEX0_TEX1);
		if (pDecl == 0)
			return false;
		CGlobals::GetRenderDevice()->SetIndices(0);
		CGlobals::GetRenderDevice()->SetStreamSource(0, 0, 0, 0);
		EnableGlobalLighting(bEnableSunLight && bEnableLight);
		EnableLocalLighting(bEnableLight);

		// Just for test, 
		applyLocalLightingData();
		applyFogParameters();

		ParaMaterial matTerrain = CGlobals::GetSceneState()->GetGlobalMaterial();
		BlockWorldClient* pBlockWorldClient = BlockWorldClient::GetInstance();
		if (pBlockWorldClient && pBlockWorldClient->IsInBlockWorld())
		{
			float fLightness = max(1.f - fabs(CGlobals::GetScene()->GetSunLight().GetTimeOfDaySTD()), 0.25f);
			matTerrain.Ambient = LinearColor(fLightness*0.7f, fLightness*0.7f, fLightness*0.7f, 1.f);
			matTerrain.Diffuse = LinearColor(fLightness*0.4f, fLightness*0.4f, fLightness*0.4f, 1.f);
			// pEffect->setParameter(CEffectFile::k_ambientLight, &(matTerrain.Ambient));
		}
		else
		{
			matTerrain.Diffuse = LinearColor(1.f, 1.f, 1.f, 1.f);
		}
		pEffect->applySurfaceMaterial(&matTerrain);

		SetCullingMode(true);
		pd3dDevice->SetRenderState(ERenderState::ZENABLE, TRUE);m_bZEnable = true;
		pd3dDevice->SetRenderState(ERenderState::ZWRITEENABLE, TRUE);
		pd3dDevice->SetRenderState(ERenderState::ALPHATESTENABLE, FALSE);
		pd3dDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_ONE);
		pd3dDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_ONE);
		
		// it should not be Less function. Because we have multi-texturing
		pd3dDevice->SetRenderState(ERenderState::ZFUNC, D3DCMP_LESSEQUAL);

		CGlobals::GetRenderDevice()->SetVertexDeclaration(pDecl);
		break;
	}
	default:
	{
		if (pEffect->GetParamBlock())
		{
			// apply per effect parameters. 
			pEffect->GetParamBlock()->ApplyToEffect(pEffect);
		}
		break;
	}
	}
#endif
	return true;
}

void EffectManager::EndEffect()
{
	auto pd3dDevice = CGlobals::GetRenderDevice();
#ifdef USE_DIRECTX_RENDERER
	if((m_pCurrentEffect == 0) || (!m_pCurrentEffect->IsValid()))
	{
		// fixed programming pipeline
		switch(m_nCurrentEffect)
		{
		case TECH_OCCLUSION_TEST:
			pd3dDevice->SetRenderState( ERenderState::ALPHABLENDENABLE, FALSE );
			pd3dDevice->SetRenderState( ERenderState::SRCBLEND,  D3DBLEND_SRCALPHA );
			pd3dDevice->SetRenderState( ERenderState::DESTBLEND, D3DBLEND_INVSRCALPHA );
			pd3dDevice->SetRenderState( ERenderState::ZWRITEENABLE, TRUE );
			break;
		case TECH_TERRAIN:
			{
				// restore texture states
				for(int i=0;i<8;++i)
				{
					GETD3D(CGlobals::GetRenderDevice())->SetTexture( i, NULL );
				}
			
				pd3dDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_SRCALPHA);
				pd3dDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_INVSRCALPHA);
				GETD3D(CGlobals::GetRenderDevice())->SetSamplerState( 0, D3DMapping::toD3DSamplerSatetType(ESamplerStateType::ADDRESSU),  D3DTADDRESS_WRAP );
				GETD3D(CGlobals::GetRenderDevice())->SetSamplerState( 0, D3DMapping::toD3DSamplerSatetType(ESamplerStateType::ADDRESSV),  D3DTADDRESS_WRAP );
				if(CGlobals::GetGlobalTerrain()->GetSettings()->UseNormals() && GetScene()->IsLightEnabled())
				{
					pd3dDevice->SetRenderState( ERenderState::LIGHTING, FALSE );
					applyLocalLightingData(NULL, 0);
				}
				/*if(IsUsingFullScreenGlow())
				{
					pd3dDevice->SetRenderState(ERenderState::SEPARATEALPHABLENDENABLE, false);
				}*/
				break;
			}
		case TECH_SIMPLE_MESH:
		case TECH_SKY_DOME:
			break;
		case TECH_SIMPLE_MESH_NORMAL:
		case TECH_SIMPLE_MESH_NORMAL_CTOR:
		case TECH_SIMPLE_MESH_NORMAL_VEGETATION:
		case TECH_SIMPLE_MESH_NORMAL_SELECTED:
		case TECH_SIMPLE_MESH_NORMAL_BORDER:
		case TECH_SIMPLE_MESH_NORMAL_TRANSPARENT:
		case TECH_SIMPLE_MESH_NORMAL_SHADOW:
		case TECH_SIMPLE_CAD_MODEL:
			if(GetScene()->IsLightEnabled())
				pd3dDevice->SetRenderState( ERenderState::LIGHTING, FALSE );
			break;
		case TECH_SIMPLE_DROPSHADOW:
			if(GetScene()->IsLightEnabled())
				pd3dDevice->SetRenderState( ERenderState::LIGHTING, FALSE );
			break;
		case TECH_SKY_MESH:
			GETD3D(CGlobals::GetRenderDevice())->SetSamplerState( 0, D3DMapping::toD3DSamplerSatetType(ESamplerStateType::ADDRESSU),  D3DTADDRESS_WRAP );
			GETD3D(CGlobals::GetRenderDevice())->SetSamplerState( 0, D3DMapping::toD3DSamplerSatetType(ESamplerStateType::ADDRESSV),  D3DTADDRESS_WRAP );
			break;
		case TECH_OCEAN_CLOUD:
		case TECH_OCEAN_SIMPLE:
		case TECH_OCEAN:
			if(GetScene()->IsLightEnabled())
				pd3dDevice->SetRenderState( ERenderState::LIGHTING, FALSE );
			pd3dDevice->SetRenderState( ERenderState::ZWRITEENABLE,     TRUE );
			pd3dDevice->SetRenderState( ERenderState::ALPHABLENDENABLE, FALSE );
			break;
		case TECH_OCEAN_UNDERWATER:
			// Restore render states
			pd3dDevice->SetRenderState(ERenderState::ZENABLE, TRUE); m_bZEnable = true;
			pd3dDevice->SetRenderState(ERenderState::ZWRITEENABLE, TRUE);
			pd3dDevice->SetRenderState( ERenderState::FOGENABLE,        GetScene()->IsFogEnabled() );
			break;
		case TECH_CHARACTER:
			if(GetScene()->IsLightEnabled()){
				pd3dDevice->SetRenderState( ERenderState::LIGHTING, FALSE );
				applyLocalLightingData(NULL, 0);
			}
			break;
		case TECH_PARTICLES:
			SetCullingMode(true);
			pd3dDevice->SetRenderState( ERenderState::ZWRITEENABLE, TRUE );
			break;
		case TECH_GUI:
			pd3dDevice->SetRenderState(ERenderState::ZENABLE, TRUE); m_bZEnable = true;
			pd3dDevice->SetRenderState( ERenderState::ZWRITEENABLE,     TRUE );
			pd3dDevice->SetRenderState( ERenderState::FOGENABLE,        GetScene()->IsFogEnabled() );
			break;
		case TECH_BLOCK_FANCY:
		case TECH_BLOCK:
			GETD3D(CGlobals::GetRenderDevice())->SetSamplerState( 0, D3DMapping::toD3DSamplerSatetType(ESamplerStateType::ADDRESSU),  D3DTADDRESS_WRAP );
			GETD3D(CGlobals::GetRenderDevice())->SetSamplerState( 0, D3DMapping::toD3DSamplerSatetType(ESamplerStateType::ADDRESSV),  D3DTADDRESS_WRAP );
			SetSamplerState( 0, ESamplerStateType::MAGFILTER,  D3DTEXF_LINEAR, true);
			SetSamplerState( 0, ESamplerStateType::MINFILTER,  D3DTEXF_LINEAR, true);
			pd3dDevice->SetRenderState( ERenderState::ALPHAREF, FIXED_FUNCTION_ALPHA_TESTING_REF);
			pd3dDevice->SetRenderState( ERenderState::FOGENABLE,        GetScene()->IsFogEnabled() );
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

		// effect files
		switch(m_nCurrentEffect)
		{
		case TECH_OCCLUSION_TEST:
			m_pCurrentEffect->EndPass();
			m_pCurrentEffect->end();
			break;
		case TECH_SKY_MESH:
			{
				GETD3D(CGlobals::GetRenderDevice())->SetSamplerState( 0, D3DMapping::toD3DSamplerSatetType(ESamplerStateType::ADDRESSU),  D3DTADDRESS_WRAP );
				GETD3D(CGlobals::GetRenderDevice())->SetSamplerState( 0, D3DMapping::toD3DSamplerSatetType(ESamplerStateType::ADDRESSV),  D3DTADDRESS_WRAP );
				break;
			}
		case TECH_SKY_DOME:
			{
				GETD3D(CGlobals::GetRenderDevice())->SetSamplerState( 0, D3DMapping::toD3DSamplerSatetType(ESamplerStateType::ADDRESSU),  D3DTADDRESS_WRAP );
				GETD3D(CGlobals::GetRenderDevice())->SetSamplerState( 0, D3DMapping::toD3DSamplerSatetType(ESamplerStateType::ADDRESSV),  D3DTADDRESS_WRAP );
				DisableD3DCulling(false);
				SetCullingMode(true);
				break;
			}
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
					if (pShadowMap !=NULL)
					{
						EnableUsingShadowMap(false);
						m_pCurrentEffect->EnableShadowmap(0);
						// restore texture states
						GETD3D(CGlobals::GetRenderDevice())->SetTexture( 2, NULL );
					}
				}
			}
			else if(m_nCurrentEffect == TECH_SIMPLE_MESH_NORMAL_INSTANCED)
			{
				GETD3D(CGlobals::GetRenderDevice())->SetStreamSourceFreq(0,1);
				// Set the second stream to be per instance data and iterate once per instance
				GETD3D(CGlobals::GetRenderDevice())->SetStreamSourceFreq(1,1);
			}
			else if(m_nCurrentEffect == TECH_SIMPLE_MESH_NORMAL_TRANSPARENT)
			{
				CGlobals::GetRenderDevice()->SetRenderState(ERenderState::ZWRITEENABLE, TRUE);
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
			EnableLocalLighting(false);
			break;
		case TECH_SIMPLE_MESH_NORMAL_CTOR:
			CGlobals::GetRenderDevice()->SetRenderState(ERenderState::ALPHATESTENABLE, FALSE);
			DisableD3DCulling(false);
			SetCullingMode(true);
			break;
		case TECH_OCEAN_CLOUD:
		case TECH_OCEAN_SIMPLE:
		case TECH_OCEAN:
			SetCullingMode(true);

		case TECH_TERRAIN:
			{
				for(int i=0;i<8;++i)
				{
					GETD3D(CGlobals::GetRenderDevice())->SetTexture( i, NULL );
				}
				if(GetScene()->IsShadowMapEnabled())
				{
					CShadowMap* pShadowMap = GetShadowMap();
					if (pShadowMap !=NULL)
					{
						EnableUsingShadowMap(false);
						m_pCurrentEffect->EnableShadowmap(0);
						pShadowMap->UnsetShadowTexture(2);
					}
				}
				pd3dDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_SRCALPHA);
				pd3dDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_INVSRCALPHA);
				GETD3D(CGlobals::GetRenderDevice())->SetSamplerState( 0, D3DMapping::toD3DSamplerSatetType(ESamplerStateType::ADDRESSU),  D3DTADDRESS_WRAP );
				GETD3D(CGlobals::GetRenderDevice())->SetSamplerState( 0, D3DMapping::toD3DSamplerSatetType(ESamplerStateType::ADDRESSV),  D3DTADDRESS_WRAP );
				break;
			}
		case TECH_PARTICLES:
		case TECH_WATER_RIPPLE:
			SetCullingMode(true);
			pd3dDevice->SetRenderState( ERenderState::ZWRITEENABLE, TRUE );
			break;
		case TECH_FULL_SCREEN_GLOW:
			{
				break;
			}
		case TECH_SCREEN_WAVE:
			{
				break;
			}
		case TECH_BLOCK_FANCY:
		case TECH_BLOCK:
			{
				SetCullingMode(true);
				pd3dDevice->SetRenderState( ERenderState::SRCBLEND,  D3DBLEND_SRCALPHA );
				pd3dDevice->SetRenderState( ERenderState::DESTBLEND, D3DBLEND_INVSRCALPHA );
				pd3dDevice->SetRenderState( ERenderState::FOGENABLE, GetScene()->IsFogEnabled() );
				GETD3D(CGlobals::GetRenderDevice())->SetStreamSource(0,NULL,0,0);
				GETD3D(CGlobals::GetRenderDevice())->SetIndices(NULL);
				GETD3D(CGlobals::GetRenderDevice())->SetTexture(0,NULL);
				GETD3D(CGlobals::GetRenderDevice())->SetTexture(1,NULL);
				GETD3D(CGlobals::GetRenderDevice())->SetTexture(2,NULL);
				GETD3D(CGlobals::GetRenderDevice())->SetTexture(3,NULL);
				break;
			}
		case TECH_SHADOWMAP_BLUR:
			{
				break;
			}
		default:
			if(m_nCurrentEffect>1000)
			{
				if(GetScene()->IsLightEnabled())
					pd3dDevice->SetRenderState( ERenderState::LIGHTING, FALSE );
			}
			break;
		}
		DisableD3DAlphaTesting(false);
	}
#elif defined(USE_OPENGL_RENDERER)
	switch (m_nCurrentEffect)
	{
	case TECH_TERRAIN:
	{
		EnableD3DAlphaBlending(true);
		pd3dDevice->SetRenderState(ERenderState::SRCBLEND, D3DBLEND_SRCALPHA);
		pd3dDevice->SetRenderState(ERenderState::DESTBLEND, D3DBLEND_INVSRCALPHA);
		pd3dDevice->SetSamplerState(0, ESamplerStateType::ADDRESSU, D3DTADDRESS_WRAP);
		pd3dDevice->SetSamplerState(0, ESamplerStateType::ADDRESSV, D3DTADDRESS_WRAP);
		break;
	}
	default:
		break;
	}
#endif
	m_nCurrentEffect = 0;
	m_pCurrentEffect = NULL;
}

void EffectManager::SetAllEffectsTechnique(EffectTechniques nTech)
{
//#ifdef USE_DIRECTX_RENDERER
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
//#endif
}

void EffectManager::SetDefaultEffectMapping(int nLevel)
{
	m_nEffectLevel = nLevel;
#ifdef USE_DIRECTX_RENDERER
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

	if(nLevel<10)
	{
		//////////////////////////////////////////////////////////////////////////
		// fixed programming pipeline, simply clear all mapping
		m_HandleMap.clear();
		// do not render terrain with normal.
		CGlobals::GetGlobalTerrain()->EnableLighting(false);

		//CGlobals::GetOceanManager()->SetRenderTechnique(COceanManager::OCEAN_TECH_QUAD);
		//DWORD PixelShaderVersion = CGlobals::GetDirectXEngine().m_d3dCaps.PixelShaderVersion;
		//DWORD VertexShaderVersion = CGlobals::GetDirectXEngine().m_d3dCaps.VertexShaderVersion;

		//if(PixelShaderVersion>=D3DPS_VERSION(2,0) || VertexShaderVersion>=D3DVS_VERSION(2,0))
		//{
		//	pEffect = GetByName("ocean_fft");
		//	if(pEffect == 0)
		//		pEffect = CGlobals::GetAssetManager()->LoadEffectFile("ocean_fft", ":IDR_FX_OCEANWATER"); //"shaders/ocean_water.fx"
		//	MapHandleToEffect(TECH_OCEAN, pEffect);
		//}
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
	}

#elif defined(USE_OPENGL_RENDERER)
	// clear current effect
	if (m_pCurrentEffect != 0)
		EndEffect();
	m_nCurrentEffect = 0;
	m_pCurrentEffect = NULL;

	CEffectFile* pEffect = NULL;

	pEffect = GetEffectByName("block");
	MapHandleToEffect(TECH_BLOCK, pEffect);

	pEffect = GetEffectByName("BMaxModel");
	MapHandleToEffect(TECH_BMAX_MODEL, pEffect);

	pEffect = GetEffectByName("singleColor");
	MapHandleToEffect(TECH_SINGLE_COLOR, pEffect);

	pEffect = GetEffectByName("simple_mesh_normal");
	MapHandleToEffect(TECH_SIMPLE_MESH_NORMAL, pEffect);
	MapHandleToEffect(TECH_CHARACTER, pEffect);

	pEffect = GetEffectByName("simple_particle");
	MapHandleToEffect(TECH_PARTICLES, pEffect);

	pEffect = GetEffectByName("guiEffect");
	MapHandleToEffect(TECH_GUI, pEffect);
	MapHandleToEffect(TECH_NONE, pEffect);
	MapHandleToEffect(TECH_UNKNOWN, pEffect);

	pEffect = GetEffectByName("guiTextEffect");
	MapHandleToEffect(TECH_GUI_TEXT, pEffect);

	pEffect = GetEffectByName("sky");
	MapHandleToEffect(TECH_SKY_MESH, pEffect);

	pEffect = GetEffectByName("skydome");
	MapHandleToEffect(TECH_SKY_DOME, pEffect);

	pEffect = GetEffectByName("terrain_normal");
	MapHandleToEffect(TECH_TERRAIN, pEffect);
	// using normal in vertex shader
	CGlobals::GetGlobalTerrain()->EnableLighting(true);

	// preload mapping
	pEffect = GetEffectByName("RedBlueStereo");
#endif
}

int EffectManager::GetDefaultEffectMapping()
{
	return m_nEffectLevel;
}

CEffectFile* EffectManager::GetEffectByName( const string& sName )
{
	CEffectFile* pEffect = NULL;

#ifdef USE_DIRECTX_RENDERER
	if (sName == "simple_mesh_normal_low")
	{
		pEffect = GetByName("simple_mesh_normal_low");
		if(pEffect == 0)
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("simple_mesh_normal_low", ":IDR_FX_SIMPLE_MESH_NORMAL_LOW");//"shaders/simple_mesh_normal_low.fx"
	}
	else if (sName == "simple_mesh_normal")
	{
		pEffect = GetByName("simple_mesh_normal");
		if(pEffect == 0)
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("simple_mesh_normal", ":IDR_FX_SIMPLE_MESH_NORMAL");//"shaders/simple_mesh_normal.fx"
	}
	else if (sName == "simple_mesh_normal_vegetation")
	{
		pEffect = GetByName("simple_mesh_normal_vegetation");
		if(pEffect == 0)
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("simple_mesh_normal_vegetation", ":IDR_FX_SIMPLE_MESH_VEGETATION");//"shaders/simple_mesh_normal_vegetation.fx"
	}
	else if (sName == "simple_mesh_normal_instanced")
	{
		pEffect = GetByName("simple_mesh_normal_instanced");
		if(pEffect == 0)
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("simple_mesh_normal_instanced", ":IDR_FX_SIMPLE_MESH_NORMAL_INSTANCED");//"shaders/simple_mesh_normal_instanced.fx"
	}
	else if (sName == "simple_mesh")
	{
		pEffect = GetByName("simple_mesh");
		if(pEffect == 0)
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("simple_mesh", ":IDR_FX_SIMPLE_MESH");//"shaders/simple_mesh.fx"

	}
	else if (sName == "simple_mesh_normal_ctor")
	{
		pEffect = GetByName("simple_mesh_normal_ctor");
		if(pEffect == 0)
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("simple_mesh_normal_ctor", ":IDR_FX_SMESH_CTOR");//"shaders/smesh_ctor.fx"
	}
	else if (sName == "simple_mesh_normal_specialeffects")
	{
		pEffect = GetByName("simple_mesh_normal_specialeffects");
		if(pEffect == 0)
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("simple_mesh_normal_specialeffects", ":IDR_FX_SIMPLE_MESH_NORMAL_SPECIALEFFECTS");//"shaders/simple_mesh_normal_specialeffects.fx"
	}
	else if (sName == "simple_mesh_normal_unlit")
	{
		pEffect = GetByName("simple_mesh_normal_unlit");
		if(pEffect == 0)
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("simple_mesh_normal_unlit", ":IDR_FX_SIMPLE_MESH_NORMAL_UNLIT");//"shaders/simple_mesh_normal_unlit.fx"

	}
	else if (sName == "simple_mesh_normal_selected")
	{
		pEffect = GetByName("simple_mesh_normal_selected");
		if(pEffect == 0)
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("simple_mesh_normal_selected", ":IDR_FX_SIMPLE_MESH_NORMAL_SELECTED");//"shaders/simple_mesh_normal_selected.fx"

	}
	else if (sName == "simple_mesh_normal_border")
	{
		pEffect = GetByName("simple_mesh_normal_border");
		if(pEffect == 0)
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("simple_mesh_normal_border", ":IDR_FX_SIMPLE_MESH_NORMAL_BORDER");//"shaders/simple_mesh_normal_border.fx"

	}
	else if (sName == "simple_mesh_normal_shadow")
	{
		pEffect = GetByName("simple_mesh_normal_shadow");
		if(pEffect == 0)
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("simple_mesh_normal_shadow", ":IDR_FX_SIMPLE_MESH_SHADOW");//"shaders/simple_mesh_shadow.fx"

	}
	else if (sName == "simple_mesh_normal_tex2")
	{
		pEffect = GetByName("simple_mesh_normal_tex2");
		if(pEffect == 0)
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("simple_mesh_normal_tex2", ":IDR_FX_SIMPLE_MESH_NORMAL_TEX2");//"shaders/simple_mesh_tex2.fx"

	}
	else if (sName == "occlusion_test")
	{
		pEffect = GetByName("occlusion_test");
		if(pEffect == 0)
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("occlusion_test", ":IDR_FX_OCCLUSION_TEST");//"shaders/occlusion_test.fx"

	}
	else if (sName == "ocean_fft")
	{
		pEffect = GetByName("ocean_fft");
		if(pEffect == 0)
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("ocean_fft", ":IDR_FX_OCEANWATER"); //"shaders/ocean_water.fx"
	}
	else if (sName == "ocean_fft_high")
	{
		pEffect = GetByName("ocean_fft_high");
		if(pEffect == 0)
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("ocean_fft_high", ":IDR_FX_OCEANWATER_HIGH"); //"shaders/ocean_water_high.fx"
	}
	else if (sName == "ocean_simple")
	{
		pEffect = GetByName("ocean_simple");
		if(pEffect == 0)
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("ocean_simple", ":IDR_FX_OCEANWATER_QUAD"); //"shaders/ocean_water_quad.fx"
	}
	else if (sName == "ocean_cloud")
	{
		pEffect = GetByName("ocean_cloud");
		if(pEffect == 0)
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("ocean_cloud", ":IDR_FX_OCEANWATER_CLOUD"); //"shaders/ocean_water_cloud.fx"
	}
	else if (sName == "sky")
	{
		pEffect = GetByName("sky");
		if(pEffect == 0)
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("sky", ":IDR_FX_SKY");//"shaders/sky.fx"

	}
	else if (sName == "skydome")
	{
		pEffect = GetByName("skydome");
		if(pEffect == 0)
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("skydome", ":IDR_FX_SKYDOME");//"shaders/skydome.fx"
	}
	else if (sName == "waterripple")
	{
		pEffect = GetByName("waterripple");
		if(pEffect == 0)
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("waterripple", ":IDR_FX_WATER_RIPPLE");//"shaders/water_ripple.fx"

	}
	else if (sName == "fullscreen_glow")
	{
		pEffect = GetByName("fullscreen_glow");
		if(pEffect == 0)
		{
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("fullscreen_glow", ":IDR_FX_FULLSCREEN_GLOW");//"shaders/fullscreen_glow.fx"
			// this should be removed in future
			m_nGlowTechnique = pEffect->GetCurrentTechniqueIndex();
		}
	}
	else if (sName == "shadowmap_blur")
	{
		pEffect = GetByName("shadowmap_blur");
		if(pEffect == 0)
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("shadowmap_blur", ":IDR_FX_SHADOWMAP_BLUR");//"shaders/shadowmap_blur.fx"
	}
	else if (sName == "terrain_normal")
	{
		pEffect = GetByName("terrain_normal");
		if(pEffect == 0)
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("terrain_normal", ":IDR_FX_TERRAIN_NORMAL");//"shaders/terrain_normal.fx"
	}
	else if (sName == "simple_particle")
	{
		pEffect = GetByName("simple_particle");
		if(pEffect == 0)
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("simple_particle", ":IDR_FX_SIMPLE_PARTICLE");//"shaders/simple_particle.fx"
	}
	else if (sName == "simple_cad_model")
	{
		pEffect = GetByName("simple_cad_model");
		if(pEffect == 0)
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("simple_cad_model", ":IDR_FX_SIMPLE_CAD_MODEL");//"shaders/simple_mesh_normal_low.fx"
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
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("screenWave",":IDR_FX_SCREENWAVE");
	}
	else if(sName == "block")
	{
		pEffect = GetByName("block");
		if(pEffect == 0)
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("block",":IDR_FX_BLOCK");
	}
	else if (sName == "BMaxModel")
	{
		pEffect = GetByName("BMaxModel");
		if (pEffect == 0)
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("BMaxModel",":IDR_FX_BMAXMODEL");
	}
	else
	{
		pEffect = 0;
	}
#elif defined(USE_OPENGL_RENDERER)
	if (sName == "block")
	{
		pEffect = GetByName("block");
		if (pEffect == 0)
		{
			// multiple pass effect in opengl is implemented as shader arrays.
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("block", ":IDR_FX_BLOCK");
		}
	}
	else if (sName == "singleColor")
	{
		pEffect = GetByName("singleColor");
		if (pEffect == 0)
		{
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("singleColor", ":IDR_FX_SINGLECOLOR");
		}
	}
	else if (sName == "RedBlueStereo")
	{
		pEffect = GetByName("RedBlueStereo");
		if (pEffect == 0)
		{
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("RedBlueStereo", ":IDR_FX_REDBLUESTEREO");
		}
	}
	else if (sName == "simple_mesh_normal")
	{
		pEffect = GetByName("simple_mesh_normal");
		if (pEffect == 0)
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("simple_mesh_normal", ":IDR_FX_SIMPLE_MESH_NORMAL");
	}
	else if (sName == "simple_particle")
	{
		pEffect = GetByName("simple_particle");
		if (pEffect == 0)
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("simple_particle", ":IDR_FX_SIMPLE_PARTICLE");
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
	else if (sName == "sky")
	{
		pEffect = GetByName("sky");
		if(pEffect == 0)
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("sky", ":IDR_FX_SKY");

	}
	else if (sName == "skydome")
	{
		pEffect = GetByName("skydome");
		if (pEffect == 0)
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("skydome", ":IDR_FX_SKYDOME");
	}
	else if (sName == "terrain_normal")
	{
		pEffect = GetByName("terrain_normal");
		if (pEffect == 0)
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("terrain_normal", ":IDR_FX_TERRAIN_NORMAL");
	}
	else if (sName == "BMaxModel")
	{
		pEffect = GetByName("BMaxModel");
		if (pEffect == 0)
			pEffect = CGlobals::GetAssetManager()->LoadEffectFile("BMaxModel",":IDR_FX_BMAXMODEL");
	}
// force compiling to test for shader
// #define PRECOMPILE_SHADER
#ifdef PRECOMPILE_SHADER
	if (pEffect)
		pEffect->LoadAsset(); 
#endif
#endif

	return pEffect;
}


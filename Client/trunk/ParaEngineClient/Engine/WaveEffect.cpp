//-----------------------------------------------------------------------------
// Class: WaveEffect
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2005.1.16
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "WaveEffect.h"
#include "Globals.h"
#include "EffectManager.h"
#include "DirectXEngine.h"
#include "ParaWorldAsset.h"

namespace ParaEngine
{
	WaveEffect::WaveEffect()
		:m_pBackbufferCopy(NULL),m_pBackbufferCopySurface(NULL)
	{
		m_waveParam[0] = 0;
		m_waveParam[1] = 0.5f;
		m_waveParam[2] = 0.05f;
		m_waveParam[3] = 0.05f;
	}

	WaveEffect::~WaveEffect()
	{
		InvalidateDeviceObjects();
	}

	HRESULT WaveEffect::InvalidateDeviceObjects()
	{
		SAFE_RELEASE(m_pBackbufferCopySurface);
		SAFE_RELEASE(m_pBackbufferCopy);
		return S_OK;
	}

	HRESULT WaveEffect::RestoreDeviceObjects()
	{
		IDirect3DDevice9* pDevice = CGlobals::GetRenderDevice();

		D3DFORMAT surfaceFmt = D3DFMT_A8R8G8B8;
		D3DVIEWPORT9 viewport;
		pDevice->GetViewport(&viewport);

		HRESULT hr = pDevice->CreateTexture(viewport.Width,viewport.Height,1,D3DUSAGE_RENDERTARGET,surfaceFmt,
			D3DPOOL_DEFAULT,&m_pBackbufferCopy,NULL);

		if(FAILED(hr))
			return E_FAIL;

		hr = m_pBackbufferCopy->GetSurfaceLevel(0,&m_pBackbufferCopySurface);
		if(FAILED(hr))
			return E_FAIL;

		if(m_pNoiseMap == NULL)
			m_pNoiseMap = CGlobals::GetAssetManager()->LoadTexture("","Texture/Aries/ShaderResource/waveMap.dds",TextureEntity::StaticTexture);

		m_quadVertices[0].p  = Vector3(-1,-1,0);
		m_quadVertices[0].uv = Vector2(0,1);
		m_quadVertices[1].p  = Vector3(1,-1,0);
		m_quadVertices[1].uv = Vector2(1,1);
		m_quadVertices[2].p  = Vector3(-1,1,0);
		m_quadVertices[2].uv = Vector2(0,0);
		m_quadVertices[3].p  = Vector3(1,1,0);
		m_quadVertices[3].uv = Vector2(1,0);

		float xHalfPixel = 0.5f/viewport.Width;
		float yHalfPixel = 0.5f/viewport.Height;

		for (int i=0;i<4;++i)
		{
			m_quadVertices[i].uv.x += xHalfPixel;
			m_quadVertices[i].uv.y += yHalfPixel;
		}
		return S_OK;
	}

	HRESULT WaveEffect::Render()
	{
		IDirect3DDevice9* pDevice = CGlobals::GetRenderDevice();
		
		EffectManager* pEffectManager = CGlobals::GetEffectManager();
		pEffectManager->BeginEffect(TECH_SCREEN_WAVE);
		CEffectFile* pEffectFile = pEffectManager->GetCurrentEffectFile();

		if(pEffectFile !=0 && pEffectFile->begin(true,0))
		{
			if(pEffectFile->BeginPass(0))
			{
				IDirect3DSurface9* pBackBuffer = CGlobals::GetDirectXEngine().GetRenderTarget();
				HRESULT hr = pDevice->StretchRect(pBackBuffer,NULL,m_pBackbufferCopySurface,NULL,D3DTEXF_LINEAR);
				if(FAILED(hr))
				{
					OUTPUT_LOG("wave effect StretchRect() failed!\r\n");
					return hr;
				}
				
				pDevice->SetTexture(0,m_pBackbufferCopy);
				pDevice->SetTexture(1,m_pNoiseMap->GetTexture());
				

				pDevice->SetRenderState(D3DRS_CULLMODE,D3DCULL_NONE);
				pDevice->SetRenderState(D3DRS_ZENABLE,false);

				m_waveParam[0] = fmodf((float)CGlobals::GetGameTime() * m_waveParam[3],900000.0f);
				pDevice->SetVertexShaderConstantF(0,m_waveParam,1);

				RenderDevice::DrawPrimitiveUP(pDevice, RenderDevice::DRAW_PERF_TRIANGLES_MESH, D3DPT_TRIANGLESTRIP,2,m_quadVertices,sizeof(mesh_vertex_plain));
			
				pDevice->SetRenderState(D3DRS_CULLMODE,D3DCULL_CCW);
				pDevice->SetRenderState(D3DRS_ZENABLE,true);
				//clean up
				pBackBuffer->Release();
				pDevice->SetTexture(1,NULL);
				pEffectFile->EndPass();
				return S_OK;
			}
		}
		return S_FALSE;
	}
}
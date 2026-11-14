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
#include "effect_file.h"
#include "ParaWorldAsset.h"

namespace ParaEngine
{
	WaveEffect::WaveEffect()
		: m_rhiBackbufferCopy(nullptr), m_rtWidth(0), m_rtHeight(0)
	{
		m_waveParam[0] = 0.0f;
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
		ReleaseRHIResource(m_rhiBackbufferCopy);
		m_rtWidth = 0;
		m_rtHeight = 0;
		return S_OK;
	}

	bool WaveEffect::EnsureRenderTarget(IRenderDevice* pDevice, uint32_t width, uint32_t height)
	{
		if (!pDevice || width == 0 || height == 0)
			return false;

		if (m_rhiBackbufferCopy && m_rtWidth == width && m_rtHeight == height)
			return true;

		ReleaseRHIResource(m_rhiBackbufferCopy);
		m_rhiBackbufferCopy = pDevice->CreateRenderTarget(width, height, PixelFormat::A8R8G8B8, PixelFormat::Unknown);
		if (!m_rhiBackbufferCopy)
		{
			m_rtWidth = 0;
			m_rtHeight = 0;
			return false;
		}
		m_rtWidth = width;
		m_rtHeight = height;

		const RHIDeviceCaps& caps = pDevice->GetCaps();
		const float vTop = caps.topLeftScreenOrigin ? 0.0f : 1.0f;
		const float vBottom = caps.topLeftScreenOrigin ? 1.0f : 0.0f;
		const float xHalfPixel = caps.needsHalfPixelOffset ? (0.5f / (float)width) : 0.0f;
		const float yHalfPixel = caps.needsHalfPixelOffset ? (0.5f / (float)height) : 0.0f;

		m_quadVertices[0].p = Vector3(-1.0f, -1.0f, 0.0f);
		m_quadVertices[0].uv = Vector2(0.0f + xHalfPixel, vBottom + yHalfPixel);
		m_quadVertices[1].p = Vector3(1.0f, -1.0f, 0.0f);
		m_quadVertices[1].uv = Vector2(1.0f + xHalfPixel, vBottom + yHalfPixel);
		m_quadVertices[2].p = Vector3(-1.0f, 1.0f, 0.0f);
		m_quadVertices[2].uv = Vector2(0.0f + xHalfPixel, vTop + yHalfPixel);
		m_quadVertices[3].p = Vector3(1.0f, 1.0f, 0.0f);
		m_quadVertices[3].uv = Vector2(1.0f + xHalfPixel, vTop + yHalfPixel);
		return true;
	}

	HRESULT WaveEffect::RestoreDeviceObjects()
	{
		auto* pDevice = CGlobals::GetRenderDevice();
		if (!pDevice)
			return E_FAIL;

		ParaViewport viewport = pDevice->GetViewport();
		uint32_t width = viewport.Width > 0 ? viewport.Width : pDevice->GetBackBufferWidth();
		uint32_t height = viewport.Height > 0 ? viewport.Height : pDevice->GetBackBufferHeight();
		if (!EnsureRenderTarget(pDevice, width, height))
			return E_FAIL;

		if (!m_pNoiseMap)
			m_pNoiseMap = CGlobals::GetAssetManager()->LoadTexture("", "Texture/Aries/ShaderResource/waveMap.dds", TextureEntity::StaticTexture);

		return S_OK;
	}

	HRESULT WaveEffect::Render()
	{
		auto pDevice = CGlobals::GetRenderDevice();
		if (!pDevice)
			return E_FAIL;

		auto rtBinding = pDevice->GetCurrentRenderTargetBinding();
		IRHIRenderTarget* activeRT = (rtBinding.count > 0) ? rtBinding.targets[0] : nullptr;
		ParaViewport viewport = pDevice->GetViewport();
		uint32_t width = activeRT ? activeRT->GetWidth() : (viewport.Width > 0 ? viewport.Width : pDevice->GetBackBufferWidth());
		uint32_t height = activeRT ? activeRT->GetHeight() : (viewport.Height > 0 ? viewport.Height : pDevice->GetBackBufferHeight());
		if (!EnsureRenderTarget(pDevice, width, height))
			return E_FAIL;

		if (!m_pNoiseMap)
			m_pNoiseMap = CGlobals::GetAssetManager()->LoadTexture("", "Texture/Aries/ShaderResource/waveMap.dds", TextureEntity::StaticTexture);

		EffectManager* pEffectManager = CGlobals::GetEffectManager();
		pEffectManager->BeginEffect(TECH_SCREEN_WAVE);
		CEffectFile* pEffectFile = pEffectManager->GetCurrentEffectFile();

		if (pEffectFile != 0 && pEffectFile->begin(true, 0))
		{
			HRESULT hr = S_FALSE;
			if (pEffectFile->BeginPass(0))
			{
				if (!pDevice->BlitRenderTarget(activeRT, m_rhiBackbufferCopy, nullptr, nullptr, true))
				{
					OUTPUT_LOG("wave effect BlitRenderTarget() failed!\r\n");
					pEffectFile->EndPass();
					pEffectFile->end();
					return E_FAIL;
				}

				pEffectFile->setTexture(0, m_rhiBackbufferCopy->GetColorTexture(0));
				pEffectFile->setTexture(1, m_pNoiseMap ? m_pNoiseMap->GetTexture() : nullptr);

				pDevice->SetRenderState(ERenderState::CULLMODE, RSV_CULL_NONE);
				pDevice->SetRenderState(ERenderState::ZENABLE, FALSE);

				m_waveParam[0] = fmodf((float)CGlobals::GetGameTime() * m_waveParam[3], 900000.0f);
				pEffectFile->SetVector3("waveParam", Vector3(m_waveParam[0], m_waveParam[1], m_waveParam[2]));
				pEffectFile->CommitChanges();

				pDevice->DrawPrimitiveUP(EPrimitiveType::TRIANGLESTRIP, 2, m_quadVertices, sizeof(mesh_vertex_plain));

				pDevice->SetRenderState(ERenderState::CULLMODE, RSV_CULL_CCW);
				pDevice->SetRenderState(ERenderState::ZENABLE, TRUE);

				pEffectFile->setTexture(0, (DeviceTexturePtr_type)nullptr);
				pEffectFile->setTexture(1, (DeviceTexturePtr_type)nullptr);
				pEffectFile->EndPass();
				hr = S_OK;
			}
			pEffectFile->end();
			return hr;
		}
		return S_FALSE;
	}
}

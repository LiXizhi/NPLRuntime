//-----------------------------------------------------------------------------
// Class:	CGlowEffect
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2006.5.31
// Note: based on the paper Real-Time Glow By Greg James and John O'Rorke on Gamasutra, May 26, 2004
// remark: pay attention to texel to pixel mapping. there is half texel offset issue with directX 9.
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "EffectManager.h"
#include "effect_file.h"
#include "SceneObject.h"
#include "ParaImage.h"
#include "GlowEffect.h"

using namespace ParaEngine;

CGlowEffect::CGlowEffect(void)
	: m_rhiSceneCopyRT(nullptr)
	, m_rhiGlowSourceRT(nullptr)
	, m_rhiBlurHorizRT(nullptr)
	, m_fullWidth(0)
	, m_fullHeight(0)
	, m_glowtextureWidth(0.0f)
	, m_glowtextureHeight(0.0f)
{
}

CGlowEffect::~CGlowEffect(void)
{
	InvalidateDeviceObjects();
}

HRESULT CGlowEffect::InvalidateDeviceObjects()
{
	ReleaseRHIResource(m_rhiSceneCopyRT);
	ReleaseRHIResource(m_rhiGlowSourceRT);
	ReleaseRHIResource(m_rhiBlurHorizRT);
	m_fullWidth = 0;
	m_fullHeight = 0;
	return S_OK;
}

bool CGlowEffect::EnsureRenderTargets(IRenderDevice* pRenderDevice, uint32_t fullWidth, uint32_t fullHeight)
{
	if (!pRenderDevice || fullWidth == 0 || fullHeight == 0)
		return false;

	if (m_rhiSceneCopyRT && m_rhiGlowSourceRT && m_rhiBlurHorizRT &&
		m_fullWidth == fullWidth && m_fullHeight == fullHeight)
	{
		return true;
	}

	InvalidateDeviceObjects();

	const uint32_t halfW = (fullWidth / 2) > 0 ? (fullWidth / 2) : 1;
	const uint32_t halfH = (fullHeight / 2) > 0 ? (fullHeight / 2) : 1;
	m_glowtextureWidth = (float)halfW;
	m_glowtextureHeight = (float)halfH;

	m_rhiSceneCopyRT = pRenderDevice->CreateRenderTarget(fullWidth, fullHeight, PixelFormat::A8R8G8B8, PixelFormat::Unknown);
	m_rhiGlowSourceRT = pRenderDevice->CreateRenderTarget(halfW, halfH, PixelFormat::A8R8G8B8, PixelFormat::Unknown);
	m_rhiBlurHorizRT = pRenderDevice->CreateRenderTarget(halfW, halfH, PixelFormat::A8R8G8B8, PixelFormat::Unknown);

	if (!m_rhiSceneCopyRT || !m_rhiGlowSourceRT || !m_rhiBlurHorizRT)
	{
		InvalidateDeviceObjects();
		return false;
	}

	m_fullWidth = fullWidth;
	m_fullHeight = fullHeight;
	return true;
}

HRESULT CGlowEffect::RestoreDeviceObjects()
{
	auto pRenderDevice = CGlobals::GetRenderDevice();
	if (!pRenderDevice)
		return E_FAIL;

	ParaViewport viewport = pRenderDevice->GetViewport();
	uint32_t fullW = viewport.Width > 0 ? viewport.Width : pRenderDevice->GetBackBufferWidth();
	uint32_t fullH = viewport.Height > 0 ? viewport.Height : pRenderDevice->GetBackBufferHeight();
	return EnsureRenderTargets(pRenderDevice, fullW, fullH) ? S_OK : E_FAIL;
}

HRESULT CGlowEffect::Render(float fGlowThreshold, float fGlowAmount, bool bUseAlpha)
{
	auto pRenderDevice = CGlobals::GetRenderDevice();
	if (!pRenderDevice)
		return E_FAIL;

	auto rtBinding = pRenderDevice->GetCurrentRenderTargetBinding();
	IRHIRenderTarget* activeRT = (rtBinding.count > 0) ? rtBinding.targets[0] : nullptr;
	ParaViewport oldViewport = pRenderDevice->GetViewport();

	uint32_t fullW = activeRT ? activeRT->GetWidth() : (oldViewport.Width > 0 ? oldViewport.Width : pRenderDevice->GetBackBufferWidth());
	uint32_t fullH = activeRT ? activeRT->GetHeight() : (oldViewport.Height > 0 ? oldViewport.Height : pRenderDevice->GetBackBufferHeight());
	if (!EnsureRenderTargets(pRenderDevice, fullW, fullH))
		return E_FAIL;

	// Copy active scene render target to full-resolution scene copy texture
	if (!pRenderDevice->BlitRenderTarget(activeRT, m_rhiSceneCopyRT, nullptr, nullptr, true))
	{
		OUTPUT_LOG("GlowEffect BlitRenderTarget() failed!\r\n");
		return E_FAIL;
	}

	EffectManager* pEffectManager = CGlobals::GetEffectManager();
	pEffectManager->BeginEffect(TECH_FULL_SCREEN_GLOW);
	CEffectFile* pEffectFile = pEffectManager->GetCurrentEffectFile();

	if (pEffectFile != 0 && pEffectFile->begin(true, 0))
	{
		PE_ASSERT(pEffectFile->totalPasses() == 4);

		const RHIDeviceCaps& caps = pRenderDevice->GetCaps();
		const float vTop = caps.topLeftScreenOrigin ? 0.0f : 1.0f;
		const float vBottom = caps.topLeftScreenOrigin ? 1.0f : 0.0f;
		const float halfTexelX = caps.needsHalfPixelOffset ? (0.5f / m_glowtextureWidth) : 0.0f;
		const float halfTexelY = caps.needsHalfPixelOffset ? (0.5f / m_glowtextureHeight) : 0.0f;

		mesh_vertex_plain quadVertices[4] = {
			{ Vector3(-1.0f, -1.0f, 0.0f), Vector2(0.0f + halfTexelX, vBottom + halfTexelY) },
			{ Vector3(1.0f, -1.0f, 0.0f), Vector2(1.0f + halfTexelX, vBottom + halfTexelY) },
			{ Vector3(-1.0f, 1.0f, 0.0f), Vector2(0.0f + halfTexelX, vTop + halfTexelY) },
			{ Vector3(1.0f, 1.0f, 0.0f), Vector2(1.0f + halfTexelX, vTop + halfTexelY) },
		};

		ParaViewport halfViewport;
		halfViewport.X = 0;
		halfViewport.Y = 0;
		halfViewport.Width = (uint32_t)m_glowtextureWidth;
		halfViewport.Height = (uint32_t)m_glowtextureHeight;
		halfViewport.MinZ = 0.0f;
		halfViewport.MaxZ = 1.0f;

		pRenderDevice->PushRenderTarget(m_rhiGlowSourceRT);

		bUseAlpha = true;
		if (bUseAlpha)
		{
			// Step 1: Extract bright glow source at half resolution
			pRenderDevice->BeginRenderTarget(m_rhiGlowSourceRT);
			pRenderDevice->SetViewport(halfViewport);

			if (pEffectFile->BeginPass(0))
			{
				pEffectFile->setTexture(0, m_rhiSceneCopyRT->GetColorTexture(0));
				pEffectFile->SetFloat("glowThreshold", fGlowThreshold);
				pEffectFile->CommitChanges();

				pRenderDevice->DrawPrimitiveUP(EPrimitiveType::TRIANGLESTRIP, 2, quadVertices, sizeof(mesh_vertex_plain));
				pEffectFile->EndPass();
			}
			pEffectFile->setTexture(0, (DeviceTexturePtr_type)nullptr);
			pRenderDevice->EndRenderTarget();
		}

		// Step 2: Horizontal blur into m_rhiBlurHorizRT
		pRenderDevice->SetRenderTarget(0, m_rhiBlurHorizRT);
		pRenderDevice->BeginRenderTarget(m_rhiBlurHorizRT);
		pRenderDevice->SetViewport(halfViewport);

		if (pEffectFile->BeginPass(1))
		{
			SetBlurEffectParameters(1.0f / m_glowtextureWidth, 0.0f, fGlowAmount, pEffectFile);
			pEffectFile->setTexture(0, m_rhiGlowSourceRT->GetColorTexture(0));
			pEffectFile->CommitChanges();

			pRenderDevice->DrawPrimitiveUP(EPrimitiveType::TRIANGLESTRIP, 2, quadVertices, sizeof(mesh_vertex_plain));
			pEffectFile->EndPass();
		}
		pEffectFile->setTexture(0, (DeviceTexturePtr_type)nullptr);
		pRenderDevice->EndRenderTarget();

		// Step 3: Vertical blur back into m_rhiGlowSourceRT
		pRenderDevice->SetRenderTarget(0, m_rhiGlowSourceRT);
		pRenderDevice->BeginRenderTarget(m_rhiGlowSourceRT);
		pRenderDevice->SetViewport(halfViewport);

		if (pEffectFile->BeginPass(1))
		{
			SetBlurEffectParameters(0.0f, 1.0f / m_glowtextureHeight, fGlowAmount, pEffectFile);
			pEffectFile->setTexture(0, m_rhiBlurHorizRT->GetColorTexture(0));
			pEffectFile->CommitChanges();

			pRenderDevice->DrawPrimitiveUP(EPrimitiveType::TRIANGLESTRIP, 2, quadVertices, sizeof(mesh_vertex_plain));
			pEffectFile->EndPass();
		}
		pEffectFile->setTexture(0, (DeviceTexturePtr_type)nullptr);
		pRenderDevice->EndRenderTarget();

		// Restore original render target and viewport for final composite pass
		pRenderDevice->PopRenderTarget();
		pRenderDevice->SetViewport(oldViewport);

		if (caps.needsHalfPixelOffset)
		{
			for (int i = 0; i < 4; ++i)
			{
				quadVertices[i].uv.x -= halfTexelX * 0.5f;
				quadVertices[i].uv.y -= halfTexelY * 0.5f;
			}
		}

		// Step 4: Composite blurred glow + scene copy onto the active render target
		if (pEffectFile->BeginPass(3))
		{
			pEffectFile->setTexture(0, m_rhiSceneCopyRT->GetColorTexture(0));
			pEffectFile->setTexture(1, m_rhiGlowSourceRT->GetColorTexture(0));

			Vector4 glowParam = pEffectManager->GetGlowness();
			pEffectFile->SetVector4("glowParams", glowParam);
			pEffectFile->CommitChanges();

			pRenderDevice->DrawPrimitiveUP(EPrimitiveType::TRIANGLESTRIP, 2, quadVertices, sizeof(mesh_vertex_plain));

			pEffectFile->EndPass();
			pEffectFile->setTexture(0, (DeviceTexturePtr_type)nullptr);
			pEffectFile->setTexture(1, (DeviceTexturePtr_type)nullptr);
		}

		pEffectFile->end();
	}
	else
	{
		CGlobals::GetScene()->EnableFullScreenGlow(false);
	}
	return S_OK;
}

void CGlowEffect::SetBlurEffectParameters(float dx, float dy, float blurAmount, CEffectFile* pEffect)
{
	const int sampleCount = 9;
	float sampleWeights[sampleCount];
	float sampleOffset[sampleCount * 2];

	if (blurAmount <= 0.0001f)
		blurAmount = 0.0001f;

	sampleWeights[0] = ComputeGaussian(0.0f, blurAmount);
	sampleOffset[0] = 0.0f;
	sampleOffset[1] = 0.0f;

	float totalWeight = sampleWeights[0];

	for (int i = 0; i < sampleCount / 2; i++)
	{
		float weight = ComputeGaussian((float)(i + 1), blurAmount);
		sampleWeights[i * 2 + 1] = weight;
		sampleWeights[i * 2 + 2] = weight;

		totalWeight += weight * 2.0f;

		float offset = i * 2.0f + 1.5f;
		float offsetX = dx * offset;
		float offsetY = dy * offset;

		int index = (i * 2) * 2 + 2;
		sampleOffset[index] = offsetX;
		sampleOffset[index + 1] = offsetY;
		sampleOffset[index + 2] = -offsetX;
		sampleOffset[index + 3] = -offsetY;
	}

	for (int i = 0; i < sampleCount; i++)
	{
		sampleWeights[i] /= totalWeight;
	}

	pEffect->SetRawValue("sampleWeight", sampleWeights, 0, sizeof(sampleWeights));
	pEffect->SetRawValue("sampleOffset", sampleOffset, 0, sizeof(sampleOffset));
}

float CGlowEffect::ComputeGaussian(float n, float blurAmount)
{
	return (1.0f / sqrtf(2.0f * 3.141592f * blurAmount)) * expf(-(n * n) / (2.0f * blurAmount * blurAmount));
}

bool CGlowEffect::SaveRenderTargetToFile(IRenderDevice* pRenderDevice, IRHIRenderTarget* pRT, const std::string& filename)
{
	if (!pRenderDevice || !pRT || !pRT->GetColorTexture(0))
		return false;

	const uint32_t rtWidth = pRT->GetWidth();
	const uint32_t rtHeight = pRT->GetHeight();
	if (rtWidth == 0 || rtHeight == 0)
		return false;

	std::vector<uint8_t> pixels(rtWidth * rtHeight * 4);
	if (!pRenderDevice->ReadTexturePixels(pRT->GetColorTexture(0), pixels.data()))
		return false;

	if (!pRenderDevice->GetCaps().topLeftScreenOrigin)
	{
		const size_t rowBytes = rtWidth * 4;
		std::vector<uint8_t> rowBuf(rowBytes);
		for (uint32_t y = 0; y < rtHeight / 2; ++y)
		{
			uint8_t* top = pixels.data() + y * rowBytes;
			uint8_t* bot = pixels.data() + (rtHeight - 1 - y) * rowBytes;
			memcpy(rowBuf.data(), top, rowBytes);
			memcpy(top, bot, rowBytes);
			memcpy(bot, rowBuf.data(), rowBytes);
		}
	}

	ParaImage img;
	if (!img.initWithRawData(pixels.data(), pixels.size(), (int)rtWidth, (int)rtHeight, 32, false))
		return false;

	CParaFile::CreateDirectory(filename.c_str());
	return img.saveToFile(filename, true);
}

void CGlowEffect::SaveGlowTexturesToFile(const std::string& filename)
{
	auto pRenderDevice = CGlobals::GetRenderDevice();
	SaveRenderTargetToFile(pRenderDevice, m_rhiGlowSourceRT, filename);
	SaveRenderTargetToFile(pRenderDevice, m_rhiSceneCopyRT, filename + "halfsize.jpg");
	SaveRenderTargetToFile(pRenderDevice, m_rhiBlurHorizRT, filename + "horiz.jpg");
}

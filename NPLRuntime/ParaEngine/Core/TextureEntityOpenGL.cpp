//-----------------------------------------------------------------------------
// Class:	TextureEntityOpengGL
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2014.8
// Desc: currently all opengl stuffs are implemented using cocos2dx objects. 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"

#ifndef USE_NULL_RENDERER
#include "ImageEntity.h"
#include "TextureEntity.h"
#include "ViewportManager.h"
#include "OpenGLWrapper/GLImage.h"
#include "StringHelper.h"
#include "os_calls.h"

#include "ParaScriptingIO.h"

using namespace ParaEngine;

namespace ParaEngine
{
	extern int64_t globalTime;
}

TextureEntityOpenGL::TextureEntityOpenGL(const AssetKey& key)
	:TextureEntity(key), m_texture(nullptr), m_nSamplerState(0)
{
}

TextureEntityOpenGL::TextureEntityOpenGL()
	: TextureEntity(), m_texture(nullptr), m_nSamplerState(0)
{
}

ParaEngine::TextureEntityOpenGL::TextureEntityOpenGL(GLTexture2D* texture)
	: TextureEntity(), m_texture(nullptr), m_nSamplerState(0)
{
	SetInnerTexture(texture);
}

TextureEntityOpenGL::~TextureEntityOpenGL()
{
	DeleteDeviceObjects();
}

HRESULT ParaEngine::TextureEntityOpenGL::RendererRecreated()
{
    m_bIsInitialized = false;

    if (SurfaceType == TextureSequence) {
        if (m_pTextureSequence != 0) {
            AnimatedTextureInfo *pInfo = GetAnimatedTextureInfo();
            int nTotalTextureSequence = -1;

            if (pInfo != 0)
                nTotalTextureSequence = pInfo->m_nFrameCount;

            if (nTotalTextureSequence > 0) {
                for (int i = 0; i < nTotalTextureSequence; ++i) {
                    if (m_pTextureSequence[i])
                        m_pTextureSequence[i]->RendererRecreated();
                }
            }
        }
    } else if (m_texture) {
        m_texture->RendererRecreated();
    }

    return S_OK;
}

GLTexture2D* ParaEngine::TextureEntityOpenGL::CreateTextureFromFile_Serial(const std::string& sFileName)
{
	GLTexture2D * texture = nullptr;
	CParaFile file;
	if (file.OpenFile(sFileName.c_str(), true))
	{
		LoadFromMemory(file.getBuffer(), file.getSize(), 0, PixelFormat::Unknown, (void**)(&texture));
		if (texture){
			texture->AddToAutoReleasePool();
		}
	}
	else
	{
		SetState(AssetEntity::ASSET_STATE_FAILED_TO_LOAD);
		OUTPUT_LOG("warn: texture file not found: %s\n", sFileName.c_str());
	}
	return texture;
}

HRESULT ParaEngine::TextureEntityOpenGL::InitDeviceObjects()
{
	if (m_bIsInitialized)
		return S_OK;
	if (SurfaceType == TextureEntityOpenGL::RenderTarget || SurfaceType == TextureEntityOpenGL::DEPTHSTENCIL)
	{
		// render target is only created in RestoreDeviceObjects();
		return S_OK;
	}
	m_bIsInitialized = true;

	const std::string& sTextureFileName = GetLocalFileName();

	// if sTextureFileName is empty or texture is locked, this is most likely a remote texture that is just being downloaded, so it has empty local file name. 
	if (sTextureFileName.empty() || IsLocked())
	{
		return S_OK;
	}
	if (SurfaceType == StaticTexture)
	{
		SAFE_RELEASE(m_texture);
		if(!IsAsyncLoad())
			SetInnerTexture(CreateTextureFromFile_Serial(sTextureFileName));
		else
			return CreateTextureFromFile_Async(NULL, NULL, NULL);
	}
	else if (SurfaceType == TextureSequence)
	{
		int nSize = (int)sTextureFileName.size();
		if (nSize > 8)
		{
			AnimatedTextureInfo* pInfo = GetAnimatedTextureInfo();
			int nTotalTextureSequence = -1;
			if (pInfo != 0)
				nTotalTextureSequence = pInfo->m_nFrameCount;

			if (nTotalTextureSequence > 0)
			{
				if (m_pTextureSequence)
				{
					for (int i = 0; i < nTotalTextureSequence; ++i)
					{
						SAFE_RELEASE(m_pTextureSequence[i]);
					}
				}
				else
				{
					m_pTextureSequence = new GLTexture2D*[nTotalTextureSequence];
					memset(m_pTextureSequence, 0, sizeof(GLTexture2D*)*nTotalTextureSequence);
				}

				// if there are texture sequence, export all bitmaps in the texture sequence. 
				string sTemp = sTextureFileName;
				int nOffset = nSize - 7;
				for (int i = 1; i <= nTotalTextureSequence; ++i)
				{
					int digit, tmp;
					tmp = i;
					digit = (int)(tmp / 100);
					sTemp[nOffset + 0] = ((char)('0' + digit));
					tmp = tmp - 100 * digit;
					digit = (int)(tmp / 10);
					sTemp[nOffset + 1] = ((char)('0' + digit));
					tmp = tmp - 10 * digit;
					digit = tmp;
					sTemp[nOffset + 2] = ((char)('0' + digit));

					if (m_pTextureSequence[i - 1] == 0)
					{
						if (!IsAsyncLoad())
						{
							auto texture = CreateTextureFromFile_Serial(sTemp);
							if(texture)
								texture->addref();
							m_pTextureSequence[i - 1] = texture;
						}
						else
						{
							if (FAILED(CreateTextureFromFile_Async(NULL, NULL, sTemp.c_str(), (void**)&(m_pTextureSequence[i - 1]))))
							{
								OUTPUT_LOG("failed creating animated texture for %s", sTemp.c_str());
							}
						}
					}
				}
			}
		}
	}
	else
	{
		OUTPUT_LOG("warn: unsupported texture file format\n");
	}

	return S_OK;
}


void ParaEngine::TextureEntityOpenGL::SetInnerTexture(GLTexture2D* texture)
{
	if (SurfaceType == StaticTexture)
	{
		SAFE_RELEASE(m_texture);
		m_bIsInitialized = true;
		if (texture)
		{
			m_texture = texture;
			if(m_texture)
				m_texture->addref();
		}
	}
}

void ParaEngine::TextureEntityOpenGL::AttachRHITexture(IRHITexture* rhiTex, int width, int height)
{
	if (rhiTex)
	{
		if (!m_texture)
			m_texture = new (std::nothrow) GLTexture2D();
		if (m_texture)
			m_texture->initWithRHITexture(rhiTex, false);
		m_bIsInitialized = true;
		m_bIsValid = true;
		SetTextureInfo(TextureInfo(width, height, TextureInfo::FMT_A8R8G8B8, TextureInfo::TYPE_UNKNOWN));
	}
	else
	{
		if (m_texture)
		{
			m_texture->detachRHITexture();
			SAFE_RELEASE(m_texture);
		}
		m_bIsInitialized = false;
	}
}

void ParaEngine::TextureEntityOpenGL::AttachRHIRenderTarget(IRHIRenderTarget* pRT, bool bOwnRT)
{
	if (m_bOwnsRHIRenderTarget && m_rhiRenderTarget && m_rhiRenderTarget != pRT)
	{
		ReleaseRHIResource(m_rhiRenderTarget);
	}
	m_rhiRenderTarget = pRT;
	m_bOwnsRHIRenderTarget = bOwnRT;
	if (pRT && pRT->GetColorTexture(0))
	{
		AttachRHITexture(pRT->GetColorTexture(0), (int)pRT->GetWidth(), (int)pRT->GetHeight());
	}
	else if (SurfaceType == TextureSurface)
	{
		if (m_texture)
		{
			m_texture->detachRHITexture();
			SAFE_RELEASE(m_texture);
		}
		m_bIsInitialized = true;
		m_bIsValid = true;
	}
}

void ParaEngine::TextureEntityOpenGL::EnsureScreenSizeRenderTargetUpToDate(bool allowWhileActiveRT)
{
	if (SurfaceType != RenderTarget || !m_bIsInitialized || !m_bOwnsRHIRenderTarget || !m_rhiRenderTarget)
		return;
	if (!m_pTextureInfo || m_pTextureInfo->m_subtype != TextureInfo::TYPE_SCREENSIZE)
		return;
	auto* pDev = CGlobals::GetRenderDevice();
	if (!pDev)
		return;
	const uint32_t bbW = pDev->GetBackBufferWidth();
	const uint32_t bbH = pDev->GetBackBufferHeight();
	if (bbW == 0 || bbH == 0)
		return;
	if (m_rhiRenderTarget->GetWidth() == bbW && m_rhiRenderTarget->GetHeight() == bbH)
		return;

	if (!allowWhileActiveRT)
	{
		const auto& binding = pDev->GetCurrentRenderTargetBinding();
		if (binding.depthStencilOverride == m_rhiRenderTarget)
			return;
		for (uint32_t i = 0; i < binding.count && i < 4u; ++i)
		{
			if (binding.targets[i] == m_rhiRenderTarget)
				return;
		}
	}

	InvalidateDeviceObjects();
	RestoreDeviceObjects();
}

IRHIRenderTarget* ParaEngine::TextureEntityOpenGL::GetRHIRenderTarget()
{
	if (SurfaceType == RenderTarget)
	{
		if (!m_bIsInitialized)
			LoadAsset();
		else
			EnsureScreenSizeRenderTargetUpToDate(true);
	}
	return m_rhiRenderTarget;
}

bool ParaEngine::TextureEntityOpenGL::SetRenderTarget(int nIndex)
{
	auto* pDev = CGlobals::GetRenderDevice();
	if (!pDev)
		return false;
	bool res = false;
	if (SurfaceType == RenderTarget)
	{
		LoadAsset();
		EnsureScreenSizeRenderTargetUpToDate(true);
		if (!m_rhiRenderTarget)
			return false;
		res = pDev->SetRenderTarget((uint32_t)nIndex, m_rhiRenderTarget);
	}
	else if (SurfaceType == TextureSurface)
	{
		res = pDev->SetRenderTarget((uint32_t)nIndex, m_rhiRenderTarget);
	}

	if (res && nIndex == 0)
	{
		if (!m_rhiRenderTarget || (m_rhiRenderTarget->GetWidth() == pDev->GetBackBufferWidth() && m_rhiRenderTarget->GetHeight() == pDev->GetBackBufferHeight()))
		{
			if (CGlobals::GetViewportManager() && CGlobals::GetViewportManager()->GetActiveViewPort())
				CGlobals::GetViewportManager()->GetActiveViewPort()->ApplyViewport();
		}
		else
		{
			ParaViewport vp;
			vp.X = 0;
			vp.Y = 0;
			vp.Width = m_rhiRenderTarget->GetWidth();
			vp.Height = m_rhiRenderTarget->GetHeight();
			vp.MinZ = 0.0f;
			vp.MaxZ = 1.0f;
			pDev->SetViewport(vp);
		}
	}
	return res;
}

HRESULT ParaEngine::TextureEntityOpenGL::RestoreDeviceObjects()
{
	if (m_bIsInitialized)
		return S_OK;

	if (SurfaceType == RenderTarget || SurfaceType == DEPTHSTENCIL)
	{
		m_bIsInitialized = true;

		IRenderDevice* pDev = CGlobals::GetRenderDevice();
		DWORD defaultW = (pDev && pDev->GetBackBufferWidth() > 0) ? pDev->GetBackBufferWidth() : 256;
		DWORD defaultH = (pDev && pDev->GetBackBufferHeight() > 0) ? pDev->GetBackBufferHeight() : 256;
		DWORD width = defaultW, height = defaultH;
		PixelFormat format = (SurfaceType == DEPTHSTENCIL) ? PixelFormat::D24S8 : PixelFormat::A8R8G8B8;

		const bool isScreenSize = (m_pTextureInfo == nullptr
			|| m_pTextureInfo->m_subtype == TextureInfo::TYPE_SCREENSIZE
			|| m_pTextureInfo->m_width <= 0
			|| m_pTextureInfo->m_height <= 0);

		if (m_pTextureInfo != nullptr)
		{
			if (!isScreenSize)
			{
				width = static_cast<DWORD>(m_pTextureInfo->m_width);
				height = static_cast<DWORD>(m_pTextureInfo->m_height);
			}
			if (SurfaceType == RenderTarget && m_pTextureInfo->m_format == TextureInfo::FMT_X8R8G8B8)
			{
				format = PixelFormat::X8R8G8B8;
			}
		}

		if (SurfaceType == RenderTarget && pDev)
		{
			const std::string& sKey = GetKey();
			if (sKey.find("_R32F") != std::string::npos)
			{
				if (pDev->GetCaps().supportsFloatRenderTargets)
					format = PixelFormat::R32F;
			}
			if (sKey.find("_HDR") != std::string::npos)
			{
				if (pDev->GetCaps().supportsFloatRenderTargets)
					format = PixelFormat::A16B16G16R16F;
			}
		}

		if (m_bOwnsRHIRenderTarget && m_rhiRenderTarget)
		{
			ReleaseRHIResource(m_rhiRenderTarget);
		}
		m_rhiRenderTarget = nullptr;
		m_bOwnsRHIRenderTarget = false;

		SAFE_RELEASE(m_texture);
		m_texture = new (std::nothrow) GLTexture2D();
		if (m_texture)
		{
			if (SurfaceType == RenderTarget && pDev)
			{
				const ERHIBackend backend = pDev->GetBackendType();
				const bool useSharedDefaultDepth = (backend == ERHIBackend::D3D11 || backend == ERHIBackend::D3D12 || backend == ERHIBackend::Vulkan);
				const PixelFormat rtDepthFormat = useSharedDefaultDepth ? PixelFormat::Unknown : PixelFormat::D24S8;
				m_rhiRenderTarget = pDev->CreateRenderTarget(width, height, format, rtDepthFormat);
				if (m_rhiRenderTarget && m_rhiRenderTarget->GetColorTexture(0))
				{
					m_bOwnsRHIRenderTarget = true;
					m_texture->initWithRHITexture(m_rhiRenderTarget->GetColorTexture(0), false);
					const auto texFmt = (format == PixelFormat::X8R8G8B8) ? TextureInfo::FMT_X8R8G8B8 : TextureInfo::FMT_A8R8G8B8;
					const auto subType = isScreenSize ? TextureInfo::TYPE_SCREENSIZE : TextureInfo::TYPE_UNKNOWN;
					if (!m_pTextureInfo)
					{
						m_pTextureInfo = new TextureInfo((int)width, (int)height, texFmt, subType);
					}
					else
					{
						m_pTextureInfo->m_width = (int)width;
						m_pTextureInfo->m_height = (int)height;
						m_pTextureInfo->m_format = texFmt;
						m_pTextureInfo->m_subtype = subType;
					}
				}
			}

			if (!m_rhiRenderTarget)
			{
				ERHITextureUsage usage = ERHITextureUsage::ShaderResource |
					((SurfaceType == DEPTHSTENCIL) ? ERHITextureUsage::DepthStencil : ERHITextureUsage::RenderTarget);
				IRHITexture* rhiTex = pDev ? pDev->CreateTexture(width, height, format, 1, usage, nullptr, 0) : nullptr;
				if (rhiTex)
				{
					m_texture->initWithRHITexture(rhiTex, true);
				}
				else if (SurfaceType == RenderTarget)
				{
					auto dataLen = static_cast<size_t>(width) * static_cast<size_t>(height) * 4u;
					void* data = malloc(dataLen);
					if (data)
					{
						memset(data, 0, dataLen);
						m_texture->initWithData(data, dataLen, PixelFormat::A8R8G8B8, static_cast<int>(width), static_cast<int>(height), Size((float)width, (float)height));
						free(data);
					}
				}
			}

#ifdef PARA_ENABLE_OPENGL
			if (m_texture->getName() != 0 && pDev && pDev->GetBackendType() == ERHIBackend::OpenGL)
			{
				GL::bindTexture2D(m_texture->getName());
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
					(m_texture->getPixelsWide() == GLTexture2D::ccNextPOT(m_texture->getPixelsWide())) ? GL_REPEAT : GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
					(m_texture->getPixelsHigh() == GLTexture2D::ccNextPOT(m_texture->getPixelsHigh())) ? GL_REPEAT : GL_CLAMP_TO_EDGE);
			}
#endif
		}
	}
	return S_OK;
}

HRESULT ParaEngine::TextureEntityOpenGL::InvalidateDeviceObjects()
{
	if (SurfaceType == RenderTarget || SurfaceType == DEPTHSTENCIL)
	{
		m_bIsInitialized = false;
		if (m_texture)
		{
			m_texture->detachRHITexture();
			SAFE_RELEASE(m_texture);
		}
		if (m_bOwnsRHIRenderTarget && m_rhiRenderTarget)
		{
			ReleaseRHIResource(m_rhiRenderTarget);
		}
		m_rhiRenderTarget = nullptr;
		m_bOwnsRHIRenderTarget = false;
	}
	return S_OK;
}

HRESULT ParaEngine::TextureEntityOpenGL::DeleteDeviceObjects()
{
	m_bIsInitialized = false;
	m_dynamicPixels.clear();
	if (m_bOwnsRHIRenderTarget && m_rhiRenderTarget)
	{
		if (m_texture)
			m_texture->detachRHITexture();
		ReleaseRHIResource(m_rhiRenderTarget);
	}
	m_rhiRenderTarget = nullptr;
	m_bOwnsRHIRenderTarget = false;
	switch (SurfaceType)
	{
	case TextureSequence:
	{
		if (m_pTextureSequence != 0)
		{
			AnimatedTextureInfo* pInfo = GetAnimatedTextureInfo();
			int nTotalTextureSequence = -1;
			if (pInfo != 0)
				nTotalTextureSequence = pInfo->m_nFrameCount;
			if (nTotalTextureSequence > 0)
			{
				for (int i = 0; i < nTotalTextureSequence; ++i)
				{
					SAFE_RELEASE(m_pTextureSequence[i]);
				}
				SAFE_DELETE_ARRAY(m_pTextureSequence);
			}
		}
		break;
	}
	default:
	{
		if (m_texture)
		{
			// m_texture->releaseGLTexture();
			SAFE_RELEASE(m_texture);
		}
		break;
	}
	}
	return S_OK;
}

DeviceTexturePtr_type ParaEngine::TextureEntityOpenGL::GetTexture()
{
	++m_nHitCount;
	if (IsLocked())
	{
		return 0;
	}
	LoadAsset();
	if (SurfaceType == RenderTarget)
	{
		EnsureScreenSizeRenderTargetUpToDate(false);
	}
	switch (SurfaceType)
	{
		case TextureSequence:
		{
			AnimatedTextureInfo* pInfo = GetAnimatedTextureInfo();
			if (pInfo != 0 && m_pTextureSequence != 0 && pInfo->m_nCurrentFrameIndex < pInfo->m_nFrameCount)
			{
				if (pInfo->m_bAutoAnimation)
				{
					if (pInfo->m_fFPS >= 0)
					{
						pInfo->m_nCurrentFrameIndex = ((int64_t)(globalTime * pInfo->m_fFPS / 1000)) % pInfo->m_nFrameCount;
					}
					else
					{
						pInfo->m_nCurrentFrameIndex = pInfo->m_nFrameCount - 1 - ((int64_t)(-globalTime * pInfo->m_fFPS / 1000)) % pInfo->m_nFrameCount;
					}
				}
				auto tex = m_pTextureSequence[pInfo->m_nCurrentFrameIndex];
				return tex;
			}
			break;
		}
		default:
		{
			return m_texture;
			break;
		}
	}
	return 0;
}

int32 ParaEngine::TextureEntityOpenGL::GetWidth()
{
	if (SurfaceType == TextureSequence)
	{
		if (m_pTextureSequence)
			return m_pTextureSequence[0] ? m_pTextureSequence[0]->getPixelsWide() : -1;
	}
	else
		return m_texture ? m_texture->getPixelsWide() : -1;
	return -1;
}

int32 ParaEngine::TextureEntityOpenGL::GetHeight()
{
	if (SurfaceType == TextureSequence)
	{
		if (m_pTextureSequence)
			return m_pTextureSequence[0] ? m_pTextureSequence[0]->getPixelsHigh() : -1;
	}
	else
		return m_texture ? m_texture->getPixelsHigh() : -1;
	return -1;
}

const TextureEntityOpenGL::TextureInfo* ParaEngine::TextureEntityOpenGL::GetTextureInfo()
{
	if (SurfaceType == RenderTarget || SurfaceType == DEPTHSTENCIL)
	{
		if (!m_bIsInitialized)
			LoadAsset();
		else
			EnsureScreenSizeRenderTargetUpToDate(false);
	}
	else if (SurfaceType == StaticTexture)
	{
		if (!m_pTextureInfo)
		{
			if (m_texture)
			{
				//int nWidth = Math::NextPowerOf2(m_texture->getPixelsWide());
				//int nHeight = Math::NextPowerOf2(m_texture->getPixelsHigh());
				int nWidth = m_texture->getPixelsWide();
				int nHeight = m_texture->getPixelsHigh();
				/*if (nWidth > nHeight)
					nHeight = nWidth;
				else if (nWidth < nHeight)
					nWidth = nHeight;*/
				m_pTextureInfo = new TextureInfo(nWidth, nHeight, TextureEntity::TextureInfo::FMT_UNKNOWN, TextureEntity::TextureInfo::TYPE_UNKNOWN);
			}
		}
	}
	else if (SurfaceType == TextureSequence)
	{
		GetAnimatedTextureInfo();
		if ((m_pAnimatedTextureInfo && m_pAnimatedTextureInfo->m_width == 0))
		{
			if (m_pTextureSequence && m_pAnimatedTextureInfo->m_nFrameCount > 0 && m_pTextureSequence[0])
			{
				m_pAnimatedTextureInfo->m_width = m_pTextureSequence[0]->getPixelsWide();
				m_pAnimatedTextureInfo->m_height = m_pTextureSequence[0]->getPixelsHigh();
			}
			else
				return &(TextureInfo::Empty);
		}
	}
	return m_pTextureInfo ? m_pTextureInfo : (&(TextureInfo::Empty));
}

void ParaEngine::TextureEntityOpenGL::SetSamplerStateBlocky(bool bIsBlocky)
{
	m_nSamplerState = bIsBlocky ? 1 : 0;
}

bool ParaEngine::TextureEntityOpenGL::IsSamplerStateBlocky()
{
	return m_nSamplerState == 1;
}

TextureEntity* ParaEngine::TextureEntityOpenGL::CreateTexture(const char* pFileName, uint32 nMipLevels /*= 0*/, EPoolType dwCreatePool /*= D3DPOOL_MANAGED*/)
{
	TextureEntityOpenGL* pTextureEntity = new TextureEntityOpenGL(AssetKey(pFileName));
	if (pTextureEntity)
	{
		auto pTexture = pTextureEntity->CreateTextureFromFile_Serial(pFileName);
		pTextureEntity->SetInnerTexture(pTexture);
		pTextureEntity->AddToAutoReleasePool();
		return pTextureEntity;
	}
	return NULL;
}

TextureEntity* ParaEngine::TextureEntityOpenGL::CreateTexture(const uint8 * pTexels, int width, int height, int rowLength, int bytesPerPixel, uint32 nMipLevels /*= 0*/, EPoolType dwCreatePool /*= D3DPOOL_MANAGED*/, DWORD nFormat /*= 0*/)
{
	if (!pTexels)
		return NULL;
	GLTexture2D * texture = new GLTexture2D();
	if (texture)
	{
		int dataLen = width * height * bytesPerPixel;
		if (bytesPerPixel == 4)
		{
			if (!texture->initWithData(pTexels, dataLen, PixelFormat::A8R8G8B8, width, height, Size((float)width, (float)height)))
			{
				SAFE_DELETE(texture);
			}
		}
		else if (bytesPerPixel == 3)
		{
			dataLen = width * height * 4;
			uint8 *pp = new uint8[dataLen];
			int index = 0, x = 0, y = 0;
			for (y = 0; y < height; y++)
			{
				for (x = 0; x < width; x++)
				{
					int n = (y * 3 * width) + 3 * x;
					pp[index++] = pTexels[n];
					pp[index++] = pTexels[n + 1];
					pp[index++] = pTexels[n + 2];
					pp[index++] = 0xff;
				}
			}
			if (!texture->initWithData(pp, dataLen, PixelFormat::A8R8G8B8, width, height, Size((float)width, (float)height)))
			{
				SAFE_DELETE(texture);
			}
			delete[] pp;
		}
		else if (bytesPerPixel == 1)
		{
			/** -1 not determined, 0 do not support A8, 1 support A8. we will only try it once. */
			static int nSupportA8PixelFormat = -1;
			
			if (nSupportA8PixelFormat == 0 || !texture->initWithData(pTexels, dataLen, PixelFormat::A8, width, height, Size((float)width, (float)height)))
			{
				if (nSupportA8PixelFormat == -1)
					nSupportA8PixelFormat = 0;
				int nSize = width*height;
				DWORD* pData = new DWORD[nSize];
				for (int x = 0; x < nSize; ++x)
				{
					pData[x] = (pTexels[x]) << 24;
				}
				if (!texture->initWithData(pData, width*height * 4, PixelFormat::A8R8G8B8, width, height, Size((float)width, (float)height)))
				{
					SAFE_DELETE(texture);
				}
				delete[] pData;
			}
			else if (nSupportA8PixelFormat == -1)
				nSupportA8PixelFormat = 1;
		}
	}
	
	if (texture)
	{
		TextureEntityOpenGL* pTextureEntity = new TextureEntityOpenGL();
		if (pTextureEntity)
		{
			pTextureEntity->SetInnerTexture(texture);
			pTextureEntity->AddToAutoReleasePool();
			return pTextureEntity;
		}
		SAFE_RELEASE(texture);
	}
	return NULL;
}

TextureEntity* TextureEntityOpenGL::LoadUint8Buffer(const uint8 * pTexels, int width, int height, int rowLength, int bytesPerPixel, uint32 nMipLevels /*= 0*/, D3DPOOL dwCreatePool /*= D3DPOOL_MANAGED*/, DWORD nFormat /*= 0*/)
{
	if (!pTexels)
		return NULL;

	if (SurfaceType != DynamicTexture)
	{
		UnloadAsset();
		SurfaceType = DynamicTexture;
	}
	if (m_pTextureInfo && (m_pTextureInfo->m_width != width || m_pTextureInfo->m_height != height))
	{
		UnloadAsset();
		m_pTextureInfo->m_width = -1;
		m_pTextureInfo->m_height = -1;
	}

	int dataLen = width * height * bytesPerPixel;
	if (bytesPerPixel == 4)
	{
		m_dynamicPixels.assign(pTexels, pTexels + dataLen);
		if (!m_texture)
		{
			// for now, we do not generate mipmap.
			m_texture = new (std::nothrow) GLTexture2D();
			m_texture->initWithData(pTexels, dataLen, PixelFormat::A8R8G8B8, width, height, Size((float)width, (float)height));
		}
		else
		{
			m_texture->updateWithData(pTexels, 0, 0, width, height);
		}
	}
	else if (bytesPerPixel == 3)
	{
		dataLen = width * height * 4;
		uint8 *pp = new uint8[dataLen];
		int index = 0, x = 0, y = 0;
		for (y = 0; y < height; y++)
		{
			for (x = 0; x < width; x++)
			{
				int n = (y * 3 * width) + 3 * x;
				pp[index++] = pTexels[n];
				pp[index++] = pTexels[n + 1];
				pp[index++] = pTexels[n + 2];
				pp[index++] = 0xff;
			}
		}
		m_dynamicPixels.assign(pp, pp + dataLen);
		pTexels = pp;
		if (!m_texture)
		{
			// for now, we do not generate mipmap.
			m_texture = new (std::nothrow) GLTexture2D();
			m_texture->initWithData(pTexels, dataLen, PixelFormat::A8R8G8B8, width, height, Size((float)width, (float)height));
		}
		else
		{
			m_texture->updateWithData(pTexels, 0, 0, width, height);
		}
		delete[] pp;
	}
	else if (bytesPerPixel == 1)
	{
		/** -1 not determined, 0 do not support A8, 1 support A8. we will only try it once. */
		static int nSupportA8PixelFormat = -1;
		if (!m_texture)
		{
			// for now, we do not generate mipmap.
			m_texture = new (std::nothrow) GLTexture2D();
		}

		int nSize = width * height;
		m_dynamicPixels.resize(nSize * 4);
		for (int x = 0; x < nSize; ++x)
		{
			m_dynamicPixels[x * 4 + 0] = 255;
			m_dynamicPixels[x * 4 + 1] = 255;
			m_dynamicPixels[x * 4 + 2] = 255;
			m_dynamicPixels[x * 4 + 3] = pTexels[x];
		}

		if (nSupportA8PixelFormat == 0 || !m_texture->initWithData(pTexels, dataLen, PixelFormat::A8, width, height, Size((float)width, (float)height)))
		{
			if (nSupportA8PixelFormat == -1)
				nSupportA8PixelFormat = 0;
			DWORD* pData = new DWORD[nSize];
			for (int x = 0; x < nSize; ++x)
			{
				pData[x] = (pTexels[x]) << 24;
			}
			if (!m_texture->initWithData(pData, width*height * 4, PixelFormat::A8R8G8B8, width, height, Size((float)width, (float)height)))
			{
				SAFE_DELETE(m_texture);
			}
			delete[] pData;
		}
		else if (nSupportA8PixelFormat == -1)
			nSupportA8PixelFormat = 1;
	}
	
	m_bIsInitialized = true;
	m_bIsValid = true;
	if (m_pTextureInfo == NULL)
		m_pTextureInfo = new TextureInfo();
	m_pTextureInfo->m_width = width;
	m_pTextureInfo->m_height = height;

	return this;
}

void ParaEngine::TextureEntityOpenGL::LoadImage(char *sBufMemFile, int sizeBuf, int &width, int &height, unsigned char ** ppBuffer, bool bAlpha)
{
	width = 0;
	height = 0;
	if (ppBuffer)
		*ppBuffer = nullptr;
	if (!sBufMemFile || sizeBuf <= 0 || !ppBuffer)
		return;

	unsigned char* rgba = nullptr;
	int bpp = 0;
	if (!LoadImageOfFormat("temp.png", sBufMemFile, sizeBuf, width, height, &rgba, &bpp, -1) || !rgba)
		return;

	if (bAlpha)
	{
		*ppBuffer = rgba;
	}
	else
	{
		int pixelCount = width * height;
		unsigned char* rgb = new unsigned char[pixelCount * 3];
		for (int i = 0; i < pixelCount; ++i)
		{
			rgb[i * 3 + 0] = rgba[i * 4 + 0];
			rgb[i * 3 + 1] = rgba[i * 4 + 1];
			rgb[i * 3 + 2] = rgba[i * 4 + 2];
		}
		delete[] rgba;
		*ppBuffer = rgb;
	}
}

bool ParaEngine::TextureEntityOpenGL::LoadImageOfFormat(const std::string& sTextureFileName, char *sBufMemFile, int sizeBuf, int &width, int &height, unsigned char ** ppBuffer, int* pBytesPerPixel /*= NULL*/, int nFormat /*= -1*/)
{
	// only support 32 bit texture
	if (nFormat >= 0)
		return false;

	ParaImage img;
	if (!img.initWithImageData((unsigned char*)sBufMemFile, sizeBuf))
	{
		OUTPUT_LOG("warning:unable to load image when doing GetImageInfo, %s\n", sTextureFileName.c_str());
		return false;
	}
	width = (int)img.getWidth();
	height = (int)img.getHeight();
	
	unsigned char* pPixels = img.getData();

	// force using 32 bit texture
	int nBytesPerPixel = 4;
	if (pBytesPerPixel)
		*pBytesPerPixel = nBytesPerPixel;
	PixelFormat		pixelFormat = PixelFormat::A8R8G8B8;
	PixelFormat     renderFormat = img.getRenderFormat();
	
	unsigned char* outTempData = nullptr;
	size_t outTempDataLen = 0;

	auto format = ParaImage::convertDataToFormat(img.getData(), img.getDataLen(), renderFormat, pixelFormat, &outTempData, &outTempDataLen);
	if(format == pixelFormat)
	{
		if (outTempData == img.getData())
		{
			outTempData = new unsigned char[img.getDataLen()];
			if (nFormat == -2)
			{
				// do nothing to the data
				memcpy(outTempData, img.getData(), img.getDataLen());
			}
			else // if (nFormat == -1)
			{
				// convert from 0xAABBGGRR to 0xAARRGGBB
				int nSize = img.getDataLen() / 4;
				DWORD* pData = (DWORD*)img.getData();
				DWORD* pOutData = (DWORD*)outTempData;
				for (int i = 0; i < nSize; ++i)
				{
					DWORD color = *pData;
					*pOutData = (color & 0xff00ff00) | ((color & 0x00ff0000) >> 16) | ((color & 0x000000ff) << 16);
					pData++;
					pOutData++;
				}
			}
			
		}

		*ppBuffer = outTempData;

		// caller needs to release the buffer
		return true;
	}
	return false;
}

bool ParaEngine::TextureEntityOpenGL::SaveToFile(const char* filename, PixelFormat dwFormat, int width, int height, UINT MipLevels /*= 1*/, DWORD Filter /*= D3DX_DEFAULT*/, Color ColorKey /*= 0*/)
{
	if (!filename || filename[0] == '\0')
		return false;

	if (!IsLoaded())
	{
		LoadAsset();
	}

	const int dstWidth = width;
	const int dstHeight = height;

	void* pTextureImage = NULL;
	int nSize = 0;
	int srcWidth = 0;
	int srcHeight = 0;
	int nBytesPerPixel = 0;
	if (GetImageDataOriginal(&pTextureImage, &nSize, &srcWidth, &srcHeight, &nBytesPerPixel))
	{
		const unsigned char* srcBytes = reinterpret_cast<const unsigned char*>(pTextureImage);
		int finalW = srcWidth;
		int finalH = srcHeight;
		std::vector<unsigned char> resized;

		if (dstWidth > 0 && dstHeight > 0 && (dstWidth != srcWidth || dstHeight != srcHeight) && srcWidth > 0 && srcHeight > 0)
		{
			finalW = dstWidth;
			finalH = dstHeight;
			resized.resize(static_cast<size_t>(finalW) * static_cast<size_t>(finalH) * 4u);
			for (int y = 0; y < finalH; ++y)
			{
				const float gy = ((static_cast<float>(y) + 0.5f) * static_cast<float>(srcHeight) / static_cast<float>(finalH)) - 0.5f;
				const int y0 = (std::max)(0, (std::min)(srcHeight - 1, static_cast<int>(std::floor(gy))));
				const int y1 = (std::max)(0, (std::min)(srcHeight - 1, y0 + 1));
				const float fy = (std::max)(0.0f, (std::min)(1.0f, gy - static_cast<float>(y0)));
				for (int x = 0; x < finalW; ++x)
				{
					const float gx = ((static_cast<float>(x) + 0.5f) * static_cast<float>(srcWidth) / static_cast<float>(finalW)) - 0.5f;
					const int x0 = (std::max)(0, (std::min)(srcWidth - 1, static_cast<int>(std::floor(gx))));
					const int x1 = (std::max)(0, (std::min)(srcWidth - 1, x0 + 1));
					const float fx = (std::max)(0.0f, (std::min)(1.0f, gx - static_cast<float>(x0)));

					const unsigned char* p00 = srcBytes + (static_cast<size_t>(y0) * srcWidth + x0) * 4u;
					const unsigned char* p10 = srcBytes + (static_cast<size_t>(y0) * srcWidth + x1) * 4u;
					const unsigned char* p01 = srcBytes + (static_cast<size_t>(y1) * srcWidth + x0) * 4u;
					const unsigned char* p11 = srcBytes + (static_cast<size_t>(y1) * srcWidth + x1) * 4u;

					unsigned char* dstPx = resized.data() + (static_cast<size_t>(y) * finalW + x) * 4u;
					for (int c = 0; c < 4; ++c)
					{
						const float top = static_cast<float>(p00[c]) * (1.0f - fx) + static_cast<float>(p10[c]) * fx;
						const float bot = static_cast<float>(p01[c]) * (1.0f - fx) + static_cast<float>(p11[c]) * fx;
						const float val = top * (1.0f - fy) + bot * fy;
						dstPx[c] = static_cast<unsigned char>((std::max)(0, (std::min)(255, static_cast<int>(val + 0.5f))));
					}
				}
			}
		}

		std::string outPath = filename;
		if (!CParaFile::IsAbsolutePath(outPath))
		{
			outPath = CParaFile::GetWritablePath() + outPath;
		}
		CParaFile::CreateDirectory(outPath.c_str());

		ParaImage img;
		const unsigned char* rawPtr = resized.empty() ? srcBytes : resized.data();
		const size_t rawLen = static_cast<size_t>(finalW) * static_cast<size_t>(finalH) * 4u;
		bool ok = false;
		if (img.initWithRawData(rawPtr, rawLen, finalW, finalH, 32))
		{
			ok = img.saveToFile(outPath);
		}

		delete[] static_cast<uint8*>(pTextureImage);
		return ok;
	}
	return false;
}

bool ParaEngine::TextureEntityOpenGL::LoadFromImage(const ParaImage* pImage, UINT nMipLevels, PixelFormat dwTextureFormat/* = PixelFormat::Unknown*/, void** ppTexture /*= nullptr*/)
{
	GLTexture2D* texture = new GLTexture2D();
#ifdef PARA_ENABLE_OPENGL
	if (CGlobals::GetRenderDevice() && CGlobals::GetRenderDevice()->GetBackendType() == ERHIBackend::OpenGL)
	{
		// tricky: this fixed a cocos bug inside initWithImage() where a previous opengl error will lead to loading empty image. 
		auto errorCode = glGetError();
		if (errorCode) {
			OUTPUT_LOG("unknown opengl error: 0x%04X before LoadTexture: \n", errorCode);
		}
	}
#endif

	const ParaImage& image = *pImage;

	auto format = dwTextureFormat;
	if (texture && texture->initWithImage(pImage, format))
	{
		if (texture->getPixelsWide() == 0) {
			OUTPUT_LOG("warn: texture %d invalid size:%s (size:%d %d, image_mipmap: %d, image_size:%d,%d, format:%d)\n", (int)(texture->getName()), GetKey().c_str(), texture->getPixelsWide(), texture->getPixelsHigh(), image.getNumberOfMipmaps(), image.getWidth(), image.getHeight(), image.getRenderFormat());
		}
	}
	else
	{
		OUTPUT_LOG("error: Couldn't create texture for file:%s \n", GetKey().c_str());
	}

	if (texture)
	{
		const bool isBlockTex = (GetKey().find("blocks") != string::npos);
		SetSamplerStateBlocky(isBlockTex);
#ifdef PARA_ENABLE_OPENGL
		if (texture->getName() != 0 && CGlobals::GetRenderDevice() && CGlobals::GetRenderDevice()->GetBackendType() == ERHIBackend::OpenGL)
		{
			if (isBlockTex)
			{
				GLTexture2D::TexParams s_block_texture_params = { GL_NEAREST, GL_NEAREST, GL_REPEAT, GL_REPEAT };
				texture->setTexParameters(s_block_texture_params);
			}
			else
			{
				GL::bindTexture2D(texture->getName());
				if (texture->getPixelsWide() == GLTexture2D::ccNextPOT(texture->getPixelsWide()) && texture->getPixelsHigh() == GLTexture2D::ccNextPOT(texture->getPixelsHigh()))
				{
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				}
				else
				{
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				}
			}
		}
#endif
		if (ppTexture != NULL)
		{
			if (texture)
				texture->addref();
			(*ppTexture) = (void*)texture;
		}
		else
		{
			SetInnerTexture(texture);
		}
		SAFE_RELEASE(texture);
		return true;
	}

	SetState(AssetEntity::ASSET_STATE_FAILED_TO_LOAD);
	return false;
}

bool ParaEngine::TextureEntityOpenGL::LoadFromImage(ImageEntity * imageEntity, PixelFormat dwTextureFormat /*= D3DFMT_UNKNOWN*/, UINT nMipLevels, void** ppTexture)
{
	if (imageEntity && imageEntity->IsValid())
	{
		const char* buffer = (const char*)(imageEntity->getData());
		size_t nFileSize = imageEntity->getDataLen();
		GLImage image;
		bool bRet = false;
		if (imageEntity->getRenderFormat() == PixelFormat::A8R8G8B8)
			bRet = image.initWithRawData((const unsigned char*)buffer, nFileSize, imageEntity->getWidth(), imageEntity->getHeight(), 32);
		else
			bRet = image.initWithImageData((const unsigned char*)buffer, nFileSize);
		
		if (bRet)
		{
			GLTexture2D* texture = new GLTexture2D();

#ifdef PARA_ENABLE_OPENGL
			if (CGlobals::GetRenderDevice() && CGlobals::GetRenderDevice()->GetBackendType() == ERHIBackend::OpenGL)
			{
				// tricky: this fixed a cocos bug inside initWithImage() where a previous opengl error will lead to loading empty image. 
				auto errorCode = glGetError();
				if (errorCode){
					OUTPUT_LOG("unknown opengl error: 0x%04X before LoadTexture: \n", errorCode);
				}
			}
#endif
			auto format = dwTextureFormat;

			if (texture && texture->initWithImage(&image, format))
			{
				if (texture->getPixelsWide() == 0){
					OUTPUT_LOG("warn: texture %d invalid size:%s (size:%d %d, image_mipmap: %d, image_size:%d,%d, format:%d)\n", (int)(texture->getName()), GetKey().c_str(), texture->getPixelsWide(), texture->getPixelsHigh(), image.getNumberOfMipmaps(), image.getWidth(), image.getHeight(), image.getRenderFormat());
				}
			}
			else
			{
				OUTPUT_LOG("error: Couldn't create texture for file:%s \n", GetKey().c_str());
			}

			if (texture)
			{
				const bool isBlockTex = (GetKey().find("blocks") != string::npos);
				SetSamplerStateBlocky(isBlockTex);
#ifdef PARA_ENABLE_OPENGL
				if (texture->getName() != 0 && CGlobals::GetRenderDevice() && CGlobals::GetRenderDevice()->GetBackendType() == ERHIBackend::OpenGL)
				{
					if (isBlockTex)
					{
						GLTexture2D::TexParams s_block_texture_params = { GL_NEAREST, GL_NEAREST, GL_REPEAT, GL_REPEAT };
						texture->setTexParameters(s_block_texture_params);
					}
					else
					{
						GL::bindTexture2D(texture->getName());
						if (texture->getPixelsWide() == GLTexture2D::ccNextPOT(texture->getPixelsWide()) && texture->getPixelsHigh() == GLTexture2D::ccNextPOT(texture->getPixelsHigh()))
						{
							glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
							glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
						}
						else
						{
							glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
							glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
						}
					}
				}
#endif
				if (ppTexture != NULL)
				{
					if (texture)
						texture->addref();
					(*ppTexture) = (void*)texture;
				}
				else
				{
					SetInnerTexture(texture);
				}
				SAFE_RELEASE(texture);
				return true;
			}
		}
	}
	SetState(AssetEntity::ASSET_STATE_FAILED_TO_LOAD);
	return false;
}



HRESULT ParaEngine::TextureEntityOpenGL::LoadFromMemory(const char* buffer, DWORD nFileSize, UINT nMipLevels, PixelFormat dwTextureFormat, void** ppTexture /*= NULL*/)
{
	ImageEntity image;
	image.LoadFromMemory((const unsigned char*)buffer, nFileSize, false);
	return LoadFromImage(&image, dwTextureFormat, nMipLevels, ppTexture) ? S_OK : E_FAIL;
}

void ParaEngine::TextureEntityOpenGL::SetAliasTexParameters()
{
	if (m_texture)
		m_texture->setAliasTexParameters();
}

bool ParaEngine::TextureEntityOpenGL::IsFlipY()
{
	if (SurfaceType != RenderTarget)
		return false;
	auto* pDev = CGlobals::GetRenderDevice();
	return pDev ? !pDev->GetCaps().topLeftScreenOrigin : true;
}

bool ParaEngine::TextureEntityOpenGL::LoadImageFromString(const char* cmd)
{
	static std::string curCmd;
	static int32 curLevel = 1;
	static int32 curColor = 0;
	static int32 offsetX = 0, offsetY = 0, offsetZ = 0;
	auto parseNextCmd = [&]() {
		const char* pos = cmd;
		while (*pos != '\0' && StringHelper::isalphaLowerCase(*pos))
			pos++;
		curCmd.assign(cmd, pos - cmd);
		if (*pos != '\0')
			pos++;
		cmd = pos;
	};
	// "1", "-1", "#ff0000" are all valid values. ' ', ':', ';' are all valid separaters.
	auto parseInteger = [&]() {
		int n = 0;
		const char* pos = cmd;
		bool isPositive = true;
		if (!StringHelper::isdigit(*pos))
		{
			if (*pos == '-') {
				pos++;
				isPositive = false;
			}
			else if (*pos == '#') {
				pos++;
				// parse value in hex
				char c;
				while ((c = *pos) != '\0' && ((c >= 'a' && c <= 'f') || ((c >= '0') && (c <= '9')))) {
					int hex = c - 'a';
					n = n << 4;
					n += (hex >= 0) ? (hex + 10) : (c - '0');
					pos++;
				}
				if (c != '\0' && c != '#')
					pos++;
				cmd = pos;
				return n;
			}
		}
		char c;
		while ((c = *pos) != '\0' && StringHelper::isdigit(c)) {
			n = n * 10 + (c - '0');
			pos++;
		}
		if (c != '\0' && c != '#')
			pos++;
		cmd = pos;
		return isPositive ? n : -n;
	};
	while (*cmd != '\0')
	{
		if (StringHelper::isalphaLowerCase(*cmd))
			parseNextCmd();
		if (curCmd == "paintrect") {
			int fromX = parseInteger();
			int fromY = parseInteger();
			int toX = parseInteger();
			int toY = parseInteger();
			int nTextureWidth = std::max(fromX + 1, toX + 1);
			int nTextureHeight = std::max(fromY + 1, toY + 1);

			if (*cmd == 'd') {
				std::string dataFormat;
				// "data:image/png;base64,"
				dataFormat.assign(cmd, 22);
				cmd += 22;
				if (dataFormat == "data:image/png;base64,")
					dataFormat = "temp.png";
				else if (dataFormat == "data:image/jpg;base64,")
					dataFormat = "temp.jpg";
				else
					dataFormat = "";
				if (!dataFormat.empty())
				{
					std::string buffer = StringHelper::unbase64(cmd, -1);
					int texWidth, texHeight, nBytesPerPixel;
					unsigned char* imageData = NULL;
					// tricky: nFormat is -2 to preserve the AGBR byte order instead of default ARGB
					if (TextureEntity::LoadImageOfFormat(dataFormat, (char*)(buffer.c_str()), (int)buffer.size(), texWidth, texHeight, &imageData, &nBytesPerPixel, -2))
					{
						nTextureWidth = std::max(nTextureWidth, texWidth);
						nTextureHeight = std::max(nTextureHeight, texHeight);
						if (toX < 0)
							toX = nTextureWidth - 1;
						if (toY < 0)
							toY = nTextureHeight - 1;
						if (nBytesPerPixel == 4)
						{
							DWORD* pData = (DWORD*)imageData;
							uint32 x = fromX, y = fromY;

							if (SurfaceType != DynamicTexture)
							{
								UnloadAsset();
								SurfaceType = DynamicTexture;
							}
							if (m_pTextureInfo && (m_pTextureInfo->m_width != nTextureWidth || m_pTextureInfo->m_height != nTextureHeight))
							{
								UnloadAsset();
								m_pTextureInfo->m_width = -1;
								m_pTextureInfo->m_height = -1;
							}
							DWORD* pDest = new DWORD[nTextureWidth * nTextureHeight];
							while (true) {
								x = fromX;
								int srcX = 0;
								while (true) {
									pDest[x + y * nTextureWidth] = *(pData + srcX);
									if (x == toX)
										break;
									x += (fromX < toX) ? 1 : -1;
									srcX++;
								}
								pData += texWidth;
								if (y == toY)
									break;
								y += (fromY < toY) ? 1 : -1;
							}
							if (!m_texture)
							{
								// for now, we do not generate mipmap.
								m_texture = new (std::nothrow) GLTexture2D();
								m_texture->initWithData(pDest, nTextureWidth * nTextureHeight * 4, PixelFormat::A8R8G8B8, nTextureWidth, nTextureHeight, Size((float)nTextureWidth, (float)nTextureHeight));
							}
							else
							{
								m_texture->updateWithData(pDest, 0, 0, nTextureWidth, nTextureHeight);
							}
							m_dynamicPixels.assign(reinterpret_cast<const uint8*>(pDest), reinterpret_cast<const uint8*>(pDest) + nTextureWidth * nTextureHeight * 4);
							SAFE_DELETE_ARRAY(pDest);
							m_bIsInitialized = true;
							m_bIsValid = true;
							if (m_pTextureInfo == NULL)
								m_pTextureInfo = new TextureInfo();
							m_pTextureInfo->m_width = nTextureWidth;
							m_pTextureInfo->m_height = nTextureHeight;
						}
						SAFE_DELETE_ARRAY(imageData);
					}
				}
			}
			else
			{
				DWORD* imageData = new DWORD[nTextureWidth * nTextureHeight];
				uint32 x = fromX, y = fromY;
				while (true) {
					x = fromX;
					while (true) {
						uint32 color = parseInteger();
						// convert from 0xAARRGGBB to 0xAABBGGRR
						imageData[x + y * nTextureWidth] = (color & 0xff00ff00) | ((color & 0x00ff0000) >> 16) | ((color & 0x000000ff) << 16);
						// imageData[x + y * nTextureWidth] = color;
						if (x == toX)
							break;
						x += (fromX < toX) ? 1 : -1;
					}
					if (y == toY)
						break;
					y += (fromY < toY) ? 1 : -1;
				}
				LoadUint8Buffer((const uint8*)imageData, nTextureWidth, nTextureHeight, nTextureWidth * 4, 4);
				SAFE_DELETE_ARRAY(imageData);
			}
		}
		else {
			break;
		}
	}
	return false;
}

bool ParaEngine::TextureEntityOpenGL::GetImageData(void** ppData, int* pSize, int* pWidth, int* pHeight, int* pBytesPerPixel)
{
	if(GetImageDataOriginal(ppData, pSize, pWidth, pHeight, pBytesPerPixel))
	{
		if (*pBytesPerPixel == 4)
		{
			// convert from 0xAARRGGBB to 0xAABBGGRR
			int nSize = *pSize / 4;
			uint32* pData = (uint32*)(*ppData);
			for (int i = 0; i < nSize; ++i)
			{
				uint32 color = pData[i];
				pData[i] = (color & 0xff00ff00) | ((color & 0x00ff0000) >> 16) | ((color & 0x000000ff) << 16);
			}
		}
		return true;
	}
	return false;
}

bool ParaEngine::TextureEntityOpenGL::GetImageDataOriginal(void** ppData, int* pSize, int* pWidth, int* pHeight, int* pBytesPerPixel)
{
	if (m_texture)
	{
		auto pTextureInfo = GetTextureInfo();
		if(!pTextureInfo)
			return false;
		int width = pTextureInfo->GetWidth();
		int height = pTextureInfo->GetHeight();
		if (width <= 0 || height <= 0)
			return false;
		*pWidth = width;
		*pHeight = height;
		*pBytesPerPixel = 4;
		*pSize = width * height * 4;
		uint8* pData = new uint8[*pSize];

		auto* pDev = CGlobals::GetRenderDevice();
		if (pDev && m_texture->getRHITexture() && pDev->ReadTexturePixels(m_texture->getRHITexture(), pData))
		{
			*ppData = pData;
			return true;
		}

		if (!m_dynamicPixels.empty() && static_cast<int>(m_dynamicPixels.size()) == *pSize)
		{
			memcpy(pData, m_dynamicPixels.data(), *pSize);
			*ppData = pData;
			return true;
		}

		delete[] pData;
	}
	return false;
}
#endif

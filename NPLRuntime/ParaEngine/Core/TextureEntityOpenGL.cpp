//-----------------------------------------------------------------------------
// Class:	TextureEntityOpengGL
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2014.8
// Desc: currently all opengl stuffs are implemented using cocos2dx objects. 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"

#ifdef USE_OPENGL_RENDERER
#include "ImageEntity.h"
#include "TextureEntity.h"
#include "OpenGLWrapper/GLImage.h"
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

HRESULT ParaEngine::TextureEntityOpenGL::RestoreDeviceObjects()
{
	if (m_bIsInitialized)
		return S_OK;

	if (SurfaceType == RenderTarget)
	{
		m_bIsInitialized = true;

		DWORD width, height;
		// Default: create the render target with alpha. this will allow some mini scene graphs to render alpha pixels. 

		PixelFormat format = PixelFormat::A8R8G8B8;

		if (GetTextureInfo() != NULL)
		{
			width = GetTextureInfo()->m_width;
			height = GetTextureInfo()->m_height;
			if (width == 0 || height == 0)
			{
				width = 256;
				height = 256;
			}
			if (GetTextureInfo()->m_format == TextureInfo::FMT_X8R8G8B8)
			{
				format = PixelFormat::X8R8G8B8;
			}
		}
		else
		{
			width = 256;
			height = 256;
		}
		m_texture = new (std::nothrow) GLTexture2D();
		if (m_texture)
		{
			void *data = nullptr;
			auto dataLen = width * height * 4;
			data = malloc(dataLen);
			memset(data, 0, dataLen);
			m_texture->initWithData(data, dataLen, PixelFormat::A8R8G8B8, width, height, Size((float)width, (float)height));
			free(data);

			GL::bindTexture2D(m_texture->getName());
			if (m_texture->getPixelsWide() == GLTexture2D::ccNextPOT(m_texture->getPixelsWide()))
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			}
			else
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			}


			if (m_texture->getPixelsHigh() == GLTexture2D::ccNextPOT(m_texture->getPixelsHigh()))
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			}
			else
			{
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			}
		}
		else
			return S_OK;
	}
	return S_OK;
}

HRESULT ParaEngine::TextureEntityOpenGL::InvalidateDeviceObjects()
{
	return S_OK;
}

HRESULT ParaEngine::TextureEntityOpenGL::DeleteDeviceObjects()
{
	m_bIsInitialized = false;
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
	if (SurfaceType == StaticTexture)
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
			if (!texture->initWithData(pTexels, dataLen, PixelFormat::A8R8G8B8, width, height, Size((float)width, (float)height)))
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
	GLTexture2D * texture = m_texture;
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
			if (!texture->initWithData(pTexels, dataLen, PixelFormat::A8R8G8B8, width, height, Size((float)width, (float)height)))
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
				int nSize = width * height;
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

	return this;
}

void ParaEngine::TextureEntityOpenGL::LoadImage(char *sBufMemFile, int sizeBuf, int &width, int &height, byte ** ppBuffer, bool bAlpha)
{
	/*
	GLImage* image = new Image();

	if (image)
	{
		if (image->initWithImageData((const unsigned char*)sBufMemFile, sizeBuf))
		{
			width = image->getWidth();
			height = image->getHeight();
			unsigned char* pData = image->getData();
			int nSize = (int)(image->getDataLen());
			if (image->isCompressed())
			{

			}
			else
			{

			}
		}
		CC_SAFE_RELEASE(image);
	}
	*/
}

static void copySurfaceRGB(void* pDest, const void* pSrc, UINT pitch, UINT width, UINT height)
{
	BYTE* pDst = (BYTE*)pDest;
	const BYTE* data = (const BYTE*)pSrc;

	int index = 0, x = 0, y = 0;

	for (y = 0; y < height; y++)
	{
		for (x = 0; x < width; x++)
		{
			int n = (y * 3 * width) + 3 * x;
			pDst[index++] = data[n];
			pDst[index++] = data[n + 1];
			pDst[index++] = data[n + 2];
			pDst[index++] = 0xff;
		}
		index += pitch - (width * 4);
	}
}

static void copySurface(void* pDest, const void* pSrc, UINT pitch, UINT width, UINT height, UINT bpp)
{
	BYTE* pDst = (BYTE*)pDest;
	const BYTE* data = (const BYTE*)pSrc;

	for (UINT h = 0; h < height; h++)
	{
		memcpy(pDst, data, bpp * width / 8);

		data += bpp * width / 8;
		pDst += pitch;
	}
}

bool ParaEngine::TextureEntityOpenGL::LoadImageOfFormat(const std::string& sTextureFileName, char *sBufMemFile, int sizeBuf, int &width, int &height, byte ** ppBuffer, int* pBytesPerPixel /*= NULL*/, int nFormat /*= -1*/)
{
	// only support 32 bit texture
	if (nFormat >= 0)
		return false;

	CParaFile file;
	if (!file.OpenFile(sTextureFileName.c_str()))
	{
		OUTPUT_LOG("warning:unable to open file when doing GetImageInfo, %s\n", sTextureFileName.c_str());
		return false;
	}

	ParaImage img;
	if (!img.initWithImageData((unsigned char*)file.getBuffer(), file.getSize()))
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
			outTempData = new byte[img.getDataLen()];
			memcpy(outTempData, img.getData(), img.getDataLen());
		}

		*ppBuffer = outTempData;

		// caller needs to release the buffer
		return true;
	}
	return false;
}

bool ParaEngine::TextureEntityOpenGL::SaveToFile(const char* filename, PixelFormat dwFormat, int width, int height, UINT MipLevels /*= 1*/, DWORD Filter /*= D3DX_DEFAULT*/, Color ColorKey /*= 0*/)
{
	// std::string src_file = GetKey();
	// width = GetWidth();
	// height = GetHeight();
	// GLuint textureId = GetTexture()->getName();
	// GLuint framebuffer;
	// glGenFramebuffers(1, &framebuffer);
	// glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
	// glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, textureId, 0);
	// GLint readType, readFormat;
	// glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_TYPE, &readType);
	// glGetIntegerv(GL_IMPLEMENTATION_COLOR_READ_FORMAT, &readFormat);
	// unsigned int bytesPerPixel = 0;
	// switch (readType)
	// {
	// case GL_UNSIGNED_BYTE:
	// case GL_BYTE:
	// 	switch (readFormat)
	// 	{
	// 	case GL_RGBA:
	// 		bytesPerPixel = 4;
	// 		break;
	// 	case GL_RGB:
	// 	case GL_RGB_INTEGER:
	// 		bytesPerPixel = 3;
	// 		break;		
	// 	case GL_RG:
	// 	case GL_RG_INTEGER:
	// 	case GL_LUMINANCE_ALPHA:
	// 		bytesPerPixel = 2;
	// 		break;
	// 	case GL_RED:
	// 	case GL_RED_INTEGER:
	// 	case GL_ALPHA:
	// 	case GL_LUMINANCE:
	// 		bytesPerPixel = 1;
	// 		break;
	// 	default:
	// 		break;
	// 	}
	// 	break;
	// case GL_FLOAT:
	// case GL_UNSIGNED_INT:
	// case GL_INT:
	// 	switch (readFormat)
	// 	{
	// 	case GL_RGBA:
	// 		bytesPerPixel = 16;
	// 		break;
	// 	case GL_RGB:
	// 	case GL_RGB_INTEGER:
	// 		bytesPerPixel = 12;
	// 		break;		
	// 	case GL_RG:
	// 	case GL_RG_INTEGER:
	// 	case GL_LUMINANCE_ALPHA:
	// 		bytesPerPixel = 8;
	// 		break;
	// 	case GL_RED:
	// 	case GL_RED_INTEGER:
	// 	case GL_ALPHA:
	// 	case GL_LUMINANCE:
	// 		bytesPerPixel = 4;
	// 		break;
	// 	default:
	// 		break;
	// 	}
	// 	break;
	// default:
	// 	break;
	// }
	// GLubyte* pixels = (GLubyte*)malloc(width * height * bytesPerPixel);
	// glReadPixels(0, 0, width, height, readFormat, readType, pixels);
	// ParaImage image;
	// image.initWithRawData(pixels, width * height * bytesPerPixel, width, height, bytesPerPixel);
	// image.saveImageToPNG(filename, false);
	// free(pixels);
	// glBindFramebuffer(GL_FRAMEBUFFER, 0);
	// glDeleteFramebuffers(1, &framebuffer);
	return false;
}

bool ParaEngine::TextureEntityOpenGL::LoadFromImage(const ParaImage* pImage, UINT nMipLevels, PixelFormat dwTextureFormat/* = PixelFormat::Unknown*/, void** ppTexture /*= nullptr*/)
{
	GLTexture2D* texture = new GLTexture2D();
	{
		// tricky: this fixed a cocos bug inside initWithImage() where a previous opengl error will lead to loading empty image. 
		auto errorCode = glGetError();
		if (errorCode) {
			OUTPUT_LOG("unknown opengl error: 0x%04X before LoadTexture: \n", errorCode);
		}
	}

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
		if (texture == NULL || texture->getName() == 0)
		{

		}
		else
		{
			if (GetKey().find("blocks") != string::npos)
			{
				// if texture filename contains "blocks" either in folder name or filename, we will force POINT mip mapping
				//Texture2D::TexParams s_block_texture_params = { GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE };
				GLTexture2D::TexParams s_block_texture_params = { GL_NEAREST, GL_NEAREST, GL_REPEAT, GL_REPEAT };
				texture->setTexParameters(s_block_texture_params);
				SetSamplerStateBlocky(true);
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

				SetSamplerStateBlocky(false);
			}
		}
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
			bRet = image.initWithRawData((const unsigned char*)buffer, nFileSize, imageEntity->getWidth(), imageEntity->getHeight(), imageEntity->hasPremultipliedAlpha());
		else
			bRet = image.initWithImageData((const unsigned char*)buffer, nFileSize);
		
		if (bRet)
		{
			GLTexture2D* texture = new GLTexture2D();

			{
				// tricky: this fixed a cocos bug inside initWithImage() where a previous opengl error will lead to loading empty image. 
				auto errorCode = glGetError();
				if (errorCode){
					OUTPUT_LOG("unknown opengl error: 0x%04X before LoadTexture: \n", errorCode);
				}
			}
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
				if (texture == NULL || texture->getName() == 0)
				{

				}
				else
				{
					if (GetKey().find("blocks") != string::npos)
					{
						// if texture filename contains "blocks" either in folder name or filename, we will force POINT mip mapping
						//Texture2D::TexParams s_block_texture_params = { GL_NEAREST, GL_NEAREST, GL_CLAMP_TO_EDGE, GL_CLAMP_TO_EDGE };
						GLTexture2D::TexParams s_block_texture_params = { GL_NEAREST, GL_NEAREST, GL_REPEAT, GL_REPEAT };
						texture->setTexParameters(s_block_texture_params);
						SetSamplerStateBlocky(true);
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

						SetSamplerStateBlocky(false);
					}
				}
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
	return SurfaceType == RenderTarget;
}

#endif

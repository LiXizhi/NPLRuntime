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
#include "OpenGLWrapper.h"
#include "ImageEntity.h"
#include "TextureEntity.h"

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

ParaEngine::TextureEntityOpenGL::TextureEntityOpenGL(GLWrapper::Texture2D* texture)
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
	return S_OK;
}


GLWrapper::Texture2D* ParaEngine::TextureEntityOpenGL::CreateTextureFromFile_Serial(const std::string& sFileName)
{
	GLWrapper::Texture2D * texture = nullptr;
	CParaFile file;
	if (file.OpenFile(sFileName.c_str(), true))
	{
		LoadFromMemory(file.getBuffer(), file.getSize(), 0, D3DFMT_UNKNOWN, (void**)(&texture));
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
					m_pTextureSequence = new GLWrapper::Texture2D*[nTotalTextureSequence];
					memset(m_pTextureSequence, 0, sizeof(GLWrapper::Texture2D*)*nTotalTextureSequence);
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


void ParaEngine::TextureEntityOpenGL::SetInnerTexture(GLWrapper::Texture2D* texture)
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

		D3DFORMAT format = D3DFMT_A8R8G8B8;

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
				format = D3DFMT_X8R8G8B8;
			}
		}
		else
		{
			width = 256;
			height = 256;
		}
		m_texture = new (std::nothrow) Texture2D();
		if (m_texture)
		{
			void *data = nullptr;
			auto dataLen = width * height * 4;
			data = malloc(dataLen);
			memset(data, 0, dataLen);
			m_texture->initWithData(data, dataLen, (Texture2D::PixelFormat)Texture2D::PixelFormat::RGBA8888, width, height, Size((float)width, (float)height));
			free(data);

			GL::bindTexture2D(m_texture->getName());
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
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
			return tex ? tex->getName() : 0;
		}
		break;
	}
	default:
	{
		return m_texture ? m_texture->getName() : 0;
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

TextureEntity* ParaEngine::TextureEntityOpenGL::CreateTexture(const char* pFileName, uint32 nMipLevels /*= 0*/, D3DPOOL dwCreatePool /*= D3DPOOL_MANAGED*/)
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

TextureEntity* ParaEngine::TextureEntityOpenGL::CreateTexture(const uint8 * pTexels, int width, int height, int rowLength, int bytesPerPixel, uint32 nMipLevels /*= 0*/, D3DPOOL dwCreatePool /*= D3DPOOL_MANAGED*/, DWORD nFormat /*= 0*/)
{
	if (!pTexels)
		return NULL;
	GLWrapper::Texture2D * texture = new Texture2D();
	if (texture)
	{
		int dataLen = width * height * bytesPerPixel;
		if (bytesPerPixel == 4)
		{
			if (!texture->initWithData(pTexels, dataLen, GLWrapper::Texture2D::PixelFormat::BGRA8888, width, height, Size((float)width, (float)height)))
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
			if (!texture->initWithData(pTexels, dataLen, Texture2D::PixelFormat::BGRA8888, width, height, Size((float)width, (float)height)))
			{
				SAFE_DELETE(texture);
			}
			delete[] pp;
		}
		else if (bytesPerPixel == 1)
		{
			/** -1 not determined, 0 do not support A8, 1 support A8. we will only try it once. */
			static int nSupportA8PixelFormat = -1;
			
			if (nSupportA8PixelFormat == 0 || !texture->initWithData(pTexels, dataLen, Texture2D::PixelFormat::A8, width, height, Size((float)width, (float)height)))
			{
				if (nSupportA8PixelFormat == -1)
					nSupportA8PixelFormat = 0;
				int nSize = width*height;
				DWORD* pData = new DWORD[nSize];
				for (int x = 0; x < nSize; ++x)
				{
					pData[x] = (pTexels[x]) << 24;
				}
				if (!texture->initWithData(pData, width*height * 4, Texture2D::PixelFormat::BGRA8888, width, height, Size((float)width, (float)height)))
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

void ParaEngine::TextureEntityOpenGL::LoadImage(char *sBufMemFile, int sizeBuf, int &width, int &height, byte ** ppBuffer, bool bAlpha)
{
	/*
	GLWrapper::Image* image = new Image();

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

bool ParaEngine::TextureEntityOpenGL::LoadImageOfFormat(const std::string& sTextureFileName, char *sBufMemFile, int sizeBuf, int &width, int &height, byte ** ppBuffer, int* pBytesPerPixel /*= NULL*/, int nFormat /*= -1*/)
{
	return false;
}

bool ParaEngine::TextureEntityOpenGL::SaveToFile(const char* filename, D3DFORMAT dwFormat, int width, int height, UINT MipLevels /*= 1*/, DWORD Filter /*= D3DX_DEFAULT*/, Color ColorKey /*= 0*/)
{
	// TODO:
	return false;
}

bool ParaEngine::TextureEntityOpenGL::LoadFromImage(ImageEntity * imageEntity, D3DFORMAT dwTextureFormat /*= D3DFMT_UNKNOWN*/, UINT nMipLevels, void** ppTexture)
{
	if (imageEntity && imageEntity->IsValid())
	{
		const char* buffer = (const char*)(imageEntity->getData());
		size_t nFileSize = imageEntity->getDataLen();
		GLWrapper::Image image;
		bool bRet = false;
		if (imageEntity->getRenderFormat() == D3DFMT_A8R8G8B8)
			bRet = image.initWithRawData((const unsigned char*)buffer, nFileSize, imageEntity->getWidth(), imageEntity->getHeight(), imageEntity->hasPremultipliedAlpha());
		else
			bRet = image.initWithImageData((const unsigned char*)buffer, nFileSize);
		
		if (bRet)
		{
			auto texture = new GLWrapper::Texture2D();
			{
				// tricky: this fixed a cocos bug inside initWithImage() where a previous opengl error will lead to loading empty image. 
				auto errorCode = glGetError();
				if (errorCode){
					OUTPUT_LOG("unknown opengl error: 0x%04X before LoadTexture: \n", errorCode);
				}
			}
			auto format = GLWrapper::Texture2D::PixelFormat::AUTO;
			if (dwTextureFormat != 0){
				if (dwTextureFormat == D3DFORMAT::D3DFMT_DXT1)
					format = GLWrapper::Texture2D::PixelFormat::S3TC_DXT1;
				else if (dwTextureFormat == D3DFORMAT::D3DFMT_DXT3)
					format = GLWrapper::Texture2D::PixelFormat::S3TC_DXT3;
				else if (dwTextureFormat == D3DFORMAT::D3DFMT_DXT5)
					format = GLWrapper::Texture2D::PixelFormat::S3TC_DXT5;
			}

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
						GLWrapper::Texture2D::TexParams s_block_texture_params = { GL_NEAREST, GL_NEAREST, GL_REPEAT, GL_REPEAT };
						texture->setTexParameters(s_block_texture_params);
						SetSamplerStateBlocky(true);
					}
					else
					{
						GL::bindTexture2D(texture->getName());
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
						glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
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


HRESULT ParaEngine::TextureEntityOpenGL::LoadFromMemory(const char* buffer, DWORD nFileSize, UINT nMipLevels, D3DFORMAT dwTextureFormat, void** ppTexture /*= NULL*/)
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
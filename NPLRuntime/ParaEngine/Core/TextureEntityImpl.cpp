//-----------------------------------------------------------------------------
// Class:	TextureEntityDirectX
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2004.3.8
// Revised: 2006.7.12, 2009.8.18(AsyncLoader added)
//-----------------------------------------------------------------------------
#include "ParaEngine.h"

#if defined(USE_DIRECTX_RENDERER) || defined(USE_OPENGL_RENDERER)
/**
Which DXT Compression to Use?
Obviously, there are some trade-offs between the different formats which make them better or worse for different types of images. Some general rules of thumb for good use of DXT textures are as follows:
If your image has no alpha, use DXT1 compression. Using DXT3/5 will double your image size over DXT1 and not gain anything. 
If your image has 1-bit (on or off) alpha information, use DXT1 with one-bit alpha. If the DXT1 image quality is too low and you don't mind doubling image size, use DXT3 or DXT5 (which one doesn't matter, they'll give the same results). 
If your image has smooth gradations of alpha (fading in/out slowly), DXT5 is almost certainly your best bet, as it will give you the most accurate transparency representation. 
If your image has sharp transitions between multiple alpha levels (one pixel is 100%, the next one is 50%, and another neighbor is 12%), DXT3 is probably your best bet. You may want to compare the alpha results in DXT1, DXT3 and DXT5 compression, however, to make sure. 
*/
#ifdef USE_FLASH_MANAGER
#include "FlashTextureManager.h"
#endif
#include "HtmlBrowserManager.h"
#include "ParaWorldAsset.h"
#ifdef USE_FREEIMAGE
#include "FreeImageMemIO.h"
#endif

#include "ContentLoaders.h"
#include "AsyncLoader.h"
#include "ViewportManager.h"
#include "TextureEntityImpl.h"

#if USE_DIRECTX_RENDERER
#include <gdiplus.h>
#include "RenderDeviceD3D9.h"
#include "TextureD3D9.h"
#include "D3DMapping.h"
#endif



#ifdef PARAENGINE_CLIENT
	#include "memdebug.h"
#endif

// to lower case
#define MAKE_LOWER(c)  if((c)>='A' && (c)<='Z'){(c) = (c)-'A'+'a';}

using namespace ParaEngine;
using namespace IParaEngine;
namespace ParaEngine
{
	extern int globalTime;
}


TextureEntityImpl::TextureEntityImpl(const AssetKey& key)
:m_pTexture(NULL), TextureEntity(key)
{
}

TextureEntityImpl::TextureEntityImpl()
: m_pTexture(NULL), TextureEntity()
{
}

TextureEntityImpl::~TextureEntityImpl()
{
	if (m_bIsInitialized)
	{
		UnloadAsset();
	}
}

EPixelFormat TextureEntityImpl::GetFormat()
{
	auto format = m_pTextureInfo->GetFormat();
	if (format == TextureInfo::FMT_A8R8G8B8)
		return EPixelFormat::A8R8G8B8;
	else if (format == TextureInfo::FMT_X8R8G8B8)
		return EPixelFormat::X8R8G8B8;
	else if (format == TextureInfo::FMT_UNKNOWN)
		return EPixelFormat::Unkonwn;
	else
		return EPixelFormat::X8R8G8B8;
}

IParaEngine::ITexture* TextureEntityImpl::GetSurface()
{
	if(SurfaceType == SysMemoryTexture || SurfaceType == TextureSurface)
	{
		LoadAsset();
		return m_pTexture;
	}
	else
		return NULL;
}

IParaEngine::ITexture* TextureEntityImpl::GetTexture()
{
	++m_nHitCount;
	ITexture* tex = NULL;
	if(IsLocked())
	{
		return NULL;
	}
	switch(SurfaceType)
	{
	case TextureSequence:
		{
			// animated texture sequence
			LoadAsset();
			AnimatedTextureInfo* pInfo = GetAnimatedTextureInfo();
			if(pInfo!=0 && m_pTextureSequence!=0 && pInfo->m_nCurrentFrameIndex<pInfo->m_nFrameCount)
			{
				if(pInfo->m_bAutoAnimation)
				{
					if(pInfo->m_fFPS>=0)
					{
						pInfo->m_nCurrentFrameIndex = ((int)(globalTime * pInfo->m_fFPS / 1000)) % pInfo->m_nFrameCount;
					}
					else
					{
						pInfo->m_nCurrentFrameIndex = pInfo->m_nFrameCount-1-((int)(-globalTime * pInfo->m_fFPS / 1000)) % pInfo->m_nFrameCount;
					}
				}
				tex = m_pTextureSequence[pInfo->m_nCurrentFrameIndex];
			}
			break;
		}
	case FlashTexture:
		// flash texture
		LoadAsset();
#ifdef USE_FLASH_MANAGER
		tex = CGlobals::GetAssetManager()->GetFlashManager().GetTexture(GetKey().c_str());
#endif
		break;
	case HTMLTexture:
		{
#if USE_DIRECTX_RENDERER
			// HTML web texture
			LoadAsset();
			CHTMLBrowser* pHTML = CGlobals::GetAssetManager()->GetHTMLBrowserManager().CreateGetBrowserWindow(GetKey().c_str());
			if(pHTML != 0)
			{
				// this is tricky, since many html textures share the same browser window. we will only load if the current browser window's last navigation url is empty.
				if(pHTML->GetLastNavURL().empty() && !GetLocalFileName().empty())
					pHTML->navigateTo(GetLocalFileName());
				tex = pHTML->GetTexture();
			}
#endif
			break;
		}
	default:
		// all other texture types
		LoadAsset();
		tex = m_pTexture;
		//if(tex == NULL)
		//{
		//	// if texture is not available we shall return the default texture. 
		//	tex = CGlobals::GetAssetManager()->GetDefaultTexture()->m_pTexture;
		//}
		break;
	}
	return tex;
}

bool TextureEntityImpl::IsLoaded()
{
	return GetTexture()!=0;
}

const TextureEntityImpl::TextureInfo* TextureEntityImpl::GetTextureInfo()
{
	/** lazy get. Since not all texture users want to get the texture info,
	we will only allocate memory when this texture entity instance's info is used 
	for the first time. */
	switch(SurfaceType)
	{
	case TextureSequence:
		{
			GetAnimatedTextureInfo();
			if( (m_pAnimatedTextureInfo && m_pAnimatedTextureInfo->m_width==0) )
			{
				ITexture* pTexture = NULL;
				if(IsLocked() || (pTexture = GetTexture()) == 0)
				{
					// if texture is locked (being downloaded)
					m_pTextureInfo->m_width = -1;
					m_pTextureInfo->m_height = -1;
				}
				else
				{
					if(pTexture)
					{
						m_pAnimatedTextureInfo->m_width = pTexture->GetWidth();
						m_pAnimatedTextureInfo->m_height = pTexture->GetHeight();
					}
				}
			}
			return m_pAnimatedTextureInfo;
		}
	case FlashTexture:
		{
			// the texture info of a flash texture
			// TODO: what happens if the texture is not local?
			if(m_pTextureInfo==NULL)
			{
				m_pTextureInfo = new TextureInfo();
				m_pTextureInfo->m_width = 128;
				m_pTextureInfo->m_height = 128;
#ifdef USE_FLASH_MANAGER
				CGlobals::GetAssetManager()->GetFlashManager().GetTextureInfo(GetKey().c_str(), (int*)&(m_pTextureInfo->m_width), (int*)&(m_pTextureInfo->m_height));
#endif
			}
			break;
		}
	case HTMLTexture:
		{

#ifdef USE_DIRECTX_RENDERER
			// web texture
			if(m_pTextureInfo==NULL)
			{
				m_pTextureInfo = new TextureInfo();
				m_pTextureInfo->m_width = 512;
				m_pTextureInfo->m_height = 512;
				CHTMLBrowser* pHTML = CGlobals::GetAssetManager()->GetHTMLBrowserManager().CreateGetBrowserWindow(GetKey().c_str());
				if(pHTML != 0)
				{
					// may vary slightly when page is rendered. How to deal with it?
					m_pTextureInfo->m_width = pHTML->getBrowserWidth();
					m_pTextureInfo->m_height = pHTML->getBrowserHeight();
					if(m_pTextureInfo->m_width == 0 || m_pTextureInfo->m_height ==0)
					{
						// sometimes, such things happens.
						m_pTextureInfo->m_width = 512;
						m_pTextureInfo->m_height = 512;
						OUTPUT_LOG("warning: HTML browser size is not returned correctly. Returned value is %d %d, where 512,512 is used instead\n", pHTML->getBrowserWidth(), pHTML->getBrowserHeight());
					}
				}
			}
#endif
			break;
		}
	default:
		{
			// for other texture types
			if(m_pTextureInfo==NULL)
			{
				m_pTextureInfo = new TextureInfo();
				if( (SurfaceType == TextureEntityImpl::RenderTarget) || 
					(SurfaceType == TextureEntityImpl::DEPTHSTENCIL) ||
					(SurfaceType == TextureEntityImpl::SysMemoryTexture))
				{
					if(GetTexture())
					{
							m_pTextureInfo->m_width = GetTexture()->GetWidth();
							m_pTextureInfo->m_height = GetTexture()->GetHeight();
					}
				}
				else
				{
					if (IsLocked() || (GetTexture() == 0 && GetSurface() == 0))
					{
						// if texture is locked (being downloaded)
						m_pTextureInfo->m_width = -1;
						m_pTextureInfo->m_height = -1;
					}
					else if (GetSurface())
					{
						m_pTextureInfo->m_width = GetSurface()->GetWidth();
						m_pTextureInfo->m_height = GetSurface()->GetHeight();
					}
					else if(m_pTexture)
					{
						m_pTextureInfo->m_width = m_pTexture->GetWidth();
						m_pTextureInfo->m_height = m_pTexture->GetHeight();
					}
				}
			}
			break;
		}
	}
	
	return m_pTextureInfo;
}

HRESULT TextureEntityImpl::CreateTextureFromFile_Serial(IRenderDevice* pDev, const char* sFileName, ITexture** ppTexture, EPixelFormat dwTextureFormat, UINT nMipLevels, Color dwColorKey )
{
	// Load Texture sequentially
	asset_ptr<TextureEntity> my_asset(this);
	CTextureLoader loader_( my_asset, sFileName );
	CTextureLoader* pLoader = &loader_;
	CTextureProcessor processor_( my_asset );
	CTextureProcessor* pProcessor = &processor_;

	pLoader->m_dwTextureFormat = dwTextureFormat;
	pLoader->m_nMipLevels = nMipLevels;
	pLoader->m_ppTexture = (void**)ppTexture;
	pLoader->m_dwColorKey = dwColorKey;

	pProcessor->m_dwTextureFormat = dwTextureFormat;
	pProcessor->m_nMipLevels = nMipLevels;
	pProcessor->m_pDevice = pDev;
	pProcessor->m_ppTexture = (void**)ppTexture;
	pProcessor->m_dwColorKey = dwColorKey;

	void* pLocalData;
	int Bytes;
	if( SUCCEEDED(pLoader->Load()) && 
		SUCCEEDED(pLoader->Decompress( &pLocalData, &Bytes )) && 
		SUCCEEDED(pProcessor->Process( pLocalData, Bytes )) && 
		SUCCEEDED(pProcessor->LockDeviceObject()) && 
		SUCCEEDED(pProcessor->CopyToResource()) && 
		SUCCEEDED(pProcessor->UnLockDeviceObject()) )
	{
	}
	else
	{
		pProcessor->SetResourceError();
	}
	pProcessor->Destroy();
	pLoader->Destroy();
	return S_OK;
}

void TextureEntityImpl::GetFormatAndMipLevelFromFileNameEx(const string& sTextureFileName, EPixelFormat* pdwTextureFormat, UINT* pnMipLevels)
{
	int nSize = (int)sTextureFileName.size();
	EPixelFormat dwTextureFormat = EPixelFormat::Unkonwn;
	UINT MipLevels = D3DX_DEFAULT;
	if(nSize>4)
	{
		char c1 = sTextureFileName[nSize-3];MAKE_LOWER(c1)
		char c2 = sTextureFileName[nSize-2];MAKE_LOWER(c2)
		char c3 = sTextureFileName[nSize-1];MAKE_LOWER(c3)

		// if it is png file we will use dxt3 internally since it contains alpha. 
		if(c1=='d' && c2=='d' && c3=='s')
			dwTextureFormat = EPixelFormat::Unkonwn;
		else if(c1=='p' && c2=='n' && c3=='g')
		{
			/** whether we treat png file as DXT3 by default. if the texture filename ends with "_32bits.png", we will load with D3DFMT_A8R8G8B8 instead of DXT3. 
			If one wants to ensure high resolution texture, use TGA format instead. */
			if(sTextureFileName.find("_32bits") != string::npos)
			{
				dwTextureFormat = EPixelFormat::A8R8G8B8;
				MipLevels = 1;
			}
			//else if(sTextureFileName.find("blocks") != string::npos)
			//{
			//	// if texture filename contains "blocks" either in folder name or filename, we will force using D3DFMT_A8R8G8B8 with full mip levels
			//	dwTextureFormat = D3DFMT_A8R8G8B8;
			//}
			else if(sTextureFileName.find("_dxt1") != string::npos)
			{
				/** if the texture filename ends with "_dxt1.png", we will load with DXT1 */
				dwTextureFormat = EPixelFormat::DXT1;
				MipLevels = 1;
			}
			else
			{
				dwTextureFormat = EPixelFormat::DXT3;
				MipLevels = 1;
			}
			
		}
		else if(c1=='j' && c2=='p' && c3=='g')
		{
			dwTextureFormat = EPixelFormat::DXT1;
			MipLevels = 1;
		}
	}
	if(pdwTextureFormat)
		*pdwTextureFormat = dwTextureFormat;
	if(pnMipLevels)
		*pnMipLevels = MipLevels;
}

void TextureEntityImpl::GetFormatAndMipLevelFromFileName(const string& sTextureFileName, EPixelFormat* pdwTextureFormat, UINT* pnMipLevels)
{
	int nSize = (int)sTextureFileName.size();
	EPixelFormat dwTextureFormat = EPixelFormat::Unkonwn;
	UINT MipLevels = D3DX_DEFAULT;
	if(nSize>4)
	{
		char c1 = sTextureFileName[nSize-3];MAKE_LOWER(c1)
		char c2 = sTextureFileName[nSize-2];MAKE_LOWER(c2)
		char c3 = sTextureFileName[nSize-1];MAKE_LOWER(c3)

		// if it is png file we will use dxt3 internally since it contains alpha. 
		if(c1=='d' && c2=='d' && c3=='s')
			dwTextureFormat = EPixelFormat::Unkonwn;
		else if(c1=='p' && c2=='n' && c3=='g')
		{
			/** whether we treat png file as DXT3 by default. if the texture filename ends with "_32bits.png", we will load with D3DFMT_A8R8G8B8 instead of DXT3. 
			If one wants to ensure high resolution texture, use TGA format instead. */
			if(nSize>11 && sTextureFileName[nSize-11]=='_' && sTextureFileName[nSize-10]=='3' && sTextureFileName[nSize-9]=='2' && sTextureFileName[nSize-8]=='b')
			{
				dwTextureFormat = g_bEnable32bitsTexture ? EPixelFormat::A8R8G8B8 : EPixelFormat::DXT3;
				MipLevels = 1;
			}
			else if(sTextureFileName.find("blocks") != string::npos)
			{
				// if texture filename contains "blocks" either in folder name or filename, we will force using D3DFMT_A8R8G8B8 with full mip levels
				if (sTextureFileName.find("leaves") != string::npos || sTextureFileName.find("_mip1") != string::npos || sTextureFileName.find("torch") != string::npos)
				{
					MipLevels = 1;
				}
				else
				{
					MipLevels = 4;
				}
				if(sTextureFileName.find("_dxt") != string::npos)
				{
					if(sTextureFileName.find("_dxt1") != string::npos)
						dwTextureFormat = EPixelFormat::DXT1;
					else
						dwTextureFormat = EPixelFormat::DXT3;
				}
				else
					dwTextureFormat = EPixelFormat::A8R8G8B8;
			}
			else if(nSize>9 && sTextureFileName[nSize-9]=='_'  && sTextureFileName[nSize-8]=='d' && sTextureFileName[nSize-7]=='x' && sTextureFileName[nSize-6]=='t' && sTextureFileName[nSize-5]=='1')
			{
				/** if the texture filename ends with "_dxt1.png", we will load with DXT1 */
				dwTextureFormat = EPixelFormat::DXT1;
				MipLevels = 1;
			}
			else
			{
				dwTextureFormat = EPixelFormat::DXT3;
				MipLevels = 1;
			}
		}
		else if(c1=='j' && c2=='p' && c3=='g')
		{
			dwTextureFormat = EPixelFormat::DXT1;
			MipLevels = 1;
		}
	}
	if(pdwTextureFormat)
		*pdwTextureFormat = dwTextureFormat;
	if(pnMipLevels)
		*pnMipLevels = MipLevels;
}

///color key is always 0xff000000 for both StaticTexture and RenderTarget
HRESULT TextureEntityImpl::InitDeviceObjects()
{
	if(m_bIsInitialized)
		return S_OK;
	if(SurfaceType == TextureEntityImpl::RenderTarget || SurfaceType == TextureEntityImpl::DEPTHSTENCIL)
	{
		// render target is only created in RestoreDeviceObjects();
		return S_OK;
	}
	m_bIsInitialized = true;
	
	const string& sTextureFileName = GetLocalFileName();

	// if sTextureFileName is empty or texture is locked, this is most likely a remote texture that is just being downloaded, so it has empty local file name. 
	if(sTextureFileName.empty() || IsLocked())
	{
		return S_OK;
	}

	auto pRenderDevice = CGlobals::GetRenderDevice();

	if(SurfaceType == TextureEntityImpl::StaticTexture || SurfaceType == TextureEntityImpl::TerrainHighResTexture)
	{
		if(m_pTexture == 0)
		{
			// Load Texture sequentially
			EPixelFormat dwTextureFormat;
			UINT MipLevels;
			GetFormatAndMipLevelFromFileName(sTextureFileName, &dwTextureFormat, &MipLevels);
			if(SurfaceType == TextureEntityImpl::TerrainHighResTexture)
				MipLevels = 0;

			if(!IsAsyncLoad())
				return CreateTextureFromFile_Serial(NULL, NULL, (&m_pTexture), dwTextureFormat, MipLevels, GetColorKey() /*COLOR_XRGB(0,0,0) this makes black transparent*/);
			else
				return CreateTextureFromFile_Async(NULL, NULL, NULL, (void**)(&m_pTexture), dwTextureFormat, MipLevels, GetColorKey() /*COLOR_XRGB(0,0,0) this makes black transparent*/);
		}
		return S_OK;
	}
	else if(SurfaceType == TextureEntityImpl::SysMemoryTexture || SurfaceType == TextureEntityImpl::BlpTexture || SurfaceType == TextureEntityImpl::CubeTexture)
	{
		if(m_pTexture == 0)
		{
			if(!IsAsyncLoad())
				return CreateTextureFromFile_Serial(NULL, NULL, (&m_pTexture), EPixelFormat::Unkonwn, 0, GetColorKey() /*COLOR_XRGB(0,0,0) this makes black transparent*/);
			else
				return CreateTextureFromFile_Async(NULL, NULL, NULL, (void**)(&m_pTexture), EPixelFormat::Unkonwn, 0, GetColorKey() /*COLOR_XRGB(0,0,0) this makes black transparent*/);
		}
		return S_OK;
	}
	else if(SurfaceType == TextureEntityImpl::FlashTexture || SurfaceType == TextureEntityImpl::HTMLTexture)
	{
		// ignore flash texture
		return S_OK;
	}
	else if(SurfaceType == TextureEntityImpl::TextureSequence)
	{
		int nSize = (int)sTextureFileName.size();
		if(nSize > 8)
		{
			AnimatedTextureInfo* pInfo = GetAnimatedTextureInfo();
			int nTotalTextureSequence = -1;
			if(pInfo!=0)
				nTotalTextureSequence = pInfo->m_nFrameCount;

			if(nTotalTextureSequence>0)
			{
				if(m_pTextureSequence == 0)
				{
					m_pTextureSequence = new ITexture*[nTotalTextureSequence];
					memset(m_pTextureSequence, 0, sizeof(ITexture*)*nTotalTextureSequence);
				}

				// if there are texture sequence, export all bitmaps in the texture sequence. 
				string sTemp = sTextureFileName;
				int nOffset = nSize - 7;
				for (int i=1;i<=nTotalTextureSequence;++i)
				{
					int digit, tmp;
					tmp = i;
					digit = (int)(tmp/100);
					sTemp[nOffset+0] = ((char)('0'+digit));
					tmp = tmp - 100*digit;
					digit = (int)(tmp/10);
					sTemp[nOffset+1] = ((char)('0'+digit));
					tmp = tmp - 10*digit;
					digit = tmp;
					sTemp[nOffset+2] = ((char)('0'+digit));

					if(m_pTextureSequence[i-1] == 0)
					{
						// Load Texture sequentially
						EPixelFormat dwTextureFormat;
						UINT MipLevels;
						GetFormatAndMipLevelFromFileNameEx(sTextureFileName, &dwTextureFormat, &MipLevels);

						if(!IsAsyncLoad())
						{
							if(FAILED(CreateTextureFromFile_Serial(NULL, sTemp.c_str(), &(m_pTextureSequence[i-1]), dwTextureFormat, MipLevels, 
								COLOR_XRGB(0,0,0) /*COLOR_XRGB(0,0,0) this makes black transparent*/)))
							{
								OUTPUT_LOG("failed creating animated texture for %s", sTemp.c_str());
							}
						}
						else
						{
							if (FAILED(CreateTextureFromFile_Async(NULL, NULL, sTemp.c_str(), (void**)(&(m_pTextureSequence[i - 1])), dwTextureFormat, MipLevels,
								COLOR_XRGB(0,0,0) /*COLOR_XRGB(0,0,0) this makes black transparent*/)))
							{
								OUTPUT_LOG("failed creating animated texture for %s", sTemp.c_str());
							}
						}
					}
				}
			}
		}
		return S_OK;
	}
	return S_OK;
}

void TextureEntityImpl::CreateTexture(ITexture* pSrcTexture, EPixelFormat format, int width, int height, uint32_t MipLevels)
{

	if (SurfaceType != TextureEntityImpl::StaticTexture) return;
	// PERF1("CreateTexture");
	UnloadAsset();
	m_bIsInitialized = true;

	auto pRenderDevice = CGlobals::GetRenderDevice();

	

	m_pTexture  = pRenderDevice->CreateTexture(width, height,format,ETextureUsage::Default);
	if (m_pTexture == nullptr) return;

	if (!pSrcTexture->CopyTo(m_pTexture))
	{
		m_pTexture->DefRef();
		m_pTexture = nullptr;
		return;
	}

	SAFE_DELETE(m_pTextureInfo);
	m_pTextureInfo = new TextureInfo();
	m_pTextureInfo->m_width = width;
	m_pTextureInfo->m_height = height;
}

void TextureEntityImpl::SetSurface( IParaEngine::ITexture* pSurface )
{
	m_bIsValid = true;
	m_bIsInitialized = true;
	m_pTexture = pSurface;
}


// Define a function that matches the prototype of LPD3DXFILL3D
#if USE_DIRECTX_RENDERER
VOID WINAPI ColorFill (D3DXVECTOR4* pOut, const D3DXVECTOR2* pTexCoord, const D3DXVECTOR2* pTexelSize, LPVOID pData)
{
	*pOut = D3DXVECTOR4(0.f, 0.f, 0.0f, 0.0f);
}
#endif

HRESULT TextureEntityImpl::RestoreDeviceObjects()
{

	auto pRenderDevice = CGlobals::GetRenderDevice();

	if(m_bIsInitialized)
		return S_OK;
	if(SurfaceType == TextureEntityImpl::RenderTarget)
	{
		m_bIsInitialized = true;

		uint32_t width=0,height=0;
		// Default: create the render target with alpha. this will allow some mini scene graphs to render alpha pixels. 

		EPixelFormat format = EPixelFormat::A8R8G8B8;
		
		if(GetTextureInfo() != NULL)
		{
			width = GetTextureInfo()->m_width;
			height = GetTextureInfo()->m_height;
			if(width == 0 || height == 0)
			{
				width = pRenderDevice->GetBackbufferRenderTarget()->GetWidth();
				height = pRenderDevice->GetBackbufferRenderTarget()->GetHeight();
			}
			if(GetTextureInfo()->m_format == TextureInfo::FMT_X8R8G8B8)
			{
				format = EPixelFormat::X8R8G8B8;
			}
		}
		else
		{
			width = pRenderDevice->GetBackbufferRenderTarget()->GetWidth();
			height = pRenderDevice->GetBackbufferRenderTarget()->GetHeight();
		}

		/** if surface name contains  "_R32F", we will create using 32bits red channel format */
		if(GetKey().find("_R32F") != std::string::npos )
		{
			format = EPixelFormat::R32F;
		}
		if (GetKey().find("_HDR") != std::string::npos)
		{
			format = EPixelFormat::A16B16G16R16F;
		}


		m_pTexture = pRenderDevice->CreateTexture(width, height, format, ETextureUsage::RenderTarget);
		if (m_pTexture == nullptr)
		{
			if (format == EPixelFormat::A8R8G8B8 || format == EPixelFormat::R32F)
			{
				// try EPixelFormat::X8R8G8B8 if EPixelFormat::A8R8G8B8 is not supported; perhaps D3DXCreateTexture already secretly does this for us. 

				
				m_pTexture = pRenderDevice->CreateTexture(width, height, EPixelFormat::X8R8G8B8, ETextureUsage::RenderTarget);

				if (m_pTexture == nullptr)
				{
					OUTPUT_LOG("failed creating render target for %s", GetKey().c_str());
					m_bIsValid = false;
					return E_FAIL;
				}
			}
			else
			{
				OUTPUT_LOG("failed creating render target for %s", GetKey().c_str());
				m_bIsValid = false;
				return E_FAIL;
			}

		}
	}
	else if(SurfaceType == TextureEntityImpl::DEPTHSTENCIL)
	{
		m_bIsInitialized = true;

		uint32_t width=0,height=0;
		// Default: create the render target with alpha. this will allow some mini scene graphs to render alpha pixels. 

		EPixelFormat format = EPixelFormat::D24S8;

		if(GetTextureInfo() != NULL)
		{
			width = GetTextureInfo()->m_width;
			height = GetTextureInfo()->m_height;
			if(width == 0 || height == 0)
			{
				width = CGlobals::GetRenderDevice()->GetBackbufferRenderTarget()->GetWidth();
				height = CGlobals::GetRenderDevice()->GetBackbufferRenderTarget()->GetHeight();
			}
		}
		else
		{
			width = CGlobals::GetRenderDevice()->GetBackbufferRenderTarget()->GetWidth();
			height = CGlobals::GetRenderDevice()->GetBackbufferRenderTarget()->GetHeight();
		}

		/** if surface name contains  "_R32F", we will create using 32bits red channel format */
		if(GetKey().find("_R32F") != std::string::npos )
		{
			format = EPixelFormat::R32F;
		}

		m_pTexture = pRenderDevice->CreateTexture(width, height, format, ETextureUsage::DepthStencil);
		if (m_pTexture == nullptr)
		{
			OUTPUT_LOG("failed creating depth stencil for %s", GetKey().c_str());
			m_bIsValid = false;
			return E_FAIL;
		}
	}
	else if(SurfaceType == TextureEntityImpl::FlashTexture || SurfaceType == TextureEntityImpl::HTMLTexture)
	{
		// ignore flash texture
		return S_OK;
	}
	return S_OK;
}

HRESULT TextureEntityImpl::InvalidateDeviceObjects()
{
	if(SurfaceType == TextureEntityImpl::RenderTarget || (SurfaceType == TextureEntityImpl::DEPTHSTENCIL))
	{
		SAFE_RELEASE(m_pTexture);
		m_bIsInitialized = false;
	}
	return S_OK;
}

HRESULT TextureEntityImpl::DeleteDeviceObjects()
{
	m_bIsInitialized = false;
	switch(SurfaceType)
	{
	case TextureEntityImpl::FlashTexture:
		{
#ifdef USE_FLASH_MANAGER
			// delete flash texture
			CGlobals::GetAssetManager()->GetFlashManager().UnloadTexture(GetKey().c_str());
#endif
			break;
		}
	case TextureEntityImpl::TextureSurface:
	{
		// do nothing for pure texture surface
		//m_pSurface = NULL;
		break;
	}
	case TextureEntityImpl::HTMLTexture:
		{
			// web texture
			break;
		}
	case TextureEntityImpl::SysMemoryTexture:
		{
			//SAFE_RELEASE( m_pSurface );
			break;
		}
	case TextureEntityImpl::TextureSequence:
		{
			AnimatedTextureInfo* pInfo = GetAnimatedTextureInfo();
			int nTotalTextureSequence = -1;
			if(pInfo!=0)
				nTotalTextureSequence = pInfo->m_nFrameCount;
			if(nTotalTextureSequence>0 && m_pTextureSequence!=0)
			{
				for (int i=1;i<=nTotalTextureSequence;++i)
				{
					SAFE_RELEASE(m_pTextureSequence[i-1]);
				}
				SAFE_DELETE_ARRAY(m_pTextureSequence);
			}
			break;
		}
	default:
		{
			SAFE_RELEASE( m_pTexture );
			break;
		}
	}
	// We will not delete texture info since 2008.8.1: let us also delete the texture info as well. 
	// however, we will delete if it is called from the the scripting interface. see. 2008.10.9 change log. 
	// SAFE_DELETE(m_pTextureInfo);
	return S_OK;
}


void TextureEntityImpl::LoadImage(char *sBufMemFile, int sizeBuf, int &width, int &height, byte ** ppBuffer, bool bAlpha)
{

	auto pRenderDevice = CGlobals::GetRenderDevice();
	ITexture* pTexture = NULL;

	EPixelFormat format;
	if (bAlpha)
		format = EPixelFormat::A8R8G8B8;
	else
		format = EPixelFormat::R8G8B8;

	// read from file
	pTexture = pRenderDevice->CreateTexture(sBufMemFile, sizeBuf, format, 0);
	if(pTexture == nullptr)
		return;


	// set size
	width = pTexture->GetWidth();
	height = pTexture->GetHeight();

	byte *pBufferTemp = NULL;
	int nSize;
	if (bAlpha)
		nSize = width * height * 4;
	else
		nSize = width * height * 3;
	pBufferTemp = new byte[nSize];

	if(pBufferTemp==0)
		return;


	unsigned int pitch = 0;
	auto image = pTexture->GetImage(0);
	if(image)
	{
		byte *pImagePixels = (byte *)image->data;
		memcpy(pBufferTemp, pImagePixels, nSize);
		//
		*ppBuffer = pBufferTemp;
	}
	else
	{
		width = 0;
		height = 0;
		*ppBuffer = NULL;
		SAFE_DELETE_ARRAY(pBufferTemp);
	}

	SAFE_RELEASE( pTexture );
}

bool TextureEntityImpl::LoadImageOfFormatEx(const std::string& sTextureFileName, char *sBufMemFile, int sizeBuf, int &width, int &height, byte ** ppBuffer, int* pBytesPerPixel, int nFormat, ImageExtendInfo *exInfo)
{
#ifdef USE_FREEIMAGE
	if (nFormat <= 0)
		nFormat = PixelFormat32bppARGB;

	int nBytesPerPixel = 0;
	MemIO memIO((BYTE*)sBufMemFile, sizeBuf);
	FIBITMAP *dib = FreeImage_LoadFromHandle((FREE_IMAGE_FORMAT)GetFormatByFileName(sTextureFileName), &memIO, (fi_handle)&memIO);
	if (dib == 0)
	{
		OUTPUT_LOG("warning: can not load terrain region file %s \n", sTextureFileName.c_str());
		return false;
	}


	if (exInfo)
	{
		FITAG *tag = nullptr;
		FreeImage_GetMetadata(FIMD_EXIF_EXIF, dib, "FocalLength", &tag);
		if (tag)
		{
			// value type is  FIDT_RATIONAL  Two LONGs: the first represents the numerator of a fraction; the second, the denominator 
			if (FreeImage_GetTagType(tag) == FIDT_RATIONAL)
			{
				long* value = (long*)FreeImage_GetTagValue(tag);
				exInfo->FocalLength = (double)value[0] / value[1];
			}
		}
	}
	

	BITMAPINFOHEADER* pInfo = FreeImage_GetInfoHeader(dib);

	if (pInfo)
	{
		width = pInfo->biWidth;
		height = pInfo->biHeight;
		nBytesPerPixel = pInfo->biBitCount / 8;

		if (((nFormat == PixelFormat32bppARGB && pInfo->biBitCount != 32) || (nFormat == PixelFormat24bppRGB && pInfo->biBitCount != 24) || pInfo->biBitCount < 8)
			&& FreeImage_GetColorType(dib) == FIC_PALETTE)
		{
			auto oldDib = dib;
			if (nFormat == PixelFormat24bppRGB) {
				dib = FreeImage_ConvertTo24Bits(dib);
			}
			else {
				dib = FreeImage_ConvertTo32Bits(dib);
			}
			FreeImage_Unload(oldDib);
			pInfo = FreeImage_GetInfoHeader(dib);
			if (pInfo)
			{
				width = pInfo->biWidth;
				height = pInfo->biHeight;
				nBytesPerPixel = pInfo->biBitCount / 8;
			}
		}
	}
	else
		return false;


	BYTE* pPixels = FreeImage_GetBits(dib);
	if (pPixels)
	{
		int nSize = width * height * nBytesPerPixel;
		char* pData = new char[nSize];
		memcpy(pData, pPixels, nSize);
		if (ppBuffer)
			*ppBuffer = (byte*)pData;
		if (pBytesPerPixel)
			*pBytesPerPixel = nBytesPerPixel;
	}
	FreeImage_Unload(dib);
	return true;
#else
	return false;
#endif
}

bool TextureEntityImpl::LoadImageOfFormat(const std::string& sTextureFileName, char *sBufMemFile, int sizeBuf, int &width, int &height, byte ** ppBuffer, int* pBytesPerPixel, int nFormat)
{
	return TextureEntityImpl::LoadImageOfFormatEx(sTextureFileName, sBufMemFile, sizeBuf, width, height, ppBuffer, pBytesPerPixel, nFormat);
}


bool TextureEntityImpl::StretchRect(TextureEntityImpl * pSrcTexture, TextureEntityImpl * pDestTexture)
{

	if (pSrcTexture->GetTexture() != nullptr && pDestTexture->GetTexture() != nullptr)
	{
		return CGlobals::GetRenderDevice()->StretchRect(pSrcTexture->GetTexture(), pDestTexture->GetTexture(), nullptr, nullptr);
	}
	return false;
}



bool TextureEntityImpl::SetRenderTarget(int nIndex)
{

	bool res = CGlobals::GetRenderDevice()->SetRenderTarget(0, GetTexture());
	if (res && nIndex == 0){
		CGlobals::GetViewportManager()->GetActiveViewPort()->ApplyViewport();
	}
	return res;
}

void TextureEntityImpl::SetTexture(IParaEngine::ITexture* pSrcTexture)
{
	if (m_pTexture != pSrcTexture)
	{
		UnloadAsset();
		// take owner ship of the texture. 
		if (pSrcTexture){
			m_pTexture = pSrcTexture;
			m_pTexture->AddRef();
		}
		m_bIsInitialized = true;
	}
}

TextureEntity* TextureEntityImpl::CreateTexture(const char* pFileName, uint32 nMipLevels /*= 0*/, EPoolType dwCreatePool /*= D3DPOOL_MANAGED*/)
{
	auto pRenderDevice = CGlobals::GetRenderDevice();

	CParaFile file;
	if (file.OpenFile(pFileName, true))
	{
		ITexture* pTexture = pRenderDevice->CreateTexture(file.getBuffer(), file.getSize(), EPixelFormat::Unkonwn, 0);
		if (pTexture != NULL)
		{
			TextureEntityImpl* pTextureEntity = new TextureEntityImpl(AssetKey(pFileName));
			if (pTextureEntity)
			{
				pTextureEntity->SetTexture(pTexture);
				SAFE_RELEASE(pTexture);
				pTextureEntity->AddToAutoReleasePool();
				return pTextureEntity;
			}
		}
	}

	OUTPUT_LOG("warn: failed to create texture: %s \n", pFileName);
	return NULL;
}


bool TextureEntityImpl::SaveToFile(const char* filename, EPixelFormat dwFormat, int width, int height, UINT MipLevels /*= 1*/, DWORD Filter /*= D3DX_DEFAULT*/, Color ColorKey /*= 0*/)
{

#if USE_DIRECTX_RENDERER

	if (!GetTexture()) return false;
	LPDIRECT3DTEXTURE9 pSrcTexture = static_cast<TextureD3D9*>(GetTexture())->GetTexture();
	if (!pSrcTexture)
		return false;
	auto pRenderDevice = GETD3D(CGlobals::GetRenderDevice());
	DeviceTexturePtr_type pDestTexture = NULL;
	HRESULT hr = pRenderDevice->CreateTexture(width, height, 1, D3DUSAGE_AUTOGENMIPMAP, D3DMapping::toD3DFromat(dwFormat), D3DPOOL_MANAGED, &pDestTexture,NULL);
	if (SUCCEEDED(hr))
	{
		LPDIRECT3DSURFACE9 pSrcSurface = NULL;
		LPDIRECT3DSURFACE9 pDestSurface = NULL;
		hr = pSrcTexture->GetSurfaceLevel(0, &pSrcSurface);
		if (SUCCEEDED(hr))
		{
			hr = pDestTexture->GetSurfaceLevel(0, &pDestSurface);
			if (SUCCEEDED(hr))
			{
				hr = D3DXLoadSurfaceFromSurface(pDestSurface, NULL, NULL, pSrcSurface, NULL, NULL, Filter, ColorKey);
				SAFE_RELEASE(pDestSurface);
			}
			SAFE_RELEASE(pSrcSurface);
		}
		if (SUCCEEDED(hr))
		{
			// write file to disk
			hr = D3DXSaveTextureToFile(filename, D3DXIFF_DDS, pDestTexture, NULL);

			if (SUCCEEDED(hr))
			{
				if (MipLevels != 1)
				{
					// TODO: for some reason, this does not work. 
					LPDIRECT3DTEXTURE9 pDestTextureMipMapped = NULL;
					hr = D3DXCreateTextureFromFileEx(pRenderDevice,filename, D3DX_DEFAULT, D3DX_DEFAULT, MipLevels, D3DUSAGE_AUTOGENMIPMAP, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, Filter, D3DX_DEFAULT, 0, NULL, NULL, &pDestTextureMipMapped);
					if (SUCCEEDED(hr))
					{
						// generate all mip levels;
						pDestTextureMipMapped->GenerateMipSubLevels();

						/**
						IIRC GenerateMipSubLevels is only actually valid when D3DUSAGE_AUTOGENMIPMAP is specified. I think the docs are a bit vague about this.
						GenerateMipSubLevels exists to allow you tell the gfx driver when is a good time to generate the mip map levels. This is sometimes required as
						the AUTOGENMIPMAP flag only specifies that the mip maps are auto-generated at some point but not at any particular time. This autogeneration
						could occur after a Lock has taken place on the texture or at first use. If the texture is large mip-map generation may take a while so having
						the mip-maps auto generate at first use may not be the best idea. Therefore GenerateMipSubLevels exists for you to notify the driver that "now"
						is a good time to generate the mips if it hasn't already (e.g. "now" = while a loading screen is being displayed).
						*/
						hr = D3DXSaveTextureToFile(filename, D3DXIFF_DDS, pDestTextureMipMapped, NULL);
						if (FAILED(hr))
						{
							OUTPUT_LOG("warning: failed SaveTextureToFile -->GenerateMipSubLevels %s\n", filename);
						}

						OUTPUT_LOG("warning: MIP map are not supported for %s. TODO this in future s\n", filename);
					}
					else
					{
						OUTPUT_LOG("warning: failed SaveTextureToFile -->D3DXCreateTextureFromFileEx %s\n", filename);
					}
					SAFE_RELEASE(pDestTextureMipMapped);
				}
			}
			else
			{
				OUTPUT_LOG("warning: failed SaveTextureToFile %s\n", filename);
			}
		}
		SAFE_RELEASE(pDestTexture);
	}
	return SUCCEEDED(hr);

#else
	return E_FAIL;
#endif
}

TextureEntity* TextureEntityImpl::CreateTexture(const uint8 * pTexels, int width, int height, int rowLength, int bytesPerPixel, uint32 nMipLevels /*= 0*/, EPoolType dwCreatePool /*= D3DPOOL_MANAGED*/, DWORD nFormat /*= 0*/)
{
	auto pRenderDevice = CGlobals::GetRenderDevice();
	ITexture* pTexture = NULL;

	if (!pTexels)
		return 0;

	if (bytesPerPixel == 4)
	{
		pTexture = pRenderDevice->CreateTexture(width, height,EPixelFormat::A8R8G8B8,ETextureUsage::Default);
		if (!pTexture)
		{
			OUTPUT_LOG("failed creating terrain texture\n");
		}
		else
		{
			pTexture->UpdateImage(0, 0, 0, width, height, pTexels);
		}
	}
	else if (bytesPerPixel == 3)
	{
		pTexture = pRenderDevice->CreateTexture(width, height, EPixelFormat::A8R8G8B8, ETextureUsage::Default);
		if (!pTexture)
		{
			OUTPUT_LOG("failed creating terrain texture\n");
		}
		else
		{
			unsigned int pitch = 4 * width;
			unsigned char* pp = new unsigned char[width*height*4];
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
				index += pitch - (width * 4);
			}
			pTexture->UpdateImage(0, 0, 0, width, height, pp);
			delete[] pp;

		}
	}
	else if (bytesPerPixel == 1)
	{
		HRESULT hr;
		// whether D3DFMT_A8 is supported. -1 is unknown, 0 is not supported, 1 is supported. 
		static int nSupportAlphaTexture = -1;
#ifdef USE_DIRECTX_RENDERER
		if (nSupportAlphaTexture == -1)
		{
			D3DFORMAT nDesiredFormat = D3DFMT_A8;
			hr = D3DXCheckTextureRequirements(GETD3D(pRenderDevice),NULL, NULL, NULL, 0, &nDesiredFormat, D3DMapping::toD3DPool(dwCreatePool));
			if (SUCCEEDED(hr))
			{
				if (nDesiredFormat == D3DFMT_A8)
					nSupportAlphaTexture = 1;
				else
				{
					OUTPUT_LOG("warning: D3DFMT_A8 is not supported during terrain mask file creation, try using D3DFMT_A8R8G8B8\n");
					nSupportAlphaTexture = 0;
				}
			}
			else
			{
				OUTPUT_LOG("error: D3DXCheckTextureRequirements with D3DFMT_A8 failed.\n");
				nSupportAlphaTexture = -2;
			}
		}
#endif
		if (nSupportAlphaTexture == 1)
		{
			// D3DFMT_A8 is supported
			pTexture = pRenderDevice->CreateTexture(width, height, EPixelFormat::A8, ETextureUsage::Default);
			if (!pTexture)
			{
				OUTPUT_LOG("failed creating terrain texture\n");
			}
			else
			{
				pTexture->UpdateImage(0, 0, 0, width, height, pTexels);
			}
		}
		else if (nSupportAlphaTexture == 0)
		{
			// D3DFMT_A8 is not supported, try D3DFMT_A8R8G8B8
			pTexture = pRenderDevice->CreateTexture(width, height, EPixelFormat::A8R8G8B8, ETextureUsage::Default);
			if (!pTexture)
			{
				OUTPUT_LOG("failed creating alpha terrain texture\n");
			}
			else
			{
				unsigned int pitch = 4 * width;
				unsigned char* pp = new unsigned char[width*height * 4];
				int nSize = width*height;
				DWORD* pData = (DWORD*)(pp);
				for (int x = 0; x < nSize; ++x)
				{
					pData[x] = (pTexels[x]) << 24;
				}
				pTexture->UpdateImage(0, 0, 0, width, height,pp);
				delete[] pp;
			}
		}
	}
	else
	{
		OUTPUT_LOG("error: Unsupported texture format (bits per pixel must be 8,24, or 32)\n");
	}
	if (pTexture != NULL)
	{
		TextureEntityImpl* pTextureEntity = new TextureEntityImpl();
		if (pTextureEntity)
		{
			pTextureEntity->SetTexture(pTexture);
			SAFE_RELEASE(pTexture);
			pTextureEntity->AddToAutoReleasePool();
			return pTextureEntity;
		}
	}
	return NULL;
}

HRESULT TextureEntityImpl::LoadFromMemory(const char* buffer, DWORD nFileSize, UINT nMipLevels, EPixelFormat format, void** ppTexture_)
{


	HRESULT hr = S_OK;
	ITexture** ppTexture = (ITexture**)ppTexture_;
	auto pRenderDevice = CGlobals::GetRenderDevice();

	ITexture* pTexture = NULL;

	switch (SurfaceType)
	{
	case TextureEntity::SysMemoryTexture:
	{
		//D3DXIMAGE_INFO SrcInfo;
		//if (GetTextureInfo())
		//{
		//	SrcInfo.Width = GetTextureInfo()->GetWidth();
		//	SrcInfo.Height = GetTextureInfo()->GetHeight();
		//	SrcInfo.Format = GetD3DFormat();
		//}
		//else
		//{
		//	if (FAILED(hr = D3DXGetImageInfoFromFileInMemory(buffer, nFileSize, &SrcInfo)))
		//	{
		//		OUTPUT_LOG("Could not D3DXGetImageInfoFromFileInMemory %s\n", GetKey().c_str());
		//		break;
		//	}
		//	SrcInfo.Format = D3DFMT_X8R8G8B8;
		//}

		//hr = pRenderDevice->CreateOffscreenPlainSurface(SrcInfo.Width, SrcInfo.Height, SrcInfo.Format, D3DPOOL_SYSTEMMEM, &(m_pSurface), NULL);
		//if (SUCCEEDED(hr))
		//{
		//	// Load the image again, this time with D3D to load directly to the surface
		//	hr = D3DXLoadSurfaceFromFileInMemory(m_pSurface, NULL, NULL, buffer, nFileSize, NULL, D3DX_FILTER_NONE,
		//		GetColorKey() /*COLOR_XRGB(0,0,0) this makes black transparent*/, NULL);
		//	if (SUCCEEDED(hr))
		//	{
		//		if (SrcInfo.Format == D3DFMT_A8R8G8B8)
		//		{
		//			/** discovered by clayman, xizhi: for unknown reasons, D3DXLoadSurfaceFromFileInMemory loaded alpha has randomly incorrect alpha values.
		//			* the following code fix it in most cases.
		//			*/
		//			LPDIRECT3DSURFACE9 pCursorSurface = m_pSurface;
		//			if (pCursorSurface)
		//			{
		//				D3DLOCKED_RECT data;
		//				pCursorSurface->LockRect(&data, NULL, 0);
		//				uint32* pData = (uint32*)data.pBits;
		//				int count = SrcInfo.Width * SrcInfo.Height;
		//				for (int i = 0; i<count; i++)
		//				{
		//					uint32 color = *pData;
		//					uint32 alpha = color >> 24;
		//					if (alpha > 127)
		//					{
		//						color |= 0xff000000;
		//					}
		//					else
		//					{
		//						color &= 0xffffff;
		//					}
		//					*pData = color;
		//					pData++;
		//				}
		//				pCursorSurface->UnlockRect();
		//			}
		//		}
		//	}
		//	else
		//	{
		//		OUTPUT_LOG("Could not load the following bitmap to a surface %s\n", GetKey().c_str());
		//	}
		//}
		//else
		//{
		//	SetState(AssetEntity::ASSET_STATE_FAILED_TO_LOAD);
		//	OUTPUT_LOG("Unable to create a new surface for the bitmap %s\n", GetKey().c_str());
		//}
		//break;
	}
	case TextureEntity::CubeTexture:
	{
		//hr = D3DXCreateCubeTextureFromFileInMemoryEx(pRenderDevice,buffer, nFileSize,
		//	D3DX_DEFAULT, D3DX_FROM_FILE /**D3DX_FROM_FILE:  mip-mapping from file */, 0, D3DFMT_UNKNOWN,
		//	D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT,
		//	0, NULL, NULL, &(m_pCubeTexture));
		//if (FAILED(hr))
		//{
		//	SetState(AssetEntity::ASSET_STATE_FAILED_TO_LOAD);
		//	OUTPUT_LOG("failed creating cube texture for %s\n", GetKey().c_str());
		//}
		//break;
	}
	default:
	{

		pTexture = pRenderDevice->CreateTexture(buffer, nFileSize, format, m_dwColorKey);
		if (pTexture)
		{
			/** we will only set LOD if the texture file is larger than 40KB */
			//if (nMipLevels != 1 && TextureEntity::g_nTextureLOD>0 && nFileSize>40000)
			//{
			//	//pTexture->SetLOD(TextureEntity::g_nTextureLOD);
			//}
		}
		else
		{
			SetState(AssetEntity::ASSET_STATE_FAILED_TO_LOAD);
			OUTPUT_LOG("failed creating texture for %s\n", GetKey().c_str());
		}
		if (ppTexture)
		{
			SAFE_RELEASE(*ppTexture);
			*ppTexture = pTexture;
		}
		else
		{
			SAFE_RELEASE(m_pTexture);
			m_pTexture = pTexture;
		}

		// update texture info
		if (m_pTextureInfo)
		{
			if (m_pTextureInfo->m_width == -1 || m_pTextureInfo->m_height == -1)
			{
				if (pTexture != 0)
				{
					m_pTextureInfo->m_width = pTexture->GetWidth();
					m_pTextureInfo->m_height = pTexture->GetHeight();
				}
			}
		}
		break;
	}
	}
	return hr;
}



#endif
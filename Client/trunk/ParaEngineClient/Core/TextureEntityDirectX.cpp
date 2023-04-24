//-----------------------------------------------------------------------------
// Class:	TextureEntityDirectX
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2004.3.8
// Revised: 2006.7.12, 2009.8.18(AsyncLoader added)
//-----------------------------------------------------------------------------
#include "ParaEngine.h"

#ifdef USE_DIRECTX_RENDERER
/**
Which DXT Compression to Use?
Obviously, there are some trade-offs between the different formats which make them better or worse for different types of images. Some general rules of thumb for good use of DXT textures are as follows:
If your image has no alpha, use DXT1 compression. Using DXT3/5 will double your image size over DXT1 and not gain anything. 
If your image has 1-bit (on or off) alpha information, use DXT1 with one-bit alpha. If the DXT1 image quality is too low and you don't mind doubling image size, use DXT3 or DXT5 (which one doesn't matter, they'll give the same results). 
If your image has smooth gradations of alpha (fading in/out slowly), DXT5 is almost certainly your best bet, as it will give you the most accurate transparency representation. 
If your image has sharp transitions between multiple alpha levels (one pixel is 100%, the next one is 50%, and another neighbor is 12%), DXT3 is probably your best bet. You may want to compare the alpha results in DXT1, DXT3 and DXT5 compression, however, to make sure. 
*/
#include "DirectXEngine.h"
#ifdef USE_FLASH_MANAGER
#include "FlashTextureManager.h"
#endif
#include "HtmlBrowserManager.h"
#include "ParaWorldAsset.h"
#ifdef USE_FREEIMAGE
#include "FreeImageMemIO.h"
#endif
#include <gdiplus.h>
#include "ContentLoaders.h"
#include "AsyncLoader.h"
#include "ViewportManager.h"
#include "TextureEntityDirectX.h"
#include "StringHelper.h"

#ifdef PARAENGINE_CLIENT
	#include "memdebug.h"
#endif

// to lower case
#define MAKE_LOWER(c)  if((c)>='A' && (c)<='Z'){(c) = (c)-'A'+'a';}

using namespace ParaEngine;

namespace ParaEngine
{
	extern int globalTime;
}


TextureEntityDirectX::TextureEntityDirectX(const AssetKey& key)
:m_pTexture(NULL), TextureEntity(key)
{
}

TextureEntityDirectX::TextureEntityDirectX()
: m_pTexture(NULL), TextureEntity()
{
}

TextureEntityDirectX::~TextureEntityDirectX()
{
	if (m_bIsInitialized)
	{
		UnloadAsset();
	}
}

D3DFORMAT TextureEntityDirectX::GetD3DFormat()
{
	auto format = m_pTextureInfo->GetFormat();
	if (format == TextureInfo::FMT_A8R8G8B8)
		return D3DFMT_A8R8G8B8;
	else if (format == TextureInfo::FMT_X8R8G8B8)
		return D3DFMT_X8R8G8B8;
	else if (format == TextureInfo::FMT_UNKNOWN)
		return D3DFMT_UNKNOWN;
	else
		return D3DFMT_X8R8G8B8;
}

LPDIRECT3DSURFACE9 TextureEntityDirectX::GetSurface()
{
	if(SurfaceType == SysMemoryTexture || SurfaceType == TextureSurface)
	{
		LoadAsset();
		return m_pSurface;
	}
	else
		return NULL;
}

DeviceTexturePtr_type TextureEntityDirectX::GetTexture()
{
	++m_nHitCount;
	DeviceTexturePtr_type tex = NULL;
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

bool TextureEntityDirectX::IsLoaded()
{
	return GetTexture()!=0;
}

const TextureEntityDirectX::TextureInfo* TextureEntityDirectX::GetTextureInfo()
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
				LPDIRECT3DTEXTURE9 pTexture = NULL;
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
						D3DSURFACE_DESC desc;
						if(SUCCEEDED(pTexture->GetLevelDesc(0, &desc)))
						{
							m_pAnimatedTextureInfo->m_width = desc.Width;
							m_pAnimatedTextureInfo->m_height = desc.Height;
						}
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
			break;
		}
	default:
		{
			// for other texture types
			if(m_pTextureInfo==NULL)
			{
				m_pTextureInfo = new TextureInfo();
				if( (SurfaceType == TextureEntityDirectX::RenderTarget) || 
					(SurfaceType == TextureEntityDirectX::DEPTHSTENCIL) ||
					(SurfaceType == TextureEntityDirectX::SysMemoryTexture))
				{
					if(GetTexture())
					{
						D3DSURFACE_DESC desc;
						if(SUCCEEDED(m_pSurface->GetDesc(&desc)))
						{
							m_pTextureInfo->m_width = desc.Width;
							m_pTextureInfo->m_height = desc.Height;
						}
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
						D3DSURFACE_DESC desc;
						if (SUCCEEDED(GetSurface()->GetDesc(&desc)))
						{
							m_pTextureInfo->m_width = desc.Width;
							m_pTextureInfo->m_height = desc.Height;
						}
					}
					else if(m_pTexture)
					{
						D3DSURFACE_DESC desc;
						if(SUCCEEDED(m_pTexture->GetLevelDesc(0, &desc)))
						{
							m_pTextureInfo->m_width = desc.Width;
							m_pTextureInfo->m_height = desc.Height;
						}
					}
				}
			}
			break;
		}
	}
	
	return m_pTextureInfo;
}

HRESULT TextureEntityDirectX::CreateTextureFromFile_Serial(IDirect3DDevice9* pDev, const char* sFileName, IDirect3DTexture9** ppTexture, D3DFORMAT dwTextureFormat, UINT nMipLevels, Color dwColorKey )
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

void TextureEntityDirectX::GetFormatAndMipLevelFromFileNameEx(const string& sTextureFileName, D3DFORMAT* pdwTextureFormat, UINT* pnMipLevels)
{
	int nSize = (int)sTextureFileName.size();
	D3DFORMAT dwTextureFormat = D3DFMT_UNKNOWN;
	UINT MipLevels = D3DX_DEFAULT;
	if(nSize>4)
	{
		char c1 = sTextureFileName[nSize-3];MAKE_LOWER(c1)
		char c2 = sTextureFileName[nSize-2];MAKE_LOWER(c2)
		char c3 = sTextureFileName[nSize-1];MAKE_LOWER(c3)

		// if it is png file we will use dxt3 internally since it contains alpha. 
		if(c1=='d' && c2=='d' && c3=='s')
			dwTextureFormat = D3DFMT_UNKNOWN;
		else if(c1=='p' && c2=='n' && c3=='g')
		{
			/** whether we treat png file as DXT3 by default. if the texture filename ends with "_32bits.png", we will load with D3DFMT_A8R8G8B8 instead of DXT3. 
			If one wants to ensure high resolution texture, use TGA format instead. */
			if(sTextureFileName.find("_32bits") != string::npos)
			{
				dwTextureFormat = D3DFMT_A8R8G8B8;
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
				dwTextureFormat = D3DFMT_DXT1;
				MipLevels = 1;
			}
			else
			{
				dwTextureFormat = D3DFMT_DXT3;
				MipLevels = 1;
			}
			
		}
		else if(c1=='j' && c2=='p' && c3=='g')
		{
			dwTextureFormat = D3DFMT_DXT1;
			MipLevels = 1;
		}
	}
	if(pdwTextureFormat)
		*pdwTextureFormat = dwTextureFormat;
	if(pnMipLevels)
		*pnMipLevels = MipLevels;
}

void TextureEntityDirectX::GetFormatAndMipLevelFromFileName(const string& sTextureFileName, D3DFORMAT* pdwTextureFormat, UINT* pnMipLevels)
{
	int nSize = (int)sTextureFileName.size();
	D3DFORMAT dwTextureFormat = D3DFMT_UNKNOWN;
	UINT MipLevels = D3DX_DEFAULT;
	if(nSize>4)
	{
		char c1 = sTextureFileName[nSize-3];MAKE_LOWER(c1)
		char c2 = sTextureFileName[nSize-2];MAKE_LOWER(c2)
		char c3 = sTextureFileName[nSize-1];MAKE_LOWER(c3)

		// if it is png file we will use dxt3 internally since it contains alpha. 
		if(c1=='d' && c2=='d' && c3=='s')
			dwTextureFormat = D3DFMT_UNKNOWN;
		else if(c1=='p' && c2=='n' && c3=='g')
		{
			/** whether we treat png file as DXT3 by default. if the texture filename ends with "_32bits.png", we will load with D3DFMT_A8R8G8B8 instead of DXT3. 
			If one wants to ensure high resolution texture, use TGA format instead. */
			if(nSize>11 && sTextureFileName[nSize-11]=='_' && sTextureFileName[nSize-10]=='3' && sTextureFileName[nSize-9]=='2' && sTextureFileName[nSize-8]=='b')
			{
				dwTextureFormat = g_bEnable32bitsTexture ? D3DFMT_A8R8G8B8 : D3DFMT_DXT3;
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
						dwTextureFormat = D3DFMT_DXT1;
					else
						dwTextureFormat = D3DFMT_DXT3;
				}
				else
					dwTextureFormat = D3DFMT_A8R8G8B8;
			}
			else if(nSize>9 && sTextureFileName[nSize-9]=='_'  && sTextureFileName[nSize-8]=='d' && sTextureFileName[nSize-7]=='x' && sTextureFileName[nSize-6]=='t' && sTextureFileName[nSize-5]=='1')
			{
				/** if the texture filename ends with "_dxt1.png", we will load with DXT1 */
				dwTextureFormat = D3DFMT_DXT1;
				MipLevels = 1;
			}
			else
			{
				dwTextureFormat = D3DFMT_DXT3;
				MipLevels = 1;
			}
		}
		else if(c1=='j' && c2=='p' && c3=='g')
		{
			dwTextureFormat = D3DFMT_DXT1;
			MipLevels = 1;
		}
	}
	if(pdwTextureFormat)
		*pdwTextureFormat = dwTextureFormat;
	if(pnMipLevels)
		*pnMipLevels = MipLevels;
}

///color key is always 0xff000000 for both StaticTexture and RenderTarget
HRESULT TextureEntityDirectX::InitDeviceObjects()
{
	if(m_bIsInitialized)
		return S_OK;
	if(SurfaceType == TextureEntityDirectX::RenderTarget || SurfaceType == TextureEntityDirectX::DEPTHSTENCIL)
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

	LPDIRECT3DDEVICE9 pd3dDevice = CGlobals::GetRenderDevice();

	if(SurfaceType == TextureEntityDirectX::StaticTexture || SurfaceType == TextureEntityDirectX::TerrainHighResTexture)
	{
		if(m_pTexture == 0)
		{
			// Load Texture sequentially
			D3DFORMAT dwTextureFormat;
			UINT MipLevels;
			GetFormatAndMipLevelFromFileName(sTextureFileName, &dwTextureFormat, &MipLevels);
			if(SurfaceType == TextureEntityDirectX::TerrainHighResTexture)
				MipLevels = 0;

			if(!IsAsyncLoad())
				return CreateTextureFromFile_Serial(NULL, NULL, (&m_pTexture), dwTextureFormat, MipLevels, GetColorKey() /*COLOR_XRGB(0,0,0) this makes black transparent*/);
			else
				return CreateTextureFromFile_Async(NULL, NULL, NULL, (void**)(&m_pTexture), dwTextureFormat, MipLevels, GetColorKey() /*COLOR_XRGB(0,0,0) this makes black transparent*/);
		}
		return S_OK;
	}
	else if(SurfaceType == TextureEntityDirectX::SysMemoryTexture || SurfaceType == TextureEntityDirectX::BlpTexture || SurfaceType == TextureEntityDirectX::CubeTexture)
	{
		if(m_pSurface == 0)
		{
			if(!IsAsyncLoad())
				return CreateTextureFromFile_Serial(NULL, NULL, (&m_pTexture), D3DFMT_UNKNOWN, 0, GetColorKey() /*COLOR_XRGB(0,0,0) this makes black transparent*/);
			else
				return CreateTextureFromFile_Async(NULL, NULL, NULL, (void**)(&m_pTexture), D3DFMT_UNKNOWN, 0, GetColorKey() /*COLOR_XRGB(0,0,0) this makes black transparent*/);
		}
		return S_OK;
	}
	else if(SurfaceType == TextureEntityDirectX::FlashTexture || SurfaceType == TextureEntityDirectX::HTMLTexture)
	{
		// ignore flash texture
		return S_OK;
	}
	else if(SurfaceType == TextureEntityDirectX::TextureSequence)
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
					m_pTextureSequence = new LPDIRECT3DTEXTURE9[nTotalTextureSequence];
					memset(m_pTextureSequence, 0, sizeof(LPDIRECT3DTEXTURE9)*nTotalTextureSequence);
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
						D3DFORMAT dwTextureFormat;
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

void TextureEntityDirectX::CreateTexture(LPDIRECT3DTEXTURE9 pSrcTexture, D3DFORMAT dwFormat, int width, int height, UINT MipLevels)
{
	// only StaticTexture should call this function. 
	if(SurfaceType == TextureEntityDirectX::StaticTexture)
	{
		// PERF1("CreateTexture");
		UnloadAsset();
		m_bIsInitialized = true;

		LPDIRECT3DDEVICE9 pd3dDevice =  CGlobals::GetRenderDevice();
		HRESULT hr = D3DXCreateTexture(pd3dDevice, width, height,  MipLevels, D3DUSAGE_AUTOGENMIPMAP, dwFormat, D3DPOOL_MANAGED, &m_pTexture);
		if( SUCCEEDED(hr) )
		{
			LPDIRECT3DSURFACE9 pSrcSurface = NULL;
			LPDIRECT3DSURFACE9 pDestSurface = NULL;
			hr = pSrcTexture->GetSurfaceLevel(0, &pSrcSurface);
			if( SUCCEEDED(hr) )
			{
				hr = m_pTexture->GetSurfaceLevel(0, &pDestSurface);
				if( SUCCEEDED(hr) )
				{
					//PERF1("D3DXLoadSurfaceFromSurface");

					//byte * pBuffer = new byte[width*height*4];
					//D3DLOCKED_RECT lr;
					//hr = pSrcSurface->LockRect(&lr, NULL, D3DLOCK_READONLY);
					//if( SUCCEEDED(hr) )
					//{
					//	memcpy(pBuffer, lr.pBits, width*height*4);	
					//	pSrcSurface->UnlockRect();
					//}
					//// RECT rect = {0,0,width, height};
					//// D3DX_FILTER_NONE
					//// D3DXLoadSurfaceFromMemory(pDestSurface, NULL, NULL, pBuffer, D3DFMT_A8R8G8B8, width*4, NULL, &rect,D3DX_DEFAULT, 0);
					//CLogger::GetSingleton().Write((const char*)pBuffer, width*height*4);
					//delete [] pBuffer;

					// D3DXLoadSurfaceFromSurface is super slow. takes 0.042 seconds to run on 2.5 GHZ CPU. 
					// this is because fetching data from render target is super slow. 
					hr = D3DXLoadSurfaceFromSurface(pDestSurface, NULL, NULL, pSrcSurface, NULL, NULL, D3DX_DEFAULT, 0);
					SAFE_RELEASE(pDestSurface);
				}
				SAFE_RELEASE(pSrcSurface);
			}
			if( SUCCEEDED(hr) )
			{
				// generate all mip levels
				// TODO:
				// m_pTexture->SetAutoGenFilterType(D3DTEXF_NONE);
				m_pTexture->GenerateMipSubLevels();
				
				/** we will only set LOD if the texture file is larger than 40KB */
				if(MipLevels!=1 && g_nTextureLOD>0 && width>128)
				{
					m_pTexture->SetLOD(g_nTextureLOD);
				}

				SAFE_DELETE(m_pTextureInfo);
				m_pTextureInfo = new TextureInfo();
				m_pTextureInfo->m_width = width;
				m_pTextureInfo->m_height = height;
			}
		}
	}
}

void TextureEntityDirectX::SetSurface( LPDIRECT3DSURFACE9 pSurface )
{
	m_bIsValid = true;
	m_bIsInitialized = true;
	m_pSurface = pSurface;
}


// Define a function that matches the prototype of LPD3DXFILL3D
VOID WINAPI ColorFill (D3DXVECTOR4* pOut, const D3DXVECTOR2* pTexCoord, const D3DXVECTOR2* pTexelSize, LPVOID pData)
{
	*pOut = D3DXVECTOR4(0.f, 0.f, 0.0f, 0.0f);
}

HRESULT TextureEntityDirectX::RestoreDeviceObjects()
{
	if(m_bIsInitialized)
		return S_OK;
	if(SurfaceType == TextureEntityDirectX::RenderTarget)
	{
		m_bIsInitialized = true;

		DWORD width,height;
		// Default: create the render target with alpha. this will allow some mini scene graphs to render alpha pixels. 

		D3DFORMAT format = D3DFMT_A8R8G8B8;
		
		if(GetTextureInfo() != NULL)
		{
			width = GetTextureInfo()->m_width;
			height = GetTextureInfo()->m_height;
			if(width == 0 || height == 0)
			{
				width = CGlobals::GetDirectXEngine().m_d3dsdBackBuffer.Width;
				height = CGlobals::GetDirectXEngine().m_d3dsdBackBuffer.Height;
			}
			if(GetTextureInfo()->m_format == TextureInfo::FMT_X8R8G8B8)
			{
				format = D3DFMT_X8R8G8B8;
			}
		}
		else
		{
			width = CGlobals::GetDirectXEngine().m_d3dsdBackBuffer.Width;
			height = CGlobals::GetDirectXEngine().m_d3dsdBackBuffer.Height;
		}

		/** if surface name contains  "_R32F", we will create using 32bits red channel format */
		if(GetKey().find("_R32F") != std::string::npos )
		{
			format = D3DFMT_R32F;
		}
		if (GetKey().find("_HDR") != std::string::npos)
		{
			format = D3DFMT_A16B16G16R16F;
		}

		if( FAILED(D3DXCreateTexture(CGlobals::GetRenderDevice(),width,height,1,D3DUSAGE_RENDERTARGET,
			format, D3DPOOL_DEFAULT,&m_pTexture)) )
		{
			if(format == D3DFMT_A8R8G8B8 || format == D3DFMT_R32F)
			{
				// try D3DFMT_X8R8G8B8 if FMT_A8R8G8B8 is not supported; perhaps D3DXCreateTexture already secretly does this for us. 
				if( FAILED(D3DXCreateTexture(CGlobals::GetRenderDevice(),width,height,1,D3DUSAGE_RENDERTARGET,
					D3DFMT_X8R8G8B8, D3DPOOL_DEFAULT,&m_pTexture)) )
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
		if(m_bIsValid && m_pTexture)
		{
			// Fill the texture with 0-alpha using D3DXFillTexture
			if( FAILED(D3DXFillTexture (m_pTexture, ColorFill, NULL)) )
			{
				OUTPUT_LOG("failed filling render target %s with alpha value", GetKey().c_str());
			}
		}
	}
	else if(SurfaceType == TextureEntityDirectX::DEPTHSTENCIL)
	{
		m_bIsInitialized = true;

		DWORD width,height;
		// Default: create the render target with alpha. this will allow some mini scene graphs to render alpha pixels. 

		D3DFORMAT format = D3DFMT_D24S8;

		if(GetTextureInfo() != NULL)
		{
			width = GetTextureInfo()->m_width;
			height = GetTextureInfo()->m_height;
			if(width == 0 || height == 0)
			{
				width = CGlobals::GetDirectXEngine().m_d3dsdBackBuffer.Width;
				height = CGlobals::GetDirectXEngine().m_d3dsdBackBuffer.Height;
			}
		}
		else
		{
			width = CGlobals::GetDirectXEngine().m_d3dsdBackBuffer.Width;
			height = CGlobals::GetDirectXEngine().m_d3dsdBackBuffer.Height;
		}

		/** if surface name contains  "_R32F", we will create using 32bits red channel format */
		if(GetKey().find("_R32F") != std::string::npos )
		{
			format = D3DFMT_R32F;
		}

		if( FAILED(D3DXCreateTexture(CGlobals::GetRenderDevice(),width,height,1,D3DUSAGE_DEPTHSTENCIL,
			format, D3DPOOL_DEFAULT,&m_pTexture)) )
		{
			OUTPUT_LOG("failed creating depth stencil for %s", GetKey().c_str());
			m_bIsValid = false;
			return E_FAIL;
		}
	}
	else if(SurfaceType == TextureEntityDirectX::FlashTexture || SurfaceType == TextureEntityDirectX::HTMLTexture)
	{
		// ignore flash texture
		return S_OK;
	}
	return S_OK;
}

HRESULT TextureEntityDirectX::InvalidateDeviceObjects()
{
	if(SurfaceType == TextureEntityDirectX::RenderTarget || (SurfaceType == TextureEntityDirectX::DEPTHSTENCIL))
	{
		SAFE_RELEASE(m_pTexture);
		m_bIsInitialized = false;
	}
	return S_OK;
}

HRESULT TextureEntityDirectX::DeleteDeviceObjects()
{
	m_bIsInitialized = false;
	switch(SurfaceType)
	{
	case TextureEntityDirectX::FlashTexture:
		{
#ifdef USE_FLASH_MANAGER
			// delete flash texture
			CGlobals::GetAssetManager()->GetFlashManager().UnloadTexture(GetKey().c_str());
#endif
			break;
		}
	case TextureEntityDirectX::TextureSurface:
	{
		// do nothing for pure texture surface
		m_pSurface = NULL;
		break;
	}
	case TextureEntityDirectX::HTMLTexture:
		{
			// web texture
			break;
		}
	case TextureEntityDirectX::SysMemoryTexture:
		{
			SAFE_RELEASE( m_pSurface );
			break;
		}
	case TextureEntityDirectX::TextureSequence:
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


void TextureEntityDirectX::LoadImage(char *sBufMemFile, int sizeBuf, int &width, int &height, byte ** ppBuffer, bool bAlpha)
{
	HRESULT hr;
	D3DSURFACE_DESC desc;
	IDirect3DTexture9* pTexture = NULL;

	D3DFORMAT d3dFormat;
	if (bAlpha)
		d3dFormat = D3DFMT_A8R8G8B8;
	else
		d3dFormat = D3DFMT_R8G8B8;

	// read from file
	hr = D3DXCreateTextureFromFileInMemoryEx( CGlobals::GetRenderDevice(), sBufMemFile, sizeBuf,
		0, 0, 1, 0, 
		d3dFormat, D3DPOOL_SCRATCH, 
		D3DX_FILTER_NONE, D3DX_FILTER_NONE,
		0, NULL, NULL, &pTexture );
	if( FAILED(hr) )
		return;

	pTexture->GetLevelDesc( 0, &desc ); 

	// set size
	width = desc.Width;
	height = desc.Height;

	byte *pBufferTemp = NULL;
	int nSize;
	if (bAlpha)
		nSize = width * height * 4;
	else
		nSize = width * height * 3;
	pBufferTemp = new byte[nSize];

	if(pBufferTemp==0)
		return;
	D3DLOCKED_RECT lockedRect;

	hr = pTexture->LockRect( 0, &lockedRect, NULL, D3DLOCK_READONLY );
	if( SUCCEEDED(hr) )
	{
		byte *pImagePixels = (byte *) lockedRect.pBits;
		memcpy(pBufferTemp, pImagePixels, nSize);
		//
		*ppBuffer = pBufferTemp;
		pTexture->UnlockRect( 0 );
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

bool TextureEntityDirectX::LoadImageOfFormatEx(const std::string& sTextureFileName, char *sBufMemFile, int sizeBuf, int &width, int &height, byte ** ppBuffer, int* pBytesPerPixel, int nFormat, ImageExtendInfo *exInfo)
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

bool TextureEntityDirectX::LoadImageOfFormat(const std::string& sTextureFileName, char *sBufMemFile, int sizeBuf, int &width, int &height, byte ** ppBuffer, int* pBytesPerPixel, int nFormat)
{
	return TextureEntityDirectX::LoadImageOfFormatEx(sTextureFileName, sBufMemFile, sizeBuf, width, height, ppBuffer, pBytesPerPixel, nFormat);
}


bool TextureEntityDirectX::StretchRect(TextureEntityDirectX * pSrcTexture, TextureEntityDirectX * pDestTexture)
{
	bool bReleaseSrc = false;
	bool bReleaseDest = false;
	bool res = false;

	LPDIRECT3DSURFACE9 pSrcSurface = pSrcTexture->GetSurface();

	LPDIRECT3DSURFACE9 pDestSurface = pDestTexture->GetSurface();
	if (pSrcSurface == 0)
	{
		LPDIRECT3DTEXTURE9 pTex = pSrcTexture->GetTexture();
		bReleaseSrc = pTex != 0 && SUCCEEDED(pTex->GetSurfaceLevel(0, &pSrcSurface));
	}
	if (pDestSurface == 0)
	{
		LPDIRECT3DTEXTURE9 pTex = pDestTexture->GetTexture();
		bReleaseDest = pTex != 0 && SUCCEEDED(pTex->GetSurfaceLevel(0, &pDestSurface));
	}

	if (pSrcSurface && pDestSurface)
	{
		res = SUCCEEDED(CGlobals::GetRenderDevice()->StretchRect(pSrcSurface, NULL, pDestSurface, NULL, D3DTEXF_LINEAR));
	}

	if (bReleaseSrc)
	{
		SAFE_RELEASE(pSrcSurface);
	}
	if (bReleaseDest)
	{
		SAFE_RELEASE(pDestSurface);
	}
	return res;
}


bool TextureEntityDirectX::SetRenderTarget(int nIndex)
{
	bool bReleaseSrc = false;
	bool res = false;
	LPDIRECT3DSURFACE9 pSrcSurface = GetSurface();
	if (pSrcSurface == 0)
	{
		LPDIRECT3DTEXTURE9 pTex = GetTexture();
		bReleaseSrc = pTex != 0 && SUCCEEDED(pTex->GetSurfaceLevel(0, &pSrcSurface));
	}
	if (pSrcSurface)
	{
		res = SUCCEEDED(CGlobals::GetRenderDevice()->SetRenderTarget(nIndex, pSrcSurface));
	}
	if (bReleaseSrc)
	{
		SAFE_RELEASE(pSrcSurface);
	}
	if (res && nIndex == 0){
		CGlobals::GetViewportManager()->GetActiveViewPort()->ApplyViewport();
	}
	return res;
}

void TextureEntityDirectX::SetTexture(LPDIRECT3DTEXTURE9 pSrcTexture)
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

TextureEntity* TextureEntityDirectX::CreateTexture(const char* pFileName, uint32 nMipLevels /*= 0*/, D3DPOOL dwCreatePool /*= D3DPOOL_MANAGED*/)
{
	LPDIRECT3DTEXTURE9 pTexture = NULL;
#if WIN32&&defined(DEFAULT_FILE_ENCODING)
	LPCWSTR pFileName16 = StringHelper::MultiByteToWideChar(pFileName, DEFAULT_FILE_ENCODING);
	HRESULT hr = D3DXCreateTextureFromFileExW(CGlobals::GetRenderDevice(), pFileName16, D3DX_DEFAULT, D3DX_DEFAULT, D3DX_FROM_FILE, 0, D3DFMT_UNKNOWN, dwCreatePool, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &pTexture);
#else 
	HRESULT hr = D3DXCreateTextureFromFileEx(CGlobals::GetRenderDevice(), pFileName, D3DX_DEFAULT, D3DX_DEFAULT, D3DX_FROM_FILE, 0, D3DFMT_UNKNOWN, dwCreatePool, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL, &pTexture);
#endif
	if (SUCCEEDED(hr) && pTexture != NULL)
	{
		TextureEntityDirectX* pTextureEntity = new TextureEntityDirectX(AssetKey(pFileName));
		if (pTextureEntity)
		{
			pTextureEntity->SetTexture(pTexture);
			SAFE_RELEASE(pTexture);
			pTextureEntity->AddToAutoReleasePool();
			return pTextureEntity;
		}
	}
	else
	{
		OUTPUT_LOG("warn: failed to create texture: %s \n", pFileName);
	}
	return NULL;
}


bool TextureEntityDirectX::SaveToFile(const char* filename, D3DFORMAT dwFormat, int width, int height, UINT MipLevels /*= 1*/, DWORD Filter /*= D3DX_DEFAULT*/, Color ColorKey /*= 0*/)
{
	DeviceTexturePtr_type pSrcTexture = GetTexture();
	if (!pSrcTexture)
		return false;

	string sFile = filename;
	string sExt = CParaFile::GetFileExtension(sFile);
	D3DXIMAGE_FILEFORMAT FileFormat = D3DXIFF_PNG;
	if (sExt == "dds")
	{
		FileFormat = D3DXIFF_DDS;
	}
	else if (sExt == "jpg")
	{
		FileFormat = D3DXIFF_JPG;
	}
	else if (sExt == "tga")
	{
		FileFormat = D3DXIFF_TGA;
	}
	else // if(sExt == "png")
	{
		sFile = CParaFile::ChangeFileExtension(sFile, "png");
	}

	if ((FileFormat != D3DXIFF_DDS) && (width <= 0 || width >= GetWidth()))
	{
#if WIN32&&defined(DEFAULT_FILE_ENCODING)
		std::wstring sFile16 = StringHelper::MultiByteToWideChar(sFile.c_str(), DEFAULT_FILE_ENCODING);
		HRESULT hr = D3DXSaveTextureToFileW(sFile16.c_str(), FileFormat, pSrcTexture, nullptr);
#else 
		HRESULT hr = D3DXSaveTextureToFile(sFile.c_str(), FileFormat, pSrcTexture, nullptr);
#endif
		return SUCCEEDED(hr);
	}
	else if (FileFormat == D3DXIFF_DDS)
	{
		RenderDevicePtr pd3dDevice = CGlobals::GetRenderDevice();
		DeviceTexturePtr_type pDestTexture = NULL;
		HRESULT hr = D3DXCreateTexture(pd3dDevice, width, height, 1, D3DUSAGE_AUTOGENMIPMAP, dwFormat, D3DPOOL_MANAGED, &pDestTexture);
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
#if WIN32&&defined(DEFAULT_FILE_ENCODING)
				LPCWSTR filename16 = StringHelper::MultiByteToWideChar(filename, DEFAULT_FILE_ENCODING);
				hr = D3DXSaveTextureToFileW(filename16, D3DXIFF_DDS, pDestTexture, NULL);
#else 
				hr = D3DXSaveTextureToFile(filename, D3DXIFF_DDS, pDestTexture, NULL);
#endif

				if (SUCCEEDED(hr))
				{
					if (MipLevels != 1)
					{
						// TODO: for some reason, this does not work. 
						LPDIRECT3DTEXTURE9 pDestTextureMipMapped = NULL;
#if WIN32&&defined(DEFAULT_FILE_ENCODING)
						LPCWSTR filename16 = StringHelper::MultiByteToWideChar(filename, DEFAULT_FILE_ENCODING);
						hr = D3DXCreateTextureFromFileExW(pd3dDevice, filename16, D3DX_DEFAULT, D3DX_DEFAULT, MipLevels, D3DUSAGE_AUTOGENMIPMAP, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, Filter, D3DX_DEFAULT, 0, NULL, NULL, &pDestTextureMipMapped);
#else 
						hr = D3DXCreateTextureFromFileEx(pd3dDevice, filename, D3DX_DEFAULT, D3DX_DEFAULT, MipLevels, D3DUSAGE_AUTOGENMIPMAP, D3DFMT_UNKNOWN, D3DPOOL_MANAGED, Filter, D3DX_DEFAULT, 0, NULL, NULL, &pDestTextureMipMapped);
#endif
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
#if WIN32&&defined(DEFAULT_FILE_ENCODING)
							LPCWSTR filename16 = StringHelper::MultiByteToWideChar(filename, DEFAULT_FILE_ENCODING);
							hr = D3DXSaveTextureToFileW(filename16, D3DXIFF_DDS, pDestTextureMipMapped, NULL);
#else 
							hr = D3DXSaveTextureToFile(filename, D3DXIFF_DDS, pDestTextureMipMapped, NULL);
#endif
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
	}
	return false;
}

TextureEntity* TextureEntityDirectX::CreateTexture(const uint8 * pTexels, int width, int height, int rowLength, int bytesPerPixel, uint32 nMipLevels /*= 0*/, D3DPOOL dwCreatePool /*= D3DPOOL_MANAGED*/, DWORD nFormat /*= 0*/)
{
	IDirect3DDevice9* pD3d = CGlobals::GetRenderDevice();
	LPDIRECT3DTEXTURE9 pTexture = NULL;

	if (!pTexels)
		return 0;

	if (bytesPerPixel == 4)
	{
		HRESULT hr = D3DXCreateTexture(pD3d, width, height, 0, D3DUSAGE_AUTOGENMIPMAP, D3DFMT_A8R8G8B8, dwCreatePool, &pTexture);
		if (FAILED(hr))
		{
			OUTPUT_LOG("failed creating terrain texture\n");
		}
		else
		{
			D3DLOCKED_RECT lr;
			pTexture->LockRect(0, &lr, NULL, 0);
			memcpy(lr.pBits, pTexels, width*height * 4);
			pTexture->UnlockRect(0);
		}
	}
	else if (bytesPerPixel == 3)
	{
		// please note, we will create D3DFMT_A8R8G8B8 instead of , D3DFMT_R8G8B8, since our device will use D3DFMT_A8R8G8B8 only
		HRESULT hr = D3DXCreateTexture(pD3d, width, height, 0, D3DUSAGE_AUTOGENMIPMAP, D3DFMT_A8R8G8B8, dwCreatePool, &pTexture);
		if (FAILED(hr))
		{
			OUTPUT_LOG("failed creating terrain texture\n");
		}
		else
		{
			D3DLOCKED_RECT lockedRect;
			pTexture->LockRect(0, &lockedRect, NULL, 0);

			//memcpy(lockedRect.pBits, pTexels, width*height*3);
			uint8 *pp = (uint8*)lockedRect.pBits;
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
				index += lockedRect.Pitch - (width * 4);
			}
			pTexture->UnlockRect(0);
		}
	}
	else if (bytesPerPixel == 1)
	{
		HRESULT hr;
		// whether D3DFMT_A8 is supported. -1 is unknown, 0 is not supported, 1 is supported. 
		static int nSupportAlphaTexture = -1;
		if (nSupportAlphaTexture == -1)
		{
			D3DFORMAT nDesiredFormat = D3DFMT_A8;
			hr = D3DXCheckTextureRequirements(pD3d, NULL, NULL, NULL, 0, &nDesiredFormat, dwCreatePool);
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

		if (nSupportAlphaTexture == 1)
		{
			// D3DFMT_A8 is supported
			HRESULT hr = D3DXCreateTexture(pD3d, width, height, 0, D3DUSAGE_AUTOGENMIPMAP, D3DFMT_A8, dwCreatePool, &pTexture);
			if (FAILED(hr))
			{
				OUTPUT_LOG("failed creating alpha terrain texture\n");
			}
			else
			{
				D3DLOCKED_RECT lr;
				pTexture->LockRect(0, &lr, NULL, 0);
				memcpy(lr.pBits, pTexels, width*height * 1);
				pTexture->UnlockRect(0);
			}
		}
		else if (nSupportAlphaTexture == 0)
		{
			// D3DFMT_A8 is not supported, try D3DFMT_A8R8G8B8
			HRESULT hr = D3DXCreateTexture(pD3d, width, height, 0, D3DUSAGE_AUTOGENMIPMAP, D3DFMT_A8R8G8B8, dwCreatePool, &pTexture);
			if (FAILED(hr))
			{
				OUTPUT_LOG("failed creating alpha terrain texture\n");
			}
			else
			{
				D3DLOCKED_RECT lr;
				pTexture->LockRect(0, &lr, NULL, 0);
				int nSize = width*height;
				DWORD* pData = (DWORD*)(lr.pBits);
				for (int x = 0; x < nSize; ++x)
				{
					pData[x] = (pTexels[x]) << 24;
				}
				pTexture->UnlockRect(0);
			}
		}
	}
	else
	{
		OUTPUT_LOG("error: Unsupported texture format (bits per pixel must be 8,24, or 32)\n");
	}
	if (pTexture != NULL)
	{
		TextureEntityDirectX* pTextureEntity = new TextureEntityDirectX();
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

TextureEntity* TextureEntityDirectX::LoadUint8Buffer(const uint8 * pTexels, int width, int height, int rowLength, int bytesPerPixel, uint32 nMipLevels /*= 0*/, D3DPOOL dwCreatePool /*= D3DPOOL_MANAGED*/, DWORD nFormat /*= 0*/)
{
	IDirect3DDevice9* pD3d = CGlobals::GetRenderDevice();
	LPDIRECT3DTEXTURE9 pTexture = NULL;

	if (!pTexels)
		return 0;

	if (bytesPerPixel == 4)
	{
		HRESULT hr = D3DXCreateTexture(pD3d, width, height, 0, D3DUSAGE_AUTOGENMIPMAP, D3DFMT_A8R8G8B8, dwCreatePool, &pTexture);
		if (FAILED(hr))
		{
			OUTPUT_LOG("failed creating terrain texture\n");
		}
		else
		{
			D3DLOCKED_RECT lr;
			pTexture->LockRect(0, &lr, NULL, 0);
			memcpy(lr.pBits, pTexels, width*height * 4);
			pTexture->UnlockRect(0);
		}
	}
	else if (bytesPerPixel == 3)
	{
		// please note, we will create D3DFMT_A8R8G8B8 instead of , D3DFMT_R8G8B8, since our device will use D3DFMT_A8R8G8B8 only
		HRESULT hr = D3DXCreateTexture(pD3d, width, height, 0, D3DUSAGE_AUTOGENMIPMAP, D3DFMT_A8R8G8B8, dwCreatePool, &pTexture);
		if (FAILED(hr))
		{
			OUTPUT_LOG("failed creating terrain texture\n");
		}
		else
		{
			D3DLOCKED_RECT lockedRect;
			pTexture->LockRect(0, &lockedRect, NULL, 0);

			//memcpy(lockedRect.pBits, pTexels, width*height*3);
			uint8 *pp = (uint8*)lockedRect.pBits;
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
				index += lockedRect.Pitch - (width * 4);
			}
			pTexture->UnlockRect(0);
		}
	}
	else if (bytesPerPixel == 1)
	{
		HRESULT hr;
		// whether D3DFMT_A8 is supported. -1 is unknown, 0 is not supported, 1 is supported. 
		static int nSupportAlphaTexture = -1;
		if (nSupportAlphaTexture == -1)
		{
			D3DFORMAT nDesiredFormat = D3DFMT_A8;
			hr = D3DXCheckTextureRequirements(pD3d, NULL, NULL, NULL, 0, &nDesiredFormat, dwCreatePool);
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

		if (nSupportAlphaTexture == 1)
		{
			// D3DFMT_A8 is supported
			HRESULT hr = D3DXCreateTexture(pD3d, width, height, 0, D3DUSAGE_AUTOGENMIPMAP, D3DFMT_A8, dwCreatePool, &pTexture);
			if (FAILED(hr))
			{
				OUTPUT_LOG("failed creating alpha terrain texture\n");
			}
			else
			{
				D3DLOCKED_RECT lr;
				pTexture->LockRect(0, &lr, NULL, 0);
				memcpy(lr.pBits, pTexels, width*height * 1);
				pTexture->UnlockRect(0);
			}
		}
		else if (nSupportAlphaTexture == 0)
		{
			// D3DFMT_A8 is not supported, try D3DFMT_A8R8G8B8
			HRESULT hr = D3DXCreateTexture(pD3d, width, height, 0, D3DUSAGE_AUTOGENMIPMAP, D3DFMT_A8R8G8B8, dwCreatePool, &pTexture);
			if (FAILED(hr))
			{
				OUTPUT_LOG("failed creating alpha terrain texture\n");
			}
			else
			{
				D3DLOCKED_RECT lr;
				pTexture->LockRect(0, &lr, NULL, 0);
				int nSize = width * height;
				DWORD* pData = (DWORD*)(lr.pBits);
				for (int x = 0; x < nSize; ++x)
				{
					pData[x] = (pTexels[x]) << 24;
				}
				pTexture->UnlockRect(0);
			}
		}
	}
	else
	{
		OUTPUT_LOG("error: Unsupported texture format (bits per pixel must be 8,24, or 32)\n");
	}
	if (pTexture != NULL)
	{
		TextureEntityDirectX* pTextureEntity = this;
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

HRESULT TextureEntityDirectX::LoadFromMemory(const char* buffer, DWORD nFileSize, UINT nMipLevels, D3DFORMAT dwTextureFormat, void** ppTexture_)
{
	HRESULT hr;
	DeviceTexturePtr_type* ppTexture = (DeviceTexturePtr_type*)ppTexture_;
	RenderDevicePtr pd3dDevice = CGlobals::GetRenderDevice();

	LPDIRECT3DTEXTURE9 pTexture = NULL;

	switch (SurfaceType)
	{
	case TextureEntity::SysMemoryTexture:
	{
		D3DXIMAGE_INFO SrcInfo;
		if (GetTextureInfo())
		{
			SrcInfo.Width = GetTextureInfo()->GetWidth();
			SrcInfo.Height = GetTextureInfo()->GetHeight();
			SrcInfo.Format = GetD3DFormat();
		}
		else
		{
			if (FAILED(hr = D3DXGetImageInfoFromFileInMemory(buffer, nFileSize, &SrcInfo)))
			{
				OUTPUT_LOG("Could not D3DXGetImageInfoFromFileInMemory %s\n", GetKey().c_str());
				break;
			}
			SrcInfo.Format = D3DFMT_X8R8G8B8;
		}

		hr = pd3dDevice->CreateOffscreenPlainSurface(SrcInfo.Width, SrcInfo.Height, SrcInfo.Format, D3DPOOL_SYSTEMMEM, &(m_pSurface), NULL);
		if (SUCCEEDED(hr))
		{
			// Load the image again, this time with D3D to load directly to the surface
			hr = D3DXLoadSurfaceFromFileInMemory(m_pSurface, NULL, NULL, buffer, nFileSize, NULL, D3DX_FILTER_NONE,
				GetColorKey() /*COLOR_XRGB(0,0,0) this makes black transparent*/, NULL);
			if (SUCCEEDED(hr))
			{
				if (SrcInfo.Format == D3DFMT_A8R8G8B8)
				{
					/** discovered by clayman, xizhi: for unknown reasons, D3DXLoadSurfaceFromFileInMemory loaded alpha has randomly incorrect alpha values.
					* the following code fix it in most cases.
					*/
					LPDIRECT3DSURFACE9 pCursorSurface = m_pSurface;
					if (pCursorSurface)
					{
						D3DLOCKED_RECT data;
						pCursorSurface->LockRect(&data, NULL, 0);
						uint32* pData = (uint32*)data.pBits;
						int count = SrcInfo.Width * SrcInfo.Height;
						for (int i = 0; i<count; i++)
						{
							uint32 color = *pData;
							uint32 alpha = color >> 24;
							if (alpha > 127)
							{
								color |= 0xff000000;
							}
							else
							{
								color &= 0xffffff;
							}
							*pData = color;
							pData++;
						}
						pCursorSurface->UnlockRect();
					}
				}
			}
			else
			{
				OUTPUT_LOG("Could not load the following bitmap to a surface %s\n", GetKey().c_str());
			}
		}
		else
		{
			SetState(AssetEntity::ASSET_STATE_FAILED_TO_LOAD);
			OUTPUT_LOG("Unable to create a new surface for the bitmap %s\n", GetKey().c_str());
		}
		break;
	}
	case TextureEntity::CubeTexture:
	{
		hr = D3DXCreateCubeTextureFromFileInMemoryEx(pd3dDevice, buffer, nFileSize,
			D3DX_DEFAULT, D3DX_FROM_FILE /**D3DX_FROM_FILE:  mip-mapping from file */, 0, D3DFMT_UNKNOWN,
			D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT,
			0, NULL, NULL, &(m_pCubeTexture));
		if (FAILED(hr))
		{
			SetState(AssetEntity::ASSET_STATE_FAILED_TO_LOAD);
			OUTPUT_LOG("failed creating cube texture for %s\n", GetKey().c_str());
		}
		break;
	}
	default:
	{
		// TextureEntity::StaticTexture

/** whether to print big directX texture that may take a long time to load into log file */
#ifdef _DEBUG
#define PRINT_BIG_TEXTURE
#endif

#ifdef PRINT_BIG_TEXTURE
		int64 nTick = GetTickCount();
#endif
		hr = D3DXCreateTextureFromFileInMemoryEx(pd3dDevice, buffer, nFileSize,
			D3DX_DEFAULT, D3DX_DEFAULT, nMipLevels, 0, dwTextureFormat,
			D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT,
			m_dwColorKey, NULL, NULL, &(pTexture));
		
#ifdef PRINT_BIG_TEXTURE
		nTick = GetTickCount() - nTick;
		if (nTick > 100) {
			OUTPUT_LOG("%d, %d: %s\n", GetTickCount(), (int)(nTick), GetKey().c_str());
		}
#endif

		

		if (SUCCEEDED(hr))
		{
			/** we will only set LOD if the texture file is larger than 40KB */
			if (nMipLevels != 1 && TextureEntity::g_nTextureLOD>0 && nFileSize>40000)
			{
				pTexture->SetLOD(TextureEntity::g_nTextureLOD);
			}
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
				D3DSURFACE_DESC desc;
				if (pTexture != 0 && SUCCEEDED(pTexture->GetLevelDesc(0, &desc)))
				{
					m_pTextureInfo->m_width = desc.Width;
					m_pTextureInfo->m_height = desc.Height;
				}
			}
		}
		break;
	}
	}
	return hr;
}



#endif
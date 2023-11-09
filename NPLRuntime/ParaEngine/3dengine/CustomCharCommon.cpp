//-----------------------------------------------------------------------------
// Class:	
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2009.8.6
// Desc: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "AssetManifest.h"
#include "ParaWorldAsset.h"
#include "util/HttpUtility.h"
#include "CustomCharModelInstance.h"
#include "CustomCharCommon.h"
#if USE_DIRECTX_RENDERER
#include "RenderDeviceD3D9.h"
#endif
using namespace ParaEngine;

/************************************************************************/
/* CharTextureComponent				                                    */
/************************************************************************/

CharTextureComponent::CharTextureComponent()
	:m_dwColor(0xffffffff), region(0), layer(0)
{
}

bool CharTextureComponent::GetPixels(unsigned char* pBuffer, int* nWidth, int* nHeight)
{
	bool bOK = true;
#ifdef USE_DIRECTX_RENDERER
	auto pRenderDevice = GETD3D(CGlobals::GetRenderDevice());
	
	D3DXIMAGE_INFO SrcInfo;

	CParaFile myFile(name.c_str());
	char* buffer = myFile.getBuffer();
	DWORD nFileSize = (DWORD)myFile.getSize();
	if (myFile.isEof() || (buffer == 0))
	{
		OUTPUT_LOG("warning: texture not found %s\n", name.c_str());
		return false;
	}

	// load to the specified format
	LPDIRECT3DTEXTURE9 pTexture = NULL;
	HRESULT hr = D3DXCreateTextureFromFileInMemoryEx(pRenderDevice,buffer, nFileSize, D3DX_DEFAULT, D3DX_DEFAULT, 1, 0,
		D3DFMT_A8R8G8B8, D3DPOOL_SCRATCH, D3DX_DEFAULT, D3DX_DEFAULT, 0, &SrcInfo, NULL, &pTexture);
	if (SUCCEEDED(hr))
	{
		// write the image file information.
		*nWidth = SrcInfo.Width;
		*nHeight = SrcInfo.Height;

		D3DLOCKED_RECT lr;
		hr = pTexture->LockRect(0, &lr, NULL, D3DLOCK_READONLY);
		if (SUCCEEDED(hr))
		{
			memcpy(pBuffer, lr.pBits, SrcInfo.Width*SrcInfo.Height * 4);
			pTexture->UnlockRect(0);
		}
		else
			bOK = false;

		SAFE_RELEASE(pTexture);
	}
	else
	{
		OUTPUT_LOG("warning: texture can not be created %s\n", name.c_str());
		return false;
	}
#else
	bOK = false;
#endif
	return bOK;
}

DWORD CharTextureComponent::GetColor() const
{
	return m_dwColor;
}

void CharTextureComponent::SetColor(DWORD color)
{
	m_dwColor = color;
}

bool ParaEngine::CharTextureComponent::CheckLoad()
{
	// load the component texture
	if (!name.empty())
	{
		if (!m_texture)
		{
			string componentfilename = name;
			AssetFileEntry* pEntry = CAssetManifest::GetSingleton().GetFile(componentfilename);
			if (pEntry)
				componentfilename = pEntry->GetLocalFileName();
			m_texture = CGlobals::GetAssetManager()->LoadTexture(componentfilename, componentfilename);
		}

		if (m_texture)
		{
			if (!(m_texture->GetTexture()) || !m_texture->IsLoaded())
			{
				if (!m_texture->IsValid()){
					// skip invalid textures
					m_texture.reset();
					name.clear();
				}
				else
					return false;
			}
		}
	}
	return true;
}

/************************************************************************/
/* Character Texture(composite texture)									*/
/************************************************************************/
CharTexture::CharTexture()
	:m_bNeedUpdateCache(false), m_bNeedUpdate(false)
{

}

CharTexture::~CharTexture()
{
}

const string& CharTexture::GetCacheFileName()
{
	if (m_bNeedUpdateCache)
	{
		m_bNeedUpdateCache = false;
		string sLen;
		std::sort(components.begin(), components.end());
		for (std::vector<CharTextureComponent>::iterator it = components.begin(); it != components.end(); ++it)
		{
			CharTextureComponent &comp = *it;
			if (!comp.name.empty())
				sLen += comp.name;
			if ((comp.GetColor()) != 0xffffffff)
			{
				char temp[256];
				sprintf(temp, "%x", comp.GetColor());
				sLen += temp;
			}
		}
		m_sCachedFileName = "temp/composeskin/";
		m_sCachedFileName += CHttpUtility::HashStringMD5(sLen.c_str());
		// Note it is important to end with "Skin" since md5 that end with aXXX may be wrongly interpreted as sequence textures. 
		m_sCachedFileName += "Skin.png"; // used to ".dds", however, we may use GDI for texture composition.
	}
	return m_sCachedFileName;
}


void CharTexture::Reset()
{
	components.clear();
	m_sCachedFileName.clear();
	m_bNeedUpdateCache = false;
	m_bNeedUpdate = false;
}

void CharTexture::compose(asset_ptr<TextureEntity>& tex, CharModelInstance* pCharInst)
{
	m_bNeedUpdate = false;

	// delete the old composed character texture
	if (tex)
	{
		if (tex.get() != 0)
		{
			// TODO: this is actually never called, because ref count is always 2. 
			tex->GarbageCollectMe();
		}
		//CGlobals::GetAssetManager()->GetTextureManager().DeleteByName(filename);
	}
	// if we only have one texture then don't bother with compositing
	if (components.size() == 1 && components[0].GetColor() == 0xffffffff)
	{
		tex = CGlobals::GetAssetManager()->LoadTexture("", components[0].name.c_str(), TextureEntity::StaticTexture);
		return;
	}
#ifdef USE_DIRECTX_RENDERER
	// if we have composed this texture before, use it. 
	if (CParaFile::DoesFileExist(GetCacheFileName().c_str(), false))
	{
		tex = CGlobals::GetAssetManager()->LoadTexture("", GetCacheFileName().c_str(), TextureEntity::StaticTexture);
		return;
	}
#endif
#ifdef USE_OPENGL_RENDERER
	// TODO: shall we search in render target?
#endif

	if (pCharInst)
		CharModelInstance::AddPendingBodyTexture(pCharInst);
}
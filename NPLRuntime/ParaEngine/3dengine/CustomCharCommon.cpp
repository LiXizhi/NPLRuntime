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
#ifndef USE_NULL_RENDERER
#include "ParaImage.h"
#endif
#include "CustomCharModelInstance.h"
#include "CustomCharCommon.h"
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
#ifndef USE_NULL_RENDERER
	CParaFile myFile(name.c_str());
	char* buffer = myFile.getBuffer();
	DWORD nFileSize = (DWORD)myFile.getSize();
	if (myFile.isEof() || (buffer == 0))
	{
		OUTPUT_LOG("warning: texture not found %s\n", name.c_str());
		return false;
	}

	ParaImage img;
	if (!img.initWithImageData((const unsigned char*)buffer, nFileSize))
	{
		OUTPUT_LOG("warning: texture can not be created %s\n", name.c_str());
		return false;
	}

	const int w = (int)img.getWidth();
	const int h = (int)img.getHeight();
	if (w <= 0 || h <= 0)
		return false;

	if (nWidth)
		*nWidth = w;
	if (nHeight)
		*nHeight = h;
	if (!pBuffer)
		return true;

	unsigned char* rgbaData = nullptr;
	size_t rgbaLen = 0;
	PixelFormat outFmt = ParaImage::convertDataToFormat(img.getData(), img.getDataLen(), img.getRenderFormat(), PixelFormat::A8R8G8B8, &rgbaData, &rgbaLen);
	if (outFmt != PixelFormat::A8R8G8B8 || !rgbaData)
	{
		if (rgbaData && rgbaData != img.getData())
			delete[] rgbaData;
		return false;
	}

	// Convert RGBA8888 -> BGRA8888 (D3DFMT_A8R8G8B8)
	const size_t pixelCount = (size_t)w * (size_t)h;
	for (size_t i = 0; i < pixelCount; ++i)
	{
		pBuffer[i * 4 + 0] = rgbaData[i * 4 + 2];
		pBuffer[i * 4 + 1] = rgbaData[i * 4 + 1];
		pBuffer[i * 4 + 2] = rgbaData[i * 4 + 0];
		pBuffer[i * 4 + 3] = rgbaData[i * 4 + 3];
	}
	if (rgbaData != img.getData())
		delete[] rgbaData;
	return true;
#else
	return false;
#endif
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
	}
	// if we only have one texture then don't bother with compositing
	if (components.size() == 1 && components[0].GetColor() == 0xffffffff)
	{
		tex = CGlobals::GetAssetManager()->LoadTexture("", components[0].name.c_str(), TextureEntity::StaticTexture);
		return;
	}
	// if we have composed this texture before, use it. 
	if (CParaFile::DoesFileExist(GetCacheFileName().c_str(), false))
	{
		tex = CGlobals::GetAssetManager()->LoadTexture("", GetCacheFileName().c_str(), TextureEntity::StaticTexture);
		return;
	}

	if (pCharInst)
		CharModelInstance::AddPendingBodyTexture(pCharInst);
}

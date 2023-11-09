//-----------------------------------------------------------------------------
// Class:	CTerrainRegions
// Authors:	LiXizhi
// Company: ParaEngine Corporation
// Emails:	LiXizhi@yeah.net
// Date:	2009.9.11
// Revised: 2009.9.11
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "TerrainRegions.h"
#ifdef USE_DIRECTX_RENDERER

#ifdef USE_FREEIMAGE
#include "FreeImageMemIO.h"
#endif
#include "TextureEntity.h"
#include <objbase.h>
#include <comdef.h>
#include <gdiplus.h>

using namespace Gdiplus;
#endif

#include "memdebug.h"

using namespace ParaTerrain;
using namespace ParaEngine;




//////////////////////////////////////////////////////////////////////////
//
// CRegionLayer
//
//////////////////////////////////////////////////////////////////////////
CRegionLayer::CRegionLayer()
:m_nHeight(0), m_nWidth(0), m_pData(NULL), m_nSize(0), m_nBytesPerPixel(0), m_nBytesPerRow(0), m_dwTextureFormat(0)
{
}

CRegionLayer::~CRegionLayer()
{
	SAFE_DELETE(m_pData);
}

void CRegionLayer::LoadFromFile(const char* filename, int nBytesPerPixel)
{
#if defined(USE_FREEIMAGE) && defined(USE_DIRECTX_RENDERER)
	if(filename == 0)
		return;
	string sTextureFileName = filename;
	// save file name. 
	m_filename = sTextureFileName;

	int nSize = (int)(sTextureFileName.size());
	/** whether we treat png file as DXT3 by default. if the texture filename ends with "_32bits.png", we will load with D3DFMT_A8R8G8B8 instead of DXT3. 
	If one wants to ensure high resolution texture, use TGA format instead. */
	
	if(sTextureFileName[nSize-11]=='_' && sTextureFileName[nSize-10]=='2' && sTextureFileName[nSize-9]=='4' && sTextureFileName[nSize-8]=='b')
	{
		m_nBytesPerPixel = 3;
		m_dwTextureFormat = PixelFormat24bppRGB;
	}
	else if(sTextureFileName[nSize-11]=='_' && sTextureFileName[nSize-10]=='1' && sTextureFileName[nSize-9]=='6' && sTextureFileName[nSize-8]=='b')
	{
		m_nBytesPerPixel = 2;
		m_dwTextureFormat = PixelFormat16bppGrayScale;
	}
	else if(sTextureFileName[nSize-10]=='_' && sTextureFileName[nSize-9]=='8' && sTextureFileName[nSize-8]=='b')
	{
		m_nBytesPerPixel = 1;
		m_dwTextureFormat = PixelFormat8bppIndexed;
	}
	else
	{
		// default to 32 bits.
		m_dwTextureFormat = PixelFormat32bppARGB;
		m_nBytesPerPixel = 4;
	}
	
	CPathReplaceables::GetSingleton().DecodePath(sTextureFileName, sTextureFileName);
	CParaFile file;
	file.OpenAssetFile(sTextureFileName.c_str());
	if(file.isEof())
	{
		OUTPUT_LOG("warning: can not load terrain region file %s \n", sTextureFileName.c_str());
		return;
	}

	MemIO memIO((BYTE*)(file.getBuffer()), file.getSize());
	FIBITMAP *dib = FreeImage_LoadFromHandle( (FREE_IMAGE_FORMAT) TextureEntity::GetFormatByFileName(sTextureFileName), &memIO, (fi_handle)&memIO );
	if(dib == 0)
	{
		OUTPUT_LOG("warning: can not load terrain region file %s \n", sTextureFileName.c_str());
		return;
	}
	BITMAPINFOHEADER* pInfo = FreeImage_GetInfoHeader(dib);

	if(pInfo)
	{
		m_nWidth = pInfo->biWidth;
		m_nHeight = pInfo->biHeight;
		m_nBytesPerPixel = pInfo->biBitCount/8;
	}
	else
		return;

	BYTE* pPixels = FreeImage_GetBits(dib);
	if(pPixels)
	{
		m_nBytesPerRow = m_nWidth * m_nBytesPerPixel;
		m_nSize = m_nBytesPerRow * m_nHeight;
		m_pData = new char[m_nSize];
		memcpy(m_pData, pPixels, m_nSize);
	}
	FreeImage_Unload(dib);
#endif
}

DWORD CRegionLayer::GetPixelValue(int x, int y)
{
	if(x<0)
		x = 0;
	if(x>=m_nWidth)
		x = m_nWidth -1;
	if(y<0)
		y = 0;
	if(y>=m_nHeight)
		y = m_nHeight -1;

	unsigned char* pPixel = (unsigned char*)(m_pData + m_nBytesPerRow * y + x * m_nBytesPerPixel); 
	DWORD color = 0;
	for(int i=0; i < m_nBytesPerPixel; ++i, ++pPixel)
	{
		// Following is just unsigned char order. 
		// color += (*pPixel)<<(8 * (m_nBytesPerPixel-i-1));
		color += (*pPixel)<<(8 *i);
	}
	return color;
}

void CRegionLayer::SetFileName(const string& filename)
{
	m_filename = filename;
}

//////////////////////////////////////////////////////////////////////////
//
// CTerrainRegions
//
//////////////////////////////////////////////////////////////////////////
CTerrainRegions::CTerrainRegions()
:m_nCurRegionIndex(-1)
{
}

CTerrainRegions::~CTerrainRegions()
{
	RegionLayer_Map_Type::iterator itCur, itEnd = m_layers.end();
	
	for(itCur = m_layers.begin(); itCur != itEnd; ++itCur)
	{
		SAFE_DELETE(itCur->second);
	}
	m_layers.clear();
}

void CTerrainRegions::SetSize(float fWidth, float fHeight)
{
	m_fWidth = fWidth;
	m_fHeight = fHeight;
}

CRegionLayer* CTerrainRegions::GetRegion(const string& sLayerName)
{
	RegionLayer_Map_Type::iterator iter = m_layers.find(sLayerName);
	if(iter != m_layers.end())
	{
		return iter->second;
	}
	return NULL;
}

DWORD CTerrainRegions::GetValue(const string& sLayerName, float x, float y)
{
	return GetValue(GetRegion(sLayerName), x, y);
}

DWORD CTerrainRegions::GetValue(CRegionLayer* pRegion, float x, float y)
{
	if(pRegion !=0)
	{
		return pRegion->GetPixelValue((int)(x/m_fWidth*pRegion->GetWidth()), (int)(y/m_fHeight*pRegion->GetHeight()));
	}
	return 0;
}

void CTerrainRegions::LoadRegion(const string& sLayerName, const char* filename)
{
	CRegionLayer* pRegion = GetRegion(sLayerName);
	if(pRegion !=0)
	{
		// TODO: shall we reload?
		return;
	}

	pRegion = new CRegionLayer();
	pRegion->LoadFromFile(filename);
	m_layers[sLayerName] = pRegion;
}

int CTerrainRegions::GetNumOfRegions()
{
	return (int)(m_layers.size());
}

void CTerrainRegions::SetCurrentRegionIndex(int nRegion)
{
	m_nCurRegionIndex = nRegion;

	int i = 0;
	RegionLayer_Map_Type::iterator itCur, itEnd = m_layers.end();

	for(itCur = m_layers.begin(); itCur != itEnd; ++itCur, ++i)
	{
		if(i == m_nCurRegionIndex)
		{
			m_sCurRegionName = itCur->first;
			return;
		}
	}
	m_nCurRegionIndex = -1;
}

int CTerrainRegions::GetCurrentRegionIndex()
{
	return m_nCurRegionIndex;
}

void CTerrainRegions::SetCurrentRegionName(const string& name)
{
	m_sCurRegionName = name;
	
	int i = 0;
	RegionLayer_Map_Type::iterator itCur, itEnd = m_layers.end();

	for(itCur = m_layers.begin(); itCur != itEnd; ++itCur, ++i)
	{
		if(itCur->first == m_sCurRegionName)
		{
			m_nCurRegionIndex = i;
			return;
		}
	}
	m_nCurRegionIndex = -1;
}


const string& CTerrainRegions::GetCurrentRegionName()
{
	return m_sCurRegionName;
}


void CTerrainRegions::SetCurrentRegionFilepath(const string& filename)
{
	if(m_sCurRegionName.empty())
	{
		OUTPUT_LOG("warning: please set CurrentRegionName before setting CurrentRegionFilepath\n");
		return;
	}
	CRegionLayer* pLayer = GetRegion(m_sCurRegionName);
	if(pLayer != 0)
	{
		pLayer->SetFileName(filename);
	}
	else
	{
		LoadRegion(m_sCurRegionName, filename.c_str());
	}
}


const string&  CTerrainRegions::GetCurrentRegionFilepath()
{
	CRegionLayer* pLayer = GetRegion(m_sCurRegionName);
	if(pLayer != 0)
	{
		return pLayer->GetFileName();
	}
	else
	{
		return CGlobals::GetString(0);
	}
}
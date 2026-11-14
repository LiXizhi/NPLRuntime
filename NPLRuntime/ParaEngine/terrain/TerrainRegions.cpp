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
#include "TextureEntity.h"
#include "ParaImage.h"
#include "FileManager.h"

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
	if (filename == 0 || filename[0] == '\0')
		return;

	constexpr int kPixelFormat8bppIndexed = 0x30803;
	constexpr int kPixelFormat16bppGrayScale = 0x101004;
	constexpr int kPixelFormat24bppRGB = 0x21808;
	constexpr int kPixelFormat32bppARGB = 0x26200a;

	string sTextureFileName = filename;
	m_filename = sTextureFileName;

	int nSize = (int)(sTextureFileName.size());
	if (nSize >= 11 && sTextureFileName[nSize - 11] == '_' && sTextureFileName[nSize - 10] == '2' && sTextureFileName[nSize - 9] == '4' && sTextureFileName[nSize - 8] == 'b')
	{
		m_nBytesPerPixel = 3;
		m_dwTextureFormat = kPixelFormat24bppRGB;
	}
	else if (nSize >= 11 && sTextureFileName[nSize - 11] == '_' && sTextureFileName[nSize - 10] == '1' && sTextureFileName[nSize - 9] == '6' && sTextureFileName[nSize - 8] == 'b')
	{
		m_nBytesPerPixel = 2;
		m_dwTextureFormat = kPixelFormat16bppGrayScale;
	}
	else if (nSize >= 10 && sTextureFileName[nSize - 10] == '_' && sTextureFileName[nSize - 9] == '8' && sTextureFileName[nSize - 8] == 'b')
	{
		m_nBytesPerPixel = 1;
		m_dwTextureFormat = kPixelFormat8bppIndexed;
	}
	else
	{
		m_dwTextureFormat = kPixelFormat32bppARGB;
		m_nBytesPerPixel = 4;
	}

	CPathReplaceables::GetSingleton().DecodePath(sTextureFileName, sTextureFileName);
	CParaFile file;
	file.OpenAssetFile(sTextureFileName.c_str());
	if (file.isEof())
	{
		OUTPUT_LOG("warning: can not load terrain region file %s \n", sTextureFileName.c_str());
		return;
	}

	SAFE_DELETE_ARRAY(m_pData);

	unsigned char* pPixels = nullptr;
	int loadedBpp = m_nBytesPerPixel;
	if (TextureEntity::LoadImageOfFormat(sTextureFileName, file.getBuffer(), (int)file.getSize(), m_nWidth, m_nHeight, &pPixels, &loadedBpp, static_cast<int>(m_dwTextureFormat)) && pPixels)
	{
		m_nBytesPerPixel = loadedBpp;
		m_nBytesPerRow = m_nWidth * m_nBytesPerPixel;
		m_nSize = m_nBytesPerRow * m_nHeight;
		m_pData = reinterpret_cast<char*>(pPixels);
		return;
	}

	// Portable fallback via ParaImage (Linux, Emscripten, headless server)
	ParaImage img;
	if (!img.initWithImageData(reinterpret_cast<const unsigned char*>(file.getBuffer()), file.getSize()))
	{
		OUTPUT_LOG("warning: can not load terrain region file %s \n", sTextureFileName.c_str());
		return;
	}

	m_nWidth = img.getWidth();
	m_nHeight = img.getHeight();
	if (m_nWidth <= 0 || m_nHeight <= 0)
		return;

	unsigned char* rgba = nullptr;
	size_t rgbaLen = 0;
	if (ParaImage::convertDataToFormat(img.getData(), img.getDataLen(), img.getRenderFormat(), PixelFormat::A8R8G8B8, &rgba, &rgbaLen) != PixelFormat::A8R8G8B8 || !rgba)
		return;

	m_nBytesPerRow = m_nWidth * m_nBytesPerPixel;
	m_nSize = m_nBytesPerRow * m_nHeight;
	m_pData = new char[m_nSize];

	// Match FreeImage DIB bottom-up scanline and BGRA/BGR/Gray byte layout
	for (int dstY = 0; dstY < m_nHeight; ++dstY)
	{
		const int srcY = m_nHeight - 1 - dstY;
		unsigned char* dstRow = reinterpret_cast<unsigned char*>(m_pData) + static_cast<size_t>(dstY) * m_nBytesPerRow;
		const unsigned char* srcRow = rgba + static_cast<size_t>(srcY) * m_nWidth * 4u;
		for (int x = 0; x < m_nWidth; ++x)
		{
			const unsigned char r = srcRow[x * 4 + 0];
			const unsigned char g = srcRow[x * 4 + 1];
			const unsigned char b = srcRow[x * 4 + 2];
			const unsigned char a = srcRow[x * 4 + 3];
			if (m_nBytesPerPixel == 4)
			{
				dstRow[x * 4 + 0] = b;
				dstRow[x * 4 + 1] = g;
				dstRow[x * 4 + 2] = r;
				dstRow[x * 4 + 3] = a;
			}
			else if (m_nBytesPerPixel == 3)
			{
				dstRow[x * 3 + 0] = b;
				dstRow[x * 3 + 1] = g;
				dstRow[x * 3 + 2] = r;
			}
			else if (m_nBytesPerPixel == 2)
			{
				const uint16_t gray16 = static_cast<uint16_t>(r) | (static_cast<uint16_t>(g) << 8);
				dstRow[x * 2 + 0] = static_cast<unsigned char>(gray16 & 0xFFu);
				dstRow[x * 2 + 1] = static_cast<unsigned char>((gray16 >> 8) & 0xFFu);
			}
			else
			{
				dstRow[x] = r;
			}
		}
	}

	if (rgba != img.getData())
		delete[] rgba;
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
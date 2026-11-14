// Terrain Engine used in ParaEngine
// Based on the Demeter Terrain Visualization Library by Clay Fowler, 2002
// File marked with the above information inherits the GNU License from Demeter Terrain Engine.
// Other portion of ParaEngine is subjected to its own License.

#include "ParaEngine.h"
#include "ParaWorldAsset.h"
#include "ParaImage.h"
#include "TextureEntity.h"
#include "Loader.h"
#include "memdebug.h"
using namespace ParaEngine;

using namespace ParaTerrain;
using namespace std;

void LoadImage(char *buf, int sizeBuf, int &width, int &height, uint8 ** ppBuffer, bool bAlpha);

Loader::Loader()
{
}

Loader::~Loader()
{
}

Loader *Loader::GetInstance()
{
	static Loader myinstance;
	return &myinstance;
}


HRESULT Loader::LoadHoleFile(Terrain * pTerrain, const char * szFilename, short nHoleScale)
{
	CParaFile cFile;
	cFile.OpenAssetFile(szFilename, true, ParaTerrain::Settings::GetInstance()->GetMediaPath());
	if(cFile.isEof())
		return E_FAIL;

	/// Load from raw elevation 
	int nWidth = (int)sqrt((float)(((int)cFile.getSize()/sizeof(BYTE))));
	/// just use the file buffer as the image buffer
	BYTE* pImageData = (BYTE*)cFile.getBuffer();
	pTerrain->CreateHoleMap(pImageData, nWidth*nWidth);
	return S_OK;
}

HRESULT Loader::LoadElevations( float **ppImageData, int* nSize, const char * szFilename, bool swapVertical /*= true*/)
{
	float* pImageData = NULL;
	CParaFile cFile;
	cFile.OpenAssetFile(szFilename, true, ParaTerrain::Settings::GetInstance()->GetMediaPath());
	if(cFile.isEof())
		return E_FAIL;
	
	int elevWidth=0;
	int elevHeight=0;

	if(strcmp(szFilename+strlen(szFilename)-4, ".raw") == 0)
	{
		/// Load from raw elevation 
		elevWidth = (int)sqrt((float)(((int)cFile.getSize()/4)));
		elevHeight = elevWidth;
		/// just use the file buffer as the image buffer
		pImageData = (float*)cFile.getBuffer();
		cFile.GiveupBufferOwnership();
	}
	else
	{
		unsigned char* pPixels = nullptr;
		int bytesPerPixel = 0;
		if (!TextureEntity::LoadImageOfFormat(szFilename, cFile.getBuffer(), (int)cFile.getSize(), elevWidth, elevHeight, &pPixels, &bytesPerPixel, -1) || !pPixels)
		{
			ParaImage img;
			if (!img.initWithImageData(reinterpret_cast<const unsigned char*>(cFile.getBuffer()), cFile.getSize()))
				return E_FAIL;
			elevWidth = img.getWidth();
			elevHeight = img.getHeight();
			if (elevWidth <= 0 || elevHeight <= 0)
				return E_FAIL;
			pImageData = new (std::nothrow) float[elevWidth * elevHeight];
			if (!pImageData)
				return E_OUTOFMEMORY;

			unsigned char* rgba = nullptr;
			size_t rgbaLen = 0;
			if (ParaImage::convertDataToFormat(img.getData(), img.getDataLen(), img.getRenderFormat(), PixelFormat::A8R8G8B8, &rgba, &rgbaLen) != PixelFormat::A8R8G8B8 || !rgba)
			{
				delete[] pImageData;
				return E_FAIL;
			}
			for (int iY = 0; iY < elevHeight; ++iY)
			{
				for (int iX = 0; iX < elevWidth; ++iX)
				{
					const unsigned char* px = rgba + (static_cast<size_t>(iY) * elevWidth + iX) * 4u;
					const float r = px[0] / 255.0f;
					const float g = px[1] / 255.0f;
					const float b = px[2] / 255.0f;
					const float fValue = (r + g + b) / 3.0f - 0.5f;
					const int dstY = swapVertical ? (elevHeight - 1 - iY) : iY;
					pImageData[dstY * elevWidth + iX] = fValue;
				}
			}
			if (rgba != img.getData())
				delete[] rgba;
		}
		else
		{
			pImageData = new (std::nothrow) float[elevWidth * elevHeight];
			if (!pImageData)
			{
				delete[] pPixels;
				return E_OUTOFMEMORY;
			}
			const DWORD* pBuffer = reinterpret_cast<const DWORD*>(pPixels);
			for (int iY = 0; iY < elevHeight; ++iY)
			{
				for (int iX = 0; iX < elevWidth; ++iX)
				{
					LinearColor color(*pBuffer++);
					const float fValue = (color.r + color.g + color.b) / 3.0f - 0.5f;
					const int dstY = swapVertical ? (elevHeight - 1 - iY) : iY;
					pImageData[dstY * elevWidth + iX] = fValue;
				}
			}
			delete[] pPixels;
		}
	}
	// TODO: re-enable this when you are ready. 
	const bool FORCE_256_mesh = false;
	if(elevWidth == 129 && FORCE_256_mesh)
	{
		// make it 257. 
		int elevWidthDest = 257;
		
		float * pImageDataDest = new float[elevWidthDest * elevWidthDest];
		for (int i=0; i<elevWidthDest; i++)
		{
			for (int j=0; j<elevWidthDest; j++)
			{
				int rI = i%2; int ltX = i/2;
				int rJ = j%2; int ltY = j/2;
				if(rI == 0)
				{
					if(rJ == 0)
					{
						pImageDataDest[i*elevWidthDest+j] = pImageData[ltX*elevWidth+ltY];
					}
					else
					{
						pImageDataDest[i*elevWidthDest+j] = (pImageData[ltX*elevWidth+ltY+1] + pImageData[ltX*elevWidth+ltY]) * 0.5f;
					}
				}
				else
				{
					if(rJ == 0)
					{
						pImageDataDest[i*elevWidthDest+j] = (pImageData[(ltX+1)*elevWidth+ltY] + pImageData[ltX*elevWidth+ltY])*0.5f;
					}
					else
					{
						pImageDataDest[i*elevWidthDest+j] = (pImageData[(ltX+1)*elevWidth+ltY]+pImageData[ltX*elevWidth+ltY]+pImageData[ltX*elevWidth+ltY+1]+pImageData[(ltX+1)*elevWidth+ltY+1])*0.25f;
					}
				}
			}
		}
		SAFE_DELETE(pImageData);
		pImageData = pImageDataDest;
		elevWidth = elevWidthDest;
	}
	
	*nSize = elevWidth;
	*ppImageData = pImageData;
	return S_OK;
}

HRESULT Loader::LoadElevations( Terrain * pTerrain, const char * szFilename, float fTerrainSize, float elevationScale, bool swapVertical /*= true*/ )
{
    HRESULT hr;

	float *pImageData = NULL;

	int elevWidth = 0, elevHeight = 0;
	if(SUCCEEDED(hr = LoadElevations(&pImageData, &elevWidth, szFilename, swapVertical)))
	{
		if(pImageData != NULL)
		{
			elevHeight = elevWidth;
			// since the texture tile is always 8*8 per terrain object, the MaxBlockSize can not be smaller then this value. 
			int MaxBlockSize = (elevWidth-1) / 8;
			if(pTerrain->GetMaximumVisibleBlockSize() < MaxBlockSize)
			{
				pTerrain->SetMaximumVisibleBlockSize(MaxBlockSize);
			}

			pTerrain->SetAllElevations(pImageData, elevWidth, elevHeight, fTerrainSize, elevationScale);
			delete[]pImageData;
		}
	}
    return hr;
}

HRESULT Loader::LoadTerrainInfo(Terrain *pTerrain,const char* szFilename)
{
	CParaFile cFile;
	cFile.OpenAssetFile(szFilename,true,ParaTerrain::Settings::GetInstance()->GetMediaPath());
	if(cFile.isEof())
		return E_FAIL;

	int width;
	int height;
	if(strcmp(szFilename+strlen(szFilename)-8,"info.raw")==0)
	{
		width = (int)sqrt((float)(((int)cFile.getSize()/4)));
		height = width;

		uint32* pImgData = (uint32*)cFile.getBuffer();
		cFile.GiveupBufferOwnership();

		if(pImgData != NULL)
		{
			pTerrain->SetTerrainInfoData(pImgData,width);
			delete[] pImgData;
			return S_OK;
		}
	}
	return E_FAIL;
}


// obsoleted
void Loader::LoadTerrainTexture(Terrain * pTerrain, const char * fileName)
{
	// ApplyTexture(pTerrain, fileName, true);
	pTerrain->SetBaseTexture(fileName);
}

// obsoleted
void Loader::LoadCommonTerrainTexture(Terrain * pTerrain, const char * fileName)
{
	pTerrain->SetCommonTexture(fileName);
}

void Loader::ApplyTexture(Terrain * pTerrain, const char *szFilename, bool isBaseTexture)
{
	CParaFile cFile;
	cFile.OpenAssetFile(szFilename, true, ParaTerrain::Settings::GetInstance()->GetMediaPath());

	if(cFile.isEof())
		return;
	// Load the texture data.
	int texWidth, texHeight;
	uint8 *pTextureImage = NULL;
	
	LoadImage(cFile.getBuffer(), (int)cFile.getSize(), texWidth, texHeight, &pTextureImage, false);

	if (isBaseTexture)
		pTerrain->SetTexture(pTextureImage, texWidth, texHeight);
	else
		pTerrain->SetCommonTexture(pTextureImage, texWidth, texHeight);

	delete[]pTextureImage;
}

void LoadImage(char *buf, int sizeBuf, int &width, int &height, uint8 ** ppBuffer, bool bAlpha)
{
	width = 0;
	height = 0;
	*ppBuffer = NULL;
	TextureEntity::LoadImage(buf, sizeBuf, width, height, ppBuffer, bAlpha);
}



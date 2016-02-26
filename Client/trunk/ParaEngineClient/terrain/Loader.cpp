// Terrain Engine used in ParaEngine
// Based on the Demeter Terrain Visualization Library by Clay Fowler, 2002
// File marked with the above information inherits the GNU License from Demeter Terrain Engine.
// Other portion of ParaEngine is subjected to its own License.

#include "ParaEngine.h"
#include "ParaWorldAsset.h"
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
#ifdef USE_DIRECTX_RENDERER
		D3DSURFACE_DESC desc;
		IDirect3DTexture9* pTexture = NULL;
		HRESULT hr;

		/// Create a D3DFMT_X8R8G8B8 texture -- use D3DPOOL_SCRATCH to 
		// ensure that this will succeeded independent of the device
		hr = D3DXCreateTextureFromFileInMemoryEx( CGlobals::GetRenderDevice(), cFile.getBuffer(), (int)cFile.getSize(),
			0, 0, 1, 0, 
			D3DFMT_X8R8G8B8, D3DPOOL_SCRATCH, 
			D3DX_FILTER_NONE, D3DX_FILTER_NONE,
			0, NULL, NULL, &pTexture );
		if( FAILED(hr) )
			return hr;

		pTexture->GetLevelDesc( 0, &desc ); 

		elevWidth = desc.Width;
		elevHeight = desc.Height;

		// create an array of floats to store the values of the bmp
		pImageData = new float[elevWidth * elevHeight];

		if( pImageData == NULL )
		{
			SAFE_RELEASE( pTexture );
			return E_OUTOFMEMORY;
		}

		//ZeroMemory( pImageData, elevWidth * elevHeight * sizeof(FLOAT) );

		D3DLOCKED_RECT lockedRect;

		hr = pTexture->LockRect( 0, &lockedRect, NULL, D3DLOCK_READONLY );
		float fMin=1.0f,fMax=-1.0f;
		if( SUCCEEDED(hr) )
		{
			DWORD* pBuffer = (DWORD*) lockedRect.pBits;
			for( DWORD iY=0; iY<desc.Height; iY++ )
			{
				pBuffer = (DWORD*)((BYTE*)lockedRect.pBits + lockedRect.Pitch * iY);

				for( DWORD iX=0; iX<desc.Width; iX++ )
				{
					LinearColor color((DWORD)(*pBuffer));
					float fValue = (color.r+color.g+color.b)/3.0f-0.5f;
					if (fValue<fMin)
						fMin = fValue;
					if (fValue>fMax)
						fMax = fValue;

					if (swapVertical)
					{
						// Invert Y, so it appears the same as the bmp
						pImageData[ (desc.Height-1-iY)*elevWidth + iX] = fValue;
					}
					else
					{
						pImageData[ iY*elevWidth + iX] = fValue;
					}
					pBuffer++;
				}
			}
			pTexture->UnlockRect( 0 );

			/// Normalize all values between 0.0f and 1.0f
			/*float fHeight = (fMax-fMin);
			for( DWORD iY=0; iY<desc.Height; iY++ )
			{
			for( DWORD iX=0; iX<desc.Width; iX++ )
			{
			pImageData[ iY*elevWidth + iX] = (pImageData[ iY*elevWidth + iX]-fMin)/fHeight;
			}
			}*/
		}
		SAFE_RELEASE( pTexture );
#else 
		return E_FAIL;
#endif
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
#ifdef USE_DIRECTX_RENDERER
	HRESULT hr;
	D3DSURFACE_DESC desc;
	IDirect3DTexture9* pTexture = NULL;

	D3DFORMAT d3dFormat;
	if (bAlpha)
		d3dFormat = D3DFMT_A8R8G8B8;
	else
		d3dFormat = D3DFMT_R8G8B8;
	
	// read from file
	hr = D3DXCreateTextureFromFileInMemoryEx( CGlobals::GetRenderDevice(), buf, sizeBuf,
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

	uint8 *pBufferTemp;
	int nSize;
	if (bAlpha)
		nSize = width * height * 4;
	else
		nSize = width * height * 3;
	pBufferTemp = new uint8[nSize];

	D3DLOCKED_RECT lockedRect;

	hr = pTexture->LockRect( 0, &lockedRect, NULL, D3DLOCK_READONLY );
	if( SUCCEEDED(hr) )
	{
		uint8 *pImagePixels = (uint8 *) lockedRect.pBits;
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
	}

	SAFE_RELEASE( pTexture );
#endif
}



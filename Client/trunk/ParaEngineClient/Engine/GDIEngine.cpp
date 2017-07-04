//-----------------------------------------------------------------------------
// Class:	GDIEngine
// Authors:	Li, Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2009.8.27
// Note: in order for GDIEngine to work one needs to call Gdiplus::GdiplusStartup() and Gdiplus::GdiplusShutdown();
// as in the ParaEngineApp.cpp
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#ifdef USE_FREEIMAGE
#include "FreeImageMemIO.h"
#endif
#include "TextureEntity.h"
#include "GDIEngine.h"

using namespace ParaEngine;
using namespace Gdiplus;

CGDIEngine::CGDIEngine()
: m_hDC(NULL)
{
}

CGDIEngine::~CGDIEngine(void)
{
}


void CGDIEngine::TestMe()
{
	CGDIEngine engine;
	engine.Create();
	engine.Begin();
	engine.Clear(0x00000000);

	float image_width = 80.f;
	float image_height = 80.f;
	engine.DrawImage(engine.LoadTexture("Texture/16number.png"), 10,10, image_width, image_width);
	// Create a rotation matrix.
	Matrix transformMatrix;
	transformMatrix.Rotate(45.0f);
	transformMatrix.Translate(image_width/2, image_height/2, MatrixOrderAppend);
	engine.SetTransform(&transformMatrix);
	engine.DrawImage(engine.LoadTexture("Texture/16number.png"), -image_width/2, -image_height/2, image_width, image_width);

	transformMatrix.Reset();
	engine.SetTransform(&transformMatrix);
	engine.DrawImage(engine.LoadTexture("Texture/16number.png"), 110,110, 80, 80);

	transformMatrix.Reset();
	transformMatrix.Scale(2.0f, 2.0f); // scale around center
	transformMatrix.Rotate(45.0f); // rotate around center
	transformMatrix.Translate(image_width/2+110, image_height/2+110, MatrixOrderAppend); // translate
	engine.SetTransform(&transformMatrix);
	engine.DrawImage(engine.LoadTexture("Texture/16number.png"), -image_width/2, -image_height/2, image_width, image_width);

	engine.SaveRenderTarget("temp/GDI_output_ok.png");
	engine.End();
}


HRESULT CGDIEngine::Create()
{
	// Start clean
	Destroy();

	m_pDefaultRenderTarget = CreateGetRenderTarget("256", 256);
	SetRenderTarget(m_pDefaultRenderTarget);

	return S_OK;
}

bool CGDIEngine::SaveRenderTarget(const string& filename, int nWidth, int nHeight, bool bHasAlpha, DWORD colorKey)
{
#ifdef USE_FREEIMAGE
	if(!IsValid())
		return false;

	int nRenderTargetSize = m_pRenderTarget->GetRenderTargetSize();

	FIBITMAP *dest_dib = FreeImage_Allocate(nRenderTargetSize,nRenderTargetSize,32);

	Rect rect(0, 0, nRenderTargetSize, nRenderTargetSize);
	BitmapData bitmapData;
	bitmapData.Width = nRenderTargetSize;
	bitmapData.Height = nRenderTargetSize;
	bitmapData.Stride = nRenderTargetSize*4;
	bitmapData.PixelFormat = PixelFormat32bppARGB;
	bitmapData.Scan0 = 0;

	BYTE* pPixels = FreeImage_GetBits(dest_dib);
	// Get the data from the gdi+ object.
	if(m_pRenderTarget->GetBitmapSurface()->LockBits(&rect,ImageLockModeRead,PixelFormat32bppARGB,&bitmapData) == Ok)
	{
		int DestinationStride = -bitmapData.Stride;
		int BytesPerRow = bitmapData.Stride;
		BYTE *pCurrentSourceByte = (BYTE *)bitmapData.Scan0;
		// Our DIBsection is bottom-up...start at the bottom row...
		BYTE *pCurrentDestinationByte = pPixels + ((bitmapData.Height - 1) * BytesPerRow);
		for (UINT CurY = 0; CurY < bitmapData.Height; ++CurY)
		{
			// copy one row of pixel data
			if(colorKey == 0)
			{
				// no color key
				memcpy(pCurrentDestinationByte, pCurrentSourceByte, BytesPerRow);
			}
			else
			{
				// use color key
				DWORD * pDestColor = (DWORD*)pCurrentDestinationByte;
				DWORD * pSrcColor = (DWORD*)pCurrentSourceByte;
				for (UINT CurX = 0; CurX < bitmapData.Width; ++CurX, ++pSrcColor, ++pDestColor)
				{
					DWORD color = (*pSrcColor);
					*pDestColor = (color != colorKey) ? color : 0;
				}
			}
			pCurrentSourceByte += bitmapData.Stride;
			pCurrentDestinationByte += DestinationStride;
		}
		m_pRenderTarget->GetBitmapSurface()->UnlockBits(&bitmapData);
	}

	FREE_IMAGE_FORMAT dwTextureFormat = FIF_PNG;
	int nSize = (int)filename.size();
	if(nSize>4)
	{
		// to lower case
#define MAKE_LOWER(c)  if((c)>='A' && (c)<='Z'){(c) = (c)-'A'+'a';}
		char c1 = filename[nSize-3];MAKE_LOWER(c1)
		char c2 = filename[nSize-2];MAKE_LOWER(c2)
		char c3 = filename[nSize-1];MAKE_LOWER(c3)

		if(c1=='d' && c2=='d' && c3=='s')
			dwTextureFormat = FIF_DDS;
		else if(c1=='p' && c2=='n' && c3=='g')
			dwTextureFormat = FIF_PNG;
		else if(c1=='j' && c2=='p' && c3=='g')
			dwTextureFormat = FIF_JPEG;
		else if(c1=='t' && c2=='g' && c3=='a')
			dwTextureFormat = FIF_TARGA;
	}

	bool bSucceed = !!FreeImage_Save( dwTextureFormat, dest_dib, filename.c_str());
	FreeImage_Unload(dest_dib);
	if(!bSucceed)
	{
		OUTPUT_LOG("warning: failed saving GDI render target to file %s\n", filename.c_str());
	}
	return bSucceed;
#else
	return false;
#endif
}

bool CGDIEngine::Begin()
{
	if(!IsValid())
		return false;
	m_pRenderTarget->GetGraphics()->ResetTransform();
	return true;
}

bool CGDIEngine::DrawImage(Gdiplus::Image * pImage, float x, float y,float width, float height)
{
	if(!IsValid() || pImage == 0)
		return false;
	m_pRenderTarget->GetGraphics()->DrawImage( pImage, x,y,width, height);
	return true;
}

bool CGDIEngine::DrawImage(Gdiplus::Image *pImage, float x, float y,float width, float height, DWORD dwColor)
{
	if(dwColor == 0xffffffff)
		return DrawImage(pImage, x, y,width, height);

	if(!IsValid() || pImage == 0)
		return false;

	LinearColor imageColor(dwColor);

	ColorMatrix matColor = {
		imageColor.r, 0.f, 0.f, 0.f, 0.f,
		0.f, imageColor.g, 0.f, 0.f, 0.f,
		0.f, 0.f, imageColor.b, 0.f, 0.f,
		0.f, 0.f, 0.f, imageColor.a, 0.f,
		0.f, 0.f, 0.f, 0.f, 1.f
	};
	ImageAttributes attr;
	attr.SetColorMatrix(&matColor, ColorMatrixFlagsDefault, ColorAdjustTypeDefault);

	RectF destination(x,y,width, height);
	m_pRenderTarget->GetGraphics()->DrawImage(pImage, destination, 0.f, 0.f, (float)pImage->GetWidth(), (float)pImage->GetHeight(), (Unit)UnitPixel, &attr);
	return true;
}

void CGDIEngine::End()
{
	if(!IsValid())
		return;
}

bool CGDIEngine::SetRenderTarget(CGDIRenderTarget_ptr pRenderTarget)
{
	m_pRenderTarget = pRenderTarget;
	return true;
}

bool CGDIEngine::IsValid()
{
	return (m_pRenderTarget && m_pRenderTarget->IsValid());
}

HRESULT CGDIEngine::Destroy()
{
	// release all textures
	TextureAsset_Map_Type::iterator itCur, itEnd = m_textures.end();
	for(itCur = m_textures.begin(); itCur != itEnd; ++itCur)
	{
		SAFE_DELETE(itCur->second);
	}
	m_textures.clear();

	m_render_targets.clear();
	return S_OK;
}


ParaEngine::CGDIRenderTarget_ptr ParaEngine::CGDIEngine::CreateGetRenderTarget( const std::string& sName, int nRenderTargetSize /*= 256*/ )
{
	RenderTarget_Map_Type::iterator itCur =  m_render_targets.find(sName);
	if(itCur!=m_render_targets.end())
	{
		return itCur->second;
	}
	else
	{
		m_render_targets[sName] = new CGDIRenderTarget(nRenderTargetSize);
		return m_render_targets[sName];
	}
}

ParaEngine::CGDIRenderTarget_ptr ParaEngine::CGDIEngine::CreateGetRenderTargetBySize( int nRenderTargetSize /*= 256*/ )
{
	if(nRenderTargetSize == 128)
	{
		return CreateGetRenderTarget("128", nRenderTargetSize);
	}
	else if(nRenderTargetSize == 256)
	{
		return CreateGetRenderTarget("256", nRenderTargetSize);
	}
	else if(nRenderTargetSize == 512)
	{
		return CreateGetRenderTarget("512", nRenderTargetSize);
	}
	else if(nRenderTargetSize == 1024)
	{
		return CreateGetRenderTarget("1024", nRenderTargetSize);
	}
	else
	{
		char sName[16];
		itoa(nRenderTargetSize, sName, 10);
		return CreateGetRenderTarget(sName, nRenderTargetSize);
	}
}


Gdiplus::Bitmap* CGDIEngine::LoadTexture(const string& filename, int nFileFormat)
{
	TextureAsset_Map_Type::iterator itCur = m_textures.find(filename);
	if(itCur != m_textures.end())
		return itCur->second;
#ifdef USE_FREEIMAGE
	FREE_IMAGE_FORMAT dwTextureFormat = (FREE_IMAGE_FORMAT) nFileFormat;

	CParaFile file(filename.c_str());
	if(file.isEof())
	{
		OUTPUT_LOG("warning: GDIEngine can not load file %s \n", filename.c_str());
		m_textures[filename] = NULL;
		return NULL;
	}

	MemIO memIO((BYTE*)(file.getBuffer()), file.getSize());
	FIBITMAP *dib = FreeImage_LoadFromHandle( dwTextureFormat, &memIO, (fi_handle)&memIO );
	if(dib == 0)
	{
		OUTPUT_LOG("warning: GDIEngine can not load file %s \n", filename.c_str());
		m_textures[filename] = NULL;
		return NULL;
	}
	
	BITMAPINFOHEADER* pInfo = FreeImage_GetInfoHeader(dib);
	if(pInfo->biBitCount != 32)
	{
		FIBITMAP* dib_old = dib;
		dib = FreeImage_ConvertTo32Bits(dib);
		if(dib==0)
		{
			FreeImage_Unload(dib);
			OUTPUT_LOG("warning: GDIEngine can not convert file %s to 32bits image\n", filename.c_str());
			m_textures[filename] = NULL;
			return NULL;
		}
		else
		{
			pInfo = FreeImage_GetInfoHeader(dib);
			assert(pInfo->biBitCount == 32);
		}
		// old one is not used. 
		FreeImage_Unload(dib_old);
	}
	
	Bitmap* pImage = new Bitmap(pInfo->biWidth, pInfo->biHeight,PixelFormat32bppARGB);
	if(pImage != 0)
	{
		BYTE* pPixels = FreeImage_GetBits(dib);

		Rect rect(0, 0, pInfo->biWidth, pInfo->biHeight);
		BitmapData bitmapData;
		bitmapData.Width = pInfo->biWidth;
		bitmapData.Height = pInfo->biHeight;
		bitmapData.Stride = pInfo->biWidth*4;
		bitmapData.PixelFormat = PixelFormat32bppARGB;
		bitmapData.Scan0 = 0;
		int BytesPerRow = bitmapData.Stride;
		if(pImage->LockBits(&rect,ImageLockModeWrite,PixelFormat32bppARGB,&bitmapData) == Ok)
		{
			// copy one row of pixel data
			BYTE *pCurrentDestinationByte = (BYTE *)bitmapData.Scan0;
			BYTE *pCurrentSourceByte= pPixels + ((bitmapData.Height - 1) * BytesPerRow);
			// Our DIBsection is bottom-up...start at the bottom row...
			int SourceStride = - bitmapData.Stride;
			for (UINT CurY = 0; CurY < bitmapData.Height; ++CurY)
			{
				memcpy(pCurrentDestinationByte, pCurrentSourceByte, BytesPerRow);
				pCurrentSourceByte += SourceStride;
				pCurrentDestinationByte += BytesPerRow;
			}
			pImage->UnlockBits(&bitmapData);
		}
	}
	FreeImage_Unload(dib);
	m_textures[filename] = pImage;
	return pImage;
#else
	return NULL;;
#endif
}

Gdiplus::Bitmap* CGDIEngine::LoadTexture(const string& filename)
{
	return LoadTexture(filename, filename);
}

Gdiplus::Bitmap* CGDIEngine::LoadTexture(const string& filename, const string& fileextension)
{
	return LoadTexture(filename, TextureEntity::GetFormatByFileName(fileextension));
}

HRESULT CGDIEngine::Clear(const ParaEngine::Color &color)
{
	if(!IsValid())
		return E_FAIL;

	int nRenderTargetSize = m_pRenderTarget->GetRenderTargetSize();
	Rect rect(0, 0, nRenderTargetSize, nRenderTargetSize);
	BitmapData bitmapData;
	bitmapData.Width = nRenderTargetSize;
	bitmapData.Height = nRenderTargetSize;
	bitmapData.Stride = nRenderTargetSize*4;
	bitmapData.PixelFormat = PixelFormat32bppARGB;
	bitmapData.Scan0 = 0;
	int BytesPerRow = bitmapData.Stride;
	if(m_pRenderTarget->GetBitmapSurface()->LockBits(&rect,ImageLockModeWrite,PixelFormat32bppARGB,&bitmapData) == Ok)
	{
		// copy one row of pixel data
		BYTE *pCurrentDestinationByte = (BYTE *)bitmapData.Scan0;
		for (UINT CurY = 0; CurY < bitmapData.Height; ++CurY)
		{
			for (UINT CurX = 0; CurX < bitmapData.Width; ++CurX)
			{
				*(((DWORD*)pCurrentDestinationByte) + CurX) = color;
			}
			pCurrentDestinationByte += BytesPerRow;
		}
		m_pRenderTarget->GetBitmapSurface()->UnlockBits(&bitmapData);
	}
	return S_OK;
}

void CGDIEngine::SetTransform(const Gdiplus::Matrix * matTransform)
{
	if(!IsValid())
		return;
	m_pRenderTarget->GetGraphics()->SetTransform(matTransform);
}

//////////////////////////////////////////////////////////////
//
// CGDIRenderTarget
//
//////////////////////////////////////////////////////////////
ParaEngine::CGDIRenderTarget::CGDIRenderTarget( int nImageSize /*= 256*/ )
	:m_nRenderTargetSize(0), m_pGraphics(NULL), m_pBitmapSurface(NULL)
{
	if(nImageSize>0)
	{
		Init(nImageSize);
	}
}

ParaEngine::CGDIRenderTarget::~CGDIRenderTarget()
{
	
}

void ParaEngine::CGDIRenderTarget::Cleanup()
{
	if(m_pGraphics || m_pBitmapSurface)
	{
		SAFE_DELETE(m_pGraphics);
		SAFE_DELETE(m_pBitmapSurface);
		OUTPUT_LOG("CGDIRenderTarget is successfully destroyed\n");
	}
}

void ParaEngine::CGDIRenderTarget::Init( int nImageSize )
{
	if(m_nRenderTargetSize == nImageSize)
		return;
	Cleanup();

	m_nRenderTargetSize = nImageSize;
	m_pBitmapSurface = new Bitmap(m_nRenderTargetSize, m_nRenderTargetSize, PixelFormat32bppARGB);
	m_pGraphics = new Graphics(m_pBitmapSurface);
	OUTPUT_LOG("CGDIRenderTarget of size %d is successfully created\n", m_nRenderTargetSize);
}

bool ParaEngine::CGDIRenderTarget::IsValid()
{
	return (m_pGraphics!=0 && m_pBitmapSurface!=0);
}

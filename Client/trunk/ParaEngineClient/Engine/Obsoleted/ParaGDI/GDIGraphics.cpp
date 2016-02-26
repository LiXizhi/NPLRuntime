//----------------------------------------------------------------------
// Class:	CGDIGraphics
// Authors:	Liu Weili
// Date:	2005.12.20
// Revised: rewritten by LiXizhi 2007.1.13
//
// desc: 
//-----------------------------------------------------------------------
#include "ParaEngine.h"
#include "ParaWorldAsset.h"
#include "../GUIResource.h"
#include "GDIGraphics.h"
#include "GDIPen.h"
#include "GDIDC.h"
#include "GDIBrush.h"
#include "memdebug.h"

using namespace ParaEngine;
//////////////////////////////////////////////////////////////////////////
// CGDIGraphics
//////////////////////////////////////////////////////////////////////////
CGDIGraphics::CGDIGraphics()
{
	m_pDC=NULL;
	m_bLocked=false;
	m_bInvalidated = false;
}

CGDIGraphics::~CGDIGraphics()
{
	SAFE_DELETE(m_pDC);
}


void CGDIGraphics::Invalidate( bool bInvalidate /*= true*/ )
{
	m_bInvalidated = bInvalidate;
}

bool CGDIGraphics::IsInvalidated()
{
	return m_bInvalidated;
}

void CGDIGraphics::Reset()
{
	m_path.Reset();
	Invalidate(true);
}

HRESULT CGDIGraphics::Clear(const Color &color)
{
	if (m_bLocked) {
		return ObjectBusy;
	}
	m_path.AddClear(color);
	return S_OK;
}
void CGDIGraphics::Undo(int nStep)
{
	m_path.Undo(nStep);
}

void CGDIGraphics::Redo(int nStep)
{
	m_path.Redo(nStep);
}

CGDIPen* CGDIGraphics::CreatePen( string name )
{
	CGDIPen* pPen = GetPen(name);
	if(pPen==0)
	{
		m_pens[name] = CGDIPen();
		return &(m_pens[name]);
	}
	return pPen;
}

CGDIPen* CGDIGraphics::GetPen( string name )
{
	map<string, CGDIPen>::iterator iter = m_pens.find(name);
	if(iter!=m_pens.end())
	{
		return &(iter->second);
	}
	return NULL;
}

void CGDIGraphics::SetClip(const CGDIRegion &region)
{
	m_region=region;
	m_path.SetClip(m_region);

}

void CGDIGraphics::SetRenderTarget( TextureEntity * pEntity )
{
	SAFE_DELETE(m_pDC);
	m_pDC=new CGDIDC(pEntity);
}

TextureEntity* CGDIGraphics::GetRenderTarget()
{
	if(m_pDC)
	{
		return m_pDC->GetRenderTarget();
	}
	return NULL;
}

HRESULT CGDIGraphics::DrawLine(const CGDIPen *pPen,const Vector3 &point1,const Vector3 &point2)
{
	if(pPen==0)
		pPen = GetCurrentPen();
	assert(pPen!=0);

	if (m_bLocked) {
		return ObjectBusy;
	}
	
	m_path.AddLine(point1,point2,*pPen);
	
	return S_OK;
}

HRESULT CGDIGraphics::DrawLines(const CGDIPen *pPen, const Vector3 *points,int nNumPoints)
{
	if(pPen==0)
		pPen = GetCurrentPen();
	assert(pPen!=0);

	if (m_bLocked) {
		return ObjectBusy;
	}
	m_path.AddLines(points,nNumPoints,*pPen);

	return S_OK;
}

HRESULT CGDIGraphics::DrawPoint(const CGDIPen *pPen,const Vector3 &point)
{
	if(pPen==0)
		pPen = GetCurrentPen();
	assert(pPen!=0);
	if (m_bLocked) {
		return ObjectBusy;
	}
	m_path.AddPoint(point,*pPen);
	return S_OK;
}
HRESULT CGDIGraphics::Render()
{
	if(m_pDC==0 || m_pDC->GetRenderTarget()==0)
		return E_FAIL;

	if (m_bLocked) {
		return ObjectBusy;
	}
	
	const GDIFigure *tempfigure;
	CalculatePath(m_path);
	if (m_path.FigureBegin()==m_path.FigureEnd() && m_pDiskTexture==0) {
		return S_OK;
	}
	LPDIRECT3DDEVICE9 pd3dDevice = CGlobals::GetRenderDevice();

	if(SUCCEEDED(m_pDC->Begin()))
	{
		//////////////////////////////////////////////////////////////////////////
		// the background texture
		if(m_pDiskTexture)
		{
			// m_pDC->Clear(Color(0x00000000));
			TextureEntity* pRenderTarget = m_pDC->GetRenderTarget();
			RECT rcScreen;
			rcScreen.left = 0;
			rcScreen.top = 0;
			rcScreen.right = pRenderTarget->GetTextureInfo()->m_width;
			rcScreen.bottom = pRenderTarget->GetTextureInfo()->m_height;
			float depth = 0.f;
			Color color = 0xffffffff;
			float fShift = -0.5f;
			DXUT_SCREEN_VERTEX vertices[4] =
			{
				(float) rcScreen.left +fShift,  (float) rcScreen.top +fShift,    depth , 1.0f, color, 0, 0,
				(float) rcScreen.right +fShift, (float) rcScreen.top +fShift,    depth, 1.0f, color, 1, 0, 
				(float) rcScreen.left +fShift,  (float) rcScreen.bottom +fShift, depth , 1.0f, color, 0, 1,
				(float) rcScreen.right +fShift, (float) rcScreen.bottom +fShift, depth , 1.0f, color, 1, 1,
			};
			// we will write alpha of original texture to render target. 
			pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
			pd3dDevice->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, TRUE);
			pd3dDevice->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_SRCALPHA);
			m_pDC->DrawTriangleStrip(m_pDiskTexture.get(),4,vertices);
			pd3dDevice->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, FALSE);
			pd3dDevice->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ONE);
		}

		list<GDIFigure>::const_iterator iter;
		
		for (iter=m_path.FigureBegin();iter!=m_path.FigureEnd();iter++) 
		{
			tempfigure=&(*iter);
			if (tempfigure->pElement->pen.IsTransparent())
			{
				// for fully transparent brush
				pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);
				pd3dDevice->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, TRUE);
				pd3dDevice->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_SRCALPHA);
			}

			switch(tempfigure->operationtype) 
			{
			case PathPointTypeOperationClear:
				m_pDC->Clear(tempfigure->pElement->color);
				break;
			case PathPointTypeRectangle:
				m_pDC->SetTransform(&tempfigure->matTransform);
				switch (tempfigure->PrimitiveType) {
				case D3DPT_TRIANGLESTRIP:
					m_pDC->DrawTriangleStrip(tempfigure->pElement->pen.GetBrush()->GetTexture(), (UINT)tempfigure->vertices.size(), &tempfigure->vertices.front());
					break;
				case D3DPT_TRIANGLELIST:
					m_pDC->DrawTriangleList(tempfigure->pElement->pen.GetBrush()->GetTexture(), (UINT)tempfigure->vertices.size(), &tempfigure->vertices.front());
					break;
				}
				m_pDC->SetTransform(&Matrix4::IDENTITY);
				break;
			default:
				switch (tempfigure->PrimitiveType) {
				case D3DPT_TRIANGLESTRIP:
					m_pDC->DrawTriangleStrip(tempfigure->pElement->pen.GetBrush()->GetTexture(), (UINT)tempfigure->vertices.size(), &tempfigure->vertices.front());
					break;
				case D3DPT_TRIANGLELIST:
					m_pDC->DrawTriangleList(tempfigure->pElement->pen.GetBrush()->GetTexture(), (UINT)tempfigure->vertices.size(), &tempfigure->vertices.front());
					break;
				}
			}
			if (tempfigure->pElement->pen.IsTransparent())
			{
				pd3dDevice->SetRenderState(D3DRS_SEPARATEALPHABLENDENABLE, FALSE);
				pd3dDevice->SetRenderState(D3DRS_SRCBLENDALPHA, D3DBLEND_ONE);
			}
		}
		m_pDC->End();
	}

	return S_OK;
};


HRESULT CGDIGraphics::RestoreDeviceObjects()
{
	return S_OK;
}

HRESULT CGDIGraphics::InvalidateDeviceObjects()
{
	// automatically invalidate
	Invalidate(true);
	return S_OK;
}

HRESULT CGDIGraphics::CalculatePath(CGDIGraphicsPath &pPath)
{
	if (&pPath==NULL) {
		return E_INVALIDARG;
	}
	pPath.SetClip(m_region);
	pPath.UpdateVertices();
	
	return S_OK;
}


bool CGDIGraphics::LoadFromTexture( TextureEntity* pTexture )
{
	if(pTexture)
	{
		m_pDiskTexture = pTexture;
		if(pTexture)
		{
			// force reloading the texture from file.
			pTexture->UnloadAsset();
		}
		Reset();
		return true;
	}
	return false;
}

bool CGDIGraphics::SaveAs( const char* filename )
{
	if(m_pDC && filename)
	{
		string sFile = filename;
		string sExt = CParaFile::GetFileExtension(sFile);

		D3DXIMAGE_FILEFORMAT FileFormat = D3DXIFF_PNG;

		if(sExt == "dds")
		{
			FileFormat = D3DXIFF_DDS;
			// TODO: we need to copy render target to disk file.
			//m_pDC->GetRenderTarget()->GetTexture()->GenerateMipSubLevels();
		}
		else if(sExt == "jpg")
		{
			FileFormat = D3DXIFF_JPG;
		}
		else // if(sExt == "png")
		{
			FileFormat = D3DXIFF_PNG;
		}
		if(SUCCEEDED(D3DXSaveTextureToFile(sFile.c_str(),FileFormat, m_pDC->GetRenderTarget()->GetTexture(), NULL )))
		{
			bool bSuc = true;
			if(sExt == "dds")
			{
				// if it is dds file, we will ensure mipmap is generated for it. 
				bSuc = false;
				LPDIRECT3DTEXTURE9 pTexture = NULL;
				if(SUCCEEDED(D3DXCreateTextureFromFileEx(CGlobals::GetRenderDevice(), sFile.c_str(), D3DX_DEFAULT, D3DX_DEFAULT,
					0,0, /* assume no alpha */ D3DFMT_DXT1, D3DPOOL_MANAGED, D3DX_DEFAULT, D3DX_DEFAULT, 0, NULL, NULL,&pTexture )))
				{
					// generate mip-mapping and save to file.
					pTexture->GenerateMipSubLevels();
					if( SUCCEEDED( D3DXSaveTextureToFile( sFile.c_str(), D3DXIFF_DDS, pTexture, NULL ) ) )
					{
						bSuc = true;
						OUTPUT_LOG("GDI graphics is saved to %s \r\n", sFile.c_str());
					}
					SAFE_RELEASE(pTexture);
				}
			}
			TextureEntity* pTex = CGlobals::GetAssetManager()->LoadTexture("", filename, TextureEntity::StaticTexture);
			LoadFromTexture(pTex);
			return true;
		}
	}
	return false;
}

TextureEntity* CGDIGraphics::GetDiskTexture()
{
	return m_pDiskTexture.get();
}
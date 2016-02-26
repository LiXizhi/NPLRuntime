//----------------------------------------------------------------------
// Class:	CGDIBrush
// Authors:	Liu Weili
// Date:	2005.12.20
// Revised: 2005.12.20
//
// desc: 
//-----------------------------------------------------------------------
#include "ParaEngine.h"
#include "../GUIResource.h"
#include "GDIMisc.h"
#include "GDIBrush.h"
#include "memdebug.h"
using namespace ParaEngine;


//////////////////////////////////////////////////////////////////////////
// CGDIBrush
//////////////////////////////////////////////////////////////////////////
CGDIBrush* CGDIBrush::Clone()const 
{
	CGDIBrush *newbrush=new CGDIBrush();
	newbrush->m_pTexture=m_pTexture;
	newbrush->m_eType=m_eType;
	newbrush->m_rect=m_rect;
	return newbrush;
}
CGDIBrush::~CGDIBrush()
{
}
CGDIBrush &CGDIBrush::operator =(const CGDIBrush &brush)
{
	m_pTexture=brush.m_pTexture;
	m_eType=brush.m_eType;
	m_rect=brush.m_rect;
	return *this;
}

bool CGDIBrush::operator ==(const CGDIBrush &brush)
{
	return (m_eType==brush.m_eType&&EqualRect(&m_rect,&brush.m_rect)&&m_pTexture==m_pTexture);
}
//////////////////////////////////////////////////////////////////////////
// CGDISolidBrush
//////////////////////////////////////////////////////////////////////////
CGDISolidBrush::CGDISolidBrush(const Color &color)
{
	m_eType=BrushTypeSolidColor;
	SetColor(color);
	m_pTexture=NULL;
		/*CGlobals::GetAssetManager()->LoadTexture("__painter_solid", 
		"Texture/painter_solid.png",TextureEntity::StaticTexture);
	GUITextureElement temp;
	temp.SetElement(m_pTexture,NULL,m_color);
	m_rect=temp.rcTexture;*/
}

//////////////////////////////////////////////////////////////////////////
// CGDITextureBrush
//////////////////////////////////////////////////////////////////////////
CGDITextureBrush::CGDITextureBrush(TextureEntity *pTexture, const RECT *rect)
{
	if (rect==NULL) {
		SetRect(&m_rect,0,0,pTexture->GetTextureInfo()->m_width,pTexture->GetTextureInfo()->m_height);
	}else
	{
		m_rect=*rect;
	}
	m_eType=BrushTypeTextureFill;
	m_pTexture=pTexture;
}

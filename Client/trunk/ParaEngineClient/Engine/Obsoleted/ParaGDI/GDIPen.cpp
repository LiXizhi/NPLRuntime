//----------------------------------------------------------------------
// Class:	CGDIPen
// Authors:	Liu Weili
// Date:	2005.12.20
// Revised: 2005.12.20
//
// desc: 
//-----------------------------------------------------------------------
#include "ParaEngine.h"
#include "GDIMisc.h"
#include "GDIBrush.h"
#include "GDIPen.h"
#include "memdebug.h"

using namespace ParaEngine;

//////////////////////////////////////////////////////////////////////////
// CGDIPen
//////////////////////////////////////////////////////////////////////////
CGDIPen::CGDIPen()
{
	Color color=0xff000000;
	m_brush=CGDISolidBrush(color);
	m_fWidth=1;
	m_color=color;
	m_eLineEndCap=m_eLineStartCap=LineCapSquare;
	m_eScale=PenScaleNone;
}
CGDIPen::CGDIPen(Color color, float width/* =1.0 */)
{
	m_brush=CGDISolidBrush(color);
	m_fWidth=width;
	m_color=color;
	m_eLineEndCap=m_eLineStartCap=LineCapSquare;
	m_eScale=PenScaleNone;
}

CGDIPen::CGDIPen(const CGDIBrush &brush,float width/* =1.0 */)
{
	m_brush=brush;
	m_fWidth=width;
	m_color=0xffffffff;
	m_eLineEndCap=m_eLineStartCap=LineCapSquare;
	m_eScale=PenScaleNone;
}

CGDIPen::CGDIPen(TextureEntity *pTexture, float width)
{
	m_brush = CGDITextureBrush(pTexture, NULL);
	m_fWidth=width;
	m_color=0xffffffff;
	m_eLineEndCap=m_eLineStartCap=LineCapSquare;
	m_eScale=PenScaleNone;
}

CGDIPen::~CGDIPen()
{
}
CGDIPen &CGDIPen::operator =(const CGDIPen &pen)
{
	m_brush=pen.m_brush;
	m_color=pen.m_color;
	m_fWidth=pen.m_fWidth;
	m_eLineEndCap=pen.m_eLineEndCap;
	m_eLineStartCap=pen.m_eLineStartCap;
	return *this;
}
bool CGDIPen::operator ==(const CGDIPen &pen)
{
	return (m_brush==pen.m_brush&&m_fWidth==pen.m_fWidth&&m_color==pen.m_color);
}


CGDIPen &CGDIPen::GetDefaultPen()
{
	static CGDIPen pen=CGDIPen();
	return pen;
}
CGDIPen *CGDIPen::Clone()const
{
	CGDIPen *newpen=new CGDIPen(m_brush,m_fWidth);
	newpen->SetColor(m_color);
	return newpen;
}

bool ParaEngine::CGDIPen::IsTransparent() const 
{
	return (m_color & 0xff000000) == 0;
}

void ParaEngine::CGDIPen::SetColor(const Color &color)
{
	m_color = color;
}

Color ParaEngine::CGDIPen::GetColor() const
{
	return m_color;
}

CGDIBrush* ParaEngine::CGDIPen::GetBrush() const
{
	return (CGDIBrush*)&m_brush;
}

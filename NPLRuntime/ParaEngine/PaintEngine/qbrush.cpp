//----------------------------------------------------------------------
// Class:	
// Authors:	LiXizhi
// company: ParaEngine
// Date:	2015.3.5
// Revised: 
//-----------------------------------------------------------------------
#include "ParaEngine.h"
#include "qbrush.h"

using namespace ParaEngine;

ParaEngine::QBrush::QBrush()
{
	init(Color::Black, NoBrush);
}

ParaEngine::QBrush::QBrush(BrushStyle bs)
{
	init(Color::Black, bs);
}

ParaEngine::QBrush::QBrush(const Color &color, BrushStyle bs /*= SolidPattern*/)
{
	init(color, bs);
}

ParaEngine::QBrush::QBrush(const Color &color, TextureEntity* pTexture)
{
	init(color, TexturePattern);
	setTexture(pTexture);
}

ParaEngine::QBrush::QBrush(TextureEntity* pTexture)
{
	init(Color::Black, TexturePattern);
	setTexture(pTexture);
}

ParaEngine::QBrush::~QBrush()
{
}

void ParaEngine::QBrush::setStyle(BrushStyle s)
{
	m_style = s;
}

void ParaEngine::QBrush::setMatrix(const QMatrix &mat)
{
	setTransform(QTransform(mat));
}

void ParaEngine::QBrush::setTransform(const QTransform & t)
{
	m_transform = t;
}

void ParaEngine::QBrush::setTexture(TextureEntity* pTexture)
{
	m_pTexture = pTexture;
}

TextureEntity* ParaEngine::QBrush::texture() const
{
	return m_pTexture.get();
}

void ParaEngine::QBrush::setColor(const Color &color)
{
	m_color = color;
}

bool ParaEngine::QBrush::isOpaque() const
{
	return m_color.a == 0;
}

bool ParaEngine::QBrush::operator==(const QBrush &b) const
{
	return m_pTexture == b.m_pTexture && m_color == b.m_color && m_style == b.m_style && m_transform == b.m_transform;
}

void ParaEngine::QBrush::init(const Color &color, BrushStyle style)
{
	switch (style) {
	case NoBrush:
		setColor(color);
		return;
	case TexturePattern:
		break;
	default:
		break;
	}
	m_style = style;
	m_color = color;
}

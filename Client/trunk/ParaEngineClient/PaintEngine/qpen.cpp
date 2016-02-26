//----------------------------------------------------------------------
// Class:	
// Authors:	LiXizhi
// company: ParaEngine
// Date:	2015.3.4
// Revised: 
//-----------------------------------------------------------------------
#include "ParaEngine.h"
#include "qpen.h"

using namespace ParaEngine;


ParaEngine::QPen::QPen()
	:QPen(SolidLine)
{
}

ParaEngine::QPen::QPen(PenStyle style)
	:m_brush(), m_width(1.f), m_style(style), m_capStyle(SquareCap), m_joinStyle(BevelJoin), m_dashOffset(0.f), m_miterLimit(2.f)
{
	setStyle(style);
}

ParaEngine::QPen::QPen(const Color &color)
	: m_brush(color), m_width(1.f), m_style(SolidLine), m_capStyle(SquareCap), m_joinStyle(BevelJoin), m_dashOffset(0.f), m_miterLimit(2.f)
{
	setColor(color);
}

ParaEngine::QPen::QPen(const QBrush &brush, float width, PenStyle s /*= SolidLine*/, PenCapStyle c /*= SquareCap*/, PenJoinStyle j /*= BevelJoin*/)
	:m_brush(brush), m_width(1.f), m_style(s), m_capStyle(c), m_joinStyle(j), m_dashOffset(0.f), m_miterLimit(2.f)
{
}

ParaEngine::QPen::~QPen()
{

}

ParaEngine::PenStyle ParaEngine::QPen::style() const
{
	return m_style;
}

void ParaEngine::QPen::setStyle(PenStyle s)
{
	m_style = s;
}

std::vector<float> ParaEngine::QPen::dashPattern() const
{
	return m_dashPattern;
}

void ParaEngine::QPen::setDashPattern(const std::vector<float> &pattern)
{
	m_dashPattern = pattern;
}

float ParaEngine::QPen::dashOffset() const
{
	return m_dashOffset;
}

void ParaEngine::QPen::setDashOffset(float doffset)
{
	m_dashOffset = doffset;
}

float ParaEngine::QPen::miterLimit() const
{
	return m_miterLimit;
}

void ParaEngine::QPen::setMiterLimit(float limit)
{
	m_miterLimit = limit;
}

float ParaEngine::QPen::widthF() const
{
	return m_width;
}

void ParaEngine::QPen::setWidthF(float width)
{
	m_width = width;
}

int ParaEngine::QPen::width() const
{
	return (int)m_width;
}

void ParaEngine::QPen::setWidth(int width)
{
	m_width = (float)width;
}

Color ParaEngine::QPen::color() const
{
	return m_brush.color();
}

void ParaEngine::QPen::setColor(const Color &color)
{
	m_brush.setColor(color);
}

void ParaEngine::QPen::setBrush(const QBrush &brush)
{
	m_brush = brush;
}

bool ParaEngine::QPen::isSolid() const
{
	return m_brush.isOpaque();
}

ParaEngine::PenCapStyle ParaEngine::QPen::capStyle() const
{
	return m_capStyle;
}

void ParaEngine::QPen::setCapStyle(PenCapStyle pcs)
{
	m_capStyle = pcs;
}

ParaEngine::PenJoinStyle ParaEngine::QPen::joinStyle() const
{
	return m_joinStyle;
}

void ParaEngine::QPen::setJoinStyle(PenJoinStyle pcs)
{
	m_joinStyle = pcs;
}

bool ParaEngine::QPen::operator==(const QPen &r) const
{
	return m_brush == r.m_brush && m_width == r.m_width && m_style == r.m_style;
}

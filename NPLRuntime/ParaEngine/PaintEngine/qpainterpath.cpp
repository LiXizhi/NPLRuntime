//----------------------------------------------------------------------
// Class:	
// Authors:	LiXizhi
// company: ParaEngine
// Date:	2015.3.4
// Revised: 
//-----------------------------------------------------------------------
#include "ParaEngine.h"
#include "qpolygon.h"
#include "qvectorpath.h"
#include "qpainterpath.h"

using namespace ParaEngine;

void ParaEngine::QPainterPath::addPolygon(const QPolygonF &polygon)
{

}

std::list<QPolygonF> ParaEngine::QPainterPath::toFillPolygons(const QMatrix &matrix /*= QMatrix()*/) const
{
	return std::list<QPolygonF>();
}

ParaEngine::QPolygonF ParaEngine::QPainterPath::toFillPolygon(const QMatrix &matrix /*= QMatrix()*/) const
{
	return QPolygonF();
}

ParaEngine::QPainterPath::QPainterPath()
{

}

ParaEngine::QPainterPath::QPainterPath(const QPointF &startPoint)
{

}

ParaEngine::QPainterPath::QPainterPath(const QPainterPath &other)
{

}

QPainterPath & ParaEngine::QPainterPath::operator=(const QPainterPath &other)
{
	return *this;
}

ParaEngine::QPainterPath::~QPainterPath()
{

}

QPainterPath ParaEngine::QPainterPath::intersected(const QPainterPath &r) const
{
	return QPainterPath(r);
}

QPainterPath ParaEngine::QPainterPath::subtracted(const QPainterPath &r) const
{
	return QPainterPath(r);
}

const QVectorPath & ParaEngine::QPainterPath::vectorPath() const
{
	// TODO:
	static QVectorPath s(0, 0);
	return s;
}

QPainterPath QPainterPath::united(const QPainterPath &r) const
{
	return QPainterPath(r);
}

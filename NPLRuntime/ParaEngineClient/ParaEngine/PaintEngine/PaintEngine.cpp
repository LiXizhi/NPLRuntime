//-----------------------------------------------------------------------------
// Class: PaintEngine
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date: 2015.2.23
// Desc: I have referred to QT framework's qpaintengine.h, but greatly simplified. 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "Painter.h"
#include "PainterState.h"
#include "PaintEngine.h"

using namespace ParaEngine;


ParaEngine::CPaintEngine::CPaintEngine()
	:pdev(0), currentClipDevice(0), state(0), active(false),
	selfDestruct(false), hasSystemTransform(true), hasSystemViewport(true)
{

}

ParaEngine::CPaintEngine::~CPaintEngine()
{

}

void ParaEngine::CPaintEngine::drawTriangles(const Vector3* vertices, int nTriangleCount)
{

}

void ParaEngine::CPaintEngine::drawRects(const QRect *rects, int rectCount)
{

}

void ParaEngine::CPaintEngine::drawRects(const QRectF *rects, int rectCount)
{

}

void ParaEngine::CPaintEngine::drawLines(const QLine *lines, int lineCount)
{

}

void ParaEngine::CPaintEngine::drawLines(const QLineF *lines, int lineCount)
{

}

void ParaEngine::CPaintEngine::drawLines(const Vector3 *vertices, int lineCount)
{

}

void ParaEngine::CPaintEngine::drawEllipse(const QRectF &r)
{

}

void ParaEngine::CPaintEngine::drawEllipse(const QRect &r)
{

}

void ParaEngine::CPaintEngine::drawPath(const CPainterPath &path)
{

}

void ParaEngine::CPaintEngine::drawPoints(const QPointF *points, int pointCount)
{

}

void ParaEngine::CPaintEngine::drawPoints(const QPoint *points, int pointCount)
{

}

void ParaEngine::CPaintEngine::drawPolygon(const QPointF *points, int pointCount, PolygonDrawMode mode)
{

}

void ParaEngine::CPaintEngine::drawPolygon(const QPoint *points, int pointCount, PolygonDrawMode mode)
{

}

void ParaEngine::CPaintEngine::drawPixmap(const QRectF &r, const TextureEntity &pm, const QRectF &sr)
{

}

void ParaEngine::CPaintEngine::drawTextItem(const QPointF &p, const CTextItem &textItem)
{

}

void ParaEngine::CPaintEngine::drawImage(const QRectF &r, const ImageEntity &pm, const QRectF &sr)
{

}

void ParaEngine::CPaintEngine::setPaintDevice(CPaintDevice *device)
{
	pdev = device;
}

CPaintDevice * ParaEngine::CPaintEngine::paintDevice() const
{
	return pdev;
}

void ParaEngine::CPaintEngine::setSystemClip(const QRegion &baseClip)
{

}

QRegion ParaEngine::CPaintEngine::systemClip() const
{
	return QRegion();
}

void ParaEngine::CPaintEngine::setSystemRect(const QRect &rect)
{

}

ParaEngine::QRect ParaEngine::CPaintEngine::systemRect() const
{
	return QRect(0, 0, 0, 0);
}

ParaEngine::QPoint ParaEngine::CPaintEngine::coordinateOffset() const
{
	return QPoint(0, 0);
}

void ParaEngine::CPaintEngine::setState(CPainterState *s)
{
	state = s;
}

CPainter * ParaEngine::CPaintEngine::painter() const
{
	return state ? state->painter() : 0;
}

CPainterState * ParaEngine::CPaintEngine::createState(CPainterState *orig) const
{
	return new CPainterState();
}

void ParaEngine::CPaintEngine::Flush()
{

}

void ParaEngine::CPaintEngine::ApplyStateChanges()
{

}

CPainterState * ParaEngine::CPaintEngine::GetState()
{
	return static_cast<CPainterState *>(CPaintEngine::state);
}

const CPainterState * ParaEngine::CPaintEngine::GetState() const
{
	return static_cast<const CPainterState *>(CPaintEngine::state);
}

void ParaEngine::CPaintEngine::clip(const QRect & rect, ClipOperation op)
{
	
}


CPainter * ParaEngine::CPaintEngineState::painter() const
{
	return static_cast<const CPainterState *>(this)->m_painter;
}

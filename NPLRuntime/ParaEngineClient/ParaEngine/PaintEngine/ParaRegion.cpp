//-----------------------------------------------------------------------------
// Class: PaintDevice
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Date: 2015.2.23
// Desc: I have referred to QT framework's qpaintdevice.h, but greatly simplified. 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ParaRegion.h"

using namespace ParaEngine;


ParaEngine::QRegion::QRegion()
{

}

ParaEngine::QRegion::QRegion(int x, int y, int w, int h, RegionType t /*= Rectangle*/)
	: QRegion(QRect(x,y,w,h), t)
{

}

ParaEngine::QRegion::QRegion(const QRect &r, RegionType t /*= Rectangle*/)
	: numRects(0), innerArea(-1)
{
	if (!r.isEmpty())
	{
		if (t == Rectangle) 
		{
			numRects = 1;
			extents = r;
			innerRect = r;
			innerArea = r.width() * r.height();
		}
		else
		{
			OUTPUT_LOG("unsupported region type\n");
		}
	}
}

ParaEngine::QRegion::QRegion(const QRegion &r)
{
	rects = r.rects;
	numRects = r.numRects;
	extents = r.extents;
	innerRect = r.innerRect;
	innerArea = r.innerArea;
}

ParaEngine::QRegion::~QRegion()
{

}

QRegion & ParaEngine::QRegion::operator=(const QRegion & r)
{
	rects = r.rects;
	numRects = r.numRects;
	extents = r.extents;
	innerRect = r.innerRect;
	innerArea = r.innerArea;
	return *this;
}

bool ParaEngine::QRegion::contains(const QRect &r2) const
{
	const QRect &r1 = innerRect;
	return r2.left() >= r1.left() && r2.right() <= r1.right()
		&& r2.top() >= r1.top() && r2.bottom() <= r1.bottom();
}

ParaEngine::QRect ParaEngine::QRegion::boundingRect() const
{
	if (isEmpty())
		return QRect();
	return extents;
}

bool ParaEngine::QRegion::isEmpty() const
{
	return numRects == 0;
}

bool ParaEngine::QRegion::isNull() const
{
	return numRects == 0;
}

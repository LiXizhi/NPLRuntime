//----------------------------------------------------------------------
// Class:
// Authors:	LiXizhi
// company: ParaEngine
// Date:	2015.3
// Revised: 
//-----------------------------------------------------------------------
#include "ParaEngine.h"
#include "qpolygon.h"
#include "ParaRect.h"
#include "qpainterpath.h"
#include "qmatrix.h"

#include <stdarg.h>

using namespace ParaEngine;

//same as qt_painterpath_isect_line in qpainterpath.cpp
static void qt_polygon_isect_line(const QPointF &p1, const QPointF &p2, const QPointF &pos,
                                  int *winding)
{
    float x1 = p1.x();
    float y1 = p1.y();
    float x2 = p2.x();
    float y2 = p2.y();
    float y = pos.y();

    int dir = 1;

    if (Math::FuzzyCompare(y1, y2)) {
        // ignore horizontal lines according to scan conversion rule
        return;
    } else if (y2 < y1) {
        float x_tmp = x2; x2 = x1; x1 = x_tmp;
        float y_tmp = y2; y2 = y1; y1 = y_tmp;
        dir = -1;
    }

    if (y >= y1 && y < y2) {
        float x = x1 + ((x2 - x1) / (y2 - y1)) * (y - y1);

        // count up the winding number if we're
        if (x<=pos.x()) {
            (*winding) += dir;
        }
    }
}
QPolygon::QPolygon(const QRect &r, bool closed)
{
    reserve(closed ? 5 : 4);
    *this << QPoint(r.x(), r.y())
          << QPoint(r.x() + r.width(), r.y())
          << QPoint(r.x() + r.width(), r.y() + r.height())
          << QPoint(r.x(), r.y() + r.height());
    if (closed)
        *this << QPoint(r.left(), r.top());
}

QPolygon::QPolygon(int nPoints, const int *points)
{
    setPoints(nPoints, points);
}

void QPolygon::translate(int dx, int dy)
{
    if (dx == 0 && dy == 0)
        return;

    QPoint *p = data();
    int i = size();
    QPoint pt(dx, dy);
    while (i--) {
        *p += pt;
        ++p;
    }
}

QPolygon QPolygon::translated(int dx, int dy) const
{
    QPolygon copy(*this);
    copy.translate(dx, dy);
    return copy;
}

void QPolygon::point(int index, int *x, int *y) const
{
    QPoint p = at(index);
    if (x)
        *x = (int)p.x();
    if (y)
        *y = (int)p.y();
}

void QPolygon::setPoints(int nPoints, const int *points)
{
    resize(nPoints);
    int i = 0;
    while (nPoints--) {
        setPoint(i++, *points, *(points+1));
        points += 2;
    }
}

void QPolygon::setPoints(int nPoints, int firstx, int firsty, ...)
{
    va_list ap;
    resize(nPoints);
    setPoint(0, firstx, firsty);
    int i = 0, x, y;
    va_start(ap, firsty);
    while (--nPoints) {
        x = va_arg(ap, int);
        y = va_arg(ap, int);
        setPoint(++i, x, y);
    }
    va_end(ap);
}

void QPolygon::putPoints(int index, int nPoints, const int *points)
{
    if (index + nPoints > (int)size())
        resize(index + nPoints);
    int i = index;
    while (nPoints--) {
        setPoint(i++, *points, *(points+1));
        points += 2;
    }
}
void QPolygon::putPoints(int index, int nPoints, int firstx, int firsty, ...)
{
    va_list ap;
	if (index + nPoints > (int)size())
        resize(index + nPoints);
    if (nPoints <= 0)
        return;
    setPoint(index, firstx, firsty);
    int i = index, x, y;
    va_start(ap, firsty);
    while (--nPoints) {
        x = va_arg(ap, int);
        y = va_arg(ap, int);
        setPoint(++i, x, y);
    }
    va_end(ap);
}

void QPolygon::putPoints(int index, int nPoints, const QPolygon & from, int fromIndex)
{
	if (index + nPoints > (int)size())
        resize(index + nPoints);
    if (nPoints <= 0)
        return;
    int n = 0;
    while(n < nPoints) {
        setPoint(index + n, from[fromIndex+n]);
        ++n;
    }
}


QRect QPolygon::boundingRect() const
{
    if (empty())
        return QRect(0, 0, 0, 0);
	const QPoint *pd = constVectorData(*this);
    int minx, maxx, miny, maxy;
    minx = maxx = pd->x();
    miny = maxy = pd->y();
    ++pd;
	for (int i = 1; i < (int)size(); ++i) {
        if (pd->x() < minx)
            minx = pd->x();
        else if (pd->x() > maxx)
            maxx = pd->x();
        if (pd->y() < miny)
            miny = pd->y();
        else if (pd->y() > maxy)
            maxy = pd->y();
        ++pd;
    }
    return QRect(QPoint(minx,miny), QPoint(maxx,maxy));
}
QPolygonF::QPolygonF(const QRectF &r)
{
    reserve(5);
    push_back(QPointF(r.x(), r.y()));
    push_back(QPointF(r.x() + r.width(), r.y()));
    push_back(QPointF(r.x() + r.width(), r.y() + r.height()));
    push_back(QPointF(r.x(), r.y() + r.height()));
    push_back(QPointF(r.x(), r.y()));
}


QPolygonF::QPolygonF(const QPolygon &a)
{
    reserve(a.size());
	for (int i = 0; i<(int)a.size(); ++i)
        push_back(a.at(i));
}

void QPolygonF::translate(const QPointF &offset)
{
    if (offset.isNull())
        return;

    QPointF *p = data();
    int i = size();
    while (i--) {
        *p += offset;
        ++p;
    }
}

QPolygonF QPolygonF::translated(const QPointF &offset) const
{
    QPolygonF copy(*this);
    copy.translate(offset);
    return copy;
}

QRectF QPolygonF::boundingRect() const
{
    if (empty())
        return QRectF(0, 0, 0, 0);
    const QPointF *pd = constVectorData(*this);
    float minx, maxx, miny, maxy;
    minx = maxx = pd->x();
    miny = maxy = pd->y();
    ++pd;
	for (int i = 1; i < (int)size(); ++i) {
        if (pd->x() < minx)
            minx = pd->x();
        else if (pd->x() > maxx)
            maxx = pd->x();
        if (pd->y() < miny)
            miny = pd->y();
        else if (pd->y() > maxy)
            maxy = pd->y();
        ++pd;
    }
    return QRectF(minx,miny, maxx - minx, maxy - miny);
}

QPolygon QPolygonF::toPolygon() const
{
    QPolygon a;
    a.reserve(size());
	for (int i = 0; i<(int)size(); ++i)
        a.push_back(at(i).toPoint());
    return a;
}

bool QPolygonF::containsPoint(const QPointF &pt, FillRule fillRule) const
{
    if (empty())
        return false;

    int winding_number = 0;

    QPointF last_pt = at(0);
    QPointF last_start = at(0);
	for (int i = 1; i < (int)size(); ++i) {
        const QPointF &e = at(i);
        qt_polygon_isect_line(last_pt, e, pt, &winding_number);
        last_pt = e;
    }

    // implicitly close last sub path
    if (last_pt != last_start)
        qt_polygon_isect_line(last_pt, last_start, pt, &winding_number);

    return (fillRule == WindingFill
            ? (winding_number != 0)
            : ((winding_number % 2) != 0));
}


bool QPolygon::containsPoint(const QPoint &pt, FillRule fillRule) const
{
    if (empty())
        return false;

    int winding_number = 0;

    QPoint last_pt = at(0);
    QPoint last_start = at(0);
	for (int i = 1; i < (int)size(); ++i) {
        const QPoint &e = at(i);
        qt_polygon_isect_line(last_pt, e, pt, &winding_number);
        last_pt = e;
    }

    // implicitly close last subpath
    if (last_pt != last_start)
        qt_polygon_isect_line(last_pt, last_start, pt, &winding_number);

    return (fillRule == WindingFill
            ? (winding_number != 0)
            : ((winding_number % 2) != 0));
}


QPolygon QPolygon::united(const QPolygon &r) const
{
    QPainterPath subject; subject.addPolygon(*this);
    QPainterPath clip; clip.addPolygon(r);

    return subject.united(clip).toFillPolygon().toPolygon();
}

QPolygon QPolygon::intersected(const QPolygon &r) const
{
    QPainterPath subject; subject.addPolygon(*this);
    QPainterPath clip; clip.addPolygon(r);

    return subject.intersected(clip).toFillPolygon().toPolygon();
}

QPolygon QPolygon::subtracted(const QPolygon &r) const
{
    QPainterPath subject; subject.addPolygon(*this);
    QPainterPath clip; clip.addPolygon(r);

    return subject.subtracted(clip).toFillPolygon().toPolygon();
}

const ParaEngine::QPoint* ParaEngine::QPolygon::constData() const
{
	return constVectorData(*this);
}


QPolygonF QPolygonF::united(const QPolygonF &r) const
{
    QPainterPath subject; subject.addPolygon(*this);
    QPainterPath clip; clip.addPolygon(r);

    return subject.united(clip).toFillPolygon();
}

QPolygonF QPolygonF::intersected(const QPolygonF &r) const
{
    QPainterPath subject; subject.addPolygon(*this);
    QPainterPath clip; clip.addPolygon(r);

    return subject.intersected(clip).toFillPolygon();
}

QPolygonF QPolygonF::subtracted(const QPolygonF &r) const
{
    QPainterPath subject; subject.addPolygon(*this);
    QPainterPath clip; clip.addPolygon(r);
    return subject.subtracted(clip).toFillPolygon();
}

const ParaEngine::QPointF* ParaEngine::QPolygonF::constData() const
{
	return constVectorData(*this);
}


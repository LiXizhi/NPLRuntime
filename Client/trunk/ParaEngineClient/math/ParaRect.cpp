//-----------------------------------------------------------------------------
// Class:	
// Authors:	Li, Xizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2015.2.13
// Desc: based on QT framework. 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ParaRect.h"

using namespace ParaEngine;

QRect QRect::normalized() const
{
	QRect r;
	if (x2 < x1 - 1) {                                // swap bad x values
		r.x1 = x2;
		r.x2 = x1;
	}
	else {
		r.x1 = x1;
		r.x2 = x2;
	}
	if (y2 < y1 - 1) {                                // swap bad y values
		r.y1 = y2;
		r.y2 = y1;
	}
	else {
		r.y1 = y1;
		r.y2 = y2;
	}
	return r;
}


bool QRect::contains(const QPoint &p, bool proper) const
{
	int32 l, r;
	if (x2 < x1 - 1) {
		l = x2;
		r = x1;
	}
	else {
		l = x1;
		r = x2;
	}
	if (proper) {
		if (p.x() <= l || p.x() >= r)
			return false;
	}
	else {
		if (p.x() < l || p.x() > r)
			return false;
	}
	int32 t, b;
	if (y2 < y1 - 1) {
		t = y2;
		b = y1;
	}
	else {
		t = y1;
		b = y2;
	}
	if (proper) {
		if (p.y() <= t || p.y() >= b)
			return false;
	}
	else {
		if (p.y() < t || p.y() > b)
			return false;
	}
	return true;
}

bool QRect::contains(const QRect &r, bool proper) const
{
	if (isNull() || r.isNull())
		return false;

	int32 l1 = x1;
	int32 r1 = x1;
	if (x2 - x1 + 1 < 0)
		l1 = x2;
	else
		r1 = x2;

	int32 l2 = r.x1;
	int32 r2 = r.x1;
	if (r.x2 - r.x1 + 1 < 0)
		l2 = r.x2;
	else
		r2 = r.x2;

	if (proper) {
		if (l2 <= l1 || r2 >= r1)
			return false;
	}
	else {
		if (l2 < l1 || r2 > r1)
			return false;
	}

	int32 t1 = y1;
	int32 b1 = y1;
	if (y2 - y1 + 1 < 0)
		t1 = y2;
	else
		b1 = y2;

	int32 t2 = r.y1;
	int32 b2 = r.y1;
	if (r.y2 - r.y1 + 1 < 0)
		t2 = r.y2;
	else
		b2 = r.y2;

	if (proper) {
		if (t2 <= t1 || b2 >= b1)
			return false;
	}
	else {
		if (t2 < t1 || b2 > b1)
			return false;
	}

	return true;
}


/*!
\fn QRect QRect::operator|(const QRect &rectangle) const

Returns the bounding rectangle of this rectangle and the given \a
rectangle.

\sa operator|=(), united()
*/
QRect QRect::operator|(const QRect &r) const
{
	if (isNull())
		return r;
	if (r.isNull())
		return *this;

	int32 l1 = x1;
	int32 r1 = x1;
	if (x2 - x1 + 1 < 0)
		l1 = x2;
	else
		r1 = x2;

	int32 l2 = r.x1;
	int32 r2 = r.x1;
	if (r.x2 - r.x1 + 1 < 0)
		l2 = r.x2;
	else
		r2 = r.x2;

	int32 t1 = y1;
	int32 b1 = y1;
	if (y2 - y1 + 1 < 0)
		t1 = y2;
	else
		b1 = y2;

	int32 t2 = r.y1;
	int32 b2 = r.y1;
	if (r.y2 - r.y1 + 1 < 0)
		t2 = r.y2;
	else
		b2 = r.y2;

	QRect tmp;
	tmp.x1 = Math::Min(l1, l2);
	tmp.x2 = Math::Max(r1, r2);
	tmp.y1 = Math::Min(t1, t2);
	tmp.y2 = Math::Max(b1, b2);
	return tmp;
}


/*!
\fn QRect QRect::operator&(const QRect &rectangle) const

Returns the intersection of this rectangle and the given \a
rectangle. Returns an empty rectangle if there is no intersection.

\sa operator&=(), intersected()
*/

QRect QRect::operator&(const QRect &r) const
{
	if (isNull() || r.isNull())
		return QRect();

	int32 l1 = x1;
	int32 r1 = x1;
	if (x2 - x1 + 1 < 0)
		l1 = x2;
	else
		r1 = x2;

	int32 l2 = r.x1;
	int32 r2 = r.x1;
	if (r.x2 - r.x1 + 1 < 0)
		l2 = r.x2;
	else
		r2 = r.x2;

	if (l1 > r2 || l2 > r1)
		return QRect();

	int32 t1 = y1;
	int32 b1 = y1;
	if (y2 - y1 + 1 < 0)
		t1 = y2;
	else
		b1 = y2;

	int32 t2 = r.y1;
	int32 b2 = r.y1;
	if (r.y2 - r.y1 + 1 < 0)
		t2 = r.y2;
	else
		b2 = r.y2;

	if (t1 > b2 || t2 > b1)
		return QRect();

	QRect tmp;
	tmp.x1 = Math::Max(l1, l2);
	tmp.x2 = Math::Min(r1, r2);
	tmp.y1 = Math::Max(t1, t2);
	tmp.y2 = Math::Min(b1, b2);
	return tmp;
}


bool QRect::intersects(const QRect &r) const
{
	if (isNull() || r.isNull())
		return false;

	int32 l1 = x1;
	int32 r1 = x1;
	if (x2 - x1 + 1 < 0)
		l1 = x2;
	else
		r1 = x2;

	int32 l2 = r.x1;
	int32 r2 = r.x1;
	if (r.x2 - r.x1 + 1 < 0)
		l2 = r.x2;
	else
		r2 = r.x2;

	if (l1 > r2 || l2 > r1)
		return false;

	int32 t1 = y1;
	int32 b1 = y1;
	if (y2 - y1 + 1 < 0)
		t1 = y2;
	else
		b1 = y2;

	int32 t2 = r.y1;
	int32 b2 = r.y1;
	if (r.y2 - r.y1 + 1 < 0)
		t2 = r.y2;
	else
		b2 = r.y2;

	if (t1 > b2 || t2 > b1)
		return false;

	return true;
}

ParaEngine::QRect::QRect(const QRectF &rect)
{
	x1 = (int)rect.x();
	y1 = (int)rect.y();
	setWidth((int)rect.width());
	setHeight((int)rect.height());
}

QRectF QRectF::normalized() const
{
	QRectF r = *this;
	if (r.w < 0) {
		r.xp += r.w;
		r.w = -r.w;
	}
	if (r.h < 0) {
		r.yp += r.h;
		r.h = -r.h;
	}
	return r;
}


/*!
\fn bool QRectF::contains(const QPointF &point) const

Returns \c true if the given \a point is inside or on the edge of the
rectangle; otherwise returns \c false.

\sa intersects()
*/

bool QRectF::contains(const QPointF &p) const
{
	float l = xp;
	float r = xp;
	if (w < 0)
		l += w;
	else
		r += w;
	if (l == r) // null rect
		return false;

	if (p.x() < l || p.x() > r)
		return false;

	float t = yp;
	float b = yp;
	if (h < 0)
		t += h;
	else
		b += h;
	if (t == b) // null rect
		return false;

	if (p.y() < t || p.y() > b)
		return false;

	return true;
}


/*!
\fn bool QRectF::contains(float x, float y) const
\overload

Returns \c true if the point (\a x, \a y) is inside or on the edge of
the rectangle; otherwise returns \c false.
*/

/*!
\fn bool QRectF::contains(const QRectF &rectangle) const
\overload

Returns \c true if the given \a rectangle is inside this rectangle;
otherwise returns \c false.
*/

bool QRectF::contains(const QRectF &r) const
{
	float l1 = xp;
	float r1 = xp;
	if (w < 0)
		l1 += w;
	else
		r1 += w;
	if (l1 == r1) // null rect
		return false;

	float l2 = r.xp;
	float r2 = r.xp;
	if (r.w < 0)
		l2 += r.w;
	else
		r2 += r.w;
	if (l2 == r2) // null rect
		return false;

	if (l2 < l1 || r2 > r1)
		return false;

	float t1 = yp;
	float b1 = yp;
	if (h < 0)
		t1 += h;
	else
		b1 += h;
	if (t1 == b1) // null rect
		return false;

	float t2 = r.yp;
	float b2 = r.yp;
	if (r.h < 0)
		t2 += r.h;
	else
		b2 += r.h;
	if (t2 == b2) // null rect
		return false;

	if (t2 < t1 || b2 > b1)
		return false;

	return true;
}


/*!
\fn QRectF QRectF::operator|(const QRectF &rectangle) const

Returns the bounding rectangle of this rectangle and the given \a rectangle.

\sa united(), operator|=()
*/

QRectF QRectF::operator|(const QRectF &r) const
{
	if (isNull())
		return r;
	if (r.isNull())
		return *this;

	float left = xp;
	float right = xp;
	if (w < 0)
		left += w;
	else
		right += w;

	if (r.w < 0) {
		left = Math::Min(left, r.xp + r.w);
		right = Math::Max(right, r.xp);
	}
	else {
		left = Math::Min(left, r.xp);
		right = Math::Max(right, r.xp + r.w);
	}

	float top = yp;
	float bottom = yp;
	if (h < 0)
		top += h;
	else
		bottom += h;

	if (r.h < 0) {
		top = Math::Min(top, r.yp + r.h);
		bottom = Math::Max(bottom, r.yp);
	}
	else {
		top = Math::Min(top, r.yp);
		bottom = Math::Max(bottom, r.yp + r.h);
	}

	return QRectF(left, top, right - left, bottom - top);
}


/*!
\fn QRectF QRectF::operator &(const QRectF &rectangle) const

Returns the intersection of this rectangle and the given \a
rectangle. Returns an empty rectangle if there is no intersection.

\sa operator&=(), intersected()
*/

QRectF QRectF::operator&(const QRectF &r) const
{
	float l1 = xp;
	float r1 = xp;
	if (w < 0)
		l1 += w;
	else
		r1 += w;
	if (l1 == r1) // null rect
		return QRectF();

	float l2 = r.xp;
	float r2 = r.xp;
	if (r.w < 0)
		l2 += r.w;
	else
		r2 += r.w;
	if (l2 == r2) // null rect
		return QRectF();

	if (l1 >= r2 || l2 >= r1)
		return QRectF();

	float t1 = yp;
	float b1 = yp;
	if (h < 0)
		t1 += h;
	else
		b1 += h;
	if (t1 == b1) // null rect
		return QRectF();

	float t2 = r.yp;
	float b2 = r.yp;
	if (r.h < 0)
		t2 += r.h;
	else
		b2 += r.h;
	if (t2 == b2) // null rect
		return QRectF();

	if (t1 >= b2 || t2 >= b1)
		return QRectF();

	QRectF tmp;
	tmp.xp = Math::Max(l1, l2);
	tmp.yp = Math::Max(t1, t2);
	tmp.w = Math::Min(r1, r2) - tmp.xp;
	tmp.h = Math::Min(b1, b2) - tmp.yp;
	return tmp;
}

/*!
\fn bool QRectF::intersects(const QRectF &rectangle) const

Returns \c true if this rectangle intersects with the given \a
rectangle (i.e. there is a non-empty area of overlap between
them), otherwise returns \c false.

The intersection rectangle can be retrieved using the intersected()
function.

\sa contains()
*/

bool QRectF::intersects(const QRectF &r) const
{
	float l1 = xp;
	float r1 = xp;
	if (w < 0)
		l1 += w;
	else
		r1 += w;
	if (l1 == r1) // null rect
		return false;

	float l2 = r.xp;
	float r2 = r.xp;
	if (r.w < 0)
		l2 += r.w;
	else
		r2 += r.w;
	if (l2 == r2) // null rect
		return false;

	if (l1 >= r2 || l2 >= r1)
		return false;

	float t1 = yp;
	float b1 = yp;
	if (h < 0)
		t1 += h;
	else
		b1 += h;
	if (t1 == b1) // null rect
		return false;

	float t2 = r.yp;
	float b2 = r.yp;
	if (r.h < 0)
		t2 += r.h;
	else
		b2 += r.h;
	if (t2 == b2) // null rect
		return false;

	if (t1 >= b2 || t2 >= b1)
		return false;

	return true;
}

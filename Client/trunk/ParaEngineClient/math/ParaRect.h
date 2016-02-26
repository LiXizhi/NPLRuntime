#pragma once

#include "ParaMargins.h"
#include "ParaSize.h"
#include "ParaPoint.h"

namespace ParaEngine
{
	class QRectF;

	/** when concerting QRect with QRectF and RECT, we ensure that their left, top and width, height are the same. 
	however, QRect's internal bottom right may be 1 pixel smaller than QRectF and RECT. 
	width = right - left + 1
	height = bottom - top + 1
	*/
	class QRect
	{
	public:
		QRect() : x1(0), y1(0), x2(-1), y2(-1) {}
		QRect(const QPoint &topleft, const QPoint &bottomright);
		QRect(const QPoint &topleft, const QSize &size);
		QRect(int32 left, int32 top, int32 width, int32 height);
		QRect(const RECT& r) :x1(r.left), y1(r.top), x2(r.right-1), y2(r.bottom-1) {}
		explicit QRect(const QRectF &rect);

		inline operator RECT() const{
			RECT r = { x1, y1, x1 + width(), y1 + height() };
			return r;
		}

		inline bool isNull() const;
		inline bool isEmpty() const;
		inline bool isValid() const;

		inline int32 left() const;
		inline int32 top() const;
		inline int32 right() const;
		inline int32 bottom() const;
		QRect normalized() const;

		inline int32 x() const;
		inline int32 y() const;
		inline void setLeft(int32 pos);
		inline void setTop(int32 pos);
		inline void setRight(int32 pos);
		inline void setBottom(int32 pos);
		inline void setX(int32 x);
		inline void setY(int32 y);

		inline void setTopLeft(const QPoint &p);
		inline void setBottomRight(const QPoint &p);
		inline void setTopRight(const QPoint &p);
		inline void setBottomLeft(const QPoint &p);

		inline QPoint topLeft() const;
		inline QPoint bottomRight() const;
		inline QPoint topRight() const;
		inline QPoint bottomLeft() const;
		inline QPoint center() const;

		inline void moveLeft(int32 pos);
		inline void moveTop(int32 pos);
		inline void moveRight(int32 pos);
		inline void moveBottom(int32 pos);
		inline void moveTopLeft(const QPoint &p);
		inline void moveBottomRight(const QPoint &p);
		inline void moveTopRight(const QPoint &p);
		inline void moveBottomLeft(const QPoint &p);
		inline void moveCenter(const QPoint &p);

		inline void translate(int32 dx, int32 dy);
		inline void translate(const QPoint &p);
		inline QRect translated(int32 dx, int32 dy) const;
		inline QRect translated(const QPoint &p) const;

		inline void moveTo(int32 x, int32 t);
		inline void moveTo(const QPoint &p);

		inline void setRect(int32 x, int32 y, int32 w, int32 h);
		inline void getRect(int32 *x, int32 *y, int32 *w, int32 *h) const;

		inline void setCoords(int32 x1, int32 y1, int32 x2, int32 y2);
		inline void getCoords(int32 *x1, int32 *y1, int32 *x2, int32 *y2) const;

		inline void adjust(int32 x1, int32 y1, int32 x2, int32 y2);
		inline QRect adjusted(int32 x1, int32 y1, int32 x2, int32 y2) const;

		inline QSize size() const;
		inline int32 width() const;
		inline int32 height() const;
		inline void setWidth(int32 w);
		inline void setHeight(int32 h);
		inline void setSize(const QSize &s);

		QRect operator|(const QRect &r) const;
		QRect operator&(const QRect &r) const;
		inline QRect& operator|=(const QRect &r);
		inline QRect& operator&=(const QRect &r);

		bool contains(const QRect &r, bool proper = false) const;
		bool contains(const QPoint &p, bool proper = false) const;
		inline bool contains(int32 x, int32 y) const;
		inline bool contains(int32 x, int32 y, bool proper) const;
		inline QRect united(const QRect &other) const;
		inline QRect intersected(const QRect &other) const;
		bool intersects(const QRect &r) const;

		inline QRect marginsAdded(const QMargins &margins) const;
		inline QRect marginsRemoved(const QMargins &margins) const;
		inline QRect &operator+=(const QMargins &margins);
		inline QRect &operator-=(const QMargins &margins);


		friend inline bool operator==(const QRect &, const QRect &);
		friend inline bool operator!=(const QRect &, const QRect &);

	private:
		int32 x1;
		int32 y1;
		int32 x2;
		int32 y2;
	};
	inline bool operator==(const QRect &, const QRect &);
	inline bool operator!=(const QRect &, const QRect &);

	/*****************************************************************************
	QRect inline member functions
	*****************************************************************************/

	inline QRect::QRect(int32 aleft, int32 atop, int32 awidth, int32 aheight)
		: x1(aleft), y1(atop), x2(aleft + awidth - 1), y2(atop + aheight - 1) {}

	inline QRect::QRect(const QPoint &atopLeft, const QPoint &abottomRight)
		: x1(atopLeft.x()), y1(atopLeft.y()), x2(abottomRight.x()), y2(abottomRight.y()) {}

	inline QRect::QRect(const QPoint &atopLeft, const QSize &asize)
		: x1(atopLeft.x()), y1(atopLeft.y()), x2(atopLeft.x() + asize.width() - 1), y2(atopLeft.y() + asize.height() - 1) {}

	inline bool QRect::isNull() const
	{
		return x2 == x1 - 1 && y2 == y1 - 1;
	}

	inline bool QRect::isEmpty() const
	{
		return x1 > x2 || y1 > y2;
	}

	inline bool QRect::isValid() const
	{
		return x1 <= x2 && y1 <= y2;
	}

	inline int32 QRect::left() const
	{
		return x1;
	}

	inline int32 QRect::top() const
	{
		return y1;
	}

	inline int32 QRect::right() const
	{
		return x2;
	}

	inline int32 QRect::bottom() const
	{
		return y2;
	}

	inline int32 QRect::x() const
	{
		return x1;
	}

	inline int32 QRect::y() const
	{
		return y1;
	}

	inline void QRect::setLeft(int32 pos)
	{
		x1 = pos;
	}

	inline void QRect::setTop(int32 pos)
	{
		y1 = pos;
	}

	inline void QRect::setRight(int32 pos)
	{
		x2 = pos;
	}

	inline void QRect::setBottom(int32 pos)
	{
		y2 = pos;
	}

	inline void QRect::setTopLeft(const QPoint &p)
	{
		x1 = p.x(); y1 = p.y();
	}

	inline void QRect::setBottomRight(const QPoint &p)
	{
		x2 = p.x(); y2 = p.y();
	}

	inline void QRect::setTopRight(const QPoint &p)
	{
		x2 = p.x(); y1 = p.y();
	}

	inline void QRect::setBottomLeft(const QPoint &p)
	{
		x1 = p.x(); y2 = p.y();
	}

	inline void QRect::setX(int32 ax)
	{
		x1 = ax;
	}

	inline void QRect::setY(int32 ay)
	{
		y1 = ay;
	}

	inline QPoint QRect::topLeft() const
	{
		return QPoint(x1, y1);
	}

	inline QPoint QRect::bottomRight() const
	{
		return QPoint(x2, y2);
	}

	inline QPoint QRect::topRight() const
	{
		return QPoint(x2, y1);
	}

	inline QPoint QRect::bottomLeft() const
	{
		return QPoint(x1, y2);
	}

	inline QPoint QRect::center() const
	{
		return QPoint((x1 + x2) / 2, (y1 + y2) / 2);
	}

	inline int32 QRect::width() const
	{
		return  x2 - x1 + 1;
	}

	inline int32 QRect::height() const
	{
		return  y2 - y1 + 1;
	}

	inline QSize QRect::size() const
	{
		return QSize(width(), height());
	}

	inline void QRect::translate(int32 dx, int32 dy)
	{
		x1 += dx;
		y1 += dy;
		x2 += dx;
		y2 += dy;
	}

	inline void QRect::translate(const QPoint &p)
	{
		x1 += p.x();
		y1 += p.y();
		x2 += p.x();
		y2 += p.y();
	}

	inline QRect QRect::translated(int32 dx, int32 dy) const
	{
		return QRect(QPoint(x1 + dx, y1 + dy), QPoint(x2 + dx, y2 + dy));
	}

	inline QRect QRect::translated(const QPoint &p) const
	{
		return QRect(QPoint(x1 + p.x(), y1 + p.y()), QPoint(x2 + p.x(), y2 + p.y()));
	}

	inline void QRect::moveTo(int32 ax, int32 ay)
	{
		x2 += ax - x1;
		y2 += ay - y1;
		x1 = ax;
		y1 = ay;
	}

	inline void QRect::moveTo(const QPoint &p)
	{
		x2 += p.x() - x1;
		y2 += p.y() - y1;
		x1 = p.x();
		y1 = p.y();
	}

	inline void QRect::moveLeft(int32 pos)
	{
		x2 += (pos - x1); x1 = pos;
	}

	inline void QRect::moveTop(int32 pos)
	{
		y2 += (pos - y1); y1 = pos;
	}

	inline void QRect::moveRight(int32 pos)
	{
		x1 += (pos - x2);
		x2 = pos;
	}

	inline void QRect::moveBottom(int32 pos)
	{
		y1 += (pos - y2);
		y2 = pos;
	}

	inline void QRect::moveTopLeft(const QPoint &p)
	{
		moveLeft(p.x());
		moveTop(p.y());
	}

	inline void QRect::moveBottomRight(const QPoint &p)
	{
		moveRight(p.x());
		moveBottom(p.y());
	}

	inline void QRect::moveTopRight(const QPoint &p)
	{
		moveRight(p.x());
		moveTop(p.y());
	}

	inline void QRect::moveBottomLeft(const QPoint &p)
	{
		moveLeft(p.x());
		moveBottom(p.y());
	}

	inline void QRect::moveCenter(const QPoint &p)
	{
		int32 w = x2 - x1;
		int32 h = y2 - y1;
		x1 = p.x() - w / 2;
		y1 = p.y() - h / 2;
		x2 = x1 + w;
		y2 = y1 + h;
	}

	inline void QRect::getRect(int32 *ax, int32 *ay, int32 *aw, int32 *ah) const
	{
		*ax = x1;
		*ay = y1;
		*aw = x2 - x1 + 1;
		*ah = y2 - y1 + 1;
	}

	inline void QRect::setRect(int32 ax, int32 ay, int32 aw, int32 ah)
	{
		x1 = ax;
		y1 = ay;
		x2 = (ax + aw - 1);
		y2 = (ay + ah - 1);
	}

	inline void QRect::getCoords(int32 *xp1, int32 *yp1, int32 *xp2, int32 *yp2) const
	{
		*xp1 = x1;
		*yp1 = y1;
		*xp2 = x2;
		*yp2 = y2;
	}

	inline void QRect::setCoords(int32 xp1, int32 yp1, int32 xp2, int32 yp2)
	{
		x1 = xp1;
		y1 = yp1;
		x2 = xp2;
		y2 = yp2;
	}

	inline QRect QRect::adjusted(int32 xp1, int32 yp1, int32 xp2, int32 yp2) const
	{
		return QRect(QPoint(x1 + xp1, y1 + yp1), QPoint(x2 + xp2, y2 + yp2));
	}

	inline void QRect::adjust(int32 dx1, int32 dy1, int32 dx2, int32 dy2)
	{
		x1 += dx1;
		y1 += dy1;
		x2 += dx2;
		y2 += dy2;
	}

	inline void QRect::setWidth(int32 w)
	{
		x2 = (x1 + w - 1);
	}

	inline void QRect::setHeight(int32 h)
	{
		y2 = (y1 + h - 1);
	}

	inline void QRect::setSize(const QSize &s)
	{
		x2 = (s.width() + x1 - 1);
		y2 = (s.height() + y1 - 1);
	}

	inline bool QRect::contains(int32 ax, int32 ay, bool aproper) const
	{
		return contains(QPoint(ax, ay), aproper);
	}

	inline bool QRect::contains(int32 ax, int32 ay) const
	{
		return contains(QPoint(ax, ay), false);
	}

	inline QRect& QRect::operator|=(const QRect &r)
	{
		*this = *this | r;
		return *this;
	}

	inline QRect& QRect::operator&=(const QRect &r)
	{
		*this = *this & r;
		return *this;
	}

	inline QRect QRect::intersected(const QRect &other) const
	{
		return *this & other;
	}

	inline QRect QRect::united(const QRect &r) const
	{
		return *this | r;
	}

	inline bool operator==(const QRect &r1, const QRect &r2)
	{
		return r1.x1 == r2.x1 && r1.x2 == r2.x2 && r1.y1 == r2.y1 && r1.y2 == r2.y2;
	}

	inline bool operator!=(const QRect &r1, const QRect &r2)
	{
		return r1.x1 != r2.x1 || r1.x2 != r2.x2 || r1.y1 != r2.y1 || r1.y2 != r2.y2;
	}

	inline QRect operator+(const QRect &rectangle, const QMargins &margins)
	{
		return QRect(QPoint(rectangle.left() - margins.left(), rectangle.top() - margins.top()),
			QPoint(rectangle.right() + margins.right(), rectangle.bottom() + margins.bottom()));
	}

	inline QRect operator+(const QMargins &margins, const QRect &rectangle)
	{
		return QRect(QPoint(rectangle.left() - margins.left(), rectangle.top() - margins.top()),
			QPoint(rectangle.right() + margins.right(), rectangle.bottom() + margins.bottom()));
	}

	inline QRect operator-(const QRect &lhs, const QMargins &rhs)
	{
		return QRect(QPoint(lhs.left() + rhs.left(), lhs.top() + rhs.top()),
			QPoint(lhs.right() - rhs.right(), lhs.bottom() - rhs.bottom()));
	}

	inline QRect QRect::marginsAdded(const QMargins &margins) const
	{
		return QRect(QPoint(x1 - margins.left(), y1 - margins.top()),
			QPoint(x2 + margins.right(), y2 + margins.bottom()));
	}

	inline QRect QRect::marginsRemoved(const QMargins &margins) const
	{
		return QRect(QPoint(x1 + margins.left(), y1 + margins.top()),
			QPoint(x2 - margins.right(), y2 - margins.bottom()));
	}

	inline QRect &QRect::operator+=(const QMargins &margins)
	{
		*this = marginsAdded(margins);
		return *this;
	}

	inline QRect &QRect::operator-=(const QMargins &margins)
	{
		*this = marginsRemoved(margins);
		return *this;
	}

	class QRectF
	{
	public:
		QRectF() : xp(0.), yp(0.), w(0.), h(0.) {}
		QRectF(const QPointF &topleft, const QSizeF &size);
		QRectF(const QPointF &topleft, const QPointF &bottomRight);
		QRectF(float left, float top, float width, float height);
		QRectF(const QRect &rect);
		inline explicit QRectF(const RECT& r) :xp((float)r.left), yp((float)r.top), w((float)(r.right - r.left)), h((float)(r.bottom - r.top)) {}

		inline operator RECT() const{
			RECT r = { (int)x(), (int)y(), (int)right(), (int)bottom() };
			return r;
		}

		inline bool isNull() const;
		inline bool isEmpty() const;
		inline bool isValid() const;
		QRectF normalized() const;

		inline float left() const { return xp; }
		inline float top() const { return yp; }
		inline float right() const { return xp + w; }
		inline float bottom() const { return yp + h; }

		inline float x() const;
		inline float y() const;
		inline void setLeft(float pos);
		inline void setTop(float pos);
		inline void setRight(float pos);
		inline void setBottom(float pos);
		inline void setX(float pos) { setLeft(pos); }
		inline void setY(float pos) { setTop(pos); }

		inline QPointF topLeft() const { return QPointF(xp, yp); }
		inline QPointF bottomRight() const { return QPointF(xp + w, yp + h); }
		inline QPointF topRight() const { return QPointF(xp + w, yp); }
		inline QPointF bottomLeft() const { return QPointF(xp, yp + h); }
		inline QPointF center() const;

		inline void setTopLeft(const QPointF &p);
		inline void setBottomRight(const QPointF &p);
		inline void setTopRight(const QPointF &p);
		inline void setBottomLeft(const QPointF &p);

		inline void moveLeft(float pos);
		inline void moveTop(float pos);
		inline void moveRight(float pos);
		inline void moveBottom(float pos);
		inline void moveTopLeft(const QPointF &p);
		inline void moveBottomRight(const QPointF &p);
		inline void moveTopRight(const QPointF &p);
		inline void moveBottomLeft(const QPointF &p);
		inline void moveCenter(const QPointF &p);

		inline void translate(float dx, float dy);
		inline void translate(const QPointF &p);

		inline QRectF translated(float dx, float dy) const;
		inline QRectF translated(const QPointF &p) const;

		inline void moveTo(float x, float y);
		inline void moveTo(const QPointF &p);

		inline void setRect(float x, float y, float w, float h);
		inline void getRect(float *x, float *y, float *w, float *h) const;

		inline void setCoords(float x1, float y1, float x2, float y2);
		inline void getCoords(float *x1, float *y1, float *x2, float *y2) const;

		inline void adjust(float x1, float y1, float x2, float y2);
		inline QRectF adjusted(float x1, float y1, float x2, float y2) const;

		inline QSizeF size() const;
		inline float width() const;
		inline float height() const;
		inline void setWidth(float w);
		inline void setHeight(float h);
		inline void setSize(const QSizeF &s);

		QRectF operator|(const QRectF &r) const;
		QRectF operator&(const QRectF &r) const;
		inline QRectF& operator|=(const QRectF &r);
		inline QRectF& operator&=(const QRectF &r);

		bool contains(const QRectF &r) const;
		bool contains(const QPointF &p) const;
		inline bool contains(float x, float y) const;
		inline QRectF united(const QRectF &other) const;
		inline QRectF intersected(const QRectF &other) const;
		bool intersects(const QRectF &r) const;

		inline QRectF marginsAdded(const QMarginsF &margins) const;
		inline QRectF marginsRemoved(const QMarginsF &margins) const;
		inline QRectF &operator+=(const QMarginsF &margins);
		inline QRectF &operator-=(const QMarginsF &margins);

		friend inline bool operator==(const QRectF &, const QRectF &);
		friend inline bool operator!=(const QRectF &, const QRectF &);

		inline QRect toRect() const;
		QRect toAlignedRect() const;

	private:
		float xp;
		float yp;
		float w;
		float h;
	};

	inline bool operator==(const QRectF &, const QRectF &);
	inline bool operator!=(const QRectF &, const QRectF &);

	/*****************************************************************************
	QRectF inline member functions
	*****************************************************************************/

	inline QRectF::QRectF(float aleft, float atop, float awidth, float aheight)
		: xp(aleft), yp(atop), w(awidth), h(aheight)
	{
	}

	inline QRectF::QRectF(const QPointF &atopLeft, const QSizeF &asize)
		: xp(atopLeft.x()), yp(atopLeft.y()), w(asize.width()), h(asize.height())
	{
	}


	inline QRectF::QRectF(const QPointF &atopLeft, const QPointF &abottomRight)
		: xp(atopLeft.x()), yp(atopLeft.y()), w(abottomRight.x() - atopLeft.x()), h(abottomRight.y() - atopLeft.y())
	{
	}

	inline QRectF::QRectF(const QRect &r)
		: xp((float)r.x()), yp((float)r.y()), w((float)r.width()), h((float)r.height())
	{
	}

	inline bool QRectF::isNull() const
	{
		return w == 0. && h == 0.;
	}

	inline bool QRectF::isEmpty() const
	{
		return w <= 0. || h <= 0.;
	}

	inline bool QRectF::isValid() const
	{
		return w > 0. && h > 0.;
	}

	inline float QRectF::x() const
	{
		return xp;
	}

	inline float QRectF::y() const
	{
		return yp;
	}

	inline void QRectF::setLeft(float pos) { float diff = pos - xp; xp += diff; w -= diff; }

	inline void QRectF::setRight(float pos) { w = pos - xp; }

	inline void QRectF::setTop(float pos) { float diff = pos - yp; yp += diff; h -= diff; }

	inline void QRectF::setBottom(float pos) { h = pos - yp; }

	inline void QRectF::setTopLeft(const QPointF &p) { setLeft(p.x()); setTop(p.y()); }

	inline void QRectF::setTopRight(const QPointF &p) { setRight(p.x()); setTop(p.y()); }

	inline void QRectF::setBottomLeft(const QPointF &p) { setLeft(p.x()); setBottom(p.y()); }

	inline void QRectF::setBottomRight(const QPointF &p) { setRight(p.x()); setBottom(p.y()); }

	inline QPointF QRectF::center() const
	{
		return QPointF(xp + w / 2, yp + h / 2);
	}

	inline void QRectF::moveLeft(float pos) { xp = pos; }

	inline void QRectF::moveTop(float pos) { yp = pos; }

	inline void QRectF::moveRight(float pos) { xp = pos - w; }

	inline void QRectF::moveBottom(float pos) { yp = pos - h; }

	inline void QRectF::moveTopLeft(const QPointF &p) { moveLeft(p.x()); moveTop(p.y()); }

	inline void QRectF::moveTopRight(const QPointF &p) { moveRight(p.x()); moveTop(p.y()); }

	inline void QRectF::moveBottomLeft(const QPointF &p) { moveLeft(p.x()); moveBottom(p.y()); }

	inline void QRectF::moveBottomRight(const QPointF &p) { moveRight(p.x()); moveBottom(p.y()); }

	inline void QRectF::moveCenter(const QPointF &p) { xp = p.x() - w / 2; yp = p.y() - h / 2; }

	inline float QRectF::width() const
	{
		return w;
	}

	inline float QRectF::height() const
	{
		return h;
	}

	inline QSizeF QRectF::size() const
	{
		return QSizeF(w, h);
	}

	inline void QRectF::translate(float dx, float dy)
	{
		xp += dx;
		yp += dy;
	}

	inline void QRectF::translate(const QPointF &p)
	{
		xp += p.x();
		yp += p.y();
	}

	inline void QRectF::moveTo(float ax, float ay)
	{
		xp = ax;
		yp = ay;
	}

	inline void QRectF::moveTo(const QPointF &p)
	{
		xp = p.x();
		yp = p.y();
	}

	inline QRectF QRectF::translated(float dx, float dy) const
	{
		return QRectF(xp + dx, yp + dy, w, h);
	}

	inline QRectF QRectF::translated(const QPointF &p) const
	{
		return QRectF(xp + p.x(), yp + p.y(), w, h);
	}

	inline void QRectF::getRect(float *ax, float *ay, float *aaw, float *aah) const
	{
		*ax = this->xp;
		*ay = this->yp;
		*aaw = this->w;
		*aah = this->h;
	}

	inline void QRectF::setRect(float ax, float ay, float aaw, float aah)
	{
		this->xp = ax;
		this->yp = ay;
		this->w = aaw;
		this->h = aah;
	}

	inline void QRectF::getCoords(float *xp1, float *yp1, float *xp2, float *yp2) const
	{
		*xp1 = xp;
		*yp1 = yp;
		*xp2 = xp + w;
		*yp2 = yp + h;
	}

	inline void QRectF::setCoords(float xp1, float yp1, float xp2, float yp2)
	{
		xp = xp1;
		yp = yp1;
		w = xp2 - xp1;
		h = yp2 - yp1;
	}

	inline void QRectF::adjust(float xp1, float yp1, float xp2, float yp2)
	{
		xp += xp1; yp += yp1; w += xp2 - xp1; h += yp2 - yp1;
	}

	inline QRectF QRectF::adjusted(float xp1, float yp1, float xp2, float yp2) const
	{
		return QRectF(xp + xp1, yp + yp1, w + xp2 - xp1, h + yp2 - yp1);
	}

	inline void QRectF::setWidth(float aw)
	{
		this->w = aw;
	}

	inline void QRectF::setHeight(float ah)
	{
		this->h = ah;
	}

	inline void QRectF::setSize(const QSizeF &s)
	{
		w = s.width();
		h = s.height();
	}

	inline bool QRectF::contains(float ax, float ay) const
	{
		return contains(QPointF(ax, ay));
	}

	inline QRectF& QRectF::operator|=(const QRectF &r)
	{
		*this = *this | r;
		return *this;
	}

	inline QRectF& QRectF::operator&=(const QRectF &r)
	{
		*this = *this & r;
		return *this;
	}

	inline QRectF QRectF::intersected(const QRectF &r) const
	{
		return *this & r;
	}

	inline QRectF QRectF::united(const QRectF &r) const
	{
		return *this | r;
	}

	inline bool operator==(const QRectF &r1, const QRectF &r2)
	{
		return Math::FuzzyCompare(r1.xp, r2.xp) && Math::FuzzyCompare(r1.yp, r2.yp)
			&& Math::FuzzyCompare(r1.w, r2.w) && Math::FuzzyCompare(r1.h, r2.h);
	}

	inline bool operator!=(const QRectF &r1, const QRectF &r2)
	{
		return !Math::FuzzyCompare(r1.xp, r2.xp) || !Math::FuzzyCompare(r1.yp, r2.yp)
			|| !Math::FuzzyCompare(r1.w, r2.w) || !Math::FuzzyCompare(r1.h, r2.h);
	}

	inline QRect QRectF::toRect() const
	{
		return QRect(Math::Round(xp), Math::Round(yp), Math::Round(w), Math::Round(h));
	}

	inline QRectF operator+(const QRectF &lhs, const QMarginsF &rhs)
	{
		return QRectF(QPointF(lhs.left() - rhs.left(), lhs.top() - rhs.top()),
			QSizeF(lhs.width() + rhs.left() + rhs.right(), lhs.height() + rhs.top() + rhs.bottom()));
	}

	inline QRectF operator+(const QMarginsF &lhs, const QRectF &rhs)
	{
		return QRectF(QPointF(rhs.left() - lhs.left(), rhs.top() - lhs.top()),
			QSizeF(rhs.width() + lhs.left() + lhs.right(), rhs.height() + lhs.top() + lhs.bottom()));
	}

	inline QRectF operator-(const QRectF &lhs, const QMarginsF &rhs)
	{
		return QRectF(QPointF(lhs.left() + rhs.left(), lhs.top() + rhs.top()),
			QSizeF(lhs.width() - rhs.left() - rhs.right(), lhs.height() - rhs.top() - rhs.bottom()));
	}

	inline QRectF QRectF::marginsAdded(const QMarginsF &margins) const
	{
		return QRectF(QPointF(xp - margins.left(), yp - margins.top()),
			QSizeF(w + margins.left() + margins.right(), h + margins.top() + margins.bottom()));
	}

	inline QRectF QRectF::marginsRemoved(const QMarginsF &margins) const
	{
		return QRectF(QPointF(xp + margins.left(), yp + margins.top()),
			QSizeF(w - margins.left() - margins.right(), h - margins.top() - margins.bottom()));
	}

	inline QRectF &QRectF::operator+=(const QMarginsF &margins)
	{
		*this = marginsAdded(margins);
		return *this;
	}

	inline QRectF &QRectF::operator-=(const QMarginsF &margins)
	{
		*this = marginsRemoved(margins);
		return *this;
	}

}

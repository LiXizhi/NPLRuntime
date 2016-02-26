#pragma once
#include "ParaPoint.h"

namespace ParaEngine
{
	class QMatrix;
	class QTransform;
	class QRect;

	class QPolygon : public std::vector < QPoint >
	{
	public:
		inline QPolygon() {}
		inline ~QPolygon() {}
		inline explicit QPolygon(int size);
		inline QPolygon(const QPolygon &a) : std::vector<QPoint>(a) {}
		inline /*implicit*/ QPolygon(const std::vector<QPoint> &v) : std::vector<QPoint>(v) {}
		QPolygon(const QRect &r, bool closed = false);
		QPolygon(int nPoints, const int *points);
		inline void swap(QPolygon &other) { std::vector<QPoint>::swap(other); } // prevent std::vector<QPoint><->QPolygon swaps

		const ParaEngine::QPoint* constData() const;

		void translate(int dx, int dy);
		void translate(const QPoint &offset);

		QPolygon translated(int dx, int dy) const;
		inline QPolygon translated(const QPoint &offset) const;

		QRect boundingRect() const;

		void point(int i, int *x, int *y) const;
		QPoint point(int i) const;
		void setPoint(int index, int x, int y);
		void setPoint(int index, const QPoint &p);
		void setPoints(int nPoints, const int *points);
		void setPoints(int nPoints, int firstx, int firsty, ...);
		void putPoints(int index, int nPoints, const int *points);
		void putPoints(int index, int nPoints, int firstx, int firsty, ...);
		void putPoints(int index, int nPoints, const QPolygon & from, int fromIndex = 0);

		bool containsPoint(const QPoint &pt, FillRule fillRule) const;

		QPolygon united(const QPolygon &r) const;
		QPolygon intersected(const QPolygon &r) const;
		QPolygon subtracted(const QPolygon &r) const;

		inline QPolygon& operator << (const QPoint& pt){
			push_back(pt);
			return *this;
		}
	};

	inline QPolygon::QPolygon(int asize) : std::vector<QPoint>(asize) {}

	/*****************************************************************************
	  Misc. QPolygon functions
	  *****************************************************************************/

	inline void QPolygon::setPoint(int index, const QPoint &pt)
	{
		(*this)[index] = pt;
	}

	inline void QPolygon::setPoint(int index, int x, int y)
	{
		(*this)[index] = QPoint(x, y);
	}

	inline QPoint QPolygon::point(int index) const
	{
		return at(index);
	}

	inline void QPolygon::translate(const QPoint &offset)
	{
		translate(offset.x(), offset.y());
	}

	inline QPolygon QPolygon::translated(const QPoint &offset) const
	{
		return translated(offset.x(), offset.y());
	}

	class QRectF;

	class QPolygonF : public std::vector < QPointF >
	{
	public:
		inline QPolygonF() {}
		inline ~QPolygonF() {}
		inline explicit QPolygonF(int size);
		inline QPolygonF(const QPolygonF &a) : std::vector<QPointF>(a) {}
		inline /*implicit*/ QPolygonF(const std::vector<QPointF> &v) : std::vector<QPointF>(v) {}
		QPolygonF(const QRectF &r);
		/*implicit*/ QPolygonF(const QPolygon &a);
		inline void swap(QPolygonF &other) { std::vector<QPointF>::swap(other); } // prevent std::vector<QPointF><->QPolygonF swaps
		
		const ParaEngine::QPointF* constData() const;

		inline void translate(float dx, float dy);
		void translate(const QPointF &offset);

		inline QPolygonF translated(float dx, float dy) const;
		QPolygonF translated(const QPointF &offset) const;

		QPolygon toPolygon() const;

		bool isClosed() const { return !empty() && front() == back(); }

		QRectF boundingRect() const;

		bool containsPoint(const QPointF &pt, FillRule fillRule) const;

		QPolygonF united(const QPolygonF &r) const;
		QPolygonF intersected(const QPolygonF &r) const;
		QPolygonF subtracted(const QPolygonF &r) const;

		inline QPolygonF& operator << (const QPointF& pt){
			push_back(pt);
			return *this;
		}
	};

	inline QPolygonF::QPolygonF(int asize) : std::vector<QPointF>(asize) {}

	inline void QPolygonF::translate(float dx, float dy)
	{
		translate(QPointF(dx, dy));
	}

	inline QPolygonF QPolygonF::translated(float dx, float dy) const
	{
		return translated(QPointF(dx, dy));
	}


}
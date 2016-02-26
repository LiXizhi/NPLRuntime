#pragma once

#include "qmatrix.h"
#include "ParaRect.h"
#include "ParaLine.h"
#include <list>

namespace ParaEngine
{

	class QFont;
	class QPainterPathData;
	class QPen;
	class QPolygonF;
	class QRegion;
	class QVectorPath;

	class  QPainterPath
	{
	public:
		enum ElementType {
			MoveToElement,
			LineToElement,
			CurveToElement,
			CurveToDataElement
		};

		class Element {
		public:
			float x;
			float y;
			ElementType type;

			bool isMoveTo() const { return type == MoveToElement; }
			bool isLineTo() const { return type == LineToElement; }
			bool isCurveTo() const { return type == CurveToElement; }

			operator QPointF () const { return QPointF(x, y); }

			bool operator==(const Element &e) const {
				return Math::FuzzyCompare(x, e.x)
					&& Math::FuzzyCompare(y, e.y) && type == e.type;
			}
			inline bool operator!=(const Element &e) const { return !operator==(e); }
		};

		QPainterPath();
		explicit QPainterPath(const QPointF &startPoint);
		QPainterPath(const QPainterPath &other);
		QPainterPath &operator=(const QPainterPath &other);
		~QPainterPath();
		void swap(QPainterPath &other);

		void closeSubpath();

		void moveTo(const QPointF &p);
		inline void moveTo(float x, float y);

		void lineTo(const QPointF &p);
		inline void lineTo(float x, float y);

		void arcMoveTo(const QRectF &rect, float angle);
		inline void arcMoveTo(float x, float y, float w, float h, float angle);

		void arcTo(const QRectF &rect, float startAngle, float arcLength);
		inline void arcTo(float x, float y, float w, float h, float startAngle, float arcLength);

		void cubicTo(const QPointF &ctrlPt1, const QPointF &ctrlPt2, const QPointF &endPt);
		inline void cubicTo(float ctrlPt1x, float ctrlPt1y, float ctrlPt2x, float ctrlPt2y,
			float endPtx, float endPty);
		void quadTo(const QPointF &ctrlPt, const QPointF &endPt);
		inline void quadTo(float ctrlPtx, float ctrlPty, float endPtx, float endPty);

		QPointF currentPosition() const;

		void addRect(const QRectF &rect);
		inline void addRect(float x, float y, float w, float h);
		void addEllipse(const QRectF &rect);
		inline void addEllipse(float x, float y, float w, float h);
		inline void addEllipse(const QPointF &center, float rx, float ry);
		void addPolygon(const QPolygonF &polygon);
		void addText(const QPointF &point, const QFont &f, const std::string &text);
		inline void addText(float x, float y, const QFont &f, const std::string &text);
		void addPath(const QPainterPath &path);
		void addRegion(const QRegion &region);

		void addRoundedRect(const QRectF &rect, float xRadius, float yRadius,
			SizeMode mode = AbsoluteSize);
		inline void addRoundedRect(float x, float y, float w, float h,
			float xRadius, float yRadius,
			SizeMode mode = AbsoluteSize);

		void addRoundRect(const QRectF &rect, int xRnd, int yRnd);
		inline void addRoundRect(float x, float y, float w, float h,
			int xRnd, int yRnd);
		inline void addRoundRect(const QRectF &rect, int roundness);
		inline void addRoundRect(float x, float y, float w, float h,
			int roundness);

		void connectPath(const QPainterPath &path);

		bool contains(const QPointF &pt) const;
		bool contains(const QRectF &rect) const;
		bool intersects(const QRectF &rect) const;

		void translate(float dx, float dy);
		inline void translate(const QPointF &offset);

		QPainterPath translated(float dx, float dy) const;
		inline QPainterPath translated(const QPointF &offset) const;

		QRectF boundingRect() const;
		QRectF controlPointRect() const;

		FillRule fillRule() const;
		void setFillRule(FillRule fillRule);

		bool isEmpty() const;

		QPainterPath toReversed() const;
		std::list<QPolygonF> toSubpathPolygons(const QMatrix &matrix = QMatrix()) const;
		std::list<QPolygonF> toFillPolygons(const QMatrix &matrix = QMatrix()) const;
		QPolygonF toFillPolygon(const QMatrix &matrix = QMatrix()) const;
		std::list<QPolygonF> toSubpathPolygons(const QTransform &matrix) const;
		std::list<QPolygonF> toFillPolygons(const QTransform &matrix) const;
		QPolygonF toFillPolygon(const QTransform &matrix) const;

		int elementCount() const;
		QPainterPath::Element elementAt(int i) const;
		void setElementPositionAt(int i, float x, float y);

		float   length() const;
		float   percentAtLength(float t) const;
		QPointF pointAtPercent(float t) const;
		float   angleAtPercent(float t) const;
		float   slopeAtPercent(float t) const;

		bool intersects(const QPainterPath &p) const;
		bool contains(const QPainterPath &p) const;
		QPainterPath united(const QPainterPath &r) const;
		QPainterPath intersected(const QPainterPath &r) const;
		QPainterPath subtracted(const QPainterPath &r) const;
		QPainterPath subtractedInverted(const QPainterPath &r) const;

		QPainterPath simplified() const;

		bool operator==(const QPainterPath &other) const;
		bool operator!=(const QPainterPath &other) const;

		QPainterPath operator&(const QPainterPath &other) const;
		QPainterPath operator|(const QPainterPath &other) const;
		QPainterPath operator+(const QPainterPath &other) const;
		QPainterPath operator-(const QPainterPath &other) const;
		QPainterPath &operator&=(const QPainterPath &other);
		QPainterPath &operator|=(const QPainterPath &other);
		QPainterPath &operator+=(const QPainterPath &other);
		QPainterPath &operator-=(const QPainterPath &other);

		const QVectorPath & vectorPath() const;
	private:
		void setDirty(bool);
		void computeBoundingRect() const;
		void computeControlPointRect() const;
		
		friend class QMatrix;
		friend class QTransform;
	};

	class  QPainterPathStroker
	{
	public:
		QPainterPathStroker();
		explicit QPainterPathStroker(const QPen &pen);
		~QPainterPathStroker();

		void setWidth(float width);
		float width() const;

		QPainterPath createStroke(const QPainterPath &path) const;
	};

	inline void QPainterPath::moveTo(float x, float y)
	{
		moveTo(QPointF(x, y));
	}

	inline void QPainterPath::lineTo(float x, float y)
	{
		lineTo(QPointF(x, y));
	}

	inline void QPainterPath::arcTo(float x, float y, float w, float h, float startAngle, float arcLength)
	{
		arcTo(QRectF(x, y, w, h), startAngle, arcLength);
	}

	inline void QPainterPath::arcMoveTo(float x, float y, float w, float h, float angle)
	{
		arcMoveTo(QRectF(x, y, w, h), angle);
	}

	inline void QPainterPath::cubicTo(float ctrlPt1x, float ctrlPt1y, float ctrlPt2x, float ctrlPt2y,
		float endPtx, float endPty)
	{
		cubicTo(QPointF(ctrlPt1x, ctrlPt1y), QPointF(ctrlPt2x, ctrlPt2y),
			QPointF(endPtx, endPty));
	}

	inline void QPainterPath::quadTo(float ctrlPtx, float ctrlPty, float endPtx, float endPty)
	{
		quadTo(QPointF(ctrlPtx, ctrlPty), QPointF(endPtx, endPty));
	}

	inline void QPainterPath::addEllipse(float x, float y, float w, float h)
	{
		addEllipse(QRectF(x, y, w, h));
	}

	inline void QPainterPath::addEllipse(const QPointF &center, float rx, float ry)
	{
		addEllipse(QRectF(center.x() - rx, center.y() - ry, 2 * rx, 2 * ry));
	}

	inline void QPainterPath::addRect(float x, float y, float w, float h)
	{
		addRect(QRectF(x, y, w, h));
	}

	inline void QPainterPath::addRoundedRect(float x, float y, float w, float h,
		float xRadius, float yRadius,
		SizeMode mode)
	{
		addRoundedRect(QRectF(x, y, w, h), xRadius, yRadius, mode);
	}

	inline void QPainterPath::addRoundRect(float x, float y, float w, float h,
		int xRnd, int yRnd)
	{
		addRoundRect(QRectF(x, y, w, h), xRnd, yRnd);
	}

	inline void QPainterPath::addRoundRect(const QRectF &rect,
		int roundness)
	{
		int xRnd = roundness;
		int yRnd = roundness;
		if (rect.width() > rect.height())
			xRnd = int(roundness * rect.height() / rect.width());
		else
			yRnd = int(roundness * rect.width() / rect.height());
		addRoundRect(rect, xRnd, yRnd);
	}

	inline void QPainterPath::addRoundRect(float x, float y, float w, float h,
		int roundness)
	{
		addRoundRect(QRectF(x, y, w, h), roundness);
	}

	inline void QPainterPath::addText(float x, float y, const QFont &f, const std::string &text)
	{
		addText(QPointF(x, y), f, text);
	}

	inline void QPainterPath::translate(const QPointF &offset)
	{
		translate(offset.x(), offset.y());
	}

	inline QPainterPath QPainterPath::translated(const QPointF &offset) const
	{
		return translated(offset.x(), offset.y());
	}
}
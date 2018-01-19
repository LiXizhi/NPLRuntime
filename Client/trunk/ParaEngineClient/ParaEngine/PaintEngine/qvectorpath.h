#pragma once
#include "Painter.h"
#include "ParaRect.h"
#include "PaintEngine.h"
#include "qpainterpath.h"

namespace ParaEngine
{
	class CPaintEngine;

	struct QRealRect {
		float x1, y1, x2, y2;
	};

	typedef void(*qvectorpath_cache_cleanup)(CPaintEngine *engine, void *data);

	class QVectorPath
	{
	public:
		enum Hint {
			// Shape hints, in 0x000000ff, access using shape()
			AreaShapeMask = 0x0001,       // shape covers an area
			NonConvexShapeMask = 0x0002,       // shape is not convex
			CurvedShapeMask = 0x0004,       // shape contains curves...
			LinesShapeMask = 0x0008,
			RectangleShapeMask = 0x0010,
			ShapeMask = 0x001f,

			// Shape hints merged into basic shapes..
			LinesHint = LinesShapeMask,
			RectangleHint = AreaShapeMask | RectangleShapeMask,
			EllipseHint = AreaShapeMask | CurvedShapeMask,
			ConvexPolygonHint = AreaShapeMask,
			PolygonHint = AreaShapeMask | NonConvexShapeMask,
			RoundedRectHint = AreaShapeMask | CurvedShapeMask,
			ArbitraryShapeHint = AreaShapeMask | NonConvexShapeMask | CurvedShapeMask,

			// Other hints
			IsCachedHint = 0x0100, // Set if the cache hint is set
			ShouldUseCacheHint = 0x0200, // Set if the path should be cached when possible..
			ControlPointRect = 0x0400, // Set if the control point rect has been calculated...

			// Shape rendering specifiers...
			OddEvenFill = 0x1000,
			WindingFill = 0x2000,
			ImplicitClose = 0x4000
		};

		QVectorPath(const float *points, int count, const QPainterPath::ElementType *elements = 0, uint32 hints = ArbitraryShapeHint);
		~QVectorPath();

		QRectF controlPointRect() const;

		inline Hint shape() const { return (Hint)(m_hints & ShapeMask); }
		inline bool isConvex() const { return (m_hints & NonConvexShapeMask) == 0; }
		inline bool isCurved() const { return (m_hints & CurvedShapeMask)!=0; }

		inline bool isCacheable() const { return (m_hints & ShouldUseCacheHint) != 0; }
		inline bool hasImplicitClose() const { return (m_hints & ImplicitClose) != 0; }
		inline bool hasWindingFill() const { return (m_hints & WindingFill) != 0; }

		inline void makeCacheable() const { m_hints |= ShouldUseCacheHint; m_cache = 0; }
		inline uint32 hints() const { return m_hints; }

		inline const QPainterPath::ElementType *elements() const { return m_elements; }
		inline const float *points() const { return m_points; }
		inline bool isEmpty() const { return m_points == 0; }

		inline int elementCount() const { return m_count; }
		inline const QPainterPath convertToPainterPath() const;

		static inline uint32 polygonFlags(CPaintEngine::PolygonDrawMode mode)
		{
			switch (mode) {
			case CPaintEngine::ConvexMode: return ConvexPolygonHint | ImplicitClose;
			case CPaintEngine::OddEvenMode: return PolygonHint | OddEvenFill | ImplicitClose;
			case CPaintEngine::WindingMode: return PolygonHint | WindingFill | ImplicitClose;
			case CPaintEngine::PolylineMode: return PolygonHint;
			default: return 0;
			}
		}

		struct CacheEntry {
			CPaintEngine *engine;
			void *data;
			qvectorpath_cache_cleanup cleanup;
			CacheEntry *next;
		};

		CacheEntry *addCacheData(CPaintEngine *engine, void *data, qvectorpath_cache_cleanup cleanup) const;
		inline CacheEntry *lookupCacheData(CPaintEngine *engine) const {
			PE_ASSERT(m_hints & ShouldUseCacheHint);
			CacheEntry *e = m_cache;
			while (e) {
				if (e->engine == engine)
					return e;
				e = e->next;
			}
			return 0;
		}

		template <typename T> static inline bool isRect(const T *pts, int elementCount) {
			return (elementCount == 5 // 5-point polygon, check for closed rect
				&& pts[0] == pts[8] && pts[1] == pts[9] // last point == first point
				&& pts[0] == pts[6] && pts[2] == pts[4] // x values equal
				&& pts[1] == pts[3] && pts[5] == pts[7] // y values equal...
				&& pts[0] < pts[4] && pts[1] < pts[5]
				) ||
				(elementCount == 4 // 4-point polygon, check for unclosed rect
				&& pts[0] == pts[6] && pts[2] == pts[4] // x values equal
				&& pts[1] == pts[3] && pts[5] == pts[7] // y values equal...
				&& pts[0] < pts[4] && pts[1] < pts[5]
				);
		}

		inline bool isRect() const
		{
			const QPainterPath::ElementType * const types = elements();

			return (shape() == QVectorPath::RectangleHint)
				|| (isRect(points(), elementCount())
				&& (!types || (types[0] == QPainterPath::MoveToElement
				&& types[1] == QPainterPath::LineToElement
				&& types[2] == QPainterPath::LineToElement
				&& types[3] == QPainterPath::LineToElement)));
		}


	private:
		const QPainterPath::ElementType *m_elements;
		const float *m_points;
		const int m_count;

		mutable uint32 m_hints;
		mutable QRealRect m_cp_rect;

		mutable CacheEntry *m_cache;
	};

	const QVectorPath &qtVectorPathForPath(const QPainterPath &path);
}
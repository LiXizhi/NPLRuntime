//----------------------------------------------------------------------
// Class:	
// Authors:	LiXizhi
// company: ParaEngine
// Date:	2015.3.4
// Revised: 
//-----------------------------------------------------------------------
#include "ParaEngine.h"
#include "qvectorpath.h"

using namespace ParaEngine;


ParaEngine::QVectorPath::QVectorPath(const float *points, int count, const QPainterPath::ElementType *elements /*= 0*/, uint32 hints /*= ArbitraryShapeHint*/) 
	: m_elements(elements),m_points(points),m_count(count),m_hints(hints)
{

}

ParaEngine::QVectorPath::~QVectorPath()
{
	if (m_hints & ShouldUseCacheHint) {
		CacheEntry *e = m_cache;
		while (e) {
			if (e->data)
				e->cleanup(e->engine, e->data);
			CacheEntry *n = e->next;
			delete e;
			e = n;
		}
	}
}

QVectorPath::CacheEntry * QVectorPath::addCacheData(CPaintEngine *engine, void *data, qvectorpath_cache_cleanup cleanup) const
{
	PE_ASSERT(!lookupCacheData(engine));
	if ((m_hints & IsCachedHint) == 0) {
		m_cache = 0;
		m_hints |= IsCachedHint;
	}
	CacheEntry *e = new CacheEntry;
	e->engine = engine;
	e->data = data;
	e->cleanup = cleanup;
	e->next = m_cache;
	m_cache = e;
	return m_cache;
}

ParaEngine::QRectF QVectorPath::controlPointRect() const
{
	if (m_hints & ControlPointRect)
		return QRectF(QPointF(m_cp_rect.x1, m_cp_rect.y1), QPointF(m_cp_rect.x2, m_cp_rect.y2));

	if (m_count == 0) {
		m_cp_rect.x1 = m_cp_rect.x2 = m_cp_rect.y1 = m_cp_rect.y2 = 0;
		m_hints |= ControlPointRect;
		return QRectF(QPointF(m_cp_rect.x1, m_cp_rect.y1), QPointF(m_cp_rect.x2, m_cp_rect.y2));
	}
	PE_ASSERT(m_points && m_count > 0);

	const float *pts = m_points;
	m_cp_rect.x1 = m_cp_rect.x2 = *pts;
	++pts;
	m_cp_rect.y1 = m_cp_rect.y2 = *pts;
	++pts;

	const float *epts = m_points + (m_count << 1);
	while (pts < epts) {
		float x = *pts;
		if (x < m_cp_rect.x1) m_cp_rect.x1 = x;
		else if (x > m_cp_rect.x2) m_cp_rect.x2 = x;
		++pts;

		float y = *pts;
		if (y < m_cp_rect.y1) m_cp_rect.y1 = y;
		else if (y > m_cp_rect.y2) m_cp_rect.y2 = y;
		++pts;
	}

	m_hints |= ControlPointRect;
	return QRectF(QPointF(m_cp_rect.x1, m_cp_rect.y1), QPointF(m_cp_rect.x2, m_cp_rect.y2));
}

const QVectorPath & ParaEngine::qtVectorPathForPath(const QPainterPath &path)
{
	return path.vectorPath();
}

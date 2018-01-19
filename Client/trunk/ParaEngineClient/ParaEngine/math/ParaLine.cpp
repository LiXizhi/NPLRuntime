//-----------------------------------------------------------------------------
// Class: PaintDevice
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Date: 2015.2.23
// Desc: I have referred to QT framework's qpaintdevice.h, but greatly simplified. 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ParaLine.h"

using namespace ParaEngine;

#ifndef M_2PI
#define M_2PI 6.28318530717958647692528676655900576
#endif

float QLineF::length() const
{
    float x = pt2.x() - pt1.x();
    float y = pt2.y() - pt1.y();
    return Math::Sqrt(x*x + y*y);
}

float QLineF::angle() const
{
    const float dx = pt2.x() - pt1.x();
    const float dy = pt2.y() - pt1.y();

    const float theta = (float)((float)(Math::ATan2(-dy, dx)) * 360.0f / M_2PI);

    const float theta_normalized = theta < 0 ? theta + 360 : theta;

    if (Math::FuzzyCompare(theta_normalized, float(360)))
        return float(0);
    else
        return theta_normalized;
}


QLineF QLineF::unitVector() const
{
    float x = pt2.x() - pt1.x();
    float y = pt2.y() - pt1.y();

    float len = Math::Sqrt(x*x + y*y);
    QLineF f(p1(), QPointF(pt1.x() + x/len, pt1.y() + y/len));
    return f;
}


QLineF::IntersectType QLineF::intersect(const QLineF &l, QPointF *intersectionPoint) const
{
    // implementation is based on Graphics Gems III's "Faster Line Segment Intersection"
    const QPointF a = pt2 - pt1;
    const QPointF b = l.pt1 - l.pt2;
    const QPointF c = pt1 - l.pt1;

    const float denominator = a.y() * b.x() - a.x() * b.y();
	if (denominator == 0 || Math::is_infinity(denominator))
        return NoIntersection;

    const float reciprocal = 1 / denominator;
    const float na = (b.y() * c.x() - b.x() * c.y()) * reciprocal;
    if (intersectionPoint)
        *intersectionPoint = pt1 + a * na;

    if (na < 0 || na > 1)
        return UnboundedIntersection;

    const float nb = (a.x() * c.y() - a.y() * c.x()) * reciprocal;
    if (nb < 0 || nb > 1)
        return UnboundedIntersection;

    return BoundedIntersection;
}

float QLineF::angleTo(const QLineF &l) const
{
    if (isNull() || l.isNull())
        return 0;

    const float a1 = angle();
    const float a2 = l.angle();

    const float delta = a2 - a1;
    const float delta_normalized = delta < 0 ? delta + 360 : delta;

    if (Math::FuzzyCompare(delta, float(360)))
        return 0;
    else
        return delta_normalized;
}

float QLineF::angle(const QLineF &l) const
{
    if (isNull() || l.isNull())
        return 0;
    float cos_line = (dx()*l.dx() + dy()*l.dy()) / (length()*l.length());
    float rad = 0;
    // only accept cos_line in the range [-1,1], if it is outside, use 0 (we return 0 rather than PI for those cases)
    if (cos_line >= -1.0 && cos_line <= 1.0) rad = Math::ACos(cos_line );
    return (float)(rad * 360 / M_2PI);
}
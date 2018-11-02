//-----------------------------------------------------------------------------
// Class: 
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Date: 2015.2.24
// Desc: I have referred to QT framework, but greatly simplified. 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "qtransform.h"

using namespace ParaEngine;

#define Q_NEAR_CLIP (sizeof(float) == sizeof(double) ? 0.000001 : 0.0001)

#ifdef MAP
#  undef MAP
#endif
#define MAP(x, y, nx, ny) \
    do { \
        float FX_ = x; \
        float FY_ = y; \
        switch(t) {   \
        case TxNone:  \
            nx = FX_;   \
            ny = FY_;   \
            break;    \
        case TxTranslate:    \
            nx = FX_ + affine._dx;                \
            ny = FY_ + affine._dy;                \
            break;                              \
        case TxScale:                           \
            nx = affine._m11 * FX_ + affine._dx;  \
            ny = affine._m22 * FY_ + affine._dy;  \
            break;                              \
        case TxRotate:                          \
        case TxShear:                           \
        case TxProject:                                      \
            nx = affine._m11 * FX_ + affine._m21 * FY_ + affine._dx;        \
            ny = affine._m12 * FX_ + affine._m22 * FY_ + affine._dy;        \
            if (t == TxProject) {                                       \
                float w = (m_13 * FX_ + m_23 * FY_ + m_33);              \
                if (w < float(Q_NEAR_CLIP)) w = float(Q_NEAR_CLIP);     \
                w = 1.f/w;                                               \
                nx *= w;                                                \
                ny *= w;                                                \
            }                                                           \
        }                                                               \
    } while (0)


QTransform::QTransform()
    : affine(true)
    , m_13(0), m_23(0), m_33(1)
    , m_type(TxNone)
    , m_dirty(TxNone)
{
}

QTransform::QTransform(float h11, float h12, float h13,
                       float h21, float h22, float h23,
                       float h31, float h32, float h33)
    : affine(h11, h12, h21, h22, h31, h32, true)
    , m_13(h13), m_23(h23), m_33(h33)
    , m_type(TxNone)
    , m_dirty(TxProject)
{
}

QTransform::QTransform(float h11, float h12, float h21,
                       float h22, float dx, float dy)
    : affine(h11, h12, h21, h22, dx, dy, true)
    , m_13(0), m_23(0), m_33(1)
    , m_type(TxNone)
    , m_dirty(TxShear)
{
}

QTransform::QTransform(const QMatrix &mtx)
    : affine(mtx._m11, mtx._m12, mtx._m21, mtx._m22, mtx._dx, mtx._dy, true),
      m_13(0), m_23(0), m_33(1)
    , m_type(TxNone)
    , m_dirty(TxShear)
{
}


QTransform QTransform::adjoint() const
{
    float h11, h12, h13,
        h21, h22, h23,
        h31, h32, h33;
    h11 = affine._m22*m_33 - m_23*affine._dy;
    h21 = m_23*affine._dx - affine._m21*m_33;
    h31 = affine._m21*affine._dy - affine._m22*affine._dx;
    h12 = m_13*affine._dy - affine._m12*m_33;
    h22 = affine._m11*m_33 - m_13*affine._dx;
    h32 = affine._m12*affine._dx - affine._m11*affine._dy;
    h13 = affine._m12*m_23 - m_13*affine._m22;
    h23 = m_13*affine._m21 - affine._m11*m_23;
    h33 = affine._m11*affine._m22 - affine._m12*affine._m21;

    return QTransform(h11, h12, h13,
                      h21, h22, h23,
                      h31, h32, h33, true);
}


QTransform QTransform::transposed() const
{
    QTransform t(affine._m11, affine._m21, affine._dx,
                 affine._m12, affine._m22, affine._dy,
                 m_13, m_23, m_33, true);
    t.m_type = m_type;
    t.m_dirty = m_dirty;
    return t;
}

QTransform QTransform::inverted(bool *invertible) const
{
    QTransform invert(true);
    bool inv = true;

    switch(inline_type()) {
    case TxNone:
        break;
    case TxTranslate:
        invert.affine._dx = -affine._dx;
        invert.affine._dy = -affine._dy;
        break;
    case TxScale:
        inv = !Math::FuzzyIsNull(affine._m11);
        inv &= !Math::FuzzyIsNull(affine._m22);
        if (inv) {
            invert.affine._m11 = 1.f / affine._m11;
            invert.affine._m22 = 1.f / affine._m22;
            invert.affine._dx = -affine._dx * invert.affine._m11;
            invert.affine._dy = -affine._dy * invert.affine._m22;
        }
        break;
    case TxRotate:
    case TxShear:
        invert.affine = affine.inverted(&inv);
        break;
    default:
        // general case
        float det = determinant();
        inv = !Math::FuzzyIsNull(det);
        if (inv)
            invert = adjoint() / det;
        break;
    }

    if (invertible)
        *invertible = inv;

    if (inv) {
        // inverting doesn't change the type
        invert.m_type = m_type;
        invert.m_dirty = m_dirty;
    }

    return invert;
}


QTransform &QTransform::translate(float dx, float dy)
{
    if (dx == 0 && dy == 0)
        return *this;
    switch(inline_type()) {
    case TxNone:
        affine._dx = dx;
        affine._dy = dy;
        break;
    case TxTranslate:
        affine._dx += dx;
        affine._dy += dy;
        break;
    case TxScale:
        affine._dx += dx*affine._m11;
        affine._dy += dy*affine._m22;
        break;
    case TxProject:
        m_33 += dx*m_13 + dy*m_23;
        // Fall through
    case TxShear:
    case TxRotate:
        affine._dx += dx*affine._m11 + dy*affine._m21;
        affine._dy += dy*affine._m22 + dx*affine._m12;
        break;
    }
    if (m_dirty < TxTranslate)
        m_dirty = TxTranslate;
    return *this;
}

QTransform QTransform::fromTranslate(float dx, float dy)
{
    QTransform transform(1, 0, 0, 0, 1, 0, dx, dy, 1, true);
    if (dx == 0 && dy == 0)
        transform.m_type = TxNone;
    else
        transform.m_type = TxTranslate;
    transform.m_dirty = TxNone;
    return transform;
}


QTransform & QTransform::scale(float sx, float sy)
{
    if (sx == 1 && sy == 1)
        return *this;

    switch(inline_type()) {
    case TxNone:
    case TxTranslate:
        affine._m11 = sx;
        affine._m22 = sy;
        break;
    case TxProject:
        m_13 *= sx;
        m_23 *= sy;
        // fall through
    case TxRotate:
    case TxShear:
        affine._m12 *= sx;
        affine._m21 *= sy;
        // fall through
    case TxScale:
        affine._m11 *= sx;
        affine._m22 *= sy;
        break;
    }
    if (m_dirty < TxScale)
        m_dirty = TxScale;
    return *this;
}


QTransform QTransform::fromScale(float sx, float sy)
{
    QTransform transform(sx, 0, 0, 0, sy, 0, 0, 0, 1, true);
    if (sx == 1. && sy == 1.)
        transform.m_type = TxNone;
    else
        transform.m_type = TxScale;
    transform.m_dirty = TxNone;
    return transform;
}

QTransform & QTransform::shear(float sh, float sv)
{
    if (sh == 0 && sv == 0)
        return *this;

    switch(inline_type()) {
    case TxNone:
    case TxTranslate:
        affine._m12 = sv;
        affine._m21 = sh;
        break;
    case TxScale:
        affine._m12 = sv*affine._m22;
        affine._m21 = sh*affine._m11;
        break;
    case TxProject: {
        float tm13 = sv*m_23;
        float tm23 = sh*m_13;
        m_13 += tm13;
        m_23 += tm23;
    }
        // fall through
    case TxRotate:
    case TxShear: {
        float tm11 = sv*affine._m21;
        float tm22 = sh*affine._m12;
        float tm12 = sv*affine._m22;
        float tm21 = sh*affine._m11;
        affine._m11 += tm11; affine._m12 += tm12;
        affine._m21 += tm21; affine._m22 += tm22;
        break;
    }
    }
    if (m_dirty < TxShear)
        m_dirty = TxShear;
    return *this;
}

const float deg2rad = float(0.017453292519943295769);        // pi/180
const float inv_dist_to_plane = 1. / 1024.;

QTransform & QTransform::rotate(float a, Axis axis)
{
    if (a == 0)
        return *this;

    float sina = 0;
    float cosa = 0;
    if (a == 90. || a == -270.)
        sina = 1.;
    else if (a == 270. || a == -90.)
        sina = -1.;
    else if (a == 180.)
        cosa = -1.;
    else{
        float b = deg2rad*a;          // convert to radians
        sina = Math::Sin(b);               // fast and convenient
		cosa = Math::Cos(b);
    }

    if (axis == ZAxis) {
        switch(inline_type()) {
        case TxNone:
        case TxTranslate:
            affine._m11 = cosa;
            affine._m12 = sina;
            affine._m21 = -sina;
            affine._m22 = cosa;
            break;
        case TxScale: {
            float tm11 = cosa*affine._m11;
            float tm12 = sina*affine._m22;
            float tm21 = -sina*affine._m11;
            float tm22 = cosa*affine._m22;
            affine._m11 = tm11; affine._m12 = tm12;
            affine._m21 = tm21; affine._m22 = tm22;
            break;
        }
        case TxProject: {
            float tm13 = cosa*m_13 + sina*m_23;
            float tm23 = -sina*m_13 + cosa*m_23;
            m_13 = tm13;
            m_23 = tm23;
            // fall through
        }
        case TxRotate:
        case TxShear: {
            float tm11 = cosa*affine._m11 + sina*affine._m21;
            float tm12 = cosa*affine._m12 + sina*affine._m22;
            float tm21 = -sina*affine._m11 + cosa*affine._m21;
            float tm22 = -sina*affine._m12 + cosa*affine._m22;
            affine._m11 = tm11; affine._m12 = tm12;
            affine._m21 = tm21; affine._m22 = tm22;
            break;
        }
        }
        if (m_dirty < TxRotate)
            m_dirty = TxRotate;
    } else {
        QTransform result;
        if (axis == YAxis) {
            result.affine._m11 = cosa;
            result.m_13 = -sina * inv_dist_to_plane;
        } else {
            result.affine._m22 = cosa;
            result.m_23 = -sina * inv_dist_to_plane;
        }
        result.m_type = TxProject;
        *this = result * *this;
    }

    return *this;
}

QTransform & QTransform::rotateRadians(float a, Axis axis)
{

    float sina = Math::Sin(a);
    float cosa = Math::Cos(a);

    if (axis == ZAxis) {
        switch(inline_type()) {
        case TxNone:
        case TxTranslate:
            affine._m11 = cosa;
            affine._m12 = sina;
            affine._m21 = -sina;
            affine._m22 = cosa;
            break;
        case TxScale: {
            float tm11 = cosa*affine._m11;
            float tm12 = sina*affine._m22;
            float tm21 = -sina*affine._m11;
            float tm22 = cosa*affine._m22;
            affine._m11 = tm11; affine._m12 = tm12;
            affine._m21 = tm21; affine._m22 = tm22;
            break;
        }
        case TxProject: {
            float tm13 = cosa*m_13 + sina*m_23;
            float tm23 = -sina*m_13 + cosa*m_23;
            m_13 = tm13;
            m_23 = tm23;
            // fall through
        }
        case TxRotate:
        case TxShear: {
            float tm11 = cosa*affine._m11 + sina*affine._m21;
            float tm12 = cosa*affine._m12 + sina*affine._m22;
            float tm21 = -sina*affine._m11 + cosa*affine._m21;
            float tm22 = -sina*affine._m12 + cosa*affine._m22;
            affine._m11 = tm11; affine._m12 = tm12;
            affine._m21 = tm21; affine._m22 = tm22;
            break;
        }
        }
        if (m_dirty < TxRotate)
            m_dirty = TxRotate;
    } else {
        QTransform result;
        if (axis == YAxis) {
            result.affine._m11 = cosa;
            result.m_13 = -sina * inv_dist_to_plane;
        } else {
            result.affine._m22 = cosa;
            result.m_23 = -sina * inv_dist_to_plane;
        }
        result.m_type = TxProject;
        *this = result * *this;
    }
    return *this;
}


bool QTransform::operator==(const QTransform &o) const
{
    return affine._m11 == o.affine._m11 &&
           affine._m12 == o.affine._m12 &&
           affine._m21 == o.affine._m21 &&
           affine._m22 == o.affine._m22 &&
           affine._dx == o.affine._dx &&
           affine._dy == o.affine._dy &&
           m_13 == o.m_13 &&
           m_23 == o.m_23 &&
           m_33 == o.m_33;
}


bool QTransform::operator!=(const QTransform &o) const
{
    return !operator==(o);
}


QTransform & QTransform::operator*=(const QTransform &o)
{
    const TransformationType otherType = o.inline_type();
    if (otherType == TxNone)
        return *this;

    const TransformationType thisType = inline_type();
    if (thisType == TxNone)
        return operator=(o);

    TransformationType t = Math::Max(thisType, otherType);
    switch(t) {
    case TxNone:
        break;
    case TxTranslate:
        affine._dx += o.affine._dx;
        affine._dy += o.affine._dy;
        break;
    case TxScale:
    {
        float m11 = affine._m11*o.affine._m11;
        float m22 = affine._m22*o.affine._m22;

        float m31 = affine._dx*o.affine._m11 + o.affine._dx;
        float m32 = affine._dy*o.affine._m22 + o.affine._dy;

        affine._m11 = m11;
        affine._m22 = m22;
        affine._dx = m31; affine._dy = m32;
        break;
    }
    case TxRotate:
    case TxShear:
    {
        float m11 = affine._m11*o.affine._m11 + affine._m12*o.affine._m21;
        float m12 = affine._m11*o.affine._m12 + affine._m12*o.affine._m22;

        float m21 = affine._m21*o.affine._m11 + affine._m22*o.affine._m21;
        float m22 = affine._m21*o.affine._m12 + affine._m22*o.affine._m22;

        float m31 = affine._dx*o.affine._m11 + affine._dy*o.affine._m21 + o.affine._dx;
        float m32 = affine._dx*o.affine._m12 + affine._dy*o.affine._m22 + o.affine._dy;

        affine._m11 = m11; affine._m12 = m12;
        affine._m21 = m21; affine._m22 = m22;
        affine._dx = m31; affine._dy = m32;
        break;
    }
    case TxProject:
    {
        float m11 = affine._m11*o.affine._m11 + affine._m12*o.affine._m21 + m_13*o.affine._dx;
        float m12 = affine._m11*o.affine._m12 + affine._m12*o.affine._m22 + m_13*o.affine._dy;
        float m13 = affine._m11*o.m_13 + affine._m12*o.m_23 + m_13*o.m_33;

        float m21 = affine._m21*o.affine._m11 + affine._m22*o.affine._m21 + m_23*o.affine._dx;
        float m22 = affine._m21*o.affine._m12 + affine._m22*o.affine._m22 + m_23*o.affine._dy;
        float m23 = affine._m21*o.m_13 + affine._m22*o.m_23 + m_23*o.m_33;

        float m31 = affine._dx*o.affine._m11 + affine._dy*o.affine._m21 + m_33*o.affine._dx;
        float m32 = affine._dx*o.affine._m12 + affine._dy*o.affine._m22 + m_33*o.affine._dy;
        float m33 = affine._dx*o.m_13 + affine._dy*o.m_23 + m_33*o.m_33;

        affine._m11 = m11; affine._m12 = m12; m_13 = m13;
        affine._m21 = m21; affine._m22 = m22; m_23 = m23;
        affine._dx = m31; affine._dy = m32; m_33 = m33;
    }
    }

    m_dirty = t;
    m_type = t;

    return *this;
}


QTransform QTransform::operator*(const QTransform &m) const
{
    const TransformationType otherType = m.inline_type();
    if (otherType == TxNone)
        return *this;

    const TransformationType thisType = inline_type();
    if (thisType == TxNone)
        return m;

    QTransform t(true);
    TransformationType type = Math::Max(thisType, otherType);
    switch(type) {
    case TxNone:
        break;
    case TxTranslate:
        t.affine._dx = affine._dx + m.affine._dx;
        t.affine._dy += affine._dy + m.affine._dy;
        break;
    case TxScale:
    {
        float m11 = affine._m11*m.affine._m11;
        float m22 = affine._m22*m.affine._m22;

        float m31 = affine._dx*m.affine._m11 + m.affine._dx;
        float m32 = affine._dy*m.affine._m22 + m.affine._dy;

        t.affine._m11 = m11;
        t.affine._m22 = m22;
        t.affine._dx = m31; t.affine._dy = m32;
        break;
    }
    case TxRotate:
    case TxShear:
    {
        float m11 = affine._m11*m.affine._m11 + affine._m12*m.affine._m21;
        float m12 = affine._m11*m.affine._m12 + affine._m12*m.affine._m22;

        float m21 = affine._m21*m.affine._m11 + affine._m22*m.affine._m21;
        float m22 = affine._m21*m.affine._m12 + affine._m22*m.affine._m22;

        float m31 = affine._dx*m.affine._m11 + affine._dy*m.affine._m21 + m.affine._dx;
        float m32 = affine._dx*m.affine._m12 + affine._dy*m.affine._m22 + m.affine._dy;

        t.affine._m11 = m11; t.affine._m12 = m12;
        t.affine._m21 = m21; t.affine._m22 = m22;
        t.affine._dx = m31; t.affine._dy = m32;
        break;
    }
    case TxProject:
    {
        float m11 = affine._m11*m.affine._m11 + affine._m12*m.affine._m21 + m_13*m.affine._dx;
        float m12 = affine._m11*m.affine._m12 + affine._m12*m.affine._m22 + m_13*m.affine._dy;
        float m13 = affine._m11*m.m_13 + affine._m12*m.m_23 + m_13*m.m_33;

        float m21 = affine._m21*m.affine._m11 + affine._m22*m.affine._m21 + m_23*m.affine._dx;
        float m22 = affine._m21*m.affine._m12 + affine._m22*m.affine._m22 + m_23*m.affine._dy;
        float m23 = affine._m21*m.m_13 + affine._m22*m.m_23 + m_23*m.m_33;

        float m31 = affine._dx*m.affine._m11 + affine._dy*m.affine._m21 + m_33*m.affine._dx;
        float m32 = affine._dx*m.affine._m12 + affine._dy*m.affine._m22 + m_33*m.affine._dy;
        float m33 = affine._dx*m.m_13 + affine._dy*m.m_23 + m_33*m.m_33;

        t.affine._m11 = m11; t.affine._m12 = m12; t.m_13 = m13;
        t.affine._m21 = m21; t.affine._m22 = m22; t.m_23 = m23;
        t.affine._dx = m31; t.affine._dy = m32; t.m_33 = m33;
    }
    }

    t.m_dirty = type;
    t.m_type = type;

    return t;
}

QTransform & QTransform::operator=(const QTransform &matrix)
{
    affine._m11 = matrix.affine._m11;
    affine._m12 = matrix.affine._m12;
    affine._m21 = matrix.affine._m21;
    affine._m22 = matrix.affine._m22;
    affine._dx = matrix.affine._dx;
    affine._dy = matrix.affine._dy;
    m_13 = matrix.m_13;
    m_23 = matrix.m_23;
    m_33 = matrix.m_33;
    m_type = matrix.m_type;
    m_dirty = matrix.m_dirty;

    return *this;
}


void QTransform::reset()
{
    affine._m11 = affine._m22 = m_33 = 1.0;
    affine._m12 = m_13 = affine._m21 = m_23 = affine._dx = affine._dy = 0;
    m_type = TxNone;
    m_dirty = TxNone;
}
QPoint QTransform::map(const QPoint &p) const
{
    float fx = (float)p.x();
	float fy = (float)p.y();

    float x = 0, y = 0;

    TransformationType t = inline_type();
    switch(t) {
    case TxNone:
        x = fx;
        y = fy;
        break;
    case TxTranslate:
        x = fx + affine._dx;
        y = fy + affine._dy;
        break;
    case TxScale:
        x = affine._m11 * fx + affine._dx;
        y = affine._m22 * fy + affine._dy;
        break;
    case TxRotate:
    case TxShear:
    case TxProject:
        x = affine._m11 * fx + affine._m21 * fy + affine._dx;
        y = affine._m12 * fx + affine._m22 * fy + affine._dy;
        if (t == TxProject) {
            float w = 1.f/(m_13 * fx + m_23 * fy + m_33);
            x *= w;
            y *= w;
        }
    }
    return QPoint(Math::Round(x), Math::Round(y));
}

QPointF QTransform::map(const QPointF &p) const
{
    float fx = p.x();
    float fy = p.y();

    float x = 0, y = 0;

    TransformationType t = inline_type();
    switch(t) {
    case TxNone:
        x = fx;
        y = fy;
        break;
    case TxTranslate:
        x = fx + affine._dx;
        y = fy + affine._dy;
        break;
    case TxScale:
        x = affine._m11 * fx + affine._dx;
        y = affine._m22 * fy + affine._dy;
        break;
    case TxRotate:
    case TxShear:
    case TxProject:
        x = affine._m11 * fx + affine._m21 * fy + affine._dx;
        y = affine._m12 * fx + affine._m22 * fy + affine._dy;
        if (t == TxProject) {
            float w = 1.f/(m_13 * fx + m_23 * fy + m_33);
            x *= w;
            y *= w;
        }
    }
    return QPointF(x, y);
}

QLine QTransform::map(const QLine &l) const
{
	float fx1 = (float)l.x1();
    float fy1 = (float)l.y1();
    float fx2 = (float)l.x2();
    float fy2 = (float)l.y2();

    float x1 = 0, y1 = 0, x2 = 0, y2 = 0;

    TransformationType t = inline_type();
    switch(t) {
    case TxNone:
        x1 = fx1;
        y1 = fy1;
        x2 = fx2;
        y2 = fy2;
        break;
    case TxTranslate:
        x1 = fx1 + affine._dx;
        y1 = fy1 + affine._dy;
        x2 = fx2 + affine._dx;
        y2 = fy2 + affine._dy;
        break;
    case TxScale:
        x1 = affine._m11 * fx1 + affine._dx;
        y1 = affine._m22 * fy1 + affine._dy;
        x2 = affine._m11 * fx2 + affine._dx;
        y2 = affine._m22 * fy2 + affine._dy;
        break;
    case TxRotate:
    case TxShear:
    case TxProject:
        x1 = affine._m11 * fx1 + affine._m21 * fy1 + affine._dx;
        y1 = affine._m12 * fx1 + affine._m22 * fy1 + affine._dy;
        x2 = affine._m11 * fx2 + affine._m21 * fy2 + affine._dx;
        y2 = affine._m12 * fx2 + affine._m22 * fy2 + affine._dy;
        if (t == TxProject) {
            float w = 1.f/(m_13 * fx1 + m_23 * fy1 + m_33);
            x1 *= w;
            y1 *= w;
            w = 1.f/(m_13 * fx2 + m_23 * fy2 + m_33);
            x2 *= w;
            y2 *= w;
        }
    }
    return QLine(Math::Round(x1), Math::Round(y1), Math::Round(x2), Math::Round(y2));
}

QLineF QTransform::map(const QLineF &l) const
{
    float fx1 = l.x1();
    float fy1 = l.y1();
    float fx2 = l.x2();
    float fy2 = l.y2();

    float x1 = 0, y1 = 0, x2 = 0, y2 = 0;

    TransformationType t = inline_type();
    switch(t) {
    case TxNone:
        x1 = fx1;
        y1 = fy1;
        x2 = fx2;
        y2 = fy2;
        break;
    case TxTranslate:
        x1 = fx1 + affine._dx;
        y1 = fy1 + affine._dy;
        x2 = fx2 + affine._dx;
        y2 = fy2 + affine._dy;
        break;
    case TxScale:
        x1 = affine._m11 * fx1 + affine._dx;
        y1 = affine._m22 * fy1 + affine._dy;
        x2 = affine._m11 * fx2 + affine._dx;
        y2 = affine._m22 * fy2 + affine._dy;
        break;
    case TxRotate:
    case TxShear:
    case TxProject:
        x1 = affine._m11 * fx1 + affine._m21 * fy1 + affine._dx;
        y1 = affine._m12 * fx1 + affine._m22 * fy1 + affine._dy;
        x2 = affine._m11 * fx2 + affine._m21 * fy2 + affine._dx;
        y2 = affine._m12 * fx2 + affine._m22 * fy2 + affine._dy;
        if (t == TxProject) {
            float w = 1.f/(m_13 * fx1 + m_23 * fy1 + m_33);
            x1 *= w;
            y1 *= w;
            w = 1.f/(m_13 * fx2 + m_23 * fy2 + m_33);
            x2 *= w;
            y2 *= w;
        }
    }
    return QLineF(x1, y1, x2, y2);
}


struct QHomogeneousCoordinate
{
    float x;
    float y;
    float w;

    QHomogeneousCoordinate() {}
    QHomogeneousCoordinate(float x_, float y_, float w_) : x(x_), y(y_), w(w_) {}

    const QPointF toPoint() const {
        float iw = 1.f / w;
        return QPointF(x * iw, y * iw);
    }
};

static inline QHomogeneousCoordinate mapHomogeneous(const QTransform &transform, const QPointF &p)
{
    QHomogeneousCoordinate c;
    c.x = transform.m11() * p.x() + transform.m21() * p.y() + transform.m31();
    c.y = transform.m12() * p.x() + transform.m22() * p.y() + transform.m32();
    c.w = transform.m13() * p.x() + transform.m23() * p.y() + transform.m33();
    return c;
}

bool qt_scaleForTransform(const QTransform &transform, float *scale);

void QTransform::setMatrix(float m11, float m12, float m13,
                           float m21, float m22, float m23,
                           float m31, float m32, float m33)
{
    affine._m11 = m11; affine._m12 = m12; m_13 = m13;
    affine._m21 = m21; affine._m22 = m22; m_23 = m23;
    affine._dx = m31; affine._dy = m32; m_33 = m33;
    m_type = TxNone;
    m_dirty = TxProject;
}

static inline bool needsPerspectiveClipping(const QRectF &rect, const QTransform &transform)
{
    const float wx = Math::Min(transform.m13() * rect.left(), transform.m13() * rect.right());
    const float wy = Math::Min(transform.m23() * rect.top(), transform.m23() * rect.bottom());

    return wx + wy + transform.m33() < Q_NEAR_CLIP;
}

QRect QTransform::mapRect(const QRect &rect) const
{
    TransformationType t = inline_type();
    if (t <= TxTranslate)
        return rect.translated(Math::Round(affine._dx), Math::Round(affine._dy));

    if (t <= TxScale) {
        int x = Math::Round(affine._m11*rect.x() + affine._dx);
        int y = Math::Round(affine._m22*rect.y() + affine._dy);
        int w = Math::Round(affine._m11*rect.width());
        int h = Math::Round(affine._m22*rect.height());
        if (w < 0) {
            w = -w;
            x -= w;
        }
        if (h < 0) {
            h = -h;
            y -= h;
        }
        return QRect(x, y, w, h);
    } else if (t < TxProject || !needsPerspectiveClipping(rect, *this)) {
        // see mapToPolygon for explanations of the algorithm.
        float x = 0, y = 0;
		MAP((float)rect.left(), (float)rect.top(), x, y);
        float xmin = x;
        float ymin = y;
        float xmax = x;
        float ymax = y;
		MAP((float)rect.right() + 1, (float)rect.top(), x, y);
        xmin = Math::Min(xmin, x);
        ymin = Math::Min(ymin, y);
        xmax = Math::Max(xmax, x);
        ymax = Math::Max(ymax, y);
		MAP((float)rect.right() + 1, (float)rect.bottom() + 1, x, y);
        xmin = Math::Min(xmin, x);
        ymin = Math::Min(ymin, y);
        xmax = Math::Max(xmax, x);
        ymax = Math::Max(ymax, y);
		MAP((float)rect.left(), (float)rect.bottom() + 1, x, y);
        xmin = Math::Min(xmin, x);
        ymin = Math::Min(ymin, y);
        xmax = Math::Max(xmax, x);
        ymax = Math::Max(ymax, y);
        return QRect(Math::Round(xmin), Math::Round(ymin), Math::Round(xmax)-Math::Round(xmin), Math::Round(ymax)-Math::Round(ymin));
    } else {
        /* TODO:Xizhi
		QPainterPath path;
        path.addRect(rect);
        return map(path).boundingRect().toRect();*/
		return QRect();
    }
}

QRectF QTransform::mapRect(const QRectF &rect) const
{
    TransformationType t = inline_type();
    if (t <= TxTranslate)
        return rect.translated(affine._dx, affine._dy);

    if (t <= TxScale) {
        float x = affine._m11*rect.x() + affine._dx;
        float y = affine._m22*rect.y() + affine._dy;
        float w = affine._m11*rect.width();
        float h = affine._m22*rect.height();
        if (w < 0) {
            w = -w;
            x -= w;
        }
        if (h < 0) {
            h = -h;
            y -= h;
        }
        return QRectF(x, y, w, h);
    } else if (t < TxProject || !needsPerspectiveClipping(rect, *this)) {
        float x = 0, y = 0;
		MAP((float)rect.x(), (float)rect.y(), x, y);
        float xmin = x;
        float ymin = y;
        float xmax = x;
        float ymax = y;
		MAP((float)(rect.x() + rect.width()), (float)rect.y(), x, y);
        xmin = Math::Min(xmin, x);
        ymin = Math::Min(ymin, y);
        xmax = Math::Max(xmax, x);
        ymax = Math::Max(ymax, y);
		MAP((float)(rect.x() + rect.width()), (float)(rect.y() + rect.height()), x, y);
        xmin = Math::Min(xmin, x);
        ymin = Math::Min(ymin, y);
        xmax = Math::Max(xmax, x);
        ymax = Math::Max(ymax, y);
		MAP((float)rect.x(), (float)(rect.y() + rect.height()), x, y);
        xmin = Math::Min(xmin, x);
        ymin = Math::Min(ymin, y);
        xmax = Math::Max(xmax, x);
        ymax = Math::Max(ymax, y);
        return QRectF(xmin, ymin, xmax-xmin, ymax - ymin);
    } else {
       /* QPainterPath path;
        path.addRect(rect);
        return map(path).boundingRect();*/
		return QRectF();
    }
}

void QTransform::map(float x, float y, float *tx, float *ty) const
{
    TransformationType t = inline_type();
    MAP(x, y, *tx, *ty);
}

void QTransform::map(int x, int y, int *tx, int *ty) const
{
    TransformationType t = inline_type();
    float fx = 0, fy = 0;
	MAP((float)x, (float)y, fx, fy);
    *tx = Math::Round(fx);
    *ty = Math::Round(fy);
}

const QMatrix &QTransform::toAffine() const
{
    return affine;
}


QTransform::TransformationType QTransform::type() const
{
    if(m_dirty == TxNone || m_dirty < m_type)
        return static_cast<TransformationType>(m_type);

    switch (static_cast<TransformationType>(m_dirty)) {
    case TxProject:
        if (!Math::FuzzyIsNull(m_13) || !Math::FuzzyIsNull(m_23) || !Math::FuzzyIsNull(m_33 - 1)) {
             m_type = TxProject;
             break;
         }
    case TxShear:
    case TxRotate:
        if (!Math::FuzzyIsNull(affine._m12) || !Math::FuzzyIsNull(affine._m21)) {
            const float dot = affine._m11 * affine._m12 + affine._m21 * affine._m22;
            if (Math::FuzzyIsNull(dot))
                m_type = TxRotate;
            else
                m_type = TxShear;
            break;
        }
    case TxScale:
        if (!Math::FuzzyIsNull(affine._m11 - 1) || !Math::FuzzyIsNull(affine._m22 - 1)) {
            m_type = TxScale;
            break;
        }
    case TxTranslate:
        if (!Math::FuzzyIsNull(affine._dx) || !Math::FuzzyIsNull(affine._dy)) {
            m_type = TxTranslate;
            break;
        }
    case TxNone:
        m_type = TxNone;
        break;
    }

    m_dirty = TxNone;
    return static_cast<TransformationType>(m_type);
}

// returns true if the transform is uniformly scaling
// (same scale in x and y direction)
// scale is set to the max of x and y scaling factors
bool qt_scaleForTransform(const QTransform &transform, float *scale)
{
    const QTransform::TransformationType type = transform.type();
    if (type <= QTransform::TxTranslate) {
        if (scale)
            *scale = 1;
        return true;
    } else if (type == QTransform::TxScale) {
        const float xScale = Math::Abs(transform.m11());
        const float yScale = Math::Abs(transform.m22());
        if (scale)
            *scale = Math::Max(xScale, yScale);
        return Math::FuzzyCompare(xScale, yScale);
    }

    // rotate then scale: compare columns
    const float xScale1 = transform.m11() * transform.m11()
                         + transform.m21() * transform.m21();
    const float yScale1 = transform.m12() * transform.m12()
                         + transform.m22() * transform.m22();

    // scale then rotate: compare rows
    const float xScale2 = transform.m11() * transform.m11()
                         + transform.m12() * transform.m12();
    const float yScale2 = transform.m21() * transform.m21()
                         + transform.m22() * transform.m22();

    // decide the order of rotate and scale operations
    if (Math::Abs(xScale1 - yScale1) > Math::Abs(xScale2 - yScale2)) {
        if (scale)
            *scale = Math::Sqrt(Math::Max(xScale1, yScale1));

        return type == QTransform::TxRotate && Math::FuzzyCompare(xScale1, yScale1);
    } else {
        if (scale)
            *scale = Math::Sqrt(Math::Max(xScale2, yScale2));

        return type == QTransform::TxRotate && Math::FuzzyCompare(xScale2, yScale2);
    }
}

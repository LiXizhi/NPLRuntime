//-----------------------------------------------------------------------------
// Class: 
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Date: 2015.2.24
// Desc: I have referred to QT framework, but greatly simplified. 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "qmatrix.h"

using namespace ParaEngine;

// some defines to inline some code
#define MAPDOUBLE(x, y, nx, ny) \
{ \
    float fx = x; \
    float fy = y; \
    nx = _m11*fx + _m21*fy + _dx; \
    ny = _m12*fx + _m22*fy + _dy; \
}

#define MAPINT(x, y, nx, ny) \
{ \
    float fx = x; \
    float fy = y; \
    nx = Math::Round(_m11*fx + _m21*fy + _dx); \
    ny = Math::Round(_m12*fx + _m22*fy + _dy); \
}


QMatrix::QMatrix()
    : _m11(1.)
    , _m12(0.)
    , _m21(0.)
    , _m22(1.)
    , _dx(0.)
    , _dy(0.)
{
}

QMatrix::QMatrix(float m11, float m12, float m21, float m22, float dx, float dy)
    : _m11(m11)
    , _m12(m12)
    , _m21(m21)
    , _m22(m22)
    , _dx(dx)
    , _dy(dy)
{
}


QMatrix::QMatrix(const QMatrix &matrix)
    : _m11(matrix._m11)
    , _m12(matrix._m12)
    , _m21(matrix._m21)
    , _m22(matrix._m22)
    , _dx(matrix._dx)
    , _dy(matrix._dy)
{
}

void QMatrix::setMatrix(float m11, float m12, float m21, float m22, float dx, float dy)
{
    _m11 = m11;
    _m12 = m12;
    _m21 = m21;
    _m22 = m22;
    _dx  = dx;
    _dy  = dy;
}


void QMatrix::map(float x, float y, float *tx, float *ty) const
{
    MAPDOUBLE(x, y, *tx, *ty);
}

void QMatrix::map(int x, int y, int *tx, int *ty) const
{
	MAPINT((float)x, (float)y, *tx, *ty);
}

QRect QMatrix::mapRect(const QRect &rect) const
{
    QRect result;
    if (_m12 == 0.0F && _m21 == 0.0F) {
        int x = Math::Round(_m11*rect.x() + _dx);
        int y = Math::Round(_m22*rect.y() + _dy);
        int w = Math::Round(_m11*rect.width());
        int h = Math::Round(_m22*rect.height());
        if (w < 0) {
            w = -w;
            x -= w;
        }
        if (h < 0) {
            h = -h;
            y -= h;
        }
        result = QRect(x, y, w, h);
    } else {
        // see mapToPolygon for explanations of the algorithm.
        float x0, y0;
        float x, y;
		MAPDOUBLE((float)rect.left(), (float)rect.top(), x0, y0);
        float xmin = x0;
        float ymin = y0;
        float xmax = x0;
        float ymax = y0;
		MAPDOUBLE((float)(rect.right() + 1), (float)rect.top(), x, y);
        xmin = Math::Min(xmin, x);
        ymin = Math::Min(ymin, y);
        xmax = Math::Max(xmax, x);
        ymax = Math::Max(ymax, y);
		MAPDOUBLE((float)(rect.right() + 1), (float)(rect.bottom() + 1), x, y);
        xmin = Math::Min(xmin, x);
        ymin = Math::Min(ymin, y);
        xmax = Math::Max(xmax, x);
        ymax = Math::Max(ymax, y);
		MAPDOUBLE((float)rect.left(), (float)rect.bottom() + 1, x, y);
        xmin = Math::Min(xmin, x);
        ymin = Math::Min(ymin, y);
        xmax = Math::Max(xmax, x);
        ymax = Math::Max(ymax, y);
        result = QRect(Math::Round(xmin), Math::Round(ymin), Math::Round(xmax)-Math::Round(xmin), Math::Round(ymax)-Math::Round(ymin));
    }
    return result;
}

QRectF QMatrix::mapRect(const QRectF &rect) const
{
    QRectF result;
    if (_m12 == 0.0F && _m21 == 0.0F) {
        float x = _m11*rect.x() + _dx;
        float y = _m22*rect.y() + _dy;
        float w = _m11*rect.width();
        float h = _m22*rect.height();
        if (w < 0) {
            w = -w;
            x -= w;
        }
        if (h < 0) {
            h = -h;
            y -= h;
        }
        result = QRectF(x, y, w, h);
    } else {
        float x0, y0;
        float x, y;
        MAPDOUBLE(rect.x(), rect.y(), x0, y0);
        float xmin = x0;
        float ymin = y0;
        float xmax = x0;
        float ymax = y0;
        MAPDOUBLE(rect.x() + rect.width(), rect.y(), x, y);
        xmin = Math::Min(xmin, x);
        ymin = Math::Min(ymin, y);
        xmax = Math::Max(xmax, x);
        ymax = Math::Max(ymax, y);
        MAPDOUBLE(rect.x() + rect.width(), rect.y() + rect.height(), x, y);
        xmin = Math::Min(xmin, x);
        ymin = Math::Min(ymin, y);
        xmax = Math::Max(xmax, x);
        ymax = Math::Max(ymax, y);
        MAPDOUBLE(rect.x(), rect.y() + rect.height(), x, y);
        xmin = Math::Min(xmin, x);
        ymin = Math::Min(ymin, y);
        xmax = Math::Max(xmax, x);
        ymax = Math::Max(ymax, y);
        result = QRectF(xmin, ymin, xmax-xmin, ymax - ymin);
    }
    return result;
}

QPoint QMatrix::map(const QPoint &p) const
{
	float fx = (float)p.x();
	float fy = (float)p.y();
    return QPoint(Math::Round(_m11*fx + _m21*fy + _dx),
                   Math::Round(_m12*fx + _m22*fy + _dy));
}

QPointF QMatrix::map(const QPointF &point) const
{
    float fx = point.x();
    float fy = point.y();
    return QPointF(_m11*fx + _m21*fy + _dx, _m12*fx + _m22*fy + _dy);
}
QLineF QMatrix::map(const QLineF &line) const
{
    return QLineF(map(line.p1()), map(line.p2()));
}

QLine QMatrix::map(const QLine &line) const
{
    return QLine(map(line.p1()), map(line.p2()));
}


void QMatrix::reset()
{
    _m11 = _m22 = 1.0;
    _m12 = _m21 = _dx = _dy = 0.0;
}

QMatrix &QMatrix::translate(float dx, float dy)
{
    _dx += dx*_m11 + dy*_m21;
    _dy += dy*_m22 + dx*_m12;
    return *this;
}

QMatrix &QMatrix::scale(float sx, float sy)
{
    _m11 *= sx;
    _m12 *= sx;
    _m21 *= sy;
    _m22 *= sy;
    return *this;
}

QMatrix &QMatrix::shear(float sh, float sv)
{
    float tm11 = sv*_m21;
    float tm12 = sv*_m22;
    float tm21 = sh*_m11;
    float tm22 = sh*_m12;
    _m11 += tm11;
    _m12 += tm12;
    _m21 += tm21;
    _m22 += tm22;
    return *this;
}

const float deg2rad = float(0.017453292519943295769);        // pi/180

QMatrix &QMatrix::rotate(float a)
{
    float sina = 0;
    float cosa = 0;
    if (a == 90. || a == -270.)
        sina = 1.;
    else if (a == 270. || a == -90.)
        sina = -1.;
    else if (a == 180.)
        cosa = -1.;
    else{
        float b = deg2rad*a;                        // convert to radians
        sina = Math::Sin(b);               // fast and convenient
        cosa = Math::Cos(b);
    }
    float tm11 = cosa*_m11 + sina*_m21;
    float tm12 = cosa*_m12 + sina*_m22;
    float tm21 = -sina*_m11 + cosa*_m21;
    float tm22 = -sina*_m12 + cosa*_m22;
    _m11 = tm11; _m12 = tm12;
    _m21 = tm21; _m22 = tm22;
    return *this;
}

QMatrix QMatrix::inverted(bool *invertible) const
{
    float dtr = determinant();
    if (dtr == 0.0) {
        if (invertible)
            *invertible = false;                // singular matrix
        return QMatrix(true);
    }
    else {                                        // invertible matrix
        if (invertible)
            *invertible = true;
        float dinv = 1.0f/dtr;
        return QMatrix((_m22*dinv),        (-_m12*dinv),
                       (-_m21*dinv), (_m11*dinv),
                       ((_m21*_dy - _m22*_dx)*dinv),
                       ((_m12*_dx - _m11*_dy)*dinv),
                       true);
    }
}

bool QMatrix::operator==(const QMatrix &m) const
{
    return _m11 == m._m11 &&
           _m12 == m._m12 &&
           _m21 == m._m21 &&
           _m22 == m._m22 &&
           _dx == m._dx &&
           _dy == m._dy;
}

bool QMatrix::operator!=(const QMatrix &m) const
{
    return _m11 != m._m11 ||
           _m12 != m._m12 ||
           _m21 != m._m21 ||
           _m22 != m._m22 ||
           _dx != m._dx ||
           _dy != m._dy;
}

QMatrix &QMatrix::operator *=(const QMatrix &m)
{
    float tm11 = _m11*m._m11 + _m12*m._m21;
    float tm12 = _m11*m._m12 + _m12*m._m22;
    float tm21 = _m21*m._m11 + _m22*m._m21;
    float tm22 = _m21*m._m12 + _m22*m._m22;

    float tdx  = _dx*m._m11  + _dy*m._m21 + m._dx;
    float tdy =  _dx*m._m12  + _dy*m._m22 + m._dy;

    _m11 = tm11; _m12 = tm12;
    _m21 = tm21; _m22 = tm22;
    _dx = tdx; _dy = tdy;
    return *this;
}

QMatrix QMatrix::operator *(const QMatrix &m) const
{
    float tm11 = _m11*m._m11 + _m12*m._m21;
    float tm12 = _m11*m._m12 + _m12*m._m22;
    float tm21 = _m21*m._m11 + _m22*m._m21;
    float tm22 = _m21*m._m12 + _m22*m._m22;

    float tdx  = _dx*m._m11  + _dy*m._m21 + m._dx;
    float tdy =  _dx*m._m12  + _dy*m._m22 + m._dy;
    return QMatrix(tm11, tm12, tm21, tm22, tdx, tdy, true);
}


QMatrix &QMatrix::operator=(const QMatrix &matrix)
{
    _m11 = matrix._m11;
    _m12 = matrix._m12;
    _m21 = matrix._m21;
    _m22 = matrix._m22;
    _dx  = matrix._dx;
    _dy  = matrix._dy;
    return *this;
}

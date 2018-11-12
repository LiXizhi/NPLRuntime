/** This file is part of the Pinocchio automatic rigging library.
*  Copyright (C) 2007 Ilya Baran (ibaran@mit.edu)
*
*  This library is an implementation of Automatic Rigging and Animation of 3D Characters,
*	Ilya Baran∗ Jovan Popovic´†, 2007.
*
*/

#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "pvector.h"

template<class Real = double>
class Quaternion //normalized quaternion for representing rotations
{
public:
    //constructors
    Quaternion() : m_r(1.) { } //initialize to identity
    Quaternion(const Quaternion &q) : m_r(q.m_r), m_v(q.m_v) {} //copy constructor
    template<class R> Quaternion(const Quaternion<R> &q) : m_r(q.m_r), m_v(q.m_v) {} //convert quaternions of other types
    //axis angle constructor:
    template<class R> Quaternion(const PVector<R, 3> &axis, const R &angle) : m_r(cos(angle * Real(0.5))), m_v(sin(angle * Real(0.5)) * axis.normalize()) {}
    //minimum rotation constructor:
    template<class R> Quaternion(const PVector<R, 3> &from, const PVector<R, 3> &to) : m_r(1.)
    {
        R fromLenSq = from.lengthsq(), toLenSq = to.lengthsq();
        if(fromLenSq < toLenSq) {
            if(fromLenSq < R(1e-16))
                return;
            PVector<R, 3> mid = from * sqrt(toLenSq / fromLenSq) + to;
            R fac = 1. / sqrt(mid.lengthsq() * toLenSq);
            m_r = (mid * to) * fac;
            m_v = (mid % to) * fac;
        }
        else {
            if(toLenSq < R(1e-16))
                return;
            PVector<R, 3> mid = from + to * sqrt(fromLenSq / toLenSq);
            R fac = 1. / sqrt(mid.lengthsq() * fromLenSq);
            m_r = (from * mid) * fac;
            m_v = (from % mid) * fac;
        }
    }

    //quaternion multiplication
    Quaternion operator*(const Quaternion &q) const { return Quaternion(m_r * q.m_r - m_v * q.m_v, m_r * q.m_v + q.m_r * m_v + m_v % q.m_v); }
    
    //transforming a vector
    PVector<Real, 3> operator*(const PVector<Real, 3> &p) const
    {
        PVector<Real, 3> v2 = m_v + m_v;
        PVector<Real, 3> vsq2 = m_v.apply(multiplies<Real>(), v2);
        PVector<Real, 3> rv2 = m_r * v2;
        PVector<Real, 3> vv2(m_v[1] * v2[2], m_v[0] * v2[2], m_v[0] * v2[1]);
        return PVector<Real, 3>(p[0] * (Real(1.) - vsq2[1] - vsq2[2]) + p[1] * (vv2[2] - rv2[2]) + p[2] * (vv2[1] + rv2[1]),
                               p[1] * (Real(1.) - vsq2[2] - vsq2[0]) + p[2] * (vv2[0] - rv2[0]) + p[0] * (vv2[2] + rv2[2]),
                               p[2] * (Real(1.) - vsq2[0] - vsq2[1]) + p[0] * (vv2[1] - rv2[1]) + p[1] * (vv2[0] + rv2[0]));
    }

    //equality
    template<class R> bool operator==(const Quaternion<R> &oth) const
    {
        return (m_r == oth.m_r && m_v == oth.m_v) || (m_r == -oth.m_r && m_v == -oth.m_v);
    }
    
    Quaternion inverse() const { return Quaternion(-m_r, m_v); }
    
    Real getAngle() const { return Real(2.) * atan2(m_v.length(), m_r); }
    PVector<Real, 3> getAxis() const { return m_v.normalize(); }
    
    const Real &operator[](int i) const { return (i == 0) ? m_r : m_v[i - 1]; }
    void set(const Real &inR, const PVector<Real, 3> &inV) {
        Real ratio = Real(1.) / sqrt(inR * inR + inV.lengthsq()); 
        m_r = inR * ratio; m_v = inV * ratio; //normalize
    }
    
private:
    Quaternion(const Real &inR, const PVector<Real, 3> &inV) : m_r(inR), m_v(inV) {}
    
    Real m_r;
    PVector<Real, 3> m_v;
};

template<class Real = double> class Transform { //T(v) = (rot * v * scale) + trans
public:
    typedef PVector<Real, 3> Vec;
    
    Transform() : m_scale(1.) {}
    explicit Transform(const Real &inScale) : m_scale(inScale) {}
    explicit Transform(const Vec &inTrans) : m_scale(1.), m_trans(inTrans) {}
    Transform(const Quaternion<Real> &inRot, Real inScale = Real(1.), Vec inTrans = Vec()) : m_rot(inRot), m_scale(inScale), m_trans(inTrans) {}
    Transform(const Transform &t) : m_rot(t.m_rot), m_scale(t.m_scale), m_trans(t.m_trans) {}
    
    Transform operator*(const Transform &t) const { return Transform(m_rot * t.m_rot, m_scale * t.m_scale, m_trans + m_rot * (m_scale * t.m_trans)); }
    Vec operator*(const Vec &v) const { return m_rot * (v * m_scale) + m_trans; }
    
    Transform inverse() const { return Transform(m_rot.inverse(), 1. / m_scale, m_rot.inverse() * -m_trans * (1. / m_scale)); }
    
    Transform linearComponent() const { return Transform(m_rot, m_scale); }
    Vec mult3(const Vec &v) const { return m_rot * (v * m_scale); }
    
    Real getScale() const { return m_scale; }
    Vec getTrans() const { return m_trans; }
    Quaternion<Real> getRot() const { return m_rot; }
    
private:
    Quaternion<Real> m_rot;
    Real m_scale;
    Vec m_trans;
};

template<class Real = double> class Matrix3 {
public:
    typedef PVector<Real, 3> Vec;
    typedef Matrix3<Real> Self;
    
    Matrix3(const Real &diag = Real()) { m_m[0] = m_m[4] = m_m[8] = diag; m_m[1] = m_m[2] = m_m[3] = m_m[5] = m_m[6] = m_m[7] = Real(); }
    Matrix3(const Vec &c1, const Vec &c2, const Vec &c3) {
        m_m[0] = c1[0]; m_m[1] = c2[0]; m_m[2] = c3[0];
        m_m[3] = c1[1]; m_m[4] = c2[1]; m_m[5] = c3[1];
        m_m[6] = c1[2]; m_m[7] = c2[2]; m_m[8] = c3[2];
    }
    Matrix3(const Self &inM) { for(int i = 0; i < 9; ++i) m_m[i] = inM[i]; }
    
    Real &operator[](int idx) { return m_m[idx]; }
    const Real &operator[](int idx) const { return m_m[idx]; }
    Real &operator()(int row, int col) { return m_m[row * 3 + col]; }
    const Real &operator()(int row, int col) const { return m_m[row * 3 + col]; }
    
    Vec getRow(int row) const { row *= 3; return Vec(m_m[row], m_m[row + 1], m_m[row + 2]); }
    Vec getColumn(int col) const { return Vec(m_m[col], m_m[col + 3], m_m[col + 6]); }
    
    Self operator+(const Self &o) { Self out(S(0)); for(int i = 0; i < 9; ++i) out[i] = m_m[i] + o[i]; return out; }
    Self operator-(const Self &o) { Self out(S(0)); for(int i = 0; i < 9; ++i) out[i] = m_m[i] - o[i]; return out; }
    Self operator*(const Real &x) { Self out(S(0)); for(int i = 0; i < 9; ++i) out[i] = m_m[i] * x; return out; }
    Self operator/(const Real &x) { Self out(S(0)); for(int i = 0; i < 9; ++i) out[i] = m_m[i] / x; return out; }

#define OPAS(op, typ, idx) Self &operator op(const typ &x) { for(int i = 0; i < 9; ++i) m_m[i] op x idx; return *this; }
    OPAS(+=, Self, [i])
    OPAS(-=, Self, [i])
    OPAS(*=, Real, )
    OPAS(/=, Real, )
#undef OPAS
    
    Vec operator*(const Vec &v) const { 
        return Vec(m_m[0] * v[0] + m_m[1] * v[1] + m_m[2] * v[2],
                   m_m[3] * v[0] + m_m[4] * v[1] + m_m[5] * v[2],
                   m_m[6] * v[0] + m_m[7] * v[1] + m_m[8] * v[2]);
    }
    
    Self operator*(const Self &o) const {
        return Self((*this) * Vec(o[0], o[3], o[6]), (*this) * Vec(o[1], o[4], o[7]), (*this) * Vec(o[2], o[5], o[8]));
    }
    
    Self operator~() const { //transpose
        Self out(S(0)); //uninitialized
        out[0] = m_m[0]; out[4] = m_m[4]; out[8] = m_m[8];
        out[1] = m_m[3]; out[3] = m_m[1]; out[2] = m_m[6]; out[6] = m_m[2]; out[5] = m_m[7]; out[7] = m_m[5];
        return out;
    }
    
    Self operator!() const { //invert
        Self out(S(0));
        Real d = det();
        if(d == Real())
            return Self();
        d = Real(1.) / d;
        out[0] = d * (m_m[4] * m_m[8] - m_m[5] * m_m[7]);
        out[1] = d * (m_m[2] * m_m[7] - m_m[1] * m_m[8]);
        out[2] = d * (m_m[1] * m_m[5] - m_m[2] * m_m[4]);
        out[3] = d * (m_m[5] * m_m[6] - m_m[3] * m_m[8]);
        out[4] = d * (m_m[0] * m_m[8] - m_m[2] * m_m[6]);
        out[5] = d * (m_m[2] * m_m[3] - m_m[0] * m_m[5]);
        out[6] = d * (m_m[3] * m_m[7] - m_m[4] * m_m[6]);
        out[7] = d * (m_m[1] * m_m[6] - m_m[0] * m_m[7]);
        out[8] = d * (m_m[0] * m_m[4] - m_m[1] * m_m[3]);
        return out;
    }
    
    Real det() const
    { return m_m[0] * (m_m[4] * m_m[8] - m_m[5] * m_m[7]) - m_m[1] * (m_m[3] * m_m[8] - m_m[5] * m_m[6]) + m_m[2] * (m_m[3] * m_m[7] - m_m[4] * m_m[6]); }
    
private:
    struct S { S(int) { } };
    Matrix3(const S &) { } //no initialization
    
    Real m_m[9];
};

template <class charT, class traits, class Real>
        basic_ostream<charT,traits>& operator<<(basic_ostream<charT,traits>& os, const Matrix3<Real> &m)
{
    os << "[[" << m[0] << "," << m[1] << "," << m[2] << "]";
    os << "[" << m[3] << "," << m[4] << "," << m[5] << "]";
    os << "[" << m[6] << "," << m[7] << "," << m[8] << "]]";
    return os;
}

#endif

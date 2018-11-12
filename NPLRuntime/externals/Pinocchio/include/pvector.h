/** This file is part of the Pinocchio automatic rigging library.
*  Copyright (C) 2007 Ilya Baran (ibaran@mit.edu)
*
*  This library is an implementation of Automatic Rigging and Animation of 3D Characters,
*	Ilya Baran∗ Jovan Popovic´†, 2007.
*
*/

#ifndef VECTOR_H_INCLUDED
#define VECTOR_H_INCLUDED

#include <iostream>
#include <functional>
#include <vector>
#include <numeric>

#include "hashutils.h"
#include "mathutils.h"

// using namespace in a header file is not recomended, MUST be fixed ！
using namespace std;

namespace _VectorPrivate {
template <int Dim> class VecOp;
}

template <class Real, int Dim>
class PVector
{
public:
    typedef PVector<Real, Dim> Self;
    typedef _VectorPrivate::VecOp<Dim> VO;

    PVector() { VO::assign(Real(), *this); }
    PVector(const Self &other) { VO::assign(other, *this); }
    explicit PVector(const Real &m) { VO::assign(m, *this); }
    PVector(const Real &m1, const Real &m2) { m[0] = m1; m[1] = m2; checkDim<2>(VO()); }
    PVector(const Real &m1, const Real &m2, const Real &m3) { m[0] = m1; m[1] = m2; m[2] = m3; checkDim<3>(VO()); }
    template<class R> PVector(const PVector<R, Dim> &other) { VO::assign(other, *this); }

    Real &operator[](int n) { return m[n]; }
    const Real &operator[](int n) const { return m[n]; }

//basic recursive functions
    template<class F> PVector<typename F::result_type, Dim> apply(const F &func) const
    { return VO::apply(func, *this); }

    template<class F> PVector<typename F::result_type, Dim> apply(const F &func, const Self &other) const
    { return VO::apply(func, *this, other); }

    template<class Op, class Accum>
    typename Accum::result_type accumulate(const Op &op, const Accum &accum) const
    { return VO::accumulate(op, accum, *this); }

    template<class Op, class Accum>
    typename Accum::result_type accumulate(const Op &op, const Accum &accum, const Self &other) const
    { return VO::accumulate(op, accum, *this, other); }

//operators
    Real operator*(const Self &other) const { return accumulate(multiplies<Real>(), plus<Real>(), other); }
    Self operator+(const Self &other) const { return apply(plus<Real>(), other); }
    Self operator-(const Self &other) const { return apply(minus<Real>(), other); }
    Self operator*(const Real &scalar) const { return apply(bind2nd(multiplies<Real>(), scalar)); }
    Self operator/(const Real &scalar) const { return apply(bind2nd(divides<Real>(), scalar)); }
    Self operator-() const { return apply(negate<Real>()); }
    bool operator==(const Self &other) const { return accumulate(equal_to<Real>(), logical_and<Real>(), other); }

#define OPAS(op, typ) Self &operator op##=(const typ &x) { (*this) = (*this) op x; return *this; }
    OPAS(+, Self)
    OPAS(-, Self)
    OPAS(*, Real)
    OPAS(/, Real)
#undef OPAS

    Real lengthsq() const { return (*this) * (*this); }
    Real length() const { return sqrt(lengthsq()); }

    Self normalize() const { return (*this) / length(); }

    int size() const { return Dim; }
    
private:
    template<class R, int D> friend class PVector;
    template<int WantedDim> void checkDim(const _VectorPrivate::VecOp<WantedDim> &) const {}

    Real m[Dim];
};

template <class Real>
class PVector<Real, -1>
{
public:
    typedef PVector<Real, -1> Self;

    PVector() { }
    PVector(const Self &other) : m(other.m) { }
    PVector(const vector<Real> &inM) : m(inM) { }
    explicit PVector(const Real &inM) { m.push_back(inM); }

    Real &operator[](int n) { if((int)m.size() <= n) m.resize(n + 1); return m[n]; }
    const Real &operator[](int n) const { if((int)m.size() <= n) const_cast<PVector<Real, -1> *>(this)->m.resize(n + 1); return m[n]; }

//basic recursive functions
    template<class F> PVector<typename F::result_type, -1> apply(const F &func) const
    { 
        vector<typename F::result_type> out(m.size());
        transform(m.begin(), m.end(), out.begin(), func);
        return PVector<typename F::result_type, -1>(out);
    }

    template<class F> PVector<typename F::result_type, -1> apply(const F &func, const Self &other) const
    {
        vector<typename F::result_type> out(max(m.size(), other.m.size()));
        if(m.size() == other.m.size())
            transform(m.begin(), m.end(), other.m.begin(), out.begin(), func);
        else if(m.size() < other.m.size()) {
            transform(m.begin(), m.end(), other.m.begin(), out.begin(), func);
            for(int i = m.size(); i < (int)other.m.size(); ++i) out[i] = func(Real(), other.m[i]);
        } else {
            transform(m.begin(), m.begin() + (other.m.end() - other.m.begin()), other.m.begin(), out.begin(), func);
            for(int i = other.m.size(); i < (int)m.size(); ++i) out[i] = func(m[i], Real());
        }
        return PVector<typename F::result_type, -1>(out);
    }

    template<class Op, class Accum>
    typename Accum::result_type accumulate(const Op &op, const Accum &accum) const
    {
        if(m.empty())
            return typename Accum::result_type();
        typename Accum::result_type out = op(m[0]);
        for(int i = 1; i < (int)m.size(); ++i) out = accum(out, op(m[i]));
        return out;
    }

    template<class Op, class Accum>
    typename Accum::result_type accumulate(const Op &op, const Accum &accum, const Self &other) const
    { 
        typename Accum::result_type out;
        if(m.empty() || other.m.empty()) {
            if(m.empty() && other.m.empty()) return typename Accum::result_type();
            if(m.empty()) out = op(Real(), other.m[0]);
            else out = op(m[0], Real());
        }
        else out = op(m[0], other.m[0]);
        if(m.size() == other.m.size())
            for(int i = 1; i < (int)m.size(); ++i) out = accum(out, op(m[i], other.m[i]));
        else if(m.size() < other.m.size()) {
            for(int i = 1; i < (int)m.size(); ++i) out = accum(out, op(m[i], other.m[i]));
            for(int i = m.size(); i < (int)other.m.size(); ++i) out = accum(out, op(Real(), other.m[i]));
        } else {
            for(int i = 1; i < (int)other.m.size(); ++i) out = accum(out, op(m[i], other.m[i]));
            for(int i = other.m.size(); i < (int)m.size(); ++i) out = accum(out, op(m[i], Real()));
        }
        return out;
    }

//operators
    Real operator*(const Self &other) const { return accumulate(multiplies<Real>(), plus<Real>(), other); }
    Self operator+(const Self &other) const { return apply(plus<Real>(), other); }
    Self operator-(const Self &other) const { return apply(minus<Real>(), other); }
    Self operator*(const Real &scalar) const { return apply(bind2nd(multiplies<Real>(), scalar)); }
    Self operator/(const Real &scalar) const { return apply(bind2nd(divides<Real>(), scalar)); }
    Self operator-() const { return apply(negate<Real>()); }

#define OPAS(op, typ) Self &operator op##=(const typ &x) { (*this) = (*this) op x; return *this; }
    OPAS(+, Self)
    OPAS(-, Self)
    OPAS(*, Real)
    OPAS(/, Real)
#undef OPAS

    Real lengthsq() const { return (*this) * (*this); }
    Real length() const { return sqrt(lengthsq()); }

    Self normalize() const { return (*this) / length(); }

    int size() const { return m.size(); }
    
private:
    vector<Real> m;
};


template <class Real, int Dim>
PVector<Real, Dim> operator*(const Real &scalar, const PVector<Real, Dim> &vec)
{
    return vec * scalar; //multiplication commutes around here
}

//cross product
template <class Real>
PVector<Real, 3> operator%(const PVector<Real, 3> &v1, const PVector<Real, 3> &v2)
{
    return PVector<Real, 3>(v1[1] * v2[2] - v1[2] * v2[1],
                   v1[2] * v2[0] - v1[0] * v2[2],
                   v1[0] * v2[1] - v1[1] * v2[0]);
}

typedef PVector<double, 3> PVector3;
typedef PVector<double, 2> PVector2;

template <class charT, class traits, class Real, int Dim>
basic_ostream<charT,traits>& operator<<(basic_ostream<charT,traits>& os, const PVector<Real, Dim> &v)
{
    os << "[";
    int ms = Dim;
    if(ms == -1)
        ms = v.size();
    for(int i = 0; i < ms; ++i) {
        os << v[i];
        if(i < ms - 1)
            os << ", ";
    }
    os << "]";
    return os;
}

namespace _VectorPrivate {
#define VRD PVector<R, D>
#define VRD1 PVector<R1, D>
template <int Dim>
class VecOp
{
private:
    static const int last = Dim - 1;
    typedef VecOp<Dim - 1> Next;
    template<int D> friend class VecOp;
    template<class R, int D> friend class PVector;

    template<class R, class R1, int D>
    static void assign(const VRD1 &from, VRD &to) { to[last] = from[last]; Next::assign(from, to); }

    template<class R, class R1, int D>
    static void assign(const R1 &from, VRD &to) { to[last] = from; Next::assign(from, to); }

    template<class R, int D, class F>
    static PVector<typename F::result_type, D> apply(const F &func, const VRD &v)
    { PVector<typename F::result_type, D> out; _apply(func, v, out); return out; }

    template<class R, int D, class F>
    static PVector<typename F::result_type, D> apply(const F &func, const VRD &v, const VRD &other)
    { PVector<typename F::result_type, D> out; _apply(func, v, other, out); return out; }

    template<class R, int D, class Op, class Accum>
    static typename Accum::result_type accumulate(const Op &op, const Accum &accum, const VRD &v)
    { return accum(op(v[last]), Next::accumulate(op, accum, v)); }

    template<class R, int D, class Op, class Accum>
    static typename Accum::result_type accumulate(const Op &op, const Accum &accum, const VRD &v, const VRD &other)
    { return accum(op(v[last], other[last]), Next::accumulate(op, accum, v, other)); }

    template<class R, int D, class F>
    static void _apply(const F &func, const VRD &v, PVector<typename F::result_type, D> &out)
    { out[last] = func(v[last]); Next::_apply(func, v, out); }

    template<class R, int D, class F>
    static void _apply(const F &func, const VRD &v, const VRD &other, PVector<typename F::result_type, D> &out)
    { out[last] = func(v[last], other[last]); Next::_apply(func, v, other, out); }
};

template <>
class VecOp<1>
{
private:
    template<int D> friend class VecOp;

    template<class R, class R1, int D> static void assign(const VRD1 &from, VRD &to) { to[0] = from[0]; }

    template<class R, class R1, int D> static void assign(const R1 &from, VRD &to) { to[0] = from; }
    
    template<class R, int D, class F>
    static PVector<typename F::result_type, D> apply(const F &func, const VRD &v)
    { PVector<typename F::result_type, D> out; _apply(func, v, out); return out; }

    template<class R, int D, class F>
    static PVector<typename F::result_type, D> apply(const F &func, const VRD &v, const VRD &other)
    { PVector<typename F::result_type, D> out; _apply(func, v, other, out); return out; }
    
    template<class R, int D, class Op, class Accum>
    static typename Accum::result_type accumulate(const Op &op, const Accum &, const VRD &v)
    { return op(v[0]); }

    template<class R, int D, class Op, class Accum>
    static typename Accum::result_type accumulate(const Op &op, const Accum &, const VRD &v, const VRD &other)
    { return op(v[0], other[0]); }

    template<class R, int D, class F>
    static void _apply(const F &func, const VRD &v, PVector<typename F::result_type, D> &out)
    { out[0] = func(v[0]); }

    template<class R, int D, class F>
    static void _apply(const F &func, const VRD &v, const VRD &other, PVector<typename F::result_type, D> &out)
    { out[0] = func(v[0], other[0]); }
};
} //namespace _VectorPrivate

//BitComparator is a utility class that helps with rectangle and dtree indices
template<int Dim> class BitComparator
{
public:
    static const int last = Dim - 1;
    typedef BitComparator<Dim - 1> Next;

    template<class R, int D> static unsigned int less(const VRD &v1, const VRD &v2)
    { return ((v1[last] < v2[last]) ? (1 << last) : 0) + Next::less(v1, v2); }

    template<class R, int D> static void assignCorner(int idx, const VRD &v1, const VRD &v2, VRD &out)
    { out[last] = (idx & (1 << last)) ? v1[last] : v2[last]; Next::assignCorner(idx, v1, v2, out); }
};

template<> class BitComparator<1>
{
public:
    template<class R, int D> static unsigned int less(const VRD &v1, const VRD &v2)
    { return (v1[0] < v2[0]) ? 1 : 0; }
    
    template<class R, int D> static void assignCorner(int idx, const VRD &v1, const VRD &v2, VRD &out)
    { out[0] = (idx & 1) ? v1[0] : v2[0];}
};
#undef VRD
#undef VRD1

#endif //VECTOR_H_INCLUDED

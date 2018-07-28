/** This file is part of the Pinocchio automatic rigging library.
*  Copyright (C) 2007 Ilya Baran (ibaran@mit.edu)
*
*  This library is an implementation of Automatic Rigging and Animation of 3D Characters,
*	Ilya Baran∗ Jovan Popovic´†, 2007.
*
*/

#ifndef MATRIX_H_INCLUDED
#define MATRIX_H_INCLUDED

#include <vector>
#include <algorithm>
#include <iostream>
#include <cassert>

#include "mathutils.h"

template<class Real>
class Vectorn : public vector<Real>
{
public:
    typedef Vectorn<Real> Self;
    typedef vector<Real> super;

    Vectorn() {}
    Vectorn(const Vectorn &other) : super(other) {}
    Vectorn(int sz, const Real &init = Real()) : super(sz, init) {}
    
    template<class R> Vectorn(const vector<R> &other) : super(other.begin(), other.end()) {}
    template<class Iter> Vectorn(const Iter &from, const Iter &to) : super(from, to) {}

    using super::size;

    //basic recursive functions
    template<class F> Vectorn<typename F::result_type> apply(const F &func) const
    {
        Vectorn<typename F::result_type> out(size());
        for(int i = 0; i < (int)size(); ++i)
            out[i] = func((*this)[i]);
        return out;
    }

    template<class F> Vectorn<typename F::result_type> apply(const F &func, const Self &other) const
    {
        assert(size() == other.size());
        Vectorn<typename F::result_type> out(size());
        for(int i = 0; i < (int)size(); ++i)
            out[i] = func((*this)[i], other[i]);
        return out;
    }


    template<class Op, class Accum>
    typename Accum::result_type accumulate(const Op &op, const Accum &accum) const
    {
        if(size() == 0)
            return typename Accum::result_type();
        typename Accum::result_type out(op((*this)[0]));
        for(int i = 1; i < (int)size(); ++i)
            out = accum(out, op((*this)[i]));
        return out;
    }

    template<class Op, class Accum>
    typename Accum::result_type accumulate(const Op &op, const Accum &accum, const Self &other) const
    {
        if(size() == 0)
            return typename Accum::result_type();
        assert(size() == other.size());
        typename Accum::result_type out(op((*this)[0], other[0]));
        for(int i = 1; i < (int)size(); ++i)
            out = accum(out, op((*this)[i], other[i]));
        return out;
    }

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

    Real sum() const { return accumulate(ident<Real>(), plus<Real>()); }

    Self normalize() const { return (*this) / length(); }

};

template<class Real>
inline ostream &operator<<(ostream &os, const Vectorn<Real> &v)
{
    os << "[";
    for(int i = 0; i < (int)v.size(); ++i) {
        os << v[i];
        if(i < (int)v.size() - 1)
            os << " ";
    }
    os << "]";
    return os;
}

template<class Real>
class Matrixn
{
public:
    typedef Matrixn<Real> Self;
    typedef Vectorn<Real> Vec;

    Matrixn() {}
    Matrixn(int rows, int cols, const Real &init = Real()) : m(rows, Vec(cols, init)) {}
    static Self identity(int sz, const Real &diag = Real(1.))
    {
        Self out(sz, sz);
        for(int i = 0; i < sz; ++i)
            out[i][i] = diag;
        return out;
    }

    static Self identity(const Vec &diag)
    {
        Self out(diag.size(), diag.size());
        for(int i = 0; i < (int)diag.size(); ++i)
            out[i][i] = diag[i];
        return out;
    }

    const Real &operator()(int row, int col) const { return m[row][col]; }
    Real &operator()(int row, int col) { return m[row][col]; }

    const Vec &operator[](int row) const { return m[row]; }
    Vec &operator[](int row) { return m[row]; }

    int getRows() const { return m.size(); }
    int getCols() const { return m[0].size(); }

    Real sum() const { return m.sum().sum(); }

    Self operator+(const Self &oth) const { return Self(m + oth.m); }
    Self operator-(const Self &oth) const { return Self(m - oth.m); }
    Self operator*(double oth) const { return Self(m.apply(bind2nd(myMult<Vec, Real, Vec>(), oth))); }
    Self operator/(double oth) const { return Self(m.apply(bind2nd(myDiv<Vec, Real, Vec>(), oth))); }
    Self operator-() const { return Self(-m); }

    Vec operator*(const Vec &oth) const { return m.apply(bind2nd(myMult<Vec, Vec, Real>(), oth)); }

    Self operator*(const Self &oth) const
    {
        assert(getCols() == oth.getRows());
        Self out(getRows(), oth.getCols());
        for(int i = 0; i < getRows(); ++i) for(int j = 0; j < oth.getCols(); ++j) {
            out[i][j] = m[i] * oth.getColumn(j);
        }
        return out;
    }

    Self operator~() const //transpose
    {
        Self out(getCols(), getRows());
        for(int i = 0; i < out.getRows(); ++i)
            out[i] = getColumn(i);
        return out;
    }

    Self operator!() const //invert
    {
        assert(getRows() == getCols());

        Self out = identity(m.size());
        Self tmp = *this;
        
        int i, j;
        for(i = 0; i < getRows(); ++i) {
            //find pivot
            int pivot = -1;
            double biggestSoFar = -1.;
            for(j = i; j < getRows(); ++j) {
                double cur = myabs(tmp.m[j][i]);
                if(cur > biggestSoFar) {
                    biggestSoFar = cur;
                    pivot = j;
                }
            }
            
            assert(biggestSoFar > 1e-10); //nonsingular
            if(biggestSoFar <= 1e-10)
                return out; //whatever
            
            //move pivot to the right place
            std::swap(tmp.m[i], tmp.m[pivot]);
            std::swap(out.m[i], out.m[pivot]);
            
            double cur = tmp.m[i][i];
            
            //divide row by element
            tmp.m[i] /= cur;
            out.m[i] /= cur;
            
            //now subtract something times this row from other rows
            for(j = 0; j < getRows(); ++j) {
                if(j == i)
                    continue;
                
                double cur = tmp.m[j][i];
                tmp.m[j] -= tmp.m[i] * cur;
                out.m[j] -= out.m[i] * cur;
            }
        }
        
        return out;
    }

    Vec getColumn(int idx) const
    {
        assert(idx >= 0 && idx < getCols());
        Vec out(getRows());
        for(int i = 0; i < getRows(); ++i)
            out[i] = m[i][idx];
        return out;
    }

    Real det() const
    {
        Real out = 1.;

        Self tmp = *this;
        int i, j;
        for(i = 0; i < (int)m.size(); ++i) {
            //find pivot
            int pivot = -1;
            double biggestSoFar = -1.;
            for(j = i; j < (int)m.size(); ++j) {
                double cur = myabs(tmp.m[j][i]);
                if(cur > biggestSoFar) {
                    biggestSoFar = cur;
                    pivot = j;
                }
            }

            if(biggestSoFar <= 1e-10)
                return Real(0.); //singular

            //move pivot to the right place
            std::swap(tmp.m[i], tmp.m[pivot]);
            if(pivot != i)
                out = -out;

            Real cur = tmp.m[i][i];

            //multiply determinant by element
            out *= cur;

            //now subtract something times this row from other rows
            for(j = i + 1; j < (int)m.size(); ++j) {
                Real fact = tmp.m[j][i] / cur;
                for(int k = i + 1; k < (int)m.size(); ++k) {
                    tmp.m[j][k] -= fact * tmp.m[i][k];
                }
            }
        }

        return out;
    }

private:
    static double myabs(double d) { return fabs(d); }

    Matrixn(const Vectorn<Vectorn<Real> > &inM) : m(inM) {}
    Vectorn<Vectorn<Real> > m;
};

Vectorn<double> getEigensystem(Matrixn<double> m, Matrixn<double> *eigenvectors = NULL);
Vectorn<double> getSVD(const Matrixn<double> &m, Matrixn<double> &u, Matrixn<double> &v);

template<class Real>
inline ostream &operator<<(ostream &os, const Matrixn<Real> &m)
{
    os << "[";
    for(int i = 0; i < (int)m.getRows(); ++i) {
        if(i > 0)
            os << " ";
        os << m[i];
        if(i + 1 < (int)m.getRows())
            os << endl;
    }
    os << "]" << endl;
    return os;
}

#endif //MATRIX_H_INCLUDED

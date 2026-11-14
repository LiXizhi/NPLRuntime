/** This file is part of the Pinocchio automatic rigging library.
*  Copyright (C) 2007 Ilya Baran (ibaran@mit.edu)
*
*  This library is an implementation of Automatic Rigging and Animation of 3D Characters,
*	Ilya Baran∗ Jovan Popovic´†, 2007.
*
*/

#ifndef MATHUTILS_H_INCLUDED
#define MATHUTILS_H_INCLUDED

#include <math.h>
#include <algorithm>
#include <functional>
#include <string>
#include <type_traits>
#include <utility>
#include "Pinocchio.h"

#ifdef _WIN32
#pragma warning(disable:4355)
#pragma warning(disable:4996)
#pragma warning(disable:4251)
#endif //_WIN32

using namespace std;

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

inline int ROUND(double x) { return (int)(x + 0.5); }
inline int SIGN(double x) { return (x > 0.) ? 1 : -1; }
template<class T> T SQR(const T & x) { return x * x; }
template<class T> T CUBE(const T & x) { return x * x * x; }
template<class T> T QUAD(const T & x) { return SQR(SQR(x)); }

template <class Op, class Arg2>
class PinocchioBinder2nd
{
public:
    PinocchioBinder2nd(const Op &inOp, const Arg2 &inArg2) : op(inOp), arg2(inArg2) {}
    template <class Arg1>
    auto operator()(const Arg1 &arg1) const -> typename std::decay<decltype(std::declval<const Op&>()(std::declval<const Arg1&>(), std::declval<const Arg2&>()))>::type
    {
        return op(arg1, arg2);
    }
private:
    Op op;
    Arg2 arg2;
};

template <class Op, class Arg2>
inline PinocchioBinder2nd<Op, Arg2> pinocchio_bind2nd(const Op &op, const Arg2 &arg2)
{
    return PinocchioBinder2nd<Op, Arg2>(op, arg2);
}

template <class F, class Arg>
using pinocchio_unary_result_t = typename std::decay<decltype(std::declval<const F&>()(std::declval<const Arg&>()))>::type;

template <class F, class A1, class A2>
using pinocchio_binary_result_t = typename std::decay<decltype(std::declval<const F&>()(std::declval<const A1&>(), std::declval<const A2&>()))>::type;

template <class T> class maximum
{
public:
    typedef T result_type;
    T operator()(const T &a1, const T &a2) const { return (std::max)(a1, a2); }
};

template <class T> class minimum
{
public:
    typedef T result_type;
    T operator()(const T &a1, const T &a2) const { return (std::min)(a1, a2); }
};

template <class A1, class A2, class R> class myMult
{
public:
    typedef A1 first_argument_type;
    typedef A2 second_argument_type;
    typedef R result_type;
    R operator()(const A1 &a1, const A2 &a2) const { return a1 * a2; }
};

template <class A1, class A2, class R> class myDiv
{
public:
    typedef A1 first_argument_type;
    typedef A2 second_argument_type;
    typedef R result_type;
    R operator()(const A1 &a1, const A2 &a2) const { return a1 / a2; }
};

template <class T> class ident
{
public:
    typedef T argument_type;
    typedef T result_type;
    T operator()(const T &v) const { return v; }
};

template<class T> class construct
{
public:
    typedef T result_type;
    template<class From> T operator()(const From &x) const { return T(x); }
};

#if defined(_WIN32) && defined(_MSC_VER) && (_MSC_VER < 1600)
inline bool operator==(const string &s1, const string &s2) { return s1.compare(s2) == 0; }
inline bool operator<(const string &s1, const string &s2) { return s1.compare(s2) < 0; }
#endif

#endif //MATHUTILS_H_INCLUDED

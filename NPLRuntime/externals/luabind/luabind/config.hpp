// Copyright (c) 2003 Daniel Wallin and Arvid Norberg

// Permission is hereby granted, free of charge, to any person obtaining a
// copy of this software and associated documentation files (the "Software"),
// to deal in the Software without restriction, including without limitation
// the rights to use, copy, modify, merge, publish, distribute, sublicense,
// and/or sell copies of the Software, and to permit persons to whom the
// Software is furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included
// in all copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF
// ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED
// TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A
// PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT
// SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR
// ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN
// ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE
// OR OTHER DEALINGS IN THE SOFTWARE.


#ifndef LUABIND_CONFIG_HPP_INCLUDED
#define LUABIND_CONFIG_HPP_INCLUDED

#include <boost/config.hpp>

#include <cstddef>
#include <cstdint>
#include <cstdlib>

#ifdef BOOST_MSVC
	#define LUABIND_ANONYMOUS_FIX static
#else
	#define LUABIND_ANONYMOUS_FIX
#endif

#if defined (BOOST_MSVC) && (BOOST_MSVC <= 1200)

#define for if (false) {} else for

#include <cstring>

namespace std
{
	using ::strlen;
	using ::strcmp;
	using ::type_info;
}

#endif


#if defined (BOOST_MSVC) && (BOOST_MSVC <= 1300)
	#define LUABIND_MSVC_TYPENAME
#else
	#define LUABIND_MSVC_TYPENAME typename
#endif

// the maximum number of arguments of functions that's
// registered. Must at least be 2
#ifndef LUABIND_MAX_ARITY
	#define LUABIND_MAX_ARITY 10
#elif LUABIND_MAX_ARITY <= 1
	#undef LUABIND_MAX_ARITY
	#define LUABIND_MAX_ARITY 2
#endif

// the maximum number of classes one class
// can derive from
// max bases must at least be 1
#ifndef LUABIND_MAX_BASES
	#define LUABIND_MAX_BASES 4
#elif LUABIND_MAX_BASES <= 0
	#undef LUABIND_MAX_BASES
	#define LUABIND_MAX_BASES 1
#endif

// Branch prediction hints
#if defined(__GNUC__) || defined(__clang__)
	#define LUABIND_LIKELY(x)   __builtin_expect(!!(x), 1)
	#define LUABIND_UNLIKELY(x) __builtin_expect(!!(x), 0)
	#define LUABIND_NOINLINE    __attribute__((noinline))
#elif defined(_MSC_VER)
	#define LUABIND_LIKELY(x)   (x)
	#define LUABIND_UNLIKELY(x) (x)
	#define LUABIND_NOINLINE    __declspec(noinline)
#else
	#define LUABIND_LIKELY(x)   (x)
	#define LUABIND_UNLIKELY(x) (x)
	#define LUABIND_NOINLINE
#endif

// Internal template symbol visibility (hides pure-code registration/thunk templates
// that carry no static ODR state, shrinking symbol & string tables dramatically).
#if !defined(LUABIND_DYNAMIC_LINK) && (defined(__GNUC__) || defined(__clang__))
	#define LUABIND_DETAIL_HIDDEN __attribute__((visibility("hidden")))
#else
	#define LUABIND_DETAIL_HIDDEN
#endif

#ifdef LUABIND_DYNAMIC_LINK
# ifdef BOOST_WINDOWS
#  ifdef LUABIND_BUILDING
#   define LUABIND_API __declspec(dllexport)
#  else
#   define LUABIND_API __declspec(dllimport)
#  endif
# else
#  if defined(__GNUC__) && __GNUC__ >= 4
#   define LUABIND_API __attribute__ ((visibility("default")))
#  endif
# endif
#endif

#ifndef LUABIND_API
# define LUABIND_API
#endif

namespace luabind {

LUABIND_API void disable_super_deprecation();

} // namespace luabind

#endif // LUABIND_CONFIG_HPP_INCLUDED

// Copyright Daniel Wallin 2008. Use, modification and distribution is
// subject to the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#if !BOOST_PP_IS_ITERATING

# ifndef LUABIND_CALL2_080911_HPP
#  define LUABIND_CALL2_080911_HPP

#  include <boost/mpl/apply_wrap.hpp>
#  include <boost/mpl/begin_end.hpp>
#  include <boost/mpl/deref.hpp>
#  include <boost/mpl/front.hpp>
#  include <boost/mpl/long.hpp>
#  include <boost/mpl/size.hpp>
#  include <boost/preprocessor/control/if.hpp>
#  include <boost/preprocessor/iteration/iterate.hpp>
#  include <boost/preprocessor/iteration/local.hpp>
#  include <boost/preprocessor/repetition/enum.hpp>
#  include <boost/preprocessor/repetition/enum_trailing_params.hpp>
#  include <boost/type_traits/is_member_function_pointer.hpp>
#  include <boost/type_traits/is_same.hpp>
#  include <boost/type_traits/is_void.hpp>

#  include <luabind/config.hpp>
#  include <luabind/detail/policy.hpp>
#  include <luabind/yield_policy.hpp>

namespace luabind { namespace detail {

struct invoke_context;

// Non-virtual function_object: eliminates 1,900+ vtables, RTTI typeinfo structs,
// 250-byte typeinfo name strings, and virtual destructors across all bound functions.
struct LUABIND_API function_object
{
    // If score_out == 0: matches args and executes immediately, returning nresults (>= 0) or -1 on mismatch.
    // If score_out != 0: computes match score into *score_out (>= 0 or -1) without executing.
    typedef int (*call_fn_t)(lua_State* L, function_object const* self, int nargs, int* score_out);
    typedef void (*format_fn_t)(lua_State* L, char const* function);
    typedef void (*destroy_fn_t)(function_object* self);

    function_object(
        lua_CFunction entry_
      , call_fn_t call_fn_
      , format_fn_t format_fn_
      , destroy_fn_t destroy_fn_
      , int arity_
    ) noexcept
      : entry(entry_)
      , call_fn(call_fn_)
      , format_fn(format_fn_)
      , destroy_fn(destroy_fn_)
      , next(0)
      , arity(arity_)
      , has_same_arity_overload(false)
    {}

    static void default_destroy(function_object* self) noexcept
    {
        self->~function_object();
        ::operator delete(self);
    }

    template <class Derived>
    static void typed_destroy(function_object* self) noexcept
    {
        delete static_cast<Derived*>(self);
    }

    lua_CFunction entry;
    call_fn_t call_fn;
    format_fn_t format_fn;
    destroy_fn_t destroy_fn;
    function_object* next;
    int arity;
    bool has_same_arity_overload;
    std::string name;
    object keepalive;
};

LUABIND_API int luabind_function_entry(lua_State* L);
LUABIND_API int invoke_function_object(lua_State* L, function_object const* impl, int nargs);

inline int maybe_yield_aux(lua_State*, int results, mpl::false_) noexcept
{
    return results;
}

inline int maybe_yield_aux(lua_State* L, int results, mpl::true_)
{
    return lua_yield(L, results);
}

template <class Policies>
inline int maybe_yield(lua_State* L, int results, Policies*)
{
    return maybe_yield_aux(
        L, results, mpl::bool_<has_yield<Policies>::value>());
}

#  define LUABIND_INVOKE_NEXT_ITER(n) \
    typename mpl::next< \
        BOOST_PP_IF( \
            n, BOOST_PP_CAT(iter,BOOST_PP_DEC(n)), first) \
    >::type

#  define LUABIND_INVOKE_NEXT_INDEX(n) \
    BOOST_PP_IF( \
        n \
      , BOOST_PP_CAT(index,BOOST_PP_DEC(n)) + \
            BOOST_PP_CAT(c,BOOST_PP_DEC(n)).consumed_args() \
      , 1 \
    )

#  define LUABIND_INVOKE_COMPUTE_ARITY(n) + BOOST_PP_CAT(c,n).consumed_args()

#  define LUABIND_INVOKE_DECLARE_CONVERTER(n) \
    typedef LUABIND_INVOKE_NEXT_ITER(n) BOOST_PP_CAT(iter,n); \
    typedef typename mpl::deref<BOOST_PP_CAT(iter,n)>::type \
        BOOST_PP_CAT(a,n); \
    typedef typename find_conversion_policy<n + 1, Policies>::type \
        BOOST_PP_CAT(p,n); \
    typename mpl::apply_wrap2< \
        BOOST_PP_CAT(p,n), BOOST_PP_CAT(a,n), lua_to_cpp>::type BOOST_PP_CAT(c,n); \
    int const BOOST_PP_CAT(index,n) = LUABIND_INVOKE_NEXT_INDEX(n);

// Short-circuiting argument score accumulation: exits immediately on first mismatch!
#  define LUABIND_INVOKE_MATCH_SHORTCIRCUIT(n) \
    { \
        int const s_ = BOOST_PP_CAT(c,n).match( \
            L, LUABIND_DECORATE_TYPE(BOOST_PP_CAT(a,n)), BOOST_PP_CAT(index,n)); \
        if (LUABIND_UNLIKELY(s_ < 0)) \
        { \
            if (score_out) *score_out = -1; \
            return -1; \
        } \
        total_score += s_; \
    }

#  define LUABIND_INVOKE_ARG(z, n, base) \
    BOOST_PP_CAT(c,base(n)).apply( \
        L, LUABIND_DECORATE_TYPE(BOOST_PP_CAT(a,base(n))), BOOST_PP_CAT(index,base(n)))

#  define LUABIND_INVOKE_CONVERTER_POSTCALL(n) \
    BOOST_PP_CAT(c,n).converter_postcall( \
        L, LUABIND_DECORATE_TYPE(BOOST_PP_CAT(a,n)), BOOST_PP_CAT(index,n));

// Compile-time arity helper
template <class Signature, class Policies, long N>
struct arity_calculator;

#  define LUABIND_DECLARE_ARITY_CALC(z, n, _) \
template <class Signature, class Policies> \
struct arity_calculator<Signature, Policies, n> \
{ \
    static int compute() noexcept \
    { \
        typedef typename mpl::begin<Signature>::type first; \
        (void)sizeof(first); \
        return compute_impl(); \
    } \
    static int compute_impl() noexcept; \
};

#  define BOOST_PP_ITERATION_PARAMS_1 \
    (3, (0, LUABIND_MAX_ARITY, <luabind/detail/call.hpp>))
#  include BOOST_PP_ITERATE()

#  define LUABIND_INVOKE_VOID
#  define BOOST_PP_ITERATION_PARAMS_1 \
    (3, (0, LUABIND_MAX_ARITY, <luabind/detail/call.hpp>))
#  include BOOST_PP_ITERATE()

#  undef LUABIND_INVOKE_VOID
#  define LUABIND_INVOKE_MEMBER
#  define BOOST_PP_ITERATION_PARAMS_1 \
    (3, (0, LUABIND_MAX_ARITY, <luabind/detail/call.hpp>))
#  include BOOST_PP_ITERATE()

#  define LUABIND_INVOKE_VOID
#  define BOOST_PP_ITERATION_PARAMS_1 \
    (3, (0, LUABIND_MAX_ARITY, <luabind/detail/call.hpp>))
#  include BOOST_PP_ITERATE()
#  undef LUABIND_INVOKE_MEMBER
#  undef LUABIND_INVOKE_VOID

template <class F, class Signature, class Policies, class IsVoid>
inline int invoke_fast0(
    lua_State* L, int nargs, int* score_out
  , F const& f, Signature, Policies const& policies, IsVoid, mpl::true_)
{
    return invoke_member_fast(
        L, nargs, score_out, f, Signature(), policies
      , mpl::long_<mpl::size<Signature>::value - 1>(), IsVoid()
    );
}

template <class F, class Signature, class Policies, class IsVoid>
inline int invoke_fast0(
    lua_State* L, int nargs, int* score_out
  , F const& f, Signature, Policies const& policies, IsVoid, mpl::false_)
{
    return invoke_normal_fast(
        L, nargs, score_out, f, Signature(), policies
      , mpl::long_<mpl::size<Signature>::value - 1>(), IsVoid()
    );
}

template <class F, class Signature, class Policies>
inline int invoke_fast(
    lua_State* L, int nargs, int* score_out
  , F const& f, Signature, Policies const& policies)
{
    return invoke_fast0(
        L, nargs, score_out, f, Signature(), policies
      , boost::is_void<typename mpl::front<Signature>::type>()
      , boost::is_member_function_pointer<F>()
    );
}

template <class Signature, class Policies>
inline int compute_signature_arity(Signature, Policies const&) noexcept
{
    return compute_arity_n(
        Signature(), (Policies const*)0
      , mpl::long_<mpl::size<Signature>::value - 1>()
    );
}

}} // namespace luabind::detail

# endif // LUABIND_CALL2_080911_HPP

#else // BOOST_PP_IS_ITERATING

# ifdef LUABIND_INVOKE_MEMBER
#  define N BOOST_PP_INC(BOOST_PP_ITERATION())
# else
#  define N BOOST_PP_ITERATION()
# endif

# if !defined(LUABIND_INVOKE_MEMBER) && !defined(LUABIND_INVOKE_VOID)
template <class Signature, class Policies>
inline int compute_arity_n(Signature, Policies const*, mpl::long_<N>) noexcept
{
#  if N > 0
    typedef typename mpl::begin<Signature>::type first;
#   define BOOST_PP_LOCAL_MACRO(n) LUABIND_INVOKE_DECLARE_CONVERTER(n)
#   define BOOST_PP_LOCAL_LIMITS (0,N-1)
#   include BOOST_PP_LOCAL_ITERATE()
    (void)index0;
#  endif
    return 0
#  if N > 0
#   define BOOST_PP_LOCAL_MACRO(n) LUABIND_INVOKE_COMPUTE_ARITY(n)
#   define BOOST_PP_LOCAL_LIMITS (0,N-1)
#   include BOOST_PP_LOCAL_ITERATE()
#  endif
    ;
}
#  if N == LUABIND_MAX_ARITY
// Also cover member functions with LUABIND_MAX_ARITY explicit args (N + 1 total in Signature)
template <class Signature, class Policies>
inline int compute_arity_n(Signature, Policies const*, mpl::long_<LUABIND_MAX_ARITY + 1>) noexcept
{
    typedef typename mpl::begin<Signature>::type first;
#   define BOOST_PP_LOCAL_MACRO(n) LUABIND_INVOKE_DECLARE_CONVERTER(n)
#   define BOOST_PP_LOCAL_LIMITS (0,LUABIND_MAX_ARITY)
#   include BOOST_PP_LOCAL_ITERATE()
    (void)index0;
    return 0
#   define BOOST_PP_LOCAL_MACRO(n) LUABIND_INVOKE_COMPUTE_ARITY(n)
#   define BOOST_PP_LOCAL_LIMITS (0,LUABIND_MAX_ARITY)
#   include BOOST_PP_LOCAL_ITERATE()
    ;
}
#  endif
# endif

template <class F, class Signature, class Policies>
LUABIND_DETAIL_HIDDEN inline int
# ifdef LUABIND_INVOKE_MEMBER
invoke_member_fast
# else
invoke_normal_fast
# endif
(
    lua_State* L, int arguments, int* score_out
  , F const& f, Signature, Policies const&, mpl::long_<N>
# ifdef LUABIND_INVOKE_VOID
  , mpl::true_
# else
  , mpl::false_
# endif
)
{
    typedef typename mpl::begin<Signature>::type first;
# ifndef LUABIND_INVOKE_VOID
    typedef typename mpl::deref<first>::type result_type;
    typedef typename find_conversion_policy<0, Policies>::type result_policy;
    typename mpl::apply_wrap2<
        result_policy, result_type, cpp_to_lua>::type result_converter;
# endif

# if N > 0
#  define BOOST_PP_LOCAL_MACRO(n) LUABIND_INVOKE_DECLARE_CONVERTER(n)
#  define BOOST_PP_LOCAL_LIMITS (0,N-1)
#  include BOOST_PP_LOCAL_ITERATE()
# else
    (void)sizeof(first);
# endif

    int const arity = 0
# if N > 0
#  define BOOST_PP_LOCAL_MACRO(n) LUABIND_INVOKE_COMPUTE_ARITY(n)
#  define BOOST_PP_LOCAL_LIMITS (0,N-1)
#  include BOOST_PP_LOCAL_ITERATE()
# endif
    ;

    if (LUABIND_UNLIKELY(arity != arguments))
    {
        if (score_out) *score_out = -1;
        return -1;
    }

    int total_score = 0;
# if N > 0
#  define BOOST_PP_LOCAL_MACRO(n) LUABIND_INVOKE_MATCH_SHORTCIRCUIT(n)
#  define BOOST_PP_LOCAL_LIMITS (0,N-1)
#  include BOOST_PP_LOCAL_ITERATE()
# endif

    if (score_out)
    {
        *score_out = total_score;
        return 0;
    }

# ifndef LUABIND_INVOKE_VOID
    result_converter.apply(
        L,
# endif
# ifdef LUABIND_INVOKE_MEMBER
        (c0.apply(L, LUABIND_DECORATE_TYPE(a0), index0).*f)(
            BOOST_PP_ENUM(BOOST_PP_DEC(N), LUABIND_INVOKE_ARG, BOOST_PP_INC)
        )
# else
#  define LUABIND_INVOKE_IDENTITY(x) x
        f(
            BOOST_PP_ENUM(N, LUABIND_INVOKE_ARG, LUABIND_INVOKE_IDENTITY)
        )
#  undef LUABIND_INVOKE_IDENTITY
# endif
# ifndef LUABIND_INVOKE_VOID
    )
# endif
    ;

    // Fast compile-time elision when Policies is null_type (93%+ of all bindings):
    // skips converter_postcall, maybe_yield, indices[], and policy_list_postcall,
    // while still honoring C++ void functions (e.g. CNPL::load) that push results onto L!
    if (boost::is_same<Policies, null_type>::value)
    {
        int const nres = lua_gettop(L) - arguments;
        return LUABIND_LIKELY(nres >= 0) ? nres : 0;
    }

# if N > 0
#  define BOOST_PP_LOCAL_MACRO(n) LUABIND_INVOKE_CONVERTER_POSTCALL(n)
#  define BOOST_PP_LOCAL_LIMITS (0,N-1)
#  include BOOST_PP_LOCAL_ITERATE()
# endif

    int const raw_res = lua_gettop(L) - arguments;
    int const results = maybe_yield(L, LUABIND_LIKELY(raw_res >= 0) ? raw_res : 0, (Policies*)0);

    int const indices[] = {
        arguments + results BOOST_PP_ENUM_TRAILING_PARAMS(N, index)
    };

    policy_list_postcall<Policies>::apply(L, indices);

    return results;
}

# undef N

#endif

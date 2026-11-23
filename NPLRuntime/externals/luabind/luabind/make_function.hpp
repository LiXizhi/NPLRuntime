// Copyright Daniel Wallin 2008. Use, modification and distribution is
// subject to the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef LUABIND_MAKE_FUNCTION_081014_HPP
# define LUABIND_MAKE_FUNCTION_081014_HPP

# include <type_traits>
# include <luabind/config.hpp>
# include <luabind/object.hpp>
# include <luabind/detail/call.hpp>
# include <luabind/detail/compute_score.hpp>
# include <luabind/detail/deduce_signature.hpp>
# include <luabind/detail/format_signature.hpp>

namespace luabind {

namespace detail
{
# ifndef LUABIND_NO_EXCEPTIONS
  LUABIND_API void handle_exception_aux(lua_State* L);
# endif

// MSVC complains about member being sensitive to alignment (C4121)
// when F is a pointer to member of a class with virtual bases.
# ifdef BOOST_MSVC
#  pragma pack(push)
#  pragma pack(16)
# endif

  template <class F, class Signature, class Policies>
  struct LUABIND_DETAIL_HIDDEN function_object_impl : function_object
  {
      static constexpr bool kTrivialFields =
          std::is_trivially_destructible<F>::value
          && std::is_trivially_destructible<Policies>::value;

      function_object_impl(F f_, Policies const& policies_)
        : function_object(
              &luabind_function_entry
            , &function_object_impl::call_thunk
            , &function_object_impl::format_thunk
            , kTrivialFields
                ? &function_object::default_destroy
                : &function_object::typed_destroy<function_object_impl>
            , compute_signature_arity(Signature(), policies_)
          )
        , f(f_)
        , policies(policies_)
      {}

      static int call_thunk(
          lua_State* L, function_object const* base, int nargs, int* score_out)
      {
          function_object_impl const* self =
              static_cast<function_object_impl const*>(base);
          return invoke_fast(L, nargs, score_out, self->f, Signature(), self->policies);
      }

      static void format_thunk(lua_State* L, char const* function)
      {
          detail::format_signature(L, function, Signature());
      }

      F f;
      Policies policies;
  };

# ifdef BOOST_MSVC
#  pragma pack(pop)
# endif

  LUABIND_API object make_function_aux(
      lua_State* L, function_object* impl
  );

  LUABIND_API void add_overload(object const&, char const*, object const&);

  // Directly registers make_function + add_overload onto the table at stack top (-1)
  // without creating intermediate luabind::object registry references.
  LUABIND_API void register_function_on_stack_top(
      lua_State* L, char const* name, function_object* impl
  );

} // namespace detail

template <class F, class Signature, class Policies>
object make_function(lua_State* L, F f, Signature, Policies)
{
    return detail::make_function_aux(
        L
      , new detail::function_object_impl<F, Signature, Policies>(
            f, Policies()
        )
    );
}

template <class F>
object make_function(lua_State* L, F f)
{
    return make_function(L, f, detail::deduce_signature(f), detail::null_type());
}

} // namespace luabind

#endif // LUABIND_MAKE_FUNCTION_081014_HPP

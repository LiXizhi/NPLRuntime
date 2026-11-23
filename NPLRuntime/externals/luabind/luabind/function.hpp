// Copyright Daniel Wallin 2008. Use, modification and distribution is
// subject to the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef LUABIND_FUNCTION2_081014_HPP
# define LUABIND_FUNCTION2_081014_HPP

# include <new>
# include <type_traits>
# include <utility>
# include <luabind/make_function.hpp>
# include <luabind/scope.hpp>
# include <luabind/detail/call_function.hpp>

namespace luabind {

namespace detail
{

  template <class F>
  struct deduce_signature_t
  {
      typedef decltype(deduce_signature(std::declval<F>())) type;
  };

  template <class Reg, class... Args>
  inline registration* alloc_reg_nothrow(Args const&... args) noexcept
  {
      void* mem = ::operator new(sizeof(Reg), std::nothrow);
      if (LUABIND_UNLIKELY(!mem))
          std::abort();
      return new (mem) Reg(args...);
  }

  template <class F, class Policies>
  struct LUABIND_DETAIL_HIDDEN function_registration : registration
  {
      static constexpr bool kTrivial =
          std::is_trivially_destructible<F>::value
          && std::is_trivially_destructible<Policies>::value;

      function_registration(char const* name_, F f_, Policies const& policies_) noexcept
        : registration(
              &function_registration::do_register
            , kTrivial ? &registration::default_destroy
                       : &registration::typed_destroy<function_registration>
          )
        , name(name_)
        , f(f_)
        , policies(policies_)
      {}

      static void do_register(lua_State* L, registration const* base)
      {
          function_registration const* self =
              static_cast<function_registration const*>(base);
          typedef typename deduce_signature_t<F>::type sig_type;
          register_function_on_stack_top(
              L
            , self->name
            , new function_object_impl<F, sig_type, Policies>(
                  self->f, self->policies
              )
          );
      }

      char const* name;
      F f;
      Policies policies;
  };

  LUABIND_API bool is_luabind_function(lua_State* L, int index);

} // namespace detail

template <class F, class Policies>
inline scope def(char const* name, F f, Policies const& policies) noexcept
{
    return scope(detail::alloc_reg_nothrow<detail::function_registration<F, Policies> >(
        name, f, policies));
}

template <class F>
inline scope def(char const* name, F f) noexcept
{
    return def(name, f, detail::null_type());
}

} // namespace luabind

#endif // LUABIND_FUNCTION2_081014_HPP

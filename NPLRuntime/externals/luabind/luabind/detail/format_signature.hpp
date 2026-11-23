// Copyright Daniel Wallin 2008. Use, modification and distribution is
// subject to the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#ifndef LUABIND_FORMAT_SIGNATURE_081014_HPP
# define LUABIND_FORMAT_SIGNATURE_081014_HPP

# include <luabind/config.hpp>
# include <luabind/lua_include.hpp>
# include <luabind/typeid.hpp>

# include <boost/mpl/at.hpp>
# include <boost/mpl/begin_end.hpp>
# include <boost/mpl/next.hpp>
# include <boost/mpl/size.hpp>
# include <boost/preprocessor/repetition/enum.hpp>
# include <boost/preprocessor/iteration/local.hpp>
# include <string>

namespace luabind {
    namespace adl
    {
        class object;
        class argument;
        template <class Base>
        struct table;

    } // namespace adl

    using adl::object;
    using adl::argument;
    using adl::table;

} // namespace luabind

namespace luabind { namespace detail {

LUABIND_API std::string get_class_name(lua_State* L, type_id const& i);
LUABIND_API void push_class_name_str(lua_State* L, type_id const& i);
LUABIND_API void format_signature_steps(
    lua_State* L
  , char const* function
  , void (* const* push_fns)(lua_State*)
  , std::size_t count
);

template <class T>
struct type_to_string
{
    static void get(lua_State* L)
    {
        push_class_name_str(L, typeid(T));
    }
};

template <class T>
struct type_to_string<T*>
{
    static void get(lua_State* L)
    {
        type_to_string<T>::get(L);
        lua_pushliteral(L, "*");
        lua_concat(L, 2);
    }
};

template <class T>
struct type_to_string<T&>
{
    static void get(lua_State* L)
    {
        type_to_string<T>::get(L);
        lua_pushliteral(L, "&");
        lua_concat(L, 2);
    }
};

template <class T>
struct type_to_string<T const>
{
    static void get(lua_State* L)
    {
        type_to_string<T>::get(L);
        lua_pushliteral(L, " const");
        lua_concat(L, 2);
    }
};

# define LUABIND_TYPE_TO_STRING(x) \
    template <> \
    struct type_to_string<x> \
    { \
        static void get(lua_State* L) \
        { \
            lua_pushliteral(L, #x); \
        } \
    };

# define LUABIND_INTEGRAL_TYPE_TO_STRING(x) \
    LUABIND_TYPE_TO_STRING(x) \
    LUABIND_TYPE_TO_STRING(unsigned x)

LUABIND_INTEGRAL_TYPE_TO_STRING(char)
LUABIND_INTEGRAL_TYPE_TO_STRING(short)
LUABIND_INTEGRAL_TYPE_TO_STRING(int)
LUABIND_INTEGRAL_TYPE_TO_STRING(long)

LUABIND_TYPE_TO_STRING(float)
LUABIND_TYPE_TO_STRING(double)
LUABIND_TYPE_TO_STRING(void)
LUABIND_TYPE_TO_STRING(bool)
LUABIND_TYPE_TO_STRING(std::string)
LUABIND_TYPE_TO_STRING(lua_State)

LUABIND_TYPE_TO_STRING(luabind::object)
LUABIND_TYPE_TO_STRING(luabind::argument)

# undef LUABIND_INTEGRAL_TYPE_TO_STRING
# undef LUABIND_TYPE_TO_STRING

template <class Base>
struct type_to_string<table<Base> >
{
    static void get(lua_State* L)
    {
        lua_pushliteral(L, "table");
    }
};

template <class Signature, std::size_t Size>
struct signature_formatter;

# define LUABIND_SIG_ELEM(z, i, _) \
    &type_to_string<typename mpl::at_c<Signature, i>::type>::get

# define BOOST_PP_LOCAL_MACRO(k) \
template <class Signature> \
struct LUABIND_DETAIL_HIDDEN signature_formatter<Signature, k> \
{ \
    static void format(lua_State* L, char const* function) \
    { \
        static void (* const fns[k])(lua_State*) = { \
            BOOST_PP_ENUM(k, LUABIND_SIG_ELEM, _) \
        }; \
        format_signature_steps(L, function, fns, k); \
    } \
};

# define BOOST_PP_LOCAL_LIMITS (1, LUABIND_MAX_ARITY + 2)
# include BOOST_PP_LOCAL_ITERATE()
# undef LUABIND_SIG_ELEM

template <class Signature>
inline void format_signature(lua_State* L, char const* function, Signature)
{
    signature_formatter<Signature, mpl::size<Signature>::value>::format(L, function);
}

}} // namespace luabind::detail

#endif // LUABIND_FORMAT_SIGNATURE_081014_HPP

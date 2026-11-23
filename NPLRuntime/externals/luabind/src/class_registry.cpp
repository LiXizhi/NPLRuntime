// Copyright (c) 2004 Daniel Wallin

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

#define LUABIND_BUILDING

#include <luabind/lua_include.hpp>

#include <luabind/luabind.hpp>
#include <luabind/detail/class_registry.hpp>
#include <luabind/detail/class_rep.hpp>
#include <luabind/detail/operator_id.hpp>

namespace luabind { namespace detail {

    LUABIND_API void push_instance_metatable(lua_State* L);

    namespace {

        int create_cpp_class_metatable(lua_State* L)
        {
            lua_newtable(L);

            lua_pushstring(L, "__luabind_classrep");
            lua_pushboolean(L, 1);
            lua_rawset(L, -3);

            lua_pushstring(L, "__gc");
            lua_pushcclosure(
                L
              , &garbage_collector_s<
                    detail::class_rep
                >::apply
                , 0);

            lua_rawset(L, -3);

            lua_pushstring(L, "__call");
            lua_pushcclosure(L, &class_rep::constructor_dispatcher, 0);
            lua_rawset(L, -3);

            lua_pushstring(L, "__index");
            lua_pushcclosure(L, &class_rep::static_class_gettable, 0);
            lua_rawset(L, -3);

            lua_pushstring(L, "__newindex");
            lua_pushcclosure(L, &class_rep::lua_settable_dispatcher, 0);
            lua_rawset(L, -3);

            return luaL_ref(L, LUA_REGISTRYINDEX);
        }

        int create_lua_class_metatable(lua_State* L)
        {
            lua_newtable(L);

            lua_pushstring(L, "__luabind_classrep");
            lua_pushboolean(L, 1);
            lua_rawset(L, -3);

            lua_pushstring(L, "__gc");
            lua_pushcclosure(
                L
              , &detail::garbage_collector_s<
                    detail::class_rep
                >::apply
                , 0);

            lua_rawset(L, -3);

            lua_pushstring(L, "__newindex");
            lua_pushcclosure(L, &class_rep::lua_settable_dispatcher, 0);
            lua_rawset(L, -3);

            lua_pushstring(L, "__call");
            lua_pushcclosure(L, &class_rep::constructor_dispatcher, 0);
            lua_rawset(L, -3);

            lua_pushstring(L, "__index");
            lua_pushcclosure(L, &class_rep::static_class_gettable, 0);
            lua_rawset(L, -3);

            return luaL_ref(L, LUA_REGISTRYINDEX);
        }

    } // namespace unnamed

    class class_rep;

    class_registry::class_registry(lua_State* L)
        : m_cpp_class_metatable(create_cpp_class_metatable(L))
        , m_lua_class_metatable(create_lua_class_metatable(L))
        , m_lua_function_metatable(LUA_NOREF)
    {
        push_instance_metatable(L);
        m_instance_metatable = luaL_ref(L, LUA_REGISTRYINDEX);
    }

    class_registry* class_registry::get_registry(lua_State* L)
    {
        if (state_context* ctx = get_state_context(L))
            return ctx->registry;

        lua_pushstring(L, "__luabind_classes");
        lua_rawget(L, LUA_REGISTRYINDEX);
        class_registry* p = static_cast<class_registry*>(lua_touserdata(L, -1));
        lua_pop(L, 1);

        return p;
    }

    void class_registry::add_class(type_id const& info, class_rep* crep)
    {
        assert((m_classes.find(info) == m_classes.end())
            && "you are trying to register a class twice");
        m_classes[info] = crep;
    }

    class_rep* class_registry::find_class(type_id const& info) const
    {
        std::map<type_id, class_rep*>::const_iterator i(
            m_classes.find(info));

        if (i == m_classes.end()) return 0;
        return i->second;
    }

}} // namespace luabind::detail

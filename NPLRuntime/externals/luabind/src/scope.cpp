// Copyright (c) 2004 Daniel Wallin and Arvid Norberg

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

#include <luabind/scope.hpp>
#include <luabind/detail/debug.hpp>
#include <luabind/detail/stack_utils.hpp>
#include <cassert>

namespace luabind {

    void scope::destroy_chain() noexcept
    {
        detail::registration* cur = m_chain;
        m_chain = 0;
        m_tail = 0;
        while (cur != 0)
        {
            detail::registration* next = cur->m_next;
            cur->m_destroy_fn(cur);
            cur = next;
        }
    }

    void scope::register_(lua_State* L) const
    {
        for (detail::registration* r = m_chain; r != 0; r = r->m_next)
        {
            LUABIND_CHECK_STACK(L);
            r->register_(L);
        }
    }

} // namespace luabind

namespace luabind {

    namespace {

        struct lua_pop_stack
        {
            explicit lua_pop_stack(lua_State* L) noexcept
                : m_state(L)
            {}

            ~lua_pop_stack()
            {
                lua_pop(m_state, 1);
            }

            lua_State* m_state;
        };

    } // namespace unnamed

    module_::module_(lua_State* L, char const* name)
        : m_state(L)
        , m_name(name)
    {
    }

    void module_::operator[](scope s)
    {
        if (m_name)
        {
            lua_getfield(m_state, LUA_GLOBALSINDEX, m_name);

            if (!lua_istable(m_state, -1))
            {
                lua_pop(m_state, 1);

                lua_newtable(m_state);
                lua_pushvalue(m_state, -1);
                lua_setfield(m_state, LUA_GLOBALSINDEX, m_name);
            }
        }
        else
        {
            lua_pushvalue(m_state, LUA_GLOBALSINDEX);
        }

        lua_pop_stack guard(m_state);

        s.register_(m_state);
    }

    struct namespace_::registration_ : detail::registration
    {
        explicit registration_(char const* name) noexcept
            : detail::registration(&registration_::do_register, &detail::registration::typed_destroy<registration_>)
            , m_name(name)
        {
        }

        static void do_register(lua_State* L, detail::registration const* base)
        {
            registration_ const* self = static_cast<registration_ const*>(base);
            LUABIND_CHECK_STACK(L);
            assert(lua_gettop(L) >= 1);

            lua_getfield(L, -1, self->m_name);

            detail::stack_pop p(L, 1); // pops the table on exit

            if (!lua_istable(L, -1))
            {
                lua_pop(L, 1);

                lua_newtable(L);
                lua_pushvalue(L, -1);
                lua_setfield(L, -3, self->m_name);
            }

            self->m_scope.register_(L);
        }

        char const* m_name;
        scope m_scope;
    };

    namespace_::namespace_(char const* name)
        : scope(m_registration = new registration_(name))
    {
    }

    namespace_& namespace_::operator[](scope s)
    {
        m_registration->m_scope.operator,(static_cast<scope&&>(s));
        return *this;
    }

} // namespace luabind

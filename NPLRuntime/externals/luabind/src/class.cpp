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

#include <luabind/config.hpp>
#include <luabind/class.hpp>
#include <luabind/nil.hpp>

#include <cstring>

namespace luabind
{
   LUABIND_API detail::nil_type nil;
}

namespace luabind { namespace detail {

    namespace
    {
      struct cast_entry
      {
          cast_entry(class_id src, class_id target, cast_function cast) noexcept
            : src(src)
            , target(target)
            , cast(cast)
          {}

          class_id src;
          class_id target;
          cast_function cast;
      };

    } // namespace unnamed

    struct class_registration : registration
    {
        explicit class_registration(char const* name);

        static void do_register(lua_State* L, registration const* base);

        const char* m_name;

        mutable std::map<const char*, int, detail::ltstr> m_static_constants;

        typedef std::pair<type_id, cast_function> base_desc;
        mutable std::vector<base_desc> m_bases;

        type_id m_type;
        class_id m_id;
        class_id m_wrapper_id;
        type_id m_wrapper_type;
        std::vector<cast_entry> m_casts;

        scope m_scope;
        scope m_members;
        scope m_default_members;
    };

    class_registration::class_registration(char const* name)
        : registration(&class_registration::do_register, &registration::typed_destroy<class_registration>)
        , m_name(name)
    {
    }

    void class_registration::do_register(lua_State* L, registration const* base_reg)
    {
        class_registration const* self = static_cast<class_registration const*>(base_reg);
        LUABIND_CHECK_STACK(L);

        assert(lua_type(L, -1) == LUA_TTABLE);

        lua_pushstring(L, self->m_name);

        detail::state_context* ctx = detail::get_state_context(L);
        detail::class_registry* r = ctx ? ctx->registry : detail::class_registry::get_registry(L);

        lua_newuserdata(L, sizeof(detail::class_rep));
        detail::class_rep* crep = reinterpret_cast<detail::class_rep*>(lua_touserdata(L, -1));

        new(crep) detail::class_rep(
            self->m_type
          , self->m_name
          , L
        );

        r->add_class(self->m_type, crep);

        class_map* classes = ctx ? ctx->classes : 0;
        if (!classes)
        {
            lua_pushstring(L, "__luabind_class_map");
            lua_rawget(L, LUA_REGISTRYINDEX);
            classes = static_cast<class_map*>(lua_touserdata(L, -1));
            lua_pop(L, 1);
        }

        classes->put(self->m_id, crep);

        bool const has_wrapper = self->m_wrapper_id != registered_class<null_type>::id;

        if (has_wrapper)
            classes->put(self->m_wrapper_id, crep);

        crep->m_static_constants.swap(self->m_static_constants);

        crep->get_default_table(L);
        self->m_scope.register_(L);
        self->m_default_members.register_(L);
        lua_pop(L, 1);

        crep->get_table(L);
        self->m_members.register_(L);
        lua_pop(L, 1);

        cast_graph* casts = ctx ? ctx->casts : 0;
        if (!casts)
        {
            lua_pushstring(L, "__luabind_cast_graph");
            lua_rawget(L, LUA_REGISTRYINDEX);
            casts = static_cast<cast_graph*>(lua_touserdata(L, -1));
            lua_pop(L, 1);
        }

        class_id_map* class_ids = ctx ? ctx->class_ids : 0;
        if (!class_ids)
        {
            lua_pushstring(L, "__luabind_class_id_map");
            lua_rawget(L, LUA_REGISTRYINDEX);
            class_ids = static_cast<class_id_map*>(lua_touserdata(L, -1));
            lua_pop(L, 1);
        }

        class_ids->put(self->m_id, self->m_type);

        if (has_wrapper)
            class_ids->put(self->m_wrapper_id, self->m_wrapper_type);

        for (std::size_t ci = 0, cn = self->m_casts.size(); ci < cn; ++ci)
        {
            cast_entry const& e = self->m_casts[ci];
            casts->insert(e.src, e.target, e.cast);
        }

        for (std::vector<base_desc>::iterator i = self->m_bases.begin();
            i != self->m_bases.end(); ++i)
        {
            LUABIND_CHECK_STACK(L);

            detail::class_rep* bcrep = r->find_class(i->first);

            detail::class_rep::base_info base;
            base.pointer_offset = 0;
            base.base = bcrep;

            crep->add_base_class(base);

            crep->get_table(L);
            bcrep->get_table(L);
            lua_pushnil(L);

            while (lua_next(L, -2))
            {
                lua_pushvalue(L, -2);
                lua_gettable(L, -5);

                if (!lua_isnil(L, -1))
                {
                    lua_pop(L, 2);
                    continue;
                }

                lua_pop(L, 1);
                lua_pushvalue(L, -2);
                lua_insert(L, -2);

                lua_settable(L, -5);
            }
            lua_pop(L, 2);

            crep->get_default_table(L);
            bcrep->get_default_table(L);
            lua_pushnil(L);

            while (lua_next(L, -2))
            {
                lua_pushvalue(L, -2);
                lua_gettable(L, -5);

                if (!lua_isnil(L, -1))
                {
                    lua_pop(L, 2);
                    continue;
                }

                lua_pop(L, 1);
                lua_pushvalue(L, -2);
                lua_insert(L, -2);

                lua_settable(L, -5);
            }
            lua_pop(L, 2);
        }

        crep->refresh_member_flags(L);

        lua_settable(L, -3);
    }

    // -- interface ---------------------------------------------------------

    class_base::class_base(char const* name)
        : scope(m_registration = new class_registration(name))
    {
    }

    void class_base::init(
        type_id const& type_id_, class_id id
      , type_id const& wrapper_type, class_id wrapper_id)
    {
        m_registration->m_type = type_id_;
        m_registration->m_id = id;
        m_registration->m_wrapper_type = wrapper_type;
        m_registration->m_wrapper_id = wrapper_id;
    }

    void class_base::add_base(type_id const& base, cast_function cast)
    {
        m_registration->m_bases.push_back(std::make_pair(base, cast));
    }

    void class_base::add_member(registration* member) noexcept
    {
        m_registration->m_members.operator,(scope(member));
    }

    void class_base::add_default_member(registration* member) noexcept
    {
        m_registration->m_default_members.operator,(scope(member));
    }

    const char* class_base::name() const
    {
        return m_registration->m_name;
    }

    void class_base::add_static_constant(const char* name, int val)
    {
        m_registration->m_static_constants[name] = val;
    }

    void class_base::add_inner_scope(scope& s)
    {
        m_registration->m_scope.operator,(s);
    }

    void class_base::add_cast(
        class_id src, class_id target, cast_function cast)
    {
        m_registration->m_casts.push_back(cast_entry(src, target, cast));
    }

    void add_custom_name(type_id const& i, std::string& s)
    {
        s += " [";
        s += i.name();
        s += "]";
    }

    std::string get_class_name(lua_State* L, type_id const& i)
    {
        std::string ret;
        assert(L);

        class_registry* r = class_registry::get_registry(L);
        class_rep* crep = r ? r->find_class(i) : 0;

        if (crep == 0)
        {
            ret = "custom";
            add_custom_name(i, ret);
        }
        else
        {
            if (i == crep->type())
            {
                ret += crep->name();
            }
            else
            {
                ret += "smart_ptr<";
                ret += crep->name();
                ret += ">";
            }
        }
        return ret;
    }

}} // namespace luabind::detail

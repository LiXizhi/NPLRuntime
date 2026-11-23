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

#define LUABIND_BUILDING

#include <luabind/lua_include.hpp>

#include <luabind/detail/stack_utils.hpp>
#include <luabind/detail/conversion_storage.hpp>
#include <luabind/luabind.hpp>
#include <luabind/exception_handler.hpp>
#include <luabind/get_main_thread.hpp>
#include <cstring>
#include <utility>

using namespace luabind::detail;

namespace luabind { namespace detail
{
	LUABIND_API int property_tag(lua_State* L)
	{
		lua_pushstring(L, "luabind: property_tag function can't be called");
		lua_error(L);
		return 0;
	}
}}

luabind::detail::class_rep::class_rep(type_id const& type
	, const char* name
	, lua_State* L
)
	: m_type(type)
	, m_name(name)
	, m_class_type(cpp_class)
	, m_operator_cache(0)
	, m_has_properties(false)
	, m_has_finalize(false)
{
	lua_newtable(L);
	handle(L, -1).swap(m_table);
	lua_newtable(L);
	handle(L, -1).swap(m_default_table);
	lua_pop(L, 2);

	state_context* ctx = get_state_context(L);
	class_registry* r = ctx ? ctx->registry : class_registry::get_registry(L);
	assert((r->cpp_class() != LUA_NOREF) && "you must call luabind::open()");

	lua_rawgeti(L, LUA_REGISTRYINDEX, r->cpp_class());
	lua_setmetatable(L, -2);

	lua_pushvalue(L, -1); // duplicate our user data
	m_self_ref.set(L);

	m_fallback_instance_metatable = r->cpp_instance();

	// Create per-class instance metatable whose __index defaults directly to m_table
	// (enabling LuaJIT to trace-compile obj:Method() without any C metamethod call
	// whenever the class has no properties).
	push_instance_metatable(L);
	m_table.push(L);
	lua_setfield(L, -2, "__index");
	m_instance_metatable = luaL_ref(L, LUA_REGISTRYINDEX);

	if (ctx)
	{
		m_casts = ctx->casts;
		m_classes = ctx->class_ids;
	}
	else
	{
		lua_pushstring(L, "__luabind_cast_graph");
		lua_rawget(L, LUA_REGISTRYINDEX);
		m_casts = static_cast<cast_graph*>(lua_touserdata(L, -1));
		lua_pop(L, 1);

		lua_pushstring(L, "__luabind_class_id_map");
		lua_rawget(L, LUA_REGISTRYINDEX);
		m_classes = static_cast<class_id_map*>(lua_touserdata(L, -1));
		lua_pop(L, 1);
	}
}

luabind::detail::class_rep::class_rep(lua_State* L, const char* name)
	: m_type(typeid(null_type))
	, m_name(name)
	, m_class_type(lua_class)
	, m_operator_cache(0)
	, m_has_properties(true)
	, m_has_finalize(true)
{
	lua_newtable(L);
	handle(L, -1).swap(m_table);
	lua_newtable(L);
	handle(L, -1).swap(m_default_table);
	lua_pop(L, 2);

	state_context* ctx = get_state_context(L);
	class_registry* r = ctx ? ctx->registry : class_registry::get_registry(L);
	assert((r->cpp_class() != LUA_NOREF) && "you must call luabind::open()");

	lua_rawgeti(L, LUA_REGISTRYINDEX, r->lua_class());
	lua_setmetatable(L, -2);
	lua_pushvalue(L, -1);
	m_self_ref.set(L);

	m_fallback_instance_metatable = r->lua_instance();
	m_instance_metatable = r->lua_instance();

	if (ctx)
	{
		m_casts = ctx->casts;
		m_classes = ctx->class_ids;
	}
	else
	{
		lua_pushstring(L, "__luabind_cast_graph");
		lua_rawget(L, LUA_REGISTRYINDEX);
		m_casts = static_cast<cast_graph*>(lua_touserdata(L, -1));
		lua_pop(L, 1);

		lua_pushstring(L, "__luabind_class_id_map");
		lua_rawget(L, LUA_REGISTRYINDEX);
		m_classes = static_cast<class_id_map*>(lua_touserdata(L, -1));
		lua_pop(L, 1);
	}
}

luabind::detail::class_rep::~class_rep()
{
	if (m_class_type == cpp_class
		&& m_instance_metatable != LUA_NOREF
		&& m_instance_metatable != m_fallback_instance_metatable
		&& m_table.interpreter())
	{
		luaL_unref(m_table.interpreter(), LUA_REGISTRYINDEX, m_instance_metatable);
	}
}

void luabind::detail::class_rep::refresh_member_flags(lua_State* L)
{
	if (m_class_type != cpp_class)
		return;

	bool found_prop = false;
	bool found_finalize = false;

	m_table.push(L);
	lua_pushnil(L);
	while (lua_next(L, -2) != 0)
	{
		if (!found_prop && lua_tocfunction(L, -1) == &property_tag)
			found_prop = true;

		if (!found_finalize && lua_type(L, -2) == LUA_TSTRING)
		{
			char const* k = lua_tostring(L, -2);
			if (k && std::strcmp(k, "__finalize") == 0 && !lua_isnil(L, -1))
				found_finalize = true;
		}
		lua_pop(L, 1);
	}
	lua_pop(L, 1);

	m_has_properties = found_prop;
	m_has_finalize = found_finalize;

	// Update per-class metatable __index in-place so both existing and new instances match
	lua_rawgeti(L, LUA_REGISTRYINDEX, m_instance_metatable);
	if (found_prop)
	{
		lua_rawgeti(L, -1, 1); // [1] holds &get_instance_value CClosure
		lua_setfield(L, -2, "__index");
	}
	else
	{
		m_table.push(L);
		lua_setfield(L, -2, "__index");
	}
	lua_pop(L, 1);
}

std::pair<void*,void*>
luabind::detail::class_rep::allocate(lua_State* L) const
{
	const int size = sizeof(object_rep);
	char* mem = static_cast<char*>(lua_newuserdata(L, size));
	return std::pair<void*,void*>(mem, (void*)0);
}

namespace
{
  bool super_deprecation_disabled = false;
} // namespace unnamed

int luabind::detail::class_rep::constructor_dispatcher(lua_State* L)
{
    class_rep* cls = static_cast<class_rep*>(lua_touserdata(L, 1));

    int args = lua_gettop(L);

    push_new_instance(L, cls);

    if (super_deprecation_disabled
        && cls->get_class_type() == class_rep::lua_class
        && !cls->bases().empty())
    {
        lua_pushstring(L, "super");
        lua_pushvalue(L, 1);
        lua_pushvalue(L, -3);
        lua_pushcclosure(L, super_callback, 2);
        lua_settable(L, LUA_GLOBALSINDEX);
    }

    lua_pushvalue(L, -1);
    lua_replace(L, 1);

    cls->get_table(L);
    lua_pushliteral(L, "__init");
    lua_rawget(L, -2);

    lua_insert(L, 1);

    lua_pop(L, 1);
    lua_insert(L, 1);

    lua_call(L, args, 0);

    if (super_deprecation_disabled)
    {
        lua_pushstring(L, "super");
        lua_pushnil(L);
        lua_settable(L, LUA_GLOBALSINDEX);
    }

    return 1;
}

void luabind::detail::class_rep::add_base_class(const luabind::detail::class_rep::base_info& binfo)
{
	assert(binfo.base && "You cannot derive from an unregistered type");

	class_rep* bcrep = binfo.base;

	for (std::map<const char*, int, ltstr>::const_iterator i = bcrep->m_static_constants.begin();
			i != bcrep->m_static_constants.end(); ++i)
	{
		int& v = m_static_constants[i->first];
		v = i->second;
	}

	m_bases.push_back(binfo);
	if (bcrep->m_has_finalize)
		m_has_finalize = true;
}

LUABIND_API void luabind::disable_super_deprecation()
{
    super_deprecation_disabled = true;
}

int luabind::detail::class_rep::super_callback(lua_State* L)
{
	int args = lua_gettop(L);

	class_rep* crep = static_cast<class_rep*>(lua_touserdata(L, lua_upvalueindex(1)));
	class_rep* base = crep->bases()[0].base;

	if (base->bases().empty())
	{
		lua_pushstring(L, "super");
		lua_pushnil(L);
		lua_settable(L, LUA_GLOBALSINDEX);
	}
	else
	{
		lua_pushstring(L, "super");
		lua_pushlightuserdata(L, base);
		lua_pushvalue(L, lua_upvalueindex(2));
		lua_pushcclosure(L, super_callback, 2);
		lua_settable(L, LUA_GLOBALSINDEX);
	}

	base->get_table(L);
	lua_pushstring(L, "__init");
	lua_gettable(L, -2);
	lua_insert(L, 1);
	lua_pop(L, 1);

	lua_pushvalue(L, lua_upvalueindex(2));
	lua_insert(L, 2);

	lua_call(L, args + 1, 0);

	lua_pushstring(L, "super");
	lua_pushnil(L);
	lua_settable(L, LUA_GLOBALSINDEX);

	return 0;
}

int luabind::detail::class_rep::lua_settable_dispatcher(lua_State* L)
{
	class_rep* crep = static_cast<class_rep*>(lua_touserdata(L, 1));

	crep->get_table(L);

	lua_pushvalue(L, -3);
	lua_pushvalue(L, -3);
	lua_rawset(L, -3);
	lua_pop(L, 1);

	crep->get_default_table(L);
	lua_replace(L, 1);
	lua_rawset(L, -3);

	crep->m_operator_cache = 0;
	crep->refresh_member_flags(L);

	return 0;
}

int luabind::detail::class_rep::static_class_gettable(lua_State* L)
{
	class_rep* crep = static_cast<class_rep*>(lua_touserdata(L, 1));

	crep->get_default_table(L);
	lua_pushvalue(L, 2);
	lua_gettable(L, -2);
	if (!lua_isnil(L, -1)) return 1;
	else lua_pop(L, 2);

	const char* key = lua_tostring(L, 2);
	if (!key || std::strlen(key) != lua_strlen(L, 2))
	{
		lua_pushnil(L);
		return 1;
	}

	std::map<const char*, int, ltstr>::const_iterator j = crep->m_static_constants.find(key);

	if (j != crep->m_static_constants.end())
	{
		lua_pushnumber(L, j->second);
		return 1;
	}

#ifndef LUABIND_NO_ERROR_CHECKING
	{
		std::string msg = "no static '";
		msg += key;
		msg += "' in class '";
		msg += crep->name();
		msg += "'";
		lua_pushstring(L, msg.c_str());
	}
	lua_error(L);
#endif

	lua_pushnil(L);
	return 1;
}

bool luabind::detail::is_class_rep(lua_State* L, int index)
{
	if (lua_getmetatable(L, index) == 0) return false;

	lua_pushstring(L, "__luabind_classrep");
	lua_rawget(L, -2);
	if (lua_toboolean(L, -1))
	{
		lua_pop(L, 2);
		return true;
	}

	lua_pop(L, 2);
	return false;
}

void luabind::detail::finalize(lua_State* L, class_rep* crep)
{
	if (crep->get_class_type() != class_rep::lua_class) return;

	crep->get_table(L);
	lua_pushliteral(L, "__finalize");
	lua_gettable(L, -2);
	lua_remove(L, -2);

	if (lua_isnil(L, -1))
	{
		lua_pop(L, 1);
	}
	else
	{
		lua_pushvalue(L, -2);
		lua_call(L, 1, 0);
	}

	for (std::vector<class_rep::base_info>::const_iterator
			i = crep->bases().begin(); i != crep->bases().end(); ++i)
	{
		if (i->base) finalize(L, i->base);
	}
}

void luabind::detail::class_rep::cache_operators(lua_State* L)
{
	m_operator_cache = 0x1;

	for (int i = 0; i < number_of_operators; ++i)
	{
		get_table(L);
		lua_pushstring(L, get_operator_name(i));
		lua_rawget(L, -2);

		if (lua_isfunction(L, -1)) m_operator_cache |= 1 << (i + 1);

		lua_pop(L, 2);
	}
}

bool luabind::detail::class_rep::has_operator_in_lua(lua_State* L, int id)
{
	if ((m_operator_cache & 0x1) == 0)
		cache_operators(L);

	const int mask = 1 << (id + 1);

	return (m_operator_cache & mask) != 0;
}

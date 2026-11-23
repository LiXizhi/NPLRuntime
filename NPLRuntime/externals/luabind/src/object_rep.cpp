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

#include <luabind/object.hpp>
#include <luabind/detail/object_rep.hpp>
#include <luabind/detail/class_rep.hpp>
#include <luabind/detail/call.hpp>
#include <stdexcept>

namespace luabind { namespace detail
{
    int object_rep::s_magic_anchor = 0;

    LUABIND_API std::pair<void*, int> cast_class_ptr(
        class_rep const* cls
      , void* naked_ptr
      , class_id src
      , class_id target
      , class_id dynamic_id
      , void const* dynamic_ptr
    )
    {
        return cls->casts().cast(naked_ptr, src, target, dynamic_id, dynamic_ptr);
    }

    LUABIND_API LUABIND_NOINLINE void throw_unregistered_class()
    {
#ifndef LUABIND_NO_EXCEPTIONS
        throw std::runtime_error("Trying to use unregistered class");
#else
        std::abort();
#endif
    }

    object_rep::object_rep(instance_holder* instance, class_rep* crep) noexcept
        : m_cached_ptr(0)
        , m_cached_cls_id(unknown_class)
        , m_instance(instance)
        , m_classrep(crep)
        , m_magic(expected_magic())
        , m_dependency_cnt(0)
        , m_flags((instance && instance->pointee_const()) ? kFlagIsConst : 0)
    {}

    object_rep::~object_rep()
    {
        m_magic = 0;
        if (!m_instance)
            return;
        m_instance->~instance_holder();
        deallocate(m_instance);
    }

    void object_rep::add_dependency(lua_State* L, int index)
    {
        assert(m_dependency_cnt < sizeof(object_rep));

        void* key = (char*)this + m_dependency_cnt;

        lua_pushlightuserdata(L, key);
        lua_pushvalue(L, index);
        lua_rawset(L, LUA_REGISTRYINDEX);

        ++m_dependency_cnt;
    }

    void object_rep::release_dependency_refs(lua_State* L)
    {
        for (std::size_t i = 0; i < m_dependency_cnt; ++i)
        {
            void* key = (char*)this + i;
            lua_pushlightuserdata(L, key);
            lua_pushnil(L);
            lua_rawset(L, LUA_REGISTRYINDEX);
        }
    }

    int destroy_instance(lua_State* L)
    {
        object_rep* instance = static_cast<object_rep*>(lua_touserdata(L, 1));
        if (LUABIND_UNLIKELY(!instance))
            return 0;

        class_rep const* crep = instance->crep();
        if (LUABIND_UNLIKELY((crep && crep->has_finalize()) || instance->has_custom_fenv()))
        {
            lua_pushliteral(L, "__finalize");
            lua_gettable(L, 1);

            if (lua_isnil(L, -1))
            {
                lua_pop(L, 1);
            }
            else
            {
                lua_pushvalue(L, 1);
                lua_call(L, 1, 0);
            }
        }

        if (LUABIND_UNLIKELY(instance->dependency_count() != 0))
            instance->release_dependency_refs(L);

        instance->~object_rep();
        return 0;
    }

    namespace
    {

      int set_instance_value(lua_State* L)
      {
          lua_getfenv(L, 1);
          lua_pushvalue(L, 2);
          lua_rawget(L, -2);

          int t = lua_type(L, -1);
          if (LUABIND_UNLIKELY(t == LUA_TNIL))
          {
              object_rep const* inst = static_cast<object_rep const*>(lua_touserdata(L, 1));
              if ((!inst || !inst->has_valid_magic() || inst->has_custom_fenv()) && lua_getmetatable(L, -2))
              {
                  lua_pushvalue(L, 2);
                  lua_rawget(L, -2);
                  lua_replace(L, -3);
                  lua_pop(L, 1);
                  t = lua_type(L, -1);
              }
          }

          if (t == LUA_TFUNCTION && lua_tocfunction(L, -1) == &property_tag)
          {
              // Fast path: invoke C++ property setter function_object directly without lua_call
              if (lua_getupvalue(L, -1, 4))
              {
                  function_object const* setter_fo =
                      static_cast<function_object const*>(lua_touserdata(L, -1));
                  if (LUABIND_LIKELY(setter_fo != 0))
                  {
                      lua_settop(L, 3);
                      lua_replace(L, 2);
                      invoke_function_object(L, setter_fo, 2);
                      return 0;
                  }
                  lua_pop(L, 1);
              }

              // Fallback for custom Lua property setters or read-only error
              lua_getupvalue(L, -1, 2);

              if (lua_isnil(L, -1))
              {
                  lua_pushfstring(L, "property '%s' is read only", lua_tostring(L, 2));
                  lua_error(L);
              }

              lua_pushvalue(L, 1);
              lua_pushvalue(L, 3);
              lua_call(L, 2, 0);
              return 0;
          }

          lua_pop(L, 1);

          if (!lua_getmetatable(L, 4))
          {
              lua_newtable(L);
              lua_pushvalue(L, -1);
              lua_setfenv(L, 1);
              lua_pushvalue(L, 4);
              lua_setmetatable(L, -2);

              object_rep* inst = static_cast<object_rep*>(lua_touserdata(L, 1));
              if (inst && inst->has_valid_magic())
              {
                  inst->mark_custom_fenv();
                  if (inst->crep())
                  {
                      lua_rawgeti(L, LUA_REGISTRYINDEX, inst->crep()->fallback_metatable_ref());
                      lua_setmetatable(L, 1);
                  }
              }
          }
          else
          {
              lua_pop(L, 1);
          }

          lua_pushvalue(L, 2);
          lua_pushvalue(L, 3);
          lua_rawset(L, -3);

          return 0;
      }

      int get_instance_value(lua_State* L)
      {
          lua_getfenv(L, 1);
          lua_pushvalue(L, 2);
          lua_rawget(L, -2);

          int t = lua_type(L, -1);

          if (LUABIND_UNLIKELY(t == LUA_TNIL))
          {
              object_rep const* inst = static_cast<object_rep const*>(lua_touserdata(L, 1));
              if (inst && inst->has_valid_magic() && !inst->has_custom_fenv())
                  return 1;

              if (lua_getmetatable(L, -2))
              {
                  lua_pushvalue(L, 2);
                  lua_rawget(L, -2);
                  t = lua_type(L, -1);
              }
          }

          if (t == LUA_TFUNCTION && lua_tocfunction(L, -1) == &property_tag)
          {
              // Fast path: invoke C++ property getter function_object directly without lua_call
              if (lua_getupvalue(L, -1, 3))
              {
                  function_object const* getter_fo =
                      static_cast<function_object const*>(lua_touserdata(L, -1));
                  if (LUABIND_LIKELY(getter_fo != 0))
                  {
                      lua_settop(L, 1);
                      return invoke_function_object(L, getter_fo, 1);
                  }
                  lua_pop(L, 1);
              }

              lua_getupvalue(L, -1, 1);
              lua_pushvalue(L, 1);
              lua_call(L, 1, 1);
          }

          return 1;
      }

      int dispatch_operator(lua_State* L)
      {
          for (int i = 0; i < 2; ++i)
          {
              if (get_instance_fast(L, 1 + i))
              {
                  int nargs = lua_gettop(L);

                  lua_pushvalue(L, lua_upvalueindex(1));
                  lua_gettable(L, 1 + i);

                  if (lua_isnil(L, -1))
                  {
                      lua_pop(L, 1);
                      continue;
                  }

                  lua_insert(L, 1); // move the function to the bottom

                  bool const is_unary = lua_toboolean(L, lua_upvalueindex(2)) != 0;
                  nargs = is_unary ? 1 : nargs;

                  if (is_unary) // remove trailing nil
                      lua_remove(L, 3);

                  lua_call(L, nargs, 1);
                  return 1;
              }
          }

          lua_pop(L, lua_gettop(L));
          lua_pushstring(L, "No such operator defined");
          lua_error(L);

          return 0;
      }

    } // namespace unnamed

    LUABIND_API void push_instance_metatable(lua_State* L)
    {
        lua_newtable(L);

        lua_pushboolean(L, 1);
        lua_setfield(L, -2, "__luabind_class");

        lua_pushnumber(L, 1);
        lua_pushcclosure(L, get_instance_value, 0);
        lua_rawset(L, -3);

        lua_pushcclosure(L, destroy_instance, 0);
        lua_setfield(L, -2, "__gc");

        lua_pushcclosure(L, get_instance_value, 0);
        lua_setfield(L, -2, "__index");

        lua_pushcclosure(L, set_instance_value, 0);
        lua_setfield(L, -2, "__newindex");

        for (int op = 0; op < number_of_operators; ++op)
        {
            lua_pushstring(L, get_operator_name(op));
            lua_pushvalue(L, -1);
            lua_pushboolean(L, op == op_unm || op == op_len);
            lua_pushcclosure(L, &dispatch_operator, 2);
            lua_rawset(L, -3);
        }
    }

    LUABIND_API object_rep* get_instance(lua_State* L, int index) noexcept
    {
        return get_instance_fast(L, index);
    }

    LUABIND_API object_rep* push_new_instance(lua_State* L, class_rep* cls)
    {
        void* storage = lua_newuserdata(L, sizeof(object_rep));
        object_rep* result = new (storage) object_rep(0, cls);
        cls->get_table(L);
        lua_setfenv(L, -2);
        lua_rawgeti(L, LUA_REGISTRYINDEX, cls->metatable_ref());
        lua_setmetatable(L, -2);
        return result;
    }

}}

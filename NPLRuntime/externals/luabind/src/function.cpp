// Copyright Daniel Wallin 2008. Use, modification and distribution is
// subject to the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)

#define LUABIND_BUILDING

#include <limits>
#include <luabind/make_function.hpp>
#include <luabind/detail/class_registry.hpp>

namespace luabind { namespace detail {

namespace
{

  int function_destroy(lua_State* L)
  {
      function_object* fn = *(function_object**)lua_touserdata(L, 1);
      if (fn)
          fn->destroy_fn(fn);
      return 0;
  }

  void push_function_metatable(lua_State* L)
  {
      if (state_context* ctx = get_state_context(L))
      {
          if (ctx->function_metatable_ref != LUA_NOREF)
          {
              lua_rawgeti(L, LUA_REGISTRYINDEX, ctx->function_metatable_ref);
              return;
          }
      }

      lua_pushstring(L, "luabind.function");
      lua_rawget(L, LUA_REGISTRYINDEX);

      if (lua_istable(L, -1))
          return;

      lua_pop(L, 1);

      lua_newtable(L);

      lua_pushstring(L, "__gc");
      lua_pushcclosure(L, &function_destroy, 0);
      lua_rawset(L, -3);

      lua_pushstring(L, "luabind.function");
      lua_pushvalue(L, -2);
      lua_rawset(L, LUA_REGISTRYINDEX);
  }

  // A pointer to this is used as a tag value to identify functions exported
  // by luabind.
  int function_tag = 0;

  LUABIND_NOINLINE void format_invoke_error(
      lua_State* L
    , function_object const* overloads
    , int candidate_index
    , function_object const* const* candidates
  )
  {
      char const* function_name =
          overloads->name.empty() ? "<unknown>" : overloads->name.c_str();

      if (candidate_index == 0)
      {
          lua_pushstring(L, "No matching overload found, candidates:\n");
          int count = 0;
          for (function_object const* f = overloads; f != 0; f = f->next)
          {
              if (count != 0)
                  lua_pushstring(L, "\n");
              f->format_fn(L, function_name);
              ++count;
          }
          lua_concat(L, count * 2);
      }
      else
      {
          lua_pushstring(L, "Ambiguous, candidates:\n");
          for (int i = 0; i < candidate_index; ++i)
          {
              if (i != 0)
                  lua_pushstring(L, "\n");
              candidates[i]->format_fn(L, function_name);
          }
          lua_concat(L, candidate_index * 2);
      }
  }

  LUABIND_NOINLINE int dispatch_same_arity_overloads(
      lua_State* L
    , function_object const* impl
    , int nargs
  )
  {
      function_object const* candidates[10];
      int best_score = (std::numeric_limits<int>::max)();
      int cand_idx = 0;
      int results = -1;

#ifndef LUABIND_NO_EXCEPTIONS
      bool caught_exception = false;
      try
      {
#endif
          for (function_object const* f = impl; f != 0; f = f->next)
          {
              if (f->arity >= 0 && f->arity != nargs)
                  continue;

              int score = -1;
              f->call_fn(L, f, nargs, &score);
              if (score < 0)
                  continue;

              if (score < best_score)
              {
                  best_score = score;
                  candidates[0] = f;
                  cand_idx = 1;
              }
              else if (score == best_score)
              {
                  if (cand_idx < 10)
                      candidates[cand_idx++] = f;
              }
          }

          if (LUABIND_LIKELY(cand_idx == 1))
          {
              function_object const* winner = candidates[0];
              results = winner->call_fn(L, winner, nargs, 0);
          }
#ifndef LUABIND_NO_EXCEPTIONS
      }
      catch (...)
      {
          handle_exception_aux(L);
          caught_exception = true;
      }
      if (LUABIND_UNLIKELY(caught_exception))
          lua_error(L);
#endif

      if (LUABIND_UNLIKELY(results < 0))
      {
          format_invoke_error(L, impl, cand_idx, candidates);
          lua_error(L);
      }
      return results;
  }

  LUABIND_NOINLINE void raise_no_matching_overload(lua_State* L, function_object const* impl)
  {
      format_invoke_error(L, impl, 0, 0);
      lua_error(L);
  }

} // namespace unnamed

LUABIND_API int ensure_function_metatable_ref(lua_State* L)
{
    push_function_metatable(L);
    return luaL_ref(L, LUA_REGISTRYINDEX);
}

LUABIND_API void push_class_name_str(lua_State* L, type_id const& i)
{
    std::string name = get_class_name(L, i);
    lua_pushlstring(L, name.data(), name.size());
}

LUABIND_API void format_signature_steps(
    lua_State* L
  , char const* function
  , void (* const* push_fns)(lua_State*)
  , std::size_t count
)
{
    push_fns[0](L);
    lua_pushliteral(L, " ");
    lua_pushstring(L, function);
    lua_pushliteral(L, "(");
    for (std::size_t i = 1; i < count; ++i)
    {
        if (i > 1)
            lua_pushliteral(L, ",");
        push_fns[i](L);
    }
    lua_pushliteral(L, ")");
    int const nparts = (count <= 1) ? 4 : static_cast<int>(count * 2 + 2);
    lua_concat(L, nparts);
}

LUABIND_API int invoke_function_object(lua_State* L, function_object const* impl, int nargs)
{
    if (LUABIND_UNLIKELY(impl->has_same_arity_overload))
        return dispatch_same_arity_overloads(L, impl, nargs);

    int results = -1;
#ifndef LUABIND_NO_EXCEPTIONS
    bool caught_exception = false;
    try
    {
#endif
        if (LUABIND_LIKELY(impl->next == 0))
        {
            results = impl->call_fn(L, impl, nargs, 0);
        }
        else
        {
            for (function_object const* f = impl; f != 0; f = f->next)
            {
                if (f->arity == nargs)
                {
                    results = f->call_fn(L, f, nargs, 0);
                    break;
                }
            }
        }
#ifndef LUABIND_NO_EXCEPTIONS
    }
    catch (...)
    {
        handle_exception_aux(L);
        caught_exception = true;
    }
    if (LUABIND_UNLIKELY(caught_exception))
        lua_error(L);
#endif

    if (LUABIND_UNLIKELY(results < 0))
        raise_no_matching_overload(L, impl);

    return results;
}

// Single shared lua_CFunction entry point for ALL bound functions/methods in the engine!
LUABIND_API int luabind_function_entry(lua_State* L)
{
    function_object const* impl =
        *static_cast<function_object const* const*>(lua_touserdata(L, lua_upvalueindex(1)));
    return invoke_function_object(L, impl, lua_gettop(L));
}

LUABIND_API bool is_luabind_function(lua_State* L, int index)
{
    if (!lua_getupvalue(L, index, 2))
        return false;
    bool result = lua_touserdata(L, -1) == &function_tag;
    lua_pop(L, 1);
    return result;
}

LUABIND_API function_object* try_extract_function_object(lua_State* L, int index) noexcept
{
    if (!lua_isfunction(L, index))
        return 0;
    if (!lua_getupvalue(L, index, 2))
        return 0;
    bool is_lb = (lua_touserdata(L, -1) == &function_tag);
    lua_pop(L, 1);
    if (!is_lb)
        return 0;
    if (!lua_getupvalue(L, index, 1))
        return 0;
    function_object** storage = static_cast<function_object**>(lua_touserdata(L, -1));
    function_object* result = storage ? *storage : 0;
    lua_pop(L, 1);
    return result;
}

namespace
{
  inline bool is_luabind_function_obj(object const& obj)
  {
      obj.push(obj.interpreter());
      bool result = detail::is_luabind_function(obj.interpreter(), -1);
      lua_pop(obj.interpreter(), 1);
      return result;
  }

  inline void recompute_arity_collision(function_object* head) noexcept
  {
      bool collision = false;
      for (function_object const* a = head; a != 0 && !collision; a = a->next)
      {
          if (a->arity < 0)
          {
              collision = true;
              break;
          }
          for (function_object const* b = a->next; b != 0; b = b->next)
          {
              if (b->arity < 0 || a->arity == b->arity)
              {
                  collision = true;
                  break;
              }
          }
      }
      head->has_same_arity_overload = collision;
  }

  // Pushes new luabind CClosure onto stack top.
  void push_function_closure(lua_State* L, function_object* impl)
  {
      void* storage = lua_newuserdata(L, sizeof(function_object*));
      push_function_metatable(L);
      *(function_object**)storage = impl;
      lua_setmetatable(L, -2);

      lua_pushlightuserdata(L, &function_tag);
      lua_pushcclosure(L, impl->entry, 2);
  }
} // namespace unnamed

LUABIND_API void add_overload(
    object const& context, char const* name, object const& fn)
{
    function_object* f = *touserdata<function_object*>(getupvalue(fn, 1));
    f->name = name;

    if (object overloads = context[name])
    {
        if (is_luabind_function_obj(overloads) && is_luabind_function_obj(fn))
        {
            f->next = *touserdata<function_object*>(getupvalue(overloads, 1));
            f->keepalive = overloads;
            recompute_arity_collision(f);
        }
    }

    context[name] = fn;
}

// Fast registration directly onto the table at stack index -1 without temporary registry refs!
LUABIND_API void register_function_on_stack_top(
    lua_State* L, char const* name, function_object* impl)
{
    impl->name = name;

    // Stack [-1] is target table (module/namespace/class table).
    lua_getfield(L, -1, name);
    if (!lua_isnil(L, -1) && is_luabind_function(L, -1))
    {
        lua_getupvalue(L, -1, 1);
        function_object* prev = *static_cast<function_object**>(lua_touserdata(L, -1));
        lua_pop(L, 1);
        impl->next = prev;
        impl->keepalive = object(from_stack(L, -1));
        recompute_arity_collision(impl);
    }
    lua_pop(L, 1); // pop existing field or nil

    push_function_closure(L, impl);
    lua_setfield(L, -2, name);
}

LUABIND_API object make_function_aux(lua_State* L, function_object* impl)
{
    push_function_closure(L, impl);
    stack_pop pop(L, 1);
    return object(from_stack(L, -1));
}

}} // namespace luabind::detail

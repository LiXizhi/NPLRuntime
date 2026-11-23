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

#include <luabind/luabind.hpp>
#include <luabind/function.hpp>
#include <luabind/get_main_thread.hpp>

#ifndef EMSCRIPTEN_SINGLE_THREAD
#include <atomic>
#endif

namespace luabind {

namespace detail
{
    LUABIND_API int ensure_function_metatable_ref(lua_State* L);
    LUABIND_API function_object* try_extract_function_object(lua_State* L, int index) noexcept;
}

namespace
{
  int main_thread_tag;
  int state_context_tag;

#ifndef EMSCRIPTEN_SINGLE_THREAD
  std::atomic<std::uint64_t> g_state_close_epoch(1);

  struct tls_state_cache
  {
      lua_State* L;
      detail::state_context* ctx;
      std::uint64_t epoch;
  };

  thread_local tls_state_cache g_tls_state_cache = { 0, 0, 0 };

  inline std::uint64_t load_epoch() noexcept
  {
      return g_state_close_epoch.load(std::memory_order_acquire);
  }

  inline void bump_epoch() noexcept
  {
      g_state_close_epoch.fetch_add(1, std::memory_order_acq_rel);
      g_tls_state_cache.L = 0;
      g_tls_state_cache.ctx = 0;
      g_tls_state_cache.epoch = 0;
  }
#else
  std::uint64_t g_state_close_epoch = 1;
  lua_State* g_cached_L = 0;
  detail::state_context* g_cached_ctx = 0;
  std::uint64_t g_cached_epoch = 0;
#endif

  int make_property(lua_State* L)
  {
      int args = lua_gettop(L);

      if (args == 0 || args > 2)
      {
          lua_pushstring(L, "make_property() called with wrong number of arguments.");
          lua_error(L);
      }

      detail::push_property_closure(L, args == 2);
      return 1;
  }

  int deprecated_super(lua_State* L)
  {
      lua_pushstring(L,
          "DEPRECATION: 'super' has been deprecated in favor of "
          "directly calling the base class __init() function. "
          "This error can be disabled by calling 'luabind::disable_super_deprecation()'."
      );
      lua_error(L);

      return 0;
  }

  int destroy_class_id_map(lua_State* L)
  {
      detail::class_id_map* m =
          (detail::class_id_map*)lua_touserdata(L, 1);
      m->~class_id_map();
      return 0;
  }

  int destroy_cast_graph(lua_State* L)
  {
      detail::cast_graph* g =
          (detail::cast_graph*)lua_touserdata(L, 1);
      g->~cast_graph();
      return 0;
  }

  int destroy_class_map(lua_State* L)
  {
      detail::class_map* m =
          (detail::class_map*)lua_touserdata(L, 1);
      m->~class_map();
      return 0;
  }

  int destroy_state_context(lua_State* L)
  {
      detail::state_context* ctx =
          static_cast<detail::state_context*>(lua_touserdata(L, 1));
      if (ctx)
      {
          ctx->main_thread = 0;
          ctx->registry = 0;
          ctx->class_ids = 0;
          ctx->casts = 0;
          ctx->classes = 0;
          ctx->function_metatable_ref = LUA_NOREF;
      }
#ifndef EMSCRIPTEN_SINGLE_THREAD
      bump_epoch();
#else
      ++g_state_close_epoch;
      g_cached_L = 0;
      g_cached_ctx = 0;
      g_cached_epoch = 0;
#endif
      return 0;
  }

} // namespace unnamed

namespace detail
{
    LUABIND_API state_context* get_state_context(lua_State* L) noexcept
    {
        if (LUABIND_UNLIKELY(!L))
            return 0;

#ifndef EMSCRIPTEN_SINGLE_THREAD
        std::uint64_t const cur_epoch = load_epoch();
        if (LUABIND_LIKELY(g_tls_state_cache.L == L && g_tls_state_cache.epoch == cur_epoch))
            return g_tls_state_cache.ctx;
#else
        if (LUABIND_LIKELY(g_cached_L == L && g_cached_epoch == g_state_close_epoch))
            return g_cached_ctx;
#endif

        lua_pushlightuserdata(L, &state_context_tag);
        lua_rawget(L, LUA_REGISTRYINDEX);
        state_context* ctx = static_cast<state_context*>(lua_touserdata(L, -1));
        lua_pop(L, 1);

        if (LUABIND_LIKELY(ctx != 0 && ctx->main_thread != 0))
        {
#ifndef EMSCRIPTEN_SINGLE_THREAD
            g_tls_state_cache.L = L;
            g_tls_state_cache.ctx = ctx;
            g_tls_state_cache.epoch = cur_epoch;
#else
            g_cached_L = L;
            g_cached_ctx = ctx;
            g_cached_epoch = g_state_close_epoch;
#endif
        }
        return ctx;
    }

    // Stack on entry:
    //   if has_setter == false: [..., getter]
    //   if has_setter == true : [..., getter, setter]
    // Stack on exit:
    //   [..., property_cclosure] (replaces getter [& setter])
    LUABIND_API void push_property_closure(lua_State* L, bool has_setter)
    {
        if (!has_setter)
            lua_pushnil(L);

        // Stack: [..., getter(-2), setter(-1)]
        function_object* getter_fo = try_extract_function_object(L, -2);
        function_object* setter_fo = has_setter ? try_extract_function_object(L, -1) : 0;

        lua_pushlightuserdata(L, getter_fo);
        lua_pushlightuserdata(L, setter_fo);
        lua_pushcclosure(L, &detail::property_tag, 4);
    }
} // namespace detail

    LUABIND_API lua_State* get_main_thread(lua_State* L)
    {
        if (detail::state_context* ctx = detail::get_state_context(L))
        {
            if (ctx->main_thread)
                return ctx->main_thread;
        }

        lua_pushlightuserdata(L, &main_thread_tag);
        lua_rawget(L, LUA_REGISTRYINDEX);
        lua_State* result = static_cast<lua_State*>(lua_touserdata(L, -1));
        lua_pop(L, 1);

        if (!result)
            throw std::runtime_error("Unable to get main thread, luabind::open() not called?");

        return result;
    }

    LUABIND_API void open(lua_State* L)
    {
        bool is_main_thread = lua_pushthread(L) == 1;
        lua_pop(L, 1);

        if (!is_main_thread)
        {
            throw std::runtime_error(
                "luabind::open() must be called with the main thread "
                "lua_State*"
            );
        }

        if (detail::class_registry::get_registry(L))
            return;

        lua_pushstring(L, "__luabind_classes");
        detail::class_registry* r = static_cast<detail::class_registry*>(
            lua_newuserdata(L, sizeof(detail::class_registry)));

        // set gc metatable
        lua_newtable(L);
        lua_pushstring(L, "__gc");
        lua_pushcclosure(
            L
          , detail::garbage_collector_s<
                detail::class_registry
            >::apply
          , 0);

        lua_settable(L, -3);
        lua_setmetatable(L, -2);

        new(r) detail::class_registry(L);
        lua_settable(L, LUA_REGISTRYINDEX);

        lua_pushstring(L, "__luabind_class_id_map");
        void* classes_storage = lua_newuserdata(L, sizeof(detail::class_id_map));
        detail::class_id_map* class_ids = new (classes_storage) detail::class_id_map;

        lua_newtable(L);
        lua_pushcclosure(L, &destroy_class_id_map, 0);
        lua_setfield(L, -2, "__gc");
        lua_setmetatable(L, -2);

        lua_settable(L, LUA_REGISTRYINDEX);

        lua_pushstring(L, "__luabind_cast_graph");
        void* cast_graph_storage = lua_newuserdata(
            L, sizeof(detail::cast_graph));
        detail::cast_graph* graph = new (cast_graph_storage) detail::cast_graph;

        lua_newtable(L);
        lua_pushcclosure(L, &destroy_cast_graph, 0);
        lua_setfield(L, -2, "__gc");
        lua_setmetatable(L, -2);

        lua_settable(L, LUA_REGISTRYINDEX);

        lua_pushstring(L, "__luabind_class_map");
        void* class_map_storage = lua_newuserdata(
            L, sizeof(detail::class_map));
        detail::class_map* classes = new (class_map_storage) detail::class_map;

        lua_newtable(L);
        lua_pushcclosure(L, &destroy_class_map, 0);
        lua_setfield(L, -2, "__gc");
        lua_setmetatable(L, -2);

        lua_settable(L, LUA_REGISTRYINDEX);

        int fn_meta_ref = detail::ensure_function_metatable_ref(L);

        // Unified fast-lookup state_context keyed by lightuserdata
        lua_pushlightuserdata(L, &state_context_tag);
        void* ctx_storage = lua_newuserdata(L, sizeof(detail::state_context));
        detail::state_context* ctx = static_cast<detail::state_context*>(ctx_storage);
        ctx->main_thread = L;
        ctx->registry = r;
        ctx->class_ids = class_ids;
        ctx->casts = graph;
        ctx->classes = classes;
        ctx->function_metatable_ref = fn_meta_ref;

        lua_newtable(L);
        lua_pushcclosure(L, &destroy_state_context, 0);
        lua_setfield(L, -2, "__gc");
        lua_setmetatable(L, -2);
        lua_rawset(L, LUA_REGISTRYINDEX);

#ifndef EMSCRIPTEN_SINGLE_THREAD
        g_tls_state_cache.L = L;
        g_tls_state_cache.ctx = ctx;
        g_tls_state_cache.epoch = load_epoch();
#else
        g_cached_L = L;
        g_cached_ctx = ctx;
        g_cached_epoch = g_state_close_epoch;
#endif

        // add functions (class, cast etc...)
        lua_pushcclosure(L, detail::create_class::stage1, 0);
        lua_setfield(L, LUA_GLOBALSINDEX, "class");

        lua_pushcclosure(L, &make_property, 0);
        lua_setfield(L, LUA_GLOBALSINDEX, "property");

        lua_pushlightuserdata(L, &main_thread_tag);
        lua_pushlightuserdata(L, L);
        lua_rawset(L, LUA_REGISTRYINDEX);

        lua_pushcclosure(L, &deprecated_super, 0);
        lua_setfield(L, LUA_GLOBALSINDEX, "super");
    }

} // namespace luabind

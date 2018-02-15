#pragma once

struct lua_State;

#define luaL_error luaL_error_nobreak
#ifdef __cplusplus
extern "C" {
#endif

int luaL_error_nobreak(lua_State *L, const char *fmt, ...);

#ifdef __cplusplus
}
#endif
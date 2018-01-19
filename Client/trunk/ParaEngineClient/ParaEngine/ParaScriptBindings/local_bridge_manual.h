
#ifndef lua_manual_H
#define lua_manual_H

#ifdef __cplusplus
extern "C" {
#endif
#include "tolua++.h"
#ifdef __cplusplus
}
#endif

#include "base/CCRef.h"

TOLUA_API int register_all_local_bridge(lua_State* L);


#endif

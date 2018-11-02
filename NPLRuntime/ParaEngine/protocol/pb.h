#ifndef PB_H
#define PB_H
#include <stdint.h>
#include <string.h>
extern "C"
{
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#ifdef _ALLBSD_SOURCE
//#include <machine/endian.h>
#else
//#include <endian.h>
#endif

#if __BYTE_ORDER == __LITTLE_ENDIAN
#define IS_LITTLE_ENDIAN
#endif

#define IOSTRING_META "protobuf.IOString"

#define checkiostring(L) \
    (IOString*) luaL_checkudata(L, 1, IOSTRING_META)

#define IOSTRING_BUF_LEN 65535

typedef struct{
    size_t size;
    char buf[IOSTRING_BUF_LEN];
} IOString;

int luaopen_pb(lua_State *L);
#endif


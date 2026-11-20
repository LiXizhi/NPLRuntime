/* Exercise compiled loops, VM return paths and GC in the linked macOS SDK. */
#include "lua.h"
#include "lualib.h"
#include "luajit.h"
#include "lauxlib.h"
#include <stdio.h>

int main(void)
{
    lua_State *L = luaL_newstate();
    if (!L) return 1;
    luaL_openlibs(L);
    int result = luaL_dostring(L,
        "assert(jit.version == '" LUAJIT_VERSION "'); assert(jit.status()); "
        "local x=0; for i=1,100000 do x=x+i end; assert(x==5000050000); "
        "assert(require('jit.util').traceinfo(1), 'no compiled trace'); "
        "local sum=0; "
        "for i=1,100000 do local t={}; "
        "for j=1,100 do t[j]=tostring(i+j/3) end; "
        "sum=sum+#t end; assert(sum==10000000)");
    if (result) fprintf(stderr, "%s\n", lua_tostring(L, -1));
    lua_close(L);
    return result ? 1 : 0;
}

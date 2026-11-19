/* Exercise VM return paths across JIT traces and garbage collections.
 * Apple's linker deduplication corrupts these paths in the bundled LuaJIT
 * unless the luajit21 target propagates its macOS linker workaround.
 */
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
#include <stdio.h>

int main(void)
{
    lua_State *L = luaL_newstate();
    if (!L) return 1;
    luaL_openlibs(L);
    int result = luaL_dostring(L,
        "assert(jit.status()); local sum=0; "
        "for i=1,100000 do local t={}; "
        "for j=1,100 do t[j]=tostring(i+j/3) end; "
        "sum=sum+#t end; assert(sum==10000000)");
    if (result) fprintf(stderr, "%s\n", lua_tostring(L, -1));
    lua_close(L);
    return result ? 1 : 0;
}

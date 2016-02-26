#pragma once

struct lua_State;

namespace ParaScripting
{
	
	int luaopen_sqlite3(lua_State * L);
};
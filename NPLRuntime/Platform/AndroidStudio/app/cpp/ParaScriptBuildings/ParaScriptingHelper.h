//-----------------------------------------------------------------------------
// Class: ParaScriptingHelper.h
// Authors: Assistant
// CreateDate: 2024.12.20
// Description: Lua bindings for ParaEngineHelper functions
//-----------------------------------------------------------------------------

#pragma once

#include <string>

struct lua_State;

namespace ParaScripting
{
    class ParaScriptingHelper
    {
    public:
        static void LuabindRegisterHelperFunctions(lua_State *L);
        
        /** Intent to the outer MainActivity */
        static void intentToMainActivity();
    };
}
//-----------------------------------------------------------------------------
// Class: ParaScriptingHelper.cpp
// Authors: Assistant
// CreateDate: 2024.12.20
// Description: Lua bindings for ParaEngineHelper functions
//-----------------------------------------------------------------------------

#include "ParaScriptingHelper.h"
#include "../jni/ParaEngineHelper.h"
#include <luabind/luabind.hpp>

namespace ParaScripting
{
    void ParaScriptingHelper::LuabindRegisterHelperFunctions(lua_State *L)
    {
        using namespace luabind;

        module(L)
        [
            namespace_("ParaEngineHelper")
            [
                class_<ParaScripting::ParaScriptingHelper>("ParaScriptingHelper"),
                def("intentToMainActivity", ParaScripting::ParaScriptingHelper::intentToMainActivity)
            ]
        ];
    }

    void ParaScriptingHelper::intentToMainActivity()
    {
        ParaEngine::ParaEngineHelper::intentToMainActivity();
    }
}
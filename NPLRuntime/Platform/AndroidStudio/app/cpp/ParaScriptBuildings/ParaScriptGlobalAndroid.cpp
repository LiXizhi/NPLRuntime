//-----------------------------------------------------------------------------
// ParaScriptGlobalAndroid.cpp
// Authors: big
// Emails: onedous@gmail.com
// CreateDate: 2019.8.12
// ModifyDate: 2022.1.11
//-----------------------------------------------------------------------------

#include "ParaEngine.h"
#include "ParaScriptingGlobal.h"
#include "NPL/NPLScriptingState.h"
#include <luabind/object.hpp>

static void LuabindRegisterAndroidSettingsGlobalFunctions(lua_State *L)
{
    using namespace luabind;
}

void ParaScripting::CNPLScriptingState::LoadHAPI_Platform()
{
    using namespace luabind;
    lua_State* L = GetLuaState();

    LuabindRegisterAndroidSettingsGlobalFunctions(L);
}

bool ParaScripting::ParaGlobal::OpenFileDialog(const object& inout)
{
    //refact to platform
    return false;
}

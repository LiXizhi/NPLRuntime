//-----------------------------------------------------------------------------
// ParaScriptGlobalAndroid.cpp
// Authors: big
// Emails: onedous@gmail.com
// CreateDate: 2019.8.12
// ModifyDate: 2022.2.25
//-----------------------------------------------------------------------------

#include "ParaEngine.h"
#include "ParaScriptingGlobal.h"
#include "NPL/NPLScriptingState.h"
#include "ParaScriptingScreenRecorder.h"
#include <luabind/object.hpp>
#include <luabind/luabind.hpp>

static void LuabindRegisterScreenRecorderGlobalFunctions(lua_State *L)
{
    using namespace luabind;

    module(L)
    [
        namespace_("ScreenRecorder")
        [
            class_<ParaScripting::ParaScriptingScreenRecorder>("ParaScriptingScreenRecorder"),
            def("start", ParaScripting::ParaScriptingScreenRecorder::Start),
            def("stop", ParaScripting::ParaScriptingScreenRecorder::Stop),
            def("play", ParaScripting::ParaScriptingScreenRecorder::Play),
            def("save", ParaScripting::ParaScriptingScreenRecorder::Save)
        ]
    ];
}

static void LuabindRegisterAndroidSettingsGlobalFunctions(lua_State *L)
{
    using namespace luabind;
}

void ParaScripting::CNPLScriptingState::LoadHAPI_Platform()
{
    using namespace luabind;
    lua_State* L = GetLuaState();

    LuabindRegisterAndroidSettingsGlobalFunctions(L);
    LuabindRegisterScreenRecorderGlobalFunctions(L);
}

bool ParaScripting::ParaGlobal::OpenFileDialog(const object& inout)
{
    //refact to platform
    return false;
}

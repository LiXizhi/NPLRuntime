//-----------------------------------------------------------------------------
// Class: ParaScriptingGlobaliOS.cpp
// Authors: big
// CreateDate: 2019.08.12
// ModifyDate: 2021.12.14
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
            def("stop", ParaScripting::ParaScriptingScreenRecorder::Stop)
        ]
    ];
}

void ParaScripting::CNPLScriptingState::LoadHAPI_Platform()
{
    using namespace luabind;
    lua_State *L = GetLuaState();

    LuabindRegisterScreenRecorderGlobalFunctions(L);
}

bool ParaScripting::ParaGlobal::OpenFileDialog(const object& inout)
{
    //refact to platform
    return false;
}

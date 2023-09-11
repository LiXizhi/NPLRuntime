//-----------------------------------------------------------------------------
// Class: ParaScriptingGlobaliOS.cpp
// Authors: big
// Emails: onedous@gmail.com
// CreateDate: 2019.08.12
// ModifyDate: 2022.05.12
//-----------------------------------------------------------------------------

#include "FileDialogiOS.h"

#include "ParaEngine.h"
#include "ParaScriptingGlobal.h"
#include "NPL/NPLScriptingState.h"
#include "ParaScriptingScreenRecorder.h"
#include "ParaScriptingIDFA.h"
#include <luabind/object.hpp>
#include <luabind/luabind.hpp>

#ifdef NPLRUNTIME_OCE
namespace NplOceScripting
{
    int Register(lua_State *L);
}
#endif

 static void LuabindRegisterIDFAGlobalFunctions(lua_State *L)
 {
     using namespace luabind;

     module(L)
     [
         namespace_("IDFA")
         [
             class_<ParaScripting::ParaScriptingIDFA>("ParaScriptingIDFA"),
             def("get", ParaScripting::ParaScriptingIDFA::Get),
             def("requestTrackingAuthorization", ParaScripting::ParaScriptingIDFA::RequestTrackingAuthorization)
         ]
     ];
 }

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
            def("save", ParaScripting::ParaScriptingScreenRecorder::Save),
            def("removeVideo", ParaScripting::ParaScriptingScreenRecorder::RemoveVideo)
        ]
    ];
}

void ParaScripting::CNPLScriptingState::LoadHAPI_Platform()
{
    using namespace luabind;
    lua_State *L = GetLuaState();

    LuabindRegisterScreenRecorderGlobalFunctions(L);
    LuabindRegisterIDFAGlobalFunctions(L);
#ifdef NPLRUNTIME_OCE
    NplOceScripting::Register(L);
#endif
}

bool ParaScripting::ParaGlobal::OpenFileDialog(const object& inout)
{
    //refact to platform
    const char* initialdir = NULL;
    const char* filter = NULL;
    const char* title = NULL;
    bool save = false;
    bool multi = false;
    const char* activate_file = NULL;
    int callback_id = 0;

    for (luabind::iterator itCur(inout), itEnd; itCur != itEnd; ++itCur)
    {
        // we only serialize item with a string key
        const object& key = itCur.key();

        if (type(key) == LUA_TSTRING)
        {
            string sKey = object_cast<const char*>(key);
            const object& Value = *itCur;

            if (sKey == "initialdir")
            {
                initialdir = object_cast<const char*>(Value);
            }
            else if (sKey == "filter")
            {
                filter = object_cast<const char*>(Value);
            }
            else if (sKey == "title")
            {
                title = object_cast<const char*>(Value);
            }
            else if (sKey == "activate_file")
            {
                activate_file = object_cast<const char*>(Value);
            }
            else if (sKey == "callback_id")
            {
                callback_id = object_cast<int>(Value);
            }
            else if (sKey == "save")
            {
                if (object_cast<bool>(Value))
                {
                    save = true;
                }
            }
            else if (sKey == "multi")
            {
                if (object_cast<bool>(Value))
                {
                    multi = true;
                }
            }
        }
    }

    if (filter == NULL)
        filter = "image/*";

    FileDialogiOS::open(filter, callback_id, activate_file);

    return false;
}

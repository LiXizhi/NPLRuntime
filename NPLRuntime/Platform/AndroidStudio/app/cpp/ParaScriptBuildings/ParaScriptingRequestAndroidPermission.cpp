//-----------------------------------------------------------------------------
// Class: ParaScriptingRequestPermission.cpp
// Authors: big
// CreateDate: 2023.11.3
//-----------------------------------------------------------------------------

#include "ParaScriptingRequestAndroidPermission.h"
#include "../jni/JniHelper.h"

namespace ParaScripting
{
    const std::string ParaScriptingRequestAndroidPermission::classname = "com/tatfook/paracraft/RequestAndroidPermission";

    void ParaScriptingRequestAndroidPermission::LuabindRegisterRequestAndroidPermissionFunctions(lua_State *L)
    {
        using namespace luabind;

        module(L)
        [
            namespace_("RequestAndroidPermission")
            [
                class_<ParaScripting::ParaScriptingRequestAndroidPermission>("ParaScriptingRequestAndroidPermission"),
                def("RequestRecordAudioPermission", ParaScripting::ParaScriptingRequestAndroidPermission::RequestRecordAudioPermission)
            ]
        ];
    }

    void ParaScriptingRequestAndroidPermission::RequestRecordAudioPermission()
    {
        ParaEngine::JniHelper::callStaticBooleanMethod(classname, "RequestRecordAudio");
    }
}

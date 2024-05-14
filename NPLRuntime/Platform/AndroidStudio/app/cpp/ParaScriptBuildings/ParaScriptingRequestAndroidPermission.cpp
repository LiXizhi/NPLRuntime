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
                def("RequestRecordAudioPermission", ParaScripting::ParaScriptingRequestAndroidPermission::RequestRecordAudioPermission),
                def("HasPermission", ParaScripting::ParaScriptingRequestAndroidPermission::HasPermission),
                def("RequestPermission", ParaScripting::ParaScriptingRequestAndroidPermission::RequestPermission)
            ]
        ];
    }

    void ParaScriptingRequestAndroidPermission::RequestRecordAudioPermission()
    {
        ParaEngine::JniHelper::callStaticBooleanMethod(classname, "RequestRecordAudio");
    }

    bool ParaScriptingRequestAndroidPermission::HasPermission(int permissionCode)
    {
        return ParaEngine::JniHelper::callStaticBooleanMethod(classname, "HasPermission", permissionCode);
    }

    void ParaScriptingRequestAndroidPermission::RequestPermission(int permissionCode)
    {
        ParaEngine::JniHelper::callStaticVoidMethod(classname, "RequestPermission", permissionCode);
    }
}

//-----------------------------------------------------------------------------
// Class: ParaScriptingRequestPermission.h
// Authors: big
// CreateDate: 2023.11.3
//-----------------------------------------------------------------------------

#include "ParaEngine.h"
#include <luabind/object.hpp>
#include <luabind/luabind.hpp>

namespace ParaScripting
{
    class PE_CORE_DECL ParaScriptingRequestAndroidPermission
    {
    public:
        static void LuabindRegisterRequestAndroidPermissionFunctions(lua_State *L);
        static void RequestRecordAudioPermission();
    private:
        static const std::string classname;
    };
}

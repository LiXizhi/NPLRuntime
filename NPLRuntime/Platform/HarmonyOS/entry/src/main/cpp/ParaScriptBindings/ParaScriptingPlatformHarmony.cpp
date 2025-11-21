#include "ParaEngine.h"
#include "NPL/NPLScriptingState.h"
#include "luabind/luabind.hpp"

namespace ParaScripting
{
    using namespace luabind;

    void CNPLScriptingState::LoadHAPI_Platform()
    {
        // TODO: Load HarmonyOS-specific platform APIs
        // This is where platform-specific Lua bindings would be registered
    }
}

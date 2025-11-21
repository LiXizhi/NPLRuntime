#include "ParaEngine.h"
#include "ParaScriptBindings/ParaScriptingGlobal.h"
#include "luabind/luabind.hpp"

namespace ParaScripting
{
    using namespace luabind;

    object ParaGlobal::OpenFileDialog(const object& inout)
    {
        // TODO: Implement file dialog for HarmonyOS
        return object();
    }
}

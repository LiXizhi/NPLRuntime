#include "ParaEngine.h"
#include "ParaScriptBindings/ParaScriptingGlobal.h"
#include "luabind/luabind.hpp"

namespace ParaScripting
{
    using namespace luabind;

    bool ParaGlobal::OpenFileDialog(const object& inout)
    {
        // TODO: Implement file dialog for HarmonyOS
        return false;
    }
}

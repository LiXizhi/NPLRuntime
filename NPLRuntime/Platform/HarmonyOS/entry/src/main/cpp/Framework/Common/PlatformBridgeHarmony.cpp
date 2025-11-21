#include "ParaEngine.h"
#include "Framework/Common/PlatformBridge/PlatformBridge.h"

namespace ParaEngine
{
    std::string PlatformBridge::call_native(std::string funcName, std::string params)
    {
        // TODO: Implement native function calls for HarmonyOS
        return "";
    }

    std::string PlatformBridge::call_native_withCB(std::string funcName, LuaCB* cb, std::string params)
    {
        // TODO: Implement native function calls with callback for HarmonyOS
        return "";
    }
}

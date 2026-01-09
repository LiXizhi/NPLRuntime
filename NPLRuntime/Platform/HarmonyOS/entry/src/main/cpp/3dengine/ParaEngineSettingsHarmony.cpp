#include "ParaEngine.h"
#include "3dengine/ParaEngineSettings.h"

namespace ParaEngine
{
    const std::string& ParaEngineSettings::GetMachineID()
    {
        // TODO: Get unique machine ID for HarmonyOS
        static std::string machineID = "harmonyos-device-id";
        return machineID;
    }
}

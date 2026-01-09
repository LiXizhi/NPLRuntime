#include "ParaEngine.h"
#include "util/NetworkAdapter.h"

namespace ParaEngine
{
    void NetworkAdapter::Init()
    {
        // TODO: Initialize network adapter for HarmonyOS
    }

    const std::string& NetworkAdapter::GetMaxMacAddress()
    {
        // TODO: Get MAC address for HarmonyOS
        static std::string macAddress = "00:00:00:00:00:00";
        return macAddress;
    }

    const std::string& NetworkAdapter::GetMaxIPAddress()
    {
        // TODO: Get IP address for HarmonyOS  
        static std::string ipAddress = "127.0.0.1";
        return ipAddress;
    }
}

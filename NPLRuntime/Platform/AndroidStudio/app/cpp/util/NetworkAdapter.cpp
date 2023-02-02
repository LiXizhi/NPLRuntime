//-----------------------------------------------------------------------------
// NetworkAdapter.cpp
// Authors: LanZhiHong, big
// CreateDate: 2019.7.16
// ModifyDate: 2022.1.11
//-----------------------------------------------------------------------------

#include "ParaEngine.h"
#include "util/NetworkAdapter.h"
#include "jni/ParaEngineHelper.h"
#include <sstream>

namespace ParaEngine {
    void NetworkAdapter::Init()
    {
        if (m_bInitialized)
            return;

        m_bInitialized = true;
        m_sMacAddress = ParaEngineHelper::getWifiMAC();

        auto ip = ParaEngineHelper::getWifiIP();

        Color ipv4(ip);

        std::stringstream ss;
        ss << (int)ipv4.b << "." << (int)ipv4.g << "." << (int)ipv4.r << "." << (int)ipv4.a;
        m_sIpAddress = ss.str();
    }

    const std::string& NetworkAdapter::GetMaxMacAddress()
    {
        if(m_sMacAddress==""){
            m_sMacAddress = ParaEngineHelper::getWifiMAC();
        }
        return m_sMacAddress;
    }

    const std::string& NetworkAdapter::GetMaxIPAddress()
    {
        if(m_sIpAddress==""){
            auto ip = ParaEngineHelper::getWifiIP();

            Color ipv4(ip);

            std::stringstream ss;
            ss << (int)ipv4.b << "." << (int)ipv4.g << "." << (int)ipv4.r << "." << (int)ipv4.a;
            m_sIpAddress = ss.str();
        }
        return m_sIpAddress;
    }
}

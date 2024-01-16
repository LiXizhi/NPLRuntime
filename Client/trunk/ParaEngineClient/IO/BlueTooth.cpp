//-----------------------------------------------------------------------------
// Class: Blue Tooth
// Authors:	big
// Emails:	onedous@gmail.com
// Date:	2023.12.18
// Notes: 
//-----------------------------------------------------------------------------

#include "ParaEngine.h"
#include "NPLRuntime.h"
#include "NPLTable.h"
#include "NPLHelper.h"
#include "INPLRuntimeState.h"

#include <windows.devices.bluetooth.h>
#include <windows.devices.bluetooth.advertisement.h>
#include <windows.foundation.h>

#include "BlueTooth.h"

#ifdef WIN32

namespace ParaEngine
{
    static BlueTooth* g_bluetooth = NULL;

    BlueTooth::BlueTooth(void) {
    }

    BlueTooth::~BlueTooth(void) {
    }

    BlueTooth* BlueTooth::GetSingleton() {
        if (g_bluetooth != NULL) {
            return g_bluetooth;
        } else {
            g_bluetooth = new BlueTooth();
            return g_bluetooth;
        }
    }

    void BlueTooth::setDeviceName() {
        
    }

    void BlueTooth::setupBluetoothDelegate() {
    
    }

    void BlueTooth::readAllBlueGatt() {
    
    }

    void BlueTooth::disconnectBlueTooth() {
    
    }

    void BlueTooth::linkDevice() {
    
    }

    void BlueTooth::writeToCharacteristic() {
    
    }

    void BlueTooth::readCharacteristic() {
    
    }

    void BlueTooth::setCharacteristicNotification() {
    
    }

    void BlueTooth::setDescriptorNotification() {
    
    }
}

extern "C"
{
    PE_CORE_DECL NPL::NPLReturnCode NPL_activate_script_bluetooth_cpp(NPL::INPLRuntimeState *pState)
    {
        NPL::NPLObjectProxy msg = NPL::NPLHelper::MsgStringToNPLTable(pState->GetCurrentMsg(), pState->GetCurrentMsgLength());

        try
        {
            std::string cmd = msg["cmd"];
            
            if (cmd == "setDeviceName") {
                NPL::NPLObjectProxy args = msg["args"];
                std::string name = args["name"];

                OUTPUT_LOG(name.c_str());
            } else if (cmd == "setupBluetoothDelegate") {
                
            } else if (cmd == "readAllBlueGatt") {
            
            } else if (cmd == "disconnectBlueTooth") {

            } else if (cmd == "linkDevice") {
                
            } else if (cmd == "writeToCharacteristic") {
                
            } else if (cmd == "readCharacteristic") {
                
            } else if (cmd == "setCharacteristicNotification") {
                
            } else if (cmd == "setDescriptorNotification") {
                
            }

            
        }
        catch (...)
        {

        }

        return NPL::NPL_OK;
    }
}

#endif

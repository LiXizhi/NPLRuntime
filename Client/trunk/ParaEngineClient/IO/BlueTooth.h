//-----------------------------------------------------------------------------
// Class: Blue Tooth
// Authors:	big
// Emails:	onedous@gmail.com
// Date:	2023.12.18
// Notes: 
//-----------------------------------------------------------------------------

#pragma once

namespace ParaEngine
{
    class BlueTooth
    {
    public:
        BlueTooth();
        ~BlueTooth();

        static BlueTooth* GetSingleton();

        void setDeviceName();
        void setupBluetoothDelegate();
        void readAllBlueGatt();
        void disconnectBlueTooth();
        void linkDevice();
        void writeToCharacteristic();
        void readCharacteristic();
        void setCharacteristicNotification();
        void setDescriptorNotification();
    };
}

//-----------------------------------------------------------------------------
// Class: SerialPortMac.h
// Authors: big
// Emails: onedou@126.com
// Date: 2023.8.11
//-----------------------------------------------------------------------------

#include "ParaEngine.h"

namespace ParaEngine {
    class SerialPortMac
    {
    public:
        std::vector<std::string> GetPortNames();
    };
}

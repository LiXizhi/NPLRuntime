#include "LocalBridge_ios.h"
#include "ParaEngine.h"
#include "NPLRuntime.h"
#include "NPL/NPLHelper.h"
#include <string>
int LocalBridge::callLuaFunctionWithString(const char* key, const char* data)
{
        NPL::NPLObjectProxy msg;
        msg["key"] = key;
        msg["value"] = data;
        std::string strOutput;
        if (!NPL::NPLHelper::NPLTableToString("msg", msg, strOutput)) {
            return 0;
        }
        std::string activate_file = "script/mobile/API/local_service_wrapper.lua";
        if (ParaEngine::CGlobals::GetNPLRuntime()->GetRuntimeState("")->activate(activate_file.c_str(), strOutput.c_str(),strOutput.size())==0) {
            return 1;
        }else
        {
            return 0;
        }
}



//-----------------------------------------------------------------------------
// Class:	Android ParaEngineSettings
// Authors:	big
// Emails:	onedou@126.com
// Date:	2020.6.11
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "3dengine/ParaEngineSettings.h"
#include "jni/ParaEngineHelper.h"

namespace ParaEngine {

const std::string& ParaEngineSettings::GetMachineID()
{
    static std::string str = "";

    if (str.empty())
    {
        str = ParaEngineHelper::getMachineID();
    }

	return str;
}

} //namespace ParaEngine
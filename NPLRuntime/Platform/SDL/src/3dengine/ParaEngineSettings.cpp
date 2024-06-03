//-----------------------------------------------------------------------------
// Class:	Linux ParaEngineSettings
// Authors:	wxa
// Date:	2022.7.8
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ParaEngineSettings.h"

const std::string& ParaEngine::ParaEngineSettings::GetMachineID()
{
	static std::string machineID = "";
	return machineID;
}

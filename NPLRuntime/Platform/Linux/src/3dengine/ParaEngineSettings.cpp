//-----------------------------------------------------------------------------
// Class:	Linux ParaEngineSettings
// Authors:	big
// Emails:	onedou@126.com
// Date:	2020.6.15
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ParaEngineSettings.h"

const std::string& ParaEngine::ParaEngineSettings::GetMachineID()
{
	static const std::string s_empty;
	return s_empty;
}
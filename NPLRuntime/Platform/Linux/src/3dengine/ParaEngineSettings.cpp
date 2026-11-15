//-----------------------------------------------------------------------------
// Class:	Linux ParaEngineSettings
// Authors:	big
// Emails:	onedou@126.com
// Date:	2020.6.15
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ParaEngineSettings.h"
#include <fstream>

const std::string& ParaEngine::ParaEngineSettings::GetMachineID()
{
	static std::string s_machineID;
	if (!s_machineID.empty())
		return s_machineID;

	const char* idPaths[] = {
		"/etc/machine-id",
		"/var/lib/dbus/machine-id",
		"/sys/class/dmi/id/product_uuid"
	};
	for (const char* path : idPaths)
	{
		std::ifstream ifs(path);
		if (ifs.is_open())
		{
			std::string line;
			if (std::getline(ifs, line))
			{
				while (!line.empty() && (line.back() == '\n' || line.back() == '\r' || line.back() == ' '))
					line.pop_back();
				if (!line.empty())
				{
					s_machineID = line;
					break;
				}
			}
		}
	}
	if (s_machineID.empty())
	{
		s_machineID = GetMaxMacAddress();
	}
	return s_machineID;
}

//-----------------------------------------------------------------------------
// Class:	SDL2 / Linux / Emscripten ParaEngineSettings
// Authors:	wxa
// Date:	2022.7.8
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ParaEngineSettings.h"
#include <fstream>

#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif

const std::string& ParaEngine::ParaEngineSettings::GetMachineID()
{
	static std::string machineID;
	if (!machineID.empty())
		return machineID;

#ifdef EMSCRIPTEN
	char idBuf[128] = { 0 };
	EM_ASM({
		var id = "";
		try {
			id = window.localStorage.getItem("paraengine_machine_id") || "";
			if (!id) {
				id = "web-" + Math.random().toString(16).slice(2) + "-" + Date.now().toString(16);
				window.localStorage.setItem("paraengine_machine_id", id);
			}
		} catch (e) {
			id = "web-session-" + Math.random().toString(16).slice(2);
		}
		stringToUTF8(id, $0, 127);
	}, idBuf);
	machineID = idBuf;
#elif defined(__linux__)
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
					machineID = line;
					break;
				}
			}
		}
	}
	if (machineID.empty())
	{
		machineID = GetMaxMacAddress();
	}
#else
	machineID = GetMaxMacAddress();
#endif
	return machineID;
}

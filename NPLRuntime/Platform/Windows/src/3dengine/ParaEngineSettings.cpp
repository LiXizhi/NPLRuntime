//-----------------------------------------------------------------------------
// Class:	Windows ParaEngineSettings
// Authors:	big
// Emails:	onedou@126.com
// Date:	2020.6.11
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ParaEngineSettings.h"
#include "ParaScriptingGlobal.h"
#include "util/StringHelper.h"

namespace ParaEngine {

	const std::string& ParaEngineSettings::GetMachineID()
	{
		static std::string str = "";
		if (!str.empty())
		{
			return str;
		}
		str = GetMachineID_old();

		std::string cpu_ProcessorId;
		ParaScripting::ParaGlobal::ExecWmicCmd1(cpu_ProcessorId, "wmic cpu get ProcessorId", "ProcessorId");
		std::string csproduct_id;
		ParaScripting::ParaGlobal::ExecWmicCmd1(csproduct_id, "wmic csproduct get IdentifyingNumber", "IdentifyingNumber");

		if (cpu_ProcessorId.empty())
		{
			cpu_ProcessorId = "0";
		}
		if (csproduct_id.empty())
		{
			csproduct_id = "0";
		}
		if (str.empty())
		{
			std::string csproduct_uuid;
			ParaScripting::ParaGlobal::ExecWmicCmd1(csproduct_uuid, "wmic csproduct get UUID", "UUID");
			if (csproduct_uuid.empty())
			{
				csproduct_uuid = "0";
			}
			str = csproduct_uuid;
		}
		int pos = static_cast<int>(str.length());
		if (pos > 36)
		{
			pos -= 36;
			str = str.substr(pos);
		}

		std::string macadress = GetMaxMacAddress();
		std::string md5Str = ParaEngine::StringHelper::md5(cpu_ProcessorId + "_" + csproduct_id);
		str = md5Str + "_" + str + "_" + macadress;

		if (str.empty())
		{
			str = GetMaxMacAddress();
		}
		return str;
	}

} // namespace ParaEngine

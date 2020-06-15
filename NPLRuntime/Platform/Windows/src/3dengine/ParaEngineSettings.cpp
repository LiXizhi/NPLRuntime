//-----------------------------------------------------------------------------
// Class:	Windows ParaEngineSettings
// Authors:	big
// Emails:	onedou@126.com
// Date:	2020.6.11
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ParaEngineSettings.h"

namespace ParaEngine {

	struct dmi_header
	{
		BYTE type;
		BYTE length;
		WORD handle;
	};

	struct RawSMBIOSData
	{
		BYTE Used20CallingMethod;
		BYTE    SMBIOSMajorVersion;
		BYTE    SMBIOSMinorVersion;
		BYTE    DmiRevision;
		DWORD   Length;
		BYTE    SMBIOSTableData[1];
	};

	static void dmi_system_uuid(std::string& ret, const BYTE* p, short ver)
	{
		int only0xFF = 1, only0x00 = 1;
		
		for (int i = 0; i < 16 && (only0x00 || only0xFF); i++)
		{
			if (p[i] != 0x00) only0x00 = 0;
			if (p[i] != 0xFF) only0xFF = 0;
		}

		if (only0xFF)
		{
			OUTPUT_LOG("dmi_system_uuid Not Present\n");
			return;
		}

		if (only0x00)
		{
			OUTPUT_LOG("dmi_system_uuid Not Settable\n");
			return;
		}

		char str[64];

		if (ver >= 0x0206)
		{
			snprintf(str, 64, "%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X", p[3], p[2], p[1], p[0], p[5], p[4], p[7], p[6], p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15]);
		}
		else
		{
			snprintf(str, 64, "-%02X%02X%02X%02X-%02X%02X-%02X%02X-%02X%02X-%02X%02X%02X%02X%02X%02X", p[0], p[1], p[2], p[3], p[4], p[5], p[6], p[7], p[8], p[9], p[10], p[11], p[12], p[13], p[14], p[15]);
		}

		ret = str;
	}

	const std::string& ParaEngineSettings::GetMachineID()
	{
		static std::string str = "";

		if (str.empty())
		{
			DWORD bufSize = 0;
			BYTE buf[65535] = { 0 };

			RawSMBIOSData *Smbios;
			dmi_header *h = nullptr;
			int flag = 1;

			do
			{
				auto ret = GetSystemFirmwareTable('RSMB', 0, 0, 0);
				if (!ret)
				{
					OUTPUT_LOG("GetSystemFirmwareTable function init failed\n");
					break;
				}

				bufSize = ret;

				ret = GetSystemFirmwareTable('RSMB', 0, buf, bufSize);

				if (!ret)
				{
					OUTPUT_LOG("GetSystemFirmwareTable function failed\n");
					break;
				}

				Smbios = (RawSMBIOSData *)buf;
				BYTE *p = Smbios->SMBIOSTableData;

				if (Smbios->Length != bufSize - 8)
				{
					OUTPUT_LOG("Smbios length error\n");
					break;
				}

				for (DWORD i = 0; i < Smbios->Length; i++)
				{
					h = (dmi_header *)p;

					if (h->type == 0 && flag)
					{
						flag = 0;
					}
					else if (h->type == 1)
					{
						dmi_system_uuid(str, p + 0x8, Smbios->SMBIOSMajorVersion * 0x100 + Smbios->SMBIOSMinorVersion);
						break;
					}


					p += h->length;
					while ((*(WORD *)p) != 0)
					{
						p++;
					}
					p += 2;
				}

			} while (false);
		}

		return str;
	}

} // namespace ParaEngine
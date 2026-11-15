//-----------------------------------------------------------------------------
// Class: CEditorHelper
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2006.10
// Notes: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#undef ShellExecute
#include "NPLCommon.h"
#include "ParaEngineSettings.h"
#include "memdebug.h"

#include "EditorHelper.h"
#include <time.h>

namespace ParaEngine {


	bool CEditorHelper::SearchFileNameInScript(string & output, const char* sScript, bool bRelativePath)
	{
		if (sScript == 0)
			return false;
		char sScriptFile[MAX_PATH];
		memset(sScriptFile, 0, sizeof(sScriptFile));

		int i = 0;
		for (; sScript[i] != '\0' && sScript[i] != '"'; ++i)
		{
		}
		const char str[] = "NPL.load(\"";
		const int nStrLen = sizeof(str) - 2;
		if (nStrLen <= i &&  sScript[i] == '"')
		{
			int k = 0;
			for (; k < nStrLen && sScript[i - nStrLen + k] == str[k]; ++k)
			{
			}
			if (k == nStrLen)
			{
				int j = i + 1;
				for (; sScript[j] != '\0' && sScript[j] != '"'; ++j)
				{
				}
				if (sScript[j] == '"' && MAX_PATH > (j - i))
				{
					for (k = i + 1; k < j; ++k)
					{
						sScriptFile[k - i - 1] = sScript[k];
					}
				}
			}
		}
		if (sScriptFile[0] == '\0')
		{
			output.clear();
			return false;
		}
		else
		{
			output = sScriptFile;
			if (bRelativePath)
			{
				NPL::NPLFileName nplFileName(output.c_str());
				output = nplFileName.sRelativePath;
			}
			return true;
		}
	}

	const char* CEditorHelper::SearchFileNameInScript_(const char* sScript, bool bRelativePath)
	{
		static string g_str;
		SearchFileNameInScript(g_str, sScript, bRelativePath);
		return g_str.c_str();
	}

	bool CEditorHelper::CreateEmptyCharacterEventFileShared(const char* scriptFile, const char* sName)
	{
		if (!scriptFile || !sName)
			return false;

		string sScriptFile;
		CPathReplaceables::GetSingleton().DecodePath(sScriptFile, scriptFile);
		if (!CParaFile::DoesFileExist(sScriptFile.c_str(), false))
		{
			CParaFile file;
			if (file.CreateNewFile(sScriptFile.c_str(), true))
			{
				int nSize = (int)strlen(sName);
				char name[MAX_PATH];
				if (nSize >= MAX_PATH - 2)
					nSize = MAX_PATH - 3;

				int j = 0;
				for (int i = 0; i < nSize && j < MAX_PATH - 2; ++i, ++j)
				{
					char c = sName[i];
					if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z'))
					{
						name[j] = c;
					}
					else if (c >= '0' && c <= '9')
					{
						if (i == 0)
						{
							name[j] = '_';
							j++;
						}
						name[j] = c;
					}
					else
					{
						name[j] = '_';
					}
				}
				name[j] = '\0';

				CParaFile cTemplate(":IDR_NPL_CHARACTER_EVENTS");
				if (!cTemplate.isEof() && cTemplate.getBuffer() != nullptr)
				{
					int bufSize = (int)(cTemplate.getSize() + j * 20 + 128);
					std::vector<char> buffer(bufSize, 0);
					time_t ltime = 0;
					time(&ltime);
					const char* timeStr = ctime(&ltime);
					snprintf(buffer.data(), bufSize, cTemplate.getBuffer(), name, timeStr ? timeStr : "",
						name, name, name, name, name, name, name, name, name);
					file.write(buffer.data(), (int)strlen(buffer.data()));
				}
				return true;
			}
		}
		return false;
	}

} // end namespace
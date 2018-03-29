//-----------------------------------------------------------------------------
// Class: CEditorHelper
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2006.10
// Notes: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
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

} // end namespace
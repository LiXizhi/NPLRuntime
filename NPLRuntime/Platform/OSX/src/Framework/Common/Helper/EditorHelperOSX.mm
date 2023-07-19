#import <Cocoa/Cocoa.h>
#include "ParaEngine.h"
#include "Framework/Common/Helper/EditorHelper.h"
#include "WebView/WebView.h"

namespace ParaEngine {

	bool CEditorHelper::OpenWithDefaultEditor(const char* sScriptFile, bool bWaitOnReturn)
	{
		return false;
	}

	bool CEditorHelper::CreateEmptyCharacterEventFile(const char* scriptFile, const char* sName)
	{
		return false;
	}

	bool CEditorHelper::CreateProcess(const char* lpApplicationName, const char* lpCommandLine, bool bWaitOnReturn)
	{
		return false;
	}

	static bool openUrl(const char* url)
	{
        ParaEngineWebView::openWebView(url);

		return true;
	}

	static bool execute(const char* lpFile, const char* lpParameters, const char* lpDirectory, int nShowCmd)
	{
        std::string cmd = "open ";

		if (strcmp(lpFile, "explorer.exe") == 0)
			cmd += lpParameters;
		else
			cmd += lpFile;

        system(cmd.c_str());

        return true;
	}

	bool CEditorHelper::ShellExecute(const char* lpOperation, const char* lpFile, const char* lpParameters, const char* lpDirectory, int nShowCmd)
	{
 		if (strcmp(lpOperation, "open") == 0)
		{
			bool isUrl = false;

			auto len = strlen(lpFile);

			if (len > 4)
			{
				if (lpFile[0] == 'h' && lpFile[1] == 't' && lpFile[2] == 't' && lpFile[3] == 'p')
				{
					if (lpFile[4] == ':')
						isUrl = true;
					else
					{
						if (len > 5 && lpFile[4] == 's' && lpFile[5] == ':')
							isUrl = true;
					}
				}
			}

			if (isUrl)
			{
				return openUrl(lpFile);
			}
			else
			{
				return execute(lpFile, lpParameters, lpDirectory, nShowCmd);
			}

		}
        else if(strcmp(lpOperation, "openExternalBrowser") == 0)
        {
            NSString* sUrl = [NSString stringWithCString:lpFile encoding:[NSString defaultCStringEncoding]];
            [[NSWorkspace sharedWorkspace] openURL: [NSURL URLWithString:sUrl]];
            [sUrl release];
        }
		else
		{

		}

		return false;
	}
} // end namespace

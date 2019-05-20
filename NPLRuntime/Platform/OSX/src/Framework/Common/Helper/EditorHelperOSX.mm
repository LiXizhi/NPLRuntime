#include "ParaEngine.h"
#include "Framework/Common/Helper/EditorHelper.h"
#include "IParaWebView.h"
#include <Cocoa/Cocoa.h>

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
        std::string url_(url);
        const bool USE_BUILDIN_BROWSER_FOR_LOCALHOST = true;
        if(USE_BUILDIN_BROWSER_FOR_LOCALHOST && (url_.find("http://localhost") == 0 || url_.find("http://127.0.0.1") == 0))
        {
            auto pWnd = CGlobals::GetApp()->GetRenderWindow();
            int w = pWnd->GetWidth();
            int h = pWnd->GetHeight();
            auto scaleX = pWnd->GetScaleX();
            auto scaleY = pWnd->GetScaleY();
            
            auto pView = IParaWebView::createWebView(0, 0, w / scaleX, h / scaleY);
            if (!pView)
                return false;
            
            pView->loadUrl(url);
            pView->setAlpha(0.95f);
        }
        else
        {
            NSString* sUrl = [NSString stringWithCString:url encoding:[NSString defaultCStringEncoding]];
            [[NSWorkspace sharedWorkspace] openURL: [NSURL URLWithString:sUrl]];
            [sUrl release];
        }
		return true;
	}

    
	static bool execute(const char* lpFile, const char* lpParameters, const char* lpDirectory, int nShowCmd)
	{
        std::string cmd =  "open ";
		
		if (strcmp(lpParameters, "explorer.exe") == 0)
			cmd += lpDirectory;
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
		else
		{

		}

		return false;
	}
} // end namespace

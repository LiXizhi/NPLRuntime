#include "ParaEngine.h"
#include "Framework/Common/Helper/EditorHelper.h"
#include "jni/ParaEngineWebViewHelper.h"

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
		auto pWnd = CGlobals::GetApp()->GetRenderWindow();
		int w = pWnd->GetWidth();
		int h = pWnd->GetHeight();

		auto pView = ParaEngineWebView::createWebView(-1, -1, w, h);
		if (!pView)
			return false;

		pView->setAlpha(0.95f);
		pView->loadUrl(url);
		
		return true;
	}

	static bool execute(const char* lpFile, const char* lpParameters, const char* lpDirectory, int nShowCmd)
	{

		return false;
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
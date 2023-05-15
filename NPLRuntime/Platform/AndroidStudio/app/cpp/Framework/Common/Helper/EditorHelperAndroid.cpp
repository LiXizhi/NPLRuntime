//-----------------------------------------------------------------------------
// EditorHelperAndroid.cpp
// Authors: LanZhiHong, big
// CreateDate: 2019.12.30
// ModifyDate: 2022.1.11
//-----------------------------------------------------------------------------

#include "ParaEngine.h"
#include "Framework/Common/Helper/EditorHelper.h"
#include "jni/ParaEngineWebViewHelper.h"
#include "ParaScriptingGlobal.h"
#include "NPLHelper.h"

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

	static bool openUrl(const char *url, int x, int y, int width, int height)
	{
		ParaEngineWebView::openWebView(x, y, width, height, url);

		return true;
	}

	static bool execute(const char* lpFile, const char* lpParameters, const char* lpDirectory, int nShowCmd)
	{

		return false;
	}


	bool CEditorHelper::ShellExecute(const char* lpOperation, const char* lpFile, const char* lpParameters, const char* lpDirectory, int nShowCmd)
	{
	    if (strcmp(lpOperation, "openExternalBrowser") == 0)
        {
	        ParaEngineWebView::openExternalBrowser(lpFile);
	        return true;
        }

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

			if (isUrl) {
				auto pWnd = CGlobals::GetApp()->GetRenderWindow();
				int width = pWnd->GetWidth();
				int height = pWnd->GetHeight();
				int x = 0;
				int y = 0;

				//OUTPUT_LOG("hyz  lpParameters=%s",lpParameters);
				auto obj = NPL::NPLHelper::StringToNPLTable(lpParameters);
				//OUTPUT_LOG("hyz zzz:%d,x:%d",obj.GetType(),obj["x"].GetType());
				if (obj.GetType() == NPL::NPLObjectBase::NPLObjectType_Table){
					if (obj["x"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number) {
						x = obj["x"].toInt();
					}if (obj["y"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number) {
						y = obj["y"].toInt();
					}if (obj["width"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number) {
						width = obj["width"].toInt();
					}if (obj["height"].GetType() == NPL::NPLObjectBase::NPLObjectType_Number) {
						height = obj["height"].toInt();
					}
				}

				return openUrl(lpFile, x, y, width, height);
			} else {
				return execute(lpFile, lpParameters, lpDirectory, nShowCmd);
			}
		}
		else
		{

		}

		return false;
	}
} // end namespace
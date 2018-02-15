//-----------------------------------------------------------------------------
// Class:	Common File Dialog
// Authors:	LiXizhi
// Company: ParaEngine, TatFook
// Date: common file dialog
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#if defined(WIN32) && !defined(PARAENGINE_MOBILE)
#include <shlobj.h>

#include "CommonFileDialog.h"

using namespace ParaEngine;

CCommonFileDialog::CCommonFileDialog()
{
}


CCommonFileDialog::~CCommonFileDialog()
{
}

INT CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg,
	LPARAM lp, LPARAM pData)
{
	TCHAR szDir[MAX_PATH];

	switch (uMsg)
	{
	case BFFM_INITIALIZED:
		if (pData) 
			strcpy(szDir, (TCHAR *)pData);
		else 
			GetCurrentDirectory(sizeof(szDir) / sizeof(TCHAR), szDir);

		//selects the specified folder 
		//in the Browse For Folder dialog box
		SendMessage(hwnd, BFFM_SETSELECTION,
			TRUE, (LPARAM)szDir);

		break;

	case BFFM_SELCHANGED:
		if (SHGetPathFromIDList(
			(LPITEMIDLIST)lp, szDir))
		{
			//sets the status text 
			//in the Browse For Folder dialog box
			SendMessage(hwnd, BFFM_SETSTATUSTEXT,
				0, (LPARAM)szDir);
		}
		break;
	}
	return 0;
}

const char* ParaEngine::CCommonFileDialog::OpenFolder(const char* sDefaultFolder)
{
	SetDefaultOpenFileFolder(sDefaultFolder);

	HWND hWnd = CGlobals::GetAppHWND();
	HRESULT hr = S_OK;

	static TCHAR szwNewPath[1024];

	BROWSEINFO bInfo;
	bInfo.hwndOwner = hWnd;
	bInfo.pidlRoot = NULL;
	bInfo.pszDisplayName = szwNewPath;
	bInfo.lpszTitle = _T("Select a folder");

	bInfo.ulFlags = BIF_NONEWFOLDERBUTTON
		| BIF_NEWDIALOGSTYLE
		| BIF_EDITBOX
		//| BIF_SHAREABLE
		//| BIF_RETURNFSANCESTORS
		//| BIF_USENEWUI
		| BIF_RETURNONLYFSDIRS;

	bInfo.lpfn = BrowseCallbackProc;
	bInfo.lParam = (LPARAM)(m_sDefaultFolder.c_str());

	LPITEMIDLIST lpItem = SHBrowseForFolder(&bInfo);

	if (lpItem != NULL)
	{
		SHGetPathFromIDList(lpItem, szwNewPath);
		CoTaskMemFree(lpItem);
		return szwNewPath;
	}
	else 
		return NULL;
	
}

CCommonFileDialog* ParaEngine::CCommonFileDialog::GetSingleton()
{
	static CCommonFileDialog s_dlg;
	return &s_dlg;
}

void ParaEngine::CCommonFileDialog::SetDefaultOpenFileFolder(const char* sDefaultOpenFileFolder)
{
	if (sDefaultOpenFileFolder != NULL)
		m_sDefaultFolder = sDefaultOpenFileFolder;
}

const char* ParaEngine::CCommonFileDialog::OpenFileDialog(const char* sDefaultFolder /*= NULL*/)
{
	//HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED |
	//	COINIT_DISABLE_OLE1DDE);
	//if (SUCCEEDED(hr))
	//{
	//	IFileOpenDialog *pFileOpen;

	//	// Create the FileOpenDialog object.
	//	hr = CoCreateInstance(CLSID_FileOpenDialog, NULL, CLSCTX_ALL,
	//		IID_IFileOpenDialog, reinterpret_cast<void**>(&pFileOpen));

	//	if (SUCCEEDED(hr))
	//	{
	//		// Show the Open dialog box.
	//		hr = pFileOpen->Show(NULL);

	//		// Get the file name from the dialog box.
	//		if (SUCCEEDED(hr))
	//		{
	//			IShellItem *pItem;
	//			hr = pFileOpen->GetResult(&pItem);
	//			if (SUCCEEDED(hr))
	//			{
	//				PSTR pszFilePath;
	//				hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);

	//				// Display the file name to the user.
	//				if (SUCCEEDED(hr))
	//				{
	//					static std::string s_output;
	//					s_output = pszFilePath;

	//					CoTaskMemFree(pszFilePath);
	//					return s_output.c_str();
	//				}
	//				pItem->Release();
	//			}
	//		}
	//		pFileOpen->Release();
	//	}
	//	CoUninitialize();
	//}
	return NULL;
}

const char* ParaEngine::CCommonFileDialog::GetDirectory(const char* sKnownDir)
{
	char path[MAX_PATH + 1];
	if (!sKnownDir)
		return NULL;
	std::string sDir = sKnownDir;
	static std::string s_filepath;
	if (sDir == "desktop")
	{
		if (SHGetSpecialFolderPath(HWND_DESKTOP, path, CSIDL_DESKTOPDIRECTORY, FALSE))
		{
			s_filepath = path;
			s_filepath += "\\";
			return s_filepath.c_str();
		}
	}
	else if (sDir == "desktop_virtual")
	{
		if (SHGetSpecialFolderPath(HWND_DESKTOP, path, CSIDL_DESKTOP, FALSE))
		{
			s_filepath = path;
			s_filepath += "\\";
			return s_filepath.c_str();
		}
	}
	else
	{
		// TODO: for other shell directories. 
	}
	
	return NULL;
}

#endif
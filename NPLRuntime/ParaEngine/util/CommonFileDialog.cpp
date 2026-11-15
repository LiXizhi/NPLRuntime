//-----------------------------------------------------------------------------
// Class:	Common File Dialog
// Authors:	LiXizhi
// Company: ParaEngine, TatFook
// Date: common file dialog
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "CommonFileDialog.h"
#include "util/StringHelper.h"
#include <cstdlib>
#include <cstdio>

#if defined(WIN32) && !defined(PARAENGINE_MOBILE) && !defined(NPLRUNTIME_SERVER)
#include <shlobj.h>
#endif

using namespace ParaEngine;

CCommonFileDialog::CCommonFileDialog()
{
}

CCommonFileDialog::~CCommonFileDialog()
{
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
	return NULL;
}

#if defined(WIN32) && !defined(PARAENGINE_MOBILE) && !defined(NPLRUNTIME_SERVER)
INT CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg,
	LPARAM lp, LPARAM pData)
{
	WCHAR szDir[MAX_PATH + 1] = { 0 };

	switch (uMsg)
	{
	case BFFM_INITIALIZED:
		if (pData && ((const WCHAR*)pData)[0] != L'\0')
			wcsncpy_s(szDir, (const WCHAR*)pData, MAX_PATH);
		else
			GetCurrentDirectoryW(MAX_PATH, szDir);

		//selects the specified folder 
		//in the Browse For Folder dialog box
		SendMessageW(hwnd, BFFM_SETSELECTIONW,
			TRUE, (LPARAM)szDir);

		break;

	case BFFM_SELCHANGED:
		if (SHGetPathFromIDListW(
			(LPITEMIDLIST)lp, szDir))
		{
			//sets the status text 
			//in the Browse For Folder dialog box
			SendMessageW(hwnd, BFFM_SETSTATUSTEXTW,
				0, (LPARAM)szDir);
		}
		break;
	}
	return 0;
}
#endif

const char* ParaEngine::CCommonFileDialog::OpenFolder(const char* sDefaultFolder)
{
	SetDefaultOpenFileFolder(sDefaultFolder);
#if defined(WIN32) && !defined(PARAENGINE_MOBILE) && !defined(NPLRUNTIME_SERVER)
	HWND hWnd = CGlobals::GetAppHWND();

	WCHAR szwNewPath[MAX_PATH + 1] = { 0 };
	std::wstring wDefaultFolder;
	if (!m_sDefaultFolder.empty())
	{
		wDefaultFolder = StringHelper::MultiByteToWideChar(m_sDefaultFolder.c_str(), DEFAULT_FILE_ENCODING);
	}

	BROWSEINFOW bInfo = { 0 };
	bInfo.hwndOwner = hWnd;
	bInfo.pidlRoot = NULL;
	bInfo.pszDisplayName = szwNewPath;
	bInfo.lpszTitle = L"Select a folder";

	bInfo.ulFlags = BIF_NONEWFOLDERBUTTON
		| BIF_NEWDIALOGSTYLE
		| BIF_EDITBOX
		| BIF_RETURNONLYFSDIRS;

	bInfo.lpfn = BrowseCallbackProc;
	bInfo.lParam = wDefaultFolder.empty() ? 0 : (LPARAM)(wDefaultFolder.c_str());

	LPITEMIDLIST lpItem = SHBrowseForFolderW(&bInfo);

	if (lpItem != NULL)
	{
		BOOL bOk = SHGetPathFromIDListW(lpItem, szwNewPath);
		CoTaskMemFree(lpItem);
		if (bOk)
		{
			static std::string s_output;
			s_output = StringHelper::WideCharToMultiByte(szwNewPath, DEFAULT_FILE_ENCODING);
			return s_output.c_str();
		}
	}
	return NULL;
#elif defined(__linux__) && !defined(ANDROID) && !defined(EMSCRIPTEN) && !defined(NPLRUNTIME_SERVER)
	auto shellQuote = [](const std::string& s) -> std::string {
		std::string out = "'";
		for (char c : s)
		{
			if (c == '\'')
				out += "'\\''";
			else
				out += c;
		}
		out += "'";
		return out;
	};
	std::string cmd = "zenity --file-selection --directory";
	if (!m_sDefaultFolder.empty())
		cmd += " --filename=" + shellQuote(m_sDefaultFolder);
	cmd += " 2>/dev/null";
	FILE* fp = popen(cmd.c_str(), "r");
	std::string result;
	if (fp)
	{
		char buf[1024];
		while (fgets(buf, sizeof(buf), fp))
			result.append(buf);
		int rc = pclose(fp);
		if (rc != 0)
			result.clear();
	}
	if (result.empty())
	{
		cmd = "kdialog --getexistingdirectory";
		if (!m_sDefaultFolder.empty())
			cmd += " " + shellQuote(m_sDefaultFolder);
		cmd += " 2>/dev/null";
		fp = popen(cmd.c_str(), "r");
		if (fp)
		{
			char buf[1024];
			while (fgets(buf, sizeof(buf), fp))
				result.append(buf);
			int rc = pclose(fp);
			if (rc != 0)
				result.clear();
		}
	}
	while (!result.empty() && (result.back() == '\n' || result.back() == '\r'))
		result.pop_back();
	if (!result.empty())
	{
		static std::string s_output;
		s_output = result;
		return s_output.c_str();
	}
	return NULL;
#elif defined(__APPLE__) && !defined(PARAENGINE_MOBILE) && !defined(NPLRUNTIME_SERVER)
	FILE* fp = popen("osascript -e 'POSIX path of (choose folder with prompt \"Select a folder\")' 2>/dev/null", "r");
	if (fp)
	{
		std::string result;
		char buf[1024];
		while (fgets(buf, sizeof(buf), fp))
			result.append(buf);
		int rc = pclose(fp);
		while (!result.empty() && (result.back() == '\n' || result.back() == '\r'))
			result.pop_back();
		if (rc == 0 && !result.empty())
		{
			static std::string s_output;
			s_output = result;
			return s_output.c_str();
		}
	}
	return NULL;
#else
	return NULL;
#endif
}

const char* ParaEngine::CCommonFileDialog::GetDirectory(const char* sKnownDir)
{
	if (!sKnownDir)
		return NULL;
	std::string sDir = sKnownDir;
	static std::string s_filepath;

#if defined(WIN32) && !defined(PARAENGINE_MOBILE) && !defined(NPLRUNTIME_SERVER)
	WCHAR path[MAX_PATH + 1] = { 0 };
	int csidl = -1;
	if (sDir == "desktop")
		csidl = CSIDL_DESKTOPDIRECTORY;
	else if (sDir == "desktop_virtual")
		csidl = CSIDL_DESKTOP;
	else if (sDir == "mydocuments")
		csidl = CSIDL_PERSONAL;
	else if (sDir == "favorites")
		csidl = CSIDL_FAVORITES;
	else if (sDir == "music")
		csidl = CSIDL_MYMUSIC;
	else if (sDir == "pictures")
		csidl = CSIDL_MYPICTURES;
	else if (sDir == "videos")
		csidl = CSIDL_MYVIDEO;

	if (csidl >= 0 && SHGetSpecialFolderPathW(HWND_DESKTOP, path, csidl, FALSE))
	{
		s_filepath = StringHelper::WideCharToMultiByte(path, DEFAULT_FILE_ENCODING);
		s_filepath += "\\";
		return s_filepath.c_str();
	}
	return NULL;
#elif !defined(EMSCRIPTEN) && !defined(PARAENGINE_MOBILE)
	const char* homeEnv = std::getenv("HOME");
	if (!homeEnv || homeEnv[0] == '\0')
		return NULL;
	std::string home(homeEnv);
	if (home.back() != '/')
		home += '/';

	const char* xdgEnv = nullptr;
	std::string subFolder;
	if (sDir == "desktop" || sDir == "desktop_virtual")
	{
		xdgEnv = std::getenv("XDG_DESKTOP_DIR");
		subFolder = "Desktop/";
	}
	else if (sDir == "mydocuments")
	{
		xdgEnv = std::getenv("XDG_DOCUMENTS_DIR");
		subFolder = "Documents/";
	}
	else if (sDir == "music")
	{
		xdgEnv = std::getenv("XDG_MUSIC_DIR");
		subFolder = "Music/";
	}
	else if (sDir == "pictures")
	{
		xdgEnv = std::getenv("XDG_PICTURES_DIR");
		subFolder = "Pictures/";
	}
	else if (sDir == "videos")
	{
		xdgEnv = std::getenv("XDG_VIDEOS_DIR");
#if defined(__APPLE__)
		subFolder = "Movies/";
#else
		subFolder = "Videos/";
#endif
	}
	else if (sDir == "favorites")
	{
		subFolder = "";
	}
	else
	{
		return NULL;
	}

	if (xdgEnv && xdgEnv[0] != '\0')
	{
		s_filepath = xdgEnv;
		if (s_filepath.back() != '/')
			s_filepath += '/';
	}
	else
	{
		s_filepath = home + subFolder;
	}
	return s_filepath.c_str();
#else
	return NULL;
#endif
}

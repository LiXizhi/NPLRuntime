//-----------------------------------------------------------------------------
// Class: For ParaGlobal namespace.
// Authors: Big
// Emails: onedous@gmail.com
// Date: 2019.8.12
//-----------------------------------------------------------------------------

#include "ParaEngine.h"
#include "ParaScriptingGlobal.h"
#include <luabind/object.hpp>

bool ParaScripting::ParaGlobal::OpenFileDialog(const object& inout)
{
#ifdef PARAENGINE_CLIENT
	if (type(inout) != LUA_TTABLE)
	{
		return false;
	}
	// OpenFileDialog
	OPENFILENAME ofn = { 0 };
	memset(&ofn, 0, sizeof(ofn));
	char szFileName[MAX_LINE] = { 0 };
	ofn.lStructSize = sizeof(OPENFILENAME);
	ofn.lpstrFile = szFileName;
	ofn.lpstrFilter = "All Files (*.*)\0*.*\0";
	ofn.nFilterIndex = 0;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
	ofn.nMaxFile = MAX_LINE;
	bool bIsSavingFile = false;
	for (luabind::iterator itCur(inout), itEnd; itCur != itEnd; ++itCur)
	{
		// we only serialize item with a string key
		const object& key = itCur.key();
		if (type(key) == LUA_TSTRING)
		{
			string sKey = object_cast<const char*>(key);
			const object& Value = *itCur;
			if (sKey == "initialdir")
			{
				ofn.lpstrInitialDir = object_cast<const char*>(Value);
			}
			else if (sKey == "filter")
			{
				ofn.lpstrFilter = object_cast<const char*>(Value);
			}
			else if (sKey == "title")
			{
				ofn.lpstrTitle = object_cast<const char*>(Value);
			}
			else if (sKey == "save")
			{
				if (object_cast<bool>(Value))
				{
					bIsSavingFile = true;
					ofn.Flags &= ~OFN_FILEMUSTEXIST;
				}
			}
			else if (sKey == "multi")
			{
				if (object_cast<bool>(Value))
				{
					ofn.Flags |= OFN_ALLOWMULTISELECT | OFN_EXPLORER;
				}
			}
		}
	}
	char buf[MAX_LINE + 1] = { 0 };
	int nCount = GetCurrentDirectory(MAX_LINE, buf);

	// switch to windowed mode to display the win32 common dialog.
	bool bOldWindowed = CGlobals::GetApp()->IsWindowedMode();  // Preserve original windowed flag
	if (bOldWindowed == false)
	{
		CGlobals::GetApp()->SetWindowedMode(true);
	}

	bool bResult = bIsSavingFile ? (!!::GetSaveFileName(&ofn)) : (!!::GetOpenFileName(&ofn));

	if (bOldWindowed == false)
	{
		CGlobals::GetApp()->SetWindowedMode(false);
	}

	// reset directory. 
	if (nCount > 0)
		SetCurrentDirectory(buf);

	inout["result"] = bResult;
	if (bResult)
	{
		if (ofn.lpstrFile)
		{
			if ((ofn.Flags & OFN_ALLOWMULTISELECT) != 0)
			{
				for (int i = 0; i < MAX_LINE; ++i)
				{
					if (ofn.lpstrFile[i] == 0 && ofn.lpstrFile[i + 1] != 0)
						ofn.lpstrFile[i] = '|';
}
			}
			string filename = ofn.lpstrFile;
			inout["filename"] = filename;
		}
	}
	return bResult;
#else
	return false;
#endif
}
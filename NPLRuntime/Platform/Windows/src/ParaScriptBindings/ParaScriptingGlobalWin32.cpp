//-----------------------------------------------------------------------------
// Class: For ParaGlobal namespace.
// Authors: big
// Emails: onedous@gmail.com
// CreateDate: 2019.8.12
// ModifyDate: 2022.1.11
//-----------------------------------------------------------------------------

#include "ParaEngine.h"
#include "ParaScriptingGlobal.h"
#include "NPL/NPLScriptingState.h"
#include "util/StringHelper.h"
#include <luabind/object.hpp>
#include <commdlg.h>
#include <vector>

static void LuabindRegisterScreenRecorderGlobalFunctions(lua_State* L)
{
    using namespace luabind;
}

void ParaScripting::CNPLScriptingState::LoadHAPI_Platform()
{
    using namespace luabind;
    lua_State* L = GetLuaState();

    LuabindRegisterScreenRecorderGlobalFunctions(L);
}

#ifndef MAX_LINE
/**@def max number of characters in a single line */
#define MAX_LINE	1024
#endif

bool ParaScripting::ParaGlobal::OpenFileDialog(const luabind::object& inout)
{
    if (type(inout) != LUA_TTABLE)
    {
        return false;
    }

    constexpr DWORD MAX_FILE_BUF = 20480;
    OPENFILENAMEW ofn = { 0 };
    memset(&ofn, 0, sizeof(ofn));
    std::vector<WCHAR> szFileName(MAX_FILE_BUF, 0);
    ofn.lStructSize = sizeof(OPENFILENAMEW);
    ofn.hwndOwner = CGlobals::GetAppHWND();
    ofn.lpstrFile = szFileName.data();
    ofn.lpstrFilter = L"All Files (*.*)\0*.*\0";
    ofn.nFilterIndex = 0;
    ofn.lpstrFileTitle = NULL;
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = NULL;
    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
    ofn.nMaxFile = MAX_FILE_BUF;
    bool bIsSavingFile = false;

    std::wstring wInitialDir;
    std::wstring wTitle;
    std::vector<WCHAR> wFilterBuf;

    for (luabind::iterator itCur(inout), itEnd; itCur != itEnd; ++itCur)
    {
        // we only serialize item with a string key
        const object& key = itCur.key();
        if (type(key) == LUA_TSTRING)
        {
            const char* pKey = object_cast<const char*>(key);
            if (!pKey)
                continue;
            string sKey = pKey;
            const object& Value = *itCur;
            if (sKey == "initialdir")
            {
                if (type(Value) == LUA_TSTRING)
                {
                    const char* sVal = object_cast<const char*>(Value);
                    if (sVal && sVal[0] != '\0')
                    {
                        const WCHAR* pWide = StringHelper::MultiByteToWideChar(sVal, DEFAULT_FILE_ENCODING);
                        if (pWide)
                        {
                            wInitialDir = pWide;
                            ofn.lpstrInitialDir = wInitialDir.c_str();
                        }
                    }
                }
            }
            else if (sKey == "filter")
            {
                if (type(Value) == LUA_TSTRING)
                {
                    lua_State* L = Value.interpreter();
                    if (L)
                    {
                        Value.push(L);
                        size_t rawLen = 0;
                        const char* pRaw = lua_tolstring(L, -1, &rawLen);
                        std::string rawFilter;
                        if (pRaw && rawLen > 0)
                        {
                            rawFilter.assign(pRaw, rawLen);
                        }
                        lua_pop(L, 1);

                        if (!rawFilter.empty())
                        {
                            int wLen = ::MultiByteToWideChar(DEFAULT_FILE_ENCODING, 0, rawFilter.data(), (int)rawFilter.size(), nullptr, 0);
                            if (wLen > 0)
                            {
                                wFilterBuf.assign((size_t)wLen + 2, 0);
                                ::MultiByteToWideChar(DEFAULT_FILE_ENCODING, 0, rawFilter.data(), (int)rawFilter.size(), wFilterBuf.data(), wLen);
                                bool hasEmbeddedNull = false;
                                for (int i = 0; i < wLen - 1; ++i)
                                {
                                    if (wFilterBuf[i] == L'\0')
                                    {
                                        hasEmbeddedNull = true;
                                        break;
                                    }
                                }
                                if (!hasEmbeddedNull)
                                {
                                    for (int i = 0; i < wLen; ++i)
                                    {
                                        if (wFilterBuf[i] == L'|')
                                            wFilterBuf[i] = L'\0';
                                    }
                                }
                                ofn.lpstrFilter = wFilterBuf.data();
                            }
                        }
                    }
                }
            }
            else if (sKey == "title")
            {
                if (type(Value) == LUA_TSTRING)
                {
                    const char* sVal = object_cast<const char*>(Value);
                    if (sVal)
                    {
                        const WCHAR* pWide = StringHelper::MultiByteToWideChar(sVal, DEFAULT_FILE_ENCODING);
                        if (pWide)
                        {
                            wTitle = pWide;
                            ofn.lpstrTitle = wTitle.c_str();
                        }
                    }
                }
            }
            else if (sKey == "filename")
            {
                if (type(Value) == LUA_TSTRING)
                {
                    const char* sVal = object_cast<const char*>(Value);
                    if (sVal && sVal[0] != '\0')
                    {
                        const WCHAR* pWide = StringHelper::MultiByteToWideChar(sVal, DEFAULT_FILE_ENCODING);
                        if (pWide)
                        {
                            wcsncpy(szFileName.data(), pWide, MAX_FILE_BUF - 1);
                            szFileName[MAX_FILE_BUF - 1] = L'\0';
                        }
                    }
                }
            }
            else if (sKey == "save")
            {
                if (type(Value) == LUA_TBOOLEAN && object_cast<bool>(Value))
                {
                    bIsSavingFile = true;
                    ofn.Flags &= ~OFN_FILEMUSTEXIST;
                }
            }
            else if (sKey == "multi")
            {
                if (type(Value) == LUA_TBOOLEAN && object_cast<bool>(Value))
                {
                    ofn.Flags |= OFN_ALLOWMULTISELECT | OFN_EXPLORER;
                }
            }
        }
    }

    WCHAR sWorkingDir16[MAX_PATH + 1] = { 0 };
    DWORD nCount = ::GetCurrentDirectoryW(MAX_PATH, sWorkingDir16);

    // switch to windowed mode to display the win32 common dialog.
    bool bOldWindowed = CGlobals::GetApp() ? CGlobals::GetApp()->IsWindowedMode() : true;  // Preserve original windowed flag
    if (bOldWindowed == false && CGlobals::GetApp())
    {
        CGlobals::GetApp()->SetWindowedMode(true);
    }

    bool bResult = bIsSavingFile ? (!!::GetSaveFileNameW(&ofn)) : (!!::GetOpenFileNameW(&ofn));

    if (bOldWindowed == false && CGlobals::GetApp())
    {
        CGlobals::GetApp()->SetWindowedMode(false);
    }

    // reset directory. 
    if (nCount > 0)
        ::SetCurrentDirectoryW(sWorkingDir16);

    inout["result"] = bResult;
    if (bResult)
    {
        if (ofn.lpstrFile)
        {
            if ((ofn.Flags & OFN_ALLOWMULTISELECT) != 0)
            {
                for (DWORD i = 0; i + 1 < MAX_FILE_BUF; ++i)
                {
                    if (ofn.lpstrFile[i] == L'\0')
                    {
                        if (ofn.lpstrFile[i + 1] != L'\0')
                            ofn.lpstrFile[i] = L'|';
                        else
                            break;
                    }
                }
            }
            const char* pUtf8 = StringHelper::WideCharToMultiByte(ofn.lpstrFile, DEFAULT_FILE_ENCODING);
            string filename = pUtf8 ? pUtf8 : "";
            inout["filename"] = filename;
        }
    }

    return bResult;
}

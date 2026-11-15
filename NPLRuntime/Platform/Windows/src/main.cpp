//-----------------------------------------------------------------------------
// Class:	ParaWorld Game Entry File
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine Dev Studio
// Date:	2004.3.8
// Revised: 2010.2.18
// Desc: The main game class. WinMain(). Game specific logic is here.
// Please see the macro comments
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "resource.h"

#include "ParaEngineService.h"
#include "ParaEngineCore.h"

#include "PluginAPI.h"
#include "IParaEngineCore.h"
#include "IParaEngineApp.h"
#include "guicon.h"

#include "memdebug.h"
#include "CommandLineParams.h"
#include "util/StringHelper.h"
#include "debugtools/paradebug.h"

#include "RenderWindowDelegate.h"
#include "WindowsApplication.h"

using namespace ParaEngine;
extern HINSTANCE g_hAppInstance;

#ifndef ID_GAME_COMMANDLINE
#define ID_GAME_COMMANDLINE 40061
#endif

static std::string GetUtf8CommandLineArgs(LPSTR fallbackAnsiCmdLine)
{
	const WCHAR* wFullCmd = ::GetCommandLineW();
	if (!wFullCmd || wFullCmd[0] == L'\0')
	{
		return fallbackAnsiCmdLine ? fallbackAnsiCmdLine : "";
	}

	const WCHAR* p = wFullCmd;
	while (*p == L' ' || *p == L'\t')
		++p;

	if (*p == L'"')
	{
		++p;
		while (*p != L'\0')
		{
			if (*p == L'"')
			{
				++p;
				break;
			}
			++p;
		}
	}
	else
	{
		while (*p != L'\0' && *p != L' ' && *p != L'\t')
			++p;
	}

	while (*p == L' ' || *p == L'\t')
		++p;

	if (*p == L'\0')
		return "";

	const char* utf8Args = StringHelper::WideCharToMultiByte(p, CP_UTF8);
	return utf8Args ? utf8Args : (fallbackAnsiCmdLine ? fallbackAnsiCmdLine : "");
}

static LONG WINAPI ParaEngineUnhandledExceptionFilter(EXCEPTION_POINTERS* pExceptionPointers)
{
#if defined(WIN32) && !defined(PARAENGINE_MOBILE) && !defined(NPLRUNTIME_SERVER)
	return GenerateDump(pExceptionPointers);
#else
	(void)pExceptionPointers;
	return EXCEPTION_EXECUTE_HANDLER;
#endif
}

static void EnablePerMonitorDpiAwareness()
{
	HMODULE hUser32 = ::GetModuleHandleW(L"user32.dll");
	if (hUser32)
	{
		typedef BOOL(WINAPI* PSetProcessDpiAwarenessContext)(HANDLE);
		auto pSetCtx = reinterpret_cast<PSetProcessDpiAwarenessContext>(::GetProcAddress(hUser32, "SetProcessDpiAwarenessContext"));
		// DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2 = ((HANDLE)-4)
		if (pSetCtx && pSetCtx(reinterpret_cast<HANDLE>(static_cast<intptr_t>(-4))))
		{
			return;
		}
	}
	HMODULE hShcore = ::LoadLibraryW(L"Shcore.dll");
	if (hShcore)
	{
		typedef HRESULT(WINAPI* PSetProcessDpiAwareness)(int);
		auto pSetDpi = reinterpret_cast<PSetProcessDpiAwareness>(::GetProcAddress(hShcore, "SetProcessDpiAwareness"));
		if (pSetDpi)
		{
			pSetDpi(2); // PROCESS_PER_MONITOR_DPI_AWARE
		}
		::FreeLibrary(hShcore);
	}
}

static void ForceForegroundWindow(HWND hWnd)
{
	if (!::IsWindow(hWnd))
		return;
	if (::IsIconic(hWnd))
		::ShowWindow(hWnd, SW_RESTORE);
	HWND hFore = ::GetForegroundWindow();
	DWORD dwCurThread = ::GetCurrentThreadId();
	DWORD dwForeThread = hFore ? ::GetWindowThreadProcessId(hFore, nullptr) : 0;
	if (dwForeThread != 0 && dwForeThread != dwCurThread)
	{
		::AttachThreadInput(dwCurThread, dwForeThread, TRUE);
		::SetForegroundWindow(hWnd);
		::BringWindowToTop(hWnd);
		::AttachThreadInput(dwCurThread, dwForeThread, FALSE);
	}
	else
	{
		::SetForegroundWindow(hWnd);
		::BringWindowToTop(hWnd);
	}
}

INT WINAPI WinMain( HINSTANCE hInst, HINSTANCE, LPSTR lpCmdLine, INT )
{
	g_hAppInstance = hInst;
	int exit_code = 0;

	const bool bNoCrashDialog = (::GetEnvironmentVariableA("PARAENGINE_NO_CRASH_DIALOG", nullptr, 0) > 0);
	if (bNoCrashDialog)
	{
		::SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX);
	}
	else
	{
		::SetUnhandledExceptionFilter(ParaEngineUnhandledExceptionFilter);
	}

	EnablePerMonitorDpiAwareness();

	std::string sCmdLineUtf8 = GetUtf8CommandLineArgs(lpCmdLine);
	const char* cmdLinePtr = sCmdLineUtf8.c_str();

	bool bRedirectConsole = false;
#if defined(_DEBUG)
	bRedirectConsole = true;
#endif

	// Support single-instance command-line forwarding via WM_COPYDATA (-single / single="true")
	if (!sCmdLineUtf8.empty())
	{
		CCommandLineParams cmdParams(cmdLinePtr);
		const char* sSingle = cmdParams.GetAppCommandLineByParam("single", nullptr);
		bool bSingleInstance = (sSingle && (strcmp(sSingle, "true") == 0 || strcmp(sSingle, "%22true%22") == 0))
			|| (strstr(cmdLinePtr, "-single") != nullptr);
		if (bSingleInstance)
		{
			HWND hExisting = ::FindWindowW(RenderWindowWin32::ClassName, nullptr);
			if (hExisting && ::IsWindow(hExisting))
			{
				ForceForegroundWindow(hExisting);
				COPYDATASTRUCT cds = {};
				cds.dwData = ID_GAME_COMMANDLINE;
				cds.cbData = static_cast<DWORD>(sCmdLineUtf8.size() + 1);
				cds.lpData = const_cast<char*>(cmdLinePtr);
				DWORD_PTR dwResult = 0;
				::SendMessageTimeoutW(hExisting, WM_COPYDATA, 0, reinterpret_cast<LPARAM>(&cds), SMTO_ABORTIFHUNG, 3000, &dwResult);
				return 0;
			}
		}

		const char* sServerMode = cmdParams.GetAppCommandLineByParam("servermode", nullptr);
		const char* sInteractive = cmdParams.GetAppCommandLineByParam("i", nullptr);
		if ((sServerMode && strcmp(sServerMode, "true") == 0)
			|| (sInteractive && strcmp(sInteractive, "true") == 0))
		{
			bRedirectConsole = true;
		}
	}

	if (bRedirectConsole)
	{
		RedirectIOToConsole();
	}

	RenderWindowDelegate renderWindow;

	CWindowsApplication app(hInst);
	app.InitApp(&renderWindow, cmdLinePtr);
	exit_code = app.Run(hInst);

	if (bRedirectConsole)
	{
		FreeDebugConsole();
	}

	return exit_code;
}

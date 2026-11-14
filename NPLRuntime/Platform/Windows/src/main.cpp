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

#include "RenderWindowDelegate.h"
#include "WindowsApplication.h"

using namespace ParaEngine;
extern HINSTANCE g_hAppInstance;

#ifndef ID_GAME_COMMANDLINE
#define ID_GAME_COMMANDLINE 40061
#endif

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

	if (::GetEnvironmentVariableA("PARAENGINE_NO_CRASH_DIALOG", nullptr, 0) > 0)
	{
		::SetErrorMode(SEM_FAILCRITICALERRORS | SEM_NOGPFAULTERRORBOX | SEM_NOOPENFILEERRORBOX);
	}

	EnablePerMonitorDpiAwareness();

	// Support single-instance command-line forwarding via WM_COPYDATA (-single / single="true")
	if (lpCmdLine && lpCmdLine[0] != '\0')
	{
		CCommandLineParams cmdParams(lpCmdLine);
		const char* sSingle = cmdParams.GetAppCommandLineByParam("single", nullptr);
		bool bSingleInstance = (sSingle && (strcmp(sSingle, "true") == 0 || strcmp(sSingle, "%22true%22") == 0))
			|| (strstr(lpCmdLine, "-single") != nullptr);
		if (bSingleInstance)
		{
			HWND hExisting = ::FindWindowW(RenderWindowWin32::ClassName, nullptr);
			if (hExisting && ::IsWindow(hExisting))
			{
				ForceForegroundWindow(hExisting);
				COPYDATASTRUCT cds = {};
				cds.dwData = ID_GAME_COMMANDLINE;
				cds.cbData = static_cast<DWORD>(strlen(lpCmdLine) + 1);
				cds.lpData = const_cast<LPSTR>(lpCmdLine);
				DWORD_PTR dwResult = 0;
				::SendMessageTimeoutW(hExisting, WM_COPYDATA, 0, reinterpret_cast<LPARAM>(&cds), SMTO_ABORTIFHUNG, 3000, &dwResult);
				return 0;
			}
		}
	}

#if defined(_DEBUG)
	RedirectIOToConsole();
#endif

	RenderWindowDelegate renderWindow;

	CWindowsApplication app(hInst);
	app.InitApp(&renderWindow, lpCmdLine);
	exit_code = app.Run(hInst);

#if defined(_DEBUG)
	FreeDebugConsole();
#endif

	return exit_code;
}

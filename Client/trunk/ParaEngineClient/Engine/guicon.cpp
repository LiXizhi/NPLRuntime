#include "ParaEngine.h"
#include <windows.h>

#include <stdio.h>

#include <fcntl.h>

#include <io.h>

#include <iostream>

#include <fstream>

#ifndef _USE_OLD_IOSTREAMS

using namespace std;

#endif

// maximum mumber of lines the output console should have

static const WORD MAX_CONSOLE_LINES = 500;



void RedirectIOToConsole()

{

	int hConHandle;

	long lStdHandle;

	CONSOLE_SCREEN_BUFFER_INFO coninfo;

	FILE *fp;

	// allocate a console for this app
	{
		const UINT ProcessBasicInformation = 0;
		typedef LONG(WINAPI *PROCNTQSIP)(HANDLE, UINT, PVOID, ULONG, PULONG);
		PROCNTQSIP NtQueryInformationProcess;
		NtQueryInformationProcess = (PROCNTQSIP)GetProcAddress(GetModuleHandle("ntdll.dll"),
			"NtQueryInformationProcess");

		typedef struct
		{
			DWORD ExitStatus;
			DWORD PebBaseAddress;
			DWORD AffinityMask;
			DWORD BasePriority;
			ULONG UniqueProcessId;
			ULONG InheritedFromUniqueProcessId;
		} PROCESS_BASIC_INFORMATION;

		PROCESS_BASIC_INFORMATION pbi;
		auto hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, GetCurrentProcessId());
		NtQueryInformationProcess(hProcess, ProcessBasicInformation, (PVOID)&pbi,
			sizeof(PROCESS_BASIC_INFORMATION), NULL);

		CloseHandle(hProcess);

		if (!AttachConsole(pbi.InheritedFromUniqueProcessId))
		{
			AllocConsole();
		}
	}


	// set the screen buffer to be big enough to let us scroll text

	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), 

		&coninfo);

	coninfo.dwSize.Y = MAX_CONSOLE_LINES;

	SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE), 

		coninfo.dwSize);

	// redirect unbuffered STDOUT to the console

	lStdHandle = (long)GetStdHandle(STD_OUTPUT_HANDLE);

	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);

	fp = _fdopen( hConHandle, "w" );

	*stdout = *fp;

	setvbuf( stdout, NULL, _IONBF, 0 );

	// redirect unbuffered STDIN to the console

	lStdHandle = (long)GetStdHandle(STD_INPUT_HANDLE);

	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);

	fp = _fdopen( hConHandle, "r" );

	*stdin = *fp;

	setvbuf( stdin, NULL, _IONBF, 0 );

	// redirect unbuffered STDERR to the console

	lStdHandle = (long)GetStdHandle(STD_ERROR_HANDLE);

	hConHandle = _open_osfhandle(lStdHandle, _O_TEXT);

	fp = _fdopen( hConHandle, "w" );

	*stderr = *fp;

	setvbuf( stderr, NULL, _IONBF, 0 );


	// make cout, wcout, cin, wcin, wcerr, cerr, wclog and clog 

	// point to console as well

	ios::sync_with_stdio();

	freopen("CONIN$", "r", stdin);
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);

}



//End of File


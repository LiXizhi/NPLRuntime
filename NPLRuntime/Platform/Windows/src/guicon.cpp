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

// maximum number of lines the output console should have
static const WORD MAX_CONSOLE_LINES = 500;

void RedirectIOToConsole()
{
	CONSOLE_SCREEN_BUFFER_INFO coninfo = {};

	if (!::AttachConsole(ATTACH_PARENT_PROCESS))
	{
		if (!::AllocConsole())
		{
			// Already attached to a console or cannot allocate one
		}
	}

	::SetConsoleOutputCP(CP_UTF8);
	::SetConsoleCP(CP_UTF8);

	HANDLE hStdOut = ::GetStdHandle(STD_OUTPUT_HANDLE);
	if (hStdOut && hStdOut != INVALID_HANDLE_VALUE)
	{
		if (::GetConsoleScreenBufferInfo(hStdOut, &coninfo))
		{
			if (coninfo.dwSize.Y < MAX_CONSOLE_LINES)
				coninfo.dwSize.Y = MAX_CONSOLE_LINES;
			::SetConsoleScreenBufferSize(hStdOut, coninfo.dwSize);
		}

		int hConHandle = _open_osfhandle(reinterpret_cast<intptr_t>(hStdOut), _O_TEXT);
		if (hConHandle != -1)
		{
			FILE* fp = _fdopen(hConHandle, "w");
			if (fp)
			{
				*stdout = *fp;
				setvbuf(stdout, NULL, _IONBF, 0);
			}
		}
	}

	HANDLE hStdIn = ::GetStdHandle(STD_INPUT_HANDLE);
	if (hStdIn && hStdIn != INVALID_HANDLE_VALUE)
	{
		int hConHandle = _open_osfhandle(reinterpret_cast<intptr_t>(hStdIn), _O_TEXT);
		if (hConHandle != -1)
		{
			FILE* fp = _fdopen(hConHandle, "r");
			if (fp)
			{
				*stdin = *fp;
				setvbuf(stdin, NULL, _IONBF, 0);
			}
		}
	}

	HANDLE hStdErr = ::GetStdHandle(STD_ERROR_HANDLE);
	if (hStdErr && hStdErr != INVALID_HANDLE_VALUE)
	{
		int hConHandle = _open_osfhandle(reinterpret_cast<intptr_t>(hStdErr), _O_TEXT);
		if (hConHandle != -1)
		{
			FILE* fp = _fdopen(hConHandle, "w");
			if (fp)
			{
				*stderr = *fp;
				setvbuf(stderr, NULL, _IONBF, 0);
			}
		}
	}

	freopen("CONIN$", "r", stdin);
	freopen("CONOUT$", "w", stdout);
	freopen("CONOUT$", "w", stderr);

	ios::sync_with_stdio();
}

void FreeDebugConsole()
{
	FreeConsole();
}

//End of File

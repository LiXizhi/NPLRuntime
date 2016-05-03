//-----------------------------------------------------------------------------
// Class:	Console in win32
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine 
// Date:	2010.3.16
// Desc:  Mostly for debugging purposes. based on http://www.halcyon.com/~ast/dload/guicon.htm
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#ifdef WIN32
#include <windows.h>
#include <stdio.h>
#include <fcntl.h>
#include <io.h>
#include <iostream>
#include <fstream>
#endif

#include "GuiConsole.h"

using namespace std;

// maximum number of lines the output console should have
static const int MAX_CONSOLE_LINES = 500;
// maximum number of chars in a line 
static const int CONSOLE_COLUMNS_COUNT = 128;

void ParaEngine::RedirectIOToConsole()
{
#ifdef WIN32
	static bool s_bRedirected = false;
	if(s_bRedirected)
		return;
	s_bRedirected = true;
		
	int hConHandle;
	long lStdHandle;

	CONSOLE_SCREEN_BUFFER_INFO coninfo;
	FILE *fp;

	// allocate a console for this app
	AllocConsole();
	// use UTF8 code page
	//SetConsoleCP(CP_UTF8);
	//SetConsoleOutputCP(CP_UTF8);
	SetConsoleTitle("NPL debugger");

	// set the screen buffer to be big enough to let us scroll text
	GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE),&coninfo);
	/*if(coninfo.dwSize.X < CONSOLE_COLUMNS_COUNT){
		coninfo.dwSize.X = CONSOLE_COLUMNS_COUNT;
	}*/
	if(coninfo.dwSize.Y < MAX_CONSOLE_LINES){
		coninfo.dwSize.Y = MAX_CONSOLE_LINES;
	}
	SetConsoleScreenBufferSize(GetStdHandle(STD_OUTPUT_HANDLE),coninfo.dwSize);

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
#endif
}

bool ParaEngine::SetOutputConsoleTextAttribute( WORD wAttributes )
{
#ifdef WIN32
	return !!SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), wAttributes);
#endif
}
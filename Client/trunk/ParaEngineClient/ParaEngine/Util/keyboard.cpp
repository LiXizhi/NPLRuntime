//-----------------------------------------------------------------------------
// Class:	Keyboard functions
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine Corporation
// Date:	2009.7.26
// Desc: Cross platformed.
//-----------------------------------------------------------------------------
#include "ParaEngine.h"

#if defined(PLATFORM_MAC)
// TODO: wangpeng for mac input
int _kbhit()
{
	return 0;
}
#elif !defined(WIN32) && !defined(PARAENGINE_MOBILE)

//  Linux emulation of kbhit
#include <stdio.h>
#include <sys/select.h>
#include <sys/ioctl.h>
#include <termios.h>
#include <stropts.h>

int _kbhit()
{
	static const int STDIN = 0;
	static bool initialized = false;
	if (!initialized)
	{
		// Use termios to turn off line buffering
		termios term;
		tcgetattr(STDIN, &term);
		term.c_lflag &= ~ICANON;
		tcsetattr(STDIN, TCSANOW, &term);
		setbuf(stdin, NULL);
		initialized = true;
	}
	int bytesWaiting;
	ioctl(STDIN, FIONREAD, &bytesWaiting);
	return bytesWaiting;
}
#endif

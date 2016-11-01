#ifndef FSSIMPLEWINDOW_IS_INCLUDED
#define FSSIMPLEWINDOW_IS_INCLUDED
/* { */


#ifndef FS_MOUSEEVENT_TYPE_IS_DEFINED
#define FS_MOUSEEVENT_TYPE_IS_DEFINED
enum
{
	FSMOUSEEVENT_NONE,
	FSMOUSEEVENT_LBUTTONDOWN,
	FSMOUSEEVENT_LBUTTONUP,
	FSMOUSEEVENT_MBUTTONDOWN,
	FSMOUSEEVENT_MBUTTONUP,
	FSMOUSEEVENT_RBUTTONDOWN,
	FSMOUSEEVENT_RBUTTONUP,
	FSMOUSEEVENT_MOVE
};
#endif



#ifndef FS_KEYCODE_TYPE_IS_DEFINED
#define FS_KEYCODE_TYPE_IS_DEFINED
enum
{
	FSKEY_NULL,
	FSKEY_SPACE,
	FSKEY_0,
	FSKEY_1,
	FSKEY_2,
	FSKEY_3,
	FSKEY_4,
	FSKEY_5,
	FSKEY_6,
	FSKEY_7,
	FSKEY_8,
	FSKEY_9,
	FSKEY_A,
	FSKEY_B,
	FSKEY_C,
	FSKEY_D,
	FSKEY_E,
	FSKEY_F,
	FSKEY_G,
	FSKEY_H,
	FSKEY_I,
	FSKEY_J,
	FSKEY_K,
	FSKEY_L,
	FSKEY_M,
	FSKEY_N,
	FSKEY_O,
	FSKEY_P,
	FSKEY_Q,
	FSKEY_R,
	FSKEY_S,
	FSKEY_T,
	FSKEY_U,
	FSKEY_V,
	FSKEY_W,
	FSKEY_X,
	FSKEY_Y,
	FSKEY_Z,
	FSKEY_ESC,
	FSKEY_F1,
	FSKEY_F2,
	FSKEY_F3,
	FSKEY_F4,
	FSKEY_F5,
	FSKEY_F6,
	FSKEY_F7,
	FSKEY_F8,
	FSKEY_F9,
	FSKEY_F10,
	FSKEY_F11,
	FSKEY_F12,
	FSKEY_PRINTSCRN,
	FSKEY_CAPSLOCK,
	FSKEY_SCROLLLOCK,
	FSKEY_PAUSEBREAK,
	FSKEY_BS,
	FSKEY_TAB,
	FSKEY_ENTER,
	FSKEY_SHIFT,
	FSKEY_CTRL,
	FSKEY_ALT,
	FSKEY_INS,
	FSKEY_DEL,
	FSKEY_HOME,
	FSKEY_END,
	FSKEY_PAGEUP,
	FSKEY_PAGEDOWN,
	FSKEY_UP,
	FSKEY_DOWN,
	FSKEY_LEFT,
	FSKEY_RIGHT,
	FSKEY_NUMLOCK,
	FSKEY_TILDA,
	FSKEY_MINUS,
	FSKEY_PLUS,
	FSKEY_LBRACKET,
	FSKEY_RBRACKET,
	FSKEY_BACKSLASH,
	FSKEY_SEMICOLON,
	FSKEY_SINGLEQUOTE,
	FSKEY_COMMA,
	FSKEY_DOT,
	FSKEY_SLASH,
	FSKEY_TEN0,
	FSKEY_TEN1,
	FSKEY_TEN2,
	FSKEY_TEN3,
	FSKEY_TEN4,
	FSKEY_TEN5,
	FSKEY_TEN6,
	FSKEY_TEN7,
	FSKEY_TEN8,
	FSKEY_TEN9,
	FSKEY_TENDOT,
	FSKEY_TENSLASH,
	FSKEY_TENSTAR,
	FSKEY_TENMINUS,
	FSKEY_TENPLUS,
	FSKEY_TENENTER,
	FSKEY_WHEELUP,
	FSKEY_WHEELDOWN,

FSKEY_NUM_KEYCODE
};
#endif



#ifndef FSSIMPLEWINDOW_MACRO_ONLY

	#ifndef FSSIMPLEWINDOW_DONT_INCLUDE_OPENGL_HEADERS
		#if defined(_WIN32) || defined(WIN32)
			#include <windows.h>
			#include <GL/gl.h>
			#include <GL/glu.h>
		#elif defined(__linux) || defined(__linux__)
			#include <GL/gl.h>
			#include <GL/glu.h>
		#elif defined(__APPLE__)
			#include <OpenGL/gl.h>
			#include <OpenGL/glu.h>
		#else // Unknown operating system
			#include <GL/gl.h>
			#include <GL/glu.h>
		#endif
	#endif // << #ifndef FSSIMPLEWINDOW_DONT_INCLUDE_OPENGL_HEADERS



#ifdef __cplusplus
// This needs to be included from Objective-C code for mouse-event enums.
// C++ specific declaration must be enclosed by #ifdef __cplucplus and #endif

void FsOpenWindow(int x0,int y0,int wid,int hei,int useDoubleBuffer);
void FsGetWindowSize(int &wid,int &hei);
void FsPollDevice(void);
void FsSleep(int ms);
int FsPassedTime(void);
void FsGetMouseState(int &lb,int &mb,int &rb,int &mx,int &my);
int FsGetMouseEvent(int &lb,int &mb,int &rb,int &mx,int &my);
void FsSwapBuffers(void);
int FsInkey(void);
int FsInkeyChar(void);
int FsGetKeyState(int fsKeyCode);
int FsCheckWindowExposure(void);

void FsChangeToProgramDir(void); // Mainly for Mac OS X
#endif // << #ifdef __cplusplus

#endif // << #ifndef FSSIMPLEWINDOW_MACRO_ONLY



/* } */
#endif

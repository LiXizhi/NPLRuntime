#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>
#include <OpenGL/gl.h>
#include <OpenGL/glu.h>

extern "C" void FsOpenWindowC(int x0,int y0,int wid,int hei,int useDoubleBuffer);
extern "C" void FsGetWindowSizeC(int *wid,int *hei);
extern "C" void FsPollDeviceC(void);
extern "C" void FsSleepC(int ms);
extern "C" int FsPassedTimeC(void);
extern "C" void FsMouseC(int *lb,int *mb,int *rb,int *mx,int *my);
extern "C" int FsGetMouseEventC(int *lb,int *mb,int *rb,int *mx,int *my);
extern "C" void FsSwapBufferC(void);
extern "C" int FsInkeyC(void);
extern "C" int FsInkeyCharC(void);
extern "C" int FsKeyStateC(int fsKeyCode);
extern "C" void FsChangeToProgramDirC(void);
extern "C" void FsCheckExposureC(void);

void FsOpenWindow(int x0,int y0,int wid,int hei,int useDoubleBuffer)
{
	FsOpenWindowC(x0,y0,wid,hei,useDoubleBuffer);
}

void FsGetWindowSize(int &wid,int &hei)
{
	FsGetWindowSizeC(&wid,&hei);
}

void FsPollDevice(void)
{
	FsPollDeviceC();
}

void FsSleep(int ms)
{
	FsSleepC(ms);
}

int FsPassedTime(void)
{
	return FsPassedTimeC();
}

void FsGetMouseState(int &lb,int &mb,int &rb,int &mx,int &my)
{
	FsMouseC(&lb,&mb,&rb,&mx,&my);
}

int FsGetMouseEvent(int &lb,int &mb,int &rb,int &mx,int &my)
{
	return FsGetMouseEventC(&lb,&mb,&rb,&mx,&my);
}

void FsSwapBuffers(void)
{
	FsSwapBufferC();
}

int FsInkey(void)
{
	return FsInkeyC();
}

int FsInkeyChar(void)
{
	return FsInkeyCharC();
}

int FsGetKeyState(int fsKeyCode)
{
	return FsKeyStateC(fsKeyCode);
}

void FsCheckWindowExposure(void)
{
	return FsCheckExposureC();
}

void FsChangeToProgramDir(void)
{
	FsChangeToProgramDirC();
}


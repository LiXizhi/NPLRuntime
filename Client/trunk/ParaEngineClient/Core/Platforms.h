#pragma once
#include "ParaPlatformConfig.h"
#include "ParaPlatformMacros.h"

/** word packing order: in x86, android, mac, ios it is all little endian. */
#define PLATFORM_LITTLE_ENDIAN	1
#include "RenderCore.h"
#include "PEtypes.h"

#ifndef FORCEINLINE
#define FORCEINLINE
#endif

#ifndef GCC_ALIGN						
#define GCC_ALIGN(n)
#endif
#ifndef MS_ALIGN						
#define MS_ALIGN(n)
#endif

// Cross-platform needed functions
#ifdef WIN32

#define vsnprintf _vsnprintf
#define snprintf _snprintf
#define vsnwprintf _vsnwprintf
#if _MSC_VER >= 1400 //vc8.0 use new secure
#define snwprintf _snwprintf_s
#else
#define snwprintf _snwprintf
#endif // _MSC_VER

#define strlower _strlwr
#define stricmp _stricmp

#else // WIN32
#define vsnwprintf vswprintf
#define snwprintf swprintf
#define stricmp strcasecmp

#endif // WIN32

// Cross-platform sleep functions
#ifdef WIN32
#define SLEEP(x)   ::Sleep(x)
#else // WIN32
#define SLEEP(x)   usleep(x*1000)
#endif // WIN32 

#ifdef WIN32
#define itoa _itoa
#else // _WIN32
extern unsigned int GetTickCount(void);
extern char* itoa(int value, char*  str, int radix);
extern char* strrev(char* szT);
#define _itoa   itoa

extern char *_gcvt( double value,	int digits,	char *buffer );
extern wchar_t * _itow(int value,	wchar_t *str,	int radix);
extern BOOL PtInRect(const RECT *pos, const POINT &pt );
extern BOOL SetRect(RECT* lprc, int xLeft, int yTop, int xRight, int yBottom);
#ifndef ZeroMemory
#define ZeroMemory(Destination,Length) memset((Destination),0,(Length))
#endif
#endif // _WIN32 

#ifndef CP_UTF8
#define CP_UTF8 65001
#endif

/** native implementation of strnlen */
extern int strnlen(const char *str, int maxsize);
namespace ParaEngine
{
	/* returns a random number between 0,RAND_MAX*/
	int my_rand (void);
	/* returns a random number between 0,1*/
	float frand();
	float randfloat(float lower, float upper);
	int randint(int lower, int upper);
}
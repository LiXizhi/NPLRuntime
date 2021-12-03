#pragma once
//-----------------------------------------------------------------------------
// Copyright (C) 2004 - 2009 ParaEngine Corporation, All Rights Reserved.
// Author: LiXizhi
// Date:	2009.7
// Desc: Cross Platform Defines
//-----------------------------------------------------------------------------
#include "coreexport.h"
#ifdef WIN32
#	ifndef _CRT_SECURE_NO_WARNINGS
#		define _CRT_SECURE_NO_WARNINGS
#	endif
#endif

#ifdef WIN32
#ifndef PARAENGINE_CLIENT
/* Prevent inclusion of winsock.h in windows.h, otherwise boost::Asio will produce error in ParaEngineServer project: WinSock.h has already been included*/ 
#define WIN32_LEAN_AND_MEAN    
#endif
#include <windows.h>
#include <stdio.h>
#include <wchar.h>
#else// For LINUX
#include <ctype.h>
#include <wctype.h>
#include <unistd.h>
#include <string.h>
#include <wchar.h>
#include <stdarg.h>
#include <malloc.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#endif // WIN32

#include <assert.h>

// Cross-platform type definitions
#ifdef WIN32
typedef signed char int8;
typedef short int16;
typedef long int32;
typedef __int64 int64;
typedef unsigned char uint8;
typedef unsigned short uint16;
typedef unsigned long uint32;
typedef unsigned __int64 uint64;
typedef unsigned char byte;
#ifndef STDCALL 
#define STDCALL __stdcall
#endif 

#else // For LINUX
typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;
typedef uint8_t byte;
#define FALSE 0
#define TRUE 1
#define VOID            void
typedef void * HANDLE;
typedef void * HWND;
typedef void * HANDLE;
typedef void * HMODULE;
typedef void * HINSTANCE;
typedef void *PVOID;
typedef void *LPVOID;
typedef float FLOAT;
//typedef unsigned long DWORD;
typedef uint32_t DWORD;
typedef DWORD *LPDWORD;
typedef const void * LPCVOID;
typedef char CHAR;
typedef wchar_t WCHAR;
//typedef unsigned short WORD;
typedef uint16_t WORD;
typedef float               FLOAT;
typedef int                 BOOL;
typedef unsigned char       BYTE;
typedef int                 INT;
typedef unsigned int        UINT;
//typedef long LONG;
//typedef unsigned long ULONG;
//typedef long HRESULT;
typedef int32_t LONG;
typedef uint32_t ULONG;
typedef int32_t HRESULT;

typedef struct tagRECT
{
	LONG left;
	LONG top;
	LONG right;
	LONG bottom;
} 	RECT;

#define FILE_BEGIN           0
#define FILE_CURRENT         1
#define FILE_END             2

#ifndef STDCALL 
#define STDCALL __attribute__((stdcall)) 
#endif 

#define _HRESULT_TYPEDEF_(_sc) ((HRESULT)_sc)
#define E_FAIL		((HRESULT)(0x80000008L))
// emulate windows error msg in linux
#define E_ACCESSDENIED                   _HRESULT_TYPEDEF_(0x80070005L)
#define E_INVALIDARG                     _HRESULT_TYPEDEF_(0x80000003L)

typedef struct _FILETIME
{
	DWORD dwLowDateTime;
	DWORD dwHighDateTime;
} 	FILETIME;
#define MAX_PATH          260
#endif // WIN32

//-----------------------------------------------------------------------------
// File: DxStdAfx.h
//
// Desc: Header file that is the standard includes for the DirectX SDK samples
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//-----------------------------------------------------------------------------
#if !defined(DXSDK_STDAFX_H)

#define DXSDK_STDAFX_H
#ifndef _CRT_SECURE_NO_WARNINGS
#define _CRT_SECURE_NO_WARNINGS
#endif
#define VC_EXTRALEAN		// Exclude rarely-used stuff from Windows headers

#define STRICT
#define NOMINMAX 
#ifndef WINVER
#define WINVER         0x0500
#endif
#ifndef _WIN32_WINDOWS
#define _WIN32_WINDOWS 0x0410 
#endif
#ifndef _WIN32_WINNT
#define _WIN32_WINNT   0x0500 
#endif

#include <winsock2.h>

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <stdio.h>
#include <assert.h>
#include <tchar.h>
#include <commctrl.h>

#ifdef WIN32
#pragma warning( push )
// warning C4819: The file contains a character that cannot be represented in the current code page (936). Save the file in Unicode format to prevent data loss
#pragma warning( disable : 4819 ) 
#endif

#include <D3DX9.h>

#include "DXUtil.h"
#include "D3DEnumeration.h"
#include "D3DSettings.h"
// #include "D3DApp.h"
#include "D3DRes.h"

#ifdef WIN32
#pragma warning( pop ) 
#endif

/** use direct input 8 interface */
#define DIRECTINPUT_VERSION		0x0800
#include <dinput.h>

#ifndef V_RETURN
#define V_RETURN(x)    { hr = x; if( FAILED(hr) ) { return hr; } }
#endif

#ifndef MAX_STRING_LENGTH
#define MAX_STRING_LENGTH 0xffff
#endif

#define PE_CORE_EXPORTING
#include "coreexport.h"

#endif // !defined(DXSDK_STDAFX_H)

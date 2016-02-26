//-----------------------------------------------------------------------------
// Copyright (C) 2004 - 2006 ParaEngine Dev Studio, All Rights Reserved.
// Date:	2006.8
// Description:	includes this header when making plug-in with managed C++. 
//-----------------------------------------------------------------------------
#pragma once

#define MANAGED_PARAENGINE

typedef unsigned long DWORD;
typedef void *HINSTANCE;
typedef void* HWND;

#define FALSE   0
#define TRUE    1
#define NULL    0

typedef struct tagRECT
{
	long left;
	long top;
	long right;
	long bottom;
} 	RECT;
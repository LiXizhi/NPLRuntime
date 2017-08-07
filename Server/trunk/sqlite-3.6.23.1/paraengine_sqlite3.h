#pragma once
//-----------------------------------------------------------------------------
// Copyright (C) 2004 - 2010 ParaEngine Co., All Rights Reserved.
// Author: LiXizhi
// Date:	2010.4
// Description: This will compile sqlite3 as dll in win32.
//-----------------------------------------------------------------------------
#ifdef WIN32

  #ifdef PE_CORE_EXPORTING
    #define SQLITE_API    __declspec(dllimport)
  #else
    #define SQLITE_API    __declspec(dllexport)
  #endif

#else

#endif
extern void WriteLog(const char* zFormat, ...);
// #define sqlite3DebugPrintf WriteLog
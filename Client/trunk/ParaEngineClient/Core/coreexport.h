//-----------------------------------------------------------------------------
// Copyright (C) 2004 - 2006 ParaEngine Dev Studio, All Rights Reserved.
// Date:	2006.8
// Description: 
//-----------------------------------------------------------------------------
#pragma once

#ifdef WIN32

#ifdef PE_CORE_EXPORTING
#define PE_CORE_DECL    __declspec(dllexport)
#define EXPIMP_TEMPLATE
#else
#define PE_CORE_DECL    __declspec(dllimport)
#define EXPIMP_TEMPLATE extern
#endif

#else

#ifdef PE_CORE_EXPORTING
// #define PE_CORE_DECL    
#define PE_CORE_DECL    __attribute__ ((visibility("default")))
#define EXPIMP_TEMPLATE
#else
#define PE_CORE_DECL
#define EXPIMP_TEMPLATE extern
#endif

#endif

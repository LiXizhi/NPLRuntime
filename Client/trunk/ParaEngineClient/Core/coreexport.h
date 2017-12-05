//-----------------------------------------------------------------------------
// Copyright (C) 2004 - 2006 ParaEngine Dev Studio, All Rights Reserved.
// Date:	2006.8
// Description: 
//-----------------------------------------------------------------------------
#pragma once

#ifdef WIN32
	#ifdef PE_CORE_EXPORTING
		#define PE_CORE_DECL    __declspec(dllexport)
	#else
		#define PE_CORE_DECL    __declspec(dllimport)
	#endif

#else

	#ifdef PE_CORE_EXPORTING
		#define PE_CORE_DECL    __attribute__ ((visibility("default")))
	#else
		#define PE_CORE_DECL
	#endif
#endif
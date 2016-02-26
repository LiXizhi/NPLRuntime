#pragma once
#ifdef PARAENGINE_CLIENT
// VC++ uses this macro as debug/release mode indicator 
#ifdef _DEBUG 
// Need to undef new if including crtdbg.h which redefines new itself 
#ifdef new 
#undef new 
#endif 

// we need this to show file & line number of the allocation that caused 
// the leak 
#define _CRTDBG_MAP_ALLOC 
#include <stdlib.h> 
#ifndef _CRTBLD 
// Need when builded with pure MS SDK 
#define _CRTBLD 
#endif 

#include <crtdbg.h> 

// this define works around a bug with inline declarations of new, see 
// 
//      http://support.microsoft.com/support/kb/articles/Q140/8/58.asp 
// 
// for the details 
#define new  new( _NORMAL_BLOCK, __FILE__, __LINE__) 

#endif 
#endif
#pragma once

#ifdef WIN32
#include <windows.h>
#else
#include <dlfcn.h>
#endif
#include <string>
#include <time.h>

namespace ParaEngine
{
	///////////////////////////////////////////////////////
	//
	// cross platform Load Library
	//
	///////////////////////////////////////////////////////
#ifdef WIN32
#define CLIB_DEFHANDLE	((void *)-1)
#else
#if defined(RTLD_DEFAULT)
#define CLIB_DEFHANDLE	RTLD_DEFAULT
#elif (PARA_TARGET_PLATFORM == PARA_PLATFORM_MAC) || (PARA_TARGET_PLATFORM == PARA_PLATFORM_IOS)
#if defined(RTLD_MAIN_ONLY)
	// this is mostly for iOS?
#define CLIB_DEFHANDLE	RTLD_MAIN_ONLY
#else
#define CLIB_DEFHANDLE	((void *)(intptr_t)-2)
#endif
#else
#define CLIB_DEFHANDLE	NULL
#endif
#endif

	/* same as LoadLibrary in windows or dlopen() in linux
	#define RTLD_LAZY   1
	#define RTLD_NOW    2
	#define RTLD_GLOBAL 4
	*/
	PE_CORE_DECL void* LoadLibrary(const char *pcDllname, int iMode = 2);
	/** @param Lib: this can be CLIB_DEFHANDLE for extern "C" namespace for clib. */
	PE_CORE_DECL void *GetProcAddress(void *Lib, const char *Fnname);
	PE_CORE_DECL bool FreeLibrary(void *hDLL);


	///////////////////////////////////////////////////////
	//
	// file time functions
	//
	///////////////////////////////////////////////////////

	// GetLastWriteTime - Retrieves the last-write time and converts
	//                    the time to a string
	//
	// Return value - TRUE if successful, FALSE otherwise
	// hFile      - Valid file handle
	// lpszString - Pointer to buffer to receive string
	PE_CORE_DECL bool GetLastFileWriteTime(const char* filename, time_t& ftWrite);


	///////////////////////////////////////////////////////
	//
	// misc functions
	//
	///////////////////////////////////////////////////////

	/** get the id of the calling thread. */
	PE_CORE_DECL int GetThisThreadID();

	/** get the process id */
	PE_CORE_DECL int GetProcessID();

	/**
	* Returns the peak (maximum so far) resident set size (physical
	* memory use) measured in bytes, or zero if the value cannot be
	* determined on this OS.
	*/
	PE_CORE_DECL size_t GetPeakMemoryUse();

	/**
	* Returns the current resident set size (physical memory use) measured
	* in bytes, or zero if the value cannot be determined on this OS.
	*/
	PE_CORE_DECL size_t GetCurrentMemoryUse();

	/** get current executable path.
	*/
	PE_CORE_DECL std::string GetExecutablePath();
}

///////////////////////////////////////////////////////
//
// cross platform _findfirst()
//
///////////////////////////////////////////////////////

#ifndef WIN32
#include <dirent.h>

#define _A_NORMAL 		0x00 		// Normal file
#define _A_RDONLY 		0x01 		// Read-only file
#define _A_HIDDEN 		0x02 		// Hidden file
#define _A_SYSTEM 		0x04 		// System file
#define _A_VOLID 			0x08 		// Volume ID
#define _A_SUBDIR 		0x10 		// Subdirectory
#define _A_ARCH 			0x20 		// File changed since last archive
#define FA_NORMAL 		0x00 		// Synonym of _A_NORMAL
#define FA_RDONLY 		0x01 		// Synonym of _A_RDONLY
#define FA_HIDDEN 		0x02 		// Synonym of _A_HIDDEN
#define FA_SYSTEM 		0x04 		// Synonym of _A_SYSTEM
#define FA_LABEL 			0x08 		// Synonym of _A_VOLID
#define FA_DIREC 			0x10 		// Synonym of _A_SUBDIR
#define FA_ARCH 			0x20 		// Synonym of _A_ARCH


const unsigned STRING_BUFFER_SIZE = 512;

typedef struct _finddata_t
{
	char            name[STRING_BUFFER_SIZE];
	int            attrib;
	unsigned long   size;
} _finddata;

/** Hold information about the current search
*/
typedef struct _findinfo_t
{
	DIR*	openedDir;
	std::string filter;
	std::string dirName;
} _findinfo;

long _findfirst(const char *name, _finddata_t *f);
int _findnext(long h, _finddata_t *f);
int _findclose(long h);


#endif // #ifndef WIN32
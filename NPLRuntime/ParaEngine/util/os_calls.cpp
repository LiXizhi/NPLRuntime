//-----------------------------------------------------------------------------
// Class:	cross platform operating system functions 
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2009.4.14
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "os_calls.h"
#include "FileUtils.h"

#include <sys/stat.h>
#include <sys/types.h>

#include <boost/log/attributes/current_process_name.hpp>

#ifdef WIN32
#include <io.h>
#include <stdio.h>
#else
#include <unistd.h>
#endif

#if defined(NPL_USE_READLINE)
#include <readline/readline.h>
#include <readline/history.h>
#endif

#include "StringHelper.h"

///////////////////////////////////////////////////////
//
// misc functions
//
///////////////////////////////////////////////////////

int ParaEngine::GetThisThreadID()
{
#ifdef WIN32
	return (int)(::GetCurrentThreadId());
#elif (PARA_TARGET_PLATFORM == PARA_PLATFORM_IOS || PARA_TARGET_PLATFORM == PARA_PLATFORM_MAC)
	return (int)pthread_mach_thread_np(pthread_self());
#else
	//return (int)gettid();
	return (int)pthread_self();
#endif
}

int ParaEngine::GetProcessID()
{
#ifdef WIN32
	return (int)GetCurrentProcessId();
#else
	return (int)::getpid();
#endif
}

///////////////////////////////////////////////////////
//
// cross platform Load Library
//
///////////////////////////////////////////////////////

void* ParaEngine::LoadLibrary(const char *pcDllname, int iMode)
{
	std::string sDllName = pcDllname;
#ifdef WIN32 // Microsoft compiler
	if (sDllName.find(".") == string::npos)
		sDllName += ".dll";
#define USE_ABS_DLL_PATH
#ifdef USE_ABS_DLL_PATH
	// convert relative path to absolute path, just in case some dev folder or writable folder is used
	if (!(CFileUtils::IsAbsolutePath(sDllName)) && sDllName.find_first_of("/\\"))
	{
		string sDllFullpath = CParaFile::GetAbsolutePath(sDllName, CParaFile::GetCurDirectory(CParaFile::APP_ROOT_DIR));
		if (!CParaFile::DoesFileExist(sDllFullpath.c_str()))
		{
			sDllFullpath = CParaFile::GetAbsolutePath(sDllName, CParaFile::GetCurDirectory(CParaFile::APP_DEV_DIR));
			if (!CParaFile::DoesFileExist(sDllFullpath.c_str()))
			{
				sDllFullpath = CParaFile::GetAbsolutePath(sDllName, CParaFile::GetWritablePath());
				if (!CParaFile::DoesFileExist(sDllFullpath.c_str()))
				{
					sDllFullpath.clear();
				}
			}
		}
		if (!sDllFullpath.empty())
			sDllName = sDllFullpath;
		CParaFile::ToCanonicalFilePath(sDllName, sDllName);
		OUTPUT_LOG("Absolute path is used for dll: %s\n", sDllName.c_str());
	}
#endif
#ifndef LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR
	return (void*)::LoadLibraryEx(sDllName.c_str(), NULL, 0);
#else
#if WIN32 && defined(DEFAULT_FILE_ENCODING)
	LPCWSTR sDllName16 = StringHelper::MultiByteToWideChar(sDllName.c_str(), DEFAULT_FILE_ENCODING);
	return (void*)::LoadLibraryExW(sDllName16, NULL,
		LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_APPLICATION_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS | LOAD_LIBRARY_SEARCH_SYSTEM32 | LOAD_LIBRARY_SEARCH_USER_DIRS);
#else
	return (void*)::LoadLibraryEx(sDllName.c_str(), NULL,
		LOAD_LIBRARY_SEARCH_DLL_LOAD_DIR | LOAD_LIBRARY_SEARCH_APPLICATION_DIR | LOAD_LIBRARY_SEARCH_DEFAULT_DIRS | LOAD_LIBRARY_SEARCH_SYSTEM32 | LOAD_LIBRARY_SEARCH_USER_DIRS);
#endif
	
#endif
#else
	if (sDllName.find(".") == string::npos)
		sDllName += ".so";
	void* ret = dlopen(sDllName.c_str(), iMode);
	if (!ret)
	{
		const char* error = dlerror();
		OUTPUT_LOG(error);
	}
	return ret;
	
#endif


}
void * ParaEngine::GetProcAddress(void *Lib, const char *Fnname)
{
#ifdef WIN32 // Microsoft compiler
	if (Lib == CLIB_DEFHANDLE)
	{
		void *p = NULL;
		/* Default libraries. */
		enum {
			CLIB_HANDLE_EXE, // ParaEngineClient.exe
			CLIB_HANDLE_DLL, // ParaEngineClient(_d).dll
			CLIB_HANDLE_MAX,
		};
		static void *clib_def_handle[CLIB_HANDLE_MAX] = { 0,0 };

		for (int i = 0; i < CLIB_HANDLE_MAX; i++)
		{
			HINSTANCE h = (HINSTANCE)clib_def_handle[i];
			if (!(void *)h) {  /* Resolve default library handles (once). */
				switch (i) {
				case CLIB_HANDLE_EXE:
					GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, NULL, &h);
					break;
				case CLIB_HANDLE_DLL:
					GetModuleHandleExA(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_UNCHANGED_REFCOUNT, (const char *)clib_def_handle, &h);
					break;
				}
				if (!h) continue;
				clib_def_handle[i] = (void *)h;
			}
			p = (void *)::GetProcAddress(h, Fnname);
			if (p)
				break;
		}
		return p;
	}
	else
		return (void*)::GetProcAddress((HINSTANCE)Lib, Fnname);
#else
	return dlsym(Lib, Fnname);
#endif
}

bool ParaEngine::FreeLibrary(void *hDLL)
{
	if (hDLL == CLIB_DEFHANDLE)
		return true;
#ifdef WIN32 // Microsoft compiler
	return !!::FreeLibrary((HINSTANCE)hDLL);
#else
	return dlclose(hDLL);
#endif
}

//////////////////////////////////////////////////////////////////////////
//
// file time stat() functions
//
//////////////////////////////////////////////////////////////////////////

bool ParaEngine::GetLastFileWriteTime(const char* filename, time_t& ftWrite)
{
	struct stat filestat;
	if (stat(filename, &filestat) != 0)
	{
		ftWrite = filestat.st_mtime;
		return true;
	}
	return false;
}

///////////////////////////////////////////////////////
//
// cross platform _findfirst()
//
///////////////////////////////////////////////////////

#ifndef WIN32

#include <fnmatch.h>

std::vector< _findinfo_t* > fileInfo;

/**
* _findfirst - equivalent
*/
long _findfirst(const char *name, _finddata_t *f)
{
	std::string nameCopy = name;
	std::string filter;

	// This is linux only, so don't bother with '\'
	const char* lastSep = strrchr(name, '/');
	if (!lastSep)
	{
		// filter pattern only is given, search current directory.
		filter = nameCopy;
		nameCopy = ".";
	}
	else
	{
		// strip filter pattern from directory name, leave
		// trailing '/' intact.
		filter = lastSep + 1;
		unsigned sepIndex = lastSep - name;
		nameCopy.erase(sepIndex + 1, nameCopy.size() - sepIndex - 1);
	}

	DIR* dir = opendir(nameCopy.c_str());

	if (!dir) return -1;

	_findinfo_t* fi = new _findinfo_t();
	fi->filter = filter;
	fi->dirName = nameCopy;  // we need to remember this for stat()
	fi->openedDir = dir;
	fileInfo.push_back(fi);

	long ret = fileInfo.size() - 1;

	// Retrieve the first file. We cannot rely on the first item
	// being '.'
	if (_findnext(ret, f) == -1) return -1;
	else return ret;
}

int _findnext(long h, _finddata_t *f)
{
	PE_ASSERT(h >= 0 && h < (long)fileInfo.size());
	if (h < 0 || h >= (long)fileInfo.size()) return -1;

	_findinfo_t* fi = fileInfo[h];

	while (true)
	{
		dirent* entry = readdir(fi->openedDir);
		if (entry == 0) return -1;

		// Only report stuff matching our filter
		if (fnmatch(fi->filter.c_str(), entry->d_name, FNM_PATHNAME) != 0) continue;

		// To reliably determine the entry's type, we must do
		// a stat...  don't rely on entry->d_type, as this
		// might be unavailable!
		struct stat filestat;
		std::string fullPath = fi->dirName + entry->d_name;
		if (stat(fullPath.c_str(), &filestat) != 0)
		{
			OUTPUT_LOG("Cannot stat %s\n", fullPath.c_str());
			continue;
		}

		if (S_ISREG(filestat.st_mode))
		{
			f->attrib = _A_NORMAL;
		}
		else if (S_ISDIR(filestat.st_mode))
		{
			f->attrib = _A_SUBDIR;
		}
		else continue; // We are interested in files and
					   // directories only. Links currently
					   // are not supported.

		f->size = filestat.st_size;
		strncpy(f->name, entry->d_name, STRING_BUFFER_SIZE);

		return 0;
	}

	return -1;
}


/**
* _findclose - equivalent
*/
int _findclose(long h)
{
	if (h == -1) return 0;

	if (h < 0 || h >= (long)fileInfo.size())
	{
		PE_ASSERT(false);
		return -1;
	}

	_findinfo_t* fi = fileInfo[h];
	closedir(fi->openedDir);
	fileInfo.clear();
	delete fi;
	return 0;
}

#endif // #ifndef WIN32


///////////////////////////////////////////////////////
//
// cross platform memory usage
//
///////////////////////////////////////////////////////

/*
* Author:  David Robert Nadeau
* Site:    http://NadeauSoftware.com/
* License: Creative Commons Attribution 3.0 Unported License
*          http://creativecommons.org/licenses/by/3.0/deed.en_US
*/

#if defined(_WIN32)
#include <psapi.h>

#elif defined(__unix__) || defined(__unix) || defined(unix) || (defined(__APPLE__) && defined(__MACH__))
#include <unistd.h>
#include <sys/resource.h>

#if defined(__APPLE__) && defined(__MACH__)
#include <mach/mach.h>
#include <mach-o/dyld.h>

#elif (defined(_AIX) || defined(__TOS__AIX__)) || (defined(__sun__) || defined(__sun) || defined(sun) && (defined(__SVR4) || defined(__svr4__)))
#include <fcntl.h>
#include <procfs.h>

#elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__gnu_linux__)
#include <stdio.h>
#include <limits.h>
#endif

#else
#error "Cannot define getPeakRSS( ) or getCurrentRSS( ) for an unknown OS."
#endif

PE_CORE_DECL size_t ParaEngine::GetPeakMemoryUse()
{
#if defined(_WIN32) && defined(PARAENGINE_CLIENT) && !defined(NPLRUNTIME)
	/* Windows -------------------------------------------------- */
	PROCESS_MEMORY_COUNTERS info;
	GetProcessMemoryInfo(GetCurrentProcess(), &info, sizeof(info));
	return (size_t)info.PeakWorkingSetSize;

#elif (defined(_AIX) || defined(__TOS__AIX__)) || (defined(__sun__) || defined(__sun) || defined(sun) && (defined(__SVR4) || defined(__svr4__)))
	/* AIX and Solaris ------------------------------------------ */
	struct psinfo psinfo;
	int fd = -1;
	if ((fd = open("/proc/self/psinfo", O_RDONLY)) == -1)
		return (size_t)0L;		/* Can't open? */
	if (read(fd, &psinfo, sizeof(psinfo)) != sizeof(psinfo))
	{
		close(fd);
		return (size_t)0L;		/* Can't read? */
	}
	close(fd);
	return (size_t)(psinfo.pr_rssize * 1024L);

#elif defined(__unix__) || defined(__unix) || defined(unix) || (defined(__APPLE__) && defined(__MACH__))
	/* BSD, Linux, and OSX -------------------------------------- */
	struct rusage rusage;
	getrusage(RUSAGE_SELF, &rusage);
#if defined(__APPLE__) && defined(__MACH__)
	return (size_t)rusage.ru_maxrss;
#else
	return (size_t)(rusage.ru_maxrss * 1024L);
#endif

#else
	/* Unknown OS ----------------------------------------------- */
	return (size_t)0L;			/* Unsupported. */
#endif
}

PE_CORE_DECL size_t ParaEngine::GetCurrentMemoryUse()
{
#if defined(_WIN32) && defined(PARAENGINE_CLIENT) && !defined(NPLRUNTIME)
	/* Windows -------------------------------------------------- */

	PROCESS_MEMORY_COUNTERS info;
	GetProcessMemoryInfo(GetCurrentProcess(), &info, sizeof(info));
	return (size_t)info.WorkingSetSize;
#elif defined(__APPLE__) && defined(__MACH__)
	/* OSX ------------------------------------------------------ */
	struct mach_task_basic_info info;
	mach_msg_type_number_t infoCount = MACH_TASK_BASIC_INFO_COUNT;
	if (task_info(mach_task_self(), MACH_TASK_BASIC_INFO,
		(task_info_t)&info, &infoCount) != KERN_SUCCESS)
		return (size_t)0L;		/* Can't access? */
	return (size_t)info.resident_size;

#elif defined(__linux__) || defined(__linux) || defined(linux) || defined(__gnu_linux__)
	/* Linux ---------------------------------------------------- */
	long rss = 0L;
	FILE* fp = NULL;
	if ((fp = fopen("/proc/self/statm", "r")) == NULL)
		return (size_t)0L;		/* Can't open? */
	if (fscanf(fp, "%*s%ld", &rss) != 1)
	{
		fclose(fp);
		return (size_t)0L;		/* Can't read? */
	}
	fclose(fp);
	return (size_t)rss * (size_t)sysconf(_SC_PAGESIZE);

#else
	/* AIX, BSD, Solaris, and Unknown OS ------------------------ */
	return (size_t)0L;			/* Unsupported. */
#endif
}

PE_CORE_DECL std::string ParaEngine::GetExecutablePath()
{
#if (PARA_TARGET_PLATFORM == PARA_PLATFORM_ANDROID) || defined(ANDROID)
	return std::string();
#elif (PARA_TARGET_PLATFORM == PARA_PLATFORM_LINUX) || (PARA_TARGET_PLATFORM == PARA_PLATFORM_MAC) || defined(WIN32)
	return boost::log::aux::get_process_name();
#else
	return std::string();
#endif
}

PE_CORE_DECL bool ParaEngine::stdin_is_tty()
{
#ifdef WIN32
	return _isatty(_fileno(stdin)) != 0;
#elif (PARA_TARGET_PLATFORM == PARA_PLATFORM_LINUX || PARA_TARGET_PLATFORM == PARA_PLATFORM_MAC)
	return isatty(0) != 0;
#else
	return 1; // assume it is tty
#endif
}

#define NPL_MAXINPUT 512
PE_CORE_DECL bool ParaEngine::ReadLine(std::string& output, const char* prmt)
{
#if defined(NPL_USE_READLINE)
	char* buf = readline(prmt);
	if (buf != NULL)
	{
		output = buf;

		if (!output.empty())  /* non-empty line? */ 
			add_history(output.c_str());  /* add it to history */
		free(buf);
		return true;
	}
	return false;
#else
	char buf[NPL_MAXINPUT+1];
	if (prmt != 0 && prmt[0]!=0)
	{
		fputs(prmt, stdout);
		fflush(stdout);
	}

	if (fgets(buf, NPL_MAXINPUT, stdin) != NULL)  /* get line */
	{
		size_t len = strlen(buf);
		if (len > 0 && buf[len - 1] == '\n')
		{ 
			buf[len - 1] = '\0';
			output = buf;
		}
		return true;
	}
	return false;
#endif
}

//-----------------------------------------------------------------------------
// Class:	COSInfo
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine Tech Studio
// Date:	2010.4
// Desc: Operating system information for win32
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "OSWindows.h"
#include <strsafe.h>
#pragma comment(lib, "User32.lib")

using namespace ParaEngine;

/** default windows raw message queue size */
#define DEFAULT_WINMSG_QUEUE_SIZE	100

CWinRawMsgQueue::CWinRawMsgQueue()
: NPL::concurrent_ptr_queue<CWinRawMsg_ptr>(DEFAULT_WINMSG_QUEUE_SIZE)
{
	SetUseEvent(false);
}

CWinRawMsgQueue::CWinRawMsgQueue(int capacity)
: NPL::concurrent_ptr_queue<CWinRawMsg_ptr>(capacity)
{
	SetUseEvent(false);
}

CWinRawMsgQueue::~CWinRawMsgQueue()
{
}


#define BUFSIZE 256
typedef void (WINAPI *PGNSI)(LPSYSTEM_INFO);
typedef BOOL (WINAPI *PGPI)(DWORD, DWORD, DWORD, DWORD, PDWORD);
typedef LONG (WINAPI *PRtlGetVersion)(PRTL_OSVERSIONINFOW);

static bool QueryRealOSVersion(OSVERSIONINFOEXW& osvi)
{
	ZeroMemory(&osvi, sizeof(osvi));
	osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEXW);
	HMODULE hNtdll = ::GetModuleHandleW(L"ntdll.dll");
	if (hNtdll)
	{
		auto pRtlGetVersion = reinterpret_cast<PRtlGetVersion>(::GetProcAddress(hNtdll, "RtlGetVersion"));
		if (pRtlGetVersion && pRtlGetVersion(reinterpret_cast<PRTL_OSVERSIONINFOW>(&osvi)) == 0)
		{
			return true;
		}
	}
	return ::GetVersionExW(reinterpret_cast<LPOSVERSIONINFOW>(&osvi)) != FALSE;
}

BOOL COSInfo::GetOSDisplayString( LPTSTR pszOS)
{
	OSVERSIONINFOEXW osvi;
	SYSTEM_INFO si;
	PGNSI pGNSI;
	PGPI pGPI;
	DWORD dwType = 0;

	ZeroMemory(&si, sizeof(SYSTEM_INFO));
	if (!QueryRealOSVersion(osvi))
		return FALSE;

	pGNSI = (PGNSI) GetProcAddress(
		GetModuleHandle(TEXT("kernel32.dll")), 
		"GetNativeSystemInfo");
	if(NULL != pGNSI)
		pGNSI(&si);
	else 
		GetSystemInfo(&si);

	if (VER_PLATFORM_WIN32_NT == osvi.dwPlatformId && osvi.dwMajorVersion >= 5)
	{
		StringCchCopy(pszOS, BUFSIZE, TEXT("Microsoft "));

		if (osvi.dwMajorVersion == 10)
		{
			if (osvi.wProductType == VER_NT_WORKSTATION)
			{
				if (osvi.dwBuildNumber >= 22000)
					StringCchCat(pszOS, BUFSIZE, TEXT("Windows 11"));
				else
					StringCchCat(pszOS, BUFSIZE, TEXT("Windows 10"));
			}
			else
			{
				if (osvi.dwBuildNumber >= 20348)
					StringCchCat(pszOS, BUFSIZE, TEXT("Windows Server 2022"));
				else if (osvi.dwBuildNumber >= 17763)
					StringCchCat(pszOS, BUFSIZE, TEXT("Windows Server 2019"));
				else
					StringCchCat(pszOS, BUFSIZE, TEXT("Windows Server 2016"));
			}
		}
		else if (osvi.dwMajorVersion == 6)
		{
			if (osvi.dwMinorVersion == 3)
				StringCchCat(pszOS, BUFSIZE, osvi.wProductType == VER_NT_WORKSTATION ? TEXT("Windows 8.1 ") : TEXT("Windows Server 2012 R2 "));
			else if (osvi.dwMinorVersion == 2)
				StringCchCat(pszOS, BUFSIZE, osvi.wProductType == VER_NT_WORKSTATION ? TEXT("Windows 8 ") : TEXT("Windows Server 2012 "));
			else if (osvi.dwMinorVersion == 1)
				StringCchCat(pszOS, BUFSIZE, osvi.wProductType == VER_NT_WORKSTATION ? TEXT("Windows 7 ") : TEXT("Windows Server 2008 R2 "));
			else if (osvi.dwMinorVersion == 0)
				StringCchCat(pszOS, BUFSIZE, osvi.wProductType == VER_NT_WORKSTATION ? TEXT("Windows Vista ") : TEXT("Windows Server 2008 "));

			pGPI = (PGPI) GetProcAddress(GetModuleHandle(TEXT("kernel32.dll")), "GetProductInfo");
			if (pGPI && pGPI(osvi.dwMajorVersion, osvi.dwMinorVersion, 0, 0, &dwType))
			{
				switch (dwType)
				{
				case PRODUCT_ULTIMATE: StringCchCat(pszOS, BUFSIZE, TEXT("Ultimate Edition")); break;
				case PRODUCT_HOME_PREMIUM: StringCchCat(pszOS, BUFSIZE, TEXT("Home Premium Edition")); break;
				case PRODUCT_HOME_BASIC: StringCchCat(pszOS, BUFSIZE, TEXT("Home Basic Edition")); break;
				case PRODUCT_ENTERPRISE: StringCchCat(pszOS, BUFSIZE, TEXT("Enterprise Edition")); break;
				case PRODUCT_BUSINESS: StringCchCat(pszOS, BUFSIZE, TEXT("Business Edition")); break;
				case PRODUCT_STARTER: StringCchCat(pszOS, BUFSIZE, TEXT("Starter Edition")); break;
				default: break;
				}
			}
		}
		else
		{
			StringCchCat(pszOS, BUFSIZE, TEXT("Windows NT"));
		}

		TCHAR buf[80];
		StringCchPrintf(buf, 80, TEXT(" %u.%u (build %u)"), osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.dwBuildNumber);
		StringCchCat(pszOS, BUFSIZE, buf);

		if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64)
			StringCchCat(pszOS, BUFSIZE, TEXT(", 64-bit"));
		else if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_ARM64)
			StringCchCat(pszOS, BUFSIZE, TEXT(", ARM64"));
		else if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL)
			StringCchCat(pszOS, BUFSIZE, TEXT(", 32-bit"));

		return TRUE;
	}
	return FALSE;
}

/** dump system info */
void COSInfo::DumpSystemInfo()
{
	OUTPUT_LOG("OS information:\n%s\n", GetSystemInfoString().c_str());
}


DWORD ParaEngine::COSInfo::GetOSMajorVersion()
{
	OSVERSIONINFOEXW osvi;
	if (QueryRealOSVersion(osvi))
	{
		return osvi.dwMajorVersion;
	}
	OUTPUT_LOG("error GetOSMajorVersion()\n");
	return (DWORD)-1;
}

const std::string& ParaEngine::COSInfo::GetSystemInfoString( bool bRefresh /*= false*/ )
{
	static std::string g_last_info;
	if(!g_last_info.empty() && !bRefresh)
		return g_last_info;

	g_last_info.clear();
	g_last_info.reserve(512);

	char buf[512];
	#define OUTPUT_OS_INFO(format, ...)	snprintf(buf, 512, format, ## __VA_ARGS__); g_last_info+=buf;

	SYSTEM_INFO siSysInfo;
	TCHAR szOS[BUFSIZE];

	if( GetOSDisplayString( szOS ) )
	{
		OUTPUT_OS_INFO("OS version:%s\n", szOS);
	}

	HKEY hCpuKey = NULL;
	if (::RegOpenKeyExA(HKEY_LOCAL_MACHINE, "HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0", 0, KEY_READ, &hCpuKey) == ERROR_SUCCESS)
	{
		char cpuBrand[256] = { 0 };
		DWORD cbData = sizeof(cpuBrand) - 1;
		DWORD dwType = 0;
		if (::RegQueryValueExA(hCpuKey, "ProcessorNameString", NULL, &dwType, reinterpret_cast<LPBYTE>(cpuBrand), &cbData) == ERROR_SUCCESS && dwType == REG_SZ)
		{
			OUTPUT_OS_INFO("CPU:%s\n", cpuBrand);
		}
		::RegCloseKey(hCpuKey);
	}

	// Copy the hardware information to the SYSTEM_INFO structure. 
	GetSystemInfo(&siSysInfo); 

	// Display the contents of the SYSTEM_INFO structure. 
	OUTPUT_OS_INFO("Number of processors:%u\n", siSysInfo.dwNumberOfProcessors); 
	OUTPUT_OS_INFO("Processor type:%u\n", siSysInfo.dwProcessorType); 

	// Get memory info
	MEMORYSTATUSEX statex;
	statex.dwLength = sizeof (statex);
	GlobalMemoryStatusEx (&statex);

	// Use to convert bytes to KB
	#define DIV 1024

	OUTPUT_OS_INFO("TotalPhysicalMemory:%llu\n", (unsigned long long)(statex.ullTotalPhys/DIV)); 
	OUTPUT_OS_INFO("FreePhysicalMemory:%llu\n", (unsigned long long)(statex.ullAvailPhys/DIV)); 
	return g_last_info;
}

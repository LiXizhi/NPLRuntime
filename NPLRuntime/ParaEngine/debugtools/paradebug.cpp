//-----------------------------------------------------------------------------
// Class:	
// Authors:	
// Emails:	
// Company: ParaEngine
// Date:	2005.11.8
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#if defined(WIN32) && !defined(PARAENGINE_MOBILE) && !defined(NPLRUNTIME_SERVER)
#pragma warning(disable:4995)
#include "ParaEngineInfo.h"
#include "paradebug.h"
#include <strsafe.h>

#include <dbghelp.h>
#include <shellapi.h>
#include <shlobj.h>

#ifdef _MSC_VER
#pragma comment(lib, "Dbghelp.lib")
#endif

#include <vector>
#include <string>
using namespace std;
using namespace ParaEngine;


/** call this function to save mini-dump to the current directory.
dump file name from the application name, version number, process id, thread id, day, and time to avoid filename collisions 
this function is not thread-safe.
*/
int GenerateDump(EXCEPTION_POINTERS* pExceptionPointers)
{
	BOOL bMiniDumpSuccessful = FALSE;
	char szFileName[MAX_PATH] = { 0 };
	string szAppName = "ParaEngine";
	string szVersion = ParaEngineInfo::CParaEngineInfo::GetVersion();

	HANDLE hDumpFile = INVALID_HANDLE_VALUE;
	SYSTEMTIME stLocalTime;
	MINIDUMP_EXCEPTION_INFORMATION ExpParam;

	GetLocalTime(&stLocalTime);

	StringCchPrintfA(szFileName, MAX_PATH, "%s_%s-%04d%02d%02d-%02d%02d%02d-%ld-%ld.dmp",
		szAppName.c_str(), szVersion.c_str(),
		stLocalTime.wYear, stLocalTime.wMonth, stLocalTime.wDay,
		stLocalTime.wHour, stLocalTime.wMinute, stLocalTime.wSecond,
		GetCurrentProcessId(), GetCurrentThreadId());
	hDumpFile = CreateFileA(szFileName, GENERIC_READ | GENERIC_WRITE,
		FILE_SHARE_WRITE | FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);

	if (hDumpFile != INVALID_HANDLE_VALUE)
	{
		ExpParam.ThreadId = GetCurrentThreadId();
		ExpParam.ExceptionPointers = pExceptionPointers;
		ExpParam.ClientPointers = TRUE;

		bMiniDumpSuccessful = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(),
			hDumpFile, MiniDumpWithDataSegs, &ExpParam, NULL, NULL);
		CloseHandle(hDumpFile);
	}

	OUTPUT_LOG("Application crash: min dump generated at %s (success=%d)\r\n", szFileName, (int)bMiniDumpSuccessful);

	return EXCEPTION_EXECUTE_HANDLER;
}

string LookupError(HRESULT errorcode)
{
	HRESULT hrErr = errorcode;
	char strHRESULTCopy[MAX_PATH * 2] = { 0 };

	if (hrErr != 0)
	{
		char strInternals[100] = { 0 };
		char strFacility[100] = { 0 };
		char strDescription[MAX_PATH] = { 0 };

		StringCchPrintfA(strHRESULTCopy, MAX_PATH * 2, "HRESULT: 0x%0.8x (%lu)",
			(unsigned int)hrErr, (unsigned long)hrErr);

		DWORD dwLen = ::FormatMessageA(
			FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
			NULL,
			(DWORD)hrErr,
			MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
			strDescription,
			MAX_PATH - 1,
			NULL);
		if (dwLen > 0)
		{
			StringCchCatA(strHRESULTCopy, MAX_PATH * 2, "\r\nDescription: ");
			StringCchCatA(strHRESULTCopy, MAX_PATH * 2, strDescription);
		}

		StringCchPrintfA(strInternals, 100, "\r\nSeverity code: %s",
			(HRESULT_SEVERITY(hrErr) == 1) ? "Failed" : "Success");
		StringCchCatA(strHRESULTCopy, MAX_PATH * 2, strInternals);

		int nFacility = HRESULT_FACILITY(hrErr);
		switch (nFacility)
		{
		case FACILITY_WINDOWS_CE:  StringCchCopyA(strFacility, 100, "FACILITY_WINDOWS_CE"); break;
		case FACILITY_WINDOWS: StringCchCopyA(strFacility, 100, "FACILITY_WINDOWS"); break;
		case FACILITY_URT: StringCchCopyA(strFacility, 100, "FACILITY_URT"); break;
		case FACILITY_UMI: StringCchCopyA(strFacility, 100, "FACILITY_UMI"); break;
		case FACILITY_SXS: StringCchCopyA(strFacility, 100, "FACILITY_SXS"); break;
		case FACILITY_STORAGE: StringCchCopyA(strFacility, 100, "FACILITY_STORAGE"); break;
		case FACILITY_STATE_MANAGEMENT: StringCchCopyA(strFacility, 100, "FACILITY_STATE_MANAGEMENT"); break;
		case FACILITY_SCARD: StringCchCopyA(strFacility, 100, "FACILITY_SCARD"); break;
		case FACILITY_SETUPAPI: StringCchCopyA(strFacility, 100, "FACILITY_SETUPAPI"); break;
		case FACILITY_SECURITY: StringCchCopyA(strFacility, 100, "FACILITY_SECURITY or FACILITY_SSPI"); break;
		case FACILITY_RPC: StringCchCopyA(strFacility, 100, "FACILITY_RPC"); break;
		case FACILITY_WIN32: StringCchCopyA(strFacility, 100, "FACILITY_WIN32"); break;
		case FACILITY_CONTROL: StringCchCopyA(strFacility, 100, "FACILITY_CONTROL"); break;
		case FACILITY_NULL: StringCchCopyA(strFacility, 100, "FACILITY_NULL"); break;
		case FACILITY_MSMQ: StringCchCopyA(strFacility, 100, "FACILITY_MSMQ"); break;
		case FACILITY_MEDIASERVER: StringCchCopyA(strFacility, 100, "FACILITY_MEDIASERVER"); break;
		case FACILITY_INTERNET: StringCchCopyA(strFacility, 100, "FACILITY_INTERNET"); break;
		case FACILITY_ITF: StringCchCopyA(strFacility, 100, "FACILITY_ITF"); break;
		case FACILITY_HTTP: StringCchCopyA(strFacility, 100, "FACILITY_HTTP"); break;
		case FACILITY_DPLAY: StringCchCopyA(strFacility, 100, "FACILITY_DPLAY"); break;
		case FACILITY_DISPATCH: StringCchCopyA(strFacility, 100, "FACILITY_DISPATCH"); break;
		case FACILITY_CONFIGURATION: StringCchCopyA(strFacility, 100, "FACILITY_CONFIGURATION"); break;
		case FACILITY_COMPLUS: StringCchCopyA(strFacility, 100, "FACILITY_COMPLUS"); break;
		case FACILITY_CERT: StringCchCopyA(strFacility, 100, "FACILITY_CERT"); break;
		case FACILITY_BACKGROUNDCOPY: StringCchCopyA(strFacility, 100, "FACILITY_BACKGROUNDCOPY"); break;
		case FACILITY_ACS: StringCchCopyA(strFacility, 100, "FACILITY_ACS"); break;
		case FACILITY_AAF: StringCchCopyA(strFacility, 100, "FACILITY_AAF"); break;
		case FACILITY_D3DX: StringCchCopyA(strFacility, 100, "FACILITY_D3DX"); break;
		case FACILITY_D3D: StringCchCopyA(strFacility, 100, "FACILITY_D3D"); break;
		default: StringCchCopyA(strFacility, 100, "Unknown"); break;
		}
		StringCchPrintfA(strInternals, 100, "\r\nFacility Code: %s (%d)",
			strFacility, HRESULT_FACILITY(hrErr));
		StringCchCatA(strHRESULTCopy, MAX_PATH * 2, strInternals);

		StringCchPrintfA(strInternals, 100, "\r\nError Code: 0x%0.4x (%lu)",
			HRESULT_CODE(hrErr), HRESULT_CODE(hrErr));
		StringCchCatA(strHRESULTCopy, MAX_PATH * 2, strInternals);
	}

	return strHRESULTCopy;
}

void InterpretError(HRESULT hr, const char* file, int line)
{
	//this error will be written to log if the NOT_INTERPRETERROR is not defined.
#ifdef NOT_INTERPRETERROR
	return;
#endif
	switch (hr)
	{
	case S_OK:
		break;
	case E_UNEXPECTED:
		OUTPUT_LOG("Catastrophic failure Error at %s, line %d\n", file, line);
		break;
	case E_NOTIMPL:
		OUTPUT_LOG("Not implemented Error at %s, line %d\n", file, line);
		break;
	case E_OUTOFMEMORY:
		OUTPUT_LOG("Out of memory Error at %s, line %d\n", file, line);
		break;
	case E_INVALIDARG:
		OUTPUT_LOG("An invalid parameter was passed to the returning function at %s, line %d\n", file, line);
		break;
	case E_NOINTERFACE:
		OUTPUT_LOG("No object interface is available at %s, line %d\n", file, line);
		break;
	case E_POINTER:
		OUTPUT_LOG("Invalid pointer Error at %s, line %d\n", file, line);
		break;
	case E_HANDLE:
		OUTPUT_LOG("Invalid handle Error at %s, line %d\n", file, line);
		break;
	case E_ABORT:
		OUTPUT_LOG("Abort Error at %s, line %d\n", file, line);
		break;
	case E_FAIL:
		OUTPUT_LOG("An undetermined error occurred at %s, line %d\n", file, line);
		break;
	case E_ACCESSDENIED:
		OUTPUT_LOG("Access Denied Error at %s, line %d\n", file, line);
		break;
	case E_PENDING:
		OUTPUT_LOG("Pending Error at %s, line %d\n", file, line);
		break;
	default:
		OUTPUT_LOG("%s\nFile: %s\nLine: %d\n", LookupError(hr).c_str(), file, line);
	}
}

#endif

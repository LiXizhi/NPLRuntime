//-----------------------------------------------------------------------------
// Class:	
// Authors:	
// Emails:	
// Company: ParaEngine
// Date:	2005.11.8
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#ifdef PARAENGINE_CLIENT
#pragma warning(disable:4995)
#include "ParaEngineInfo.h"
#include "paradebug.h"
#include "sceneobject.h"
#include <dxerr.h>
#include <strsafe.h>

#include <dbghelp.h>
#include <shellapi.h>
#include <shlobj.h>

#include <vector>
#include <string>
using namespace std;
using namespace ParaEngine;


/** call this function to save mini-dump to the current directory.
dump file name from the application name, version number, process id, thread id, day, and time to avoid filename collisions 
this function is not thread-safe.

void SomeFunction()
{
    __try
    {
        int *pBadPtr = NULL;
        *pBadPtr = 0;
    }
    __except(GenerateDump(GetExceptionInformation()))
    {
		exit(0);
    }
}

Analysis of a minidump

[method1] (if crashed on the same computer)
Open Visual Studio
Select File->Open->Project
Select the minidump file
Run the debugger
The debugger will create a simulated process. The simulated process will be halted at the instruction that caused the crash.

[method2] (if crashed on a different computer)
Run windbg tool
set symbol table to D:\lxzsrc\ParaEngine\ParaWorld\Release\;srv*c:\cache*http://msdl.microsoft.com/download/symbols
wait a few seconds as symbols are downloaded from the web server. 
Type "ecxr" as instructed. Wait a few minutes until you are brought to the exception point.
@remark: Whenever release an executable, save its symbol table and source code as well. 

[method3] debug with remote debugger(msvcmon.exe)
on the remote computer, run "msvcmon /noauth /anyuser" from command line. 
on the build machine, attach process to the ip address(qualifier) of the remote computer and select transport(with no authentication). 
Then we can run the ParaEngineClient from the remote and debug from build machine. 
*/
int GenerateDump(EXCEPTION_POINTERS* pExceptionPointers)
{
	BOOL bMiniDumpSuccessful;
	char szFileName[MAX_PATH]; 
	string szAppName = "ParaEngine";
	string szVersion = ParaEngineInfo::CParaEngineInfo::GetVersion();
	
	DWORD dwBufferSize = MAX_PATH;
	HANDLE hDumpFile;
	SYSTEMTIME stLocalTime;
	MINIDUMP_EXCEPTION_INFORMATION ExpParam;

	GetLocalTime( &stLocalTime );

	StringCchPrintf( szFileName, MAX_PATH, "%s_%s-%04d%02d%02d-%02d%02d%02d-%ld-%ld.dmp", 
		szAppName.c_str(), szVersion.c_str(), 
		stLocalTime.wYear, stLocalTime.wMonth, stLocalTime.wDay, 
		stLocalTime.wHour, stLocalTime.wMinute, stLocalTime.wSecond, 
		GetCurrentProcessId(), GetCurrentThreadId());
	hDumpFile = CreateFile(szFileName, GENERIC_READ|GENERIC_WRITE, 
		FILE_SHARE_WRITE|FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);

	ExpParam.ThreadId = GetCurrentThreadId();
	ExpParam.ExceptionPointers = pExceptionPointers;
	ExpParam.ClientPointers = TRUE;

	bMiniDumpSuccessful = MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), 
		hDumpFile, MiniDumpWithDataSegs, &ExpParam, NULL, NULL);

	OUTPUT_LOG("Application crash: min dump generated at %s\r\n", szFileName);
	
	return EXCEPTION_EXECUTE_HANDLER;
}

string LookupError( HRESULT errorcode )
{
	HRESULT hrErr = 0;
	const TCHAR* strHRESULT;
	const TCHAR* strDescription;
	TCHAR strHRESULTCopy[MAX_PATH*2];
	int nPower = 0;
	int nPowerDec = 1;
	int nDigit = 0;

	if( hrErr !=0  )
	{
		TCHAR strInternals[100];
		TCHAR strFacility[100];

		// Use dxerr.lib to lookup HRESULT.
		strHRESULT = DXGetErrorString( hrErr );
		StringCchPrintf( strHRESULTCopy, MAX_PATH*2, TEXT("HRESULT: 0x%0.8x (%lu)"), 
			hrErr, hrErr );
		StringCchCat( strHRESULTCopy, MAX_PATH*2, TEXT("\r\nName: ") );
		StringCchCat( strHRESULTCopy, MAX_PATH*2, strHRESULT );

		strDescription = DXGetErrorDescription( hrErr );

		TCHAR* strTemp;
		while( ( strTemp = _tcschr( strHRESULTCopy, '&') ) != '\0' )
		{
			strTemp[0] = '\r';
			strTemp[1] = '\n';
		}

		if( lstrlen(strDescription) > 0 )
		{
			StringCchCat( strHRESULTCopy, MAX_PATH*2, TEXT("\r\nDescription: ") );
			StringCchCat( strHRESULTCopy, MAX_PATH*2, strDescription );
		}

		StringCchPrintf( strInternals, 100, TEXT("\r\nSeverity code: %s"), 
			( HRESULT_SEVERITY(hrErr) == 1 ) ? TEXT("Failed") : TEXT("Success") );
		StringCchCat( strHRESULTCopy, MAX_PATH*2, strInternals );

		int nFacility = HRESULT_FACILITY(hrErr);
		switch( nFacility )
		{
		case FACILITY_WINDOWS_CE:  StringCchCopy( strFacility, 100, TEXT("FACILITY_WINDOWS_CE") ); break;
		case FACILITY_WINDOWS: StringCchCopy( strFacility, 100, TEXT("FACILITY_WINDOWS") ); break;
		case FACILITY_URT: StringCchCopy( strFacility, 100, TEXT("FACILITY_URT") ); break;
		case FACILITY_UMI: StringCchCopy( strFacility, 100, TEXT("FACILITY_UMI") ); break;
		case FACILITY_SXS: StringCchCopy( strFacility, 100, TEXT("FACILITY_SXS") ); break;
		case FACILITY_STORAGE: StringCchCopy( strFacility, 100, TEXT("FACILITY_STORAGE") ); break;
		case FACILITY_STATE_MANAGEMENT: StringCchCopy( strFacility, 100, TEXT("FACILITY_STATE_MANAGEMENT") ); break;
		case FACILITY_SCARD: StringCchCopy( strFacility, 100, TEXT("FACILITY_SCARD") ); break;
		case FACILITY_SETUPAPI: StringCchCopy( strFacility, 100, TEXT("FACILITY_SETUPAPI") ); break;
		case FACILITY_SECURITY: StringCchCopy( strFacility, 100, TEXT("FACILITY_SECURITY or FACILITY_SSPI") ); break;
		case FACILITY_RPC: StringCchCopy( strFacility, 100, TEXT("FACILITY_RPC") ); break;
		case FACILITY_WIN32: StringCchCopy( strFacility, 100, TEXT("FACILITY_WIN32") ); break;
		case FACILITY_CONTROL: StringCchCopy( strFacility, 100, TEXT("FACILITY_CONTROL") ); break;
		case FACILITY_NULL: StringCchCopy( strFacility, 100, TEXT("FACILITY_NULL") ); break;
		case FACILITY_MSMQ: StringCchCopy( strFacility, 100, TEXT("FACILITY_MSMQ") ); break;
		case FACILITY_MEDIASERVER: StringCchCopy( strFacility, 100, TEXT("FACILITY_MEDIASERVER") ); break;
		case FACILITY_INTERNET: StringCchCopy( strFacility, 100, TEXT("FACILITY_INTERNET") ); break;
		case FACILITY_ITF: StringCchCopy( strFacility, 100, TEXT("FACILITY_ITF") ); break;
		case FACILITY_HTTP: StringCchCopy( strFacility, 100, TEXT("FACILITY_HTTP") ); break;
		case FACILITY_DPLAY: StringCchCopy( strFacility, 100, TEXT("FACILITY_DPLAY") ); break;
		case FACILITY_DISPATCH: StringCchCopy( strFacility, 100, TEXT("FACILITY_DISPATCH") ); break;
		case FACILITY_CONFIGURATION: StringCchCopy( strFacility, 100, TEXT("FACILITY_CONFIGURATION") ); break;
		case FACILITY_COMPLUS: StringCchCopy( strFacility, 100, TEXT("FACILITY_COMPLUS") ); break;
		case FACILITY_CERT: StringCchCopy( strFacility, 100, TEXT("FACILITY_CERT") ); break;
		case FACILITY_BACKGROUNDCOPY: StringCchCopy( strFacility, 100, TEXT("FACILITY_BACKGROUNDCOPY") ); break;
		case FACILITY_ACS: StringCchCopy( strFacility, 100, TEXT("FACILITY_ACS") ); break;
		case FACILITY_AAF: StringCchCopy( strFacility, 100, TEXT("FACILITY_AAF") ); break;
		case FACILITY_D3DX: StringCchCopy( strFacility, 100, TEXT("FACILITY_D3DX") ); break;
		case FACILITY_D3D: StringCchCopy( strFacility, 100, TEXT("FACILITY_D3D") ); break;
		default: StringCchCopy( strFacility, 100, TEXT("Unknown") ); break;
		}
		StringCchPrintf( strInternals, 100, TEXT("\r\nFacility Code: %s (%d)"), 
			strFacility, HRESULT_FACILITY(hrErr) );
		StringCchCat( strHRESULTCopy, MAX_PATH*2, strInternals );

		StringCchPrintf( strInternals, 100, TEXT("\r\nError Code: 0x%0.4x (%lu)"), 
			HRESULT_CODE(hrErr), HRESULT_CODE(hrErr) );
		StringCchCat( strHRESULTCopy, MAX_PATH*2, strInternals );

	}

	return strHRESULTCopy;
}


void InterpretError(HRESULT hr,const char* file,int line)
{
	//this error will be written to log if the NOT_INTERPRETERROR is not defined.
#ifdef NOT_INTERPRETERROR
	return;
#endif
	switch(hr) {
	case S_OK:
		break;
	case D3DERR_WRONGTEXTUREFORMAT:
		OUTPUT_LOG("The pixel format of the texture surface is not valid at %s, line %d\n",file,line);
		break;
	case D3DERR_UNSUPPORTEDCOLOROPERATION:
		OUTPUT_LOG("The device does not support a specified texture-blending operation for color values at %s, line %d\n",file,line);
		break;
	case D3DERR_UNSUPPORTEDCOLORARG:
		OUTPUT_LOG("The device does not support a specified texture-blending argument for color values at %s, line %d\n",file,line);
		break;
	case D3DERR_UNSUPPORTEDALPHAOPERATION:
		OUTPUT_LOG("The device does not support a specified texture-blending operation for the alpha channel at %s, line %d\n",file,line);
		break;
	case D3DERR_UNSUPPORTEDALPHAARG:
		OUTPUT_LOG("The application is requesting more texture-filtering operations than the device supports at %s, line %d\n",file,line);
		break;
	case D3DERR_TOOMANYOPERATIONS:
		OUTPUT_LOG("The current texture filters cannot be used together at %s, line %d\n",file,line);
		break;
	case D3DERR_CONFLICTINGTEXTUREFILTER:
		OUTPUT_LOG(" at %s, line %d\n",file,line);
		break;
	case D3DERR_UNSUPPORTEDFACTORVALUE:
		OUTPUT_LOG("The device does not support the specified texture factor value at %s, line %d\n",file,line);
		break;
	case D3DERR_CONFLICTINGRENDERSTATE:
		OUTPUT_LOG("The currently set render states cannot be used together at %s, line %d\n",file,line);
		break;
	case D3DERR_UNSUPPORTEDTEXTUREFILTER:
		OUTPUT_LOG("The device does not support the specified texture filter at %s, line %d\n",file,line);
		break;
	case D3DERR_CONFLICTINGTEXTUREPALETTE:
		OUTPUT_LOG("The current textures cannot be used simultaneously at %s, line %d\n",file,line);
		break;
	case D3DERR_DRIVERINTERNALERROR:
		OUTPUT_LOG("Internal driver error at %s, line %d\n",file,line);
		break;
	case D3DERR_NOTFOUND:
		OUTPUT_LOG("The requested item was not found at %s, line %d\n",file,line);
		break;
	case D3DERR_MOREDATA:
		OUTPUT_LOG("There is more data available than the specified buffer size can hold at %s, line %d\n",file,line);
		break;
	case D3DERR_DEVICELOST:
		OUTPUT_LOG("The device has been lost but cannot be reset at this time at %s, line %d\n",file,line);
		break;
	case D3DERR_DEVICENOTRESET:
		OUTPUT_LOG("The device has been lost but can be reset at this time at %s, line %d\n",file,line);
		break;
	case D3DERR_NOTAVAILABLE:
		OUTPUT_LOG("Device does not support the queried technique at %s, line %d\n",file,line);
		break;
	case D3DERR_OUTOFVIDEOMEMORY:
		OUTPUT_LOG("Not Enough Video memory  at %s, line %d\n",file,line);
		break;
	case D3DERR_INVALIDDEVICE:
		OUTPUT_LOG("The requested device type is not valid at %s, line %d\n",file,line);
		break;
	case D3DERR_INVALIDCALL:
		OUTPUT_LOG("The method call is invalid at %s, line %d\n",file,line);
		break;
	case D3DERR_DRIVERINVALIDCALL:
		OUTPUT_LOG("D3DERR_DRIVERINVALIDCALL at %s, line %d\n",file,line);
		break;
	case D3DERR_WASSTILLDRAWING:
		OUTPUT_LOG("The previous blit operation that is transferring information to or from this surface is incomplete at %s, line %d\n",file,line);
		break;
	case D3DOK_NOAUTOGEN:
		OUTPUT_LOG("Succeeded, but autogeneration of mipmaps is not supported at %s, line %d\n",file,line);
		break;
	case E_UNEXPECTED:
		OUTPUT_LOG("Catastrophic failure Error at %s, line %d\n",file,line);
		break;
	case E_NOTIMPL:
		OUTPUT_LOG("Not implemented Error at %s, line %d\n",file,line);
		break;
	case E_OUTOFMEMORY:
		OUTPUT_LOG("Out of memory Error at %s, line %d\n",file,line);
		break;
	case E_INVALIDARG:
		OUTPUT_LOG("An invalid parameter was passed to the returning function at %s, line %d\n",file,line);
		break;
	case E_NOINTERFACE:
		OUTPUT_LOG("No object interface is available at %s, line %d\n",file,line);
		break;
	case E_POINTER:
		OUTPUT_LOG("Invalid pointer Error at %s, line %d\n",file,line);
		break;
	case E_HANDLE:
		OUTPUT_LOG("Invalid handle Error at %s, line %d\n",file,line);
		break;
	case E_ABORT:
		OUTPUT_LOG("Abort Error at %s, line %d\n",file,line);
		break;
	case E_FAIL:
		OUTPUT_LOG("An undetermined error occurred at %s, line %d\n",file,line);
		break;
	case E_ACCESSDENIED:
		OUTPUT_LOG("Access Denied Error at %s, line %d\n",file,line);
		break;
	case E_PENDING:
		OUTPUT_LOG("Pending Error at %s, line %d\n",file,line);
		break;
	default:
		OUTPUT_LOG("%s\nFile: %s\nLine: %d\n",LookupError(hr).c_str(),file,line);
	}
}

#endif
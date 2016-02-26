/**
* paradebug.h
* Author: Liu Weili, Li Xizhi,
* Date: 2005.11.15
* Description:
* This is the global debug tool for all paraengine files. It provides debug and release warning, error or log outputs.
* It also provides performance counters for developers to test the performance of their code. It may also
* provide some auto or semi-auto debug procedures. All these tools are in the debugtools directory.
* Programmers who wants to use these tools only need to include paradebug.h
*/
#pragma once
#include "util/Log.h"
/** Performance monitor */
// #ifdef _PERFORMANCE_MONITOR
#include "Profiler.h"

#ifdef PARAENGINE_CLIENT
#define OUTPUT_DEFAULT_ERROR(a) InterpretError((a),__FILE__,__LINE__);
#define OUTPUT_INTEGER_ERROR(a) InterpretError((a),__FILE__,__LINE__);

/**
* Output to log file the interpreted error
*/
extern void InterpretError(HRESULT hr, const char * sFile, int nLine);

/** call this function to save mini-dump to the current directory.*/
extern int GenerateDump(EXCEPTION_POINTERS* pExceptionPointers);

/**
* This is some error facility type definition, in case the winerror.h is not up to date
*/
#define FACILITY_WINDOWS_CE              24
#define FACILITY_WINDOWS                 8
#define FACILITY_URT                     19
#define FACILITY_UMI                     22
#define FACILITY_SXS                     23
#define FACILITY_STORAGE                 3
#define FACILITY_STATE_MANAGEMENT        34
#define FACILITY_SSPI                    9
#define FACILITY_SCARD                   16
#define FACILITY_SETUPAPI                15
#define FACILITY_SECURITY                9
#define FACILITY_RPC                     1
#define FACILITY_WIN32                   7
#define FACILITY_CONTROL                 10
#define FACILITY_NULL                    0
#define FACILITY_MSMQ                    14
#define FACILITY_MEDIASERVER             13
#define FACILITY_INTERNET                12
#define FACILITY_ITF                     4
#define FACILITY_HTTP                    25
#define FACILITY_DPLAY                   21
#define FACILITY_DISPATCH                2
#define FACILITY_CONFIGURATION           33
#define FACILITY_COMPLUS                 17
#define FACILITY_CERT                    11
#define FACILITY_BACKGROUNDCOPY          32
#define FACILITY_ACS                     20
#define FACILITY_AAF                     18

#define FACILITY_D3DX                    0x877
#define FACILITY_D3D                     0x876
#endif

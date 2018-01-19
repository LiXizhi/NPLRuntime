#pragma once
//-----------------------------------------------------------------------------
// Class:	Header only utility functions 
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine Co.
// Date:	2010.4.21
// Desc: Some util functions that can be used in plugins without the need to link with the core library. 
//-----------------------------------------------------------------------------
#include "PEtypes.h"

#ifdef WIN32
#include <time.h>
#else
// linux implementation
#include <sys/time.h>
#include <unistd.h>
#endif 

namespace ParaEngine
{
	/**
	* Main class. 
	*/
	class CParaUtils
	{
	public:
		static int64 GetTimeMS()
		{
			return (GetTimeUS()/1000);
		}

		/** get time in nano seconds. this ensures that time is always forward. */
		static int64 GetTimeUS()
		{
			static bool initialized=false;
			static int queryCount=0;
#ifdef WIN32
			// win32 implementation
			static DWORD g_ProcMask;
			static DWORD g_SysMask;
			static HANDLE g_Thread;
			static LARGE_INTEGER g_yo;

			// Win32
			if ( initialized == false)
			{
				initialized = true;
				// Save the current process
				HANDLE mProc = GetCurrentProcess();

				// Get the current Affinity
#if _MSC_VER >= 1400 && defined (_M_X64)
				GetProcessAffinityMask(mProc, (PDWORD_PTR)&g_ProcMask, (PDWORD_PTR)&g_SysMask);
#else
				GetProcessAffinityMask(mProc, &g_ProcMask, &g_SysMask);
#endif
				g_Thread = GetCurrentThread();

				QueryPerformanceFrequency( &g_yo );
			}

			int64 curTime;
			static int64 lastQueryVal=(int64)0;
			//	static unsigned long lastTickCountVal = GetTickCount();

			LARGE_INTEGER PerfVal;

			// Set affinity to the first core
			SetThreadAffinityMask(g_Thread, 1);

			// Docs: On a multiprocessor computer, it should not matter which processor is called.
			// However, g_you can get different results on different processors due to bugs in the basic input/output system (BIOS) or the hardware abstraction layer (HAL). To specify processor affinity for a thread, use the SetThreadAffinityMask function.
			// Query the timer
			QueryPerformanceCounter( &PerfVal );

			// Reset affinity
			SetThreadAffinityMask(g_Thread, g_ProcMask);


			int64 quotient, remainder;
			quotient=((PerfVal.QuadPart) / g_yo.QuadPart);
			remainder=((PerfVal.QuadPart) % g_yo.QuadPart);
			curTime = (int64) quotient*(int64)1000000 + (remainder*(int64)1000000 / g_yo.QuadPart);

			// 08/26/08 - With the below workaround, the time seems to jump forward regardless.
			// Just make sure the time doesn't go backwards
			if (curTime < lastQueryVal)
				return lastQueryVal;
			lastQueryVal=curTime;

			return curTime;
#else
			static timeval tp;
			static int64 initialTime;

			// Linux implementation
			if ( initialized == false)
			{
				gettimeofday( &tp, 0 );
				initialized=true;
				// I do this because otherwise RakNetTime in milliseconds won't work as it will underflow when dividing by 1000 to do the conversion
				initialTime = ( tp.tv_sec ) * (int64) 1000000 + ( tp.tv_usec );
			}

			// GCC
			int64 curTime;
			gettimeofday( &tp, 0 );

			curTime = ( tp.tv_sec ) * (int64) 1000000 + ( tp.tv_usec );
			// Subtract from initialTime so the millisecond conversion does not underflow
			return curTime - initialTime;
#endif
		}
	};
}

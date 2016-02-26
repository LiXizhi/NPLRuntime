//-----------------------------------------------------------------------------
// Class:	Cross platform time related
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine corporation
// Date:	2009.4.18
// Desc:	
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ParaTime.h"
#include "StringHelper.h"
#include <time.h>
#include <boost/date_time/posix_time/posix_time.hpp>
#include <boost/date_time/gregorian/gregorian.hpp>

using namespace ParaEngine;

static bool initialized=false;
static int queryCount=0;

int64 ParaEngine::GetTimeMS( void )
{
	return (GetTimeUS()/1000);
}


#ifdef WIN32
// win32 implementation
static DWORD g_ProcMask;
static DWORD g_SysMask;
static HANDLE g_Thread;
static LARGE_INTEGER g_yo;

int64 ParaEngine::GetTimeUS( void )
{

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
}
#else
// linux implementation

#include <sys/time.h>
#include <unistd.h>

static timeval tp;
static int64 initialTime;

int64 ParaEngine::GetTimeUS( void )
{
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
}

#endif


std::string ParaEngine::GetDateFormat( const char*  sFormat )
{
#ifdef WIN32
	char buffer[256];
	::GetDateFormat(
		LOCALE_USER_DEFAULT, // the locale for which the formatting is being done
		(sFormat==NULL || sFormat[0]=='\0') ? DATE_LONGDATE : 0,       // date format (long, short, ...)
		NULL,                // the date to be formatted (here current system date)
		sFormat,             // style of date format
		buffer,            // output buffer
		255              // size of output buffer
		);
	return std::string(buffer);
#else
	// only do a fixed iso output format
	using namespace boost::gregorian;

	if(sFormat == NULL || sFormat[0] == '\0')
	{
		return to_iso_string(day_clock::local_day());
	}
	else
	{
		char format[256];
		int j = 0;
		for(int i=0; (sFormat[i] != '\0') && j<254; i++)
		{
			char token = sFormat[i];
			char next_token = sFormat[i+1];
			if(token == '%' && next_token != '\0')
			{
				format[j++] = token;
				format[j++] = next_token;
				i++;
			}
			else if(token == 'y' && next_token =='y' && sFormat[i+2] == 'y' && sFormat[i+3] == 'y')
			{
				format[j++] = '%';
				format[j++] = 'Y';
				i+=3;
			}
			else if(token == 'y' && next_token =='y')
			{
				format[j++] = '%';
				format[j++] = 'y';
				i++;
			}
			else if(token == 'M' && next_token =='M')
			{
				format[j++] = '%';
				format[j++] = 'm';
				i++;
			}
			else if(token == 'M')
			{
				format[j++] = '%';
				format[j++] = 'm';
			}
			else if(token == 'd' && next_token =='d')
			{
				format[j++] = '%';
				format[j++] = 'd';
				i++;
			}
			else if(token == 'd')
			{
				format[j++] = '%';
				format[j++] = 'd';
			}
			else
			{
				format[j++] = token;
			}
		}
		format[j] = '\0';
		stringstream ss; 
		date_facet facet(format);
		facet.put(ss, ss, ' ', day_clock::local_day()); 
		return ss.str();
	}
#endif
}

std::string ParaEngine::GetTimeFormat( const char*  sFormat )
{
#ifdef WIN32
	char buffer[256];
	// Formats time as a time string for a specified locale.
	::GetTimeFormat(LOCALE_USER_DEFAULT,  // predefined current user locale
		0,          // option flag for things like no usage of seconds or
		NULL,       // time - NULL to go with the current system locale time
		sFormat,       // time format string - NULL to go with default locale
		buffer,  // formatted string buffer
		255);   // size of string buffer
	return std::string(buffer);
#else
	// only do a fixed iso output format
	using namespace boost::posix_time;
	using namespace boost::gregorian;
	using namespace std;

	//get the current time from the clock -- one second resolution
	ptime now = second_clock::local_time();
	if(sFormat == NULL || sFormat[0] == '\0')
	{
		ptime today_start(now.date()); // today start time
		time_duration elapsedTime = now - today_start;
		return to_simple_string(elapsedTime);
	}
	else
	{
		char format[256];
		int j = 0;
		for(int i=0; (sFormat[i] != '\0') && j<254; i++)
		{
			char token = sFormat[i];
			char next_token = sFormat[i+1];
			if(token == '%' && next_token != '\0')
			{
				format[j++] = token;
				format[j++] = next_token;
				i++;
			}
			else if(token == 'H' && next_token =='H')
			{
				format[j++] = '%';
				format[j++] = 'H';
				i++;
			}
			else if(token == 'H')
			{
				format[j++] = '%';
				format[j++] = 'H';
			}
			else if(token == 'm' && next_token =='m')
			{
				format[j++] = '%';
				format[j++] = 'M';
				i++;
			}
			else if(token == 'm')
			{
				format[j++] = '%';
				format[j++] = 'M';
			}
			else if(token == 's' && next_token =='s')
			{
				format[j++] = '%';
				format[j++] = 'S';
				i++;
			}
			else if(token == 's')
			{
				format[j++] = '%';
				format[j++] = 'S';
			}
			else
			{
				format[j++] = token;
			}
		}
		format[j] = '\0';
		stringstream ss; 
		time_facet facet(format);
		facet.put(ss, ss, ' ', now); 
		return ss.str();
	}
#endif
}


std::string ParaEngine::GenerateUniqueIDByTime()
{
	using namespace boost::posix_time; 
	ptime now(microsec_clock::universal_time());
	std::string timePart = to_iso_string(now);

	static int g_counter = 0;
	char str[32];
	ParaEngine::StringHelper::fast_itoa(g_counter++, str, 32);
	timePart.append("-");
	timePart.append(str);
	return timePart;
}

double ParaEngine::GetSysDateTime()
{
	using namespace boost::posix_time; 
	using namespace boost::gregorian;
	double dateTime = 0;
	ptime time_t_epoch(date(2008,1,1)); 
	ptime now(microsec_clock::universal_time());
	// first convert nyc_time to utc via the utc_time() 
	// call and subtract the ptime.
	time_duration diff = now - time_t_epoch;

	dateTime = diff.total_seconds();
	return dateTime;
}

void ParaEngine::CParaTimeInterval::Print(const char* sMsg)
{
	OUTPUT_LOG("CParaTimeInterval: %d us for %s\n", (int)GetIntervalUS(), sMsg);
}

int32 ParaEngine::CParaTimeInterval::GetIntervalUS()
{
	return (int32)(GetTimeUS() - m_nFromTime);
}

ParaEngine::CParaTimeInterval::CParaTimeInterval(bool bStartNow/*=true*/)
{
	if (bStartNow)
		Begin();
	else
		m_nFromTime = 0;
}

void ParaEngine::CParaTimeInterval::Begin()
{
	m_nFromTime = GetTimeUS();
}

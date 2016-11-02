//----------------------------------------------------------------------
// Class:	Cross Platform functions
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Date:	2009.4.9
//-----------------------------------------------------------------------
#include "ParaEngine.h"

#ifdef WIN32
#else
#include <sys/times.h> 
#include <sys/time.h> 
#endif

#include <time.h>

#define FAST_RAND_MAX 0x7fff

namespace ParaEngine
{
	unsigned long g_holdrand = (unsigned long)time( NULL );

	// returns a random number between 0,RAND_MAX
	int my_rand (void)
	{
		return( ((g_holdrand = g_holdrand * 214013L
			+ 2531011L) >> 16) & FAST_RAND_MAX);
	}

	float frand()
	{
		return my_rand() / (float)FAST_RAND_MAX;
	}

	float randfloat(float lower, float upper)
	{
		return lower + (upper - lower)*(my_rand() / (float)FAST_RAND_MAX);
	}

	int randint(int lower, int upper)
	{
		if(upper>lower)
		{
			int v = lower + (rand()%(upper+1-lower));
			return (v<upper) ? v : upper;
		}
		else
		{
			return upper;
		}
	}
}


/** native implementation of strnlen */
int strnlen(const char *str, int maxsize)
{
	int n;

	/* Note that we do not check if s == NULL, because we do not
	* return errno_t...
	*/
	for (n = 0; n < maxsize && *str; n++, str++)
		;
	return n;
}

#ifndef WIN32
static bool g_timer_initialized=false;
static int64 g_initialTime;

#ifndef CLOCK_MONOTONIC
#define CLOCK_MONOTONIC 0
#endif


#if (PARA_TARGET_PLATFORM == PARA_PLATFORM_IOS || PARA_TARGET_PLATFORM == PARA_PLATFORM_MAC)
#include <mach/mach_time.h>
#define ORWL_NANO (+1.0E-9)
#define ORWL_GIGA UINT64_C(1000000000)

static double orwl_timebase = 0.0;
static uint64_t orwl_timestart = 0;

int clock_gettime(int nUnUsedType, timespec * ts) {
    // be more careful in a multithreaded environement
    if (!orwl_timestart) {
        mach_timebase_info_data_t tb = { 0 };
        mach_timebase_info(&tb);
        orwl_timebase = tb.numer;
        orwl_timebase /= tb.denom;
        orwl_timestart = mach_absolute_time();
    }
    double diff = (mach_absolute_time() - orwl_timestart) * orwl_timebase;
    ts->tv_sec = diff * ORWL_NANO;
    ts->tv_nsec = diff - (ts->tv_sec * ORWL_GIGA);
    return 0;
}
#endif


// 
/** Linux implementation of GetTickCount: it will return the milliseconds since first call. 
*/
unsigned int GetTickCount(void)
{
	timespec tsnow;
	/*
	** clock_gettime() is granted to be increased monotonically when the
	** monotonic clock is queried. Time starting point is unspecified, it
	** could be the system start-up time, the Epoch, or something else,
	** in any case the time starting point does not change once that the
	** system has started up.
	*/
	if(0 == clock_gettime(CLOCK_MONOTONIC, &tsnow))
	{
		int64 curTime =	((int64)(tsnow.tv_sec) * (int64)1000L + (tsnow.tv_nsec / 1000000L));

		if ( g_timer_initialized == false)
		{
			g_timer_initialized = true;
			g_initialTime = curTime;
		}
		return  (unsigned int)(curTime - g_initialTime);
	}
	/*
	** Even when the configure process has truly detected monotonic clock
	** availability, it might happen that it is not actually available at
	** run-time. When this occurs simply fallback to other time source.
	*/
	else
	{
		timeval ts;
		gettimeofday(&ts,0);
		int64 curTime =	((int64)(ts.tv_sec) * (int64)1000L + (ts.tv_usec / 1000));
		if ( g_timer_initialized == false)
		{
			g_timer_initialized = true;
			g_initialTime = curTime;
		}
		return  (unsigned int)(curTime - g_initialTime);
	}
}

// porting strrev function to linux
char* strrev(char* szT)
{
	if ( !szT )                 // handle null passed strings.
		return NULL;
	int i = strlen(szT);
	int t = !(i%2)? 1 : 0;      // check the length of the string .
	for(int j = i-1 , k = 0 ; j > (i/2 -t) ; j-- )
	{
		char ch  = szT[j];
		szT[j]   = szT[k];
		szT[k++] = ch;
	}
	return szT;
}

// porting itoa function to linux
char* itoa(int value, char*  str, int radix)
{
	int  rem = 0;
	int  pos = 0;
	char ch  = '!' ;
	do
	{
		rem    = value % radix ;
		value /= radix;
		if ( 16 == radix )
		{
			if( rem >= 10 && rem <= 15 )
			{
				switch( rem )
				{
				case 10:
					ch = 'a' ;
					break;
				case 11:
					ch ='b' ;
					break;
				case 12:
					ch = 'c' ;
					break;
				case 13:
					ch ='d' ;
					break;
				case 14:
					ch = 'e' ;
					break;
				case 15:
					ch ='f' ;
					break;
				}
			}
		}
		if( '!' == ch )
		{
			str[pos++] = (char) ( rem + 0x30 );
		}
		else
		{
			str[pos++] = ch ;
		}
	}while( value != 0 );
	str[pos] = '\0' ;
	return strrev(str);
}

char *_gcvt( double value,	int digits,	char *buffer )
{
	sprintf(buffer, "%f", (float)value);
	return buffer;
}
wchar_t * _itow(int value,	wchar_t *str,	int radix)
{
	PE_ASSERT(radix == 10);
	// assume buffer is smaller than 100. This is not full port. 
	swprintf(str, 100, L"%d", value);
	return str;
}

BOOL PtInRect(const RECT *pos, const POINT &pt)
{
	return ( (pos->left<=pt.x && pos->top<=pt.y && pos->right>pt.x && pos->bottom>pt.y));
}

BOOL SetRect(RECT* lprc, int xLeft, int yTop, int xRight, int yBottom)
{
	if(lprc)
	{
		lprc->left = xLeft;
		lprc->top = yTop;
		lprc->right = xRight;
		lprc->bottom = yBottom;
	}
	return TRUE;
}

#endif

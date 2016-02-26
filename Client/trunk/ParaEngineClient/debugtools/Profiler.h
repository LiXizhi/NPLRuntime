#pragma once
#include <string>
/** A C++ scope based profiler. It also support name based profiler.
Scope based profiler is preferred, since its start()/stop() functions are guarenteed to be paired.
Usage:
{
PERF()
...// your tested code here
}
*/
class CProfiler
{
public:
	/** @param name: it must be valid under the profiler object is destructed. */
	CProfiler(const char* name);
	~CProfiler(void);
public:
	/** start the time measurement with m_name */
	void Start();
	/** stops the time measurement and add the result to the list of measurements */
	void Stop();
private:
	const char* m_name;
	static bool m_bEnableProfiling;
public:
	/* providing name based profiler */
	
	static void Start_S(const char* name);
	static void Stop_S(const char* name);
	static void Clear_S(const char* name);
	static void ReportAll_S();
	static void ResetRange_S(const char* name, int nFrom, int nMaxCount);
	/** start/stop all profilers. anything using Start_S does not take effect. */
	static void EnableProfiling_S(bool bEnable);
	static bool IsProfilingEnabled_S();
	
};

/** Performance monitor
*/
#ifdef _PERFORMANCE_MONITOR
#	define PERF() CProfiler profiler__(__FUNCTION__);
#	define PERF1(x) CProfiler profiler__(x);
#	define PERF_BEGIN(x) CProfiler::Start_S(x);
#	define PERF_RANGE(name, nFrom, nMaxCount) CProfiler::ResetRange_S(name, nFrom, nMaxCount);
#	define PERF_END(x) CProfiler::Stop_S(x);
#	define PERF_CLEAR(x) CProfiler::Clear_S((x));
#	define PERF_REPORT() CProfiler::ReportAll_S();
#else
#	define PERF()
#	define PERF1(x)
#	define PERF_BEGIN(x)
#	define PERF_END(x) 
#	define PERF_CLEAR(x)
#	define PERF_REPORT()
#endif

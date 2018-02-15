//-----------------------------------------------------------------------------
// Class: CProfiler
// Authors:	LiXizhi, LiuWeili
// Emails:	
// Company: ParaEngine
// Date:2006.2.16
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "util/ParaTime.h"
#include "Profiler.h"

#include <map>

bool CProfiler::m_bEnableProfiling = false;

struct _performance_item{
	int64 finish;
	int64 start;
};
struct _performance_struct {
	vector<_performance_item> items;
	string m_name;
	bool m_counting;

	int64 m_min;
	int64 m_max;
	int64 m_avg;
	/** this is not the standard deviation. but calculated as follows:
	deviation(n) = [(n-1)*deviation(n-1)+|a(n)-avg(n)|]/n
	*/
	int64 m_deviation;
	/// start time of the first frame
	int64 m_nFirstTime;
	/// start time of the last frame
	int64 m_nLastTime;
	/// a frame counter, the first one is frame 1
	int m_nFrameCounter;

	/** from which to count. 
	if this is -1, it will keep the most recent m_nMaxItemSize.and that min, max, avg are for all items since frame 0.
	if it is positive, it means the frame number from which to start recording for the next m_nMaxItemSize frames. */
	int m_nFrom;
	/** max item number. default is 0. Set this number to a negative number in order to report per item data.
	otherwise, only statistical data can be reported. */
	int m_nMaxItemSize;
	/// current item being timed
	_performance_item m_curItem;
	void init(const char *name,int nFrom=-1, int nMaxCount=0){
		m_name=name;
		m_counting=false;
		m_nFrom = nFrom;
		m_nFrameCounter = 0;
		m_nMaxItemSize = nMaxCount;
		if(nMaxCount>0)
			items.reserve(nMaxCount);
		m_avg = 0;
		m_max=-1000000000000;
		m_min=1000000000000;
		m_deviation = 0;
	}
public:
	void AddCurrentItem()
	{
		// calculate statistical data
		int64 An = (m_curItem.finish-m_curItem.start);
		if (m_min>An)
			m_min=An;
		if (m_max<An)
			m_max=An;
		assert(m_nFrameCounter>0);
		m_avg = ((m_nFrameCounter-1)*m_avg + An)/m_nFrameCounter;
		m_deviation = ((m_nFrameCounter-1)*m_deviation+abs((long)(An-m_avg)))/m_nFrameCounter;

#ifdef KEEP_TIME_RANGE
		// add time interval, this gives absolute time. Better use Game Time in mm:ss
		if(m_nFrameCounter==1)
			m_nFirstTime = m_curItem.start;
		m_nLastTime = m_curItem.start;
#endif
	
		// add current item to item list. 
		if(m_nMaxItemSize>0 && m_nMaxItemSize>(int)items.size())
		{
			if(m_nFrom<0){
				// TODO: keep most recent m_nMaxItemSize items
			}
			else if(m_nFrameCounter>=m_nFrom){
				// keep m_nMaxItemSize items from the frame m_nFrom
				items.push_back(m_curItem);
			}
		}
	}
};
static map<string,_performance_struct> _performance_data;

/************************************************************************/
/* CProfiler                                                            */
/************************************************************************/
CProfiler::CProfiler(const char* name)
	: m_name(name)
{
	Start();
}

CProfiler::~CProfiler(void)
{
	Stop();
}

void CProfiler::Start()
{
	Start_S(m_name);
}
void CProfiler::Stop()
{
	Stop_S(m_name);
}
void CProfiler::ResetRange_S(const char* name, int nFrom, int nMaxCount)
{
	if (_performance_data.find(name)==_performance_data.end()) {
		_performance_data[name].init(name, nFrom, nMaxCount);
	}
	else
	{
		_performance_struct *pProfile=&_performance_data[name];
		if(pProfile->m_nFrom != nFrom)
		{
			pProfile->m_nFrom = nFrom;
			pProfile->items.clear();
			if(nMaxCount>0)
				pProfile->items.reserve(nMaxCount);
		}
		pProfile->m_nMaxItemSize = nMaxCount;
	}
}

// TODO: to avoid creating string,  we should use const char* (or its fast hash value) as key instead of string.
void CProfiler::Start_S(const char* name)
{
	if(m_bEnableProfiling)
	{
		if (_performance_data.find(name)==_performance_data.end()) {
			_performance_data[name].init(name);
		}
		_performance_struct *pProfile=&_performance_data[name];
		if (pProfile->m_counting==true) {
			OUTPUT_LOG("error: unpaired profiler%s\n", pProfile->m_name.c_str());
		}

		pProfile->m_counting=true;

		// advance counter
		++(pProfile->m_nFrameCounter);

		pProfile->m_curItem.start = ParaEngine::GetTimeUS();
	}
}

void CProfiler::Stop_S(const char* name)
{
	if(m_bEnableProfiling)
	{
		int64 currentTime;
		currentTime = ParaEngine::GetTimeUS();
		
		if (_performance_data.find(name)==_performance_data.end()) {
			return; // ID not found
		}
		_performance_struct *pProfile=&_performance_data[name];
		if (pProfile->m_counting==false) {
			return; // multiple stop()
		}

		pProfile->m_curItem.finish = currentTime;
		pProfile->m_counting=false;

		pProfile->AddCurrentItem();
	}
}

void CProfiler::Clear_S(const char* name)
{
	if (_performance_data.find(name)==_performance_data.end()) return;
	_performance_data.erase(name);
}

void CProfiler::ReportAll_S()
{
	if (_performance_data.empty())
		return;
	FILE *file;
	if (NULL==(file=fopen("perf.txt","w+"))) {
		return;
	}
	map<string,_performance_struct>::iterator iter=_performance_data.begin();
	vector<_performance_item>::iterator iteritem;
	_performance_struct *pProfile=NULL;
	int64 fre;
	fre = ParaEngine::GetTimeUS();
	for (;iter!=_performance_data.end();iter++) {
		pProfile=&(*iter).second;
		if (pProfile->m_counting) {
			Stop_S(pProfile->m_name.c_str());
		}
#define FRE_TO_TIME(x) (((double)(x))/fre)
		fprintf(file,"\n<%s>",pProfile->m_name.c_str());
		fprintf(file,"\nAvg: %.9f    Dev: %.9f    Max: %.9f    Min: %.9f    Total Frames: %d\n",
			FRE_TO_TIME(pProfile->m_avg),FRE_TO_TIME(pProfile->m_deviation),FRE_TO_TIME(pProfile->m_max),FRE_TO_TIME(pProfile->m_min),pProfile->m_nFrameCounter);
		int nItemSize = (int)pProfile->items.size();
		if(nItemSize>0)
		{
			fprintf(file,"\nFrame data at [%d, %d]\n",pProfile->m_nFrom,pProfile->m_nFrom+nItemSize);
			for (int i=0;i<nItemSize;++i)
			{
				const _performance_item & item = pProfile->items[i];
				fprintf(file,"%d %f\n",i+1,FRE_TO_TIME(item.finish-item.start));
			}
		}
	}
	fclose(file);
}


void CProfiler::EnableProfiling_S( bool bEnable )
{
	m_bEnableProfiling = bEnable;
}

bool CProfiler::IsProfilingEnabled_S()
{
	return m_bEnableProfiling;
}

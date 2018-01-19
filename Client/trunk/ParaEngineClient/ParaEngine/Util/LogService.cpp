//-----------------------------------------------------------------------------
// Class:	CLogService
// Authors:	LiXizhi
// Company: ParaEngine Corporation
// Date:	Provides formated service log
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "util/mutex.h"
#include "util/ParaTime.h"
#include "LogService.h"
#include "util/os_calls.h"

/** the etc length when too many characters are printed. This is larger then strlen("...\n")*/
#define LOG_TAIL_ETC_LENGTH		5

ParaEngine::CServiceLogger::CServiceLogger(const char* filename, bool bAppendMode)
{
	if (filename)
		SetLogFile(filename);

	// service log is appended by default, so we will set m_is_first_time_open to false. 
	m_is_first_time_open = !bAppendMode;
}

ParaEngine::CServiceLogger::~CServiceLogger()
{
}

void ParaEngine::CServiceLogger::WriteServiceFormated(const char * zFormat, ...)
{
	// TODO: LXZ, shall we cache the date string, since it does not change often?
	// TODO LXZ: does STL in linux has SSO(small string optimization) like in windows? i.e. first 16 bytes on stack, instead of on heap?
	std::string date_str = ParaEngine::GetDateFormat("yyyy-MM-dd");
	std::string time_str = ParaEngine::GetTimeFormat(NULL);

	ParaEngine::Lock lock_(m_mutex);
	int nSize = snprintf(m_buffer, MAX_DEBUG_STRING_LENGTH, "%s %s|%d|", date_str.c_str(), time_str.c_str(), ParaEngine::GetThisThreadID());

	va_list args;
	va_start(args, zFormat);

	int nSize2 = vsnprintf(m_buffer + nSize, MAX_DEBUG_STRING_LENGTH - LOG_TAIL_ETC_LENGTH - nSize, zFormat, args);
	if (nSize2 < 0 || nSize2 >= (MAX_DEBUG_STRING_LENGTH - LOG_TAIL_ETC_LENGTH - 1 - nSize))
	{
		nSize2 = (MAX_DEBUG_STRING_LENGTH - LOG_TAIL_ETC_LENGTH - 1 - nSize);
		//nSize2 += snprintf(m_buffer+nSize+nSize2, MAX_DEBUG_STRING_LENGTH, "==>log message ignored, because it is longer than %s\n", MAX_DEBUG_STRING_LENGTH);
		nSize2 += snprintf(m_buffer + MAX_DEBUG_STRING_LENGTH - LOG_TAIL_ETC_LENGTH - 1, LOG_TAIL_ETC_LENGTH, "...\n");

	}
	va_end(args);
	Write_st(m_buffer, nSize + nSize2);
}

ParaEngine::CServiceLogger_ptr ParaEngine::CServiceLogger::GetLogger(const std::string& name)
{
	return CLogService::GetSingleton().GetLogger(name);
}

ParaEngine::CServiceLogger_ptr ParaEngine::CServiceLogger::GetLogger(const char* const name)
{
	return CLogService::GetSingleton().GetLogger(name);
}


ParaEngine::CLogService& ParaEngine::CLogService::GetSingleton()
{
	static CLogService g_singleton;
	return g_singleton;
}

ParaEngine::CServiceLogger_ptr ParaEngine::CLogService::GetLogger(const std::string& name)
{
	ParaEngine::Lock lock_(m_mutex);
	LoggerMap_Type::iterator iter = m_logger_map.find(name);
	if (iter != m_logger_map.end())
	{
		return iter->second;
	}
	else
	{
		CServiceLogger_ptr logger(new CServiceLogger());
		m_logger_map[name] = logger;

		// the file name is like "name_20090709.log"
		std::string filename = name;
		filename += "_";
		filename += ParaEngine::GetDateFormat("yyyyMMdd");
		filename += ".log";
		logger->SetLogFile(filename);
		return logger;
	}
}

ParaEngine::CServiceLogger_ptr ParaEngine::CLogService::GetLogger(const char* const name)
{
	std::string strName = (name != 0) ? name : "";
	return GetLogger(strName);
}


void Test_ServiceLog()
{
	ParaEngine::CServiceLogger_ptr logger = ParaEngine::CServiceLogger::GetLogger("ServiceLogger");
	logger->SetLevel(2);

	SERVICE_LOG(logger, 2, "Test logging1");
	SERVICE_LOG(logger, 3, "Test logging2");
	SERVICE_LOG(logger, 0, "This should not be logged, since log level is too small");
}
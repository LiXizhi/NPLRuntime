#pragma once
#include <string>
#include <map>
#include "Log.h"
#include "util/intrusive_ptr.h"
#include "util/mutex.h"

#ifdef _MSC_VER
#pragma warning( push )
#pragma warning( disable:4275 )
#pragma warning( disable:4251 )
#endif

namespace ParaEngine
{
	class CServiceLogger;
	
	/** CServiceLogger is a more advanced multi-threaded logger than the simple CLogger
	* it supports log level, formatting, log time output, etc. 
	* Please use CLogService to create an instance of CServiceLogger
	* @note: All server daemon can GetLogger on startup (only once), keep the CServiceLogger_ptr and use it for logging thereafterwards. 
	* Example:
		// keep the g_router_logger, and use it for logging, do not call GetLogger again.
		CServiceLogger_ptr g_router_logger = CLogService::GetLogger("router");
		// create an unnamed logger. 
		CServiceLogger_ptr logger(new CServiceLogger("mylog.txt", false));
		
		SERVICE_LOG(g_router_logger, 0, "some message1 here");
		SERVICE_LOG(g_router_logger, 0, "some message2 here");

		// Another note: service log do not provide formatted string like application log. 
		// Such design is deliberate because snprintf is a slow function and should be used sparingly on server side. 
		// If you do need to use formated string, use

		SERVICE_LOG1(g_router_logger, "Hello");
		SERVICE_LOG1(g_router_logger, "Hello %s", "ParaEngine");
	*/
	class PE_CORE_DECL CServiceLogger : public CLogger, public ParaEngine::intrusive_ptr_thread_safe_base
	{
	public:
		/**
		* @param filename: the file name, such as "mylog.txt"
		* @param bAppendMode: if true, we will append log to the existing one.
		*/
		CServiceLogger(const char* filename=NULL, bool bAppendMode=true);
		virtual ~CServiceLogger();

		typedef ParaIntrusivePtr <CServiceLogger> CServiceLogger_ptr_type;
	public:

		/**
		* Retrieve a logger by name in current encoding.
		* @param name logger name. 
		*/
		static CServiceLogger_ptr_type GetLogger(const std::string& name);
		/**
		* Retrieve a logger by name in current encoding.
		* @param name logger name. 
		*/
		static CServiceLogger_ptr_type GetLogger(const char* const name);

		/** it will write an ordinary formated message to service log with current date time prepended. 
		* Please note internally it uses snprintf. However, snprintf is a slow function (which needs to retrieve the current system locale) and should be used sparingly on server side. 
		*/
		void WriteServiceFormated(const char *, ...);
	};
	typedef ParaIntrusivePtr <CServiceLogger> CServiceLogger_ptr;
	/**
	Log rules

	There are 2 types of log:
	a.	runtime log:
	Programming info. debug info will be written here.
	b.	service log:
	Formatted date info. for stat, monitor and other things.

	I£®open mode
	To open a log file,we need use ¡°a+¡± mode:
	fopen(const char * filename, ¡°a+¡±)
	Open for reading and writing.  The file is created if it does not exist.
	So we need not to worry about log missing.

	II. log name
	File name rule
	a.	runtime log,use program name for first part of log name,like£º
	para_engine_server.log
	b.	service log. Use service name + _date + .log,like:
	UserLogin_20090709.log

	III fields content
	Every log need a time formate on the first field of line.
	2009-07-09 16:40:05

	Separator : |

	Content:
	a.	runtime log:
	cat para_engine_server.log
	2009-07-09 16:40:05|the id value=5
	2009-07-09 16:40:05|app "EditApps" loaded
	2009-07-09 16:40:05|counter=10000

	b.	service log,log format like:
	cat user_click_20090709.log
	2009-07-09 09:34:23|13307||||70|2|1|61.144.207.30
	2009-07-09 09:42:52|13307||||74|2|1|61.144.207.30
	2009-07-09 09:45:31|13307||||44|2|1|61.144.207.30
	2009-07-09 09:45:31|13307||||44|2|1|61.144.207.30

	IV. others
	If we need write same service log from several threads or processes . we do not lock the file. Just writing. The analyse process will deal with it if the data is confused.
	*/
	class PE_CORE_DECL CLogService
	{
	public:
		CLogService(){};

		/** get an instance of this class */
		static CLogService& GetSingleton();

		/**
		* Retrieve a logger by name. It will create one if not exist
		* @param name logger name. 
		*/
		CServiceLogger_ptr GetLogger(const std::string& name);
		/**
		* Retrieve a logger by name. It will create one if not exist
		* @param name logger name. 
		*/
		CServiceLogger_ptr GetLogger(const char* const name);

	private:
		typedef std::map<std::string, CServiceLogger_ptr>	LoggerMap_Type;

		// all loggers
		LoggerMap_Type m_logger_map;

		// mutex
		ParaEngine::mutex m_mutex;
	};
}

// required by DLL interface
// EXPIMP_TEMPLATE template class PE_CORE_DECL ParaIntrusivePtr<ParaEngine::CServiceLogger>;

/**
Logs a service message to a specified logger with a specified level.

@param logger the logger to be used.
@param level the level to log.
@param message the message string to log.
*/
#define SERVICE_LOG(logger, level, message) { \
	if (logger->IsEnabledFor(level)) {\
	logger->ForcedLog(level, message, SERVICE_LOG_LOCATION); } }

/** similar to SERVICE_LOG, but it takes only 1 parameter, that is the log message. And it also support formated strings. 
* @note: It does not output the file position where the log is called. 
*/
#define SERVICE_LOG1(logger, message, ...) \
	logger->WriteServiceFormated(message, ## __VA_ARGS__);

#ifdef _MSC_VER
#pragma warning( pop ) 
#endif
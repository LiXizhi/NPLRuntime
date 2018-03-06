#pragma once
#include "util/mutex.h"
#include <string>

#define MAX_DEBUG_STRING_LENGTH 1024

namespace ParaEngine
{
	class CLogger;

	namespace LogDetail
	{
		/**
		* This class represents the location of a logging statement.
		*
		*/
		class PE_CORE_DECL LocationInfo
		{
		public:

			/**
			*   When location information is not available the constant
			* <code>NA</code> is returned. Current value of this string constant is <b>?</b>.
			*/
			static const char * const NA;
			static const char * const NA_METHOD;

			static const LocationInfo& getLocationUnavailable();


			/**
			*   Constructor.
			*   @remarks Used by SERVICE_LOG_LOCATION to generate
			*       location info for current code site
			*/
			LocationInfo( const char * const fileName,
				const char * const functionName,
				int lineNumber);

			/**
			*   Default constructor.
			*/
			LocationInfo();

			/**
			*   Copy constructor.
			*   @param src source location
			*/
			LocationInfo( const LocationInfo & src );

			/**
			*  Assignment operator.
			* @param src source location
			*/
			LocationInfo & operator = ( const LocationInfo & src );

			/**
			*   Resets location info to default state.
			*/
			void clear();


			/** Return the class name of the call site. */
			const std::string getClassName();

			/**
			*   Return the file name of the caller.
			*   @returns file name, may be null.
			*/
			const char * getFileName();

			/**
			*   Returns the line number of the caller.
			* @returns line number, -1 if not available.
			*/
			int getLineNumber();

			/** Returns the method name of the caller. */
			const std::string getMethodName();

			/** write to log file. */
			void write(CLogger* pOutput) const;

		private:
			/** Caller's line number. */
			int lineNumber;

			/** Caller's file name. */
			const char * fileName;

			/** Caller's method name. */
			const char * methodName;
		};
	}

#ifdef WIN32
#pragma warning( push )
// warning C4251: 'ParaEngine::CLogger::m_log_file_name' : class 'std::basic_string<_Elem,_Traits,_Ax>' needs to have dll-interface to be used by clients of class 'ParaEngine::CLogger'
#pragma warning( disable : 4251 ) 
#endif

	/** a logger can only write to a given file. Please use LogService to create different loggers. 
	* char and wchar_t are supported. Both multi-threaded and single-threaded logging functions are supported. 
	* Internally we use a mutex to sync write. 
	* Messages are written immediately to file; it uses the system IO cache. 
	*/
	class PE_CORE_DECL CLogger
	{
	public:
		CLogger();
		~CLogger();

		/** get the default application logger. */
		static CLogger& GetSingleton();

		/** 
		* [thread safe]
		*/
		void AddLogStr(const char * pStr);
		void AddLogStr(const wchar_t * pStr);

		/** single threaded version */
		void AddLogStr_st(const char * pStr);
		void AddLogStr_st(const wchar_t * pStr);

		/** change the log file.*/
		void SetLogFile(const std::string& sFile);
		const std::string& GetLogFile();

		/** output to log file a formatted string 
		* [thread safe]
		*/
		void WriteFormated(const char *, ...);
		void WriteFormatedVarList(const char *, va_list args);
		void WriteFormated(const wchar_t *, ...);
		void WriteFormated_WithTime(const char *, ...);

		/** single threaded version */
		void WriteFormated_st(const char *, ...);
		void WriteFormated_st(const wchar_t *, ...);

		/** write byte array
		* [thread safe]
		@param buf: buffer pointer
		@param nLength: buffer length in bytes. 
		@return: bytes return. or -1 if failed. 
		*/
		int Write(const char * buf, int nLength);

		/** single threaded version */
		int Write_st(const char * buf, int nLength);

		/** get the current log file position. it is equivalent to the log file size in bytes. 
		one can later get log text between two Log positions. 
		*/
		int GetPos();

		/**get log text between two Log positions. 
		* @param fromPos: position in bytes. if nil, it defaults to 0
		* @param nCount: count in bytes. if nil, it defaults to end of log file. 
		* @return string returned. 
		*/
		const char* GetLog(int fromPos, int nCount);

		/** close the log file handle */
		void CloseLog();

		/** get the log file handle, and seek to end of file for immediate writing. */
		FILE* GetLogFileHandle();
	
		/** output a string to log file with a given file name and line number in a given format,when compiled in debug mode
		* e.g. WriteDebugStr("%s failed in %s() [%d]\n","ErrorXXX", __FILE__,__LINE__);
		* @param str: should be a formated string.
		*/
		void WriteDebugStr(const char * zFormat,const char * str, const char* sFile,int nLine);

	public:
		/**
		* Get the logger name.
		*/
		const std::string& GetName();


		/** by default, append mode is enabled in server mode, and disabled in client build.*/
		void SetAppendMode(bool bAppendToExistingFile);

		/** if true we will flush the new log to file immediately. otherwise, flush operation is determined by the system. 
		* default to true for client log and false for service log. 
		*/
		void SetForceFlush(bool bForceFlush);

		/**
		Returns the assigned Level
		@return Level - the assigned Level
		*/
		int GetLevel();

		/** set level of this logger. */
		void SetLevel(const int level1);

		/**
		Check whether this logger is enabled for a given Level passed as parameter.
		@return bool True if this logger is enabled for level. It just checks (level>=this->m_level)
		*/
		bool IsEnabledFor(int level);

		/**
		This is the most generic printing method. It is intended to be
		invoked by <b>wrapper</b> classes.

		@param level The level of the logging request.
		@param message The message of the logging request.
		@param location The source file of the logging request, may be null. */
		void log(const int level, const std::string& message,
			const ParaEngine::LogDetail::LocationInfo& location);
		void log(const int level, const char* message,
			const ParaEngine::LogDetail::LocationInfo& location);
		/**
		This is the most generic printing method. It is intended to be
		invoked by <b>wrapper</b> classes.

		@param level The level of the logging request.
		@param message The message of the logging request.
		*/
		void log(const int level, const std::string& message);
		void log(const int level, const char* message);

		/**
		This method creates a new logging event and logs the event
		without further checks.
		@param level the level to log.
		@param message message.
		@param location location of source of logging request.
		*/
		void ForcedLog(const int level, const std::string& message,
			const ParaEngine::LogDetail::LocationInfo& location);
		void ForcedLog(const int level, const char* message,
			const ParaEngine::LogDetail::LocationInfo& location);
		/**
		This method creates a new logging event and logs the event
		without further checks.
		@param level the level to log.
		@param message message.
		*/
		void ForcedLog(const int level, const std::string& message);
		void ForcedLog(const int level, const char* message);

	protected:
		// file name
		std::string m_log_file_name;
		// file handle
		FILE* m_file_handle;
		// debug string buffer.
		char m_buffer[MAX_DEBUG_STRING_LENGTH*2+2];
		// for multi-threaded logging. 
		mutex	m_mutex;
		// whether this is first time this logger is used. we will create new file if it is true, otherwise we will append to existing file. 
		bool m_is_first_time_open;
		/** if true we will flush the new log to file immediately. otherwise, flush operation is determined by the system. 
		* default to true for client log and false for service log. 
		*/
		bool m_bForceFlush;

		/** The name of this logger.*/
		std::string m_name;

		/**	The assigned level of this logger. Default to 0, the higher, the more important.  The level variable need not be assigned a value in
		which case it is inherited from parent. */
		int m_level;

	};
#ifdef WIN32
#pragma warning( pop ) 
#endif

}

#ifndef OUTPUT_LOG
#define OUTPUT_LOG ParaEngine::CLogger::GetSingleton().WriteFormated
#endif

/** Logs formatted message to log file */
#define OUTPUT_LOG1 ParaEngine::CLogger::GetSingleton().WriteFormated_WithTime

#ifdef _DEBUG
#define OUTPUT_DEBUG(a) ParaEngine::CLogger::GetSingleton().WriteDebugStr("%s called from %s:%d\n",a, __FILE__,__LINE__);
#else
#define OUTPUT_DEBUG(a)
#endif


#if !defined(SERVICE_LOG_LOCATION)
#if defined(_MSC_VER)
#if _MSC_VER >= 1300
#define __SERVICE_LOG_FUNC__ __FUNCSIG__
#endif
#else
#if defined(__GNUC__)
#define __SERVICE_LOG_FUNC__ __PRETTY_FUNCTION__
#endif
#endif
#if !defined(__SERVICE_LOG_FUNC__)
#define __SERVICE_LOG_FUNC__ ""
#endif
#define SERVICE_LOG_LOCATION ParaEngine::LogDetail::LocationInfo(__FILE__, \
	__SERVICE_LOG_FUNC__,                                                         \
	__LINE__)
#endif


/**
Logs a service message to the application log
@param message the message string to log.
*/
#define APP_LOG(message) ParaEngine::CLogger::GetSingleton().ForcedLog(0, message, SERVICE_LOG_LOCATION);

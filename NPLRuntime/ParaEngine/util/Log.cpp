//-----------------------------------------------------------------------------
// Class:	CLog
// Authors:	LiXizhi
// Company: ParaEngine
// Date:	2006.3.4, revised 2009.5.9.it is now thread safe. 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"

#include "util/ParaTime.h"
#include "util/StringHelper.h"
#include "FileManager.h"
#include "util/mutex.h"
#include "Log.h"
#include "util/os_calls.h"
#ifndef EMSCRIPTEN_SINGLE_THREAD
#include <boost/thread/tss.hpp>
#endif
#if ANDROID
#include <android/log.h>
#define LOGI(...) ((void)__android_log_print(ANDROID_LOG_INFO, "ParaEngine", __VA_ARGS__))
#endif

using namespace std;

/** the etc length when too many characters are printed. This is larger then strlen("...\n")*/
#define LOG_TAIL_ETC_LENGTH		5

namespace ParaEngine
{
	namespace LogDetail
	{
		/**
		When location information is not available the constant
		<code>NA</code> is returned. Current value of this string
		constant is <b>?</b>.  */
		const char* const LocationInfo::NA = "?";
		const char* const LocationInfo::NA_METHOD = "?::?";

		const LocationInfo& LocationInfo::getLocationUnavailable() 
		{
			static const LocationInfo unavailable;
			return unavailable;
		}

		LocationInfo::LocationInfo( const char * const fileName1,
			const char * const methodName1,
			int lineNumber1 )
			:  lineNumber( lineNumber1 ),
			fileName( fileName1 ),
			methodName( methodName1 ) 
		{
		}

		LocationInfo::LocationInfo()
			: lineNumber( -1 ),
			fileName(LocationInfo::NA),
			methodName(LocationInfo::NA_METHOD) 
		{
		}

		LocationInfo::LocationInfo( const LocationInfo & src )
			:  lineNumber( src.lineNumber ),
			fileName( src.fileName ),
			methodName( src.methodName ) 
		{
		}

		LocationInfo & LocationInfo::operator = ( const LocationInfo & src )
		{
			fileName = src.fileName;
			methodName = src.methodName;
			lineNumber = src.lineNumber;
			return * this;
		}

		void LocationInfo::clear() 
		{
			fileName = NA;
			methodName = NA_METHOD;
			lineNumber = -1;
		}


		const char * LocationInfo::getFileName()
		{
			return fileName;
		}

		int LocationInfo::getLineNumber()
		{
			return lineNumber;
		}

		const std::string LocationInfo::getMethodName()
		{
			std::string tmp(methodName);
			size_t colonPos = tmp.find("::");
			if (colonPos != std::string::npos) {
				tmp.erase(0, colonPos + 2);
			} else {
				size_t spacePos = tmp.find(' ');
				if (spacePos != std::string::npos) {
					tmp.erase(0, spacePos + 1);
				}
			}
			size_t parenPos = tmp.find('(');
			if (parenPos != std::string::npos) {
				tmp.erase(parenPos);
			}
			return tmp;
		}


		const std::string LocationInfo::getClassName() 
		{
			std::string tmp(methodName);
			size_t colonPos = tmp.find("::");
			if (colonPos != std::string::npos) {
				tmp.erase(colonPos);
				size_t spacePos = tmp.find_last_of(' ');
				if (spacePos != std::string::npos) {
					tmp.erase(0, spacePos + 1);
				}
				return tmp;
			}
			tmp.erase(0, tmp.length() );
			return tmp;
		}

		void LocationInfo::write(CLogger* pOutput) const
		{
			if (lineNumber == -1 && fileName == NA && methodName == NA_METHOD) 
			{
				// Do nothing;
			} 
			else 
			{
				bool bNiceLooking = false;
				if(bNiceLooking)
				{
					char line[16];
					ParaEngine::StringHelper::fast_itoa(lineNumber, line, 16);
					//
					//   construct Java-like fullInfo (replace "::" with ".")
					//
					std::string fullInfo(methodName);
					size_t openParen = fullInfo.find('(');
					if (openParen != std::string::npos) {
						size_t space = fullInfo.find(' ');
						if (space != std::string::npos && space < openParen) {
							fullInfo.erase(0, space + 1);
						}
					}
					openParen = fullInfo.find('(');
					if (openParen != std::string::npos) {
						size_t classSep = fullInfo.rfind("::", openParen);
						if (classSep != std::string::npos) {
							fullInfo.replace(classSep, 2, ".");
						} else {
							fullInfo.insert(0, ".");
						}
					}
					fullInfo.append(1, '(');
					fullInfo.append(fileName);
					fullInfo.append(1, ':');
					fullInfo.append(line);
					fullInfo.append(1, ')');
					pOutput->Write(fullInfo.c_str(), (int)(fullInfo.size()));
				}
				else
				{
					pOutput->WriteFormated("Line %d: %s:%s", lineNumber, methodName, fileName);
				}
			}
		}
	}
}

namespace ParaEngine
{
	CLogger::CLogger(void)
		:m_file_handle(NULL), m_level(0)
	{
#ifdef PARAENGINE_MOBILE
		m_is_first_time_open = true;
#ifdef WIN32
		m_bForceFlush = true;
#else
		// for android client never flush, since disk is slow. 
		m_bForceFlush = false; 
#endif
#else
#if defined(PARAENGINE_CLIENT) || defined(PLATFORM_MAC) || defined(WIN32)
		m_bForceFlush = true;
		m_is_first_time_open = true;
#else
		m_bForceFlush = false;
		m_is_first_time_open = false;
#endif
#endif
		SetLogFile("log.txt");
	}

	CLogger::~CLogger(void)
	{
		CloseLog();
	}

	void CLogger::SetAppendMode(bool bAppendToExistingFile)
	{
		m_is_first_time_open = !bAppendToExistingFile;
	}

	void CLogger::SetForceFlush(bool bForceFlush)
	{
		m_bForceFlush = bForceFlush;
	}

	CLogger& CLogger::GetSingleton()
	{
		static CLogger g_app_logger;
		return g_app_logger;
	}

	FILE* CLogger::GetLogFileHandle()
	{
		if (m_file_handle==NULL) 
		{
#if WIN32 && defined(DEFAULT_FILE_ENCODING)
			LPCWSTR path16 = StringHelper::MultiByteToWideChar(m_log_file_name.c_str(), DEFAULT_FILE_ENCODING);
			m_file_handle = ::_wfopen(path16, m_is_first_time_open ? L"w+" : L"a+");
#else
			m_file_handle = fopen(m_log_file_name.c_str(), m_is_first_time_open ? "w+" : "a+");
#endif
		}
		if(m_file_handle)
		{
			// seek to end for simultaneous write 
			fseek(m_file_handle, 0,SEEK_END);
		}
		return m_file_handle;
	}

	void CLogger::CloseLog()
	{
		if(m_file_handle){
			fclose(m_file_handle);
			m_file_handle = NULL;
		}
	}

	void CLogger::SetLogFile(const string& sFile)
	{
		std::string filepath = sFile;

		if (!CParaFile::IsAbsolutePath(sFile))
		{
			filepath = CParaFile::GetWritablePath() + filepath;
		}

		if (filepath != m_log_file_name)
		{
			m_log_file_name = filepath;
			CloseLog();
		}
	}

	const string& CLogger::GetLogFile()
	{
		return m_log_file_name;
	}

	void CLogger::AddLogStr_st(const char * pStr)
	{

		if (pStr==NULL) {
			return;
		}

		// On some mobile phones, typing in Chinese logs will crash, just cancel it
#if (ANDROID && defined(_DEBUG))
		LOGI(pStr);
#endif

#if __APPLE__
    int appleStrLength = strnlen(pStr, MAX_DEBUG_STRING_LENGTH);

    if (appleStrLength > 0 && appleStrLength < MAX_DEBUG_STRING_LENGTH) {
        printf("%s", pStr);
    } else {
        char pDest[MAX_DEBUG_STRING_LENGTH] = {""};
        strncpy(pDest, pStr, MAX_DEBUG_STRING_LENGTH);
        printf("%s", pDest);
    }
#endif

#if defined(WIN32) && defined(_DEBUG) 
		printf("%s", pStr);
		::OutputDebugStringA(pStr);
#endif

		int nLength = strnlen(pStr, MAX_DEBUG_STRING_LENGTH);
		if(nLength>=0 && nLength<MAX_DEBUG_STRING_LENGTH)
		{
			Write_st(pStr, nLength);
		}
		else
		{
			nLength = MAX_DEBUG_STRING_LENGTH - 1;
			Write_st(pStr, nLength);
			AddLogStr_st("==>log message ignored, because it is longer than 1024\n");
		}
	}

	void CLogger::AddLogStr(const char * pStr)
	{
		ParaEngine::Lock lock_(m_mutex);
		AddLogStr_st(pStr);
	}

	int CLogger::Write_st(const char * buf, int nLength)
	{
		int nCount = 0;
		if (buf==NULL || nLength<=0) {
			return -1;
		}
#ifdef EMSCRIPTEN
		printf("%s", buf);
#endif
		FILE * pFile = GetLogFileHandle();
		if(pFile)
		{
			int nCount = (int)fwrite(buf, sizeof(char), nLength, pFile);
			if(m_bForceFlush)
				fflush(pFile);
			return nCount;
		}
		return -1;
	}
	int CLogger::Write(const char * buf, int nLength)
	{
		ParaEngine::Lock lock_(m_mutex);
		return Write_st(buf, nLength);
	}

	void CLogger::AddLogStr_st(const wchar_t * pStr)
	{
		if (pStr==NULL) {
			return;
		}
		FILE * pFile = GetLogFileHandle();
		if(pFile)
		{
			// seek to end for simultaneous write 
			fwprintf(pFile, pStr);
			if(m_bForceFlush)
				fflush(pFile);
		}
	}

	void CLogger::AddLogStr(const wchar_t * pStr)
	{
		ParaEngine::Lock lock_(m_mutex);
		AddLogStr_st(pStr);
	}

	void CLogger::WriteFormated_WithTime(const char * zFormat, ...)
	{
		
		// TODO: LXZ, shall we cache the date string, since it does not change often?
		// TODO LXZ: does STL in linux has SSO(small string optimization) like in windows? i.e. first 16 bytes on stack, instead of on heap?
		std::string date_str = ParaEngine::GetDateFormat("yyyy-MM-dd");
		std::string time_str = ParaEngine::GetTimeFormat(NULL);

		ParaEngine::Lock lock_(m_mutex);
		int nSize = snprintf(m_buffer, MAX_DEBUG_STRING_LENGTH, "%s %s|%d|", date_str.c_str(), time_str.c_str(), ParaEngine::GetThisThreadID());
		va_list args;
		va_start(args, zFormat);
		int nSize2 = vsnprintf(m_buffer+nSize, MAX_DEBUG_STRING_LENGTH-LOG_TAIL_ETC_LENGTH-nSize, zFormat, args);
		if(nSize2<0 || nSize2>=(MAX_DEBUG_STRING_LENGTH-LOG_TAIL_ETC_LENGTH-1-nSize))
		{
			nSize2 = MAX_DEBUG_STRING_LENGTH-LOG_TAIL_ETC_LENGTH-1-nSize;
			//nSize2 += snprintf(m_buffer+nSize+nSize2, MAX_DEBUG_STRING_LENGTH, "==>log message ignored, because it is longer than %d\n", MAX_DEBUG_STRING_LENGTH);
			nSize2 += snprintf(m_buffer+MAX_DEBUG_STRING_LENGTH-LOG_TAIL_ETC_LENGTH-1, LOG_TAIL_ETC_LENGTH, "...\n");
		}
		va_end(args);
		Write_st(m_buffer, nSize+nSize2);
	}

	void CLogger::WriteFormated(const char * zFormat,...)
	{
		
		ParaEngine::Lock lock_(m_mutex);
		va_list args;
		va_start(args, zFormat);
		int nSize2 = vsnprintf(m_buffer, MAX_DEBUG_STRING_LENGTH-LOG_TAIL_ETC_LENGTH, zFormat, args);
		if(nSize2<0 || nSize2>=(MAX_DEBUG_STRING_LENGTH-LOG_TAIL_ETC_LENGTH-1))
		{
			nSize2 = MAX_DEBUG_STRING_LENGTH-LOG_TAIL_ETC_LENGTH-1;
			//nSize2 += snprintf(m_buffer+nSize2, MAX_DEBUG_STRING_LENGTH, "==>log message ignored, because it is longer than %d\n", MAX_DEBUG_STRING_LENGTH);
			nSize2 += snprintf(m_buffer+MAX_DEBUG_STRING_LENGTH-LOG_TAIL_ETC_LENGTH-1, LOG_TAIL_ETC_LENGTH, "...\n");
		}
		va_end(args);
		AddLogStr_st(m_buffer);
	}

	void CLogger::WriteFormated(const wchar_t* zFormat,...)
	{
		ParaEngine::Lock lock_(m_mutex);
		va_list args;
		va_start(args, zFormat);
		int nSize = vsnwprintf((wchar_t*)m_buffer, MAX_DEBUG_STRING_LENGTH, zFormat, args);
		if(nSize < 0  || nSize>=MAX_DEBUG_STRING_LENGTH)
		{
			m_buffer[MAX_DEBUG_STRING_LENGTH-1] = '\0';
		}
		va_end(args);
	}

	void CLogger::WriteFormatedVarList(const char * zFormat, va_list args)
	{
		ParaEngine::Lock lock_(m_mutex);
		int nSize2 = vsnprintf(m_buffer, MAX_DEBUG_STRING_LENGTH - LOG_TAIL_ETC_LENGTH, zFormat, args);
		if (nSize2 < 0 || nSize2 >= (MAX_DEBUG_STRING_LENGTH - LOG_TAIL_ETC_LENGTH - 1))
		{
			nSize2 = MAX_DEBUG_STRING_LENGTH - LOG_TAIL_ETC_LENGTH - 1;
			//nSize2 += snprintf(m_buffer+nSize2, MAX_DEBUG_STRING_LENGTH, "==>log message ignored, because it is longer than %d\n", MAX_DEBUG_STRING_LENGTH);
			nSize2 += snprintf(m_buffer + MAX_DEBUG_STRING_LENGTH - LOG_TAIL_ETC_LENGTH - 1, LOG_TAIL_ETC_LENGTH, "...\n");
		}
		AddLogStr_st(m_buffer);
	}

	int CLogger::GetPos()
	{
		ParaEngine::Lock lock_(m_mutex);
		FILE * pFile = GetLogFileHandle();
		if(pFile)
		{
			fpos_t pos;
			fgetpos(pFile, &pos);
#ifdef WIN32
			return static_cast<int>(pos);
#elif defined (PLATFORM_ANDROID)
			return static_cast<int>(pos);
#elif defined (PLATFORM_IPHONE)
			return static_cast<int>(pos);
#elif defined (PLATFORM_MAC)
            return static_cast<int>(pos);
#elif defined(EMSCRIPTEN)
			// return static_cast<int>(pos);
#else
			return static_cast<int>(pos.__pos);
#endif
		}
		return 0;
	}

	const char* CLogger::GetLog(int fromPos, int nCount)
	{
		ParaEngine::Lock lock_(m_mutex);
		thread_local static std::vector<char> g_text;
		FILE * pFile = GetLogFileHandle();
		if(pFile)
		{
			if(nCount <0)
			{
				nCount = GetPos() - fromPos;
			}
			if(nCount>0)
			{
				fseek(pFile, fromPos, SEEK_SET);
				g_text.resize(nCount+1);
				nCount = (int)fread(&(g_text[0]), sizeof(char), nCount, pFile);
				g_text[nCount] = '\0';
				fseek(pFile, 0,SEEK_END);
				return &(g_text[0]);
			}
		}
		return NULL;
	}

	void CLogger::WriteDebugStr(const char * zFormat,const char * str, const char* sFile,int nLine)
	{
#ifdef _DEBUG
		ParaEngine::Lock lock_(m_mutex);
		int nSize = snprintf(m_buffer, MAX_DEBUG_STRING_LENGTH, zFormat, str, sFile, nLine);
		if(nSize< 0 || nSize>=MAX_DEBUG_STRING_LENGTH)
		{
			nSize = MAX_DEBUG_STRING_LENGTH-1;
			m_buffer[MAX_DEBUG_STRING_LENGTH] = '\0';
		}
#	ifdef WIN32
		OutputDebugStringA(m_buffer);
#	else
		Write_st(m_buffer, nSize);
#	endif
#endif
	}

	const std::string& CLogger::GetName()
	{
		return m_name;
	}

	int CLogger::GetLevel()
	{
		return m_level;
	}

	void CLogger::SetLevel( const int level1 )
	{
		m_level = level1;
	}

	bool CLogger::IsEnabledFor( int level )
	{
		return (level>=m_level);
	}


	void CLogger::log( const int level, const std::string& message, const ParaEngine::LogDetail::LocationInfo& location )
	{
		if (IsEnabledFor(level)) 
		{
			ForcedLog(level, message, location);
		}
	}

	void CLogger::log( const int level, const std::string& message )
	{
		if (IsEnabledFor(level)) 
		{
			ForcedLog(level, message);
		}
	}

	void CLogger::log( const int level, const char* message, const ParaEngine::LogDetail::LocationInfo& location )
	{
		if (IsEnabledFor(level)) 
		{
			ForcedLog(level, message, location);
		}
	}

	void CLogger::log( const int level, const char* message )
	{
		if (IsEnabledFor(level)) 
		{
			ForcedLog(level, message);
		}
	}
	void CLogger::ForcedLog( const int level, const std::string& message, const ParaEngine::LogDetail::LocationInfo& location )
	{
		ForcedLog(level, message.c_str(), location);
	}

	void CLogger::ForcedLog( const int level, const char* message, const ParaEngine::LogDetail::LocationInfo& location )
	{
		// TODO: LXZ, shall we cache the date string, since it does not change often?
		// TODO LXZ: does STL in linux has SSO(small string optimization) like in windows? i.e. first 16 bytes on stack, instead of on heap?
		std::string date_str = ParaEngine::GetDateFormat("yyyy-MM-dd");
		std::string time_str = ParaEngine::GetTimeFormat(NULL);
		WriteFormated("%s %s|%d|%d|%s|", date_str.c_str(), time_str.c_str(), ParaEngine::GetThisThreadID(), level, message);
		location.write(this);
		Write("|\n", 2);
	}

	void CLogger::ForcedLog( const int level, const std::string& message )
	{
		ForcedLog(level, message.c_str());
	}

	void CLogger::ForcedLog( const int level, const char* message )
	{
		// TODO: LXZ, shall we cache the date string, since it does not change often?
		// TODO LXZ: does STL in linux has SSO(small string optimization) like in windows? i.e. first 16 bytes on stack, instead of on heap?
		std::string date_str = ParaEngine::GetDateFormat("yyyy-MM-dd");
		std::string time_str = ParaEngine::GetTimeFormat(NULL);
		WriteFormated("%s %s|%d|%d|%s||\n", date_str.c_str(), time_str.c_str(), ParaEngine::GetThisThreadID(), level, message);
	}
}


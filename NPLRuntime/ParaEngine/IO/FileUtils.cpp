
//-----------------------------------------------------------------------------
// Class: FileUtil
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2005.4, 2014.8
// Desc: file platform related API and helper functions
// three sets of API: 1. native win32  2. boost  3. cocos2dx
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "FileUtils.h"
#include "StringHelper.h"
#include "Framework/FileSystem/ParaFileUtils.h"
#include "ZipWriter.h"
#include <time.h>
#include <sys/stat.h>

#ifdef WIN32
#include <direct.h>
#include <windows.h>
#include <io.h>
#include <sys/locking.h>
#include <sys/utime.h>
#include <fcntl.h>
#define lfs_setmode(L,file,m)   ((void)L, _setmode(_fileno(file), m))
#define STAT_STRUCT struct _stati64
#define STAT_FUNC _stati64
#define LSTAT_FUNC STAT_FUNC

#ifndef S_ISDIR
#define S_ISDIR(mode)  (mode&_S_IFDIR)
#endif
#ifndef S_ISREG
#define S_ISREG(mode)  (mode&_S_IFREG)
#endif
#ifndef S_ISLNK
#define S_ISLNK(mode)  (0)
#endif
#ifndef S_ISSOCK
#define S_ISSOCK(mode)  (0)
#endif
#ifndef S_ISFIFO
#define S_ISFIFO(mode)  (0)
#endif
#ifndef S_ISCHR
#define S_ISCHR(mode)  (mode&_S_IFCHR)
#endif
#ifndef S_ISBLK
#define S_ISBLK(mode)  (0)
#endif

#else
#include <unistd.h>
#include <dirent.h>
#include <fcntl.h>
#include <sys/types.h>
#include <utime.h>
#define _O_TEXT               0
#define _O_BINARY             0
#define lfs_setmode(L,file,m)   0
#define STAT_STRUCT struct stat
#define STAT_FUNC stat
#define LSTAT_FUNC lstat
#endif

#ifdef NPLRUNTIME_STATICLIB
 	#ifndef USE_BOOST_FILE_API
		#define USE_BOOST_FILE_API
	#endif
#endif

#if defined(PARAENGINE_SERVER) && !defined(WIN32)
	// the following macro fixed a linking bug if boost lib is not compiled with C++11
	#define BOOST_NO_CXX11_SCOPED_ENUMS
#endif
#include <boost/system/config.hpp>
#include <boost/system/system_error.hpp>
#include <boost/system/error_code.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>
#include <iostream>

namespace fs = boost::filesystem;
#define BOOST_FILESYSTEM_NO_DEPRECATED


std::map<std::string, ParaEngine::CFileUtils::EmbeddedResource> ParaEngine::CFileUtils::s_all_resources;

namespace ParaEngine
{
	std::string ParaEngine::CFileUtils::s_writepath;

#define CHECK_BIT(x,y) (((x)&(y))>0)

#ifndef Int32x32To64
#define Int32x32To64(a, b)  (((int64_t)((int32_t)(a))) * ((int64_t)((int32_t)(b))))
#endif

	/**
	A FILETIME is defined as : Contains a 64-bit value representing the number of 100-nanosecond intervals since January 1, 1601 (UTC).
	*/
	void TimetToFileTime(const std::time_t& t, FILETIME* pft)
	{
		// Microseconds between 1601-01-01 00:00:00 UTC and 1970-01-01 00:00:00 UTC
		int64_t ll = Int32x32To64(t, 10000000) + 116444736000000000;
		pft->dwLowDateTime = (DWORD)ll;
		pft->dwHighDateTime = ll >> 32;
	}

	/**
	A FILETIME is defined as : Contains a 64-bit value representing the number of 100-nanosecond intervals since January 1, 1601 (UTC).
	time_t is an integral value holding the number of seconds (not counting leap seconds) since 00:00, Jan 1 1970 UTC, corresponding to POSIX time
	*/
	time_t FileTimeToTimet(const FILETIME& ft)
	{
		// Microseconds between 1601-01-01 00:00:00 UTC and 1970-01-01 00:00:00 UTC
		static const uint64_t EPOCH_DIFFERENCE_MICROS = 11644473600000000ull;

		// First convert 100-ns intervals to microseconds, then adjust for the
		// epoch difference
		uint64_t total_us = (((uint64_t)ft.dwHighDateTime << 32) | (uint64_t)ft.dwLowDateTime) / 10;
		total_us -= EPOCH_DIFFERENCE_MICROS;
		return (time_t)(total_us / 1000000);
	}
}

ParaEngine::CParaFileInfo::CParaFileInfo() 
	: m_mode(ParaEngine::CParaFileInfo::ModeNoneExist), m_dwFileSize(0), m_dwFileAttributes(0), m_ftLastWriteTime(0), m_ftCreationTime(0), m_ftLastAccessTime(0)
{
}

void ParaEngine::CFileUtils::MakeFileNameFromRelativePath(char * output, const char* filename, const char* relativePath)
{
	int nLastSlash = -1;
	if (relativePath != NULL)
	{
		char c;
		for (int i = 0; (c = relativePath[i]) != '\0' && i < MAX_PATH_LENGTH; i++)
		{
			output[i] = c;
			if ((c == '/') || (c == '\\'))
				nLastSlash = i;
		}
	}
	if (filename != 0 && filename[0] != '\0')
	{
		strncpy(output + nLastSlash + 1, filename, MAX_PATH_LENGTH - nLastSlash - 1);
	}
	else
	{
		output[nLastSlash + 1] = '\0';
	}
}

bool ParaEngine::CFileUtils::FileExist(const char* filename)
{
	if (FileExistRaw(filename))
	{
		return true;
	}
	else
	{
		if (!CParaFile::GetDevDirectory().empty())
		{
			if (filename[0] != '\0' && filename[1] != ':')
			{
				std::string sAbsFilePath = CParaFile::GetDevDirectory() + filename;
				return FileExistRaw(sAbsFilePath.c_str());
			}
		}
	}
	return false;
}

bool ParaEngine::CFileUtils::FileExist2(const char * filename, std::string * pDiskFile)
{
	if (FileExistRaw(filename))
	{
		if (pDiskFile)
			*pDiskFile = filename;
		return true;
	}
	else
	{
		if (!CParaFile::GetDevDirectory().empty())
		{
			if (filename[0] != '\0' && filename[1] != ':')
			{
				std::string sAbsFilePath = CParaFile::GetDevDirectory() + filename;
				if (FileExistRaw(sAbsFilePath.c_str()))
				{
					if (pDiskFile)
						*pDiskFile = sAbsFilePath;
					return true;
				}
			}
		}
	}
	return false;
}

bool ParaEngine::CFileUtils::IsAbsolutePath(const std::string& filename)
{
	return CParaFileUtils::GetInstance()->IsAbsolutePath(filename);
}

void ParaEngine::CFileUtils::TrimString(std::string & str)
{
	int nLast = (int)str.size() - 1;
	int nTo = nLast;
	for (; nTo >= 0 && str[nTo] == ' '; --nTo)
	{
	}
	int nFrom = 0;
	for (; nFrom <= nLast && str[nFrom] == ' '; ++nFrom)
	{
	}

	if (nFrom == 0 && nTo == nLast)
		return;
	else
	{
		str = str.substr(nFrom, nTo - nFrom + 1);
	}
}

bool ParaEngine::CFileUtils::CopyFile(const char* src, const char* dest, bool bOverride)
{
	if (!bOverride && FileExist(dest))
	{
		return false;
	}
	CParaFile fileFrom;
	fileFrom.OpenAssetFile(src);
	if (!fileFrom.isEof())
	{
		CParaFile fileTo;
		if (fileTo.CreateNewFile(dest))
		{
			fileTo.write(fileFrom.getBuffer(), (int)(fileFrom.getSize()));

			//try to restore timestamp of the file
			if(fileFrom.GetLastModifiedTime()>0)
				fileTo.WriteLastModifiedTime(fileFrom.GetLastModifiedTime());
			return true;
		}
	}
	else
	{
		return CParaFileUtils::GetInstance()->Copy(src, dest, bOverride);
	}
	return false;
}

bool ParaEngine::CFileUtils::MoveFile(const char* src, const char* dest)
{
	return CParaFileUtils::GetInstance()->Move(src, dest);
}

bool ParaEngine::CFileUtils::MakeDirectoryFromFilePath(const char * filename)
{
	return CParaFileUtils::GetInstance()->MakeDirectoryFromFilePath(filename);
}

std::string ParaEngine::CFileUtils::GetFileName(const std::string& sfilename)
{
	for (int i = (int)sfilename.size() - 1; i >= 0; --i)
	{
		if (sfilename[i] == '/' || sfilename[i] == '\\')
		{
			int nCount = (int)sfilename.size() - i - 1;
			if (nCount > 0)
				return sfilename.substr(i + 1, nCount);
			else
				return "";
		}
	}
	return sfilename;
}

std::string ParaEngine::CFileUtils::GetParentDirectoryFromPath(const std::string& sfilename, int nParentCounts)
{
	int nSlashCount = 0;
	int i;
	for (i = (int)sfilename.size() - 1; i >= 0; --i)
	{
		if (sfilename[i] == '\\' || sfilename[i] == '/')
		{
			if (nSlashCount >= nParentCounts)
			{
				return sfilename.substr(0, i + 1);
			}
			else
				nSlashCount++;
		}
	}
	return "";
}

bool ParaEngine::CFileUtils::SaveBufferToFile(const string& filename, bool bReplace, char* buffer, DWORD nBufSize)
{
	return CParaFileUtils::GetInstance()->SaveBufferToFile(filename, bReplace, buffer, nBufSize);
}

bool ParaEngine::CFileUtils::DeleteFile(const char* filename)
{
	return CParaFileUtils::GetInstance()->Delete(filename);
}

int ParaEngine::CFileUtils::DeleteDirectory(const char* filename)
{
	return CParaFileUtils::GetInstance()->DeleteDirectory(filename);
}

int ParaEngine::CFileUtils::DeleteFiles(const std::string& sFilePattern, bool bSecureFolderOnly /*= true*/, int nSubFolderCount)
{

	// search in writable disk files
	// Note: bSecureFolderOnly is ignore, since it is guaranteed by the OS's writable path.
	int nCount = 0;
	std::string sRootPath = GetParentDirectoryFromPath(sFilePattern);
	std::string sPattern = GetFileName(sFilePattern);
	sRootPath = GetWritableFullPathForFilename(sRootPath);
	std::string sFullPattern = GetWritableFullPathForFilename(sFilePattern);
	boost::system::error_code err_code;
#if defined(WIN32) && defined(DEFAULT_FILE_ENCODING)
	LPCWSTR sFullPattern16 = StringHelper::MultiByteToWideChar(sFullPattern.c_str(), DEFAULT_FILE_ENCODING);
#else 
	const auto& sFullPattern16 = sFullPattern;
#endif
	if (fs::is_directory(sFullPattern16,err_code))
	{
		nCount = CFileUtils::DeleteDirectory(sRootPath.c_str());
	}
	else
	{
		CSearchResult result;
		result.InitSearch(sRootPath, 0, 10000, 0);
		FindDiskFiles(result, result.GetRootPath(), sPattern, nSubFolderCount);

		nCount = result.GetNumOfResult();
		for (int i = 0; i < nCount; ++i)
		{
			const CFileFindData* pFileDesc = result.GetItemData(i);
			if (pFileDesc)
			{
				std::string sFilePath = sRootPath + result.GetItem(i);
				CFileUtils::DeleteFile(sFilePath.c_str());
			}
		}
		OUTPUT_LOG("%d files removed from directory %s\n", nCount, sFilePattern.c_str());
	}
	return nCount;
}

namespace ParaEngine
{
	bool _GetFileInfo_(const char* filename, CParaFileInfo& fileInfo)
	{
		STAT_STRUCT info;
#ifdef DEFAULT_FILE_ENCODING
		LPCWSTR filename16 = StringHelper::MultiByteToWideChar(filename, DEFAULT_FILE_ENCODING);
		if (_wstati64(filename16, &info)) {
			// file not found
			return false;
		}
#else
		if (STAT_FUNC(filename, &info)) {
			// file not found
			return false;
		}
#endif
		auto mode = info.st_mode;
		if (S_ISREG(mode))
			fileInfo.m_mode = CParaFileInfo::ModeFile;
		else if (S_ISDIR(mode))
			fileInfo.m_mode = CParaFileInfo::ModeDirectory;
		else if (S_ISLNK(mode))
			fileInfo.m_mode = CParaFileInfo::ModeLink;
		else if (S_ISSOCK(mode))
			fileInfo.m_mode = CParaFileInfo::ModeSocket;
		else if (S_ISFIFO(mode))
			fileInfo.m_mode = CParaFileInfo::ModeNamedPipe;
		else if (S_ISCHR(mode))
			fileInfo.m_mode = CParaFileInfo::ModeCharDevice;
		else if (S_ISBLK(mode))
			fileInfo.m_mode = CParaFileInfo::ModeBlockDevice;
		else
			fileInfo.m_mode = CParaFileInfo::ModeOther;

		fileInfo.m_ftLastWriteTime = info.st_mtime;
		fileInfo.m_ftCreationTime = info.st_ctime;
		fileInfo.m_ftLastAccessTime = info.st_atime;
		fileInfo.m_dwFileSize = (DWORD)info.st_size;
		fileInfo.m_dwFileAttributes = (DWORD)info.st_mode;
		return true;
	}
}

bool ParaEngine::CFileUtils::GetFileInfo(const char* filename, CParaFileInfo& fileInfo)
{
	if (!CParaFile::GetDevDirectory().empty())
	{
		std::string sAbsFilePath = CParaFile::GetDevDirectory() + filename;
		if (_GetFileInfo_(sAbsFilePath.c_str(), fileInfo))
		{
			fileInfo.m_dwFileAttributes = 0;
			return true;
		}
	}
	if (_GetFileInfo_(filename, fileInfo))
	{
		fileInfo.m_sFullpath = filename;
		return true;
	}
#if (PARA_TARGET_PLATFORM == PARA_PLATFORM_ANDROID)
	else if (!IsAbsolutePath(filename))
	{
		// we will also search in asset folder in APK file for android version only. 
		ParaEngine::IReadFile* pFile = CParaFileUtils::GetInstance()->OpenFileForRead(filename);
		if (pFile)
		{
			bool bExist = pFile->isOpen();
			if (bExist)
			{
				fileInfo.m_dwFileSize = pFile->getSize();
				fileInfo.m_mode = CParaFileInfo::ModeFileInZip;
			}
			SAFE_DELETE(pFile);
			return bExist;
		}
	}
#endif
	return false;
}

ParaEngine::FileData ParaEngine::CFileUtils::GetDataFromFile(const char* filename)
{
	FileData data;
	if (!CParaFile::GetDevDirectory().empty() && !IsAbsolutePath(filename))
	{
		std::string devFilePath = CParaFile::GetDevDirectory() + filename;
		data = CParaFileUtils::GetInstance()->GetDataFromFile(devFilePath.c_str());
		if (data.isNull())
		{
			data = CParaFileUtils::GetInstance()->GetDataFromFile(filename);
		}
	}
	else 
	{
		data = CParaFileUtils::GetInstance()->GetDataFromFile(filename);
	}
	return data;
}

std::string ParaEngine::CFileUtils::GetStringFromFile(const std::string& filename)
{
	auto data = GetDataFromFile(filename.c_str());
	if (data.isNull())
		return "";

	std::string ret((const char*)data.GetBytes());
	return ret;
}

ParaEngine::FileData ParaEngine::CFileUtils::GetResDataFromFile(const std::string& filename)
{
	FileData data;
	// always search for code embedded data first
	auto it = s_all_resources.find(filename);
	if (it != s_all_resources.end())
	{
		size_t nSize = (size_t)(it->second.size());
		char* buffer = (char*)(it->second.data());
		data.SetOwnBuffer(buffer, nSize);
		return data;
	}
	return data;
}

bool ParaEngine::CFileUtils::DoesResFileExist(const std::string& filename)
{
	FileData data = GetResDataFromFile(filename);
	if (!data.isNull())
	{
		data.ReleaseOwnership();
		return true;
	}
	return false;
}

void ParaEngine::CFileUtils::AddEmbeddedResource(const char* name, const char* buffer, size_t nSize)
{
	s_all_resources[name] = EmbeddedResource(buffer, nSize);
}

std::string ParaEngine::CFileUtils::GetWritableFullPathForFilename(const std::string& filename)
{
	std::string sFilePath = filename;
	if (!IsAbsolutePath(sFilePath))
	{
		sFilePath = GetWritablePath() + sFilePath;
		// sFilePath = GetFullPathForFilename(sFilePath);
	}
	return sFilePath;
}

std::string ParaEngine::CFileUtils::GetFullPathForFilename(const std::string &filename)
{
	return CParaFileUtils::GetInstance()->GetFullPathForFilename(filename);
}

bool ParaEngine::CFileUtils::FileExistRaw(const char* filename)
{

	return CParaFileUtils::GetInstance()->Exists(filename);
}

int ParaEngine::CFileUtils::GetFileSize(const char* sFilePath)
{
	int nResult = 0;
	try
	{
#if defined(WIN32) && defined(DEFAULT_FILE_ENCODING)
		LPCWSTR sFilePath16 = StringHelper::MultiByteToWideChar(sFilePath, DEFAULT_FILE_ENCODING);
#else 
		const auto& sFilePath16 = sFilePath;
#endif
		nResult = (int)fs::file_size(sFilePath16);
	}
	catch (...)
	{
		OUTPUT_LOG("error: failed to get file size of %s\n", sFilePath);
	}
	return nResult;
}

ParaEngine::FileHandle ParaEngine::CFileUtils::OpenFile(const char* filename, bool bRead /*= false*/, bool bWrite/*=true*/)
{
	std::string sFilePath;
	if (bWrite && !IsAbsolutePath(filename))
	{
#ifdef EMSCRIPTEN
		// 设置可写路径情况 排除 temp 目录
		if (sFilePath.substr(0, 5) != "temp/") sFilePath = GetWritablePath() + filename;
		else sFilePath = filename;
#else
		sFilePath = GetWritablePath() + filename;
#endif
	}
	else {
		sFilePath = filename;
	}

#if WIN32 && defined(DEFAULT_FILE_ENCODING)
	std::wstring sFilePath16 = StringHelper::MultiByteToWideChar(sFilePath.c_str(), DEFAULT_FILE_ENCODING);
	FILE* pFile = _wfopen(sFilePath16.c_str(), bRead ? (bWrite ? L"w+b" : L"rb") : L"wb");
#else
	FILE* pFile = fopen(sFilePath.c_str(), bRead ? (bWrite ? "w+b" : "rb") : "wb");
#endif
	
	if (pFile == 0) 
	{
		OUTPUT_LOG("failed to open file: %s with mode %s\n", sFilePath.c_str(), bRead ? (bWrite ? "w+b" : "rb") : "wb");
	}
	else
	{
		if (sFilePath != filename)
		{
			OUTPUT_LOG("open file rename %s => %s\n", filename, sFilePath.c_str());
		}
	}
	FileHandle fileHandle;
	fileHandle.m_pFile = pFile;
	return fileHandle;
}

bool ParaEngine::CFileUtils::SetFilePointer(FileHandle& fileHandle, int lDistanceToMove, int dwMoveMethod)
{
	if (fileHandle.IsValid())
	{
		if (dwMoveMethod == 0)
			dwMoveMethod = FILE_BEGIN;
		else if (dwMoveMethod == 1)
			dwMoveMethod = FILE_CURRENT;
		else if (dwMoveMethod == 2)
			dwMoveMethod = FILE_END;
		else
		{
			OUTPUT_LOG("error: unknown SetFilePointer() move method\r\n");
			return false;
		}
		fseek(fileHandle.m_pFile, lDistanceToMove, dwMoveMethod);		
		return true;
	}
	return false;
}

int ParaEngine::CFileUtils::GetFilePosition(FileHandle& fileHandle)
{
	if (fileHandle.IsValid())
	{
		return ftell(fileHandle.m_pFile);		
	}
	return 0;
}

bool ParaEngine::CFileUtils::SetEndOfFile(FileHandle& fileHandle)
{
	if (fileHandle.IsValid())
	{
		fseek(fileHandle.m_pFile, 0, SEEK_END);		
		return true;
	}
	return false;
}

int ParaEngine::CFileUtils::WriteBytes(FileHandle& fileHandle, const void* src, int bytes)
{
	if (fileHandle.IsValid())
	{
		return (int)fwrite(src, 1, bytes, fileHandle.m_pFile);
	}
	return 0;
}

int ParaEngine::CFileUtils::ReadBytes(FileHandle& fileHandle, void* dest, int bytes)
{
	if (fileHandle.IsValid())
	{
		return (int)fread(dest, 1, bytes, fileHandle.m_pFile);		
	}
	return 0;
}

void ParaEngine::CFileUtils::CloseFile(FileHandle& fileHandle)
{
	if (fileHandle.IsValid())
	{
		fclose(fileHandle.m_pFile);		
		fileHandle.m_pFile = nullptr;
	}
}

const std::string& ParaEngine::CFileUtils::GetInitialDirectory()
{
	std::string sWorkingDir = CParaFileUtils::GetInstance()->GetInitialDirectory();
	static std::string sRootDir;
	sRootDir = "";
	CParaFile::ToCanonicalFilePath(sRootDir, sWorkingDir, false);
	if(sRootDir.size()>0 && (sRootDir[sRootDir.size()-1] != '/' && sRootDir[sRootDir.size()-1] != '\\'))
	{
		sRootDir += "/";
	}
	return sRootDir;
}

const std::string& ParaEngine::CFileUtils::GetExternalStoragePath()
{
	return CParaFileUtils::GetInstance()->GetExternalStoragePath();
}

const std::string& ParaEngine::CFileUtils::GetWritablePath()
{
	if (s_writepath.empty())
	{
		s_writepath = CParaFileUtils::GetInstance()->GetWritablePath();
	}
	return s_writepath;
}

void ParaEngine::CFileUtils::SetWritablePath(const std::string& writable_path)
{
	if (s_writepath != writable_path)
	{
		s_writepath = writable_path;
		if (!writable_path.empty())
		{
			if (MakeDirectoryFromFilePath(writable_path.c_str()))
			{
				s_writepath = writable_path;
			}
			else
			{
				OUTPUT_LOG("warn: failed to set writable path to %s\n", writable_path.c_str());
			}
		}

		if (!s_writepath.empty())
		{
			char nLastChar = writable_path[writable_path.size() - 1];
			if (nLastChar != '/' && nLastChar != '\\')
			{
				s_writepath = s_writepath + "/";
			}
			CParaFile::ToCanonicalFilePath(s_writepath, s_writepath);
		}
	}
}


void FindFiles_Recursive(ParaEngine::CSearchResult& result, fs::path rootPath, const std::string& reFilePattern, int nSubLevel)
{
	try
	{
		int nFileCount = 0;

		fs::directory_iterator end_itr; // default construction yields past-the-end
		for (fs::directory_iterator iter(rootPath); iter != end_itr; ++iter)
		{
			//cellfy: file_attr is only marked with directory(16) and regular_file(32) for now
			DWORD file_attr = 0;
			if (fs::is_directory(iter->status()))
			{
				// Found directory;
				file_attr = 16;
				if (nSubLevel > 0)
				{
					FindFiles_Recursive(result, iter->path(), reFilePattern, nSubLevel - 1);
				}
				if (reFilePattern == "*." || reFilePattern == "*.*")
				{
					// both directory and sub directory files are turned. we will return directory in the end to allow recursive file deletion in a folder. 
					nFileCount++;
					auto lastWriteTime = fs::last_write_time(iter->path());
					FILETIME fileLastWriteTime;
					ParaEngine::TimetToFileTime(lastWriteTime, &fileLastWriteTime);

#if defined(WIN32) && defined(DEFAULT_FILE_ENCODING)
					std::wstring sFullPath16 = iter->path().wstring();
					std::string sFullPath = ParaEngine::StringHelper::WideCharToMultiByte(sFullPath16.c_str(), DEFAULT_FILE_ENCODING);
#else
					std::string sFullPath = iter->path().string();
#endif
#ifdef WIN32
					ParaEngine::CParaFile::ToCanonicalFilePath(sFullPath, sFullPath, false);
#endif

					if (!result.AddResult(sFullPath, 0, file_attr, &fileLastWriteTime, &fileLastWriteTime, &fileLastWriteTime))
						return;
				}
				else if (ParaEngine::StringHelper::MatchWildcard(iter->path().filename().string(), reFilePattern))
				{
					nFileCount++;
					auto lastWriteTime = fs::last_write_time(iter->path());
					FILETIME fileLastWriteTime;
					ParaEngine::TimetToFileTime(lastWriteTime, &fileLastWriteTime);
#if defined(WIN32) && defined(DEFAULT_FILE_ENCODING)
					std::wstring sFullPath16 = iter->path().wstring();
					std::string sFullPath = ParaEngine::StringHelper::WideCharToMultiByte(sFullPath16.c_str(), DEFAULT_FILE_ENCODING);
#else
					std::string sFullPath = iter->path().string();
#endif
#ifdef WIN32
					ParaEngine::CParaFile::ToCanonicalFilePath(sFullPath, sFullPath, false);
#endif

					if (!result.AddResult(sFullPath, 0, file_attr, &fileLastWriteTime, &fileLastWriteTime, &fileLastWriteTime))
						return;
				}
			}
			else if (fs::is_regular_file(iter->status()))
			{
				file_attr = 32;
				if (ParaEngine::StringHelper::MatchWildcard(iter->path().filename().string(), reFilePattern))
				{
					// Found file;
					nFileCount++;
					auto lastWriteTime = fs::last_write_time(iter->path());
					FILETIME fileLastWriteTime;
					ParaEngine::TimetToFileTime(lastWriteTime, &fileLastWriteTime);
#if defined(WIN32) && defined(DEFAULT_FILE_ENCODING)
					std::wstring sFullPath16 = iter->path().wstring();
					std::string sFullPath = ParaEngine::StringHelper::WideCharToMultiByte(sFullPath16.c_str(), DEFAULT_FILE_ENCODING);
#else
					std::string sFullPath = iter->path().string();
#endif
#ifdef WIN32
					ParaEngine::CParaFile::ToCanonicalFilePath(sFullPath, sFullPath, false);
#endif
					if (!result.AddResult(sFullPath, (DWORD)fs::file_size(iter->path()), file_attr, &fileLastWriteTime, &fileLastWriteTime, &fileLastWriteTime))
						return;
				}
			}
		}
	}
	catch (...) {}
}

void ParaEngine::CFileUtils::FindDiskFiles(CSearchResult& result, const std::string& sRootPath, const std::string& sFilePattern, int nSubLevel)
{
	std::string path = GetWritableFullPathForFilename(sRootPath);
#if defined(WIN32) && defined(DEFAULT_FILE_ENCODING)
	LPCWSTR path16 = StringHelper::MultiByteToWideChar(path.c_str(), DEFAULT_FILE_ENCODING);
#else
	auto path16 = path;
#endif
	fs::path rootPath(path16);
	if (!fs::exists(rootPath) || !fs::is_directory(rootPath)) {
		OUTPUT_LOG("directory does not exist %s \n", rootPath.string().c_str());
		return;
	}
#if defined(WIN32) && defined(DEFAULT_FILE_ENCODING)
	std::string root = StringHelper::WideCharToMultiByte(rootPath.wstring().c_str(), DEFAULT_FILE_ENCODING);
	result.SetRootPath(root);
#else
	result.SetRootPath(rootPath.string());
#endif
	FindFiles_Recursive(result, rootPath, sFilePattern, nSubLevel);
}

void ParaEngine::CFileUtils::FindLocalFiles(CSearchResult& result, const std::string& sRootPath, const std::string& sFilePattern, int nSubLevel)
{
	CParaFileUtils::GetInstance()->FindLocalFiles(result, sRootPath, sFilePattern, nSubLevel);
}


bool ParaEngine::CFileUtils::AddDiskSearchPath(const std::string& sFile, bool nFront /*= false*/)
{
	OUTPUT_LOG("warn: AddDiskSearchPath :%s not supported. ", sFile.c_str());
	return false;
}

bool ParaEngine::CFileUtils::WriteLastModifiedTimeToDisk(FileHandle& fileHandle, const std::string& fileName, const time_t& lastModifiedTime)
{
	bool op_result = false;
	if (!fileName.empty())
	{
		std::string sFilePath;
		if (IsAbsolutePath(fileName))
		{
			sFilePath = GetFullPathForFilename(fileName);
		}
		else
		{
			sFilePath = GetWritablePath() + fileName;
			sFilePath = GetFullPathForFilename(sFilePath);
		}
#if defined(WIN32) && defined(DEFAULT_FILE_ENCODING)
		std::wstring sFilePath16 = StringHelper::MultiByteToWideChar(sFilePath.c_str(), DEFAULT_FILE_ENCODING);
#else
		auto sFilePath16 = sFilePath;
#endif
		fs::path filePath(sFilePath16);
		boost::system::error_code err_code;
		fs::last_write_time(filePath, lastModifiedTime, err_code);
		op_result = (err_code.value() == boost::system::errc::success);
	}

	return op_result;
}

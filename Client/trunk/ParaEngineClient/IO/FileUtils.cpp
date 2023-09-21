
//-----------------------------------------------------------------------------
// Class: FileUtil
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2005.4, 2014.8
// Desc: file platform related API and helper functions
// three sets of API: 1. native win32  2. boost  3. cocos2dx
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#if defined (PARAENGINE_CLIENT) && defined(WIN32) 
	#ifdef USE_OPENGL_RENDERER
		#include "platform/win32/ParaEngineApp.h"
	#else
		#include "ParaEngineApp.h"
	#endif
#endif
#include "FileUtils.h"
#include "StringHelper.h"
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


#if defined(NPLRUNTIME_STATICLIB) || defined(WINXP)
	#ifndef USE_BOOST_FILE_API
		#define USE_BOOST_FILE_API
	#endif
#endif

#ifdef _DEBUG
 //#define USE_BOOST_FILE_API
#endif

#if defined(PARAENGINE_SERVER) && !defined(WIN32)
	// the following macro fixed a linking bug if boost lib is not compiled with C++11
	#define BOOST_NO_CXX11_SCOPED_ENUMS
#endif
#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>
#include <iostream>
namespace fs = boost::filesystem;
#define BOOST_FILESYSTEM_NO_DEPRECATED


#ifdef USE_COCOS_FILE_API
	#include "OpenGLWrapper.h"
	/** @Note by LiXizhi: all cocos read file api are not thread safe, hence we need to use a lock. 
	remove this when cocos no longer uses non-thread-safe file cache map. */
	static std::mutex  s_cocos_file_io_mutex;
#ifdef COCOS_IO_THREAD_SAFE_PARAENGINE
	typedef ParaEngine::no_lock<std::mutex> FileLock_type;
#else
	typedef std::unique_lock<std::mutex> FileLock_type;
#endif
#endif

std::map<std::string, ParaEngine::CFileUtils::EmbeddedResource> ParaEngine::CFileUtils::s_all_resources;

namespace ParaEngine
{
	std::string ParaEngine::CFileUtils::s_writepath;
	
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
	if (filename != 0)
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
#ifdef WIN32
	if (filename.find(":") != string::npos)
		return true;
#endif
#ifdef USE_COCOS_FILE_API
	return cocos2d::FileUtils::getInstance()->isAbsolutePath(filename);
#else
	return filename[0] == '/';
#endif	
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
#ifdef USE_COCOS_FILE_API
		//OUTPUT_LOG("copy file not implemented in mobile version: from %s to %s\n", src, dest);
		try
		{
			std::string destWritablePath = GetWritableFullPathForFilename(dest);
			std::string srcWritablePath = GetWritableFullPathForFilename(src);
			fs::path destFile(destWritablePath);
			if (fs::exists(destFile))
			{
				if (bOverride)
				{
					if (!fs::remove(destFile))
					{
						return false;
					}
				}
				else
				{
					return false;
				}
			}
			fs::copy_file(fs::path(srcWritablePath), destFile);
		}
		catch (...)
		{
			return false;
		}
		return true;
#elif defined(USE_BOOST_FILE_API)
		try
		{
#ifdef WIN32
			std::wstring src16 = StringHelper::MultiByteToWideChar(src, DEFAULT_FILE_ENCODING);
			std::wstring dest16 = StringHelper::MultiByteToWideChar(dest, DEFAULT_FILE_ENCODING);
#else
			auto src16 = src;
			auto dest16 = dest;
#endif
			fs::path destFile(dest16);
			if (fs::exists(destFile))
			{
				if (bOverride)
				{
					if (!fs::remove(destFile))
					{
						return false;
					}
				}
				else
				{
					return false;
				}
			}
			fs::copy_file(fs::path(src16), destFile);
		}
		catch (...)
		{
			return false;
		}
		return true;
#else
		// try the system version, in case the src file is currently in use, such as the database. 

		std::string src_ = std::string("\\\\?\\") + GetWritableFullPathForFilename(src);
		std::replace(src_.begin(), src_.end(), '/', '\\');

		std::string dest_ = std::string("\\\\?\\") + GetWritableFullPathForFilename(dest);
		std::replace(dest_.begin(), dest_.end(), '/', '\\');

#ifdef DEFAULT_FILE_ENCODING
		std::wstring src16 = StringHelper::MultiByteToWideChar(src_.c_str(), DEFAULT_FILE_ENCODING);
		std::wstring dest16 = StringHelper::MultiByteToWideChar(dest_.c_str(), DEFAULT_FILE_ENCODING);
		return ::CopyFileW(src16.c_str(), dest16.c_str(), !bOverride) == TRUE;
#else
		return ::CopyFile(src_.c_str(), dest_.c_str(), !bOverride) == TRUE;
#endif
		
#endif
	}
	return false;
}

bool ParaEngine::CFileUtils::MoveFile(const char* src, const char* dest)
{
#ifdef USE_COCOS_FILE_API
	// OUTPUT_LOG("move file not implemented in mobile version: from %s to %s\n", src, dest);
	std::string src_path = GetWritableFullPathForFilename(src);
	std::string dest_path = GetWritableFullPathForFilename(dest);
	try
	{
		fs::path sSrc(src_path);
		fs::copy_file(sSrc, fs::path(dest_path), fs::copy_option::overwrite_if_exists);
		return fs::remove(sSrc);
	}
	catch (...)
	{
		OUTPUT_LOG("warning: move file failed: from %s to %s\n", src, dest);
		return false;
	}
#elif defined(USE_BOOST_FILE_API)
	try
	{
#ifdef WIN32
		std::wstring src16 = StringHelper::MultiByteToWideChar(src, DEFAULT_FILE_ENCODING);
		std::wstring dest16 = StringHelper::MultiByteToWideChar(dest, DEFAULT_FILE_ENCODING);
#else
		auto src16 = src;
		auto dest16 = dest;
#endif
		fs::path sSrc(src16);
		boost::system::error_code err_code;
		fs::rename(sSrc, fs::path(dest16), err_code);
		OUTPUT_LOG("info (boost-fs): moved file/directory from %s to %s result message: %s\n", src, dest, err_code.message().c_str());
		return err_code.value() == 0;
	}
	catch (...)
	{
		return false;
	}
#else
	if (dest != NULL && src != NULL)
	{
		std::string src_ = std::string("\\\\?\\") + GetWritableFullPathForFilename(src);
		std::replace(src_.begin(), src_.end(), '/', '\\');

		std::string dest_ = std::string("\\\\?\\") + GetWritableFullPathForFilename(dest);
		std::replace(dest_.begin(), dest_.end(), '/', '\\');

#ifdef DEFAULT_FILE_ENCODING
		std::wstring src16 = StringHelper::MultiByteToWideChar(src_.c_str(), DEFAULT_FILE_ENCODING);
		std::wstring dest16 = StringHelper::MultiByteToWideChar(dest_.c_str(), DEFAULT_FILE_ENCODING);
		return ::MoveFileExW(src16.c_str(), dest16.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) == TRUE;
#else
		return ::MoveFileEx(src_.c_str(), dest_.c_str(), MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) == TRUE;
#endif
	}
#endif
	return false;
}

bool ParaEngine::CFileUtils::MakeDirectoryFromFilePath(const char * filename)
{
#ifdef USE_COCOS_FILE_API
	try
	{
		std::string path = filename;
		if (!IsAbsolutePath(path))
		{
			path = GetWritablePath() + path;
		}
		fs::path filePath(path);
		fs::path fileDir = filePath.parent_path();
		if (!fs::is_directory(fileDir))
		{
			return fs::create_directories(fileDir);
		}
			
		else
			return true;
	}
	catch (...){}
	return false;
#elif defined(USE_BOOST_FILE_API)
	try
	{
#ifdef WIN32
		LPCWSTR filename16 = StringHelper::MultiByteToWideChar(filename, DEFAULT_FILE_ENCODING);
#else
		auto filename16 = filename;
#endif
		fs::path filePath(filename16);
		fs::path fileDir = filePath.parent_path();
		if( !fs::exists(fileDir))
			return fs::create_directories(fileDir);
		else
			return true;
	}
	catch (...){}
	return false;
#else
	std::string sParentDir = GetParentDirectoryFromPath(filename);

	int i, len;
	char buf[512];
	int start = 0;

	/* try to create all the intermediate directories */
	len = (int)(sParentDir.size());
	buf[0] = sParentDir[0];
	for (i = 1; i<len; i++){
		if ((sParentDir[i] == '/' || sParentDir[i] == '\\') && (i - start > 0)){
			/* only mkdir if leaf dir != "." or "/" or ".." */
			if (i - start > 2 || (i - start == 1 && buf[start] != '.' && (buf[start] != '/' && buf[start] != '\\'))
				|| (i - start == 2 && buf[start] != '.' && buf[start + 1] != '.')){
				buf[i] = '\0';
#ifdef DEFAULT_FILE_ENCODING
#ifdef WIN32
				std::string dirpath = std::string("\\\\?\\") + GetWritableFullPathForFilename(buf);
				std::replace(dirpath.begin(), dirpath.end(), '/', '\\');

				LPCWSTR buf16 = StringHelper::MultiByteToWideChar(dirpath.c_str(), DEFAULT_FILE_ENCODING);
				if (dirpath[dirpath.size() - 1] != ':' && !fs::exists(buf16)) {
#else
				if (!fs::exists(buf)) {
#endif
					BOOL ret = ::CreateDirectoryW(buf16, 0);
					if (!ret) {
						int err = errno;
						if (err != EEXIST) {
							return false;
						}
					}
				}
#else
				int ret = _mkdir(buf);
				if (ret != 0) {
					int err = errno;
					if (err != EEXIST) {
						return false;
					}
				}
#endif
			}
			start = i + 1;
		}
		buf[i] = sParentDir[i];
	}
	return true;
#endif
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
#ifdef USE_COCOS_FILE_API
	OUTPUT_LOG("SaveBufferToFile not implemented in mobile version\n");
	return false;
#elif defined(USE_BOOST_FILE_API)
	try
	{
		if(MakeDirectoryFromFilePath(filename.c_str())==false)
			return false;
#ifdef WIN32
		LPCWSTR filename16 = StringHelper::MultiByteToWideChar(filename.c_str(), DEFAULT_FILE_ENCODING);
#else
		auto filename16 = filename;
#endif
		fs::path filePath(filename16);
		if(fs::exists(filePath) && (bReplace == false))
			return false;

		fs::ofstream file( filePath);
		if(!file)
		{
			file.write(buffer, nBufSize);
		}
	}
	catch (...){}
	return false;
#else
	if (MakeDirectoryFromFilePath(filename.c_str()) == false)
		return false;
	if (FileExist(filename.c_str()) && (bReplace == false))
		return false;
	HANDLE hFile;
	// proccess long file name to append "\\?\"
	std::string filename_ = std::string("\\\\?\\") + GetWritableFullPathForFilename(filename);
	std::replace(filename_.begin(), filename_.end(), '/', '\\');
#ifdef DEFAULT_FILE_ENCODING
	LPCWSTR filename16 = StringHelper::MultiByteToWideChar(filename_.c_str(), DEFAULT_FILE_ENCODING);
	hFile = ::CreateFileW(filename16, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
#else
	hFile = ::CreateFile(filename_.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
#endif
 
	if (hFile != INVALID_HANDLE_VALUE)
	{
		DWORD w;
		::WriteFile(hFile, buffer, nBufSize, &w, NULL);
		::CloseHandle(hFile);
		return true;
	}
	else
		return false;
#endif
}

bool ParaEngine::CFileUtils::DeleteFile(const char* filename)
{
#ifdef USE_COCOS_FILE_API
	std::string sFilePath = GetWritableFullPathForFilename(filename);
	try
	{
		return fs::remove(sFilePath);
	}
	catch (...)
	{
		return false;
	}
#elif defined(USE_BOOST_FILE_API)
	try
	{
#ifdef WIN32
		LPCWSTR filename16 = StringHelper::MultiByteToWideChar(filename, DEFAULT_FILE_ENCODING);
#else
		auto filename16 = filename;
#endif
		return fs::remove(filename16);
	}
	catch (...)
	{
		return false;
	}
#else
	std::string filename_ = std::string("\\\\?\\") + GetWritableFullPathForFilename(filename);
	std::replace(filename_.begin(), filename_.end(), '/', '\\');

#ifdef DEFAULT_FILE_ENCODING
	LPCWSTR filename16 = StringHelper::MultiByteToWideChar(filename_.c_str(), DEFAULT_FILE_ENCODING);
	return (::DeleteFileW(filename16) != 0);
#else
	return (::DeleteFile(filename_.c_str()) != 0);
#endif
#endif
}

int ParaEngine::CFileUtils::DeleteDirectory(const char* filename)
{
#ifdef USE_COCOS_FILE_API
	std::string sFilePath = GetWritableFullPathForFilename(filename);
	try
	{
		return fs::remove_all(sFilePath);
	}
	catch (...)
	{
		return 0;
	}
#elif defined(USE_BOOST_FILE_API)
	try
	{
#ifdef WIN32
		LPCWSTR filename16 = StringHelper::MultiByteToWideChar(filename, DEFAULT_FILE_ENCODING);
#else
		auto filename16 = filename;
#endif
		return (int)fs::remove_all(filename16);
	}
	catch (...)
	{
		return 0;
	}
#else
	
	std::string filename_ = std::string("\\\\?\\") + GetWritableFullPathForFilename(filename);
	std::replace(filename_.begin(), filename_.end(), '/', '\\');

#ifdef DEFAULT_FILE_ENCODING
	LPCWSTR filename16 = StringHelper::MultiByteToWideChar(filename_.c_str(), DEFAULT_FILE_ENCODING);
	return ::DeleteFileW(filename16);
#else
	return ::DeleteFile(filename_.c_str());
#endif
#endif
}

int ParaEngine::CFileUtils::DeleteFiles(const std::string& sFilePattern, bool bSecureFolderOnly /*= true*/, int nSubFolderCount)
{
#if defined(USE_COCOS_FILE_API) || defined(USE_BOOST_FILE_API)
	// search in writable disk files
	// Note: bSecureFolderOnly is ignore, since it is guaranteed by the OS's writable path.
	int nCount = 0;
	std::string sRootPath = GetParentDirectoryFromPath(sFilePattern);
	std::string sPattern = GetFileName(sFilePattern);
	sRootPath = GetWritableFullPathForFilename(sRootPath);
#ifdef WIN32
	std::string sFullPattern = GetWritableFullPathForFilename(sFilePattern);
	LPCWSTR sFullPattern16 = StringHelper::MultiByteToWideChar(sFullPattern.c_str(), DEFAULT_FILE_ENCODING);
#else
	std::string sFullPattern16 = GetWritableFullPathForFilename(sFilePattern);
#endif
	if (fs::is_directory(sFullPattern16))
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
	if (nCount > 0){
#if defined(USE_COCOS_FILE_API)
		cocos2d::FileUtils::getInstance()->purgeCachedEntries();
#endif
	}
	return nCount;
#else

#ifdef DEFAULT_FILE_ENCODING
	WIN32_FIND_DATAW FindFileData;
	HANDLE hFind = INVALID_HANDLE_VALUE;

	wstring tmpDir_ = StringHelper::MultiByteToWideChar(GetParentDirectoryFromPath(sFilePattern, 0).c_str(), DEFAULT_FILE_ENCODING);
	if (bSecureFolderOnly)
	{
		//TODO: search if the directory is outside the application directory. If so, we should now allow user to delete file there.
		if (!CParaFile::IsWritablePath(sFilePattern, false))
		{
			// only relative path is allowed.
			OUTPUT_LOG("security alert: some one is telling the engine to delete a file %s which is not allowed\r\n", sFilePattern.c_str());
			return 0;
		}
	}

	string DirSpec = sFilePattern;
	DWORD dwError;
	hFind = FindFirstFileW(StringHelper::MultiByteToWideChar(DirSpec.c_str(), DEFAULT_FILE_ENCODING), &FindFileData);

	if (sFilePattern.size() > 0 && sFilePattern[sFilePattern.size() - 1] == '/')
	{
		std::string filename;
		CParaFile::ToCanonicalFilePath(filename, sFilePattern, true);
		std::wstring filename16 = StringHelper::MultiByteToWideChar(filename.c_str(), DEFAULT_FILE_ENCODING);
		filename16.push_back(L'\0');
		filename16.push_back(L'\0');
		SHFILEOPSTRUCTW file_op = {
			NULL,
			FO_DELETE,
			filename16.c_str(),
			NULL,
			FOF_NOCONFIRMATION |
			FOF_NOERRORUI |
			FOF_SILENT,
			false,
			0,
			L"" };
		auto ret = SHFileOperationW(&file_op);

		return (ret == S_OK) ? 1 : 0;
	}

	int nFileCount = 0;
	if (hFind == INVALID_HANDLE_VALUE)
	{
		return nFileCount;
	}
	else
	{
		// we will first read file to a cache location, and then delete in a batch. This may prevent file error on old FAT32 system. 
		// NTFS works file to delete file during the FindNextFile() loop.
		std::vector<std::wstring> files;
		do
		{
			files.push_back(FindFileData.cFileName);
			++nFileCount;
		} while (FindNextFileW(hFind, &FindFileData) != 0);

		dwError = GetLastError();
		FindClose(hFind);

		std::vector<std::wstring>::iterator itCur, itEnd = files.end();
		for (itCur = files.begin(); itCur != itEnd; ++itCur)
		{
			std::wstring filename(tmpDir_ + (*itCur));
			if (DeleteFileW(filename.c_str()))
			{
			}
			else
			{
				OUTPUT_LOG("warning, unable to delete file %s\n", filename.c_str());
			}
		}

		if (dwError == ERROR_NO_MORE_FILES)
		{
			return nFileCount;
		}
	}
	return nFileCount;
#else
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = INVALID_HANDLE_VALUE;

	string tmpDir_ = GetParentDirectoryFromPath(sFilePattern, 0);
	if (bSecureFolderOnly)
	{
		//TODO: search if the directory is outside the application directory. If so, we should now allow user to delete file there.
		if (!CParaFile::IsWritablePath(sFilePattern, false))
		{
			// only relative path is allowed.
			OUTPUT_LOG("security alert: some one is telling the engine to delete a file %s which is not allowed\r\n", sFilePattern.c_str());
			return 0;
		}
	}

	string DirSpec = sFilePattern;
	DWORD dwError;
	hFind = FindFirstFile(DirSpec.c_str(), &FindFileData);

	if (sFilePattern.size() > 0 && sFilePattern[sFilePattern.size() - 1] == '/')
	{
		std::string filename;
		CParaFile::ToCanonicalFilePath(filename, sFilePattern, true);
		filename.push_back('\0');
		filename.push_back('\0');
		SHFILEOPSTRUCT file_op = {
			NULL,
			FO_DELETE,
			filename.c_str(),
			NULL,
			FOF_NOCONFIRMATION |
			FOF_NOERRORUI |
			FOF_SILENT,
			false,
			0,
			"" };
		return (SHFileOperation(&file_op) == S_OK) ? 1 : 0;
	}

	int nFileCount = 0;
	if (hFind == INVALID_HANDLE_VALUE)
	{
		return nFileCount;
	}
	else
	{
		// we will first read file to a cache location, and then delete in a batch. This may prevent file error on old FAT32 system. 
		// NTFS works file to delete file during the FindNextFile() loop.
		std::vector<std::string> files;
		do
		{
			files.push_back(FindFileData.cFileName);
			++nFileCount;
		} while (FindNextFile(hFind, &FindFileData) != 0);

		dwError = GetLastError();
		FindClose(hFind);

		std::vector<std::string>::iterator itCur, itEnd = files.end();
		for (itCur = files.begin(); itCur != itEnd; ++itCur)
		{
			std::string filename(tmpDir_ + (*itCur));
			if (DeleteFile(filename.c_str()))
			{
			}
			else
			{
				OUTPUT_LOG("warning, unable to delete file %s\n", filename.c_str());
			}
		}

		if (dwError == ERROR_NO_MORE_FILES)
		{
			return nFileCount;
		}
	}
	return nFileCount;
#endif
	
#endif
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
		if (STAT_FUNC(filename, &info)){
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
			fileInfo.m_sFullpath = sAbsFilePath;
			return true;
		}
	}
	if (_GetFileInfo_(filename, fileInfo))
	{
		fileInfo.m_sFullpath = filename;
		return true;
	}
	return false;
}

ParaEngine::FileData ParaEngine::CFileUtils::GetDataFromFile(const char* filename)
{
#ifdef USE_COCOS_FILE_API
	FileLock_type lock_(s_cocos_file_io_mutex);
	cocos2d::Data buf = cocos2d::FileUtils::getInstance()->getDataFromFile(filename);
	lock_.unlock();
	FileData data;
	if (!buf.isNull())
	{
		size_t nSize = (size_t)buf.getSize();
		char* pBuffer = new char[nSize + 1];
		memcpy(pBuffer, buf.getBytes(), nSize);
		pBuffer[nSize] = '\0'; // always add an ending '\0' for ease for text parsing. 
		data.SetOwnBuffer(pBuffer, (DWORD)nSize);
	}
	return data;
#elif defined(USE_BOOST_FILE_API)
	FileData data;
	try
	{
		fs::ifstream file;
#ifdef WIN32
		if (!CParaFile::GetDevDirectory().empty() && !IsAbsolutePath(filename))
		{
			file.open(StringHelper::MultiByteToWideChar((CParaFile::GetDevDirectory() + filename).c_str(), DEFAULT_FILE_ENCODING), ios::in | ios::binary | ios::ate);
			if (!file.is_open())
			{
				file.open(StringHelper::MultiByteToWideChar(filename, DEFAULT_FILE_ENCODING), ios::in | ios::binary | ios::ate);
			}
	}
		else
		{
			file.open(StringHelper::MultiByteToWideChar(filename, DEFAULT_FILE_ENCODING), ios::in | ios::binary | ios::ate);
		}
#else
		if (!CParaFile::GetDevDirectory().empty() && !IsAbsolutePath(filename))
		{
			file.open(CParaFile::GetDevDirectory() + filename, ios::in | ios::binary | ios::ate);
			if (!file.is_open())
			{
				file.open(filename, ios::in | ios::binary | ios::ate);
			}
		}
		else
		{
			file.open(filename, ios::in | ios::binary | ios::ate);
		}
#endif
		

		if (file.is_open())
		{
			size_t nSize = (size_t)file.tellg();
			char* pBuffer = new char[nSize + 1];
			pBuffer[nSize] = '\0'; // always add an ending '\0' for ease for text parsing. 
			file.seekg(0, ios::beg);
			file.read(pBuffer, nSize);
			file.close();
			data.SetOwnBuffer(pBuffer, nSize);
		}
	}
	catch (...)
	{
	}
	return data;
#else
	HANDLE hFile = INVALID_HANDLE_VALUE;

	if (!CParaFile::GetDevDirectory().empty())
	{
		std::string sAbsFilePath = std::string("\\\\?\\") + GetWritableFullPathForFilename(CParaFile::GetDevDirectory() + filename);
		std::replace(sAbsFilePath.begin(), sAbsFilePath.end(), '/', '\\');
#ifdef DEFAULT_FILE_ENCODING
		LPCWSTR sAbsFilePath16 = StringHelper::MultiByteToWideChar(sAbsFilePath.c_str(), DEFAULT_FILE_ENCODING);
		hFile = ::CreateFileW(sAbsFilePath16, GENERIC_READ, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
#else
		hFile = ::CreateFile(sAbsFilePath.c_str(), GENERIC_READ, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
#endif
	}
	if (hFile == INVALID_HANDLE_VALUE)
	{
		// proccess long file name to append "\\?\"
		std::string filename_ = std::string("\\\\?\\") + GetWritableFullPathForFilename(filename);
		std::replace(filename_.begin(), filename_.end(), '/', '\\');
#ifdef DEFAULT_FILE_ENCODING
		LPCWSTR filename16 = StringHelper::MultiByteToWideChar(filename_.c_str(), DEFAULT_FILE_ENCODING);
		hFile = ::CreateFileW(filename16, GENERIC_READ, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
#else
		hFile = ::CreateFile(filename_.c_str(), GENERIC_READ, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
#endif
	}

	FileData data;
	if (hFile != INVALID_HANDLE_VALUE)
	{
		/// load from disk
		size_t nSize = (size_t)::GetFileSize(hFile, NULL);
		DWORD nBytesRead = 0;
		char* pBuffer = new char[nSize + 1];
		pBuffer[nSize] = '\0'; // always add an ending '\0' for ease for text parsing. 
		::ReadFile(hFile, pBuffer, (DWORD)nSize, &nBytesRead, NULL);
		::CloseHandle(hFile);
		data.SetOwnBuffer(pBuffer, nSize);
	}
	return data;
#endif
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

#ifdef USE_COCOS_FILE_API
	
#elif defined(WIN32) && defined(PARAENGINE_CLIENT)
	if (!filename.empty() && filename[0] == ':')
	{
		HINSTANCE hInstance = CParaEngineApp::GetInstance()->GetModuleHandle();
		HRSRC hSrc;
#ifdef DEFAULT_FILE_ENCODING
		LPCWSTR filename16 = StringHelper::MultiByteToWideChar(filename.c_str(), DEFAULT_FILE_ENCODING);
		hSrc = ::FindResourceW(hInstance, filename16 + 1, L"textfile");
#else
		hSrc = ::FindResource(hInstance, filename.c_str() + 1, "textfile");
#endif
		if (hSrc != NULL)
		{
			HGLOBAL hHandle = ::LoadResource(hInstance, hSrc);
			if (hHandle != NULL)
			{
				size_t nSize = (size_t)::SizeofResource(hInstance, hSrc);
				char* buffer = (char*)::LockResource(hHandle);
				data.SetOwnBuffer(buffer, nSize);
			}
		}
	}
#endif
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
#ifdef USE_COCOS_FILE_API
	FileLock_type lock_(s_cocos_file_io_mutex);
	return cocos2d::FileUtils::getInstance()->fullPathForFilename(filename);
#elif defined(USE_BOOST_FILE_API)
#ifdef WIN32
	LPCWSTR filename16 = StringHelper::MultiByteToWideChar(filename.c_str(), DEFAULT_FILE_ENCODING);
#else
	auto filename16 = filename;
#endif
	fs::path filepath(filename16);
	fs::path abs_path = fs::absolute(filepath);
	return abs_path.string();
#else
	return filename;
#endif
}

bool ParaEngine::CFileUtils::FileExistRaw(const char* filename)
{
#ifdef USE_COCOS_FILE_API
	std::string sFile(filename);
	FileLock_type lock_(s_cocos_file_io_mutex);
	// TODO: Cocos API FileUtils::getInstance()->isDirectoryExist(sFile) could not detect directory correctly
	return !sFile.empty() && (cocos2d::FileUtils::getInstance()->isFileExist(sFile) || cocos2d::FileUtils::getInstance()->isDirectoryExist(sFile));
#elif defined USE_BOOST_FILE_API
#ifdef WIN32
	LPCWSTR filename16 = StringHelper::MultiByteToWideChar(filename, DEFAULT_FILE_ENCODING);
#else
	auto filename16 = filename;
#endif
	return fs::exists(filename16);
	/*bool bFound = fs::exists(filename);
	OUTPUT_LOG("%s check raw %s\n", filename, bFound ? "true" : "false");
	return bFound;*/
#else
	
	bool blnReturn;
	int intStat;

	// Attempt to get the file attributes
	
#ifdef WIN32
#ifdef DEFAULT_FILE_ENCODING
	LPCWSTR filename16 = StringHelper::MultiByteToWideChar(filename, DEFAULT_FILE_ENCODING);
	struct _stat64i32 wstFileInfo;
	intStat = _wstat(filename16, &wstFileInfo);
#else
	struct stat stFileInfo;
	intStat = stat(filename, &stFileInfo);
#endif
#else
	struct stat stFileInfo;
	intStat = stat(filename, &stFileInfo);
#endif
	if (intStat == 0) {
		// We were able to get the file attributes
		// so the file obviously exists.
		blnReturn = true;
	}
	else {
		// We were not able to get the file attributes.
		// This may mean that we don't have permission to
		// access the folder which contains this file. If you
		// need to do that level of checking, lookup the
		// return values of stat which will give you
		// more details on why stat failed.
		blnReturn = false;
	}

	return(blnReturn);
#endif
}

int ParaEngine::CFileUtils::GetFileSize(const char* sFilePath)
{
#ifdef USE_COCOS_FILE_API
	return 0;
#elif defined USE_BOOST_FILE_API
	int nSize = 0;
	try {
#ifdef WIN32
		LPCWSTR sFilePath16 = StringHelper::MultiByteToWideChar(sFilePath, DEFAULT_FILE_ENCODING);
#else
		auto sFilePath16 = sFilePath;
#endif
		nSize = (int)fs::file_size(sFilePath16);
	}
	catch (...)	{}
	return nSize;
#else
	DWORD dwFileSize = 0;
	HANDLE hFile;
	std::string filename_ = std::string("\\\\?\\") + GetWritableFullPathForFilename(sFilePath);
	std::replace(filename_.begin(), filename_.end(), '/', '\\');
#ifdef DEFAULT_FILE_ENCODING
	LPCWSTR sFilePath16 = StringHelper::MultiByteToWideChar(filename_.c_str(), DEFAULT_FILE_ENCODING);
	hFile = ::CreateFileW(sFilePath16, FILE_READ_DATA/*GENERIC_READ*/, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
#else
	hFile = ::CreateFile(filename_.c_str(), FILE_READ_DATA/*GENERIC_READ*/, FILE_SHARE_DELETE | FILE_SHARE_READ | FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
#endif
	
	if (hFile != INVALID_HANDLE_VALUE)
	{
		dwFileSize = ::GetFileSize(hFile, NULL);
		if (dwFileSize == INVALID_FILE_SIZE)
			dwFileSize = 0;
		::CloseHandle(hFile);
	}
	return dwFileSize;
#endif
}

ParaEngine::FileHandle ParaEngine::CFileUtils::OpenFile(const char* filename, bool bRead /*= false*/, bool bWrite/*=true*/)
{
#ifdef USE_COCOS_FILE_API
	std::string sFilePath;
	if (bWrite && !IsAbsolutePath(filename))
	{
		sFilePath = GetWritablePath() + filename;
		sFilePath = GetFullPathForFilename(sFilePath);
	}
	else{
		sFilePath = GetFullPathForFilename(filename);
	}
	FILE* pFile = fopen(sFilePath.c_str(), bRead ? (bWrite ? "w+b" : "rb") : "wb");
	FileHandle fileHandle;
	fileHandle.m_pFile = pFile;
	return fileHandle;
#elif (defined USE_BOOST_FILE_API && !defined(WIN32))
	std::string sFilePath = GetFullPathForFilename(filename);
	FILE* pFile = fopen(sFilePath.c_str(), bRead ? (bWrite ? "w+b" : "rb") : "wb");
	FileHandle fileHandle;
	fileHandle.m_pFile = pFile;
	return fileHandle;
#else
	HANDLE hFile;

	std::string filename_ = std::string("\\\\?\\") + GetWritableFullPathForFilename(filename);
	std::replace(filename_.begin(), filename_.end(), '/', '\\');
#ifdef DEFAULT_FILE_ENCODING
	LPCWSTR filename16 = StringHelper::MultiByteToWideChar(filename_.c_str(), DEFAULT_FILE_ENCODING);
	hFile = ::CreateFileW(filename16, (bRead ? GENERIC_READ : 0) | (bWrite ? GENERIC_WRITE : 0), (bRead ? FILE_SHARE_READ : 0) | (bWrite ? FILE_SHARE_WRITE : 0),
		NULL, bWrite ? OPEN_ALWAYS : 0, NULL, NULL);
#else
	hFile = ::CreateFile(filename_.c_str(), (bRead ? GENERIC_READ : 0) | (bWrite ? GENERIC_WRITE : 0), (bRead ? FILE_SHARE_READ : 0) | (bWrite ? FILE_SHARE_WRITE : 0),
		NULL, bWrite ? OPEN_ALWAYS : 0, NULL, NULL);
#endif
	
	if (hFile != INVALID_HANDLE_VALUE)
	{
		return FileHandle(hFile);
	}
	return FileHandle();
#endif
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
#ifdef USE_COCOS_FILE_API
		fseek(fileHandle.m_pFile, lDistanceToMove, dwMoveMethod);
#elif (defined USE_BOOST_FILE_API && !defined(WIN32))
		fseek(fileHandle.m_pFile, lDistanceToMove, dwMoveMethod);
#else
		::SetFilePointer(fileHandle.m_handle, lDistanceToMove, 0, dwMoveMethod);
#endif
		return true;
	}
	return false;
}

int ParaEngine::CFileUtils::GetFilePosition(FileHandle& fileHandle)
{
	if (fileHandle.IsValid())
	{
#ifdef USE_COCOS_FILE_API
		return ftell(fileHandle.m_pFile);
#elif (defined USE_BOOST_FILE_API && !defined(WIN32))
		return ftell(fileHandle.m_pFile);
#else
		int nPos = ::SetFilePointer(fileHandle.m_handle, 0, NULL, FILE_CURRENT);
		return nPos != INVALID_SET_FILE_POINTER ? nPos : 0;
#endif
	}
	return 0;
}

bool ParaEngine::CFileUtils::SetEndOfFile(FileHandle& fileHandle)
{
	if (fileHandle.IsValid())
	{
#ifdef USE_COCOS_FILE_API
		fseek(fileHandle.m_pFile, 0, SEEK_END);
		return true;
#elif (defined USE_BOOST_FILE_API && !defined(WIN32))
		fseek(fileHandle.m_pFile, 0, SEEK_END);
		return true;
#else
		return ::SetEndOfFile(fileHandle.m_handle) == TRUE;
#endif
	}
	return false;
}

int ParaEngine::CFileUtils::WriteBytes(FileHandle& fileHandle, const void* src, int bytes)
{
	if (fileHandle.IsValid())
	{
#ifdef USE_COCOS_FILE_API
		return (int)fwrite(src, 1, bytes, fileHandle.m_pFile);
#elif (defined USE_BOOST_FILE_API && !defined(WIN32))
		return (int)fwrite(src, 1, bytes, fileHandle.m_pFile);
#else
		DWORD bytesWritten;
		::WriteFile(fileHandle.m_handle, src, bytes, &bytesWritten, NULL);
		return bytesWritten;
#endif
	}
	return 0;
}

int ParaEngine::CFileUtils::ReadBytes(FileHandle& fileHandle, void* dest, int bytes)
{
	if (fileHandle.IsValid())
	{
#ifdef USE_COCOS_FILE_API
		return (int)fread(dest, 1, bytes, fileHandle.m_pFile);
#elif (defined USE_BOOST_FILE_API && !defined(WIN32))
		return (int)fread(dest, 1, bytes, fileHandle.m_pFile);
#else
		DWORD bytesRead = 0;
		::ReadFile(fileHandle.m_handle, dest, bytes, &bytesRead, NULL);
		return bytesRead;
#endif
	}
	return 0;
}

void ParaEngine::CFileUtils::CloseFile(FileHandle& fileHandle)
{
	if (fileHandle.IsValid())
	{
#ifdef USE_COCOS_FILE_API
		fclose(fileHandle.m_pFile);
		fileHandle.m_pFile = nullptr;
#elif (defined USE_BOOST_FILE_API && !defined(WIN32))
		fclose(fileHandle.m_pFile);
		fileHandle.m_pFile = nullptr;
#else
		::CloseHandle(fileHandle.m_handle);
		fileHandle.m_handle = INVALID_HANDLE_VALUE;
#endif
	}
}

std::string ParaEngine::CFileUtils::GetInitialDirectory()
{
#ifdef USE_COCOS_FILE_API
	FileLock_type lock_(s_cocos_file_io_mutex);
#ifdef WIN32
	std::string sRootDir = cocos2d::FileUtils::getInstance()->getSearchPaths()[0];
	if(sRootDir.size()>0 && (sRootDir[sRootDir.size()-1] != '/' && sRootDir[sRootDir.size()-1] != '\\'))
	{
		sRootDir += "/";
	}
#else
	// Provide empty initial directory here
	std::string sRootDir = "";
#endif
	return sRootDir;
#elif defined(USE_BOOST_FILE_API)
	fs::path sWorkingDir = fs::initial_path();
#ifndef WIN32
	sWorkingDir = fs::path(sWorkingDir.string());
#endif
	std::string sRootDir;
	CParaFile::ToCanonicalFilePath(sRootDir, sWorkingDir.string(), false);
	if(sRootDir.size()>0 && (sRootDir[sRootDir.size()-1] != '/' && sRootDir[sRootDir.size()-1] != '\\'))
	{
		sRootDir += "/";
	}
	return sRootDir;
#else
	char buf[1024 + 1];
	int nCount;
#ifdef DEFAULT_FILE_ENCODING
	wchar_t buf16[1024 + 1];
	nCount = ::GetCurrentDirectoryW(1024, buf16);
	auto _buf = StringHelper::WideCharToMultiByte(buf16, DEFAULT_FILE_ENCODING);
	strcpy(buf,_buf);
#else
	nCount = ::GetCurrentDirectory(1024, buf);
#endif
	if (nCount > 0) {
		string bufstr = buf;
		const char backC = bufstr.back();
		if (backC != '/' && backC != '\\') {
			bufstr.push_back('/');
		}
		string sAppRootPath;
		CParaFile::ToCanonicalFilePath(sAppRootPath, bufstr, false);
		return sAppRootPath;
}
	return "";
#endif
}

const std::string& ParaEngine::CFileUtils::GetExternalStoragePath()
{
	return GetWritablePath();
}

const std::string& ParaEngine::CFileUtils::GetWritablePath()
{
	if (s_writepath.empty())
	{
#ifdef USE_COCOS_FILE_API
		s_writepath = cocos2d::FileUtils::getInstance()->getWritablePath();
#else
		s_writepath = ParaEngine::CParaFile::GetCurDirectory(ParaEngine::CParaFile::APP_ROOT_DIR);
#endif
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

#define CHECK_BIT(x,y) (((x)&(y))>0)

#ifndef Int32x32To64
#define Int32x32To64(a, b)  (((int64_t)((int32_t)(a))) * ((int64_t)((int32_t)(b))))
#endif

void TimetToFileTime(const std::time_t& t, FILETIME* pft)
{
	int64_t ll = Int32x32To64(t, 10000000) + 116444736000000000;
	pft->dwLowDateTime = (DWORD)ll;
	pft->dwHighDateTime = ll >> 32;
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
					TimetToFileTime(lastWriteTime, &fileLastWriteTime);

					
#ifdef DEFAULT_FILE_ENCODING
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
					TimetToFileTime(lastWriteTime, &fileLastWriteTime);
#ifdef DEFAULT_FILE_ENCODING
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
					TimetToFileTime(lastWriteTime, &fileLastWriteTime);
#ifdef DEFAULT_FILE_ENCODING
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
#ifdef  WIN32
#ifdef DEFAULT_FILE_ENCODING
	std::wstring path;
	std::string path8 = GetWritableFullPathForFilename(sRootPath);
	path = StringHelper::MultiByteToWideChar(path8.c_str(), DEFAULT_FILE_ENCODING);
#else
	std::string path = GetWritableFullPathForFilename(sRootPath);
#endif
#else
	std::string path = GetWritableFullPathForFilename(sRootPath);
#endif //  DEFAULT_FILE_ENCODING
	
	fs::path rootPath(path);
	if (!fs::exists(rootPath) || !fs::is_directory(rootPath)) {
		OUTPUT_LOG("directory does not exist %s \n", rootPath.string().c_str());
		return;
	}
#ifdef DEFAULT_FILE_ENCODING
	std::string root = StringHelper::WideCharToMultiByte(rootPath.wstring().c_str(), DEFAULT_FILE_ENCODING);
	result.SetRootPath(root);
#else
	result.SetRootPath(rootPath.string());
#endif
	FindFiles_Recursive(result, rootPath, sFilePattern, nSubLevel);

#ifdef OLD_FILE_SEARCH
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = INVALID_HANDLE_VALUE;
	DWORD dwError;

	string DirSpec = sRootPath + sFilePattern;
	CParaFile::ToCanonicalFilePath(DirSpec, DirSpec);
	hFind = FindFirstFile(DirSpec.c_str(), &FindFileData);

	if (hFind != INVALID_HANDLE_VALUE)
	{
		if (FindFileData.cFileName[0] != '.')
		{
			string sFile = sRootPath + FindFileData.cFileName;
			CParaFile::ToCanonicalFilePath(sFile, sFile, false);
			if (!result.AddResult(sFile, FindFileData.nFileSizeLow, FindFileData.dwFileAttributes, &(FindFileData.ftLastWriteTime), &(FindFileData.ftCreationTime), &(FindFileData.ftLastAccessTime)))
				return;
			/*if(CHECK_BIT(FindFileData.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY) && nSubLevel>0)
			{
			string sNewRoot = sRootPath+FindFileData.cFileName+"\\";
			sNewRoot = CParaFile::ToCanonicalFilePath(sNewRoot);
			FindDiskFiles(result, sNewRoot, sFilePattern, nSubLevel-1);
			}*/
		}
		while (FindNextFile(hFind, &FindFileData) != 0)
		{
			if (FindFileData.cFileName[0] == '.')
				continue;
			string sFile = sRootPath + FindFileData.cFileName;
			CParaFile::ToCanonicalFilePath(sFile, sFile, false);
			if (!result.AddResult(sFile, FindFileData.nFileSizeLow, FindFileData.dwFileAttributes, &(FindFileData.ftLastWriteTime), &(FindFileData.ftCreationTime), &(FindFileData.ftLastAccessTime)))
				return;
			/*if(CHECK_BIT(FindFileData.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY) && nSubLevel>0)
			{
			string sNewRoot = sRootPath+FindFileData.cFileName+"\\";
			sNewRoot = CParaFile::ToCanonicalFilePath(sNewRoot);
			FindDiskFiles(result, sNewRoot, sFilePattern, nSubLevel-1);
			}*/
		}
		dwError = GetLastError();
		FindClose(hFind);
		if (dwError != ERROR_NO_MORE_FILES)
		{
			OUTPUT_LOG("FindNextFile error. Error is %u\n", dwError);
		}
	}

	// search all sub directories, without adding files
	int nLen = (int)sFilePattern.size();
	if (nSubLevel > 0/* && sFilePattern[nLen-1] != '.'*/)
	{
		DirSpec = sRootPath + "*.*";
		CParaFile::ToCanonicalFilePath(DirSpec, DirSpec);
		hFind = FindFirstFile(DirSpec.c_str(), &FindFileData);

		if (hFind != INVALID_HANDLE_VALUE)
		{
			if (FindFileData.cFileName[0] != '.')
			{
				if (CHECK_BIT(FindFileData.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY))
				{
					string sNewRoot = sRootPath + FindFileData.cFileName + "\\";
					CParaFile::ToCanonicalFilePath(sNewRoot, sNewRoot);
					FindDiskFiles(result, sNewRoot, sFilePattern, nSubLevel - 1);
				}
			}
			while (FindNextFile(hFind, &FindFileData) != 0)
			{
				if (FindFileData.cFileName[0] == '.')
					continue;
				if (CHECK_BIT(FindFileData.dwFileAttributes, FILE_ATTRIBUTE_DIRECTORY))
				{
					string sNewRoot = sRootPath + FindFileData.cFileName + "\\";
					CParaFile::ToCanonicalFilePath(sNewRoot, sNewRoot);
					FindDiskFiles(result, sNewRoot, sFilePattern, nSubLevel - 1);
				}
			}
			dwError = GetLastError();
			FindClose(hFind);
			if (dwError != ERROR_NO_MORE_FILES)
			{
				OUTPUT_LOG("FindNextFile error. Error is %u\n", dwError);
			}
		}
	}
#endif
}


bool ParaEngine::CFileUtils::AddDiskSearchPath(const std::string& sFile, bool nFront /*= false*/)
{
#ifdef USE_COCOS_FILE_API
	FileLock_type lock_(s_cocos_file_io_mutex);
	cocos2d::FileUtils::getInstance()->addSearchPath(sFile, nFront);
	return true;
#else
	OUTPUT_LOG("warn: AddDiskSearchPath :%s not supported. ", sFile.c_str());
	return false;
#endif
}

bool ParaEngine::CFileUtils::WriteLastModifiedTimeToDisk(FileHandle& fileHandle, const std::string& fileName, const time_t& lastModifiedTime)
{
	bool op_result = false;
#if (defined(USE_COCOS_FILE_API) || defined(USE_BOOST_FILE_API)) && !defined(WIN32)
	if (!fileName.empty())
	{
		//platform: mobile
		time_t platform_time;
		standardtime2osfiletime(lastModifiedTime, &platform_time);

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
		fs::path filePath(sFilePath.c_str());
		boost::system::error_code err_code;
		fs::last_write_time(filePath, platform_time, err_code);
		op_result = (err_code.value() == boost::system::errc::success);
	}
#else
	
	if (fileHandle.IsValid())
	{
#if defined(WIN32)
		//platform: win32
		FILETIME platform_time;
		standardtime2osfiletime(lastModifiedTime, &platform_time);
		op_result = (SetFileTime(fileHandle.m_handle, &platform_time, &platform_time, &platform_time) != FALSE);
#else
		//other platform : not implemented for now
#endif	
	}
#endif
	return op_result;
}

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
	#include "ParaEngineApp.h"
#endif
#include "FileUtils.h"
#include "StringHelper.h"
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



#if defined(PARAENGINE_MOBILE)
	#define USE_COCOS_FILE_API
#else
	#if !defined(WIN32) || !defined(PARAENGINE_CLIENT) //  || defined(_DEBUG)
		#define USE_BOOST_FILE_API
	#endif
#endif

#ifdef _DEBUG
// #define USE_BOOST_FILE_API
#endif

#if defined USE_BOOST_FILE_API || defined(USE_COCOS_FILE_API)
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
#endif

#ifdef USE_COCOS_FILE_API
	#include "platform/OpenGLWrapper.h"
	USING_NS_CC;
	/** @Note by LiXizhi: all cocos read file api are not thread safe, hence we need to use a lock. 
	remove this when cocos no longer uses non-thread-safe file cache map. */
	static std::mutex  s_cocos_file_io_mutex;
#ifdef COCOS_IO_THREAD_SAFE_PARAENGINE
	typedef ParaEngine::no_lock<std::mutex> FileLock_type;
#else
	typedef std::unique_lock<std::mutex> FileLock_type;
#endif
#endif

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
		for (int i = 0; (c = relativePath[i]) != '\0' && i < MAX_PATH; i++)
		{
			output[i] = c;
			if ((c == '/') || (c == '\\'))
				nLastSlash = i;
		}
	}
	if (filename != 0)
	{
		strncpy(output + nLastSlash + 1, filename, MAX_PATH);
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
			return true;
		}
	}
	else
	{
#ifdef USE_COCOS_FILE_API
		OUTPUT_LOG("copy file not implemented in mobile version: from %s to %s\n", src, dest);
#elif defined(USE_BOOST_FILE_API)
		try
		{
			fs::path destFile(dest);
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
			fs::copy_file(fs::path(src), destFile);
		}
		catch (...)
		{
			return false;
		}
		return true;
#else
		// try the system version, in case the src file is currently in use, such as the database. 
		return ::CopyFile(src, dest, !bOverride) == TRUE;
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
		fs::copy_file(sSrc, fs::path(dest_path));
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
		fs::path sSrc(src);
		fs::copy_file(sSrc, fs::path(dest));
		return fs::remove(sSrc);
	}
	catch (...)
	{
		return false;
	}
#else
	if (dest != NULL && src != NULL)
	{
		return ::MoveFileEx(src, dest, MOVEFILE_REPLACE_EXISTING | MOVEFILE_WRITE_THROUGH) == TRUE;
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
		//OUTPUT_LOG("MakeDirectoryFromFilePath directory: %s of file:%s\n", fileDir.c_str(), filename);
		if (!fs::is_directory(fileDir))
			return fs::create_directories(fileDir);
		else
			return true;
	}
	catch (...){}
	return false;
#elif defined(USE_BOOST_FILE_API)
	try
	{
		fs::path filePath(filename);
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
				if (_mkdir(buf) != 0){
					int err = errno;
					if (err != EEXIST) {
						return false;
					}
				}
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
		fs::path filePath(filename);
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
	HANDLE hFile = ::CreateFile(filename.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
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
		return fs::remove(filename);
	}
	catch (...)
	{
		return false;
	}
#else
	::DeleteFile(filename);
	return true;
#endif
}

int ParaEngine::CFileUtils::DeleteFiles(const std::string& sFilePattern, bool bSecureFolderOnly /*= true*/, int nSubFolderCount)
{
#if defined(USE_COCOS_FILE_API) || defined(USE_BOOST_FILE_API)
	// search in writable disk files
	// Note: bSecureFolderOnly is ignore, since it is guaranteed by the OS's writable path. 
	std::string sRootPath = GetParentDirectoryFromPath(sFilePattern);
	std::string sPattern = GetFileName(sFilePattern);
	sRootPath = GetWritableFullPathForFilename(sRootPath);
	CSearchResult result;
	result.InitSearch(sRootPath, 0, 10000, 0);
	FindDiskFiles(result, result.GetRootPath(), sPattern, nSubFolderCount);

	int nCount = result.GetNumOfResult();
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
	if (nCount > 0){
#if defined(USE_COCOS_FILE_API)
		cocos2d::FileUtils::getInstance()->purgeCachedEntries();
#endif
	}
	return nCount;
#else
	WIN32_FIND_DATA FindFileData;
	HANDLE hFind = INVALID_HANDLE_VALUE;

	string tmpDir_ = GetParentDirectoryFromPath(sFilePattern, 0);
	if (bSecureFolderOnly)
	{
		//TODO: search if the directory is outside the application directory. If so, we should now allow user to delete file there.
		if (sFilePattern.find_first_of(":") != string::npos)
		{
			// only relative path is allowed.
			OUTPUT_LOG("security alert: some one is telling the engine to delete a file %s which is not allowed\r\n", sFilePattern.c_str());
			return 0;
		}
		else
		{
			// TODO: only allow deletion in some given folder. we will only allow deletion in the specified user directory
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
}

namespace ParaEngine
{
	bool _GetFileInfo_(const char* filename, CParaFileInfo& fileInfo)
	{
		STAT_STRUCT info;
		if (STAT_FUNC(filename, &info)){
			// file not found
			return false;
		}
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
	fs::ifstream file(filename, ios::in | ios::binary | ios::ate);
	if(file.is_open())
	{
		size_t nSize = (size_t)file.tellg();
		char* pBuffer = new char[nSize + 1];
		pBuffer[nSize] = '\0'; // always add an ending '\0' for ease for text parsing. 
		file.seekg (0, ios::beg);
		file.read(pBuffer, nSize);
		file.close();
		data.SetOwnBuffer(pBuffer, nSize);
	}
	return data;
#else
	HANDLE hFile = INVALID_HANDLE_VALUE;
	if (!CParaFile::GetDevDirectory().empty())
	{
		std::string sAbsFilePath = CParaFile::GetDevDirectory() + filename;
		hFile = ::CreateFile(sAbsFilePath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);
	}
	if (hFile == INVALID_HANDLE_VALUE)
		hFile = ::CreateFile(filename, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, NULL, NULL);

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

ParaEngine::FileData ParaEngine::CFileUtils::GetResDataFromFile(const std::string& filename)
{
	FileData data;
#ifdef USE_COCOS_FILE_API
	
#elif defined(WIN32) && defined(PARAENGINE_CLIENT)
	if (!filename.empty() && filename[0] == ':')
	{
		HINSTANCE hInstance = CParaEngineApp::GetInstance()->GetModuleHandle();
		HRSRC hSrc = ::FindResource(hInstance, filename.c_str() + 1, "textfile");
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
	fs::path filepath(filename);
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
	return !sFile.empty() && (cocos2d::FileUtils::getInstance()->isFileExist(sFile) || cocos2d::FileUtils::getInstance()->isDirectoryExist(sFile));
#elif defined USE_BOOST_FILE_API
	return fs::exists(filename);
#else
	struct stat stFileInfo;
	bool blnReturn;
	int intStat;

	// Attempt to get the file attributes
	intStat = stat(filename, &stFileInfo);
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
	return (int)fs::file_size(sFilePath);
#else
	DWORD dwFileSize = 0;
	HANDLE hFile = ::CreateFile(sFilePath, FILE_READ_DATA/*GENERIC_READ*/, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
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
	FILE* pFile = fopen(sFilePath.c_str(), bRead ? "rb" : bWrite ? "wb" : "rw");
	FileHandle fileHandle;
	fileHandle.m_pFile = pFile;
	return fileHandle;
#elif defined USE_BOOST_FILE_API
	std::string sFilePath = GetFullPathForFilename(filename);
	FILE* pFile = fopen(sFilePath.c_str(), bRead ? "rb" : bWrite ? "wb" : "rw");
	FileHandle fileHandle;
	fileHandle.m_pFile = pFile;
	return fileHandle;
#else
	HANDLE hFile = ::CreateFile(filename, (bRead ? GENERIC_READ : 0) | (bWrite ? GENERIC_WRITE : 0), (bRead ? FILE_SHARE_READ : 0) | (bWrite ? FILE_SHARE_WRITE : 0),
		NULL, bWrite ? OPEN_ALWAYS : 0, NULL, NULL);
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
#elif defined USE_BOOST_FILE_API
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
#elif defined USE_BOOST_FILE_API
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
#elif defined USE_BOOST_FILE_API
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
#elif defined USE_BOOST_FILE_API
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
#elif defined USE_BOOST_FILE_API
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
#elif defined USE_BOOST_FILE_API
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
	std::string sRootDir = cocos2d::FileUtils::getInstance()->getSearchPaths()[0];
	if(sRootDir.size()>0 && (sRootDir[sRootDir.size()-1] != '/' && sRootDir[sRootDir.size()-1] != '\\'))
	{
		sRootDir += "/";
	}
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
	if ((nCount = ::GetCurrentDirectory(1024, buf)) > 0)
	{
		if (buf[nCount - 1] != '/' && buf[nCount - 1] != '\\'){
			buf[nCount] = '/';
			buf[++nCount] = '\0';
		}
		string sAppRootPath;
		CParaFile::ToCanonicalFilePath(sAppRootPath, buf, false);
		return sAppRootPath;
	}
	return "";
#endif
}

std::string ParaEngine::CFileUtils::GetWritablePath()
{
	if (!s_writepath.empty())
		return s_writepath;
#ifdef USE_COCOS_FILE_API
	return cocos2d::FileUtils::getInstance()->getWritablePath();
#else
	return ParaEngine::CParaFile::GetCurDirectory(ParaEngine::CParaFile::APP_ROOT_DIR);
#endif
}

void ParaEngine::CFileUtils::SetWritablePath(const std::string& writable_path)
{
	if (!writable_path.empty() && s_writepath != writable_path)
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
}

#define CHECK_BIT(x,y) (((x)&(y))>0)
#if defined(USE_COCOS_FILE_API) || defined(USE_BOOST_FILE_API)
void FindFiles_Recursive(ParaEngine::CSearchResult& result, fs::path rootPath, const std::string& reFilePattern, int nSubLevel)
{
	try
	{
		int nFileCount = 0;

		fs::directory_iterator end_itr; // default construction yields past-the-end
		for (fs::directory_iterator iter(rootPath); iter != end_itr; ++iter)
		{
			if (fs::is_directory(iter->status()))
			{
				// Found directory;
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
					memcpy(&fileLastWriteTime, &lastWriteTime, sizeof(fileLastWriteTime));
					if (!result.AddResult(iter->path().string(), 0, 0, &fileLastWriteTime, 0, 0))
						return;
				}
				else if (ParaEngine::StringHelper::MatchWildcard(iter->path().filename().string(), reFilePattern))
				{
					nFileCount++;
					auto lastWriteTime = fs::last_write_time(iter->path());
					FILETIME fileLastWriteTime;
					memcpy(&fileLastWriteTime, &lastWriteTime, sizeof(fileLastWriteTime));
					if (!result.AddResult(iter->path().string(), 0, 0, &fileLastWriteTime, 0, 0))
						return;
				}
			}
			else if (fs::is_regular_file(iter->status()))
			{
				if(ParaEngine::StringHelper::MatchWildcard(iter->path().filename().string(), reFilePattern))
				{
					// Found file;
					nFileCount++;
					auto lastWriteTime = fs::last_write_time(iter->path());
					FILETIME fileLastWriteTime;
					memcpy(&fileLastWriteTime, &lastWriteTime, sizeof(fileLastWriteTime));
					if (!result.AddResult(iter->path().string(), (DWORD)fs::file_size(iter->path()), 0, &fileLastWriteTime, 0, 0))
						return;
				}
			}
		}
	}
	catch (...){}
}
#endif

void ParaEngine::CFileUtils::FindDiskFiles(CSearchResult& result, const std::string& sRootPath, const std::string& sFilePattern, int nSubLevel)
{
#if defined(USE_COCOS_FILE_API) || defined(USE_BOOST_FILE_API)
	std::string path = GetWritableFullPathForFilename(sRootPath);
	
	fs::path rootPath(path);
	if ( !fs::exists( rootPath) || !fs::is_directory(rootPath) ) {
		OUTPUT_LOG("directory does not exist %s \n", rootPath.string().c_str());
		return;
	}
	result.SetRootPath(rootPath.string());
	FindFiles_Recursive(result, rootPath, sFilePattern, nSubLevel);
	
#elif defined(WIN32)
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
		DirSpec = sRootPath + "*.";
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


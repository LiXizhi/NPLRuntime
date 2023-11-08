//-----------------------------------------------------------------------------
// Class: ParaFile
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2005.4, 2014.8
// Revised: refactored from file manager
// Notes: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include <zlib.h>
#include "UrlLoaders.h"
#include "AsyncLoader.h"
#include "AssetManifest.h"
#include "ZipArchive.h"
#include "ZipWriter.h"
#include "IParaEngineApp.h"
#include "FileUtils.h"
#include "util/StringBuilder.h"
#include "ParaEngineSettings.h"
#include "ParaFile.h"

#if defined (WIN32) && !defined(PARAENGINE_MOBILE) && defined(PARAENGINE_CLIENT)
#include "util/CommonFileDialog.h"
#endif

/**@def define this to build with a hook on CParaFile::Open(), so that file open activity will be logged by CFileLogger*/
#if defined(WIN32) && defined(_DEBUG)
#define FILE_LOGGER_HOOK
#include "FileLogger.h"
#endif

/** @def max number of bytes in a line. */
#ifndef MAX_PARAFILE_LINE_LENGTH
#define MAX_PARAFILE_LINE_LENGTH 1024
#endif


namespace ParaEngine
{
	const char paraengine_app_dir_[][40] = {
		"",			//APP_ROOT_DIR = 0
		"script/",	//APP_SCRIPT_DIR,
		"xmodels/",	//APP_ARCHIVE_DIR, 
		"model/",	//APP_MODEL_DIR,
		"shaders/",	//APP_SHADER_DIR
		"database/",//APP_DATABASE_DIR
		"temp/",	//APP_TEMP_DIR
		"users/",	//APP_USER_DIR,
		"_backup/",	//APP_BACKUP_DIR
		"Screen Shots/",	//APP_SCREENSHOT_DIR
		"plugin/",	//APP_PLUGIN_DIR
		"config/",	//APP_CONFIG_DIR
		"character/",	//APP_CHARACTER_DIR
	};

	/** developer directory path. default to empty. */
	static std::string g_dev_dir;

	/** the global file interface mutex */
	ParaEngine::mutex g_file_mutex;
}

using namespace ParaEngine;

std::string ParaEngine::CParaFile::m_strExtractRootPath = "_EXTRACT\\";
bool ParaEngine::CParaFile::m_bExtractFileToDisk = false;
int ParaEngine::CParaFile::m_nDiskFilePriority = 0;

StringBuilder* ToStringBuilder(CParaFile* pThis)
{
	return (StringBuilder*)(pThis->GetHandlePtr());
}

CParaFile::CParaFile()
	:m_bMemoryFile(false), m_size(0)
{
	m_buffer = 0;
	m_eof = true;
	m_curPos = 0;
	m_bIsOwner = true;
	m_bIsCompressed = false;
	m_uncompressed_size = 0;

	m_lastModifiedTime = 0;
	m_restoreLastModifiedTimeAfterClose = false;

	m_bDiskFileOpened = false;
	synchBits();
}
CParaFile::CParaFile(const CParaFile &f)
{
	// disable copying
}
CParaFile::CParaFile(char* buf, int nBufferSize, bool bCopyBuffer)
	:m_bMemoryFile(false), m_size(0)
{
	m_buffer = buf;
	m_size = nBufferSize;
	m_eof = (buf == NULL) || (nBufferSize == 0);
	m_curPos = 0;
	m_bIsOwner = false;
	m_bDiskFileOpened = false;
	m_bIsCompressed = false;
	m_uncompressed_size = 0;

	if (bCopyBuffer && !m_eof)
	{
		m_buffer = new char[m_size + 1];
		memcpy(m_buffer, buf, m_size);
		m_buffer[m_size] = '\0';

		m_bIsOwner = true;
	}
	synchBits();
}

CParaFile::CParaFile(const char* filename)
	:m_bMemoryFile(false), m_size(0)
{
	m_curPos = 0;
	m_buffer = 0;
	m_bIsOwner = true;
	m_bDiskFileOpened = false;
	m_bIsCompressed = false;
	m_uncompressed_size = 0;
	m_eof = true;
	OpenFile(filename);
	synchBits();
}

CParaFile::CParaFile(const char* filename, const char* relativePath)
	:m_bMemoryFile(false), m_size(0)
{
	m_curPos = 0;
	m_buffer = 0;
	m_eof = true;
	m_bIsOwner = true;
	m_bDiskFileOpened = false;
	m_bIsCompressed = false;
	m_uncompressed_size = 0;

	if ((OpenFile(filename) == false) && (relativePath != NULL))
	{
		OpenFile(filename, true, relativePath);
	}
	synchBits();
}
CParaFile::~CParaFile()
{
	close();
}

unsigned long CParaFile::CRC32(const char* filename)
{
	unsigned long crc = crc32(0L, Z_NULL, 0);
	CParaFile file(filename);
	if (!file.isEof())
	{
		return crc32(crc, (const Bytef*)file.getBuffer(), (uInt)file.getSize());
	}
	return 0;
}

bool CParaFile::MoveFile(const char* src, const char* dest)
{
	return CFileUtils::MoveFile(src, dest);
}


bool CParaFile::CopyFile(const char* src, const char* dest, bool bOverride)
{
	return CFileUtils::CopyFile(src, dest, bOverride);
}

bool CParaFile::MakeDirectoryFromFilePath(const char * filename)
{
	return CFileUtils::MakeDirectoryFromFilePath(filename);
}

bool CParaFile::ExtractFileToDisk(const string& filename, bool bReplace)
{
	return CFileUtils::SaveBufferToFile(filename, bReplace, getBuffer(), (DWORD)getSize());
}

void CParaFile::GiveupBufferOwnership()
{
	m_bIsOwner = false;
}

void CParaFile::TakeBufferOwnership()
{
	m_bIsOwner = true;
}

bool ParaEngine::CParaFile::IsAbsolutePath(const std::string & path)
{
	return CFileUtils::IsAbsolutePath(path);
}

int CParaFile::DeleteTempFile(const string& sFilePattern)
{
	std::string sPattern = "temp/";
	sPattern += sFilePattern;
	return CFileUtils::DeleteFiles(sPattern);
}

int CParaFile::DeleteFile(const std::string& sFilePattern, bool bSecureFolderOnly)
{
	return CFileUtils::DeleteFiles(sFilePattern, bSecureFolderOnly);
}

void CParaFile::ToCanonicalFilePath(char* filename, const char* sfilename, bool bBackSlash/*=true*/)
{
	if (sfilename == NULL) {
		return;
	}
	int i = 0;
	int j = 0;
#ifdef WIN32
	// remove the heading slash
	if ((sfilename[0] == '/') || (sfilename[0] == '\\'))
		i++;
#endif
	if (bBackSlash)
	{
		// replace '/' with '\\'
		for (; sfilename[i] != '\0' && j<(MAX_PARAFILE_LINE_LENGTH - 1); i++, j++)
		{
			if (sfilename[i] == '/')
				filename[j] = '\\';
			else
				filename[j] = sfilename[i];
		}
	}
	else
	{
		// replace '\\' with '/'
		for (; sfilename[i] != '\0' && j<(MAX_PARAFILE_LINE_LENGTH - 1); i++, j++)
		{
			if (sfilename[i] == '\\')
				filename[j] = '/';
			else
				filename[j] = sfilename[i];
		}
	}
	filename[j] = '\0';
}

void CParaFile::ToCanonicalFilePath(string& output, const string& sfilename, bool bBackSlash)
{
	char filename[MAX_PARAFILE_LINE_LENGTH];
	ToCanonicalFilePath(filename, sfilename.c_str(), bBackSlash);
	output = filename;
}

bool CParaFile::DoesFileExist(const char* filename, bool bSearchZipFiles, bool bUseSearchPath)
{
	if (!filename)
		return false;
	uint32 dwWhereToSearch = (bSearchZipFiles ? (FILE_ON_DISK | FILE_ON_ZIP_ARCHIVE) : FILE_ON_DISK);
	dwWhereToSearch = bUseSearchPath ? (dwWhereToSearch | FILE_ON_SEARCH_PATH) : dwWhereToSearch;
	return DoesFileExist2(filename, dwWhereToSearch) != 0;
}

int32 CParaFile::DoesFileExist2(const char* filename, uint32 dwWhereToSearch /*= FILE_ON_DISK*/, std::string* pDiskFilePath /*= NULL*/)
{
	if (!filename)
		return 0;
	int32 dwFoundPlace = FILE_NOT_FOUND;
	if ((dwWhereToSearch & FILE_ON_SEARCH_PATH) > 0)
	{
		dwWhereToSearch &= (~((uint32)FILE_ON_SEARCH_PATH));
		
		std::list<SearchPath>& searchPaths = CFileManager::GetInstance()->GetSearchPaths();

		if (searchPaths.size() == 0 || filename[0] == '/' || filename[0] == '\\')
		{
			if (dwWhereToSearch != 0)
				return DoesFileExist2(filename, dwWhereToSearch, pDiskFilePath);
			return FILE_NOT_FOUND;
		}
		else
		{
			if (dwWhereToSearch!=0)
				dwFoundPlace = DoesFileExist2(filename, dwWhereToSearch, pDiskFilePath);
			if (!dwFoundPlace)
			{
				std::list<SearchPath>::iterator itCurCP, itEndCP = searchPaths.end();
				for (itCurCP = searchPaths.begin(); !dwFoundPlace && itCurCP != itEndCP; ++itCurCP)
				{
					// only search on disk
					dwFoundPlace = DoesFileExist2(((*itCurCP).GetPath() + filename).c_str(), FILE_ON_DISK, pDiskFilePath);
				}
				if (dwFoundPlace)
					dwFoundPlace |= FILE_ON_SEARCH_PATH;
			}
			return dwFoundPlace;
		}
	}

	char sFilename[MAX_PARAFILE_LINE_LENGTH];
	ToCanonicalFilePath(sFilename, filename);

	if ((dwWhereToSearch & FILE_ON_DISK) >0 && (dwWhereToSearch & FILE_ON_ZIP_ARCHIVE) == 0)
	{
		if (CFileUtils::FileExist2(sFilename, pDiskFilePath)){
			dwFoundPlace = FILE_ON_DISK;
		}
	}
	else
	{
		// we will use GetDiskFilePriority(), to determine which one to search first.
		if ((dwWhereToSearch & FILE_ON_DISK) > 0 && GetDiskFilePriority() >= 0)
		{
			if (CFileUtils::FileExist2(sFilename, pDiskFilePath)){
				dwFoundPlace = FILE_ON_DISK;
			}
		}

		if (!dwFoundPlace && (dwWhereToSearch & FILE_ON_ZIP_ARCHIVE)>0)
		{
			// perform further search in ZIP files. 
			CFileManager* pFileManager = CFileManager::GetInstance();
			if (pFileManager->DoesFileExist(sFilename))
				dwFoundPlace = FILE_ON_ZIP_ARCHIVE;
		}

		if (!dwFoundPlace && (dwWhereToSearch & FILE_ON_DISK) > 0 && GetDiskFilePriority() < 0)
		{
			if (CFileUtils::FileExist2(sFilename, pDiskFilePath)) {
				dwFoundPlace = FILE_ON_DISK;
			}
		}
	}
	if (!dwFoundPlace && (dwWhereToSearch & FILE_ON_EXECUTABLE) > 0)
	{
		if (CFileUtils::DoesResFileExist(filename))
		{
			dwFoundPlace = FILE_ON_EXECUTABLE;
		}
	}
	return dwFoundPlace;
}

bool CParaFile::BackupFile(const char* filename)
{
	CParaFile file(filename);
	if (file.isEof())
		return false;

	CParaFile fileBK;
	string fileBKname = filename;
	fileBKname.append(".bak");

	if (fileBK.CreateNewFile(fileBKname.c_str()))
	{
		fileBK.write(file.getBuffer(), (int)file.m_size);
		return true;
	}
	return false;
}

bool CParaFile::CreateDirectory(const char* filename)
{
	if (filename == NULL)
		return false;
	string sFile;
	CParaFile::ToCanonicalFilePath(sFile, filename);
	return CFileUtils::MakeDirectoryFromFilePath(sFile.c_str());
}

bool CParaFile::CreateNewFile(const char* filename, bool bAutoMakeFilePath)
{
	string sFile;
	CParaFile::ToCanonicalFilePath(sFile, filename);
	if (bAutoMakeFilePath)
	{
		if (!CFileUtils::MakeDirectoryFromFilePath(sFile.c_str()))
		{
			std::cout << "create directory failed:" << sFile << std::endl;
		}
	}

	bool bSuc = OpenFile(sFile.c_str(), false);
	if (bSuc)
	{
		// truncate the file to zero size.
		SetFilePointer(0, FILE_BEGIN);
		SetEndOfFile();
		SetFilePointer(0, FILE_BEGIN);
	}
	return bSuc;
}

int CParaFile::OpenAssetFile(const char* filename, bool bDownloadIfNotUpToDate, const char* relativePath)
{
	if (relativePath != NULL)
	{
		int nRes = OpenAssetFile(filename, bDownloadIfNotUpToDate);
		if (nRes == 0)
		{
			// try relative path if not exist. 
			char sNewFilename[MAX_PATH_LENGTH];
			CFileUtils::MakeFileNameFromRelativePath(sNewFilename, filename, relativePath);

			nRes = OpenAssetFile(sNewFilename, bDownloadIfNotUpToDate);
		}
		return nRes;
	}
	AssetFileEntry* pEntry = CAssetManifest::GetSingleton().GetFile(filename);
	if (pEntry)
	{
		if (CParaFile::DoesFileExist(filename, false))
		{
			string sTmp = string("ParaFile.OpenAssetFile using disk file:") + filename + "\n";
			CAsyncLoader::GetSingleton().log(sTmp);
			if (OpenFile(filename, true, relativePath))
				return 1;
			else
			{
				OUTPUT_LOG("error: failed open disk file: %s\n", filename);
			}
		}

		if (pEntry->DoesFileExist())
		{
			if (m_filename.empty())
				m_filename = filename;
			return OpenFile(pEntry->GetLocalFileName().c_str(), true, relativePath) ? 1 : 0;
		}
		else
		{
			if (!bDownloadIfNotUpToDate)
			{
				return 0;
			}
			else
			{
				// download the file here. 
				if (pEntry->SyncFile())
				{
					// open the file now. 
					if (m_filename.empty())
						m_filename = filename;
					return OpenFile(pEntry->GetLocalFileName().c_str(), true, relativePath) ? 1 : 0;
				}
			}
		}
	}
	else
	{
		string sTmp = string("ParaFile.OpenAssetFile using local file:") + filename + "\n";
		CAsyncLoader::GetSingleton().log(sTmp);
#if defined(PARAENGINE_MOBILE) || defined(EMSCRIPTEN)
// #if defined(PARAENGINE_MOBILE)
		if (relativePath == NULL)
		{
			uint32 dwFound = FILE_NOT_FOUND;
			// check file existence, then open file will prevent file not found warnings in mobile version. 
			if ((dwFound = CParaFile::DoesFileExist2(filename, FILE_ON_DISK | FILE_ON_ZIP_ARCHIVE | FILE_ON_SEARCH_PATH)))
			{
				return OpenFile(filename, true, relativePath, false, dwFound) ? 1 : 0;
			}
			return 0;
		}
#endif
		return OpenFile(filename, true, relativePath) ? 1 : 0;
	}
	return 0;
}

bool CParaFile::DoesAssetFileExist(const char* filename)
{
	return CParaFile::DoesAssetFileExist2(filename, false);
}


bool CParaFile::DoesAssetFileExist2(const char* filename, bool bSearchZipFile/*=false*/)
{
	AssetFileEntry* pEntry = CAssetManifest::GetSingleton().GetFile(filename);
	if (pEntry)
	{
		// return pEntry->DoesFileExist();
		return true;
	}
	return DoesFileExist(filename, bSearchZipFile);
}

bool CParaFile::Decompress()
{
	if (!IsCompressed() || m_uncompressed_size <= 0 || isEof())
		return true;
	SetIsCompressed(false);

	char* buff = new char[m_uncompressed_size + 1];
	if (buff)
	{
		buff[m_uncompressed_size] = '\0';
		if (CZipArchive::Decompress(m_buffer, m_size, buff, (DWORD)m_uncompressed_size))
		{
			char* pOld = m_buffer;
			m_buffer = buff;
			m_size = m_uncompressed_size;

			if (m_bIsOwner && pOld)
				delete[] pOld;

			m_curPos = 0;
			synchBits();
			return true;
		}
	}
	OUTPUT_LOG("warning: unable to Decompress file \n");
	return false;
}

bool CParaFile::IsCompressed()
{
	return m_bIsCompressed;
}

void CParaFile::SetIsCompressed(bool bCompressed)
{
	m_bIsCompressed = bCompressed;
}

bool CParaFile::UnzipMemToFile(const char* buffer, int nSize, const char* destFilename, bool bAutoMakeDirectory)
{
	bool bRes = false;
	CZipArchive zip_file;
	zip_file.OpenMemFile(buffer, nSize, false);
	for (int nIndex = 0; nIndex < zip_file.GetFileCount(); nIndex++)
	{
		FileHandle file_handle;
		file_handle.m_pArchive = &zip_file;
		file_handle.m_index = nIndex;
		DWORD nSize = zip_file.GetFileSize(file_handle);
		DWORD nBytesRead = 0;
		if (nSize > 0)
		{
			// skip zero length file. 
			char * buffer = new char[nSize];
			DWORD lastWriteTime;
			if (zip_file.ReadFile(file_handle, buffer, nSize, &nBytesRead, &lastWriteTime))
			{
				CParaFile file;
				// save file to temp/cache directory.
				if (file.CreateNewFile(destFilename, bAutoMakeDirectory))
				{
					// TODO: check the MD5 
					// TODO: make file writing atomic operation, such as writing to a temporary file and then call file rename function. 
					file.write(buffer, nSize);
					file.close();
					SAFE_DELETE_ARRAY(buffer);

					bRes = true;
				}
				else
				{
					OUTPUT_LOG("warning: failed creating file %s for saving cached asset file.\n", destFilename);
				}
			}
			SAFE_DELETE_ARRAY(buffer);
			return bRes;
		}
	}

	OUTPUT_LOG("warning: no file in the downloaded asset file %s\n", destFilename);
	return bRes;
}


bool ParaEngine::CParaFile::GetFileInfo(const char* sfilename, CParaFileInfo& fileInfo, uint32 dwWhereToOpen /*= FILE_ON_DISK | FILE_ON_ZIP_ARCHIVE | FILE_ON_SEARCH_PATH*/)
{
	int32 dwFoundPlace = FILE_NOT_FOUND;
	if (!GetDevDirectory().empty() && dwWhereToOpen != FILE_ON_ZIP_ARCHIVE && ((dwWhereToOpen & FILE_ON_ZIP_ARCHIVE) > 0))
	{
		// in development version, find in current directory and search path and then zip archive
		dwWhereToOpen &= (~((uint32)FILE_ON_ZIP_ARCHIVE));
		return GetFileInfo(sfilename, fileInfo, dwWhereToOpen) ||
			GetFileInfo(fileInfo.m_sFullpath.empty() ? sfilename : fileInfo.m_sFullpath.c_str(), fileInfo, FILE_ON_ZIP_ARCHIVE);
	}

	if ((dwWhereToOpen & FILE_ON_SEARCH_PATH) > 0)
	{
		dwWhereToOpen &= (~((uint32)FILE_ON_SEARCH_PATH));

		std::list<SearchPath>& searchPaths = CFileManager::GetInstance()->GetSearchPaths();

		if (searchPaths.size() == 0 || (sfilename[0] == '/') || (sfilename[0] == '\\'))
		{
			if (dwWhereToOpen != 0)
				return GetFileInfo(sfilename, fileInfo, dwWhereToOpen);
			return false;
		}
		else
		{
			// find in current directory and zip file, and then in search path
			bool bFound = (dwWhereToOpen != 0) && GetFileInfo(sfilename, fileInfo, dwWhereToOpen);
			if (!bFound)
			{
				if (!fileInfo.m_sFullpath.empty())
					sfilename = fileInfo.m_sFullpath.c_str();
				list<SearchPath>::iterator itCurCP, itEndCP = searchPaths.end();
				for (itCurCP = searchPaths.begin(); !bFound && itCurCP != itEndCP; ++itCurCP)
				{
					auto fullpath = (*itCurCP).GetPath() + sfilename;
					bFound = GetFileInfo(fullpath.c_str(), fileInfo, FILE_ON_DISK);
				}
			}
			return bFound;
		}
	}
	char filename[MAX_PATH_LENGTH];

	int i = 0;
	int j = 0;
#ifdef WIN32
	// remove the heading slash
	if ((sfilename[0] == '/') || (sfilename[0] == '\\'))
		i++;
#endif

	// replace '\\' with '/'
	for (; sfilename[i] != '\0' && j < (MAX_PATH_LENGTH - 1); i++, j++)
	{
		if (sfilename[i] == '\\')
			filename[j] = '/';
		else
			filename[j] = sfilename[i];
	}
	filename[j] = '\0';

	if (fileInfo.m_sFullpath.empty())
		fileInfo.m_sFullpath = filename;

	if (CFileReplaceMap::GetSingleton().ReplaceFile(fileInfo.m_sFullpath))
	{
		strcpy(filename, fileInfo.m_sFullpath.c_str());
	}

	if ((dwWhereToOpen & FILE_ON_DISK)>0 && GetDiskFilePriority() >= 0)
	{
		if (CFileUtils::GetFileInfo(filename, fileInfo))
			return true;
	}

	if ((dwWhereToOpen & FILE_ON_ZIP_ARCHIVE) > 0)
	{
		CFileManager* pFileManager = CFileManager::GetInstance();
		/// load from Zip files, if the file does not exist on disk
		FileHandle fileHandle;
		if (pFileManager->OpenFile(filename, fileHandle))
		{
			fileInfo.m_mode = CParaFileInfo::ModeFileInZip;
			fileInfo.m_dwFileSize = pFileManager->GetFileSize(fileHandle);
			if (fileHandle.m_pArchive){
				fileInfo.m_sFullpath = std::string("zip://") + fileHandle.m_pArchive->GetArchiveName() + "/" + filename;
			}
			pFileManager->CloseFile(fileHandle);
			return true;
		}
	}

	if (GetDiskFilePriority() < 0 && (dwWhereToOpen & FILE_ON_DISK) > 0)
	{
		if (CFileUtils::GetFileInfo(filename, fileInfo))
			return true;
	}
	return false;
}


void* ParaEngine::CParaFile::GetHandlePtr()
{
	return m_handle.m_pVoidPtr;
}

bool ParaEngine::CParaFile::IsWritablePath(const std::string& sFilename, bool bLogWarning)
{
	bool bWritable = true;
	
	// TODO security alert: only verify on win32, linux is more permissive at the moment. 
	if (ParaEngineSettings::IsSandboxMode())
	{
		// skip writable, application and dev directory when it is absolute path.
		if (IsAbsolutePath(sFilename))
		{
			std::string output, writablePath;
			writablePath = CParaFile::GetWritablePath();
			ToCanonicalFilePath(writablePath, writablePath);
			ToCanonicalFilePath(output, sFilename);

			if (output.compare(0, writablePath.length(), writablePath) != 0)
			{
				ToCanonicalFilePath(writablePath, CParaFile::GetCurDirectory(0));
				if (output.compare(0, writablePath.length(), writablePath) != 0)
				{
					if (!CParaFile::GetDevDirectory().empty())
					{
						ToCanonicalFilePath(writablePath, CParaFile::GetDevDirectory());
						if (output.compare(0, writablePath.length(), writablePath) != 0)
							bWritable = false;
					}
					else
						bWritable = false;
				}
			}
		}
		else
		{
			// TODO: only allow move in some given folder. we will only allow deletion in the specified user directory
		}
	}

	if (!bWritable && bLogWarning)
	{
		OUTPUT_LOG("security alert: some one is telling the engine to open a file %s which is not allowed\r\n", sFilename.c_str());
	}
	return true;
}

bool ParaEngine::CParaFile::OpenFile(CArchive *pArchive, const char* filename, bool bUseCompressed)
{
	m_eof = true;
	if (pArchive->OpenFile(filename, m_handle))
	{
		CFileManager* pFileManager = CFileManager::GetInstance();
		m_curPos = 0;
		if (bUseCompressed)
		{
			DWORD compressedSize = 0;
			DWORD uncompressedSize = 0;
			if (pFileManager->ReadFileRaw(m_handle, (LPVOID*)(&m_buffer), &compressedSize, &uncompressedSize))
			{
				m_size = compressedSize;
				m_uncompressed_size = uncompressedSize;
				// this fix a bug for non-compressed files
				if (m_uncompressed_size > 0)
					SetIsCompressed(true);
				m_eof = false;
			}
			else
			{
				m_buffer = 0;
				m_eof = true;
			}
		}
		else
		{
			DWORD s = pFileManager->GetFileSize(m_handle);
			DWORD bytesRead = 0;
			DWORD lastWriteTime = 0;

			m_buffer = new char[s + 1];
			m_buffer[s] = '\0';

			pFileManager->ReadFile(m_handle, m_buffer, s, &bytesRead, &lastWriteTime);
			pFileManager->CloseFile(m_handle);
			m_size = (size_t)bytesRead;
			m_eof = false;
		}
	}
	else
	{
		m_eof = true;
		m_buffer = 0;
	}
	return !m_eof;
}

bool CParaFile::OpenFile(const char* sfilename, bool bReadyOnly, const char* relativePath, bool bUseCompressed, uint32 dwWhereToOpen)
{
	if(sfilename == NULL || sfilename[0] == '\0'){
		m_eof = true;
		return false;
	}
	
	int32 dwFoundPlace = FILE_NOT_FOUND;
	if (dwWhereToOpen != FILE_ON_ZIP_ARCHIVE && ((dwWhereToOpen & FILE_ON_ZIP_ARCHIVE) > 0))
	{
		// find in current directory and search path and then zip archive
		dwWhereToOpen &= (~((uint32)FILE_ON_ZIP_ARCHIVE));
		return OpenFile(sfilename, bReadyOnly, relativePath, bUseCompressed, dwWhereToOpen) ||
			OpenFile(m_filename.empty() ? sfilename : m_filename.c_str(), bReadyOnly, relativePath, bUseCompressed, FILE_ON_ZIP_ARCHIVE);
	}
	if ((dwWhereToOpen & FILE_ON_SEARCH_PATH) > 0)
	{
		dwWhereToOpen &= (~((uint32)FILE_ON_SEARCH_PATH));
		std::list<SearchPath>& searchPaths = CFileManager::GetInstance()->GetSearchPaths();

		if (searchPaths.size() == 0 || (sfilename[0] == '/') || (sfilename[0] == '\\') || relativePath != NULL)
		{
			if (dwWhereToOpen!=0)
				return OpenFile(sfilename, bReadyOnly, relativePath, bUseCompressed, dwWhereToOpen);
			return false;
		}
		else
		{
			// find in current directory and zip file, and then in search path
			bool bFound = (dwWhereToOpen != 0) && OpenFile(sfilename, bReadyOnly, relativePath, bUseCompressed, dwWhereToOpen);
			if (!bFound)
			{
				if (!m_filename.empty())
					sfilename = m_filename.c_str();
				list<SearchPath>::iterator itCurCP, itEndCP = searchPaths.end();
				for (itCurCP = searchPaths.begin(); !bFound && itCurCP != itEndCP; ++itCurCP)
				{
					bFound = OpenFile(sfilename, bReadyOnly, (*itCurCP).GetPath().c_str(), bUseCompressed, FILE_ON_DISK);
				}
			}
			return bFound;
		}
	}
	m_bIsOwner = true;
	char filename[MAX_PATH_LENGTH];

	int i = 0;
	int j = 0;
#ifdef WIN32
	// remove the heading slash
	if ((sfilename[0] == '/') || (sfilename[0] == '\\'))
		i++;
#endif

	// replace '\\' with '/'
	for (; sfilename[i] != '\0' && j<(MAX_PATH_LENGTH - 1); i++, j++)
	{
		if (sfilename[i] == '\\')
			filename[j] = '/';
		else
			filename[j] = sfilename[i];
	}
	filename[j] = '\0';

	/// append the relative path
	if (relativePath != NULL)
	{
		char sRelativePath[MAX_PATH_LENGTH];
		int nLastSlash = -1;
		for (int i = 0; relativePath[i] != '\0' && i<(MAX_PATH_LENGTH - 1); i++)
		{
			if (relativePath[i] == '\\')
				sRelativePath[i] = '/';
			else
				sRelativePath[i] = relativePath[i];
			if (sRelativePath[i] == '/')
				nLastSlash = i;
		}
		sRelativePath[nLastSlash + 1] = '\0';

		int nRelativePathLength = nLastSlash + 1;
		int nFilenameLength = j;
		if (sRelativePath[0] != '\0' && (nRelativePathLength + nFilenameLength) < MAX_PATH_LENGTH)
		{
			// prepend relative path to filename
			for (int i = nFilenameLength - 1; i >= 0; i--)
				filename[i + nRelativePathLength] = filename[i];
			for (int i = 0; i < nRelativePathLength; i++)
				filename[i] = sRelativePath[i];
			filename[nRelativePathLength + nFilenameLength] = '\0';
		}
	}

	if (m_filename.empty())
		m_filename = filename;

	if (CFileReplaceMap::GetSingleton().ReplaceFile(m_filename))
	{
		strcpy(filename, m_filename.c_str());
	}

	if (bReadyOnly)
	{
		if (!(!m_filename.empty() && m_filename[0] == ':') && ((dwWhereToOpen & FILE_ON_EXECUTABLE) == 0) && !(!m_filename.empty() && m_filename[m_filename.size() - 1] == '/') && !m_filename.empty())
		{
			BOOL succ = FALSE;
			/// for ready-only file, we will read everything in to the buffer, and close the file handle
			if ((dwWhereToOpen & FILE_ON_DISK)>0 && GetDiskFilePriority() >= 0)
			{
				FileData data = CFileUtils::GetDataFromFile(filename);
				if (!data.isNull())
				{
					m_buffer = data.GetBytes();
					m_size = data.GetSize();
					data.ReleaseOwnership();
					m_eof = FALSE;
					succ = TRUE;
				}
			}

			if (!succ && (dwWhereToOpen & FILE_ON_ZIP_ARCHIVE)>0)
			{
				CFileManager* pFileManager = CFileManager::GetInstance();
				/// load from Zip files, if the file does not exist on disk
				succ = pFileManager->OpenFile(filename, m_handle);
				m_curPos = 0;
				if (succ)
				{
					if (bUseCompressed)
					{
						DWORD compressedSize = 0;
						DWORD uncompressedSize = 0;
						if (pFileManager->ReadFileRaw(m_handle, (LPVOID*)(&m_buffer), &compressedSize, &uncompressedSize))
						{
							m_size = compressedSize;
							m_uncompressed_size = uncompressedSize;
							// this fix a bug for non-compressed files
							if (m_uncompressed_size > 0)
								SetIsCompressed(true);
							m_eof = false;
						}
						else
						{
							m_buffer = 0;
							m_eof = true;
						}
					}
					else
					{
						DWORD s = pFileManager->GetFileSize(m_handle);
						DWORD bytesRead = 0;
						DWORD lastWriteTime = 0;
						m_buffer = new char[s + 1];
						m_buffer[s] = '\0';
						pFileManager->ReadFile(m_handle, m_buffer, s, &bytesRead, &lastWriteTime);
						pFileManager->CloseFile(m_handle);
						m_size = (size_t)bytesRead;
						m_lastModifiedTime = lastWriteTime;
						m_eof = false;
					}

				}
				else
				{
					m_eof = true;
					m_buffer = 0;
				}
			}

			if (!succ && GetDiskFilePriority()<0 && (dwWhereToOpen & FILE_ON_DISK)>0)
			{
				FileData data = CFileUtils::GetDataFromFile(filename);
				if (!data.isNull())
				{
					m_buffer = data.GetBytes();
					m_size = data.GetSize();
					data.ReleaseOwnership();
					m_eof = FALSE;
					succ = TRUE;
				}
			}

			/** extract file to disk for debugging purposes. */
			if (m_bExtractFileToDisk && !isEof())
			{
				string filenameDest = m_strExtractRootPath;
				filenameDest.append(filename);
				ExtractFileToDisk(filenameDest, false);
			}
		}
		else
		{
			/** If the file name begins with ':' or FILE_ON_EXECUTABLE is specified, it is treated as a win32 resource.
			e.g.":IDR_FX_OCEANWATER". loads data from a resource of type "TEXTFILE". See MSDN for more information about Windows resources.
			we also support C++ embedded binary file as external global variables in this way. 
			*/
			m_eof = true;
			FileData data = CFileUtils::GetResDataFromFile(filename);
			if (!data.isNull())
			{
				m_buffer = data.GetBytes();
				m_size = data.GetSize();
				data.ReleaseOwnership();
				m_bIsOwner = false; // resource file do not need to be deleted on Close().
				m_eof = false;
			}
		}
	}
	else
	{
		/// for write-only file, we will save file handle.
		m_eof = true; /// set end of file to prevent read access.

		if (m_filename == "<memory>")
		{
			m_bMemoryFile = true;
			m_bIsOwner = false;
			m_handle.m_pVoidPtr = new StringBuilder();
			return true;
		}
		else
		{
			FileHandle fileHandle = CFileUtils::OpenFile(filename, true, true);

			if (fileHandle.IsValid())
			{
				m_handle = fileHandle;
				m_bDiskFileOpened = true;
				return true;
			}
			else
				m_bDiskFileOpened = false;
		}
		return false;
	}
#ifdef FILE_LOGGER_HOOK
	if (CFileLogger::GetInstance()->IsBegin())
	{
		if (!m_eof && !m_filename.empty() && m_filename[0] != ':')
		{
			CFileLogger::GetInstance()->AddFile(m_filename.c_str(), bReadyOnly);
		}
	}
#endif
	return !m_eof;
}

bool CParaFile::SetEndOfFile()
{
	if (m_bDiskFileOpened)
	{
		return CFileUtils::SetEndOfFile(m_handle);
	}
	else if (m_bMemoryFile)
	{
		ToStringBuilder(this)->resize(m_curPos);
		m_size = m_curPos;
		m_eof = true;
	}
	return false;
}


void CParaFile::SetFilePointer(int lDistanceToMove, int dwMoveMethod)
{
	if (m_bDiskFileOpened)
	{
		CFileUtils::SetFilePointer(m_handle, lDistanceToMove, dwMoveMethod);
		if (dwMoveMethod == FILE_BEGIN)
		{
			m_curPos = lDistanceToMove;
		}
		else if (dwMoveMethod == FILE_END)
		{
			m_curPos = getPos();
		}
		else if (dwMoveMethod == FILE_CURRENT)
		{
			m_curPos += lDistanceToMove;
		}
	}
	else if (m_bMemoryFile)
	{
		if (dwMoveMethod == FILE_BEGIN)
		{
			m_curPos = lDistanceToMove;
		}
		else if (dwMoveMethod == FILE_END)
		{
			m_curPos = ToStringBuilder(this)->size() - lDistanceToMove;
		}
		else if (dwMoveMethod == FILE_CURRENT)
		{
			m_curPos += lDistanceToMove;
		}
	}
}

size_t CParaFile::read(void* dest, size_t bytes)
{
	if (!m_bDiskFileOpened)
	{
		if (m_eof) return 0;

		size_t rpos = m_curPos + bytes;
		if (rpos >= m_size) {
			bytes = m_size - m_curPos;
			m_eof = true;
		}

		memcpy(dest, &(m_buffer[m_curPos]), bytes);

		m_curPos = rpos;

		return bytes;
	}
	else
	{
		int bytesRead = CFileUtils::ReadBytes(m_handle, dest, bytes);
		m_curPos += bytesRead;
		return bytesRead;
	}
}

int CParaFile::write(const void* src, int bytes)
{
	if (m_bDiskFileOpened)
	{
		DWORD bytesWritten = CFileUtils::WriteBytes(m_handle, src, bytes);
		m_curPos += bytesWritten;
		return bytesWritten;
	}
	else if (m_bMemoryFile)
	{
		auto pBuilder = ToStringBuilder(this);
		if (m_curPos == m_size)
		{
			pBuilder->append((const char*)src, bytes);
			m_curPos += bytes;
		}
		else
		{
			pBuilder->WriteAt(m_curPos, (const char*)src, bytes);
			m_curPos += bytes;
		}
		m_size = pBuilder->size();
		m_buffer = pBuilder->str();
	}
	return 0;
}

int CParaFile::WriteString(const string& sStr)
{
	return write(sStr.c_str(), (int)sStr.size());
}

int CParaFile::WriteString(const char* sStr, int nLen)
{
	if (sStr)
	{
		return write(sStr, (nLen > 0) ? nLen : (int)strlen(sStr));
	}else
		return 0;
}

int CParaFile::WriteFormated(const char * zFormat, ...)
{
	char buf_[MAX_PARAFILE_LINE_LENGTH + 1];
	va_list args;
	va_start(args, zFormat);
	vsnprintf(buf_, MAX_PARAFILE_LINE_LENGTH, zFormat, args);
	return WriteString(buf_);
}

bool CParaFile::isEof()
{
	return m_eof;
}

void CParaFile::seek(int offset)
{
	if (m_bDiskFileOpened)
	{
		SetFilePointer(offset, FILE_BEGIN);
	}
	else
	{
		m_curPos = offset;
		m_eof = (m_curPos >= m_size);
	}
}

void CParaFile::seekRelative(int offset)
{
	if (! m_bDiskFileOpened)
	{
		m_curPos += offset;
		m_eof = (m_curPos >= m_size);
	}
	else
	{
		SetFilePointer(offset, FILE_CURRENT);
	}
}

void CParaFile::close()
{
	if (m_bDiskFileOpened)
	{
		CFileUtils::CloseFile(m_handle);
		if (m_restoreLastModifiedTimeAfterClose && m_lastModifiedTime > 0)
		{
			m_restoreLastModifiedTimeAfterClose = false;
			time_t standard_time;
			DWORD lastWriteTime = m_lastModifiedTime;
			WORD dosdate = lastWriteTime>>16;
			WORD dostime = lastWriteTime&0xffff;
			dosdatetime2filetime(dosdate, dostime, &standard_time);
			CFileUtils::WriteLastModifiedTimeToDisk(m_handle, m_filename, standard_time);
		}
	}
	else if (m_bMemoryFile && m_handle.m_pVoidPtr!=NULL)
	{
		delete ToStringBuilder(this);
		m_handle.m_pVoidPtr = NULL;
	}
	if (m_bIsOwner && m_buffer)
		delete[] m_buffer;
	m_buffer = 0;
	m_curPos = 0;
	m_eof = true;
}

size_t CParaFile::getSize()
{
	if(!m_bDiskFileOpened)
		return m_size;
	else
	{
		int nPos = getPos();
		SetFilePointer(0, FILE_END);
		int nSize = getPos();
		SetFilePointer(nPos, FILE_BEGIN);
		return nSize;
	}
}

size_t CParaFile::getPos()
{
	if(!m_bDiskFileOpened)
		return m_curPos;
	else
	{
		return CFileUtils::GetFilePosition(m_handle);
	}
}

char* CParaFile::getBuffer()
{
	return m_buffer;
}

int CParaFile::GetNextLine(char* buf, int sizeBuf)
{
	if (isEof())
	{
		buf[0] = '\0';
		return 0;
	}

	bool bEndOfLine = false;
	int i = 0;
	sizeBuf--;
	while (i<sizeBuf)
	{
		if (isEof())
			break;
		char c = *getPointer();

		if ((c == '\r') || (c == '\n'))
		{
			bEndOfLine = true;
		}
		else
		{
			if (bEndOfLine == true)
				break;
			else
			{
				buf[i++] = c;
			}
		}
		seekRelative(1);
	}
	buf[i] = '\0';
	if ((i == 0) || ((buf[0] == '-') && (buf[1] == '-')))
		return GetNextLine(buf, sizeBuf);
	else
		return i;
}

void CParaFile::SetExtractFileProperty(bool bExtractFile)
{
	m_bExtractFileToDisk = bExtractFile;
}
bool CParaFile::GetExtractFileProperty()
{
	return m_bExtractFileToDisk;
}

const string& CParaFile::GetFileName()
{
	return m_filename;
}

DWORD CParaFile::GetLastModifiedTime() const
{
	return m_lastModifiedTime;
}

bool CParaFile::SetLastModifiedTime(DWORD lastWriteTime)
{
	if (m_lastModifiedTime != lastWriteTime)
	{
		m_lastModifiedTime = lastWriteTime;
		return true;
	}
	return false;
}

bool CParaFile::WriteLastModifiedTime(DWORD lastWriteTime)
{
	SetLastModifiedTime(lastWriteTime);

	bool result = false;
	if(lastWriteTime>0 && m_bDiskFileOpened)
	{
		if(m_handle.IsValid())
		{
			time_t standard_time;
			WORD dosdate = lastWriteTime>>16;
			WORD dostime = lastWriteTime&0xffff;
			dosdatetime2filetime(dosdate, dostime, &standard_time);
			result = CFileUtils::WriteLastModifiedTimeToDisk(m_handle, m_filename, standard_time);
			if (result)
				m_restoreLastModifiedTimeAfterClose = true;
		}
	}
	return result;
}

bool CParaFile::GetRestoreLastModifiedTimeAfterClose() const
{
	return m_restoreLastModifiedTimeAfterClose;
}

void CParaFile::SetRestoreLastModifiedTimeAfterClose(bool shouldRestore)
{
	m_restoreLastModifiedTimeAfterClose = shouldRestore;
}

//////////////////////////////////////////////////////////////////////////
//
// Some file name operations
//
//////////////////////////////////////////////////////////////////////////

std::string CParaFile::GetParentDirectoryFromPath(const string& sfilename, int nParentCounts)
{
	return CFileUtils::GetParentDirectoryFromPath(sfilename, nParentCounts);
}

string CParaFile::AutoFindParaEngineRootPath(const string& sFile)
{
	string sRootPath = GetParentDirectoryFromPath(sFile, 0);
	CSearchResult result;
	while (!sRootPath.empty())
	{
		result.InitSearch(sRootPath);
		CFileManager::FindDiskFiles(result, result.GetRootPath(), PARAENGINE_SIG_FILE, 0);
		if (result.GetNumOfResult() >= 1)
		{
			string sSigFile = result.GetItem(0);
			return result.GetRootPath() + GetParentDirectoryFromPath(sSigFile, 0);
		}
		string tmp = GetParentDirectoryFromPath(sRootPath, 1);
		sRootPath = tmp;
	}
	return "";
}

string CParaFile::ChangeFileExtension(const string& sfilename, const string & sExt)
{
	int nSlashCount = 0;
	int nLastSlashPosition = 0;
	int i;
	for (i = (int)sfilename.size() - 1; i >= 0; --i)
	{
		if (sfilename[i] == '.')
		{
			string sNewFile = sfilename.substr(0, i + 1);
			sNewFile += sExt;
			return sNewFile;
		}
	}
	return sfilename;
}
string CParaFile::GetFileExtension(const string& sfilename)
{
	for (int i = (int)sfilename.size() - 1; i >= 0; --i)
	{
		if (sfilename[i] == '.')
		{
			int nCount = (int)sfilename.size() - (i + 1);
			if (nCount>0)
				return sfilename.substr(i + 1, nCount);
			else
				return "";
		}
		else if (sfilename[i] == '/' || sfilename[i] == '\\')
			return "";
	}
	return "";
}

string CParaFile::GetAbsolutePath(const string& sRelativePath, const string& sRootPath)
{
	std::string fullPath = sRootPath;
	if (fullPath.size() > 0 && fullPath.back() != '/' && fullPath.back() != '\\')
		fullPath += "/";

	if (sRelativePath[0] == '.' && sRelativePath.size() > 3)
	{
		if (sRelativePath[1] == '/')
		{
			fullPath.append(sRelativePath.c_str() + 2);
		}
		else if (sRelativePath[1] == '.' && sRelativePath[2] == '/')
		{
			// such as ../../
			fullPath = ParaEngine::CParaFile::GetParentDirectoryFromPath(fullPath, 1);
			int nOffset = 3;
			while (sRelativePath[nOffset] == '.' && sRelativePath[nOffset + 1] == '.' && sRelativePath[nOffset + 2] == '/')
			{
				fullPath = ParaEngine::CParaFile::GetParentDirectoryFromPath(fullPath, 1);
				nOffset += 3;
			}
			fullPath.append(sRelativePath.c_str() + nOffset);
		}
		else
		{
			fullPath += sRelativePath;
		}
	}
	else
	{
		fullPath += sRelativePath;
	}
	return fullPath;
}

string CParaFile::GetRelativePath(const string& sAbsolutePath, const string& sRootPath)
{
	int nRootSize = (int)sRootPath.size();
	if (((int)sAbsolutePath.size())>nRootSize)
	{
		for (int i = 0; i<nRootSize; ++i)
		{
			// shall we make the comparison case insensitive.
			char a = sRootPath[i];
			char b = sAbsolutePath[i];
			if (!((a == b) || ((a - b) == 'a' - 'A') || ((b - a) == 'a' - 'A') ||
				((a == '\\') && (b == '/')) || ((b == '\\') && (a == '/'))))
			{
				return sAbsolutePath;
			}
		}
		return sAbsolutePath.substr(nRootSize, (int)sAbsolutePath.size() - nRootSize);
	}
	return sAbsolutePath;
}

string CParaFile::GetFileName(const string& sfilename)
{
	int i;
	for (i = (int)sfilename.size() - 1; i >= 0; --i)
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

void CParaFile::SetDevDirectory(const string& sFilePath)
{
	int nCount;
	if ((nCount = sFilePath.size()) > 0)
	{
		string sDevRootPath = sFilePath;
		if (sFilePath[nCount - 1] != '/' && sFilePath[nCount - 1] != '\\')
		{
			sDevRootPath.append("/");
		}
		ToCanonicalFilePath(g_dev_dir, sDevRootPath, false);
		OUTPUT_LOG("Development directory is set to : %s\n", g_dev_dir.c_str());
	}
	else
		g_dev_dir = "";
}

const string& CParaFile::GetDevDirectory()
{
	return g_dev_dir;
}

// thread-safe now
const string& CParaFile::GetCurDirectory(DWORD dwDirectoryType)
{
	static string sRootDir;
	static string g_CurDirs[APP_LAST_DIR + 1];

	if (dwDirectoryType >= APP_LAST_DIR)
	{
		OUTPUT_LOG("directory path index out of range %d\n", dwDirectoryType);
		dwDirectoryType = 0;
	}
	else if (dwDirectoryType == APP_DEV_DIR)
	{
		return GetDevDirectory();
	}
	ParaEngine::Lock lock_(g_file_mutex);

	if (dwDirectoryType == APP_EXECUTABLE_DIR)
	{
		if (g_CurDirs[dwDirectoryType].empty())
			g_CurDirs[dwDirectoryType] = CGlobals::GetApp()->GetModuleDir();
		return g_CurDirs[dwDirectoryType];
	}
	
	if (dwDirectoryType<APP_SH_DESKTOP_DIR)
	{
		if (!g_CurDirs[dwDirectoryType].empty())
			return g_CurDirs[dwDirectoryType];
		else
		{
			if (sRootDir.empty())
			{
				// sRootDir = GetDevDirectory().empty() ? CFileUtils::GetInitialDirectory() : GetDevDirectory();
				sRootDir = CFileUtils::GetInitialDirectory();
			}
			g_CurDirs[dwDirectoryType] = sRootDir + paraengine_app_dir_[dwDirectoryType];
			return g_CurDirs[dwDirectoryType];
		}
	}
	else if (dwDirectoryType == APP_EXTERNAL_STORAGE_DIR)
	{
		// this will return "" on PC. 
		if (!g_CurDirs[dwDirectoryType].empty())
			return g_CurDirs[dwDirectoryType];
		else
		{
			g_CurDirs[dwDirectoryType] = CFileUtils::GetExternalStoragePath();
			return g_CurDirs[dwDirectoryType];
		}
	}
#if defined (WIN32) && !defined(PARAENGINE_MOBILE) && defined(PARAENGINE_CLIENT)
	else
	{
		if (!g_CurDirs[dwDirectoryType].empty())
			return g_CurDirs[dwDirectoryType];
		else
		{
			if (dwDirectoryType == APP_SH_DESKTOP_DIR)
			{
				g_CurDirs[dwDirectoryType] = CCommonFileDialog::GetSingleton()->GetDirectory("desktop");
			}
			else if (dwDirectoryType == APP_SH_DESKTOP_FOLDER_DIR)
			{
				g_CurDirs[dwDirectoryType] = CCommonFileDialog::GetSingleton()->GetDirectory("desktop_virtual");
			}
			return g_CurDirs[dwDirectoryType];
		}
	}
#endif
	return g_CurDirs[APP_LAST_DIR]; // this is always ""
}

bool CParaFile::IsFileName(const string& sfilename)
{
	int i;
	for (i = (int)sfilename.size() - 1; i >= 0; --i)
	{
		if (sfilename[i] == '/' || sfilename[i] == '\\')
		{
			return false;
		}
	}
	return true;
}

int CParaFile::SkipString(const char* sName)
{
	if (isEof())
		return 0;
	const char* sTarget = getPointer();
	int nMax = (int)(m_size - m_curPos);
	int i = 0;
	for (i = 0; (sName[i] != '\0') && (sName[i] == sTarget[i]) && i<nMax; ++i)
	{
	}
	if (sName[i] == '\0')
	{
		seekRelative(i);
		return i;
	}
	return 0;
}


int CParaFile::SkipEqualMark()
{
	int nCount = 0;
	for (char c = *getPointer(); (!isEof()) && (c == ' ' || c == '='); ++nCount)
	{
		seekRelative(1);
		c = *getPointer();
	}
	return nCount;
}

int CParaFile::SkipCurrentLine()
{
	int nCount = 0;
	for (char c = *getPointer(); (!isEof()); ++nCount)
	{
		seekRelative(1);
		if (c == '\r' || c == '\n')
			break;
		c = *getPointer();
	}
	return nCount;
}

int CParaFile::SkipComment()
{
	int nCount = 0;
	while (true)
	{
		if (isEof())
			break;
		char c = *getPointer();

		if ((c == '\r') || (c == '\n') || (c == ' '))
		{
			++nCount;
			seekRelative(1);
		}
		else if (c == '-')
		{
			c = *(getPointer() + 1);
			if (c == '-')
			{
				nCount += 2;
				seekRelative(2);
				// a comment line found, seek to the end of line. 
				nCount += SkipCurrentLine();
			}
		}
		else
			break;
	}
	return nCount;
}

bool CParaFile::GetNextAttribute(const char * sName, string& output)
{
	bool bSucceed = false;
	SkipComment();
	size_t nOldPos = m_curPos;

	if (isEof())
		return false;

	if (SkipString(sName) > 0 && SkipEqualMark()>0 && !isEof())
	{
		int nCount = 0;
		const char* pStart = getPointer();
		for (char c = *pStart; (!isEof()) && (c != '\r' && c != '\n'); ++nCount)
		{
			seekRelative(1);
			c = *getPointer();
		}
		output.clear();
		if (nCount > 0)
		{
			output.append(pStart, nCount);
			CFileUtils::TrimString(output);
		}
		bSucceed = true;
	}
	else
		m_curPos = nOldPos;
	return bSucceed;
}

bool CParaFile::GetNextAttribute(const char * sName, float& output)
{
	bool bSucceed = false;
	SkipComment();
	size_t nOldPos = m_curPos;

	if (isEof())
		return false;

	if (SkipString(sName) > 0 && SkipEqualMark()>0 && !isEof())
	{
#ifdef WIN32
		if (_snscanf(getPointer(), m_size - m_curPos, "%f", &output) > 0)
#else
		if (sscanf(getPointer(), "%f", &output) > 0)
#endif
		{
			bSucceed = true;
		}
		SkipCurrentLine();
	}
	else
		m_curPos = nOldPos;
	return bSucceed;
}

bool CParaFile::GetNextAttribute(const char * sName, int& output)
{
	bool bSucceed = false;
	SkipComment();
	size_t nOldPos = m_curPos;

	if (isEof())
		return false;

	if (SkipString(sName) > 0 && SkipEqualMark()>0 && !isEof())
	{
#ifdef WIN32
		if (_snscanf(getPointer(), m_size - m_curPos, "%d", &output) > 0)
#else
		if (sscanf(getPointer(), "%d", &output) > 0)
#endif
		{
			bSucceed = true;
		}
		SkipCurrentLine();
	}
	else
		m_curPos = nOldPos;

	return bSucceed;
}

bool CParaFile::GetNextAttribute(const char * sName)
{
	bool bSucceed = false;
	SkipComment();
	size_t nOldPos = m_curPos;

	if (isEof())
		return false;

	if (SkipString(sName) > 0 && !isEof())
	{
		bSucceed = true;
		SkipCurrentLine();
	}
	else
		m_curPos = nOldPos;
	return bSucceed;
}

bool CParaFile::GetNextAttribute(const char * sName, double& output)
{
	float value;
	if (GetNextAttribute(sName, value))
	{
		output = value;
		return true;
	}
	return false;
}

bool CParaFile::GetNextAttribute(const char * sName, bool& output)
{
	string value;
	if (GetNextAttribute(sName, value))
	{
		output = !(value == "0" || value == "false");
		return true;
	}
	return false;
}

// I do not know why visual c++ does not provide vsnscanf, as does with vsnprintf. It seems that most compilers in UNIX have such a function.
#ifdef WHYNOT_vsnscanf
int vsnscanf(const _TCHAR *string, size_t count, const _TCHAR *format, va_list arglist)
{
	FILE str;
	FILE *infile = &str;
	int retval;

	infile->_flag = _IOREAD | _IOSTRG | _IOMYBUF;
	infile->_ptr = infile->_base = (char *)string;
	infile->_cnt = (int)count*sizeof(_TCHAR);
	retval = (_input(infile, format, arglist));
	return(retval);
}
#endif

void CParaFile::synchBits()
{
	m_bitBuf = 0;
	m_bitPos = 0;
}
unsigned int  CParaFile::readUBits(int numBits)
{
	if (numBits == 0) return 0;

	int  bitsLeft = numBits;
	unsigned int  result = 0;

	if (m_bitPos == 0) //no value in the buffer - read a unsigned char
	{
		unsigned char tmp = 0; read(&tmp, 1);
		m_bitBuf = tmp;
		m_bitPos = 8;
	}

	while (true)
	{
		int shift = bitsLeft - m_bitPos;
		if (shift > 0)
		{
			// Consume the entire buffer
			result |= m_bitBuf << shift;
			bitsLeft -= m_bitPos;

			// Get the next unsigned char from the input stream
			unsigned char tmp = 0; read(&tmp, 1);
			m_bitBuf = tmp;
			m_bitPos = 8;
		}
		else
		{
			// Consume a portion of the buffer
			result |= m_bitBuf >> -shift;
			m_bitPos -= bitsLeft;
			m_bitBuf &= 0xff >> (8 - m_bitPos);	// mask off the consumed bits

			return result;
		}
	}
}

int CParaFile::readSBits(int numBits)
{
	// Get the number as an unsigned value.
	long uBits = readUBits(numBits);

	// Is the number negative?
	if ((uBits & (1 << (numBits - 1))) != 0)
	{
		// Yes. Extend the sign.
		uBits |= -1 << numBits;
	}
	return (int)uBits;
}

DWORD CParaFile::ReadDWORD_LE()
{
	unsigned char data[4];
	read(data, 4);
	return ((DWORD)data[0]) + (((DWORD)data[1]) << 8) + (((DWORD)data[2]) << 16) + (((DWORD)data[3]) << 24);
}

WORD CParaFile::ReadWORD_LE()
{
	unsigned char data[2];
	read(data, 2);
	return ((WORD)data[0]) + (((WORD)data[1]) << 8);
}

void CParaFile::SetDiskFilePriority(int nPriority)
{
	m_nDiskFilePriority = nPriority;
}

int CParaFile::GetDiskFilePriority()
{
	return m_nDiskFilePriority;
}

int CParaFile::GetFileSize(const char* sFilePath)
{
	return CFileUtils::GetFileSize(sFilePath);
}

uint32_t CParaFile::ReadEncodedUInt()
{
	uint32_t value;
	int b = 0;
	if (read(&b, 1) == 0)
		return 0;

	if ((b & 0x80) == 0)
	{
		// single-unsigned char
		value = (uint32_t)b;
		return value;
	}

	int shift = 7;

	value = (uint32_t)(b & 0x7F);
	bool keepGoing;
	int i = 0;
	do
	{
		if (read(&b, 1) == 0)
			return 0;


		i++;
		keepGoing = (b & 0x80) != 0;
		value |= ((uint32_t)(b & 0x7F)) << shift;
		shift += 7;
	} while (keepGoing && i < 4);
	if (keepGoing && i == 4)
	{
		return 0;
	}
	return value;
}

int CParaFile::WriteEncodedUInt(uint32_t value)
{
	BYTE buffer[10];
	int count = 0, index = 0;
	do
	{
		buffer[index++] = (unsigned char)((value & 0x7F) | 0x80);
		value >>= 7;
		count++;
	} while (value != 0);
	buffer[index - 1] &= 0x7F;

	return write((const char*)buffer, count);
}

const std::string& CParaFile::GetWritablePath()
{
	return CFileUtils::GetWritablePath();
}

void CParaFile::SetWritablePath(const std::string& writable_path)
{
	CFileUtils::SetWritablePath(writable_path);
}


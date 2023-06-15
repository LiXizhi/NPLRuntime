//-----------------------------------------------------------------------------
// Class:	ParaIO
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine Dev Studio
// Date:	2005.11
// Desc: partially cross platform.
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "FileManager.h"
#include "FileUtils.h"
#include "ZipArchive.h"
#include "ZipWriter.h"
#ifdef USE_TINYXML2
	#include <tinyxml2.h>
#else
	#include <tinyxml.h>
#endif
#include "AISimulator.h"
#include "AssetManifest.h"
#include "TextureEntity.h"
#include "util/StringHelper.h"
#include "NPLWriter.h"
#include "NPLHelper.h"
#ifndef EMSCRIPTEN_SINGLE_THREAD
#include <boost/thread/tss.hpp>
#endif
#include <vector>
#include <time.h>

#include "util/StringHelper.h"
#include "util/CyoEncode.h"

#include "FileLogger.h"
#ifdef EMSCRIPTEN_SINGLE_THREAD
#define auto_ptr unique_ptr
#endif
extern "C" {
#include "lua.h"
#include "lauxlib.h"
#include "lualib.h"
}

#include <luabind/luabind.hpp>
#include <luabind/object.hpp>
#include "os_calls.h"
#include "ParaScriptingIO.h"

#ifdef EMSCRIPTEN
#include <emscripten.h>
#endif

#ifdef WIN32
#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>
#include <iostream>
namespace fs = boost::filesystem;
#endif

using namespace luabind;

//@def the maximum number of bytes in a text file line.
#define MAX_LINE_CHARACTER_NUM	512

namespace ParaEngine
{
	extern time_t FileTimeToTimet(const FILETIME& ft);
}


namespace ParaScripting
{
	/// the current IO file.
	CParaFile g_currentIOfile;

	ParaIO::ParaIO(void)
	{
	}

	ParaIO::~ParaIO(void)
	{
	}
	void ParaIO::UpdateMirrorFiles( const char* dirName, bool bOverwrite)
	{
#ifdef WIN32
		CFileLogger::GetInstance()->UpdateMirrorFiles(dirName, bOverwrite);
#endif
	}

	bool ParaIO::CopyFile(const char* src, const char* dest, bool bOverride)
	{
		if(dest!=NULL && src!=NULL)
		{
			if (!CParaFile::IsWritablePath(dest))
				return false;
		}
		return CParaFile::CopyFile(src, dest, bOverride);
	}

	bool ParaIO::CreateDirectory(const char* filename)
	{
		return CParaFile::CreateDirectory(filename);
	}
	bool ParaIO::CreateNewFile(const char * filename)
	{
		OUTPUT_LOG("warning: ParaIO::CreateNewFile is absoleted. Use ParaIO.open() instead.\r\n");

		if (!CParaFile::IsWritablePath(filename))
			return false;
		g_currentIOfile.close();
		return g_currentIOfile.CreateNewFile(filename);
	}

	bool ParaIO::OpenFileWrite(const char * filename)
	{
		OUTPUT_LOG("warning: ParaIO::OpenFileWrite is absoleted. Use ParaIO.open() instead.\r\n");

		if (!CParaFile::IsWritablePath(filename))
			return false;

		g_currentIOfile.close();
		return g_currentIOfile.OpenFile(filename, false);
	}

	bool ParaIO::OpenFile(const char * filename)
	{
		OUTPUT_LOG("warning: ParaIO::OpenFile is absoleted. Use ParaIO.open() instead.\r\n");

		if (!CParaFile::IsWritablePath(filename))
			return false;

		g_currentIOfile.close();
		return g_currentIOfile.OpenFile(filename);
	}


	ParaScripting::ParaFileObject ParaIO::openimage2(const char * filename, const char* mode, const object& oExInfo)
	{
		ParaFileObject file;
		if (filename == NULL)
			return file;
		if (!CParaFile::IsWritablePath(filename))
			return file;

		// open the given file according to its file type. currently image files are automatically opened.
		// Load the texture data.
		CParaFile cFile;
		cFile.OpenAssetFile(filename);

		if (!cFile.isEof())
		{
			int texWidth, texHeight, nBytesPerPixel;
			unsigned char *pTextureImage = NULL;
			ImageExtendInfo exInfo;
			if (TextureEntity::LoadImageOfFormatEx(filename, cFile.getBuffer(), (int)cFile.getSize(), texWidth, texHeight, &pTextureImage, &nBytesPerPixel, -1, &exInfo))
			{
				int nSize = texWidth*texHeight*nBytesPerPixel;
				int nHeaderSize = sizeof(DWORD) * 4;
				unsigned char* pFileBuffer = new unsigned char[nSize + nHeaderSize];
				DWORD * pData = (DWORD*)pFileBuffer;
				*pData = 0; pData++;
				*pData = texWidth; pData++;
				*pData = texHeight; pData++;
				*pData = nBytesPerPixel; pData++;
				memcpy(pData, pTextureImage, nSize);

				file.m_pFile.reset(new CParaFile((char*)pFileBuffer, nSize + nHeaderSize, true));

				// must be to delete, because bCopyBuffer is true
				delete[] pFileBuffer;

				file.m_pFile->SetFilePointer(0, FILE_BEGIN);
				file.m_pFile->TakeBufferOwnership();
				SAFE_DELETE_ARRAY(pTextureImage);

				if (type(oExInfo) == LUA_TTABLE)
				{
					oExInfo["FocalLength"] = exInfo.FocalLength;
				}
			}
		}

		return file;
	}

	ParaScripting::ParaFileObject ParaIO::openimage(const object& filename, const char *mode )
	{
		return ParaIO::open(filename, "image");
	}


	ParaFileObject ParaIO::OpenAssetFile(const char* filename)
	{
		return OpenAssetFile2(filename, true);
	}

	ParaFileObject ParaIO::OpenAssetFile2(const char* filename, bool bDownloadIfNotUpToDate)
	{
		ParaFileObject file;
		if(filename == NULL)
			return file;

		file.m_pFile.reset(new CParaFile());
		if (!file.m_pFile->OpenAssetFile(filename, bDownloadIfNotUpToDate))
			file.m_pFile.reset();
		return file;
	}

	bool ParaIO::DoesAssetFileExist(const char* filename)
	{
		return CParaFile::DoesAssetFileExist(filename);
	}

	bool ParaIO::DoesAssetFileExist2(const char* filename, bool bSearchZipFile)
	{
		return CParaFile::DoesAssetFileExist2(filename, bSearchZipFile);
	}

	/** terrain mask file callback data */
	class CAssetScriptCallbackData
	{
	public:
		CAssetScriptCallbackData(const char* sCallbackScript) : m_sCallbackScript(sCallbackScript){}
		string m_sCallbackScript;
		void operator()(int nResult, AssetFileEntry* pAssetFileEntry)
		{
			if(!m_sCallbackScript.empty())
			{
				string filename, scode;
				ParaEngine::StringHelper::DevideString(m_sCallbackScript, filename, scode);
				if(nResult == 0)
				{
					scode = "msg={res=0};"+scode;
				}
				else
				{
					scode = "msg={res=-1};"+scode;
				}
				ParaEngine::CGlobals::GetAISim()->NPLActivate(filename.c_str(), scode.c_str(), (int)(scode.size()));
			}
		}
	};

	int ParaIO::SyncAssetFile_Async(const char* filename, const char* sCallBackScript)
	{
		ParaEngine::AssetFileEntry* pEntry = ParaEngine::CAssetManifest::GetSingleton().GetFile(filename);
		if(pEntry)
		{
			if(pEntry->DoesFileExist())
			{
				return 1;
			}
			else
			{
				// download and async load the file
				if(FAILED(pEntry->SyncFile_Async(CAssetScriptCallbackData(sCallBackScript))))
				{
					return -1;
				}
				return 0;
			}
		}
		else
		{
			return -2;
		}
	}

	int ParaIO::CheckAssetFile(const char* filename)
	{
		ParaEngine::AssetFileEntry* pEntry = ParaEngine::CAssetManifest::GetSingleton().GetFile(filename);
		if(pEntry)
		{
			if(pEntry->IsDownloading())
			{
				return -3;
			}
			else if(pEntry->DoesFileExist())
			{
				return 1;
			}
			else if(pEntry->HasReachedMaxRetryCount())
			{
				return -1;
			}
			else
			{
				return 0;
			}
		}
		else
			return -4;
	}

	void ParaIO::LoadReplaceFile(const char* filename, bool bReplaceExistingOnes)
	{
		ParaEngine::CFileReplaceMap::GetSingleton().LoadReplaceFile(filename, bReplaceExistingOnes);
	}

	ParaFileObject ParaIO::open( const object& obj, const char *mode )
	{
		ParaFileObject file;

		if (type(obj) != LUA_TSTRING)
			return file;

		int len = 0;
		auto filename = NPL::NPLHelper::LuaObjectToString(obj, &len);

		if(filename == NULL)
			return file;
		if (!CParaFile::IsWritablePath(filename))
			return file;

		if (mode[0] == 'r')
		{
			if(mode[1] != 'w')
			{
				file.m_pFile.reset(new CParaFile());
				// 'r', if we have opened an read only file, we will first look in asset manifest.
				file.m_pFile->OpenAssetFile(filename);
				if (file.m_pFile->isEof())
				{
					file.m_pFile.reset();
				}
			}
			else
			{
				// read/write and file pointer at head.
				file.m_pFile.reset(new CParaFile());
				if (file.m_pFile->OpenFile(filename, false))
				{
					file.m_pFile->SetFilePointer(0,FILE_BEGIN);
				}
				else
				{
					file.m_pFile.reset();
				}
			}

		}
		else if (mode[0] == 'w')
		{
			file.m_pFile.reset(new CParaFile());
			if (file.m_pFile->OpenFile(filename, false))
			{
				file.m_pFile->SetFilePointer(0,FILE_BEGIN);
				file.m_pFile->SetEndOfFile();
				file.m_pFile->SetFilePointer(0,FILE_BEGIN);
			}
			else
			{
				file.m_pFile.reset();
			}
		}
		else if (mode[0] == 'a')
		{
			// append to the end of file
			file.m_pFile.reset(new CParaFile());
			if (file.m_pFile->OpenFile(filename, false))
			{
				file.m_pFile->SetFilePointer(0,FILE_END);
			}
			else
			{
				file.m_pFile.reset();
			}
		}
		else if (strcmp(mode, "image") == 0)
		{
			// open the given file according to its file type. currently image files are automatically opened.
			// Load the texture data.
			CParaFile cFile;
			cFile.OpenAssetFile(filename);

			if (!cFile.isEof())
			{
				int texWidth, texHeight, nBytesPerPixel;
				unsigned char *pTextureImage = NULL;
				if (TextureEntity::LoadImageOfFormat(filename, cFile.getBuffer(), (int)cFile.getSize(), texWidth, texHeight, &pTextureImage, &nBytesPerPixel))
				{
					int nSize = texWidth*texHeight*nBytesPerPixel;
					int nHeaderSize = sizeof(DWORD) * 4;
					unsigned char* pFileBuffer = new unsigned char[nSize + nHeaderSize];
					DWORD * pData = (DWORD*)pFileBuffer;
					*pData = 0; pData++;
					*pData = texWidth; pData++;
					*pData = texHeight; pData++;
					*pData = nBytesPerPixel; pData++;
					memcpy(pData, pTextureImage, nSize);

					file.m_pFile.reset(new CParaFile((char*)pFileBuffer, nSize + nHeaderSize, true));

					// must be to delete, because bCopyBuffer is true
					delete[] pFileBuffer;

					file.m_pFile->SetFilePointer(0, FILE_BEGIN);
					file.m_pFile->TakeBufferOwnership();
					SAFE_DELETE_ARRAY(pTextureImage);
				}
			}
		}
		else if (strcmp(mode, "buffer") == 0)
		{
			file.m_pFile.reset(new CParaFile((char*)filename, len, false));
		}

		return file;
	}

	void ParaIO::CloseFile()
	{
		g_currentIOfile.close();
	}

	void ParaIO::WriteString(const char* str)
	{
		if (!g_currentIOfile.isEof())
			g_currentIOfile.WriteString(str);
		else
		{
			// write to stdout
			fputs(str, stdout);
			fflush(stdout);
		}
	}

	void ParaIO::write(const char* buffer, int nSize)
	{
		if (!g_currentIOfile.isEof())
			g_currentIOfile.write(buffer, nSize);
		else
		{
			WriteString(buffer);
		}
	}

	void ParaIO::flush()
	{
#ifdef EMSCRIPTEN
	EM_ASM(FS.syncfs(false, function(err) { if (err) { console.log("FS.syncfs", err); } }););
#endif
	}

	const char* ParaIO::readline()
	{
		// not thread safe
		static char line[MAX_LINE_CHARACTER_NUM];
		if (g_currentIOfile.isEof())
		{
			// read from stdio
			return readline2("");
		}
		else
			g_currentIOfile.GetNextLine(line, MAX_LINE_CHARACTER_NUM-1);
		return line;
	}

	const char* ParaIO::readline2(const char* prompt)
	{
		// read from stdio
		static std::string sLine;
		if (ParaEngine::ReadLine(sLine, prompt))
		{
			return sLine.c_str();
		}
		return NULL;
	}

	bool ParaIO::DoesFileExist(const char * filename, bool bSearchZipFiles)
	{
		return CParaFile::DoesFileExist(filename, bSearchZipFiles, true);
	}

	bool ParaIO::DoesFileExist_(const char * filename)
	{
		return CParaFile::DoesFileExist(filename, false, true);
	}

	bool ParaIO::BackupFile(const char* filename)
	{
		return CParaFile::BackupFile(filename);
	}

	ParaSearchResult ParaIO::SearchFiles(const char* sRootPath, const char* sFilePattern, const char* sZipArchive, int nSubLevel, int nMaxFilesNum, int nFrom)
	{
		if(sRootPath!=NULL && sFilePattern!=NULL && sZipArchive!=NULL)
			return ParaSearchResult(CFileManager::GetInstance()->SearchFiles(sRootPath, sFilePattern, sZipArchive, nSubLevel, nMaxFilesNum, nFrom));
		else
			return ParaSearchResult();
	}
	ParaSearchResult ParaIO::SearchFiles_c(const char* sRootPath, const char* sFilePattern, const char* sZipArchive, int nSubLevel)
	{
		return SearchFiles(sRootPath, sFilePattern, sZipArchive, nSubLevel, 5000, 0);
	}

	/************************************************************************/
	/* ParaSearchResult                                                     */
	/************************************************************************/
	bool ParaSearchResult::IsValid()
	{
		return m_pResult != NULL;
	}

	void ParaSearchResult::Release()
	{
		if(IsValid())
		{
			m_pResult->Release();
		}
	}

	string ParaSearchResult::GetRootPath()
	{
		if(IsValid())
		{
			return m_pResult->GetRootPath();
		}
		return "";
	}

	int ParaSearchResult::GetNumOfResult()
	{
		if(IsValid())
		{
			return m_pResult->GetNumOfResult();
		}
		return 0;
	}

	bool ParaSearchResult::AddResult(const char* sItem)
	{
		if(IsValid())
		{
			return m_pResult->AddResult(sItem);
		}
		return false;
	}

	string ParaSearchResult::GetItem(int nIndex)
	{
		if(IsValid())
		{
			return m_pResult->GetItem(nIndex);
		}
		return "";
	}

	// file time to 1982-11-26 type string.
	string FileTimeToDateString(const FILETIME* pTime)
	{
#ifdef PARAENGINE_CLIENT
		if(pTime->dwHighDateTime==0 && pTime->dwLowDateTime==0)
			return "2000-01-01-00-00";
		SYSTEMTIME stUTC, stLocal;
		char outputTime[256];
		memset(outputTime, 0, sizeof(outputTime));
		// Convert the last-write time to local time.
		FileTimeToSystemTime(pTime, &stUTC);
		SystemTimeToTzSpecificLocalTime(NULL, &stUTC, &stLocal);
		// Build a string showing the date and time.
		snprintf(outputTime, 256, "%d-%d-%d-%d-%d",stLocal.wYear, stLocal.wMonth, stLocal.wDay, stLocal.wHour, stLocal.wMinute);

		return string(outputTime);
#else
		if (pTime->dwHighDateTime == 0 && pTime->dwLowDateTime == 0)
			return "2000-01-01-00-00";
		std::time_t time = ParaEngine::FileTimeToTimet(*pTime);
		auto pGmTime = std::localtime(&time);
		if (pGmTime)
		{
			char outputTime[256];
			memset(outputTime, 0, sizeof(outputTime));
			snprintf(outputTime, 256, "%d-%d-%d-%d-%d", (pGmTime->tm_year+1900), pGmTime->tm_mon+1, pGmTime->tm_mday, pGmTime->tm_hour, pGmTime->tm_min);
			return string(outputTime);
		}
		else
		{
			return "2000-01-01-00-00";
		}
#endif
	}

	object ParaSearchResult::GetItemData( int nIndex, const object& output )
	{
		if(IsValid())
		{
			const ParaEngine::CFileFindData* pData = m_pResult->GetItemData(nIndex);
			if(pData)
			{
				output["filename"] = pData->m_sFileName;
				output["filesize"] = (int)(pData->m_dwFileSize);
				output["fileattr"] = (int)(pData->m_dwFileAttributes);

				output["createdate"] = FileTimeToDateString(&(pData->m_ftCreationTime));
				output["writedate"] = FileTimeToDateString(&(pData->m_ftLastWriteTime));
				output["accessdate"] = FileTimeToDateString(&(pData->m_ftLastAccessTime));
			}
		}
		return object(output);
	}
	//////////////////////////////////////////////////////////////////////////
	//
	// Some file name operations
	//
	//////////////////////////////////////////////////////////////////////////

	string ParaIO::GetParentDirectoryFromPath(const char* sfilename, int nParentCounts)
	{
		if(sfilename != NULL)
			return CParaFile::GetParentDirectoryFromPath(sfilename, nParentCounts);
		else
			return "";
	}

	const char* ParaIO::GetParentDirectoryFromPath_(const char* sfilename, int nParentCounts)
	{
		thread_local static std::string g_str;

		if(sfilename != NULL)
			g_str = CParaFile::GetParentDirectoryFromPath(sfilename, nParentCounts);
		else
			g_str.clear();
		return g_str.c_str();
	}

	string ParaIO::AutoFindParaEngineRootPath(const char* sFile)
	{
		if(sFile == NULL)
			return "";
		return CParaFile::AutoFindParaEngineRootPath(sFile);
	}

	const char* ParaIO::AutoFindParaEngineRootPath_(const char* sFile)
	{
		thread_local static std::string g_str;

		if(sFile != NULL)
			g_str = CParaFile::AutoFindParaEngineRootPath(sFile);
		else
			g_str.clear();
		return g_str.c_str();
	}

	string ParaIO::ChangeFileExtension(const char* sfilename, const string & sExt)
	{
		if(sfilename == NULL)
			return "";
		//TODO: search if the directory is outside the application directory. If so, we should now allow user to delete file there.
		string sFileName = sfilename;
		if(!CParaFile::IsWritablePath(sfilename, false))
		{
			// only relative path is allowed.
			OUTPUT_LOG("security alert: some one is telling the engine to change file extension to a file %s which is not allowed\r\n", sFileName.c_str());
			return "";
		}
		return CParaFile::ChangeFileExtension(sfilename, sExt);
	}
	const char* ParaIO::ChangeFileExtension_(const char* sfilename, const string & sExt)
	{
		thread_local static std::string g_str;

		if(sfilename != NULL)
			g_str = CParaFile::ChangeFileExtension(sfilename, sExt);
		else
			g_str.clear();
		return g_str.c_str();
	}

	string ParaIO::GetFileExtension(const char* sfilename)
	{
		if(sfilename != NULL)
			return CParaFile::GetFileExtension(sfilename);
		else
			return "";
	}

	const char* ParaIO::GetFileExtension_(const char* sfilename)
	{
		thread_local static std::string g_str;

		if(sfilename != NULL)
			g_str = CParaFile::GetFileExtension(sfilename);
		else
			g_str.clear();
		return g_str.c_str();
	}

	string ParaIO::GetAbsolutePath(const char* sRelativePath, const char* sRootPath)
	{
		if(sRelativePath != NULL && sRootPath!=NULL)
			return CParaFile::GetAbsolutePath(sRelativePath, sRootPath);
		else
			return "";
	}

	const char* ParaIO::GetAbsolutePath_(const char* sRelativePath, const char* sRootPath)
	{
		thread_local static std::string g_str;

		if(sRelativePath != NULL && sRootPath!=NULL)
			g_str = CParaFile::GetAbsolutePath(sRelativePath, sRootPath);
		else
			g_str.clear();
		return g_str.c_str();
	}

	string ParaIO::GetRelativePath(const char* sAbsolutePath, const char* sRootPath)
	{
		if(sAbsolutePath != NULL && sRootPath!=NULL)
			return CParaFile::GetRelativePath(sAbsolutePath, sRootPath);
		else
			return "";
	}

	const char* ParaIO::GetRelativePath_(const char* sAbsolutePath, const char* sRootPath)
	{
		thread_local static std::string g_str;

		if(sAbsolutePath != NULL && sRootPath!=NULL)
			g_str = CParaFile::GetRelativePath(sAbsolutePath, sRootPath);
		else
			g_str.clear();
		return g_str.c_str();
	}

	string ParaIO::GetFileName(const char* sfilename)
	{
		if(sfilename != NULL)
			return CParaFile::GetFileName(sfilename);
		else
			return "";
	}
	const char* ParaIO::GetFileName_(const char* sfilename)
	{
		thread_local static std::string g_str;

		if(sfilename != NULL)
			g_str = CParaFile::GetFileName(sfilename);
		else
			g_str.clear();
		return g_str.c_str();
	}

	string ParaIO::GetFileOriginalName(const char* sfilename)
	{
		return CFileManager::GetInstance()->GetFileOriginalName(sfilename);
	}

	string ParaIO::ToCanonicalFilePath(const char* sfilename, bool bBackSlash)
	{
		if(sfilename != NULL)
		{
			string output;
			CParaFile::ToCanonicalFilePath(output, sfilename, bBackSlash);
			return output;
		}
		else
			return "";
	}

	const char* ParaIO::ToCanonicalFilePath__(const char* sfilename, bool bBackSlash)
	{
		thread_local static std::string g_str;

		if(sfilename != NULL)
		{
			CParaFile::ToCanonicalFilePath(g_str, sfilename, bBackSlash);
		}
		else
			g_str.clear();
		return g_str.c_str();
	}

	string ParaIO::GetCurDirectory(DWORD dwDirectoryType)
	{
		return CParaFile::GetCurDirectory(dwDirectoryType);
	}

	const char* ParaIO::GetCurDirectory_(DWORD dwDirectoryType)
	{
		thread_local static std::string g_str;


		g_str = CParaFile::GetCurDirectory(dwDirectoryType);

		return g_str.c_str();
	}

	int ParaIO::GetFileSize(const char* sFilePath)
	{
		if(sFilePath == NULL)
			return 0;

		return CParaFile::GetFileSize(sFilePath);
	}

	bool ParaIO::GetFileInfo(const char* sFilePath, const object& inout)
	{
		CParaFileInfo fileInfo;
		if (CParaFile::GetFileInfo(sFilePath, fileInfo))
		{
			if (type(inout) == LUA_TTABLE)
			{
				inout["size"] = (int)fileInfo.m_dwFileSize;
				if (fileInfo.m_mode == CParaFileInfo::ModeFile)
					inout["mode"] = "file";
				else if (fileInfo.m_mode == CParaFileInfo::ModeDirectory)
					inout["mode"] = "directory";
				else if (fileInfo.m_mode == CParaFileInfo::ModeFileInZip)
					inout["mode"] = "fileinzip";
				else
					inout["mode"] = "";

				inout["modification"] = (double)((int64)fileInfo.m_ftLastWriteTime);
				inout["create"] = (double)((int64)fileInfo.m_ftCreationTime);
				inout["access"] = (double)((int64)fileInfo.m_ftLastAccessTime);
				inout["fullpath"] = fileInfo.m_sFullpath;
				inout["attr"] = (int)fileInfo.m_dwFileAttributes;
			}
			return true;
		}
		return false;
	}

	int ParaIO::DeleteFile(const char* sFilePattern)
	{
		if(sFilePattern!=0)
		{
			return CParaFile::DeleteFile(sFilePattern);
		}
		return 0;
	}

	bool ParaIO::MoveFile( const char* src, const char* dest )
	{
		if(dest!=NULL && src!=NULL)
		{
			string sDestPath = dest;
			//TODO: search if the directory is outside the application directory. If so, we should now allow user to delete file there.
			if(!CParaFile::IsWritablePath(sDestPath))
			{
				// only relative path is allowed.
				OUTPUT_LOG("security alert: some one is telling the engine to move a file to %s which is not allowed\r\n", sDestPath.c_str());
				return false;
			}
			

			string sSrcPath = dest;
			//TODO: search if the directory is outside the application directory. If so, we should now allow user to delete file there.
			if (!CParaFile::IsWritablePath(sSrcPath))
			{
				// only relative path is allowed.
				OUTPUT_LOG("security alert: some one is telling the engine to move a file from %s which is not allowed\r\n", sSrcPath.c_str());
				return false;
			}
			return CParaFile::MoveFile(src, dest);
		}
		return false;
	}

	unsigned long ParaIO::CRC32( const char* filename )
	{
		if(filename == NULL)
			return 0;
		return CParaFile::CRC32(filename);
	}

	ParaScripting::ParaZipWriter ParaIO::CreateZip( const char *fn, const char *password )
	{
		return ParaScripting::ParaZipWriter(CZipWriter::CreateZip(fn, password));
	}

	void ParaIO::SetDiskFilePriority( int nPriority )
	{
		CParaFile::SetDiskFilePriority(nPriority);
	}

	int ParaIO::GetDiskFilePriority()
	{
		return CParaFile::GetDiskFilePriority();
	}

	bool ParaIO::AddSearchPath( const char* sFile )
	{
		return AddSearchPath2(sFile, 0);
	}

	bool ParaIO::AddSearchPath2( const char* sFile, int nPriority )
	{
		return CFileManager::GetInstance()->AddSearchPath(sFile, nPriority);
	}

	bool ParaIO::RemoveSearchPath( const char* sFile )
	{
		return CFileManager::GetInstance()->RemoveSearchPath(sFile);
	}

	bool ParaIO::ClearAllSearchPath()
	{
		return CFileManager::GetInstance()->ClearAllSearchPath();
	}

	const char* ParaIO::DecodePath( const char* input )
	{
		thread_local static std::string g_str;
		if(input)
		{
			CPathReplaceables::GetSingleton().DecodePath(g_str, input);
			return g_str.c_str();
		}
		else
			return NULL;
	}

	const char* ParaIO::EncodePath( const char* input )
	{
		thread_local static std::string g_str;
		if(input)
		{
			CPathReplaceables::GetSingleton().EncodePath(g_str, input);
			return g_str.c_str();
		}
		else
			return NULL;
	}

	const char* ParaIO::EncodePath2(const char* input, const char* varNames)
	{
		if(varNames == NULL)
			return EncodePath(input);
		else if(input)
		{
		thread_local static std::string g_str;

			CPathReplaceables::GetSingleton().EncodePath(g_str, input, varNames);
			return g_str.c_str();
		}
		else
			return NULL;
	}

	bool ParaIO::AddPathVariable( const char * sVarName, const char * sVarValue )
	{
		if(sVarName)
		{
			if(sVarValue)
				return CPathReplaceables::GetSingleton().AddVariable(CPathVariable(sVarName, sVarValue, true, true));
			else
				return CPathReplaceables::GetSingleton().RemoveVariable(sVarName, true);
		}
		else
			return false;
	}

	ParaScripting::ParaFileSystemWatcher ParaIO::GetFileSystemWatcher( const char* filename )
	{
#if defined(PARAENGINE_MOBILE) || defined(EMSCRIPTEN_SINGLE_THREAD)
		return ParaFileSystemWatcher();
#else
		return ParaFileSystemWatcher(CFileSystemWatcherService::GetInstance()->GetDirWatcher(filename).get());
#endif
	}

	void ParaIO::DeleteFileSystemWatcher( const char* name )
    {
#if !defined(PARAENGINE_MOBILE) && !defined(EMSCRIPTEN_SINGLE_THREAD)
        CFileSystemWatcherService::GetInstance()->DeleteDirWatcher(name);
#endif
	}

	const std::string& ParaIO::GetWritablePath()
	{
		return CParaFile::GetWritablePath();
	}

	std::string ParaIO::ConvertPathFromUTF8ToAnsci(const char* path)
	{
#if WIN32 && defined(DEFAULT_FILE_ENCODING)
		LPCWSTR path16 = StringHelper::MultiByteToWideChar(path, DEFAULT_FILE_ENCODING);
		fs::path pathStr(path16);
		std::string ret = pathStr.string();
		return ret.c_str();
#else
		return path;
#endif
	}


	//////////////////////////////////////////////////////////////////////////
	//
	// ParaFileObject
	//
	//////////////////////////////////////////////////////////////////////////
	ParaFileObject::~ParaFileObject()
	{
		close();
	}

	void ParaFileObject::close()
	{
		m_pFile.reset();
	}

	void ParaFileObject::writeline(const char* str)
	{
		if (IsValid())
		{
			m_pFile->WriteString(str);
			m_pFile->WriteString("\n");
		}
	}

	void ParaFileObject::WriteString(const char* str)
	{
		if(IsValid())
		{
			m_pFile->WriteString(str);
		}
	}

	const char* ParaFileObject::readline()
	{
		if(IsValid())
		{
			if(m_pFile->isEof())
				return NULL;
			else
			{
				if(m_sTempBuffer.size()<MAX_LINE_CHARACTER_NUM)
					m_sTempBuffer.resize(MAX_LINE_CHARACTER_NUM);

				int nFromIndex = 0;
				while (m_pFile->GetNextLine(&(m_sTempBuffer[nFromIndex]), (int)m_sTempBuffer.size() - nFromIndex) == ((int)m_sTempBuffer.size() - nFromIndex - 1))
				{
					// have not finished.
					if(m_pFile->isEof())
						break;
					char c = *m_pFile->getPointer();

					if((c != '\r') || (c != '\n'))
					{
						// have not finished last line.
						nFromIndex = (int)m_sTempBuffer.size() - 1;
						m_sTempBuffer.resize(m_sTempBuffer.size() + MAX_LINE_CHARACTER_NUM);
					}
				}
				return &(m_sTempBuffer[0]);
			}
		}
		else
			return NULL;
	}


	const char* ParaFileObject::GetText()
	{
		if (IsValid())
		{
			const std::string& output = GetText2(0, -1);
			const char* text = output.c_str();
			if (text)
			{
				// https://en.wikipedia.org/wiki/Byte_order_mark
				// encoding is escaped.
				if ((((unsigned char)text[0]) == 0xEF) && (((unsigned char)text[1]) == 0xBB) && (((unsigned char)text[2]) == 0xBF))
				{
					// UTF-8[t 1]	EF BB BF
					text += 3;
				}
				else if ( ((((unsigned char)text[0]) == 0xFF) && (((unsigned char)text[1]) == 0xFE)) || ((((unsigned char)text[0]) == 0xFE) && (((unsigned char)text[1]) == 0xFF)))
				{
					// UTF - 16 (BigEndian)    FE FF
					// UTF - 16 (LittleEndian) FF FE
					text += 2;
					std::u16string input;
					input.resize((output.size() - 2) / 2);
					memcpy((char*)(&(input[0])), text, input.size() * 2);
					
					m_sTempBuffer.clear();
					if (StringHelper::UTF16ToUTF8(input, m_sTempBuffer))
					{
						return m_sTempBuffer.c_str();
					}
				}
			}
			return text;
		}
		else
			return NULL;
	}

	const std::string& ParaFileObject::GetText2(int fromPos, int nCount)
	{
		// this is now thread-safe and multiple instance can be used at the same time
		m_sTempBuffer.clear();
		if(IsValid())
		{
			int nSize = (int)m_pFile->getSize();
			if(nCount <0)
				nCount = nSize - fromPos;
			if(nCount>0)
			{
				m_sTempBuffer.resize(nCount);
				int nOldPos = m_pFile->getPos();
				m_pFile->seek(fromPos);
				m_pFile->read((char*)(&(m_sTempBuffer[0])), nCount);
				m_pFile->seek(nOldPos);
			}
		}
		return m_sTempBuffer;
	}

	object ParaFileObject::ReadString(int nCount, lua_State* L)
	{
		// this is now thread-safe and multiple instance can be used at the same time
		if (IsValid())
		{
			int fromPos = (int)m_pFile->getPos();

			int nSize = (int)m_pFile->getSize();
			if (nCount < 0)
				nCount = nSize - fromPos;
			else if (nCount > nSize - fromPos)
			{
				nCount = nSize - fromPos;
			}

			if (nCount > 0)
			{
				m_pFile->seek(fromPos + nCount);
	
				lua_pushlstring(L, m_pFile->getBuffer() + fromPos, nCount);
				object o(from_stack(L, -1));
				lua_pop(L, 1);

				return o;
			}
		}
		return object(L, "");
	}

	void ParaFileObject::WriteString2(const char* buffer, int nSize)
	{
		return write(buffer, nSize);
	}

	void ParaFileObject::write(const char* buffer, int nSize)
	{
		if(IsValid())
		{
			m_pFile->write(buffer, nSize);
		}
	}

	void ParaFileObject::seek( int offset )
	{
		if(IsValid())
		{
			m_pFile->seek(offset);
		}
	}

	void ParaFileObject::seekRelative( int offset )
	{
		if(IsValid())
		{
			m_pFile->seekRelative(offset);
		}
	}

	int ParaFileObject::getpos()
	{
		if (IsValid())
		{
			return m_pFile->getPos();
		}
		return 0;
	}

	void ParaFileObject::SetFilePointer(int lDistanceToMove, int dwMoveMethod)
	{
		if(IsValid())
		{
			m_pFile->SetFilePointer(lDistanceToMove, dwMoveMethod);
		}
	}

	bool ParaFileObject::SetEndOfFile()
	{
		if(IsValid())
		{
			return m_pFile->SetEndOfFile();
		}
		return true;
	}

	int ParaFileObject::WriteBytes( int nSize, const object& input )
	{
		if(IsValid())
		{
			std::vector<unsigned char> data;
			data.resize(nSize);
			// convert script array object to vector unsigned char array.
			for (int i=0; i<nSize; ++i)
			{
				int value = object_cast<int>(input[i+1]);
				data[i] = (unsigned char)value;
			}

			if(nSize>0)
			{
				return m_pFile->write(&(data[0]), nSize);
			}
		}
		return 0;
	}

	object ParaFileObject::ReadBytes( int nSize, const object& output )
	{
		if(IsValid())
		{
			std::string data;
			if(nSize <= 0)
			{
				nSize = GetFileSize();
			}
			data.resize(nSize);

			nSize = (int)m_pFile->read(&(data[0]), nSize);

			if(type(output) == LUA_TTABLE)
			{
				// convert vector unsigned char array to script array object.
				for (int i=0; i<nSize; ++i)
				{
					output[i+1] = (int)((unsigned char)(data[i]));
				}
			}
			else
			{
				return object(output.interpreter(), data);
			}
		}
		return object(output);
	}

	void ParaFileObject::WriteFloat(float data)
	{
		if(IsValid())
		{
			m_pFile->write(&data, 4);
		}
	}
	float ParaFileObject::ReadFloat()
	{
		if(IsValid())
		{
			float data;
			m_pFile->read(&data, 4);
			return data;
		}
		return 0.f;
	}

	void ParaFileObject::WriteWord(int value)
	{
		if (IsValid())
		{
			if (value >= 0)
			{
				uint16 data_ = value;
				m_pFile->write(&data_, 2);
			}
			else
			{
				int16 data_ = (int16)value;
				m_pFile->write(&data_, 2);
			}
		}
	}

	int ParaFileObject::ReadWord()
	{
		if (IsValid())
		{
			uint16 data;
			m_pFile->read(&data, 2);
			return data;
		}
		return 0;
	}

	void ParaFileObject::WriteDouble(double data)
	{
		if (IsValid())
		{
			m_pFile->write(&data, 8);
		}
	}

	double ParaFileObject::ReadDouble()
	{
		if (IsValid())
		{
			double data;
			m_pFile->read(&data, 8);
			return data;
		}
		return 0;
	}

	void ParaFileObject::WriteInt(int data)
	{
		if(IsValid())
		{
			int32 data_ = (int32)data;
			m_pFile->write(&data_, 4);
		}
	}
	int ParaFileObject::ReadInt()
	{
		if(IsValid())
		{
			int32 data;
			m_pFile->read(&data, 4);
			return data;
		}
		return 0;
	}

	void ParaFileObject::WriteUShort(unsigned short value)
	{
		if (IsValid())
		{
			m_pFile->write(&value, 2);
		}
	}

	unsigned short ParaFileObject::ReadUShort()
	{
		if (IsValid())
		{
			unsigned short data;
			m_pFile->read(&data, 2);
			return data;
		}
		return 0;
	}

	void ParaFileObject::WriteChar(char value)
	{
		if (IsValid())
		{
			m_pFile->write(&value, 1);
		}
	}

	char ParaFileObject::ReadChar()
	{
		if (IsValid())
		{
			char data;
			m_pFile->read(&data, 1);
			return data;
		}
		return 0;
	}


	void ParaFileObject::WriteByte(unsigned char value)
	{
		if (IsValid())
		{
			m_pFile->write(&value, 1);
		}
	}

	unsigned char ParaFileObject::ReadByte()
	{
		if (IsValid())
		{
			unsigned char data;
			m_pFile->read(&data, 1);
			return data;
		}
		return 0;
	}

	void ParaFileObject::WriteShort(int value)
	{
		if (IsValid())
		{
			int16 data_ = (int16)value;
			m_pFile->write(&data_, 2);
		}
	}

	int ParaFileObject::ReadShort()
	{
		if (IsValid())
		{
			int16 data;
			m_pFile->read(&data, 2);
			return data;
		}
		return 0;
	}

	void ParaFileObject::WriteUInt(unsigned int value)
	{
		if (IsValid())
		{
			uint32 data_ = (uint32)value;
			m_pFile->write(&data_, 4);
		}
	}

	unsigned int ParaFileObject::ReadUInt()
	{
		if (IsValid())
		{
			uint32 data;
			m_pFile->read(&data, 4);
			return data;
		}
		return 0;
	}

	int ParaFileObject::GetFileSize()
	{
		if(IsValid())
		{
			return (int)(m_pFile->getSize());
		}
		return 0;
	}

	int ParaFileObject::SetSegment( int nFromByte, int nByteCount )
	{
		m_nFromByte = nFromByte;
		m_nByteCount = nByteCount;
		return -1;
	}

	const char* ParaFileObject::GetBase64StringEx( int * pnStrLength/*=0*/ )
	{
		if(IsValid())
		{
			unsigned long nSrcSize = (m_nByteCount>0 && (m_nFromByte + m_nByteCount) <= (int)m_pFile->getSize()) ? m_nByteCount : ((int)m_pFile->getSize() - m_nFromByte);
			if(nSrcSize == 0)
			{
				return CGlobals::GetString(0).c_str();
			}
			int nBufferSize = CyoEncode::Base64EncodeGetLength(nSrcSize);
			const int MAX_BINARY_DATA_SIZE = 1024000;
			if(nBufferSize<MAX_BINARY_DATA_SIZE)
			{
				if((int)m_sTempBuffer.size()<(nBufferSize+10))
				{
					m_sTempBuffer.resize(nBufferSize+10);
				}

				int filledsize = (int)CyoEncode::Base64Encode(&(m_sTempBuffer[0]), m_pFile->getBuffer()+m_nFromByte, nSrcSize);
				if (filledsize >= 0 && filledsize < (int)m_sTempBuffer.size())
				{
					m_sTempBuffer[filledsize] = '\0';
					if(pnStrLength!=0)
					{
						(*pnStrLength) = filledsize;
					}

					return &(m_sTempBuffer[0]);
				}
				else
				{
					OUTPUT_LOG("warning: the result of CyoEncode::Base64Encode is invalid \n");
				}

			}
			else
			{
				OUTPUT_LOG("warning: the result of ParaFileObject::GetBase64StringEx is longer than the maximum of %s bytes\n", MAX_BINARY_DATA_SIZE);
			}
		}
		return NULL;
	}

	const char* ParaFileObject::GetBase64String()
	{
		return GetBase64StringEx(NULL);
	}

	bool ParaFileObject::IsValid()
	{
		return m_pFile.get() != 0;
	}

	//////////////////////////////////////////////////////////////////////////
	//
	// class ParaZipWriter
	//
	//////////////////////////////////////////////////////////////////////////
	ParaZipWriter::ParaZipWriter()
	{

	}

	ParaZipWriter::ParaZipWriter(CZipWriter * writer)
		:m_writer(writer)
	{
	}

	DWORD ParaZipWriter::ZipAddData(const char* dstzn, const std::string& buff)
	{
		if (m_writer)
		{
			auto pFile = new ParaEngine::CParaFile((char*)buff.c_str(), buff.size(), true);
			return m_writer->ZipAdd(dstzn, pFile);
		}
		else
			return -1;
	}

	DWORD ParaZipWriter::ZipAdd( const char* dstzn, const char* fn )
	{
		if(m_writer)
			return m_writer->ZipAdd(dstzn, fn);
		else
			return -1;
	}

	DWORD ParaZipWriter::ZipAddFolder( const char* dstzn )
	{
		if(m_writer)
			return m_writer->ZipAddFolder(dstzn);
		else
			return -1;
	}

	DWORD ParaZipWriter::AddDirectory( const char* dstzn, const char* filepattern, int nSubLevel/*=0*/ )
	{
		if(m_writer)
			return m_writer->AddDirectory(dstzn, filepattern, nSubLevel);
		else
			return -1;
	}

	DWORD ParaZipWriter::close()
	{
		if(m_writer)
		{
			m_writer.reset();
			return 0;
		}
		else
			return -1;
	}

	bool ParaZipWriter::IsValid()
	{
		return m_writer && (m_writer->IsValid());
	}


	//////////////////////////////////////////////////////////////////////////
	//
	// lua xml
	//
	//////////////////////////////////////////////////////////////////////////

	/** Produces an XMLTree like :
	<paragraph justify='centered'>first child<b>bold</b>second child</paragraph>

	{name="paragraph", attr={justify="centered"},
	"first child",
	{name="b", "bold", n=1}
	"second child",
	n=3
	}

	comments and other definitions are ignored
	*/
#ifdef USE_TINYXML2
	void ParaXML::LuaXML_ParseNode(lua_State *L, void* pNode_)
	{
		if (!pNode_) return;
		using namespace tinyxml2;
		XMLNode* pNode = (XMLNode*)pNode_;

		// resize stack if necessary
		// LXZ 2008.9.6. this is changed from 5 to 10, since I have added unknown type nodes, which need some more stack.
		luaL_checkstack(L, 10, "LuaXML_ParseNode : recursion too deep");

		XMLElement* pElem = pNode->ToElement();
		if (pElem) {
			// element name
			lua_pushstring(L, "name");
			lua_pushstring(L, pElem->Value());
			lua_settable(L, -3);
			//lua_setfield(L,-2,"name");

			// parse attributes
			const XMLAttribute* pAttr = pElem->FirstAttribute();
			if (pAttr) {
				lua_pushstring(L, "attr");
				lua_newtable(L);
				for (; pAttr; pAttr = pAttr->Next()) {
					lua_pushstring(L, pAttr->Name());
					// LXZ: convert UTF8 to ANSI.
					//lua_pushstring(L,ParaEngine::StringHelper::UTF8ToAnsi(pAttr->ValueStr().c_str()));
					lua_pushstring(L, pAttr->Value());

					//lua_setfield(L,-2,pAttr->Name());
					lua_settable(L, -3);

				}
				//lua_setfield(L,-2,"attr");
				lua_settable(L, -3);
			}
		}

		// children
		XMLNode *pChild = pNode->FirstChild();
		if (pChild) {
			int iChildCount = 0;
			for (; pChild; pChild = pChild->NextSibling())
			{
				if (pChild->ToElement())
				{
					// normal element, parse recursive
					lua_newtable(L);
					LuaXML_ParseNode(L, pChild);
					lua_rawseti(L, -2, ++iChildCount);
				}
				else if (pChild->ToText())
				{
					// plaintext, push raw
					// LXZ: convert UTF8 to ANSI.
					//lua_pushstring(L,ParaEngine::StringHelper::UTF8ToAnsi(pChild->ValueStr().c_str()));
					lua_pushstring(L, pChild->Value());
					lua_rawseti(L, -2, ++iChildCount);
				}
				else if (pChild->ToUnknown())
				{
					// LiXizhi 2008.5.16
					// this will handle all unknown tags inside <>, such as <!...> <?...> <%...%>
					// it will return text inside <>, such as !..., ?..., %...
					// the result is returned inside a NPL table {name="unknown", [1] = "value", n=1}
					lua_newtable(L);
					{
						// element name
						lua_pushstring(L,"name");
						lua_pushstring(L,"unknown");
						lua_settable(L,-3);

						// output value in child[1]
						// convert UTF8 to ANSI.
						//lua_pushstring(L,ParaEngine::StringHelper::UTF8ToAnsi(pChild->ValueStr().c_str()));
						lua_pushstring(L,pChild->Value());
						lua_rawseti(L,-2, 1);
#ifdef OUTPUT_XML_WITH_TABLE_N
						// set n = 1
						lua_pushstring(L,"n");
						lua_pushnumber(L, 1);
						lua_settable(L,-3);
#endif
					}
					lua_rawseti(L,-2,++iChildCount);
				}
			}
#ifdef OUTPUT_XML_WITH_TABLE_N
			lua_pushstring(L,"n");
			lua_pushnumber(L,iChildCount);
			lua_settable(L, -3);
#endif
		}
	}
#else
	void ParaXML::LuaXML_ParseNode (lua_State *L,void* pNode_) {
		if (!pNode_) return;

		TiXmlNode* pNode = (TiXmlNode*)pNode_;

		// resize stack if necessary
		// LXZ 2008.9.6. this is changed from 5 to 10, since I have added unknown type nodes, which need some more stack.
		luaL_checkstack(L, 10, "LuaXML_ParseNode : recursion too deep");

		TiXmlElement* pElem = pNode->ToElement();
		if (pElem) {
			// element name
			lua_pushstring(L,"name");
			lua_pushstring(L,pElem->Value());
			lua_settable(L,-3);
			//lua_setfield(L,-2,"name");

			// parse attributes
			TiXmlAttribute* pAttr = pElem->FirstAttribute();
			if (pAttr) {
				lua_pushstring(L,"attr");
				lua_newtable(L);
				for (;pAttr;pAttr = pAttr->Next()) {
					lua_pushstring(L,pAttr->Name());
					// LXZ: convert UTF8 to ANSI.
					//lua_pushstring(L,ParaEngine::StringHelper::UTF8ToAnsi(pAttr->ValueStr().c_str()));
					lua_pushstring(L,pAttr->ValueStr().c_str());

					//lua_setfield(L,-2,pAttr->Name());
					lua_settable(L,-3);

				}
				//lua_setfield(L,-2,"attr");
				lua_settable(L,-3);
			}
		}

		// children
		TiXmlNode *pChild = pNode->FirstChild();
		if (pChild) {
			int iChildCount = 0;
			for(;pChild;pChild = pChild->NextSibling()) {
				switch (pChild->Type()) {
					case TiXmlNode::DOCUMENT: break;
					case TiXmlNode::ELEMENT:
						// normal element, parse recursive
						lua_newtable(L);
						LuaXML_ParseNode(L,pChild);
						lua_rawseti(L,-2,++iChildCount);
						break;
					case TiXmlNode::COMMENT: break;
					case TiXmlNode::TEXT:
						// plaintext, push raw

#ifdef _DEBUG
						//{
						//	// find where the UTF8 char and convert them to default NPL encoding
						//	bool bNeedUTF8 = false;
						//
						//	int nSize = (int)pChild->ValueStr().size();
						//	for (int i=0; i<nSize; ++i)
						//	{
						//		if(bNeedUTF8 || str[i]=='\0' || str[i] >= 127)
						//		{
						//			bNeedUTF8 = true;
						//			OUTPUT_LOG("\\U+%d", (int)str[i]);
						//			//break;
						//		}
						//	}
						//
						//	OUTPUT_LOG("%s\n", str.c_str());
						//	if(bNeedUTF8)
						//	{
						//		OUTPUT_LOG("UTF8 non ANSI detected.");
						//	}
						//	OUTPUT_LOG("Length: %d \n", nSize);
						//}

#endif
						// LXZ: convert UTF8 to ANSI.
						//lua_pushstring(L,ParaEngine::StringHelper::UTF8ToAnsi(pChild->ValueStr().c_str()));
						lua_pushstring(L,pChild->ValueStr().c_str());
						lua_rawseti(L,-2,++iChildCount);
						break;
					case TiXmlNode::DECLARATION: break;
					case TiXmlNode::UNKNOWN:
						{
							// LiXizhi 2008.5.16
							// this will handle all unknown tags inside <>, such as <!...> <?...> <%...%>
							// it will return text inside <>, such as !..., ?..., %...
							// the result is returned inside a NPL table {name="unknown", [1] = "value", n=1}
							lua_newtable(L);
							{
								// element name
								lua_pushstring(L,"name");
								lua_pushstring(L,"unknown");
								lua_settable(L,-3);

								// output value in child[1]
								// convert UTF8 to ANSI.
								//lua_pushstring(L,ParaEngine::StringHelper::UTF8ToAnsi(pChild->ValueStr().c_str()));
								lua_pushstring(L,pChild->ValueStr().c_str());
								lua_rawseti(L,-2, 1);
#ifdef OUTPUT_XML_WITH_TABLE_N
								// set n = 1
								lua_pushstring(L,"n");
								lua_pushnumber(L, 1);
								lua_settable(L,-3);
#endif
							}
							lua_rawseti(L,-2,++iChildCount);
							break;
						}
				};
			}
#ifdef OUTPUT_XML_WITH_TABLE_N
			lua_pushstring(L,"n");
			lua_pushnumber(L,iChildCount);
			lua_settable(L, -3);
#endif
		}
	}
#endif
	int ParaXML::LuaXML_ParseFile (lua_State *L) {
		const char* sFileName = luaL_checkstring(L,1);

		int nResult = 0;

		// assume it is UTF-8?, or use TIXML_DEFAULT_ENCODING if one want it to be automatically determined from data
		CParaFile file;
		file.OpenAssetFile(sFileName);
		if(!file.isEof())
		{
			try
			{
				std::string sCode;
				std::string* pData = &sCode;
				std::string uncompressedData;
				if((int)file.getSize()> 0)
				{
					sCode.resize((int)file.getSize());
					file.read(&(sCode[0]), (int)file.getSize());
				}

				if (ParaEngine::IsZipData(sCode.c_str(), sCode.size()))
				{
					if (ParaEngine::GetFirstFileData(sCode.c_str(), sCode.size(), uncompressedData))
					{
						pData = &uncompressedData;
					}
					else
					{
						return nResult;
					}
				}

#ifdef USE_TINYXML2
				namespace TXML = tinyxml2;
				TXML::XMLDocument doc(true, TXML::COLLAPSE_WHITESPACE);
				doc.Parse(pData->c_str(), (int)pData->size());
				if(doc.Error())
				{
					OUTPUT_LOG("error: failed parsing xml file : %s. error %d msg:%s, %s \n", sFileName, (int)doc.ErrorID(), doc.GetErrorStr1(), doc.GetErrorStr2());
					return nResult;
				}
#else
				TiXmlDocument doc;
				doc.Parse(pData->c_str(), 0, TIXML_ENCODING_UTF8);
#endif
				lua_newtable(L);
				LuaXML_ParseNode(L,&doc);
				nResult = 1;
			}
			catch (...)
			{
				OUTPUT_LOG("error: failed parsing xml file : %s \n", sFileName);
			}
		}
		return nResult;
	}

	int ParaXML::LuaXML_ParseString (lua_State *L) {
		const char* sString = nullptr;
		int len = -1;

		if (lua_isstring(L, 1))
		{
			sString = lua_tostring(L, 1);
			len = lua_strlen(L, 1);
		}
		else
		{
			return 0;
		}

		std::string uncompressedData;
		int nResult = 0;

		try
		{
			if (len > 0 && ParaEngine::IsZipData(sString, len))
			{
				if (ParaEngine::GetFirstFileData(sString, len, uncompressedData))
				{
					sString = uncompressedData.c_str();
				}
				else
				{
					return nResult;
				}
			}


#ifdef USE_TINYXML2
			namespace TXML = tinyxml2;
			TXML::XMLDocument doc(true, IsWhiteSpaceCondensed() ? TXML::COLLAPSE_WHITESPACE : TXML::PRESERVE_WHITESPACE);
			// Note: LiXizhi: TinyXML2 will actually change the content of input string, so we can not pass sString directly to Parse() function.
			std::string sTemp = sString;

			// assume it is UTF-8?, or use TIXML_DEFAULT_ENCODING if one want it to be automatically determined from data
			doc.Parse(sTemp.c_str(), -1);
			if(doc.Error())
			{
				OUTPUT_LOG("error: failed parsing xml string: %s. error %d msg:%s, %s \n", sString, (int)doc.ErrorID(), doc.GetErrorStr1(), doc.GetErrorStr2());
				return nResult;
			}
#else
			TiXmlDocument doc;
			// assume it is UTF-8?, or use TIXML_DEFAULT_ENCODING if one want it to be automatically determined from data
			doc.Parse(sString,0,TIXML_ENCODING_UTF8);
#endif
			lua_newtable(L);
			LuaXML_ParseNode(L,&doc);
			nResult = 1;
		}
		catch (...)
		{
			OUTPUT_LOG("error: failed parsing xml string: %s \n", sString);
		}
		return nResult;
	}


	static bool s_bIsWhiteSpaceCollapsed = true;
	void ParaXML::SetCondenseWhiteSpace( bool condense )
	{
#ifdef USE_TINYXML2
		s_bIsWhiteSpaceCollapsed = condense;
#else
		TiXmlBase::SetCondenseWhiteSpace(condense);
#endif
	}

	bool ParaXML::IsWhiteSpaceCondensed()
	{
#ifdef USE_TINYXML2
		return s_bIsWhiteSpaceCollapsed;
#else
		return TiXmlBase::IsWhiteSpaceCondensed();
#endif
	}

	//////////////////////////////////////////////////////////////////////////
	//
	// class ParaFileSystemWatcher
	//
	//////////////////////////////////////////////////////////////////////////

	void ParaFileSystemWatcher::AddDirectory( const char* filename )
	{
#if !defined(PARAENGINE_MOBILE) && !defined(EMSCRIPTEN_SINGLE_THREAD)
		if (m_watcher)
			m_watcher->add_directory(filename);
#endif
	}

	void ParaFileSystemWatcher::RemoveDirectory( const char* filename )
	{
#if !defined(PARAENGINE_MOBILE) && !defined(EMSCRIPTEN_SINGLE_THREAD)
		if (m_watcher)
			m_watcher->remove_directory(filename);
#endif
	}
#if !defined(PARAENGINE_MOBILE) && !defined(EMSCRIPTEN_SINGLE_THREAD)
	/** callback to npl runtime */
	struct FileSystemWatcher_NPLCallback
	{
	public:
		FileSystemWatcher_NPLCallback(const std::string& sCallback):m_sCallbackScript(sCallback){};

		void operator()(const ParaEngine::CFileSystemWatcher::DirMonitorEvent& event)
		{
			NPL::CNPLWriter writer;
			writer.WriteName("msg");
			writer.BeginTable();

			writer.WriteName("type");
			writer.WriteValue((int)event.type);
			writer.WriteName("dirname");
			writer.WriteValue(event.path.parent_path().generic_string() + "/");
			writer.WriteName("filename");
			writer.WriteValue(event.path.filename().generic_string());

			writer.EndTable();
			writer.WriteParamDelimiter();

			ParaEngine::CGlobals::GetAISim()->AddNPLCommand(writer.ToString() + m_sCallbackScript);
		}
	private:
		std::string m_sCallbackScript;
	};
#endif
	void ParaFileSystemWatcher::AddCallback( const char* sCallbackScript )
	{
#if !defined(PARAENGINE_MOBILE) && !defined(EMSCRIPTEN_SINGLE_THREAD)
		if (m_watcher)
			m_watcher->AddEventCallback(FileSystemWatcher_NPLCallback(sCallbackScript));
#endif
	}


	ParaFileSystemWatcher::ParaFileSystemWatcher() : m_watcher(NULL)
	{

	}

#if defined(PARAENGINE_MOBILE) || defined(EMSCRIPTEN_SINGLE_THREAD)
	ParaFileSystemWatcher::ParaFileSystemWatcher(ParaFileSystemWatcher* watcher) : m_watcher(watcher)
	{

	}
#else
    ParaFileSystemWatcher::ParaFileSystemWatcher(CFileSystemWatcher* watcher) : m_watcher(watcher)
    {

    }
#endif

	}//namespace ParaScripting

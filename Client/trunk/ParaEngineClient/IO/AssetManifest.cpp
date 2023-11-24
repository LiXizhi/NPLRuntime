//-----------------------------------------------------------------------------
// Class:	Asset Manifest
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2009.9.2
// Desc: it works with the async loader.
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "FileManager.h"
#include "IO/FileUtils.h"
#include "UrlLoaders.h"
#include "AsyncLoader.h"
#include "AssetManifest.h"
#include "AssetEntity.h"
#include "util/MD5.h"
#include "util/StringHelper.h"
#include "ParaWorldAsset.h"
#include "util/regularexpression.h"
#include <fstream>
/** we will load these files as assets manifest file. such as "assets_manifest*.txt" */
#define ASSETS_MANIFEST_FILE_PATTERN		"assets_manifest*.txt"

/** how many bytes we shall consider a file to be big. we will use a different download queue for big asset file. */
#define BIG_FILE_THESHOLD	1024000

using namespace ParaEngine;

/** how many times we will try to download a given asset file. */
#define MAX_DOWNLOAD_RETRY_COUNT	1

/** make a string valid file name, by converting to lower case and replace \\ with / */
void MakeValidFileName(string& inout)
{
	StringHelper::make_lower(inout);
	int nSize = (int)(inout.size());
	// replace '\\' with '/'
	for (int i = 0; i < nSize; i++)
	{
		if (inout[i] == '\\')
			inout[i] = '/';
	}
}

//////////////////////////////////////////////////////////////////////////
//
// AssetFileEntry
//
//////////////////////////////////////////////////////////////////////////

AssetFileEntry::AssetFileEntry() :m_bIsZipFile(false), m_nDownloadCount(0), m_nFileSize(0), m_file_type(AssetFileType_default),
m_sync_callback(NULL), m_nStatus(AssetFileStatus_Unknown)
{
}

AssetFileEntry::~AssetFileEntry()
{
	SAFE_DELETE(m_sync_callback);
}

bool AssetFileEntry::DoesFileExist()
{
	if (m_nStatus == AssetFileStatus_Unknown)
	{
		if (CParaFile::DoesFileExist2(m_localFileName.c_str(), FILE_ON_DISK))
		{
			m_nStatus = AssetFileStatus_Downloaded;
			return true;
		}
		else
		{
#ifdef PARAENGINE_CLIENT
			/**
			since FAT32 has a limit of 65535 files per folder, and if file name is long,
			it can be as few as 15000 files per folder. so I separate files into 36 folders according to
			file md5's first letter.

			For backward compatibility, we will move files in cache folder to corresponding sub folder.
			*/

			// if the files does not exist in "temp/cache/[a-z]/"
			int nSize = (int)(m_localFileName.size());
			if (nSize > 13)
			{
				char filename[256];
				for (int i = 0; i < 11; i++)
				{
					filename[i] = m_localFileName[i];
				}
				for (int i = 13; i < nSize; i++)
				{
					filename[i - 2] = m_localFileName[i];
				}
				filename[nSize - 2] = '\0';
				if (CParaFile::DoesFileExist(filename, false))
				{
					if (!CParaFile::MoveFile(filename, m_localFileName.c_str()))
					{
						if (!CParaFile::CopyFile(filename, m_localFileName.c_str(), true))
						{
							OUTPUT_LOG("failed moving or copying file from %s to %s\n", filename, m_localFileName.c_str());
							return false;
						}
					}
					return true;
				}
			}
#endif
		}
		return false;
	}
	else
	{
		return (m_nStatus == AssetFileStatus_Downloaded);
	}
}

string AssetFileEntry::GetAbsoluteUrl()
{
	if (m_url.find_first_of(' ') == string::npos)
	{
		return AssetEntity::GetAssetServerUrl() + m_url;
	}
	else
	{
		std::string url = regex_replace(m_url, regex(" "), std::string("%20"));
		return AssetEntity::GetAssetServerUrl() + url;
	}
}

bool AssetFileEntry::HasReachedMaxRetryCount()
{
	return m_nDownloadCount > MAX_DOWNLOAD_RETRY_COUNT;
}

bool AssetFileEntry::AddDownloadCount()
{
	m_nDownloadCount++;
	return !HasReachedMaxRetryCount();
}

bool AssetFileEntry::CheckSyncFile()
{
	if (DoesFileExist())
	{
		return true;
	}
	else
	{
		return SyncFile();
	}
}

bool AssetFileEntry::SyncFile()
{
	string url = GetAbsoluteUrl();

	// download the file here. 
	while (!HasReachedMaxRetryCount())
	{
		AddDownloadCount();
		CAsyncLoader* pAsyncLoader = &(CAsyncLoader::GetSingleton());
		if (pAsyncLoader->interruption_requested())
			return false;
		string sTmp = string("AssetFile Sync Started:") + url + "\n";
		pAsyncLoader->log(sTmp);
		CUrlLoader loader;
		CUrlProcessor processor;

		loader.SetUrl(url.c_str());
		processor.SetUrl(url.c_str());
		if (pAsyncLoader->RunWorkItem(&loader, &processor, NULL, NULL) != S_OK)
		{
			string sTmp = string("AssetFile Sync Failed:") + url + "\n";
			pAsyncLoader->log(sTmp);
			return false;
		}
		else
		{
			// save to disk
			if (processor.m_responseCode == 200 && processor.GetData().size() > 0)
			{
				if (SaveToDisk(&(processor.GetData()[0]), (int)(processor.GetData().size())))
				{
					string sTmp = string("AssetFile Sync Completed:") + url + "\n";
					pAsyncLoader->log(sTmp);
					// open the file now. 
					return true;
				}
				else
				{
					string sTmp = string("AssetFile Sync Failed cannot save to disk:") + url + "\n";
					pAsyncLoader->log(sTmp);
				}
			}
			else
			{
				string sTmp = string("AssetFile Sync Failed http code!=200:") + url + "\n";
				pAsyncLoader->log(sTmp);
			}
		}
	}
	m_nStatus = AssetFileStatus_Failed;
	return false;
}

namespace ParaEngine
{
	class CAssetFileEntryUserData : public CUrlProcessorUserData
	{
	public:
		CAssetFileEntryUserData(AssetFileEntry* pAssetFileEntry, URL_LOADER_CALLBACK pFuncCallback, CUrlProcessorUserData* pUserData, bool bDeleteUserData)
			: m_pAssetFileEntry(pAssetFileEntry), m_pFuncCallback(pFuncCallback), m_pUserData(pUserData), m_bDeleteUserData(bDeleteUserData) {};

		virtual ~CAssetFileEntryUserData()
		{
			if (m_bDeleteUserData)
			{
				SAFE_DELETE(m_pUserData);
			}
		}
	public:
		URL_LOADER_CALLBACK m_pFuncCallback;
		CUrlProcessorUserData* m_pUserData;
		AssetFileEntry* m_pAssetFileEntry;
		bool m_bDeleteUserData;
	};
}

bool AssetFileEntry::IsUrlFileCompressed()
{
	const std::string& url = GetUrl();
	int nSize = (int)(url.size());
	//  we will assume url file is a compressed file unless the file name ends with .p 
	return !(nSize > 3 && url[nSize - 1] == 'p' && url[nSize - 1] == '.');
}

DWORD AssetFileEntry_Request_CallBack(int nResult, CUrlProcessor* pRequest, CUrlProcessorUserData* lpUserData)
{
	bool bSucceed = false;
	if (lpUserData)
	{
		CAssetFileEntryUserData* pData = (CAssetFileEntryUserData*)lpUserData;
		CAsyncLoader* pAsyncLoader = &(CAsyncLoader::GetSingleton());

		string url = pData->m_pAssetFileEntry->GetAbsoluteUrl();

		if (nResult == CURLE_OK && pRequest->m_responseCode == 200 && pRequest->GetData().size() > 0)
		{
			if (pData->m_pAssetFileEntry->SaveToDisk(&(pRequest->GetData()[0]), (int)(pRequest->GetData().size())))
			{
				string sTmp = string("AssetFile ASync Completed:") + url + "\n";
				pAsyncLoader->log(sTmp);
				bSucceed = true;
			}
			else
			{
				string sTmp = string("AssetFile ASync Failed cannot save to disk:") + url + "\n filename: " + pData->m_pAssetFileEntry->GetLocalFileName() + "\n";
				pAsyncLoader->log(sTmp);
			}
		}
		else
		{
			string sTmp = string("AssetFile ASync Failed http code!=200:") + url + "\n";
			pAsyncLoader->log(sTmp);
		}

		if (pData->m_pFuncCallback)
		{
			return pData->m_pFuncCallback(bSucceed ? 0 : -1, pRequest, pData->m_pUserData);
		}
	}
	return 0;
}


ParaEngine::ResourceRequestID AssetFileEntry::GetRequestQueueID()
{
	ResourceRequestID nRequestID = ResourceRequestID_Asset;
	switch (GetFileType())
	{
	case AssetFileType_audio:
		// audio file goes to audio pool.  
		nRequestID = ResourceRequestID_AudioFile;
		break;
	case AssetFileType_big_file:
		// for big files(larger than 1MB), insert to a separate queue. 
		nRequestID = ResourceRequestID_Asset_BigFile;
		break;
	default:
		nRequestID = ResourceRequestID_Asset;
		break;
	}
	return nRequestID;
}


// OBSOLETED: use the boost.signal version of SyncFile_Async
HRESULT AssetFileEntry::SyncFile_Async(URL_LOADER_CALLBACK pFuncCallback, CUrlProcessorUserData* pUserData, bool bDeleteUserData)
{
	string url = GetAbsoluteUrl();

	// we need to download from the web server. 
	CAsyncLoader* pAsyncLoader = &(CAsyncLoader::GetSingleton());

	if (pAsyncLoader->interruption_requested())
		return E_FAIL;

	string sTmp = string("AssetFile ASync Started:") + url + "\n";
	pAsyncLoader->log(sTmp);

	CUrlLoader* pLoader = new CUrlLoader();
	pLoader->SetEstimatedSizeInBytes(m_nFileSize);
	CUrlProcessor* pProcessor = new CUrlProcessor();

	pLoader->SetUrl(url.c_str());
	pProcessor->SetUrl(url.c_str());
	pProcessor->SetCallBack(AssetFileEntry_Request_CallBack, new CAssetFileEntryUserData(this, pFuncCallback, pUserData, bDeleteUserData), true);

	return pAsyncLoader->AddWorkItem(pLoader, pProcessor, NULL, NULL, GetRequestQueueID());
}

DWORD AssetFileEntry_Request_Signal_CallBack(int nResult, CUrlProcessor* pRequest, CUrlProcessorUserData* lpUserData)
{
	bool bSucceed = false;
	if (lpUserData)
	{
		CAssetFileEntryUserData* pData = (CAssetFileEntryUserData*)lpUserData;
		CAsyncLoader* pAsyncLoader = &(CAsyncLoader::GetSingleton());

		string url = pData->m_pAssetFileEntry->GetAbsoluteUrl();

		if (nResult == CURLE_OK && pRequest->m_responseCode == 200 && pRequest->GetData().size() > 0)
		{
			if (pData->m_pAssetFileEntry->SaveToDisk(&(pRequest->GetData()[0]), (int)(pRequest->GetData().size())))
			{
				string sTmp = string("AssetFile ASync Completed:") + url + "\n";
				pAsyncLoader->log(sTmp);
				bSucceed = true;
			}
			else
			{
				string sTmp = string("AssetFile ASync Failed cannot save to disk:") + url + "\n filename: " + pData->m_pAssetFileEntry->GetLocalFileName() + "\n";
				pAsyncLoader->log(CAsyncLoader::Log_Warn, sTmp);
				OUTPUT_LOG("warn: %s", sTmp.c_str());
			}
		}
		else
		{
			string sTmp = string("AssetFile ASync Failed http code!=200:") + url + "\n";
			pAsyncLoader->log(CAsyncLoader::Log_Warn, sTmp);

			if (!pData->m_pAssetFileEntry->HasReachedMaxRetryCount() && nResult == CURLE_OPERATION_TIMEDOUT)
			{
				// try downloading again
				pData->m_pAssetFileEntry->SyncFile_AsyncRetry();
				return 0;
			}
			else
			{
				OUTPUT_LOG("warn: %s curl return code", sTmp.c_str(), nResult);
			}
		}

		pData->m_pAssetFileEntry->SignalComplete(bSucceed ? 0 : -1);

	}
	return 0;
}

bool AssetFileEntry::IsDownloading()
{
	return (m_sync_callback != 0);
}


HRESULT AssetFileEntry::SyncFile_AsyncRetry()
{
	if (m_sync_callback != 0)
	{
		// start downloading the file.  
		if (!HasReachedMaxRetryCount())
		{
			AddDownloadCount();

			// start the download. 
			string url = GetAbsoluteUrl();

			// we need to download from the web server. 
			CAsyncLoader* pAsyncLoader = &(CAsyncLoader::GetSingleton());

			if (pAsyncLoader->interruption_requested())
				return E_FAIL;

			string sTmp = string("AssetFile ASync Started:") + url + "\n";
			pAsyncLoader->log(sTmp);

			CUrlLoader* pLoader = new CUrlLoader();
			pLoader->SetEstimatedSizeInBytes(m_nFileSize);
			CUrlProcessor* pProcessor = new CUrlProcessor();

			pLoader->SetUrl(url.c_str());
			pProcessor->SetUrl(url.c_str());
			pProcessor->SetCallBack(AssetFileEntry_Request_Signal_CallBack, new CAssetFileEntryUserData(this, NULL, NULL, false), true);

			OUTPUT_LOG("warn: try downloading %d times: for %s", m_nDownloadCount, url.c_str());

			return pAsyncLoader->AddWorkItem(pLoader, pProcessor, NULL, NULL, GetRequestQueueID());
		}
		else
		{
			return E_FAIL;
		}
	}
	return E_FAIL;
}

// this function is NOT thread-safe, because it uses boost.signal 
HRESULT AssetFileEntry::SyncFile_Async(const SyncFile_Callback_t::slot_type& slot)
{
	if (m_sync_callback == 0)
	{
		if (DoesFileExist())
		{
			// if already downloaded. invoke the callback with complete data. 
			SyncFile_Callback_t callback_;
			callback_.connect(slot);
			callback_(0, this);
			return S_OK;
		}

		// start downloading the file.  
		if (!HasReachedMaxRetryCount())
		{
			AddDownloadCount();

			m_sync_callback = new SyncFile_Callback_t();
			m_sync_callback->connect(slot);

			// start the download. 
			string url = GetAbsoluteUrl();

			// we need to download from the web server. 
			CAsyncLoader* pAsyncLoader = &(CAsyncLoader::GetSingleton());

			if (pAsyncLoader->interruption_requested())
				return E_FAIL;

			string sTmp = string("AssetFile ASync Started:") + url + "\n";
			pAsyncLoader->log(sTmp);

			CUrlLoader* pLoader = new CUrlLoader();
			pLoader->SetEstimatedSizeInBytes(m_nFileSize);
			CUrlProcessor* pProcessor = new CUrlProcessor();

			pLoader->SetUrl(url.c_str());
			pProcessor->SetUrl(url.c_str());
			pProcessor->SetCallBack(AssetFileEntry_Request_Signal_CallBack, new CAssetFileEntryUserData(this, NULL, NULL, false), true);

			return pAsyncLoader->AddWorkItem(pLoader, pProcessor, NULL, NULL, GetRequestQueueID());
		}
		else
		{
			OUTPUT_LOG("AssetFileEntry Error: unable to sync file %s\n", m_localFileName.c_str());
			return E_FAIL;
		}
	}
	else
	{
		m_sync_callback->connect(slot);
	}
	return S_OK;
}

void AssetFileEntry::SignalComplete(int nResult)
{
	if (m_sync_callback)
	{
		(*m_sync_callback)(nResult, this);

		SAFE_DELETE(m_sync_callback);
	}
}

bool AssetFileEntry::CheckMD5AndSize(const char* buffer, int nSize)
{
	// compare the file size first. 
	char sSize[64];
	ParaEngine::StringHelper::fast_itoa(nSize, sSize, 40);
	string file_size = sSize;
	int nCount = (int)file_size.size();

	int nFileNameCount = (int)m_localFileName.size();
	if (nFileNameCount <= nCount)
	{
		return false;
	}

	for (int i = 0; i < nCount; ++i)
	{
		if (file_size[i] != m_localFileName[nFileNameCount - nCount + i])
		{
			return false;
		}
	}

	// compare the md5 part
	int nFrom = (int)(m_localFileName.find_last_of('/') + 1);
	nCount = nFileNameCount - nFrom - nCount;
	string md5_str = m_localFileName.substr(nFrom, nCount);

	ParaEngine::MD5 md5_hash;
	md5_hash.feed((const unsigned char*)buffer, nSize);
	string sMD5String = md5_hash.hex();

	return (stricmp(md5_str.c_str(), sMD5String.c_str()) == 0);
}

std::string AssetFileEntry::GetFullFilePath()
{
#if defined(PARAENGINE_MOBILE) || defined(EMSCRIPTEN)
	return CFileUtils::GetWritableFullPathForFilename(GetLocalFileName());
#else
	return GetLocalFileName();
#endif
}


bool AssetFileEntry::SaveToDisk(const char* buffer, int nSize, bool bCheckMD5)
{
	if (bCheckMD5 && !CheckMD5AndSize(buffer, nSize))
	{
		string sTmp = string("Asset md5 check Failed:") + m_url + "\n";
		CAsyncLoader::GetSingleton().log(sTmp);
		// OUTPUT_LOG("warning: md5 check for asset file failed for file: %s.\n", m_url.c_str());
		m_nStatus = AssetFileStatus_Unknown;
		return false;
	}

	static ParaEngine::mutex s_save_file_mutex;
	ParaEngine::Lock lock_(s_save_file_mutex);

#ifdef EMSCRIPTEN
	string tmpName = GetLocalFileName();
#else
	string tmpName = GetLocalFileName() + ".tmp";
#endif
	if (m_bIsZipFile)
	{
		// for zipped file
		if (CParaFile::UnzipMemToFile(buffer, nSize, tmpName.c_str(), false))
		{
			if (CParaFile::MoveFile(tmpName.c_str(), GetLocalFileName().c_str()))
			{
				m_nStatus = AssetFileStatus_Downloaded;
				return true;
			}
			else
			{
				string sTmp = string("Failed To MoveFile:") + m_url + "\n";
				CAsyncLoader::GetSingleton().log(sTmp);
			}
		}
		else
		{
			string sTmp = string("Failed To UnzipMemToFile:") + m_url + "\n";
			CAsyncLoader::GetSingleton().log(sTmp);
		}
	}
	else
	{
		CParaFile file;
		// save file to temp/cache directory.
		if (file.CreateNewFile(tmpName.c_str(), false))
		{
			file.write(buffer, nSize);
			file.close();

			if (CParaFile::MoveFile(tmpName.c_str(), GetLocalFileName().c_str()))
			{
				m_nStatus = AssetFileStatus_Downloaded;
				return true;
			}
			else
			{
				string sTmp = string("Failed To MoveFile:") + m_url + "\n";
				CAsyncLoader::GetSingleton().log(sTmp);
			}
		}
		else
		{
			string sTmp = string("Failed To CreateNewFile:") + m_url + "\n";
			CAsyncLoader::GetSingleton().log(sTmp);
		}
	}
	m_nStatus = AssetFileStatus_Unknown;
	return false;
}

AssetFileEntry::AssetFileTypeEnum AssetFileEntry::SetFileType(const std::string& file_extension)
{
	if (file_extension == "ogg" || file_extension == "wav" || file_extension == "mp3")
		m_file_type = AssetFileType_audio;
	else
	{
		if (GetFileSize() < BIG_FILE_THESHOLD)
		{
			if (file_extension == "dds")
				m_file_type = AssetFileType_dds_file;
			else if (file_extension == "x")
				m_file_type = AssetFileType_x_file;
			else if (file_extension == "png")
				m_file_type = AssetFileType_ui_texture;
		}
		else
		{
			m_file_type = AssetFileType_big_file;
		}
	}
	return m_file_type;
}

//////////////////////////////////////////////////////////////////////////
//
// CAssetManifest
//
//////////////////////////////////////////////////////////////////////////
CAssetManifest::CAssetManifest(void)
	:m_bEnableManifest(true), m_bUseLocalFileFirst(false)
{
	LoadManifest();
}

CAssetManifest::~CAssetManifest(void)
{
	CleanUp();
}

bool CAssetManifest::IsUseLocalFileFirst() const
{
	return m_bUseLocalFileFirst;
}

void CAssetManifest::SetUseLocalFileFirst(bool val)
{
	m_bUseLocalFileFirst = val;
}
void CAssetManifest::CleanUp()
{
	Asset_Manifest_Map_Type::iterator itCur, itEnd = m_files.end();
	for (itCur = m_files.begin(); itCur != itEnd; ++itCur)
	{
		SAFE_DELETE(itCur->second);
	}
	m_files.clear();
}

CAssetManifest& CAssetManifest::GetSingleton()
{
	static CAssetManifest g_instance;
	return g_instance;
}

void CAssetManifest::EnableManifest(bool bEnable)
{
	m_bEnableManifest = bEnable;
}

void CAssetManifest::PrintStat()
{
	OUTPUT_LOG("CAssetManifest loaded %d files\n", (int)(m_files.size()));
}

int CAssetManifest::CheckSyncFile(const char* filename)
{
	AssetFileEntry* pEntry = GetFile(filename);
	if (pEntry)
	{
		return pEntry->CheckSyncFile() ? 1 : -1;
	}
	else
		return 0;
}


void CAssetManifest::PrepareCacheFolders()
{
	if (!CParaFile::CreateDirectory("temp/cache/"))
	{
		OUTPUT_LOG("warning: failed creating directory temp/cache/\n");
	}

	/**
	since FAT32 has a limit of 65535 files per folder, and if file name is long,
	it can be as few as 15000 files per folder. so I separate files into 36 folders according to
	file md5's first letter
	*/
	std::string sSubFolder = "temp/cache/a/";
	bool bHasDeletedFile = false;
	for (int i = 0; i < 36; i++)
	{
		if (i < 26)
			sSubFolder[sSubFolder.size() - 2] = (char)('a' + i);
		else
			sSubFolder[sSubFolder.size() - 2] = (char)('0' + i - 26);

		if (!CParaFile::CreateDirectory(sSubFolder.c_str()))
		{
			OUTPUT_LOG("warning: failed creating directory %s \n", sSubFolder.c_str());
#if defined(WIN32) && !defined(PARAENGINE_MOBILE)
			if (!bHasDeletedFile)
			{
				OUTPUT_LOG("maybe there are so many files in the cache folder already we will delete some files \n");
				bHasDeletedFile = true;
				i--;
				std::string sRootPath = "temp/cache/";
				CSearchResult* pResult = CFileManager::GetInstance()->SearchFiles(sRootPath, "*.*", "", 0, 500, 0);
				int nSize = pResult->GetNumOfResult();
				for (int i = 0; i < nSize; ++i)
				{
					const std::string& filename = pResult->GetItem(i);
					if (filename.size() >= 32)
					{
						std::string fullname = sRootPath + filename;
						OUTPUT_LOG("delete file %s in the cache folder \n", fullname.c_str());
						CParaFile::DeleteFile(fullname.c_str());
					}
				}
			}
#endif
		}
	}
}

void CAssetManifest::LoadManifest()
{
	CSearchResult* result = CFileManager::GetInstance()->SearchFiles(
		CParaFile::GetCurDirectory(0),
		ASSETS_MANIFEST_FILE_PATTERN, "", 0, 1000, 0);

	int nNum = 0;
	if (result != 0 && result->GetNumOfResult() > 0)
	{
		nNum = result->GetNumOfResult();
		for (int i = 0; i < nNum; ++i)
		{
			LoadManifestFile(result->GetItem(i));
		}
	}
	else
	{
#if defined(PARAENGINE_MOBILE) || defined(PLATFORM_MAC) || defined(EMSCRIPTEN)
		const char* sDefaultFile = "assets_manifest.txt";
		if (CParaFile::DoesFileExist(sDefaultFile))
		{
			nNum = 1;
			LoadManifestFile(sDefaultFile);
		}
#endif
	}
	if (nNum > 0)
		PrepareCacheFolders();
}

void CAssetManifest::LoadManifestFile(const string& filename)
{
	OUTPUT_LOG("Asset manifest file %s is loaded\n", filename.c_str());
	CParaFile file(filename.c_str());
	if (file.isEof())
		return;
	char line[512];
	while (file.GetNextLine(line, 511))
	{
		AddEntry(line);
	}
}

/** replacement of following code, to make it faster in debug mode.
static regex re("([^,]+),([^,]+),([^,]+)");
smatch result;
string sFilename = filename;
if (regex_search(sFilename, result, re) && result.size() > 3)
{
string fileKey = result.str(1);
string md5 = result.str(2);
string filesize = result.str(3);
}
*/
bool CAssetManifest::ParseAssetEntryStr(const string& sFilename, string& fileKey, string& md5, string& filesize)
{
	int nSize = (int)sFilename.size();
	const char* line = sFilename.c_str();

	int nFromPos = 0;
	for (int i = 0; i < nSize; ++i)
	{
		char c = line[i];
		if (c == ',') {
			if (fileKey.empty()) {
				fileKey = sFilename.substr(nFromPos, i - nFromPos);
				nFromPos = i + 1;
			}
			else if (md5.empty()) {
				md5 = sFilename.substr(nFromPos, i - nFromPos);
				nFromPos = i + 1;
				filesize = sFilename.substr(nFromPos);
				return true;
			}
		}
	}
	return false;
}


void CAssetManifest::AddEntry(const char* filename)
{
	if (filename == 0)
		return;
	string sFilename = filename;
	string fileKey;
	string md5;
	string filesize;

	if (ParseAssetEntryStr(sFilename, fileKey, md5, filesize))
	{
		string file_extension;

		// to lower case and replace "\\" with "/"
		MakeValidFileName(fileKey);

		bool bIsZipfile = false;
		int nLen = (int)(fileKey.size());
		if (nLen > 2 && fileKey[nLen - 2] == '.')
		{
			if (fileKey[nLen - 1] == 'p')
			{
				// non-compressed file
				fileKey.resize(nLen - 2);
			}
			else if (fileKey[nLen - 1] == 'z')
			{
				// compressed file
				bIsZipfile = true;
				fileKey.resize(nLen - 2);
			}
		}

		int nFileSize = atoi(filesize.c_str());
		if (nFileSize == 0)
		{
			// ignore file whose size is 0.
			return;
		}

		Asset_Manifest_Map_Type::iterator iter = m_files.find(fileKey);
		if (iter != m_files.end())
		{
			// replace old one. 
			SAFE_DELETE(iter->second);
		}

		// extract file extension and parent directory
		for (int i = (int)fileKey.size() - 1; i >= 0; --i)
		{
			char ch = fileKey[i];
			if (ch == '.' && file_extension.empty())
			{
				int nCount = (int)fileKey.size() - (i + 1);
				if (nCount > 0)
					file_extension = fileKey.substr(i + 1, nCount);
			}
			else if (ch == '/')
			{
				// also add parent directory to list, so that DoesAssetFileExist() will return true for directory entries as well. 
				string sParentDir = fileKey.substr(0, i);
				iter = m_files.find(sParentDir);
				if (iter == m_files.end())
				{
					AssetFileEntry* pEntry = new AssetFileEntry();
					pEntry->m_url = sParentDir;
					pEntry->m_bIsZipFile = false;
					m_files[sParentDir] = pEntry;
				}
				break;
			}
		}

		AssetFileEntry* pEntry = new AssetFileEntry();
		pEntry->m_url = filename;
		pEntry->m_localFileName.reserve(pEntry->m_localFileName.size() + 63);
		pEntry->m_localFileName += "temp/cache/a/";
		pEntry->m_localFileName[11] = md5[0];
		pEntry->m_localFileName += md5;
		pEntry->m_localFileName += filesize;
		pEntry->m_bIsZipFile = bIsZipfile;
		pEntry->m_nFileSize = nFileSize;
		pEntry->SetFileType(file_extension); // note: this function must be called after file size is set. 
		m_files[fileKey] = pEntry;

		// OUTPUT_LOG("asset manifest: %s, %s, ziped:%d\n", pEntry->m_url.c_str(), pEntry->m_localFileName.c_str(), pEntry->m_bIsZipFile ? 1 : 0);
	}
}

bool CAssetManifest::DoesFileExist(const char* filename)
{
	if (!filename)
		return false;
	if (!IsEnabled())
		return false;
	string filename_ = filename;
	MakeValidFileName(filename_);

	Asset_Manifest_Map_Type::const_iterator iter = m_files.find(filename_);
	return (iter != m_files.end());
}

bool CAssetManifest::DoesFileExist(const string& filename)
{
	if (!IsEnabled())
		return false;

	string filename_ = filename;
	MakeValidFileName(filename_);

	Asset_Manifest_Map_Type::const_iterator iter = m_files.find(filename_);
	return (iter != m_files.end());
}

AssetFileEntry* CAssetManifest::GetFile(const string& filename, bool bUseReplaceMap, bool bIgnoreLocalFile)
{
	if (!IsEnabled())
		return NULL;

	string filename_ = filename;
	if (bUseReplaceMap)
		CFileReplaceMap::GetSingleton().ReplaceFile(filename_);

	if (!bIgnoreLocalFile && IsUseLocalFileFirst())
	{
		// GetFile() will return null, if a local disk or zip file is found even there is an entry in the assetmanifest.
		if (CParaFile::DoesFileExist(filename.c_str(), true))
			return NULL;
	}

	MakeValidFileName(filename_);

	Asset_Manifest_Map_Type::iterator iter = m_files.find(filename_);
	if (iter != m_files.end())
	{
		return iter->second;
	}
	return NULL;
}

//////////////////////////////////////////////////////////////////////////
//
// CFileReplaceMap
//
//////////////////////////////////////////////////////////////////////////

CFileReplaceMap::CFileReplaceMap()
{
}

CFileReplaceMap::CFileReplaceMap(const string& filename, bool bReplaceExistingOnes)
{
	LoadReplaceFile(filename, bReplaceExistingOnes);
}

CFileReplaceMap::~CFileReplaceMap()
{
}

CFileReplaceMap& CFileReplaceMap::GetSingleton()
{
	static CFileReplaceMap g_singleton;
	return g_singleton;
}

bool CFileReplaceMap::IsEmpty()
{
	return m_replace_map.empty();
}

bool CFileReplaceMap::ReplaceFile(string& inout)
{
	ParaEngine::Lock lock_(m_mutex);

	Asset_Replace_Map_Type::const_iterator iter = m_replace_map.find(inout);
	if (iter != m_replace_map.end())
	{
		inout = iter->second;
		return true;
	}
	return false;
}

void CFileReplaceMap::LoadReplaceFile(const string& filename, bool bReplaceExistingOnes)
{
	ParaEngine::Lock lock_(m_mutex);

	if (bReplaceExistingOnes)
	{
		if (!m_replace_map.empty())
		{
			m_replace_map.clear();
			OUTPUT_LOG("all replacement files are cleared.\n");
		}
	}
	if (filename.empty())
		return;
	string::size_type nCommaPos = filename.find(",");
	if (nCommaPos != string::npos)
	{
		// if filename contains ",", we will simply replace the two specified file separated by comma. 
		string sFromFile = filename.substr(0, nCommaPos);
		string sToFile = filename.substr(nCommaPos + 1);
		bool bIsChanged = false;
		if (sToFile.empty())
		{
			auto iterLastFile = m_replace_map.end();
			if ((iterLastFile = m_replace_map.find(sFromFile)) != m_replace_map.end())
			{
				m_replace_map.erase(iterLastFile);
				bIsChanged = true;
			}
		}
		else
		{
			if (m_replace_map.find(sFromFile) == m_replace_map.end() || m_replace_map[sFromFile] != sToFile)
			{
				m_replace_map[sFromFile] = sToFile;
				bIsChanged = true;
			}
		}

		if (bIsChanged && CGlobals::GetAssetManager()->UnloadAssetByKeyName(sFromFile))
		{
#ifdef _DEBUG
			// OUTPUT_LOG("Replace asset file %s->%s\n", sFromFile.c_str(), sToFile.c_str());
#endif
		}
		return;
	}

	CParaFile file;

	if (file.OpenAssetFile(filename.c_str()))
	{
		regex re("([^,]+),([^,]+)");

		int nCount = 0;
		char line[1024];
		while (file.GetNextLine(line, 1023))
		{
			string sLine = line;
			smatch result;
			if (regex_search(sLine, result, re) && result.size() > 2)
			{
				string fileFrom = result.str(1);
				string fileTo = result.str(2);

				Asset_Replace_Map_Type::iterator iter = m_replace_map.find(fileFrom);
				if (iter == m_replace_map.end())
				{
					nCount++;
					m_replace_map[fileFrom] = fileTo;
				}
				else
				{
					if (iter->second == fileTo)
					{
						// OUTPUT_LOG("warning: recursive file replace found: %s->%s\n", fileFrom.c_str(), fileTo.c_str());
					}
					else
					{
						// overwrite old one with new mapping. 
						iter->second = fileTo;
						if (CGlobals::GetAssetManager()->UnloadAssetByKeyName(fileFrom))
						{
#ifdef _DEBUG
							OUTPUT_LOG("warning: unload a replace asset file %s->%s\n", fileFrom.c_str(), fileTo.c_str());
#endif
						}
					}
				}
			}
		}
		OUTPUT_LOG("Asset Replace file %s is loaded, %d file replaced. Overwrite(%s)\n", filename.c_str(), nCount, bReplaceExistingOnes ? "true" : "false");

	}

}

int CAssetManifest::InstallFields(CAttributeClass* pClass, bool bOverride)
{
	IAttributeFields::InstallFields(pClass, bOverride);

	pClass->AddField("Clear", FieldType_void, (void*)Clear_s, NULL, NULL, NULL, bOverride);
	pClass->AddField("LoadManifest", FieldType_void, (void*)LoadManifest_s, NULL, NULL, NULL, bOverride);

	pClass->AddField("LoadManifestFile", FieldType_String, (void*)LoadManifestFile_s, (void*)0, NULL, NULL, bOverride);
	pClass->AddField("Enabled", FieldType_Bool, (void*)SetEnabled_s, (void*)IsEnabled_s, NULL, NULL, bOverride);
	pClass->AddField("UseLocalFileFirst", FieldType_Bool, (void*)SetUseLocalFileFirst_s, (void*)IsUseLocalFileFirst_s, NULL, NULL, bOverride);
	return S_OK;
}
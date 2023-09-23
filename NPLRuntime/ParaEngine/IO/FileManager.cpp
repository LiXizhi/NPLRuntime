//-----------------------------------------------------------------------------
// Class: CFileManager
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Date:	2005.4
// Revised: 2005.4
// Notes: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "util/CSingleton.h"
#include "ZipArchive.h"
#include "FileUtils.h"
#include "BlockEngine/BlockReadWriteLock.h"
#include "FileManager.h"


using namespace ParaEngine;

CFileManager::CFileManager(void)
	: m_pArchiveLock(new BlockReadWriteLock())
{
	m_priority = 0;
}

CFileManager::~CFileManager(void)
{
	{
		Scoped_WriteLock<BlockReadWriteLock> lock_(*m_pArchiveLock);

		std::list<CArchive*>::iterator itCurCP, itEndCP = m_archivers.end();
		for (itCurCP = m_archivers.begin(); itCurCP != itEndCP; ++itCurCP)
		{
			(*itCurCP)->Close();
			delete (*itCurCP);
		}
	}

	SAFE_DELETE(m_pArchiveLock);
}

CFileManager * CFileManager::GetInstance()
{
	return CAppSingleton<CFileManager>::GetInstance();
}

bool CFileManager::OpenArchiveEx(const std::string& path, const std::string& sRootDir)
{
	{
		Scoped_ReadLock<BlockReadWriteLock> lock_(*m_pArchiveLock);
		/// we will not reopen it.
		std::list<CArchive*>::iterator itCurCP, itEndCP = m_archivers.end();
		for (itCurCP = m_archivers.begin(); itCurCP != itEndCP; ++itCurCP)
		{
			if ((*itCurCP)->GetArchiveName().compare(path) == 0)
			{
				// move package to back of the queue? 
				return true;
			}
		}
	}
	string sArchiveName = path;
	int nLen = (int)sArchiveName.size();
	bool bRes = true;
	if (nLen > 4)
	{
		string sFileExt = sArchiveName.substr(nLen - 3, 3);
		CArchive* pArchive = NULL;
		if (sFileExt == "zip" || sFileExt == "pkg" || sFileExt == "p3d")
			pArchive = new CZipArchive(); // TODO
		if (pArchive != NULL)
		{
			bRes = pArchive->Open(sArchiveName, ++m_priority);
			if (bRes)
			{
				if (!sRootDir.empty())
					pArchive->SetRootDirectory(sRootDir);

				Scoped_WriteLock<BlockReadWriteLock> lock_(*m_pArchiveLock);
				m_archivers.push_back(pArchive);
			}
			else
			{
				SAFE_DELETE(pArchive);
			}
		}
		else
			bRes = false;
	}
	return bRes;
}

bool CFileManager::OpenArchive(const std::string&  path, bool bUseRelativePath)
{
	return OpenArchiveEx(path, bUseRelativePath ? path : "");
}

void CFileManager::CloseArchive(const std::string&  path)
{
	Scoped_WriteLock<BlockReadWriteLock> lock_(*m_pArchiveLock);

	std::list<CArchive*>::iterator itCurCP, itEndCP = m_archivers.end();
	for (itCurCP = m_archivers.begin(); itCurCP != itEndCP; ++itCurCP)
	{
		if ((*itCurCP)->GetArchiveName().compare(path) == 0)
		{
			(*itCurCP)->Close();
			delete (*itCurCP);
			m_archivers.erase(itCurCP);
			break;
		}
	}
}

void CFileManager::LoadFile(CParaFile* pFile, const char* fileName)
{
	pFile->OpenFile(fileName);
}

bool CFileManager::OpenFile(const char* filename, FileHandle& handle)
{
	bool bOpened = false;
	bool bMPQProcessed = false;

	char tempStr[1024];
	int i = 0;

	while (filename[i])
	{
		tempStr[i] = filename[i] == '\\' ? '/' : filename[i];
		i++;
	};
	tempStr[i] = 0;

	uint32 hash = SZipFileEntry::Hash(tempStr, true);
	ArchiveFileFindItem item(tempStr, nullptr, &hash);

	Scoped_ReadLock<BlockReadWriteLock> lock_(*m_pArchiveLock);
	std::list<CArchive*>::iterator itCurCP, itEndCP = m_archivers.end();
	for (itCurCP = m_archivers.begin(); (!bOpened) && itCurCP != itEndCP; ++itCurCP)
	{
		CArchive* pArchive = (*itCurCP);
		//PERF_BEGIN("ZIP_Search");
		bOpened = pArchive->OpenFile(&item, handle);
		//PERF_END("ZIP_Search");
	}
	return bOpened;
}

bool CFileManager::DoesFileExist(const char* filename)
{
	if (!filename)
		return false;
	FileHandle handle;
	bool bExists = OpenFile(filename, handle);
	CloseFile(handle);
	return bExists;
}

string CFileManager::GetFileOriginalName(const char* filename)
{
	FileHandle handle;
	string originalNameFull(filename);
	bool bExists = OpenFile(filename, handle);
	if (bExists)
	{
		string nameInArchive = handle.m_pArchive->GetNameInArchive(handle);
		int nStart = (int)originalNameFull.size() - (int)nameInArchive.size();
		int nCount = (int)nameInArchive.size();
		if (nCount > 0)
			originalNameFull.replace(nStart, nCount, handle.m_pArchive->GetOriginalNameInArchive(handle));
	}
	CloseFile(handle);
	return originalNameFull;
}

DWORD CFileManager::GetFileSize(FileHandle& handle)
{
	if (handle.m_pArchive)
		return handle.m_pArchive->GetFileSize(handle);
	return 0;
}

bool CFileManager::ReadFile(FileHandle& handle,LPVOID lpBuffer,DWORD nNumberOfBytesToRead,LPDWORD lpNumberOfBytesRead, LPDWORD lpLastWriteTime)
{
	if(handle.m_pArchive)
		return handle.m_pArchive->ReadFile(handle, lpBuffer, nNumberOfBytesToRead, lpNumberOfBytesRead, lpLastWriteTime);
	return false;
}

bool CFileManager::ReadFileRaw(FileHandle& handle, LPVOID* lppBuffer, LPDWORD pnCompressedSize, LPDWORD pnUncompressedSize)
{
	if (handle.m_pArchive)
		return handle.m_pArchive->ReadFileRaw(handle, lppBuffer, pnCompressedSize, pnUncompressedSize);
	return false;
}

bool CFileManager::CloseFile(FileHandle& hFile)
{
	if (hFile.m_pArchive)
		return hFile.m_pArchive->CloseFile(hFile);
	return false;
}


//////////////////////////////////////////////////////////////////////////
// Search																//
//////////////////////////////////////////////////////////////////////////
#define CHECK_BIT(x,y) (((x)&(y))>0)
void CFileManager::FindDiskFiles(CSearchResult& result, const string& sRootPath, const string& sFilePattern, int nSubLevel)
{
	CFileUtils::FindDiskFiles(result, sRootPath, sFilePattern, nSubLevel);
}

void CFileManager::FindLocalFiles(CSearchResult& result, const string& sRootPath, const string& sFilePattern, int nSubLevel)
{
	CFileUtils::FindLocalFiles(result, sRootPath, sFilePattern, nSubLevel);
}


CSearchResult* CFileManager::SearchFiles(const string& sRootPath, const string& sFilePattern, const string& sZipArchive, int nSubLevel, int nMaxFilesNum, int nFrom)
{
	// TODO: allocate result on heap will enable asynchronous calls to this function.
	// for simplicity of memory management, I used this static instance approach.
	static CSearchResult result;
	result.InitSearch(sRootPath, nSubLevel, nMaxFilesNum, nFrom);
	if (sFilePattern == "")
		return &result;
	if (sZipArchive == "")
	{
		// search in disk files
		FindDiskFiles(result, result.GetRootPath(), sFilePattern, nSubLevel);
		FindLocalFiles(result, sRootPath, sFilePattern, nSubLevel);
	}
	else if (sZipArchive == "*.zip")
	{
		Scoped_ReadLock<BlockReadWriteLock> lock_(*m_pArchiveLock);
		// search in all currently opened zip archives
		list<CArchive*>::iterator itCurCP, itEndCP = m_archivers.end();
		for (itCurCP = m_archivers.begin(); itCurCP != itEndCP; ++itCurCP)
		{
			(*itCurCP)->FindFiles(result, sRootPath, sFilePattern, nSubLevel);
		}
	}
	else if (sZipArchive == "*.*")
	{
		// search in disk followed by zip files.
		FindDiskFiles(result, result.GetRootPath(), sFilePattern, nSubLevel);

		Scoped_ReadLock<BlockReadWriteLock> lock_(*m_pArchiveLock);
		list<CArchive*>::iterator itCurCP, itEndCP = m_archivers.end();
		for (itCurCP = m_archivers.begin(); itCurCP != itEndCP; ++itCurCP)
		{
			(*itCurCP)->FindFiles(result, sRootPath, sFilePattern, nSubLevel);
		}
	}
	else
	{
		// search in the given zip file
		{
			Scoped_ReadLock<BlockReadWriteLock> lock_(*m_pArchiveLock);
			list<CArchive*>::iterator itCurCP, itEndCP = m_archivers.end();
			for (itCurCP = m_archivers.begin(); itCurCP != itEndCP; ++itCurCP)
			{
				if ((*itCurCP)->GetArchiveName().compare(sZipArchive) == 0)
				{
					(*itCurCP)->FindFiles(result, sRootPath, sFilePattern, nSubLevel);
					return &result;
				}
			}
		}

		if (CParaFile::DoesFileExist(sZipArchive.c_str()) && CParaFile::GetFileExtension(sZipArchive.c_str()) == "zip")
		{
			// if the zip file is not loaded before, we will just load it and then unload after the search.
			CZipArchive zipfile;
			if (zipfile.Open(sZipArchive, 0))
			{
				zipfile.FindFiles(result, sRootPath, sFilePattern, nSubLevel);
			}
		}
	}
	return &result;
}

int ParaEngine::CFileManager::InstallFields(CAttributeClass* pClass, bool bOverride)
{
	ISearchPathManager::InstallFields(pClass, bOverride);

	return S_OK;
}

IAttributeFields* ParaEngine::CFileManager::GetChildAttributeObject(const char * sName)
{
	Scoped_ReadLock<BlockReadWriteLock> lock_(*m_pArchiveLock);
	for (CArchive* pArchive : m_archivers)
	{
		if (pArchive->IsArchive(sName))
			return pArchive;
	}
	return NULL;
}

PE_CORE_DECL CArchive* ParaEngine::CFileManager::GetArchive(const string& path)
{
	Scoped_ReadLock<BlockReadWriteLock> lock_(*m_pArchiveLock);
	for (CArchive* pArchive : m_archivers)
	{
		if (pArchive->IsArchive(path))
			return pArchive;
	}
	return NULL;
}

int ParaEngine::CFileManager::GetChildAttributeObjectCount(int nColumnIndex /*= 0*/)
{
	Scoped_ReadLock<BlockReadWriteLock> lock_(*m_pArchiveLock);
	return (int)m_archivers.size();
}

IAttributeFields* ParaEngine::CFileManager::GetChildAttributeObject(int nRowIndex, int nColumnIndex /*= 0*/)
{
	if (nColumnIndex == 0)
	{
		Scoped_ReadLock<BlockReadWriteLock> lock_(*m_pArchiveLock);
		if (nRowIndex < (int)m_archivers.size())
		{
			auto iter = m_archivers.begin();
			std::advance(iter, nRowIndex);
			return *iter;
		}
	}
	return NULL;
}


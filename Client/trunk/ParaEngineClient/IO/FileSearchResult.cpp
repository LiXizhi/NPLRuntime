//-----------------------------------------------------------------------------
// Class: FileSearchResult
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2005.4, 2014.8
// Revised: refactored from file manager
// Notes: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "FileSearchResult.h"
using namespace ParaEngine;

CSearchResult::CSearchResult()
{
	InitSearch("");
}

CSearchResult::~CSearchResult()
{
	Release();
}

void CSearchResult::Release()
{
	m_rootPath.clear();
	m_results.clear();
	m_filenames.clear();
	m_nFileNumber = 0;
}

void CSearchResult::InitSearch(const string& sRootPath, int nSubLevel, int nMaxFilesNum, int nFrom)
{
	m_rootPath = sRootPath;
	m_results.clear();
	m_filenames.clear();
	m_nFileNumber = 0;

	m_nSubLevel = nSubLevel;
	m_nMaxFilesNum = nMaxFilesNum;
	m_nFrom = nFrom;
}

void CSearchResult::SetRootPath(const string& sRoot)
{
	m_rootPath = sRoot;
}

const string& CSearchResult::GetRootPath()
{
	return m_rootPath;
}

int CSearchResult::GetNumOfResult()
{
	return (int)m_results.size();
}

const string& CSearchResult::GetItem(int nIndex)
{
	if (nIndex >= 0 && nIndex < GetNumOfResult())
	{
		return m_results[nIndex].m_sFileName;
	}
	return CGlobals::GetString(0);
}


const CFileFindData* CSearchResult::GetItemData(int nIndex)
{
	if (nIndex >= 0 && nIndex < GetNumOfResult())
	{
		return &(m_results[nIndex]);
	}
	return NULL;
}

bool CSearchResult::AddResult(const string& sFileName, DWORD dwFileSize/*=0*/, DWORD dwFileAttributes/*=0*/, const FILETIME* ftLastWriteTime/*=0*/, const FILETIME* ftCreationTime/*=0*/, const FILETIME* ftLastAccessTime/*=0*/)
{
	return AddResult(CFileFindData(CParaFile::GetRelativePath(sFileName, m_rootPath), dwFileSize, dwFileAttributes, ftLastWriteTime, ftCreationTime, ftLastAccessTime));
}

bool CSearchResult::AddResult(const CFileFindData& item)
{
	if (m_filenames.find(item.m_sFileName) != m_filenames.end())
	{
		// already added before. 
		return true;
	}
	else
	{
		m_filenames.insert(item.m_sFileName);
	}
	++m_nFileNumber;
	if (m_nFileNumber <= m_nFrom)
		return true;
	else if (m_nFileNumber <= (m_nFrom + m_nMaxFilesNum))
	{
		m_results.push_back(item);
		return true;
	}
	else
		return false;
}

CFileFindData::CFileFindData(const string& sFileName, DWORD dwFileSize/*=0*/, DWORD dwFileAttributes/*=0*/, const FILETIME* ftLastWriteTime/*=0*/, const FILETIME* ftCreationTime/*=0*/, const FILETIME* ftLastAccessTime/*=0*/)
{
	m_sFileName = sFileName;
	m_dwFileSize = dwFileSize;
	m_dwFileAttributes = dwFileAttributes;
	if (ftLastWriteTime)
		m_ftLastWriteTime = *ftLastWriteTime;
	else
		memset(&m_ftLastWriteTime, 0, sizeof(FILETIME));
	if (ftCreationTime)
		m_ftCreationTime = *ftCreationTime;
	else
		memset(&m_ftCreationTime, 0, sizeof(FILETIME));
	if (ftLastAccessTime)
		m_ftLastAccessTime = *ftLastAccessTime;
	else
		memset(&m_ftLastAccessTime, 0, sizeof(FILETIME));
}



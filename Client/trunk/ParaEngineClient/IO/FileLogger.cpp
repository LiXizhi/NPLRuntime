//-----------------------------------------------------------------------------
// Class: CFileLogger
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2006.12
// Notes: 
//-----------------------------------------------------------------------------
/*
============================
HOW TO USE THE CFILELOGGER
============================
@see also InstallFiles.txt

-- (1) define FILE_LOGGER_HOOK in FileManager.cpp and define LOG_FILES_ACTIVITY in ParaEngineAPP.cpp.
-- (2) run the application as many times as possible, and make sure all files have been covered at least once during these runs. The result will be at "temp/filelog.txt"
-- (3) delete all files in ./_InstallFiles/ directory, since the extractor will not override older files. A clean rebuild requires deleting all files in this destination directory.
-- (4) call CFileLogger::MirrorFiles("_InstallFiles/"), or define EXTRACT_INSTALL_FILE in ParaEngineAPP.cpp. All files will be copied to _InstallFiles/ directory.
*/
#include "ParaEngine.h"
#ifdef WIN32
#include "ParaEngineInfo.h"
#include "FileManager.h"
#include "Filelogger.h"
#include "StringHelper.h"

using namespace ParaEngine;

CFileLogger::CFileLogger(void)
{
	m_bIsBegin = false;
}

CFileLogger::~CFileLogger(void)
{
}

CFileLogger* CFileLogger::GetInstance()
{
	static CFileLogger g_singleton;
	return &g_singleton;
}

void CFileLogger::Reset()
{
	m_entries.clear();
}

bool CFileLogger::IsBegin()
{
	return m_bIsBegin;
}

bool CFileLogger::BeginFileLog()
{
	m_bIsBegin = true;
	return true;
}

void CFileLogger::EndFileLog()
{
	m_bIsBegin = false;
}

void CFileLogger::AddFile( const char* sfilename,bool bReadOnly /*= true*/ )
{
	if (!IsBegin())
		return;
	string filename;
	CParaFile::ToCanonicalFilePath(filename, sfilename, false);
	
	if(!HasFile(filename))
	{
		// check if the filename contains wild characters like '*', if we will add all files with the matching name. 
		if(filename.find('*')==string::npos)
		{
			// this is a single file.
			m_entries.insert(filename);
		}
		else
		{
			// it contains wild characters, we will perform a file search. 
			string rootpath = CParaFile::GetParentDirectoryFromPath(filename);

			CSearchResult* result = CFileManager::GetInstance()->SearchFiles(rootpath, CParaFile::GetFileName(filename), "", 10, 10000000, 0);
			if(result!=0)
			{
				int nNum = result->GetNumOfResult();
				for(int i=0;i<nNum;++i)
				{
					string item = result->GetItem(i);
					if(CParaFile::GetFileExtension(item)  != "")
					{
						// file
						CFileLogger::AddFile((rootpath+item).c_str(), bReadOnly);
					}
					else
					{
						// folder
						CFileLogger::AddFile((rootpath+item).c_str(), bReadOnly);
					}
				}
				SAFE_RELEASE(result);
			}

		}
	}
}

bool CFileLogger::HasFile(const string& filename)
{
	return (m_entries.find(filename) != m_entries.end());
}


void CFileLogger::AddExcludeFilter( const char* sPattern )
{

}

bool CFileLogger::LoadLogFromFile( const char* filename )
{
	CParaFile file(filename);
	if (file.isEof())
	{
		return false;
	}
	bool bOldValue = m_bIsBegin;
	m_bIsBegin = true; // this ensures that AddFile() will be operational 
	char buf[500];
	memset(buf, 0, sizeof(buf));
	while (file.GetNextLine(buf, 500)>0)
	{
		AddFile(buf);
	}
	m_bIsBegin = bOldValue;
	OUTPUT_LOG("CFileLogger loaded logger file %s\r\n", filename);
	return true;
}

bool CFileLogger::SaveLogToFile( const char* filename )
{
	CParaFile file;
	if(file.CreateNewFile(filename))
	{
		file.WriteFormated("-- generated by CFileLogger of ParaEngine %s\r\n", ParaEngineInfo::CParaEngineInfo::GetVersion().c_str());
		file.WriteFormated("-- total files:  %d\r\n", (int)m_entries.size());
		set<string>::const_iterator itCur, itEnd = m_entries.end();
		for (itCur = m_entries.begin(); itCur!=itEnd; ++itCur)
		{
			file.WriteFormated("%s\r\n", itCur->c_str());
		}
		OUTPUT_LOG("CFileLogger saved logger file to %s\r\n", filename);
		return true;
	}
	return false;
}

void CFileLogger::MirrorFiles( const char* dirName,bool bOverwrite )
{
	OUTPUT_LOG("\r\n\r\nCFileLogger Mirrored files to to %s\r\n", dirName);
	bool bOldValue = m_bIsBegin;
	m_bIsBegin = false; // this ensures that AddFile() will NOT be operational 
	string destPath;
	string sExtractPath = dirName;
	set<string>::const_iterator itCur, itEnd = m_entries.end();
	for (itCur = m_entries.begin(); itCur!=itEnd; ++itCur)
	{
		destPath = sExtractPath+(*itCur);

		if(bOverwrite)
		{
			// do not override if the destination file has an older date. 
#ifdef DEFAULT_FILE_ENCODING
			WIN32_FIND_DATAW destData, srcData;
			std::wstring despPath16 = StringHelper::MultiByteToWideChar(destPath.c_str(), DEFAULT_FILE_ENCODING);
			LPCWSTR src16 = StringHelper::MultiByteToWideChar(itCur->c_str(), DEFAULT_FILE_ENCODING);
			if (FindFirstFileW(despPath16.c_str(), &destData) != INVALID_HANDLE_VALUE && FindFirstFileW(src16, &srcData) != INVALID_HANDLE_VALUE)
#else
			WIN32_FIND_DATA destData, srcData;
			if (FindFirstFile(destPath.c_str(), &destData) != INVALID_HANDLE_VALUE && FindFirstFile(itCur->c_str(), &srcData) != INVALID_HANDLE_VALUE)
#endif			
			{
				ULARGE_INTEGER destFileTime;
				destFileTime.LowPart = destData.ftLastWriteTime.dwLowDateTime;
				destFileTime.HighPart = destData.ftLastWriteTime.dwHighDateTime;
				ULARGE_INTEGER srcFileTime;
				srcFileTime.LowPart = srcData.ftLastWriteTime.dwLowDateTime;
				srcFileTime.HighPart = srcData.ftLastWriteTime.dwHighDateTime;
				if(destFileTime.QuadPart>=srcFileTime.QuadPart)
				{
					continue;
				}
			}
		}
		else
		{
			// we will skip files which have been mirrored before
			if (CParaFile::DoesFileExist(destPath.c_str(), false))
			{
				continue;
			}
		}

		bool bSuc = false;
		CParaFile fileFrom(itCur->c_str());
		if(!fileFrom.isEof())
		{
			
			CParaFile fileTo;
			if(fileTo.CreateNewFile(destPath.c_str()))
			{
				fileTo.write(fileFrom.getBuffer(), (int)(fileFrom.getSize()));
				bSuc = true;
			}
		}
		if(bSuc)
		{
			OUTPUT_LOG("%s is successfully mirrored\r\n", destPath.c_str());
		}
		else
		{
			OUTPUT_LOG("warning: %s is not extracted during CFileLogger::MirrorFiles\r\n", destPath.c_str());
		}
	}
	m_bIsBegin = bOldValue;
}

void CFileLogger::UpdateMirrorFiles( const char* dirName, bool bOverwrite /*= true*/ )
{
#ifdef _DEBUG
	// to prevent people to hack into pkg file, we will only enable it in debug build. 	
	LoadLogFromFile("InstallFiles.txt");
	LoadLogFromFile("temp/filelog.txt");
	return MirrorFiles(dirName, bOverwrite);
#endif
}
#endif
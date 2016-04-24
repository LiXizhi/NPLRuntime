//-----------------------------------------------------------------------------
// Class: ZipWriter
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2006.1.10
// Revised: 2016.4.24
// Notes: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ZipWriter.h"


#ifdef PARAENGINE_CLIENT
#include "zlib/zip.h"
#endif

/**@def define this macro to compile with ZLIB. */
#define COMPILE_WITH_ZLIB
#ifdef COMPILE_WITH_ZLIB
#include <zlib.h>
#endif

using namespace ParaEngine;

CZipWriter::CZipWriter()
	:m_handle(NULL)
{

}

CZipWriter::CZipWriter(void* handle)
	: m_handle(handle)
{
}

CZipWriter* CZipWriter::CreateZip(const char *fn, const char *password)
{
#ifdef PARAENGINE_CLIENT
	return new CZipWriter(::CreateZip(fn, password));
#else
	return 0;
#endif
}

DWORD CZipWriter::ZipAdd(const char* dstzn, const char* fn)
{
#ifdef PARAENGINE_CLIENT
	return ::ZipAdd((HZIP)m_handle, dstzn, fn);
#else
	return 0;
#endif
}

DWORD CZipWriter::ZipAddFolder(const char* dstzn)
{
#ifdef PARAENGINE_CLIENT
	return ::ZipAddFolder((HZIP)m_handle, dstzn);
#else
	return 0;
#endif
}

DWORD CZipWriter::AddDirectory(const char* dstzn, const char* filepattern, int nSubLevel)
{
#ifdef PARAENGINE_CLIENT
	string sDestFolder = dstzn;
	if (sDestFolder.size() > 0)
	{
		char lastChar = sDestFolder[sDestFolder.size() - 1];
		if (lastChar != '\\' &&  lastChar != '/')
		{
			sDestFolder += "/";
		}
	}

	string rootpath = CParaFile::GetParentDirectoryFromPath(filepattern);

	//////////////////////////////////////////////////////////////////////////
	// add the dstzn folder itself
	if (ZipAddFolder((sDestFolder).c_str()) != ZR_OK)
	{
		OUTPUT_LOG("warning: unable to add folder %s to zip. \n", (sDestFolder).c_str());
	}

	CSearchResult* result = CFileManager::GetInstance()->SearchFiles(rootpath, CParaFile::GetFileName(filepattern), "", nSubLevel, 10000000, 0);
	if (result != 0)
	{
		int nNum = result->GetNumOfResult();
		for (int i = 0; i < nNum; ++i)
		{
			string item = result->GetItem(i);
			if (CParaFile::GetFileExtension(item) != "")
			{
				DWORD result = ZipAdd((sDestFolder + item).c_str(), (rootpath + item).c_str());

				if (result == ZR_NOFILE)
				{
					// ZipAdd internally uses synchronous IO, however some database files may be opened with asynchronous flag,
					// please see CreateFile(..., FILE_FLAG_OVERLAPPED,...). In such situations, I will :
					// - copy the file using asynchronous IO to a temp location
					// - add zip from the temp location with synchronous IO
					// - delete the temp zip at the temp location. 
					string diskfile = (rootpath + item).c_str();
					string tempfile = diskfile + ".temp";
					if (CopyFile(diskfile.c_str(), tempfile.c_str(), FALSE))
					{
						result = ZipAdd((sDestFolder + item).c_str(), tempfile.c_str());
						if (!DeleteFile(tempfile.c_str()))
						{
							OUTPUT_LOG("warning: unable to delete temp file %s during zipping \n", tempfile.c_str());
						}
					}
					else
					{
						OUTPUT_LOG("warning: unable to add file %s to zip. It may be used by another application.\n", diskfile.c_str());
					}
				}

				if (result != ZR_OK)
				{
					OUTPUT_LOG("warning: unable to add file %s to zip. \n", (rootpath + item).c_str());
				}
			}
			else
			{
				if (!item.empty())
				{
					if (ZipAddFolder((sDestFolder + item).c_str()) != ZR_OK)
					{
						OUTPUT_LOG("warning: unable to add folder %s to zip. \n", (sDestFolder + item).c_str());
					}
				}
			}
		}
		SAFE_RELEASE(result);
	}

	return 0;
#else
	return 0;
#endif
}

DWORD CZipWriter::close()
{
#ifdef PARAENGINE_CLIENT
	DWORD result = ::CloseZip((HZIP)m_handle);
	m_handle = 0;
	return result;
#else
	return 0;
#endif
}

void CZipWriter::Release()
{
	close();
	delete this;
}

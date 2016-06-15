//-----------------------------------------------------------------------------
// Copyright (C) 2004 - 2006 ParaEngine Dev Studio, All Rights Reserved.
// Date:	2005.3
// Description:	API for File IO in ParaEngine
//-----------------------------------------------------------------------------
#pragma once
#include "FileHandle.h"
#include "FilePath.h"
#include "FileSearchResult.h"
#include "ParaFile.h"

namespace ParaEngine
{
	using namespace std;
	class CArchive;
	class CParaFile;
	class CSearchResult;
	class BlockReadWriteLock;

	/**
	* this is the main file interface exposed by ParaEngine.
	* it is mainly used as a singleton class.
	*/
	class CFileManager : public ISearchPathManager
	{
	public:
		CFileManager(void);
		virtual ~CFileManager(void);
		
		ATTRIBUTE_DEFINE_CLASS(CFileManager);
		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);

		/** get attribute by child object. used to iterate across the attribute field hierarchy. */
		virtual IAttributeFields* GetChildAttributeObject(const std::string& sName);
		/** get the number of child objects (row count) in the given column. please note different columns can have different row count. */
		virtual int GetChildAttributeObjectCount(int nColumnIndex = 0);
		virtual IAttributeFields* GetChildAttributeObject(int nRowIndex, int nColumnIndex = 0);


		/** singleton instance */
		PE_CORE_DECL static CFileManager * GetInstance();
	public:
		
		/** add archive to manager 
		* @param path: path of the zip or pkg file. 
		* @param bUseRelativePath: if this is true, files in the archive file will be relative to the parent directory of archive path. 
		*/
		PE_CORE_DECL bool OpenArchive(const string&  path, bool bUseRelativePath=false);
		
		/** add archive to manager 
		* @param path: path of the zip or pkg file. 
		* @param sRootDir: files in the archive will be regarded as relative to this this root directory. If this is "", there is no root directory set. 
		* such as "model/", "script/", characters after the last slash is always stripped off. 
		*/
		PE_CORE_DECL bool OpenArchiveEx(const string&  path, const string&  sRootDir);

		/** close an archive. When done with an archive, one should always close it. 
		Too many opened archives will compromise the IO performances. */
		PE_CORE_DECL void CloseArchive(const string&  path);

		/**
		* load a file by name.
		* delete the pFile object when u does not need it. This is because, if the file
		* is on disk, the file needs to release its handle. 
		* @param pFile: [out] the returned file will be loaded in it.
		* @param fileName: file name
		*/
		PE_CORE_DECL void LoadFile(CParaFile* pFile, const char* fileName);

		
		/** Open a file for immediate reading */
		PE_CORE_DECL bool OpenFile(const char* filename, FileHandle& handle);

		/** get uncompressed file size. */
		PE_CORE_DECL DWORD GetFileSize(FileHandle& handle);

		/** read file. */
		PE_CORE_DECL bool ReadFile(FileHandle& handle,LPVOID lpBuffer,DWORD nNumberOfBytesToRead,LPDWORD lpNumberOfBytesRead);

		/** read the raw (may be compressed file) 
		* @param lppBuffer: the buffer to hold the (compressed) output data. one need to use the SAFE_DELETE_ARRAY() to delete the output data. 
		* @param pnCompressedSize: output the size of the compressed data
		* @param pnUncompressedSize:output the size of the uncompressed data. if 0, it means that the data is not compressed. 
		* @return true if succeed. 
		*/
		PE_CORE_DECL bool ReadFileRaw(FileHandle& handle,LPVOID* lppBuffer,LPDWORD pnCompressedSize, LPDWORD pnUncompressedSize);

		/** close file. */
		PE_CORE_DECL bool CloseFile(FileHandle& hFile);

		/** search files at once. @see CSearchResult
		* the current version of this function can support only one query at a time. The search result is invalid if called intermitantly
		* @param sRootPath: the root path. for example: "", "xmodel/","xmodel/models/". Other format is not acceptable
		* @param sFilePattern: file pattern, e.g. "*.x" (all files with x extension), "*" (any files), "*."(directories only)
		* @param sZipArchive: it can be "" or a zip archive file name. if it is "", only disk files are saved. if it is not, only that archive files are saved.
		* @param nSubLevel: how many sub folders of sRootPath to look into. default value is 0, which only searches the sRootPath folder.
		* @param nMaxFilesNum: one can limit the total number of files in the search result. Default value is 50. the search will stop at this value even there are more matching files.
		* @param nFrom: only contains results from nFrom to (nFrom+nMaxFilesNum)
		* @return: one should manually release the search result.
		*/
		PE_CORE_DECL CSearchResult* SearchFiles(const string& sRootPath, const string& sFilePattern, const string& sZipArchive, int nSubLevel=0, int nMaxFilesNum=50, int nFrom=0);

		/**
		* Check whether a given file exists on disk. 
		* @param filename: file name to check
		*/
		PE_CORE_DECL bool DoesFileExist(const char* filename);

	protected:
		/** a list of all archives */
		list <CArchive*> m_archivers;
		int m_priority;
		BlockReadWriteLock* m_pArchiveLock;
	private:
		/** this is a recursive function. @see SearchFiles */
		static void FindDiskFiles(CSearchResult& result, const string& sRootPath, const string& sFilePattern, int nSubLevel);

		friend class CParaFile;
	};
}
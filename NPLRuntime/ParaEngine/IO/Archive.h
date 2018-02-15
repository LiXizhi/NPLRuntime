#pragma once
#include "FileHandle.h"
#include <string>
#include "IAttributeFields.h"

namespace ParaEngine
{
	class CSearchResult;
	using namespace std;

	/** file archiver base class. */
	class CArchive : public IAttributeFields
	{
	public:
		CArchive(void):m_archiveHandle(NULL), m_bOpened(false),m_nPriority(0){};
		virtual ~CArchive(void);
		ATTRIBUTE_DEFINE_CLASS(CArchive);

		inline static DWORD TypeID() {return 0;};
		virtual DWORD GetType(){ return TypeID();};
	public:
		virtual const std::string& GetIdentifier();

		const string& GetArchiveName() const;

		/** whether the archive represent the given file. Please note that pkg and zip file extension and slash letters are ignored. 
		* @param sFilename: is archive name
		*/
		virtual bool IsArchive(const std::string& sFilename);

		/** open archive 
		* @param nPriority: the smaller the number, the higher the priority with regard to other archive 
		*/
		virtual bool Open(const string& sArchiveName, int nPriority);

		/** close archive */
		virtual void Close(){};
		/**
		* Check whether a given file exists
		* @param filename: file name to check
		*/
		virtual bool DoesFileExist(const string& filename)=0;
		
		/**
		* Open a file for immediate reading.
		* call getBuffer() to retrieval the data
		* @param filename: the file name to open
		* @param handle to the opened file.
		* @return : true if succeeded.
		*/
		virtual bool OpenFile(const char* filename, FileHandle& handle) = 0;

		/** get file size. */
		virtual DWORD GetFileSize(FileHandle& handle) = 0;

		/** read file. */
		virtual bool ReadFile(FileHandle& handle,LPVOID lpBuffer,DWORD nNumberOfBytesToRead,LPDWORD lpNumberOfBytesRead) = 0;
		
		/** read the raw (may be compressed file) 
		* @param lppBuffer: the buffer to hold the (compressed) output data. one need to use the SAFE_DELETE_ARRAY() to delete the output data. 
		* @param pnCompressedSize: output the size of the compressed data
		* @param pnUncompressedSize:output the size of the uncompressed data. if 0, it means that the data is not compressed. 
		* @return true if succeed. 
		*/
		virtual bool ReadFileRaw(FileHandle& handle,LPVOID* lppBuffer,LPDWORD pnCompressedSize, LPDWORD pnUncompressedSize){return false;};

		virtual bool WriteFile(FileHandle& handle,LPCVOID lpBuffer,DWORD nNumberOfBytesToWrite,LPDWORD lpNumberOfBytesWritten){return false;};
		
		/** close file. */
		virtual bool CloseFile(FileHandle& hFile) = 0;
		
		/** create a new file for writing
		* @param bAutoMakeFilePath: if true, the file path will be created, if not exists
		*/
		virtual bool CreateNewFile(const char* filename, FileHandle& handle, bool bAutoMakeFilePath = true){return false;};

		/** set root directory 
		* @param filename: the parent directory filename will be regarded as the root directory.
		* m_bRelativePath will be set to false if filename or its parent directory is ""; otherwise it will be set to true.
		*/
		virtual void SetRootDirectory(const string& filename){};

		/** set the base directory to be removed from the relative path of all files in the zip file.
		* call this function only once, it will actually modify the relative file path.
		*/
		virtual void SetBaseDirectory(const char * filename) {};

		/** 
		* this is a recursive function. Finding a file inside the zip. 
		* search files at once. @see CSearchResult
		* the current version of this function can support only one query at a time. The search result is invalid if called intermittently
		* @param sRootPath: the parent path inside the zip where to search for. for example: "", "xmodel/","xmodel/models/". Other format is not acceptable
		* @param sFilePattern: file pattern, e.g. "*.x" (all files with x extension), "*" (any files), "*."(directories only)
		* @param nSubLevel: how many sub folders of sRootPath to look into. default value is 0, which only searches the sRootPath folder.
		*/
		virtual void FindFiles(CSearchResult& result, const string& sRootPath, const string& sFilePattern, int nSubLevel){};

	protected:
		/// file name
		string		m_filename;
		FileHandle	m_archiveHandle;
		bool		m_bOpened;
		int			m_nPriority;
	};
}

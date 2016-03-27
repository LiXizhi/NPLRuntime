#pragma once

/// define this macro to save zip header in memory.
// #define SAVE_ZIP_HEADER

#include "Archive.h"
#include "Array.h"
#include <vector>
#include "util/mutex.h"

#ifdef PARAENGINE_CLIENT
#include "zlib/zip.h"
#endif

namespace ParaEngine
{
	using namespace std;
	class IReadFile;

	const WORD ZIP_FILE_ENCRYPTED =			0x0001; // set if the file is encrypted
	const WORD ZIP_INFO_IN_DATA_DESCRITOR =	0x0008; // the fields crc-32, compressed size
	/// Size of end of central record (excluding variable fields)
	const int ZIP_CONST_ENDHDR = 22;
	/// End of central directory record signature
	const DWORD ZIP_CONST_ENDSIG = 0x06054B50;
	/// Signature for central header
	const int ZIP_CONST_CENSIG = 'P' | ('K' << 8) | (1 << 16) | (2 << 24);


	// and uncompressed size are set to zero in the local
	// header

	// Set byte packing for ZIP file header definition
#	pragma pack( push, packing )
#	pragma pack( 1 )

	struct ZIP_EndOfCentralDirectory
	{
		short thisDiskNumber;
		short startCentralDirDisk;
		short entriesForThisDisk;
		short entriesForWholeCentralDir;
		int centralDirSize;
		int offsetOfCentralDir;
		short commentSize;
	};

	struct ZIP_CentralDirectory
	{
		DWORD Sig;
		WORD MadeByVersion;
		WORD ExtractVersion;
		WORD Flags;
		WORD CompressionMethod;
		DWORD Time;
		DWORD FileCRC;
		DWORD PackSize;
		DWORD UnPackSize;
		WORD NameSize;
		WORD ExtraSize;
		WORD CommentSize;
		WORD DiskNumberStart;
		WORD InternalAttributes;
		DWORD ExternalAttributes;
		DWORD LocalHeaderOffset;
	};

	struct SZIPFileDataDescriptor
	{
		DWORD CRC32;
		DWORD CompressedSize;
		DWORD UncompressedSize;
	};

	struct SZIPFileHeader
	{
		DWORD Sig;
		WORD VersionToExtract;
		WORD GeneralBitFlag;
		WORD CompressionMethod;
		WORD LastModFileTime;
		WORD LastModFileDate;
		SZIPFileDataDescriptor DataDescriptor;
		WORD FilenameLength;
		WORD ExtraFieldLength;
	};

	// Default alignment
#	pragma pack( pop, packing )


	/** light-weighted file record header in memory. */
	struct SZipFileEntry
	{
		string zipFileName;
		DWORD fileDataPosition; // position of compressed data in file

		WORD CompressionMethod;
		DWORD CompressedSize;
		DWORD UncompressedSize;

		DWORD LastModifiedTime;
		string zipFileNameOriginal;

#ifdef SAVE_ZIP_HEADER
		SZIPFileHeader header;
#endif 

	public:
		SZipFileEntry()
			: fileDataPosition(0), CompressedSize(0),UncompressedSize(0),CompressionMethod(0),LastModifiedTime(0)
		{
#ifdef SAVE_ZIP_HEADER
			memset(&header, 0, sizeof(SZIPFileHeader)); 
#endif 
		};
		~SZipFileEntry(){};

		bool operator < (const SZipFileEntry& other) const
		{
			return zipFileName < other.zipFileName;
		}
		bool operator == (const SZipFileEntry& other) const
		{
			return zipFileName == other.zipFileName;
		}
	};

	struct SZipFileEntryPtr
	{
		SZipFileEntry* m_pEntry;
	public:
		SZipFileEntryPtr():m_pEntry(NULL){};
		SZipFileEntryPtr(const SZipFileEntryPtr& entry):m_pEntry(entry.m_pEntry){};
		SZipFileEntryPtr(SZipFileEntry* pEntry):m_pEntry(pEntry){};

		bool operator < (const SZipFileEntryPtr& other) const
		{
			return m_pEntry->zipFileName < other.m_pEntry->zipFileName;
		}
		bool operator == (const SZipFileEntryPtr& other) const
		{
			return m_pEntry->zipFileName == other.m_pEntry->zipFileName;
		}
	};

	/**
	Both the zip and ParaEngine's pkg file encryption logics are supported. pkg file is a slightly encrypted version of zip file. 

	Load Archive File Logics
	========================
	if(sArchiveName is "[filename].pkg") then
	Load "[filename].pkg"
	return
	end
	Suppose the loaded sArchiveName is "[filename].zip", 
	if(At debug mode) then
	if "[filename].zip" does not exist then
	if "[filename].pkg" exists then
	Load "[filename].pkg"
	else
	No file loaded.
	end
	else
	if "[filename].pkg" exists and its modified date is newer than the "[filename].zip" then
	Load "[filename].pkg"
	else
	Load "[filename].zip"
	generate a new file called "[filename].pkg" based on "[filename].zip"
	end
	end
	else 
	if "[filename].zip" exist then
	Load "[filename].zip"
	else if "[filename].pkg" exist then
	Load "[filename].pkg"
	else
	No file loaded.
	end
	end

	ZIP file logics
	========================
	It uses the ZIP file's CentralDirectory record at the end of the zip file to build the 
	file list for random file access. 

	ParaEngine pkg File Format
	========================
	char[4] <file type id> ".pkg"
	byte[4] <pkg file version> 0.0.0.1
	[int] <length of string>
	[string] "ParaEngine Tech Studio Package File Format. Please note that content in 
	this file is meant to be protected and copyrighted by their author(s). Decoding this pkg file is illegal."
	[int] reserved1
	[int] reserved2
	[int] reserved3
	[int] reserved4
	[int] Central Directory Size in Bytes
	[int] number of directory record
	[item 1]: LenOfFilename, filename, compression method, compressed size,uncompressed size, fileDataPosition(this field is encrypted)
	...
	[item n]
	[data 1]: header+data
	...
	[data n]
	*/
	class CZipArchive :	public CArchive
	{
	public:
		CZipArchive(void);
		CZipArchive(bool bIgnoreCase);
		virtual ~CZipArchive(void);
		inline static DWORD TypeID() { return 2; };
		virtual DWORD GetType(){ return TypeID(); };

		ATTRIBUTE_DEFINE_CLASS(CZipArchive);
		ATTRIBUTE_METHOD1(CZipArchive, SetBaseDirectory_s, const char*)	{ cls->SetBaseDirectory(p1); return S_OK; }

		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);
		
	public:
		/** open archive */
		virtual bool Open(const string& sArchiveName, int nPriority);

		/** close archive */
		virtual void Close();

		/**
		* Check whether a given file exists
		* @param filename: file name to check
		*/
		virtual bool DoesFileExist(const string& filename);

		/**
		* Open a file for immediate reading. It could be either a zip file or a pkg file. 
		* call getBuffer() to retrieval the data
		* @param filename: the file name to open
		* @param handle to the opened file.
		* @return : true if succeeded.
		*/
		virtual bool OpenFile(const char* filename, FileHandle& handle);

		/* open a zip file in memory 
		* @param buffer: 
		* @param nLen: size in byte of the buffer
		* @param bDeleteBuffer: true if the zip file will take the ownership of the buffer and will delete it on exit.
		*/
		bool OpenMemFile(const char* buffer, DWORD nLen, bool bDeleteBuffer=true);

		/** get file size. */
		virtual DWORD GetFileSize(FileHandle& handle);

		/** get file name in the zip package */
		virtual string GetNameInArchive(FileHandle& handle);

		/** get file original name in the zip package (in case the name is converted lower-cases when case-insensitive). */
		virtual string GetOriginalNameInArchive(FileHandle& handle);

		/** read file. */
		virtual bool ReadFile(FileHandle& handle,LPVOID lpBuffer,DWORD nNumberOfBytesToRead,LPDWORD lpNumberOfBytesRead,LPDWORD lpLastWriteTime);

		/** read the raw (may be compressed file) 
		* @param lppBuffer: the buffer to hold the (compressed) output data. one need to use the SAFE_DELETE_ARRAY() to delete the output data. 
		* @param pnCompressedSize: output the size of the compressed data
		* @param pnUncompressedSize:output the size of the uncompressed data. if 0, it means that the data is not compressed. 
		* @return true if succeed. 
		*/
		virtual bool ReadFileRaw(FileHandle& handle,LPVOID* lppBuffer,LPDWORD pnCompressedSize, LPDWORD pnUncompressedSize);

		/** decompress a file buffer */
		static bool Decompress(LPVOID lpCompressedBuffer, DWORD nCompressedSize, LPVOID lpUnCompressedBuffer, DWORD nUncompressedSize);

		/** close file. */
		virtual bool CloseFile(FileHandle& hFile);

		/** set root directory 
		* @param filename: the parent directory filename will be regarded as the root directory.
		* m_bRelativePath will be set to false if filename or its parent directory is ""; otherwise it will be set to true.
		*/
		virtual void SetRootDirectory(const string& filename);

		/** set the base directory to be removed from the relative path of all files in the zip file. 
		*/
		virtual void SetBaseDirectory(const char * filename);

		/** 
		* this is a recursive function. Finding a file inside the zip. 
		* search files at once. @see CSearchResult
		* the current version of this function can support only one query at a time. The search result is invalid if called intermittently
		* @param sRootPath: the parent path inside the zip where to search for. for example: "", "xmodel/","xmodel/models/". Other format is not acceptable
		* @param sFilePattern: file pattern, e.g. "*.x" (all files with x extension), "*" (any files), "*."(directories only)
		* @param nSubLevel: how many sub folders of sRootPath to look into. default value is 0, which only searches the sRootPath folder.
		*  Note: currently only we only support nSubLevel=0
		*/
		virtual void FindFiles(CSearchResult& result, const string& sRootPath, const string& sFilePattern, int nSubLevel);

		/**
		* Generate a pkg file which is equivalent to the currently loaded zip file. 
		* This function can only be called, when a zip file has been successfully loaded. 
		* @param filename the pkg file name to be generated. It will overwrite existing file
		* @return true if successful.
		*/
		bool GeneratePkgFile(const char* filename);

		/** get total file count. */
		int GetFileCount();
	private:
		IReadFile* m_pFile;
		array<SZipFileEntryPtr> m_FileList;
		SZipFileEntry* m_pEntries;


		bool m_bIgnoreCase;
		/** if this is true, all file path in this archive will be regarded as in the m_sRootPath, where m_sRootPath is usually the archive file's parent directory.*/
		bool m_bRelativePath;
		/** @see m_bRelativePath*/
		string m_sRootPath;
		char* m_zipComment;
		/** For sequential File IO. */
		ParaEngine::mutex m_mutex;
	private:
		/** return file index. -1 is returned if file not found.*/
		int findFile(const string& sFilename);
		IReadFile* openFile(int index);
		/* open a zip file. this function is only called inside OpenFile() virtual method */
		bool OpenZipFile(const string& filename);
		/* open a pkg file. this function is only called inside OpenFile() virtual method */
		bool OpenPkgFile(const string& filename);

		/**  
		* search the last occurrence of a integer signature in the range [endLocation-minimumBlockSize-maximumVariableData, endLocation-minimumBlockSize]
		* -1 is returned if not found.
		*/
		int LocateBlockWithSignature(DWORD signature, long endLocation, int minimumBlockSize, int maximumVariableData);

		/** Search for and read the central directory of a zip file filling the entries
		* array.  This is called exactly once by the constructors.
		*/
		bool ReadEntries();

		/** read pkg entry*/
		bool ReadEntries_pkg();
	};


	/**
	* creating zip files
	* 
	* e.g.
	*  (1) Traditional use, creating a zipfile from existing files
	* CZipWriter* writer = CZipWriter::CreateZip("c:\\simple1.zip","");
	* writer->ZipAdd("znsimple.bmp", "c:\\simple.bmp");
	* writer->ZipAdd("znsimple.txt", "c:\\simple.txt");
	* writer->close();
	*/
	class CZipWriter
	{
	public:
		CZipWriter();
		CZipWriter(void* handle);
		/** 
		* call this to start the creation of a zip file.
		* one need to call Release()
		*/
		static CZipWriter* CreateZip(const char *fn, const char *password);

		/**
		* add a zip file to the zip. file call this for each file to be added to the zip.
		* @return: 0 if succeed.
		*/
		DWORD ZipAdd(const char* dstzn, const char* fn);
		/**
		* add a zip folder to the zip file. call this for each folder to be added to the zip.
		* @return: 0 if succeed.
		*/
		DWORD ZipAddFolder(const char* dstzn);

		/**
		* add everything in side a directory to the zip. 
		* e.g. AddDirectory("myworld/", "worlds/myworld/*.*", 10);
		* @param dstzn: all files in fn will be appended with this string to be saved in the zip file.
		* @param filepattern: file patterns, which can include wild characters in the file portion.
		* @param nSubLevel: sub directory levels. 0 means only files at parent directory.
		*/
		DWORD AddDirectory(const char* dstzn, const char* filepattern, int nSubLevel=0);

		/**
		* call this when you have finished adding files and folders to the zip file.
		* Note: you can't add any more after calling this.
		*/
		DWORD close();

		/** close and delete this*/
		void Release();

	public:
		void* m_handle;
	};
}

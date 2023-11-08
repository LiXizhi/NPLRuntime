#pragma once

/// define this macro to save zip header in memory.
// #define SAVE_ZIP_HEADER

#include "Archive.h"
#include "Array.h"
#include <vector>
#include "util/mutex.h"

namespace ParaEngine
{
	using namespace std;
	class IReadFile;

	const WORD ZIP_FILE_ENCRYPTED =			0x0001; // set if the file is encrypted
	const WORD ZIP_INFO_IN_DATA_DESCRITOR =	0x0008; // the fields crc-32, compressed size
	/// Size of end of central record (excluding variable fields)
	const int ZIP_CONST_ENDHDR = 22;
	/// local file header
	const int ZIP_CONST_LOCALHEADERSIG = 'P' | ('K' << 8) | (3 << 16) | (4 << 24);
	/// Signature for central header
	const int ZIP_CONST_CENSIG = 'P' | ('K' << 8) | (1 << 16) | (2 << 24);
	/// End of central directory record signature
	const DWORD ZIP_CONST_ENDSIG = 'P' | ('K' << 8) | (5 << 16) | (6 << 24);

	// and uncompressed size are set to zero in the local
	// header

	// Set unsigned char packing for ZIP file header definition
#	pragma pack( push, packing )
#	pragma pack( 1 )

	struct ZIP_EndOfCentralDirectory
	{
		uint16_t thisDiskNumber;
		uint16_t startCentralDirDisk;
		uint16_t entriesForThisDisk;
		uint16_t entriesForWholeCentralDir;
		uint32_t centralDirSize;
		uint32_t offsetOfCentralDir;
		uint16_t commentSize;
	};

	struct ZIP_EndOfCentralDirectoryBlock
	{
		DWORD sig;
		uint16_t thisDiskNumber;
		uint16_t startCentralDirDisk;
		uint16_t entriesForThisDisk;
		uint16_t entriesForWholeCentralDir;
		uint32_t centralDirSize;
		uint32_t offsetOfCentralDir;
		uint16_t commentSize;
	};

	struct ZIP_CentralDirectory
	{
		DWORD Sig;
		WORD MadeByVersion;
		WORD ExtractVersion;
		WORD Flags;
		WORD CompressionMethod;
		WORD LastModFileTime;
		WORD LastModFileDate;
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
		//string zipFileName;
		WORD fileNameLen;
		const char* zipFileName;
		uint32 hashValue;
		DWORD fileDataPosition; // position of compressed data in file

		WORD CompressionMethod;
		DWORD CompressedSize;
		DWORD UncompressedSize;

		DWORD LastModifiedTime;

		const char* zipFileNameOriginal;
#ifdef SAVE_ZIP_HEADER
		SZIPFileHeader header;
#endif 

	public:
		SZipFileEntry()
			: fileDataPosition(0), CompressedSize(0), UncompressedSize(0), CompressionMethod(0), hashValue(0), zipFileName(nullptr), fileNameLen(0), LastModifiedTime(0), zipFileNameOriginal(nullptr)
		{
#ifdef SAVE_ZIP_HEADER
			memset(&header, 0, sizeof(SZIPFileHeader)); 
#endif 
		};
		~SZipFileEntry(){};

		void SetFileName(const char* str, WORD len)
		{
			zipFileName = str;
			fileNameLen = len;
		}

		void RefreshHash(bool ignoreCase)
		{
			if (zipFileName)
				hashValue = Hash(zipFileName, ignoreCase);
		}

		static uint32 Hash(const char* str, bool ignoreCase)
		{
			const size_t seed = 2166136261U;
			const size_t prime = 16777619U;
			const char diff = 'a' - 'A';

			uint32 ret = seed;

			const char* p = str;

			if (ignoreCase)
			{
				while (*p != 0)
				{
					auto cur = *p;

					if (cur >= 'A' && cur <= 'Z')
						cur += diff;
					ret ^= cur;
					ret *= prime;

					p++;
				}
			}
			else
			{
				while (*p != 0)
				{
					auto cur = *p;

					ret ^= cur;
					ret *= prime;

					p++;
				}
			}

			return ret;
		}
	};

	struct SZipFileEntryPtr
	{
		SZipFileEntry* m_pEntry;
	public:
		SZipFileEntryPtr():m_pEntry(NULL){};
		SZipFileEntryPtr(const SZipFileEntryPtr& entry):m_pEntry(entry.m_pEntry){};
		SZipFileEntryPtr(SZipFileEntry* pEntry):m_pEntry(pEntry){};
	};

	// check src data is zip file data
	bool IsZipData(const char* src, size_t size);
	// get first file info in zip file data
	const SZIPFileHeader* GetFirstFileInfo(const char* src, std::string* filename = nullptr);
	bool GetFirstFileData(const char* src, size_t size, std::string& out);

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
	unsigned char[4] <pkg file version> 0.0.0.1
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

		ATTRIBUTE_METHOD1(CZipArchive, GetRootDirectory_s, const char**) { *p1 = cls->GetRootDirectory().c_str(); return S_OK; }
		ATTRIBUTE_METHOD1(CZipArchive, SetRootDirectory_s, const char*) { cls->SetRootDirectory(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CZipArchive, GeneratePkgFileV1_s, const char*) { cls->GeneratePkgFile(p1); return S_OK; }
		ATTRIBUTE_METHOD1(CZipArchive, GeneratePkgFileV2_s, const char*) { cls->GeneratePkgFile2(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CZipArchive, GetFileCount_s, int*) { *p1 = cls->GetFileCount(); return S_OK; }

		ATTRIBUTE_METHOD1(CZipArchive, IsIgnoreCase_s, bool*) { *p1 = cls->IsIgnoreCase(); return S_OK; }
		
		ATTRIBUTE_METHOD1(CZipArchive, AddAliasFrom_s, const char*) { cls->AddAliasFrom(p1); return S_OK; }
		ATTRIBUTE_METHOD1(CZipArchive, AddAliasTo_s, const char*) { cls->AddAliasTo(p1); return S_OK; }


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

		/**
		* Open a file for immediate reading.
		* call getBuffer() to retrieval the data
		* @param item: the file find item to open
		* @param handle to the opened file.
		* @return : true if succeeded.
		*/
		virtual bool OpenFile(const ArchiveFileFindItem* item, FileHandle& handle);

		/* open a zip file in memory 
		* @param buffer: 
		* @param nLen: size in unsigned char of the buffer
		* @param bDeleteBuffer: true if the zip file will take the ownership of the buffer and will delete it on exit.
		*/
		bool OpenMemFile(const char* buffer, DWORD nLen, bool bDeleteBuffer=true);

		/** get file size. */
		virtual DWORD GetFileSize(FileHandle& handle);

		/** get file name in the package */
		virtual string GetNameInArchive(FileHandle& handle) override;

		/** get file original name in the package (in case the name is converted lower-cases when case-insensitive). */
		virtual string GetOriginalNameInArchive(FileHandle& handle) override;

		/** read file. */
		virtual bool ReadFile(FileHandle& handle, LPVOID lpBuffer, DWORD nNumberOfBytesToRead, LPDWORD lpNumberOfBytesRead, LPDWORD lpLastWriteTime);

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

		/** get root directory. all relative file path in zip files is regarded as relative to this directory. */
		const std::string& GetRootDirectory();

		/** set the base directory to be removed from the relative path of all files in the zip file. 
		* call this function only once, it will actually modify the relative file path.
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
		bool GeneratePkgFile2(const char* filename);

		/** get total file count. */
		int GetFileCount();

		virtual bool IsIgnoreCase() const { return m_bIgnoreCase;  }


		void AddAliasFrom(const char* from);
		void AddAliasTo(const char* to);

		/* add file alias*/
		void AddAlias(const std::string& from, const std::string& to);
		/* return true if there is an alias, and out contains the alias */
		const std::string * GetAlias(const std::string& from);
	private:
		IReadFile* m_pFile;
		vector<SZipFileEntryPtr> m_FileList;
		bool m_bDirty;
		SZipFileEntry* m_pEntries;
		// save name block
		vector<char> m_nameBlock;
		std::map<std::string, std::string> m_fileAliasMap;

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
		int findFile(const ArchiveFileFindItem* item);
		int findFileImp(const ArchiveFileFindItem* item, const char* filename, bool bRefreshHash);

		IReadFile* openFile(int index);
		/* open a zip file. this function is only called inside OpenFile() virtual method */
		bool OpenZipFile(const string& filename);
		/* open a pkg file. this function is only called inside OpenFile() virtual method */
		bool OpenPkgFile(const string& filename);

		void ReBuild();
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
		bool _ReadEntries_pkg();
		bool ReadEntries_pkg();
		bool ReadEntries_pkg2();
	};
}

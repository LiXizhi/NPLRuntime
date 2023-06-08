#pragma once
#include <list>
#include <vector>
#include <string>

namespace ParaEngine
{
	struct CParaFileInfo;
	class CArchive;
	
	using namespace std;
	
	/** where to search the file and where the file is found */
	enum FileLocation
	{
		FILE_NOT_FOUND = 0,
		FILE_ON_DISK = 1,
		FILE_ON_ZIP_ARCHIVE = 2,
		FILE_ON_SEARCH_PATH = 4,
		FILE_ON_EXECUTABLE = 8, // embedded resource file
	};

	/**
	* it presents a real or virtual file in ParaEngine.
	* The most simple use is
	*	CParaFile file("a.x");
	*	char* pBuf = file.GetBuffer();
	* It is very important to release the file object or call Close() method
	* to read the memory allocated during file loading. Currently it is automatically called in the destructor
	*/
	class CParaFile
	{
		// disable copying
		CParaFile(const CParaFile &f);

	private:
		static string m_strExtractRootPath;
		static bool m_bExtractFileToDisk;
		/** by default it is 0, which is same(above) archived files.*/
		static int m_nDiskFilePriority;
	public:
		/**
		* create an undefined file. This is usually for creating a new file. Call CreateNewFile() after this.
		*/
		PE_CORE_DECL CParaFile();

		/**
		* create a memory file from a given buffer. The memory file is usually read only.
		* @param buf : buffer input
		* @param nBufferSize : sizes of input buffer in bytes.
		* @param bCopyBuffer: if false, this file object will not release the buffer when the file is closed. so the caller has to do it after CParaFile is released.
		* if true, it will immediately make an internal copy of the buffer and release the newly created buffer when file closes.
		* one can later on use TakeBufferOwnership() when this is false.
		*/
		PE_CORE_DECL CParaFile(char* buf, int nBufferSize, bool bCopyBuffer = false);

		/** Open a file for immediate reading.
		* this is an alternative way to use CParaFile.
		* filenames are not case sensitive. Release the object will release the file.
		*/
		PE_CORE_DECL CParaFile(const char* filename);
		/** Open a file for immediate reading.
		* this is an alternative way to use CParaFile.
		* filenames are not case sensitive. Release the object will release the file.
		* @param relativePath: a relative path file name, such as "terrain/data/", or "terrain/data/abc.txt"
		*	in the latter case, "abc.txt" is ignored automatically.
		*	please note that it will first search the file as "filename", and if failed, it will search for "relativePath..filename"
		*/
		PE_CORE_DECL CParaFile(const char* filename, const char* relativePath);
		PE_CORE_DECL ~CParaFile();
		// static method
	public:
		/** same as MakeDirectoryFromFilePath. except that file name need not to be a Canonical File Path */
		PE_CORE_DECL static bool CreateDirectory(const char* filename);

		/** make directory
		* @param filename: file name should be a Canonical File Path. Use ,ToCanonicalFilePath
		* @return: true if the directory is made or already exists*/
		PE_CORE_DECL static bool MakeDirectoryFromFilePath(const char * filename);

		/** convert a file name to canonical file path
		* @param output: the output converted string. it can be the same string as input string
		* @param input: the input file name. it is assumed that strlen(filename) <= MAX_PATH
		* @param bBackSlash: if true, the file will use '\\'; otherwise use '/'. '\\' is win32 compatible. '/' is more user friendly.
		* @return: the canonical file path name returned. */
#ifdef WIN32
		PE_CORE_DECL static void ToCanonicalFilePath(string & output, const string& input, bool bBackSlash = true);
#else
		PE_CORE_DECL static void ToCanonicalFilePath(string & output, const string& input, bool bBackSlash = false);
#endif
		/** save as above
		* @param output: should be char output[MAX_PARAFILE_LINE_LENGTH]
		*/
#ifdef WIN32
		PE_CORE_DECL static void ToCanonicalFilePath(char* output, const char* input, bool bBackSlash = true);
#else
		PE_CORE_DECL static void ToCanonicalFilePath(char* output, const char* input, bool bBackSlash = false);
#endif

		/**
		* Check whether a given file exists on disk.
		* @param filename: file name to check
		* @param bSearchZipFiles: if false, not disk file is searched. If true, both the disk file and zip file will be searched.
		* @param bUseSearchPath: true to use search path. please note this function will NOT be thread thead if this is true. It must be in the same thread as the AddSearchPath.
		*/
		PE_CORE_DECL static bool DoesFileExist(const char* filename, bool bSearchZipFiles = false, bool bUseSearchPath = false);

		/** check file exist based on file location. similar to DoesFileExist, but gives more info on where the file is first found. 
		* @param dwWhereToSearch: bitwise field of FileLocation, default to FILE_ON_DISK
		* @param pDiskFilePath: if not NULL, it will contain the  actual disk file path. In case it is on search path, it is different from filename. 
		* @return int32: enum of FileLocation. return 0 if not found. 
		*/
		static int32 DoesFileExist2(const char* filename, uint32 dwWhereToSearch = FILE_ON_DISK, std::string* pDiskFilePath = NULL);

		/**
		*  Checks whether the path is an absolute path.
		*
		*  @note On Android, if the parameter passed in is relative to "assets/", this method will treat it as an absolute path.
		*        Also on Blackberry, path starts with "app/native/Resources/" is treated as an absolute path.
		*
		*  @param strPath The path that needs to be checked.
		*  @return true if it's an absolute path, otherwise it will return false.
		*/
		static bool IsAbsolutePath(const std::string& path);

		/** delete temporary file. temporary files are file in the ./temp/ directory of the ParaEngine's root dir.
		* this function is general used to delete temporary texture file generated during the game.
		* @param sFilePattern: such as "*.dds", "temp.txt", etc
		* @return: the number of file deleted.
		*/
		PE_CORE_DECL static int DeleteTempFile(const string& sFilePattern);
		/** delete a given file. It will reject any system files outside the application directory.
		* after all, this function is of high security level.
		* @param sFilePattern: such as "*.dds", "temp.txt", etc
		* @param bSecureFolderOnly: if true, we only allow user to delete allowed folders.
		* @return: the number of files deleted.
		*/
		PE_CORE_DECL static int DeleteFile(const string& sFilePattern, bool bSecureFolderOnly = true);

		/**
		* The MoveFile function will move (rename) either a file or a directory (including its children) either in the same directory or across directories.
		* @param src specifies the name of an existing file
		* @param dest specifies the name of the new file
		* @return true if succeeds
		*/
		PE_CORE_DECL static bool MoveFile(const char* src, const char* dest);

		/** backup a specified file, if the file exists. A new file with an extension ".bak" appended
		* to the end of the original file will be created, whose content is identical to the original file.
		* @param filename: file name to back up
		* @return: return true if the file is backed up. return false, if the file does not exist or some error occurs during backup.
		*/
		PE_CORE_DECL static bool BackupFile(const char* filename);

		/**
		* Get the CRC 32 code of a given file.
		* @return: return 0 if not succeed, otherwise the CRC32 code is returned.
		*/
		PE_CORE_DECL static unsigned long CRC32(const char* filename);

		/**
		* The CopyFile function copies an existing file to a new file
		* @param src specifies the name of an existing file
		* @param dest specifies the name of the new file
		* @param bOverride [in] If this parameter is false and the new file specified by src already exists, the function fails.
		* If this parameter is true and the new file already exists, the function overwrites the existing file and succeeds.
		* @return true if succeeds
		*/
		PE_CORE_DECL static bool CopyFile(const char* src, const char* dest, bool bOverride);

		/**
		* trim the sFile by nParentCounts number of parent directories.
		@param nParentCounts: number of parent directory to remove. 0 means the current directory,which is also the default value
		@return: return "" if the input directory does not have that many parent directories.
		e.g.
		"C:/abc/" = GetDirectoryFromPath("C:/abc/aaa",0);
		"C:/" = GetDirectoryFromPath("C:/abc/",1);
		*/
		PE_CORE_DECL static string GetParentDirectoryFromPath(const string& sfilename, int nParentCounts = 0);

		/** This will find the root path from a given directory path using the following rule:
		* find a file called "ParaEngine.sig" in the parent directories of sFile, from near to far.
		* e.g. if sFile is "c:/a/b/c/xxx.x", then it will search for "c:/a/b/c/","c:/a/b/","c:/a/" and "c:/".
		* the function will return the first parent directory that contains the file, otherwise "" is returned.
		*/
		PE_CORE_DECL static string AutoFindParaEngineRootPath(const string& sFile);
		/** change the file extension.
		* @param sFile: the file whose extension to change.
		* @param sExt: the file extension to change to. such as "dds","x"
		* @return: return the file with the changed extension. the input file does not contain
		* a valid file extension, the returned string will be identical to the input file.
		*/
		PE_CORE_DECL static string ChangeFileExtension(const string& sFile, const string & sExt);

		/** get the file extension. this function may return "" if no file extension is found */
		PE_CORE_DECL static string GetFileExtension(const string& sFile);

		/** Get the relative file path by stripping the root path from the beginning.
		* please note that all paths should uses slash "/", instead of backslash "\", in the path name.letter case is ignored
		* @param sAbsolutePath: the absolute path from which to obtain the relative path.
		* @param sRootPath: the parent root path, which will be removed from the absolute path. It should end with "/"
		* @return: the relative path is returned. If the absolute path does not math the root path, the absolute path
		*		is returned unchanged. the relative path does not begin with "/"
		* e.g. "a/b.x" = GetRelativePath("c:/lxzsrc/a/b.x", "c:/lxzsrc/");
		*      "c:/lxzsrc/a/b.x" = GetRelativePath("c:/lxzsrc/a/b.x", "c:/srclxz/"); // not match
		*/
		PE_CORE_DECL static string GetRelativePath(const string& sAbsolutePath, const string& sRootPath);

		/** Get the absolute file path by appending the root path before the relative path.
		* please note that all paths should uses slash "/", instead of backslash "\", in the path name. letter case is ignored
		* @param sRelativePath: the relative path. it also begin with "../../", "./", etc. 
		* @param sRootPath: the parent root path, it may end with "/". 
		* @return: the relative path is returned. If the absolute path does not math the root path, the absolute path
		*		is returned unchanged.
		* e.g. "c:/lxzsrc/a/b.x" = GetAbsolutePath("a/b.x", "c:/lxzsrc/");
		*/
		PE_CORE_DECL static string GetAbsolutePath(const string& sRelativePath, const string& sRootPath);

		/** get only the file name from the file path.
		* "a.x" = GetFileName("c:/lxzsrc/a.x");
		*/
		PE_CORE_DECL static string GetFileName(const string& sFilePath);

		/** return whether the given path is only a file name which contains no directory information. */
		PE_CORE_DECL static bool IsFileName(const string& sFilePath);

		/** get file size of a disk file in bytes. */
		PE_CORE_DECL static int GetFileSize(const char* sFilePath);

		/** in win32, this is the root directory,
		*  Gets the writable path.
		*  @return  The path that can be write/read a file in
		*/
		PE_CORE_DECL static const std::string& GetWritablePath();

		/** Note: NOT thread safe, only set at startup when there is just one thread running. */
		PE_CORE_DECL static void SetWritablePath(const std::string& writable_path);
		/** whether the given file is a writable path. For absolute file path, we only allow files in initial working directory and writable path. */
		PE_CORE_DECL static bool IsWritablePath(const std::string& filepath, bool bLogWarning = true);

		enum PARAENGINE_DIRECTORY{
			APP_ROOT_DIR = 0,
			APP_SCRIPT_DIR = 1,
			APP_ARCHIVE_DIR = 2, // xmodels
			APP_MODEL_DIR = 3,
			APP_SHADER_DIR = 4,
			APP_DATABASE_DIR = 5,
			APP_TEMP_DIR = 6,
			APP_USER_DIR = 7,
			APP_BACKUP_DIR = 8,
			APP_SCREENSHOT_DIR = 9,
			APP_PLUGIN_DIR = 10,
			APP_CONFIG_DIR = 11,
			APP_CHARACTER_DIR = 12,
			APP_SH_DESKTOP_DIR = 13,
			APP_SH_DESKTOP_FOLDER_DIR = 14,
			APP_SH_MYDOC_DIR = 15,
			APP_SH_FAV_DIR = 16,
			APP_SH_MUSIC_DIR = 17,
			APP_SH_PICTURE_DIR = 18,
			APP_SH_VIDEO_DIR = 19,
			APP_DEV_DIR = 20,
			APP_EXECUTABLE_DIR = 21,
			// only used in android/iOS. This is a directory where we can save user files.
			APP_EXTERNAL_STORAGE_DIR = 22,
			APP_LAST_DIR,
		};

		/** get the current directory of the application. it allows querying a number of standard directories.
		* please note that all directory are returned as absolute path with slash "/" between two level of directories.
		* and that it always ends with "/". e.g. "c:/lxzsrc/paraengineSDK/" or "c:/lxzsrc/paraengineSDK/script/"
		* @param dwDirectoryType: it can be one of the PARAENGINE_DIRECTORY enumeration type
		* @return: the directory is returned. */
		PE_CORE_DECL static const string& GetCurDirectory(DWORD dwDirectoryType);

		/** set developer path. the developer directory is a readonly path that is searched first for all file reading operations. */
		PE_CORE_DECL static void SetDevDirectory(const string& sFilePath);
		PE_CORE_DECL static const string& GetDevDirectory();
	public:
		/** set the disk file priority. it affects whether the disk will be searched first or the one in the archive files.
		* default disk file will be searched first.
		* @param nPriority: 0 is the same priority as the disk file. so 0 or above will cause the disk file to be searched before archive files. below 0, such as -1 will cause the archive files go first.
		*/
		static void SetDiskFilePriority(int nPriority);

		/** set the disk file priority. it affects whether the disk will be searched first or the one in the archive files.
		* default disk file will be searched first.
		* @return: 0 is the same priority as the disk file. so 0 or above will cause the disk file to be searched before archive files. below 0, such as -1 will cause the archive files go first.
		*/
		static int GetDiskFilePriority();

		/**
		* Open a file for immediate reading.If the file name begins with ':', it is treated as a win32 resource.
		* e.g.":IDR_FX_OCEANWATER". loads data from a resource of type "TEXTFILE". See MSDN for more information about Windows resources.
		* Caller calls getBuffer() to retrieval the data
		* @param filename: the file name to open. if it is "<memory>" and bReadOnly is false. it is a memory file. 
		* @param bReadyOnly: if true, the file is loaded for read-only access.
		*	The disk file is searched, then the ZIP files are searched. If false,
		*	only the disk file is searched, and that the file is a write-only file.
		*	NOTE1: Calling any of the write method in read-only mode or calling any read method
		*	in write-only mode will lead to unexpected result.
		*	NOTE2: for opening write-only file, check the file existence first to prevent file overridden
		*	the file will be opened with OPEN_ALWAYS tag by Windows API CreateFile(). The file pointer is
		*	the end of file.
		* @param relativePath: a relative path file name. it can be NULL,
		*	in which case filename is treated as a root path name
		* @param bUseCompressed: default to false. if this is true and the file is stored compressed on disk, it will be loaded as raw compressed file.
		*	getBuffer() will return raw compressed data buffer. One needs to manually call Decompress() at a later time, in order to access the uncompressed data.
		*	this feature is usually used by the content streaming architecture, where the IO thread load compressed data to memory and one of the worker thread decompress it.
		* @return : true if succeeded.
		*/
		PE_CORE_DECL bool OpenFile(const char* filename, bool bReadyOnly = true, const char* relativePath = NULL, bool bUseCompressed = false, uint32 dwWhereToOpen = FILE_ON_DISK | FILE_ON_ZIP_ARCHIVE | FILE_ON_SEARCH_PATH);

		/** mostly used for reading from an archive file handle */
		PE_CORE_DECL bool OpenFile(CArchive* pArchive, const char* filename, bool bUseCompressed = false);

		/** get file attributes like file type, where the file is found, absolute path, modification time, size, etc.
		@param ParaFileInfo: file info.
		*/
		PE_CORE_DECL static bool GetFileInfo(const char* filename, CParaFileInfo& fileInfo, uint32 dwWhereToOpen = FILE_ON_DISK | FILE_ON_ZIP_ARCHIVE | FILE_ON_SEARCH_PATH);

		/**
		* This is rather similar to OpenFile() method, except that it will first look in the AssetManifest to see if the file exit.
		* If the file does appear in manifest list, we will download the latest version from the current asset server, if not done before.
		* the download process is SYNCHRONOUS. If the file does not appear in AssetManifest list, this function is equivalent to OpenFile()
		* @param filename: the asset file key to open. The actual file opened may from the temp/cache/ folder.
		* @param bDownloadIfNotUpToDate: default to true. if true, we will download the latest version from the current asset server. the download process is synchronous.
		*	If false, the function will return 0 immediately, when the caller may consider download the file asynchronously, and then open the file again.
		* @param relativePath: a relative path file name. it can be NULL, in which case filename is treated as a root path name
		* @return: 1 if succeed. 0, if file is not downloaded successfully.
		*/
		PE_CORE_DECL int OpenAssetFile(const char* filename, bool bDownloadIfNotUpToDate = true, const char* relativePath = NULL);

		/** check to see whether we have a up to date version of an asset file. if the asset file does not appear in asset manifest list, it will return the result of DoesFileExist() instead. */
		PE_CORE_DECL static bool DoesAssetFileExist(const char* filename);
		PE_CORE_DECL static bool DoesAssetFileExist2(const char* filename, bool bSearchZipFile = false);

		/** decompress the file if it is currently compressed.
		* This function is usually used by the content streaming architecture, where the IO thread load compressed data to memory
		* and one of the worker thread decompress it.
		*/
		PE_CORE_DECL bool Decompress();

		/** get whether the buffer and size is a zip compressed. */
		PE_CORE_DECL bool IsCompressed();

		/** set whether the buffer and size is a zip compressed. if true, one can use Decompress() function to turn the buffer to uncompressed data. */
		PE_CORE_DECL void SetIsCompressed(bool bCompressed);

		/** create a new file for writing
		* @param bAutoMakeFilePath: if true, the file path will be created, if not exists
		*/
		PE_CORE_DECL bool CreateNewFile(const char* filename, bool bAutoMakeFilePath = true);
		/**
		* no longer release the file buffer when this file object is destroyed.
		*/
		PE_CORE_DECL void GiveupBufferOwnership();
		PE_CORE_DECL void TakeBufferOwnership();

		/** extract this para file to disk
		* @param filenameDest: destination file name.
		* @param bReplace: replace file if the file already exists.
		*/
		PE_CORE_DECL bool ExtractFileToDisk(const string& filenameDest, bool bReplace);
		/**
		* set whether to extract file to disk. This is only for debugging purposes,
		* where one can export all files used during a game session to disk.
		*/
		PE_CORE_DECL static void SetExtractFileProperty(bool bExtractFile);
		/** check whether to extract file.*/
		PE_CORE_DECL static bool GetExtractFileProperty();


		/**
		* upzip the first file in a memory zip file to a given destFileName.
		* this function is mostly used when we downloaded a ziped file from network, and write its unzipped version to disk cache
		*/
		PE_CORE_DECL static bool UnzipMemToFile(const char* buffer, int nSize, const char* destFilename, bool bAutoMakeDirectory);

		///////////////////////////////////////////////////////////////////////////
		//
		// file writing
		//
		//////////////////////////////////////////////////////////////////////////
		/** Write unsigned char stream to file
		* the file must be opened with write access.
		*/
		PE_CORE_DECL int write(const void* src, int bytes);
		/** write string to file.
		* the file must be opened with write access.
		*/
		PE_CORE_DECL int WriteString(const string& sStr);
		/** write string to file.
		* the file must be opened with write access.
		* @param sStr: string which should be terminated with '\0'
		* @param nLen: always gives a string length, unless you are sure that sStr be terminated with '\0'
		*/
		PE_CORE_DECL int WriteString(const char* sStr, int nLen = 0);
		/** the string length can not exceed 1024*/
		PE_CORE_DECL int WriteFormated(const char *, ...);

		inline int WriteDWORD(DWORD data){ return write(&data, sizeof(data)); }
		inline int WriteWORD(WORD data) { return write(&data, sizeof(data)); } 


		/**
		* The SetFilePointer function moves the file pointer of an open file.
		* this function only works when the ParaFile object is an actual windows file, instead of a virtual file.
		* for virtual file, use the seek and seekRelative function.
		* @param lDistanceToMove
		* @param dwMoveMethod
		*	0: FILE_BEGIN The starting point is 0 (zero) or the beginning of the file.
		*	1: FILE_CURRENT The starting point is the current value of the file pointer.
		*	2: FILE_END The starting point is the current end-of-file position.
		*/
		void SetFilePointer(int lDistanceToMove, int dwMoveMethod);

		/**
		* The SetEndOfFile function moves the end-of-file (EOF) position for the specified file to
		* the current position of the file pointer.This function can be used to truncate or extend a file.
		* If the file is extended, the contents of the file between the old EOF position and the new position are not defined.
		* @return
		*/
		bool SetEndOfFile();

		///////////////////////////////////////////////////////////////////////////
		//
		// file reading: unsigned char based
		//
		//////////////////////////////////////////////////////////////////////////

		/** read unsigned char stream from file at its current location. The buffer pointer will be advanced.*/
		PE_CORE_DECL size_t read(void* dest, size_t bytes);

		/** read DWORD */
		inline DWORD ReadDWORD(){ DWORD data; read(&data, 4); return data; }
		/** read WORD */
		inline WORD ReadWORD(){ WORD data; read(&data, 2); return data; }

		/** read WORD */
		inline BYTE ReadByte(){ BYTE data; read(&data, 1); return data; }
		inline int WriteByte(BYTE data){ return write(&data, 1); }

		/** read DWORD: little endian unsigned char order */
		DWORD ReadDWORD_LE();
		/** read WORD: little endian unsigned char order */
		WORD ReadWORD_LE();

		/** read compressed unsigned int (16, 32, 64, etc)
		* @param nByteRead: number of bytes read
		*/
		uint32_t ReadEncodedUInt();
		/** return the number bytes written */
		int WriteEncodedUInt(uint32_t value);

		/**
		* return the next line of text string from the current file position.
		* if a line begins with "--", it is automatically recognized as a comment line and will be skipped.
		* a blank line will also be skipped
		* @param buf output buffer
		* @param sizeBuf: size of the output buffer in bytes.
		*	TODO: if this is 0, this function return the number of bytes required.
		* @return number of bytes read
		*/
		PE_CORE_DECL int GetNextLine(char* buf, int sizeBuf);
		/**
		* advancing file pointer, skipping all comment lines, blank lines, and blank spaces.
		* @return the number of bytes skipped.
		*/
		PE_CORE_DECL int SkipComment();
		/**
		* advancing file pointer, skipping all occurrences of ' ' and '=' mark
		* @return the number of bytes skipped.
		*/
		PE_CORE_DECL int SkipEqualMark();

		/**
		* advancing file pointer, skipping the sName.
		* @param sName the string to skip.
		* @return the number of bytes skipped. if the sName is not matched with the file buffer, the file position will not be changed.
		*/
		PE_CORE_DECL int SkipString(const char* sName);

		/** skip the current line. the file position will be at the beginning of the next line. if the line terminator is \r\n. the file pos is at \n.*/
		PE_CORE_DECL int SkipCurrentLine();

		/**
		* in order to call this method. The file must be a text based file containing  comments "--" and <attribute = value> pair in each line.
		* the supported format is given by example:
		-- a comment line \n
		Name1 = 1024 \n
		Name2 = ParaEngine Tech Studio @ STI \n
		Please note that there must be a line delimiter'\r\n' or '\n' inbetween each attribute,value pair.
		Now that one can call this function
		GetNextAttribute("Name1", nValue), where nValue will be 1024.
		GetNextAttribute("Name1", sValue), where sValue will be "ParaEngine Tech Studio @ STI".
		* @param sName : name of the attribute
		* @param output : output value
		* @return true if the attribute name is found and that the output is successfully parsed, the current file pointer will be right after the
		*   line delimiter; if return value is false, the current file pointer will be set to the beginning of the current line, ready for a second try;
		*	however if the sName is found, yet the value is unable to be parsed. the file pointer will skip the current line.
		* @remark: line length should not exceed 1024
		*/
		PE_CORE_DECL bool GetNextAttribute(const char * sName, string& output);
		/** @see GetNextAttribute(const char * sName, string& output) */
		PE_CORE_DECL bool GetNextAttribute(const char * sName, float& output);
		/** @see GetNextAttribute(const char * sName, string& output) */
		PE_CORE_DECL bool GetNextAttribute(const char * sName, double& output);
		/** @see GetNextAttribute(const char * sName, string& output) */
		PE_CORE_DECL bool GetNextAttribute(const char * sName, int& output);
		/** @see GetNextAttribute(const char * sName, string& output) */
		PE_CORE_DECL bool GetNextAttribute(const char * sName, bool& output);

		/** read a single word from the next line.
		@see GetNextAttribute(const char * sName, string& output) */
		PE_CORE_DECL bool GetNextAttribute(const char * sName);

		/** this is similar to GetNextAttribute().
		* I used to prefer bool GetNextFormatted(const char * sFormat, ...) however, visual C++ 2003 does not have vsnscanf() which accept va_list parameter.
		* @param sFormat: a format as passed to scanf()
		* @return: true if the attribute name is found and that the output is successfully parsed, the current file pointer will be right after the
		*   line delimiter; if return value is false, the current file pointer will be set to the beginning of the current line, ready for a second try.
		* @remark: there must at least be one argument in the argument list. If not use GetNextAttribute(const char * sName) instead.
		*	this function will return true if one or more arguments are parsed. However, it does not ensure all argument in format string are parsed successfully.
		*/
		template <class TYPE_ARG1>
		bool GetNextFormatted(const char * sFormat, TYPE_ARG1 a1)
		{
			bool bSucceed = false;
			SkipComment();
			size_t nOldPos = m_curPos;

			if (isEof())
				return false;
			
			const char* pSrc = getPointer();
#ifdef WIN32
			if (_snscanf(pSrc, m_size - m_curPos, sFormat, a1) >= 1)
#else
			if (sscanf(pSrc, sFormat, a1) >= 1)
#endif
			{
				bSucceed = true;
				SkipCurrentLine();
			}
			else
				m_curPos = nOldPos;

			return bSucceed;
		}
		template <class TYPE_ARG1, class TYPE_ARG2>
		bool GetNextFormatted(const char * sFormat, TYPE_ARG1 a1, TYPE_ARG2 a2)
		{
			bool bSucceed = false;
			SkipComment();
			size_t nOldPos = m_curPos;

			if (isEof())
				return false;

			const char* pSrc = getPointer();
#ifdef WIN32
			if (_snscanf(pSrc, m_size - m_curPos, sFormat, a1, a2) >= 2)
#else
			if (sscanf(pSrc, sFormat, a1, a2) >= 2)
#endif
			{
				bSucceed = true;
				SkipCurrentLine();
			}
			else
				m_curPos = nOldPos;

			return bSucceed;
		}

		template <class TYPE_ARG1, class TYPE_ARG2, class TYPE_ARG3>
		bool GetNextFormatted(const char * sFormat, TYPE_ARG1 a1, TYPE_ARG2 a2, TYPE_ARG3 a3)
		{
			bool bSucceed = false;
			SkipComment();
			size_t nOldPos = m_curPos;

			if (isEof())
				return false;

			const char* pSrc = getPointer();
#ifdef WIN32
			if (_snscanf(pSrc, m_size - m_curPos, sFormat, a1, a2, a3) >= 3)
#else
			if (sscanf(pSrc, sFormat, a1, a2, a3) >= 3)
#endif
			{
				bSucceed = true;
				SkipCurrentLine();
			}
			else
				m_curPos = nOldPos;

			return bSucceed;
		}

		///////////////////////////////////////////////////////////////////////////
		//
		// common file interface functions
		//
		//////////////////////////////////////////////////////////////////////////

		/** get the file name */
		PE_CORE_DECL const string& GetFileName();

		PE_CORE_DECL size_t getSize();
		PE_CORE_DECL size_t getPos();
		PE_CORE_DECL char* getBuffer();
		PE_CORE_DECL inline char* getPointer(){ return m_buffer + m_curPos; }

		PE_CORE_DECL bool isEof();
		PE_CORE_DECL void seek(int offset);
		PE_CORE_DECL void seekRelative(int offset);
		PE_CORE_DECL void close();

		DWORD GetLastModifiedTime() const;
		bool SetLastModifiedTime(DWORD lastWriteTime);
		bool WriteLastModifiedTime(DWORD lastWriteTime);
		bool GetRestoreLastModifiedTimeAfterClose() const;
		void SetRestoreLastModifiedTimeAfterClose(bool shouldRestore);
                
		//////////////////////////////////////////////////////////////////////////
		//
		// bit wise reading
		//
		//////////////////////////////////////////////////////////////////////////
		/**
		* Reset the bit buffer. Since ParaFile is not designed for bit wise reading in general, one need to
		* manually call synchBits() between the calls to a bit and a unsigned char reading functions.
		*/
		PE_CORE_DECL void synchBits();

		/**
		* Read an unsigned value from the given number of bits
		*/
		PE_CORE_DECL unsigned int readUBits(int numBits);
		/**
		* Read a signed value from the given number of bits
		*/
		PE_CORE_DECL int readSBits(int numBits);

	
		/** get handle ptr */
		void* GetHandlePtr();
	private:
		/// file handle
		FileHandle m_handle;
		/// whether end of file is reached. 
		bool m_eof : 1;
		/// whether this file is responsible for releasing the file handle or buffer when closing
		bool m_bIsOwner : 1;
		/// whether a disk file handle is opened
		bool m_bDiskFileOpened : 1;
		/** whether the m_buffer is compressed data. when this is true, m_size is the compressed file size and m_uncompressed_size is the uncompressed file size. */
		bool m_bIsCompressed : 1;
		/** memory file opened */
		bool m_bMemoryFile : 1;
		// Force alignment to next boundary.
		DWORD: 0;
		/// file buffer
		char *m_buffer;
		/// current file position
		size_t m_curPos;
		/// file size
		size_t m_size;
		/// the uncompressed file size, only valid when m_bIsCompressed is true. 
		size_t m_uncompressed_size;

		/// file name
		string m_filename;
		//--Bit buffer..This is only used for bit wise reading
		int m_bitBuf;
		int m_bitPos; // a value in range [0,8]

		DWORD m_lastModifiedTime;
		bool m_restoreLastModifiedTimeAfterClose;
	};
}
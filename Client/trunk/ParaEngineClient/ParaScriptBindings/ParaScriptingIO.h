//-----------------------------------------------------------------------------
// Copyright (C) 2004 - 2006 ParaEngine Dev Studio, All Rights Reserved.
// Date:	2005.11
// Description:	API for IO
//-----------------------------------------------------------------------------
#pragma once
#include "ParaScriptingCommon.h"
#include "FileSystemWatcher.h"
#include "ZipWriter.h"

namespace ParaScripting
{
	using namespace std;
	using namespace ParaEngine;
	using namespace luabind;

	/** file system watcher to monitor file changes in one or several directories
	* .NET also provides a class similar to this one, called FileSystemWatcher.
	*/
	class ParaFileSystemWatcher
	{
	public:
		ParaFileSystemWatcher();
#if defined(PARAENGINE_MOBILE)
        ParaFileSystemWatcher(ParaFileSystemWatcher* watcher);
#else
        ParaFileSystemWatcher(CFileSystemWatcher* watcher);
#endif

		/** add a directory to monitor */
		void AddDirectory(const char* filename);
		/** remove a directory from monitor */
		void RemoveDirectory(const char* filename);

		/** the call back script will be invoked with a global msg
		* msg = {type=[0,5], dirname=string, filename=string}, where
		type can be
		{
		null = 0,
		added = 1,
		removed = 2,
		modified = 3,
		renamed_old_name = 4,
		renamed_new_name = 5
		};
		*/
		void AddCallback(const char* sCallbackScript);
    public:
#if defined(PARAENGINE_MOBILE)
        ParaFileSystemWatcher* m_watcher;
#else
        CFileSystemWatcher* m_watcher;
#endif

	};

	/**
	* @ingroup ParaIO
	* file object. Always call close() when finished with the file.
	*/
	class ParaFileObject
	{
	public:
		boost::shared_ptr<CParaFile> m_pFile;
		int m_nFromByte;
		int m_nByteCount;
		// for the get line or get file API
		std::string m_sTempBuffer;
		ParaFileObject():m_pFile(), m_nFromByte(0), m_nByteCount(-1){};
		ParaFileObject(boost::shared_ptr<CParaFile> pFile):m_pFile(pFile), m_nFromByte(0), m_nByteCount(-1){};
		~ParaFileObject();
	public:
		/** whether file is valid*/
		bool IsValid();
		/** Close the current file.*/
		void close();

		/** by setting the segment, we can inform NPL that we only transmit the file content in the segment to a remote place.
		* it also affects the returned result of the GetBase64String.
		* @param nFromByte: from which byte
		* @param nByteCount: number of bytes of the segment. if -1, it is end of file.
		* @return: return the number of bytes in the segment. it is usually the same as nByteCount, unless nBytesCount exceeds the file length.
		*/
		int SetSegment(int nFromByte, int nByteCount);

		/** get base 64 string of the binary data in the current file segment.
		* to change segment, use SetSegment.
		* @param pnStrLength: [out] length of the returned string.
		* @return pOutString: a static global buffer containing the string. it ends with '\0'. it is NULL, if file is invalid.
		*/
		const char* GetBase64StringEx(int * pnStrLength=0);
		const char* GetBase64String();

		/**
		* always call seek(0) if one opens a old file for overwritten
		*/
		void seek(int offset);
		void seekRelative(int offset);
		/** get current reader or writer cursor position offset in bytes*/
		int getpos();

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
		void SetFilePointer(int lDistanceToMove,int dwMoveMethod);

		/**
		* The SetEndOfFile function moves the end-of-file (EOF) position for the specified file to
		* the current position of the file pointer.This function can be used to truncate or extend a file.
		* If the file is extended, the contents of the file between the old EOF position and the new position are not defined.
		* @return
		*/
		bool SetEndOfFile();

		void writeline(const char* str);
		/** read line as a string. The string is guaranteed to be ended with '\0'.
		* if end of file is reached, it will return NULL. which is nil in the script.
		* if a line begins with "--", it is automatically recognized as a comment line and will be skipped.
		* a blank line will also be skipped.
		*/
		const char* readline();

		/** get the content of the file as text. Text encoding is escaped. If you want to get the raw file text with the heading BOM, such as utf8 (EE BB BF), use GetText2(0,-1)*/
		const char* GetText();

		/** get the content of the file as text between two positions.
		* @param fromPos: position in bytes.
		* @param nCount: nCount in bytes. if -1, it defaults to end of file.
		*/
		const std::string& GetText2(int fromPos, int nCount);

		/** read a binary string of length nCount from current position. 
		* @param nCount: nCount in bytes. if -1, it defaults to end of file.
		*/
		object ReadString(int nCount, lua_State* L);

		/** write a string to the current file. */
		void WriteString(const char* str);
		/** write a buffer to the current file. */
		void WriteString2(const char* buffer, int nSize);

		/** write a buffer to the current file. */
		void write(const char* buffer, int nSize);

		/**
		* write bytes to file; e.g. local nBytes = file:WriteBytes(3, {[1]=255, [2]=0, [3]=128});
		* @param nSize: number of bytes to write
		* @param input: array of integers, each represents a byte
		* @return: number of bytes written
		*/
		int WriteBytes(int nSize, const object& input);

		/**
		* read bytes from file. e.g. local data={};local nBytes = file:ReadBytes(3, data); data[1], data[2], data[3]
		* @param nSize: number of bytes to read. if negative, it will be total file size
		* @param output: in/out, it should be an empty table. When the function returns, it will contain an array of integers, each represents a byte
		* if output is nil or "", the returned value will also be a string(may contain \0) of the content read
		* @return string or a table array
		*/
		object ReadBytes(int nSize, const object& output);

		/** float is 32 bits*/
		void WriteFloat(float value);
		float ReadFloat();

		/** integer is converted 16 bits unsigned word*/
		void WriteWord(int value);
		int ReadWord();

		void WriteDouble(double value);
		double ReadDouble();

		/** integer is 32 bits*/
		void WriteInt(int value);
		int ReadInt();

		/** integer is 16 bits signed int*/
		void WriteShort(int value);
		int ReadShort();

		///
		void WriteUShort(unsigned short value);
		unsigned short ReadUShort();

		///
		void WriteByte(unsigned char value);
		unsigned char ReadByte();

		void WriteChar(char value);
		char  ReadChar();

		/** integer is 32 bits unsigned*/
		void WriteUInt(unsigned int value);
		unsigned int ReadUInt();

		/** get the file size in bytes. */
		int GetFileSize();
	};

	/**
	* @ingroup ParaIO
	* it represents a search result.
	*/
	class PE_CORE_DECL ParaSearchResult
	{
	public:
		CSearchResult * m_pResult;

		ParaSearchResult():m_pResult(NULL){};
		ParaSearchResult(CSearchResult * pResult):m_pResult(pResult){};

		/**
		* check if the object is valid
		*/
		bool IsValid();

		/** release results. */
		void Release();

		/** get the root path */
		string GetRootPath();

		/** get the total number of result found */
		int GetNumOfResult();

		/** add a new item. return false if the search is completed. */
		bool AddResult(const char* sItem);

		/** get the item at the specified index. return "" if nIndex is out of range.*/
		string GetItem(int nIndex);

		/** get the item at the specified index. return nil if nIndex is out of range.
		@param output: [in|out] default value.
		@return: return the field result. If field not found, output will be returned.
			e.g. {filename="abc.txt", filesize=0, fileattr=0, createdate="1982-11-26", writedate="", accessdate=""}
			fileattr is the value same as defined in WIN32_FIND_DATA.dwFileAttributes
		*/
		object GetItemData(int nIndex, const object& output);
	};

	/**
	* @ingroup ParaIO
	* ParaZipWriter class:
	* creating zip files
	*
	* e.g.
	*  (1) Traditional use, creating a zipfile from existing files
	* local writer = ParaIO.CreateZip("c:\\simple1.zip","");
	* writer:ZipAdd("znsimple.bmp", "c:\\simple.bmp");
	* writer:ZipAdd("znsimple.txt", "c:\\simple.txt");
	* writer:close();
	*/
	class ParaZipWriter
	{
	public:
		ParaZipWriter();
		ParaZipWriter(CZipWriter* writer);
	public:
		ref_ptr<CZipWriter> m_writer;

	public:

		/** whether it is valid*/
		bool IsValid();

		/**
		* add a zip file to the zip. file call this for each file to be added to the zip.
		* @return: 0 if succeed.
		*/
		DWORD ZipAdd(const char* dstzn, const char* fn);

		/**
		* add a data to the zip.
		* @return: 0 if succeed.
		*/
		DWORD ZipAddData(const char* dstzn, const std::string& buff);

		/**
		* add a zip folder to the zip file. call this for each folder to be added to the zip.
		* @return: 0 if succeed.
		*/
		DWORD ZipAddFolder(const char* dstzn);

		/**
		* add everything in side a directory to the zip.
		* e.g. AddDirectory("myworld/", "worlds/myworld/ *.*", 10);
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
	};

	/**
	* @ingroup ParaIO
	* ParaIO class:
	* IO functions ported to the scripting system
	*/
	class PE_CORE_DECL ParaIO
	{
	public:
		ParaIO(void);
		~ParaIO(void);
	public:

		/**
		* replace variables in input path and return the result path. see AddPathVariable
		* @param input: a path with or without replaceable. make sure you have called ToCanonicalFilePath() to canonicalize the input before calling this function
		* @return the resulting path. Please note that the return value is the input itself if nothing is replaced.
		* otherwise, it is a static string reference containing the result. therefore the result is NOT thread safe.
		*/
		static const char* DecodePath(const char* input);

		/**
		* this does reverse of DecodePath. see AddPathVariable
		* @param input: a path with or without replaceable. make sure you have called ToCanonicalFilePath() to canonicalize the input before calling this function
		*/
		static const char* EncodePath(const char* input);

		/** same as EncodePath, except that it will only replace variables who name appears in varNames. varNames is a comma separated list of variable names.
		* @param varNames: a comma separated list of variable names. like "WORLD,USERID", etc.
		*/
		static const char* EncodePath2(const char* input, const char* varNames);

		/** add a new variable to the replaceable pool
		* @param sVarName: the variable name without enclosing %%, such as "WORLD", "USERID", usually uppercase.
		* @param sVarValue: the path that the variable expands to. If nil, it will remove the variable.
		* @return: true if succeed. it may return false, if a protected variable with the same name already exist but it not editable via scripting interface.
		*/
		static bool AddPathVariable(const char * sVarName, const char * sVarValue);

		/**
		* add a search path to the search path pool. It will internally normalize the path and check for duplicates
		* @security note: this function shall not be called by an untrusted client, since it will secretly swap files.
		* @TODO: shall we support remote http zip file as a valid search path?
		*/
		static bool AddSearchPath(const char* sFile);
		static bool AddSearchPath2(const char* sFile, int nPriority);

		/**
		* remove a search path from the search path pool.
		*/
		static bool RemoveSearchPath(const char* sFile);

		/** get writable path */
		static const std::string& GetWritablePath();

		/**
		* clear all search paths.
		*/
		static bool ClearAllSearchPath();

		/**
		* Get the CRC 32 code of a given file.
		* @return: return 0 if not succeed, otherwise the CRC32 code is returned.
		*/
		static unsigned long CRC32(const char* filename);

		/**
		* this function is equivalent to calling the following functions.
		* LoadLogFromFile("InstallFiles.txt");
		* LoadLogFromFile("temp/filelog.txt");
		* MirrorFiles(dirName, bOverwrite);
		*
		* e.g. UpdateMirrorFiles("_InstallFiles/", true);
		* @param dirName: such as "_InstallFiles/"
		* @param bOverwrite: if this is true, existing files will be overridden.
		*/
		static void UpdateMirrorFiles(const char* dirName, bool bOverwrite);


		/**
		* call this to start the creation of a zip file.
		*/
		static ParaZipWriter CreateZip(const char *fn, const char *password);

		/** delete a given file. It will reject any system files outside the application directory.
		* after all, this function is of high security level.
		* @param sFilePattern: such as "*.dds", "temp.txt", etc
		* @return: the number of files deleted.
		*/
		static int DeleteFile(const char* sFilePattern);

		/**
		* The MoveFile function will move (rename) either a file or a directory (including its children) either in the same directory or across directories.
		* @param src specifies the name of an existing file
		* @param dest specifies the name of the new file
		* @return true if succeeds
		*/
		static bool MoveFile(const char* src, const char* dest);

		/**
		* The CopyFile function copies an existing file to a new file
		* @param src specifies the name of an existing file
		* @param dest specifies the name of the new file
		* @param bOverride [in] If this parameter is false and the new file specified by src already exists, the function fails.
		* If this parameter is true and the new file already exists, the function overwrites the existing file and succeeds.
		* @return true if succeeds
		*/
		static bool CopyFile(const char* src, const char* dest, bool bOverride);

		/**
		* This is rather similar to OpenFile() method, except that it will first look in the AssetManifest to see if the file exit.
		* If the file does appear in manifest list, we will download the latest version from the current asset server, if not done before.
		* the download process is SYNCHRONOUS. If the file does not appear in AssetManifest list, this function is equivalent to OpenFile()
		* @param filename: the asset file key to open. The actual file opened may from the temp/cache/ folder.
		* @param bDownloadIfNotUpToDate: default to true. if true, we will download the latest version from the current asset server. the download process is synchronous.
		*	If false, the function will return 0 immediately, when the caller may consider download the file asynchronously, and then open the file again.
		* @return: 1 if succeed. 0, if file is not downloaded successfully.
		*/
		static ParaFileObject OpenAssetFile(const char* filename);
		static ParaFileObject OpenAssetFile2(const char* filename, bool bDownloadIfNotUpToDate);

		/** check to see whether we have a up to date version of an asset file. if the asset file does not appear in asset manifest list, it will return the result of DoesFileExist() instead. */
		static bool DoesAssetFileExist(const char* filename);

		/** same as DoesAssetFileExist, except that if bSearchZipFile == false, it is equivalent to DoesFileExist().
		*/
		static bool DoesAssetFileExist2(const char* filename, bool bSearchZipFile);

		/** similar to SyncFile(), except that this function will return immediately and does not redownload or call AddDownloadCount. And use callback.
		* @param sCallBackScript: the callback script code to be executed when the download is complete. it must begin with ";", such as ";log([[download is complete]]);"
		*  the global "msg.res" table contains the error code in case an error is met. msg.res == 0 if succeed, otherwise -1.
		* @return: 0 if download has begun, 1 if file is already downloaded, -1 if failed, -2 if input is not an asset file.
		*/
		static int SyncAssetFile_Async(const char* filename, const char* sCallBackScript);

		/** check to see whether an asset file is already downloaded to local disk.
		* generally return value is larger than 1 if succeed.
		* @return :
		* 1 if already downloaded;
		* 0 if asset has not been downloaded;
		* -1 if we are unable to download the asset file after trying serveral times;
		* -3 if asset is being downloaded but is not completed;
		* -4 if input file is not an asset file.
		*/
		static int CheckAssetFile(const char* filename);

		/* load a replace file mapping. it is just file to file pairs. If the file is requested on the left during GetFile(filename), file on the right is returned instead.
		* This function is very useful to temporarily change the 3D and 2D theme of the entire game world, in which only a text file needs to be updated.
		* The following API are also aware of replace file: DoesFileExist(), OpenFile(), OpenAssetFile(), etc.
		* @param filename: the file map file.
		* @param bReplaceExistingOnes: whether we will overwrite any previous calls to this function.
		*/
		static void LoadReplaceFile(const char* filename, bool bReplaceExistingOnes);

		/**
		* Open or create a file
		* e.g. ParaIO.open("temp/test.txt", "w");
		* e.g. ParaIO.open("", "buffer");
		* e.g. ParaIO.open("<memory>", "w");
		* @param filename: the file name to open. if it is "<memory>" and mode is "w". it is a memory buffer. 
		* @param mode : access mode
		*  - "r" Opens for reading. If the file does not exist or cannot be found, the call fails.
		*  - "w" Opens an empty file for writing. If the given file exists, its contents are destroyed.If not, file will be created.
		*  - "a" append to the end of an existing file. if file does not exist, a new one is created.
		*  - "image" load image file 
		*  - "buffer" filename/obj is a string(byte£©buffer
		* @return file object is returned.
		*/
		static ParaFileObject open(const object& obj, const char *mode);

		/**
		* open an image file. The r,g,b can then be retrieved as bytes arrays using ReadBytes() function.
		* @param filename: such as BMP, DDS, JPG, etc. It must be a square image. The size of the image can thus be calculated by file size.
		* @param mode : access mode
		*  - "r8g8b8": each pixel is a three bytes of R,G,B
		*  - "a8r8g8b8": each pixel is a four bytes of A,R,G,B
		*  - "float32": each pixel is a four bytes of float. [Not supported yet]
		*/
		static ParaFileObject openimage(const object& filename, const char *mode);
		/* like openimage, but can get exinfo*/
		/*
			local exinfo = {};
			local file = ParaIO.openimage("test.jpg", "", exinfo);
			if (file) then
				print(exinfo.FocalLength);
			end
		*/
		static ParaFileObject openimage2(const char * filename, const char* mode, const object& oExInfo);

		/** make directory
		* @param filename: director path. file portion will be automatically striped off. So it is ok to pass in file name, instead of directory name.
		* @return: true if the directory is made or already exists*/
		static bool CreateDirectory(const char* filename);

		/** deprecated: use ParaIO.open
		* create a new file for writing.
		* it will make all necessary directories in order to create the file.
		*/
		static bool CreateNewFile(const char * filename);
		/** deprecated: use ParaIO.open
		* open a new file for write-only access. If the file does not exist, it will be created.
		* if the file exists, the file pointer is at the end of file.*/
		static bool OpenFileWrite(const char * filename);
		/** deprecated: use ParaIO.open
		* Open a file for read-only access.
		*/
		static bool OpenFile(const char * filename);

		/** deprecated: use ParaIO.open
		* Close the current file.
		*/
		static void CloseFile();
		/** deprecated: use ParaIO.open
		* write a string to the current file. 
		*/
		static void WriteString(const char* str);
		/**
		* deprecated: use ParaIO.open
		* write a buffer to the current file.
		*/
		static void write(const char* buffer, int nSize);

		/** if no file is opened, it means readline from stdin.
		* a deprecated usage is reading from current open file. 
		* read line as a string. The string is guaranteed to be ended with '\0'.
		* if end of file is reached, it will return NULL. which is nil in the script.
		* if a line begins with "--", it is automatically recognized as a comment line and will be skipped.
		* a blank line will also be skipped.
		*/
		static const char* readline();
		
		/** read line from stdin and automatically add to history
		* @param prompt: the default prompt like "> ", ">> " or just ""
		*/
		static const char* readline2(const char* prompt);

		/**
		* Check whether a given file exists on disk.
		* @param filename: file name to check
		* @param bSearchZipFiles: if false, not disk file is searched. If true, both the disk file and zip file will be searched.
		*	currently bSearchZipFiles can only be false.
		*	Because it is not efficient to check existence of ZIPPED files. Use OpenFile() and check for return value instead.
		*/
		static bool DoesFileExist(const char* filename, bool bSearchZipFiles);
		/** see DoesFileExist(). This version is same as DoesFileExist(filename, false); */
		static bool DoesFileExist_(const char* filename);

		/** backup a specified file, if the file exists. A new file with an extension ".bak" appended
		* to the end of the original file will be created, whose content is identical to the original file.
		* @param filename: file name to back up
		* @return: return true if the file is backed up. return false, if the file does not exist or some error occurs during backup.
		*/
		static bool BackupFile(const char* filename);

		/** search files at once. @see CSearchResult
		* the current version of this function can support only one query at a time. The search result is invalid if called intermitantly
		* @param sRootPath: the root path. for example: "", "xmodel/","xmodel/models/". Other format is not acceptable
		* @param sFilePattern: file pattern, e.g. "*.x" (all files with x extension), "*" (any files), "*."(directories only)
		*  if sZipArchive is non-empty, sFilePattern support both regular expression and wild card search.
		*  it performs wild cards search by default, where "/\\" matches to directory. "* "matches to anything except "/\\.", and "." matches to "." itself.
		*		e.g.. "*.*", "*.", "worlds/ *.abc", "*abc/ *.jpg", etc
		*  it sFilePattern begins with ":", things after ":" is treated like a regular expression. It has the same syntax with the perl regular expression and uses full match.
		*		e.g.. ":.*\\.jpg", etc.
		* @param sZipArchive: it can be "" or a zip archive file name. . if it is not, only that archive files are saved.
		*	- "": only disk files are searched
		*	- "*.zip": currently opened zip files are searched
		*	- "*.*": search disk file followed by all zip files.
		* @param nSubLevel: how many sub folders of sRootPath to look into. default value is 0, which only searches the sRootPath folder.
		* @param nMaxFilesNum: one can limit the total number of files in the search result. Default value is 50. the search will stop at this value even there are more matching files.
		* @param nFrom: only contains results from nFrom to (nFrom+nMaxFilesNum)
		* @return: one should manually release the search result.
		*/
		static ParaSearchResult SearchFiles(const char* sRootPath, const char* sFilePattern, const char* sZipArchive, int nSubLevel, int nMaxFilesNum, int nFrom);
		static ParaSearchResult SearchFiles_c(const char* sRootPath, const char* sFilePattern, const char* sZipArchive, int nSubLevel);

		/** get the current directory of the application. it allows querying a number of standard directories.
		* please note that all directory are returned as absolute path with slash "/" between two level of directories.
		* and that it always ends with "\". e.g. "c:/lxzsrc/paraengineSDK/" or "c:/lxzsrc/paraengineSDK/script/"
		* @param dwDirectoryType: it can be one of the PARAENGINE_DIRECTORY enumeration type
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
			APP_LAST_DIR,
			};
		* @return: the directory is returned. */
		static string GetCurDirectory(DWORD dwDirectoryType);
		/** this should never be called from the scripting interface. it is only for API exportation.
		it uses a static string for the output. so it is not thread-safe.*/
		static const char* GetCurDirectory_(DWORD dwDirectoryType);

		/**
		* trim the sFile by nParentCounts number of parent directories.
		@param nParentCounts: number of parent directory to remove
		@return: return "" if the input directory does not have that many parent directories.
		e.g.
		"C:/abc/" = GetDirectoryFromPath("C:/abc/aaa",0);
		"C:/" = GetDirectoryFromPath("C:/abc/",1);
		*/
		static string GetParentDirectoryFromPath(const char* sfilename, int nParentCounts);
		/** this should never be called from the scripting interface. it is only for API exportation.
		it uses a static string for the output. so it is not thread-safe.*/
		static const char* GetParentDirectoryFromPath_(const char* sfilename, int nParentCounts);

		/** This will find the root path from a given directory path using the following rule:
		* find a file called "ParaEngine.sig" in the parent directories of sFile, from near to far.
		* e.g. if sFile is "c:/a/b/c/xxx.x", then it will search for "c:/a/b/c/","c:/a/b/","c:/a/" and "c:/".
		* the function will return the first parent directory that contains the file, otherwise "" is returned.
		*/
		static string AutoFindParaEngineRootPath(const char* sFile);
		/** this should never be called from the scripting interface. it is only for API exportation.
		it uses a static string for the output. so it is not thread-safe.*/
		static const char* AutoFindParaEngineRootPath_(const char* sFile);
		/** change the file extension.
		* @param sFile: the file whose extension to change.
		* @param sExt: the file extension to change to. such as "dds","x"
		* @return: return the file with the changed extension. the input file does not contain
		* a valid file extension, the returned string will be identical to the input file.
		*/
		static string ChangeFileExtension(const char* sFile, const string & sExt);
		/** this should never be called from the scripting interface. it is only for API exportation.
		it uses a static string for the output. so it is not thread-safe.*/
		static const char* ChangeFileExtension_(const char* sFile, const string & sExt);

		/** get the file extension. this function may return "" if no file extension is found */
		static string GetFileExtension(const char* sFile);
		/** this should never be called from the scripting interface. it is only for API exportation.
		it uses a static string for the output. so it is not thread-safe.*/
		static const char* GetFileExtension_(const char* sFile);

		/** Get the relative file path by stripping the root path from the beginning.
		* please note that all paths should uses slash "/", instead of backslash "\", in the path name.letter case is ignored
		* @param sAbsolutePath: the absolute path from which to obtain the relative path.
		* @param sRootPath: the parent root path, which will be removed from the absolute path. It should end with "/"
		* @return: the relative path is returned. If the absolute path does not math the root path, the absolute path
		*		is returned unchanged. the relative path does not begin with "/"
		* e.g. "a/b.x" = GetRelativePath("c:/lxzsrc/a/b.x", "c:/lxzsrc/");
		*      "c:/lxzsrc/a/b.x" = GetRelativePath("c:/lxzsrc/a/b.x", "c:/srclxz/"); // not match
		*/
		static string GetRelativePath(const char* sAbsolutePath, const char* sRootPath);
		/** this should never be called from the scripting interface. it is only for API exportation.
		it uses a static string for the output. so it is not thread-safe.*/
		static const char* GetRelativePath_(const char* sAbsolutePath, const char* sRootPath);

		/** Get the absolute file path by appending the root path before the relative path.
		* please note that all paths should uses slash "/", instead of backslash "\", in the path name. letter case is ignored
		* @param sRelativePath: the absolute path from which to obtain the relative path. It should not begin with "/"
		* @param sRootPath: the parent root path, which will be removed from the absolute path. It should end with "/"
		* @return: the relative path is returned. If the absolute path does not math the root path, the absolute path
		*		is returned unchanged.
		* e.g. "c:/lxzsrc/a/b.x" = GetAbsolutePath("a/b.x", "c:/lxzsrc/");
		*/
		static string GetAbsolutePath(const char* sRelativePath, const char* sRootPath);
		/** this should never be called from the scripting interface. it is only for API exportation.
		it uses a static string for the output. so it is not thread-safe.*/
		static const char* GetAbsolutePath_(const char* sRelativePath, const char* sRootPath);

		/** get only the file name from the file path.
		* "a.x" = GetFileName("c:/lxzsrc/a.x");
		*/
		static string GetFileName(const char* sFilePath);
		/** this should never be called from the scripting interface. it is only for API exportation.
		it uses a static string for the output. so it is not thread-safe.*/
		static const char* GetFileName_(const char* sFilePath);

		/** get original file name (in case of name being changed somehow, e.g. CZipArchive may change filename to lower case when case-insensitive).
		* "ABcdEF.x" = GetFileOriginalName("c:/lxzsrc/abcdef.x");
		*/
		static string GetFileOriginalName(const char* sFilePath);

		/** The GetFileSize function retrieves the size of the specified file. The file size that can be reported by this function is limited to a DWORD value
		* @return: size of the file. If the file does not exist or the file size is 0, the return value is 0.
		* @note: only disk file is searched.files inside zip file are ignored.
		*/
		static int GetFileSize(const char* sFilePath);

		/**
		* get file info
		* @param inout: {modification, attr, access, create, size, mode="file|directoy|fileinzip|", fullpath=string}
		*/
		static bool GetFileInfo(const char* sFilePath, const object& inout);

		/** convert a file name to canonical file path
		* @param sfilename: it is assumed that strlen(filename) <= MAX_PATH
		* @param bBackSlash: if true, the file will use '\\'; otherwise use '/'. '\\' is win32 compatible. '/' is more user friendly.
		* @return: the canonical file path name returned. */
		static string ToCanonicalFilePath(const char* sfilename, bool bBackSlash);
		/** this should never be called from the scripting interface. it is only for API exportation.
		it uses a static string for the output. so it is not thread-safe.*/
		static const char* ToCanonicalFilePath__(const char* sfilename, bool bBackSlash);


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

		/** create and get a file system watcher object.
		always use local to retrieve the object to ensure that the object is properly released when out of scope. */
		static ParaFileSystemWatcher GetFileSystemWatcher(const char* filename);

		/** delete a watcher, it will no longer receive callbacks.
		* @please note that if someone else still keeps a pointer to the directory watcher, it will not be deleted.
		*/
		static void DeleteFileSystemWatcher(const char* name);

		/** 
		* if DEFAULT_FILE_ENCODING is defined£¬ParaIO.GetCurDirectory() return a utf8 string£¬mostly£¬it works well with NplRuntime£¬
		* but sometimes we need to Need to pass parameters to the external environment,such as write the path to a ".bat" file£¬it must be ansci.
		*/
		static std::string ConvertPathFromUTF8ToAnsci(const char* path);
	};

	/**
	* @ingroup ParaIO
	*
	* ParaXML class
	*/
	class ParaXML
	{
	private:
		static void LuaXML_ParseNode(lua_State *L, void* pNode);
	public:
		/** only used for documentation generation.
		* @param filename: string
		* @return return a table containing the lua table of xml
			Specifications:
			A tree is a Lua table representation of an element and its contents. The table must have a name key, giving the element name.
			The tree may have a attr key, which gives a table of all of the attributes of the element. Only string keys are relevant.
			If the element is not empty, each child node is contained in tree[1], tree[2], etc. Child nodes may be either strings, denoting character data content, or other trees.

			Spec by example
				lz = ParaXML.LuaXML_ParseString[[<paragraph justify='centered'>first child<b>bold</b>second child</paragraph>]]
				lz = {name="paragraph", attr={justify="centered"},
				[1] = "first child",
				[2] = {name="b", "bold", n=1}
				[3] = "second child",
				n=3
				}
		*/
		static int LuaXML_ParseString (lua_State *L);
		static int LuaXML_ParseFile (lua_State *L);

		/**	The world does not agree on whether white space should be kept or
		not. In order to make everyone happy, these global, static functions
		are provided to set whether or not the parser will condense all white space
		into a single space or not. The default is to condense. Note changing this
		value is not thread safe.
		*/
		static void SetCondenseWhiteSpace( bool condense );

		/// Return the current white space setting.
		static bool IsWhiteSpaceCondensed();
	};
}

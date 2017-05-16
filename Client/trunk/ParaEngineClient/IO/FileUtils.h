#pragma once
#include "FileData.h"

namespace ParaEngine
{
	class CSearchResult;

	/** file information. */
	struct CParaFileInfo
	{
		enum FileMode{
			ModeNoneExist,
			ModeFile,
			ModeFileInZip,
			ModeDirectory,
			ModeLink,
			ModeSocket,
			ModeNamedPipe,
			ModeCharDevice,
			ModeBlockDevice,
			ModeOther,
		};
	public:
		CParaFileInfo();
		
		std::string m_sFullpath;
		FileMode m_mode;
		DWORD m_dwFileAttributes;
		time_t m_ftCreationTime;
		time_t m_ftLastAccessTime;
		time_t m_ftLastWriteTime;
		DWORD m_dwFileSize;
	};

	/** file platform related API and helper functions */
	class CFileUtils
	{
	public:
		/* remove the starting and trailing spaces ' ' from the string */
		static void TrimString(std::string & str);

		static bool FileExistRaw(const char* filename);

		/** this is low level, use with care. Once added, can not be removed. */
		static bool AddDiskSearchPath(const std::string& sFile, bool nFront = false);

		/** check if file exist, this function is cross platform. */
		static bool FileExist(const char* filename);

		/** make a standard filename.
		* @param output: the output string buffer. such as: char output[MAX_PATH]
		* @param filename: the relative file name. if NULL, the parent directory is in output.
		* @param relativePath: the directory name. the last '/' or '\\' is used as parent directory. e.g. "temp/abc.txt" is same as "temp/". if NULL, the filename is copied to output.
		*/
		static void MakeFileNameFromRelativePath(char * output, const char* filename, const char* relativePath);

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
		* trim the sFile by nParentCounts number of parent directories.
		@param nParentCounts: number of parent directory to remove. 0 means the current directory,which is also the default value
		@return: return "" if the input directory does not have that many parent directories.
		e.g.
		"C:/abc/" = GetDirectoryFromPath("C:/abc/aaa",0);
		"C:/" = GetDirectoryFromPath("C:/abc/",1);
		*/
		static std::string GetParentDirectoryFromPath(const std::string& sfilename, int nParentCounts = 0);

		/** make directory
		* @param filename: file name should be a Canonical File Path. Use ,ToCanonicalFilePath
		* @return: true if the directory is made or already exists*/
		static bool MakeDirectoryFromFilePath(const char * filename);

		/** extract this para file to disk
		* @param filenameDest: destination file name.
		* @param bReplace: replace file if the file already exists.
		*/
		static bool SaveBufferToFile(const std::string& filename, bool bReplace, char* buffer, DWORD nBufSize);

		/** delete a given file*/
		static bool DeleteFile(const char* filename);

		/** delete a given file. It will reject any system files outside the application(or writable directory) directory.
		* after all, this function is of high security level.
		* @param sFilePattern: such as "*.dds", "temp/ *.*", etc
		* @param bSecureFolderOnly: if true, we only allow user to delete allowed folders.
		* @return: the number of files deleted.
		*/
		static int DeleteFiles(const std::string& sFilePattern, bool bSecureFolderOnly = true, int nSubFolderCount=10);

		/** if already absolute path, return it, if not we will prepend current writable path. */
		static std::string GetWritableFullPathForFilename(const std::string& filename);
		/** get file name */
		static std::string GetFileName(const std::string& sFilePath);

		/**
		*  Creates binary data from a file.
		*  @return A data object.
		*/
		static FileData GetDataFromFile(const char* filename);

		/** get file size of a disk file in bytes. */
		static bool GetFileInfo(const char* filename, CParaFileInfo& fileInfo);

		/** data need not be released, since it is from the resource file. 
		* only supported in win32
		* @return Note one must call FileData.ReleaseOwnership(). 
		*/
		static FileData GetResDataFromFile(const std::string& filename);

		/** get fullPath for filename*/
		static std::string GetFullPathForFilename(const std::string &filename);

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

		/** get file size of a disk file in bytes. */
		static int GetFileSize(const char* sFilePath);

		/** open file and return the file handle. default input is create a file for shared writing. */
		static FileHandle OpenFile(const char* sFilePath, bool bRead = false, bool bWrite=true);
		
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
		static bool SetFilePointer(FileHandle& fileHandle, int lDistanceToMove, int dwMoveMethod);

		/** get the current file pointer position.*/
		static int GetFilePosition(FileHandle& fileHandle);

		/**
		* The SetEndOfFile function moves the end-of-file (EOF) position for the specified file to
		* the current position of the file pointer.This function can be used to truncate or extend a file.
		* If the file is extended, the contents of the file between the old EOF position and the new position are not defined.
		* @return
		*/
		static bool SetEndOfFile(FileHandle& fileHandle);
		
		/** Write byte stream to file
		* the file must be opened with write access.
		*/
		static int WriteBytes(FileHandle& fileHandle, const void* src, int bytes);

		/** read byte stream from file
		* the file must be opened with read access.
		*/
		static int ReadBytes(FileHandle& fileHandle, void* dest, int bytes);

		/** close the given file*/
		static void CloseFile(FileHandle& fileHandle);

		/** get current directory */
		static std::string GetInitialDirectory();

		/** in win32, this is the root directory, 
		*  Gets the writable path.
		*  @return  The path that can be write/read a file in
		*/
		static std::string GetWritablePath();

		static void SetWritablePath(const std::string& writable_path);

		/** this is a recursive function. @see SearchFiles */
		static void FindDiskFiles(CSearchResult& result, const std::string& sRootPath, const std::string& sFilePattern, int nSubLevel);
	public:
		// this is usually /mnt/sdcard/XXX/ in android. 
		static std::string s_writepath;
	};

	
}

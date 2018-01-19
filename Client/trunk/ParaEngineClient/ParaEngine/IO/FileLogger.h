#pragma once

#include <string>
#include <set>
namespace ParaEngine
{
	using namespace std;
	/**
	* It is a singleton class that is tightly integrated to the game engine. 
	* The default behavior of CFileLogger is this. Once activated, it first checks for ./InstallFiles.txt, 
	* and load it if it exists; then it checks for log file at temp/filelog.txt and load it if it exists. 
	* Then it hooks the CParaFile interface and record each read file operation. Once deactivated, 
	* it wrote the file log to temp/filelog.txt. Usually, at debug build, we will activate the File logger 
	* when application starts, and deactivate it when application exits. Basically file logger is an ideal 
	* way to check for read-only files, which can be zipped into a package at product release time. 
	* @see #define FILE_LOGGER_HOOK in CFileManager.cpp
	*/
	class CFileLogger
	{
	public:
		CFileLogger(void);
		~CFileLogger(void);
		/** get the singleton instance. */
		static CFileLogger* GetInstance();

	public:
		/**
		* Add new entries from file. One can load multiple files, so that can append new 
		* identical files to previously recorded sessions

		The logger file is just a list of file relative paths printed on each separate line. 

		File Logger File format
		=======================
		-- any comments goes here \r\n
		filename \r\n
		filename \r\n
		filename \r\n

		* @param filename 
		* @return 
		*/
		bool LoadLogFromFile(const char* filename);

		/**
		* save all logs to file.
		*/
		bool SaveLogToFile(const char* filename);

		/** it will hook the CParaFile interface and record each read file operation.*/
		bool BeginFileLog();
		/** it will unhook the CParaFile interface and stop recording each read file operation.*/
		void EndFileLog();

		/**
		* add a new file entry.
		* @param filename 
		* @param bReadOnly whether this is a read-only file. 
		*/
		void AddFile(const char* filename,bool bReadOnly = true);
		
		/**
		* copy all files to the given directory preserving relative path info. If file is not a disk file, zip file is searched. 
		* @param dirName: such as "_InstallFiles/"
		* @param bOverwrite: if this is true, existing files will be overridden.
		*/
		void MirrorFiles(const char* dirName, bool bOverwrite = false);

		/**
		* this function is equivalent to calling the following functions. 
		* LoadLogFromFile("InstallFiles.txt");
		* LoadLogFromFile("temp/filelog.txt");
		* MirrorFiles(dirName, bOverwrite);
		* @param dirName: such as "_InstallFiles/"
		* @param bOverwrite: if this is true, existing files will be overridden.
		*/
		void UpdateMirrorFiles(const char* dirName, bool bOverwrite = true);

		/**
		* exclude certain file extension, writable files, etc.
		* @param sPattern 
		*/
		void AddExcludeFilter(const char* sPattern);

		/** clear this instance for reuse. It will remove all file entries. */
		void Reset();

		/** whether we are inside a log session. */
		bool IsBegin();

		/** whether there is already a file with the given file name in the current file entry*/
		bool HasFile(const string& filename);
	private:
		/** file entries */
		set<string> m_entries;
		bool m_bIsBegin;
	};
}

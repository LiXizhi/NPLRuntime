#pragma once
#include "IO/IFile.h"
#include "IO/FileData.h"
#include <string>
namespace ParaEngine
{
	class CSearchResult;

	class CParaFileUtils
	{
	public:
		static CParaFileUtils* GetInstance();
		CParaFileUtils() = default;
		~CParaFileUtils() = default;
	public:
		virtual	ParaEngine::FileData GetDataFromFile(const std::string& filename);
		virtual bool IsAbsolutePath(const std::string& filename);
		virtual std::string GetWritablePath();
		virtual std::string GetInitialDirectory();
		virtual bool Exists(const std::string& filename);
		virtual IReadFile* OpenFileForRead(const std::string& filename);
		virtual IWriteFile* OpenFileForWrite(const std::string& filename);
		virtual bool MakeDirectoryFromFilePath(const std::string filename);
		virtual bool SaveBufferToFile(const std::string& filename, bool replace, const char* buffer, uint32_t bufSize);

		virtual bool Copy(const std::string& src, const std::string& dest, bool override);
		virtual bool Move(const std::string& src, const std::string& dest);
		virtual bool Delete(const std::string& filename);
		virtual int DeleteDirectory(const std::string& filename);
		virtual std::string GetFullPathForFilename(const std::string &filename);
	
		virtual std::string GetExternalStoragePath();

		virtual void FindLocalFiles(CSearchResult& result, const std::string& sRootPath, const std::string& sFilePattern, int nSubLevel);
	};
}
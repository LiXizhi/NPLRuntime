#pragma once
#include "IO/IFile.h"
#include "IO/FileData.h"
#include <string>
namespace ParaEngine
{
	class CParaFileUtils
	{

	public:
		static CParaFileUtils* GetInstance();
		CParaFileUtils() = default;
		~CParaFileUtils() = default;
	public:
		virtual	ParaEngine::FileData GetDataFromFile(const std::string& filename) = 0;
		virtual bool IsAbsolutePath(const std::string& filename) = 0;
		virtual std::string GetWriteAblePath() = 0;
		virtual std::string GetInitialDirectory() = 0;
		virtual bool Exists(const std::string& filename) = 0;
		virtual IReadFile* OpenFileForRead(const std::string& filename) = 0;
		virtual IWriteFile* OpenFileForWrite(const std::string& filename) = 0;
		virtual bool MakeDirectoryFromFilePath(const std::string filename) = 0;
		virtual bool SaveBufferToFile(const std::string& filename, bool replace, const char* buffer, uint32_t bufSize) = 0;

		virtual bool Copy(const std::string& src, const std::string& dest, bool override) = 0;
		virtual bool Move(const std::string& src, const std::string& dest) = 0;
		virtual bool Delete(const std::string& filename) = 0;
		virtual int DeleteDirectory(const std::string& filename) = 0;
		virtual std::string GetFullPathForFilename(const std::string &filename) = 0;
	};
}
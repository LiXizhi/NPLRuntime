#pragma once
#include "Framework/FileSystem/ParaFileUtils.h"
#include "util/mutex.h"

namespace ParaEngine
{
	class CParaFileUtilsAndroid : public CParaFileUtils
	{

	public:
		virtual ParaEngine::FileData GetDataFromFile(const std::string& filename) override;
		virtual bool IsAbsolutePath(const std::string& filename) override;
		virtual std::string GetWritablePath() override;
		virtual std::string GetInitialDirectory() override;
		virtual bool Exists(const std::string& filename) override;
		virtual IReadFile* OpenFileForRead(const std::string& filename) override;
		virtual IWriteFile* OpenFileForWrite(const std::string& filename) override;
		virtual bool MakeDirectoryFromFilePath(const std::string filename) override;
		virtual bool SaveBufferToFile(const std::string& filename, bool replace, const char* buffer, uint32_t bufSize) override;
		virtual bool Copy(const std::string& src, const std::string& dest, bool override) override;
		virtual bool Move(const std::string& src, const std::string& dest) override;
		virtual bool Delete(const std::string& filename) override;
		virtual int DeleteDirectory(const std::string& filename) override;
		virtual std::string GetFullPathForFilename(const std::string &filename) override;

		virtual std::string GetExternalStoragePath() override;

		virtual void FindLocalFiles(CSearchResult& result, const std::string& sRootPath, const std::string& sFilePattern, int nSubLevel) override;
	protected:
		std::string m_writablePath;
		std::string m_externalStoragePath;
		static std::string _defaultResRootPath;
		ParaEngine::mutex m_fileMutex;
	};
}
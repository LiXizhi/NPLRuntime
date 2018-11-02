#pragma once
#include "Framework/FileSystem/ParaFileUtils.h"
namespace ParaEngine
{
	class CParaFileUtilsiOS : public CParaFileUtils
	{

	public:
		virtual ParaEngine::FileData GetDataFromFile(const std::string& filename) override;
		virtual bool IsAbsolutePath(const std::string& filename) override;
		virtual std::string GetWritablePath() override;
		virtual std::string GetInitialDirectory() override;
		virtual bool Exists(const std::string& filename) override;
		virtual IReadFile* OpenFileForRead(const std::string& filename) override;
		virtual IWriteFile* OpenFileForWrite(const std::string& filename) override;
		virtual bool Copy(const std::string& src, const std::string& dest, bool override) override;
		virtual bool Move(const std::string& src, const std::string& dest) override;
		virtual bool MakeDirectoryFromFilePath(const std::string filename) override;
		virtual bool SaveBufferToFile(const std::string& filename, bool replace, const char* buffer, uint32_t bufSize) override;
		virtual bool Delete(const std::string& filename) override;
	
		virtual int DeleteDirectory(const std::string& filename) override;


        virtual std::string GetFullPathForFilename(const std::string &filename) override;
        

        
    protected:
        const std::string& GetAssetPath();
	private:
		std::string m_writablePath;
        std::string m_assetPath;
	};
}

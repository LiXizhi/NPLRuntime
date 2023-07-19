#import <Cocoa/Cocoa.h>
#include "ParaEngine.h"
#include "ParaFileUtilsOSX.h"
#include <zlib.h>
#include "UrlLoaders.h"
#include "AsyncLoader.h"
#include "AssetManifest.h"
#include "ZipArchive.h"
#include "IParaEngineApp.h"
#include "FileUtils.h"
#include "util/StringBuilder.h"
#include "ParaEngineSettings.h"
#include "IO/ParaFile.h"
#include "IO/ReadFileBoost.h"
#include <boost/system/config.hpp>
#include <boost/system/system_error.hpp>
#include <boost/system/error_code.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>
#include <iostream>

namespace fs = boost::filesystem;
#define BOOST_FILESYSTEM_NO_DEPRECATED
using namespace ParaEngine;

CParaFileUtils* CParaFileUtils::GetInstance()
{
	static CParaFileUtilsOSX OSXImpl;
	return &OSXImpl;
}

ParaEngine::FileData CParaFileUtilsOSX::GetDataFromFile(const std::string& filename)
{
    auto file = OpenFileForRead(filename);
	FileData data;
    if(file->isOpen())
    {
        auto nSize = file->getSize();
        char* pBuffer = new char[nSize+1];
        pBuffer[nSize] = '\0';
        file->read(pBuffer,nSize);
        delete file;
        file = nullptr;
        data.SetOwnBuffer(pBuffer, nSize);
    }
	return data;
}

bool ParaEngine::CParaFileUtilsOSX::IsAbsolutePath(const std::string& filename)
{
    if (filename.size() > 0)
    {
        return filename[0] == '/' || filename[0] == '\\';
    }
    return false;
}

const std::string& ParaEngine::CParaFileUtilsOSX::GetWritablePath()
{
	if (m_writablePath.empty())
	{
        NSString* docDir = [NSSearchPathForDirectoriesInDomains(NSDocumentDirectory, NSUserDomainMask, YES) firstObject];
        m_writablePath = std::string([docDir UTF8String]) + "/Paracraft/files/";
	}
	return m_writablePath;
}

const std::string& ParaEngine::CParaFileUtilsOSX::GetInitialDirectory()
{
    return GetAssetPath();
}

bool ParaEngine::CParaFileUtilsOSX::Exists(const std::string& filename)
{
    if (IsAbsolutePath(filename))
    {
        boost::system::error_code err_code;
        if (fs::exists(filename, err_code))
            return true;
    }
    else
    {
        // find in writable directory first
        std::string fullPath = GetWritablePath() + filename;
        boost::system::error_code err_code;
        if (fs::exists(fullPath, err_code))
        {
            return true;
        }
        else
        {
            std::string fullPathInRes = GetAssetPath() + filename;
            if (fs::exists(fullPathInRes, err_code))
            {
                return true;
            }
        }
    }
    
    return false;
}

const std::string& ParaEngine::CParaFileUtilsOSX::GetAssetPath()
{
    if(m_assetPath.empty())
    {
        std::string resourcePath = [[[NSBundle mainBundle] resourcePath] UTF8String];
        m_assetPath = resourcePath + "/assets/";
    }
    return m_assetPath;
}

ParaEngine::IReadFile* ParaEngine::CParaFileUtilsOSX::OpenFileForRead(const std::string& filename)
{
    std::string fullPath = GetFullPathForFilename(filename);
    boost::system::error_code error_code;
    if(fs::exists(fullPath,error_code))
    {
        return new CReadFileBoost(fullPath);
    }
    else
    {
        return new CReadFileBoost(GetAssetPath() + filename);
    }
}

ParaEngine::IWriteFile* ParaEngine::CParaFileUtilsOSX::OpenFileForWrite(const std::string& filename)
{
	//throw std::logic_error("The method or operation is not implemented.");
	return nullptr;
}

bool ParaEngine::CParaFileUtilsOSX::Copy(const std::string& src, const std::string& dest, bool override)
{
    try
    {
        fs::path sSrc(GetFullPathForFilename(src));
        fs::path sDest(GetFullPathForFilename(dest));
        if (fs::exists(sDest))
        {
            if (override)
            {
                if (!fs::remove(sDest))
                {
                    return false;
                }
            }
            else
            {
                return false;
            }
        }
        fs::copy_file(sSrc, sDest);
    }
    catch (...)
    {
        return false;
    }
    return true;

}

bool ParaEngine::CParaFileUtilsOSX::Move(const std::string& src, const std::string& dest)
{
    try
    {
        fs::path sSrc(GetFullPathForFilename(src));
        fs::path sDest(GetFullPathForFilename(dest));
        fs::copy_file(sSrc,sDest, fs::copy_option::overwrite_if_exists);
        return fs::remove(sSrc);
    }
    catch (...)
    {
        return false;
    }
}

bool ParaEngine::CParaFileUtilsOSX::MakeDirectoryFromFilePath(const std::string filename)
{
    try
    {
        
        fs::path filePath(GetFullPathForFilename(filename));
        fs::path fileDir = filePath.parent_path();
        if (!fs::exists(fileDir))
            return fs::create_directories(fileDir);
        else
            return true;
    }
    catch (...) {}
    return false;
}


bool ParaEngine::CParaFileUtilsOSX::Delete(const std::string& filename)
{
    try
    {
        return fs::remove(GetFullPathForFilename(filename));
    }
    catch (...)
    {
        return false;
    }
}

int ParaEngine::CParaFileUtilsOSX::DeleteDirectory(const std::string& filename)
{
    try
    {
        return (int)fs::remove_all(GetFullPathForFilename(filename));
    }
    catch (...)
    {
        return 0;
    }
}

std::string ParaEngine::CParaFileUtilsOSX::GetFullPathForFilename(const std::string &filename)
{
    if (!IsAbsolutePath(filename))
        return GetWritablePath() + filename;
    else
        return filename;
}

bool ParaEngine::CParaFileUtilsOSX::SaveBufferToFile(const std::string& filename, bool replace, const char* buffer, uint32_t bufSize)
{
	try
	{
		if (MakeDirectoryFromFilePath(filename.c_str()) == false)
			return false;
		fs::path filePath(filename);
		if (fs::exists(filePath) && (replace == false))
			return false;

		fs::ofstream file(filePath);
		if (!file)
		{
			file.write(buffer, bufSize);
		}
	}
	catch (...) {}
	return false;
}



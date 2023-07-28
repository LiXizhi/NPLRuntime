//-----------------------------------------------------------------------------
// ParaFileUtilsAndroid.cpp
// Authors: LanZhiHong, big
// CreateDate: 2019.12.30
// ModifyDate: 2022.1.11
//-----------------------------------------------------------------------------

#include "ParaFileUtilsAndroid.h"
#include "ParaEngine.h"
#include <zlib.h>
#include "UrlLoaders.h"
#include "AsyncLoader.h"
#include "AssetManifest.h"
#include "ZipArchive.h"
#include "StringHelper.h"
#include "IParaEngineApp.h"
#include "FileUtils.h"
#include "util/StringBuilder.h"
#include "ParaEngineSettings.h"
#include "IO/ParaFile.h"
#include "IO/MemReadFile.h"
#include <boost/system/config.hpp>
#include <boost/system/system_error.hpp>
#include <boost/system/error_code.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>
#include <iostream>
#include "IO/ReadFileBoost.h"
#include "ReadFileAndroid.h"
#include "ParaAppAndroid.h"
#include "jni/ParaEngineHelper.h"
#include <android/asset_manager.h>
#include <android/native_activity.h>
#include <jni/JniHelper.h>

namespace fs = boost::filesystem;
#define BOOST_FILESYSTEM_NO_DEPRECATED
using namespace ParaEngine;

CParaFileUtils* CParaFileUtils::GetInstance()
{
	static CParaFileUtilsAndroid androidImpl;
	return &androidImpl;
}

ParaEngine::FileData ParaEngine::CParaFileUtilsAndroid::GetDataFromFile(const std::string& filename)
{
	FileData data;
	auto file = OpenFileForRead(filename);
	if (file->isOpen())
	{
		auto nSize = file->getSize();
		char* pBuffer = new char[nSize + 1];
		pBuffer[nSize] = '\0'; // always add an ending '\0' for ease for text parsing. 
		file->read(pBuffer, nSize);
		file->Release();

		data.SetOwnBuffer(pBuffer,nSize);
	}
    delete file;
    file = nullptr;

	return data;
}

bool ParaEngine::CParaFileUtilsAndroid::IsAbsolutePath(const std::string& filename)
{
	if (filename.size() > 0)
	{
		return filename[0] == '/' || filename[0] == '\\';
	}
	return false;
}

const std::string& ParaEngine::CParaFileUtilsAndroid::GetWritablePath()
{
	if (m_writablePath.empty())
	{
		m_writablePath = ParaEngineHelper::getWritablePath();

		if (m_writablePath[m_writablePath.size() - 1] != '/')
			m_writablePath += "/";

		if (m_writablePath[0] != '/')
			m_writablePath = std::string("/") + m_writablePath;
	}
	return m_writablePath;
}

const std::string& ParaEngine::CParaFileUtilsAndroid::GetExternalStoragePath()
{
	if (m_externalStoragePath.empty())
	{
		m_externalStoragePath = ParaEngineHelper::getExternalStoragePath();
		if (m_externalStoragePath[0] != '/')
			m_externalStoragePath = std::string("/") + m_externalStoragePath;
	}
	return m_externalStoragePath;
}


std::string ParaEngine::CParaFileUtilsAndroid::_defaultResRootPath = "assets/";

const std::string& ParaEngine::CParaFileUtilsAndroid::GetInitialDirectory()
{
	// on Android this will return "/", which is not right. we should return "" instead, so that current working directory is used. 
	// fs::path workingDir = fs::initial_path();
	// return workingDir.string();
	return CGlobals::GetString();
}

bool ParaEngine::CParaFileUtilsAndroid::Exists(const std::string& filename)
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
			// ParaEngine::Lock lock_(m_fileMutex); // AAssetManager is thread-safe according to andriod doc. 

			// find in android asset directory. 
			bool bFound = false;
			auto assetManager = JniHelper::getAssetManager();
			const char* s = filename.c_str();

			// Found "assets/" at the beginning of the path and we don't want it
			if (filename.find(_defaultResRootPath) == 0) 
				s += _defaultResRootPath.size();

			if (assetManager) {
				AAsset* aa = AAssetManager_open(assetManager, s, AASSET_MODE_UNKNOWN);
				if (aa)
				{
					bFound = true;
					AAsset_close(aa);
				}
			}
			if (bFound)
				return true;
		}
	}
	return false;
}

ParaEngine::IReadFile* ParaEngine::CParaFileUtilsAndroid::OpenFileForRead(const std::string& filename)
{
    if (filename.empty())
        return new CMemReadFile();

	std::string fullPath = GetFullPathForFilename(filename);
	fs::path fsFullPath(fullPath);

	if (fs::is_directory(fsFullPath)) {
		return new CMemReadFile();
	}

	boost::system::error_code err_code;
	if (fs::exists(fsFullPath, err_code))
	{
		return new CReadFileBoost(fullPath);
	}
	return new ParaEngine::CReadFileAndroid(filename);
}

ParaEngine::IWriteFile* ParaEngine::CParaFileUtilsAndroid::OpenFileForWrite(const std::string& filename)
{
	return nullptr;
}

bool ParaEngine::CParaFileUtilsAndroid::MakeDirectoryFromFilePath(const std::string filename)
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

bool ParaEngine::CParaFileUtilsAndroid::SaveBufferToFile(const std::string& filename, bool replace, const char* buffer, uint32_t bufSize)
{
	try
	{
		if (MakeDirectoryFromFilePath(filename.c_str()) == false)
			return false;
		fs::path filePath(GetFullPathForFilename(filename));
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

bool ParaEngine::CParaFileUtilsAndroid::Copy(const std::string& src, const std::string& dest, bool override)
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

bool ParaEngine::CParaFileUtilsAndroid::Move(const std::string& src, const std::string& dest)
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

bool ParaEngine::CParaFileUtilsAndroid::Delete(const std::string& filename)
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

int ParaEngine::CParaFileUtilsAndroid::DeleteDirectory(const std::string& filename)
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

std::string ParaEngine::CParaFileUtilsAndroid::GetFullPathForFilename(const std::string &filename)
{
	if (!IsAbsolutePath(filename))
		return GetWritablePath() + filename;
	else
		return filename;
}


void ParaEngine::CParaFileUtilsAndroid::FindLocalFiles(CSearchResult& result, const std::string& sRootPath, const std::string& sFilePattern, int nSubLevel)
{
	if (IsAbsolutePath(sRootPath))
		return;

	auto assetManager = JniHelper::getAssetManager();

	if (!assetManager)
		return;

	const char* s = sRootPath.c_str();
	// Found "assets/" at the beginning of the path and we don't want it
	if (sRootPath.find(_defaultResRootPath) == 0)
		s += _defaultResRootPath.size();

	auto assetDir = AAssetManager_openDir(assetManager, s);
	if (!assetDir)
		return;

	result.SetRootPath(s);

	FILETIME fileLastWriteTime;
	auto nextFilename = AAssetDir_getNextFileName(assetDir);
	while (nextFilename)
	{
		if (sFilePattern == "*." || sFilePattern == "*.*")
		{
			if (!result.AddResult(nextFilename, 0, 0, &fileLastWriteTime, &fileLastWriteTime, &fileLastWriteTime))
			{
				AAssetDir_close(assetDir);
				return;
			}
				
		}
		else if (ParaEngine::StringHelper::MatchWildcard(nextFilename, sFilePattern))
		{
			if (!result.AddResult(nextFilename, 0, 0, &fileLastWriteTime, &fileLastWriteTime, &fileLastWriteTime))
			{
				AAssetDir_close(assetDir);
				return;
			}
		}

		nextFilename = AAssetDir_getNextFileName(assetDir);
	}


	AAssetDir_close(assetDir);
}


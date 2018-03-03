#include "ParaFileUtilsAndroid.h"
#include "ParaEngine.h"
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
#include <android/asset_manager.h>
#include <android/native_activity.h>
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
	auto file = OpenFileForRead(filename);
	FileData data;
	if (file->isOpen())
	{
		auto nSize = file->getSize();
		char* pBuffer = new char[nSize + 1];
		pBuffer[nSize] = '\0'; // always add an ending '\0' for ease for text parsing. 
		file->read(pBuffer, nSize);
		file->Release();
		delete file;
		file = nullptr;
		data.SetOwnBuffer(pBuffer,nSize);
	}
	return data;
}

bool ParaEngine::CParaFileUtilsAndroid::IsAbsolutePath(const std::string& filename)
{
	/*if (filename.size() > 0)
	{
		return filename[0] == '/' || filename[0] == '\\';
	}*/
	return false;
}

std::string ParaEngine::CParaFileUtilsAndroid::GetWriteAblePath()
{
	if (m_writeAblePath.empty())
	{
		auto app = (CParaEngineAppAndroid*)(CGlobals::GetApp());
		auto state = app->GetAndroidApp();
		if (state->activity->externalDataPath)
		{
			m_writeAblePath = state->activity->externalDataPath;
		}
		else {
			JNIEnv* jni;
			state->activity->vm->AttachCurrentThread(&jni, NULL);
			jclass activityClass = jni->GetObjectClass(state->activity->clazz);
			jmethodID getFilesDir = jni->GetMethodID(activityClass, "getFilesDir", "()Ljava/io/File;");
			jobject fileObject = jni->CallObjectMethod(state->activity->clazz, getFilesDir);
			jclass fileClass = jni->GetObjectClass(fileObject);
			jmethodID getAbsolutePath = jni->GetMethodID(fileClass, "getAbsolutePath", "()Ljava/lang/String;");
			jobject pathObject = jni->CallObjectMethod(fileObject, getAbsolutePath);
			auto path = jni->GetStringUTFChars((jstring)pathObject, NULL);
			jni->DeleteLocalRef(pathObject);
			jni->DeleteLocalRef(fileClass);
			jni->DeleteLocalRef(fileObject);
			jni->DeleteLocalRef(activityClass);
			state->activity->vm->DetachCurrentThread();
			m_writeAblePath = path;
		}

		std::string sRootDir;
		CParaFile::ToCanonicalFilePath(sRootDir, m_writeAblePath, false);
		if (sRootDir.size() > 0 && (sRootDir[sRootDir.size() - 1] != '/' && sRootDir[sRootDir.size() - 1] != '\\'))
		{
			sRootDir += "/";
		}
		m_writeAblePath = sRootDir;
	}
	return m_writeAblePath;
}

std::string ParaEngine::CParaFileUtilsAndroid::GetInitialDirectory()
{
	fs::path workingDir = fs::initial_path();
	return workingDir.string();
}

bool ParaEngine::CParaFileUtilsAndroid::Exists(const std::string& filename)
{
	auto file = OpenFileForRead(filename);
	if (file->isOpen())
	{
		delete file;
		file = nullptr;
		return true;
	}
	else {
		delete file;
		file = nullptr;

		auto fullName = GetWriteAblePath() + filename;
		boost::system::error_code err_code;
		if (fs::exists(fullName, err_code))
		{
			return true;
		}else
		{
			//auto app = (CParaEngineAppAndroid*)(CGlobals::GetApp());
			//auto state = app->GetAndroidApp();
			//auto assetManager = state->activity->assetManager;
			//auto dir = AAssetManager_openDir(assetManager, filename.c_str());
			//if (dir)
			//{
			//	AAssetDir_close(dir);
			//	dir = nullptr;
			//	return true;
			//}
		}
	}
	return false;
}

ParaEngine::IReadFile* ParaEngine::CParaFileUtilsAndroid::OpenFileForRead(const std::string& filename)
{
	std::string fullPath = GetWriteAblePath() + filename;
	boost::system::error_code err_code;
	if (fs::exists(fullPath, err_code))
	{
		return new CReadFileBoost(fullPath);
	}
	return new ParaEngine::CReadFileAndroid(filename);
}

ParaEngine::IWriteFile* ParaEngine::CParaFileUtilsAndroid::OpenFileForWrite(const std::string& filename)
{
	
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
		fs::copy_file(sSrc,sDest);
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
	return GetWriteAblePath() + filename;
}


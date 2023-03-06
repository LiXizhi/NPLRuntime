#include "ParaFileUtilsWin32.h"
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
#include "ReadFileWin32.h"
#include "StringHelper.h"

namespace fs = boost::filesystem;
#define BOOST_FILESYSTEM_NO_DEPRECATED
using namespace ParaEngine;

CParaFileUtils* CParaFileUtils::GetInstance()
{
	static CParaFileUtilsWin32 win32Impl;
	return &win32Impl;
}

ParaEngine::FileData CParaFileUtilsWin32::GetDataFromFile(const std::string& filename)
{
	FileData data;
	fs::ifstream file;
#if WIN32 && defined(DEFAULT_FILE_ENCODING)
	LPCWSTR filename16 = StringHelper::MultiByteToWideChar(filename.c_str(), DEFAULT_FILE_ENCODING);
#else
	const auto& filename16 = filename;
#endif
	file.open(filename16, ios::in | ios::binary | ios::ate);
	if (file.is_open())
	{
		size_t nSize = (size_t)file.tellg();
		char* pBuffer = new char[nSize + 1];
		pBuffer[nSize] = '\0'; // always add an ending '\0' for ease for text parsing. 
		file.seekg(0, ios::beg);
		file.read(pBuffer, nSize);
		file.close();
		data.SetOwnBuffer(pBuffer, nSize);
	}
	return data;
}

bool ParaEngine::CParaFileUtilsWin32::IsAbsolutePath(const std::string& filename)
{
	if (filename.find(":") != string::npos)
		return true;
	if (filename.size() > 0)
	{
		return filename[0] == '/';
	}
	return false;
}

const std::string& ParaEngine::CParaFileUtilsWin32::GetWritablePath()
{
	if (m_writeAblePath.empty())
	{
		m_writeAblePath = GetInitialDirectory();
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

const std::string& ParaEngine::CParaFileUtilsWin32::GetInitialDirectory()
{
	fs::path workingDir = fs::initial_path();
#if WIN32 && defined(DEFAULT_FILE_ENCODING)
	static std::string ret = StringHelper::WideCharToMultiByte(workingDir.wstring().c_str(), DEFAULT_FILE_ENCODING);
#else
	static std::string ret = workingDir.string();
#endif
	
	return ret;
}

bool ParaEngine::CParaFileUtilsWin32::Exists(const std::string& filename)
{
	boost::system::error_code err_code;
#if WIN32 && defined(DEFAULT_FILE_ENCODING)
	LPCWSTR filename16 = StringHelper::MultiByteToWideChar(filename.c_str(), DEFAULT_FILE_ENCODING);
#else
	const auto& filename16 = filename;
#endif
	bool exists = fs::exists(filename16, err_code);
	return exists;
}

ParaEngine::IReadFile* ParaEngine::CParaFileUtilsWin32::OpenFileForRead(const std::string& filename)
{
	return new ParaEngine::CReadFileWin32(filename);
}

ParaEngine::IWriteFile* ParaEngine::CParaFileUtilsWin32::OpenFileForWrite(const std::string& filename)
{
	//throw std::logic_error("The method or operation is not implemented.");
	return nullptr;
}

bool ParaEngine::CParaFileUtilsWin32::Copy(const std::string& src, const std::string& dest, bool override)
{
	try
	{
#if WIN32 && defined(DEFAULT_FILE_ENCODING)
		std::wstring src16 = StringHelper::MultiByteToWideChar(src.c_str(), DEFAULT_FILE_ENCODING);
		LPCWSTR dest16 = StringHelper::MultiByteToWideChar(dest.c_str(), DEFAULT_FILE_ENCODING);
#else
		const auto& src16 = src;
		const auto& dest16 = dest;
#endif
		fs::path destFile(dest16);
		if (fs::exists(destFile))
		{
			if (override)
			{
				if (!fs::remove(destFile))
				{
					return false;
				}
			}
			else
			{
				return false;
			}
		}
		fs::copy_file(fs::path(src16.c_str()), destFile);
	}
	catch (...)
	{
		return false;
	}
	return true;
}

//bool ParaEngine::CParaFileUtilsWin32::Move(const std::string& src, const std::string& dest)
//{
//	try
//	{
//		fs::path sSrc(src);
//		fs::copy_file(sSrc, fs::path(dest));
//		return fs::remove(sSrc);
//	}
//	catch (...)
//	{
//		return false;
//	}
//}

bool ParaEngine::CParaFileUtilsWin32::MakeDirectoryFromFilePath(const std::string filename)
{
	try
	{
#if WIN32 && defined(DEFAULT_FILE_ENCODING)
		LPCWSTR filename16 = StringHelper::MultiByteToWideChar(filename.c_str(), DEFAULT_FILE_ENCODING);
#else
		const auto& filename16 = filename;
#endif
		fs::path filePath(filename16);
		fs::path fileDir = filePath.parent_path();
		if (!fs::exists(fileDir))
			return fs::create_directories(fileDir);
		else
			return true;
	}
	catch (...) {}
	return false;
}


bool ParaEngine::CParaFileUtilsWin32::Delete(const std::string& filename)
{
	try
	{
#if WIN32 && defined(DEFAULT_FILE_ENCODING)
		LPCWSTR filename16 = StringHelper::MultiByteToWideChar(filename.c_str(), DEFAULT_FILE_ENCODING);
#else
		const auto& filename16 = filename;
#endif
		return fs::remove(filename16);
	}
	catch (...)
	{
		return false;
	}
}

int ParaEngine::CParaFileUtilsWin32::DeleteDirectory(const std::string& filename)
{
	try
	{
#if WIN32 && defined(DEFAULT_FILE_ENCODING)
		LPCWSTR filename16 = StringHelper::MultiByteToWideChar(filename.c_str(), DEFAULT_FILE_ENCODING);
#else
		const auto& filename16 = filename;
#endif
		return (int)fs::remove_all(filename16);
	}
	catch (...)
	{
		return 0;
	}
}

std::string ParaEngine::CParaFileUtilsWin32::GetFullPathForFilename(const std::string &filename)
{
#if WIN32 && defined(DEFAULT_FILE_ENCODING)
	LPCWSTR filename16 = StringHelper::MultiByteToWideChar(filename.c_str(), DEFAULT_FILE_ENCODING);
#else
	const auto& filename16 = filename;
#endif
	fs::path filepath(filename16);
	fs::path abs_path = fs::absolute(filepath);
	return abs_path.string();
}

bool ParaEngine::CParaFileUtilsWin32::SaveBufferToFile(const std::string& filename, bool replace, const char* buffer, uint32_t bufSize)
{
	try
	{
		if (MakeDirectoryFromFilePath(filename.c_str()) == false)
			return false;
#if WIN32 && defined(DEFAULT_FILE_ENCODING)
		LPCWSTR filename16 = StringHelper::MultiByteToWideChar(filename.c_str(), DEFAULT_FILE_ENCODING);
#else
		const auto& filename16 = filename;
#endif
		fs::path filePath(filename16);
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



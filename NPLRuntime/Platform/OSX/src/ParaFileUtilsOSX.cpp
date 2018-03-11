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
#include <boost/system/config.hpp>
#include <boost/system/system_error.hpp>
#include <boost/system/error_code.hpp>
#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>
#include <iostream>
#include "ReadFileOSX.h"
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
	FileData data;
	fs::ifstream file;
	file.open(filename, ios::in | ios::binary | ios::ate);
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

bool ParaEngine::CParaFileUtilsOSX::IsAbsolutePath(const std::string& filename)
{
	if (filename.find(":") != string::npos)
		return true;
	if (filename.size() > 0)
	{
		return filename[0] == '/';
	}
	return false;
}

std::string ParaEngine::CParaFileUtilsOSX::GetWriteAblePath()
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

std::string ParaEngine::CParaFileUtilsOSX::GetInitialDirectory()
{
	fs::path workingDir = fs::initial_path();
	return workingDir.string();
}

bool ParaEngine::CParaFileUtilsOSX::Exists(const std::string& filename)
{
	boost::system::error_code err_code;
	bool exists = fs::exists(filename, err_code);
	return exists;
}

ParaEngine::IReadFile* ParaEngine::CParaFileUtilsOSX::OpenFileForRead(const std::string& filename)
{
	return new ParaEngine::CReadFileOSX(filename);
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
		fs::path destFile(dest);
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
		fs::copy_file(fs::path(src), destFile);
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
		fs::path sSrc(src);
		fs::copy_file(sSrc, fs::path(dest));
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
		fs::path filePath(filename);
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
		return fs::remove(filename);
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
		return (int)fs::remove_all(filename);
	}
	catch (...)
	{
		return 0;
	}
}

std::string ParaEngine::CParaFileUtilsOSX::GetFullPathForFilename(const std::string &filename)
{
	fs::path filepath(filename);
	fs::path abs_path = fs::absolute(filepath);
	return abs_path.string();
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



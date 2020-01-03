#include "ParaEngine.h"
#include "FileUtils.h"
#include "ReadFile.h"
#include <boost/filesystem.hpp>
#include <boost/filesystem/operations.hpp>
#include <boost/filesystem/path.hpp>
#include <boost/filesystem/fstream.hpp>
#include <iostream>
#include "ParaFileUtils.h"

namespace fs = boost::filesystem;
#define BOOST_FILESYSTEM_NO_DEPRECATED
using namespace ParaEngine;

#if (PARA_TARGET_PLATFORM == PARA_PLATFORM_LINUX) || defined(MAC_SERVER)
CParaFileUtils* CParaFileUtils::GetInstance()
{
	static CParaFileUtils defaultImpl;
	return &defaultImpl;
}
#endif

ParaEngine::FileData CParaFileUtils::GetDataFromFile(const std::string& filename)
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

bool ParaEngine::CParaFileUtils::IsAbsolutePath(const std::string& filename)
{
	if (filename.find(":") != string::npos)
		return true;
	if (filename.size() > 0)
	{
		return filename[0] == '/';
	}
	return false;
}

std::string g_writablePath;
const std::string& ParaEngine::CParaFileUtils::GetWritablePath()
{
	if (g_writablePath.empty())
	{
		g_writablePath = GetInitialDirectory();
		std::string sRootDir;
		CParaFile::ToCanonicalFilePath(sRootDir, g_writablePath);
		if (sRootDir.size() > 0 && (sRootDir[sRootDir.size() - 1] != '/' && sRootDir[sRootDir.size() - 1] != '\\'))
		{
			sRootDir += "/";
		}
		g_writablePath = sRootDir;
	}
	return g_writablePath;
}

const std::string& ParaEngine::CParaFileUtils::GetInitialDirectory()
{
	fs::path workingDir = fs::initial_path();
	return workingDir.string();
}

bool ParaEngine::CParaFileUtils::Exists(const std::string& filename)
{
	boost::system::error_code err_code;
	bool exists = fs::exists(filename, err_code);
	return exists;
}

ParaEngine::IReadFile* ParaEngine::CParaFileUtils::OpenFileForRead(const std::string& filename)
{
	return new ParaEngine::CReadFile(filename);
}

ParaEngine::IWriteFile* ParaEngine::CParaFileUtils::OpenFileForWrite(const std::string& filename)
{
	//throw std::logic_error("The method or operation is not implemented.");
	return nullptr;
}

bool ParaEngine::CParaFileUtils::Copy(const std::string& src, const std::string& dest, bool override)
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

bool ParaEngine::CParaFileUtils::Move(const std::string& src, const std::string& dest)
{
	try
	{
		fs::path sSrc(src);
		boost::system::error_code err_code;
		fs::rename(sSrc, fs::path(dest), err_code);
		OUTPUT_LOG("info (boost-fs): moved file/directory from %s to %s result message: %s\n", src.c_str(), dest.c_str(), err_code.message().c_str());
		return err_code.value() == 0;
	}
	catch (...)
	{
		return false;
	}
}

bool ParaEngine::CParaFileUtils::MakeDirectoryFromFilePath(const std::string filename)
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


bool ParaEngine::CParaFileUtils::Delete(const std::string& filename)
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

int ParaEngine::CParaFileUtils::DeleteDirectory(const std::string& filename)
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

std::string ParaEngine::CParaFileUtils::GetFullPathForFilename(const std::string &filename)
{
	fs::path filepath(filename);
	fs::path abs_path = fs::absolute(filepath);
	return abs_path.string();
}

const std::string& ParaEngine::CParaFileUtils::GetExternalStoragePath()
{
	return CGlobals::GetString();
}

bool ParaEngine::CParaFileUtils::SaveBufferToFile(const std::string& filename, bool replace, const char* buffer, uint32_t bufSize)
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

void ParaEngine::CParaFileUtils::FindLocalFiles(CSearchResult& result, const std::string& sRootPath, const std::string& sFilePattern, int nSubLevel)
{
	// TODO
}



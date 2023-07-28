//-----------------------------------------------------------------------------
// Class: CReadFile
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2006.1.10
// Revised: 2006.1.10
// Notes: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ReadFileBoost.h"
#include <stdio.h>
#include "StringHelper.h"

using namespace ParaEngine;

CReadFileBoost::CReadFileBoost(const std::string& fileName)
: m_FileSize(0)
{
	m_Filename = fileName;
	openFile();
}



CReadFileBoost::~CReadFileBoost()
{
	if (m_pFile)
		fclose(m_pFile);
}



/// returns if file is open
inline bool CReadFileBoost::isOpen()
{
	return m_pFile != 0;
}



/// returns how much was read
uint32_t CReadFileBoost::read(void* buffer, uint32_t sizeToRead)
{
	if (!isOpen())
		return 0;

	return (uint32_t)fread(buffer, 1, sizeToRead, m_pFile);
}



/// changes position in file, returns true if successful
/// if relativeMovement==true, the pos is changed relative to current pos,
/// otherwise from begin of file
bool CReadFileBoost::seek(uint32_t finalPos, bool relativeMovement)
{
	if (!isOpen())
		return false;

	return fseek(m_pFile, finalPos, relativeMovement ? SEEK_CUR : SEEK_SET) == 0;
}



/// returns size of file
uint32_t CReadFileBoost::getSize()
{
	return m_FileSize;
}



/// returns where in the file we are.
uint32_t CReadFileBoost::getPos()
{
	return ftell(m_pFile);
}



/// opens the file
void CReadFileBoost::openFile()
{
	if (m_Filename.size() == 0) // bugfix posted by rt
	{
		m_pFile = 0;
		return; 
	}

	std::string filename = m_Filename;
	CParaFile::DoesFileExist2(m_Filename.c_str(), FILE_ON_DISK | FILE_ON_SEARCH_PATH, &filename);
#if WIN32 && defined(DEFAULT_FILE_ENCODING)
	std::wstring filename16 = StringHelper::MultiByteToWideChar(filename.c_str(), DEFAULT_FILE_ENCODING);
	m_pFile = _wfopen(filename16.c_str(), L"rb");
#else
	m_pFile = fopen(filename.c_str(), "rb");
#endif
	

	if (m_pFile)
	{
		// TODO: get FileSize, use fstat instead? opening folder or large file will crash with fseek. 
		if(fseek(m_pFile, 0, SEEK_END) != 0) {
			// handle error
			fclose(m_pFile);
			m_pFile = 0;
			return;
		}
		auto fileSize = ftell(m_pFile);

		if(fileSize == -1 || fileSize > 1000000000){
			// handle error, ftell return 0x7FFFFFFFFFFFFFFF or -1 on some linux system. 
			m_FileSize = 0;
			fclose(m_pFile);
			m_pFile = 0;
			return;
		}
		else{
			m_FileSize = fileSize;
		}

		fseek(m_pFile, 0, SEEK_SET);
	}
}

/// returns name of file
const char* CReadFileBoost::getFileName()
{
	return m_Filename.c_str();
}

//-----------------------------------------------------------------------------
// Class: CReadFile
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2006.1.10
// Revised: 2006.1.10
// Notes: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ReadFile.h"
#include <stdio.h>

using namespace ParaEngine;

CReadFile::CReadFile(const std::string& fileName)
: m_FileSize(0)
{
	m_Filename = fileName;
	openFile();
}



CReadFile::~CReadFile()
{
	if (m_pFile)
		fclose(m_pFile);
}



/// returns if file is open
inline bool CReadFile::isOpen()
{
	return m_pFile != 0;
}



/// returns how much was read
DWORD CReadFile::read(void* buffer, DWORD sizeToRead)
{
	if (!isOpen())
		return 0;

	return (DWORD)fread(buffer, 1, sizeToRead, m_pFile);
}



/// changes position in file, returns true if successful
/// if relativeMovement==true, the pos is changed relative to current pos,
/// otherwise from begin of file
bool CReadFile::seek(DWORD finalPos, bool relativeMovement)
{
	if (!isOpen())
		return false;

	return fseek(m_pFile, finalPos, relativeMovement ? SEEK_CUR : SEEK_SET) == 0;
}



/// returns size of file
DWORD CReadFile::getSize()
{
	return m_FileSize;
}



/// returns where in the file we are.
DWORD CReadFile::getPos()
{
	return ftell(m_pFile);
}



/// opens the file
void CReadFile::openFile()
{
	if (m_Filename.size() == 0) // bugfix posted by rt
	{
		m_pFile = 0;
		return; 
	}

	std::string filename = m_Filename;
	CParaFile::DoesFileExist2(m_Filename.c_str(), FILE_ON_DISK | FILE_ON_SEARCH_PATH, &filename);
	m_pFile = fopen(filename.c_str(), "rb");

	if (m_pFile)
	{
		// get FileSize

		fseek(m_pFile, 0, SEEK_END);
		m_FileSize = ftell(m_pFile);
		fseek(m_pFile, 0, SEEK_SET);
	}
}



/// returns name of file
const char* CReadFile::getFileName()
{
	return m_Filename.c_str();
}

//
//
//CReadFile* createReadFile(const char* fileName)
//{
//	CReadFile* file = new CReadFile(fileName);
//	if (file->isOpen())
//		return file;
//	return 0;
//}
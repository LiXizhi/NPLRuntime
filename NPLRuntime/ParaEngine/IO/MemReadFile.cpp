//-----------------------------------------------------------------------------
// Class: CMemReadFile
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2006.1.12
// Revised: 2006.1.12
// Notes: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include <stdio.h>
#include "FileManager.h"
#ifdef PARAENGINE_CLIENT
#include "memdebug.h"
#endif
#include "MemReadFile.h"

using namespace ParaEngine;

CMemReadFile::CMemReadFile(void)
	:m_CacheStartPos(0), m_CacheEndPos(0), m_CacheData(NULL), m_curPos(0), m_bOwnBuffer(true)
{
}

CMemReadFile::CMemReadFile(IReadFile* pFile, uint32_t nSize)
	: m_bOwnBuffer(true)
{
	m_CacheData = new unsigned char[nSize];
	m_CacheStartPos = pFile->getPos();
	m_curPos = m_CacheStartPos;
	m_CacheEndPos = m_CacheStartPos + nSize;
	if (pFile->read(m_CacheData, nSize) != nSize)
	{
		Unload();
		return;
	}
	pFile->seek(m_CacheStartPos, false); // restore file pos.
}
CMemReadFile::CMemReadFile(unsigned char* buffer, uint32_t nSize, bool bDeleteBuffer)
	:m_bOwnBuffer(bDeleteBuffer)
{
	if (buffer != 0)
	{
		m_CacheData = buffer;
		m_curPos = 0;
		m_CacheStartPos = 0;
		m_CacheEndPos = nSize;
	}
}

ParaEngine::CMemReadFile::CMemReadFile(const char * filename)
	:m_CacheStartPos(0), m_CacheEndPos(0), m_CacheData(NULL), m_curPos(0), m_bOwnBuffer(true)
{
	CParaFile file(filename);
	if (!file.isEof())
	{
		m_CacheData = (unsigned char*)file.getBuffer();
		m_CacheEndPos = file.getSize();
		file.GiveupBufferOwnership();
	}
}

CMemReadFile::~CMemReadFile(void)
{
	Unload();
}

void CMemReadFile::Unload()
{
	m_CacheStartPos = 0;
	m_CacheEndPos = 0;
	m_curPos = 0;
	if (m_bOwnBuffer)
	{
		SAFE_DELETE_ARRAY(m_CacheData);
	}
}

/// returns how much was read
uint32_t CMemReadFile::read(void* buffer, uint32_t sizeToRead)
{
	if (!isOpen())
		return 0;

	if (m_curPos >= m_CacheStartPos && m_curPos + sizeToRead <= m_CacheEndPos)
	{
		memcpy(buffer, m_CacheData + (m_curPos - m_CacheStartPos), sizeToRead);
		m_curPos += sizeToRead;
		return sizeToRead;
	}
	return 0;
}

unsigned char* CMemReadFile::getBuffer()
{
	if (isOpen() && m_curPos >= m_CacheStartPos) {
		return m_CacheData + (m_curPos - m_CacheStartPos);
	}
	return 0;
}

/// changes position in file, returns true if successful
/// if relativeMovement==true, the pos is changed relative to current pos,
/// otherwise from begin of file
bool CMemReadFile::seek(uint32_t finalPos, bool relativeMovement)
{
	if (!isOpen())
		return false;

	if (relativeMovement)
	{
		if (m_curPos + finalPos > m_CacheEndPos)
			return false;
		m_curPos += finalPos;
	}
	else
	{
		if (finalPos > m_CacheEndPos)
			return false;
		m_curPos = finalPos;
	}
	return true;
}
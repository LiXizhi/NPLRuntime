//-----------------------------------------------------------------------------
// Class: FileHandle
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2005.4, 2014.8
// Revised: refactored from file manager
// Notes: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "FileHandle.h"

bool ParaEngine::FileHandle::IsValid()
{
#ifdef WIN32
	return m_handle != INVALID_HANDLE_VALUE && m_handle != 0;
#else
	return m_pFile != 0;
#endif
}

ParaEngine::FileHandle::FileHandle()
{
#ifdef WIN32
	m_handle = INVALID_HANDLE_VALUE;
#else
	m_handle = NULL; 
#endif
	
	m_pArchive = NULL;
}

ParaEngine::FileHandle::FileHandle(HANDLE handle)
{
	m_handle = handle; 
	m_pArchive = NULL;
}

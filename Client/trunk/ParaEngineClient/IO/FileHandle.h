//-----------------------------------------------------------------------------
// Copyright (C) 2004 - 2006 ParaEngine Dev Studio, All Rights Reserved.
// Date:	2005.3
// Description:	
//-----------------------------------------------------------------------------
#pragma once

namespace ParaEngine
{
	class CArchive;

	/** a file handle in a Archive */
	struct FileHandle
	{
		union{
			HANDLE	m_handle;
			DWORD	m_index;
			FILE*	m_pFile;
			void* m_pVoidPtr;
		};
		CArchive* m_pArchive;
		FileHandle(HANDLE handle);
		FileHandle();

		bool IsValid();
	};

}

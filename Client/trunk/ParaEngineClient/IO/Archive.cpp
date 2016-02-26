//-----------------------------------------------------------------------------
// Class: CArchive
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2006.1.10
// Revised: 2006.1.10
// Notes: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "Archive.h"

namespace ParaEngine
{
	CArchive::~CArchive(void)
	{
		Close();
	}

	const std::string& CArchive::GetIdentifier()
	{
		return GetArchiveName();
	}

	const std::string& CArchive::GetArchiveName() const
	{
		return m_filename;
	}

	bool CArchive::IsArchive(const std::string& sFilename)
	{
		if (m_filename.size() != sFilename.size() || sFilename.size() < 4)
			return false;
		// skip file name (last 4 letters)
		int nSize = (int) m_filename.size() - 4;
		for (int i = 0; i < nSize; ++i)
		{
			char c = m_filename[i];
			// ignore slashes
			if (c != sFilename[i] && c != '/'  && c != '\\')
				return false;
		}
		return true;
	}

	bool CArchive::Open(const std::string& sArchiveName, int nPriority)
	{
		m_filename = sArchiveName;
		m_nPriority = nPriority;
		return true;
	};
}

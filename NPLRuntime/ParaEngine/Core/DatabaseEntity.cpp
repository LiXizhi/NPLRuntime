//-----------------------------------------------------------------------------
// Class:	DatabaseEntity
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2005
// Revised: 2014.8
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "sqlite3.h"
#include "DatabaseEntity.h"

using namespace ParaEngine;

void DatabaseEntity::Cleanup()
{
	CloseDB();
};

sqlite3* DatabaseEntity::OpenDB()
{
	/// only open if it is not already open
	if (m_pDB == NULL)
	{
		int rc;
		string path;
		CParaFile::ToCanonicalFilePath(path, sFileName);
		rc = sqlite3_open(path.c_str(), &m_pDB);
		if (rc != SQLITE_OK){
			OUTPUT_LOG("failed open file %s", path.c_str());
			sqlite3_close(m_pDB);
			m_pDB = NULL;
		}
	}
	return m_pDB;
}

void DatabaseEntity::CloseDB()
{
	if (m_pDB)
	{
		sqlite3_close(m_pDB);
		m_pDB = NULL;
	}
}

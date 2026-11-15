//-----------------------------------------------------------------------------
// Class:	CDataProviderManager
// Authors:	Li, Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2006.5
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "InfoCenter/ICDBManager.h"
#include "InfoCenter/ICRecordSet.h"
#ifndef NPLRUNTIME_SERVER
#include "CharacterDB.h"
#include "NpcDatabase.h"
#include "KidsDBProvider.h"
#endif
#include "AttributeProvider.h"

#include "DataProviderManager.h"

using namespace ParaInfoCenter;
using namespace ParaEngine;


CDataProviderManager::CDataProviderManager(void)
{
	m_dbAttribute = new AttributeProvider();
#ifndef NPLRUNTIME_SERVER
	m_dbNPC = new CNpcDatabase();
	m_dbKids = new CKidsDBProvider();
#endif
}

CDataProviderManager::~CDataProviderManager(void)
{
	Cleanup();
}

void CDataProviderManager::Cleanup()
{
	SAFE_DELETE(m_dbAttribute);
#ifndef NPLRUNTIME_SERVER
	SAFE_DELETE(m_dbNPC);
	SAFE_DELETE(m_dbKids);
#endif
}

CDataProviderManager& CDataProviderManager::GetSingleton()
{
	static CDataProviderManager g_singleton;
	return g_singleton;
}

CCharacterDB* CDataProviderManager::GetCharacterDB()
{
#ifndef NPLRUNTIME_SERVER
	return &CCharacterDB::GetInstance();
#else
	return NULL;
#endif
}

CKidsDBProvider* CDataProviderManager::GetKidsDBProvider()
{
#ifndef NPLRUNTIME_SERVER
	if(!m_dbKids->IsValid())
	{
		//////////////////////////////////////////////////////////////////////////
		// load the default database
		const char tempdb[] = "database/Kids.db";
		SetKidsDBProvider(tempdb);
		OUTPUT_LOG("default Kids game database is loaded from %s\r\n", tempdb);
	}
	return m_dbKids;
#else
	return NULL;
#endif
}

CNpcDatabase* CDataProviderManager::GetNpcDB()
{
#ifndef NPLRUNTIME_SERVER
	if(!m_dbNPC->IsValid())
	{
		//////////////////////////////////////////////////////////////////////////
		// load a blank temp database
		const char tempdb[] = "temp/npc.db";
		SetNpcDB(tempdb);
		m_dbNPC->ResetDatabase();
		OUTPUT_LOG("temp NPC database is created at %s\r\n", tempdb);
	}
	return m_dbNPC;
#else
	return NULL;
#endif
}

AttributeProvider* CDataProviderManager::GetAttributeProvider()
{
	if(!m_dbAttribute->IsValid())
	{
		//////////////////////////////////////////////////////////////////////////
		// load a blank temp database
		const char tempdb[] = "temp/attribute.db";
		SetAttributeProvider(tempdb);
		OUTPUT_LOG("temp attribute database is created at %s\r\n", tempdb);
	}
	return m_dbAttribute; 
}

void CDataProviderManager::SetNpcDB(const string& sConnectionstring)
{
#ifndef NPLRUNTIME_SERVER
	m_dbNPC->SetDBEntity(sConnectionstring);
#endif
}

void CDataProviderManager::SetAttributeProvider(const string& sConnectionstring)
{
	m_dbAttribute->SetDBEntity(sConnectionstring);
}

void CDataProviderManager::SetKidsDBProvider(const string& sConnectionstring)
{
#ifndef NPLRUNTIME_SERVER
	m_dbKids->SetDBEntity(sConnectionstring);
#endif
}

LocalGameLanguage CDataProviderManager::GetLocalGameLanguage(void)
{
	return m_language;
}

void CDataProviderManager::SetLocalGameLanguage(const LocalGameLanguage lang)
{
	m_language = lang;
}
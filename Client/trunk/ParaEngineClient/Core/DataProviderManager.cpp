//-----------------------------------------------------------------------------
// Class:	CDataProviderManager
// Authors:	Li, Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2006.5
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ic/ICDBManager.h"
#include "ic/ICRecordSet.h"
#ifdef PARAENGINE_CLIENT
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
#ifdef PARAENGINE_CLIENT
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
#ifdef PARAENGINE_CLIENT
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
#ifdef PARAENGINE_CLIENT
	return &CCharacterDB::GetInstance();
#else
	return NULL;
#endif
}

CKidsDBProvider* CDataProviderManager::GetKidsDBProvider()
{
#ifdef PARAENGINE_CLIENT
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
#ifdef PARAENGINE_CLIENT
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
#ifdef PARAENGINE_CLIENT
	m_dbNPC->SetDBEntity(sConnectionstring);
#endif
}

void CDataProviderManager::SetAttributeProvider(const string& sConnectionstring)
{
	m_dbAttribute->SetDBEntity(sConnectionstring);
}

void CDataProviderManager::SetKidsDBProvider(const string& sConnectionstring)
{
#ifdef PARAENGINE_CLIENT
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
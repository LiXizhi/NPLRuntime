//-----------------------------------------------------------------------------
// Class:	CNpcDatabase
// Authors:	Li, Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2006.5.22
// Note: currently there does not seem to be an automatic way to check for table existence in sqlite3. However,
// can manually create a database called sys.table etc for keeping meta data in the database, but there is a cost of maintaineous anyway.
// Please see db_npc_procedures.sql and db_create_npc_table.sql
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "InfoCenter/ICDBManager.h"
#include "InfoCenter/ICRecordSet.h"
#include "NpcDatabase.h"

using namespace ParaInfoCenter;
using namespace ParaEngine;

/** perform test cases when reseting db.*/
//#define TEST_DB

///////////////////////////////////////////////////////////////////////////
//
// CNpcDbItem
//
//////////////////////////////////////////////////////////////////////////

void CNpcDbItem::LoadDefaultValues()
{
	m_nID = 0;
	m_sName.clear();
	m_sAssetName.clear();
	m_bIsGlobal = true;
	m_bSnapToTerrain = true;
	m_fRadius = 0.35f;
	m_fFacing = 0;;
	m_fScaling = 1;
	m_vPos = Vector3(0,0,0);
	m_nCharacterType = 0;

	memset(m_MentalState, 0, sizeof(m_MentalState));

	m_fLifePoint = 100;
	m_fAge = 1;
	m_fHeight = 1.78f;
	m_fWeight = 55;
	m_nOccupation = 0;
	m_nRaceSex = 0;

	m_fStrength = 0;
	m_fDexterity = 0;
	m_fIntelligence = 0;
	m_fBaseDefense = 0;
	m_fDefense = m_fDefenseflat = m_fDefenseMental = 0;
	m_fBaseAttack = 0;
	m_fAttackMelee = m_fAttackRanged = m_fAttackMental = 0;
	m_fMaxLifeLoad = 10;

	m_nHeroPoints = 0;

	m_fPerceptiveRadius = 7;
	m_fSentientRadius = 50;
	m_nGroupID = 0;
	m_dwSentientField = 0;
	m_sOnLoadScript.clear();
}

///////////////////////////////////////////////////////////////////////////
//
// CNpcDatabase
//
//////////////////////////////////////////////////////////////////////////

CNpcDatabase::CNpcDatabase(void)
:m_pProcInsertNPC(NULL),
m_pProcSelectNPCByID(NULL),
m_pProcSelectNPCByName(NULL),
m_pProcSelectNPCListByRegion(NULL),
m_pProcGetNPCIDByName(NULL),
m_pProcDeleteNPCByID(NULL),
m_pProcDeleteNPCByName(NULL),
m_pProcUpdateNPC(NULL)
{
	
}

CNpcDatabase::~CNpcDatabase(void)
{
}
void CNpcDatabase::Cleanup()
{
	m_pDataBase.reset();
	SAFE_RELEASE(m_pProcInsertNPC);
	SAFE_RELEASE(m_pProcSelectNPCByID);
	SAFE_RELEASE(m_pProcSelectNPCByName);
	SAFE_RELEASE(m_pProcSelectNPCListByRegion);
	SAFE_RELEASE(m_pProcGetNPCIDByName);
	SAFE_RELEASE(m_pProcDeleteNPCByID);
	SAFE_RELEASE(m_pProcDeleteNPCByName);
	SAFE_RELEASE(m_pProcUpdateNPC);
}
void CNpcDatabase::SetDBEntity(const string& sConnectionstring)
{
	if (sConnectionstring.empty())
	{
		Cleanup();
		return;
	}

	string fileName;
	CParaFile::ToCanonicalFilePath(fileName, sConnectionstring);
	if( (m_pDataBase.get()!=0) && 
		(m_pDataBase->GetConnectionString() == fileName))
	{
		// already opened the database
		return;
	}
	// close the old connection
	Cleanup();

	//////////////////////////////////////////////////////////////////////////
	// open the new database file. 
	
	m_pDataBase = CICDBManager::OpenDBEntity("npc", fileName.c_str());
	if (m_pDataBase.get() == 0|| !m_pDataBase->IsValid())
	{
		OUTPUT_LOG("failed open database file %s", fileName.c_str());
		return;
	}
	if(m_pDataBase->IsCreateFile())
	{
		m_pDataBase->SetCreateFile(false);
		ResetDatabase();
	}
}

void CNpcDatabase::ResetDatabase()
{
	DBEntity* pDB = GetDBEntity();
	if(pDB==NULL || !pDB->IsValid()) 
		return;
	CParaFile file(":IDR_DB_NPC_TABLE");
	if(!file.isEof())
	{
		pDB->ExecuteSqlScript(file.getBuffer());
#ifdef _DEBUG
#ifdef TEST_DB
		TestDB();
#endif
#endif
	}
	else
	{
		OUTPUT_LOG("error: failed loading create NPC table SQL script\r\n");
	}
}
#ifdef _DEBUG
void CNpcDatabase::TestDB()
{
	// test insertion
	CNpcDbItem item;
	item.LoadDefaultValues();
	item.m_sName ="hello";
	InsertNPC(item);

	// test updates 
	//item.m_fAge = 100;
	//item.m_dwSentientField = 0xFF;
	//UpdateNPC(item, CNpcDbItem::ALL_ATTRIBUTES);

	// test selection id
	//CNpcDbItem itemOut;
	//SelectNPCByID(1, itemOut);

	// test selection name
	//CNpcDbItem itemOut2;
	//SelectNPCByName("hello", itemOut2);

	// test GetNPCIDByName
	//OUTPUT_LOG("hello id is %d\r\n", GetNPCIDByName("hello"));

	// test delete and GetNPCCount
	//OUTPUT_LOG("NPC count in DB is %d\r\n", GetNPCCount());
	// DeleteNPCByID(1);
	// DeleteNPCByName("hello");
	//OUTPUT_LOG("NPC count in DB is %d\r\n", GetNPCCount());


	// test region selection
	/*item.m_sName ="hello1";
	item.m_vPos = Vector3(5, 0, 5);
	InsertNPC(item);

	item.m_sName ="hello2";
	item.m_vPos = Vector3(5, 0, 6);
	InsertNPC(item);

	NPCDBItemList listNPC;
	SelectNPCListByRegion(&listNPC, Vector3(0,0,0), Vector3(5.5,0,5.5));
	for (NPCDBItemList::iterator itCur = listNPC.begin(); itCur!=listNPC.end();++itCur)
	{
	OUTPUT_LOG("region item %s \r\n", itCur->m_sName.c_str());
	}*/
}
#endif

void CNpcDatabase::ValidateDatabase()
{
	assert(m_pDataBase.get()!=0);
	//m_pDataBase->prepare_sql("")
	// currently it does nothing, there does not seem to be an automatic way to check for table existence in sqlite3. However,
	// can manually create a database called sys.table etc for keeping meta data in the database, but there is a cost of maintaineous anyway.
}

bool CNpcDatabase::InsertNPC(CNpcDbItem& npc)
{
	DBEntity* pDB = GetDBEntity();
	assert(pDB!=NULL);
	bool bSucceed = false;
	if(m_pProcInsertNPC == NULL)
	{
		// create the SQL procedure
		m_pProcInsertNPC = pDB->CreateRecordSet1("\
			INSERT INTO NPC\
			([ID], [Name], AssetName, IsGlobal, SnapToTerrain, Radius, Facing, Scaling, posX, posY, posZ, CharacterType, MentalState0, MentalState1, MentalState2, \
			MentalState3, LifePoint, Age, Height, Weight, Occupation, RaceSex, Strength, Dexterity, Intelligence, BaseDefense, Defense, Defenseflat, \
			DefenseMental, BaseAttack, AttackMelee, AttackRanged, AttackMental, MaxLifeLoad, HeroPoints, PerceptiveRadius, SentientRadius, GroupID, \
			SentientField, OnLoadScript, CustomAppearance)\
				VALUES\
				(NULL,\
				@Name,\
				@AssetName,\
				@IsGlobal,\
				@SnapToTerrain,\
				@Radius,\
				@Facing,\
				@Scaling,\
				@posX, \
				@posY, \
				@posZ, \
				@CharacterType, \
				@MentalState0, \
				@MentalState1, \
				@MentalState2, \
				@MentalState3, \
				@LifePoint, \
				@Age, \
				@Height, \
				@Weight, \
				@Occupation, \
				@RaceSex, \
				@Strength, \
				@Dexterity, \
				@Intelligence, \
				@BaseDefense, \
				@Defense, \
				@Defenseflat, \
				@DefenseMental, \
				@BaseAttack, \
				@AttackMelee, \
				@AttackRanged, \
				@AttackMental, \
				@MaxLifeLoad, \
				@HeroPoints, \
				@PerceptiveRadius, \
				@SentientRadius, \
				@GroupID, \
				@SentientField, \
				@OnLoadScript,\
				@CustomAppearance)\
			");
	}
	if (m_pProcInsertNPC && m_pProcInsertNPC->IsValid())
	{
		CICRecordSet* rs = m_pProcInsertNPC;
		
		try
		{
			rs->DataBinding("@Name", npc.m_sName.c_str());
			rs->DataBinding("@AssetName", npc.m_sAssetName.c_str());
			rs->DataBinding("@IsGlobal", (int)npc.m_bIsGlobal);
			rs->DataBinding("@SnapToTerrain", (int)npc.m_bSnapToTerrain);
			rs->DataBinding("@Radius", (double)npc.m_fRadius);
			rs->DataBinding("@Facing", (double)npc.m_fFacing);
			rs->DataBinding("@Scaling", (double)npc.m_fScaling);
			rs->DataBinding("@posX", (double)npc.m_vPos.x);
			rs->DataBinding("@posY", (double)npc.m_vPos.y);
			rs->DataBinding("@posZ", (double)npc.m_vPos.z);
			rs->DataBinding("@CharacterType", (int)npc.m_nCharacterType);
			rs->DataBinding("@MentalState0", (int)npc.m_MentalState[0]);
			rs->DataBinding("@MentalState1", (int)npc.m_MentalState[1]);
			rs->DataBinding("@MentalState2", (int)npc.m_MentalState[2]);
			rs->DataBinding("@MentalState3", (int)npc.m_MentalState[3]);
			rs->DataBinding("@LifePoint", (double)npc.m_fLifePoint);
			rs->DataBinding("@Age", (double)npc.m_fAge);
			rs->DataBinding("@Height", (double)npc.m_fHeight);
			rs->DataBinding("@Weight", (double)npc.m_fWeight);
			rs->DataBinding("@Occupation", (int)npc.m_nOccupation);
			rs->DataBinding("@RaceSex", (int)npc.m_nRaceSex);
			rs->DataBinding("@Strength", (double)npc.m_fStrength);
			rs->DataBinding("@Dexterity", (double)npc.m_fDexterity);
			rs->DataBinding("@Intelligence", (double)npc.m_fIntelligence);
			rs->DataBinding("@BaseDefense", (double)npc.m_fBaseDefense);
			rs->DataBinding("@Defense", (double)npc.m_fDefense);
			rs->DataBinding("@Defenseflat", (double)npc.m_fDefenseflat);
			rs->DataBinding("@DefenseMental", (double)npc.m_fDefenseMental);
			rs->DataBinding("@BaseAttack", (double)npc.m_fBaseAttack);
			rs->DataBinding("@AttackMelee", (double)npc.m_fAttackMelee);
			rs->DataBinding("@AttackRanged", (double)npc.m_fAttackRanged);
			rs->DataBinding("@AttackMental", (double)npc.m_fAttackMental);
			rs->DataBinding("@MaxLifeLoad", (double)npc.m_fMaxLifeLoad);
			rs->DataBinding("@HeroPoints", (int)npc.m_nHeroPoints);
			rs->DataBinding("@PerceptiveRadius", (double)npc.m_fPerceptiveRadius);
			rs->DataBinding("@SentientRadius", (double)npc.m_fSentientRadius);
			rs->DataBinding("@GroupID", (double)npc.m_nGroupID);
			rs->DataBinding("@SentientField", (int)npc.m_dwSentientField);
			rs->DataBinding("@OnLoadScript", npc.m_sOnLoadScript.c_str());
			if(!npc.m_binCustomAppearance.empty())
				rs->DataBinding("@CustomAppearance", &npc.m_binCustomAppearance[0], (int)npc.m_binCustomAppearance.size());
			else
				rs->DataBinding("@CustomAppearance", 0,0);
			bSucceed = rs->Execute() == S_OK;
		}
		catch (CICSQLException& e)
		{
			OUTPUT_LOG("%s\n", e.errmsg.c_str());
		}
		rs->Reset();
	}
	if(bSucceed)
	{
		npc.m_nID = CNpcDatabase::GetNPCIDByName(npc.m_sName);
	}
	return bSucceed;
}

bool CNpcDatabase::SelectNPCByID(int nNPCID, CNpcDbItem& out)
{
	DBEntity* pDB = GetDBEntity();
	assert(pDB!=NULL);
	bool bSucceed = false;
	if(m_pProcSelectNPCByID == NULL)
	{
		// create the SQL procedure
		m_pProcSelectNPCByID = pDB->CreateRecordSet1("SELECT     [Name], AssetName, IsGlobal, Facing, SnapToTerrain, Radius, Scaling, posX, posY, posZ, CharacterType, MentalState0, MentalState1, MentalState2, \
			MentalState3, LifePoint, Age, Height, Weight, Occupation, RaceSex, Strength, Dexterity, Intelligence, BaseDefense, Defense, Defenseflat, \
			DefenseMental, BaseAttack, AttackMelee, AttackRanged, AttackMental, MaxLifeLoad, HeroPoints, PerceptiveRadius, SentientRadius, GroupID, \
			SentientField, OnLoadScript, CustomAppearance\
			FROM         NPC\
			WHERE     (ID = @ID) ");
	}
	if (m_pProcSelectNPCByID && m_pProcSelectNPCByID->IsValid())
	{
		CICRecordSet* rs = m_pProcSelectNPCByID;

		try
		{
			rs->DataBinding("@ID", nNPCID);

			bool bHasRecord = (rs->NextRow()==0);
			if(bHasRecord)
			{
				int nCount = rs->ColumnCount();

				out.m_nID = nNPCID;
				int i=0;
				CICRecordSetItem& item = (*rs)[0];
				out.m_sName = item;

				item = (*rs)[++i]; 
				out.m_sAssetName = item;

				item = (*rs)[++i]; 
				out.m_bIsGlobal = item;

				item = (*rs)[++i]; 
				out.m_fFacing = item;
				
				item = (*rs)[++i]; 
				out.m_bSnapToTerrain = item;

				item = (*rs)[++i]; 
				out.m_fRadius = item;

				item = (*rs)[++i]; 
				out.m_fScaling = item;

				item = (*rs)[++i]; 
				out.m_vPos.x = item;

				item = (*rs)[++i]; 
				out.m_vPos.y = item;

				item = (*rs)[++i]; 
				out.m_vPos.z = item;

				item = (*rs)[++i]; 
				out.m_nCharacterType = item;

				item = (*rs)[++i]; 
				out.m_MentalState[0] = item;

				item = (*rs)[++i]; 
				out.m_MentalState[1] = item;

				item = (*rs)[++i]; 
				out.m_MentalState[2] = item;

				item = (*rs)[++i]; 
				out.m_MentalState[3] = item;

				item = (*rs)[++i]; 
				out.m_fLifePoint = item;

				item = (*rs)[++i]; 
				out.m_fAge = item;

				item = (*rs)[++i]; 
				out.m_fHeight = item;

				item = (*rs)[++i]; 
				out.m_fWeight = item;

				item = (*rs)[++i]; 
				out.m_nOccupation = item;

				item = (*rs)[++i]; 
				out.m_nRaceSex = item;

				item = (*rs)[++i]; 
				out.m_fStrength = item;

				item = (*rs)[++i]; 
				out.m_fDexterity = item;

				item = (*rs)[++i]; 
				out.m_fIntelligence = item;

				item = (*rs)[++i]; 
				out.m_fBaseDefense = item;

				item = (*rs)[++i]; 
				out.m_fDefense = item;

				item = (*rs)[++i]; 
				out.m_fDefenseflat = item;

				item = (*rs)[++i]; 
				out.m_fDefenseMental = item;

				item = (*rs)[++i]; 
				out.m_fBaseAttack = item;

				item = (*rs)[++i]; 
				out.m_fAttackMelee = item;

				item = (*rs)[++i]; 
				out.m_fAttackRanged = item;

				item = (*rs)[++i]; 
				out.m_fAttackMental = item;				

				item = (*rs)[++i]; 
				out.m_fMaxLifeLoad = item;

				item = (*rs)[++i]; 
				out.m_nHeroPoints = item;

				item = (*rs)[++i]; 
				out.m_fPerceptiveRadius = item;

				item = (*rs)[++i]; 
				out.m_fSentientRadius = item;

				item = (*rs)[++i]; 
				out.m_nGroupID = item;

				item = (*rs)[++i]; 
				out.m_dwSentientField = item;

				item = (*rs)[++i]; 
				out.m_sOnLoadScript = item;

				item = (*rs)[++i]; 
				void* pData = item;
				int nLength = item.GetDataLength();
				if(nLength > 0 )
				{
					out.m_binCustomAppearance.resize(nLength, 0);
					memcpy(&out.m_binCustomAppearance[0], pData, nLength);
				}
				else
					out.m_binCustomAppearance.clear();
				
				bSucceed = true;
			}			
		}
		catch (CICSQLException& e)
		{
			OUTPUT_LOG("%s\n", e.errmsg.c_str());
		}
		rs->Reset();
	}
	return bSucceed;
}

bool CNpcDatabase::SelectNPCByName(const string& sName, CNpcDbItem& out)
{
	DBEntity* pDB = GetDBEntity();
	assert(pDB!=NULL);
	bool bSucceed = false;
	if(m_pProcSelectNPCByName == NULL)
	{
		// create the SQL procedure
		m_pProcSelectNPCByName = pDB->CreateRecordSet1("SELECT     [ID], AssetName, IsGlobal, Facing, SnapToTerrain, Radius, Scaling, posX, posY, posZ, CharacterType, MentalState0, MentalState1, MentalState2, \
			MentalState3, LifePoint, Age, Height, Weight, Occupation, RaceSex, Strength, Dexterity, Intelligence, BaseDefense, Defense, Defenseflat, \
			DefenseMental, BaseAttack, AttackMelee, AttackRanged, AttackMental, MaxLifeLoad, HeroPoints, PerceptiveRadius, SentientRadius, GroupID, \
			SentientField, OnLoadScript, CustomAppearance\
			FROM         NPC\
			WHERE     ([Name] = @Name) ");
	}
	if (m_pProcSelectNPCByName && m_pProcSelectNPCByName->IsValid())
	{
		CICRecordSet* rs = m_pProcSelectNPCByName;

		try
		{
			rs->DataBinding("@Name", sName.c_str());

			bool bHasRecord = (rs->NextRow()==0);
			if(bHasRecord)
			{
				int nCount = rs->ColumnCount();
				
				out.m_sName = sName;
				int i=0;
				CICRecordSetItem& item = (*rs)[0];
				out.m_nID = item;

				item = (*rs)[++i]; 
				out.m_sAssetName = item;

				item = (*rs)[++i]; 
				out.m_bIsGlobal = item;

				item = (*rs)[++i]; 
				out.m_fFacing = item;
				
				item = (*rs)[++i]; 
				out.m_bSnapToTerrain = item;

				item = (*rs)[++i]; 
				out.m_fRadius = item;

				item = (*rs)[++i]; 
				out.m_fScaling = item;

				item = (*rs)[++i]; 
				out.m_vPos.x = item;

				item = (*rs)[++i]; 
				out.m_vPos.y = item;

				item = (*rs)[++i]; 
				out.m_vPos.z = item;

				item = (*rs)[++i]; 
				out.m_nCharacterType = item;

				item = (*rs)[++i]; 
				out.m_MentalState[0] = item;

				item = (*rs)[++i]; 
				out.m_MentalState[1] = item;

				item = (*rs)[++i]; 
				out.m_MentalState[2] = item;

				item = (*rs)[++i]; 
				out.m_MentalState[3] = item;

				item = (*rs)[++i]; 
				out.m_fLifePoint = item;

				item = (*rs)[++i]; 
				out.m_fAge = item;

				item = (*rs)[++i]; 
				out.m_fHeight = item;

				item = (*rs)[++i]; 
				out.m_fWeight = item;

				item = (*rs)[++i]; 
				out.m_nOccupation = item;

				item = (*rs)[++i]; 
				out.m_nRaceSex = item;

				item = (*rs)[++i]; 
				out.m_fStrength = item;

				item = (*rs)[++i]; 
				out.m_fDexterity = item;

				item = (*rs)[++i]; 
				out.m_fIntelligence = item;

				item = (*rs)[++i]; 
				out.m_fBaseDefense = item;

				item = (*rs)[++i]; 
				out.m_fDefense = item;

				item = (*rs)[++i]; 
				out.m_fDefenseflat = item;

				item = (*rs)[++i]; 
				out.m_fDefenseMental = item;

				item = (*rs)[++i]; 
				out.m_fBaseAttack = item;

				item = (*rs)[++i]; 
				out.m_fAttackMelee = item;

				item = (*rs)[++i]; 
				out.m_fAttackRanged = item;

				item = (*rs)[++i]; 
				out.m_fAttackMental = item;				

				item = (*rs)[++i]; 
				out.m_fMaxLifeLoad = item;

				item = (*rs)[++i]; 
				out.m_nHeroPoints = item;

				item = (*rs)[++i]; 
				out.m_fPerceptiveRadius = item;

				item = (*rs)[++i]; 
				out.m_fSentientRadius = item;

				item = (*rs)[++i]; 
				out.m_nGroupID = item;

				item = (*rs)[++i]; 
				out.m_dwSentientField = item;

				item = (*rs)[++i]; 
				out.m_sOnLoadScript = item;

				item = (*rs)[++i]; 
				void* pData = item;
				int nLength = item.GetDataLength();
				if(nLength > 0 )
				{
					out.m_binCustomAppearance.resize(nLength, 0);
					memcpy(&out.m_binCustomAppearance[0], pData, nLength);
				}
				else
					out.m_binCustomAppearance.clear();

				bSucceed = true;
			}
		}
		catch (CICSQLException& e)
		{
			OUTPUT_LOG("%s\n", e.errmsg.c_str());
		}
		rs->Reset();
	}
	return bSucceed;
}

int CNpcDatabase::GetNPCIDByName(const string& name)
{
	DBEntity* pDB = GetDBEntity();
	assert(pDB!=NULL);
	int nID = -1;

	if(m_pProcGetNPCIDByName == NULL)
	{
		// create the SQL procedure
		m_pProcGetNPCIDByName = pDB->CreateRecordSet1("SELECT     [ID] FROM         NPC WHERE     ([Name] = @Name)");
	}
	if (m_pProcGetNPCIDByName && m_pProcGetNPCIDByName->IsValid())
	{
		CICRecordSet* rs = m_pProcGetNPCIDByName;
		try
		{
			rs->DataBinding("@Name", name.c_str());
			bool bHasRecord = (rs->NextRow()==0);
			if(bHasRecord)
			{
				nID = (*rs)[0];
			}
			
		}
		catch (CICSQLException& e)
		{
			OUTPUT_LOG("%s\n", e.errmsg.c_str());
		}
		rs->Reset();
	}
	return nID;
}

int CNpcDatabase::GetNPCCount()
{
	DBEntity* pDB = GetDBEntity();
	if(pDB==NULL) 
		return 0;
	CICRecordSet* rs =  pDB->CreateRecordSet("SELECT     count(*) FROM         NPC");

	int nCount = 0;
	bool bHasRecord = (rs->NextRow()==0);
	if(bHasRecord)
	{
		nCount = (*rs)[0];
	}
	rs->Release();
	return nCount;
}

bool CNpcDatabase::DeleteNPCByID(int nNPCID)
{
	DBEntity* pDB = GetDBEntity();
	assert(pDB!=NULL);
	bool bSucceed = false;

	if(m_pProcDeleteNPCByID == NULL)
	{
		// create the SQL procedure
		m_pProcDeleteNPCByID = pDB->CreateRecordSet1("DELETE FROM NPC	WHERE     ([ID] = @ID)");
	}
	if (m_pProcDeleteNPCByID && m_pProcDeleteNPCByID->IsValid())
	{
		CICRecordSet* rs = m_pProcDeleteNPCByID;

		try
		{
			rs->DataBinding("@ID", nNPCID);
			bSucceed = rs->Execute() == S_OK;
			
		}
		catch (CICSQLException& e)
		{
			OUTPUT_LOG("%s\n", e.errmsg.c_str());
		}
		rs->Reset();
	}
	return bSucceed;
}

bool CNpcDatabase::DeleteNPCByName(const string& sName)
{
	DBEntity* pDB = GetDBEntity();
	assert(pDB!=NULL);
	bool bSucceed = false;

	if(m_pProcSelectNPCByName == NULL)
	{
		// create the SQL procedure
		m_pProcSelectNPCByName = pDB->CreateRecordSet1("DELETE FROM NPC	WHERE     ([Name] = @Name)");
	}
	if (m_pProcSelectNPCByName && m_pProcSelectNPCByName->IsValid())
	{
		CICRecordSet* rs = m_pProcSelectNPCByName;

		try
		{
			rs->DataBinding("@Name", sName.c_str());
			bSucceed = rs->Execute() == S_OK;
		}
		catch (CICSQLException& e)
		{
			OUTPUT_LOG("%s\n", e.errmsg.c_str());
		}
		rs->Reset();
	}
	return bSucceed;
}

bool CNpcDatabase::SelectNPCListByRegion(NPCDBItemList* pOut, const Vector3& vMin, const Vector3& vMax)
{
	DBEntity* pDB = GetDBEntity();
	assert(pDB!=NULL);
	bool bSucceed = false;
	if(m_pProcSelectNPCByID == NULL)
	{
		// create the SQL procedure
		m_pProcSelectNPCByID = pDB->CreateRecordSet1("SELECT     [ID], [Name], AssetName, IsGlobal, Facing, SnapToTerrain, Radius, Scaling, posX, posY, posZ, CharacterType, MentalState0, MentalState1, MentalState2, \
													 MentalState3, LifePoint, Age, Height, Weight, Occupation, RaceSex, Strength, Dexterity, Intelligence, BaseDefense, Defense, Defenseflat, \
													 DefenseMental, BaseAttack, AttackMelee, AttackRanged, AttackMental, MaxLifeLoad, HeroPoints, PerceptiveRadius, SentientRadius, GroupID, \
													 SentientField, OnLoadScript, CustomAppearance\
													 FROM         NPC\
													 WHERE     (posX >= @MinX) AND (posZ >= @MinZ) AND (posX < @MaxX) AND (posZ < @MaxZ)");
	}
	if (m_pProcSelectNPCByID && m_pProcSelectNPCByID->IsValid())
	{
		CICRecordSet* rs = m_pProcSelectNPCByID;

		try
		{
			rs->DataBinding("@MinX", vMin.x);
			rs->DataBinding("@MinZ", vMin.z);
			rs->DataBinding("@MaxX", vMax.x);
			rs->DataBinding("@MaxZ", vMax.z);

			while(rs->NextRow()==0)
			{
				pOut->push_back(CNpcDbItem());
				CNpcDbItem& out = *(pOut->end()-1);
				int nCount = rs->ColumnCount();
				
				int i=0;
				CICRecordSetItem& item = (*rs)[0];
				out.m_nID = item;

				item = (*rs)[++i];
				out.m_sName = item;

				item = (*rs)[++i]; 
				out.m_sAssetName = item;

				item = (*rs)[++i]; 
				out.m_bIsGlobal = item;

				item = (*rs)[++i]; 
				out.m_fFacing = item;

				item = (*rs)[++i]; 
				out.m_bSnapToTerrain = item;

				item = (*rs)[++i]; 
				out.m_fRadius = item;

				item = (*rs)[++i]; 
				out.m_fScaling = item;

				item = (*rs)[++i]; 
				out.m_vPos.x = item;

				item = (*rs)[++i]; 
				out.m_vPos.y = item;

				item = (*rs)[++i]; 
				out.m_vPos.z = item;

				item = (*rs)[++i]; 
				out.m_nCharacterType = item;

				item = (*rs)[++i]; 
				out.m_MentalState[0] = item;

				item = (*rs)[++i]; 
				out.m_MentalState[1] = item;

				item = (*rs)[++i]; 
				out.m_MentalState[2] = item;

				item = (*rs)[++i]; 
				out.m_MentalState[3] = item;

				item = (*rs)[++i]; 
				out.m_fLifePoint = item;

				item = (*rs)[++i]; 
				out.m_fAge = item;

				item = (*rs)[++i]; 
				out.m_fHeight = item;

				item = (*rs)[++i]; 
				out.m_fWeight = item;

				item = (*rs)[++i]; 
				out.m_nOccupation = item;

				item = (*rs)[++i]; 
				out.m_nRaceSex = item;

				item = (*rs)[++i]; 
				out.m_fStrength = item;

				item = (*rs)[++i]; 
				out.m_fDexterity = item;

				item = (*rs)[++i]; 
				out.m_fIntelligence = item;

				item = (*rs)[++i]; 
				out.m_fBaseDefense = item;

				item = (*rs)[++i]; 
				out.m_fDefense = item;

				item = (*rs)[++i]; 
				out.m_fDefenseflat = item;

				item = (*rs)[++i]; 
				out.m_fDefenseMental = item;

				item = (*rs)[++i]; 
				out.m_fBaseAttack = item;

				item = (*rs)[++i]; 
				out.m_fAttackMelee = item;

				item = (*rs)[++i]; 
				out.m_fAttackRanged = item;

				item = (*rs)[++i]; 
				out.m_fAttackMental = item;				

				item = (*rs)[++i]; 
				out.m_fMaxLifeLoad = item;

				item = (*rs)[++i]; 
				out.m_nHeroPoints = item;

				item = (*rs)[++i]; 
				out.m_fPerceptiveRadius = item;

				item = (*rs)[++i]; 
				out.m_fSentientRadius = item;

				item = (*rs)[++i]; 
				out.m_nGroupID = item;

				item = (*rs)[++i]; 
				out.m_dwSentientField = item;

				item = (*rs)[++i]; 
				out.m_sOnLoadScript = item;

				item = (*rs)[++i]; 
				void* pData = item;
				int nLength = item.GetDataLength();
				if(nLength > 0 )
				{
					out.m_binCustomAppearance.resize(nLength, 0);
					memcpy(&out.m_binCustomAppearance[0], pData, nLength);
				}
				else
					out.m_binCustomAppearance.clear();
			}
			bSucceed = true;			
		}
		catch (CICSQLException& e)
		{
			OUTPUT_LOG("%s\n", e.errmsg.c_str());
		}
		rs->Reset();
	}
	return bSucceed;
}

bool CNpcDatabase::UpdateNPC(const CNpcDbItem& npc, DWORD dwFields)
{
	DBEntity* pDB = GetDBEntity();
	assert(pDB!=NULL);
	bool bSucceed = false;
	if(dwFields == CNpcDbItem::ALL_ATTRIBUTES)
	{
		if(m_pProcUpdateNPC == NULL)
		{
			// create the SQL procedure
			m_pProcUpdateNPC = pDB->CreateRecordSet1("UPDATE    NPC\
					SET     Name = @Name, AssetName = @AssetName, IsGlobal = @IsGlobal, SnapToTerrain = @SnapToTerrain, Radius = @Radius, Facing = @Facing, Scaling = @Scaling, posX = @posX, posY = @posY, posZ = @posZ, CharacterType = @CharacterType, MentalState0 = @MentalState0, \
					MentalState1 = @MentalState1, MentalState2 = @MentalState2, MentalState3 = @MentalState3, LifePoint = @LifePoint, Age = @Age, Height = @Height, Weight = @Weight, Occupation = @Occupation, RaceSex = @RaceSex, Strength = @Strength, Dexterity = @Dexterity, Intelligence = @Intelligence, \
					BaseDefense = @BaseDefense, Defense = @Defense, Defenseflat = @Defenseflat, DefenseMental = @DefenseMental, BaseAttack = @BaseAttack, AttackMelee = @AttackMelee, AttackRanged = @AttackRanged, AttackMental = @AttackMental, MaxLifeLoad = @MaxLifeLoad, \
					HeroPoints = @HeroPoints, PerceptiveRadius = @PerceptiveRadius, SentientRadius = @SentientRadius, GroupID = @GroupID, SentientField = @SentientField, OnLoadScript = @OnLoadScript, CustomAppearance = @CustomAppearance\
			WHERE     ([ID] = @ID) OR ([Name] = @Name)");
		}
		if (m_pProcUpdateNPC && m_pProcUpdateNPC->IsValid())
		{
			CICRecordSet* rs = m_pProcUpdateNPC;
			try
			{
				rs->DataBinding("@ID", npc.m_nID);
				rs->DataBinding("@Name", npc.m_sName.c_str());
				rs->DataBinding("@AssetName", npc.m_sAssetName.c_str());
				rs->DataBinding("@IsGlobal", (int)npc.m_bIsGlobal);
				rs->DataBinding("@SnapToTerrain", (int)npc.m_bSnapToTerrain);
				rs->DataBinding("@Radius", (double)npc.m_fRadius);
				rs->DataBinding("@Facing", (double)npc.m_fFacing);
				rs->DataBinding("@Scaling", (double)npc.m_fScaling);
				rs->DataBinding("@posX", (double)npc.m_vPos.x);
				rs->DataBinding("@posY", (double)npc.m_vPos.y);
				rs->DataBinding("@posZ", (double)npc.m_vPos.z);
				rs->DataBinding("@CharacterType", (int)npc.m_nCharacterType);
				rs->DataBinding("@MentalState0", (int)npc.m_MentalState[0]);
				rs->DataBinding("@MentalState1", (int)npc.m_MentalState[1]);
				rs->DataBinding("@MentalState2", (int)npc.m_MentalState[2]);
				rs->DataBinding("@MentalState3", (int)npc.m_MentalState[3]);
				rs->DataBinding("@LifePoint", (double)npc.m_fLifePoint);
				rs->DataBinding("@Age", (double)npc.m_fAge);
				rs->DataBinding("@Height", (double)npc.m_fHeight);
				rs->DataBinding("@Weight", (double)npc.m_fWeight);
				rs->DataBinding("@Occupation", (int)npc.m_nOccupation);
				rs->DataBinding("@RaceSex", (int)npc.m_nRaceSex);
				rs->DataBinding("@Strength", (double)npc.m_fStrength);
				rs->DataBinding("@Dexterity", (double)npc.m_fDexterity);
				rs->DataBinding("@Intelligence", (double)npc.m_fIntelligence);
				rs->DataBinding("@BaseDefense", (double)npc.m_fBaseDefense);
				rs->DataBinding("@Defense", (double)npc.m_fDefense);
				rs->DataBinding("@Defenseflat", (double)npc.m_fDefenseflat);
				rs->DataBinding("@DefenseMental", (double)npc.m_fDefenseMental);
				rs->DataBinding("@BaseAttack", (double)npc.m_fBaseAttack);
				rs->DataBinding("@AttackMelee", (double)npc.m_fAttackMelee);
				rs->DataBinding("@AttackRanged", (double)npc.m_fAttackRanged);
				rs->DataBinding("@AttackMental", (double)npc.m_fAttackMental);
				rs->DataBinding("@MaxLifeLoad", (double)npc.m_fMaxLifeLoad);
				rs->DataBinding("@HeroPoints", (int)npc.m_nHeroPoints);
				rs->DataBinding("@PerceptiveRadius", (double)npc.m_fPerceptiveRadius);
				rs->DataBinding("@SentientRadius", (double)npc.m_fSentientRadius);
				rs->DataBinding("@GroupID", (double)npc.m_nGroupID);
				rs->DataBinding("@SentientField", (int)npc.m_dwSentientField);
				rs->DataBinding("@OnLoadScript", npc.m_sOnLoadScript.c_str());
				if(!npc.m_binCustomAppearance.empty())
					rs->DataBinding("@CustomAppearance", &npc.m_binCustomAppearance[0], (int)npc.m_binCustomAppearance.size());
				else
					rs->DataBinding("@CustomAppearance", 0,0);
				bSucceed = rs->Execute() == S_OK;
			}
			catch (CICSQLException& e)
			{
				OUTPUT_LOG("%s\n", e.errmsg.c_str());
			}
			rs->Reset();

		}
	}
	else
	{
		// TODO: for sub set update
	}
	
	return bSucceed;	
}
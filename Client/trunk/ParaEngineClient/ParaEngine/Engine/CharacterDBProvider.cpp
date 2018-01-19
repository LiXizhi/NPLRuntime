//-----------------------------------------------------------------------------
// Class: CCharacterDBProvider
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "InfoCenter/ICDBManager.h"
#include "InfoCenter/ICRecordSet.h"

#include "CharacterDBProvider.h"

using namespace ParaInfoCenter;
using namespace ParaEngine;

CCharacterDBProvider::CCharacterDBProvider(void)
{
}

CCharacterDBProvider::CCharacterDBProvider(ParaInfoCenter::DBEntity* pDb)
{
	m_pDataBase = pDb;
}

CCharacterDBProvider::~CCharacterDBProvider(void)
{
}

#ifdef TEST_CHARACTER
void CCharacterDBProvider::TestDB()
{
	//bool returnValue = false;
	//string str;
	//stItemDbRecord record;

	//record.IconAssetName = "Building";
	//record.DescTextAssetName = "BuildingDesc";
	//record.IsGlobal = false;
	//record.IconFilePath = "Building.bmp";
	//record.ModelFilePath = "Building.x";
	//record.ItemType = 5;
	//record.IsObtained = true;
	//record.IsUnique = false;
	//record.CostExperiencePt = 123;
	//record.CostOrange = 10;
	//record.CostApple = 20;
	//record.CostPeach = 30;
	//record.ItemName.StringEnglish = "BuildingEN";
	//record.ItemName.StringSimplifiedChinese = "BuildingCN";
	//record.Desc.StringEnglish = "DescEN";
	//record.Desc.StringSimplifiedChinese = "DescCN";
	//record.Reserved1 = "R1";
	//record.Reserved2 = "R2";
	//record.Reserved3 = "R3";
	//record.Reserved4 = "R4";
	//record.Reserved5 = "R5";

	//returnValue = InsertItemRecord(record);
	//returnValue = DeleteItemRecordByID(1);
}
#endif


bool CCharacterDBProvider::InsertCharacterRecord(stCharacterDbRecord& record)
{
	DBEntity* pDB = GetDBEntity();
	assert(pDB!=NULL);
	bool bSucceed = false;
	CICRecordSet* rs = NULL;

	try
	{
		CStringTableDB::GetInstance().InsertString(record.CharacterName);

		__int64 CharacterNameStringIndex = record.CharacterName.ID;

		rs = pDB->CreateRecordSet1("INSERT INTO Character_DB \
								   ([ID], CharacterNameStringIndex, \
								   NumberOfPets, MainPetIndex, SecondaryPetIndex, ThirdPetIndex, \
								   AssetName, \
								   IsGlobal, \
								   IsSnapToTerrain, \
								   CharacterModeFilePath, CharacterModelScale, CharacterRadius, CharacterFacing, \
								   posX, posY, posZ, \
								   PerceptiveRadius, SentientRadius, OnLoadScript, \
								   CharacterTitleIndex, ExperiencePt, Orange, Apple, Peach, \
								   Reserved1, Reserved2, Reserved3, Reserved4, Reserved5)\
								   VALUES \
								   (NULL, @CharacterNameStringIndex, \
								   @NumberOfPets, @MainPetIndex, @SecondaryPetIndex, @ThirdPetIndex, \
								   @AssetName, \
								   @IsGlobal, \
								   @IsSnapToTerrain, \
								   @CharacterModeFilePath, @CharacterModelScale, @CharacterRadius, @CharacterFacing, \
								   @posX, @posY, @posZ, \
								   @PerceptiveRadius, @SentientRadius, @OnLoadScript, \
								   @CharacterTitleIndex, @ExperiencePt, @Orange, @Apple, @Peach, \
								   @Reserved1, @Reserved2, @Reserved3, @Reserved4, @Reserved5)");

		rs->DataBinding("@CharacterNameStringIndex", CharacterNameStringIndex);
		rs->DataBinding("@NumberOfPets", record.NumberOfPets);
		rs->DataBinding("@MainPetIndex", record.MainPetIndex);
		rs->DataBinding("@SecondaryPetIndex", record.SecondaryPetIndex);
		rs->DataBinding("@ThirdPetIndex", record.ThirdPetIndex);
		rs->DataBinding("@AssetName", record.AssetName.c_str());
		rs->DataBinding("@IsGlobal", record.IsGlobal);
		rs->DataBinding("@IsSnapToTerrain", record.IsSnapToTerrain);
		rs->DataBinding("@CharacterModeFilePath", record.CharacterModeFilePath.c_str());
		rs->DataBinding("@CharacterModelScale", record.CharacterModelScale);
		rs->DataBinding("@CharacterRadius", record.CharacterRadius);
		rs->DataBinding("@CharacterFacing", record.CharacterFacing);
		rs->DataBinding("@posX", record.posX);
		rs->DataBinding("@posY", record.posY);
		rs->DataBinding("@posZ", record.posZ);
		rs->DataBinding("@PerceptiveRadius", record.PerceptiveRadius);
		rs->DataBinding("@SentientRadius", record.SentientRadius);
		rs->DataBinding("@OnLoadScript", record.OnLoadScript.c_str());
		rs->DataBinding("@CharacterTitleIndex", record.CharacterTitleIndex);
		rs->DataBinding("@ExperiencePt", record.ExperiencePt);
		rs->DataBinding("@Orange", record.Orange);
		rs->DataBinding("@Apple", record.Apple);
		rs->DataBinding("@Peach", record.Peach);
		rs->DataBinding("@Reserved1", record.Reserved1.c_str());
		rs->DataBinding("@Reserved2", record.Reserved2.c_str());
		rs->DataBinding("@Reserved3", record.Reserved3.c_str());
		rs->DataBinding("@Reserved4", record.Reserved4.c_str());
		rs->DataBinding("@Reserved5", record.Reserved5.c_str());
		bSucceed = rs->Execute() == S_OK;
	}
	catch (CICSQLException& e)
	{
		OUTPUT_LOG("%s\n", e.errmsg.c_str());
	}

	if(bSucceed)
	{
		// SQLite FAQ:
		// NOTE: If you declare a column of a table to be INTEGER PRIMARY KEY, 
		//		then whenever you insert a NULL into that column of the table, 
		//		NULL is automatically converted into an integer which is one greater 
		//		than the largest value of that column over all other rows in the table, 
		//		or 1 if the table is empty.
		//		(If the largest possible integer key, 9223372036854775807, then an 
		//		unused key value is chosen at random.)

		record.ID = rs->GetLastInsertRowID();
	}
	rs->Release();

	return bSucceed;
}

bool CCharacterDBProvider::DeleteCharacterRecordByID(int ID)
{
	// TODO: Unfinished
	DBEntity* pDB = GetDBEntity();
	assert(pDB!=NULL);
	bool bSucceed = false;
	CICRecordSet* rs = NULL;

	__int64 CharacterNameStringIndex = 0;

	try
	{
		// select the target puzzle record in the Character_DB
		rs = pDB->CreateRecordSet1("SELECT     CharacterNameStringIndex \
								   FROM		Character_DB\
								   WHERE	(ID = @ID) ");
		rs->DataBinding("@ID", ID);

		bool bHasRecord = (rs->NextRow()==0);
		if(bHasRecord)
		{
			int nCount = rs->ColumnCount();
			int i = 0;

			// find the index into the string table
			CICRecordSetItem& item = (*rs)[0];
			CharacterNameStringIndex = item;
		}
		else
		{
			// No record found in Character_DB
			return false;
		}
		rs->Reset();

		// delete string table records
		CStringTableDB::GetInstance().DeleteEntryByID(CharacterNameStringIndex);

		// delete puzzle database records
		rs = pDB->CreateRecordSet1("DELETE	FROM		Character_DB	WHERE	([ID] = @ID)");
		rs->DataBinding("@ID", ID);
		bSucceed = (rs->Execute() == S_OK);
	}
	catch (CICSQLException& e)
	{
		OUTPUT_LOG("%s", e.errmsg.c_str());
	}
	rs->Release();
	return bSucceed;
}


CCharacterDBProvider& CCharacterDBProvider::GetInstance()
{
	static CCharacterDBProvider g_singletonCharacterDB;
	return g_singletonCharacterDB;
}

ParaInfoCenter::DBEntity* CCharacterDBProvider::GetDBEntity()
{
	return m_pDataBase.get();
}

void CCharacterDBProvider::SetDBEntity(ParaInfoCenter::DBEntity* pDb)
{
	m_pDataBase = pDb;
}
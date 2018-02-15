//-----------------------------------------------------------------------------
// Class: CCharacterDBProvider
// Authors:	LiXizhi, Andy
// Emails:	LiXizhi@yeah.net
// Date:	
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "InfoCenter/ICDBManager.h"
#include "InfoCenter/ICRecordSet.h"

#include "ChestDBProvider.h"

using namespace ParaInfoCenter;
using namespace ParaEngine;

CChestDBProvider::CChestDBProvider(void)
{
}

CChestDBProvider::CChestDBProvider(ParaInfoCenter::DBEntity* pDb)
{
	m_pDataBase = pDb;
}

CChestDBProvider::~CChestDBProvider(void)
{
}

#ifdef TEST_CHEST
void CChestDBProvider::TestDB()
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


bool CChestDBProvider::InsertChestRecord(stChestDbRecord& record)
{
	DBEntity* pDB = GetDBEntity();
	assert(pDB!=NULL);
	bool bSucceed = false;
	CICRecordSet* rs = NULL;

	try
	{
		CStringTableDB::GetInstance().InsertString(record.ChestName);

		__int64 ChestNameStringIndex = record.ChestName.ID;

		rs = pDB->CreateRecordSet1("INSERT INTO Chest_DB \
								   ([ID], ChestNameStringIndex, \
								   AssetName, IsSnapToTerrain, \
								   ChestModeFilePath, ChestModelScale, ChestRadius, ChestFacing, \
								   posX, posY, posZ, \
								   DifficultyRate, NumberOfAllPuzzles, NumberOfPassPuzzles, \
								   Reward, \
								   Reserved1, Reserved2, Reserved3, Reserved4, Reserved5)\
								   VALUES \
								   (NULL, @ChestNameStringIndex, \
								   @AssetName, @IsSnapToTerrain, \
								   @ChestModeFilePath, @ChestModelScale, @ChestRadius, @ChestFacing, \
								   @posX, @posY, @posZ, \
								   @DifficultyRate, @NumberOfAllPuzzles, @NumberOfPassPuzzles, \
								   @Reward, \
								   @Reserved1, @Reserved2, @Reserved3, @Reserved4, @Reserved5)");

		rs->DataBinding("@ChestNameStringIndex", ChestNameStringIndex);
		rs->DataBinding("@AssetName", record.AssetName.c_str());
		rs->DataBinding("@IsSnapToTerrain", record.IsSnapToTerrain);
		rs->DataBinding("@ChestModeFilePath", record.ChestModeFilePath.c_str());
		rs->DataBinding("@ChestModelScale", record.ChestModelScale);
		rs->DataBinding("@ChestRadius", record.ChestRadius);
		rs->DataBinding("@ChestFacing", record.ChestFacing);
		rs->DataBinding("@posX", record.posX);
		rs->DataBinding("@posY", record.posY);
		rs->DataBinding("@posZ", record.posZ);
		rs->DataBinding("@DifficultyRate", record.DifficultyRate);
		rs->DataBinding("@NumberOfAllPuzzles", record.NumberOfAllPuzzles);
		rs->DataBinding("@NumberOfPassPuzzles", record.NumberOfPassPuzzles);
		rs->DataBinding("@Reward", record.Reward.c_str());
		rs->DataBinding("@Reserved1", record.Reserved1.c_str());
		rs->DataBinding("@Reserved2", record.Reserved2.c_str());
		rs->DataBinding("@Reserved3", record.Reserved3.c_str());
		rs->DataBinding("@Reserved4", record.Reserved4.c_str());
		rs->DataBinding("@Reserved5", record.Reserved5.c_str());
		bSucceed = rs->Execute() == S_OK;
	}
	catch (CICSQLException& e)
	{
		OUTPUT_LOG("%s", e.errmsg.c_str());
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

bool CChestDBProvider::DeleteChestRecordByID(int ID)
{
	DBEntity* pDB = GetDBEntity();
	assert(pDB!=NULL);
	bool bSucceed = false;
	CICRecordSet* rs = NULL;

	__int64 ChestNameStringIndex = 0;

	try
	{
		// select the target puzzle record in the Chest_DB
		rs = pDB->CreateRecordSet1("SELECT     ChestNameStringIndex \
								   FROM		Chest_DB\
								   WHERE	(ID = @ID) ");
		rs->DataBinding("@ID", ID);

		bool bHasRecord = (rs->NextRow()==0);
		if(bHasRecord)
		{
			int nCount = rs->ColumnCount();
			int i = 0;

			// find the index into the string table
			CICRecordSetItem& item = (*rs)[0];
			ChestNameStringIndex = item;
		}
		else
		{
			// No record found in Chest_DB
			return false;
		}
		rs->Reset();

		// delete string table records
		CStringTableDB::GetInstance().DeleteEntryByID(ChestNameStringIndex);

		// delete puzzle database records
		rs = pDB->CreateRecordSet1("DELETE	FROM		Chest_DB	WHERE	([ID] = @ID)");
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


CChestDBProvider& CChestDBProvider::GetInstance()
{
	static CChestDBProvider g_singletonChestDB;
	return g_singletonChestDB;
}

ParaInfoCenter::DBEntity* CChestDBProvider::GetDBEntity()
{
	return m_pDataBase.get();
}

void CChestDBProvider::SetDBEntity(ParaInfoCenter::DBEntity* pDb)
{
	m_pDataBase = pDb;
}
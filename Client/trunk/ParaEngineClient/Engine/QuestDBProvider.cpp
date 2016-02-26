//-----------------------------------------------------------------------------
// Class: CQuestDBProvider
// Authors:	Andy, LiXizhi
// Emails:	LiXizhi@yeah.net
// Date:	
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ic/ICDBManager.h"
#include "ic/ICRecordSet.h"

#include "QuestDBProvider.h"

using namespace ParaInfoCenter;
using namespace ParaEngine;

CQuestDBProvider::CQuestDBProvider(void)
{
}

CQuestDBProvider::CQuestDBProvider(ParaInfoCenter::DBEntity* pDb)
{
	m_pDataBase = pDb;
}

CQuestDBProvider::~CQuestDBProvider(void)
{
}

#ifdef TEST_QUEST
void CQuestDBProvider::TestDB()
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



bool CQuestDBProvider::InsertQuestRecord(stQuestDbRecord& record)
{
	DBEntity* pDB = GetDBEntity();
	assert(pDB!=NULL);
	bool bSucceed = false;
	CICRecordSet* rs = NULL;

	try
	{
		CStringTableDB::GetInstance().InsertString(record.QuestName);
		CStringTableDB::GetInstance().InsertString(record.QuestDesc);

		__int64 QuestNameStringIndex = record.QuestName.ID;
		__int64 QuestDescStringIndex = record.QuestDesc.ID;

		rs = pDB->CreateRecordSet1("INSERT INTO Quest_DB \
								   ([ID], QuestNameStringIndex, \
								   TopSnapShotAssetName, IntroVideoAssetName, IsGlobal, \
								   TopSnapShotFilePath, \
								   IntroVideoFilePath, \
								   IsComplete, \
								   QuestDescStringIndex, \
								   AcquireQuestRequirement, CompleteQuestRequirement, \
								   Reward, \
								   Reserved1, Reserved2, Reserved3, Reserved4, Reserved5)\
								   VALUES \
								   (NULL, @QuestNameStringIndex, \
								   @TopSnapShotAssetName, @IntroVideoAssetName, @IsGlobal, \
								   @TopSnapShotFilePath, \
								   @IntroVideoFilePath, \
								   @IsComplete, \
								   @QuestDescStringIndex, \
								   @AcquireQuestRequirement, @CompleteQuestRequirement, \
								   @Reward, \
								   @Reserved1, @Reserved2, @Reserved3, @Reserved4, @Reserved5)");

		rs->DataBinding("@QuestNameStringIndex", QuestNameStringIndex);
		rs->DataBinding("@TopSnapShotAssetName", record.TopSnapShotAssetName.c_str());
		rs->DataBinding("@IntroVideoAssetName", record.IntroVideoAssetName.c_str());
		rs->DataBinding("@IsGlobal", record.IsGlobal);
		rs->DataBinding("@TopSnapShotFilePath", record.TopSnapShotFilePath.c_str());
		rs->DataBinding("@IntroVideoFilePath", record.IntroVideoFilePath.c_str());
		rs->DataBinding("@IsComplete", record.IsComplete);
		rs->DataBinding("@QuestDescStringIndex", QuestDescStringIndex);
		rs->DataBinding("@AcquireQuestRequirement", record.AcquireQuestRequirement.c_str());
		rs->DataBinding("@CompleteQuestRequirement", record.CompleteQuestRequirement.c_str());
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

bool CQuestDBProvider::DeleteQuestRecordByID(int ID)
{
	DBEntity* pDB = GetDBEntity();
	assert(pDB!=NULL);
	bool bSucceed = false;
	CICRecordSet* rs = NULL;

	__int64 QuestNameStringIndex = 0;
	__int64 QuestDescStringIndex = 0;

	try
	{
		// select the target puzzle record in the Quest_DB
		rs = pDB->CreateRecordSet1("SELECT     QuestNameStringIndex, \
								   QuestDescStringIndex \
								   FROM		Quest_DB\
								   WHERE	(ID = @ID) ");
		rs->DataBinding("@ID", ID);

		bool bHasRecord = (rs->NextRow()==0);
		if(bHasRecord)
		{
			int nCount = rs->ColumnCount();
			int i = 0;

			// find the index into the string table
			CICRecordSetItem& item = (*rs)[0];
			QuestNameStringIndex = item;
			item = (*rs)[++i];
			QuestDescStringIndex = item;
		}
		else
		{
			// No record found in Quest_DB
			return false;
		}
		rs->Reset();

		// delete string table records
		CStringTableDB::GetInstance().DeleteEntryByID(QuestNameStringIndex);
		CStringTableDB::GetInstance().DeleteEntryByID(QuestDescStringIndex);

		// delete quest database records
		rs = pDB->CreateRecordSet1("DELETE	FROM		Quest_DB	WHERE	([ID] = @ID)");
		rs->DataBinding("@ID", ID);
		bSucceed = (rs->Execute() == S_OK);
	}
	catch (CICSQLException& e)
	{
		OUTPUT_LOG("%s\n", e.errmsg.c_str());
	}
	rs->Release();
	return bSucceed;
}



CQuestDBProvider& CQuestDBProvider::GetInstance()
{
	static CQuestDBProvider g_singletonQuestDB;
	return g_singletonQuestDB;
}

ParaInfoCenter::DBEntity* CQuestDBProvider::GetDBEntity()
{
	return m_pDataBase.get();
}

void CQuestDBProvider::SetDBEntity(ParaInfoCenter::DBEntity* pDb)
{
	m_pDataBase = pDb;
}
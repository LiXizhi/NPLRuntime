#include "ParaEngine.h"
#include "ic/ICDBManager.h"
#include "ic/ICRecordSet.h"

#include "TitleDBProvider.h"

using namespace ParaInfoCenter;
using namespace ParaEngine;

CTitleDBProvider::CTitleDBProvider(void) : m_pDataBase(NULL)
{
}

CTitleDBProvider::CTitleDBProvider(ParaInfoCenter::DBEntity* pDb)
{
	m_pDataBase = pDb;
}

CTitleDBProvider::~CTitleDBProvider(void)
{
}

#ifdef TEST_TITLE
void CTitleDBProvider::TestDB()
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

bool CTitleDBProvider::InsertTitleRecord(stTitleDbRecord& record)
{
	DBEntity* pDB = GetDBEntity();
	assert(pDB!=NULL);
	bool bSucceed = false;
	CICRecordSet* rs = NULL;

	try
	{
		CStringTableDB::GetInstance().InsertString(record.Title);

		__int64 TitleStringIndex = record.Title.ID;

		rs = pDB->CreateRecordSet1("INSERT INTO Title_DB \
								   ([ID], TitleLevel, \
								   IsBoy, TitleStringIndex, \
								   Reserved1, Reserved2, Reserved3, Reserved4, Reserved5)\
								   VALUES \
								   (NULL, @TitleLevel, \
								   @IsBoy, @TitleStringIndex, \
								   @Reserved1, @Reserved2, @Reserved3, @Reserved4, @Reserved5)");

		rs->DataBinding("@TitleLevel", record.TitleLevel);
		rs->DataBinding("@IsBoy", record.IsBoy);
		rs->DataBinding("@TitleStringIndex", TitleStringIndex);
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

bool CTitleDBProvider::DeleteTitleRecordByID(int ID)
{
	DBEntity* pDB = GetDBEntity();
	assert(pDB!=NULL);
	bool bSucceed = false;
	CICRecordSet* rs = NULL;

	__int64 TitleStringIndex = 0;

	try
	{
		// select the target title record in the Title_DB
		rs = pDB->CreateRecordSet1("SELECT     TitleStringIndex \
								   FROM		Title_DB\
								   WHERE	(ID = @ID) ");
		rs->DataBinding("@ID", ID);

		bool bHasRecord = (rs->NextRow()==0);
		if(bHasRecord)
		{
			int nCount = rs->ColumnCount();
			int i = 0;

			// find the index into the string table
			CICRecordSetItem& item = (*rs)[0];
			TitleStringIndex = item;
		}
		else
		{
			// No record found in Title_DB
			return false;
		}
		rs->Reset();

		// delete string table records
		CStringTableDB::GetInstance().DeleteEntryByID(TitleStringIndex);

		// delete title database records
		rs = pDB->CreateRecordSet1("DELETE	FROM		Title_DB	WHERE	([ID] = @ID)");
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

CTitleDBProvider& CTitleDBProvider::GetInstance()
{
	static CTitleDBProvider g_singletonTitleDB;
	return g_singletonTitleDB;
}

ParaInfoCenter::DBEntity* CTitleDBProvider::GetDBEntity()
{
	return m_pDataBase;
}

void CTitleDBProvider::SetDBEntity(ParaInfoCenter::DBEntity* pDb)
{
	m_pDataBase = pDb;
}
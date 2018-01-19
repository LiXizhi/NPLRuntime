//-----------------------------------------------------------------------------
// Class: CFruitDBProvider
// Authors:	Andy, LiXizhi
// Emails:	LiXizhi@yeah.net
// Date:	
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "InfoCenter/ICDBManager.h"
#include "InfoCenter/ICRecordSet.h"

#include "ItemDBProvider.h"

using namespace ParaInfoCenter;
using namespace ParaEngine;

CItemDBProvider::CItemDBProvider(void)
{
}

CItemDBProvider::CItemDBProvider(ParaInfoCenter::DBEntity* pDb)
{
	m_pDataBase = pDb;
}

CItemDBProvider::~CItemDBProvider(void)
{
}

#ifdef TEST_ITEM
void CItemDBProvider::TestDB()
{
	bool returnValue = false;
	string str;
	stItemDbRecord record;

	record.IconAssetName = "Building";
	record.DescTextAssetName = "BuildingDesc";
	record.IsGlobal = false;
	record.IconFilePath = "Building.bmp";
	record.ModelFilePath = "Building.x";
	record.ItemType = 5;
	record.IsObtained = true;
	record.IsUnique = false;
	record.CostExperiencePt = 123;
	record.CostOrange = 10;
	record.CostApple = 20;
	record.CostPeach = 30;
	record.ItemName.StringEnglish = "BuildingEN";
	record.ItemName.StringSimplifiedChinese = "BuildingCN";
	record.Desc.StringEnglish = "DescEN";
	record.Desc.StringSimplifiedChinese = "DescCN";
	record.Reserved1 = "R1";
	record.Reserved2 = "R2";
	record.Reserved3 = "R3";
	record.Reserved4 = "R4";
	record.Reserved5 = "R5";

	returnValue = InsertItemRecord(record);

	stItemDbRecord temp;
	returnValue = SelectItemRecordByID(temp, 1);
	returnValue = DeleteItemRecordByID(1);
}
#endif

bool CItemDBProvider::InsertItemRecord(stItemDbRecord& record)
{
	DBEntity* pDB = GetDBEntity();
	assert(pDB!=NULL);
	bool bSucceed = false;
	CICRecordSet* rs = NULL;

	try
	{
		CStringTableDB::GetInstance().InsertString(record.ItemName);
		CStringTableDB::GetInstance().InsertString(record.Desc);

		__int64 ItemNameStringIndex = record.ItemName.ID;
		__int64 DescStringIndex = record.Desc.ID;

		rs = pDB->CreateRecordSet1("INSERT INTO Item_DB \
								   ([ID], ItemNameStringIndex, \
								   IconAssetName, DescTextAssetName, IsGlobal, \
								   IconFilePath, \
								   DescStringIndex, \
								   ModelFilePath, \
								   ItemType, IsObtained, IsUnique, \
								   CostExperiencePt, CostOrange, CostApple, CostPeach, \
								   Reserved1, Reserved2, Reserved3, Reserved4, Reserved5)\
								   VALUES \
								   (NULL, @ItemNameStringIndex, \
								   @IconAssetName, @DescTextAssetName, @IsGlobal, \
								   @IconFilePath, \
								   @DescStringIndex, \
								   @ModelFilePath, \
								   @ItemType, @IsObtained, @IsUnique, \
								   @CostExperiencePt, @CostOrange, @CostApple, @CostPeach, \
								   @Reserved1, @Reserved2, @Reserved3, @Reserved4, @Reserved5)");

		rs->DataBinding("@ItemNameStringIndex", ItemNameStringIndex);
		rs->DataBinding("@IconAssetName", record.IconAssetName.c_str());
		rs->DataBinding("@DescTextAssetName", record.DescTextAssetName.c_str());
		rs->DataBinding("@IsGlobal", record.IsGlobal);
		rs->DataBinding("@IconFilePath", record.IconFilePath.c_str());
		rs->DataBinding("@DescStringIndex", DescStringIndex);
		rs->DataBinding("@ModelFilePath", record.ModelFilePath.c_str());
		rs->DataBinding("@ItemType", record.ItemType);
		rs->DataBinding("@IsObtained", record.IsObtained);
		rs->DataBinding("@IsUnique", record.IsUnique);
		rs->DataBinding("@CostExperiencePt", record.CostExperiencePt);
		rs->DataBinding("@CostOrange", record.CostOrange);
		rs->DataBinding("@CostApple", record.CostApple);
		rs->DataBinding("@CostPeach", record.CostPeach);
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


bool CItemDBProvider::UpdateItemRecord(stItemDbRecord& record)
{
	DBEntity* pDB = GetDBEntity();
	assert(pDB!=NULL);
	bool bSucceed = false;
	CICRecordSet* rs = NULL;

	bSucceed = DeleteItemRecordByID((int)record.ID);

	if(bSucceed == true)
	{
		try
		{
			CStringTableDB::GetInstance().InsertString(record.ItemName);
			CStringTableDB::GetInstance().InsertString(record.Desc);

			__int64 ItemNameStringIndex = record.ItemName.ID;
			__int64 DescStringIndex = record.Desc.ID;

			rs = pDB->CreateRecordSet1("INSERT INTO Item_DB \
									([ID], ItemNameStringIndex, \
									IconAssetName, DescTextAssetName, IsGlobal, \
									IconFilePath, \
									DescStringIndex, \
									ModelFilePath, \
									ItemType, IsObtained, IsUnique, \
									CostExperiencePt, CostOrange, CostApple, CostPeach, \
									Reserved1, Reserved2, Reserved3, Reserved4, Reserved5)\
									VALUES \
									(@ID, @ItemNameStringIndex, \
									@IconAssetName, @DescTextAssetName, @IsGlobal, \
									@IconFilePath, \
									@DescStringIndex, \
									@ModelFilePath, \
									@ItemType, @IsObtained, @IsUnique, \
									@CostExperiencePt, @CostOrange, @CostApple, @CostPeach, \
									@Reserved1, @Reserved2, @Reserved3, @Reserved4, @Reserved5)");

			rs->DataBinding("@ID", record.ID);
			rs->DataBinding("@ItemNameStringIndex", ItemNameStringIndex);
			rs->DataBinding("@IconAssetName", record.IconAssetName.c_str());
			rs->DataBinding("@DescTextAssetName", record.DescTextAssetName.c_str());
			rs->DataBinding("@IsGlobal", record.IsGlobal);
			rs->DataBinding("@IconFilePath", record.IconFilePath.c_str());
			rs->DataBinding("@DescStringIndex", DescStringIndex);
			rs->DataBinding("@ModelFilePath", record.ModelFilePath.c_str());
			rs->DataBinding("@ItemType", record.ItemType);
			rs->DataBinding("@IsObtained", record.IsObtained);
			rs->DataBinding("@IsUnique", record.IsUnique);
			rs->DataBinding("@CostExperiencePt", record.CostExperiencePt);
			rs->DataBinding("@CostOrange", record.CostOrange);
			rs->DataBinding("@CostApple", record.CostApple);
			rs->DataBinding("@CostPeach", record.CostPeach);
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
	else
		return false;
}

bool CItemDBProvider::DeleteItemRecordByID(int ID)
{
	DBEntity* pDB = GetDBEntity();
	assert(pDB!=NULL);
	bool bSucceed = false;
	CICRecordSet* rs = NULL;

	__int64 ItemNameStringIndex = 0;
	__int64 DescStringIndex = 0;

	try
	{
		// select the target puzzle record in the Puzzle_DB
		rs = pDB->CreateRecordSet1("SELECT     ItemNameStringIndex, \
								   DescStringIndex \
								   FROM		Item_DB\
								   WHERE	(ID = @ID) ");
		rs->DataBinding("@ID", ID);

		bool bHasRecord = (rs->NextRow()==0);
		if(bHasRecord)
		{
			int nCount = rs->ColumnCount();
			int i = 0;

			// find the index into the string table
			CICRecordSetItem& item = (*rs)[0];
			ItemNameStringIndex = item;
			item = (*rs)[++i];
			DescStringIndex = item;
		}
		else
		{
			// No record found in Puzzle_DB
			return false;
		}
		rs->Reset();

		// delete string table records
		CStringTableDB::GetInstance().DeleteEntryByID(ItemNameStringIndex);
		CStringTableDB::GetInstance().DeleteEntryByID(DescStringIndex);

		// delete puzzle database records
		rs = pDB->CreateRecordSet1("DELETE	FROM		Item_DB	WHERE	([ID] = @ID)");
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

bool CItemDBProvider::SelectItemRecordByID(stItemDbRecord& record, __int64 ID)
{
	DBEntity* pDB = GetDBEntity();
	assert(pDB!=NULL);
	bool bSucceed = false;
	CICRecordSet* rs = NULL;

	try
	{

		// create the SQL procedure
		rs = pDB->CreateRecordSet1("SELECT     ItemNameStringIndex, \
								   IconAssetName, DescTextAssetName, IsGlobal, \
								   IconFilePath, DescStringIndex, ModelFilePath, \
								   ItemType, IsObtained, IsUnique, CostExperiencePt, CostOrange, CostApple, CostPeach, \
								   Reserved1, Reserved2, Reserved3, Reserved4, Reserved5 \
								   FROM		Item_DB\
								   WHERE (ID = @ID) ");

		rs->DataBinding("@ID", ID);

		bool bHasRecord = (rs->NextRow()==0);
		if(bHasRecord)
		{
			int nCount = rs->ColumnCount();

			record.ID = ID;
			int i=0;
			CICRecordSetItem& item = (*rs)[0];
			record.ItemName.ID = item;

			item = (*rs)[++i]; 
			record.IconAssetName = item;
			item = (*rs)[++i]; 
			record.DescTextAssetName = item;
			item = (*rs)[++i]; 
			record.IsGlobal = item;

			item = (*rs)[++i]; 
			record.IconFilePath = item;
			item = (*rs)[++i]; 
			record.Desc.ID = item;
			item = (*rs)[++i]; 
			record.ModelFilePath = item;

			item = (*rs)[++i]; 
			record.ItemType = item;
			item = (*rs)[++i]; 
			record.IsObtained = item;
			item = (*rs)[++i]; 
			record.IsUnique = item;
			item = (*rs)[++i]; 
			record.CostExperiencePt = item;
			item = (*rs)[++i]; 
			record.CostOrange = item;
			item = (*rs)[++i]; 
			record.CostApple = item;
			item = (*rs)[++i]; 
			record.CostPeach = item;

			item = (*rs)[++i]; 
			record.Reserved1 = item;
			item = (*rs)[++i]; 
			record.Reserved2 = item;
			item = (*rs)[++i]; 
			record.Reserved3 = item;
			item = (*rs)[++i]; 
			record.Reserved4 = item;
			item = (*rs)[++i]; 
			record.Reserved5 = item;
		}
		else // if(bHasRecord)
		{
			return false;
		}
		rs->Reset();

		rs = pDB->CreateRecordSet1("SELECT     StringEnglish, StringSimplifiedChinese, \
								   Reserved1, Reserved2, Reserved3, Reserved4, Reserved5 \
								   FROM		StringTable_DB\
								   WHERE (ID = @ID) ");

		rs->DataBinding("@ID", record.ItemName.ID);

		bHasRecord = (rs->NextRow()==0);
		if(bHasRecord)
		{
			int nCount = rs->ColumnCount();

			record.ID = ID;
			int i=0;
			CICRecordSetItem& item = (*rs)[0];
			record.ItemName.StringEnglish = item;

			item = (*rs)[++i]; 
			record.ItemName.StringSimplifiedChinese = item;

			item = (*rs)[++i]; 
			record.ItemName.Reserved1 = item;
			item = (*rs)[++i]; 
			record.ItemName.Reserved2 = item;
			item = (*rs)[++i]; 
			record.ItemName.Reserved3 = item;
			item = (*rs)[++i]; 
			record.ItemName.Reserved4 = item;
			item = (*rs)[++i];
			record.ItemName.Reserved5 = item;
		}
		else // if(bHasRecord)
		{
			return false;
		}
		rs->Reset();

		rs = pDB->CreateRecordSet1("SELECT     StringEnglish, StringSimplifiedChinese, \
								   Reserved1, Reserved2, Reserved3, Reserved4, Reserved5 \
								   FROM		StringTable_DB\
								   WHERE (ID = @ID) ");

		rs->DataBinding("@ID", record.Desc.ID);

		bHasRecord = (rs->NextRow()==0);
		if(bHasRecord)
		{
			int nCount = rs->ColumnCount();

			record.ID = ID;
			int i=0;
			CICRecordSetItem& item = (*rs)[0];
			record.Desc.StringEnglish = item;

			item = (*rs)[++i]; 
			record.Desc.StringSimplifiedChinese = item;

			item = (*rs)[++i]; 
			record.Desc.Reserved1 = item;
			item = (*rs)[++i]; 
			record.Desc.Reserved2 = item;
			item = (*rs)[++i]; 
			record.Desc.Reserved3 = item;
			item = (*rs)[++i]; 
			record.Desc.Reserved4 = item;
			item = (*rs)[++i];
			record.Desc.Reserved5 = item;

			bSucceed = true;
		}
		else // if(bHasRecord)
		{
			return false;
		}
		rs->Reset();
	}
	catch (CICSQLException& e)
	{
		OUTPUT_LOG("%s\n", e.errmsg.c_str());
	}

	return bSucceed;
}


CItemDBProvider& CItemDBProvider::GetInstance()
{
	static CItemDBProvider g_singletonItemDB;
	return g_singletonItemDB;
}

ParaInfoCenter::DBEntity* CItemDBProvider::GetDBEntity()
{
	return m_pDataBase.get();
}

void CItemDBProvider::SetDBEntity(ParaInfoCenter::DBEntity* pDb)
{
	m_pDataBase = pDb;
}
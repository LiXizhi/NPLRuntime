//-----------------------------------------------------------------------------
// Class: CPetDBProvider
// Authors:	Andy, LiXizhi
// Emails:	LiXizhi@yeah.net
// Date:	
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "InfoCenter/ICDBManager.h"
#include "InfoCenter/ICRecordSet.h"

#include "PetDBProvider.h"

using namespace ParaInfoCenter;
using namespace ParaEngine;

CPetDBProvider::CPetDBProvider(void)
{
}

CPetDBProvider::CPetDBProvider(ParaInfoCenter::DBEntity* pDb)
{
	m_pDataBase = pDb;
}

CPetDBProvider::~CPetDBProvider(void)
{
}

#ifdef TEST_PET
void CPetDBProvider::TestDB()
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



bool CPetDBProvider::InsertPetRecord(stPetDbRecord& record)
{
	DBEntity* pDB = GetDBEntity();
	assert(pDB!=NULL);
	bool bSucceed = false;
	CICRecordSet* rs = NULL;

	// NOTE: record in database FORMAT "ID,ID,ID,......,ID"
	string PetAIEntryIndexArray;
	vector <__int64>::iterator iter;
	char tmp[40];
	if(record.PetAIIndexVector.size() == 0)
	{
		;
	}
	else if(record.PetAIIndexVector.size() > 0)
	{
		for(iter = record.PetAIIndexVector.begin() ; iter != record.PetAIIndexVector.end() ; iter++)
		{
			sprintf(tmp, "%I64d,", *iter);
			PetAIEntryIndexArray.append(tmp);
		}
		PetAIEntryIndexArray.erase(PetAIEntryIndexArray.size()-1);
	}
	else
	{
		// negative pet AI entry
		return false;
	}

	try
	{
		CStringTableDB::GetInstance().InsertString(record.PetName);

		__int64 PetNameStringIndex = record.PetName.ID;

		rs = pDB->CreateRecordSet1("INSERT INTO Pet_DB \
								   ([ID], PetNameStringIndex, \
								   AssetName, IsGlobal, IsSnapToTerrain, \
								   PetModeFilePath, \
								   PetModelScale, \
								   PetRadius, \
								   PetFacing, \
								   posX, posY, posZ, \
								   PetAIEntryIndexArray, \
								   PerceptiveRadius, SentientRadius, OnLoadScript, \
								   IsKeptinCage, \
								   Reserved1, Reserved2, Reserved3, Reserved4, Reserved5)\
								   VALUES \
								   (NULL, @PetNameStringIndex, \
								   @AssetName, @IsGlobal, @IsSnapToTerrain, \
								   @PetModeFilePath, \
								   @PetModelScale, \
								   @PetRadius, \
								   @PetFacing, \
								   @posX, @posY, @posZ, \
								   @PetAIEntryIndexArray, \
								   @PerceptiveRadius, @SentientRadius, @OnLoadScript, \
								   @IsKeptinCage, \
								   @Reserved1, @Reserved2, @Reserved3, @Reserved4, @Reserved5)");

		rs->DataBinding("@PetNameStringIndex", PetNameStringIndex);
		rs->DataBinding("@AssetName", record.AssetName.c_str());
		rs->DataBinding("@IsGlobal", record.IsGlobal);
		rs->DataBinding("@IsSnapToTerrain", record.IsSnapToTerrain);
		rs->DataBinding("@PetModeFilePath", record.PetModeFilePath.c_str());
		rs->DataBinding("@PetModelScale", record.PetModelScale);
		rs->DataBinding("@PetRadius", record.PetRadius);
		rs->DataBinding("@PetFacing", record.PetFacing);
		rs->DataBinding("@posX", record.posX);
		rs->DataBinding("@posY", record.posY);
		rs->DataBinding("@posZ", record.posZ);
		rs->DataBinding("@PetAIEntryIndexArray", PetAIEntryIndexArray.c_str());
		rs->DataBinding("@PerceptiveRadius", record.PerceptiveRadius);
		rs->DataBinding("@SentientRadius", record.SentientRadius);
		rs->DataBinding("@OnLoadScript", record.OnLoadScript.c_str());
		rs->DataBinding("@IsKeptinCage", record.IsKeptinCage);
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

bool CPetDBProvider::DeletePetRecordByID(int ID)
{
	DBEntity* pDB = GetDBEntity();
	assert(pDB!=NULL);
	bool bSucceed = false;
	CICRecordSet* rs = NULL;

	__int64 PetNameStringIndex = 0;

	try
	{
		// select the target puzzle record in the Pet_DB
		rs = pDB->CreateRecordSet1("SELECT     PetNameStringIndex \
								   FROM		Pet_DB\
								   WHERE	(ID = @ID) ");
		rs->DataBinding("@ID", ID);

		bool bHasRecord = (rs->NextRow()==0);
		if(bHasRecord)
		{
			int nCount = rs->ColumnCount();
			int i = 0;

			// find the index into the string table
			CICRecordSetItem& item = (*rs)[0];
			PetNameStringIndex = item;
		}
		else
		{
			// No record found in Pet_DB
			return false;
		}
		rs->Reset();

		// delete string table records
		CStringTableDB::GetInstance().DeleteEntryByID(PetNameStringIndex);

		// delete puzzle database records
		rs = pDB->CreateRecordSet1("DELETE	FROM		Pet_DB WHERE	([ID] = @ID)");
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


CPetDBProvider& CPetDBProvider::GetInstance()
{
	static CPetDBProvider g_singletonPetDB;
	return g_singletonPetDB;
}

ParaInfoCenter::DBEntity* CPetDBProvider::GetDBEntity()
{
	return m_pDataBase.get();
}

void CPetDBProvider::SetDBEntity(ParaInfoCenter::DBEntity* pDb)
{
	m_pDataBase = pDb;
}
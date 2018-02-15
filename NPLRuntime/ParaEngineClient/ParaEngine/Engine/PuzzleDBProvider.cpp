//-----------------------------------------------------------------------------
// Class:	CPuzzleDBProvider
// Authors:	Andy Wang
// Emails: mitnick_wang@hotmail.com
// Date:	2006.8.10
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "InfoCenter/ICDBManager.h"
#include "InfoCenter/ICRecordSet.h"

#include "PuzzleDBProvider.h"

using namespace ParaInfoCenter;
using namespace ParaEngine;


CPuzzleDBProvider::CPuzzleDBProvider(void) 
{
}

CPuzzleDBProvider::CPuzzleDBProvider(ParaInfoCenter::DBEntity* pDb)
{
	m_pDataBase = pDb;
}

CPuzzleDBProvider::~CPuzzleDBProvider(void)
{
}

#ifdef TEST_PUZZLE
void CPuzzleDBProvider::TestDB()
{
	//bool returnValue = false;
	//string str;
	//stPuzzleDbRecord record;

	//record.CorrectChoice = 2;
	//record.DifficultyRate = 5;
	//record.NumberOfChoices = 4;
	//record.ExplanationPicFilePath = "Explain.bmp";
	//record.PuzzleDescPicFilePath = "Desc.bmp";
	//record.Explanation.StringEnglish = "is B";
	//record.Explanation.StringSimplifiedChinese = "是2";
	//record.PuzzleAnswerA.StringEnglish = "A";
	//record.PuzzleAnswerA.StringSimplifiedChinese = "1";
	//record.PuzzleAnswerB.StringEnglish = "B";
	//record.PuzzleAnswerB.StringSimplifiedChinese = "2";
	//record.PuzzleAnswerC.StringEnglish = "C";
	//record.PuzzleAnswerC.StringSimplifiedChinese = "3";
	//record.PuzzleAnswerD.StringEnglish = "D";
	//record.PuzzleAnswerD.StringSimplifiedChinese = "4";
	//record.PuzzleDesc.StringEnglish = "One plus one";
	//record.PuzzleDesc.StringSimplifiedChinese = "1+1=?";
	//record.PuzzleName.StringEnglish = "1+1";
	//record.PuzzleName.StringSimplifiedChinese = "一加一";

	//returnValue = InsertPuzzleRecord(record);
	//returnValue = DeletePuzzleRecordByID(1);
}
#endif

bool CPuzzleDBProvider::InsertPuzzleRecord(stPuzzleDbRecord& record)
{
	DBEntity* pDB = GetDBEntity();
	assert(pDB!=NULL);
	bool bSucceed = false;
	CICRecordSet* rs = NULL;

	try
	{
		CStringTableDB::GetInstance().InsertString(record.PuzzleName);
		CStringTableDB::GetInstance().InsertString(record.PuzzleDesc);
		CStringTableDB::GetInstance().InsertString(record.PuzzleAnswerA);
		CStringTableDB::GetInstance().InsertString(record.PuzzleAnswerB);
		CStringTableDB::GetInstance().InsertString(record.PuzzleAnswerC);
		CStringTableDB::GetInstance().InsertString(record.PuzzleAnswerD);
		CStringTableDB::GetInstance().InsertString(record.Explanation);

		__int64 PuzzleNameStringIndex = record.PuzzleName.ID;
		__int64 PuzzleDescStringIndex = record.PuzzleDesc.ID;
		__int64 PuzzleAnswerAStringIndex = record.PuzzleAnswerA.ID;
		__int64 PuzzleAnswerBStringIndex = record.PuzzleAnswerB.ID;
		__int64 PuzzleAnswerCStringIndex = record.PuzzleAnswerC.ID;
		__int64 PuzzleAnswerDStringIndex = record.PuzzleAnswerD.ID;
		__int64 ExplanationStringIndex = record.Explanation.ID;

		rs = pDB->CreateRecordSet1("INSERT INTO Puzzle_DB \
								([ID], PuzzleNameStringIndex, DifficultyRate, \
								PuzzleDescStringIndex, NumberOfChoices, \
								PuzzleAnswerAStringIndex, \
								PuzzleAnswerBStringIndex, \
								PuzzleAnswerCStringIndex, \
								PuzzleAnswerDStringIndex, \
								PuzzleDescPicFilePath, \
								CorrectChoice, ExplanationStringIndex, ExplanationPicFilePath, \
								Reserved1, Reserved2, Reserved3, Reserved4, Reserved5)\
								VALUES \
								(NULL, @PuzzleNameStringIndex, @DifficultyRate, \
								@PuzzleDescStringIndex, @NumberOfChoices, \
								@PuzzleAnswerAStringIndex, \
								@PuzzleAnswerBStringIndex, \
								@PuzzleAnswerCStringIndex, \
								@PuzzleAnswerDStringIndex, \
								@PuzzleDescPicFilePath, \
								@CorrectChoice, @ExplanationStringIndex, @ExplanationPicFilePath, \
								@Reserved1, @Reserved2, @Reserved3, @Reserved4, @Reserved5)");

		rs->DataBinding("@PuzzleNameStringIndex", PuzzleNameStringIndex);
		rs->DataBinding("@DifficultyRate", record.DifficultyRate);
		rs->DataBinding("@PuzzleDescStringIndex", PuzzleDescStringIndex);
		rs->DataBinding("@NumberOfChoices", record.NumberOfChoices);
		rs->DataBinding("@PuzzleAnswerAStringIndex", PuzzleAnswerAStringIndex);
		rs->DataBinding("@PuzzleAnswerBStringIndex", PuzzleAnswerBStringIndex);
		rs->DataBinding("@PuzzleAnswerCStringIndex", PuzzleAnswerCStringIndex);
		rs->DataBinding("@PuzzleAnswerDStringIndex", PuzzleAnswerDStringIndex);
		rs->DataBinding("@PuzzleDescPicFilePath", record.PuzzleDescPicFilePath.c_str());
		rs->DataBinding("@CorrectChoice", record.CorrectChoice);
		rs->DataBinding("@ExplanationStringIndex", ExplanationStringIndex);
		rs->DataBinding("@ExplanationPicFilePath", record.ExplanationPicFilePath.c_str());
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

bool CPuzzleDBProvider::DeletePuzzleRecordByID(int ID)
{
	DBEntity* pDB = GetDBEntity();
	assert(pDB!=NULL);
	bool bSucceed = false;
	CICRecordSet* rs = NULL;

	__int64 PuzzleNameStringIndex = 0;
	__int64 PuzzleDescStringIndex = 0;
	__int64 PuzzleAnswerAStringIndex = 0;
	__int64 PuzzleAnswerBStringIndex = 0;
	__int64 PuzzleAnswerCStringIndex = 0;
	__int64 PuzzleAnswerDStringIndex = 0;
	__int64 ExplanationStringIndex = 0;

	try
	{
		// select the target puzzle record in the Puzzle_DB
		rs = pDB->CreateRecordSet1("SELECT     PuzzleNameStringIndex, \
													PuzzleDescStringIndex, \
													PuzzleAnswerAStringIndex, \
													PuzzleAnswerBStringIndex, \
													PuzzleAnswerCStringIndex, \
													PuzzleAnswerDStringIndex, \
													ExplanationStringIndex \
													FROM		Puzzle_DB\
													WHERE	(ID = @ID) ");
		rs->DataBinding("@ID", ID);

		bool bHasRecord = (rs->NextRow()==0);
		if(bHasRecord)
		{
			int nCount = rs->ColumnCount();
			int i = 0;

			// find the index into the string table
			CICRecordSetItem& item = (*rs)[0];
			PuzzleNameStringIndex = item;
			item = (*rs)[++i];
			PuzzleDescStringIndex = item;
			item = (*rs)[++i];
			PuzzleAnswerAStringIndex = item;
			item = (*rs)[++i];
			PuzzleAnswerBStringIndex = item;
			item = (*rs)[++i];
			PuzzleAnswerCStringIndex = item;
			item = (*rs)[++i];
			PuzzleAnswerDStringIndex = item;
			item = (*rs)[++i];
			ExplanationStringIndex = item;
		}
		else
		{
			// No record found in Puzzle_DB
			return false;
		}
		rs->Reset();

		// delete string table records
		CStringTableDB::GetInstance().DeleteEntryByID(PuzzleNameStringIndex);
		CStringTableDB::GetInstance().DeleteEntryByID(PuzzleDescStringIndex);
		CStringTableDB::GetInstance().DeleteEntryByID(PuzzleAnswerAStringIndex);
		CStringTableDB::GetInstance().DeleteEntryByID(PuzzleAnswerBStringIndex);
		CStringTableDB::GetInstance().DeleteEntryByID(PuzzleAnswerCStringIndex);
		CStringTableDB::GetInstance().DeleteEntryByID(PuzzleAnswerDStringIndex);
		CStringTableDB::GetInstance().DeleteEntryByID(ExplanationStringIndex);

		// delete puzzle database records
		rs = pDB->CreateRecordSet1("DELETE	FROM		Puzzle_DB	WHERE	([ID] = @ID)");
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

CPuzzleDBProvider& CPuzzleDBProvider::GetInstance()
{
	static CPuzzleDBProvider g_singletonPuzzleDB;
	return g_singletonPuzzleDB;
}

ParaInfoCenter::DBEntity* CPuzzleDBProvider::GetDBEntity()
{
	return m_pDataBase.get();
}

void CPuzzleDBProvider::SetDBEntity(ParaInfoCenter::DBEntity* pDb)
{
	m_pDataBase = pDb;
}
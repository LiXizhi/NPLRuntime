//-----------------------------------------------------------------------------
// Class:	CKidsDBProvider
// Authors:	Andy Wang
// Emails: mitnick_wang@hotmail.com
// Date:	2006.8.10
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ic/ICDBManager.h"
#include "ic/ICRecordSet.h"

#include <string>
#include <iostream>

#include ".\kidsdbprovider.h"

using namespace ParaInfoCenter;
using namespace ParaEngine;

using namespace std;


CKidsDBProvider::CKidsDBProvider(void)
{
	m_dbCharacter = NULL;
	m_dbChest = NULL;
	m_dbFruit = NULL;
	m_dbItem = NULL;
	m_dbPetAI = NULL;
	m_dbPet = NULL;
	m_dbPuzzle = NULL;
	m_dbQuest = NULL;
	m_dbStringTable = NULL;
	m_dbTitle = NULL;
}

CKidsDBProvider::~CKidsDBProvider(void)
{
}

void CKidsDBProvider::Cleanup()
{
	if(m_dbStringTable)
	{
		m_dbStringTable->Cleanup();
		m_dbStringTable = NULL;
	}
	m_pDataBase.reset();
}

#ifdef _DEBUG
void CKidsDBProvider::TestDB()
{
	// add test code to each data table provider
	//m_dbPuzzle->TestDB();

	// add test code to each data table provider
	//m_dbItem->TestDB();

	//bool returnValue = false;
	//string str;
	//stPuzzleDbRecord record;

	//record.ID = 9223372036854775800;
	//record.CorrectChoice = 2;
	//record.DifficultyRate = 5;
	//record.NumberOfChoices = 4;
	//record.ExplanationPicFilePath = "Explain.bmp";
	//record.PuzzleDescPicFilePath = "Desc.bmp";
	//record.Explanation.ID = 0;
	//record.Explanation.StringEnglish = "is B";
	//record.Explanation.StringSimplifiedChinese = "是2";
	//record.PuzzleAnswerA.ID = 0;
	//record.PuzzleAnswerA.StringEnglish = "A";
	//record.PuzzleAnswerA.StringSimplifiedChinese = "1";
	//record.PuzzleAnswerB.ID = 0;
	//record.PuzzleAnswerB.StringEnglish = "B";
	//record.PuzzleAnswerB.StringSimplifiedChinese = "2";
	//record.PuzzleAnswerC.ID = 0;
	//record.PuzzleAnswerC.StringEnglish = "C";
	//record.PuzzleAnswerC.StringSimplifiedChinese = "3";
	//record.PuzzleAnswerD.ID = 0;
	//record.PuzzleAnswerD.StringEnglish = "D";
	//record.PuzzleAnswerD.StringSimplifiedChinese = "4";
	//record.PuzzleDesc.ID = 0;
	//record.PuzzleDesc.StringEnglish = "One plus one";
	//record.PuzzleDesc.StringSimplifiedChinese = "1+1=?";
	//record.PuzzleName.ID = 0;
	//record.PuzzleName.StringEnglish = "1+1";
	//record.PuzzleName.StringSimplifiedChinese = "一加一";

	//ParsePuzzleRecordToString(str, record);

	////stPuzzleDbRecord record1;

	////ParseStringToPuzzleRecord(str, record1);

	//returnValue = InsertPuzzleRecordFromString(str);
	//returnValue = DeletePuzzleRecordByID(1);
}
#endif


string CKidsDBProvider::GetStringbyID(int ID)
{
	static string strGetBuf;
	CStringTableDB::GetInstance().GetStringbyID(strGetBuf, ID);
	return strGetBuf;
}

int CKidsDBProvider::InsertString(const char* strEN, const char * strCN)
{
	static stStringTableDbRecord recordInsertBuf;
	recordInsertBuf.StringEnglish.assign(strEN);
	recordInsertBuf.StringSimplifiedChinese.assign(strCN);
	if( CStringTableDB::GetInstance().InsertString(recordInsertBuf) )
	{
		return (int)recordInsertBuf.ID;
	}
	else
		return false;
}

bool CKidsDBProvider::InsertPuzzleRecordFromString(const string& strRecord)
{
	static stPuzzleDbRecord tempRecordBuf;
	ParseStringToPuzzleRecord(strRecord, tempRecordBuf);
	return CPuzzleDBProvider::GetInstance().InsertPuzzleRecord(tempRecordBuf);
}

bool CKidsDBProvider::DeletePuzzleRecordByID(int ID)
{
	return CPuzzleDBProvider::GetInstance().DeletePuzzleRecordByID(ID);
}

void CKidsDBProvider::ParsePuzzleRecordToString(string& str, const stPuzzleDbRecord& record)
{
	str.clear();
	// TODO: convert to record to string using ## as the tag between fields
	char tmp[40];
	__int64 tempID = 0;
	int tempINT = 0;

	//sprintf(tmp, "temp/skin%I64d.dds", tempID);
	//sscanf(tmp, "temp/skin%I64d.dds", &tempID);

	// int ID;
	tempID = record.ID;
	sprintf(tmp, "%I64d", tempID);
	str.append(tmp);

	// stStringTableDbRecord PuzzleName;	// record in string table
	AppendStringTableRecordToString(str, record.PuzzleName);

	// int DifficultyRate;
	tempINT = record.DifficultyRate;
	sprintf(tmp, "%d", tempINT);
	str.append("#");
	str.append(tmp);

	// stStringTableDbRecord PuzzleDesc;	// record in string table
	AppendStringTableRecordToString(str, record.PuzzleDesc);

	// int NumberOfChoices;	// Limit to 4 (ABCD)
	tempINT = record.NumberOfChoices;
	sprintf(tmp, "%d", tempINT);
	str.append("#");
	str.append(tmp);

	// stStringTableDbRecord PuzzleAnswerA;	// record in string table
	AppendStringTableRecordToString(str, record.PuzzleAnswerA);
	// stStringTableDbRecord PuzzleAnswerB;	// record in string table
	AppendStringTableRecordToString(str, record.PuzzleAnswerB);
	// stStringTableDbRecord PuzzleAnswerC;	// record in string table
	AppendStringTableRecordToString(str, record.PuzzleAnswerC);
	// stStringTableDbRecord PuzzleAnswerD;	// record in string table
	AppendStringTableRecordToString(str, record.PuzzleAnswerD);

	// string PuzzleDescPicFilePath;
	str.append("#");
	str.append(record.PuzzleDescPicFilePath);

	// int CorrectChoice;
	tempINT = record.CorrectChoice;
	sprintf(tmp, "%d", tempINT);
	str.append("#");
	str.append(tmp);

	// stStringTableDbRecord Explanation;	// record in string table
	AppendStringTableRecordToString(str, record.Explanation);

	// string ExplanationPicFilePath;
	str.append("#");
	str.append(record.ExplanationPicFilePath);

	// string Reserved-s
	str.append("#");
	str.append(record.Reserved1);
	str.append("#");
	str.append(record.Reserved2);
	str.append("#");
	str.append(record.Reserved3);
	str.append("#");
	str.append(record.Reserved4);
	str.append("#");
	str.append(record.Reserved5);
}

void CKidsDBProvider::AppendStringTableRecordToString(string& str, const stStringTableDbRecord& record)
{
	__int64 tempID = 0;
	tempID = record.ID;
	char tmp[40];
	sprintf(tmp, "%I64d", tempID);
	str.append("#");
	str.append(tmp); // record.ID
	str.append("#");
	str.append(record.StringEnglish);
	str.append("#");
	str.append(record.StringSimplifiedChinese);
	str.append("#");
	str.append(record.Reserved1);
	str.append("#");
	str.append(record.Reserved2);
	str.append("#");
	str.append(record.Reserved3);
	str.append("#");
	str.append(record.Reserved4);
	str.append("#");
	str.append(record.Reserved5);
}

void CKidsDBProvider::ParseStringToPuzzleRecord(const string& str, stPuzzleDbRecord& record)
{
	string strID;
	string strDifficultyRate;
	string strNumberOfChoices;	// Limit to 4 (ABCD)
	string strPuzzleDescPicFilePath;
	string strCorrectChoice;
	string strExplanationPicFilePath;
	string strReserved1;
	string strReserved2;
	string strReserved3;
	string strReserved4;
	string strReserved5;

	char backupStr[MAX_STRING_LENGTH];
	size_t i = 0;
	size_t tagFirst = 0;
	size_t tagLast = 0;
	const char * strChar = str.c_str();
	for ( i = 0 ; i < str.size() ; i++)
		backupStr[i] = strChar[i];

	string baseStr;
	baseStr.assign(str);

	tagLast = baseStr.find('#', tagFirst);
	strID.assign(baseStr, tagFirst, tagLast - tagFirst);

	tagFirst = tagLast;
	tagLast += GetStringTableRecordFromString(backupStr + tagFirst, record.PuzzleName);
	
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strDifficultyRate.assign(baseStr, tagFirst, tagLast - tagFirst);

	tagFirst = tagLast;
	tagLast += GetStringTableRecordFromString(backupStr + tagFirst, record.PuzzleDesc);

	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strNumberOfChoices.assign(baseStr, tagFirst, tagLast - tagFirst);

	tagFirst = tagLast;
	tagLast += GetStringTableRecordFromString(backupStr + tagFirst, record.PuzzleAnswerA);
	tagFirst = tagLast;
	tagLast += GetStringTableRecordFromString(backupStr + tagFirst, record.PuzzleAnswerB);
	tagFirst = tagLast;
	tagLast += GetStringTableRecordFromString(backupStr + tagFirst, record.PuzzleAnswerC);
	tagFirst = tagLast;
	tagLast += GetStringTableRecordFromString(backupStr + tagFirst, record.PuzzleAnswerD);

	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strPuzzleDescPicFilePath.assign(baseStr, tagFirst, tagLast - tagFirst);

	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strCorrectChoice.assign(baseStr, tagFirst, tagLast - tagFirst);

	tagFirst = tagLast;
	tagLast += GetStringTableRecordFromString(backupStr + tagFirst, record.Explanation);

	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strExplanationPicFilePath.assign(baseStr, tagFirst, tagLast - tagFirst);

	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strReserved1.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strReserved2.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strReserved3.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strReserved4.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strReserved5.assign(baseStr, tagFirst, tagLast - tagFirst);

	record.PuzzleDescPicFilePath = strPuzzleDescPicFilePath;
	record.ExplanationPicFilePath = strExplanationPicFilePath;
	record.Reserved1 = strReserved1;
	record.Reserved2 = strReserved2;
	record.Reserved3 = strReserved3;
	record.Reserved4 = strReserved4;
	record.Reserved5 = strReserved5;

	char tempStr[40];
	strChar = strID.c_str();
	sprintf(tempStr, "%s", strID.c_str());
	sscanf(tempStr, "%I64d", &(record.ID));
	sprintf(tempStr, "%s", strDifficultyRate.c_str());
	sscanf(tempStr, "%d", &(record.DifficultyRate));
	sprintf(tempStr, "%s", strNumberOfChoices.c_str());
	sscanf(tempStr, "%d", &(record.NumberOfChoices));
	sprintf(tempStr, "%s", strCorrectChoice.c_str());
	sscanf(tempStr, "%d", &(record.CorrectChoice));
}

int CKidsDBProvider::GetStringTableRecordFromString(char* str, stStringTableDbRecord& record)
{
	size_t tagFirst = 2;
	size_t tagLast = 0;
	__int64 tempID = 0;
	tempID = record.ID;

	string baseStr;
	baseStr.assign(str);

	string strID;
	string strStringEnglish;
	string strStringSimplifiedChinese;
	string strReserved1;
	string strReserved2;
	string strReserved3;
	string strReserved4;
	string strReserved5;

	tagLast = baseStr.find('#', tagFirst);
	strID.assign(str, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;

	tagLast = baseStr.find('#', tagFirst);
	strStringEnglish.assign(str, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;

	tagLast = baseStr.find('#', tagFirst);
	strStringSimplifiedChinese.assign(str, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;

	tagLast = baseStr.find('#', tagFirst);
	strReserved1.assign(str, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;

	tagLast = baseStr.find('#', tagFirst);
	strReserved2.assign(str, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;

	tagLast = baseStr.find('#', tagFirst);
	strReserved3.assign(str, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;

	tagLast = baseStr.find('#', tagFirst);
	strReserved4.assign(str, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;

	tagLast = baseStr.find('#', tagFirst);
	strReserved5.assign(str, tagFirst, tagLast - tagFirst);

	char tempStr[40];
	const char * strChar = strID.c_str();
	size_t i = 0;
	for ( i = 0 ; i < strID.size() ; i++)
		tempStr[i] = strChar[i];
	sscanf(tempStr, "%I64d", &(record.ID));

	record.StringEnglish = strStringEnglish;
	record.StringSimplifiedChinese = strStringSimplifiedChinese;
	record.Reserved1 = strReserved1;
	record.Reserved2 = strReserved2;
	record.Reserved3 = strReserved3;
	record.Reserved4 = strReserved4;
	record.Reserved5 = strReserved5;

	return tagLast;
}




void CKidsDBProvider::SetDBEntity(const string& sConnectionstring)
{
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

	m_pDataBase = CICDBManager::OpenDBEntity("Kids", fileName.c_str());

	// TODO: set database provider of each table provider class
	m_dbCharacter = &(CCharacterDBProvider::GetInstance());
	m_dbCharacter->SetDBEntity(m_pDataBase.get());
	m_dbChest = &(CChestDBProvider::GetInstance());
	m_dbChest->SetDBEntity(m_pDataBase.get());
	m_dbFruit = &(CFruitDBProvider::GetInstance());
	m_dbFruit->SetDBEntity(m_pDataBase.get());
	m_dbItem = &(CItemDBProvider::GetInstance());
	m_dbItem->SetDBEntity(m_pDataBase.get());
	m_dbPetAI = &(CPetAIDBProvider::GetInstance());
	m_dbPetAI->SetDBEntity(m_pDataBase.get());
	m_dbPet = &(CPetDBProvider::GetInstance());
	m_dbPet->SetDBEntity(m_pDataBase.get());
	m_dbPuzzle = &(CPuzzleDBProvider::GetInstance());
	m_dbPuzzle->SetDBEntity(m_pDataBase.get());
	m_dbQuest = &(CQuestDBProvider::GetInstance());
	m_dbQuest->SetDBEntity(m_pDataBase.get());
	m_dbStringTable = &(CStringTableDB::GetInstance());
	m_dbStringTable->SetDBEntity(m_pDataBase.get());
	m_dbTitle = &(CTitleDBProvider::GetInstance());
	m_dbTitle->SetDBEntity(m_pDataBase.get());

	if ((m_pDataBase.get()==0)|| !m_pDataBase->IsValid())
	{
		OUTPUT_LOG("failed open database file %s", fileName.c_str());
	}

	if(m_pDataBase->IsCreateFile())
	{
		m_pDataBase->SetCreateFile(false);
		// TODO: create the default tables here
	}
}

bool CKidsDBProvider::InsertItemRecordFromString(const string& strRecord)
{
	static stItemDbRecord tempItemRecordInsertBuf;
	ParseStringToItemRecord(strRecord, tempItemRecordInsertBuf);
	return CItemDBProvider::GetInstance().InsertItemRecord(tempItemRecordInsertBuf);
}
bool CKidsDBProvider::UpdateItemRecordFromString(const string& strRecord)
{
	static stItemDbRecord tempItemRecordUpdateBuf;
	ParseStringToItemRecord(strRecord, tempItemRecordUpdateBuf);
	return CItemDBProvider::GetInstance().UpdateItemRecord(tempItemRecordUpdateBuf);
}
bool CKidsDBProvider::DeleteItemRecordByID(int ID)
{
	return CItemDBProvider::GetInstance().DeleteItemRecordByID(ID);
}

const char * CKidsDBProvider::SelectItemRecordToString(__int64 ID)
{
	static stItemDbRecord tempItemRecordSelectBuf;
	bool hasRecord = CItemDBProvider::GetInstance().SelectItemRecordByID(tempItemRecordSelectBuf, ID);
	if(!hasRecord)
		tempItemRecordSelectBuf.ID = 0;
	ParseItemRecordToString(tempItemRecordSelectBuf);

	return m_strSelectBuf;
}

const char * CKidsDBProvider::NumToString(__int64 num)
{
	static char numBuf[30];
	sprintf(numBuf, "%I64d", num);
	return numBuf;
}
const char * CKidsDBProvider::NumToString(int num)
{
	static char numBuf[30];
	sprintf(numBuf, "%d", num);
	return numBuf;
}
const char * CKidsDBProvider::NumToString(float num)
{
	static char numBuf[30];
	sprintf(numBuf, "%f", num);
	return numBuf;
}
const char * CKidsDBProvider::NumToString(bool num)
{
	static char numBuf[30];
	if(num == true)
		sprintf(numBuf, "True");
	else
		sprintf(numBuf, "False");
	return numBuf;
}

void CKidsDBProvider::ParseItemRecordToString(const stItemDbRecord& record)
{
	string str;
	str.assign("");

	if(record.ID == 0)
		return ;

	str.append(NumToString(record.ID));

	str.append("#");
	str.append(NumToString(record.ItemName.ID));
	str.append("#");
	str.append(record.ItemName.StringEnglish);
	str.append("#");
	str.append(record.ItemName.StringSimplifiedChinese);
	str.append("#####");

	str.append("#");
	str.append(record.IconAssetName);
	str.append("#");
	str.append(record.DescTextAssetName);
	str.append("#");
	str.append(NumToString(record.IsGlobal));
	str.append("#");
	str.append(record.IconFilePath);

	str.append("#");
	str.append(NumToString(record.Desc.ID));
	str.append("#");
	str.append(record.Desc.StringEnglish);
	str.append("#");
	str.append(record.Desc.StringSimplifiedChinese);
	str.append("#####");


	str.append("#");
	str.append(record.ModelFilePath);
	str.append("#");
	str.append(NumToString(record.ItemType));
	str.append("#");
	str.append(NumToString(record.IsObtained));
	str.append("#");
	str.append(NumToString(record.IsUnique));
	str.append("#");
	str.append(NumToString(record.CostExperiencePt));
	str.append("#");
	str.append(NumToString(record.CostOrange));
	str.append("#");
	str.append(NumToString(record.CostApple));
	str.append("#");
	str.append(NumToString(record.CostPeach));

	str.append("#");
	str.append(record.Reserved1);
	str.append("#");
	str.append(record.Reserved2);
	str.append("#");
	str.append(record.Reserved3);
	str.append("#");
	str.append(record.Reserved4);
	str.append("#");
	str.append(record.Reserved5);

	sprintf(m_strSelectBuf , "%s", str.c_str());
}


// -------------------------------
//		Character database functions
// -------------------------------
bool CKidsDBProvider::InsertCharacterRecordFromString(const string& strRecord)
{
	static stCharacterDbRecord tempCharacterRecordBuf;
	ParseStringToCharacterRecord(strRecord, tempCharacterRecordBuf);
	return CCharacterDBProvider::GetInstance().InsertCharacterRecord(tempCharacterRecordBuf);
}
bool CKidsDBProvider::DeleteCharacterRecordByID(int ID)
{
	return CCharacterDBProvider::GetInstance().DeleteCharacterRecordByID(ID);
}

// -------------------------------
//		Chest database functions
// -------------------------------
bool CKidsDBProvider::InsertChestRecordFromString(const string& strRecord)
{
	static stChestDbRecord tempChestRecordBuf;
	ParseStringToChestRecord(strRecord, tempChestRecordBuf);
	return CChestDBProvider::GetInstance().InsertChestRecord(tempChestRecordBuf);
}
bool CKidsDBProvider::DeleteChestRecordByID(int ID)
{
	return CChestDBProvider::GetInstance().DeleteChestRecordByID(ID);
}

// -------------------------------
//		Fruit database functions
// -------------------------------
bool CKidsDBProvider::InsertFruitRecordFromString(const string& strRecord)
{
	static stFruitDbRecord tempFruitRecordBuf;
	ParseStringToFruitRecord(strRecord, tempFruitRecordBuf);
	return CFruitDBProvider::GetInstance().InsertFruitRecord(tempFruitRecordBuf);
}
bool CKidsDBProvider::DeleteFruitRecordByID(int ID)
{
	return CFruitDBProvider::GetInstance().DeleteFruitRecordByID(ID);
}

// -------------------------------
//		PetAI database functions
// -------------------------------
bool CKidsDBProvider::InsertPetAIRecordFromString(const string& strRecord)
{
	static stPetAIDbRecord tempPetAIRecordBuf;
	ParseStringToPetAIRecord(strRecord, tempPetAIRecordBuf);
	return CPetAIDBProvider::GetInstance().InsertPetAIRecord(tempPetAIRecordBuf);
}
bool CKidsDBProvider::DeletePetAIRecordByID(int ID)
{
	return CPetAIDBProvider::GetInstance().DeletePetAIRecordByID(ID);
}

// -------------------------------
//		Pet database functions
// -------------------------------
bool CKidsDBProvider::InsertPetRecordFromString(const string& strRecord)
{
	static stPetDbRecord tempPetRecordBuf;
	ParseStringToPetRecord(strRecord, tempPetRecordBuf);
	return CPetDBProvider::GetInstance().InsertPetRecord(tempPetRecordBuf);
}
bool CKidsDBProvider::DeletePetRecordByID(int ID)
{
	return CPetDBProvider::GetInstance().DeletePetRecordByID(ID);
}

// -------------------------------
//		Quest database functions
// -------------------------------
bool CKidsDBProvider::InsertQuestRecordFromString(const string& strRecord)
{
	static stQuestDbRecord tempQuestRecordBuf;
	ParseStringToQuestRecord(strRecord, tempQuestRecordBuf);
	return CQuestDBProvider::GetInstance().InsertQuestRecord(tempQuestRecordBuf);
}
bool CKidsDBProvider::DeleteQuestRecordByID(int ID)
{
	return CQuestDBProvider::GetInstance().DeleteQuestRecordByID(ID);
}

// -------------------------------
//		Title database functions
// -------------------------------
bool CKidsDBProvider::InsertTitleRecordFromString(const string& strRecord)
{
	static stTitleDbRecord tempTitleRecordBuf;
	ParseStringToTitleRecord(strRecord, tempTitleRecordBuf);
	return CTitleDBProvider::GetInstance().InsertTitleRecord(tempTitleRecordBuf);
}
bool CKidsDBProvider::DeleteTitleRecordByID(int ID)
{
	return CTitleDBProvider::GetInstance().DeleteTitleRecordByID(ID);
}



void CKidsDBProvider::ParseStringToItemRecord(const string& str, stItemDbRecord& record)
{
	string strID;
	string strIconAssetName;
	string strDescTextAssetName;
	string strIsGlobal; // stored in db 0/1
	string strIconFilePath;
	string strModelFilePath;
	string strItemType;
	string strIsObtained; // stored in db 0/1
	string strIsUnique; // stored in db 0/1
	string strCostExperiencePt; // stored in db 0/1
	string strCostOrange;
	string strCostApple;
	string strCostPeach;
	string strReserved1;
	string strReserved2;
	string strReserved3;
	string strReserved4;
	string strReserved5;

	char backupStr[MAX_STRING_LENGTH];
	size_t i = 0;
	size_t tagFirst = 0;
	size_t tagLast = 0;
	const char * strChar = str.c_str();
	for ( i = 0 ; i < str.size() ; i++)
		backupStr[i] = strChar[i];

	string baseStr;
	baseStr.assign(str);

	tagLast = baseStr.find('#', tagFirst);
	strID.assign(baseStr, tagFirst, tagLast - tagFirst);

	tagFirst = tagLast;
	tagLast += GetStringTableRecordFromString(backupStr + tagFirst, record.ItemName);

	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strIconAssetName.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strDescTextAssetName.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strIsGlobal.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strIconFilePath.assign(baseStr, tagFirst, tagLast - tagFirst);

	tagFirst = tagLast;
	tagLast += GetStringTableRecordFromString(backupStr + tagFirst, record.Desc);

	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strModelFilePath.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strItemType.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strIsObtained.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strIsUnique.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strCostExperiencePt.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strCostOrange.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strCostApple.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strCostPeach.assign(baseStr, tagFirst, tagLast - tagFirst);

	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strReserved1.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strReserved2.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strReserved3.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strReserved4.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strReserved5.assign(baseStr, tagFirst, tagLast - tagFirst);

	record.IconAssetName = strIconAssetName;
	record.DescTextAssetName = strDescTextAssetName;
	record.IconFilePath = strIconFilePath;
	record.ModelFilePath = strModelFilePath;
	record.Reserved1 = strReserved1;
	record.Reserved2 = strReserved2;
	record.Reserved3 = strReserved3;
	record.Reserved4 = strReserved4;
	record.Reserved5 = strReserved5;

	if(strIsGlobal == "True")
		record.IsGlobal = true;
	else
		record.IsGlobal = false;
	if(strIsObtained == "True")
		record.IsObtained = true;
	else
		record.IsObtained = false;
	if(strIsUnique == "True")
		record.IsUnique = true;
	else
		record.IsUnique = false;

	char tempStr[40];
	strChar = strID.c_str();
	sprintf(tempStr, "%s", strID.c_str());
	sscanf(tempStr, "%I64d", &(record.ID));

	sprintf(tempStr, "%s", strItemType.c_str());
	sscanf(tempStr, "%d", &(record.ItemType));
	sprintf(tempStr, "%s", strCostExperiencePt.c_str());
	sscanf(tempStr, "%d", &(record.CostExperiencePt));
	sprintf(tempStr, "%s", strCostOrange.c_str());
	sscanf(tempStr, "%d", &(record.CostOrange));
	sprintf(tempStr, "%s", strCostApple.c_str());
	sscanf(tempStr, "%d", &(record.CostApple));
	sprintf(tempStr, "%s", strCostPeach.c_str());
	sscanf(tempStr, "%d", &(record.CostPeach));
}



void CKidsDBProvider::ParseStringToCharacterRecord(const string& str, stCharacterDbRecord& record)
{
	/** Common Attribute */
	string strID;
	string strNumberOfPets;
	string strMainPetIndex;
	string strSecondaryPetIndex;
	string strThirdPetIndex;

	/** Engine Asset Management Attribute */
	string strAssetName;
	string strIsGlobal;
	string strIsSnapToTerrain;

	/** Appearance Related Attribute */
	string strCharacterModeFilePath;
	string strCharacterModelScale;
	string strCharacterRadius;
	string strCharacterFacing;
	string strposX;
	string strposY;
	string strposZ;

	/** AI Related Attribute */
	string strPerceptiveRadius;
	string strSentientRadius;
	string strOnLoadScript;

	/** Kids Game Related Attribute */
	string strCharacterTitleIndex;	// index in title table
	string strExperiencePt;
	string strOrange;
	string strApple;
	string strPeach;

	/** Reserved Attribute */
	string strReserved1;
	string strReserved2;
	string strReserved3;
	string strReserved4;
	string strReserved5;

	char backupStr[MAX_STRING_LENGTH];
	size_t i = 0;
	size_t tagFirst = 0;
	size_t tagLast = 0;
	const char * strChar = str.c_str();
	for ( i = 0 ; i < str.size() ; i++)
		backupStr[i] = strChar[i];

	string baseStr;
	baseStr.assign(str);

	tagLast = baseStr.find('#', tagFirst);
	strID.assign(baseStr, tagFirst, tagLast - tagFirst);

	tagFirst = tagLast;
	tagLast += GetStringTableRecordFromString(backupStr + tagFirst, record.CharacterName);

	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strNumberOfPets.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strMainPetIndex.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strSecondaryPetIndex.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strThirdPetIndex.assign(baseStr, tagFirst, tagLast - tagFirst);

	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strAssetName.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strIsGlobal.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strIsSnapToTerrain.assign(baseStr, tagFirst, tagLast - tagFirst);

	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strCharacterModeFilePath.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strCharacterModelScale.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strCharacterRadius.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strCharacterFacing.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strposX.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strposY.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strposZ.assign(baseStr, tagFirst, tagLast - tagFirst);

	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strPerceptiveRadius.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strSentientRadius.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strOnLoadScript.assign(baseStr, tagFirst, tagLast - tagFirst);

	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strCharacterTitleIndex.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strExperiencePt.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strOrange.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strApple.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strPeach.assign(baseStr, tagFirst, tagLast - tagFirst);

	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strReserved1.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strReserved2.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strReserved3.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strReserved4.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strReserved5.assign(baseStr, tagFirst, tagLast - tagFirst);


	record.AssetName = strAssetName;
	record.CharacterModeFilePath = strCharacterModeFilePath;
	record.OnLoadScript = strOnLoadScript;
	record.Reserved1 = strReserved1;
	record.Reserved2 = strReserved2;
	record.Reserved3 = strReserved3;
	record.Reserved4 = strReserved4;
	record.Reserved5 = strReserved5;

	if(strIsGlobal == "True")
		record.IsGlobal = true;
	else
		record.IsGlobal = false;
	if(strIsSnapToTerrain == "True")
		record.IsSnapToTerrain = true;
	else
		record.IsSnapToTerrain = false;

	char tempStr[40];
	strChar = strID.c_str();
	sprintf(tempStr, "%s", strID.c_str());
	sscanf(tempStr, "%I64d", &(record.ID));

	sprintf(tempStr, "%s", strNumberOfPets.c_str());
	sscanf(tempStr, "%d", &(record.NumberOfPets));
	sprintf(tempStr, "%s", strMainPetIndex.c_str());
	sscanf(tempStr, "%I64d", &(record.MainPetIndex));
	sprintf(tempStr, "%s", strSecondaryPetIndex.c_str());
	sscanf(tempStr, "%I64d", &(record.SecondaryPetIndex));
	sprintf(tempStr, "%s", strThirdPetIndex.c_str());
	sscanf(tempStr, "%I64d", &(record.ThirdPetIndex));

	sprintf(tempStr, "%s", strCharacterModelScale.c_str());
	sscanf(tempStr, "%f", &(record.CharacterModelScale));
	sprintf(tempStr, "%s", strCharacterRadius.c_str());
	sscanf(tempStr, "%f", &(record.CharacterRadius));
	sprintf(tempStr, "%s", strCharacterFacing.c_str());
	sscanf(tempStr, "%f", &(record.CharacterFacing));
	sprintf(tempStr, "%s", strposX.c_str());
	sscanf(tempStr, "%f", &(record.posX));
	sprintf(tempStr, "%s", strposY.c_str());
	sscanf(tempStr, "%f", &(record.posY));
	sprintf(tempStr, "%s", strposZ.c_str());
	sscanf(tempStr, "%f", &(record.posZ));

	sprintf(tempStr, "%s", strPerceptiveRadius.c_str());
	sscanf(tempStr, "%f", &(record.PerceptiveRadius));
	sprintf(tempStr, "%s", strSentientRadius.c_str());
	sscanf(tempStr, "%f", &(record.SentientRadius));

	sprintf(tempStr, "%s", strCharacterTitleIndex.c_str());
	sscanf(tempStr, "%I64d", &(record.CharacterTitleIndex));
	sprintf(tempStr, "%s", strExperiencePt.c_str());
	sscanf(tempStr, "%d", &(record.ExperiencePt));
	sprintf(tempStr, "%s", strOrange.c_str());
	sscanf(tempStr, "%d", &(record.Orange));
	sprintf(tempStr, "%s", strApple.c_str());
	sscanf(tempStr, "%d", &(record.Apple));
	sprintf(tempStr, "%s", strPeach.c_str());
	sscanf(tempStr, "%d", &(record.Peach));
}
void CKidsDBProvider::ParseStringToChestRecord(const string& str, stChestDbRecord& record)
{

	/** Common Attribute */
	string strID;

	/** Engine Asset Management Attribute */
	string strAssetName;
	string strIsSnapToTerrain;

	/** Appearance Related Attribute */
	string strChestModeFilePath;
	string strChestModelScale;
	string strChestRadius;
	string strChestFacing;
	string strposX;
	string strposY;
	string strposZ;

	/** Difficulty Attribute */
	string strDifficultyRate;
	string strNumberOfAllPuzzles;
	string strNumberOfPassPuzzles;

	/** Reward Attribute */
	string strReward;

	/** Reserved Attribute */
	string strReserved1;
	string strReserved2;
	string strReserved3;
	string strReserved4;
	string strReserved5;

	char backupStr[MAX_STRING_LENGTH];
	size_t i = 0;
	size_t tagFirst = 0;
	size_t tagLast = 0;
	const char * strChar = str.c_str();
	for ( i = 0 ; i < str.size() ; i++)
		backupStr[i] = strChar[i];

	string baseStr;
	baseStr.assign(str);

	tagLast = baseStr.find('#', tagFirst);
	strID.assign(baseStr, tagFirst, tagLast - tagFirst);

	tagFirst = tagLast;
	tagLast += GetStringTableRecordFromString(backupStr + tagFirst, record.ChestName);

	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strAssetName.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strIsSnapToTerrain.assign(baseStr, tagFirst, tagLast - tagFirst);

	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strChestModeFilePath.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strChestModelScale.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strChestRadius.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strChestFacing.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strposX.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strposY.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strposZ.assign(baseStr, tagFirst, tagLast - tagFirst);

	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strDifficultyRate.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strNumberOfAllPuzzles.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strNumberOfPassPuzzles.assign(baseStr, tagFirst, tagLast - tagFirst);

	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strReward.assign(baseStr, tagFirst, tagLast - tagFirst);

	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strReserved1.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strReserved2.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strReserved3.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strReserved4.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strReserved5.assign(baseStr, tagFirst, tagLast - tagFirst);

	record.AssetName = strAssetName;
	record.ChestModeFilePath = strChestModeFilePath;
	record.Reward = strReward;
	record.Reserved1 = strReserved1;
	record.Reserved2 = strReserved2;
	record.Reserved3 = strReserved3;
	record.Reserved4 = strReserved4;
	record.Reserved5 = strReserved5;

	if(strIsSnapToTerrain == "True")
		record.IsSnapToTerrain = true;
	else
		record.IsSnapToTerrain = false;

	char tempStr[40];
	strChar = strID.c_str();
	sprintf(tempStr, "%s", strID.c_str());
	sscanf(tempStr, "%I64d", &(record.ID));

	sprintf(tempStr, "%s", strChestModelScale.c_str());
	sscanf(tempStr, "%f", &(record.ChestModelScale));
	sprintf(tempStr, "%s", strChestRadius.c_str());
	sscanf(tempStr, "%f", &(record.ChestRadius));
	sprintf(tempStr, "%s", strChestFacing.c_str());
	sscanf(tempStr, "%f", &(record.ChestFacing));
	sprintf(tempStr, "%s", strposX.c_str());
	sscanf(tempStr, "%f", &(record.posX));
	sprintf(tempStr, "%s", strposY.c_str());
	sscanf(tempStr, "%f", &(record.posY));
	sprintf(tempStr, "%s", strposZ.c_str());
	sscanf(tempStr, "%f", &(record.posZ));

	sprintf(tempStr, "%s", strDifficultyRate.c_str());
	sscanf(tempStr, "%d", &(record.DifficultyRate));
	sprintf(tempStr, "%s", strNumberOfAllPuzzles.c_str());
	sscanf(tempStr, "%d", &(record.NumberOfAllPuzzles));
	sprintf(tempStr, "%s", strNumberOfPassPuzzles.c_str());
	sscanf(tempStr, "%d", &(record.NumberOfPassPuzzles));

}
void CKidsDBProvider::ParseStringToFruitRecord(const string& str, stFruitDbRecord& record)
{
	/** Common Attribute */
	string strID;

	/** Engine Asset Management Attribute */
	string strAssetName;
	string strIsSnapToTerrain;

	/** Appearance Related Attribute */
	string strFruitModeFilePath;
	string strFruitModelScale;
	string strFruitRadius;
	string strFruitFacing;
	string strposX;
	string strposY;
	string strposZ;

	/** Reward Attribute */
	string strReward;

	/** Reserved Attribute */
	string strReserved1;
	string strReserved2;
	string strReserved3;
	string strReserved4;
	string strReserved5;

	char backupStr[MAX_STRING_LENGTH];
	size_t i = 0;
	size_t tagFirst = 0;
	size_t tagLast = 0;
	const char * strChar = str.c_str();
	for ( i = 0 ; i < str.size() ; i++)
		backupStr[i] = strChar[i];

	string baseStr;
	baseStr.assign(str);

	tagLast = baseStr.find('#', tagFirst);
	strID.assign(baseStr, tagFirst, tagLast - tagFirst);

	tagFirst = tagLast;
	tagLast += GetStringTableRecordFromString(backupStr + tagFirst, record.FruitName);

	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strAssetName.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strIsSnapToTerrain.assign(baseStr, tagFirst, tagLast - tagFirst);

	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strFruitModeFilePath.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strFruitModelScale.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strFruitRadius.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strFruitFacing.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strposX.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strposY.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strposZ.assign(baseStr, tagFirst, tagLast - tagFirst);

	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strReward.assign(baseStr, tagFirst, tagLast - tagFirst);

	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strReserved1.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strReserved2.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strReserved3.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strReserved4.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strReserved5.assign(baseStr, tagFirst, tagLast - tagFirst);


	record.AssetName = strAssetName;
	record.FruitModeFilePath = strFruitModeFilePath;
	record.Reward = strReward;
	record.Reserved1 = strReserved1;
	record.Reserved2 = strReserved2;
	record.Reserved3 = strReserved3;
	record.Reserved4 = strReserved4;
	record.Reserved5 = strReserved5;

	if(strIsSnapToTerrain == "True")
		record.IsSnapToTerrain = true;
	else
		record.IsSnapToTerrain = false;

	char tempStr[40];
	strChar = strID.c_str();
	sprintf(tempStr, "%s", strID.c_str());
	sscanf(tempStr, "%I64d", &(record.ID));

	sprintf(tempStr, "%s", strFruitModelScale.c_str());
	sscanf(tempStr, "%f", &(record.FruitModelScale));
	sprintf(tempStr, "%s", strFruitRadius.c_str());
	sscanf(tempStr, "%f", &(record.FruitRadius));
	sprintf(tempStr, "%s", strFruitFacing.c_str());
	sscanf(tempStr, "%f", &(record.FruitFacing));
	sprintf(tempStr, "%s", strposX.c_str());
	sscanf(tempStr, "%f", &(record.posX));
	sprintf(tempStr, "%s", strposY.c_str());
	sscanf(tempStr, "%f", &(record.posY));
	sprintf(tempStr, "%s", strposZ.c_str());
	sscanf(tempStr, "%f", &(record.posZ));
}
void CKidsDBProvider::ParseStringToPetAIRecord(const string& str, stPetAIDbRecord& record)
{
	/** Common Attribute */
	string strID;
	string strPetAIName;
	string strPetAIDesc;

	/** Reserved Attribute */
	string strReserved1;
	string strReserved2;
	string strReserved3;
	string strReserved4;
	string strReserved5;

	char backupStr[MAX_STRING_LENGTH];
	size_t i = 0;
	size_t tagFirst = 0;
	size_t tagLast = 0;
	const char * strChar = str.c_str();
	for ( i = 0 ; i < str.size() ; i++)
		backupStr[i] = strChar[i];

	string baseStr;
	baseStr.assign(str);

	tagLast = baseStr.find('#', tagFirst);
	strID.assign(baseStr, tagFirst, tagLast - tagFirst);

	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strPetAIName.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strPetAIDesc.assign(baseStr, tagFirst, tagLast - tagFirst);

	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strReserved1.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strReserved2.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strReserved3.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strReserved4.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strReserved5.assign(baseStr, tagFirst, tagLast - tagFirst);

	record.PetAIName = strPetAIName;
	record.PetAIDesc = strPetAIDesc;
	record.Reserved1 = strReserved1;
	record.Reserved2 = strReserved2;
	record.Reserved3 = strReserved3;
	record.Reserved4 = strReserved4;
	record.Reserved5 = strReserved5;

	char tempStr[40];
	strChar = strID.c_str();
	sprintf(tempStr, "%s", strID.c_str());
	sscanf(tempStr, "%I64d", &(record.ID));

}
void CKidsDBProvider::ParseStringToPetRecord(const string& str, stPetDbRecord& record)
{
	/** Common Attribute */
	string strID;

	/** Engine Asset Management Attribute */
	string strAssetName;
	string strIsGlobal;
	string strIsSnapToTerrain;

	/** Appearance Related Attribute */
	string strPetModeFilePath;
	string strPetModelScale;
	string strPetRadius;
	string strPetFacing;
	string strposX;
	string strposY;
	string strposZ;

	/** AI Related Attribute */
	string strPetAIEntryIndexArray;
	string strPerceptiveRadius;
	string strSentientRadius;
	string strOnLoadScript;

	/** Kids Game Related Attribute */
	string strIsKeptinCage;

	string strReserved1;
	string strReserved2;
	string strReserved3;
	string strReserved4;
	string strReserved5;

	char backupStr[MAX_STRING_LENGTH];
	size_t i = 0;
	size_t tagFirst = 0;
	size_t tagLast = 0;
	const char * strChar = str.c_str();
	for ( i = 0 ; i < str.size() ; i++)
		backupStr[i] = strChar[i];

	string baseStr;
	baseStr.assign(str);

	tagLast = baseStr.find('#', tagFirst);
	strID.assign(baseStr, tagFirst, tagLast - tagFirst);

	tagFirst = tagLast;
	tagLast += GetStringTableRecordFromString(backupStr + tagFirst, record.PetName);

	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strAssetName.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strIsGlobal.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strIsSnapToTerrain.assign(baseStr, tagFirst, tagLast - tagFirst);

	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strPetModeFilePath.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strPetModelScale.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strPetRadius.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strPetFacing.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strposX.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strposY.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strposZ.assign(baseStr, tagFirst, tagLast - tagFirst);

	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strPetAIEntryIndexArray.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strPerceptiveRadius.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strSentientRadius.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strOnLoadScript.assign(baseStr, tagFirst, tagLast - tagFirst);

	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strIsKeptinCage.assign(baseStr, tagFirst, tagLast - tagFirst);

	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strReserved1.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strReserved2.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strReserved3.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strReserved4.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strReserved5.assign(baseStr, tagFirst, tagLast - tagFirst);

	record.AssetName = strAssetName;
	record.PetModeFilePath = strPetModeFilePath;
	record.OnLoadScript = strOnLoadScript;
	record.Reserved1 = strReserved1;
	record.Reserved2 = strReserved2;
	record.Reserved3 = strReserved3;
	record.Reserved4 = strReserved4;
	record.Reserved5 = strReserved5;

	// parse PetAIEntryIndexArray to vector<__int64> PetAIIndexVector
	// NOTE: record in database FORMAT "ID,ID,ID,......,ID"
	basic_string <char>::size_type indexBegin = 0;
	basic_string <char>::size_type indexEnd = 0;
	
	string temp;
	char tempScan[40];
	__int64 tempID;
	while(strPetAIEntryIndexArray.size() != 0)
	{
		indexEnd = strPetAIEntryIndexArray.find_first_of(',', indexEnd);
		temp.assign(strPetAIEntryIndexArray, indexBegin, indexEnd - indexBegin);
		sprintf(tempScan, "%s", temp.c_str());
		sscanf(tempScan, "%I64d", &tempID);
		record.PetAIIndexVector.push_back(tempID);

		if(indexEnd == string::npos)
			break;
		indexEnd++;
		indexBegin = indexEnd;
		// TODO: FULL TEST
	}

	if(strIsGlobal == "True")
		record.IsGlobal = true;
	else
		record.IsGlobal = false;
	if(strIsSnapToTerrain == "True")
		record.IsSnapToTerrain = true;
	else
		record.IsSnapToTerrain = false;
	if(strIsKeptinCage == "True")
		record.IsKeptinCage = true;
	else
		record.IsKeptinCage = false;

	char tempStr[40];
	strChar = strID.c_str();
	sprintf(tempStr, "%s", strID.c_str());
	sscanf(tempStr, "%I64d", &(record.ID));

	sprintf(tempStr, "%s", strPetModelScale.c_str());
	sscanf(tempStr, "%f", &(record.PetModelScale));
	sprintf(tempStr, "%s", strPetRadius.c_str());
	sscanf(tempStr, "%f", &(record.PetRadius));
	sprintf(tempStr, "%s", strPetFacing.c_str());
	sscanf(tempStr, "%f", &(record.PetFacing));
	sprintf(tempStr, "%s", strposX.c_str());
	sscanf(tempStr, "%f", &(record.posX));
	sprintf(tempStr, "%s", strposY.c_str());
	sscanf(tempStr, "%f", &(record.posY));
	sprintf(tempStr, "%s", strposZ.c_str());
	sscanf(tempStr, "%f", &(record.posZ));

	sprintf(tempStr, "%s", strPerceptiveRadius.c_str());
	sscanf(tempStr, "%f", &(record.PerceptiveRadius));
	sprintf(tempStr, "%s", strSentientRadius.c_str());
	sscanf(tempStr, "%f", &(record.SentientRadius));
}
void CKidsDBProvider::ParseStringToQuestRecord(const string& str, stQuestDbRecord& record)
{
	/** Common Attribute */
	string strID;

	/** Engine Asset Management Attribute */
	string strTopSnapShotAssetName;
	string strIntroVideoAssetName;
	string strIsGlobal;

	/** UI Related Attribute */
	string strTopSnapShotFilePath;
	string strIntroVideoFilePath;

	/** Quest Content Attribute */
	string strIsComplete;
	string strAcquireQuestRequirement;
	string strCompleteQuestRequirement;
	string strReward;

	/** Reserved Attribute */
	string strReserved1;
	string strReserved2;
	string strReserved3;
	string strReserved4;
	string strReserved5;

	char backupStr[MAX_STRING_LENGTH];
	size_t i = 0;
	size_t tagFirst = 0;
	size_t tagLast = 0;
	const char * strChar = str.c_str();
	for ( i = 0 ; i < str.size() ; i++)
		backupStr[i] = strChar[i];

	string baseStr;
	baseStr.assign(str);

	tagLast = baseStr.find('#', tagFirst);
	strID.assign(baseStr, tagFirst, tagLast - tagFirst);

	tagFirst = tagLast;
	tagLast += GetStringTableRecordFromString(backupStr + tagFirst, record.QuestName);

	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strTopSnapShotAssetName.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strIntroVideoAssetName.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strIsGlobal.assign(baseStr, tagFirst, tagLast - tagFirst);

	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strTopSnapShotFilePath.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strIntroVideoFilePath.assign(baseStr, tagFirst, tagLast - tagFirst);

	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strIsComplete.assign(baseStr, tagFirst, tagLast - tagFirst);

	tagFirst = tagLast;
	tagLast += GetStringTableRecordFromString(backupStr + tagFirst, record.QuestDesc);

	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strAcquireQuestRequirement.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strCompleteQuestRequirement.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strReward.assign(baseStr, tagFirst, tagLast - tagFirst);

	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strReserved1.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strReserved2.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strReserved3.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strReserved4.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strReserved5.assign(baseStr, tagFirst, tagLast - tagFirst);

	record.TopSnapShotAssetName = strTopSnapShotAssetName;
	record.IntroVideoAssetName = strIntroVideoAssetName;
	record.TopSnapShotFilePath = strTopSnapShotFilePath;
	record.IntroVideoFilePath = strIntroVideoFilePath;
	record.AcquireQuestRequirement = strAcquireQuestRequirement;
	record.CompleteQuestRequirement = strCompleteQuestRequirement;
	record.Reward = strReward;
	record.Reserved1 = strReserved1;
	record.Reserved2 = strReserved2;
	record.Reserved3 = strReserved3;
	record.Reserved4 = strReserved4;
	record.Reserved5 = strReserved5;

	if(strIsGlobal == "True")
		record.IsGlobal = true;
	else
		record.IsGlobal = false;
	if(strIsComplete == "True")
		record.IsComplete = true;
	else
		record.IsComplete = false;

	char tempStr[40];
	strChar = strID.c_str();
	sprintf(tempStr, "%s", strID.c_str());
	sscanf(tempStr, "%I64d", &(record.ID));

}
void CKidsDBProvider::ParseStringToTitleRecord(const string& str, stTitleDbRecord& record)
{
	/** Common Attribute */
	string strID;
	string strTitleLevel;
	string strIsBoy;

	/** Reserved Attribute */
	string strReserved1;
	string strReserved2;
	string strReserved3;
	string strReserved4;
	string strReserved5;

	char backupStr[MAX_STRING_LENGTH];
	size_t i = 0;
	size_t tagFirst = 0;
	size_t tagLast = 0;
	const char * strChar = str.c_str();
	for ( i = 0 ; i < str.size() ; i++)
		backupStr[i] = strChar[i];

	string baseStr;
	baseStr.assign(str);

	tagLast = baseStr.find('#', tagFirst);
	strID.assign(baseStr, tagFirst, tagLast - tagFirst);

	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strTitleLevel.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strIsBoy.assign(baseStr, tagFirst, tagLast - tagFirst);

	tagFirst = tagLast;
	tagLast += GetStringTableRecordFromString(backupStr + tagFirst, record.Title);

	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strReserved1.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strReserved2.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strReserved3.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strReserved4.assign(baseStr, tagFirst, tagLast - tagFirst);
	tagFirst = tagLast + 1;
	tagLast = baseStr.find('#', tagFirst);
	strReserved5.assign(baseStr, tagFirst, tagLast - tagFirst);

	record.Reserved1 = strReserved1;
	record.Reserved2 = strReserved2;
	record.Reserved3 = strReserved3;
	record.Reserved4 = strReserved4;
	record.Reserved5 = strReserved5;

	if(strIsBoy == "True")
		record.IsBoy = true;
	else
		record.IsBoy = false;

	char tempStr[40];
	strChar = strID.c_str();
	sprintf(tempStr, "%s", strID.c_str());
	sscanf(tempStr, "%I64d", &(record.ID));

	sprintf(tempStr, "%s", strTitleLevel.c_str());
	sscanf(tempStr, "%d", &(record.TitleLevel));
}


void CKidsDBProvider::ParseCharacterRecordToString(string& str, const stCharacterDbRecord& record)
{
}
void CKidsDBProvider::ParseChestRecordToString(string& str, const stChestDbRecord& record)
{
}
void CKidsDBProvider::ParseFruitRecordToString(string& str, const stFruitDbRecord& record)
{
}
void CKidsDBProvider::ParsePetAIRecordToString(string& str, const stPetAIDbRecord& record)
{
}
void CKidsDBProvider::ParsePetRecordToString(string& str, const stPetDbRecord& record)
{
}
void CKidsDBProvider::ParseQuestRecordToString(string& str, const stQuestDbRecord& record)
{
}
void CKidsDBProvider::ParseTitleRecordToString(string& str, const stTitleDbRecord& record)
{
}
//-----------------------------------------------------------------------------
// Class:	CStringTableDB
// Authors:	Andy Wang
// Emails: mitnick_wang@hotmail.com
// Date:	2006.8.9
// @2006.8.24 LXZ: use stored procedure for commonly used functions. 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "InfoCenter/ICDBManager.h"
#include "InfoCenter/ICRecordSet.h"

#include "StringTable.h"

using namespace ParaInfoCenter;
using namespace ParaEngine;



CStringTableDB::CStringTableDB() : m_pDataBase(NULL), m_pProcSelectByID_EN(NULL), m_pProcSelectByID_CN(NULL)
{
}
CStringTableDB::CStringTableDB(ParaInfoCenter::DBEntity* pDb)
:m_pProcSelectByID_EN(NULL), m_pProcSelectByID_CN(NULL)
{
	m_pDataBase = pDb;
}
CStringTableDB::~CStringTableDB()
{
}

void CStringTableDB::Cleanup()
{
	SAFE_RELEASE(m_pProcSelectByID_EN);
	SAFE_RELEASE(m_pProcSelectByID_CN);
}

void CStringTableDB::TestDB()
{
	bool returnValue = false;
	string str;
	stStringTableDbRecord entry;

	entry.ID = 0;
	entry.StringEnglish = "Come";
	entry.StringSimplifiedChinese = "On";
	//returnValue = InsertString(entry);

	entry.ID = 0;
	entry.StringEnglish = "Go";
	entry.StringSimplifiedChinese = "Away";
	//returnValue = InsertString(entry);

	returnValue = GetStringbyID(str, 1);
	returnValue = GetStringbyID(str, 2);

	entry.ID = 1;
	entry.StringEnglish = "Andy";
	entry.StringSimplifiedChinese = "Roddick";
	returnValue = UpdateStringTable(entry);

	entry.ID = 2;
	entry.StringEnglish = "Carlos";
	entry.StringSimplifiedChinese = "Moya";
	returnValue = UpdateStringTable(entry);

	returnValue = DeleteEntryByID(1);
	returnValue = DeleteEntryByID(2);
}

bool CStringTableDB::GetStringbyID(string& sString, const __int64& ID)
{
	DBEntity* pDB = GetDBEntity();
	assert(pDB!=NULL);
	bool bSucceed = false;
	CICRecordSet* rs = NULL;
	
	try
	{
		// switch language string according to language setting
		switch(CGlobals::GetDataProviderManager()->GetLocalGameLanguage())
		{
		case ENGLISH:
			{
				if(m_pProcSelectByID_EN==NULL)
				{
					m_pProcSelectByID_EN =  pDB->CreateRecordSet("SELECT StringTable_DB.StringEnglish\
											   FROM StringTable_DB\
											   WHERE StringTable_DB.ID = @ID");
				}
				rs =  m_pProcSelectByID_EN;
				break;
			}
			
		case SIMPLIFIEDCHINESE:
			{
				if(m_pProcSelectByID_CN==NULL)
				{
					rs =  pDB->CreateRecordSet("SELECT StringTable_DB.StringSimplifiedChinese\
											   FROM StringTable_DB\
											   WHERE StringTable_DB.ID = @ID");
				}
				rs =  m_pProcSelectByID_CN;
				break;
			}
#ifdef USE_RESERVED1
		case RESERVED1:
			rs =  pDB->CreateRecordSet("SELECT StringTable_DB.Reserved1\
									   FROM StringTable_DB\
									   WHERE StringTable_DB.ID = @ID");
			break;
#endif
		default:
			OUTPUT_LOG("Unsupported local language.\r\n");
			break;
		}
		if(rs!=0)
		{
			rs->DataBinding("@ID", ID);
			bSucceed = (rs->NextRow()==0);
			if(bSucceed)
			{
				sString = (const char*)(*rs)[0];
			}
			rs->Reset();
		}
	}
	catch (CICSQLException& e)
	{
		OUTPUT_LOG("%s\n", e.errmsg.c_str());
	}
	
	return bSucceed;
}

bool CStringTableDB::InsertString(stStringTableDbRecord& record)
{
	DBEntity* pDB = GetDBEntity();
	assert(pDB!=NULL);
	bool bSucceed = false;
	CICRecordSet* rs = NULL;

	rs = pDB->CreateRecordSet1("INSERT INTO StringTable_DB \
							   ([ID], StringEnglish, StringSimplifiedChinese, Reserved1, Reserved2, Reserved3, Reserved4, Reserved5)\
							   VALUES \
							   (NULL, @StringEnglish, @StringSimplifiedChinese, @Reserved1, @Reserved2, @Reserved3, @Reserved4, @Reserved5)");
	try
	{
		rs->DataBinding("@StringEnglish", record.StringEnglish.c_str());
		rs->DataBinding("@StringSimplifiedChinese", record.StringSimplifiedChinese.c_str());
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

bool CStringTableDB::UpdateStringTable(const stStringTableDbRecord& record)
{
	DBEntity* pDB = GetDBEntity();
	assert(pDB!=NULL);
	bool bSucceed = false;
	CICRecordSet* rs = NULL;
	rs = pDB->CreateRecordSet1("UPDATE    StringTable_DB\
										SET	StringEnglish = @StringEnglish, StringSimplifiedChinese = @StringSimplifiedChinese, \
										Reserved1 = @Reserved1, Reserved2 = @Reserved2, Reserved3 = @Reserved3, \
										Reserved4 = @Reserved4, Reserved5 = @Reserved5 \
										WHERE     ([ID] = @ID)");
	try
	{
		rs->DataBinding("@ID", record.ID);
		rs->DataBinding("@StringEnglish", record.StringEnglish.c_str());
		rs->DataBinding("@StringSimplifiedChinese", record.StringSimplifiedChinese.c_str());
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
	rs->Release();

	return bSucceed;
}

bool CStringTableDB::DeleteEntryByID(const __int64& ID)
{
	DBEntity* pDB = GetDBEntity();
	assert(pDB!=NULL);
	bool bSucceed = false;
	CICRecordSet* rs = NULL;

	try
	{
		rs = pDB->CreateRecordSet1("DELETE FROM StringTable_DB  WHERE  ([ID] = @ID)");
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

CStringTableDB& CStringTableDB::GetInstance()
{
	static CStringTableDB g_singletonStringTableDB;
	return g_singletonStringTableDB;
}

ParaInfoCenter::DBEntity* CStringTableDB::GetDBEntity()
{
	return m_pDataBase;
}

void CStringTableDB::SetDBEntity(ParaInfoCenter::DBEntity* pDb)
{
	m_pDataBase = pDb;
}
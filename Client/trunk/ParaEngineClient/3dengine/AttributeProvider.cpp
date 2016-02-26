//-----------------------------------------------------------------------------
// Class:	AttributeProvider
// Authors:	Li, Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2006.6.7
// Note: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ic/ICDBManager.h"
#include "ic/ICRecordSet.h"
#include "AttributeProvider.h"

using namespace ParaInfoCenter;
using namespace ParaEngine;

/** perform test cases when reseting db.*/
//#define TEST_DB

AttributeProvider::AttributeProvider(void)
:m_pProcSelectAttribute(NULL),
m_pProcAttributeExists(NULL),
m_pProcUpdateAttribute(NULL),
m_pProcInsertAttribute(NULL),
m_pProcDeleteAttribute(NULL)
{
}

AttributeProvider::~AttributeProvider(void)
{
	Cleanup();
}

void AttributeProvider::ReleaseStoredProcedures()
{
	SAFE_RELEASE(m_pProcAttributeExists);
	SAFE_RELEASE(m_pProcSelectAttribute);
	SAFE_RELEASE(m_pProcUpdateAttribute);
	SAFE_RELEASE(m_pProcInsertAttribute);
	SAFE_RELEASE(m_pProcDeleteAttribute);
}
void AttributeProvider::Cleanup()
{
	m_sCurrentTableName.clear();
	m_pDataBase.reset();
	ReleaseStoredProcedures();
}

void AttributeProvider::SetDBEntity(const string& sConnectionstring)
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

	m_pDataBase = CICDBManager::OpenDBEntity("attribute", fileName.c_str());
	if (m_pDataBase.get() == 0 || !m_pDataBase->IsValid())
	{
		OUTPUT_LOG("failed open database file %s", fileName.c_str());
		return;
	}
	if(m_pDataBase->IsCreateFile())
	{
		m_pDataBase->SetCreateFile(false);
		// TODO: create the default tables here
	}
}

bool AttributeProvider::ResetTable(const char*  sName)
{
	DBEntity* pDB = GetDBEntity();
	if(pDB==NULL || !pDB->IsValid()) 
		return false;
	char sql_create_attribute_table[MAX_SQL_LENGTH + 1];
	sql_create_attribute_table[MAX_SQL_LENGTH] = '\0';
	snprintf(sql_create_attribute_table, MAX_SQL_LENGTH,
		"DROP TABLE IF EXISTS [%s];", sName);
	pDB->ExecuteSqlScript(sql_create_attribute_table);
	// SQLITE_DONE is returned. 

	SetTableName(sName);
	return true;
}

void AttributeProvider::ValidateDatabase()
{
	PE_ASSERT(m_pDataBase.get()!=NULL);
	//m_pDataBase->prepare_sql("")
	// currently it does nothing, there does not seem to be an automatic way to check for table existence in sqlite3. However,
	// can manually create a database called sys.table etc for keeping meta data in the database, but there is a cost of maintaineous anyway.
}
#ifdef _DEBUG
void AttributeProvider::TestDB()
{
	bool bResult = false;
	string sValue;
	bResult = ResetTable("test");
	// insertion test
	bResult = InsertAttribute("key1", "value1");
	bResult = InsertAttribute("key2", "value2");
	
	// update and select test
	bResult = GetAttribute("key1", sValue);
	bResult = UpdateAttribute("key1", "value1 Updated");
	bResult = GetAttribute("key1", sValue);

	// using update to insert.
	bResult = UpdateAttribute("key3", "value3");
	bResult = GetAttribute("key3", sValue);

	// delete test
	bResult = DeleteAttribute("key1");
	bResult = GetAttribute("key1", sValue);

	// multiple tables
	bResult = ResetTable("test1");
	bResult = InsertAttribute("anotherkey1", "anothervalue1");
	bResult = InsertAttribute("anotherkey2", "anothervalue2");
	
	SetTableName("test");
	bResult = GetAttribute("key1", sValue);
	bResult = GetAttribute("key2", sValue);
}
#endif

bool AttributeProvider::SaveAttributeObject(const char*  sTableName, IAttributeFields* pAttributeObject)
{
	//TODO
	return false;
}

bool AttributeProvider::LoadAttributeObject(const char*  sTableName, IAttributeFields* pAttributeObject)
{
	//TODO
	return false;
}
bool AttributeProvider::DoesAttributeExists(const char*  sName)
{
	DBEntity* pDB = GetDBEntity();
	PE_ASSERT(pDB!=NULL);
	bool bSucceed = false;
	int nCount = 0;
	if(m_pProcAttributeExists == NULL)
	{
		// create the SQL procedure
		m_pProcAttributeExists = pDB->CreateRecordSet("SELECT  COUNT(*)	FROM %s WHERE     (Name = @Name)", 
			m_sCurrentTableName.c_str());
	}
	if (m_pProcAttributeExists && m_pProcAttributeExists->IsValid())
	{
		CICRecordSet* rs = m_pProcAttributeExists;

		try
		{
			rs->DataBinding("@Name", sName);

			bool bHasRecord = (rs->NextRow()==0);
			if(bHasRecord)
			{
				int i=0;
				CICRecordSetItem& item = (*rs)[0];
				nCount = (int)item;
				bSucceed = true;
			}			
		}
		catch (CICSQLException& e)
		{
			OUTPUT_LOG("%s", e.errmsg.c_str());
		}
		rs->Reset();
	}
	return nCount > 0;
}
bool AttributeProvider::GetAttribute(const char*  sName, string& sOut)
{
	static CAttributeDbItem g_temp;
	g_temp.m_sName = sName;
	if(GetAttribute(g_temp))
	{
		sOut = g_temp.m_sValue;
		return true;
	}
	return false;
}

bool AttributeProvider::GetAttribute(CAttributeDbItem& inout)
{
	DBEntity* pDB = GetDBEntity();
	PE_ASSERT(pDB!=NULL);
	bool bSucceed = false;
	if(m_pProcSelectAttribute == NULL)
	{
		// create the SQL procedure
		m_pProcSelectAttribute = pDB->CreateRecordSet("SELECT  [Type], [Value], [Desc], [Schematics]	FROM %s WHERE     (Name = @Name)", 
			m_sCurrentTableName.c_str());
	}
	if (m_pProcSelectAttribute && m_pProcSelectAttribute->IsValid())
	{
		CICRecordSet* rs = m_pProcSelectAttribute;

		try
		{
			rs->DataBinding("@Name", inout.m_sName.c_str());

			bool bHasRecord = (rs->NextRow()==0);
			if(bHasRecord)
			{
				int nCount = rs->ColumnCount();

				int i=0;
				CICRecordSetItem& item = (*rs)[0];
				inout.m_nType = (int)item;

				item = (*rs)[++i]; 
				inout.m_sValue = (const char*)item;

				item = (*rs)[++i]; 
				inout.m_sDescription = (const char*)item;

				item = (*rs)[++i]; 
				inout.m_sSchematics = (const char*)item;

				/*item = (*rs)[++i]; 
				void* pData = item;
				int nLength = item.GetDataLength();
				if(nLength > 0 )
				{
				out.m_binCustomAppearance.resize(nLength, 0);
				memcpy(&out.m_binCustomAppearance[0], pData, nLength);
				}
				else
				out.m_binCustomAppearance.clear();*/

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

bool AttributeProvider::InsertAttribute(CAttributeDbItem& sIn)
{
	DBEntity* pDB = GetDBEntity();
	PE_ASSERT(pDB!=NULL);
	bool bSucceed = false;
	if(m_pProcInsertAttribute == NULL)
	{
		// create the SQL procedure
		m_pProcInsertAttribute = pDB->CreateRecordSet("INSERT INTO %s ([Name], [Type], [Value], [Desc], [Schematics]) \
													  VALUES(@Name, @Type, @Value, @Desc, @Schematics)", m_sCurrentTableName.c_str());
	}
	if (m_pProcInsertAttribute && m_pProcInsertAttribute->IsValid())
	{
		CICRecordSet* rs = m_pProcInsertAttribute;

		try
		{
			rs->DataBinding("@Name", sIn.m_sName.c_str());
			rs->DataBinding("@Type", sIn.m_nType);
			rs->DataBinding("@Value", sIn.m_sValue.c_str());
			rs->DataBinding("@Desc", sIn.m_sDescription.c_str());
			rs->DataBinding("@Schematics", sIn.m_sSchematics.c_str());
			
			bSucceed = rs->Execute() == S_OK;
		}
		catch (CICSQLException& e)
		{
			OUTPUT_LOG("%s", e.errmsg.c_str());
		}
		rs->Reset();
	}
	return bSucceed;
}
bool AttributeProvider::InsertAttribute(const char*  sName, const char*  sValue)
{
	static CAttributeDbItem g_temp;
	g_temp.m_sName = sName;
	g_temp.m_sValue = sValue;
	return InsertAttribute(g_temp);
}

bool AttributeProvider::UpdateAttribute(const char* sName, const char* sIn)
{
	if(!DoesAttributeExists(sName))
	{
		// insert the attribute if it is not created before.
		return InsertAttribute(sName, sIn);
	}

	DBEntity* pDB = GetDBEntity();
	PE_ASSERT(pDB!=NULL);
	bool bSucceed = false;
	if(m_pProcUpdateAttribute == NULL)
	{
		// create the SQL procedure
		m_pProcUpdateAttribute = pDB->CreateRecordSet("UPDATE %s SET [Value] = @Value WHERE [Name] = @Name", m_sCurrentTableName.c_str());
	}
	if (m_pProcUpdateAttribute && m_pProcUpdateAttribute->IsValid())
	{
		CICRecordSet* rs = m_pProcUpdateAttribute;

		try
		{
			rs->DataBinding("@Value", sIn);
			rs->DataBinding("@Name", sName);

			bSucceed = rs->Execute() == S_OK;
		}
		catch (CICSQLException& e)
		{
			OUTPUT_LOG("%s", e.errmsg.c_str());
		}
		rs->Reset();
	}
	return bSucceed;
}

bool AttributeProvider::DeleteAttribute(const char*  sName)
{
	DBEntity* pDB = GetDBEntity();
	PE_ASSERT(pDB!=NULL);
	bool bSucceed = false;
	if(m_pProcDeleteAttribute == NULL)
	{
		// create the SQL procedure
		m_pProcDeleteAttribute = pDB->CreateRecordSet("DELETE FROM %s WHERE [Name] = @Name", m_sCurrentTableName.c_str());
	}
	if (m_pProcDeleteAttribute && m_pProcDeleteAttribute->IsValid())
	{
		CICRecordSet* rs = m_pProcDeleteAttribute;

		try
		{
			rs->DataBinding("@Name", sName);

			bSucceed = rs->Execute() == S_OK;
		}
		catch (CICSQLException& e)
		{
			OUTPUT_LOG("%s", e.errmsg.c_str());
		}
		rs->Reset();
	}
	return bSucceed;
}

bool AttributeProvider::CreateTableIfNotExists()
{
	DBEntity* pDB = GetDBEntity();
	if(pDB==NULL) 
		return false;
	char sql_create_attribute_table[MAX_SQL_LENGTH + 1];
	sql_create_attribute_table[MAX_SQL_LENGTH] = '\0';
	snprintf(sql_create_attribute_table, MAX_SQL_LENGTH,
		"CREATE TABLE IF NOT EXISTS [%s](\
		[Name] [nchar](30) UNIQUE NOT NULL,\
		[Type] [int] NOT NULL DEFAULT ((0)),\
		[Value] [nvarchar](255) NOT NULL,\
		[Desc] [nvarchar](512) NOT NULL,\
		[Schematics] [nvarchar](255) NOT NULL);", m_sCurrentTableName.c_str());
	pDB->ExecuteSqlScript(sql_create_attribute_table);
	// SQLITE_DONE is returned. 
	return true;
}

void AttributeProvider::SetTableName(const char*  sName) 
{ 
	if(m_sCurrentTableName != sName)
	{
		m_sCurrentTableName = sName;
		ReleaseStoredProcedures();
		if(!CreateTableIfNotExists())
		{
			OUTPUT_LOG("error creating table %s\r\n", m_sCurrentTableName.c_str());
		}
	}
}
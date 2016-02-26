//----------------------------------------------------------------------
// Class:	CICGenManager
// Authors:	Liu Weili
// Date:	2005.11.13
//
// desc: 
// Manage the GENs
//
//
//-----------------------------------------------------------------------
#ifdef USE_ICGEN_MANAGER
#include "ParaEngine.h"

#include "ICGenManager.h"
#include "ICDBManager.h"
#include "ICGen.h"
#include "ICRecordSet.h"
#include "ICTraceData.h"

#ifdef PARAENGINE_CLIENT
#include "memdebug.h"
#endif

using namespace ParaInfoCenter;

//////////////////////////////////////////////////////////////////////////
// CICGenManager
//////////////////////////////////////////////////////////////////////////
//CICGenManager* CICGenManager::m_instance=0;

CICGenManager::CICGenManager()
{
	Open();
}
CICGenManager::~CICGenManager()
{
	// By LXZ: just a guess here for singleton destructor may be wrong.
	Close();
}
CICRecordSet* CICGenManager::GetMapTrace()
{
	return GetMapTrace(L"");
}
CICRecordSet* CICGenManager::GetMapTrace(const WCHAR* where)
{
	try
	{
		return m_db->CreateRecordSet(L"SELECT * FROM memDB.maptrace %s",where);

	}
	catch (CICSQLException &err) {
		return NULL;
	}
}
CICRecordSet* CICGenManager::GetTraceData(double x, double y)
{
	//TODO: should change this function after cictracedata finish
	CICGenTraceData temp;
	temp.SetXY(x,y);
	CICRecordSet *rs=m_db->CreateRecordSet(L"SELECT data FROM memDB.tracedata WHERE logicx=%d AND logicy=%d",temp.GetX(),temp.GetY());

	if (rs->NextRow()==CICRecordSet::SEOF) {
		m_db->exec_sql("BEGIN");
		m_db->prepare_sql("INSERT INTO memDB.tracedata(logicx,logicy,data) SELECT * FROM fileDB.tracedata WHERE logicx=%d AND logicy=%d",temp.GetX(),temp.GetY());
		m_db->exec_sql();
		CICRecordSet *rs1=m_db->CreateRecordSet(L"SELECE count(*) FROM memDB.tracedata");
		rs1->Execute();
		int count=(*rs)[0];
		if (count>MAX_GEN_TRACEDATA_NUMBER) {
			CICRecordSet *rs2=m_db->CreateRecordSet("SELECT rowid FROM memDB.tracedata LIMIT %d,1",MAX_GEN_TRACEDATA_NUMBER/3);
			rs2->NextRow();
			int rowid=(*rs)[0];
			m_db->prepare_sql("DELECT FROM memDB.tracedata WHERE rowid<%d",rowid);
			m_db->exec_sql();
		}
		m_db->exec_sql("END");
		rs=m_db->CreateRecordSet(L"SELECT data FROM memDB.tracedata WHERE logicx=%d AND logicy=%d",temp.GetX(),temp.GetY());
		if (rs->NextRow()==CICRecordSet::SEOF) {
			throw;//if this line reaches, there must be something wrong.
		}
	}

	return rs;

}

CICRecordSet* CICGenManager::GetTraceData(double x1,double y1,double x2, double y2)
{
	return NULL;
}
wstring CICGenManager::QuoteReplace(const WCHAR *input)
{
	//the function copies each character to the new buffer, replacing ' with '', the \n0000 terminal is also copied.
	int len=wcslen(input)+1;
	WCHAR *temp=new WCHAR[(len<<2)];
	int a,b;
	for (a=0;a<len;a++,b++) {
		if (input[a]==L'\'') {
			temp[b++]=L'\'';
		}
		temp[b]=input[a];
	}
	wstring value=temp;
	delete temp;
	return value;
}
void CICGenManager::CreateMemTables()
{
	try{
		m_db->exec_sql("CREATE TABLE memDB.maptrace(time INTEGER PRIMARY KEY, x REAL, y REAL, z REAL, id TEXT)");
		//TODO: add memory tables
	}catch (CICSQLException &err) {
	}
}

void CICGenManager::CreateFileTables()
{
	try{
		//TODO: add file tables
		m_db->exec_sql("CREATE TABLE fileDB.tracedata(logicx INTEGER,logicy INTEGER,data BLOB);");
		m_db->exec_sql("CREATE INDEX fileDB.indextracedata ON tracedata(logicx,logicy);");
	}catch (CICSQLException &err) {
	}
}


#ifdef _DEBUG
#define TEMP_DB L"database\\temp.db"
#else
#define TEMP_DB L""
#endif
void CICGenManager::Open()
{
	m_db=CICDBManager::OpenDBEntity(L"IC",TEMP_DB);
	try
	{
		m_db->exec_sql("PRAGMA pagesize=4096;");
		m_db->exec_sql("PRAGMA temp_store=2;");
		m_db->exec_sql("PRAGMA synchronous = 0;");//shall delete
		m_db->exec_sql("ATTACH DATABASE 'database\\IC.db' AS fileDB");
		m_db->exec_sql("ATTACH DATABASE ':memory:' AS memDB");
		CreateMemTables();
		CreateFileTables();
	}
	catch (CICSQLException &sqlerr) {
		OUTPUT_LOG("%s", sqlerr.errmsg.c_str());
	}

}



void CICGenManager::Close()
{
	m_db.reset();
}

void CICGenManager::Finalize()
{
#ifdef _DEBUG
	if (m_db.get()!=0 && m_db->GetDBHandle()) {
		//dump the memory db to temp db
		try
		{
			m_db->exec_sql("CREATE TABLE maptrace(time INTEGER PRIMARY KEY, x REAL, y REAL, z REAL, id TEXT)");
		}
		catch (CICSQLException &err) {
		}
		try{
			m_db->exec_sql("INSERT INTO maptrace SELECT * FROM memDB.maptrace");
		}
		catch (CICSQLException &err) {
			OUTPUT_LOG("\nCan't write maptrace to disk");
		}
	}
#endif
}

CICGenManager* CICGenManager::Instance(){
	static CICGenManager m_instance;
	return &m_instance;
}
#endif
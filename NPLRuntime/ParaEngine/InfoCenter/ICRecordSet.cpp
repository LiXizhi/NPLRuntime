//----------------------------------------------------------------------
// Class:	CICRecordSet
// Authors:	Liu Weili
// Date:	2005.11.18
//
// desc: 
// Manage the GENs
//
//
// Revised by Andy Wang: add support to int64
// Date:	2006.8.12
//
//-----------------------------------------------------------------------
#include "ParaEngine.h"
#include <sqlite3.h>
#include "ICRecordSet.h"
#include "ICDBManager.h"

#ifdef PARAENGINE_CLIENT
#include "memdebug.h"
#endif

using namespace ParaInfoCenter;

#ifdef _DEBUG
//#define LOG_ALL_SQL_COMMANDS
#endif
//////////////////////////////////////////////////////////////////////////
// CICRecordSetItem
//////////////////////////////////////////////////////////////////////////
void CICRecordSetItem::CleanOldData()
{
	switch(this->m_type) {
	case _BLOB:
		if(this->bData)
			delete [] ((const char*)(this->bData));
		break;
	case _TEXT:
		if(this->sData)
			delete [] ((const char*)(this->sData));
		break;
	case _TEXT16:
		if(this->wData)
			delete [] ((const char16_t*)(this->wData));
		break;
	default:
		break;
	}
}

CICRecordSetItem& CICRecordSetItem::operator =(const CICRecordSetItem& value)
{
	CleanOldData();
	memcpy(this,&value,sizeof(CICRecordSetItem));
	switch(value.m_type) {
	case _BLOB:
		if(value.m_length>0)
		{
			this->bData=new char[value.m_length];
			memcpy(this->bData,value.bData,value.m_length);
		}
		else
		{
			this->bData=NULL;
		}
		break;
	case _TEXT:
		this->sData=new char[value.m_length+1];
		strncpy(this->sData,value.sData,value.m_length);
		this->sData[value.m_length]='\0';
		break;
	case _TEXT16:
		this->wData=new char16_t[value.m_length+1];
		wcsncpy((WCHAR*)this->wData, (WCHAR*)value.wData, value.m_length);
		this->wData[value.m_length]=L'\0';
		break;
	default:
		break;
	}
	return *this;
}

CICRecordSetItem& CICRecordSetItem::operator =(const bool& value)
{
	CleanOldData();
	m_needInit=false;
	iData=value;
	m_type=_INT;
	return *this;
}

CICRecordSetItem& CICRecordSetItem::operator =(const int& value)
{
	CleanOldData();
	m_needInit=false;
	iData=value;
	m_type=_INT;
	return *this;
}

CICRecordSetItem& CICRecordSetItem::operator =(const int64& value)
{
	CleanOldData();
	m_needInit=false;
	nDataInt64 = value;
	m_type=_INT64;
	return *this;
}

CICRecordSetItem& CICRecordSetItem::operator =(const double& value)
{
	CleanOldData();
	m_needInit=false;
	fData=value;
	m_type=_FLOAT;
	return *this;
}
CICRecordSetItem& CICRecordSetItem::operator =(const float& value)
{
	CleanOldData();
	m_needInit=false;
	fData=value;
	m_type=_FLOAT;
	return *this;
}

CICRecordSetItem& CICRecordSetItem::operator =(const char* value)
{
	CleanOldData();
	m_length=strlen(value);
	this->sData=new char[m_length+1];
	strncpy(this->sData,value,m_length);
	this->sData[m_length]='\0';
	m_needInit=false;
	m_type=_TEXT;
	return *this;
}
CICRecordSetItem& CICRecordSetItem::operator =(const char16_t* value)
{
	CleanOldData();
	m_length = wcslen((WCHAR*)value);
	this->wData=new char16_t[m_length+1];
	wcsncpy((WCHAR*)this->wData, (WCHAR*)value, m_length);
	this->wData[m_length]=L'\0';
	m_needInit=false;
	m_type=_TEXT16;
	return *this;
}

void CICRecordSetItem::SetBlob(const void *value,int nlength)
{
	CleanOldData();
	m_length=nlength;
	if(nlength>0){
		this->bData=new char[m_length];
		memcpy(this->bData,value,m_length);
	}
	else
		this->bData=NULL;

	m_needInit=false;
	m_type=_BLOB;
}

const char* CICRecordSetItem::GetNameA()
{
	if (m_sName==NULL) {
		const char *temp=sqlite3_column_name(m_stmt,m_index);
		int nLength=strlen(temp);
		m_sName=new char[nLength+1];
		strncpy(m_sName,temp,nLength);
		m_sName[nLength]='\0';
	}
	return m_sName;
}

const char16_t *CICRecordSetItem::GetName()
{
	if (m_wName==NULL) {
		const char16_t *temp=(const char16_t *)sqlite3_column_name16(m_stmt,m_index);
		int nLength = wcslen((WCHAR*)temp);
		m_wName=new char16_t[nLength+1];
		wcsncpy((WCHAR*)m_wName, (WCHAR*)temp, nLength);
		m_wName[nLength]=L'\0';
	}
	return m_wName;
}

CICRecordSetItem::operator char *()
{
	if (m_needInit) {
		m_length=sqlite3_column_bytes(m_stmt,m_index);
		sData=new char[m_length+1];
		strncpy(sData,(const char *)sqlite3_column_text(m_stmt,m_index),m_length);
		sData[m_length]='\0';
		m_type=_TEXT;
		m_needInit=false;
	}
	return sData;
}

CICRecordSetItem::operator char16_t *()
{
	if (m_needInit) {
		m_length=sqlite3_column_bytes(m_stmt,m_index)>>2;
		wData=new char16_t[m_length+1];
		wcsncpy((WCHAR*)wData, (const WCHAR*)sqlite3_column_text16(m_stmt, m_index), m_length);
		wData[m_length]=L'\0';
		m_type=_TEXT16;
		m_needInit=false;
	}
	return wData;
}
CICRecordSetItem::operator void *()
{
	if (m_needInit) {
		m_length=sqlite3_column_bytes(m_stmt,m_index);
		if(m_length>0) // prevent NULL, by lxz
		{
			bData=new char[m_length];
			memcpy(bData,(const void *)sqlite3_column_blob(m_stmt,m_index),m_length);
		}
		m_type=_BLOB;
		m_needInit=false;
	}
	return bData;
}
CICRecordSetItem::operator int()
{
	if (m_needInit) {
		m_length=4;
		iData=sqlite3_column_int(m_stmt,m_index);
		m_type=_INT;
		m_needInit=false;
	}
	return iData;
}

CICRecordSetItem::operator int64()
{
	if (m_needInit) {
		m_length=4;
		nDataInt64 = sqlite3_column_int64(m_stmt,m_index);
		m_type=_INT64;
		m_needInit=false;
	}
	return iData;
}

CICRecordSetItem::operator bool()
{
	if (m_needInit) {
		m_length=4;
		iData=sqlite3_column_int(m_stmt,m_index);
		m_type=_INT;
		m_needInit=false;
	}
	return iData!=0;
}
CICRecordSetItem::operator DWORD()
{
	if (m_needInit) {
		m_length=4;
		iData=sqlite3_column_int(m_stmt,m_index);
		m_type=_INT;
		m_needInit=false;
	}
	return iData;
}

CICRecordSetItem::operator double()
{
	if (m_needInit) {
		m_length=8;
		fData=sqlite3_column_double(m_stmt,m_index);
		m_type=_FLOAT;
		m_needInit=false;
	}
	return fData;
}
CICRecordSetItem::operator float()
{
	if (m_needInit) {
		m_length=8;
		fData=sqlite3_column_double(m_stmt,m_index);
		m_type=_FLOAT;
		m_needInit=false;
	}
	return (float)fData;
}
CICRecordSetItem::CICRecordSetItem(int index,sqlite3_stmt *stmt):m_index(index),m_stmt(stmt)
{
	if (index<0) {
		m_needInit=false;
	}else{
		m_needInit=true;
	}
	m_type=_NONE;
	m_sName=0;
	m_wName=0;
	fData=0;
	m_length=0;
};
CICRecordSetItem::~CICRecordSetItem()
{
	if (!m_needInit) {
		CleanOldData();
	}
	SAFE_DELETE_ARRAY(m_sName);
	SAFE_DELETE_ARRAY(m_wName);
}
//////////////////////////////////////////////////////////////////////////
// CICRecordSet
//////////////////////////////////////////////////////////////////////////
int CICRecordSet::SOK=0;
int CICRecordSet::SEOF=-1;
int CICRecordSet::SBOF=-2;

CICRecordSet::CICRecordSet()
{
	m_empty=0;
	m_columnNum=0;
}
/*
CICRecordSet::CICRecordSet(sqlite3_stmt *stmt)
{
m_empty=0;
m_isValid=false;
Initialize(stmt);
}
CICRecordSet::CICRecordSet(const char16_t* sql)
{
m_empty=0;
m_isValid=false;
Initialize(sql);
}
CICRecordSet::CICRecordSet(const char* sql)
{
m_empty=0;
m_isValid=false;
Initialize(sql);
}*/
CICRecordSet::~CICRecordSet()
{
	Release();
}
/*
void *CICRecordSet::operator new(size_t size)
{
return CICDBManager::CreateRecordSet();
}
void CICRecordSet::operator delete(void* p)
{
CICDBManager::DeleteRecordSet((CICRecordSet*)p);
}*/
void CICRecordSet::Release()
{
	if (m_stmt) {
		sqlite3_finalize(m_stmt);
		m_stmt=NULL;
	}
	SAFE_DELETE(m_empty);
	for (DWORD a=0;a<m_items.size();a++) {
		SAFE_DELETE(m_items[a]);
	}
	m_isValid=false;
}

int64 CICRecordSet::GetLastInsertRowID()
{
	if(m_db!=0)
	{
		return sqlite3_last_insert_rowid(m_db);
	}
	return 0;
}
void CICRecordSet::Initialize(sqlite3_stmt* stmt)
{
	if (stmt==NULL||m_db==NULL) {
		return;
	}
	Release();
	m_stmt=stmt;
	m_columnNum=sqlite3_column_count(stmt);
	m_bof=true;
	m_isValid=true;
	if (m_columnNum>0) {
		m_items.resize(m_columnNum);
		for (DWORD a=0;a<m_items.size();a++) {
			m_items[a]=NULL;
		}
		const char *temp=sqlite3_column_name(stmt,0);
		if (strcmp(temp,"rowid")==0) {
			m_bUpdatable=true;
		}else
			m_bUpdatable=false;
		m_eof=false;
	}else{
		m_eof=true;
	}
}
void CICRecordSet::Initialize(const char16_t *sql)
{
	if (sql==NULL||m_db==NULL) {
		return;
	}
	sqlite3_stmt* stmt;
	const void *trail;
	try
	{
		int errcode=sqlite3_prepare16(m_db,sql,-1,&stmt,&trail);
		const char *errmsg;
		if (SQLITE_OK!=errcode) {
			errmsg=sqlite3_errmsg(m_db);
			throw CICSQLException(errcode,errmsg);
		}
		Initialize(stmt);
	}catch (CICSQLException&err) {
		m_isValid=false;
		OUTPUT_LOG("%s", err.errmsg.c_str());
	}
}

void CICRecordSet::Initialize(const char *sql)
{
	if (sql==NULL||m_db==NULL) {
		return;
	}
	sqlite3_stmt* stmt;
	const char *trail;
	try
	{
		int errcode=sqlite3_prepare(m_db,sql,-1,&stmt,&trail);
		const char *errmsg;
		if (SQLITE_OK!=errcode) {
			errmsg=sqlite3_errmsg(m_db);
			throw CICSQLException(errcode,errmsg);
		}
		Initialize(stmt);
#ifdef LOG_ALL_SQL_COMMANDS
		OUTPUT_LOG("%s\n", sql);
#endif
	}catch (CICSQLException&err) {
		m_isValid=false;
		OUTPUT_LOG("%s", err.errmsg.c_str());
	}
}

const char* CICRecordSet::GetColumnName(int index)
{
	if (!m_isValid) {
		return NULL;
	}
	return sqlite3_column_name(m_stmt,index);
}
const char16_t* CICRecordSet::GetColumnName16(int index)
{
	if (!m_isValid) {
		return NULL;
	}
	return (const char16_t*)sqlite3_column_name16(m_stmt,index);
}

int CICRecordSet::NextRow()
{
	if (m_eof||!m_isValid) {
		m_eof=true;
		return CICRecordSet::SEOF;
	}

	int errcode=0;
	const char *errmsg;
	try
	{
		errcode=sqlite3_step(m_stmt);

		if (errcode==SQLITE_ROW) {
			for (DWORD a=0;a<m_items.size();a++) {
				SAFE_DELETE(m_items[a]);
			}
			errcode=SQLITE_OK;
		}else if (errcode==SQLITE_DONE) {
			m_eof=true;
			errcode=CICRecordSet::SEOF;
		}else if (SQLITE_ERROR==errcode) {
			errmsg=sqlite3_errmsg(m_db);
			throw CICSQLException(errcode,errmsg);
		}
		m_bof=false;
	}catch (CICSQLException &err) {
		m_isValid=false;
		OUTPUT_LOG("%s", err.errmsg.c_str());
	}
	return errcode;
}

CICRecordSetItem& CICRecordSet::operator [](int index)
{
	if (!m_isValid||m_eof||index<0 || index>=m_columnNum||m_bof) {
		if(m_empty==NULL)
			m_empty=new CICRecordSetItem(-1,m_stmt);
		return *m_empty;
	}
	if (m_items[index]==NULL) {
		CICRecordSetItem *newitem=new CICRecordSetItem(index,m_stmt);
		m_items[index]=newitem;
	}
	return *m_items[index];
}

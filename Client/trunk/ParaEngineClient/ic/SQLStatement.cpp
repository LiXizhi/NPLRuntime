//-----------------------------------------------------------------------------
// Class:	CSQLStatement
// Authors:	mod by LXZ
// Emails:	LiXizhi@yeah.net
// Date:	2006.5.22
// Note: modification over ICRecordSet.h. Make it more compatible with SQL and sqlite3 common structure
// TODO: seperate CRecordSet and CSQLStatement. i.e. make CRecordSet a managed list under CSQLStatement
// and make CSQLStatement a managed list of DBEntity. and make DBEntity a managed list of DBManager
//-----------------------------------------------------------------------------

#include "ParaEngine.h"
#include "ic/ICDBManager.h"
#include "ic/ICRecordSet.h"
#include <sqlite3.h>

#include "SQLStatement.h"

using namespace ParaInfoCenter;
using namespace ParaEngine;

#define DEFINE_DATABIND(type) \
int CSQLStatement::DataBinding(const char* name,type value)\
{\
	int index = sqlite3_bind_parameter_index(m_stmt, name);\
	if(index > 0)\
		return DataBinding(index, value);\
	else\
	{\
		OUTPUT_LOG("unknown SQL parameter name %s\r\n", name);\
		return SQLITE_ERROR;\
	}\
}

CSQLStatement::CSQLStatement(void)
:m_stmt(NULL), m_db(NULL),m_isValid(false), m_eof(true), m_bof(true)
{

}

CSQLStatement::~CSQLStatement(void)
{
}

int CSQLStatement::Execute()
{
	if (!m_isValid) {
		return SQLITE_ERROR;
	}
	int errcode=0;
	const char *errmsg;
	try
	{
		errcode=sqlite3_step(m_stmt);
		if (SQLITE_ERROR==errcode) {
			errmsg=sqlite3_errmsg(m_db);
			throw CICSQLException(errcode,errmsg);
		}
	}catch (CICSQLException &err) {
		OUTPUT_LOG("%s", err.errmsg.c_str());
	}
	
	// only reset so that the same statement can be reused. 
	Reset(); 
	return errcode==SQLITE_ERROR?SQLITE_ERROR:SQLITE_OK;
}


int CSQLStatement::Reset()
{
	m_bof=true;
	if (!m_isValid) {
		m_eof=true;
		return CICRecordSet::SEOF;
	}
	int errcode=0;
	try
	{
		errcode=sqlite3_reset(m_stmt);
	}catch (CICSQLException &err) {
		m_isValid=false;
		OUTPUT_LOG("%s", err.errmsg.c_str());
	}
	m_eof=false;
	return errcode;
}

DEFINE_DATABIND(const char*)
DEFINE_DATABIND(const WCHAR*)
DEFINE_DATABIND(const double)
DEFINE_DATABIND(const int)
DEFINE_DATABIND(const int64)

int CSQLStatement::DataBindingNull(const char* name)
{
	int index = sqlite3_bind_parameter_index(m_stmt, name);
	if(index > 0)
		return DataBindingNull(index);
	else
	{
		OUTPUT_LOG("unknown SQL parameter name %s, ", name);
		return SQLITE_ERROR;
	}
}

int CSQLStatement::DataBinding(const char* name,const void*value,int size)
{
	int index = sqlite3_bind_parameter_index(m_stmt, name);
	if(index > 0)
		return DataBinding(index, value, size);
	else
	{
		OUTPUT_LOG("unknown SQL parameter name %s, ", name);
		return SQLITE_ERROR;
	}
}

int CSQLStatement::DataBinding(int index,double value)
{
	if (!m_isValid||!m_bof) {
		OUTPUT_LOG("DataBinding should be called after initializing a SQL string and before the NextRow()");
		return SQLITE_ERROR;
	}
	int errcode=0;
	errcode=sqlite3_bind_double(m_stmt,index,value);
	return errcode;
}
int CSQLStatement::DataBinding(int index,int value)
{
	if (!m_isValid||!m_bof) {
		OUTPUT_LOG("DataBinding should be called after initializing a SQL string and before the NextRow()");
		return SQLITE_ERROR;
	}
	int errcode=0;
	errcode=sqlite3_bind_int(m_stmt,index,value);
	return errcode;
}
int CSQLStatement::DataBinding(int index,int64 value)
{
	if (!m_isValid||!m_bof) {
		OUTPUT_LOG("DataBinding should be called after initializing a SQL string and before the NextRow()");
		return SQLITE_ERROR;
	}
	int errcode=0;
	errcode=sqlite3_bind_int64(m_stmt,index,(sqlite_uint64)value);
	return errcode;
}
int CSQLStatement::DataBindingNull(int index)
{
	if (!m_isValid||!m_bof) {
		OUTPUT_LOG("DataBinding should be called after initializing a SQL string and before the NextRow()");
		return SQLITE_ERROR;
	}
	int errcode=0;
	errcode=sqlite3_bind_null(m_stmt,index);
	return errcode;
}

int CSQLStatement::DataBinding(int index,const char*value)
{
	if (!m_isValid||!m_bof) {
		OUTPUT_LOG("DataBinding should be called after initializing a SQL string and before the NextRow()");
		return SQLITE_ERROR;
	}
	int errcode=0;
	errcode=sqlite3_bind_text(m_stmt,index,value,-1,SQLITE_STATIC);
	return errcode;
}

int CSQLStatement::DataBinding(int index,const WCHAR*value)
{
	if (!m_isValid||!m_bof) {
		OUTPUT_LOG("DataBinding should be called after initializing a SQL string and before the NextRow()");
		return SQLITE_ERROR;
	}
	int errcode=0;
	errcode=sqlite3_bind_text16(m_stmt,index,value,-1,SQLITE_STATIC);
	return errcode;
}
int CSQLStatement::DataBinding(int index,const void*value,int size)
{
	if (!m_isValid||!m_bof) {
		OUTPUT_LOG("DataBinding should be called after initializing a SQL string and before the NextRow()");
		return SQLITE_ERROR;
	}
	int errcode=0;
	errcode=sqlite3_bind_blob(m_stmt,index,value,size,SQLITE_STATIC);
	return errcode;
}
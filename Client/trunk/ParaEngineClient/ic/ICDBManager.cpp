//----------------------------------------------------------------------
// Class:	CICDBManager
// Authors:	Liu Weili, LiXizhi
// Date:	2005.11.18
//
// desc: 
// Manage the GENs
//
// revision: 2006.5.20: by LXZ. 
//  - OpenDBEntity() will not reopen the same database multiple times. Added some documentation to the header file.
//  - log error is reported more precisely. 
//  - creating some alias to some static functions.
//  - slightly improved coding
//  - database file path is now kept in the DBEntity as wstring. 
//  - sql_* wrapper added 
//  - TODO: how to close a database when it is still busy?
//  - the sqlite3_complete is wrong when TRIGGER is ignored. please note that I have replaced sqlite3_complete with a simple search for ';'
//-----------------------------------------------------------------------

#include "ParaEngine.h"
#include <sqlite3.h>

#include "ICDBManager.h"
#include "ICRecordSet.h"
#include "FileManager.h"
#include "AsyncLoader.h"
#include "util/StringHelper.h"

using namespace ParaInfoCenter;

//////////////////////////////////////////////////////////////////////////
// CICSQLException
//////////////////////////////////////////////////////////////////////////
CICSQLException::CICSQLException(int code,const char *zFormat,...)
{
	// TODO: lxz 2006.5.5: this exception object itself may throw new exception when zFormat is invalid. 
	// error case: finalize in ICDBManager(), when sqlite return an error.
	errcode=code;
	va_list args;
	char buf[MAX_DEBUG_STRING_LENGTH+1];
	va_start(args, zFormat);
	vsnprintf(buf, MAX_DEBUG_STRING_LENGTH, zFormat, args);
	va_end(args);
	errmsg=buf;
}

CICSQLException::~CICSQLException()
{
}

///////////////////////////////////////////////////////////////////////////
//
// database management functions
//
//////////////////////////////////////////////////////////////////////////

vector<DBpair> CICDBManager::m_DBpool;
ParaEngine::mutex	CICDBManager::m_mutex;

void CICDBManager::StaticInit()
{
	ParaEngine::Lock lock_(m_mutex);

	m_DBpool.clear();
#ifdef PARAENGINE_MOBILE
	ParaEngine::CParaFile::DeleteFile("temp/tempdatabase/*.*", true);
#endif
}

void CICDBManager::Finalize()
{
	ParaEngine::Lock lock_(m_mutex);

	for (DWORD a=0;a<m_DBpool.size();a++) {
		CloseDBEntity(m_DBpool[a].first);
		SAFE_DELETE(m_DBpool[a].first);
	}
	m_DBpool.clear();
}

DBEntity* CICDBManager::GetDB(const char16_t* name)
{
	ParaEngine::Lock lock_(m_mutex);

	if (name==NULL) {
		return NULL;
	}
	for (DWORD a=0;a<m_DBpool.size();a++) {
		if (m_DBpool[a].second==true&&m_DBpool[a].first->m_name==name) {
			return m_DBpool[a].first;
		}
	}
	return NULL;
}

DBEntity* CICDBManager::OpenDBEntity()
{
	ParaEngine::Lock lock_(m_mutex);

	DWORD a;
	for (a=0;a<m_DBpool.size();a++) {
		if (m_DBpool[a].second==false) {
			m_DBpool[a].second=true;
			return m_DBpool[a].first;
		}
	}
	DWORD oldsize=(DWORD)m_DBpool.size();
	DBpair temp(new DBEntity(),false);
	m_DBpool.push_back(temp);	
	m_DBpool[oldsize].second=true;
	return	m_DBpool[oldsize].first;

}

DBEntity* CICDBManager::OpenDBEntity(const char* name, const char* dbname)
{
	ParaEngine::Lock lock_(m_mutex);

	if (name==NULL||dbname==NULL) {
		return NULL;
	}

	u16string wsName;
	::ParaEngine::StringHelper::UTF8ToUTF16(name, wsName);
	DBEntity *temp = GetDB(wsName.c_str());

	// change name to canonical name
	string sDbName = dbname; 
	if(temp!=0 && !temp->IsValid())
	{
		// try reopen the closed database
		temp->m_name=wsName;

		temp->OpenDB(sDbName.c_str());
	}
	else if(temp==NULL || !temp->IsValid())
	{
		// only load if not loaded before.
		temp=OpenDBEntity();
		temp->m_name=wsName;
		temp->OpenDB(sDbName.c_str());
	}
	return temp;
}

DBEntity* CICDBManager::OpenDBEntity(const char16_t* name, const char16_t* dbname)
{
	ParaEngine::Lock lock_(m_mutex);

	if (name==NULL||dbname==NULL) {
		return NULL;
	}

	DBEntity *temp = GetDB(name);

	// TODO: change name to canonical name
	if(temp!=0 && !temp->IsValid())
	{
		// try reopen the closed database
		temp->m_name=name;
		temp->OpenDB16(dbname);
	}
	else if(temp==NULL || !temp->IsValid())
	{
		// only load if not loaded before.
		temp=OpenDBEntity();
		temp->m_name=name;
		temp->OpenDB16(dbname);
	}
	return temp;
}

DBEntity* CICDBManager::OpenDBEntity(const char* dbname)
{
	return OpenDBEntity(dbname, dbname);
}

DBEntity* CICDBManager::OpenDBEntity(const char16_t* dbname)
{
	return OpenDBEntity(dbname, dbname);
}

bool CICDBManager::CloseDBEntity(DBEntity *dbmanager)
{
	ParaEngine::Lock lock_(m_mutex);
	DWORD a;
	if (dbmanager==NULL) {
		return false;
	}
	for (a=0;a<m_DBpool.size();a++) {
		if ((m_DBpool[a].first)==dbmanager) {
			if (m_DBpool[a].second==true) {
				m_DBpool[a].first->Release();
				m_DBpool[a].second=false;
			}
			return true;
		}
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////
//
// database entity member functions
//
//////////////////////////////////////////////////////////////////////////

DBEntity::DBEntity()
:m_refcount(0), m_nSQLite_OpenFlags(SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE ), m_bIsCreateFile(false)
{
	init();
}
DBEntity::~DBEntity()
{
	for (DWORD a=0;a<m_RSpool.size();a++) {
		m_RSpool[a].first->Release();
		SAFE_DELETE(m_RSpool[a].first);
		m_RSpool[a].second=false;
	}
}

void DBEntity::init()
{
	m_name.clear();
	m_isValid=false;
	m_db=0;
	m_RSpool.clear();
	m_bEncodingUTF8 = true;
}

bool DBEntity::IsCreateFile() 
{
	return m_bIsCreateFile;
}

void DBEntity::SetCreateFile(bool bCreateFile) 
{
	m_bIsCreateFile = bCreateFile;
}

string DBEntity::PrepareDatabaseFile(const string& filename)
{
	m_nSQLite_OpenFlags = (SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE );

#ifdef PARAENGINE_MOBILE
	std::string sTempDiskFilename = ParaEngine::CParaFile::GetWritablePath() + filename;
	if (ParaEngine::CParaFile::DoesFileExist(sTempDiskFilename.c_str(), false))
	{
		return sTempDiskFilename;
	}
	using namespace ParaEngine;
	CParaFile file;
	uint32 dwFileFound = CParaFile::DoesFileExist2(filename.c_str(), FILE_ON_DISK | FILE_ON_ZIP_ARCHIVE | FILE_ON_SEARCH_PATH);
	if ((dwFileFound & FILE_ON_ZIP_ARCHIVE) > 0)
	{
		// if file is inside a zip archive, we will 
		sTempDiskFilename = ParaEngine::CParaFile::GetWritablePath() + "temp/tempdatabase/";
		std::string sFileName = filename;
		for (int i = 0; i < (int)filename.size(); ++i)
		{
			unsigned char c = (unsigned char)filename[i];
			if (c == '\\' || c == '/' || c == ':')
				sFileName[i] = '_';
			else if (c<128 && (isalpha(c) || c == '.'))
			{
				sFileName[i] = filename[i];
			}
			else
			{
				sFileName[i] = 'a' + (c % 26);
			}
		}
		sTempDiskFilename += sFileName;
	}
	file.OpenAssetFile(filename.c_str());
	if (file.isEof())
	{
		OUTPUT_LOG("database file %s not exist \r\n", filename.c_str());
		return "";
	}

	if (!ParaEngine::CParaFile::CreateDirectory(sTempDiskFilename.c_str()))
	{
		OUTPUT_LOG("error: failed creating directory for database file %s in archive to %s \r\n", filename.c_str(), sTempDiskFilename.c_str());
	}

	ParaEngine::CParaFile fileTo;
	if (fileTo.CreateNewFile(sTempDiskFilename.c_str()))
	{
		// always open in read only mode for files from zip or remote asset
		m_nSQLite_OpenFlags = SQLITE_OPEN_READONLY;
		OUTPUT_LOG("database: %s is set to read-only mode\r\n", filename.c_str());
		fileTo.write(file.getBuffer(), (int)(file.getSize()));
	}
	else
	{
		OUTPUT_LOG("error: failed extracting database file %s to %s \r\n", filename.c_str(), sTempDiskFilename.c_str());
	}
	return sTempDiskFilename;

#else
	if(filename == ":memory:")
	{
		return filename;
	}
	else if( ParaEngine::CParaFile::DoesFileExist(filename.c_str(), false) &&
		((ParaEngine::CParaFile::GetDiskFilePriority()>=0 || !ParaEngine::CFileManager::GetInstance()->DoesFileExist(filename.c_str()))) )
	{
		// disk file exist and (disk file has priority or zip file does not contain the file).
		ParaEngine::CAsyncLoader::GetSingleton().log(string("DBEntity.PrepareDatabaseFile using local file:") + filename + "\n");
#ifdef WIN32
		// remove read only file attribute. This is actually not necessary, but just leaves here for debugging purposes. 
		DWORD dwAttrs = ::GetFileAttributes(filename.c_str());
		if (dwAttrs == INVALID_FILE_ATTRIBUTES)
		{
		}
		else if ((dwAttrs & FILE_ATTRIBUTE_READONLY))
		{
			// this fixed an issue when sqlite take very long (2 seconds) when opening read-only file.
			m_nSQLite_OpenFlags = SQLITE_OPEN_READONLY;
		}
#endif
		return filename;
	}
	else
	{
		//////////////////////////////////////////////////////////////////////////
		// extract database file from zip archive to temp/tempdatabase/[filename]
		//////////////////////////////////////////////////////////////////////////
		ParaEngine::CParaFile file;
		file.OpenAssetFile(filename.c_str());
		if(file.isEof())
			return "";

		string sTempDiskFilename = ParaEngine::CParaFile::GetCurDirectory(ParaEngine::CParaFile::APP_TEMP_DIR);
		sTempDiskFilename += "tempdatabase/";
		string sFileName = filename;
		for(int i=0;i< (int)filename.size(); ++i)
		{
			if(filename[i] == '\\' || filename[i] == '/' || filename[i] == ':')
				sFileName[i] = '_';
		}
		sTempDiskFilename += sFileName;

		if(!ParaEngine::CParaFile::CreateDirectory(sTempDiskFilename.c_str()))
		{
			OUTPUT_LOG("error: failed creating directory for database file %s in archive to %s \r\n", filename.c_str(), sTempDiskFilename.c_str());
		}

		// delete the source file if exists
		if(ParaEngine::CParaFile::DoesFileExist(sTempDiskFilename.c_str(), false))
		{
#ifdef WIN32
			// remove read only file attribute. This is actually not necessary, but just leaves here for debugging purposes. 
			DWORD dwAttrs = ::GetFileAttributes(sTempDiskFilename.c_str()); 
			if (dwAttrs == INVALID_FILE_ATTRIBUTES) 
			{
				OUTPUT_LOG("file attribute %s can not be read\n", sTempDiskFilename.c_str());
			}
			if ((dwAttrs & FILE_ATTRIBUTE_READONLY)) 
			{ 
				::SetFileAttributes(sTempDiskFilename.c_str(), dwAttrs & (~FILE_ATTRIBUTE_READONLY)); 
			} 

			if(!::DeleteFile(sTempDiskFilename.c_str()))
			{
				OUTPUT_LOG("can not replace db file %s. because we can not delete it\n", sTempDiskFilename.c_str());
			}
#endif
		}
		ParaEngine::CParaFile fileTo;
		if(fileTo.CreateNewFile(sTempDiskFilename.c_str()))
		{
			// always open in read only mode for files from zip or remote asset
			m_nSQLite_OpenFlags = SQLITE_OPEN_READONLY;
			OUTPUT_LOG("database: %s is set to read-only mode\r\n", filename.c_str());
			fileTo.write(file.getBuffer(), (int)(file.getSize()));
		}
		else
		{
			OUTPUT_LOG("error: failed extracting database file %s to %s \r\n", filename.c_str(), sTempDiskFilename.c_str());
		}
		return sTempDiskFilename;
	}
#endif
}
void DBEntity::OpenDB(const char* dbname)
{
	if (dbname==NULL) {
		return;
	}
	int errcode;

	string diskfileName = PrepareDatabaseFile(dbname);
	if(diskfileName=="")
	{
		// the database file can not be found anywhere, we will try create the database anyway. 
#ifdef PARAENGINE_MOBILE
		diskfileName = ParaEngine::CParaFile::GetWritablePath() + dbname;
#else
		diskfileName = dbname;
#endif
		ParaEngine::CParaFile::CreateDirectory(diskfileName.c_str());
		OUTPUT_LOG("try create database file %s\n", diskfileName.c_str());
		SetCreateFile(true);
	}

	string UTF8_Name = ::ParaEngine::StringHelper::AnsiToUTF8(diskfileName.c_str());
	
	int nMaxRetryTimes = IsCreateFile() ? 1 : 3;

	for (int i = 1; i <= nMaxRetryTimes; i++)
	{
		if (SQLITE_OK != (errcode = sqlite3_open_v2(UTF8_Name.c_str(), &m_db, m_nSQLite_OpenFlags, NULL)))
		{
			OUTPUT_LOG("warn: can not open database %d times: %s, because %s\r\n", i, dbname, sqlite3_errmsg(m_db));
			if (nMaxRetryTimes == i)
			{
				OUTPUT_LOG("warn: can not open database: %s\r\n", dbname);
				return;
			}
			else
			{
				// sleep some time just in case some other game instance is unzipping this database file. 
				SLEEP(500);
			}
		}
		else
			break;
	}
	m_filepath = dbname;

	m_stmt=NULL;
	m_isValid=true;
	m_bEncodingUTF8 = true;

	OUTPUT_LOG("database:%s opened\n", GetConnectionString().c_str());
}
void DBEntity::OpenDB16(const char16_t* dbname)
{
	if (dbname==NULL) {
		return;
	}

	ParaEngine::StringHelper::UTF16ToUTF8(dbname, m_filepath);

	string diskfileName = PrepareDatabaseFile(m_filepath);
	if(diskfileName=="")
	{
		// the database file can not be found anywhere, we will try create the database anyway. 
		diskfileName = m_filepath;
		SetCreateFile(true);
	}

	std::u16string UTF16_Name;
	::ParaEngine::StringHelper::UTF8ToUTF16(diskfileName, UTF16_Name);

	int errcode;
	if (SQLITE_OK!=(errcode=sqlite3_open16(UTF16_Name.c_str(),&m_db))) {
		OUTPUT_LOG(L"error: Can not open database: %s\r\n",dbname);
		OUTPUT_LOG("Error message is: %s\r\n",sqlite3_errmsg(m_db));
		return;
	}
	
	m_stmt=NULL;
	m_isValid=true;
	m_bEncodingUTF8 = false;
	OUTPUT_LOG("database:%s opened\n", GetConnectionString().c_str());
}
void DBEntity::OpenDB()
{
	if(!IsValid())
	{
		if(m_bEncodingUTF8)
			OpenDB(m_filepath.c_str());
		else
		{
			std::u16string UTF16_Name;
			::ParaEngine::StringHelper::UTF8ToUTF16(m_filepath, UTF16_Name);
			OpenDB16(UTF16_Name.c_str());
		}
	}
}
void DBEntity::CloseDB()
{
	if (GetRefCount() >= 1)
	{
		OUTPUT_LOG("warning: sql db %s is closed with %d active references\n", GetConnectionString().c_str(), GetRefCount());
	}
	{
		if (m_isValid&&m_db!=NULL) {

#ifdef RELEASE_MEMORY_DB
			if(m_filepath == ":memory:")
			{
				m_db=NULL;
				m_stmt=NULL;
				m_isValid=false;
				return;
			}
#endif
			if (SQLITE_BUSY==sqlite3_close(m_db)) {
				OUTPUT_LOG("warning: Can't close database %s because it is busy. \r\nThis is usually caused by unclosed database object in the scripts.\r\n", m_filepath.c_str());
			}else{
				m_db=NULL;
				m_stmt=NULL;
				OUTPUT_LOG("sql db closed: %s\n", GetConnectionString().c_str());
			}
		}
		m_stmt=NULL;
		m_isValid=false;
	}
}
void DBEntity::Release()
{
	/** because some of DBEntity is not reference counted. we shall do a (GetRefCount()==0) check */
	if(GetRefCount()==0 || delref())
	{
		for (DWORD a=0;a<m_RSpool.size();a++) {
			m_RSpool[a].first->Release();
			m_RSpool[a].second=false;
		}
		CloseDB();
	}
}

int64 DBEntity::GetLastInsertRowID()
{
	if(m_db!=0)
	{
		return sqlite3_last_insert_rowid(m_db);
	}
	return 0;
}


int DBEntity::ExecuteSqlScript(const char *sql, bool bBreakOnError)
{
	if (!m_isValid)
		return E_FAIL;
	int result;
	m_trail=(char*)sql;
	if(bBreakOnError)
	{
		//////////////////////////////////////////////////////////////////////////
		// break on the first error
		try
		{
			do {
				do {
					result = SQLITE_ERROR;
					exec_sql_prepare();
					result=exec_sql_step();

				} while(result==SQLITE_SCHEMA);

				// the sqlite3_complete is wrong when TRIGGER is ignored.
				// please note that I have replaced sqlite3_complete with a simple search for ';'
			} while(/*!sqlite3_complete((const char*)m_trail)*/strchr((const char*)m_trail, ';') != NULL);
		}
		catch (CICSQLException& e)
		{
			OUTPUT_LOG("%s", e.errmsg.c_str());
		}
	}
	else
	{
		//////////////////////////////////////////////////////////////////////////
		// do not break on error
		try
		{
			//const char* oldTrail = (const char*)m_trail;
			do {
				do {
					try
					{
						result = SQLITE_ERROR;
						exec_sql_prepare();
						result=exec_sql_step();
					}
					catch (CICSQLException& e)
					{
						OUTPUT_LOG("%s", e.errmsg.c_str());
						/*if(strncmp(oldTrail, (const char*)m_trail, MAX_SQL_LENGTH) == 0)
						{
						throw e;
						}*/

						//////////////////////////////////////////////////////////////////////////
						// TODO:  search for the next sql statement in m_trail, and continue execution.
						// Right now, we will break it anyway as if bBreakOnError is true.
						throw e;
					}
				} while(result==SQLITE_SCHEMA);
			} while(/*!sqlite3_complete((const char*)m_trail)*/strchr((const char*)m_trail, ';') != NULL);
		}
		catch (...)
		{
			OUTPUT_LOG("severe error: the rest of sql script are not executed, even with BreakOnError set to false.\r\n");
		}
	}
	return result;
}
int DBEntity::ExecuteSqlStringFormated(const char *sql,...)
{
	if (!m_isValid)
		return E_FAIL;
	int result;
	try
	{
		/*va_list args;
		va_start(args, sql);
		result = sql_prepare(sql, args);
		va_end(args);

		result = sql_step();
		result = sql_finalize();*/

		char sql_[MAX_SQL_LENGTH + 1];
		va_list args;
		va_start(args, sql);
		vsnprintf((char*)sql_, MAX_SQL_LENGTH, sql, args);
		va_end(args);
		sql_[MAX_SQL_LENGTH] = 0;
		m_sql = sql_;
		m_trail = (char*)(&m_sql[0]);

		do {
			do {
				exec_sql_prepare();
				result=exec_sql_step();

			} while(result==SQLITE_SCHEMA);

		} while(sqlite3_complete((const char*)m_trail)==0);
	}
	catch (CICSQLException& e)
	{
		OUTPUT_LOG("%s", e.errmsg.c_str());
	}
	return result;
}

int DBEntity::sql_prepare(const char *sql,...)
{
	if (!m_isValid || sql==NULL) 
		return 0;

	char sql_[MAX_SQL_LENGTH + 1];
	va_list args;
	va_start(args, sql);
	vsnprintf((char*)sql_, MAX_SQL_LENGTH, sql, args);
	va_end(args);
	sql_[MAX_SQL_LENGTH] = 0;
	m_sql = sql_;
	m_trail = (char*)(&m_sql[0]);
	

	int errcode;
	const char*errmsg;
	if (SQLITE_OK!=(errcode=sqlite3_prepare(m_db,(char*)m_trail,-1,&m_stmt,(const char **)&m_trail))) {
		errmsg=sqlite3_errmsg(m_db);
		throw CICSQLException(errcode,errmsg);
	}
	return errcode;
}

int DBEntity::sql_step()
{
	if (!m_isValid||m_stmt==0) {
		return SQLITE_ERROR;
	}
	int errcode;
	const char *errmsg;
	errcode=sqlite3_step(m_stmt);
	if (SQLITE_ERROR==errcode) 
	{
		errmsg=sqlite3_errmsg(m_db);
		exec_sql_finish();
		throw CICSQLException(errcode,errmsg);
	}
	return errcode;
}

int DBEntity::sql_finalize()
{
	if (!m_isValid||m_stmt==NULL) {
		return SQLITE_OK;
	}
	int errcode;
	errcode= sqlite3_finalize(m_stmt);
	m_stmt=NULL;
	return errcode;
}


void DBEntity::exec_sql()
{
	if (!m_isValid) {
		return;
	}
	int errcode=0;
	do {
		do {
			exec_sql_prepare();
			errcode=exec_sql_step();

		} while(errcode==SQLITE_SCHEMA);

	} while(sqlite3_complete((const char*)m_trail)==0);
}
void DBEntity::exec_sql(const char *sql)
{
	if (!m_isValid) {
		return;
	}
	m_sql = sql;
	m_trail=(char*)(&m_sql[0]);
	exec_sql();
}
void DBEntity::exec_sql16()
{
	if (!m_isValid) {
		return;
	}
	int errcode=0;
	do {
		do {
			exec_sql_prepare16();
			errcode=exec_sql_step();
		} while(errcode==SQLITE_SCHEMA);

	} while(sqlite3_complete16(m_trail)==0);
}
void DBEntity::exec_sql16(const char16_t *sql)
{
	if (!m_isValid||sql==NULL) {
		return;
	}
	m_wsql = sql;
	m_trail=(void*)(&m_wsql[0]);
	exec_sql16();
}
sqlite3_stmt* DBEntity::exec_sql_prepare()
{
	if (!m_isValid) {
		return NULL;
	}
	int errcode;
	const char*errmsg;
	if (SQLITE_OK!=(errcode=sqlite3_prepare(m_db,(char*)m_trail,-1,&m_stmt,(const char **)&m_trail))) {
		errmsg=sqlite3_errmsg(m_db);
		throw CICSQLException(errcode,errmsg);
	}

	return m_stmt;
}

sqlite3_stmt* DBEntity::exec_sql_prepare16()
{
	if (!m_isValid) {
		return NULL;
	}
	int errcode;
	const char*errmsg;
	if (SQLITE_OK!=(errcode=sqlite3_prepare16(m_db,(char16_t*)m_trail,-1,&m_stmt,(const void **)&m_trail))) {
		errmsg=sqlite3_errmsg(m_db);
		throw CICSQLException(errcode,errmsg);
	}
	return m_stmt;
}
void DBEntity::prepare_sql(const char *sql,...)
{
	if (sql==NULL) {
		return;
	}
	char sql_[MAX_SQL_LENGTH + 1];
	va_list args;
	va_start(args, sql);
	vsnprintf((char*)sql_, MAX_SQL_LENGTH, sql, args);
	va_end(args);
	sql_[MAX_SQL_LENGTH] = 0;
	m_sql = sql_;
	m_trail = (char*)(&m_sql[0]);
}

void DBEntity::prepare_sql(const char16_t *sql,...)
{
	if (sql==NULL) {
		return;
	}
	WCHAR sql_[MAX_SQL_LENGTH + 1];
	va_list args;
	va_start(args, sql);
	vsnwprintf(sql_, MAX_SQL_LENGTH, (WCHAR*)sql, args);
	va_end(args);
	sql_[MAX_SQL_LENGTH] = L'\0';
	m_wsql = (char16_t*)sql_;
	m_trail = (void*)(&m_wsql[0]);

}
int DBEntity::exec_sql_step()
{
	if (!m_isValid||m_stmt==0) {
		return SQLITE_ERROR;
	}
	int errcode;
	const char *errmsg;
	errcode=sqlite3_step(m_stmt);
	if (SQLITE_DONE==errcode) {
		exec_sql_finish();
	}else if (SQLITE_ERROR==errcode) {
		errmsg=sqlite3_errmsg(m_db);
		exec_sql_finish();
		throw CICSQLException(errcode,errmsg);
	}
	return errcode;

}
int DBEntity::exec_sql_step(sqlite3_stmt* stmt)
{
	if (!m_isValid||m_stmt==0) {
		return SQLITE_ERROR;
	}
	int errcode;
	const char *errmsg;
	errcode=sqlite3_step(stmt);
	if (SQLITE_DONE==errcode) {
		exec_sql_finish();
	}else if (SQLITE_ERROR==errcode) {
		errmsg=sqlite3_errmsg(m_db);
		exec_sql_finish();
		throw CICSQLException(errcode,errmsg);
	}

	return errcode;
}

int DBEntity::exec_sql_finish()
{
	if (!m_isValid||m_stmt==NULL) {
		return SQLITE_ERROR;
	}
	int errcode;
	errcode= sqlite3_finalize(m_stmt);
	m_stmt=NULL;
	return errcode;
}
int DBEntity::exec_sql_finish(sqlite3_stmt* stmt)
{
	if (!m_isValid||stmt==NULL) {
		return SQLITE_ERROR;
	}
	int errcode;
	errcode= sqlite3_finalize(stmt);
	return errcode;
}

CICRecordSet* DBEntity::CreateRecordSet1(const char16_t* sql)
{
	if (sql==NULL) {
		return NULL;
	}
	m_trail=(char*)sql;
	CICRecordSet* value=CreateRecordSet();
	value->Initialize((char16_t*)sql);
	return value;
}
CICRecordSet* DBEntity::CreateRecordSet(const char16_t* sql,...)
{
	if (sql==NULL) {
		return NULL;
	}
	char16_t wsqltemp[MAX_SQL_LENGTH+1];
	va_list args;
	va_start(args, sql);
	vsnwprintf((WCHAR*)wsqltemp, MAX_SQL_LENGTH, (WCHAR*)sql, args);
	va_end(args);
	m_trail=(char*)wsqltemp;
	wsqltemp[MAX_SQL_LENGTH]=L'\0';
	CICRecordSet* value=CreateRecordSet();
	value->Initialize((char16_t*)wsqltemp);
	return value;
}
CICRecordSet* DBEntity::CreateRecordSet1(const char* sql)
{
	if (sql==NULL) {
		return NULL;
	}
	m_trail=(char*)sql;
	CICRecordSet* value=CreateRecordSet();
	value->Initialize((char*)sql);
	return value;
}

CICRecordSet* DBEntity::CreateRecordSet(const char* sql,...)
{
	if (sql==NULL) {
		return NULL;
	}
	char sqltemp[MAX_SQL_LENGTH+1];
	va_list args;
	va_start(args, sql);
	vsnprintf((char*)sqltemp, MAX_SQL_LENGTH, sql, args);
	va_end(args);
	sqltemp[MAX_SQL_LENGTH]=0;
	m_trail=(char*)sqltemp;
	CICRecordSet* value=CreateRecordSet();
	value->Initialize((char*)sqltemp);
	return value;
}

CICRecordSet* DBEntity::CreateRecordSet()
{
	DWORD a;
	for (a=0;a<m_RSpool.size();a++) {
		if (m_RSpool[a].second==false) {
			m_RSpool[a].second=true;
			m_RSpool[a].first->m_db=m_db;
			return m_RSpool[a].first;
		}
	}
	//garbage collects an unused recordset
	for (a=0;a<m_RSpool.size();a++) {
		if (!m_RSpool[a].first->m_isValid||m_RSpool[a].first->m_eof) {
			m_RSpool[a].first->Release();
			return m_RSpool[a].first;
		}
	}
	DWORD oldsize=(DWORD)m_RSpool.size();
	RSpair temp(new CICRecordSet(),false);
	m_RSpool.push_back(temp);	
	m_RSpool[a].second=true;
	m_RSpool[a].first->m_db=this->m_db;
	m_RSpool[a].first->m_db=m_db;
	return	m_RSpool[oldsize].first;
}

void DBEntity::DeleteRecordSet(CICRecordSet* recordset)
{
	DWORD a;
	if (recordset==NULL) {
		return;
	}
	for (a=0;a<m_RSpool.size();a++) {
		if ((m_RSpool[a].first)==recordset&&m_RSpool[a].second==true) {
			m_RSpool[a].first->Release();
			m_RSpool[a].second=false;
			return;
		}
	}
}

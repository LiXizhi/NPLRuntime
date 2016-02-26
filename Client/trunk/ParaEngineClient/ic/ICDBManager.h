#pragma once
#include "ParaDatabase.h"
#include <map>
#include <vector>
#include <util/mutex.h>

struct sqlite3_stmt;
struct sqlite3;

/** The maximum length of the sql string */
#ifndef MAX_SQL_LENGTH 
#define MAX_SQL_LENGTH 2047
#endif

namespace ParaInfoCenter
{
	class CICRecordSet;
	class CICDBManager;
	class DBEntity;

	typedef pair<CICRecordSet*,bool> RSpair;
	typedef pair<DBEntity*,bool> DBpair;

	/**
	* exceptions thrown from database 
	*/
	class CICSQLException
	{
	public:
		CICSQLException(int code,const char *zFormat,...);
		~CICSQLException();
		int errcode;
		string errmsg;
	};

	/**
	* a single database file.
	*/
	class DBEntity 
	{
		friend class CICMapTrace;
		friend class CICRoot;
		friend class CICGenManager;
		friend class CICDBManager;
	public:
		
		PE_CORE_DECL ~DBEntity();
		///////////////////////////////////////////////////////////////////////////
		//
		// database entity member functions
		//
		//////////////////////////////////////////////////////////////////////////
		PE_CORE_DECL void Release();

		/** add reference count of the asset. 
		* One may later order the asset manager to remove all asset whose reference count has dropped to 0 */
		void addref()
		{
			++m_refcount;
		}

		/** decrease reference count of the asset. 
		* One may later order the asset manager to remove all asset whose reference count has dropped to 0 
		* @return : return true if the the reference count is zero after decreasing it*/
		bool delref()
		{
			assert(m_refcount>0);
			return --m_refcount<=0;
		}
		/** get the reference count */
		int GetRefCount()
		{
			return m_refcount;
		}

		/** execute a sql script. It is equivalent to calling sql_prepare(), sql_step(), sql_finalize() in that order until all commands are completed.
		this function will handle error messages and will not throw exception. 
		@param sql: sql must be less than 2048 in length.
		*/
		PE_CORE_DECL int ExecuteSqlStringFormated(const char *sql,...);
		PE_CORE_DECL int ExecuteSqlStringFormated(const char16_t *sql,...);

		/**
		* execute a sql script. similar to ExecuteSqlStringFormated(), except that there is no length limit to sql string.
		* @param *sql this string is usually read from resource file
		* @param bBreakOnError default to false. if true, the script will break on any error; otherwise, 
		it will continue execute the following commands even when there are previous errors.
		* @return the most recent error code is returned.
		*/
		PE_CORE_DECL int ExecuteSqlScript(const char *sql, bool bBreakOnError = false);


		/** prepare a sql statement, and return the statement object. 
		CICSQLException exception may be thrown. */
		PE_CORE_DECL int sql_prepare(const char *sql,...);
		PE_CORE_DECL int sql_prepare(const char16_t *sql,...);

		/**
		* step through the current sql statement. 
		* @return If the return code is SQLITE_ERROR, 
		* CICSQLException exception will be thrown, the current sql statement will be automatically finalized. 
		*/
		PE_CORE_DECL int sql_step();

		/** finalize the current statement if it has not been finalized. */
		PE_CORE_DECL int sql_finalize();

		/** get the current statement*/
		sqlite3_stmt* GetStatement(){return m_stmt;}

		/** ensure that the database is opened. */
		PE_CORE_DECL void OpenDB();

		/** ensure that the database is closed. */
		PE_CORE_DECL void CloseDB();

		/**
		* return true if database is now successfully opened.
		* @return 
		*/
		PE_CORE_DECL bool IsValid() const {return m_isValid;};
		/**
		* get the connection string. It is currently the file path.
		* @return 
		*/
		PE_CORE_DECL const string& GetConnectionString(){return m_filepath;};

		/**
		* get sqlite3 object
		* @return 
		*/
		sqlite3 *GetDBHandle(){return m_db;};

		/**
		* Create a managed sql statement(procedure). One can bind data field and execute it through the CICRecordSet,class
		* @param sql sql commands
		* @param ... 
		* @return sql statement object is returned. 
		*/
		PE_CORE_DECL CICRecordSet* CreateRecordSet(const char* sql,...);
		PE_CORE_DECL CICRecordSet* CreateRecordSet(const char16_t* sql,...);
		/** similar to CreateRecordSet(const char* sql,...), except that there is no limit to input length*/
		PE_CORE_DECL CICRecordSet* CreateRecordSet1(const char* sql);
		PE_CORE_DECL CICRecordSet* CreateRecordSet1(const char16_t* sql);
		PE_CORE_DECL CICRecordSet *CreateRecordSet();
		/**
		* explicitly release a record set. 
		* @param recordset 
		*/
		PE_CORE_DECL void DeleteRecordSet(CICRecordSet* recordset);
		PE_CORE_DECL int64 GetLastInsertRowID();
	
		/** get if database file is opened by creating a new database file. 
		* one may want to install all tables and initial data if a database file is newly created. 
		*/
		PE_CORE_DECL bool IsCreateFile();

		/** set if database file is opened by creating a new database file. 
		* one may want to install all tables and initial data if a database file is newly created. 
		*/
		PE_CORE_DECL void SetCreateFile(bool bCreateFile);

	protected:
		/* @obsolete the following exec_sql* function are not used. Maybe obsolete soon
		* These functions do not guarantee the ' in text field are properly replaced by ''
		* The exec_sql* function will execute the given sql string completely. It will execute the string as a batch operation
		*/

		/**
		* execute the command until it is completed. 
		* @param *sql 
		*/
		void exec_sql16(const char16_t *sql);
		void exec_sql(const char *sql);
		/**
		* it is always used with exec_sql16(). e.g. prepare_sql(...); exec_sql();
		* @param *sql 
		* @param ... 
		*/
		void prepare_sql(const char16_t *sql,...);
		void prepare_sql(const char *sql,...);
		void exec_sql();
		void exec_sql16();

		sqlite3_stmt* exec_sql_prepare();
		sqlite3_stmt* exec_sql_prepare16();
		int exec_sql_finish();
		int exec_sql_finish(sqlite3_stmt* stmt);
		int exec_sql_step();
		int exec_sql_step(sqlite3_stmt* stmt);

		/**
		* open database file
		* @param dbname 
		*/
		void OpenDB16(const char16_t* dbname);
		void OpenDB(const char* dbname);
		/** prepare the database file before open it. Some database files are in archives, 
		* we will first copy them to a temporary disk location and then load the disk file instead.
		* @param filename: the file to be prepared. it can be ":memory:", a disk file or a file in a zip file. 
		* @return always returns the disk file path or ":memory:", it is "", if the file is not valid. 
		*/
		string PrepareDatabaseFile(const string& filename);

	protected:
		DBEntity();
		void init();
		/** database handle */
		sqlite3 *m_db;
		/** a pool of sql statements created by the user.*/
		vector<RSpair> m_RSpool;
		/** alias name of the db */
		u16string m_name;
		/** db file path */
		string m_filepath;
		/** whether db is opened. */
		bool m_isValid;
		/** whether the database use UTF-8 encoding; if false it will be UTF-16 unicode encoding. */
		bool m_bEncodingUTF8;

		/** this is true if database file is opened by creating a new database file. */
		bool m_bIsCreateFile;

		/** the opening flags for sqlite3_open_v2(). default to SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE .  */
		int m_nSQLite_OpenFlags;

		/* the following only used for sqlite3 wrapper. */
		std::u16string m_wsql;
		std::string m_sql;
		void * m_trail;
		sqlite3_stmt *m_stmt;

		/** this makes all db query thread-safe */
		ParaEngine::mutex	m_mutex;

		/** reference count of the asset.
		* Asset may be referenced by scene objects. 
		* Once the reference count drops to 0, the asset may be unloaded, due to asset garbage collection.
		* however, the asset is not completed removed. Pointers to this object will still be valid.
		* only resources (such as texture, mesh data, etc) are unloaded.
		*/
		int m_refcount;
	};

	/**
	* it represents both a database entity and a database manager
	* the static functions are for database manager
	* basically use OpenDBEntity and CloseDBEntity;
	* this class has two alias class ParaEngine::DBEntity and ParaEngine::CDBManager
	
	@par code sample. 
		//we suppose you already know how to open a database using CICGenManager, and the open operation is done before this sample
		CICRecordSet *rs=new CICRecordSet("SELECT * FROM maptrace");
		//a new recordset always is BOF and EOF
		if (rs->IsBOF()) {
		//NextRow go to the next row of the return, here we go to the first row
		//return of NextRow is CICRecordSet::SOK if ok.
		//it is CICRecordSet::SEOF if the next row is beyond the end of the rows
		rs->NextRow();
		//the sql string is not executed before the first call to NextRow
		}
		if (rs->IsEOF()) {
		//nothing returns from the sql string, or the string is not correct
		throw;
		}
		while(!rs->IsEOF()){
		//remember to use (*rs)[x] to access the x-th column of the row if rs is (CICRecordSet *)
		//here an auto-datatype-casting is done internally, so you don't need to care about it.
		int time=(*rs)[0];
		double x,y,z;
		x=(*rs)[1];
		y=(*rs)[2];
		z=(*rs)[3];
		const char* id=(*rs)[4];
		//warning: do not put (*rs)[x] directly into the following function, you should always assign them to a clear type variable before using them
		// because the type-casting function must know which type it is casting.
		OUTPUT_LOG("time = %d x=%f y=%f z=%f id=%s",time,x,y,z,id);
		rs->NextRow();
		} 
		//when end of the recordset, the operation is automatically finalize so sqlite3_finalize is not needed.
	*/
	class CICDBManager
	{
	
	public:
		///////////////////////////////////////////////////////////////////////////
		//
		// database management functions
		//
		//////////////////////////////////////////////////////////////////////////
		/**
		* get the database with a name or file path
		* @param name 
		* @return 
		*/
		PE_CORE_DECL static DBEntity * GetDB(const char16_t* name);
		PE_CORE_DECL static DBEntity *OpenDBEntity();
		/**
		* only load if not loaded before.
		* @param dbname path of the database file
		* @return database entity
		*/
		PE_CORE_DECL static DBEntity *OpenDBEntity(const char16_t* dbname);
		PE_CORE_DECL static DBEntity *OpenDBEntity(const char* dbname);
		/**
		* @param name alias name
		* @param dbname database path
		* @return database entity
		*/
		PE_CORE_DECL static DBEntity *OpenDBEntity(const char16_t* name, const char16_t* dbname);
		PE_CORE_DECL static DBEntity *OpenDBEntity(const char* name, const char* dbname);
		/**
		* close the database entity, one should never use a closed entity any more. 
		* @param pEntity
		*/
		PE_CORE_DECL static bool CloseDBEntity(DBEntity * pEntity);

		/// delete all db managers
		PE_CORE_DECL static void Finalize();
		PE_CORE_DECL static void StaticInit();
	
	protected:
		/** all databases */
		static vector<DBpair> m_DBpool;

		/** this makes all db query thread-safe */
		static ParaEngine::mutex	m_mutex;
	
	public:
		friend class CICMapTrace;
		friend class CICRoot;
		friend class CICGenManager;
	};
}
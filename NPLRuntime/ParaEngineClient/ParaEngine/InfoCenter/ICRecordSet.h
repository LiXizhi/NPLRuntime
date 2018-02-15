#pragma once
#include "SQLStatement.h"
struct sqlite3_stmt;
struct sqlite3;

namespace ParaInfoCenter
{
	class CICDBManager;
	class DBEntity;

	/** data field in a record. */
	class PE_CORE_DECL CICRecordSetItem
	{
	public:
		enum IC_DATA_TYPE{
			_NONE,_INT,_FLOAT, _INT64,_TEXT,_TEXT16,_BLOB
		};
		//they will return 0 or NULL if the index is not valid
		//you can get data of the same item several times without any performance penalty 
		operator int();
		operator bool();
		operator DWORD();
		operator int64();
		operator char*();
		operator char16_t*();
		operator void*();
		operator double();
		operator float();
		const char *GetNameA();
		const char16_t *GetName();
		CICRecordSetItem(int index,sqlite3_stmt *stmt);
		~CICRecordSetItem();
		CICRecordSetItem& operator =(const CICRecordSetItem&);
		CICRecordSetItem& operator =(const int&);
		CICRecordSetItem& operator =(const int64&);
		CICRecordSetItem& operator =(const bool&);
		CICRecordSetItem& operator =(const double&);
		CICRecordSetItem& operator =(const float&);
		CICRecordSetItem& operator =(const char*);
		CICRecordSetItem& operator =(const char16_t*);
		void SetBlob(const void *value,int nlength);
		friend class CICRecordSet;
		/**
		* in bytes. This is usually used for blob(binary) record
		* @return 
		*/
		int GetDataLength(){return m_length;};
	protected:
		void CleanOldData();
		int m_index;
		sqlite3_stmt *m_stmt;
		bool m_needInit;
		int m_type;
		char *m_sName;
		char16_t *m_wName;
		int m_length;
		union{
			void *bData;
			int iData;
			int64 nDataInt64;
			double fData;
			char *sData;
			char16_t *wData;
		};
	};

	/** CICRecordSet is both the result and wrapper of a sql statement(procedure). 
	* All CICRecordSet instances are managed automatically by the CICDBManager. Users should not new or delete 
	* any CICRecordSet instance. Use CICDBManager's CreateRecordSet and DeleteRecordSet to get a recordset instance 
	* or release an instance. 
	*
	* @par remark
	* Because the CICRecordSet instances are managed by CICDBManager, the garbage collection will happen constantly. 
	* If a recordset is invalid or reaches its end (IsEOF()==true), the garbage collector will assign the 
	* recordset instance to other use. So all reference to that instance will not be correct. 
	* 
	* The recordset is invalid when:
	* - it was created without any sql string
	* - the sql string is incorrect
	* - the sqlite statement is finalized.
	* - some other errors.
	* 
	* The recordset is IsEOF()==true when
	* - it is invalid, in most cases, but I don't guarantee that, because these two flags are not binding together.
	* - the sql string returns nothing
	* - after calling NextRow() when the current row is the last row
	*
	* The recordset is IsBOF()==true before the first NextRow() is called;
	*/
	class CICRecordSet : public CSQLStatement
	{
		friend class DBEntity;
	public:
		static int SOK;
		static int SEOF;
		static int SBOF;
		/**
		* We can use rs[x] form sentence to get the x-the item of the current 
		*/
		PE_CORE_DECL CICRecordSetItem& operator[](int index);
		PE_CORE_DECL void Initialize(sqlite3_stmt* stmt);
		PE_CORE_DECL void Initialize(const char* sql);
		PE_CORE_DECL void Initialize(const char16_t *sql);
		PE_CORE_DECL void Release();
		//return 0 means ok, otherwise sth is wrong, either eof or bof or other error
		PE_CORE_DECL int NextRow();
		PE_CORE_DECL int ColumnCount(){return m_columnNum;}
		PE_CORE_DECL const char* GetColumnName(int index);
		PE_CORE_DECL const char16_t* GetColumnName16(int index);

		PE_CORE_DECL int64 GetLastInsertRowID();
		PE_CORE_DECL ~CICRecordSet();
		//void operator delete(void *p);
	protected:
		/*		CICRecordSet(sqlite3_stmt* stmt);
		CICRecordSet(const char *sql);
		CICRecordSet(const char16_t* sql);*/
		CICRecordSet();
		vector<CICRecordSetItem*> m_items;

		CICRecordSetItem* m_empty;
		bool m_bUpdatable;
		int m_columnNum;//number of columns in a row

	};
}
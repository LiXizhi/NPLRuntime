#pragma once

struct sqlite3_stmt;
struct sqlite3;

namespace ParaInfoCenter
{
	/**
	 this represents SQL statement or stored procedure. 
	*/
	class CSQLStatement
	{
		friend class DBEntity;
	public:
		CSQLStatement(void);
		virtual ~CSQLStatement(void);
	public:
		//just the same as NextRow, but sounds more like executing a sql string
		//but it does not tell whether the execute returns a row. it returns SQLITE_OK if success, and SQLITE_ERROR if failed;
		//it will invalidate the recordset.
		int Execute();

		/*
		* This function is called to reset a compiled SQL
		* statement obtained by a previous call to sqlite3_prepare() or
		* sqlite3_prepare16() back to it's initial state, ready to be re-executed.
		* Any SQL statement variables that had values bound to them using
		* the DataBinging() API retain their values.
		*/
		int Reset();

		int DataBinding(int index,double value);
		int DataBinding(const char* name,double value);

		int DataBinding(int index,int value);
		int DataBinding(const char* name,int value);

		int DataBinding(int index,int64 value);
		int DataBinding(const char* name,int64 value);

		int DataBindingNull(int index);
		int DataBindingNull(const char* name);

		int DataBinding(int index,const char*value);
		int DataBinding(const char* name,const char*value);

		int DataBinding(int index,const WCHAR*value);
		int DataBinding(const char* name,const WCHAR*value);

		int DataBinding(int index,const void*value,int size);
		int DataBinding(const char* name,const void*value,int size);

		/**
		* The recordset is IsEOF()==true when
		* - it is invalid, in most cases, but I don't guarantee that, because these two flags are not binding together.
		* - the sql string returns nothing
		* - after calling NextRow() when the current row is the last row
		*
		* @return 
		*/
		bool IsEOF(){return m_eof;};

		/**
		* The recordset is IsBOF()==true before the first NextRow() is called;
		* in other words, no DataBinding should occur if it is BOF is true.
		* @return 
		*/
		bool IsBOF(){return m_bof;};

		/**
		* The recordset is invalid when:
		* - it was created without any sql string
		* - the sql string is incorrect
		* - the sqlite statement is finalized.
		* - some other errors.
		* @return 
		*/
		bool IsValid(){return m_isValid;};

	protected:
		sqlite3_stmt *m_stmt;
		sqlite3 *m_db;
		bool m_isValid;
		bool m_eof;
		bool m_bof;

	};

}

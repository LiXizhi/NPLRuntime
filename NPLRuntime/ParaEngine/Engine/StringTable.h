#pragma once
#include "InfoCenter/ParaDatabase.h"
#include "DataProviderManager.h"

namespace ParaEngine
{
	struct stStringTableDbRecord
	{
		/** Common Attribute */
		__int64 ID;
		string StringEnglish;
		string StringSimplifiedChinese;

		/** Reserved Attribute */
		string Reserved1;
		string Reserved2;
		string Reserved3;
		string Reserved4;
		string Reserved5;
	};

	// Class Description: CStringTableDB contains all functions to access the string table database. 
	//		This is a singleton class. Call GetInstance() to use this class.
	class CStringTableDB
	{
	public:
		CStringTableDB(void);
		CStringTableDB(ParaInfoCenter::DBEntity* pDb);
		~CStringTableDB(void);
	protected:
	private:

		/** base database interface */
		ParaInfoCenter::DBEntity* m_pDataBase;

		CDBRecordSet* m_pProcSelectByID_EN;
		CDBRecordSet* m_pProcSelectByID_CN;

	public:
		/** Get string from ID
		* @param sString: target ID string table entry
		* @return true if the record is found in database. */
		bool GetStringbyID(string& sString, const __int64& ID);

		/** Insert the new string table entry to StringTable_DB
		* @param entry: ID of entry will be ignored and filled with actual ID if inserted successfully
		* @return true if the record is inserted in database
		*/
		bool InsertString(stStringTableDbRecord& record);

		/**
		* Update the data fields of StringTable_DB.
		* @param entry: ID field of the entry must be valid.
		* @return true if the record is updated in database.
		*/
		bool UpdateStringTable(const stStringTableDbRecord& record);

		/** Delete string table entry
		* @return true if the record is deleted in database. */
		bool DeleteEntryByID(const __int64& ID);

		/** clean up all stored procedure*/
		void Cleanup();
	public:		
		/** get the singleton class.*/
		static CStringTableDB & GetInstance();
		/** get the database interface. */
		ParaInfoCenter::DBEntity* GetDBEntity();
		/** set the database interface. */
		void SetDBEntity(ParaInfoCenter::DBEntity* pDb);

	public:
		// TODO: only for test
		void TestDB();
	};
}

#pragma once
#include "ic/ParaDatabase.h"
#include ".\StringTable.h"

namespace ParaEngine
{
	struct stTitleDbRecord
	{
		/** Common Attribute */
		__int64 ID;
		int TitleLevel;
		bool IsBoy;
		stStringTableDbRecord Title; // record in string table

		/** Reserved Attribute */
		string Reserved1;
		string Reserved2;
		string Reserved3;
		string Reserved4;
		string Reserved5;
	};

	// Class CTitleDBProvider: CTitleDBProvider contains all functions to access the 
	//		title database and the user interface data of title control. 
	//		This is a singleton class. Call GetInstance() to use this class.
	class CTitleDBProvider
	{
	public:
		CTitleDBProvider(void);
		CTitleDBProvider(ParaInfoCenter::DBEntity* pDb);
		~CTitleDBProvider(void);
	protected:
	private:

		/** base database interface */
		ParaInfoCenter::DBEntity* m_pDataBase;

	public:		
		/** get the singleton class.*/
		static CTitleDBProvider & GetInstance();
		/** get the database interface. */
		ParaInfoCenter::DBEntity* GetDBEntity();
		/** set the database interface. */
		void SetDBEntity(ParaInfoCenter::DBEntity* pDb);

//#define TEST_TITLE
#ifdef TEST_TITLE
	public:
		// TODO: only for test
		void TestDB();
#endif

	public:
		/** Insert the new title record to Title_DB
		* @param record: ID of record will be ignored and filled with actual ID if inserted successfully
		* @return true if the record is inserted in database
		*/
		bool InsertTitleRecord(stTitleDbRecord& record);

		/** delete the existing title record from Title_DB
		* @param ID: ID of target record
		* @return true if the record is deleted in database
		*/
		bool DeleteTitleRecordByID(int ID);
	};
}
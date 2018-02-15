#pragma once
#include "InfoCenter/ParaDatabase.h"
#include ".\StringTable.h"

namespace ParaEngine
{
	struct stItemDbRecord
	{
		// TODO: change the db_item.sql to new version

		/** Common Attribute */
		__int64 ID;
		stStringTableDbRecord ItemName;	// record in string table

		/** Engine Asset Management Attribute */
		string IconAssetName;
		string DescTextAssetName;
		bool IsGlobal; // stored in db 0/1

		/** UI Related Attribute */
		string IconFilePath;
		/** Desc: text shown when mouse over the icon */
		stStringTableDbRecord Desc;	// record in string table
		string ModelFilePath;

		/** Kids Game Related Attribute */
		int ItemType;
		/** NOTE: IsObtained is a tag specify if player can build */
		bool IsObtained; // stored in db 0/1
		bool IsUnique; // stored in db 0/1
		/** TODO: if special item need cost exp and fruit */
		int CostExperiencePt; // stored in db 0/1
		/** TODO: Change according to game design */
		int CostOrange;
		int CostApple;
		int CostPeach;

		/** Reserved Attribute */
		string Reserved1;
		string Reserved2;
		string Reserved3;
		string Reserved4;
		string Reserved5;
	};

	// Class CItemDBProvider: CItemDBProvider contains all functions to access the 
	//		item database and the user interface data of item control. 
	//		This is a singleton class. Call GetInstance() to use this class.
	class CItemDBProvider
	{
	public:
		CItemDBProvider(void);
		CItemDBProvider(ParaInfoCenter::DBEntity* pDb);
		~CItemDBProvider(void);
	protected:
	private:

		/** base database interface */
		ParaEngine::asset_ptr<ParaInfoCenter::DBEntity> m_pDataBase;

	public:		
		/** get the singleton class.*/
		static CItemDBProvider & GetInstance();
		/** get the database interface. */
		ParaInfoCenter::DBEntity* GetDBEntity();
		/** set the database interface. */
		void SetDBEntity(ParaInfoCenter::DBEntity* pDb);

//#define TEST_ITEM
#ifdef TEST_ITEM
	public:
		// TODO: only for test
		void TestDB();
#endif

	public:
		/** Insert the new puzzle record to Puzzle_DB
		* @param record: ID of record will be ignored and filled with actual ID if inserted successfully
		* @return true if the record is inserted in database
		*/
		bool InsertItemRecord(stItemDbRecord& record);

		/** delete the existing puzzle record from Puzzle_DB
		* @param ID: ID of target record
		* @return true if the record is deleted in database
		*/
		bool DeleteItemRecordByID(int ID);

		/** Update existing item record of Item_DB
		* @param record: update record with actual ID in record
		* @return true if the record is updated in database
		*/
		bool UpdateItemRecord(stItemDbRecord& record);

		/** Select existing item record of Item_DB
		* @param record: select record by ID
		* @return true if the record is selected in database
		*/
		bool SelectItemRecordByID(stItemDbRecord& record, __int64 ID);
	};
}
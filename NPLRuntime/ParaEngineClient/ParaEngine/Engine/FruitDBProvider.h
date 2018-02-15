#pragma once
#include "InfoCenter/ParaDatabase.h"
#include ".\StringTable.h"

namespace ParaEngine
{
	struct stFruitDbRecord
	{
		/** Common Attribute */
		__int64 ID;
		/** ATTENTION: different chests may have the same name */
		stStringTableDbRecord FruitName;	// record in string table

		/** Engine Asset Management Attribute */
		string AssetName;
		bool IsSnapToTerrain;

		/** Appearance Related Attribute */
		string FruitModeFilePath;
		float FruitModelScale;
		float FruitRadius;
		float FruitFacing;
		float posX;
		float posY;
		float posZ;

		/** Reward Attribute */
		string Reward;

		/** Reserved Attribute */
		string Reserved1;
		string Reserved2;
		string Reserved3;
		string Reserved4;
		string Reserved5;
	};

	// Class CFruitDBProvider: CFruitDBProvider contains all functions to access the 
	//		fruit database and the user interface data of fruit control. 
	//		This is a singleton class. Call GetInstance() to use this class.
	class CFruitDBProvider
	{
	public:
		CFruitDBProvider(void);
		CFruitDBProvider(ParaInfoCenter::DBEntity* pDb);
		~CFruitDBProvider(void);
	protected:
	private:

		/** base database interface */
		ParaEngine::asset_ptr<ParaInfoCenter::DBEntity> m_pDataBase;

	public:		
		/** get the singleton class.*/
		static CFruitDBProvider & GetInstance();
		/** get the database interface. */
		ParaInfoCenter::DBEntity* GetDBEntity();
		/** set the database interface. */
		void SetDBEntity(ParaInfoCenter::DBEntity* pDb);

//#define TEST_FRUIT
#ifdef TEST_FRUIT
	public:
		// TODO: only for test
		void TestDB();
#endif

	public:
		/** Insert the new fruit record to Fruit_DB
		* @param record: ID of record will be ignored and filled with actual ID if inserted successfully
		* @return true if the record is inserted in database
		*/
		bool InsertFruitRecord(stFruitDbRecord& record);

		/** delete the existing fruit record from Fruit_DB
		* @param ID: ID of target record
		* @return true if the record is deleted in database
		*/
		bool DeleteFruitRecordByID(int ID);
	};
}
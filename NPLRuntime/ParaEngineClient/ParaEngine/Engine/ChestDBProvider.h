#pragma once
#include "InfoCenter/ParaDatabase.h"
#include ".\StringTable.h"

namespace ParaEngine
{
	struct stChestDbRecord
	{
		/** Common Attribute */
		__int64 ID;
		/** ATTENTION: different chest may have the same name */
		stStringTableDbRecord ChestName;	// record in string table

		/** Engine Asset Management Attribute */
		string AssetName;
		bool IsSnapToTerrain;

		/** Appearance Related Attribute */
		string ChestModeFilePath;
		float ChestModelScale;
		float ChestRadius;
		float ChestFacing;
		float posX;
		float posY;
		float posZ;

		/** Difficulty Attribute */
		int DifficultyRate;	/** difficulty rate where puzzles are randomly picked */
		int NumberOfAllPuzzles;
		int NumberOfPassPuzzles;

		/** Reward Attribute */
		string Reward;

		/** Reserved Attribute */
		string Reserved1;
		string Reserved2;
		string Reserved3;
		string Reserved4;
		string Reserved5;
	};

	// Class CChestDBProvider: CChestDBProvider contains all functions to access the 
	//		chest database and the user interface data of chest control. 
	//		This is a singleton class. Call GetInstance() to use this class.
	class CChestDBProvider
	{
	public:
		CChestDBProvider(void);
		CChestDBProvider(ParaInfoCenter::DBEntity* pDb);
		~CChestDBProvider(void);
	protected:
	private:

		/** base database interface */
		ParaEngine::asset_ptr<ParaInfoCenter::DBEntity> m_pDataBase;

	public:		
		/** get the singleton class.*/
		static CChestDBProvider & GetInstance();
		/** get the database interface. */
		ParaInfoCenter::DBEntity* GetDBEntity();
		/** set the database interface. */
		void SetDBEntity(ParaInfoCenter::DBEntity* pDb);

//#define TEST_CHEST
#ifdef TEST_CHEST
	public:
		// TODO: only for test
		void TestDB();
#endif

	public:
		/** Insert the new chest record to Chest_DB
		* @param record: ID of record will be ignored and filled with actual ID if inserted successfully
		* @return true if the record is inserted in database
		*/
		bool InsertChestRecord(stChestDbRecord& record);

		/** delete the existing chest record from Chest_DB
		* @param ID: ID of target record
		* @return true if the record is deleted in database
		*/
		bool DeleteChestRecordByID(int ID);
	};
}
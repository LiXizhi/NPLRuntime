#pragma once
#include "ic/ParaDatabase.h"
#include ".\StringTable.h"

namespace ParaEngine
{
	struct stQuestDbRecord
	{
		/** Common Attribute */
		__int64 ID;
		stStringTableDbRecord QuestName;	// record in string table

		/** Engine Asset Management Attribute */
		string TopSnapShotAssetName;
		string IntroVideoAssetName;
		bool IsGlobal;

		/** UI Related Attribute */
		string TopSnapShotFilePath;
		string IntroVideoFilePath;

		/** Quest Content Attribute */
		bool IsComplete;
		stStringTableDbRecord QuestDesc;	// record in string table
		string AcquireQuestRequirement;
		string CompleteQuestRequirement;
		string Reward;

		/** Reserved Attribute */
		string Reserved1;
		string Reserved2;
		string Reserved3;
		string Reserved4;
		string Reserved5;
	};

	// Class CQuestDBProvider: CQuestDBProvider contains all functions to access the 
	//		quest database and the user interface data of quest control. 
	//		This is a singleton class. Call GetInstance() to use this class.
	class CQuestDBProvider
	{
	public:
		CQuestDBProvider(void);
		CQuestDBProvider(ParaInfoCenter::DBEntity* pDb);
		~CQuestDBProvider(void);
	protected:
	private:

		/** base database interface */
		ParaEngine::asset_ptr<ParaInfoCenter::DBEntity> m_pDataBase;

	public:		
		/** get the singleton class.*/
		static CQuestDBProvider & GetInstance();
		/** get the database interface. */
		ParaInfoCenter::DBEntity* GetDBEntity();
		/** set the database interface. */
		void SetDBEntity(ParaInfoCenter::DBEntity* pDb);

//#define TEST_QUEST
#ifdef TEST_QUEST
	public:
		// TODO: only for test
		void TestDB();
#endif

	public:
		/** Insert the new quest record to Quest_DB
		* @param record: ID of record will be ignored and filled with actual ID if inserted successfully
		* @return true if the record is inserted in database
		*/
		bool InsertQuestRecord(stQuestDbRecord& record);

		/** delete the existing quest record from Quest_DB
		* @param ID: ID of target record
		* @return true if the record is deleted in database
		*/
		bool DeleteQuestRecordByID(int ID);
	};
}

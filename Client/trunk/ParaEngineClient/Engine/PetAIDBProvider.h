#pragma once
#include "ic/ParaDatabase.h"

namespace ParaEngine
{
	struct stPetAIDbRecord
	{
		/** Common Attribute */
		__int64 ID;
		string PetAIName;
		string PetAIDesc;
		/** TODO: Such as AddEvent() .etc */

		/** Reserved Attribute */
		string Reserved1;
		string Reserved2;
		string Reserved3;
		string Reserved4;
		string Reserved5;
	};

	class CPetAIDBProvider
	{
	public:
		CPetAIDBProvider(void);
		CPetAIDBProvider(ParaInfoCenter::DBEntity* pDb);
		~CPetAIDBProvider(void);
	protected:
	private:

		/** base database interface */
		ParaEngine::asset_ptr<ParaInfoCenter::DBEntity> m_pDataBase;

	public:		
		/** get the singleton class.*/
		static CPetAIDBProvider & GetInstance();
		/** get the database interface. */
		ParaInfoCenter::DBEntity* GetDBEntity();
		/** set the database interface. */
		void SetDBEntity(ParaInfoCenter::DBEntity* pDb);

//#define TEST_PET_AI
#ifdef TEST_PET_AI
	public:
		// TODO: only for test
		void TestDB();
#endif

	public:
		/** Insert the new pet AI record to PetAI_DB
		* @param record: ID of record will be ignored and filled with actual ID if inserted successfully
		* @return true if the record is inserted in database
		*/
		bool InsertPetAIRecord(stPetAIDbRecord& record);

		/** delete the existing pet AI record from PetAI_DB
		* @param ID: ID of target record
		* @return true if the record is deleted in database
		*/
		bool DeletePetAIRecordByID(int ID);
	};
}
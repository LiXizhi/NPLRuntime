#pragma once
#include "InfoCenter/ParaDatabase.h"
#include ".\StringTable.h"
#include ".\PetAIDBProvider.h"

#include <vector>

namespace ParaEngine
{
	struct stPetDbRecord
	{
		/** Common Attribute */
		__int64 ID;
		stStringTableDbRecord PetName; // record in string table

		/** Engine Asset Management Attribute */
		string AssetName;
		bool IsGlobal;
		bool IsSnapToTerrain;

		/** Appearance Related Attribute */
		string PetModeFilePath;
		float PetModelScale;
		float PetRadius;
		float PetFacing;
		float posX;
		float posY;
		float posZ;

		/** AI Related Attribute */
		// NOTE: record in database FORMAT "ID,ID,ID,......,ID"
		vector<__int64> PetAIIndexVector;	/** index list into Pet AI entries*/
		float PerceptiveRadius;
		float SentientRadius;
		string OnLoadScript;

		/** Kids Game Related Attribute */
		bool IsKeptinCage;

		/** Reserved Attribute */
		string Reserved1;
		string Reserved2;
		string Reserved3;
		string Reserved4;
		string Reserved5;
	};

	// Class CPetDBProvider: CPetDBProvider contains all functions to access the 
	//		pet database and the user interface data of pet control. 
	//		This is a singleton class. Call GetInstance() to use this class.
	class CPetDBProvider
	{
	public:
		CPetDBProvider(void);
		CPetDBProvider(ParaInfoCenter::DBEntity* pDb);
		~CPetDBProvider(void);
	protected:
	private:

		/** base database interface */
		ParaEngine::asset_ptr<ParaInfoCenter::DBEntity> m_pDataBase;

	public:		
		/** get the singleton class.*/
		static CPetDBProvider & GetInstance();
		/** get the database interface. */
		ParaInfoCenter::DBEntity* GetDBEntity();
		/** set the database interface. */
		void SetDBEntity(ParaInfoCenter::DBEntity* pDb);

//#define TEST_PET
#ifdef TEST_PET
	public:
		// TODO: only for test
		void TestDB();
#endif

	public:
		/** Insert the new pet record to Pet_DB
		* @param record: ID of record will be ignored and filled with actual ID if inserted successfully
		* @return true if the record is inserted in database
		*/
		bool InsertPetRecord(stPetDbRecord& record);

		/** delete the existing pet record from Pet_DB
		* @param ID: ID of target record
		* @return true if the record is deleted in database
		*/
		bool DeletePetRecordByID(int ID);
	};
}
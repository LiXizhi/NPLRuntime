#pragma once
#include "ic/ParaDatabase.h"

#include ".\StringTable.h"

namespace ParaEngine
{
	struct stCharacterDbRecord
	{
		/** Common Attribute */
		__int64 ID;
		stStringTableDbRecord CharacterName; // record in string table
		int NumberOfPets; // limit the pet number to 3
		__int64 MainPetIndex; // index in pet table
		__int64 SecondaryPetIndex;	// index in pet table
		__int64 ThirdPetIndex;		// index in pet table

		/** Engine Asset Management Attribute */
		string AssetName;
		bool IsGlobal;
		bool IsSnapToTerrain;

		/** Appearance Related Attribute */
		string CharacterModeFilePath;
		float CharacterModelScale;
		float CharacterRadius;
		float CharacterFacing;
		float posX;
		float posY;
		float posZ;

		/** AI Related Attribute */
		/** CharacterAIEntryIndex INTEGER, 	/** Reserved for character AI*/
		float PerceptiveRadius;
		float SentientRadius;
		string OnLoadScript;

		/** Kids Game Related Attribute */
		/** ATTENTION: different characters may have the same title */
		__int64 CharacterTitleIndex;	// index in title table
		int ExperiencePt;
		/** TODO: Change according to game design */
		int Orange;
		int Apple;
		int Peach;

		/** Reserved Attribute */
		string Reserved1;
		string Reserved2;
		string Reserved3;
		string Reserved4;
		string Reserved5;
	};

	// Class CCharacterDBProvider: CCharacterDBProvider contains all functions to access the 
	//		character database and the user interface data of character control. 
	//		This is a singleton class. Call GetInstance() to use this class.
	class CCharacterDBProvider
	{
	public:
		CCharacterDBProvider(void);
		CCharacterDBProvider(ParaInfoCenter::DBEntity* pDb);
		~CCharacterDBProvider(void);

	protected:
	private:

		/** base database interface */
		ParaEngine::asset_ptr<ParaInfoCenter::DBEntity> m_pDataBase;

	public:		
		/** get the singleton class.*/
		static CCharacterDBProvider & GetInstance();
		/** get the database interface. */
		ParaInfoCenter::DBEntity* GetDBEntity();
		/** set the database interface. */
		void SetDBEntity(ParaInfoCenter::DBEntity* pDb);

//#define TEST_CHARACTER
#ifdef TEST_CHARACTER
	public:
		// TODO: only for test
		void TestDB();
#endif

	public:
		/** Insert the new character record to Character_DB
		* @param record: ID of record will be ignored and filled with actual ID if inserted successfully
		* @return true if the record is inserted in database
		*/
		bool InsertCharacterRecord(stCharacterDbRecord& record);

		/** delete the existing character record from Character_DB
		* @param ID: ID of target record
		* @return true if the record is deleted in database
		*/
		bool DeleteCharacterRecordByID(int ID);
	};
}
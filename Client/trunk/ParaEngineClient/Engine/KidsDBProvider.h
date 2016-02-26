#pragma once
#include "ic/ParaDatabase.h"
#include "DataProviderManager.h"

#include "CharacterDBProvider.h"
#include "ChestDBProvider.h"
#include "FruitDBProvider.h"
#include "ItemDBProvider.h"
#include "PetAIDBProvider.h"
#include "PetDBProvider.h"
#include "PuzzleDBProvider.h"
#include "QuestDBProvider.h"
#include "StringTable.h"
#include "TitleDBProvider.h"

namespace ParaEngine
{
	class CCharacterDBProvider;
	class CChestDBProvider;
	class CFruitDBProvider;
	class CItemDBProvider;
	class CPetAIDBProvider;
	class CPetDBProvider;
	class CPuzzleDBProvider;
	class CQuestDBProvider;
	class CStringTableDB;
	class CTitleDBProvider;


	class CKidsDBProvider
	{
	public:
		CKidsDBProvider(void);
		~CKidsDBProvider(void);

		// -------------------------------
		//		common function 
		// -------------------------------

		/** whether db is opened. */
		bool IsValid() {return m_pDataBase.get() !=0;};

		/** delete the database and set everything to NULL*/
		void Cleanup();
		/** get the database object associated with this provider*/
		DBEntity* GetDBEntity() {return m_pDataBase.get();};
		/**
		* replace the current database with current one.  the old one is closed and the new once will be opened. 
		* @param sConnectionstring: the file path
		*/
		void SetDBEntity(const string& sConnectionstring);

		/** ensure that the database has been set up properly. If not, ResetDatabase() is called to reset the database to blank */
		void ValidateDatabase() {assert(m_pDataBase.get()!=0);};

		// -------------------------------
		//		string table functions
		// -------------------------------

		/** Get string from ID
		* @param ID: ID in kids db's string table.
		* @return string in the current game language */
		static string GetStringbyID(int ID);

		/** Insert the new string table entry to StringTable_DB
		* @param str: Entry in the current game language
		* @return ID of the inserted string
		*/
		static int InsertString(const char* strEN, const char * strCN);

		// -------------------------------
		//		Puzzle database functions
		// -------------------------------

		/** Insert the new puzzle record to Puzzle_DB
		* @param record: ID of record will be ignored and filled with actual ID if inserted successfully
		* @return true if the record is inserted in database
		*/
		bool InsertPuzzleRecordFromString(const string& strRecord);

		/** delete the existing puzzle record from Puzzle_DB
		* @param ID: ID of target record
		* @return true if the record is deleted in database
		*/
		bool DeletePuzzleRecordByID(int ID);

		// -------------------------------
		//		Item database functions
		// -------------------------------

		/** Insert the new item record to Item_DB
		* @param record: ID of record will be ignored and filled with actual ID if inserted successfully
		* @return true if the record is inserted in database
		*/
		bool InsertItemRecordFromString(const string& strRecord);

		/** delete the existing item record from Item_DB
		* @param ID: ID of target record
		* @return true if the record is deleted in database
		*/
		bool DeleteItemRecordByID(int ID);

		/** Update existing item record of Item_DB
		* @param strRecord: update record with actual ID
		* @return true if the record is updated in database
		*/
		bool UpdateItemRecordFromString(const string& strRecord);

		/** Select existing item record of Item_DB
		* @param record: select record by ID
		* @return true if the record is selected in database
		*/
		const char * SelectItemRecordToString(__int64 ID);

		// -------------------------------
		//		Character database functions
		// -------------------------------

		/** Insert the new character record to Character_DB
		* @param record: ID of record will be ignored and filled with actual ID if inserted successfully
		* @return true if the record is inserted in database
		*/
		bool InsertCharacterRecordFromString(const string& strRecord);

		/** delete the existing puzzle record from Character_DB
		* @param ID: ID of target record
		* @return true if the record is deleted in database
		*/
		bool DeleteCharacterRecordByID(int ID);

		// -------------------------------
		//		Chest database functions
		// -------------------------------

		/** Insert the new chest record to Chest_DB
		* @param record: ID of record will be ignored and filled with actual ID if inserted successfully
		* @return true if the record is inserted in database
		*/
		bool InsertChestRecordFromString(const string& strRecord);

		/** delete the existing chest record from Chest_DB
		* @param ID: ID of target record
		* @return true if the record is deleted in database
		*/
		bool DeleteChestRecordByID(int ID);

		// -------------------------------
		//		Fruit database functions
		// -------------------------------

		/** Insert the new fruit record to Fruit_DB
		* @param record: ID of record will be ignored and filled with actual ID if inserted successfully
		* @return true if the record is inserted in database
		*/
		bool InsertFruitRecordFromString(const string& strRecord);

		/** delete the existing fruit record from Fruit_DB
		* @param ID: ID of target record
		* @return true if the record is deleted in database
		*/
		bool DeleteFruitRecordByID(int ID);

		// -------------------------------
		//		PetAI database functions
		// -------------------------------

		/** Insert the new petAI record to PetAI_DB
		* @param record: ID of record will be ignored and filled with actual ID if inserted successfully
		* @return true if the record is inserted in database
		*/
		bool InsertPetAIRecordFromString(const string& strRecord);

		/** delete the existing petAI record from PetAI_DB
		* @param ID: ID of target record
		* @return true if the record is deleted in database
		*/
		bool DeletePetAIRecordByID(int ID);

		// -------------------------------
		//		Pet database functions
		// -------------------------------

		/** Insert the new pet record to Pet_DB
		* @param record: ID of record will be ignored and filled with actual ID if inserted successfully
		* @return true if the record is inserted in database
		*/
		bool InsertPetRecordFromString(const string& strRecord);

		/** delete the existing pet record from Pet_DB
		* @param ID: ID of target record
		* @return true if the record is deleted in database
		*/
		bool DeletePetRecordByID(int ID);

		// -------------------------------
		//		Quest database functions
		// -------------------------------

		/** Insert the new quest record to Quest_DB
		* @param record: ID of record will be ignored and filled with actual ID if inserted successfully
		* @return true if the record is inserted in database
		*/
		bool InsertQuestRecordFromString(const string& strRecord);

		/** delete the existing quest record from Quest_DB
		* @param ID: ID of target record
		* @return true if the record is deleted in database
		*/
		bool DeleteQuestRecordByID(int ID);

		// -------------------------------
		//		Title database functions
		// -------------------------------

		/** Insert the new title record to Title_DB
		* @param record: ID of record will be ignored and filled with actual ID if inserted successfully
		* @return true if the record is inserted in database
		*/
		bool InsertTitleRecordFromString(const string& strRecord);

		/** delete the existing title record from Title_DB
		* @param ID: ID of target record
		* @return true if the record is deleted in database
		*/
		bool DeleteTitleRecordByID(int ID);


#ifdef _DEBUG
		/** test data base*/
		void TestDB();
#endif

	private:
		// FORMAT: parse record fields and link with ##
		void ParsePuzzleRecordToString(string& str, const stPuzzleDbRecord& record);
		void ParseStringToPuzzleRecord(const string& str, stPuzzleDbRecord& record);

		// Append string table record to str
		void AppendStringTableRecordToString(string& str, const stStringTableDbRecord& record);
		// Get string table record to record
		int GetStringTableRecordFromString(char* str, stStringTableDbRecord& record);

		// FORMAT: parse record fields and link with ##
		void ParseItemRecordToString(const stItemDbRecord& record);
		void ParseStringToItemRecord(const string& str, stItemDbRecord& record);

		const char * NumToString(__int64 num);
		const char * NumToString(int num);
		const char * NumToString(float num);
		const char * NumToString(bool num);

		char m_strSelectBuf[MAX_STRING_LENGTH];


		// FORMAT: parse record fields and link with ##
		void ParseCharacterRecordToString(string& str, const stCharacterDbRecord& record);
		void ParseStringToCharacterRecord(const string& str, stCharacterDbRecord& record);

		// FORMAT: parse record fields and link with ##
		void ParseChestRecordToString(string& str, const stChestDbRecord& record);
		void ParseStringToChestRecord(const string& str, stChestDbRecord& record);
		
		// FORMAT: parse record fields and link with ##
		void ParseFruitRecordToString(string& str, const stFruitDbRecord& record);
		void ParseStringToFruitRecord(const string& str, stFruitDbRecord& record);

		// FORMAT: parse record fields and link with ##
		void ParsePetAIRecordToString(string& str, const stPetAIDbRecord& record);
		void ParseStringToPetAIRecord(const string& str, stPetAIDbRecord& record);

		// FORMAT: parse record fields and link with ##
		void ParsePetRecordToString(string& str, const stPetDbRecord& record);
		void ParseStringToPetRecord(const string& str, stPetDbRecord& record);

		// FORMAT: parse record fields and link with ##
		void ParseQuestRecordToString(string& str, const stQuestDbRecord& record);
		void ParseStringToQuestRecord(const string& str, stQuestDbRecord& record);

		// FORMAT: parse record fields and link with ##
		void ParseTitleRecordToString(string& str, const stTitleDbRecord& record);
		void ParseStringToTitleRecord(const string& str, stTitleDbRecord& record);

		/** base database interface */
		ParaEngine::asset_ptr<DBEntity> m_pDataBase;

		// database providers
		CCharacterDBProvider*	m_dbCharacter;
		CChestDBProvider*		m_dbChest;
		CFruitDBProvider*			m_dbFruit;
		CItemDBProvider*			m_dbItem;
		CPetAIDBProvider*		m_dbPetAI;
		CPetDBProvider*			m_dbPet;
		CPuzzleDBProvider*		m_dbPuzzle;
		CQuestDBProvider*		m_dbQuest;
		CStringTableDB*			m_dbStringTable;
		CTitleDBProvider*			m_dbTitle;
	};
}
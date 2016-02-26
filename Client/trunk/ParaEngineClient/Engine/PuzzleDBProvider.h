#pragma once
#include "ic/ParaDatabase.h"
#include ".\StringTable.h"


namespace ParaEngine
{
	struct stPuzzleDbRecord
	{
		// TODO: change the db_puzzle.sql to new version

		/** Common Attribute */
		__int64 ID;
		stStringTableDbRecord PuzzleName;	// record in string table

		/** Difficulty Attribute */
		int DifficultyRate;

		/** Puzzle Attribute */
		stStringTableDbRecord PuzzleDesc;	// record in string table
		int NumberOfChoices;	// Limit to 4 (ABCD)
		stStringTableDbRecord PuzzleAnswerA;	// record in string table
		stStringTableDbRecord PuzzleAnswerB;	// record in string table
		stStringTableDbRecord PuzzleAnswerC;	// record in string table
		stStringTableDbRecord PuzzleAnswerD;	// record in string table
		string PuzzleDescPicFilePath;

		/** Answer Attribute */
		int CorrectChoice;
		stStringTableDbRecord Explanation;	// record in string table
		string ExplanationPicFilePath;

		/** Reserved Attribute */
		string Reserved1;
		string Reserved2;
		string Reserved3;
		string Reserved4;
		string Reserved5;
	};

	// Class Description: CPuzzleDBProvider contains all functions to access the 
	//		puzzle database and the user interface data of puzzle control. 
	//		This is a singleton class. Call GetInstance() to use this class.
	class CPuzzleDBProvider
	{
	public:
		CPuzzleDBProvider(void);
		CPuzzleDBProvider(ParaInfoCenter::DBEntity* pDb);
		~CPuzzleDBProvider(void);
	protected:
	private:

		/** base database interface */
		ParaEngine::asset_ptr<ParaInfoCenter::DBEntity> m_pDataBase;

	public:		
		/** get the singleton class.*/
		static CPuzzleDBProvider & GetInstance();
		/** get the database interface. */
		ParaInfoCenter::DBEntity* GetDBEntity();
		/** set the database interface. */
		void SetDBEntity(ParaInfoCenter::DBEntity* pDb);

#ifdef TEST_PUZZLE
	public:
		// TODO: only for test
		void TestDB();
#endif

	public:
		/** Insert the new puzzle record to Puzzle_DB
		* @param record: ID of record will be ignored and filled with actual ID if inserted successfully
		* @return true if the record is inserted in database
		*/
		bool InsertPuzzleRecord(stPuzzleDbRecord& record);

		/** delete the existing puzzle record from Puzzle_DB
		* @param ID: ID of target record
		* @return true if the record is deleted in database
		*/
		bool DeletePuzzleRecordByID(int ID);
	};
}

/** @author Andy Wang  */
DROP TABLE IF EXISTS Puzzle_DB ;
CREATE TABLE Puzzle_DB (
/** Common Attribute */
	ID INTEGER PRIMARY KEY, 
	PuzzleNameStringIndex INTEGER, /** Index into string table */
	
/** Difficulty Attribute */
	DifficultyRate INTEGER, 
	
/** Puzzle Attribute */
	PuzzleDescStringIndex INTEGER, /** Index into string table */
	NumberOfChoices INTEGER, /** Limit to 4 (ABCD) */
	PuzzleAnswerAStringIndex INTEGER, /** Index into string table */
	PuzzleAnswerBStringIndex INTEGER, /** Index into string table */
	PuzzleAnswerCStringIndex INTEGER, /** Index into string table */
	PuzzleAnswerDStringIndex INTEGER, /** Index into string table */
	
/** Answer Attribute */
	PuzzleDescPicFilePath VARCHAR, 
	CorrectChoice INTEGER, 
	ExplanationStringIndex INTEGER, /** Index into string table */
	ExplanationPicFilePath INTEGER, 
	
/** Reserved Attribute */
	Reserved1 VARCHAR, 
	Reserved2 VARCHAR, 
	Reserved3 VARCHAR, 
	Reserved4 VARCHAR, 
	Reserved5 VARCHAR
);

CREATE TABLE Puzzle_DB (
	ID INTEGER PRIMARY KEY, 
	PuzzleNameStringIndex INTEGER,
	DifficultyRate INTEGER, 
	PuzzleDescStringIndex INTEGER,
	NumberOfChoices INTEGER,
	PuzzleAnswerAStringIndex INTEGER,
	PuzzleAnswerBStringIndex INTEGER,
	PuzzleAnswerCStringIndex INTEGER,
	PuzzleAnswerDStringIndex INTEGER,
	PuzzleDescPicFilePath VARCHAR, 
	CorrectChoice INTEGER, 
	ExplanationStringIndex INTEGER,
	ExplanationPicFilePath INTEGER,
	Reserved1 VARCHAR, 
	Reserved2 VARCHAR, 
	Reserved3 VARCHAR, 
	Reserved4 VARCHAR, 
	Reserved5 VARCHAR
);
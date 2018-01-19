/** @author Andy Wang  */
DROP TABLE IF EXISTS Chest_DB ;
CREATE TABLE Chest_DB (
/** Common Attribute */
	ID INTEGER PRIMARY KEY,
	/** ATTENTION: different chest may have the same name */
	ChestNameStringIndex INTEGER,	/** Index into string table */
	
/** Engine Asset Management Attribute */
	AssetName VARCHAR,
	IsSnapToTerrain INTEGER,
	
/** Appearance Related Attribute */
	ChestModeFilePath VARCHAR,
	ChestModelScale FLOAT,
	ChestRadius FLOAT,
	ChestFacing FLOAT,
	posX FLOAT,
	posY FLOAT,
	posZ FLOAT,
	
/** Difficulty Attribute */
	DifficultyRate INTEGER,	/** difficulty rate where puzzles are randomly picked */
	NumberOfAllPuzzles INTEGER,
	NumberOfPassPuzzles INTEGER,
	
/** Reward Attribute */
	Reward VARCHAR,
	
/** Reserved Attribute */
	Reserved1 VARCHAR,
	Reserved2 VARCHAR,
	Reserved3 VARCHAR,
	Reserved4 VARCHAR,
	Reserved5 VARCHAR
);

CREATE TABLE Chest_DB (
	ID INTEGER PRIMARY KEY,
	ChestNameStringIndex INTEGER,
	AssetName VARCHAR,
	IsSnapToTerrain INTEGER,
	ChestModeFilePath VARCHAR,
	ChestModelScale FLOAT,
	ChestRadius FLOAT,
	ChestFacing FLOAT,
	posX FLOAT,
	posY FLOAT,
	posZ FLOAT,
	DifficultyRate INTEGER,
	NumberOfAllPuzzles INTEGER,
	NumberOfPassPuzzles INTEGER,
	Reward VARCHAR,
	Reserved1 VARCHAR,
	Reserved2 VARCHAR,
	Reserved3 VARCHAR,
	Reserved4 VARCHAR,
	Reserved5 VARCHAR
);

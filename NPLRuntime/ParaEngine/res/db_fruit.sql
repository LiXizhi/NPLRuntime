/** @author Andy Wang  */
DROP TABLE IF EXISTS Fruit_DB ;
CREATE TABLE Fruit_DB (
/** Common Attribute */
	ID INTEGER PRIMARY KEY,
	/** ATTENTION: different chests may have the same name */
	FruitNameStringIndex INTEGER,	/** Index into string table */
	
/** Engine Asset Management Attribute */
	AssetName VARCHAR,
	IsSnapToTerrain INTEGER,
	
/** Appearance Related Attribute */
	FruitModeFilePath VARCHAR,
	FruitModelScale FLOAT,
	FruitRadius FLOAT,
	FruitFacing FLOAT,
	posX FLOAT,
	posY FLOAT,
	posZ FLOAT,
	
/** Reward Attribute */
	Reward VARCHAR,
	
/** Reserved Attribute */
	Reserved1 VARCHAR,
	Reserved2 VARCHAR,
	Reserved3 VARCHAR,
	Reserved4 VARCHAR,
	Reserved5 VARCHAR
);

CREATE TABLE Fruit_DB (
	ID INTEGER PRIMARY KEY,
	FruitNameStringIndex INTEGER,
	AssetName VARCHAR,
	IsSnapToTerrain INTEGER,
	FruitModeFilePath VARCHAR,
	FruitModelScale FLOAT,
	FruitRadius FLOAT,
	FruitFacing FLOAT,
	posX FLOAT,
	posY FLOAT,
	posZ FLOAT,
	Reward VARCHAR,
	Reserved1 VARCHAR,
	Reserved2 VARCHAR,
	Reserved3 VARCHAR,
	Reserved4 VARCHAR,
	Reserved5 VARCHAR
);

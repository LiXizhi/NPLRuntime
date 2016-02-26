/** @author Andy Wang  */
DROP TABLE IF EXISTS Pet_DB ;
CREATE TABLE Pet_DB (
/** Common Attribute */
	ID INTEGER PRIMARY KEY,
	PetNameStringIndex INTEGER,	/** Index into string table */
	
/** Engine Asset Management Attribute */
	AssetName VARCHAR,
	IsGlobal INTEGER,
	IsSnapToTerrain INTEGER,
	
/** Appearance Related Attribute */
	PetModeFilePath VARCHAR,
	PetModelScale FLOAT,
	PetRadius FLOAT,
	PetFacing FLOAT,
	posX FLOAT,
	posY FLOAT,
	posZ FLOAT,
	
/** AI Related Attribute */
	/** NOTE: record in database format "ID,ID,ID,......" */
	PetAIEntryIndexArray VARCHAR,	/** index list into Pet AI entries*/
	PerceptiveRadius FLOAT,
	SentientRadius FLOAT,
	OnLoadScript VARCHAR,
	
/** Kids Game Related Attribute */
	IsKeptinCage INTEGER,
	
/** Reserved Attribute */
	Reserved1 VARCHAR,
	Reserved2 VARCHAR,
	Reserved3 VARCHAR,
	Reserved4 VARCHAR,
	Reserved5 VARCHAR
);


CREATE TABLE Pet_DB (
	ID INTEGER PRIMARY KEY,
	PetNameStringIndex INTEGER,
	AssetName VARCHAR,
	IsGlobal INTEGER,
	IsSnapToTerrain INTEGER,
	PetModeFilePath VARCHAR,
	PetModelScale FLOAT,
	PetRadius FLOAT,
	PetFacing FLOAT,
	posX FLOAT,
	posY FLOAT,
	posZ FLOAT,
	PetAIEntryIndexArray VARCHAR,
	PerceptiveRadius FLOAT,
	SentientRadius FLOAT,
	OnLoadScript VARCHAR,
	IsKeptinCage INTEGER,
	Reserved1 VARCHAR,
	Reserved2 VARCHAR,
	Reserved3 VARCHAR,
	Reserved4 VARCHAR,
	Reserved5 VARCHAR
);

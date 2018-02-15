/** @author Andy Wang  */
DROP TABLE IF EXISTS Character_DB ;
CREATE TABLE Character_DB (
/** Common Attribute */
	ID INTEGER PRIMARY KEY,
	CharacterNameStringIndex INTEGER,	/** Index into string table */
	NumberOfPets INTEGER,	/** limit the pet number to 3*/
	MainPetIndex INTEGER,	/** Index into pet table */
	SecondaryPetIndex INTEGER,	/** Index into pet table */
	ThirdPetIndex INTEGER,	/** Index into pet table */
	
/** Engine Asset Management Attribute */
	AssetName VARCHAR,
	IsGlobal INTEGER,
	IsSnapToTerrain INTEGER,
	
/** Appearance Related Attribute */
	CharacterModeFilePath VARCHAR,
	CharacterModelScale FLOAT,
	CharacterRadius FLOAT,
	CharacterFacing FLOAT,
	posX FLOAT,
	posY FLOAT,
	posZ FLOAT,
	
/** AI Related Attribute */
	/** CharacterAIEntryIndex INTEGER, 	/** Reserved for character AI*/
	PerceptiveRadius FLOAT,
	SentientRadius FLOAT,
	OnLoadScript VARCHAR,
	
/** Kids Game Related Attribute */
	/** ATTENTION: different characters may have the same title */
	CharacterTitleIndex INTEGER,	/** Index into title table */
	ExperiencePt INTEGER,
	/** TODO: Change according to game design */
	Orange INTEGER,
	Apple INTEGER,
	Peach INTEGER,
	
/** Reserved Attribute */
	Reserved1 VARCHAR,
	Reserved2 VARCHAR,
	Reserved3 VARCHAR,
	Reserved4 VARCHAR,
	Reserved5 VARCHAR
);

CREATE TABLE Character_DB (
	ID INTEGER PRIMARY KEY,
	CharacterNameStringIndex INTEGER,
	NumberOfPets INTEGER,	
	MainPetIndex INTEGER,
	SecondaryPetIndex INTEGER,
	ThirdPetIndex INTEGER,
	AssetName VARCHAR,
	IsGlobal INTEGER,
	IsSnapToTerrain INTEGER,
	CharacterModeFilePath VARCHAR,
	CharacterModelScale FLOAT,
	CharacterRadius FLOAT,
	CharacterFacing FLOAT,
	posX FLOAT,
	posY FLOAT,
	posZ FLOAT,
	PerceptiveRadius FLOAT,
	SentientRadius FLOAT,
	OnLoadScript VARCHAR,
	CharacterTitleIndex INTEGER,
	ExperiencePt INTEGER,
	Orange INTEGER,
	Apple INTEGER,
	Peach INTEGER,
	Reserved1 VARCHAR,
	Reserved2 VARCHAR,
	Reserved3 VARCHAR,
	Reserved4 VARCHAR,
	Reserved5 VARCHAR
);
/** @author Andy Wang  */
DROP TABLE IF EXISTS Quest_DB ;
CREATE TABLE Quest_DB (
/** Common Attribute */
	ID INTEGER PRIMARY KEY,
	QuestNameStringIndex INTEGER,	/** Index into string table */
	
/** Engine Asset Management Attribute */
	TopSnapShotAssetName VARCHAR,
	IntroVideoAssetName VARCHAR,
	IsGlobal INTEGER,
	
/** UI Related Attribute */
	TopSnapShotFilePath VARCHAR,
	IntroVideoFilePath VARCHAR,
	
/** Quest Content Attribute */
	IsComplete INTEGER,
	QuestDescStringIndex INTEGER,	/** Index into string table */
	AcquireQuestRequirement VARCHAR,
	CompleteQuestRequirement VARCHAR,
	Reward VARCHAR,
	
/** Reserved Attribute */
	Reserved1 VARCHAR, 
	Reserved2 VARCHAR, 
	Reserved3 VARCHAR, 
	Reserved4 VARCHAR, 
	Reserved5 VARCHAR
);


CREATE TABLE Quest_DB (
	ID INTEGER PRIMARY KEY,
	QuestNameStringIndex INTEGER,
	TopSnapShotAssetName VARCHAR,
	IntroVideoAssetName VARCHAR,
	IsGlobal INTEGER,
	TopSnapShotFilePath VARCHAR,
	IntroVideoFilePath VARCHAR,
	IsComplete INTEGER,
	QuestDescStringIndex INTEGER,
	AcquireQuestRequirement VARCHAR,
	CompleteQuestRequirement VARCHAR,
	Reward VARCHAR,
	Reserved1 VARCHAR, 
	Reserved2 VARCHAR, 
	Reserved3 VARCHAR, 
	Reserved4 VARCHAR, 
	Reserved5 VARCHAR
);

/** @author Andy Wang  */
DROP TABLE IF EXISTS Item_DB ;
CREATE TABLE Item_DB (
/** Common Attribute */
	ID INTEGER PRIMARY KEY, 
	ItemNameStringIndex INTEGER, /** Index into string table */
	
/** Engine Asset Management Attribute */
	IconAssetName VARCHAR, 
	/** Desc: text shown when mouse over the icon */
	DescTextAssetName VARCHAR, /** Index into string table */
	IsGlobal INTEGER, 
	
/** UI Related Attribute */
	IconFilePath VARCHAR, 
	DescStringIndex INTEGER, 
	ModelFilePath VARCHAR, 
	
/** Kids Game Related Attribute */
	ItemType INTEGER, 
	/** NOTE: IsObtained is a tag specify if player can build */
	IsObtained INTEGER, 
	IsUnique INTEGER, 
	/** TODO: if special item need cost exp and fruit */
	CostExperiencePt INTEGER, 
	/** TODO: Change according to game design */
	CostOrange INTEGER, 
	CostApple INTEGER, 
	CostPeach INTEGER, 
	
/** Reserved Attribute */
	Reserved1 VARCHAR, 
	Reserved2 VARCHAR, 
	Reserved3 VARCHAR, 
	Reserved4 VARCHAR, 
	Reserved5 VARCHAR
);


CREATE TABLE Item_DB (
	ID INTEGER PRIMARY KEY, 
	ItemNameStringIndex INTEGER,
	IconAssetName VARCHAR,
	DescTextAssetName VARCHAR,
	IsGlobal INTEGER,
	IconFilePath VARCHAR, 
	DescStringIndex INTEGER, 
	ModelFilePath VARCHAR,
	ItemType INTEGER,
	IsObtained INTEGER, 
	IsUnique INTEGER,
	CostExperiencePt INTEGER,
	CostOrange INTEGER, 
	CostApple INTEGER, 
	CostPeach INTEGER,
	Reserved1 VARCHAR, 
	Reserved2 VARCHAR, 
	Reserved3 VARCHAR, 
	Reserved4 VARCHAR, 
	Reserved5 VARCHAR
);
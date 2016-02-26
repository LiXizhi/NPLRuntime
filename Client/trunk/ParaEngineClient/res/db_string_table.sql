/** @author Andy Wang  */
DROP TABLE IF EXISTS StringTable_DB ;
CREATE TABLE StringTable_DB (
/** Common Attribute */
	ID INTEGER PRIMARY KEY UNIQUE, 
	StringEnglish VARCHAR, 
	StringSimplifiedChinese VARCHAR, 

/** Reserved Attribute */
	Reserved1 VARCHAR, 
	Reserved2 VARCHAR, 
	Reserved3 VARCHAR, 
	Reserved4 VARCHAR, 
	Reserved5 VARCHAR
);


CREATE TABLE StringTable_DB (
	ID INTEGER PRIMARY KEY UNIQUE, 
	StringEnglish VARCHAR, 
	StringSimplifiedChinese VARCHAR, 
	Reserved1 VARCHAR, 
	Reserved2 VARCHAR, 
	Reserved3 VARCHAR, 
	Reserved4 VARCHAR, 
	Reserved5 VARCHAR
);
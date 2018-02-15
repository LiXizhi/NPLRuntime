/** @author Andy Wang  */
DROP TABLE IF EXISTS Title_DB ;
CREATE TABLE Title_DB (
/** Common Attribute */
	ID INTEGER PRIMARY KEY,
	TitleLevel INTEGER,
	IsBoy INTEGER,
	TitleStringIndex INTEGER,	/** Index into string table */
	
/** Reserved Attribute */
	Reserved1 VARCHAR,
	Reserved2 VARCHAR,
	Reserved3 VARCHAR,
	Reserved4 VARCHAR,
	Reserved5 VARCHAR
);


CREATE TABLE Title_DB (
	ID INTEGER PRIMARY KEY,
	TitleLevel INTEGER,
	IsBoy INTEGER,
	TitleStringIndex INTEGER,
	Reserved1 VARCHAR,
	Reserved2 VARCHAR,
	Reserved3 VARCHAR,
	Reserved4 VARCHAR,
	Reserved5 VARCHAR
);

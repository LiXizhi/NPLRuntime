#pragma once

namespace ParaEngine
{
	// forward declare
	class CCharacterDB;
	class CKidsDBProvider;
	class CNpcDatabase;
	class AttributeProvider;

	enum LocalGameLanguage
	{
		ENGLISH = 0,
		SIMPLIFIEDCHINESE = 1,
		RESERVED1 = 2	// Reserved for other language
	};

	/** a singleton class for managing all data providers used in the game engine. 
	* such as, character database, NPC database, world, string, music, etc. */
	class CDataProviderManager
	{
	public:
		static CDataProviderManager& GetSingleton();

	public:
		/** cleanup everything. */
		void Cleanup();

		/** get the global character model database */
		CCharacterDB* GetCharacterDB();

		/** get the current kids game database */
		CKidsDBProvider* GetKidsDBProvider();

		/** this function shall never be called from the scripting interface. 
		this is solely for exporting API. and should not be used from the scripting interface.*/
		//static void GetKidsDBObject_(CKidsDBProvider* pOut);

		/** get the current NPC data provider. */
		CNpcDatabase* GetNpcDB();

		/** get the current attribute data provider. */
		AttributeProvider* GetAttributeProvider();
		
		/**
		* set the global NPC data base to a new database file. 
		* @param sConnectionstring : currently it is the file path of the database file. 
		*/
		void SetNpcDB(const string& sConnectionstring);

		/**
		* set the current attribute data base to a new database file. 
		* @param sConnectionstring : currently it is the file path of the database file. 
		*/
		void SetAttributeProvider(const string& sConnectionstring);

		/**
		* set the current kids game data base to a new database file. 
		* @param sConnectionstring : currently it is the file path of the database file. 
		*/
		void SetKidsDBProvider(const string& sConnectionstring);

		/**
		* set world database.
		* it sets attribute provider, NPC database, etc to the same database file.  
		* @param sConnectionstring : currently it is the file path of the database file. 
		*/
		void SetWorldDB(const string& sConnectionstring){
			SetNpcDB(sConnectionstring);
			SetAttributeProvider(sConnectionstring);
			SetKidsDBProvider(sConnectionstring);
		}

		/** Get local game language */
		LocalGameLanguage GetLocalGameLanguage(void);
		/** Set local game language */
		void SetLocalGameLanguage(const LocalGameLanguage lang);

	private:
		CDataProviderManager(void);
		~CDataProviderManager(void);

		LocalGameLanguage m_language;

		CNpcDatabase* m_dbNPC;
		AttributeProvider* m_dbAttribute;
		CKidsDBProvider* m_dbKids;
	};
}

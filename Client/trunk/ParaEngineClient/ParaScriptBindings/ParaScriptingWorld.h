//-----------------------------------------------------------------------------
// Copyright (C) 2004 - 2006 ParaEngine Dev Studio, All Rights Reserved.
// Date:	2006.1
// Description:	API for world
//-----------------------------------------------------------------------------
#pragma once
#include "ParaScriptingScene.h"

namespace ParaEngine
{
	class CNpcDatabase;
	class CCharacterDB;
	class CKidsDBProvider;
	class AttributeProvider;
}
namespace luabind
{
	namespace adl{
		class object;
	}
	using adl::object;	
}

namespace ParaScripting
{
	using namespace std;
	using namespace luabind;
	using namespace ParaEngine;
#ifdef USE_DIRECTX_RENDERER
	/**
	* @ingroup ParaWorld
	* 
	* Wrapper of internal ParaWorld data provider
	*/
	class PE_CORE_DECL ParaKidsDataProvider
	{
	public:
		CKidsDBProvider*	m_pObj;		// a pointer to the object

		ParaKidsDataProvider();
		ParaKidsDataProvider(CKidsDBProvider* pObj) {	m_pObj = pObj;	};
		~ParaKidsDataProvider();

		// -------------------------------
		//		Puzzle database functions
		// -------------------------------
		/** Insert the new puzzle record to Puzzle_DB
		* @param record: ID of record will be ignored and filled with actual ID if inserted successfully
		* @return true if the record is inserted in database
		*/
		bool InsertPuzzleRecordFromString(const char* strRecord);

		/** delete the existing puzzle record from Puzzle_DB
		* @param ID: ID of target record
		* @return true if the record is deleted in database
		*/
		bool DeletePuzzleRecordByID(int ID);

		// -------------------------------
		//		Item database functions
		// -------------------------------
		/** Insert the new item record to Item_DB
		* @param record: ID of record will be ignored and filled with actual ID if inserted successfully
		* @return true if the record is inserted in database
		*/
		bool InsertItemRecordFromString(const char* strRecord);

		/** delete the existing item record from Item_DB
		* @param ID: ID of target record
		* @return true if the record is deleted in database
		*/
		bool DeleteItemRecordByID(int ID);

		/** Update existing item record of Item_DB
		* @param strRecord: update record with actual ID
		* @return true if the record is updated in database
		*/
		bool UpdateItemRecordFromString(const char* strRecord);

		/** Select existing item record of Item_DB
		* @param record: select record by ID
		* @return true if the record is selected in database
		*/
		const char * SelectItemRecordToString(__int64 ID) const;

		// -------------------------------
		//		Character database functions
		// -------------------------------
		/** Insert the new character record to Character_DB
		* @param record: ID of record will be ignored and filled with actual ID if inserted successfully
		* @return true if the record is inserted in database
		*/
		bool InsertCharacterRecordFromString(const char* strRecord);

		/** delete the existing puzzle record from Character_DB
		* @param ID: ID of target record
		* @return true if the record is deleted in database
		*/
		bool DeleteCharacterRecordByID(int ID);

		// -------------------------------
		//		Chest database functions
		// -------------------------------
		/** Insert the new chest record to Chest_DB
		* @param record: ID of record will be ignored and filled with actual ID if inserted successfully
		* @return true if the record is inserted in database
		*/
		bool InsertChestRecordFromString(const char* strRecord);

		/** delete the existing chest record from Chest_DB
		* @param ID: ID of target record
		* @return true if the record is deleted in database
		*/
		bool DeleteChestRecordByID(int ID);

		// -------------------------------
		//		Fruit database functions
		// -------------------------------
		/** Insert the new fruit record to Fruit_DB
		* @param record: ID of record will be ignored and filled with actual ID if inserted successfully
		* @return true if the record is inserted in database
		*/
		bool InsertFruitRecordFromString(const char* strRecord);

		/** delete the existing fruit record from Fruit_DB
		* @param ID: ID of target record
		* @return true if the record is deleted in database
		*/
		bool DeleteFruitRecordByID(int ID);

		// -------------------------------
		//		PetAI database functions
		// -------------------------------
		/** Insert the new petAI record to PetAI_DB
		* @param record: ID of record will be ignored and filled with actual ID if inserted successfully
		* @return true if the record is inserted in database
		*/
		bool InsertPetAIRecordFromString(const char* strRecord);

		/** delete the existing petAI record from PetAI_DB
		* @param ID: ID of target record
		* @return true if the record is deleted in database
		*/
		bool DeletePetAIRecordByID(int ID);

		// -------------------------------
		//		Pet database functions
		// -------------------------------
		/** Insert the new pet record to Pet_DB
		* @param record: ID of record will be ignored and filled with actual ID if inserted successfully
		* @return true if the record is inserted in database
		*/
		bool InsertPetRecordFromString(const char* strRecord);

		/** delete the existing pet record from Pet_DB
		* @param ID: ID of target record
		* @return true if the record is deleted in database
		*/
		bool DeletePetRecordByID(int ID);

		// -------------------------------
		//		Quest database functions
		// -------------------------------
		/** Insert the new quest record to Quest_DB
		* @param record: ID of record will be ignored and filled with actual ID if inserted successfully
		* @return true if the record is inserted in database
		*/
		bool InsertQuestRecordFromString(const char* strRecord);

		/** delete the existing quest record from Quest_DB
		* @param ID: ID of target record
		* @return true if the record is deleted in database
		*/
		bool DeleteQuestRecordByID(int ID);

		// -------------------------------
		//		Title database functions
		// -------------------------------
		/** Insert the new title record to Title_DB
		* @param record: ID of record will be ignored and filled with actual ID if inserted successfully
		* @return true if the record is inserted in database
		*/
		bool InsertTitleRecordFromString(const char* strRecord);

		/** delete the existing title record from Title_DB
		* @param ID: ID of target record
		* @return true if the record is deleted in database
		*/
		bool DeleteTitleRecordByID(int ID);
	};
#endif

	/**
	* @ingroup ParaWorld
	* 
	* Wrapper of internal ParaWorld data provider
	*/
	class PE_CORE_DECL ParaDataProvider
	{
	public:
		// a pointer to the db provider 
		union{
			void* m_pDataProvider;
			AttributeProvider* m_pAttributeProvider;
#ifdef USE_DIRECTX_RENDERER
			CCharacterDB* m_pDbCharacter;
			CKidsDBProvider* m_pDbKidsProvider;
			CNpcDatabase* m_pDbNPC;
#endif
		};
		
		/**
		* check if the object is valid
		*/
		bool IsValid(){return m_pDataProvider!=NULL;}
	public:
		ParaDataProvider():m_pDataProvider(NULL){};
		ParaDataProvider(void* pObj):m_pDataProvider(pObj){};
		~ParaDataProvider(){};
	public:

#ifdef USE_DIRECTX_RENDERER
		static void GetKidsDBProvider(ParaKidsDataProvider* pOut);
#endif
		//////////////////////////////////////////////////////////////////////////
		//
		// Kids game database
		//
		//////////////////////////////////////////////////////////////////////////

		/** Insert the new puzzle record to Puzzle_DB
		* @param record: ID of record will be ignored and filled with actual ID if inserted successfully
		* @return true if the record is inserted in database
		*/
		bool InsertPuzzleRecordFromString(const char* strRecord);

		/** delete the existing puzzle record from Puzzle_DB
		* @param ID: ID of target record
		* @return true if the record is deleted in database
		*/
		bool DeletePuzzleRecordByID(int ID);

		//////////////////////////////////////////////////////////////////////////
		//
		// NPC database
		//
		//////////////////////////////////////////////////////////////////////////

		/**
		* get NPC ID by name
		* @param name name of character
		* @return id is returned if found in database; otherwise non-positive value(-1) is returned.
		*/
		int GetNPCIDByName(const char* name);

		/**
		* the total number of NPC in the database. There is no statement cached for this call.
		* @return 
		*/
		int GetNPCCount();


		//////////////////////////////////////////////////////////////////////////
		//
		// attribute provider
		//
		//////////////////////////////////////////////////////////////////////////
		/**
		* whether a given attribute exists. This function is usually used internally. 
		* One common use of this function is to test if a field exists, so that we know if a object has been saved before or not.
		* @param sName : name of the attribute field
		* @return 
		*/
		bool DoesAttributeExists(const char*  sName);

		/**
		* get the attribute of the current attribute instance in the current table
		e.g. suppose db is a ParaDataProvider object.
			local x = db:GetAttribute("PlayerX", 0);
			local name = db:GetAttribute("PlayerName", "no_name");
		* @param sName : name of the attribute field
		* @param sOut : [in|out] default value of the field. Currently it may be a string or a number
		* @return the object is returned. Currently it may be a string, boolean or a number. 
		*/
		object GetAttribute(const char*  sName, const object& sOut);

		/**
		* update the attribute of the current attribute instance in the current table
		* insert the attribute if it is not created before.
		* @param sName : name of the attribute field
		* @param sIn: value of the attribute field. Currently it may be a string, boolean or a number.
		* @return true if succeed. 
		*/
		bool UpdateAttribute(const char*  sName, const object& sIn);

		/**
		* insert by simple name value pair
		* @param sName : name of the attribute field
		* @param sIn: value of the attribute field. Currently it may be a string, boolean or a number.
		* @return true if succeed. 
		*/
		bool InsertAttribute(const char*  sName, const object& sIn);

		/**
		* delete the attribute of the current attribute instance in the current table
		* @param sName : name of the attribute field
		* @return true if succeed. 
		*/
		bool DeleteAttribute(const char*  sName);

		/** run a given sql command, commonly used commands are "BEGIN", "END", when we are batch saving attributes. */
		bool ExecSQL(const char*  sCmd);

		/**
		* set current attribute table name, the table will be created if not exists.
		* @param sName table name
		*/
		void SetTableName(const char*  sName);
		/**
		* get the current attribute table name
		* @return current attribute table name
		*/
		const char* GetTableName();
	};

	/** 
	* @ingroup ParaWorld
	*
	* A pool of currently selected objects. This is a singleton class.
	* Object may be selected into different groups. Although, there are no limit to group number, better keep it smaller than 16 groups. 
	* Selected objected may be displayed or highlighted differently. 
	* When objects are deleted from the scene. It will be deleted from the selection automatically. */
	class PE_CORE_DECL ParaSelection
	{
	public:
		/**
		* remove a given object in all selections. 
		* @param obj pointer to the object to delete
		*/
		static void RemoveObject(const ParaObject& obj);
		/**
		* remove an UI object from all selection groups. 
		* @param sName UI object name
		* @return if if removed
		*/
		static bool RemoveUIObject(const char* sName);

		/**
		* Add a new object to a given group. An object may belong to multiple groups.
		* @param obj pointer to the object to add
		* @param nGroupID which group the should be added to. by default it is added to group 0.
		* group ID must be smaller than 32. 
		*/
		static void AddObject(const ParaObject& obj, int nGroupID);
		/**
		* Please note that UI object in selection is automatically highlighted using the default highlighting effect. 
		* @param sName UI object name
		* @param nGroupID which group the should be added to. by default it is added to group 0.
		* @return true if the object is found and highlighted(selected). 
		*/
		static bool AddUIObject(const char* sName, int nGroupID);

		/**
		* get the nItemIndex object in the nGroupID group. 
		* @param nGroupID from which group the object is get
		* @param nItemIndex the index of the item to be retrieved.
		* @return selected item is returned
		*/
		static ParaObject GetObject(int nGroupID, int nItemIndex);
		/** this function shall never be called from the scripting interface. this is solely for exporting API. and should not be used from the scripting interface.*/
		static void GetObject_(ParaObject* pOut, int nGroupID, int nItemIndex);

		/**
		* get the total number item in the given group.  This function can be used with GetObject()
		* to iterate through all objects in any group. 
		* @param nGroupID group ID.
		* @return 
		*/
		static int GetItemNumInGroup(int nGroupID);

		/**
		* select the entire group. 
		* @param nGroupID 
		* @param bSelect true to select, false to de-select.
		*/
		static void SelectGroup(int nGroupID, bool bSelect);
		/**
		* Clear a given group so that there are no objects in it. 
		* @param nGroupID ID of the group. If ID is -1, all groups will be deleted.
		*/
		static void ClearGroup(int nGroupID);

		/**
		* set the maximum number of objects in the group.
		* @param nGroupID group ID
		* group ID must be smaller than 32. 
		* @param nMaxItemsNumber the number to set. default value is 1
		*/
		static void SetMaxItemNumberInGroup(int nGroupID, int nMaxItemsNumber);

		/**
		* set the maximum number of objects in the group.
		* @param nGroupID group ID,which ID must be smaller than 32. 
		* @return the maximum number in the given group
		*/
		static int GetMaxItemNumberInGroup(int nGroupID);
	};

	/** 
	* @ingroup ParaWorld
	world creation functions.*/
	class PE_CORE_DECL ParaWorld
	{
	public:
		ParaWorld(void);
		~ParaWorld(void);
	public:
		/** created a new empty world based on a given world. This is like class inheritance in C++
		* another world could derive from a given world, overriding certain terrain tile contents as it evolves. 
		* One can immediately create the new world which is identical to the base world. Every changes made to the new world
		* will be saved in the new world folder and will not affect the base world.
		* in reality, we just copy the base world's world config file to the new world using the new world's name.
		* e.g \n
			local sConfigFileName = ParaWorld.NewWorld("__TmpWorld", "sample/worldconfig.txt");\n
			if(sConfigFileName ~= "") then\n
				ParaScene.CreateWorld("", 32000, sConfigFileName); \n
			end \n
		* NOTE: if the sWorldName is the same as sBaseWorldName, the sBaseWorldName itself will be overridden
		* @param sWorldName: world name, a directory with the same name will be created containing the world config file.
		* @param sBaseWorldName: any valid world config file. 
		* @return: return the world config file. if failed, return ""
		*/
		static string NewWorld(const char* sWorldName, const char* sBaseWorldName);
		/** solely used for exporting*/
		static const char* NewWorld_(const char* sWorldName, const char* sBaseWorldName);

		/** delete a given world.*/
		static void DeleteWorld(const char* sWorldName);

		/** Create an empty world, with flat land.
		* @param sWorldName: world name, if NULL, it defaults to "_emptyworld"
		* @param fTileSize: terrain tile size in meters
		* @param nTileDimension: dimension of the tile matrix. default is 64, which is has 64*64 tiles
		*/
		static string NewEmptyWorld(const char* sWorldName=NULL, float fTileSize=533.3333f, int nTileDimension=64 );
		/** solely used for exporting*/
		static const char* NewEmptyWorld_(const char* sWorldName=NULL, float fTileSize=533.3333f, int nTileDimension=64 );

		/** set whether we will encode world related files. default to true.
		* By enabling path encoding, world related files like "worlddir/worldfile.txt" will be saved as "%WORLD%/worldfile.txt", thus 
		* even the entire world directory changes, the world files can still be found using path variables. Path encoding needs to be disabled when you are creating a template world.
		*/
		static void SetEnablePathEncoding(bool bEnable);

		/** get whether we will encode world related files. default to true.
		* By enabling path encoding, world related files like "worlddir/worldfile.txt" will be saved as "%WORLD%/worldfile.txt", thus 
		* even the entire world directory changes, the world files can still be found using path variables. Path encoding needs to be disabled when you are creating a template world.
		*/
		static bool GetEnablePathEncoding();

		//////////////////////////////////////////////////////////////////////////
		//
		// data provider functions
		// 
		//////////////////////////////////////////////////////////////////////////

		/** get the current NPC data provider. */
		static ParaDataProvider GetNpcDB();
		/** solely used for exporting*/
		static void GetNpcDB_(ParaDataProvider* out);

		/**
		* set the global NPC data base to a new database file. 
		* @param sConnectionstring : currently it is the file path of the database file. 
		*/
		static void SetNpcDB(const char* sConnectionstring);
		

		/** get the current attribute data provider. */
		static ParaDataProvider GetAttributeProvider();
		/** solely used for exporting*/
		static void GetAttributeProvider_(ParaDataProvider * out);

		/**
		* set the current attribute data base to a new database file. 
		* @param sConnectionstring : currently it is the file path of the database file. 
		*/
		static void SetAttributeProvider(const char* sConnectionstring);

		/**
		* set world database.
		* it sets attribute provider, NPC database, etc to the same database file.  
		* @param sConnectionstring : currently it is the file path of the database file. 
		*/
		static void SetWorldDB(const char* sConnectionstring);

		/** Get string from ID
		* @param ID: ID in kids db's string table.
		* @return string in the current game language */
		static string GetStringbyID(int ID);

		/** Insert the new string table entry to StringTable_DB
		* @param str: Entry in the current game language
		* @return ID of the inserted string
		*/
		static int InsertString(const char* strEN, const char * strCN);


		/** set the current server state. default value is 0.
		enum CurrentState
		{
		STATE_STAND_ALONE = 0, 
		STATE_SERVER,
		STATE_CLIENT,
		};
		*/
		static void SetServerState(int nState);
		static int GetServerState();

		/**
		* Call this function to send a copy of the local terrain to a destination world at a given resolution.
		* @param sDestination a destination namespace, it could be "all@server" or "@server", etc. This will be used to fill the destination address of the packet to be sent. 
		* @param center_x center of the terrain region to send 
		* @param center_y center of the terrain region to send
		* @param center_z center of the terrain region to send
		* @param fRadius radius of the terrain region to send
		* @param fResolution if this is 1.f, the local world terrain grid resolution is used. 
		* @return 
		*/
		static bool SendTerrainUpdate(const char* sDestination, float center_x,float center_y,float center_z, float fRadius, float fResolution);


		/** get current world name*/
		static string GetWorldName();

		/** get world root path. suppose the given world name is "sample". The generated file name is "sample/" */
		static string GetWorldDirectory();

		/**
		* Paraworld is told to execute in the given sandbox. 
		(1)	ParaWorld namespace supports a sandbox mode, which can be turned on and off on demand. Once turned on, all scripts from the current game world will be executed in a separate and newly created script runtime environment. 
		(2)	Sandbox mode is an isolated mode that does not have any link with the glia file environment.
		(3)	The world scripts protected by the sandbox runtime environment includes: terrain tile onload script, biped event handler scripts, such as character onload, onclick events. 
		(4)	The sandbox environment includes the following functions that could be used: ParaScene, ParaUI namespace functions. It also explicitly disabled the following functions:
		a)	Dofile()
		b)	Io, ParaIO, Exec
		c)	Require(),NPL.load, NPL.activate, NPL.download: cut off any way to manually load a file. It adds
		d)	Log
		e)	Download some file to replace local file.
		f)	Changing the Enter sand box function or almost any function to some fake function.
		(5)	The following attack methods should be prevented by the sandbox environment
		a)	Execute or load any external application
		b)	Write to any file, including log file
		c)	Compromise data or functions in the glia file environment. Such as, changing and hooking the string method
		d)	Compromise the sandbox itself and then affect in the next sandbox entity.
		(6)	glia file environment does not have a sandbox mode. Because I found that any global sandbox mode implementation has a way to hack in, and I give up any measure of protecting the glia file environment. Sandbox protection for the world file is enough because that is the only source file that may not be provided by ParaEngine. In order to run any other code not provided by the ParaEngine, the user should have been well informed of the danger. But so far, there is no need to have a world that should inform the user. Because most world functions are supported in the world sandbox.

		* @param sNeuronFileName: script file name. Use NPL.CreateNeuronFile() to create a sandbox neuron file
		* if you do not want to use a sandbox, please specify NULL or "". 
		*/
		static void SetScriptSandBox(const object& SandboxNeuronFile);

		/** please see SetScriptSandBox() 
		* @return: it may return NULL if the sandbox does not exist. 
		*/
		static const char* GetScriptSandBox();
	};
}
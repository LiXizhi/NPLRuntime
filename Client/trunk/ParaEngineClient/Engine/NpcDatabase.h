#pragma once
#include "ic/ParaDatabase.h"
#include <vector>
namespace ParaEngine
{
	using namespace std;
	/** NPC database item (complete)*/
	struct CNpcDbItem
	{
		/** selectively update data fields.*/
		enum UPDATE_FIELDS
		{
			POSITION_FACING		= 0x1<<0,
			APPEARANCE_ATTRIBUTES	= 0x1<<1,
			MENTAL_STATE_0		= 0x1<<2,
			MENTAL_STATE_1		= 0x1<<3,
			MENTAL_STATE_2		= 0x1<<4,
			MENTAL_STATE_3		= 0x1<<5,
			RPG_ATTRIBUTES		= 0x1<<6,
			DND_ATTRIBUTES		= 0x1<<7,
			ALL_ATTRIBUTES		= 0xffffffff
		};
		
		//////////////////////////////////////////////////////////////////////////
		// Biped attributes
		string m_sName;
		string m_sAssetName;
		bool m_bIsGlobal;
		bool m_bSnapToTerrain;
		float m_fRadius;
		float m_fFacing;
		float m_fScaling;
		Vector3 m_vPos;

		//////////////////////////////////////////////////////////////////////////
		// RPG attributes
		/** character ID in database. */
		int m_nID;

		/** @see CHARACTER_TYPE, default to CHAR_NONE*/
		int m_nCharacterType;

		/** character mental states */
		DWORD m_MentalState[4];

		//////////////////////////////////////////////////////////////////////////
		// DnD attributes
		/** the life or hit point of the character. */
		float m_fLifePoint;
		/** the age of the character. */
		float m_fAge;
		/** the height of the character. */
		float m_fHeight;
		/** the weight of the character. */
		float m_fWeight;
		/** the occupation of the character. */
		int m_nOccupation;
		/** the race and sex of the character. It 2N it is female, if 2N+1, it is male.*/
		int m_nRaceSex;
		/** character strength */
		float m_fStrength;
		/** character dexterity */
		float m_fDexterity;
		/** character Intelligence */
		float m_fIntelligence;
		/** character base defense*/
		float m_fBaseDefense;
		float m_fDefense, m_fDefenseflat, m_fDefenseMental;
		/** character base attack*/
		float m_fBaseAttack;
		float m_fAttackMelee, m_fAttackRanged, m_fAttackMental;
		/** the maximum load that a character can lift or carry */
		float m_fMaxLifeLoad;
		/** levels can be deducted from this value.*/
		int m_nHeroPoints;

		//////////////////////////////////////////////////////////////////////////
		// IGameObject attributes
		float m_fPerceptiveRadius;
		float m_fSentientRadius;
		int m_nGroupID;
		DWORD m_dwSentientField;
		string m_sOnLoadScript;
		vector<byte> m_binCustomAppearance;
	public:
		CNpcDbItem(){LoadDefaultValues();};
		/** load the default database values. */
		void LoadDefaultValues();
	};
	typedef vector<CNpcDbItem> NPCDBItemList;

	/** NPC data provider*/
	class CNpcDatabase
	{
	public:
		CNpcDatabase(void);
		~CNpcDatabase(void);

	public:
		//////////////////////////////////////////////////////////////////////////
		// common function 

		/** whether db is opened. */
		bool IsValid() {return m_pDataBase.get() !=0;};

		/** delete the database and set everything to NULL*/
		void Cleanup();
		/** get the database object associated with this provider*/
		DBEntity* GetDBEntity(){return m_pDataBase.get();};
		/**
		* replace the current database with current one.  the old one is closed and the new once will be opened. 
		* @param sConnectionstring: the file path
		*/
		void SetDBEntity(const string& sConnectionstring);

		/** reset the database to blank*/
		void ResetDatabase();

		/** ensure that the database has been set up properly. If not, ResetDatabase() is called to reset the database to blank */
		void ValidateDatabase();
#ifdef _DEBUG
		/** test data base*/
		void TestDB();
#endif

	public:
		//////////////////////////////////////////////////////////////////////////
		// query functions 

		/**
		* @param nNPCID character ID in the database
		* @param pOut : the structure will be filled with data in the database
		* @return true if succeed
		*/
		bool SelectNPCByID(int nNPCID, CNpcDbItem& pOut);
		/**
		*
		* @param sName character name
		* @param pOut : the structure will be filled with data in the database
		* @return true if succeed
		*/
		bool SelectNPCByName(const string& sName, CNpcDbItem& pOut);
		
		/**
		* return all NPC whose position is within a AABB region
		* @param pOut : the list will be filled with data in the database that meat the criterion.
		* @param vMin min point in world coordinate system, y component is ignored
		* @param vMax max point in world coordinate system, y component is ignored 
		* @return true if succeed
		*/
		bool SelectNPCListByRegion(NPCDBItemList* pOut, const Vector3& vMin, const Vector3& vMax);

		/**
		* @param npc insert the new character to the database, the ID of npc will be ignored and filled with actual ID if inserted successfully
		* @return true if succeed
		*/
		bool InsertNPC(CNpcDbItem& npc);

		/**
		* get NPC ID by name
		* @param name name of character
		* @return id is returned if found in database; otherwise non-positive value(-1) is returned.
		*/
		int GetNPCIDByName(const string& name);
		
		/**
		* the total number of NPC in the database. There is no statement cached for this call.
		* @return 
		*/
		int GetNPCCount();

		/**
		* delete the NPC from database
		* @param nNPCID NPC ID
		* @return true if succeed
		*/
		bool DeleteNPCByID(int nNPCID);
		/**
		* delete the NPC from database
		* @param sName 
		* @return true if succeed
		*/
		bool DeleteNPCByName(const string& sName);

		/**
		* Selectively update the data fields of NPC.
		* @param npc either the ID or name of the npc must be valid.
		* @param dwFields : a bitwise of database fields specifying which data is to be updated. If it is ALL_ATTRIBUTES(0xffffffff), all data will be updated.
		* please refer to CNpcDbItem::UPDATE_FIELDS for the meaning of the data fields which can be selectively updated. 
		* @return true if succeed
		*/
		bool UpdateNPC(const CNpcDbItem& npc, DWORD dwFields);

	private:
		/** base database interface */
		ParaEngine::asset_ptr<DBEntity> m_pDataBase;
		CDBRecordSet* m_pProcInsertNPC;
		CDBRecordSet* m_pProcSelectNPCByID;
		CDBRecordSet* m_pProcSelectNPCByName;
		CDBRecordSet* m_pProcSelectNPCListByRegion;
		CDBRecordSet* m_pProcGetNPCIDByName;
		CDBRecordSet* m_pProcDeleteNPCByID;
		CDBRecordSet* m_pProcDeleteNPCByName;
		CDBRecordSet* m_pProcUpdateNPC;
	};

}

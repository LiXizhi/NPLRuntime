#pragma once
#include "BipedObject.h"
#include <vector>
namespace ParaEngine
{

	using namespace std;
	// forward declare
	//class CDnDCharacterAttribute;
	struct CNpcDbItem;

	/** Dungeons and Dragons character attributes */
	class CDnDCharacterAttribute : public IAttributeFields
	{
	public:
		CDnDCharacterAttribute();
		//////////////////////////////////////////////////////////////////////////
		// implementation of IAttributeFields

		/** attribute class ID should be identical, unless one knows how overriding rules work.*/
		virtual int GetAttributeClassID(){return ATTRIBUTE_CLASSID_CDndCharacterAttribute;}
		/** a static string, describing the attribute class object's name */
		virtual const char* GetAttributeClassName(){static const char name[] = "Typical DnD Character"; return name;}
		/** a static string, describing the attribute class object */
		virtual const char* GetAttributeClassDescription(){static const char desc[] = ""; return desc;}
		/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);

	public:
		/** the life or hit point of the character. */
		float m_fLifePoint;
		/** the age of the character. */
		float m_fAge;
		/** the height of the character. */
		float m_fHeight;
		/** the weight of the character. 
		Please note that this field has been used to save density of the character. */
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

		/** character skill ID lists: such as magics */
		vector<int> m_skills;
		/** character tool ID lists: such as weapons, medicines as in its inventory */
		vector<int> m_tools;
	};

	/**
	* A typical character in role playing game. 
	* It may be an other-player controlled character(OPC), a PC, NPC, or whatever.
	* it contains attributes such as attack point, life value, defense, etc, which are typical in an RPG or MMORPG game. 
	*/
	class CRpgCharacter : public CBipedObject
	{
	public:
		CRpgCharacter(void);
		virtual ~CRpgCharacter(void);
		virtual CBaseObject::_SceneObjectType GetType(){return CBaseObject::RPGBiped;};
	public:
		//////////////////////////////////////////////////////////////////////////
		// implementation of IAttributeFields

		/** attribute class ID should be identical, unless one knows how overriding rules work.*/
		virtual int GetAttributeClassID(){return ATTRIBUTE_CLASSID_CRpgCharacter;}
		/** a static string, describing the attribute class object's name */
		virtual const char* GetAttributeClassName(){static const char name[] = "RPG Character"; return name;}
		/** a static string, describing the attribute class object */
		virtual const char* GetAttributeClassDescription(){static const char desc[] = ""; return desc;}
		/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);
		
		ATTRIBUTE_METHOD1(CRpgCharacter, GetCharacterID_s, int*)		{*p1 = cls->GetCharacterID(); return S_OK;}
		ATTRIBUTE_METHOD1(CRpgCharacter, SetCharacterID_s, int)		{cls->SetCharacterID(p1); return S_OK;}

		ATTRIBUTE_METHOD1(CRpgCharacter, GetCharacterType_s, int*)		{*p1 = cls->GetCharacterType(); return S_OK;}
		ATTRIBUTE_METHOD1(CRpgCharacter, SetCharacterType_s, int)		{cls->SetCharacterType(p1); return S_OK;}

		ATTRIBUTE_METHOD1(CRpgCharacter, GetMentalState0_s, int*)		{*p1 = cls->GetMentalState(0); return S_OK;}
		ATTRIBUTE_METHOD1(CRpgCharacter, SetMentalState0_s, int)		{cls->SetMentalState(0, (byte)p1); return S_OK;}
		ATTRIBUTE_METHOD1(CRpgCharacter, GetMentalState1_s, int*)		{*p1 = cls->GetMentalState(1); return S_OK;}
		ATTRIBUTE_METHOD1(CRpgCharacter, SetMentalState1_s, int)		{cls->SetMentalState(1, (byte)p1); return S_OK;}
		ATTRIBUTE_METHOD1(CRpgCharacter, GetMentalState2_s, int*)		{*p1 = cls->GetMentalState(2); return S_OK;}
		ATTRIBUTE_METHOD1(CRpgCharacter, SetMentalState2_s, int)		{cls->SetMentalState(2, (byte)p1); return S_OK;}
		ATTRIBUTE_METHOD1(CRpgCharacter, GetMentalState3_s, int*)		{*p1 = cls->GetMentalState(3); return S_OK;}
		ATTRIBUTE_METHOD1(CRpgCharacter, SetMentalState3_s, int)		{cls->SetMentalState(3, (byte)p1); return S_OK;}

#ifndef DEFINE_ATTRIBUTE_METHODS_DND
#define DEFINE_ATTRIBUTE_METHODS_DND(type, AttriName) \
		ATTRIBUTE_METHOD1(CRpgCharacter, Get##AttriName##_s, type*)		{*p1 = cls->Get##AttriName(); return S_OK;}\
		ATTRIBUTE_METHOD1(CRpgCharacter, Set##AttriName##_s, type)		{cls->Set##AttriName(p1); return S_OK;}
#endif

		DEFINE_ATTRIBUTE_METHODS_DND(float, LifePoint);
		DEFINE_ATTRIBUTE_METHODS_DND(float, Age);
		DEFINE_ATTRIBUTE_METHODS_DND(float, BodyHeight);
		DEFINE_ATTRIBUTE_METHODS_DND(float, Weight);
		DEFINE_ATTRIBUTE_METHODS_DND(int, Occupation);
		DEFINE_ATTRIBUTE_METHODS_DND(int, RaceSex);
		DEFINE_ATTRIBUTE_METHODS_DND(float, Strength);
		DEFINE_ATTRIBUTE_METHODS_DND(float, Dexterity);
		DEFINE_ATTRIBUTE_METHODS_DND(float, Intelligence);
		DEFINE_ATTRIBUTE_METHODS_DND(float, BaseDefense);
		DEFINE_ATTRIBUTE_METHODS_DND(float, Defense);
		DEFINE_ATTRIBUTE_METHODS_DND(float, Defenseflat);
		DEFINE_ATTRIBUTE_METHODS_DND(float, DefenseMental);
		DEFINE_ATTRIBUTE_METHODS_DND(float, BaseAttack);

		DEFINE_ATTRIBUTE_METHODS_DND(float, AttackMelee);
		DEFINE_ATTRIBUTE_METHODS_DND(float, AttackRanged);
		DEFINE_ATTRIBUTE_METHODS_DND(float, AttackMental);
		DEFINE_ATTRIBUTE_METHODS_DND(float, MaxLifeLoad);
		DEFINE_ATTRIBUTE_METHODS_DND(int, HeroPoints);
		// TODO: define GetSkill and GetTool
		
	public:
		/** character type. */
		enum CHARACTER_TYPE{
			CHAR_NPC,
			CHAR_BOSS,
			CHAR_HERO,
			CHAR_SPAWN,
			CHAR_NONE, // character without any attributes
		};
		
		/**
		* get character ID in the database
		* @return 
		*/
		int GetCharacterID(){return m_nCharacterID;}
		/**
		* set character ID in the database
		* @param nID : ID to set
		*/
		void SetCharacterID(int nID){m_nCharacterID = nID;}

		/**
		* get type
		* @return see CHARACTER_TYPE
		*/
		int GetCharacterType(){return m_nCharacterType;};
		/**
		* see CHARACTER_TYPE
		* @param nType 
		*/
		void SetCharacterType(int nType);

		/**
		* the character mental state array.
		* @param nIndex : index in the state array. usually 0-5. 
		* @return : 0 is returned if mental state at the specified index has never been set before.
		*/
		DWORD GetMentalState(int nIndex);
		/**
		* the character mental state array.
		* @param nIndex : index in the state array. usually 0-5. 
		* @param data : in range [0,0xffffffff], the data to be set at the specified index in the mental state array. 
		*/
		void SetMentalState(int nIndex, DWORD data);

		//////////////////////////////////////////////////////////////////////////
		//
		// define a group of attribute get/set member functions.
		//
		//////////////////////////////////////////////////////////////////////////
#ifndef DEFINE_CLASS_ATTRIBUTE_DND
		#define DEFINE_CLASS_ATTRIBUTE_DND(type, AttriName, memberName, defaultValue) \
			type Get##AttriName(){return (m_pDnDAttribute!=0)?m_pDnDAttribute->memberName : defaultValue;} \
			void Set##AttriName(type p1){if(m_pDnDAttribute!=0) m_pDnDAttribute->memberName = p1;}
#endif

		DEFINE_CLASS_ATTRIBUTE_DND(float, LifePoint, m_fLifePoint, 1);
		DEFINE_CLASS_ATTRIBUTE_DND(float, Age, m_fAge, 1);
		DEFINE_CLASS_ATTRIBUTE_DND(float, BodyHeight, m_fHeight, 1);
		DEFINE_CLASS_ATTRIBUTE_DND(float, Weight, m_fWeight, 1);
		DEFINE_CLASS_ATTRIBUTE_DND(int, Occupation, m_nOccupation, 1);
		DEFINE_CLASS_ATTRIBUTE_DND(int, RaceSex, m_nRaceSex, 1);
		DEFINE_CLASS_ATTRIBUTE_DND(float, Strength, m_fStrength, 1);
		DEFINE_CLASS_ATTRIBUTE_DND(float, Dexterity, m_fDexterity, 1);
		DEFINE_CLASS_ATTRIBUTE_DND(float, Intelligence, m_fIntelligence, 1);
		DEFINE_CLASS_ATTRIBUTE_DND(float, BaseDefense, m_fBaseDefense, 1);
		DEFINE_CLASS_ATTRIBUTE_DND(float, Defense, m_fDefense, 1);
		DEFINE_CLASS_ATTRIBUTE_DND(float, Defenseflat, m_fDefenseflat, 1);
		DEFINE_CLASS_ATTRIBUTE_DND(float, DefenseMental, m_fDefenseMental, 1);
		DEFINE_CLASS_ATTRIBUTE_DND(float, BaseAttack, m_fBaseAttack, 1);
		DEFINE_CLASS_ATTRIBUTE_DND(float, AttackMelee, m_fAttackMelee, 1);
		DEFINE_CLASS_ATTRIBUTE_DND(float, AttackRanged, m_fAttackRanged, 1);
		DEFINE_CLASS_ATTRIBUTE_DND(float, AttackMental, m_fAttackMental, 1);
		DEFINE_CLASS_ATTRIBUTE_DND(float, MaxLifeLoad, m_fMaxLifeLoad, 1);
		DEFINE_CLASS_ATTRIBUTE_DND(int, HeroPoints, m_nHeroPoints, 1);
		
		/**
		* get the character skill at the given slot.
		* @param nIndex : index in the state array. 
		* @return : 0 is returned if skill at the specified index has never been set before.
		*/
		int GetSkill(int nIndex);
		/**
		* set the character skill at the given slot.
		* @param nIndex : index in the state array. 
		* @param nSkillID: the ID to be set at the specified index in the skill slot. 
		*/
		void SetSkill(int nIndex, int nSkillID);

		/**
		* get the character tool at the given slot.
		* @param nIndex : index in the state array. 
		* @return : 0 is returned if tool at the specified index has never been set before.
		*/
		int GetTool(int nIndex);
		/**
		* set the character tool at the given slot.
		* @param nIndex : index in the state array. 
		* @param ntoolID: the ID to be set at the specified index in the tool slot. 
		*/
		void SetTool(int nIndex, int ntoolID);


		/**
		* convert this object to NPC database item
		* @param npc : where db item is saved. please note that if this is a newly created item, then ID will be <=0. 
		* @return : true if conversion succeeded.
		*/
		bool ToNpcDbItem(CNpcDbItem& npc);

		/**
		* Update the current NPC from data item from the database
		* @param npc the db item by which this character should be updated.
		* @param dwFields a bitwise of database fields specifying which data is to be updated. If it is ALL_ATTRIBUTES(0xffffffff), all data will be updated.
		* please refer to CNpcDbItem::UPDATE_FIELDS for the meaning of the data fields which can be selectively updated. 
		* @return true if succeed.
		*/
		bool UpdateFromNPCDbItem(const CNpcDbItem& npc, DWORD dwFields);
	private:
		/** character ID in the database */
		int m_nCharacterID;
		/** @see CHARACTER_TYPE, default to CHAR_NONE*/
		int m_nCharacterType;

		/** character mental states */
		vector<DWORD> m_MentalStates;

		/** extended character attribute data */
		union{
			CDnDCharacterAttribute* m_pDnDAttribute;
			void* m_pAttributes;
		};
	};

}

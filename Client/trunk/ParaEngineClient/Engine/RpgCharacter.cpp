//-----------------------------------------------------------------------------
// Class:	CRpgCharacter
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Date:	2005.5.18
// desc:
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ParaXAnimInstance.h"
#include "CustomCharModelInstance.h"
#include "ParaWorldAsset.h"
#include "terrain/GlobalTerrain.h"
#include "RpgCharacter.h"
#include "NpcDatabase.h"
#include "memdebug.h"

using namespace ParaEngine;

//////////////////////////////////////////////////////////////////////////
//
// CDnDCharacterAttribute
//
//////////////////////////////////////////////////////////////////////////
CDnDCharacterAttribute::CDnDCharacterAttribute()
{
	m_fLifePoint = 100;
	m_fAge = 1;
	m_fHeight = 1.78f;
	m_fWeight = 55;
	m_nOccupation = 0;
	m_nRaceSex = 0;

	m_fStrength = 0;
	m_fDexterity = 0;
	m_fIntelligence = 0;
	m_fBaseDefense = 0;
	m_fDefense = m_fDefenseflat = m_fDefenseMental = 0;
	m_fBaseAttack = 0;
	m_fAttackMelee = m_fAttackRanged = m_fAttackMental = 0;
	m_fMaxLifeLoad = 10;

	m_nHeroPoints = 0;
}
int CDnDCharacterAttribute::InstallFields(CAttributeClass* pClass, bool bOverride)
{
	// Note: CDnDCharacterAttribute does not contain fields.
	IAttributeFields::InstallFields(pClass, bOverride);


	return S_OK;
}

//////////////////////////////////////////////////////////////////////////
//
// CRpgCharacter
//
//////////////////////////////////////////////////////////////////////////

CRpgCharacter::CRpgCharacter(void)
:m_pAttributes(NULL),m_nCharacterType(CHAR_NONE),m_nCharacterID(-1)
{
	SetMyType( _RPG );
	SetAttribute(OBJ_VOLUMN_TILE_VISITOR);
}

CRpgCharacter::~CRpgCharacter(void)
{
	SAFE_DELETE(m_pDnDAttribute);
}


int CRpgCharacter::InstallFields(CAttributeClass* pClass, bool bOverride)
{
	IAttributeFields::InstallFields(pClass, bOverride);

	pClass->AddField("Character ID",FieldType_Int, SetCharacterID_s, GetCharacterID_s, NULL, "", bOverride);
	pClass->AddField("Character type",FieldType_Int, SetCharacterType_s, GetCharacterType_s, NULL, "", bOverride);
	pClass->AddField("state0",FieldType_Int, SetMentalState0_s, GetMentalState0_s, NULL, "", bOverride);
	pClass->AddField("state1",FieldType_Int, SetMentalState1_s, GetMentalState1_s, NULL, "", bOverride);
	pClass->AddField("state2",FieldType_Int, SetMentalState2_s, GetMentalState2_s, NULL, "", bOverride);
	pClass->AddField("state3",FieldType_Int, SetMentalState3_s, GetMentalState3_s, NULL, "", bOverride);

	pClass->AddField("life point",FieldType_Float, SetLifePoint_s, GetLifePoint_s, NULL, "", bOverride);
	pClass->AddField("Age",FieldType_Float, SetAge_s, GetAge_s, NULL, "", bOverride);
	pClass->AddField("Height",FieldType_Float, SetBodyHeight_s, GetBodyHeight_s, NULL, "", bOverride);
	pClass->AddField("Weight",FieldType_Float, SetWeight_s, GetWeight_s, NULL, "", bOverride);
	pClass->AddField("Occupation",FieldType_Int, SetOccupation_s, GetOccupation_s, NULL, "", bOverride);
	pClass->AddField("RaceSex",FieldType_Int, SetRaceSex_s, GetRaceSex_s, NULL, "", bOverride);
	pClass->AddField("Strength",FieldType_Float, SetStrength_s, GetStrength_s, NULL, "", bOverride);
	pClass->AddField("Dexterity",FieldType_Float, SetDexterity_s, GetDexterity_s, NULL, "", bOverride);
	pClass->AddField("Intelligence",FieldType_Float, SetIntelligence_s, GetIntelligence_s, NULL, "", bOverride);
	pClass->AddField("Base Defense",FieldType_Float, SetBaseDefense_s, GetBaseDefense_s, NULL, "", bOverride);
	pClass->AddField("Defense",FieldType_Float, SetDefense_s, GetDefense_s, NULL, "", bOverride);
	pClass->AddField("Defense flat",FieldType_Float, SetDefenseflat_s, GetDefenseflat_s, NULL, "", bOverride);
	pClass->AddField("Defense Mental",FieldType_Float, SetDefenseMental_s, GetDefenseMental_s, NULL, "", bOverride);
	pClass->AddField("Base Attack",FieldType_Float, SetBaseAttack_s, GetBaseAttack_s, NULL, "", bOverride);
	pClass->AddField("Attack Melee",FieldType_Float, SetAttackMelee_s, GetAttackMelee_s, NULL, "", bOverride);
	pClass->AddField("Attack Ranged",FieldType_Float, SetAttackRanged_s, GetAttackRanged_s, NULL, "", bOverride);
	pClass->AddField("Attack Mental",FieldType_Float, SetAttackMental_s, GetAttackMental_s, NULL, "", bOverride);
	pClass->AddField("MaxLifeLoad",FieldType_Float, SetMaxLifeLoad_s, GetMaxLifeLoad_s, NULL, "", bOverride);
	pClass->AddField("Hero Points",FieldType_Int, SetHeroPoints_s, GetHeroPoints_s, NULL, "", bOverride);

	return S_OK;
}

void CRpgCharacter::SetCharacterType(int nType)
{
	if((nType != CHAR_NONE) && m_pDnDAttribute==NULL)
	{
		m_pDnDAttribute = new CDnDCharacterAttribute();
	}
}

DWORD CRpgCharacter::GetMentalState(int nIndex)
{
	if ((int)m_MentalStates.size() > nIndex)
		return m_MentalStates[nIndex];
	else
		return 0;
}

void CRpgCharacter::SetMentalState(int nIndex, DWORD data)
{
	if ((int)m_MentalStates.size() > nIndex)
		m_MentalStates[nIndex] = data;
	else
	{
		m_MentalStates.resize(nIndex+1,0);
		m_MentalStates[nIndex] = data;
	}
}

int CRpgCharacter::GetSkill(int nIndex)
{
	if( (m_pDnDAttribute!=0) &&
		((int)m_pDnDAttribute->m_skills.size() > nIndex) )
	{
		return m_pDnDAttribute->m_skills[nIndex];
	}
	else
		return 0;
}

void CRpgCharacter::SetSkill(int nIndex, int nSkillID)
{
	if(m_pDnDAttribute!=0)
	{
		if ((int)m_pDnDAttribute->m_skills.size() > nIndex)
			m_pDnDAttribute->m_skills[nIndex] = nSkillID;
		else
		{
			m_pDnDAttribute->m_skills.resize(nIndex+1,0);
			m_pDnDAttribute->m_skills[nIndex] = nSkillID;
		}
	}
}

int CRpgCharacter::GetTool(int nIndex)
{
	if( (m_pDnDAttribute!=0) &&
		((int)m_pDnDAttribute->m_tools.size() > nIndex) )
	{
		return m_pDnDAttribute->m_tools[nIndex];
	}
	else
		return 0;
}

void CRpgCharacter::SetTool(int nIndex, int nToolID)
{
	if(m_pDnDAttribute!=0)
	{
		if ((int)m_pDnDAttribute->m_tools.size() > nIndex)
			m_pDnDAttribute->m_tools[nIndex] = nToolID;
		else
		{
			m_pDnDAttribute->m_tools.resize(nIndex+1,0);
			m_pDnDAttribute->m_tools[nIndex] = nToolID;
		}
	}
}

bool CRpgCharacter::ToNpcDbItem(CNpcDbItem& npc)
{
	CharModelInstance* pChar =  GetCharModelInstance();
	CAnimInstanceBase * pAI = GetAnimInstance();
	if(pChar && pAI)
	{
		if(pChar->GetBaseModel())
		{
			npc.m_nID = m_nCharacterID;
			npc.m_sName = GetIdentifier();
			npc.m_sAssetName = pChar->GetBaseModel()->GetKey();
			npc.m_bIsGlobal = IsGlobal();
			npc.m_bSnapToTerrain = false;
			npc.m_fRadius = GetPhysicsRadius();
			npc.m_fFacing = GetFacing();
			npc.m_fScaling = pAI->GetSizeScale();
			npc.m_vPos = GetPosition();
			npc.m_nCharacterType = m_nCharacterType;
			npc.m_fWeight = GetDensity();

			for (int i=0;i<4;++i)
			{
				npc.m_MentalState[i] = GetMentalState(i);
			}

			if(m_pDnDAttribute!=0)
			{
				npc.m_fLifePoint = m_pDnDAttribute->m_fLifePoint;
				npc.m_fAge = m_pDnDAttribute->m_fAge;
				npc.m_fHeight = m_pDnDAttribute->m_fHeight;
				//npc.m_fWeight = m_pDnDAttribute->m_fWeight;
				npc.m_nOccupation = m_pDnDAttribute->m_nOccupation;
				npc.m_nRaceSex = m_pDnDAttribute->m_nRaceSex;

				npc.m_fStrength = m_pDnDAttribute->m_fStrength;
				npc.m_fDexterity = m_pDnDAttribute->m_fDexterity;
				npc.m_fIntelligence = m_pDnDAttribute->m_fIntelligence;
				npc.m_fBaseDefense = m_pDnDAttribute->m_fDexterity;
				npc.m_fDefense = m_pDnDAttribute->m_fDefense;
				npc.m_fDefenseflat = m_pDnDAttribute->m_fDefenseflat;
				npc.m_fDefenseMental = m_pDnDAttribute->m_fDefenseMental;
				npc.m_fBaseAttack = m_pDnDAttribute->m_fBaseAttack;
				npc.m_fAttackMelee = m_pDnDAttribute->m_fAttackMelee;
				npc.m_fAttackRanged = m_pDnDAttribute->m_fAttackRanged;
				npc.m_fAttackMental = m_pDnDAttribute->m_fAttackMental;
				npc.m_fMaxLifeLoad = m_pDnDAttribute->m_fMaxLifeLoad;
				npc.m_nHeroPoints = m_pDnDAttribute->m_nHeroPoints;
			}
			
			npc.m_fPerceptiveRadius = GetPerceptiveRadius();
			npc.m_fSentientRadius = GetSentientRadius();
			npc.m_nGroupID = GetGroupID();
			npc.m_dwSentientField = GetSentientField();
			

			{ /// build on load attribute text file

				npc.m_sOnLoadScript.clear();
				if(!GetOnLoadScript().empty())
				{
					npc.m_sOnLoadScript += "OnLoad=" + GetOnLoadScript()+"\n";
				}

				IGameObject::ScriptCallback* pCallBack = NULL;
#define APPENDSCRIPT(name) \
	pCallBack = GetScriptCallback(IGameObject::Type_##name##); \
	if(pCallBack!=NULL)\
	npc.m_sOnLoadScript += #name"="+pCallBack->script_func+"\n";

				APPENDSCRIPT(EnterSentientArea);
				APPENDSCRIPT(LeaveSentientArea);
				APPENDSCRIPT(Click);
				APPENDSCRIPT(Event);
				APPENDSCRIPT(Perception);
				APPENDSCRIPT(FrameMove);
				APPENDSCRIPT(Net_Send);
				APPENDSCRIPT(Net_Receive);

				if( !npc.m_sOnLoadScript.empty() && CGlobals::GetGlobalTerrain()->GetEnablePathEncoding())
				{
					CPathReplaceables::GetSingleton().EncodePath(npc.m_sOnLoadScript, npc.m_sOnLoadScript, "WORLD");
				}
			}
			
			
			static byte buffer[1024];
			//int nLength = pChar->save(buffer, 1024);
			int nLength = 0;
			if(nLength>0)
			{
				npc.m_binCustomAppearance.resize(nLength);
				memcpy(&npc.m_binCustomAppearance[0], buffer, nLength);
			}
			else
			{
				npc.m_binCustomAppearance.clear();
			}
			
			return true;
		}
	}
	return false;	
}

bool CRpgCharacter::UpdateFromNPCDbItem(const CNpcDbItem& npc, DWORD dwFields)
{
	if(dwFields & CNpcDbItem::APPEARANCE_ATTRIBUTES)
	{
		AssetEntity* pMAE = NULL;
		if(!npc.m_sAssetName.empty()){
			pMAE = CGlobals::GetAssetManager()->LoadParaX("", npc.m_sAssetName);
			if(pMAE)
			{
				if(!pMAE->IsValid())
					pMAE = NULL;
			}
		}
		if(pMAE!=NULL)
		{
			InitObject(pMAE);
			if(!npc.m_binCustomAppearance.empty())
			{
				CharModelInstance* pChar =  GetCharModelInstance();
				if(pChar!=NULL)
				{
					// pChar->load(&npc.m_binCustomAppearance[0], (int)npc.m_binCustomAppearance.size());
					pChar->SetModified();
				}
			}	
		}
	}
	
	if(dwFields & CNpcDbItem::POSITION_FACING)
	{
		SetFacing(npc.m_fFacing);
		SetPosition(DVector3(npc.m_vPos));
	}

	if(dwFields & CNpcDbItem::RPG_ATTRIBUTES)
	{
		// biped attributes
		SetIdentifier(npc.m_sName);
		if(npc.m_fScaling!=1.0f)
			SetSizeScale(npc.m_fScaling);
		// since physics radius is affected by size scale, we will set it here. 
		SetPhysicsRadius(npc.m_fRadius);

		SetDensity(npc.m_fWeight);

		// IGameObject attributes
		SetPerceptiveRadius(npc.m_fPerceptiveRadius);
		SetSentientField(npc.m_dwSentientField);
		SetGroupID(npc.m_nGroupID);
		SetSentientRadius(npc.m_fSentientRadius);

		MakeGlobal(npc.m_bIsGlobal);
		
		{ /// parse on load attribute file type, see RPGcharacter.cpp ToNpcDbItem() for how this is generated
			string sOnLoadScript;
			CPathReplaceables::GetSingleton().DecodePath(sOnLoadScript, npc.m_sOnLoadScript);
			
			CParaFile file(const_cast<char*>(sOnLoadScript.c_str()), (int)sOnLoadScript.size());
			if(!file.isEof())
			{
				string script;
				if(file.GetNextAttribute("OnLoad",script))
					SetOnLoadScript(script);
#define PARSESCRIPT(name)\
	if(file.GetNextAttribute(#name,script))\
	AddScriptCallback(IGameObject::Type_##name##, script);

				PARSESCRIPT(EnterSentientArea);
				PARSESCRIPT(LeaveSentientArea);
				PARSESCRIPT(Click);
				PARSESCRIPT(Event);
				PARSESCRIPT(Perception);
				PARSESCRIPT(FrameMove);
				PARSESCRIPT(Net_Send);
				PARSESCRIPT(Net_Receive);
			}
		}
		
		// RPG attributes
		SetCharacterID(npc.m_nID);
		SetCharacterType(npc.m_nCharacterType);
	}
	
	if(dwFields & (CNpcDbItem::MENTAL_STATE_0|CNpcDbItem::MENTAL_STATE_1|CNpcDbItem::MENTAL_STATE_2|CNpcDbItem::MENTAL_STATE_3))
	{
		for (int i=0;i<4;++i)
		{
			if(npc.m_MentalState[i] != 0)
			{
				SetMentalState(i, npc.m_MentalState[i]);
			}
		}
	}

	if(dwFields & CNpcDbItem::DND_ATTRIBUTES)
	{
		// DnD attributes
		SetStrength(npc.m_fStrength);
		SetRaceSex(npc.m_nRaceSex);
		SetLifePoint(npc.m_fLifePoint);
		SetMaxLifeLoad(npc.m_fMaxLifeLoad);
		SetBaseDefense(npc.m_fBaseDefense);
		SetDefense(npc.m_fBaseDefense);
		SetDefenseflat(npc.m_fDefenseflat);
		SetDefenseMental(npc.m_fDefenseMental);
		SetBaseAttack(npc.m_fBaseAttack);
		SetAttackMelee(npc.m_fAttackMelee);
		SetAttackMental(npc.m_fAttackMental);
		SetAttackRanged(npc.m_fAttackRanged);
		SetAge(npc.m_fAge);
		SetBodyHeight(npc.m_fHeight);
		//SetWeight(npc.m_fWeight);
		SetAge(npc.m_fAge);
		SetOccupation(npc.m_nOccupation);
		SetDexterity(npc.m_fDexterity);
		SetIntelligence(npc.m_fIntelligence);
		SetHeroPoints(npc.m_nHeroPoints);
	}
	return true;
}
//-----------------------------------------------------------------------------
// Class: IGameObject
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2006.3
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "SceneObject.h"
#include "WorldInfo.h"
#include "EventsCenter.h"
#ifdef USE_DIRECTX_RENDERER
#include "EditorHelper.h"
#endif
#include "terrain/GlobalTerrain.h"
#include "ShapeAABB.h"
#include "NPLHelper.h"
#include "util/StringHelper.h"
#include "TerrainTileRoot.h"
#include "IGameObject.h"
#include "memdebug.h"

using namespace ParaEngine;
using namespace std;

#ifndef MAX_FIELD_LENGTH
#define MAX_FIELD_LENGTH	1024
#endif

/**@def the default sentient radius. This is usually smaller than the smallest quad-tree terrain tile radius.
The default tile radius is usually around 128 meters depending on the game settings.  */
#define DEFAULT_SENTIENT_RADIUS			50.f
/**@def the default perceptive radius. */
#define DEFAULT_PERCEPTIVE_RADIUS		7.f


//////////////////////////////////////////////////////////////////////////
//
// IGameObject
//
//////////////////////////////////////////////////////////////////////////

IGameObject::IGameObject()
:m_bModified(false), m_nSentientObjCount(0),m_fPeceptiveRadius(DEFAULT_PERCEPTIVE_RADIUS),m_fSentientRadius(DEFAULT_SENTIENT_RADIUS), 
m_bAlwaysSentient(false), m_nGroup(0), m_dwSentientField(0), m_bIsGlobal(true),m_bIsPersistent(false), m_bIsLoaded(false),m_nFrameMoveInterval(0)
{

}

IGameObject::~IGameObject()
{

}

// sensor_name perpended from now on. 2007.10.29
#define APPEND_SENSOR_NAME(sCode)	std::string script="sensor_name=";NPL::NPLHelper::EncodeStringInQuotation(script,(int)(script.size()), GetName());script+=";";script += (sCode);

void IGameObject::LoadOnLoadScriptIfNot()
{
	if(m_bIsLoaded)
		return;
	else
	{
		m_bIsLoaded = true;
		if(!m_sOnLoadScript.empty())
		{
			std::string sFile, sCode;
			StringHelper::DevideString(m_sOnLoadScript, sFile, sCode, ';');

			APPEND_SENSOR_NAME(sCode);
			CGlobals::GetScene()->GetScripts().AddScript(sFile, Type_OnLoadScript, script, this);
		}
	}
}

int IGameObject::On_Attached()
{
	if(IsGlobal())
		LoadOnLoadScriptIfNot();
	return S_OK;
}

int IGameObject::On_Detached()
{
	return S_OK;
}

int IGameObject::On_EnterSentientArea()
{
	LoadOnLoadScriptIfNot();
	ScriptCallback* pCallback = GetScriptCallback(Type_EnterSentientArea);
	if(pCallback){
		const std::string& sFile = pCallback->GetFileName();
		const std::string& sCode = pCallback->GetCode();
		APPEND_SENSOR_NAME(sCode);
		CGlobals::GetScene()->GetScripts().AddScript(sFile, pCallback->func_type, script, this);
	}
	return S_OK;
}

int IGameObject::On_LeaveSentientArea()
{
	ScriptCallback* pCallback = GetScriptCallback(Type_LeaveSentientArea);
	if(pCallback){
		const std::string& sFile = pCallback->GetFileName();
		const std::string& sCode = pCallback->GetCode();
		APPEND_SENSOR_NAME(sCode);
		CGlobals::GetScene()->GetScripts().AddScript(sFile, pCallback->func_type, script, this);
	}
	return S_OK;
}

int IGameObject::On_Click(DWORD nMouseKey, DWORD dwParam1,  DWORD dwParam2)
{
	LoadOnLoadScriptIfNot();
	ScriptCallback* pCallback = GetScriptCallback(Type_Click);
	if(pCallback){
		const std::string& sFile = pCallback->GetFileName();
		const std::string& sCode = pCallback->GetCode();
		if(nMouseKey!=0)
		{
			APPEND_SENSOR_NAME(GenerateOnMouseScript(nMouseKey, dwParam1, dwParam2)+sCode);
			CGlobals::GetScene()->GetScripts().AddScript(sFile, pCallback->func_type, script, this);
		}
		else
		{
			APPEND_SENSOR_NAME(sCode);
			CGlobals::GetScene()->GetScripts().AddScript(sFile, pCallback->func_type, script, this);
		}
	}
	return S_OK;
}

int IGameObject::On_Event(DWORD nEventType, DWORD dwParam1,  DWORD dwParam2)
{
	LoadOnLoadScriptIfNot();
	ScriptCallback* pCallback = GetScriptCallback(Type_Event);
	if(pCallback){
		const std::string& sFile = pCallback->GetFileName();
		const std::string& sCode = pCallback->GetCode();
		APPEND_SENSOR_NAME(sCode);
		CGlobals::GetScene()->GetScripts().AddScript(sFile, pCallback->func_type, script, this);
	}
	return S_OK;
}

int IGameObject::On_Perception()
{
	LoadOnLoadScriptIfNot();
	ScriptCallback* pCallback = GetScriptCallback(Type_Perception);
	if(pCallback)
	{
		unsigned int nCurTime = ::GetTickCount();
		if((pCallback->GetLastTick() + m_nFrameMoveInterval) < nCurTime)
		{
			pCallback->SetLastTick(nCurTime);
			const std::string& sFile = pCallback->GetFileName();
			const std::string& sCode = pCallback->GetCode();
			APPEND_SENSOR_NAME(sCode);
			CGlobals::GetScene()->GetScripts().AddScript(sFile, pCallback->func_type, script, this);
		}
	}
	return S_OK;
}

int IGameObject::On_FrameMove()
{
	LoadOnLoadScriptIfNot();
	ScriptCallback* pCallback = GetScriptCallback(Type_FrameMove);
	if(pCallback)
	{
		unsigned int nCurTime = ::GetTickCount();
		if((pCallback->GetLastTick() + m_nFrameMoveInterval) < nCurTime)
		{
			pCallback->SetLastTick(nCurTime);
			
			const std::string& sFile = pCallback->GetFileName();
			const std::string& sCode = pCallback->GetCode();
			APPEND_SENSOR_NAME(sCode);
			CGlobals::GetScene()->GetScripts().AddScript(sFile, pCallback->func_type, script, this);
		}
	}
	return S_OK;
}

int IGameObject::On_Net_Send(DWORD dwNetType, DWORD dwParam1, DWORD dwParam2)
{
	ScriptCallback* pCallback = GetScriptCallback(Type_Net_Send);
	if(pCallback){
		const std::string& sFile = pCallback->GetFileName();
		const std::string& sCode = pCallback->GetCode();
		APPEND_SENSOR_NAME(sCode);
		CGlobals::GetScene()->GetScripts().AddScript(sFile, pCallback->func_type, script, this);
	}
	return S_OK;
}

int IGameObject::On_Net_Receive(DWORD dwNetType, DWORD dwParam1, DWORD dwParam2)
{
	ScriptCallback* pCallback = GetScriptCallback(Type_Net_Receive);
	if(pCallback){
		const std::string& sFile = pCallback->GetFileName();
		const std::string& sCode = pCallback->GetCode();
		APPEND_SENSOR_NAME(sCode);
		CGlobals::GetScene()->GetScripts().AddScript(sFile, pCallback->func_type, script, this);
	}
	return S_OK;
}

bool IGameObject::IsSentient()
{
	return m_nSentientObjCount>0 || IsAlwaysSentient();
}

float IGameObject::GetSentientRadius()
{
	return m_fSentientRadius;
}
void IGameObject::SetSentientRadius(float fR)
{
	m_fSentientRadius = fR;
}
float IGameObject::GetPerceptiveRadius()
{
	return m_fPeceptiveRadius;
}

void IGameObject::SetPerceptiveRadius(float fNewRaduis)
{
	m_fPeceptiveRadius = fNewRaduis;

	// this ensure that sentient radius is always larger than the perceiptive radius.
	if(GetSentientRadius() < m_fPeceptiveRadius)
		SetSentientRadius(m_fPeceptiveRadius);
}

int IGameObject::GetNumOfPerceivedObject()
{
	return (int)m_PerceivedList.size();
}

IGameObject* IGameObject::GetPerceivedObject(int nIndex)
{
	if(nIndex<GetNumOfPerceivedObject() && 0<=nIndex)
	{
		const string& m_identifier = m_PerceivedList[nIndex];
		/** search global and then OPC. */
		CBaseObject* pObj = CGlobals::GetScene()->GetGlobalObject(m_identifier);
		if(pObj!=NULL)
		{
			return pObj->QueryIGameObject();
		}
	}
	return NULL;
}

float IGameObject::GetDistanceSq2D(IGameObject* AnotherObj)
{
	if(AnotherObj!=NULL)
	{
		Vector3 vPosAnother = AnotherObj->GetPosition();
		Vector2 vec2PosAnother(vPosAnother.x, vPosAnother.z);

		Vector3 vPos = GetPosition();
		Vector2 vec2Pos(vPos.x, vPos.z);

		return (vec2PosAnother-vec2Pos).squaredLength();
	}
	else
		return 0.0f;
	
}

bool IGameObject::IsAlwaysSentient() const
{
	return m_bAlwaysSentient;
}

void IGameObject::SetAlwaysSentient(bool bAlways)
{
	m_bAlwaysSentient = bAlways;
}


void IGameObject::ForceSentient(bool bSentient)
{
	if(bSentient)
	{
		CGlobals::GetScene()->AddSentientObject(this, true);
		m_nSentientObjCount = 1;
	}
	else
	{
		CGlobals::GetScene()->DeleteSentientObject(this);
		m_nSentientObjCount = 0;
	}
}


void IGameObject::MakeSentient(bool bSentient)
{
	if(bSentient)
	{
		if(m_nSentientObjCount<=0){
			CGlobals::GetScene()->AddSentientObject(this, true);
			m_nSentientObjCount = 1;
		}
	}
	else
	{
		if(IsSentient()){
			CGlobals::GetScene()->DeleteSentientObject(this);
		}
		m_nSentientObjCount = 0;
		SetAlwaysSentient(false);
	}
	
}

void IGameObject::PathFinding(double dTimeDelta)
{
	return;
}

void IGameObject::AnimateBiped( double dTimeDelta, bool bSharpTurning)
{
	return;
}

CAIBase* IGameObject::GetAIModule()
{
	return NULL;
}

void IGameObject::UpdateTileContainer()
{
	Vector3 vPos = GetPosition();

	CTerrainTile * pTile = CGlobals::GetScene()->GetRootTile()->GetTileByPoint(vPos.x, vPos.z);
	SetTileContainer(pTile);
}

void IGameObject::MakeGlobal(bool bGlobal)
{
	if(IsGlobal()!=bGlobal)
	{
		if(GetTileContainer()!=NULL)
		{
			CGlobals::GetScene()->DetachObject(this);
			m_bIsGlobal = bGlobal;
			CGlobals::GetScene()->AttachObject(this);
		}
		else
		{
			m_bIsGlobal = bGlobal;
		}
	}
}

void IGameObject::SetGroupID(int nGroup)
{
	m_nGroup = nGroup%32;
}

int IGameObject::GetGroupID()
{
	return m_nGroup;
}

void IGameObject::SetSentientField(DWORD dwFieldOrGroup, bool bIsGroup)
{
	if(bIsGroup)
		m_dwSentientField |= 0x1<<((int)dwFieldOrGroup);
	else
		m_dwSentientField = dwFieldOrGroup;
}
DWORD IGameObject::GetSentientField()
{
	return m_dwSentientField;
}

bool IGameObject::IsSentientWith(const IGameObject * pObj)
{
	if(pObj!=NULL)
	{
		return (m_dwSentientField & (0x1<<pObj->m_nGroup))>0; // || IsAlwaysSentient();
	}
	else
		return false;
}

bool IGameObject::ResetField(int nFieldID)
{
	bool bFound = false;
	CAttributeField* pField = GetAttributeClass()->GetField(nFieldID);
	if(pField!=0)
	{
		bFound = true;
		if (pField->m_sFieldname.substr(0, 3) == "On_")
		{
			/* suppose world name is sample, character name  is ABC, then the default value is 
			[[;NPL.load("sample/character/ABC.lua");_character.ABC.On_Click();]]
			*/
			char tmp[MAX_FIELD_LENGTH+1];
			string sFileName = CGlobals::GetWorldInfo()->GetWorldCharacterDirectory() + GetIdentifier()+".lua";
			/*if(CGlobals::GetGlobalTerrain()->GetEnablePathEncoding())
			{
				CPathReplaceables::GetSingleton().EncodePath(sFileName, sFileName, "WORLD");
			}*/
			snprintf(tmp, MAX_FIELD_LENGTH, ";NPL.load(\"(gl)%s\");_character.%s.%s();", sFileName.c_str(), GetIdentifier().c_str(), pField->m_sFieldname.c_str());
			string sValue(tmp);
			pField->Set(this, sValue.c_str());

#ifdef USE_DIRECTX_RENDERER
			// create the script file if not exists
			string sScriptFile;
			if(CEditorHelper::SearchFileNameInScript(sScriptFile, sValue.c_str(),true))
			{
				CEditorHelper::CreateEmptyCharacterEventFile(sScriptFile.c_str(), GetIdentifier().c_str());
			}
#endif
		}
		else if(pField->m_sFieldname == "OnLoadScript")
		{
			/* suppose world name is sample, character name  is ABC, then the default value is 
			[[;NPL.load("sample/character/ABC.lua");_character.ABC.On_Click();]]
			*/
			char tmp[MAX_FIELD_LENGTH+1];
			string sFileName = CGlobals::GetWorldInfo()->GetWorldCharacterDirectory() + GetIdentifier()+".lua";
			/*if(CGlobals::GetGlobalTerrain()->GetEnablePathEncoding())
			{
				CPathReplaceables::GetSingleton().EncodePath(sFileName, sFileName, "WORLD");
			}*/
			snprintf(tmp, MAX_FIELD_LENGTH, ";NPL.load(\"(gl)%s\");_character.%s.On_Load();", sFileName.c_str(), GetIdentifier().c_str());
			string sValue(tmp);
			pField->Set(this, sValue.c_str());
#ifdef USE_DIRECTX_RENDERER
			// create the script file if not exists
			string sScriptFile;
			if(CEditorHelper::SearchFileNameInScript(sScriptFile, sValue.c_str(),true))
			{
				CEditorHelper::CreateEmptyCharacterEventFile(sScriptFile.c_str(), GetIdentifier().c_str());
			}
#endif
		}
		else
			bFound = false;
	}
	if(!bFound)
		bFound = CTileObject::ResetField(nFieldID);
	return bFound;
}

void IGameObject::SetPersistent(bool bPersistent)
{
	m_bIsPersistent = bPersistent;
}

bool ParaEngine::IGameObject::IsPersistent()
{
	return m_bIsPersistent;
}

void ParaEngine::IGameObject::SetTileContainer(CTerrainTile * pTile)
{
	if (GetTileContainer() != pTile)
	{
		if (CheckAttribute(OBJ_VOLUMN_TILE_VISITOR))
		{
			if (GetTileContainer() != NULL)
				GetTileContainer()->RemoveVisitor(this);
			if (pTile)
				pTile->AddVisitor(this);
		}
		if (!pTile)
			MakeSentient(false);
		CTileObject::SetTileContainer(pTile);
	}
}

bool IGameObject::SaveToDB()
{
	if(IsPersistent())
	{
		return CGlobals::GetScene()->SaveCharacterToDB(this) == S_OK;
	}
	else
	{
		return CGlobals::GetScene()->RemoveCharacterFromDB(this) == S_OK;
	}
}

bool IGameObject::InvokeEditor(int nFieldID, const string& sParameters)
{
	bool bFound = false;
	CAttributeField* pField = GetAttributeClass()->GetField(nFieldID);
	if(pField!=0)
	{
		bFound = true;
		if (pField->m_sFieldname.substr(0, 2) == "On")
		{
			///////////////////////////////////////////////////////////////////////////
			// get the script file name from the field value
#ifdef USE_DIRECTX_RENDERER
			string sScriptFile;
			const char* sValue = NULL;
			pField->Get(this, &sValue);
			
			if(sValue!=0 && CEditorHelper::SearchFileNameInScript(sScriptFile, sValue,true))
			{
				CEditorHelper::CreateEmptyCharacterEventFile(sScriptFile.c_str(), GetIdentifier().c_str());
			}
			else
				return false;
			return OpenWithDefaultEditor(sScriptFile.c_str(), false);
#else
			return false;
#endif
		}
		else
			bFound = false;
	}
	if(!bFound)
		bFound = CTileObject::InvokeEditor(nFieldID, sParameters);
	return bFound;
}

static CShapeAABB g_movableRegion(Vector3(16000,0,16000), Vector3(16000,16000,16000));
void IGameObject::SetMovableRegion(const CShapeAABB* aabb)
{
	// TODO: here all regions are actually the same g_movableRegion object. 
	// there is no per object settings for movable region. we may implement it in future.
	if(aabb!=0)
	{
		g_movableRegion = *aabb;
	}
}

const CShapeAABB* IGameObject::GetMovableRegion()
{
	return &g_movableRegion;
}

void IGameObject::SetOnLoadScript(const string& str) 
{
	if(m_sOnLoadScript != str)
	{
		m_sOnLoadScript = str;
		if(m_bIsLoaded)
		{
			// try loading the script immediately if we have already loaded the script. 
			m_bIsLoaded = false;
			LoadOnLoadScriptIfNot();
		}
	}
}

int IGameObject::InstallFields(CAttributeClass* pClass, bool bOverride)
{
	CTileObject::InstallFields(pClass, bOverride);


	pClass->AddField("IsModified", FieldType_Bool, (void*)SetModified_s, (void*)IsModified_s, NULL, "", bOverride);
	pClass->AddField("Save", FieldType_void, (void*)SaveToDB_s, NULL, NULL, "", bOverride);

	pClass->AddField("OnLoadScript", FieldType_String, (void*)SetOnLoadScript_s, (void*)GetOnLoadScript_s, CAttributeField::GetSimpleSchemaOfScript(), "", bOverride);
	pClass->AddField("IsLoaded", FieldType_Bool, (void*)SetLoaded_s, (void*)IsLoaded_s, NULL, "", bOverride);

	pClass->AddField("On_EnterSentientArea", FieldType_String, (void*)SetEnterSentientArea_s, (void*)GetEnterSentientArea_s, CAttributeField::GetSimpleSchemaOfScript(), "", bOverride);
	pClass->AddField("On_LeaveSentientArea", FieldType_String, (void*)SetLeaveSentientArea_s, (void*)GetLeaveSentientArea_s, CAttributeField::GetSimpleSchemaOfScript(), "", bOverride);
	pClass->AddField("On_Click", FieldType_String, (void*)SetClick_s, (void*)GetClick_s, CAttributeField::GetSimpleSchemaOfScript(), "", bOverride);
	pClass->AddField("On_Event", FieldType_String, (void*)SetEvent_s, (void*)GetEvent_s, CAttributeField::GetSimpleSchemaOfScript(), "", bOverride);
	pClass->AddField("On_Perception", FieldType_String, (void*)SetPerception_s, (void*)GetPerception_s, CAttributeField::GetSimpleSchemaOfScript(), "", bOverride);
	pClass->AddField("On_FrameMove", FieldType_String, (void*)SetFrameMove_s, (void*)GetFrameMove_s, CAttributeField::GetSimpleSchemaOfScript(), "", bOverride);
	pClass->AddField("On_Net_Send", FieldType_String, (void*)SetNet_Send_s, (void*)GetNet_Send_s, CAttributeField::GetSimpleSchemaOfScript(), "", bOverride);
	pClass->AddField("On_Net_Receive", FieldType_String, (void*)SetNet_Receive_s, (void*)GetNet_Receive_s, CAttributeField::GetSimpleSchemaOfScript(), "", bOverride);
	pClass->AddField("FrameMoveInterval", FieldType_Int, (void*)SetFrameMoveInterval_s, (void*)GetFrameMoveInterval_s, NULL, "", bOverride);

	pClass->AddField("global", FieldType_Bool, (void*)MakeGlobal_s, (void*)IsGlobal_s, NULL, "whether object is global", bOverride);
	pClass->AddField("Sentient", FieldType_Bool, (void*)MakeSentient_s, (void*)IsSentient_s, NULL, "whether object is sentient", bOverride);
	pClass->AddField("AlwaysSentient", FieldType_Bool, (void*)SetAlwaysSentient_s, (void*)IsAlwaysSentient_s, NULL, "", bOverride);
	pClass->AddField("Sentient Radius", FieldType_Float, (void*)SetSentientRadius_s, (void*)GetSentientRadius_s, NULL, "", bOverride);
	pClass->AddField("PerceptiveRadius", FieldType_Float, (void*)SetPerceptiveRadius_s, (void*)GetPerceptiveRadius_s, NULL, "", bOverride);
	pClass->AddField("GroupID", FieldType_Int, (void*)SetGroupID_s, (void*)GetGroupID_s, NULL, "", bOverride);
	pClass->AddField("SentientField", FieldType_Int, (void*)SetSentientField_s, (void*)GetSentientField_s, NULL, "", bOverride);
	return S_OK;
}

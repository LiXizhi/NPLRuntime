//-----------------------------------------------------------------------------
// Class:	CBaseObject
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine Tech Studio
// Date:	2004.3.6
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "util/StringHelper.h"
#include "NPLHelper.h"
#include "terrain/GlobalTerrain.h"
#include "SceneObject.h"
#include "ParaWorldAsset.h"
#include "ZoneNode.h"
#include "PortalNode.h"

#include "ShapeOBB.h"
#include "ShapeAABB.h"
#include "BaseCamera.h"

#include "BaseObject.h"
#include "memdebug.h"
using namespace ParaEngine;


/** mapping from object ID to CBaseObject* */
static map<int, CBaseObject*> g_mapObjectID;

/** default selection style. */
int ParaEngine::CBaseObject::g_nObjectSelectionEffect = ParaEngine::RenderSelectionStyle_border;



//-------------------------------------------------------------------
// Class CBaseObject
//-------------------------------------------------------------------
CBaseObject::CBaseObject()
	:m_tileContainer(NULL), m_nTechniqueHandle(-1), m_objType(_undefined), m_bGeometryDirty(false), m_bEnableLOD(true),
	m_dwAttribute(0), m_pEffectParamBlock(NULL), m_nFrameNumber(0), m_nID(0), m_nSelectGroupIndex(-1), m_nRenderImportance(0), m_fRenderDistance(0.f), m_fRenderOrder(0.f), m_nMaterialId(-1)
{
}
//-----------------------------------------------------------------------------
// Name: CBaseObject::~CBaseObject
// Desc: Destructor for CBaseObject
//-----------------------------------------------------------------------------
CBaseObject::~CBaseObject()
{
	SAFE_DELETE(m_pEffectParamBlock);
	if (m_nID != 0)
	{
		map<int, CBaseObject*>::iterator iter = g_mapObjectID.find(m_nID);
		if (iter != g_mapObjectID.end())
		{
			g_mapObjectID.erase(iter);
		}
	}
}

void ParaEngine::CBaseObject::SetSelectGroupIndex(int nGroupIndex)
{
	m_nSelectGroupIndex = nGroupIndex;
}

int ParaEngine::CBaseObject::GetSelectGroupIndex()
{
	return m_nSelectGroupIndex;
}

bool ParaEngine::CBaseObject::IsTileObject()
{
	return false;
}

void ParaEngine::CBaseObject::SetTileObject(bool bIsTileObject)
{

}

int ParaEngine::CBaseObject::GetID()
{
	static int g_nLastID = 1;
	// generate on first call. 
	if (m_nID == 0)
	{
		m_nID = ++g_nLastID;
		g_mapObjectID[m_nID] = this;
	}
	return m_nID;
}

void ParaEngine::CBaseObject::SetOnAssetLoaded(const char* sCallbackScript)
{
	AddScriptCallback(Type_OnAssetLoaded, sCallbackScript);
}

CBaseObject* ParaEngine::CBaseObject::GetObjectByID(int nID)
{
	map<int, CBaseObject*>::iterator iter = g_mapObjectID.find(nID);
	if (iter != g_mapObjectID.end())
		return iter->second;
	return NULL;
}


bool CBaseObject::IsGlobal()
{
	return (m_objType >= _GlobalBiped);
}


bool CBaseObject::ActivateScript(int func_type, const string& precode, const string& postcode)
{
	ScriptCallback* pCallback = GetScriptCallback((CallBackType)func_type);
	if (pCallback)
	{
		string sFile, sCode;
		StringHelper::DevideString(pCallback->script_func, sFile, sCode, ';');
		if (!GetName().empty())
		{
			// sensor_name="[GetName()]"
			string script = "sensor_name=";
			NPL::NPLHelper::EncodeStringInQuotation(script, (int)(script.size()), GetName());
			script += ";";
			sCode = script + sCode;
		}
		{
			// sensor_id=[GetID()]
			int nID = GetID();
			char temp[32];
			ParaEngine::StringHelper::fast_itoa(nID, temp, 32);
			string script = "sensor_id=";
			script += temp;
			script += ";";
			sCode = script + sCode;
		}
		if (!precode.empty())
		{
			sCode = precode + sCode;
		}
		if (!postcode.empty())
		{
			sCode += postcode;
		}
		CGlobals::GetScene()->GetScripts().AddScript(sFile, pCallback->func_type, sCode, this);
		return true;
	}
	return false;
}
bool CBaseObject::ActivateScript(int func_type, const string& precode)
{
	return ActivateScript(func_type, precode, "");
}
bool CBaseObject::ActivateScript(int func_type)
{
	return ActivateScript(func_type, "", "");
}


void CBaseObject::SetMyType(ObjectType t)
{
	m_objType = t;
	DWORD dwVol = 0;
	switch (m_objType)
	{
	case _Tile:
	case _MeshTerrain:
	case _Floor:
		dwVol = OBJ_VOLUMN_ISOLATED | OBJ_VOLUMN_FREESPACE | OBJ_VOLUMN_CONTAINER;
		break;
	case _Scene:
		dwVol = OBJ_VOLUMN_ISOLATED | OBJ_VOLUMN_FREESPACE;
		break;
	case _House:
	case _DummyAnimMesh:// added 2004-7-28
		dwVol = OBJ_VOLUMN_CONTAINER | OBJ_VOLUMN_FREESPACE;
		break;
	case _Sky:
		dwVol = OBJ_VOLUMN_ISOLATED | OBJ_VOLUMN_FREESPACE;
		break;
	default:
		/* the following belongs to the default type.
		case _Player:
		case _Biped:
		case _NPC:
		case _Sensor:
		case _SolidHouse:	// added 2004-4-30
		case _SolidLightedHouse:	// added 2004-5-20
		*/
		dwVol = OBJ_VOLUMN_SENSOR;
		break;
	}
	m_dwAttribute &= ~VOLUMN_MASK;
	m_dwAttribute |= dwVol;
}

ObjectType CBaseObject::GetMyType()
{
	return m_objType;
};

std::string CBaseObject::ToString(DWORD nMethod)
{
	/** just a comment line with its name. */
	string sScript;
	sScript = "-- ";
	sScript.append(m_sIdentifer);
	sScript.append("\n");
	return sScript;
}

void CBaseObject::Animate(double dTimeDelta, int nRenderNumber)
{
}

void CBaseObject::AddChild(CBaseObject* pObject)
{
	if (pObject)
	{
		CBaseObject* pParent = pObject->GetParent();
		if (pParent != NULL) {
			pObject->addref();
			pParent->RemoveChild(pObject);
		}
		pObject->SetParent(this);
		m_children.push_back(pObject);

		if (pParent != NULL) {
			pObject->delref();
		}
	}
}

void CBaseObject::DestroyChildren()
{
	m_children.clear();
}

void CBaseObject::Report(vector < string >& v_sReport) {}
HRESULT CBaseObject::ResetTime() { return S_OK; }

void CBaseObject::CompressObject(CompressOption option)
{
}

void CBaseObject::SetBoundingBox(float fOBB_X, float fOBB_Y, float fOBB_Z, float fFacing)
{
}

void CBaseObject::GetBoundingBox(float* fOBB_X, float* fOBB_Y, float* fOBB_Z, float* fFacing)
{
	*fOBB_X = GetWidth();
	*fOBB_Y = GetHeight();
	*fOBB_Z = GetDepth();
	*fFacing = GetFacing();
}

void CBaseObject::SetBoundRect(float fWidth, float fHeight, float fFacing)
{
}

void CBaseObject::GetBoundRect(float* fWidth, float* fHeight, float* fFacing)
{
	*fWidth = GetWidth();
	*fHeight = GetDepth();
	*fFacing = GetFacing();
}

void CBaseObject::SetRadius(float fRadius)
{
}

float CBaseObject::GetRadius()
{
	return 0.f;
}
float CBaseObject::GetPhysicsRadius()
{
	return GetRadius();
}
float CBaseObject::GetHeight()
{
	return GetRadius() * 2.f;
}

void ParaEngine::CBaseObject::SetHeight(float fHeight)
{
}

float ParaEngine::CBaseObject::GetAssetHeight()
{
	return GetHeight();
}

float ParaEngine::CBaseObject::GetWidth()
{
	return GetRadius() * 2.f;
}

void ParaEngine::CBaseObject::SetWidth(float fWidth)
{
}

float ParaEngine::CBaseObject::GetDepth()
{
	return GetRadius() * 2.f;
}

void ParaEngine::CBaseObject::SetDepth(float fDepth)
{
}

const std::string& ParaEngine::CBaseObject::GetAssetFileName()
{
	AssetEntity* pEntity = GetPrimaryAsset();
	if (pEntity)
		return pEntity->GetKey();
	else
		return CGlobals::GetString(0);
}

void ParaEngine::CBaseObject::SetAssetFileName(const std::string& sFilename)
{

}

Vector3 CBaseObject::GetNormal()
{
	if (CGlobals::GetScene()->IsSnapToTerrain())
	{
		Vector3 vPos = GetPosition();
		CGlobals::GetGlobalTerrain()->GetNormal((float)vPos.x, vPos.z, vPos.x, vPos.y, vPos.z);
		return vPos;
	}
	return Vector3(0, 1, 0); // default is the y axis
}

void CBaseObject::SnapToTerrainSurface(bool bUseNorm)
{
	DVector3 vPos = GetPosition();

	/// set the norm of the object to the terrain surface norm.
	if (bUseNorm)
	{
		Vector3 vNorm(0, 1, 0);// default is the y axis
		CGlobals::GetGlobalTerrain()->GetNormal((float)vPos.x, (float)vPos.z, vNorm.x, vNorm.y, vNorm.z);
		SetNormal(vNorm);
	}

	/// set the position.y to the height of the terrain at that point 
	float height = CGlobals::GetGlobalTerrain()->GetElevation((float)vPos.x, (float)vPos.z);
	DVector3 v(vPos.x, height, vPos.z);
	SetPosition(v);
}


HRESULT CBaseObject::Draw(SceneState* sceneState)
{
	return S_OK;
}

float g_fLastFacing = 0;
DVector3 g_fLastPos(0, 0, 0);
void CBaseObject::PushParam()
{
	g_fLastPos = GetPosition();
	g_fLastFacing = GetFacing();
}

void CBaseObject::PopParam()
{
	SetPosition(g_fLastPos);
	SetFacing(g_fLastFacing);
}

void CBaseObject::LoadPhysics()
{

}

void CBaseObject::UnloadPhysics()
{

}

void CBaseObject::SetVisibility(bool bVisible)
{
	SetAttribute(OBJ_VOLUMN_INVISIBLE, !bVisible);
	if (!bVisible)
	{
		// unload physics
		UnloadPhysics();
	}
}

bool CBaseObject::IsVisible()
{
	return !CheckAttribute(OBJ_VOLUMN_INVISIBLE);
}

ObjectShape CBaseObject::GetObjectShape()
{
	return _ObjectShape_Box;
}

void CBaseObject::SetObjectShape(ObjectShape shape)
{
}

IGameObject* CBaseObject::QueryIGameObject()
{
	return NULL;
}

CTerrainTile* CBaseObject::GetTileContainer()
{
	return m_tileContainer;
}

void ParaEngine::CBaseObject::SetTileContainer(CTerrainTile* val)
{
	m_tileContainer = val;
}

void ParaEngine::CBaseObject::UpdateTileContainer()
{

}


int CBaseObject::GetPrimaryTechniqueHandle()
{
	return m_nTechniqueHandle;
}

void CBaseObject::SetPrimaryTechniqueHandle(int nHandle)
{
	m_nTechniqueHandle = nHandle;
}

void CBaseObject::AutoSelectTechnique()
{
	int nTech = GetPrimaryTechniqueHandle();
	if (nTech > 0)
	{
		float fPercentage = GetCtorPercentage();
		if (fPercentage < 1.0f)
		{
			if (nTech == TECH_SIMPLE_MESH_NORMAL || nTech == TECH_SIMPLE_MESH_NORMAL_VEGETATION)
				SetPrimaryTechniqueHandle(TECH_SIMPLE_MESH_NORMAL_CTOR);
		}
		else if (fPercentage >= 1.0f)
		{
			if (nTech == TECH_SIMPLE_MESH_NORMAL_CTOR)
			{
				if (IsVegetation())
					SetPrimaryTechniqueHandle(TECH_SIMPLE_MESH_NORMAL_VEGETATION);
				else
					SetPrimaryTechniqueHandle(TECH_SIMPLE_MESH_NORMAL);
			}
		}
	}
}

/** object selection technique */
// const int Object_Selection_Technique = TECH_SIMPLE_MESH_NORMAL_SELECTED;
/** object selection technique */
// const int Object_Selection_Technique = TECH_SIMPLE_MESH_NORMAL_BORDER;

void CBaseObject::OnSelect(int nGroupID)
{
	SetSelectGroupIndex(nGroupID);
	AutoSelectTechnique();

	/*int nLastTechnique = GetPrimaryTechniqueHandle();
	if(nLastTechnique == TECH_SIMPLE_MESH_NORMAL || nLastTechnique == TECH_SIMPLE_MESH_NORMAL_VEGETATION)
	{
		SetPrimaryTechniqueHandle(Object_Selection_Technique);
	}*/
}

void CBaseObject::OnDeSelect()
{
	SetSelectGroupIndex(-1);
	AutoSelectTechnique();
	//int nLastTechnique = GetPrimaryTechniqueHandle();
	//if(nLastTechnique == Object_Selection_Technique)
	//{
	//	if(IsVegetation())
	//		SetPrimaryTechniqueHandle(TECH_SIMPLE_MESH_NORMAL_VEGETATION);
	//	else
	//		SetPrimaryTechniqueHandle(TECH_SIMPLE_MESH_NORMAL);
	//}
}

CBaseObject* CBaseObject::GetChildByName(const string& name, bool bRecursive)
{
	if (!bRecursive)
	{
		for (auto pChild : m_children)
		{
			if (pChild->GetIdentifier() == name)
			{
				return pChild;
			}
		}
		return NULL;
	}
	else
	{
		for (auto pNode : m_children)
		{
			if (pNode->GetIdentifier() == name)
			{
				return pNode;
			}
			else
			{
				CBaseObject* pChild = pNode->GetChildByName(name, bRecursive);
				if (pChild != NULL)
					return pChild;
			}
		}
		return NULL;
	}
}

int CBaseObject::DestroyChildByName(const string& name, bool bRecursive)
{
	if (!bRecursive)
	{
		CChildObjectList_Type::iterator itCur, itEnd = m_children.end();
		for (itCur = m_children.begin(); itCur != itEnd; ++itCur)
		{
			CBaseObject* pNode = (*itCur);
			if (pNode->GetIdentifier() == name)
			{
				m_children.erase(itCur);
				return 1;
			}
		}
		return 0;
	}
	else
	{
		CChildObjectList_Type::iterator itCur, itEnd = m_children.end();
		for (itCur = m_children.begin(); itCur != itEnd; ++itCur)
		{
			CBaseObject* pNode = (*itCur);
			if (pNode->GetIdentifier() == name)
			{
				m_children.erase(itCur);
				return 1;
			}
			else
			{
				int nCount = pNode->DestroyChildByName(name, bRecursive);
				if (nCount > 0)
					return nCount;
			}
		}
		return 0;
	}
}

int CBaseObject::RemoveChildByName(const string& name, bool bRecursive /*= false*/)
{
	if (!bRecursive)
	{
		CChildObjectList_Type::iterator itCur, itEnd = m_children.end();
		for (itCur = m_children.begin(); itCur != itEnd; ++itCur)
		{
			CBaseObject* pNode = (*itCur);
			if (pNode->GetIdentifier() == name)
			{
				m_children.erase(itCur);
				return 1;
			}
		}
		return 0;
	}
	else
	{
		CChildObjectList_Type::iterator itCur, itEnd = m_children.end();
		for (itCur = m_children.begin(); itCur != itEnd; ++itCur)
		{
			CBaseObject* pNode = (*itCur);
			if (pNode->GetIdentifier() == name)
			{
				m_children.erase(itCur);
				return 1;
			}
			else
			{
				int nCount = pNode->RemoveChildByName(name, bRecursive);
				if (nCount > 0)
					return nCount;
			}
		}
		return 0;
	}
}

int ParaEngine::CBaseObject::RemoveChild(const CBaseObject* pObj, bool bRecursive /*= false*/)
{
	if (!bRecursive)
	{
		CChildObjectList_Type::iterator itCur, itEnd = m_children.end();
		for (itCur = m_children.begin(); itCur != itEnd; ++itCur)
		{
			CBaseObject* pNode = (*itCur);
			if (pNode == pObj)
			{
				m_children.erase(itCur);
				return 1;
			}
		}
		return 0;
	}
	else
	{
		CChildObjectList_Type::iterator itCur, itEnd = m_children.end();
		for (itCur = m_children.begin(); itCur != itEnd; ++itCur)
		{
			CBaseObject* pNode = (*itCur);
			if (pNode == pObj)
			{
				m_children.erase(itCur);
				return 1;
			}
			else
			{
				int nCount = pNode->RemoveChild(pObj, bRecursive);
				if (nCount > 0)
					return nCount;
			}
		}
		return 0;
	}
}

CParameterBlock* ParaEngine::CBaseObject::GetEffectParamBlock(bool bCreateIfNotExist /*= false*/)
{
	if (m_pEffectParamBlock)
	{
		return m_pEffectParamBlock;
	}
	else
	{
		if (bCreateIfNotExist)
			m_pEffectParamBlock = new CParameterBlock();
		return m_pEffectParamBlock;
	}
}

Matrix4* ParaEngine::CBaseObject::GetAttachmentMatrix(Matrix4& pOut, int nAttachmentID, int nRenderNumber)
{
	return NULL;
}

bool ParaEngine::CBaseObject::HasAttachmentPoint(int nAttachmentID/*=0*/)
{
	static Matrix4 mat;
	return (GetAttachmentMatrix(mat, nAttachmentID) != NULL);
}

Vector3* ParaEngine::CBaseObject::GetAttachmentPosition(Vector3& pOut, int nAttachmentID/*=0*/, int nRenderNumber)
{
	Matrix4 mat;
	if (GetAttachmentMatrix(mat, nAttachmentID, nRenderNumber))
	{
		// world translation
		Matrix4 mxWorld;
		GetRenderMatrix(mxWorld, nRenderNumber);
		mxWorld = mat * mxWorld;
		Vector3 vPos = GetRenderOffset();
		mxWorld._41 -= vPos.x;
		mxWorld._42 -= vPos.y;
		mxWorld._43 -= vPos.z;
		vPos = GetPosition();
		mxWorld._41 += vPos.x;
		mxWorld._42 += vPos.y;
		mxWorld._43 += vPos.z;
		vPos = Vector3(0, 0, 0) * mxWorld;

		// save to output
		pOut = vPos;
		return &pOut;
	}
	return NULL;
}

CZoneNode* ParaEngine::CBaseObject::GetHomeZone()
{
	RefListItem* item = GetRefObjectByTag(3);// 3 is always for reference object;
	if (item != 0)
		return (CZoneNode*)(item->m_object);
	return NULL;
}

void ParaEngine::CBaseObject::SetHomeZone(CZoneNode* pZone)
{
	if (pZone)
	{
		if (GetHomeZone() != pZone)
			pZone->addNode(this);
	}
	else
	{
		pZone = GetHomeZone();
		if (pZone)
			DeleteReference(pZone);
	}
}

void ParaEngine::CBaseObject::SetHomeZoneName(const char* sName)
{
	CZoneNode* pNode = NULL;
	if (sName && sName[0] != '\0')
		pNode = CGlobals::GetScene()->CreateGetZoneNode(sName);
	SetHomeZone(pNode);
}

const char* ParaEngine::CBaseObject::GetHomeZoneName()
{
	CZoneNode* pNode = GetHomeZone();
	if (pNode)
		return pNode->GetName().c_str();
	return CGlobals::GetString().c_str();
}

void ParaEngine::CBaseObject::UpdateFrameNumber(int nFrameNumber)
{
	SetFrameNumber(nFrameNumber);
}

bool ParaEngine::CBaseObject::CheckFrameNumber(int nFrameNumber)
{
	if (m_nFrameNumber != nFrameNumber) {
		SetFrameNumber(nFrameNumber);
		return true;
	}
	else
		return false;
}

bool ParaEngine::CBaseObject::SetReplaceableTexture(int ReplaceableTextureID, TextureEntity* pTextureEntity)
{
	return true;
}

TextureEntity* ParaEngine::CBaseObject::GetReplaceableTexture(int ReplaceableTextureID)
{
	return NULL;
}

int ParaEngine::CBaseObject::GetNumReplaceableTextures()
{
	return 0;
}

TextureEntity* ParaEngine::CBaseObject::GetDefaultReplaceableTexture(int ReplaceableTextureID)
{
	return NULL;
}

void ParaEngine::CBaseObject::EnablePhysics(bool bEnable)
{

}

void ParaEngine::CBaseObject::SetAlwaysLoadPhysics(bool bEnable)
{
	EnablePhysics(bEnable);
}

bool ParaEngine::CBaseObject::IsPhysicsEnabled()
{
	return false;
}

bool ParaEngine::CBaseObject::ViewTouch()
{
	return true;
}

void ParaEngine::CBaseObject::SetRenderOrder(float val)
{
	m_fRenderOrder = val;
}

float ParaEngine::CBaseObject::GetObjectToCameraDistance()
{
	return 0.f;
}

void ParaEngine::CBaseObject::SetObjectToCameraDistance(float val)
{

}

void ParaEngine::CBaseObject::SetRenderDistance(float fDist)
{
	m_fRenderDistance = fDist;
}

float ParaEngine::CBaseObject::GetRenderDistance()
{
	return m_fRenderDistance;
}

int ParaEngine::CBaseObject::GetSelectionEffect()
{
	return g_nObjectSelectionEffect;
}

void ParaEngine::CBaseObject::SetSelectionEffect(int nStyle)
{
	g_nObjectSelectionEffect = nStyle;
}

bool ParaEngine::CBaseObject::CanPick()
{
	return !CheckAttribute(OBJ_SKIP_PICKING); // OBJ_SKIP_PICKING | OBJ_VOLUMN_INVISIBLE 
}

bool ParaEngine::CBaseObject::CanHasPhysics()
{
	return false;
}

void ParaEngine::CBaseObject::SetSkipTerrainNormal(bool bSkip)
{
	SetAttribute(OBJ_SKIP_TERRAIN_NORMAL, bSkip);
}

bool ParaEngine::CBaseObject::IsSkipTerrainNormal()
{
	return CheckAttribute(OBJ_SKIP_TERRAIN_NORMAL);
}

void ParaEngine::CBaseObject::SetPhysicsGroupMask(DWORD dwValue)
{
}

DWORD ParaEngine::CBaseObject::GetPhysicsGroupMask()
{
	return 0xffffffff;
}

bool ParaEngine::CBaseObject::IsPersistent()
{
	return true;
}

void ParaEngine::CBaseObject::SetPersistent(bool bPersistent)
{

}

int ParaEngine::CBaseObject::GetFrameNumber()
{
	return m_nFrameNumber;
}

IViewClippingObject* ParaEngine::CBaseObject::GetViewClippingObject()
{
	if (IsGeometryDirty())
		UpdateGeometry();
	return this;
}

bool ParaEngine::CBaseObject::IsLastFrameRendered()
{
	return (GetFrameNumber() == CGlobals::GetScene()->GetFrameNumber());
}

void ParaEngine::CBaseObject::SetFrameNumber(int nFrameNumber)
{
	m_nFrameNumber = nFrameNumber;
}

void ParaEngine::CBaseObject::SetOpacity(float fOpacity)
{
	CParameterBlock* pParams = GetEffectParamBlock(true);
	if (pParams)
	{
		pParams->SetParameter("g_opacity", fOpacity);
	}
}

float ParaEngine::CBaseObject::GetOpacity()
{
	CParameterBlock* pParams = GetEffectParamBlock();
	if (pParams)
	{
		CParameter* pValue = pParams->GetParameter("g_opacity");
		if (pValue)
			return (float)(*pValue);
	}
	return 1.f;
}

IAttributeFields* ParaEngine::CBaseObject::GetChildAttributeObject(const char* sName)
{
	std::string strName = sName;
	CBaseObject* pObj = GetChildByName(strName, false);
	return (pObj) ? pObj->GetAttributeObject() : NULL;
}

void ParaEngine::CBaseObject::SetGeometryDirty(bool bDirty /*= true*/)
{
	m_bGeometryDirty = bDirty;
}

bool ParaEngine::CBaseObject::IsLODEnabled() const
{
	return m_bEnableLOD;
}

void ParaEngine::CBaseObject::EnableLOD(bool val)
{
	m_bEnableLOD = val;
}

IAttributeFields* ParaEngine::CBaseObject::GetChildAttributeObject(int nRowIndex, int nColumnIndex)
{
	if (nRowIndex < (int)m_children.size())
		return m_children.at(nRowIndex);
	return NULL;
}

IAttributeFields* ParaEngine::CBaseObject::GetAttributeObject()
{
	return (IAttributeFields*)this;
}

int ParaEngine::CBaseObject::GetChildAttributeObjectCount(int nColumnIndex)
{
	return (int)m_children.size();
}

bool ParaEngine::CBaseObject::HasAlphaBlendedObjects()
{
	return false;
}

int ParaEngine::CBaseObject::PrepareRender(CBaseCamera* pCamera, SceneState* sceneState)
{
	sceneState->GetScene()->PrepareRenderObject(this, pCamera, *sceneState);
	return 0;
}

DVector3 ParaEngine::CBaseObject::GetPosition()
{
	return DVector3(0, 0, 0);
}

void ParaEngine::CBaseObject::SetPosition(const DVector3& v)
{

}

void ParaEngine::CBaseObject::SetDead()
{

}

bool ParaEngine::CBaseObject::IsDead()
{
	return false;
}

void ParaEngine::CBaseObject::AddToDeadObjectPool()
{
	CGlobals::GetScene()->AddToDeadObjectPool(this);
}

int ParaEngine::CBaseObject::GetChildAttributeColumnCount()
{
	return 1;
}

TextureEntity* ParaEngine::CBaseObject::GetTexture()
{
	return NULL;
}

HRESULT ParaEngine::CBaseObject::InitDeviceObjects()
{
	for (CBaseObject* pChild : GetChildren())
	{
		pChild->InitDeviceObjects();
	}
	return 0;
}

HRESULT ParaEngine::CBaseObject::RestoreDeviceObjects()
{
	for (CBaseObject* pChild : GetChildren())
	{
		pChild->RestoreDeviceObjects();
	}
	return 0;
}

HRESULT ParaEngine::CBaseObject::DeleteDeviceObjects()
{
	for (CBaseObject* pChild : GetChildren())
	{
		pChild->DeleteDeviceObjects();
	}
	return 0;
}

HRESULT ParaEngine::CBaseObject::InvalidateDeviceObjects()
{
	for (CBaseObject* pChild : GetChildren())
	{
		pChild->InvalidateDeviceObjects();
	}
	return 0;
}

HRESULT ParaEngine::CBaseObject::RendererRecreated()
{
	for (CBaseObject* pChild : GetChildren())
	{
		pChild->RendererRecreated();
	}
	return 0;
}

void ParaEngine::CBaseObject::GetLocalTransform(Matrix4* localTransform)
{
	if (localTransform)
	{
		localTransform->identity();
	}
}

int ParaEngine::CBaseObject::GetMeshTriangleList(vector<Vector3>& output, int nOption)
{
	output.clear();
	auto pAsset = GetPrimaryAsset();
	if (pAsset && pAsset->IsValid())
	{
		pAsset->LoadAsset();
		if (pAsset->GetType() == AssetEntity::parax)
		{
			ParaXEntity* pParaXEntity = (ParaXEntity*)pAsset;
			CParaXModel* pModel = pParaXEntity->GetModel();
			if (pModel)
			{
				int nPass = (int)pModel->passes.size();
				auto origVertices = pModel->m_origVertices;
				auto indices = pModel->m_indices;
				for (auto& p : pModel->passes)
				{
					if (p.indexCount > 0)
					{
						int nIndexOffset = p.GetStartIndex();
						int numFaces = p.indexCount / 3;
						if (output.capacity() < (output.size() + p.indexCount))
							output.reserve(output.size() + p.indexCount);
						for (int i = 0; i < numFaces; ++i)
						{
							int nVB = 3 * i;
							for (int k = 0; k < 3; ++k)
							{
								auto a = indices[nIndexOffset + nVB + k];
								auto vert = origVertices[a];
								output.push_back(vert.pos);
							}
						}
					}
				}
			}
		}
		else if (pAsset->GetType() == AssetEntity::mesh)
		{

		}
	}
	return (int)(output.size() / 3);
}

void ParaEngine::CBaseObject::UpdateGeometry()
{
	SetGeometryDirty(false);
}

void ParaEngine::CBaseObject::SetLocalTransform(const Matrix4& mXForm)
{

}

bool ParaEngine::CBaseObject::AddChildAttributeObject(IAttributeFields* pChild, int nRowIndex /*= -1*/, int nColumnIndex /*= 0*/)
{
	if (pChild)
	{
		CBaseObject* pObject = (CBaseObject*)(pChild->QueryObject(ATTRIBUTE_CLASSID_CBaseObject));
		if (pObject)
		{
			AddChild(pObject);
			return true;
		}
	}
	return false;
}

void* ParaEngine::CBaseObject::QueryObjectByName(const std::string& sObjectType)
{
	if (sObjectType == "CBaseObject")
		return this;
	else
		return NULL;
}

void* ParaEngine::CBaseObject::QueryObject(int nObjectType)
{
	if (nObjectType == ATTRIBUTE_CLASSID_CBaseObject)
		return this;
	else
		return NULL;
}

void CBaseObject::Clone(CBaseObject* obj)
{
	if (obj != NULL)
	{
		// *obj = *this;
		memcpy((void*)obj, (void*)this, sizeof(CBaseObject));
		obj->m_refcount = 0;
	}
}

CBaseObject* CBaseObject::Clone()
{
	CBaseObject* obj = new CBaseObject();
	Clone(obj);
	return obj;
}


void CBaseObject::ApplyMaterial()
{
	if (m_nMaterialId <= 0) return;
	CBlockMaterial* material = CGlobals::GetBlockMaterialManager()->GetBlockMaterialByID(m_nMaterialId);
	CParameterBlock* paramBlock = material ? material->GetParamBlock() : nullptr;
	if (!paramBlock) return;
	EffectManager* pEffectManager = CGlobals::GetEffectManager();
	CEffectFile* pEffect = pEffectManager->GetCurrentEffectFile();
	if (!pEffect) return;
	CParameter* materialUV = paramBlock->GetParameter("MaterialUV");
	if (materialUV)
		pEffect->setParameter(CEffectFile::k_material_uv, materialUV->GetRawData(), materialUV->GetRawDataLength());
	CParameter* baseColor = paramBlock->GetParameter("BaseColor");
	if (baseColor)
		pEffect->setParameter(CEffectFile::k_material_base_color, baseColor->GetRawData(), baseColor->GetRawDataLength());
	CParameter* metallic = paramBlock->GetParameter("Metallic");
	if (metallic)
		pEffect->setParameter(CEffectFile::k_material_metallic, metallic->GetRawData(), metallic->GetRawDataLength());
	CParameter* specular = paramBlock->GetParameter("Specular");
	if (specular)
		pEffect->setParameter(CEffectFile::k_material_specular, specular->GetRawData(), specular->GetRawDataLength());
	CParameter* roughness = paramBlock->GetParameter("Roughness");
	if (roughness)
		pEffect->setParameter(CEffectFile::k_material_roughness, roughness->GetRawData(), roughness->GetRawDataLength());
	CParameter* emissiveColor = paramBlock->GetParameter("EmissiveColor");
	if (emissiveColor)
		pEffect->setParameter(CEffectFile::k_material_emissive_color, emissiveColor->GetRawData(), emissiveColor->GetRawDataLength());
	CParameter* opacity = paramBlock->GetParameter("Opacity");
	if (opacity)
		pEffect->setParameter(CEffectFile::k_material_opacity, opacity->GetRawData(), opacity->GetRawDataLength());

	RenderDevicePtr pDevice = CGlobals::GetRenderDevice();
	bool bHasDiffuseTex = false;
	CParameter* diffuse = paramBlock->GetParameter("DiffuseFullPath");
	if (diffuse)
	{
		const std::string& sFilename = diffuse->GetValueAsConstString();
		if (!sFilename.empty())
		{
			auto tex = CGlobals::GetAssetManager()->GetTexture(sFilename);
			if (tex == NULL)
				tex = CGlobals::GetAssetManager()->LoadTexture(sFilename, sFilename);
			if (tex)
			{
				bHasDiffuseTex = true;
				pDevice->SetTexture(0, tex->GetTexture());
			}
		}
	}
	if (!bHasDiffuseTex)
	{
		auto curTex = CGlobals::GetAssetManager()->GetDefaultTexture(0)->GetTexture();
		pDevice->SetTexture(0, curTex);
	}

	CParameter* normal = paramBlock->GetParameter("NormalFullPath");
	if (normal)
	{
		const std::string& sFilename = normal->GetValueAsConstString();
		if (!sFilename.empty())
		{
			auto tex = CGlobals::GetAssetManager()->GetTexture(sFilename);
			if (tex == NULL)
				tex = CGlobals::GetAssetManager()->LoadTexture(sFilename, sFilename);
			if (tex)
			{
				auto curTex = tex->GetTexture();
				pDevice->SetTexture(2, curTex);
			}
		}
	}

	CParameter* emissive = paramBlock->GetParameter("EmissiveFullPath");
	if (emissive)
	{
		const std::string& sFilename = emissive->GetValueAsConstString();
		if (!sFilename.empty())
		{
			auto tex = CGlobals::GetAssetManager()->GetTexture(sFilename);
			if (tex == NULL)
				tex = CGlobals::GetAssetManager()->LoadTexture(sFilename, sFilename);
			if (tex)
			{
				auto curTex = tex->GetTexture();
				pDevice->SetTexture(1, curTex);
			}
		}
	}
}
void CBaseObject::SetMaterialId(int materialId)
{
	m_nMaterialId = materialId;
	auto params = GetEffectParamBlock(true);
	*(params->CreateGetParameter("MaterialID")) = m_nMaterialId;
}
int CBaseObject::GetMaterialId()
{
	return m_nMaterialId;
}
int CBaseObject::InstallFields(CAttributeClass* pClass, bool bOverride)
{
	// install parent fields if there are any. Please replace __super with your parent class name.
	IViewClippingObject::InstallFields(pClass, bOverride);

	PE_ASSERT(pClass != NULL);
	pClass->AddField("id", FieldType_Int, (void*)NULL, (void*)GetID_s, NULL, NULL, bOverride);
	pClass->AddField("global", FieldType_Bool, NULL, (void*)IsGlobal_s, NULL, "whether object is global", bOverride);
	pClass->AddField("facing", FieldType_Float, (void*)SetFacing_s, (void*)GetFacing_s, CAttributeField::GetSimpleSchemaOfFloat(-3.1416f, 3.1416f), "facing in the range [-PI,PI]", bOverride);
	pClass->AddField("roll", FieldType_Float, (void*)SetRoll_s, (void*)GetRoll_s, CAttributeField::GetSimpleSchemaOfFloat(-3.1416f, 3.1416f), "in the range [-PI,PI]", bOverride);
	pClass->AddField("pitch", FieldType_Float, (void*)SetPitch_s, (void*)GetPitch_s, CAttributeField::GetSimpleSchemaOfFloat(-3.1416f, 3.1416f), "in the range [-PI,PI]", bOverride);
	pClass->AddField("yaw", FieldType_Float, (void*)SetYaw_s, (void*)GetYaw_s, CAttributeField::GetSimpleSchemaOfFloat(-3.1416f, 3.1416f), "in the range [-PI,PI]", bOverride);

	pClass->AddField("width", FieldType_Float, (void*)SetWidth_s, (void*)GetWidth_s, NULL, NULL, bOverride);
	pClass->AddField("height", FieldType_Float, (void*)SetHeight_s, (void*)GetHeight_s, NULL, NULL, bOverride);
	pClass->AddField("depth", FieldType_Float, (void*)SetDepth_s, (void*)GetDepth_s, NULL, NULL, bOverride);
	pClass->AddField("radius", FieldType_Float, (void*)SetRadius_s, (void*)GetRadius_s, NULL, NULL, bOverride);
	pClass->AddField("position", FieldType_DVector3, (void*)SetPosition_s, (void*)GetPosition_s, NULL, "global position of the character", bOverride);
	pClass->AddField("render_tech", FieldType_Int, (void*)SetTechHandle_s, (void*)GetTechHandle_s, CAttributeField::GetSimpleSchemaOfInt(0, 2048), "render technique (shader) handle in range [0,2048]", bOverride);
	pClass->AddField("progress", FieldType_Float, (void*)SetCtorPercentage_s, (void*)GetCtorPercentage_s, NULL, "how much the object is being constructed in percentage range [0,1]", bOverride);
	pClass->AddField("assetfile", FieldType_String, (void*)SetAssetFileName_s, (void*)GetAssetFileName_s, NULL, NULL, bOverride);
	pClass->AddField("homezone", FieldType_String, (void*)SetHomeZone_s, (void*)GetHomeZone_s, NULL, NULL, bOverride);
	pClass->AddField("showboundingbox", FieldType_Bool, (void*)SetShowBoundingBox_s, (void*)GetShowBoundingBox_s, NULL, NULL, bOverride);
	pClass->AddField("PhysicsGroup", FieldType_Int, (void*)SetPhysicsGroup_s, (void*)GetPhysicsGroup_s, NULL, NULL, bOverride);
	pClass->AddField("PhysicsGroupMask", FieldType_DWORD, (void*)SetPhysicsGroupMask_s, (void*)GetPhysicsGroupMask_s, NULL, NULL, bOverride);
	pClass->AddField("EnablePhysics", FieldType_Bool, (void*)EnablePhysics_s, (void*)IsPhysicsEnabled_s, NULL, "", bOverride);
	pClass->AddField("EnableDynamicPhysics", FieldType_Bool, (void*)EnableDynamicPhysics_s, (void*)IsDynamicPhysicsEnabled_s, NULL, "", bOverride);
	pClass->AddField("PhysicsShape", FieldType_String, (void*)SetPhysicsShape_s, (void*)GetPhysicsShape_s, NULL, "", bOverride);
	pClass->AddField("PhysicalProperty", FieldType_String, (void*)SetPhysicsProperty_s, (void*)GetPhysicsProperty_s, NULL, "", bOverride);
	pClass->AddField("ApplyCentralImpulse", FieldType_Vector3, (void*)ApplyCentralImpulse_s, NULL, NULL, "", bOverride);
	pClass->AddField("SelectGroupIndex", FieldType_Int, (void*)SetSelectGroupIndex_s, (void*)GetSelectGroupIndex_s, NULL, NULL, bOverride);
	pClass->AddField("On_AssetLoaded", FieldType_String, (void*)SetOnAssetLoaded_s, (void*)GetOnAssetLoaded_s, NULL, NULL, bOverride);
	pClass->AddField("ViewTouch", FieldType_Bool, (void*)NULL, (void*)GetViewTouch_s, NULL, "", bOverride);
	pClass->AddField("UpdateGeometry", FieldType_void, (void*)UpdateGeometry_s, NULL, NULL, "", bOverride);
	pClass->AddField("RenderOrder", FieldType_Float, (void*)SetRenderOrder_s, (void*)GetRenderOrder_s, NULL, "", bOverride);
	pClass->AddField("ObjectToCameraDistance", FieldType_Float, (void*)SetObjectToCameraDistance_s, (void*)GetObjectToCameraDistance_s, NULL, "", bOverride);
	pClass->AddField("RenderImportance", FieldType_Int, (void*)SetRenderImportance_s, (void*)GetRenderImportance_s, NULL, "", bOverride);
	pClass->AddField("RenderDistance", FieldType_Float, (void*)SetRenderDistance_s, (void*)GetRenderDistance_s, NULL, "", bOverride);
	pClass->AddField("reset", FieldType_void, (void*)Reset_s, NULL, NULL, "reset object", bOverride);
	pClass->AddField("ChildCount", FieldType_Int, NULL, (void*)GetChildCount_s, NULL, "", bOverride);
	pClass->AddField("normal", FieldType_Vector3, (void*)SetNormal_s, (void*)GetNormal_s, NULL, "global position of the character", bOverride);
	pClass->AddField("transparent", FieldType_Bool, (void*)SetTransparent_s, (void*)IsTransparent_s, NULL, NULL, bOverride);
	pClass->AddField("visible", FieldType_Bool, (void*)SetVisibility_s, (void*)IsVisible_s, NULL, NULL, bOverride);
	pClass->AddField("ShadowCaster", FieldType_Bool, (void*)SetShadowCaster_s, (void*)IsShadowCaster_s, NULL, NULL, bOverride);
	pClass->AddField("ShadowReceiver", FieldType_Bool, (void*)SetShadowReceiver_s, (void*)IsShadowReceiver_s, NULL, NULL, bOverride);
	pClass->AddField("billboarded", FieldType_Bool, (void*)SetBillboarded_s, (void*)IsBillboarded_s, NULL, NULL, bOverride);
	pClass->AddField("SkipRender", FieldType_Bool, (void*)SetSkipRender_s, (void*)IsSkipRender_s, NULL, "", bOverride);
	pClass->AddField("SkipPicking", FieldType_Bool, (void*)SetSkipPicking_s, (void*)IsSkipPicking_s, NULL, "", bOverride);
	pClass->AddField("SkipTerrainNormal", FieldType_Bool, (void*)SetSkipTerrainNormal_s, (void*)IsSkipTerrainNormal_s, NULL, "", bOverride);
	pClass->AddField("SelectionEffect", FieldType_Int, (void*)SetSelectionEffect_s, (void*)GetSelectionEffect_s, NULL, "", bOverride);
	pClass->AddField("persistent", FieldType_Bool, (void*)SetPersistent_s, (void*)IsPersistent_s, NULL, "whether object is persistent", bOverride);
	pClass->AddField("IsTileObject", FieldType_Bool, (void*)SetTileObject_s, (void*)IsTileObject_s, NULL, "whether object is persistent", bOverride);

	pClass->AddField("AnimID", FieldType_Int, (void*)SetAnimation_s, (void*)GetAnimation_s, NULL, "", bOverride);
	pClass->AddField("UpperAnimID", FieldType_Int, (void*)SetUpperAnimation_s, (void*)GetUpperAnimation_s, NULL, "", bOverride);
	pClass->AddField("AnimFrame", FieldType_Int, (void*)SetAnimFrame_s, (void*)GetAnimFrame_s, NULL, "", bOverride);
	pClass->AddField("EnableAnim", FieldType_Bool, (void*)EnableAnim_s, (void*)IsAnimEnabled_s, NULL, "", bOverride);
	pClass->AddField("UseGlobalTime", FieldType_Bool, (void*)SetUseGlobalTime_s, (void*)IsUseGlobalTime_s, NULL, "", bOverride);

	pClass->AddField("HeadOnZEnabled", FieldType_Bool, (void*)SetHeadOnZEnabled_s, (void*)IsHeadOnZEnabled_s, NULL, "", bOverride);
	pClass->AddField("HeadOnSolid", FieldType_Bool, (void*)SetHeadOnSolid_s, (void*)IsHeadOnSolid_s, NULL, "", bOverride);
	pClass->AddField("HeadOn3DScalingEnabled", FieldType_Bool, (void*)SetHeadOn3DScalingEnabled_s, (void*)IsHeadOn3DScalingEnabled_s, NULL, "", bOverride);
	pClass->AddField("HeadOnUseGlobal3DScaling", FieldType_Bool, (void*)SetHeadOnUseGlobal3DScaling_s, (void*)IsHeadOnUseGlobal3DScaling_s, NULL, "", bOverride);
	pClass->AddField("HeadOnNearZoomDist", FieldType_Float, (void*)SetHeadOnNearZoomDist_s, (void*)GetHeadOnNearZoomDist_s, NULL, "", bOverride);
	pClass->AddField("HeadOnFarZoomDist", FieldType_Float, (void*)SetHeadOnFarZoomDist_s, (void*)GetHeadOnFarZoomDist_s, NULL, "", bOverride);
	pClass->AddField("HeadOnMinUIScaling", FieldType_Float, (void*)SetHeadOnMinUIScaling_s, (void*)GetHeadOnMinUIScaling_s, NULL, "", bOverride);
	pClass->AddField("HeadOnMaxUIScaling", FieldType_Float, (void*)SetHeadOnMaxUIScaling_s, (void*)GetHeadOnMaxUIScaling_s, NULL, "", bOverride);
	pClass->AddField("HeadOnAlphaFadePercentage", FieldType_Float, (void*)SetHeadOnAlphaFadePercentage_s, (void*)GetHeadOnAlphaFadePercentage_s, NULL, "", bOverride);
	pClass->AddField("HeadOn3DFacing", FieldType_Float, (void*)SetHeadOn3DFacing_s, (void*)GetHeadOn3DFacing_s, NULL, "", bOverride);
	pClass->AddField("HeadOn3DUIScaling", FieldType_Float, (void*)SetHeadOnMaxUIScaling_s, (void*)GetHeadOnMaxUIScaling_s, NULL, "", bOverride);
	pClass->AddField("FrameNumber", FieldType_Int, (void*)SetFrameNumber_s, (void*)GetFrameNumber_s, NULL, "", bOverride);
	pClass->AddField("IsLastFrameRendered", FieldType_Bool, NULL, (void*)IsLastFrameRendered_s, NULL, "", bOverride);
	pClass->AddField("IsDead", FieldType_Bool, (void*)SetDead_s, (void*)IsDead_s, NULL, "", bOverride);
	pClass->AddField("opacity", FieldType_Float, (void*)SetOpacity_s, (void*)GetOpacity_s, NULL, "", bOverride);
	pClass->AddField("DestroyChildren", FieldType_void, (void*)DestroyChildren_s, (void*)0, NULL, "", bOverride);
	pClass->AddField("RenderWorldMatrix", FieldType_Matrix4, (void*)0, (void*)GetRenderMatrix_s, NULL, "", bOverride);
	pClass->AddField("LocalTransform", FieldType_Matrix4, (void*)SetLocalTransform_s, (void*)GetLocalTransform_s, NULL, "", bOverride);
	pClass->AddField("IsLodEnabled", FieldType_Bool, (void*)EnableLOD_s, (void*)IsLODEnabled_s, NULL, "", bOverride);
	pClass->AddField("MaterialID", FieldType_Int, (void*)SetMaterialId_s, (void*)GetMaterialId_s, NULL, "", bOverride);
	return S_OK;
}


//-----------------------------------------------------------------------------
// Class:	
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2004.4
// Revised: 2005.4
//-----------------------------------------------------------------------------

#include "ParaEngine.h"
#ifdef USE_DIRECTX_RENDERER
#include "SpriteObject.h"
#include "VoxelMesh.h"
#include "LightObject.h"
#include "DynamicObject.h"
#include "CadModel/CadModel.h"
#include "CadModel/CadModelNode.h"
#endif
#include "ParaWorldAsset.h"
#include "ViewportManager.h"
#include "AttributesManager.h"
#include "CanvasCamera.h"
#include "IParaEngineApp.h"
#include "TerrainTile.h"
#include "AutoCamera.h"
#include "SceneObject.h"
#include "BipedObject.h"
#include "SceneObjectPicking.h"
#include "MeshObject.h"
#include "MissileObject.h"
#include "SphereObject.h"
#include "SkyMesh.h"
#include "MeshPhysicsObject.h"
#include "IEnvironmentSim.h"
#include "IGameObject.h"
#include "2dengine/GUIRoot.h"
#include "OceanManager.h"
#include "EventsCenter.h"
#include "terrain/GlobalTerrain.h"
#include "TerrainTileRoot.h"
#include "ShapeAABB.h"
#include "ShapeOBB.h"
#include "MiniSceneGraph.h"
#include "ParaXModel/BoneAnimProvider.h"
#include "CustomCharSettings.h"
#include "ParaScriptingScene.h"
#include "ZoneNode.h"
#include "SunLight.h"
#include "PortalNode.h"
#include "SelectionManager.h"
#include "ParaXAnimInstance.h"
#include "ParaXModel/GltfModel.h"
#include <time.h>

extern "C"
{
#include "lua.h"
}
#include <luabind/luabind.hpp>
#include <luabind/object.hpp>
using namespace luabind;

#include "memdebug.h"
/** @def define this macro to enable RPG character, otherwise Biped Object will be used */
#define USE_RPG_CHARACTER

/** @def define to delete object from the database once deleted from the scene. */
#define DELETE_FROM_DB
namespace ParaEngine
{
	extern int my_rand();
}

namespace ParaScripting
{
	/*
	* @param sFnctFilter: it can be any of the following string.
	* "mesh": mesh any mesh object in the scene. Usually for selection during scene editing.
	* "cmesh": mesh object that is clickable (associated with scripts). Usually for game playing.
	* "notplayer": any object in the scene except for the current player. Usually for selection during scene editing.
	* "": any object in the scene except. Usually for selection during scene editing.
	* "light": only pick light objects
	* "biped": any character objects :local or global.
	* "anyobject": any objects, including mesh and characters. but not including helper objects, such as light.
	* "global": all global objects, such as global character and mesh. This is usually for game mode.
	* "actionmesh": mesh with action script. 
	* @return: the total number of selected objects is returned. 
	*/
	OBJECT_FILTER_CALLBACK GetFilterFuncByName(const char* sFilterFunc) 
	{
		OBJECT_FILTER_CALLBACK pFilterFunc = g_fncPickingAll;
		if(sFilterFunc == NULL)
		{
			pFilterFunc = g_fncPickingAll;
		}
		else
		{
			if(strcmp(sFilterFunc, "notplayer")==0)
				pFilterFunc = g_fncPickingNotPlayer;
			else if(strcmp(sFilterFunc, "anyobject")==0)
				pFilterFunc = g_fncPickingAnyObject;
			else if(strcmp(sFilterFunc, "light")==0)
				pFilterFunc = g_fncPickingLight;
			else if(strcmp(sFilterFunc, "actionmesh")==0)
				pFilterFunc = g_fncPickingActionMesh;
			else if(strcmp(sFilterFunc, "global")==0)
				pFilterFunc = g_fncPickingGlobal;
			else if(strcmp(sFilterFunc, "")==0)
				pFilterFunc = g_fncPickingAll;
			else if(strcmp(sFilterFunc, "mesh")==0)
				pFilterFunc = g_fncPickingMesh;
			else if(strcmp(sFilterFunc, "biped")==0)
				pFilterFunc = g_fncPickingBiped;
			else if(strcmp(sFilterFunc, "cmesh")==0)
				pFilterFunc = g_fncPickingNPCMesh;
			else if(sFilterFunc[0]>='0' && sFilterFunc[0]<='9')
			{
				// filter by type. 
				DWORD filter = (DWORD)atoll(sFilterFunc);
				// if(filter!=0)
				{
					SetPickingFilter(filter);
					pFilterFunc = g_fncPickingByObjectTypes;
				}
			}
			else if(sFilterFunc[0] == 'p' && sFilterFunc[1] == ':' && sFilterFunc[2]>='0' && sFilterFunc[2]<='9')
			{
				// filter by physics group.  e.g. "p:4294967295" matches all physics group
				DWORD filter = (DWORD)atoll(sFilterFunc+2);
				{
					SetPickingPhysicsFilter(filter);
					pFilterFunc = g_fncPickingByPhysicsGroup;
				}
			}
			else if(strcmp(sFilterFunc, "point")==0)
			{
				// TODO
			}
		}
		return pFilterFunc;
	}

	/** get the physics group mask by filter name */
	DWORD GetPhysicsGroupMaskByName(const char* sFilterFunc) 
	{
		if(sFilterFunc)
		{
			if(strcmp(sFilterFunc, "walkpoint")==0)
			{
				return 0xfffffff2; // tricky: we will ignore group 0 and 2. 
			}
			else if(strcmp(sFilterFunc, "terrain")==0)
			{
				return 0x0;
			}
		}
		return CGlobals::GetScene()->GetPhysicsGroupMask();
	}


//--------------------------------------------------------------
// for ParaObject object
//--------------------------------------------------------------
ParaObject::ParaObject(){};
ParaObject::ParaObject(CBaseObject* pObj):m_pObj(pObj){};
ParaObject::~ParaObject(){}

bool ParaObject::IsValid() const {return m_pObj;}
bool ParaObject::equals(const ParaObject obj) const
{
	return m_pObj == obj.m_pObj;
}

const char* ParaObject::ToString() const
{
	return ToString1("create");
}

const char* ParaObject::ToString1(const char* sMethod) const
{
	static string sScript_;
	if(IsValid())
	{
		if(strcmp("create", sMethod) == 0)
			sScript_ = m_pObj->ToString(CBaseObject::NPL_CREATE);
		else if(strcmp("loader", sMethod) == 0)
			sScript_ = m_pObj->ToString(CBaseObject::NPL_CREATE_IN_LOADER);
		else if(strcmp("delete", sMethod) == 0)
			sScript_ = m_pObj->ToString(CBaseObject::NPL_DELETE);
		else if(strcmp("update", sMethod) == 0)
			sScript_ = m_pObj->ToString(CBaseObject::NPL_UPDATE);
	}
	else
		sScript_="";
	return sScript_.c_str();
}
const char* ParaObject::GetType()
{
	if(!IsValid())
		return CGlobals::GetString(0).c_str();
	return m_pObj->GetAttributeClassName();
}

int ParaObject::GetMyType() const
{
	if(!IsValid())
		return 0;
	return (int)(m_pObj->GetMyType());
}

int ParaObject::GetID()
{
	if(!IsValid())
		return 0;
	return m_pObj->GetID();
}

ParaAssetObject ParaObject::GetPrimaryAsset()
{
	return ParaAssetObject(IsValid()? m_pObj->GetPrimaryAsset() : NULL);
}
void ParaObject::GetPrimaryAsset_(ParaAssetObject* pOut)
{
	*pOut = GetPrimaryAsset();
}

ParaCharacter  ParaObject::ToCharacter()
{
	return ParaCharacter(m_pObj);
}
float ParaObject::DistanceTo(ParaObject obj)
{
	if(IsValid() && obj.IsValid())
	{
		DVector3 v1 = obj.m_pObj->GetPosition();
		DVector3 v2 = m_pObj->GetPosition();
		float fDist = (float)((v1-v2).squaredLength());
		if(fDist>0.001f)
			fDist = sqrtf(fDist);
		return fDist;
	}
	else
		return 0;
}
float ParaObject::DistanceToSq(ParaObject obj)
{
	if(IsValid() && obj.IsValid())
	{
		DVector3 v1 = obj.m_pObj->GetPosition();
		DVector3 v2 = m_pObj->GetPosition();
		return (float)((v1-v2).squaredLength());
	}
	else
		return 0;
}

float ParaObject::DistanceToPlayerSq()
{
	if(IsValid())
	{
		CBipedObject* pPlayer = CGlobals::GetScene()->GetCurrentPlayer();
		if(pPlayer!=0)
		{
			DVector3 v1 = pPlayer->GetPosition();
			DVector3 v2 = m_pObj->GetPosition();
			return (float)((v1-v2).squaredLength());
		}
	}
	return 0;
}

float ParaObject::DistanceToCameraSq()
{
	if(IsValid())
	{
		CBaseCamera* pCamera = CGlobals::GetScene()->GetCurrentCamera();
		if(pCamera!=0)
		{
			DVector3 v1 = pCamera->GetEyePosition();
			DVector3 v2 = m_pObj->GetPosition();
			return (float)((v1-v2).squaredLength());
		}
	}
	return 0;
}

IGameObject* ParaObject::ToGameObject()
{
	if(IsValid())
	{
		return m_pObj->QueryIGameObject();
	}
	return NULL;
}

void ParaObject::AddEvent(const char* strEvent, int nEventType, bool bIsUnique)
{
	if (IsValid())
	{
		m_pObj->ProcessObjectEvent(ObjectEvent(strEvent, nEventType));
	}
}

bool ParaObject::IsAttached() const
{
	if(IsValid())
	{
		return (m_pObj->GetTileContainer()!=NULL);
	}
	return false;
}

ParaAttributeObject ParaObject::GetAttributeObject()
{
	return ParaAttributeObject(m_pObj);
}

void ParaObject::GetAttributeObject_(ParaAttributeObject& output)
{
	output = GetAttributeObject();
}

void ParaObject::CheckLoadPhysics()
{
	if(IsValid())
	{
		DVector3 vPos = m_pObj->GetPosition();
		CGlobals::GetEnvSim()->CheckLoadPhysics(vPos, m_pObj->GetPhysicsRadius()*2.f);
	}
}

void ParaObject::LoadPhysics()
{
	if(IsValid())
	{
		m_pObj->SetAlwaysLoadPhysics(true);
		m_pObj->LoadPhysics();
	}
}

void ParaObject::SetPosition(double x, double y, double z)
{
	if(!IsValid())
		return;
	DVector3 v(x, y, z);
	if(m_pObj->GetMyType() != _LocalLight)
		m_pObj->SetPosition(v);
	else
		m_pObj->SetObjectCenter(v);

	/*
	* The Y base coordinate acts as a 'zero point' for the height values in the upcoming height map - that is, 
	*		all height values are added to this Y height to form the final y component of the object's position.
	*/
	//m_pObj->SetPosition(&Vector3(x,y+CGlobals::GetGlobalTerrain()->GetElevation(x, z),z));

	//{
	//	/** since the object has moved, we may need to update its location in the scene graph 
	//	* we only do this for static object.
	//	*/
	//}
}

void ParaObject::GetPosition(double *x, double *y, double *z)
{
	if(!IsValid())
		return;
	DVector3 v;
	if(m_pObj->GetMyType() != _LocalLight)
		v = m_pObj->GetPosition();
	else
		v = m_pObj->GetObjectCenter();

	//x = v.x;y = v.y-CGlobals::GetGlobalTerrain()->GetElevation(v.x, v.z);z = v.z;
	*x = v.x;*y = v.y;*z = v.z;
}

void ParaObject::GetViewCenter(double *x, double *y, double *z)
{
	if(!IsValid())
		return;
	IViewClippingObject* pViewObj =  m_pObj->GetViewClippingObject();
	if(pViewObj!=0)
	{
		DVector3 v = pViewObj->GetObjectCenter();
		*x = v.x;*y = v.y;*z = v.z;
	}
}

float ParaObject::GetPhysicsRadius()
{
	if(IsCharacter())
	{
		return ((CBipedObject*)m_pObj)->GetPhysicsRadius();
	}
	else if(m_pObj)
	{
		return m_pObj->GetRadius();
	}
	return 0.f;
}

void ParaObject::SetPhysicsRadius(float fR)
{
	if(IsCharacter())
	{
		((CBipedObject*)m_pObj)->SetPhysicsRadius(fR);
	}
	else if(m_pObj)
	{
		m_pObj->SetRadius(fR);
	}
}

float ParaObject::GetPhysicsHeight()
{
	if(IsCharacter())
	{
		return ((CBipedObject*)m_pObj)->GetPhysicsHeight();
	}
	return 0.f;
}

void ParaObject::SetPhysicsHeight(float fHeight)
{
	if(IsCharacter())
	{
		((CBipedObject*)m_pObj)->SetPhysicsHeight(fHeight);
	}
}

void ParaObject::SetDensity(float fDensity)
{
	if(IsValid() && m_pObj->IsBiped())
	{
		((CBipedObject*)m_pObj)->SetDensity(fDensity);
	}
}

float ParaObject::GetDensity()
{
	if(IsValid() && m_pObj->IsBiped())
	{
		return ((CBipedObject*)m_pObj)->GetDensity();
	}
	return 1.0f;
}

void ParaObject::Rotate(float x, float y, float z)
{
	if(!IsValid())
		return;
	m_pObj->Rotate(x,y,z);
}
void ParaObject::Reset()
{
	if(IsValid())
	{
		// currently, I used scaling over the original scale
		m_pObj->Reset();
	}
}

void ParaObject::Scale(float s)
{
	if(IsValid())
	{
		// currently, I used scaling over the original scale. so the scripting interface has different meaning from the game engine interface. 
		float oldscale = m_pObj->GetScaling();
		m_pObj->SetScaling(oldscale*s);
	}
}
void ParaObject::SetScaling(float s)
{
	Scale(s);
}
void ParaObject::OffsetPosition (float dx, float dy, float dz)
{
	double x,y,z;
	GetPosition(&x,&y,&z);
	x+=dx;y+=dy;z+=dz;
	SetPosition(x,y,z);
}

float ParaObject::GetScale()
{
	if(IsValid())
	{
		return m_pObj->GetScaling();
	}
	return 1.f;
}

void ParaObject::SetScale( float s )
{
	if(IsValid())
	{
		return m_pObj->SetScaling(s);
	}
}

object ParaObject::GetRotation( const object& quat )
{
	if(type(quat) == LUA_TTABLE && IsValid())
	{
		Quaternion q(0,0,0,1);
		m_pObj->GetRotation(&q);
		quat["x"] = q.x;
		quat["y"] = q.y;
		quat["z"] = q.z;
		quat["w"] = q.w;
	}
	return object(quat);
}

void ParaObject::SetRotation( const object& quat )
{
	if(type(quat) == LUA_TTABLE && IsValid())
	{
		Quaternion q(0,0,0,1);
		const object& oX = quat["x"];
		if(type(oX) == LUA_TNUMBER){
			q.x = object_cast<float>(oX);
		}
		const object& oY = quat["y"];
		if(type(oY) == LUA_TNUMBER){
			q.y = object_cast<float>(oY);
		}
		const object& oZ = quat["z"];
		if(type(oZ) == LUA_TNUMBER){
			q.z = object_cast<float>(oZ);
		}
		const object& oW = quat["w"];
		if(type(oW) == LUA_TNUMBER){
			q.w = object_cast<float>(oW);
		}
		m_pObj->SetRotation(q);
	}
}

/** set object facing around the Y axis.
* this function is safe to call for all kind of objects except the physics mesh object. 
* for physics mesh object, one must call ParaScene.Attach() immediately after this function.
* for more information, please see SetPostion();
* @see: SetPostion();
*/
void ParaObject::SetFacing(float fFacing) {
	if(IsValid())
		m_pObj->SetFacing(fFacing);
}
/** get object facing around the Y axis	*/
float ParaObject::GetFacing() {
	if(IsValid())
		return m_pObj->GetFacing();
	return 0;
}
string ParaObject::GetName() const{
	if(IsValid())
		return m_pObj->GetIdentifier();
	return CGlobals::GetString();
}
const char* ParaObject::GetName_() const
{
	static string g_str;
	g_str = GetName();
	return g_str.c_str();
}

void ParaObject::SetName(const char* sName)
{
	if(IsValid() && sName!=0)
	{
		if(m_pObj->GetIdentifier() != sName)
		{
			// can not change the name of a certain object. 
			if( (m_pObj == CGlobals::GetScene()->GetCurrentPlayer()) || (m_pObj->GetMyType() == _OPC))
				return;

			// can not change the name if there is already an global object with the new name or the name is blank
			if( m_pObj->IsGlobal() && 
				((sName[0]=='\0') || (CGlobals::GetScene()->GetGlobalObject(sName) !=NULL)) )
				return;

			if(m_pObj->GetTileContainer() == NULL)
			{
				// if the object has never been attached before, just rename it anyway.
				m_pObj->SetIdentifier(sName);
			}
			else
			{
				// change the object name, by first detach from the scene and then reattach it to the scene.
				CGlobals::GetScene()->DetachObject(m_pObj);
				m_pObj->SetIdentifier(sName);
				CGlobals::GetScene()->AttachObject(m_pObj);
			}
		}
	}
}
void ParaObject::SnapToTerrainSurface(int bUseNorm){
	if(IsValid())
		m_pObj->SnapToTerrainSurface(bUseNorm>0);
}
/**
* @return return true if object is a character(biped) object
*/
bool ParaObject::IsCharacter() const{
	if(IsValid())
		return (m_pObj->IsBiped());
	return false;
}
bool ParaObject::IsOPC() const
{
	if(IsValid())
	{
		return (m_pObj->IsBiped()) && (m_pObj->GetMyType()==_OPC);
	}
	return false;
}

void ParaObject::AddChild(const ParaObject obj){
	if(IsValid() && obj.IsValid())
		m_pObj->AddChild(obj.get());
};
void ParaObject::EnablePhysics(bool bEnable)
{
	if(IsValid())
	{
		m_pObj->EnablePhysics(bEnable);
	}
}
bool ParaObject::IsPhysicsEnabled()
{
	if(IsValid())
	{
		return m_pObj->IsPhysicsEnabled();
	}
	return false;
}

string ParaObject::GetOnEnterSentientArea() const
{
	IGameObject* pGameObj = NULL;
	if(IsValid() && ((pGameObj = m_pObj->QueryIGameObject()) !=NULL))
	{
		IGameObject::ScriptCallback * pCallBack = pGameObj->GetScriptCallback(IGameObject::Type_EnterSentientArea);
		if(pCallBack!=NULL)
			return pCallBack->GetScriptFunc();
		else
			return "";
	}
	else
		return "";
}
void ParaObject::SetOnEnterSentientArea(const char* script)
{
	IGameObject* pGameObj = NULL;
	if(IsValid() && ((pGameObj = m_pObj->QueryIGameObject()) !=NULL))
	{
		pGameObj->AddScriptCallback(IGameObject::Type_EnterSentientArea, script);
	}
}

string ParaObject::GetOnLeaveSentientArea() const
{
	IGameObject* pGameObj = NULL;
	if(IsValid() && ((pGameObj = m_pObj->QueryIGameObject()) !=NULL))
	{
		IGameObject::ScriptCallback * pCallBack = pGameObj->GetScriptCallback(IGameObject::Type_LeaveSentientArea);
		if(pCallBack!=NULL)
			return pCallBack->GetScriptFunc();
		else
			return "";
	}
	else
		return "";
}
void ParaObject::SetOnLeaveSentientArea(const char* script)
{
	IGameObject* pGameObj = NULL;
	if(IsValid() && ((pGameObj = m_pObj->QueryIGameObject()) !=NULL))
	{
		pGameObj->AddScriptCallback(IGameObject::Type_LeaveSentientArea, script);
	}
}
void ParaObject::On_Click(DWORD nMouseKey, DWORD dwParam1,  DWORD dwParam2)
{
	IGameObject* pGameObj = NULL;
	if(IsValid() && ((pGameObj = m_pObj->QueryIGameObject()) !=NULL))
	{
		pGameObj->On_Click(nMouseKey, dwParam1, dwParam2);
	}
}
string ParaObject::GetOnClick() const
{
	IGameObject* pGameObj = NULL;
	if(IsValid() && ((pGameObj = m_pObj->QueryIGameObject()) !=NULL))
	{
		IGameObject::ScriptCallback * pCallBack = pGameObj->GetScriptCallback(IGameObject::Type_Click);
		if(pCallBack!=NULL)
			return pCallBack->GetScriptFunc();
		else
			return "";
	}
	else
		return "";
}
void ParaObject::SetOnClick(const char* script)
{
	IGameObject* pGameObj = NULL;
	if(IsValid() && ((pGameObj = m_pObj->QueryIGameObject()) !=NULL))
	{
		pGameObj->AddScriptCallback(IGameObject::Type_Click, script);
	}
}

string ParaObject::GetOnPerceived() const
{
	IGameObject* pGameObj = NULL;
	if(IsValid() && ((pGameObj = m_pObj->QueryIGameObject()) !=NULL))
	{
		IGameObject::ScriptCallback * pCallBack = pGameObj->GetScriptCallback(IGameObject::Type_Perception);
		if(pCallBack!=NULL)
			return pCallBack->GetScriptFunc();
		else
			return "";
	}
	else
		return "";
}
void ParaObject::SetOnPerceived(const char* script)
{
	IGameObject* pGameObj = NULL;
	if(IsValid() && ((pGameObj = m_pObj->QueryIGameObject()) !=NULL))
	{
		pGameObj->AddScriptCallback(IGameObject::Type_Perception, script);
	}
}

string ParaObject::GetOnFrameMove() const
{
	IGameObject* pGameObj = NULL;
	if(IsValid() && ((pGameObj = m_pObj->QueryIGameObject()) !=NULL))
	{
		IGameObject::ScriptCallback * pCallBack = pGameObj->GetScriptCallback(IGameObject::Type_FrameMove);
		if(pCallBack!=NULL)
			return pCallBack->GetScriptFunc();
		else
			return "";
	}
	else
		return "";
}
void ParaObject::SetOnFrameMove(const char* script)
{
	IGameObject* pGameObj = NULL;
	if(IsValid() && ((pGameObj = m_pObj->QueryIGameObject()) !=NULL))
	{
		pGameObj->AddScriptCallback(IGameObject::Type_FrameMove, script);
	}
}

string ParaObject::GetOnNetSend() const
{
	IGameObject* pGameObj = NULL;
	if(IsValid() && ((pGameObj = m_pObj->QueryIGameObject()) !=NULL))
	{
		IGameObject::ScriptCallback * pCallBack = pGameObj->GetScriptCallback(IGameObject::Type_Net_Send);
		if(pCallBack!=NULL)
			return pCallBack->GetScriptFunc();
		else
			return "";
	}
	else
		return "";
}
void ParaObject::SetOnNetSend(const char* script)
{
	IGameObject* pGameObj = NULL;
	if(IsValid() && ((pGameObj = m_pObj->QueryIGameObject()) !=NULL))
	{
		pGameObj->AddScriptCallback(IGameObject::Type_Net_Send, script);
	}
}

string ParaObject::GetOnNetReceive() const
{
	IGameObject* pGameObj = NULL;
	if(IsValid() && ((pGameObj = m_pObj->QueryIGameObject()) !=NULL))
	{
		IGameObject::ScriptCallback * pCallBack = pGameObj->GetScriptCallback(IGameObject::Type_Net_Receive);
		if(pCallBack!=NULL)
			return pCallBack->GetScriptFunc();
		else
			return "";
	}
	else
		return "";
}
void ParaObject::SetOnNetReceive(const char* script)
{
	IGameObject* pGameObj = NULL;
	if(IsValid() && ((pGameObj = m_pObj->QueryIGameObject()) !=NULL))
	{
		pGameObj->AddScriptCallback(IGameObject::Type_Net_Receive, script);
	}
}

bool ParaObject::IsSentient()
{
	IGameObject* pObj = ToGameObject();
	if(pObj!=NULL)
	{
		return pObj->IsSentient();
	}
	return false;
}

float ParaObject::GetSentientRadius()
{
	IGameObject* pObj = ToGameObject();
	if(pObj!=NULL)
	{
		return pObj->GetSentientRadius();
	}
	return 0.f;
}

float ParaObject::GetPerceptiveRadius()
{
	IGameObject* pObj = ToGameObject();
	if(pObj!=NULL)
	{
		return pObj->GetPerceptiveRadius();
	}
	return 0.f;
}

void ParaObject::SetPerceptiveRadius(float fNewRaduis)
{
	IGameObject* pObj = ToGameObject();
	if(pObj!=NULL)
	{
		pObj->SetPerceptiveRadius(fNewRaduis);
	}
}


int ParaObject::GetNumOfPerceivedObject()
{
	IGameObject* pObj = ToGameObject();
	if(pObj!=NULL)
	{
		return pObj->GetNumOfPerceivedObject();
	}
	return 0;
}

ParaObject ParaObject::GetPerceivedObject(int nIndex)
{
	IGameObject* pObj = ToGameObject();
	if(pObj!=NULL)
	{
		return ParaObject(pObj->GetPerceivedObject(nIndex));
	}
	return ParaObject(NULL);
}

bool ParaObject::IsAlwaysSentient()
{
	IGameObject* pObj = ToGameObject();
	if(pObj!=NULL)
	{
		return pObj->IsAlwaysSentient();
	}
	return false;
}

void ParaObject::SetAlwaysSentient(bool bAlways)
{
	IGameObject* pObj = ToGameObject();
	if(pObj!=NULL)
	{
		pObj->SetAlwaysSentient(bAlways);
	}
}

void ParaObject::MakeSentient(bool bSentient)
{
	IGameObject* pObj = ToGameObject();
	if(pObj!=NULL)
	{
		pObj->MakeSentient(bSentient);
	}
}

void ParaObject::UpdateTileContainer()
{
	if(IsValid())
	{
		m_pObj->UpdateTileContainer();
	}
}

void ParaObject::MakeGlobal(bool bGlobal)
{
	IGameObject* pObj = ToGameObject();
	if(pObj!=NULL)
	{
		pObj->MakeGlobal(bGlobal);
	}
}
bool ParaObject::IsGlobal()
{
	if(IsValid())
	{
		return m_pObj->IsGlobal();
	}
	return false;
}

bool ParaObject::IsPersistent()
{
	IGameObject* pObj = ToGameObject();
	if(pObj!=NULL)
	{
		return pObj->IsPersistent();
	}
	return true;
}


void ParaObject::SetPersistent(bool bPersistent)
{
	if(IsValid())
	{
		m_pObj->SetPersistent(bPersistent);
	}
}

void ParaObject::SetMovableRegion(float center_x, float center_y, float center_z, float extent_x, float extent_y, float extent_z)
{
	IGameObject* pObj = ToGameObject();
	if(pObj!=NULL)
	{
		CShapeAABB aabb(Vector3(center_x, center_y, center_z), Vector3(extent_x, extent_y, extent_z));
		pObj->SetMovableRegion(&aabb);
	}
}

void ParaObject::GetMovableRegion(float* center_x, float* center_y, float* center_z, float* extent_x, float* extent_y, float* extent_z)
{
	IGameObject* pObj = ToGameObject();
	if(pObj!=NULL)
	{
		const CShapeAABB* aabb = pObj->GetMovableRegion();
		if(aabb!=0)
		{
			*center_x = aabb->GetCenter(0);
			*center_y = aabb->GetCenter(1);
			*center_z = aabb->GetCenter(2);

			*extent_x = aabb->GetExtents(0);
			*extent_y = aabb->GetExtents(1);
			*extent_z = aabb->GetExtents(2);
		}
	}
}

bool ParaObject::SaveToDB()
{
	IGameObject* pObj = ToGameObject();
	if(pObj!=NULL)
	{
		return pObj->SaveToDB();
	}
	return false;
}

void ParaObject::SetGroupID(int nGroup)
{
	IGameObject* pObj = ToGameObject();
	if(pObj!=NULL)
	{
		pObj->SetGroupID(nGroup);
	}
}

void ParaObject::SetSentientField(DWORD dwFieldOrGroup, bool bIsGroup)
{
	IGameObject* pObj = ToGameObject();
	if(pObj!=NULL)
	{
		pObj->SetSentientField(dwFieldOrGroup, bIsGroup);
	}
}

bool ParaObject::IsSentientWith(const ParaObject& obj)
{
	IGameObject* pObj = ToGameObject();
	if(pObj!=NULL)
	{
		return pObj->IsSentientWith(obj.m_pObj->QueryIGameObject());
	}
	return false;
}

int ParaObject::GetEffectHandle()
{
	if(IsValid())
		return m_pObj->GetPrimaryTechniqueHandle();
	return 0;
}

void ParaObject::SetEffectHandle(int nHandle)
{
	if(IsValid())
		m_pObj->SetPrimaryTechniqueHandle(nHandle);
}

void ParaObject::SetHomeZone(const char* sHomeZone)
{
	if(IsValid())
		m_pObj->SetHomeZoneName(sHomeZone);
}

const char* ParaObject::GetHomeZone()
{
	return IsValid() ? m_pObj->GetHomeZoneName() : NULL;
}

int ParaObject::AddReference(const ParaObject& maker, int nTag)
{
	if(IsValid() && maker.IsValid())
	{
		return m_pObj->AddReference(maker.get(), nTag);
	}
	return 0;
}

int ParaObject::DeleteReference(const ParaObject&  ref)
{
	if(IsValid() && ref.IsValid())
	{
		return m_pObj->DeleteReference(ref.get());
	}
	return 0;
}

int ParaObject::DeleteAllRefs()
{
	if(IsValid())
	{
		return m_pObj->DeleteAllRefs();
	}
	return 0;
}

int ParaObject::GetRefObjNum()
{
	if(IsValid())
	{
		return (int)m_pObj->GetRefObjNum();
	}
	return 0;
}

ParaObject ParaObject::GetRefObject(int nIndex)
{
	ParaObject obj;
	if(IsValid())
	{
		RefListItem* item = m_pObj->GetRefObject(nIndex);
		if(item!=0)
			obj.m_pObj = (CBaseObject*)item->m_object;
	}
	return obj;
}

object ParaObject::GetViewBox( const object& output )
{
	if(type(output) == LUA_TTABLE && IsValid())
	{
		DVector3 vPos = m_pObj->GetPosition();
		float fOBB_X, fOBB_Y, fOBB_Z, fFacing;
		m_pObj->GetBoundingBox(&fOBB_X, &fOBB_Y, &fOBB_Z,&fFacing);
		output["pos_x"] = vPos.x;
		output["pos_y"] = vPos.y;
		output["pos_z"] = vPos.z;
		output["obb_x"] = fOBB_X;
		output["obb_y"] = fOBB_Y;
		output["obb_z"] = fOBB_Z;
	}
	return object(output);
}

int ParaObject::GetNumReplaceableTextures()
{
	if(IsValid())
	{
		return m_pObj->GetNumReplaceableTextures();
	}
	return 0;
}

ParaAssetObject ParaObject::GetDefaultReplaceableTexture( int ReplaceableTextureID )
{
	ParaAssetObject texture;
	if(IsValid())
	{
		texture.m_pAsset = m_pObj->GetDefaultReplaceableTexture(ReplaceableTextureID);
	}
	return texture;
}

ParaAssetObject ParaObject::GetReplaceableTexture( int ReplaceableTextureID )
{
	ParaAssetObject texture;
	if(IsValid())
	{
		texture.m_pAsset = m_pObj->GetReplaceableTexture(ReplaceableTextureID);
	}
	return texture;
}

bool ParaObject::SetReplaceableTexture( int ReplaceableTextureID, ParaAssetObject pTextureEntity )
{
	if(IsValid())
	{
		if(pTextureEntity.GetType() == AssetEntity::texture)
		{
			return m_pObj->SetReplaceableTexture(ReplaceableTextureID, (TextureEntity*)pTextureEntity.m_pAsset);
		}
		else if (!pTextureEntity.IsValid())
		{
			return m_pObj->SetReplaceableTexture(ReplaceableTextureID, NULL);
		}
	}
	return false;
}

int ParaObject::GetXRefScriptCount()
{
	if(IsValid())
	{
		CMeshObject* pMesh = NULL;
		if(m_pObj->GetType() == CBaseObject::MeshPhysicsObject)
			pMesh = ((CMeshPhysicsObject*)m_pObj)->GetMeshObject();
		else if(m_pObj->GetType() == CBaseObject::MeshObject)
			pMesh = (CMeshObject*)m_pObj;

		if(pMesh)
		{
			return pMesh->GetXRefScriptCount();
		}
	}
	return 0;
}

const char* ParaObject::GetXRefScript( int nIndex )
{
	if(IsValid())
	{
		CMeshObject* pMesh = NULL;
		if(m_pObj->GetType() == CBaseObject::MeshPhysicsObject)
			pMesh = ((CMeshPhysicsObject*)m_pObj)->GetMeshObject();
		else if(m_pObj->GetType() == CBaseObject::MeshObject)
			pMesh = (CMeshObject*)m_pObj;

		if(pMesh)
		{
			return pMesh->GetXRefScript(nIndex);
		}
	}
	return NULL;
}

void ParaObject::GetXRefScriptPosition( int nIndex, float *x, float *y, float *z )
{
	if(IsValid())
	{
		CMeshObject* pMesh = NULL;
		if(m_pObj->GetType() == CBaseObject::MeshPhysicsObject)
			pMesh = ((CMeshPhysicsObject*)m_pObj)->GetMeshObject();
		else if(m_pObj->GetType() == CBaseObject::MeshObject)
			pMesh = (CMeshObject*)m_pObj;

		if(pMesh)
		{
			Vector3 vOut = pMesh->GetXRefScriptPosition(nIndex);
			*x = vOut.x;
			*y = vOut.y;
			*z = vOut.z;
		}
	}
}

void ParaObject::GetXRefScriptScaling( int nIndex, float *x, float *y, float *z )
{
	if(IsValid())
	{
		CMeshObject* pMesh = NULL;
		if(m_pObj->GetType() == CBaseObject::MeshPhysicsObject)
			pMesh = ((CMeshPhysicsObject*)m_pObj)->GetMeshObject();
		else if(m_pObj->GetType() == CBaseObject::MeshObject)
			pMesh = (CMeshObject*)m_pObj;

		if(pMesh)
		{
			Vector3 vOut = pMesh->GetXRefScriptScaling(nIndex);
			*x = vOut.x;
			*y = vOut.y;
			*z = vOut.z;
		}
	}
}

float ParaObject::GetXRefScriptFacing( int nIndex )
{
	if(IsValid())
	{
		CMeshObject* pMesh = NULL;
		if(m_pObj->GetType() == CBaseObject::MeshPhysicsObject)
			pMesh = ((CMeshPhysicsObject*)m_pObj)->GetMeshObject();
		else if(m_pObj->GetType() == CBaseObject::MeshObject)
			pMesh = (CMeshObject*)m_pObj;

		if(pMesh)
		{
			return pMesh->GetXRefScriptFacing(nIndex);
		}
	}
	return 0.f;
}

const char* ParaObject::GetXRefScriptLocalMatrix( int nIndex )
{
	if(IsValid())
	{
		CMeshObject* pMesh = NULL;
		if(m_pObj->GetType() == CBaseObject::MeshPhysicsObject)
			pMesh = ((CMeshPhysicsObject*)m_pObj)->GetMeshObject();
		else if(m_pObj->GetType() == CBaseObject::MeshObject)
			pMesh = (CMeshObject*)m_pObj;

		if(pMesh)
		{
			return pMesh->GetXRefScriptLocalMatrix(nIndex);
		}
	}
	return NULL;
}

bool ParaObject::IsStanding()
{
	if(IsValid())
	{
		return m_pObj->IsStanding();
	}
	return true;
}

bool ParaObject::IsVisible()
{
	if(IsValid())
	{
		return m_pObj->IsVisible();
	}
	return true;
}

void ParaObject::SetVisible( bool bVisible )
{
	if(IsValid())
	{
		m_pObj->SetVisibility(bVisible);
	}
}

bool ParaObject::CheckAttribute( DWORD attribute )
{
	if(IsValid())
	{
		return m_pObj->CheckAttribute(attribute);
	}
	return false;
}

void ParaObject::SetAttribute( DWORD dwAtt, bool bTurnOn )
{
	if(IsValid())
	{
		m_pObj->SetAttribute(dwAtt, bTurnOn);
	}
}

void ParaObject::SetHeadOnText( const char* sText , int nIndex)
{
	if(IsValid())
	{
		m_pObj->SetHeadOnText(sText, nIndex);
	}
}

const char* ParaObject::GetHeadOnText(int nIndex)
{
	if(IsValid())
	{
		return m_pObj->GetHeadOnText(nIndex);
	}
	return NULL;
}

void ParaObject::SetHeadOnUITemplateName( const char* sUIName , int nIndex)
{
	if(IsValid())
	{
		m_pObj->SetHeadOnUITemplateName(sUIName, nIndex);
	}
}

const char* ParaObject::GetHeadOnUITemplateName(int nIndex)
{
	if(IsValid())
	{
		return m_pObj->GetHeadOnUITemplateName(nIndex);
	}
	return NULL;
}

void ParaObject::SetHeadOnTextColor( const char* strColor, int nIndex )
{
	if(IsValid())
	{
		LinearColor color;
		int r,g,b, a=255;
		if(sscanf(strColor, "%d %d %d %d", &r,&g,&b,&a)<3)
			color = LinearColor(1,1,1,1);
		else{
			color=COLOR_ARGB(a,r,g,b);
		}
		m_pObj->SetHeadOnTextColor(color, nIndex);
	}
}

void ParaObject::SetHeadOnOffest( float x, float y, float z, int nIndex)
{
	if(IsValid())
	{
		Vector3 vOffset(x,y,z);
		m_pObj->SetHeadOnOffest(vOffset, nIndex);
	}
}

void ParaObject::GetHeadOnOffset( int nIndex, float* x, float* y, float* z )
{
	Vector3 vOffset(0,0,0);
	if(IsValid())
	{
		m_pObj->GetHeadOnOffset(&vOffset, nIndex);
	}
	*x = vOffset.x;
	*y = vOffset.y;
	*z = vOffset.z;
}

void ParaObject::ShowHeadOnDisplay( bool bShow , int nIndex)
{
	if(IsValid())
	{
		m_pObj->ShowHeadOnDisplay(bShow, nIndex);
	}
}

bool ParaObject::IsHeadOnDisplayShown(int nIndex)
{
	if(IsValid())
	{
		return m_pObj->IsHeadOnDisplayShown(nIndex);
	}
	return false;
}

bool ParaObject::HasHeadOnDisplay(int nIndex)
{
	if(IsValid())
	{
		return m_pObj->HasHeadOnDisplay(nIndex);
	}
	return false;
}

ParaScripting::ParaParamBlock ParaObject::GetEffectParamBlock()
{
	return ParaParamBlock(IsValid()?m_pObj->GetEffectParamBlock(true) : NULL);
}

bool ParaObject::HasAttachmentPoint( int nAttachmentID )
{
	if(IsValid())
	{
		return m_pObj->HasAttachmentPoint(nAttachmentID);
	}
	return false;
}

void ParaObject::GetAttachmentPosition( int nAttachmentID, float * x, float * y, float *z )
{
	if(IsValid())
	{
		Vector3 vPos;
		if(m_pObj->GetAttachmentPosition(vPos, nAttachmentID))
		{
			// save to output
			*x = vPos.x;
			*y = vPos.y;
			*z = vPos.z;
		}
	}
}

void ParaObject::SetAnimation( int nAnimID )
{
	if(IsValid())
	{
		m_pObj->SetAnimation(nAnimID);
	}
}

int ParaObject::GetAnimation()
{
	if(IsValid())
	{
		return m_pObj->GetAnimation();
	}
	return 0;
}

void ParaObject::SetPhysicsGroup( int nGroup )
{
	if(IsValid())
	{
		m_pObj->SetPhysicsGroup(nGroup);
	}
}

int ParaObject::GetPhysicsGroup()
{
	if(IsValid())
	{
		return m_pObj->GetPhysicsGroup();
	}
	return 0;
}

void ParaObject::SetSelectGroupIndex(int nGroupIndex)
{
	if(IsValid())
	{
		CGlobals::GetSelectionManager()->AddObject(get(), nGroupIndex);
		// m_pObj->SetSelectGroupIndex(nGroupIndex);
	}
}

int ParaObject::GetSelectGroupIndex()
{
	if(IsValid())
	{
		return m_pObj->GetSelectGroupIndex();
	}
	return 0;
}

luabind::object ParaObject::GetField( const char* sFieldname, const object& output )
{
	ParaAttributeObject att(m_pObj);
	return att.GetField(sFieldname, output);
}

void ParaObject::SetField( const char* sFieldname, const object& input )
{
	ParaAttributeObject att(m_pObj);
	att.SetField(sFieldname, input);
}

void ParaObject::CallField(const char* sFieldname)
{
	ParaAttributeObject att(m_pObj);
	att.CallField(sFieldname);
}

luabind::object ParaObject::GetDynamicField( const char* sFieldname, const object& output )
{
	ParaAttributeObject att(m_pObj);
	return att.GetDynamicField(sFieldname, output);
}

void ParaObject::SetDynamicField( const char* sFieldname, const object& input )
{
	ParaAttributeObject att(m_pObj);
	att.SetDynamicField(sFieldname, input);
}

ParaScripting::ParaObject ParaObject::GetObject(const char* name)
{
	ParaObject obj;
	if (IsValid())
	{
		obj.m_pObj = m_pObj->GetChildByName(name);
	}
	return obj;
}

ParaScripting::ParaAssetObject ParaObject::GetTexture()
{
	return ParaAssetObject(IsValid() ? m_pObj->GetTexture() : NULL);
}

bool ParaObject::Export(const char* filepath, const char* typ) {
	if (!IsValid()) return false;
	if (m_pObj->IsBiped()) {
		CBipedObject* pBipedObj = (CBipedObject*)m_pObj;
		CharModelInstance* model = pBipedObj->GetCharModelInstance();
		GltfModel::ExportCharModel(model, filepath);
	} else {
		AssetEntity* pAsset = m_pObj->GetPrimaryAsset();
		if (pAsset && pAsset->IsValid())
		{
			pAsset->LoadAsset();
			if (pAsset->GetType() == AssetEntity::parax)
			{
				ParaXEntity* pParaXEntity = (ParaXEntity*)pAsset;
				CParaXModel* pModel = pParaXEntity->GetModel();
				GltfModel::ExportParaXModel(pModel, filepath);
				if (pModel)
				{
					// pModel->SaveToGltf("D:\\test_old.gltf");
				}
			}
			else if (pAsset->GetType() == AssetEntity::mesh)
			{
				return false;
			}
		}
	}
	
	return false;
}

//////////////////////////////////////////////////////////////////////////
//
//
// ParaScene namespace table.
//
//
//////////////////////////////////////////////////////////////////////////

ParaObject ParaScene::TogglePlayer()
{
	return ParaObject(CGlobals::GetScene()->TogglePlayer());
}

void ParaScene::Execute(const char* strCmd)
{
	if(strcmp("show report", strCmd) == 0)
	{
		CGlobals::GetScene()->SetGenerateReport(true);
	}
	else if(strcmp("hide report", strCmd) == 0)
	{
		CGlobals::GetScene()->SetGenerateReport(false);
	}
	else if(strcmp("show OBB", strCmd) == 0)
	{
		CGlobals::GetScene()->ShowBoundingBox(true);
	}
	else if(strcmp("hide OBB", strCmd) == 0)
	{
		CGlobals::GetScene()->ShowBoundingBox(false);
	}
}

ParaObject ParaScene::GetNextObject(ParaObject& obj)
{
	if(obj.IsCharacter())
	{
		CBipedObject* pCur = obj.ToCharacter().m_pCharacter;
		CBipedObject* pNext = CGlobals::GetScene()->GetNextPlayer(pCur);
		if(pNext)
		{
			return ParaObject(pNext);
		}
	}
	return ParaObject();
}

void ParaScene::GetObject_(ParaObject* pOut, const char * strObjName)
{
	*pOut = GetObject(strObjName);
}

ParaObject ParaScene::GetObject(const char * strObjName)
{
	CBaseObject * pObj = NULL;
	if (strObjName == 0 )
	{
		return ParaObject();
	}
	/// extract the type information from the name.
#define MAX_TYPE_LENGTH		47
	char _type[MAX_TYPE_LENGTH+1]="";
	if(strObjName[0] == '<')
	{
		strObjName ++;
		int i=0;
		for(; (*strObjName != '\0') && (*strObjName != '>') && (i<MAX_TYPE_LENGTH);i++, strObjName++)
		{
			_type[i] = *strObjName;
		}
		if(*strObjName == '>')
			strObjName++;
		_type[i] = 0;
	}
	/// search for each type
	if(_type[0] == '\0')
	{
		// get the global object
		pObj = CGlobals::GetScene()->GetGlobalObject(strObjName);
	}
	else if(strcmp(_type, "player") == 0)
	{
		// get the current player
		pObj = CGlobals::GetScene()->GetCurrentPlayer();
	}
	else if (strcmp(_type, "root") == 0)
	{
		pObj = CGlobals::GetScene();
	}
	else if(strcmp(_type, "portal") == 0)
	{
		// get the portal object
		pObj = CGlobals::GetScene()->GetPortalNode(strObjName);
	}
	else if(strcmp(_type, "zone") == 0)
	{
		// get the zone object
		pObj = CGlobals::GetScene()->GetZoneNode(strObjName);
	}
	else if(strcmp(_type, "managed_loader") == 0)
	{
		pObj = (CBaseObject*) CGlobals::GetScene()->GetManagedLoader(strObjName);
	}
	else if(strcmp(_type, "NPC") == 0)
	{
		// get global non-player PC
		pObj = CGlobals::GetScene()->GetRootTile()->SearchObject(strObjName);
	}
	else
	{
		// _type can be "CRenderTarget", ...
		pObj = CGlobals::GetScene()->FindObjectByNameAndType(strObjName, _type);
	}
	
	return ParaObject(pObj);
}

ParaObject ParaScene::GetObject3(float x, float y, float z)
{
	return ParaObject(CGlobals::GetScene()->GetLocalObject(Vector3 (x,y,z)));
}
ParaObject ParaScene::GetObject4(float x, float y, float z, float fEpsilon)
{
	return ParaObject(CGlobals::GetScene()->GetLocalObject(Vector3 (x,y,z), fEpsilon));
}
ParaScripting::ParaObject ParaScene::GetObject5( int nID )
{
	return ParaObject(CBaseObject::GetObjectByID(nID));
}

bool ParaScene::CheckExist(int nID)
{
	return CBaseObject::GetObjectByID(nID) != NULL;
}

ParaObject ParaScene::GetPlayer()
{
	return ParaObject(CGlobals::GetScene()->GetCurrentPlayer());
}
void ParaScene::GetPlayer_(ParaObject* pOut)
{
	*pOut = GetPlayer();
}
void ParaScene::CreateWorld(const char * sWorldName, float fWorldSize, const char* sConfigFile)
{
	CGlobals::GetScene()->SetIdentifier(sWorldName);
	/// the quad tree depth is automatically calculated. The smallest quad tree tile is of size 128 meters.
	int nDepth = (int)(log10( fWorldSize/128.f )/log10(2.f));
	if(nDepth<2)
		nDepth = 2;
	// clear scene
	CGlobals::GetScene()->GetRootTile()->ResetTerrain(fWorldSize/2.f, nDepth);
	// set the current world directory.
	string worldDir = sConfigFile;
	CParaFile::ToCanonicalFilePath(worldDir, worldDir, false);
	worldDir = CParaFile::GetParentDirectoryFromPath(worldDir);
	if(worldDir.size()>1 && worldDir[worldDir.size()-1]=='/') 
	{
		// remove the trailing slash.
		worldDir = worldDir.substr(0, worldDir.size()-1);
	}
	CPathReplaceables::GetSingleton().AddVariable(CPathVariable("WORLD", worldDir, true));

	// create terrain.
	CGlobals::GetGlobalTerrain()->CreateTerrain(sConfigFile);
}

void ParaScene::Reset()
{
	CGlobals::GetScene()->ResetScene();
}

void ParaScene::CreateGlobalTerrain(float fRadius, int nDepth, const char* sHeightmapfile, 
									float fTerrainSize, float fElevscale, int bSwapvertical, 
									const char*  sMainTextureFile, const char* sCommonTextureFile, 
									int nMaxBlockSize, float fDetailThreshold)
{
	CGlobals::GetScene()->GetRootTile()->ResetTerrain(fRadius, nDepth);
	CGlobals::GetGlobalTerrain()->CreateSingleTerrain(sHeightmapfile, 
		fTerrainSize, fElevscale, bSwapvertical, 
		sMainTextureFile, sCommonTextureFile, 
		nMaxBlockSize, fDetailThreshold);
}

void ParaScene::Attach(ParaObject& obj)
{
	if (obj.IsValid())
	{
		// if anything attached to the scene via script set it to modified state. 
		CGlobals::GetScene()->SetModified(true);

		if(obj.m_pObj->GetTileContainer() == NULL)
		{// only attach if it has not been attached before
			CGlobals::GetScene()->AttachObject(obj.m_pObj);
		}
		else
		{
			// if the object has been attached before, detach it and re-attach it to the scene.
			// this will update the physics if it is a static object. 
			if(CGlobals::GetScene()->DetachObject(obj.m_pObj))
				CGlobals::GetScene()->AttachObject(obj.m_pObj);
		}
		// set the terrain content modified, if the newly attached object is not a global biped object. 
		// TODO: this is just one automatic way to detect terrain content changes. There are some misses, such
		// as when character position changes or detached from the scene.Currently one needs to manually call Terrain::SetContentModified().
		if(!CGlobals::IsLoading())
		{
			if((!obj.IsCharacter()) || !obj.ToCharacter().m_pCharacter->IsGlobal())
			{
				Vector3 vPos = obj.m_pObj->GetPosition();
				if(CGlobals::GetScene()->IsPersistent())
				{
					CGlobals::GetGlobalTerrain()->SetContentModified(vPos.x, vPos.z,true);
				}
			}
		}
	}
}
void ParaScene::Detach(ParaObject& pObj)
{
	if(pObj.IsValid())
	{
		CGlobals::GetScene()->DetachObject(pObj.m_pObj) ;
	}
}
void ParaScene::Delete(ParaObject& obj)
{
	if(obj.IsValid())
	{
		// set the terrain content modified, if the object is not a global biped object. 
		// TODO: this is just one automatic way to detect terrain content changes. There are some misses, such
		// as when character position changes or detached from the scene.Currently one needs to manually call ParaTerrain.SetContentModified().
		if(!CGlobals::IsLoading())
		{
			if((!obj.IsCharacter()) || !obj.ToCharacter().m_pCharacter->IsGlobal())
			{
				Vector3 vPos = obj.m_pObj->GetPosition();
				if(CGlobals::GetScene()->IsPersistent())
				{
					CGlobals::GetGlobalTerrain()->SetContentModified(vPos.x, vPos.z,true);
				}
			}
		}

#ifdef DELETE_FROM_DB
		if(obj.IsPersistent())
		{
			obj.SetPersistent(false);
			obj.SaveToDB();
		}
#endif
		CGlobals::GetScene()->DeleteObject(obj.m_pObj);
		obj.m_pObj = NULL;
	}
}

void ParaScene::FireMissile(int nMissileID, float fSpeed, double fromX, double fromY, double fromZ, double toX, double toY, double toZ)
{
	ParaAssetObject asset(CGlobals::GetAssetManager()->LoadParaXByID(nMissileID));
	FireMissile2(asset, fSpeed, fromX, fromY, fromZ, toX, toY, toZ);
}

void ParaScene::FireMissile1(const char* assetname, float fSpeed, double fromX, double fromY, double fromZ, double toX, double toY, double toZ)
{
	ParaAssetObject asset (CGlobals::GetAssetManager()->LoadParaX("", assetname));
	FireMissile2(asset, fSpeed, fromX, fromY, fromZ, toX, toY, toZ);
}

void ParaScene::FireMissile2(ParaAssetObject& asset, float fSpeed, double fromX, double fromY, double fromZ, double toX, double toY, double toZ)
{
	if(asset.IsValid() && asset.m_pAsset->GetType()==AssetEntity::parax)
	{
		// it should be treated as global missile object.
		CMissileObject* pMissile =  CGlobals::GetScene()->NewMissile();
		if(pMissile)
		{
			pMissile->InitObject((ParaXEntity*)(asset.m_pAsset), DVector3(toX, toY, toZ), fSpeed);
			pMissile->SetPosition(DVector3(fromX, fromY, fromZ));
		}
		else
		{
			OUTPUT_LOG("failed creating missile object.\n");
		}
	}
}

ParaObject ParaScene::CreateManagedLoader(const char * sLoaderName)
{
	string sTmp(sLoaderName);
	ParaObject obj((CBaseObject*)CGlobals::GetScene()->CreateManagedLoader(sTmp));
	return obj;
}

bool ParaScene::CreateSkyBox(const char* strObjectName, const char* strMeshAssetName, float fScaleX, float fScaleY,float fScaleZ, float fHeightOffset)
{
	MeshEntity* pMesh =  CGlobals::GetAssetManager()->GetMesh(strMeshAssetName);
	ParaAssetObject obj(pMesh);
	return CreateSkyBox_(strObjectName, obj, fScaleX, fScaleY, fScaleZ, fHeightOffset);	
}
bool ParaScene::CreateSkyBox_(const char* strObjectName, ParaAssetObject& asset, float fScaleX, float fScaleY,float fScaleZ, float fHeightOffset)
{
	if(asset.IsValid())
		return CGlobals::GetScene()->CreateSkyBox(strObjectName, *(asset.m_pAsset), fScaleX, fScaleY, fScaleZ, fHeightOffset);	
	else
		return false;
}
void ParaScene::DeleteSkyBox(const char* strObjectName)
{
	CGlobals::GetScene()->DeleteSkyBox(strObjectName);
}


ParaScripting::ParaObject ParaScene::CreateVoxelMesh( const char* strObjectName, const char* sGridFileName, const char* sTextureFileName)
{
	ParaObject obj;
#ifdef USE_DIRECTX_RENDERER
	CVoxelMesh* pObj = new CVoxelMesh();
	TextureEntity * pBaseTexture = NULL;
	if(sTextureFileName!=NULL &&  sTextureFileName[0]!='\0')
	{
		pBaseTexture = CGlobals::GetAssetManager()->LoadTexture("", sTextureFileName, TextureEntity::StaticTexture);
	}
	pObj->InitObject(sGridFileName, DVector3(0,0,0), pBaseTexture); 
	
	obj.m_pObj = pObj;
#endif
	return obj;
}

ParaScripting::ParaObject ParaScene::CreateZone( const char* sZoneName, const char* sBoundingVolumes, 
	float width, float height, float depth, float facing)
{
	ParaObject obj;

	CZoneNode* pObj = CGlobals::GetScene()->CreateGetZoneNode(sZoneName);
	if(pObj)
	{
		pObj->SetIdentifier(sZoneName);
		pObj->SetBoundingBox(width, height, depth, facing);
		pObj->SetZonePlanes(sBoundingVolumes);
	}
	obj.m_pObj = pObj;
	return obj;
}

ParaScripting::ParaObject ParaScene::CreatePortal( const char* sPortalName, const char* sHomeZone, const char* sTargetZone, const char* sQuadVertices,
	float width, float height, float depth, float facing)
{
	ParaObject obj;

	CPortalNode* pObj = CGlobals::GetScene()->CreateGetPortalNode(sPortalName);
	if(pObj)
	{
		pObj->SetIdentifier(sPortalName);
		pObj->SetBoundingBox(width, height, depth, facing);
		pObj->SetPortalPoints(sQuadVertices);

		pObj->SetHomeZone(sHomeZone);
		pObj->SetTargetZone(sTargetZone);
	}
	obj.m_pObj = pObj;
	return obj;
}

ParaObject ParaScene::CreateMeshObject(const char* strObjectName, const char* strMeshAssetName, 
									   float fOBB_X, float fOBB_Y, float fOBB_Z, float fFacing, bool bSolid, const char* localMatrix)
{
	// --  query asset resources, if none, create it
	MeshEntity* pMesh =  CGlobals::GetAssetManager()->GetMesh(strMeshAssetName);

	ParaAssetObject obj(pMesh);
	return CreateMeshObject(strObjectName, obj, fOBB_X, fOBB_Y, fOBB_Z, fFacing, bSolid, localMatrix);
}

ParaObject ParaScene::CreateMeshObject(const char* strObjectName, ParaAssetObject& asset, 
									   float fOBB_X, float fOBB_Y, float fOBB_Z, float fFacing, bool bSolid, const char* localMatrix)
{
	ParaObject obj;
	
	MeshEntity* pMesh = NULL;
	if(asset.IsValid() && asset.m_pAsset->GetType() == AssetEntity::mesh)
		pMesh = (MeshEntity*)asset.m_pAsset;

	// create the object
	if(pMesh)
	{
		// local transform matrix
		Matrix4 mat; 
		if(sscanf(localMatrix, "%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f", &mat._11, &mat._12, &mat._13, &mat._21, &mat._22, &mat._23,&mat._31, &mat._32, &mat._33,&mat._41, &mat._42, &mat._43) >=12)
		{
			mat._14 = 0;
			mat._24 = 0;
			mat._34 = 0;
			mat._44 = 1;
		}
		else
		{
			mat = Matrix4::IDENTITY;
		}

		CMeshObject* pObj = new CMeshObject();
		pObj->InitObject(pMesh, NULL, Vector3(0,0,0)); 
		pObj->SetBoundingBox(fOBB_X,fOBB_Y, fOBB_Z, fFacing);
		pObj->SetLocalTransform(mat);

		pObj->SetMyType(_House);
		/*if(bSolid)
		{
			pObj->SetMyType(_SolidHouse);
		}
		else
		{
			pObj->SetMyType(_DummyMesh);
		}*/

		obj.m_pObj = pObj;
	}
	return obj;
}

ParaObject ParaScene::CreateLightObject(const char* strObjectName, float fPosX, float fPosY, float fPosZ, const char* sLightParams, const char* localMatrix)
{
	ParaObject obj;
#ifdef USE_DIRECTX_RENDERER

	// local transform matrix
	Matrix4 mat; 
	if(sscanf(localMatrix, "%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f", &mat._11, &mat._12, &mat._13, &mat._21, &mat._22, &mat._23,&mat._31, &mat._32, &mat._33,&mat._41, &mat._42, &mat._43) >=12)
	{
		mat._14 = 0;
		mat._24 = 0;
		mat._34 = 0;
		mat._44 = 1;
	}
	else
	{
		mat = Matrix4::IDENTITY;
	}

	CLightObject* pObj = new CLightObject();
	if(sLightParams!=NULL && sLightParams[0]!='\0')
	{
		CLightParam lightparam;
		lightparam.FromString(sLightParams);
		pObj->InitObject(&lightparam, NULL, Vector3(fPosX, fPosY, fPosZ), mat, true); 
	}
	else
		pObj->InitObject(NULL, NULL, Vector3(fPosX, fPosY, fPosZ), mat, true); 
	
	pObj->SetIdentifier(strObjectName);
	obj.m_pObj = pObj;
#endif
	return obj;
}

void ParaScene::CreateMeshPhysicsObject__(ParaObject* pOut, const char* strObjectName, ParaAssetObject& asset, float fOBB_X, float fOBB_Y, float fOBB_Z, bool bApplyPhysics,const char* localMatrix)
{
	*pOut = CreateMeshPhysicsObject(strObjectName, asset, fOBB_X, fOBB_Y, fOBB_Z, bApplyPhysics, localMatrix);
}

ParaObject ParaScene::CreateMeshPhysicsObject(const char* strObjectName, ParaAssetObject& asset, 
											  float fOBB_X, float fOBB_Y, float fOBB_Z, bool bApplyPhysics, const char* localMatrix)
{
	ParaObject obj;


	MeshEntity* pMesh = NULL;
	if(asset.IsValid() && asset.m_pAsset->GetType() == AssetEntity::mesh)
	{
		pMesh = (MeshEntity*)asset.m_pAsset;
	
		// create the object
		if(pMesh)
		{
			// local transform matrix
			Matrix4 mat; 
			if(sscanf(localMatrix, "%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f", &mat._11, &mat._12, &mat._13, &mat._21, &mat._22, &mat._23,&mat._31, &mat._32, &mat._33,&mat._41, &mat._42, &mat._43) >=12)
			{
				mat._14 = 0;
				mat._24 = 0;
				mat._34 = 0;
				mat._44 = 1;
			}
			else
			{
				mat = Matrix4::IDENTITY;
			}

			CMeshPhysicsObject* pObj = new CMeshPhysicsObject();
			pObj->InitObject(pMesh, fOBB_X,fOBB_Y, fOBB_Z, bApplyPhysics, mat); 
			pObj->SetIdentifier(strObjectName);
			obj.m_pObj = pObj;
		}
	}
#ifdef USE_DIRECTX_RENDERER

	else if(asset.IsValid() && asset.m_pAsset->GetType() == AssetEntity::cadMesh)
	{
		CadModel* pCadModel = (CadModel*)asset.m_pAsset;

		if(pCadModel)
		{
			Matrix4 mat; 
			if(sscanf(localMatrix, "%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f,%f", &mat._11, &mat._12, &mat._13, &mat._21, &mat._22, &mat._23,&mat._31, &mat._32, &mat._33,&mat._41, &mat._42, &mat._43) >=12)
			{
				mat._14 = 0;
				mat._24 = 0;
				mat._34 = 0;
				mat._44 = 1;
			}
			else
			{
				mat = Matrix4::IDENTITY;
			}

			CadModelNode* pObj = new CadModelNode();
			pObj->InitObject(pCadModel);
			pObj->SetBoundingBox(fOBB_X,fOBB_Y,fOBB_Z,0);
			pObj->SetLocalTransform(mat);
			pObj->SetIdentifier(strObjectName);
			obj.m_pObj = pObj;
		}
	}
#endif
	return obj;
}

ParaObject ParaScene::CreateMeshPhysicsObject(const char* strObjectName, const char* strMeshAssetName, 
											  float fOBB_X, float fOBB_Y, float fOBB_Z, bool bApplyPhysics, const char* localMatrix)
{
	// --  query asset resources, if none, create it
	MeshEntity* pMesh;
	pMesh =  CGlobals::GetAssetManager()->GetMesh(strMeshAssetName);
	ParaAssetObject obj(pMesh);
	return CreateMeshPhysicsObject(strObjectName, obj, fOBB_X, fOBB_Y, fOBB_Z, bApplyPhysics, localMatrix);
}

ParaObject ParaScene::CreateDynamicPhysicsObject(const char* strObjectName, const char* strMeshAssetName, 
												 float fOBB_X, float fOBB_Y, float fOBB_Z, bool bRenderMesh)
{
	ParaObject obj;
#ifdef USE_DIRECTX_RENDERER

	// --  query asset resources, if none, create it
	MeshEntity* pMesh;
	pMesh =  CGlobals::GetAssetManager()->GetMesh(strMeshAssetName);

	// create the object
	if(pMesh)
	{
		CDynamicObject* pObj = new CDynamicObject();
		pObj->InitObject(pMesh, fOBB_X,fOBB_Y, fOBB_Z, bRenderMesh); 
		pObj->SetIdentifier(strObjectName);
		obj.m_pObj = pObj;
	}
#endif
	return obj;
}
void ParaScene::CreateCharacter__(ParaObject* pOut, const char* strObjectName, ParaAssetObject& asset, const char* strScript, bool bIsGlobal,
							float fRadius, float fFacing, float fScaling)
{
	*pOut = CreateCharacter(strObjectName, asset,  strScript, bIsGlobal, fRadius, fFacing, fScaling);
}

ParaObject ParaScene::CreateCharacter(const char* strObjectName, ParaAssetObject& asset,  const char* strScript, 
									  bool bIsGlobal, 
									  float fRadius, float fFacing, float fScaling)
{
	ParaObject obj;

	// whether the base model has been specified in strMultiAnimationAssetName
	bool bBaseModelSpecified = true;
	AssetEntity* pMAE = NULL;
	if(asset.IsValid())
		pMAE = asset.m_pAsset;
	else
		bBaseModelSpecified = false;

	string sObjName;
	if(strObjectName != NULL)
		sObjName = strObjectName;

	obj = (ParaScene::GetObject(sObjName.c_str()));
	if(obj.IsValid()/* && bIsGlobal*/)
	{
		char tmp[256];
		memset(tmp, 0, sizeof(tmp));
		bool bCollide= true;
		string sNewName;
		while(bCollide)
		{
			snprintf(tmp, 256, "%d", ParaEngine::my_rand());
			sNewName = sObjName+tmp;
			obj = (ParaScene::GetObject(sNewName.c_str()));
			bCollide = obj.IsValid();
		}
		// OUTPUT_LOG("warning: global object with the name %s already exists. Thus a new object named %s is created instead.\r\n", sObjName.c_str(), sNewName.c_str());
		sObjName = sNewName;
	}

	{
		string sName = asset.GetKeyName();
		int nSize = (int)sName .size();
		if(nSize > 4 && sName [nSize-4] == '_')
		{
			char symbol=sName [nSize-3];
			if (symbol== 's')
			{
				// if model file name end with "_s", it will be treated as static and local.
				bIsGlobal = false;
			}
		}
	}
	//////////////////////////////////////////////////////////////////////////
	// a simple biped character
	CBipedObject * pObj = new CBipedObject();
	if( pObj == NULL )
		return obj;
	/// set attribute
	if(bIsGlobal)
	{
		pObj->SetMyType( _Player );
		/// set perceptive radius for the character
		pObj->SetAttribute(OBJ_VOLUMN_TILE_VISITOR) ;
		if(fRadius <= 0)
			pObj->SetAttribute(OBJ_VOLUMN_FREESPACE);

	}
	else
	{
		if(fRadius <= 0)
			pObj->SetMyType( _DummyAnimMesh );
		else
			pObj->SetMyType( _SolidAnimMesh );
	}

	pObj->SetIdentifier(sObjName);

	if(strScript != NULL)
		pObj->SetOnLoadScript(strScript);// associate on load file to this object

	pObj->SetPhysicsRadius(fRadius);
	pObj->SetFacing(fFacing);
	pObj->InitObject(pMAE);
	if(fScaling!=1.0f)
 		pObj->SetSizeScale(fScaling);

	// any newly created object is set to modified. 
	pObj->SetModified(true);

	obj.m_pObj = pObj;
	return obj;
}
ParaObject ParaScene::CreateCharacter(const char* strObjectName, const char* strMultiAnimationAssetName,  const char* strScript, 
									  bool bIsGlobal, 
									  float fRadius, float fFacing, float fScaling)
{
	// whether the base model has been specified in strMultiAnimationAssetName
	bool bBaseModelSpecified = true;
	AssetEntity* pMAE = NULL;
	if(strMultiAnimationAssetName == NULL || strMultiAnimationAssetName[0] == '\0')
		bBaseModelSpecified = false;
	else
	{
		/// get the model asset.
		if((pMAE =  CGlobals::GetAssetManager()->GetParaX(strMultiAnimationAssetName)) == NULL)
		{

#ifdef USE_SUPPORT_MDX_FILE
			if((pMAE =  CGlobals::GetAssetManager()->GetMDX(strMultiAnimationAssetName)) == NULL)
				bBaseModelSpecified = false;
#else
			bBaseModelSpecified = false;
#endif
		}
	}
	if(!bBaseModelSpecified)
		pMAE = NULL;
	ParaAssetObject obj(pMAE);
	return CreateCharacter(strObjectName, obj,  strScript, bIsGlobal, fRadius, fFacing, fScaling);
}

void ParaScene::SetFog(bool bRenderFog, const char* strFogColor, float fFogStart,float fFogEnd, float fFogDensity)
{
	LinearColor colorFog;
	if(sscanf(strFogColor, "%f %f %f", &colorFog.r,&colorFog.g,&colorFog.b)<3)
		colorFog = LinearColor(1,1,1,1);
	CGlobals::GetScene()->SetAndRestoreFog(bRenderFog, colorFog , fFogStart , fFogEnd, fFogDensity );
}

void ParaScene::EnableLighting(bool bEnable)
{
	CGlobals::GetScene()->EnableLight(bEnable);
}

bool ParaScene::IsLightingEnabled()
{
	return CGlobals::GetScene()->IsLightEnabled();
}

void ParaScene::SetGlobalWater(bool bEnable, float fWaterLevel)
{
	if(CGlobals::GetOceanManager())
		CGlobals::GetOceanManager()->SetGlobalWater(bEnable, fWaterLevel);
}

float ParaScene::GetGlobalWaterLevel()
{
	if(CGlobals::GetOceanManager())
		return CGlobals::GetOceanManager()->GetWaterLevel();
	return 0.f;
}

bool ParaScene::IsGlobalWaterEnabled()
{
	if(CGlobals::GetOceanManager())
		return CGlobals::GetOceanManager()->OceanEnabled();
	return false;
}

void ParaScene::AddWaterRipple(float x, float y, float z)
{
	if(CGlobals::GetOceanManager())
		return CGlobals::GetOceanManager()->AddRipple(Vector3(x,y,z));
}

void ParaScene::SetShadowMethod(int nMethod)
{
	CGlobals::GetScene()->SetShadow(nMethod>0);
}

int ParaScene::GetShadowMethod()
{
	return CGlobals::GetScene()->IsShadowMapEnabled()? 1:0;
}

void ParaScene::Play3DSound(const char* strSoundAssetName, float fX, float fY, float fZ)
{
}


int ParaScene::SelectObject( int nGroupIndex, float x,float y, float z, float radius, const char* sFilterFunc )
{
	CShapeSphere circle(Vector3(x,y,z), radius);
	return CGlobals::GetScene()->SelectObject(nGroupIndex, circle, GetFilterFuncByName(sFilterFunc));
}

int ParaScene::SelectObject1( int nGroupIndex, float x1,float y1, float z1,float x2,float y2, float z2, float fRotY, const char* sFilterFunc )
{
	CShapeOBB obb;
	Vector3 v1(x1,y1,z1);
	Vector3 v2(x2,y2,z2);
	obb.mCenter = (v1 + v2)/2;
	float fHalfDiagonalDist = Vector2(v1.x-v2.x, v1.z-v2.z).length()/2;
	obb.mExtents = Vector3(fabs(fHalfDiagonalDist*cos(fRotY)), fabs((y1-y2)/2), fabs(fHalfDiagonalDist*sin(fRotY)));
	ParaMatrixRotationY(&(obb.mRot), fRotY);

	return CGlobals::GetScene()->SelectObject(nGroupIndex, obb, GetFilterFuncByName(sFilterFunc));
}

ParaObject ParaScene::Pick(float rayX, float rayY, float rayZ, float dirX, float dirY, float dirZ, float fMaxDistance, const char* sFilterFunc)
{
	CBaseObject* pObj = NULL;
	OBJECT_FILTER_CALLBACK pFilterFunc = GetFilterFuncByName(sFilterFunc);
	if (strcmp(sFilterFunc, "point") == 0 || strcmp(sFilterFunc, "walkpoint") == 0 || strcmp(sFilterFunc, "terrain") == 0)
	{
		static CSphereObject obj;
		Vector3 vIntersectPos(0, 0, 0);
		float fDist = CGlobals::GetScene()->PickClosest({ rayX, rayY, rayZ }, {dirX, dirY, dirZ}, &pObj, &vIntersectPos, NULL, false, fMaxDistance, GetPhysicsGroupMaskByName(sFilterFunc));
		if (fDist<0)
		{
			return ParaObject(NULL);
		}
		else
		{
			// one can retrieve the intersection point and distance by position and scaling. 
			obj.SetPosition(DVector3(vIntersectPos));
			obj.SetScaling(fDist);
			obj.SetIdentifier(pObj ? pObj->GetIdentifier() : "");
			return ParaObject(&obj);
		}
	}

	Vector3 dir = { dirX, dirY, dirZ };
	dir.normalise();
	CGlobals::GetScene()->PickObject({ { rayX, rayY, rayZ } ,dir }, &pObj, fMaxDistance, pFilterFunc);
	return ParaObject(pObj);
}

ParaObject ParaScene::MousePick(float fMaxDistance, const char* sFilterFunc)
{
	if(!CGlobals::GetGUI()->GetMouseInClient())
		return ParaObject(NULL);
	int x,y;
	CGlobals::GetGUI()->GetMousePosition(&(x), &(y));
	POINT ptCursor;
	ptCursor.x = x;
	ptCursor.y = y;

	if (x < 0 || y < 0)
		return ParaObject(NULL);

	CBaseObject* pObj=NULL;
	OBJECT_FILTER_CALLBACK pFilterFunc = GetFilterFuncByName(sFilterFunc);
	if(strcmp(sFilterFunc, "point")==0 || strcmp(sFilterFunc, "walkpoint")==0 || strcmp(sFilterFunc, "terrain")==0)
	{
		static CSphereObject obj;
		Vector3 vIntersectPos(0,0,0);
		float fDist = CGlobals::GetScene()->PickClosest(ptCursor.x, ptCursor.y, &pObj, &vIntersectPos, NULL, false, fMaxDistance, GetPhysicsGroupMaskByName(sFilterFunc));
		if(fDist<0)
		{
			return ParaObject(NULL);
		}
		else
		{
			// one can retrieve the intersection point and distance by position and scaling. 
			obj.SetPosition(DVector3(vIntersectPos));
			obj.SetScaling(fDist);
			obj.SetIdentifier(pObj ? pObj->GetIdentifier() : "");
			return ParaObject(&obj);
		}
	}

	CGlobals::GetScene()->PickObject(ptCursor.x, ptCursor.y, &pObj, fMaxDistance, pFilterFunc);
	return ParaObject(pObj);
}

void ParaScene::RegisterEvent1(DWORD nEventType, const char* sID, const char* sScript)
{
	CGlobals::GetEventsCenter()->RegisterEvent(nEventType, sID, sScript);
}

void ParaScene::RegisterEvent(const char* sID, const char* sScript)
{
	CGlobals::GetEventsCenter()->RegisterEvent(sID, sScript);
}

void ParaScene::UnregisterEvent(const char* sID)
{
	CGlobals::GetEventsCenter()->UnregisterEvent(sID);
}

void ParaScene::UnregisterAllEvent()
{
	// CGlobals::GetEventsCenter()->UnregisterAllEvent();
	CGlobals::GetEventsCenter()->Reset();
}

void ParaScene::EnableMouseClick(bool bEnable)
{
	CGlobals::GetScene()->EnableMouseEvent(bEnable);
}

void ParaScene::SetTimeOfDay(float time)
{
	CGlobals::GetScene()->GetSunLight().SetTimeOfDay(time);
}

float ParaScene::GetTimeOfDay()
{
	return CGlobals::GetScene()->GetSunLight().GetTimeOfDay();
}

void ParaScene::SetTimeOfDaySTD(float time)
{
	CGlobals::GetScene()->GetSunLight().SetTimeOfDaySTD(time);
}

float ParaScene::GetTimeOfDaySTD()
{
	return CGlobals::GetScene()->GetSunLight().GetTimeOfDaySTD();
}


void ParaScene::SetMaximumAngle(float fMaxAngle)
{
	CGlobals::GetScene()->GetSunLight().SetMaximumAngle(fMaxAngle);
}

float ParaScene::AdvanceTimeOfDay(float timeDelta)
{
	return CGlobals::GetScene()->GetSunLight().AdvanceTimeOfDay(timeDelta);
}

void ParaScene::SetDayLength(float fMinutes)
{
	CGlobals::GetScene()->GetSunLight().SetDayLength(fMinutes);
}

float ParaScene::GetDayLength()
{
	return CGlobals::GetScene()->GetSunLight().GetDayLength();
}

ParaObject ParaScene::GetCurrentActor()
{
	return ParaObject(CGlobals::GetScene()->GetCurrentActor());
}

void ParaScene::SetCurrentActor(ParaObject actor)
{
	CGlobals::GetScene()->SetCurrentActor(actor.m_pObj);
}
void ParaScene::SetModified(bool bModified)
{
	CGlobals::GetScene()->SetModified(bModified);
}

bool ParaScene::IsModified()
{
	return CGlobals::GetScene()->IsModified();
}

ParaAttributeObject ParaScene::GetAttributeObject()
{
	return ParaAttributeObject(CGlobals::GetScene());
}

ParaScripting::ParaAttributeObject ParaScene::GetAttributeObject1(const char* name)
{
	return ParaAttributeObject(CGlobals::GetScene()->GetChildByName(name));
}

void ParaScene::GetAttributeObject_(ParaAttributeObject& output)
{
	output = GetAttributeObject();
}

ParaAttributeObject ParaScene::GetAttributeObjectSky()
{
	return ParaAttributeObject(CGlobals::GetScene()->GetCurrentSky());
}

void ParaScene::GetAttributeObjectSky_(ParaAttributeObject& output)
{
	output = GetAttributeObjectSky();
}
ParaAttributeObject ParaScene::GetAttributeObjectPlayer()
{
	return ParaAttributeObject(CGlobals::GetScene()->GetCurrentPlayer());
}
void ParaScene::GetAttributeObjectPlayer_(ParaAttributeObject& output)
{
	output = GetAttributeObjectPlayer();
}

ParaAttributeObject ParaScene::GetAttributeObjectOcean()
{
	return ParaAttributeObject((IAttributeFields*)CGlobals::GetOceanManager());
}

void ParaScene::GetAttributeObjectOcean_(ParaAttributeObject & output)
{
	output = GetAttributeObjectOcean();
}

ParaAttributeObject ParaScene::GetAttributeObjectSunLight()
{
	return ParaAttributeObject(&(CGlobals::GetScene()->GetSunLight()));
}

void ParaScene::GetAttributeObjectSunLight_(ParaAttributeObject& output)
{
	output = GetAttributeObjectSunLight();
}

void ParaScene::UpdateOcean()
{
	CGlobals::GetScene()->UpdateOcean();
}

ParaObject ParaScene::GetObjectByViewBox( const object& viewbox )
{
	CBaseObject* pObj = NULL;
	if(type(viewbox) == LUA_TTABLE)
	{
		Vector3 vPos;
		float fOBB_X, fOBB_Y, fOBB_Z;
		try
		{
#define object_cast_checkNumber(x)   ((type(x)==LUA_TNUMBER) ? (object_cast<float>(x)) : 0)
			vPos.x = object_cast_checkNumber(viewbox["pos_x"]);
			vPos.y = object_cast_checkNumber(viewbox["pos_y"]);
			vPos.z = object_cast_checkNumber(viewbox["pos_z"]);
			fOBB_X = object_cast_checkNumber(viewbox["obb_x"]);
			fOBB_Y = object_cast_checkNumber(viewbox["obb_y"]);
			fOBB_Z = object_cast_checkNumber(viewbox["obb_z"]);

			CShapeAABB aabb(Vector3(vPos.x, vPos.y, vPos.z), Vector3(fOBB_X/2, fOBB_Y/2, fOBB_Z/2));
			pObj = CGlobals::GetScene()->GetObjectByViewBox(aabb);
		}
		catch (...)
		{
			
		}
	}
	return ParaObject(pObj);
}

int ParaScene::GetActionMeshesBySphere(const object& inout, float x, float y, float z, float radius)
{
	return GetObjectsBySphere(inout, x,y,z,radius, "actionmesh");
}

int ParaScene::GetObjectsByScreenRect( const object& inout, int left, int top, int right, int bottom, const char* sFilterFunc, float fMaxDistance )
{
	OBJECT_FILTER_CALLBACK pFilterFunc = GetFilterFuncByName(sFilterFunc);
	
	list<CBaseObject*> output;
	RECT rect = {left, top, right, bottom};
	
	int nCount = CGlobals::GetScene()->GetObjectsByScreenRect(output, rect, pFilterFunc, fMaxDistance);
	if(nCount>0)
	{
		list<CBaseObject*>::iterator itCur, itEnd = output.end();
		int i=1;
		for(itCur = output.begin(); itCur!=itEnd; ++itCur)
		{
			inout[i] = ParaObject(*itCur);
			++i;
		}
	}
	return nCount;
}

int ParaScene::GetObjectsBySphere( const object& inout, float x, float y, float z, float radius, const char* sFilterFunc )
{
	OBJECT_FILTER_CALLBACK pFilterFunc = GetFilterFuncByName(sFilterFunc);
	
	list<CBaseObject*> output;
	CShapeSphere sphere(Vector3(x,y,z), radius);
	int nCount = CGlobals::GetScene()->GetObjectsBySphere(output, sphere, pFilterFunc);
	if(nCount>0)
	{
		list<CBaseObject*>::iterator itCur, itEnd = output.end();
		int i=1;
		for(itCur = output.begin(); itCur!=itEnd; ++itCur)
		{
			inout[i] = ParaObject(*itCur);
			++i;
		}
	}
	return nCount;
}

void ParaScene::OnTerrainChanged( float x,float y, float fRadius )
{
	CGlobals::GetScene()->OnTerrainChanged(Vector3(x,0,y), fRadius);
}

bool ParaScene::IsScenePaused()
{
	return CGlobals::GetScene()->IsScenePaused();
}

void ParaScene::PauseScene( bool bEnable )
{
	CGlobals::GetScene()->PauseScene(bEnable);
}

bool ParaScene::IsSceneEnabled()
{
	return CGlobals::GetScene()->IsSceneEnabled();
}

void ParaScene::EnableScene( bool bEnable )
{
	CGlobals::GetScene()->EnableScene(bEnable);
}

int ParaScene::SaveAllCharacters()
{
	return CGlobals::GetScene()->SaveAllCharacters();
}

ParaScripting::ParaMiniSceneGraph ParaScene::GetPostProcessingScene()
{
	return ParaMiniSceneGraph( CGlobals::GetScene()->GetPostProcessingScene());
}

void ParaScene::EnablePostProcessing( bool bEnable , const char* sCallbackScript)
{
	CGlobals::GetScene()->EnablePostProcessing(bEnable, sCallbackScript);
}

bool ParaScene::IsPostProcessingEnabled()
{
	return CGlobals::GetScene()->IsPostProcessingEnabled();
}

ParaScripting::ParaMiniSceneGraph ParaScene::GetMiniSceneGraph( const char* name )
{
	if(name!=0)
	{
		return ParaMiniSceneGraph( CGlobals::GetScene()->GetMiniSceneGraph(name));
	}
	return ParaMiniSceneGraph(NULL);
}

int ParaScene::DeleteMiniSceneGraph( const char* name )
{
	if(name!=0)
		return CGlobals::GetScene()->DeleteMiniSceneGraph(name);
	else
		return 0;
}

void ParaScene::EnableMiniSceneGraph( bool bEnable )
{
	CGlobals::GetScene()->EnableMiniSceneGraph(bEnable);
}

bool ParaScene::IsMiniSceneGraphEnabled()
{
	return CGlobals::GetScene()->IsMiniSceneGraphEnabled();
}

void ParaScene::ShowHeadOnDisplay( bool bShow )
{
	CGlobals::GetScene()->ShowHeadOnDisplay(bShow);
}

bool ParaScene::IsHeadOnDisplayShown()
{
	return CGlobals::GetScene()->IsHeadOnDisplayShown();
}

int ParaScene::GetMaxRenderCount(int nRenderImportance)
{
	return CGlobals::GetScene()->GetMaxRenderCount(nRenderImportance);
}

void ParaScene::SetMaxRenderCount(int nRenderImportance, int nCount)
{
	CGlobals::GetScene()->SetMaxRenderCount(nRenderImportance, nCount);
}

bool ParaScene::GetScreenPosFrom3DPoint(float x, float y, float z, const object& output)
{
	bool bVisible = false;
	Vector3 vPos(x,y,z);

	Vector3 vEyePos =  CGlobals::GetScene()->GetCurrentCamera()->GetEyePosition();
	float fDist = (vEyePos-vPos).squaredLength();
	if(fDist>0.01f)
	{
		fDist = sqrt(fDist);
	}
	output["distance"] = fDist;

	vPos -= CGlobals::GetScene()->GetRenderOrigin();
	
	if(CGlobals::GetScene()->GetCurrentCamera()->GetObjectFrustum()->CullPointsWithFrustum(&vPos, 1))
	{
		bVisible = true;
		Vector3 vOut;
		ParaViewport  viewport;
		CGlobals::GetViewportManager()->GetCurrentViewport(viewport);
		ParaVec3Project(&vOut, &vPos, &viewport, CGUIRoot::GetInstance()->Get3DViewProjMatrix(), NULL, NULL);
		output["visible"] = true;
		// scale by UI scaling. 
		output["x"] = vOut.x / CGUIRoot::GetInstance()->GetUIScalingX();
		output["y"] = vOut.y / CGUIRoot::GetInstance()->GetUIScalingY();
		output["z"] = vOut.z;
	}
	return 	bVisible;
}

int ParaScene::LoadNPCsByRegion( float min_x, float min_y, float min_z, float max_x, float max_y, float max_z, bool bReload )
{
	Vector3 vMin(min_x, min_y, min_z);
	Vector3 vMax(max_x, max_y, max_z);
	return CGlobals::GetScene()->DB_LoadNPCsByRegion(vMin, vMax, bReload);
}

void ParaScene::SetCharacterRegionPath(int slot, const std::string& path)
{
	using namespace ParaEngine;
	return ParaEngine::CCharCustomizeSysSetting::GetSingleton().SetCharacterRegionPath(slot, path);
}

void ParaScene::SetCharTextureSize( int nCharacterTexSize, int nCartoonFaceTexSize )
{
	ParaEngine::CCharCustomizeSysSetting::GetSingleton().SetCharTextureSize(nCharacterTexSize, nCartoonFaceTexSize);
}

void ParaScene::SetCharRegionCoordinates( int nRegionIndex, int xpos, int ypos, int xsize, int ysize )
{
	ParaEngine::CCharCustomizeSysSetting::GetSingleton().SetCharRegionCoordinates(nRegionIndex, xpos, ypos, xsize, ysize);
}

ParaScripting::ParaObject ParaScene::CreateObject(const char* strType, const char * strObjectName, double x, double y, double z)
{
	CBaseObject* pObj = reinterpret_cast<CBaseObject*>(CAttributesManager::GetSingleton().CreateObject(strType));
	if (pObj)
	{
		pObj->SetIdentifier(strObjectName);
		DVector3 vPos(x, y, z);
		pObj->SetPosition(vPos);
		return ParaObject(pObj);
	}
	else
	{
		OUTPUT_LOG("warning: failed to create unregistered object of type %s \n", strType);
	}
	return ParaObject();
}

//--------------------------------------------------------------
// for ParaCamera namespace table.
//--------------------------------------------------------------
ParaAttributeObject ParaCamera::GetAttributeObject()
{
	return ParaAttributeObject(CGlobals::GetScene()->GetCurrentCamera());
}
void ParaCamera::GetAttributeObject_(ParaAttributeObject& output)
{
	output = GetAttributeObject();
}
void ParaCamera::FollowObject(ParaObject obj)
{
	if(obj.m_pObj)
	{
		if(obj.m_pObj->IsBiped())
			CGlobals::GetScene()->SetCurrentPlayer((CBipedObject*)obj.m_pObj);
		((CAutoCamera*)(CGlobals::GetScene()->GetCurrentCamera()))->SetFollowTarget(obj.m_pObj);
	}
}

void ParaCamera::FollowObject(const char* strObjectName)
{
	ParaObject obj = (ParaScene::GetObject(strObjectName));
	FollowObject(obj);
}

void ParaCamera::FirstPerson(int nTransitionMode, float fRadius, float fAngle)
{
	((CAutoCamera*)(CGlobals::GetScene()->GetCurrentCamera()))->FollowBiped(NULL, nTransitionMode, CameraFollowFirstPerson, fRadius, fAngle, 0);
}

void ParaCamera::ThirdPerson(int nTransitionMode, float fRadius, float fFacing, float fAngle)
{
	((CAutoCamera*)(CGlobals::GetScene()->GetCurrentCamera()))->FollowBiped(NULL, nTransitionMode, CameraFollowThirdPerson, fRadius, fFacing, fAngle);
}
void ParaCamera::Default(int nTransitionMode, float fHeight, float fAngle)
{
	((CAutoCamera*)(CGlobals::GetScene()->GetCurrentCamera()))->FollowBiped(NULL, nTransitionMode, CameraFollowDefault, fHeight, fAngle, 0);
}
void ParaCamera::Fixed(int nTransitionMode, float fX, float fY, float fZ)
{
	((CAutoCamera*)(CGlobals::GetScene()->GetCurrentCamera()))->FollowBiped(NULL, nTransitionMode, CameraCameraFirstPerson, fX, fY, fZ);
}

void ParaCamera::GetPosition(float* x, float* y, float* z)
{
	Vector3 v;
	v = CGlobals::GetScene()->GetCurrentCamera()->GetEyePosition();
	*x = v.x;*y = v.y;*z = v.z;
}

void ParaCamera::GetLookAtPosition(float* x, float* y, float* z)
{
	Vector3 v;
	v = CGlobals::GetScene()->GetCurrentCamera()->GetLookAtPosition();
	*x = v.x;*y = v.y;*z = v.z;
}

void ParaCamera::SetKeyMap( int key, int scancode )
{
	CGlobals::GetScene()->GetCurrentCamera()->SetKeyMap((CharacterAndCameraKeys)key, (EVirtualKey)scancode);
}

int ParaCamera::GetKeyMap( int key )
{
	return (int)CGlobals::GetScene()->GetCurrentCamera()->GetKeyMap((CharacterAndCameraKeys)key);
}

//////////////////////////////////////////////////////////////////////////
//
// ParaMiniSceneGraph
//
//////////////////////////////////////////////////////////////////////////

ParaMiniSceneGraph::~ParaMiniSceneGraph()
{

}

void ParaMiniSceneGraph::EnableCamera( bool bEnable )
{
	if(IsValid())
	{
		m_pSceneGraph->EnableCamera(bEnable);
	}
}

bool ParaMiniSceneGraph::IsCameraEnabled()
{
	if(IsValid())
	{
		return m_pSceneGraph->IsCameraEnabled();
	}
	return false;
}

//void ParaMiniSceneGraph::UpdateCameraParam( float lookat_x, float lookat_y, float lookat_z, float eyeat_x, float eyeat_y, float eyeat_z )
//{
//	if(IsValid())
//	{
//		m_pSceneGraph->UpdateCameraParam(lookat_x, lookat_y, lookat_z, eyeat_x, eyeat_y, eyeat_z);
//	}
//}
//
//void ParaMiniSceneGraph::GetCameraParam( float* lookat_x_, float* lookat_y_, float* lookat_z_, float* eyeat_x_, float* eyeat_y_, float* eyeat_z_ )
//{
//	if(IsValid())
//	{
//		float lookat_x, lookat_y, lookat_z, eyeat_x, eyeat_y, eyeat_z;
//		m_pSceneGraph->GetCameraParam(&lookat_x, &lookat_y, &lookat_z, &eyeat_x, &eyeat_y, &eyeat_z);
//		*lookat_x_ = lookat_x;
//		*lookat_y_ = lookat_y;
//		*lookat_z_ = lookat_z;
//		*eyeat_x_ = eyeat_x;
//		*eyeat_y_ = eyeat_y;
//		*eyeat_z_ = eyeat_z;
//	}
//}

ParaScripting::ParaObject ParaMiniSceneGraph::GetObject( const char* name )
{
	if(IsValid() && name!=0)
	{
		return ParaObject(m_pSceneGraph->GetObject(name));
	}
	return ParaObject(NULL);
}

int ParaMiniSceneGraph::DestroyObject( const char* name )
{
	if(IsValid())
	{
		return m_pSceneGraph->DestroyObject(name);
	}
	return 0;
}

int ParaMiniSceneGraph::DestroyObject_(const ParaObject& obj)
{
	if(IsValid() && obj.m_pObj)
	{
		return m_pSceneGraph->DestroyObject(obj.get());
	}
	return 0;
}

void ParaMiniSceneGraph::Reset()
{
	if(IsValid())
	{
		m_pSceneGraph->Reset();
	}
}

void ParaMiniSceneGraph::AddChild( const ParaObject obj )
{
	if(IsValid() && obj.IsValid())
		m_pSceneGraph->AddChild(obj.get());
}

const char* ParaMiniSceneGraph::GetName()
{
	if(IsValid())
		return m_pSceneGraph->GetName();
	return NULL;
}

void ParaMiniSceneGraph::SetName( const char* sName )
{
	if(IsValid())
		m_pSceneGraph->SetName(sName);
}

ParaObject ParaMiniSceneGraph::GetObject3( float x, float y, float z )
{
	if(IsValid())
	{
		return ParaObject(m_pSceneGraph->GetObject(x,y,z));
	}
	return ParaObject(NULL);
}

ParaObject ParaMiniSceneGraph::GetObject4( float x, float y, float z , float fEpsilon)
{
	if(IsValid())
	{
		return ParaObject(m_pSceneGraph->GetObject(x,y,z, fEpsilon));
	}
	return ParaObject(NULL);
}

int ParaMiniSceneGraph::RemoveObject( const char* name )
{
	if(IsValid())
	{
		return m_pSceneGraph->RemoveObject(name);
	}
	return 0;
}

int ParaMiniSceneGraph::RemoveObject_( const ParaObject& obj )
{
	if(IsValid() && obj.m_pObj)
	{
		return m_pSceneGraph->RemoveObject(obj.get());
	}
	return 0;
}

bool ParaMiniSceneGraph::IsVisible()
{
	if(IsValid())
	{
		return m_pSceneGraph->IsVisible();
	}
	return false;
}

void ParaMiniSceneGraph::SetVisible( bool bVisible )
{
	if(IsValid())
	{
		m_pSceneGraph->SetVisibility(bVisible);
	}
}

void ParaMiniSceneGraph::SetActor( const ParaObject pActor )
{
	if(IsValid())
	{
		m_pSceneGraph->SetActor(pActor.get());
	}
}

ParaScripting::ParaObject ParaMiniSceneGraph::GetActor()
{
	return ParaScripting::ParaObject(IsValid()?m_pSceneGraph->GetActor():NULL);
}

ParaScripting::ParaAssetObject ParaMiniSceneGraph::GetTexture()
{
	return ParaScripting::ParaAssetObject(IsValid()?m_pSceneGraph->GetTexture():NULL);
}

void ParaMiniSceneGraph::CameraZoom( float fAmount )
{
	if(IsValid())
	{
		m_pSceneGraph->CameraZoom(fAmount);
	}
}

void ParaMiniSceneGraph::CameraZoomSphere( float center_x, float center_y, float center_z, float raduis)
{
	if(IsValid())
	{
		m_pSceneGraph->CameraZoomSphere(CShapeSphere(Vector3(center_x,center_y,center_z), raduis));
	}
}

void ParaMiniSceneGraph::CameraRotate( float dx, float dy, float dz )
{
	if(IsValid())
	{
		m_pSceneGraph->CameraRotate(dx, dy, dz);
	}
}

void ParaMiniSceneGraph::CameraPan( float dx, float dy )
{
	if(IsValid())
	{
		m_pSceneGraph->CameraPan(dx, dy);
	}
}

void ParaMiniSceneGraph::Draw( float fDeltaTime )
{
	if(IsValid())
	{
		if(CGlobals::GetRenderDevice() && SUCCEEDED( CGlobals::GetRenderDevice()->BeginScene() ) )
		{
			m_pSceneGraph->Draw(fDeltaTime);

			CGlobals::GetRenderDevice()->EndScene();
		}
	}
}

void ParaMiniSceneGraph::SaveToFile( const char* sFileName, int nImageSize /*= 0*/ )
{
	if(IsValid())
	{
		m_pSceneGraph->SaveToFile(sFileName, nImageSize);
	}
}

void ParaMiniSceneGraph::SaveToFileEx(const char* sFileName, int width /*= 0*/, int height /*= 0*/, DWORD dwFormat /*= 3*/, UINT MipLevels /*= 0*/, int srcLeft /*= 0*/, int srcTop /*= 0*/, int srcWidth /*= 0*/, int srcHeight /*= 0*/)
{
	if (IsValid())
	{
		m_pSceneGraph->SaveToFile(sFileName, width, height, dwFormat, MipLevels, srcLeft, srcTop, srcWidth, srcHeight);
	}
}

void ParaMiniSceneGraph::SetRenderTargetSize( int nWidth, int nHeight )
{
	if(IsValid())
	{
		m_pSceneGraph->SetRenderTargetSize(nWidth, nHeight);
	}
}

void ParaMiniSceneGraph::SetMaskTexture( ParaAssetObject pTexture )
{
	if(IsValid())
	{
		m_pSceneGraph->SetMaskTexture((TextureEntity*)pTexture.m_pAsset);
	}
}

void ParaMiniSceneGraph::EnableActiveRendering( bool bEnable )
{
	if(IsValid())
	{
		m_pSceneGraph->EnableActiveRendering(bEnable);
	}
}

bool ParaMiniSceneGraph::IsActiveRenderingEnabled()
{
	return (IsValid()?m_pSceneGraph->IsActiveRenderingEnabled():false);
}

ParaScripting::ParaObject ParaMiniSceneGraph::MousePick( float x, float y, float fMaxDistance, const char* sFilterFunc )
{
	CBaseObject* pObj=NULL;
	OBJECT_FILTER_CALLBACK pFilterFunc = GetFilterFuncByName(sFilterFunc);

	if(IsValid())
	{
		m_pSceneGraph->PickObject((int)x, (int)y, &pObj, fMaxDistance, pFilterFunc);
	}
	
	return ParaObject(pObj);
}

void ParaMiniSceneGraph::CameraSetLookAtPos( float x, float y, float z )
{
	if(IsValid())
	{
		m_pSceneGraph->CameraSetLookAtPos(x,y,z);
	}
}

void ParaMiniSceneGraph::CameraGetLookAtPos( float* x, float* y, float* z )
{
	if(IsValid())
	{
		m_pSceneGraph->CameraGetLookAtPos(x,y,z);
	}
}

void ParaMiniSceneGraph::CameraSetEyePosByAngle( float fRotY, float fLiftupAngle, float fCameraObjectDist )
{
	if(IsValid())
	{
		m_pSceneGraph->CameraSetEyePosByAngle(fRotY,fLiftupAngle, fCameraObjectDist);
	}
}

void ParaMiniSceneGraph::CameraGetEyePosByAngle( float* fRotY, float* fLiftupAngle, float* fCameraObjectDist )
{
	if(IsValid())
	{
		m_pSceneGraph->CameraGetEyePosByAngle(fRotY,fLiftupAngle, fCameraObjectDist);
	}
}

void ParaMiniSceneGraph::CameraSetEyePos(double x, double y, double z)
{
	if(IsValid())
	{
		m_pSceneGraph->CameraSetEyePos(x,y,z);
	}
}

void ParaMiniSceneGraph::CameraGetEyePos(double* x, double* y, double* z)
{
	if(IsValid())
	{
		m_pSceneGraph->CameraGetEyePos(x,y,z);
	}
}

void ParaMiniSceneGraph::ShowHeadOnDisplay( bool bShow )
{
	if(IsValid())
	{
		m_pSceneGraph->ShowHeadOnDisplay(bShow);
	}
}

bool ParaMiniSceneGraph::IsHeadOnDisplayShown()
{
	if(IsValid())
	{
		return m_pSceneGraph->IsHeadOnDisplayShown();
	}
	return false;
}

ParaAttributeObject ParaMiniSceneGraph::GetAttributeObject()
{
	return ParaAttributeObject(m_pSceneGraph);
}

void ParaMiniSceneGraph::GetAttributeObject_(ParaAttributeObject& output)
{
	output = GetAttributeObject();
}

void ParaMiniSceneGraph::EnableLighting(bool bEnable)
{
	if(IsValid())
		m_pSceneGraph->EnableLight(bEnable);
}

bool ParaMiniSceneGraph::IsLightingEnabled()
{
	return IsValid() ? m_pSceneGraph->IsLightEnabled() : false;
}

void ParaMiniSceneGraph::SetFog(bool bRenderFog, const char* strFogColor, float fFogStart,float fFogEnd, float fFogDensity)
{
	if(!IsValid())
		return;
	if(bRenderFog)
	{
		m_pSceneGraph->EnableFog(bRenderFog);

		LinearColor colorFog;
		if(sscanf(strFogColor, "%f %f %f", &colorFog.r,&colorFog.g,&colorFog.b)<3)
			colorFog = LinearColor(1,1,1,1);

		m_pSceneGraph->SetFogColor(colorFog);
		m_pSceneGraph->SetFogStart(fFogStart);
		m_pSceneGraph->SetFogEnd(fFogEnd);
		m_pSceneGraph->SetFogDensity(fFogDensity);

	}
	else
	{
		m_pSceneGraph->EnableFog(false);
	}
}

bool ParaMiniSceneGraph::CreateSkyBox(const char* strObjectName, const char* strMeshAssetName, float fScaleX, float fScaleY,float fScaleZ, float fHeightOffset)
{
	if(!IsValid())
		return false;
	MeshEntity* pMesh =  CGlobals::GetAssetManager()->GetMesh(strMeshAssetName);
	ParaAssetObject obj(pMesh);
	return CreateSkyBox_(strObjectName, obj, fScaleX, fScaleY, fScaleZ, fHeightOffset);	
}
bool ParaMiniSceneGraph::CreateSkyBox_(const char* strObjectName, ParaAssetObject& asset, float fScaleX, float fScaleY,float fScaleZ, float fHeightOffset)
{
	if(!IsValid())
		return false;
	if(asset.IsValid())
		return m_pSceneGraph->CreateSkyBox(strObjectName, *(asset.m_pAsset), fScaleX, fScaleY, fScaleZ, fHeightOffset);	
	else
		return false;
}

void ParaMiniSceneGraph::DeleteSkyBox(const char* strObjectName)
{
	if(IsValid())
		m_pSceneGraph->DeleteSkyBox(strObjectName);
}

void ParaMiniSceneGraph::SetTimeOfDaySTD(float time)
{
	if(IsValid())
		m_pSceneGraph->GetSunLight().SetTimeOfDaySTD(time);
}

float ParaMiniSceneGraph::GetTimeOfDaySTD()
{
	return IsValid() ? m_pSceneGraph->GetSunLight().GetTimeOfDaySTD() : 0.f;
}

void ParaMiniSceneGraph::SetBackGroundColor( const char* rgba )
{
	int r=255,g=255,b=255,a=255;		
	int nCount = sscanf(rgba, "%d %d %d %d", &r,&g,&b,&a);
	LinearColor color = COLOR_ARGB(a,r,g,b);

	if(IsValid())
		m_pSceneGraph->SetBackGroundColor(color);
}

void ParaMiniSceneGraph::DestroyChildren()
{
	if(IsValid())
		m_pSceneGraph->DestroyChildren();
}

ParaScripting::ParaAttributeObject ParaMiniSceneGraph::GetAttributeObjectCamera()
{
	return ParaAttributeObject(IsValid() ? m_pSceneGraph->GetCamera() : NULL);
}

ParaScripting::ParaAttributeObject ParaMiniSceneGraph::GetAttributeObject1(const char* name)
{
	if (IsValid())
	{
		if (name == NULL || name[0] == '\0')
		{
			return ParaAttributeObject(m_pSceneGraph);
		}
		else if (strcmp(name, "camera") == 0)
		{
			return ParaAttributeObject(m_pSceneGraph->GetCamera());
		}
		else if (strcmp(name, "sky") == 0)
		{
			return ParaAttributeObject(m_pSceneGraph->GetCurrentSky());
		}
		else if (strcmp(name, "sun") == 0)
		{
			return ParaAttributeObject(&(m_pSceneGraph->GetSunLight()));
		}
	}
	return ParaAttributeObject();
}

}//namespace ParaScripting

// for LuaJit, only for function that maybe called millions of time per second
extern "C" {
	PE_CORE_DECL bool ParaScene_CheckExist(int nID)
	{
		return ParaScripting::ParaScene::CheckExist(nID);
	}
};

//-----------------------------------------------------------------------------
// Class:	CBipedObject
// Authors:	Li, Xizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2004.3.8
// Revised: 2005.10.8
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "AnimInstanceBase.h"
#include "ParaWorldAsset.h"
#include "ViewCullingObject.h"
#include "DummyAnimInstance.h"
#include "SceneObject.h"
#include "AutoCamera.h"
#include "PhysicsWorld.h"
#include "IEnvironmentSim.h"
#include "terrain/GlobalTerrain.h"
#include "AIModuleNPC.h"
#include "BlockEngine/BlockWorldClient.h"
#include "BlockEngine/BlockCommon.h"
#include "OceanManager.h"
#ifdef USE_DIRECTX_RENDERER
#include "BVHSerializer.h"
#endif
#include "ParaXModel/ParaXModel.h"
#include "ParaXModel/BoneAnimProvider.h"
#include "CustomCharModelInstance.h"
#include "ParaXAnimInstance.h"
#include "BipedStateManager.h"
#include "BipedObject.h"
#include "ShapeOBB.h"
#include "ShapeAABB.h"
#include "PhysicsWorld.h"
#include "DynamicAttributeField.h"
#include "NPL/NPLHelper.h"
#include <algorithm>

using namespace ParaEngine;

/** @def whether to use smooth mounting facing angle */
#define SMOOTH_MOUNTING_FACING
/** @def whether to use smooth mounting position angle */
//#define SMOOTH_MOUNTING_POS
/** @def position follow up speed for mounted character */
#define MOUNT_POS_SPEED	 6.0f
/** @def facing angle follow up speed for mounted character */
#define MOUNT_FACING_SPEED	 2.0f

/** @def speed for body speed angle in rads. This is used in command type way points moving*/
#define BODY_TURNING_SPEED	 1.0f

/**@def if this macro is defined, collision detection between bipeds will be ignored.*/
#define TURNOFF_BIPED_COLLISION

#define GRAVITY_CONSTANT 9.81f

/** @def the ratio between the sensor ray height and the biped's physical height.*/
#define SENSOR_HEIGHT_RATIO	0.5f

/// @def it is assumed that no character will ever go down below this world height.
#define LOWEST_WORLD	-1000.f
/// @def it is assumed that no object will exceed this length
#define MAX_DIST		1000.f
/// @def the maximum penetration distance allowed. In each simulation step, if the biped moves
/// longer than this step, it is further divided in to several evenly spaced steps. 
/// the step length is determined by the biped's speed multiplied by the time delta.
/// See also NX_MIN_SEPARATION_FOR_PENALTY, and CBipedObject::MoveTowards()
#define PENETRATION_DISTANCE 0.10f
/// @def ignore PENETRATION_DISTANCE, if it takes longer than MAX_STEPS_PER_FRAME per frame.
/// Instead, it will take only one step for the time stamp.
/// this is most likely in debug mode, when the character is flying with lightening speed.
/// however, it will produce very inaccurate physics.
#define MAX_STEPS_PER_FRAME	10

/** @def the higher this number, the more accurate the collision detection will be
* the value must be 2n+1, such as 1,3,5,7,9,etc. Normally 3 will be both accurate and fast */
#define BIPED_SENSOR_RAY_NUM	3
/** @def the vertical speed at which the biped will fall down a shallow ground */
#define FALLDOWN_SPEED	4.0f
/** @def the vertical speed at which the biped will jump up on a shallow stair */
#define CLIMBUP_SPEED	2.0f
/** @def the speed at which the biped will rebound from a wall,assuming that the mass of characters are the same. This is proportional to rebound impulse*/
#define REBOUND_SPEED	0.3f
/** @def the biped will rebound from a wall, if its body penetrates into the wall for the specified distance*/
#define WALL_REBOUNCE_DISTANCE 0.05f

/** @def default physical body radius*/
#define BODY_RADIUS 0.4f

/** @def default body density */
#define DEFAULT_BODY_DENSITY	1.2f

/** @def water density */
#define WATER_DENSITY	1.f

/** @def fly density, below which the object will fly. */
#define FLY_DENSITY	0.5f

/** @def when a flying object's vertical speed is below -MAX_FLY_VERTICAL_SPEED, it will be reset to -MAX_FLY_VERTICAL_SPEED.*/
#define MAX_FLY_VERTICAL_SPEED	5.f

/** convert to vector struct */
#define CONVERT_PARAVECTOR3(x) *((PARAVECTOR3*)&(x))

/** @def groups Mask used to specify which groups of physics object this biped will collide with.
*/
#define DEFAULT_PHYSICS_GROUP_MASK 0xffffffff
/** @def define this to allow high speed biped to pass through physical walls. otherwise, we will slow down high speed object. */
//#define ALLOW_HIGH_SPEED_PASSTHROUGH

/** @def the default character jump up speed.*/
#define JUMPUP_SPEED 5.0f


namespace ParaEngine{
	extern int64_t globalTime;

	static const int g_MountIDs[] = { 0, ATT_ID_MOUNT1, ATT_ID_MOUNT2, ATT_ID_MOUNT3, ATT_ID_MOUNT4, ATT_ID_MOUNT5, ATT_ID_MOUNT6, ATT_ID_MOUNT7, ATT_ID_MOUNT8, ATT_ID_MOUNT9,
		ATT_ID_MOUNT10, ATT_ID_MOUNT11, ATT_ID_MOUNT12, ATT_ID_MOUNT13, ATT_ID_MOUNT14, ATT_ID_MOUNT15, ATT_ID_MOUNT16, ATT_ID_MOUNT17, ATT_ID_MOUNT18, ATT_ID_MOUNT19, ATT_ID_MOUNT20, };

	const float CBipedObject::SPEED_TURN = 2.7f;
	/**@def default biped walking speed. */
	const float CBipedObject::SPEED_WALK = 0.9f;
	/**@def default biped normal turning speed. */
	const float CBipedObject::SPEED_NORM_TURN = 0.7f;
}


//-----------------------------------------------------------------------------
// Name: CBipedObject::CBipedObject
// Desc: Constructor for CBipedObject
//-----------------------------------------------------------------------------
CBipedObject::CBipedObject() :
m_fSpeed(0.f),
m_fSpeedAngle(0.f),
m_fSpeedVertical(0.f),
m_vNorm(0, 1.f, 0),
m_vNormTarget(0, 1.f, 0),
m_fPitch(0.f), m_fRoll(0.f),
m_fFacingTarget(0),
m_bIgnoreFacingTarget(true),
m_pAIModule(NULL),
m_pBipedStateManager(NULL),
m_fPhysicsRadius(BODY_RADIUS),
m_fPhysicsHeight(0.f),
m_fDensity(DEFAULT_BODY_DENSITY),
m_bIsShadowEnabled(true),
m_bCanAnimOpacity(true),
m_fAssetHeight(0.f),
m_isFlyUsingCameraDir(true),
m_nMovementStyle(MOVESTYLE_SLIDINGWALL),
m_dwPhysicsGroupMask(DEFAULT_PHYSICS_GROUP_MASK),
m_dwPhysicsMethod(PHYSICS_FORCE_NO_PHYSICS), m_nPhysicsGroup(0),
m_sPhysicsShape("box"),
m_dynamicPhysicsActor(NULL),
m_fBootHeight(0.f),
m_fSizeScale(1.0f),
m_fObjectToCameraDistance(0.f), 
m_pLocalTransfrom(NULL),
m_fSpeedScale(1.0f), m_bIsAlwaysAboveTerrain(true), m_bPauseAnimation(false),
m_gravity(9.18f), m_ignoreSlopeCollision(false), m_readyToLanding(false), m_canFly(false), m_isAlwaysFlying(false), m_bAutoWalkupBlock(true), m_bIsControlledExternally(false),
m_isFlying(false), m_flyingDir(1, 0, 0), m_fLastBlockLight(0.f), m_dwLastBlockHash(0), m_fAccelerationDist(0), m_fLastSpeed(0.f)
{
	SetMyType(_Biped);
	ForceStop();		// default action is loiter 
}

//-----------------------------------------------------------------------------
// Name: CBipedObject::~CBipedObject
// Desc: Destructor for CBipedObject
//-----------------------------------------------------------------------------
CBipedObject::~CBipedObject()
{
	UnloadPhysics();
	UnloadDynamicPhysics();
	SAFE_DELETE(m_pBipedStateManager);
	SAFE_DELETE(m_pLocalTransfrom);
}

void CBipedObject::SetUseGlobalTime(bool bUseGlobalTime)
{
	CAnimInstanceBase * pAI = GetAnimInstance();
	if (pAI)
	{
		pAI->SetUseGlobalTime(bUseGlobalTime);
	}
}

bool CBipedObject::IsUseGlobalTime()
{
	CAnimInstanceBase * pAI = GetAnimInstance();
	if (pAI)
	{
		return pAI->IsUseGlobalTime();
	}
	return false;
}

std::string CBipedObject::ToString(DWORD nMethod)
{
#ifndef MAX_LINE
#define MAX_LINE	500
#endif
	/** just a comment line with its name. */
	string sScript;
	char line[MAX_LINE + 1];
	snprintf(line, MAX_LINE, "-- %s\n", m_sIdentifer.c_str());
	sScript.append(line);
	CharModelInstance* pChar = GetCharModelInstance();
	CAnimInstanceBase * pAI = GetAnimInstance();
	if (pChar && pAI)
	{
		if (pChar->GetBaseModel())
		{
			snprintf(line, MAX_LINE, "local asset = ParaAsset.LoadParaX(\"\", [[%s]]);\n",
				pChar->GetBaseModel()->GetKey().c_str());
			sScript.append(line);
			string sIsGlobal = IsGlobal() ? "true" : "false";
			snprintf(line, MAX_LINE, "local player = ParaScene.CreateCharacter (\"%s\", asset, \"\", %s, %f, %f, %f);\n",
				m_sIdentifer.c_str(), sIsGlobal.c_str(), GetPhysicsRadius(), GetFacing(), pAI->GetSizeScale());
			sScript.append(line);
			const Vector3& vPos = GetPosition();
			snprintf(line, MAX_LINE, "player:SetPosition(%f, %f, %f);player:SetFacing(%f);ParaScene.Attach(player);\nlocal playerChar = player:ToCharacter();\n",
				vPos.x, vPos.y, vPos.z, GetFacing());
			sScript.append(line);
			if (fabs(pAI->GetSizeScale() - 1.0f) > 0.0001f)
			{
				snprintf(line, MAX_LINE, "playerChar:SetSizeScale(%f);\n", pAI->GetSizeScale());
				sScript.append(line);
			}
			if (m_fPhysicsHeight > 0.f)
			{
				snprintf(line, MAX_LINE, "player:SetPhysicsHeight(%f);\n", GetPhysicsHeight());
				sScript.append(line);
			}

			if (pChar->m_bIsCustomModel)
			{
				if (pChar->nSetID > 0)
				{
					snprintf(line, MAX_LINE, "playerChar:LoadStoredModel(%d);\n", pChar->nSetID);
					sScript.append(line);
				}
				else
				{
					// TODO: save and load from equipment file.
				}
			}
			else
			{
				// set skin for creature model
				snprintf(line, MAX_LINE, "playerChar:SetSkin(%d);\n", pChar->m_skinIndex);
				sScript.append(line);
			}

			// if it is not the default animation, we will set it in the script.
			if (pAI->GetCurrentAnimation() != 0)
			{
				snprintf(line, MAX_LINE, "playerChar:PlayAnimation(%d);\n", pAI->GetCurrentAnimation());
				sScript.append(line);
			}
		}
	}
	return sScript;
}

void CBipedObject::DeleteAnimInstance()
{
	if (m_pAI && m_pMultiAnimationEntity)
	{
		if (m_pMultiAnimationEntity->GetType() == AssetEntity::parax)
		{
			// manually clear this resource
			m_pAI.reset();
		}
	}
}

void CBipedObject::Cleanup()
{
	DeleteAnimInstance();
	ReplaceAIModule(NULL);
	SAFE_DELETE(m_pBipedStateManager);
}

CBipedStateManager*  CBipedObject::GetBipedStateManager(bool bCreateOnDemand)
{
	if (bCreateOnDemand && m_pBipedStateManager == NULL)
	{
		m_pBipedStateManager = new CBipedStateManager(this);
	}
	return m_pBipedStateManager;
}

/// pNew can be NULL,in which case the old module is deleted
void CBipedObject::ReplaceAIModule(CAIBase* pNew)
{
	if (m_pAIModule != pNew)
	{
		SAFE_DELETE(m_pAIModule);
		m_pAIModule = pNew;
	}
}
CAIBase*	CBipedObject::GetAIModule()
{
	return m_pAIModule;
}

CAIBase* CBipedObject::UseAIModule(const string& sAIType)
{
	if (sAIType == "" || sAIType == "NPC")
	{
		if (!GetAIModule() || GetAIModule()->GetType() != CAIBase::NPC)
			ReplaceAIModule(new CAIModuleNPC(this));
	}
	else if (sAIType == "NULL")
	{
		ReplaceAIModule(NULL);
	}
	return GetAIModule();
}

/// the size and speed of the animation and the biped object is not synchronized.
/// you need to manually get the current size and speed from the animation instance
/// so that the biped and its animation instance could synchronize in action.
CAnimInstanceBase* CBipedObject::GetAnimInstance()
{
	if (!m_pAI)
	{
		if (m_pMultiAnimationEntity)
		{
			if (m_pMultiAnimationEntity->GetType() == AssetEntity::parax)
			{
				m_pAI = ((ParaXEntity*)m_pMultiAnimationEntity.get())->CreateAnimInstance();
				if (m_pAI)
				{
					/** load the default animation */
					m_pAI->LoadDefaultStandAnim(&m_fSpeed);
					/// if there is no Stand animation, just call ForceStop to set the speed to zero
					ForceStop();
				}
			}

		}
		else
		{
			m_pAI = CDummyAnimInstance::GetInstance();
		}
	}
	return m_pAI.get();
}

CParaXAnimInstance* CBipedObject::GetParaXAnimInstance()
{
	CAnimInstanceBase* pAI = GetAnimInstance();
	if (pAI && m_pMultiAnimationEntity)
	{
		if (m_pMultiAnimationEntity->GetType() == AssetEntity::parax)
		{
			return (CParaXAnimInstance*)pAI;
		}
	}
	return NULL;
}

CharModelInstance* CBipedObject::GetCharModelInstance()
{
	CParaXAnimInstance* pAI = GetParaXAnimInstance();
	if (pAI)
	{
		CharModelInstance * pChar = pAI->GetCharModel();
		if (pChar)
		{
			return pChar;
		}
	}
	return NULL;
}

//-----------------------------------------------------------------------------
// Name: CBipedObject::InitObject
/// Desc: It saves device references, but do not initialize those devices.
///		 it only does initialization not concerning device object. Object dependent
///       initialization is done with InitDeviceObjects().
//-----------------------------------------------------------------------------
HRESULT CBipedObject::InitObject(AssetEntity* pMAE)
{
	ResetBaseModel(pMAE);

	return S_OK;
}

struct HLEToken
{
	int nStart;
	int nLength;
	enum tokenType{ str, value }
	myType;
	union
	{
		char cmd[4];
		double dValue;
		DWORD dw;
	};
public:
	HLEToken(){};
	HLEToken(const char* cmd_){
		if (cmd_){
			for (int i = 0; i < 4; ++i)
				cmd[i] = cmd_[i];
		}
	};

	tokenType GetType() { return myType; }

	void GetString(std::string& outValue, const char* event)
	{
		outValue.clear();
		char cEnd = event[nStart + nLength - 1];
		/*nLength-1 to remove the last blank*/
		if (cEnd == ' ' || cEnd == ';')
			nLength--;
		outValue.append(event + nStart, nLength);
	};

	DWORD GetCmd() const
	{
#define DWORD_TAG(x) (DWORD)(  (((DWORD)x&0x0000ff00)<<8)+(((DWORD)x&0x000000ff)<<24)+(((DWORD)x&0x00ff0000)>>8)+(((DWORD)x&0xff000000)>>24) )
		return DWORD_TAG(dw);
	};
};

/// get the token in string event starting from position nStart
/// pToken[out]: saved to this place
/// return: true if a new token is read
bool GetHLEToken(const char * event, HLEToken* pToken, int* nStart)
{
	int state = 0; // 0 start, 1: middle, 2: out
	int nDataIndex = 0;
	int i;
	for (i = *nStart; (event[i] != '\0') && (state != 2); i++)
	{
		if (state == 0)
		{
			if (event[i] != ' ')
			{
				state = 1;
				if (isdigit(event[i]))
					pToken->myType = HLEToken::value;
				else
					pToken->myType = HLEToken::str;
			}
		}
		if (state == 1)
		{
			if (nDataIndex < 4)
				pToken->cmd[nDataIndex++] = event[i];
			if (event[i] == ' ' || event[i] == ';')
				state = 2;
		}
	}
	if (state == 0)
		return false;
	pToken->nStart = *nStart;
	pToken->nLength = i - *nStart;
	*nStart = i;

	if (pToken->myType == HLEToken::value)
	{
		string sValue;
		pToken->GetString(sValue, event);
		pToken->dValue = atof(sValue.c_str());
	}
	return true;
}

// return false if there is no way point in the way point queue
bool CBipedObject::GetWayPoint(BipedWayPoint* pOut)
{
	if (m_vPosTarget.empty())
		return false;
	else
	{
		*pOut = m_vPosTarget.back();
	}
	return true;
}
BipedWayPoint& CBipedObject::GetLastWayPoint()
{
	static BipedWayPoint InvalidPT(BipedWayPoint::INVALID_WAYPOINT);
	if (m_vPosTarget.empty())
		return InvalidPT;
	else
		return m_vPosTarget.back();
}

void CBipedObject::AddWayPoint(const BipedWayPoint& pt)
{
	m_vPosTarget.push_back(pt);
}

// remove the current way point
void CBipedObject::RemoveWayPoint()
{
	if (!m_vPosTarget.empty())
		m_vPosTarget.pop_back();
}

void ParaEngine::CBipedObject::RemoveWayPointByType(BipedWayPoint::MyType nType/*=BipedWayPoint::COMMAND_POINT*/)
{
	for (WayPointList_type::iterator itCur = m_vPosTarget.begin(); itCur != m_vPosTarget.end();)
	{
		if (itCur->GetPointType() == nType)
		{
			itCur = m_vPosTarget.erase(itCur);
		}
		else
		{
			itCur++;
		}
	}
}

/// return true, if the biped is not only standing now, but also 
/// intends to remain standing for the next frame.
bool CBipedObject::IsStandingIntentioned()
{
	if (m_vPosTarget.empty())
		return IsStanding();
	if (IsStanding() &&
		(m_vPosTarget.front().GetPointType() != BipedWayPoint::COMMAND_POINT))
		return true;
	return false;
}

bool CBipedObject::IsStanding()
{
	return (GetSpeed() == 0.f);
}

//---------------------------------------------------------------------------
/// desc: this function is called by the environment simulator, when needed. 
/// The biped object then get an opportunity to shun from any obstacles or
/// try to solve collisions by generating additional way points. 
//---------------------------------------------------------------------------
void CBipedObject::PathFinding(double dTimeDelta)
{
#ifdef TURNOFF_BIPED_COLLISION
	return;
#endif
	/// rule1: we will not foresee any further, but implement path-finding only when 
	/// there is already collisions between this biped and the environment.
	if (GetNumOfPerceivedObject() == 0)
		return;

	BipedWayPoint waypoint;
	Vector3 vSrcPos = GetPosition();

	while (GetWayPoint(&waypoint))
	{
		/// rule2: if there have been collisions in the way, we will see whether we have already given 
		/// solutions in the previous path-finding process.If so, we will not generate new ones nor canceling it

		if ((waypoint.GetPointType() == BipedWayPoint::PATHFINDING_POINT) ||
			(waypoint.GetPointType() == BipedWayPoint::BLOCKED))
			return;

		/// rule 3: we will only generate a solution when the next way point is a command type point.
		if (waypoint.GetPointType() == BipedWayPoint::COMMAND_POINT)
		{
			// source and destination points
			Vector3 vDestPos = waypoint.vPos;

			{
				/**
				* already reached destination, we will go on to the next way point in the queue
				* when there is collision, we will regard a circle(with a radius) as the destination point.
				* while in free-collision condition, a destination is really a point.
				*/
				float fLength = (vDestPos - vSrcPos).squaredLength();
				if (fLength <= (dTimeDelta*GetAbsoluteSpeed())*(dTimeDelta*GetAbsoluteSpeed()) ||
					fLength <= (GetPhysicsRadius()*GetPhysicsRadius()))
				{
					RemoveWayPoint();
					continue;
				}
			}

			/**
			* See whether there are bipeds that are moving. If so, block the current biped for some seconds
			* or if the destination point(radius 0.01f*OBJ_UNIT)is inside one of the object, remove the way point
			* Get the biggest non-mobile object
			*/
			bool bReachedObject = false;
			bool bShouldBlock = false;
			CBaseObject* objBiggest = NULL;
			float fBiggestRadius = 0.f;
			{
				float fMyRadius = GetPhysicsRadius();

				int nNumPerceived = GetNumOfPerceivedObject();
				for (int i = 0; i < nNumPerceived; ++i)
				{
					IGameObject* pObj = GetPerceivedObject(i);
					if (pObj != NULL)
					{
						if (pObj->TestCollisionSphere(&vDestPos, fMyRadius/*0.01f*OBJ_UNIT*/, 0))
						{
							bReachedObject = true;
							break;
						}
						if (!pObj->IsStanding())
							bShouldBlock = true;
						if (!bShouldBlock && fBiggestRadius <= pObj->GetPhysicsRadius())
						{
							fBiggestRadius = pObj->GetPhysicsRadius();
							objBiggest = pObj;
						}
					}

				}
			}
			if (bReachedObject == true)
			{
				RemoveWayPoint();
				continue;
			}
			else if (bShouldBlock)
			{
				// TODO: set a reasonable value. Just wait 0.5 seconds
				AddWayPoint(BipedWayPoint(0.5f));
				continue; // actually this is equivalent to return;
			}

			/**
			* We will only solve against the biggest static object, we can give very precise solution
			* according to its shape, when there is only one object.
			*/
			if (objBiggest)
			{
				OneObstaclePathFinding(objBiggest);
				break;
			}
		}
		else if (waypoint.GetPointType() == BipedWayPoint::COMMAND_MOVING)
		{
			CBaseObject* objBiggest = NULL;
			float fBiggestRadius = 0.f;
			{
				float fMyRadius = GetPhysicsRadius();
				int nNumPerceived = GetNumOfPerceivedObject();
				for (int i = 0; i < nNumPerceived; ++i)
				{
					IGameObject* pObj = GetPerceivedObject(i);
					if (pObj != NULL)
					{
						if (fBiggestRadius <= pObj->GetPhysicsRadius())
						{
							fBiggestRadius = pObj->GetPhysicsRadius();
							objBiggest = pObj;
						}
					}
				}
			}
			if (objBiggest)
			{
				OneObstaclePathFinding(objBiggest);
				break;
			}
		}
		else
		{
			return;
		}
	}
}

//----------------------------------------------------------------------
/// give a precise solution, in which the biped is guaranteed to be approaching the
/// destination point. pSolid is the object that blocks the way.
//----------------------------------------------------------------------
void CBipedObject::OneObstaclePathFinding(CBaseObject* pSolid)
{
	BipedWayPoint waypoint;
	CBipedObject* pBiped = this;

	if (!GetWayPoint(&waypoint))
		return;

	Vector3 vDestPos;
	Vector3 vSrcPos = GetPosition();

	if (waypoint.GetPointType() == BipedWayPoint::COMMAND_MOVING)
	{
		Vector3 vFacing;
		GetSpeedDirection(&vFacing);
		vDestPos = vSrcPos + vFacing;
	}
	else
		vDestPos = waypoint.vPos;

	/// if the object can be moved directly to the target, then move it without adding additional points
	{
		Vector3 dS = vDestPos - vSrcPos;
		dS.y = 0;
		ParaVec3Normalize(&dS, &dS);
		dS *= 0.0001f;

		Vector3 S = vSrcPos;
		FLOAT depth1 = pSolid->GetSphereCollisionDepth(&S, GetPhysicsRadius(), true);
		Vector3 v = vSrcPos + dS;
		FLOAT depth2 = pSolid->GetSphereCollisionDepth(&v, GetPhysicsRadius(), true);
		if (depth2 < depth1)
		{
			// we will implement direct moving
			return;
		}
	}

	/// we will add path_finding points in order to reach the target.
	Vector3 vCenter = pSolid->GetPosition();
	FLOAT radius = pBiped->GetPhysicsRadius();

	switch (pSolid->GetObjectShape())
	{
	case _ObjectShape_Circle:
	case _ObjectShape_Sphere:
	{
		// TODO: path-finding for spherical object
		/// we will talk in the tangent of the object from the source position a fixed length(0.5units).
		float x0, y0; /* cos@ = x0, sin@ = y0,   solid object position relative to the source point*/
		float x1, y1; /* cos@` = x1, sin@` = y1,  destination position relative to the source point*/
		float tx, ty; /* cosm = x1, sinm = y1,  target position relative to the source point*/
		float fRadius;
		x1 = vDestPos.x - vSrcPos.x;
		y1 = vDestPos.z - vSrcPos.z;
		fRadius = sqrt(x1*x1 + y1*y1);
		x1 /= fRadius;/* automatically guaranteed to be above 0*/
		y1 /= fRadius;

		x0 = vCenter.x - vSrcPos.x;
		y0 = vCenter.z - vSrcPos.z;
		fRadius = sqrt(x0*x0 + y0*y0);
		x0 /= fRadius;/* automatically guaranteed to be above 0*/
		y0 /= fRadius;


		float fSinDiff = y0*x1 - x0*y1;/* sin(@-@`) = sin@cos@`-cos@sin@`*/
		if (fSinDiff < 0)
		{
			tx = -y0;
			ty = x0;
		}
		else
		{
			tx = y0;
			ty = -x0;
		}
		/* move aside a fixed length 0.5 unit.*/
		//fRadius *= 0.8f;
		tx = fRadius*tx + vSrcPos.x;
		ty = fRadius*ty + vSrcPos.z;
		//ty = -fRadius*ty + vSrcPos.z; // for left hand coordinate system

		/* try animate */
		if (waypoint.GetPointType() == BipedWayPoint::COMMAND_MOVING)
		{
			float fFacing;
			if (Math::ComputeFacingTarget(Vector3(tx, (float)(pBiped->m_vPos.y), ty), vSrcPos, fFacing))
				SetFacing(fFacing);
		}
		else
			AddWayPoint(BipedWayPoint(Vector3(tx, (float)(pBiped->m_vPos.y), ty), BipedWayPoint::PATHFINDING_POINT));

		break;
	}
	case _ObjectShape_Rectangular:
	case _ObjectShape_Box:
	{
		// we will further test if the rough test returns true.
		Vector3 vAimPos = vDestPos - vCenter;
		Vector3 vPos = (pBiped->m_vPos) - vCenter;		// relative position		

		//-- make transform
		FLOAT fWidth, fHeight, fFacing;
		pSolid->GetBoundRect(&fWidth, &fHeight, &fFacing);
		Matrix4 m;
		ParaVec3TransformCoord(&vPos, &vPos, ParaMatrixRotationY(&m, -fFacing));
		ParaVec3TransformCoord(&vAimPos, &vAimPos, &m);

		fWidth /= 2;
		fHeight /= 2;
		if (radius != 0)
		{
			fWidth += radius;
			fHeight += radius;
		}

		//-- now test 
		/// Note: I only tested x,z, I care not about the y component, which is related to object's height
		FLOAT min1 = min((vPos.x + fWidth), (fWidth - vPos.x));
		FLOAT min2 = min((vPos.z + fHeight), (fHeight - vPos.z));
		FLOAT fDepth = min(min1, min2);

		if (fDepth > 0)
		{
			if (fDepth == min1)
			{
				if (min1 == (fWidth - vPos.x))
				{
					/* right edge */
					vPos.x = fWidth;
				}
				else
				{
					/* left edge */
					vPos.x = -fWidth;
				}
				vPos.z = (vAimPos.z > 0) ? fHeight : -fHeight;
			}
			else
			{
				if (min2 == (fHeight - vPos.z))
				{
					/* top edge */
					vPos.z = fHeight;
				}
				else
				{
					/* bottom edge */
					vPos.z = -fHeight;
				}
				vPos.x = (vAimPos.x > 0) ? fWidth : -fWidth;
			}
			/* map back to world coordinates */
			ParaVec3TransformCoord(&vPos, &vPos, ParaMatrixRotationY(&m, fFacing));
			vPos = vCenter + vPos;
			if (waypoint.GetPointType() == BipedWayPoint::COMMAND_MOVING)
			{
				float fFacing;
				if (Math::ComputeFacingTarget(vPos, vSrcPos, fFacing))
					SetFacing(fFacing);
			}
			else
			{
				AddWayPoint(BipedWayPoint(vPos, BipedWayPoint::PATHFINDING_POINT));
			}
		}
		break;
	}
	default:
	{
		break;
	}
	}//switch(pSolid->m_objShape)
}

//-------------------------------------------------------------------------------------
// name: AnimateBiped
/// desc: Execute Low level commands and animate the biped object by a time advance.
/// according to its current waypoint left,speed and facing, assuming no obstacles in the way.
/// this is usually called by the Environment simulator tentatively, call 
/// restore Biped if you are not satisfied with the Animation result. 
/// param: bSharpTurning: true--> biped will face the target immediately;
///        flase-->biped turns as it moves.
/// E.g.
/// pPlayer->AnimateBiped(..);
/// if(not satisfied, or a collision occurs){
///		pPlayer->restorebiped(&b)
/// }
//-------------------------------------------------------------------------------------
float g_fLastBipedSpeed;
float g_fLastBipedFacing;
float g_fLastBipedFacingTarget;
Vector3 g_fLastBipedPos;

void CBipedObject::AnimateBiped(double dTimeDelta, bool bSharpTurning)
{
	// animate biped according to biped state and physics
	if (!IsControlledExternally())
	{
		// update action state from state manager
		UpdateState((float)dTimeDelta);
		AnimateMoving(dTimeDelta);       // moving or waiting - turning toward
	}
}

void CBipedObject::UpdateState(float fTimeDelta)
{
	if (m_pBipedStateManager)
		m_pBipedStateManager->Update(fTimeDelta);

}

void CBipedObject::PushParam()
{
	// save biped to globals
	g_fLastBipedSpeed = m_fSpeed;
	g_fLastBipedFacing = m_fYaw;
	g_fLastBipedFacingTarget = m_fFacingTarget;
	g_fLastBipedPos = m_vPos;
}

void CBipedObject::PopParam()
{
	m_fSpeed = g_fLastBipedSpeed;
	m_fYaw = g_fLastBipedFacing;
	m_fFacingTarget = g_fLastBipedFacingTarget;
	m_vPos = g_fLastBipedPos;
}

void CBipedObject::Animate(double dTimeDelta, int nRenderNumber)
{
	if (dTimeDelta < 0.000001)
		return;
	if (GetIsAnimPaused())
	{
		dTimeDelta = 0.f;
	}
	/** For animation instance. */
	CAnimInstanceBase* pAI = GetAnimInstance();
	if (pAI)
	{
		bool bIsAnimationProcessed = !(pAI->GetRenderCount() < nRenderNumber || nRenderNumber == 0);
		pAI->Animate(dTimeDelta, nRenderNumber);
		if (bIsAnimationProcessed)
			return;
		// Bug fix 2008.5.26: to prevent recursively calling this function. 
		static vector<CBipedObject*> g_recursionSeen;

		if (GetLastWayPoint().GetPointType() == BipedWayPoint::COMMAND_MOUNT)
		{
			// here we assume the first reference object with tag 0 is the mount object. 
			CBaseObject* pTarget = NULL;
			RefListItem* item = GetRefObjectByTag(0);
			if (item != 0)
			{
				pTarget = (CBaseObject*)item->m_object;
				if (pTarget)
				{
					/// get world position from the mount point
					vector<CBipedObject*>::const_iterator itCur = find(g_recursionSeen.begin(), g_recursionSeen.end(), pTarget);
					if (itCur == g_recursionSeen.end())
					{
						g_recursionSeen.push_back(this);
						pTarget->Animate(dTimeDelta, nRenderNumber);
						g_recursionSeen.pop_back();
					}

					/**
					* Set world position from the mount point
					*/
					Matrix4 mxWorld;
					if (GetRenderMatrix(mxWorld, nRenderNumber))
					{
						Vector3 vMountRenderPos;
						ParaVec3TransformCoord(&vMountRenderPos, &Vector3::ZERO, &mxWorld);
						Vector3 vMountPos = vMountRenderPos + CGlobals::GetScene()->GetRenderOrigin();
						SetPosition(DVector3(vMountPos));
					}
				}
			}
		}
		else
		{
			CBaseObject* pTarget = NULL;
			RefListItem* item = GetRefObjectByTag(0);
			if (item != 0)
			{
				pTarget = (CBaseObject*)item->m_object;
				if (pTarget)
				{
					vector<CBipedObject*>::const_iterator itCur = find(g_recursionSeen.begin(), g_recursionSeen.end(), pTarget);
					if (itCur == g_recursionSeen.end())
					{
						g_recursionSeen.push_back(this);
						pTarget->Animate(dTimeDelta, nRenderNumber);
						g_recursionSeen.pop_back();
					}
				}
			}
		}
	}
}

bool ParaEngine::CBipedObject::HasAttachmentPoint(int nAttachmentID)
{
	CParaXAnimInstance* pParaXAI = GetParaXAnimInstance();
	if (pParaXAI)
	{
		return (pParaXAI->HasAttachmentMatrix(nAttachmentID));
	}
	return false;
}

Matrix4* ParaEngine::CBipedObject::GetAttachmentMatrix(Matrix4& pOut, int nAttachmentID, int nRenderNumber)
{
	CParaXAnimInstance* pParaXAI = GetParaXAnimInstance();
	if (pParaXAI)
	{
		/// get the mount point matrix using animation instance
		if (pParaXAI->GetAttachmentMatrix(&pOut, nAttachmentID, nRenderNumber) != NULL)
		{
			return &pOut;
		}
	}
	return NULL;
}

void ParaEngine::CBipedObject::SetLocalTransform(const Matrix4& mXForm)
{
	if (!m_pLocalTransfrom)
	{
		m_pLocalTransfrom = new Matrix4();
	}
	*m_pLocalTransfrom = mXForm;
	SetGeometryDirty(true);
}

void ParaEngine::CBipedObject::GetLocalTransform(Matrix4* localTransform)
{
	if (localTransform)
	{
		GetLocalWorldTransform(*localTransform);
	}
}

void ParaEngine::CBipedObject::GetLocalWorldTransform(Matrix4& mxWorld)
{
	// order of rotation: localSpace{(localTransfrom * scaling)} * worldSpace{roll * pitch * yaw * (vNorm to UnitY) * bootHeight}, where roll is applied first. 
	bool bIsIdentity = true;
	
	if (m_vNorm != Vector3::UNIT_Y)
	{
		Vector3 vAxis;
		Matrix4 matNorm;
		vAxis = Vector3::UNIT_Y.crossProduct(Vector3(m_vNorm.x, m_vNorm.y, m_vNorm.z));
		ParaMatrixRotationAxis(&matNorm, vAxis, acos(m_vNorm.y));
		mxWorld = (bIsIdentity) ? matNorm : matNorm.Multiply4x3(mxWorld);
		bIsIdentity = false;
	}

	float fYaw = GetYaw();

	if (IsBillboarded())
	{
		// TODO: how about in the reflection pass?
		Vector3 vDir = m_vPos - CGlobals::GetScene()->GetCurrentCamera()->GetEyePosition();
		if (vDir.x > 0.0f)
			fYaw += -atanf(vDir.z / vDir.x) + MATH_PI / 2;
		else
			fYaw += -atanf(vDir.z / vDir.x) - MATH_PI / 2;
	}

	if (fYaw != 0.f)
	{
		Matrix4 matYaw;
		ParaMatrixRotationY((Matrix4*)&matYaw, fYaw);
		mxWorld = (bIsIdentity) ? matYaw : matYaw.Multiply4x3(mxWorld);
		bIsIdentity = false;
	}

	if (GetPitch() != 0.f)
	{
		Matrix4 matPitch;
		ParaMatrixRotationX(&matPitch, GetPitch());
		mxWorld = (bIsIdentity) ? matPitch : matPitch.Multiply4x3(mxWorld);
		bIsIdentity = false;
	}

	if (GetRoll() != 0.f)
	{
		Matrix4 matRoll;
		ParaMatrixRotationZ(&matRoll, GetRoll());
		mxWorld = (bIsIdentity) ? matRoll : matRoll.Multiply4x3(mxWorld);
		bIsIdentity = false;
	}

	float fScaling = GetSizeScale();
	if (fabs(fScaling - 1.0f) > FLT_TOLERANCE)
	{
		Matrix4 matScale;
		matScale.makeScale(fScaling, fScaling, fScaling);
		mxWorld = (bIsIdentity) ? matScale : matScale.Multiply4x3(mxWorld);
		bIsIdentity = false;
	}

	if (m_pLocalTransfrom)
	{
		mxWorld = (bIsIdentity) ? *m_pLocalTransfrom : m_pLocalTransfrom->Multiply4x3(mxWorld);
		bIsIdentity = false;
	}

	if(bIsIdentity)
		mxWorld.identity();

	if (m_fBootHeight != 0.f)
	{
		mxWorld._42 += m_fBootHeight;
	}
}

Matrix4* CBipedObject::GetRenderMatrix(Matrix4& mxWorld, int nRenderNumber)
{
	bool bMounted = false;
	if (GetLastWayPoint().GetPointType() == BipedWayPoint::COMMAND_MOUNT)
	{
		BipedWayPoint waypoint;
		GetWayPoint(&waypoint);
		// here we assume the first reference object with tag 0 is the mount object. 
		//CBaseObject* pTarget = (CBaseObject*) GetRefObjectByName(waypoint.m_sTarget.c_str());
		CBaseObject* pTarget = NULL;
		RefListItem* item = GetRefObjectByTag(0);
		if (item != 0)
			pTarget = (CBaseObject*)item->m_object;

		if (pTarget)
		{
			/// get world position from the mount point
			// Bug fix 2008.5.26: to prevent recursively calling this function. 
			static vector<CBipedObject*> g_recursionSeen;
			vector<CBipedObject*>::const_iterator itCur = find(g_recursionSeen.begin(), g_recursionSeen.end(), pTarget);
			if (itCur == g_recursionSeen.end())
			{
				g_recursionSeen.push_back(this);
				pTarget->GetRenderMatrix(mxWorld, nRenderNumber);
				g_recursionSeen.pop_back();

				Matrix4 mat;
				/// get the mount point matrix on the target model. attachment ID=0 is the mount point
				if (pTarget->GetAttachmentMatrix(mat, waypoint.m_nReserved0, nRenderNumber) != NULL)
				{
					/// Set world position: mount position+target position==>new world position for the mounted model.
					mxWorld = mat*mxWorld;
					bMounted = true;

					// if the mounted object will be scaled when the target object is scaled, we need to scale it back, so that 
					// the mounted object will not appear to be scaled. 
					float fScalingX, fScalingY, fScalingZ;
					Math::GetMatrixScaling(mxWorld, &fScalingX, &fScalingY, &fScalingZ);
					if (fabs(fScalingX - 1.0f) > FLT_TOLERANCE || fabs(fScalingY - 1.0f) > FLT_TOLERANCE || fabs(fScalingZ - 1.0f) > FLT_TOLERANCE)
					{
						mat = Matrix4::IDENTITY;
						mat.setScale(Vector3(1.f / fScalingX, 1.f / fScalingY, 1.f / fScalingZ));
						mxWorld = mat * mxWorld;
					}

					float fScaling = GetSizeScale();
					if (fabs(fScaling - 1.0f) > FLT_TOLERANCE)
					{
						Matrix4 matScale;
						matScale.makeScale(fScaling, fScaling, fScaling);
						mxWorld = matScale.Multiply4x3(mxWorld);
					}
					if (m_pLocalTransfrom)
					{
						mxWorld = m_pLocalTransfrom->Multiply4x3(mxWorld);
					}
				}
			}
			else
			{
#ifdef _DEBUG
				OUTPUT_LOG("CBipedObject::GetRenderMatrix mount recursion detected\n");
#endif
			}
		}
	}
	// for unmounted models
	if (!bMounted)
	{
		GetLocalWorldTransform(mxWorld);

		// world translation
		Vector3 vPos = GetRenderOffset();
		mxWorld._41 += vPos.x;
		mxWorld._42 += vPos.y;
		mxWorld._43 += vPos.z;
	}
	return &mxWorld;
}

void CBipedObject::SetOnAssetLoaded(const char* sCallbackScript)
{
	CTileObject::SetOnAssetLoaded(sCallbackScript);
	if (m_pMultiAnimationEntity && (m_pMultiAnimationEntity->GetType() == AssetEntity::parax))
	{
		if (((ParaXEntity*)m_pMultiAnimationEntity.get())->GetPrimaryTechniqueHandle() > 0)
		{
			ActivateScript(Type_OnAssetLoaded);
		}
	}
}

void ParaEngine::CBipedObject::SetPrimaryTechniqueHandle(int nHandle)
{
	if (GetPrimaryTechniqueHandle() < 0 && nHandle > 1)
	{
		// very tricky here: asset is not loaded, we will not set negative nHandle. 
		// During SetParamsFromAsset(), we will negate the sign after asset is loaded
		CTileObject::SetPrimaryTechniqueHandle(-nHandle);
	}
	else
	{
		CTileObject::SetPrimaryTechniqueHandle(nHandle);
	}
}

void ParaEngine::CBipedObject::SetPosition(const DVector3& v)
{
	if (m_vPos != v)
	{
		m_vPos = v;
		UnloadPhysics();
	}
}

bool CBipedObject::SetParamsFromAsset()
{
	// in case the asset is loaded successfully, we shall set the primary asset. 
	if (m_pMultiAnimationEntity && (m_pMultiAnimationEntity->GetType() == AssetEntity::parax))
	{
		if (((ParaXEntity*)m_pMultiAnimationEntity.get())->GetPrimaryTechniqueHandle() > 0)
		{
			/** load the default animation */
			if (m_pAI)
				m_pAI->LoadDefaultStandAnim(&m_fSpeed);
			/// if there is no Stand animation, just call ForceStop to set the speed to zero
			ForceStop();

			// very tricky here: we will set primary technique handle 
			// During SetParamsFromAsset(), we will negate the sign after asset is loaded if primary technique is specified before asset is loaded
			CTileObject::SetPrimaryTechniqueHandle((GetPrimaryTechniqueHandle() < -1) ? -GetPrimaryTechniqueHandle() : ((ParaXEntity*)m_pMultiAnimationEntity.get())->GetPrimaryTechniqueHandle());

			// this will update the character's bounding box from the asset.
			SetGeometryDirty(true);

			// fire asset load event
			ActivateScript(Type_OnAssetLoaded);
			return true;
		}
	}
	else
	{
		if (GetRadius() != 0.2f)
		{
			SetPrimaryTechniqueHandle(TECH_CHARACTER);

			// just a default radius for debugging purposes if asset is not ready or invalid.
			SetRadius(0.2f);
		}
		return true;
	}
	return false;
}

bool ParaEngine::CBipedObject::ViewTouch()
{
	if (GetPrimaryTechniqueHandle() < 0)
	{
		// in case the asset is loaded successfully, we shall set the primary asset. 
		SetParamsFromAsset();
		return false;
	}
	return true;
}

/** private helper class */
class PushGlobalTime
{
public:
	PushGlobalTime(SceneState * sceneState) :m_pSceneState(sceneState), m_bSetTime(false), m_bSetIgnoreTransparent(false), m_bSetGlobalAnimTime(false), m_nLastGlobalAnimTime(0)
	{
	}
	~PushGlobalTime()
	{
		if (m_bSetTime)
		{
			m_pSceneState->SetGlobalTime(m_nLastTime);
		}
		if (m_bSetIgnoreTransparent)
		{
			m_pSceneState->SetIgnoreTransparent(m_bLastIgnoreTransparent);
		}
		if (m_bSetGlobalAnimTime)
		{
			ParaEngine::globalTime = m_nLastGlobalAnimTime;
		}
	}
	void PushTime(int nGlobalTime) {
		m_nLastTime = m_pSceneState->GetGlobalTime();
		m_pSceneState->SetGlobalTime(nGlobalTime);
		m_bSetTime = true;
	}

	void PushGlobalAnimTime(int nGlobalTime) {
		m_nLastGlobalAnimTime = ParaEngine::globalTime;
		ParaEngine::globalTime = nGlobalTime;
		m_bSetGlobalAnimTime = true;
	}

	void PushIgnoreTransparent(bool bIgnoreTransparent) {
		m_bLastIgnoreTransparent = m_pSceneState->IsIgnoreTransparent();
		m_pSceneState->SetIgnoreTransparent(bIgnoreTransparent);
		m_bSetIgnoreTransparent = true;
	}

	SceneState * m_pSceneState;
	int m_nLastTime;
	bool m_bSetTime;
	bool m_bSetIgnoreTransparent;
	bool m_bLastIgnoreTransparent;

	int m_nLastGlobalAnimTime;
	bool m_bSetGlobalAnimTime;
};
//-----------------------------------------------------------------------------
// Name: CBipedObject::Draw()
/// Desc: Render this CBipedObject instance using the current animation frames.
/// This function guarantees that the animation is rendered. animation instance
/// is forced to be created if it's not available. So never call this function
/// if the object is not in the view frustum. Otherwise, memory is wasted.
/// vNorm is calculated by the following rule:
/// (1) if vNorm.y ==0 (uninitialized), then calculate from terrain surface
/// (2) if vNorm is valid and object is moving, use smooth transform
/// (3) if vNorm is valid and object is static or stopped, use the old norm. 
//-----------------------------------------------------------------------------
HRESULT CBipedObject::Draw(SceneState * sceneState)
{
	if (!ViewTouch() || GetOpacity() == 0.f)
	{
		return E_FAIL;
	}
	EffectManager* pEffectManager = CGlobals::GetEffectManager();

	if (!pEffectManager->IsCurrentEffectValid() || GetPrimaryTechniqueHandle() < 0)
	{
		return E_FAIL;
	}

	int nLastEffectHandle = pEffectManager->GetCurrentTechHandle();
	if (nLastEffectHandle != GetPrimaryTechniqueHandle())
	{
		pEffectManager->BeginEffect(GetPrimaryTechniqueHandle(), &(sceneState->m_pCurrentEffect));
	}

	// for models with particle systems
	sceneState->SetCurrentSceneObject(this);

	SetFrameNumber(sceneState->m_nRenderCount);

	// call Draw() of biped animation instance
	CAnimInstanceBase* pAI = GetAnimInstance();
	if (pAI)
	{
		PushGlobalTime pust_global_time_(sceneState);
		if (GetIsAnimPaused())
		{
			pust_global_time_.PushTime(0);
			//pust_global_time_.PushIgnoreTransparent(true);
			pust_global_time_.PushGlobalAnimTime(pAI->GetAnimFrame());
			// tricky: we will disable motion blending when paused. 
			pAI->SetBlendingFactor(0.f);
		}
		pEffectManager->applyObjectLocalLighting(this);

		// draw the model
		Matrix4 mxWorld;
		Matrix4* mat = GetRenderMatrix(mxWorld, sceneState->GetRenderFrameCount());

		if (GetPrimaryTechniqueHandle() == 1102 && !sceneState->IsShadowPass())
		{
#ifdef USE_DIRECTX_RENDERER
			CEffectFile* pEffectFile = pEffectManager->GetCurrentEffectFile();
			if (pEffectFile != NULL)
			{
				RenderDevicePtr pd3dDevice = sceneState->m_pd3dDevice;
				pd3dDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0);
				pAI->Draw(sceneState, mat);

				pd3dDevice->SetRenderState(D3DRS_COLORWRITEENABLE, 0x0000000F);
				pd3dDevice->SetRenderState(D3DRS_ZFUNC, D3DCMP_LESSEQUAL);
				pAI->Draw(sceneState, mat);

				return S_OK;
			}
#endif
		}

		if (GetSelectGroupIndex() >= 0 && !sceneState->IsShadowPass())
		{
			if ((sceneState->m_nCurRenderGroup & (RENDER_SELECTION)) == 0)
			{
				// we will not draw selected objects during normal render pass, instead we render it during selection render pass. 
				// selection render pass is usually called after opache meshes are rendered. 
				return S_OK;
			}
#ifdef USE_DIRECTX_RENDERER
			// for selection render effect
			CEffectFile* pEffectFile = pEffectManager->GetCurrentEffectFile();
			if (pEffectFile != 0)
			{
				int nSelectionEffectStyle = GetSelectionEffect();
				if (nSelectionEffectStyle == RenderSelectionStyle_border)
				{
					// Let us render the border using the same technique as rendering text shadow.
					// i.e. render 4 times shifting 2 pixels around the border. 
					pEffectManager->BeginEffect(TECH_SIMPLE_MESH_NORMAL_BORDER, &(sceneState->m_pCurrentEffect));
					pEffectFile = pEffectManager->GetCurrentEffectFile();

					RenderDevicePtr pd3dDevice = sceneState->m_pd3dDevice;
					pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, FALSE);

					if (pEffectFile != 0)
					{
						// the border width in meters. 
						const float border_width = 0.03f;

						for (int x = -1; x <= 1; x += 2)
						{
							for (int y = -1; y <= 1; y += 2)
							{
								Vector3 vOffsets(x*border_width, y*border_width, 0.f);
								pEffectFile->GetDXEffect()->SetRawValue("g_offsets", &vOffsets, 0, sizeof(Vector3));
								pAI->Draw(sceneState, mat);
							}
						}
					}

					pd3dDevice->SetRenderState(D3DRS_ZWRITEENABLE, TRUE);

					// change back to primary technique
					pEffectManager->BeginEffect(GetPrimaryTechniqueHandle(), &(sceneState->m_pCurrentEffect));
				}
				else if (nSelectionEffectStyle == RenderSelectionStyle_unlit)
				{
					bool bIsRendered = false;
					pEffectManager->BeginEffect(TECH_SIMPLE_MESH_NORMAL_UNLIT, &(sceneState->m_pCurrentEffect));
					pEffectFile = pEffectManager->GetCurrentEffectFile();
					if (pEffectFile != 0)
					{
						Vector3 vColorAdd(0.2f, 0.2f, 0.2f);
						pEffectFile->GetDXEffect()->SetRawValue("g_color_add", &vColorAdd, 0, sizeof(Vector3));

						pAI->Draw(sceneState, mat);
						bIsRendered = true;
					}
					// change back to primary technique
					pEffectManager->BeginEffect(GetPrimaryTechniqueHandle(), &(sceneState->m_pCurrentEffect));
					if (bIsRendered)
						return S_OK;
				}
			}
			else
			{
				// TODO: find a way to render a single color with offsets with fixed function. 

				// change back to primary technique
				pEffectManager->BeginEffect(GetPrimaryTechniqueHandle(), &(sceneState->m_pCurrentEffect));
			}
#endif
		}

		bool bUsePointTextureFilter = false;
		
		// apply block space lighting for object whose size is comparable to a single block size
		if (CheckAttribute(MESH_USE_LIGHT) && !(sceneState->IsShadowPass()))
		{
			BlockWorldClient* pBlockWorldClient = BlockWorldClient::GetInstance();
			if (pBlockWorldClient && pBlockWorldClient->IsInBlockWorld())
			{
				Vector3 vPos = GetPosition();
				vPos.y += 0.1f;
				Uint16x3 blockId_ws(0, 0, 0);
				BlockCommon::ConvertToBlockIndex(vPos.x, vPos.y, vPos.z, blockId_ws.x, blockId_ws.y, blockId_ws.z);
				DWORD dwPositionHash = blockId_ws.GetHashCode();
				uint8_t brightness[2];
				pBlockWorldClient->GetBlockMeshBrightness(blockId_ws, brightness, 2);
				// block light
				float fBlockLightness = Math::Max(pBlockWorldClient->GetLightBrightnessLinearFloat(brightness[0]), 0.1f);
				sceneState->GetCurrentLightStrength().y = fBlockLightness;
				// sun light
				float fSunLightness = Math::Max(pBlockWorldClient->GetLightBrightnessLinearFloat(brightness[1]), 0.1f);
				sceneState->GetCurrentLightStrength().x = fSunLightness;

				float fLightness = Math::Max(fBlockLightness, fSunLightness*pBlockWorldClient->GetSunIntensity());
				if (m_fLastBlockLight != fLightness)
				{
					float fMaxStep = (float)(sceneState->dTimeDelta*0.5f);
					if (dwPositionHash == m_dwLastBlockHash || m_dwLastBlockHash == 0)
						m_fLastBlockLight = fLightness;
					else
						Math::SmoothMoveFloat1(m_fLastBlockLight, fLightness, fMaxStep);

					fLightness = m_fLastBlockLight;
				}
				else
				{
					m_dwLastBlockHash = dwPositionHash;
				}

				if (!sceneState->IsDeferredShading())
				{
					sceneState->GetLocalMaterial().Ambient = (LinearColor(fLightness*0.7f, fLightness*0.7f, fLightness*0.7f, 1.f));
					sceneState->GetLocalMaterial().Diffuse = (LinearColor(fLightness*0.4f, fLightness*0.4f, fLightness*0.4f, 1.f));
				}
				else
				{
					sceneState->GetLocalMaterial().Diffuse = LinearColor::White;
				}
				sceneState->EnableLocalMaterial(true);
				
				bUsePointTextureFilter = bUsePointTextureFilter || pBlockWorldClient->GetUsePointTextureFiltering();
			}
		}
		
		CDynamicAttributeField* pField = GetDynamicField("colorDiffuse");
		if (pField)
		{
			if (sceneState->IsDeferredShading())
			{
				// for deferred shading merge ambient and diffuse
				sceneState->GetLocalMaterial().Diffuse = LinearColor((DWORD)(*pField));
				pField = GetDynamicField("colorAmbient");
				if (pField) {
					sceneState->GetLocalMaterial().Diffuse += LinearColor((DWORD)(*pField));
					sceneState->GetLocalMaterial().Ambient = LinearColor::Black;
				}
			}
			else
			{
				sceneState->GetLocalMaterial().Diffuse = LinearColor((DWORD)(*pField));
				pField = GetDynamicField("colorAmbient");
				if (pField)
					sceneState->GetLocalMaterial().Ambient = LinearColor((DWORD)(*pField));
			}
			sceneState->EnableLocalMaterial(true);
		}

		if (bUsePointTextureFilter)
		{
			pEffectManager->SetSamplerState(0, D3DSAMP_MINFILTER, D3DTEXF_POINT);
			pEffectManager->SetSamplerState(0, D3DSAMP_MAGFILTER, D3DTEXF_POINT);
		}
		else
		{
			pEffectManager->SetSamplerState(0, D3DSAMP_MINFILTER, pEffectManager->GetDefaultSamplerState(0, D3DSAMP_MINFILTER));
			pEffectManager->SetSamplerState(0, D3DSAMP_MAGFILTER, pEffectManager->GetDefaultSamplerState(0, D3DSAMP_MAGFILTER));
		}

		CApplyObjectLevelParamBlock p(GetEffectParamBlock());
		pAI->Draw(sceneState, mat, p.GetParamsBlock());
		sceneState->EnableLocalMaterial(false);
	}

	for(auto child:m_children)
		child->Draw(sceneState);

	return S_OK;
}

void CBipedObject::BuildShadowVolume(SceneState * sceneState, ShadowVolume * pShadowVolume, LightParams* pLight)
{
#ifdef USE_DIRECTX_RENDERER
	if (!IsShadowEnabled())
		return;
	CAnimInstanceBase* pAI = GetAnimInstance();
	if (pAI)
	{
		// push matrix
		Matrix4 mxWorld;

		pAI->BuildShadowVolume(sceneState, pShadowVolume, pLight, GetRenderMatrix(mxWorld, sceneState->GetRenderFrameCount()));
	}
#endif
}

//-----------------------------------------------------------------------------
// Name: CBipedObject::SetUserControl()
/// Desc: Specifies whether this instance of CBipedObject is controlled by the
///       user or the program.
//-----------------------------------------------------------------------------
void CBipedObject::SetUserControl()
{
	//m_bUserControl = true;
}
//-----------------------------------------------------------------------------
// Name: CBipedObject::ChooseNewLocation()
/// Desc: Determine a new location for this character to move to.  In this case
///       we simply randomly pick a spot on the floor as the new location.
//-----------------------------------------------------------------------------
void CBipedObject::ChooseNewLocation(Vector3 *pV)
{
	pV->x = 1.0f;//(float) ( rand() % 256 ) / 256.f;
	pV->y = 0.f;
	pV->z = 1.0f;//(float) ( rand() % 256 ) / 256.f;
}


//-----------------------------------------------------------------------------
// Name: CBipedObject::AnimateUserControl()
/// Desc: Reads user input and update Tiny's state and animation accordingly.
//-----------------------------------------------------------------------------
void CBipedObject::AnimateUserControl(double dTimeDelta)
{
	// use keyboard controls to make Tiny move
}




//-----------------------------------------------------------------------------
// Name: CBipedObject::AnimateIdle()
/// Desc: Checks if Tiny has been idle for long enough.  If so, initialize Tiny
///       to move again to a new location.
//-----------------------------------------------------------------------------
void CBipedObject::AnimateIdle(double dTimeDelta)
{
}

/** used for ray casting biped for low level navigation.
* @see CBipedObject::MoveTowards()*/
struct SensorGroups
{
	/// origin shared by all sensor rays in this group.
	Vector3 m_vOrig;

	struct SensorRay
	{
		/// direction of the ray in the x,z plane
		Vector3 vDir;
		/// the impact norm of the ray 
		Vector3 impactNorm;
		/// the hit point of the ray
		Vector3 impactPoint;
		/// the distance from the impact point to origin. 
		float fDist;
		/// whether the sensor has detected anything in its range
		bool bIsHit;
		/// if perfect wall, nSide will be [0,5], otherwise it is nSide. 
		int nSide;
	public:
		SensorRay() :bIsHit(false), fDist(0), nSide(-1){};
	};
	SensorGroups() :m_bSenseBlockWorld(false), m_isPerfectWall(false) {};
	/** n (n=BIPED_SENSOR_RAY_NUM=3) rays in front of the character,which covers a region of (n-2)/(n-1)*Pi radian.*/
	SensorRay m_sensors[BIPED_SENSOR_RAY_NUM];
	/// the average of all impact norms if available.
	Vector3 m_vAvgImpactNorm;
	/// range for all sensors.
	float m_fSensorRange;
	/// number of walls we hit.
	int	m_nHitWallCount;
	/// the ray index whose impactPoint is closest to the origin. if it is negative, then no ray has hit anything.
	int m_nHitRayIndex;

	/** whether we are walking along a perfect vertical wall. only set to true if m_nHitWallCount is 1 and only the block world is met. */
	bool m_isPerfectWall;
	/** whether to detect block world. default to false;*/
	bool m_bSenseBlockWorld;
	/** block pick result. */
	PickResult m_block_pick_result;
public:
	/** whether the sensor group has hit anything. */
	bool HasHitAnything(){ return m_nHitRayIndex >= 0; }
	/** whether the sensor group has hit anything. A negative value is returned if no hit ray.*/
	int GetHitRayIndex(){ return m_nHitRayIndex; }
	/** get the hit ray */
	SensorRay& GetHitRaySensor(){ return m_sensors[m_nHitRayIndex]; }
	/** get the number of wall hits. */
	int GetHitWallCount(){ return m_nHitWallCount; }
	/** get average impact norm. this is not normalized and may not be in the y=0 plane*/
	Vector3 GetAvgImpactNorm(){ return m_vAvgImpactNorm; }

	bool IsPerfectWall() const { return m_isPerfectWall; };
	/** reset the sensor group to empty states. */
	void Reset()
	{
		m_nHitWallCount = 0;
		m_nHitRayIndex = -1;
		m_isPerfectWall = false;
		m_vAvgImpactNorm = Vector3(0, 0, 0);
		for (int i = 0; i<BIPED_SENSOR_RAY_NUM; i++)
		{
			m_sensors[i].bIsHit = false;
			m_sensors[i].nSide = -1;
		}
		memset(&m_block_pick_result, 0, sizeof(PickResult));
	}

	/** get the sensor impact information for this sensor group. All sensor rays will detect collision with anything in
	* the physical scene within its range
	* @param vOrig: the shared ray origin.
	* @param vDir: the direction of the central ray. it should be in the x,z plane,
	* @param fSensorRange: the range shared by all sensors in the group.
	* @param dwGroupMask: which physics group that the sensor should collide with.
	* @param nSensorRayCount: the number of ray computed around the center ray. it must be smaller than BIPED_SENSOR_RAY_NUM.
	* @param fAngleCoef: Default value is 0.16f. asin(sqrt(0.16)) = 23 degrees. I arbitrarily define any adjacent two walls
	* with over 23 degrees difference to be a corner. If you want to define a different wall angle, then
	* supply your own fAngleCoef = sin(angle)*sin(angle), where angle is your desired wall angle.*/
	void ComputeSensorGroup(const Vector3& vOrig, const Vector3& vDir, float fSensorRange, DWORD dwGroupMask = DEFAULT_PHYSICS_GROUP_MASK, int nSensorRayCount = BIPED_SENSOR_RAY_NUM, float fAngleCoef = 0.16f)
	{
		if (nSensorRayCount>BIPED_SENSOR_RAY_NUM || BIPED_SENSOR_RAY_NUM <= 0)
			return;

		Reset();
		m_vOrig = vOrig;
		m_fSensorRange = fSensorRange;

		Matrix4 m;
		float distLast = fSensorRange;		// the shortest distance between the wall impact point and the old biped position. 
		Vector3 impactNorm(0, 0, 0);		// the impact norm of the ray which is closest to the wall.
		Vector3 lastImpactNorm(0, 0, 0);
		int nHitCount = 0;

		BlockWorldClient* pBlockWorldClient = BlockWorldClient::GetInstance();
		int last_block_hit_side = -1;

		for (int i = 0; i < nSensorRayCount; i++)
		{
			// Get ray direction
			SensorRay& sensor = m_sensors[i];
			sensor.bIsHit = false;
			ParaVec3TransformCoord(&sensor.vDir, &vDir, ParaMatrixRotationY(&m,
				(MATH_PI / (nSensorRayCount + 1))*(i - ((nSensorRayCount - 1) / 2))));


			if (m_bSenseBlockWorld && pBlockWorldClient && pBlockWorldClient->IsInBlockWorld())
			{
				PickResult result;
				if (pBlockWorldClient->Pick(m_vOrig, sensor.vDir, BlockConfig::g_blockSize, result) && result.Distance < BlockConfig::g_blockSize)
				{
					// TODO: BlockWorldClient::GetBlockTemplateByIndex() and check if the block is collidable.

					sensor.bIsHit = true;
					sensor.fDist = result.Distance;
					sensor.impactNorm = BlockCommon::GetNormalBySide(result.Side);
					sensor.impactPoint = Vector3(result.X, result.Y, result.Z);
					sensor.nSide = result.Side;

					if (last_block_hit_side == -1 ||
						(((abs(m_block_pick_result.BlockX - result.BlockX) < 2) && (abs(m_block_pick_result.BlockZ - result.BlockZ) < 2))
						&& (BlockCommon::IsCornerSide(m_block_pick_result.Side, last_block_hit_side) && !(m_block_pick_result.BlockX == result.BlockX && m_block_pick_result.BlockZ == result.BlockZ && m_block_pick_result.BlockY == result.BlockY))))
					{
						// only count as two objects if object side is different from the previous one. since we only allow sliding alone a single hit object  
						nHitCount++;
						m_nHitWallCount++;
					}

					if (m_nHitRayIndex < 0 || distLast >= sensor.fDist || i == 1){
						// get the closest hit ray index
						distLast = sensor.fDist;

						if (m_nHitRayIndex <= 0)
						{
							// always use 
							m_vAvgImpactNorm = sensor.impactNorm;
							m_block_pick_result = result;
							last_block_hit_side = result.Side;
						}
						m_nHitRayIndex = i;
					}
				}
			}

			RayCastHitResult hit;
			//OUTPUT_LOG("biped: raycastClosestShape %f %f %f", sensor.vDir.x, sensor.vDir.y, sensor.vDir.z);
			// Some times this could cause : "invalid parameter : NxRay direction not valid: must be unit vector" error by the physics engine. 
			// but the input normal appears as legal as  -0.559834 0.000000 0.828605. and even the same input may be OK on the second call. 
			// Get the closest shape
			IParaPhysicsActor* closestShape = CGlobals::GetPhysicsWorld()->GetPhysicsInterface()->RaycastClosestShape(
				CONVERT_PARAVECTOR3(m_vOrig), CONVERT_PARAVECTOR3(sensor.vDir), 0, hit, (int16)dwGroupMask, m_fSensorRange);
			//OUTPUT_LOG(" end\n");
			if (closestShape && m_fSensorRange >= hit.m_fDistance)
			{
				nHitCount++;
				sensor.bIsHit = true;
				sensor.fDist = hit.m_fDistance;
				sensor.impactNorm = (const Vector3&)(hit.m_vHitNormalWorld);
				sensor.impactPoint = (const Vector3&)(hit.m_vHitPointWorld);
				m_vAvgImpactNorm += sensor.impactNorm;
				sensor.nSide = -1;

				if (distLast >= sensor.fDist){
					// get the closest hit ray index
					distLast = sensor.fDist;
					m_nHitRayIndex = i;
				}

				// if the impact norms of two adjacent rays deviate too much or a block is hit previously, they can not be considered a single wall.
				if ((last_block_hit_side == -1) &&
					((lastImpactNorm == Vector3(0, 0, 0)) || ((lastImpactNorm - sensor.impactNorm).squaredLength()) <= fAngleCoef))
				{
					if (m_nHitWallCount == 0)
						m_nHitWallCount = 1;
				}
				else
					m_nHitWallCount++;
				lastImpactNorm = sensor.impactNorm;
			}
		} // for
		if (m_nHitWallCount == 1 && last_block_hit_side != -1)
		{
			m_isPerfectWall = true;
		}
	}

	/** compare this sensor group with another one.
	* @param otherGroup: the sensor group to compare with.
	* @param fAngleCoef: Default value is 0.16f. asin(sqrt(0.16)) = 23 degrees. I arbitrarily define any adjacent two walls
	* with over 23 degrees difference to be a corner. If you want to define a different wall angle, then
	* supply your own fAngleCoef = sin(angle)*sin(angle), where angle is your desired wall angle.
	* @return: true if it the two sensor groups have detected a wall corner.*/
	bool CompareWith(const SensorGroups& otherGroup, float fAngleCoef = 0.16f)
	{

		if (otherGroup.IsPerfectWall() && IsPerfectWall())
		{
			if (otherGroup.m_block_pick_result.BlockX == m_block_pick_result.BlockX && otherGroup.m_block_pick_result.BlockY == m_block_pick_result.BlockY && otherGroup.m_block_pick_result.BlockZ == m_block_pick_result.BlockZ)
				// if same block is found it is not a corner
				return false;
			else
			{
				if (BlockCommon::IsCornerSide(otherGroup.m_block_pick_result.Side, m_block_pick_result.Side))
					// two different blocks with a corner
					return true;
				else
					return false;
			}
		}

		bool bHasCorner = false;
		Vector3 lastDisNorm(0, 0, 0);
		for (int i = 0; i < BIPED_SENSOR_RAY_NUM; i++)
		{
			if (m_sensors[i].bIsHit && otherGroup.m_sensors[i].bIsHit)
			{
				Vector3 vDisplacement = m_sensors[i].impactPoint - otherGroup.m_sensors[i].impactPoint;
				if (vDisplacement != Vector3(0, 0, 0))
				{
					ParaVec3Normalize(&vDisplacement, &vDisplacement);
					if ((lastDisNorm != Vector3(0, 0, 0)) && ((lastDisNorm - vDisplacement).squaredLength()) >= fAngleCoef)
					{
						Vector3 tmp;
						tmp = lastDisNorm.crossProduct(vDisplacement);
						if (tmp.y >= 0)
							bHasCorner = true;
					}
					lastDisNorm = vDisplacement;
				}
			}
		}
		return bHasCorner;
	}
};

/**
* @param fQuickMoveDistance: if positive value, it is distance. if negative values, it is seconds. where fQuickMoveDistance = (-fQuickMoveDistance)*Speed;
*/
bool ParaEngine::CBipedObject::MoveTowards_OPC(double dTimeDelta, const DVector3& vPosTarget, float fQuickMoveDistance /*= 15.f*/)
{
	bool bReachPos = false;

	// get distance from target
	DVector3 vSub = vPosTarget - m_vPos;
	float fDistSq = (float)vSub.squaredLength(); // & Vector2(vSub.x, vSub.z)  

	float fSpeed = GetSpeed();

	float fDeltaDist = (float)(fSpeed*dTimeDelta);

	if (fQuickMoveDistance < 0)
	{
		fQuickMoveDistance = fSpeed*(-fQuickMoveDistance);
	}

	// check if we have already reached the position or the y position is way too far, we will move to the target immediately. 
	if (fDistSq < fDeltaDist*fDeltaDist || fabs(vSub.y) > fQuickMoveDistance)
	{
		// we're within reach
		bReachPos = true;
		SetPosition(vPosTarget);
	}
	else if (fDistSq > fQuickMoveDistance*fQuickMoveDistance)
	{
		// if we are too far away from target, move immediately to a point that is fQuickMoveDistance from it. 
		vSub.normalise();
		m_vPos = vPosTarget - vSub*(fQuickMoveDistance - fDeltaDist);
	}
	else
	{
		// if we have not reached position, we will move on, linearly.
		if (!bReachPos && fSpeed != 0.f)
		{
			vSub.normalise();
			m_vPos += vSub*fDeltaDist;
		}
	}

	return bReachPos;
}

bool ParaEngine::CBipedObject::MoveTowards_Linear(double dTimeDelta, const DVector3& vPosTarget)
{
	bool bReachPos = false;

	if (m_isAlwaysFlying)
	{
		if (!m_isFlying)
		{
			//init flying state
			m_isFlying = true;
			Matrix4 rotMat;
			ParaMatrixRotationY(&rotMat, -1.57f + m_fYaw);
			m_flyingDir = Vector3(0, 0, -1) * rotMat;
		}
	}

	// get distance from target
	DVector3 vSub = vPosTarget - m_vPos;
	float fDistSq = (float)vSub.squaredLength();

	float fSpeed = GetSpeed();

	float fDeltaDist = (float)(fSpeed*dTimeDelta);

	// check if we have already reached the position
	if (fDistSq < fDeltaDist*fDeltaDist)
	{
		// we're within reach
		bReachPos = true;
		SetPosition(vPosTarget);
		UpdateTileContainer();
	}

	// if we have not reached position , we will move on.
	if (!bReachPos && fSpeed != 0.f)
	{
		vSub.normalise();
		DVector3 vNewPos = m_vPos + vSub*fDeltaDist;
		SetPosition(vNewPos);
	}
	return bReachPos;
}

bool CBipedObject::MoveTowards(double dTimeDelta, const DVector3& vPosTarget, float fStopDistance, bool * pIsSlidingWall)
{
	UnloadPhysics();
	if (m_nMovementStyle == MOVESTYLE_OPC)
	{
		m_fLastSpeed = 0;
		// tricky: 5 seconds*Speed of quick move distance 
		return MoveTowards_OPC(dTimeDelta, vPosTarget, -5.f);

	}
	else if (m_nMovementStyle == MOVESTYLE_LINEAR)
	{
		m_fLastSpeed = 0;
		return MoveTowards_Linear(dTimeDelta, vPosTarget);
	}

	if ((m_canFly && m_fSpeedVertical > 0) || m_isFlying || m_isAlwaysFlying)
	{
		if (!m_isFlying)
		{
			//init flying state
			m_isFlying = true;
			Matrix4 rotMat;
			ParaMatrixRotationY(&rotMat, -1.57f + m_fYaw);
			m_flyingDir = Vector3(0, 0, -1) * rotMat;
		}
		m_fLastSpeed = 0;
		return FlyTowards(dTimeDelta, vPosTarget, fStopDistance, pIsSlidingWall);
	}


	static SensorGroups g_sensorGroups[3];
	/** check to see if the maximum penetration distance allowed. In each simulation step, if the biped moves
	* longer than this step, it is further divided in to several evenly spaced steps.
	* the step length is determined by the biped's speed multiplied by the time delta.
	* See also NX_MIN_SEPARATION_FOR_PENALTY, and CBipedObject::MoveTowards()*/
	float fMaxPenetration = (float)(max(fabs(GetVerticalSpeed()), max(fabs(GetSpeed()), fabs(GetLastSpeed()))) * dTimeDelta);
	if (fMaxPenetration > PENETRATION_DISTANCE)
	{
		int nNumSteps = (int)ceil(fMaxPenetration / PENETRATION_DISTANCE);
		/// ignore PENETRATION_DISTANCE, if it takes longer than MAX_STEPS_PER_FRAME per frame.
		if (nNumSteps > MAX_STEPS_PER_FRAME)
		{

#ifdef ALLOW_HIGH_SPEED_PASSTHROUGH
			nNumSteps = MAX_STEPS_PER_FRAME;
			return MoveTowards_Linear(dTimeDelta, vPosTarget);
#else
			dTimeDelta = dTimeDelta * MAX_STEPS_PER_FRAME / nNumSteps;
			nNumSteps = MAX_STEPS_PER_FRAME;
#endif
		}
		double dT = dTimeDelta / nNumSteps;
		bool bReachedDestination = false;
		for (int i = 0; (i < nNumSteps) && (!bReachedDestination); i++)
		{
			bReachedDestination = MoveTowards(dT, vPosTarget, fStopDistance, pIsSlidingWall);
		}
		return bReachedDestination;
	}
	if (m_nMovementStyle != MOVESTYLE_HEIGHTONLY)
	{
		// check to see whether the target is in movable region, if not, we only allow object to move if the object is already outside the movable region.
		const CShapeAABB * aabb = GetMovableRegion();
		if (aabb != 0)
		{
			Vector3 vMin = aabb->GetMin();
			Vector3 vMax = aabb->GetMax();
			// we only care about xz plane.
			if (vMin.x < vPosTarget.x && vMin.z<vPosTarget.z &&
				vMax.x>vPosTarget.x && vMax.z > vPosTarget.z)
			{
				// target is inside the movable region.
			}
			else
			{
				if (vMin.x <= m_vPos.x && vMin.z <= m_vPos.z &&
					vMax.x >= m_vPos.x && vMax.z >= m_vPos.z)
				{
					// biped is inside the movable region, we will stop it. 
					return true;
				}
			}
		}
	}
	bool bMaintainHeight = false;
	bool bSwimmingOnSurface = false;
	bool bFeetUnderWater, bHeadUnderWater;
	float fMaintainHeight = 0.f;
	//float fGravity = GRAVITY_CONSTANT;
	float fGravity = m_gravity;
	CBipedStateManager* pCharState = GetBipedStateManager();

	BlockWorldClient* pBlockWorldClient = BlockWorldClient::GetInstance();
	bool is_in_block_world = (pBlockWorldClient && pBlockWorldClient->IsInBlockWorld());
	/** check whether the biped is under water. if so set water state.
	* the acceleration of biped under water is Gw= (1-e/D)*G -(V*V)*k
	* where G is gravity of the earth, D is the density of the object, V is the vertical speed of the biped.
	* e is the percentage of object body under water. It is 1 when object is completely underwater, and (0,1) when partially underwater, and 0 when not underwater.
	* k denotes water resistance, which is  0.1f.
	*/
	{
		// the height on the character body, when the character is floating on the water surface. 
		//	- this is feet position when biped density is larger than water density
		//	- this is head position when biped density is smaller than 0 (floating in the air)
		Vector3 vHeadPos = m_vPos;
		float fHeight = GetHeight();
		vHeadPos.y += fHeight;
		bFeetUnderWater = pBlockWorldClient->IsPointUnderWater(m_vPos);
		bHeadUnderWater = pBlockWorldClient->IsPointUnderWater(vHeadPos);
		if (m_fDensity > 0 && m_fDensity < 1.f && !bHeadUnderWater)
		{
			Vector3 vWaterLinePos = m_vPos;
			vWaterLinePos.y += fHeight * m_fDensity - 0.1f; // 0.1 is chosen arbitrary to make it smaller anyway. 
			bSwimmingOnSurface = pBlockWorldClient->IsPointUnderWater(vWaterLinePos);
		}

		// Note: a biped is underwater only if the biped is below water level but ABOVE global terrain 
		if (bFeetUnderWater)
		{
			if (is_in_block_world)
			{
				// if biped is in block water world, we will ignore terrain height. 
			}
			else
			{
				float fTerrainHeight = CGlobals::GetGlobalTerrain()->GetElevation((float)m_vPos.x, (float)m_vPos.z);
				if (m_vPos.y < (fTerrainHeight - 1.0f)){
					bFeetUnderWater = false;
					bHeadUnderWater = false;
				}
			}
		}


		float e = 1.f;
		if (bFeetUnderWater)
		{
			if (bHeadUnderWater)
				e = 1.f;
			else
			{
				if (!is_in_block_world)
					e = (CGlobals::GetOceanManager()->GetWaterLevel() - (float)m_vPos.y) / fHeight;
			}
		}
		else
			e = 0.f;
		if (bFeetUnderWater)
		{
			if (bSwimmingOnSurface || bHeadUnderWater)
			{
				// character is under water
				pCharState->AddAction(CBipedStateManager::S_IN_WATER);
			}
			else
			{
				// character is on its feet 
				pCharState->AddAction(CBipedStateManager::S_ON_WATER_SURFACE);
			}

			if (m_fDensity > 1.f)
			{
				fGravity = (WATER_DENSITY * m_fDensity) * GRAVITY_CONSTANT;
			}
			else if (m_fDensity > 0.f)
				fGravity = (WATER_DENSITY - e / m_fDensity) * GRAVITY_CONSTANT;
			else
				fGravity = WATER_DENSITY*GRAVITY_CONSTANT;

			/** the larger the more resistance between [0,1]. */
#define UP_WATER_RESISTENCE_COEF	0.1f
#define DOWN_WATER_RESISTENCE_COEF	0.1f
			/** when an object is crossing the water/air surface with a speed less than WATER_SKIP_VELOCITY m/s, we will not allow the object
			* to escape to air or water, but rather floating at the height of m_fDensity*fHeight;
			*/
#define WATER_SKIP_VELOCITY			1.f

			if (!is_in_block_world  && bSwimmingOnSurface && m_fSpeedVertical == FLT_TOLERANCE)
			{
				// character is swimming on the water surface~
				bMaintainHeight = true;
				fMaintainHeight = CGlobals::GetOceanManager()->GetWaterLevel() - m_fDensity*fHeight;
			}
			else
			{
				if (m_fSpeedVertical > 0)
				{
					fGravity += m_fSpeedVertical*m_fSpeedVertical*UP_WATER_RESISTENCE_COEF;
				}
				else if (m_fSpeedVertical == 0.f)
				{
					if (bHeadUnderWater && m_fDensity > 0 && m_fDensity < 1.f)
					{
						// give some initial speed, so that character can float up. 
						m_fSpeedVertical = FLT_TOLERANCE;
					}
				}
				else
				{
					/** the fall down speed should be small.*/
					fGravity -= m_fSpeedVertical*m_fSpeedVertical*DOWN_WATER_RESISTENCE_COEF;
				}
				if (!bHeadUnderWater && m_fDensity < WATER_DENSITY && !is_in_block_world)
				{
					/** when an object is crossing the water/air surface with a speed less than WATER_SKIP_VELOCITY m/s, we will not allow the object
					* to escape to air or water, but rather floating at the height of m_fDensity*fHeight;
					*/
					if (fabs(m_fSpeedVertical) < WATER_SKIP_VELOCITY)
					{
						bMaintainHeight = true;
						fMaintainHeight = CGlobals::GetOceanManager()->GetWaterLevel() - m_fDensity*fHeight;

						//if( fabs(e-m_fDensity)*fHeight <= FLT_TOLERANCE)
						{
							m_fSpeedVertical = FLT_TOLERANCE;
							fGravity *= 0.1f;
						}
					}
				}
			}
		}
		else
		{
			// character is on land or in air
			pCharState->AddAction(CBipedStateManager::S_ON_FEET);
		}
	}

	bool bReachPos = false;
	bool bSlidingWall = false; // whether the object is sliding along a wall.
	bool bUseGlobalTerrainNorm = false; // whether the biped will be oriented according to the global terrain 
	/// true to ignore global terrain physics. The object will fall through the global terrain. This is automatically
	/// set by the MoveTowards() method. If the height of the object is well below of the global terrain surface, it is assumed
	/// to be underground, such as in the cave or tunnel. The camera will also ignore collision with the terrain objects.
	bool bIgnoreTerrain = false;

	DVector3 vMovePos = m_vPos; // the position for the next frame without considering the height
	DVector3 vSub;
	float fDist;

	// get distance from target
	vSub = m_vPos - vPosTarget;
	fDist = Vector2((float)vSub.x, (float)vSub.z).squaredLength();

	// check if we have already reached the position
	if (fStopDistance == 0)
	{
		if (fMaxPenetration * fMaxPenetration >= fDist)
		{
			// we're within reach
			bReachPos = true;
			// set the exact point
			vMovePos = vPosTarget;
		}
	}
	else if (fStopDistance > fDist)
	{
		/// if we're within reach, we will stop without reaching the exact target point
		bReachPos = true;
	}

	float fSpeed = GetSpeed();

	// if player is under water speed should be half. 
	if (is_in_block_world)
	{
		if (bHeadUnderWater && bFeetUnderWater)
		{
			fSpeed *= 0.8f;
		}
		else
		{
			uint16_t block_id = pBlockWorldClient->GetBlockTemplateId((float)m_vPos.x, (float)m_vPos.y + 0.5f, (float)m_vPos.z);
			if (block_id > 0)
			{
				BlockTemplate* pBlockTemplate = pBlockWorldClient->GetBlockTemplate(block_id);
				fSpeed *= pBlockTemplate->GetSpeedReductionPercent();
			}
		}
	}

	if (!bReachPos)
	{
		if (GetAccelerationDist() > 0.f)
		{
			float fLastSpeed = GetLastSpeed();
			float fSpeedDelta = max(fMaxPenetration, (float)dTimeDelta*4.f) / GetAccelerationDist();
			if (m_fLastSpeed < fSpeed)
			{
				if (m_fLastSpeed == 0.f)
				{
					// give it initial speed of 2.f
					m_fLastSpeed = min(2.f, fSpeed);
				}
				m_fLastSpeed += fSpeedDelta;
				if (m_fLastSpeed < fSpeed)
					fSpeed = m_fLastSpeed;
				else
					m_fLastSpeed = fSpeed;
			}
			else if (m_fLastSpeed > fSpeed)
			{
				m_fLastSpeed -= fSpeedDelta;
				if (m_fLastSpeed > fSpeed)
					fSpeed = m_fLastSpeed;
				else
					m_fLastSpeed = fSpeed;
			}
			if (fLastSpeed == GetSpeed() && IsStanding())
			{
				SetStandingState();
				ForceStop();
				if (m_bAutoAnimation)
				{
					if (bHeadUnderWater)
						PlayAnimation('s', false);
					else
						PlayAnimation((const char*)NULL, false);
				}
			}
		}
	}

	// get the biped's facing vector in y=0 plane.
	Vector3 vBipedFacing;
	GetSpeedDirection(&vBipedFacing);
	// physical radius
	float fRadius = GetPhysicsRadius();

	// if we have not reached position , we will move on.
	if (m_nMovementStyle == MOVESTYLE_HEIGHTONLY)
	{
		if (bReachPos == false && fSpeed != 0.f)
		{
			Vector3 vFacing;
			vFacing = vBipedFacing*float(m_fSpeed * dTimeDelta);
			vMovePos = vFacing + m_vPos;
		}
	}
	else if (bReachPos == false && fSpeed != 0.f && fRadius>0.0001f)
	{
		/** -	Cast a group of n (n=3) rays (group 0) in front of the character, which covers a region of (n-2)/(n-1)*Pi radian.
		* if several sensor rays hit some obstacles within the radius of the object, we will see if the world
		* impact normals are roughly the same. If so, the character is considered to be blocked by
		* a single wall..In case of a single blocking wall,we will try slide the character along the wall;
		* otherwise the character is stopped.
		*/
		{
			// get origin
			Vector3 orig = m_vPos;
			float fSensorHeight = GetPhysicsHeight()*SENSOR_HEIGHT_RATIO;
			orig.y += fSensorHeight;
			// compute sensor group 0. 
			g_sensorGroups[0].ComputeSensorGroup(orig, vBipedFacing, fRadius, GetPhysicsGroupMask());
		}

		bool bCanMove = false; // whether the character can move either directly or sliding along wall.

		// move the character according to its impact forces
		if (g_sensorGroups[0].GetHitWallCount() > 1)
		{
			/** it has hit multiple things, we will not move the object */
			bCanMove = false;
		}
		else
		{
			if (g_sensorGroups[0].GetHitWallCount() == 1)
			{
				bCanMove = true;
				bSlidingWall = true;

				/**
				* we use the average impact norm to get a tentative point where the biped is most likely to be in the next frame.
				* we then cast another ray (group 1) from this tentative point to get another impact point. if there is no impact point
				* within the radius of (fRadius+m_fSpeed * dTimeDelta), the object will move using the old facing vector.
				* without further processing. Otherwise, from the two impact points(of group 0 and 1), we can calculate the wall direction vector,
				* which will be used for sliding wall.
				*/
				{
					// we will try sliding the character along the wall.The wall normal is given by impactNorm (the surface norm).
					Vector3 vWallNorm = g_sensorGroups[0].GetAvgImpactNorm(); // use only its projection on the y=0 plane.
					vWallNorm.y = 0;
					ParaVec3Normalize(&vWallNorm, &vWallNorm);

					Vector3 vTentativeFacing;
					/** we will compute a tentative sliding wall facing(not the final one) as below:
					* vFacing = vWallNorm (X) (vFacing (X) vWallNorm);
					* and get a tentative new position of the character.
					*/
					vTentativeFacing = vBipedFacing.crossProduct(vWallNorm);
					vTentativeFacing = vWallNorm.crossProduct(vTentativeFacing);
					vTentativeFacing.normalise(); // just make it valid
					vTentativeFacing = vTentativeFacing*PENETRATION_DISTANCE;
					vTentativeFacing.y = 0;
					Vector3 vHitRayOrig = g_sensorGroups[0].m_vOrig + vTentativeFacing;
					Vector3 vHitRayDir = g_sensorGroups[0].GetHitRaySensor().vDir;
					float fSensorRange = fRadius + float(fSpeed * dTimeDelta);
					// compute sensor group 1. 
					if (!g_sensorGroups[0].IsPerfectWall())
					{
						g_sensorGroups[1].ComputeSensorGroup(vHitRayOrig, vHitRayDir, fSensorRange, GetPhysicsGroupMask(), 1);
					}
				}
				bool bMoveAlongOldFacing = false;
				if (g_sensorGroups[0].IsPerfectWall())
				{
					Vector3 vWallNormal = g_sensorGroups[0].GetAvgImpactNorm();

					/**
					* check if the hit sensor ray and the biped facing vector are on the same side of the wall vector.
					* if so, it means that the biped is currently walking into the wall, otherwise it is leaving the wall.
					* If the biped facing vector (V), the wall direction(W), and the hit ray direction (R), satisfies the condition
					* ((R cross W) dot (V cross W))>=0, the character is still trying to walk into the wall, we will enforce wall sliding using
					* wall direction vector, otherwise, the character will be allowed to move away using old direction.
					*/
					float fCosAngle = vWallNormal.dotProduct(vBipedFacing);
					if (fCosAngle <= 0)
					{
						if (fCosAngle < -0.95)
						{
							// force stop, if we are sliding along a wall with too little speed(angle). 
							if (g_sensorGroups[0].m_sensors[1].bIsHit)
								bReachPos = true;
							else
								// tricky; if the front ray does not hit anything, we will allow move along old facing. 
								bMoveAlongOldFacing = true;
						}
						else
						{
							Vector3 vRotateAxis;
							float speedScale = fCosAngle + 1;
							if (speedScale < 0.5)
								speedScale = 0.5;
							vRotateAxis = vWallNormal.crossProduct(vBipedFacing);
							Vector3 vSlidingWallDirection;
							if (vRotateAxis.y > 0){
								Matrix4 mat;
								ParaMatrixRotationY(&mat, MATH_PI / 2);
								ParaVec3TransformCoord(&vSlidingWallDirection, &vWallNormal, &mat);
							}
							else{
								Matrix4 mat;
								ParaMatrixRotationY(&mat, -MATH_PI / 2);
								ParaVec3TransformCoord(&vSlidingWallDirection, &vWallNormal, &mat);
							}
							Vector3 vFacing;
							vFacing = vSlidingWallDirection*float(fSpeed * dTimeDelta*speedScale);
							vMovePos = vFacing + m_vPos;
						}
					}
					else
						bMoveAlongOldFacing = true;
				}
				else if (g_sensorGroups[1].HasHitAnything())
				{
					// check to see if the object needs to follow the wall.
					Vector3 vWallDir = (g_sensorGroups[1].GetHitRaySensor().impactPoint - g_sensorGroups[0].GetHitRaySensor().impactPoint);
					vWallDir.y = 0;
					// g_sensorGroups[1].GetHitRaySensor().fDist;
					if (vWallDir != Vector3(0, 0, 0))
					{
						ParaVec3Normalize(&vWallDir, &vWallDir);

						/**
						* check if the hit sensor ray and the biped facing vector are on the same side of the wall vector.
						* if so, it means that the biped is currently walking into the wall, otherwise it is leaving the wall.
						* If the biped facing vector (V), the wall direction(W), and the hit ray direction (R), satisfies the condition
						* ((R cross W) dot (V cross W))>=0, the character is still trying to walk into the wall, we will enforce wall sliding using
						* wall direction vector, otherwise, the character will be allowed to move away using old direction.
						*/
						Vector3 tmp1, tmp2;
						tmp1 = g_sensorGroups[1].GetHitRaySensor().vDir.crossProduct(vWallDir);
						tmp2 = vBipedFacing.crossProduct(vWallDir);
						if (tmp1.y * tmp2.y > 0)
						{
							// if the object is walking into the wall, we will slide along the wall.
							float speedScale = vBipedFacing.dotProduct(vWallDir);
							Vector3 vFacing;
							vFacing = vWallDir * float(fSpeed * dTimeDelta*speedScale);
							vMovePos = vFacing + m_vPos;
							if (speedScale < 0.5f)
							{
								// force stop, if we are sliding along a wall with too little speed(angle). 
								// NOTE: this is modified 2009.1.10
								//ForceStop();
								bReachPos = true;
							}
						}
						else
							bMoveAlongOldFacing = true;
					}
				}
				else
					bMoveAlongOldFacing = true;

				/// we permit walking along the old facing vector. This is the case when the character is walking away from the wall.
				if (bMoveAlongOldFacing)
				{
					// maintain the direction.
					Vector3 vFacing;
					vFacing = vBipedFacing * (float)(fSpeed * dTimeDelta);
					vMovePos = vFacing + m_vPos;
				}

				/** Finally, we will also move the character slightly out of the wall, if it has run too deep in to it
				* this is done by casting a third group of rays(group 2) from the newly computed position using
				* the old facing vector. At this stage, we can know if the biped is in a corner by comparing sensor group 0 and 2.
				* If the biped is cornered, the biped will be restored to its original position. */
				if (vMovePos != m_vPos)
				{
					{
						Vector3 vOrig = g_sensorGroups[0].m_vOrig + (vMovePos - m_vPos);
						// compute sensor group 2. 
						g_sensorGroups[2].ComputeSensorGroup(vOrig, vBipedFacing, fRadius, GetPhysicsGroupMask());
					}
					if (g_sensorGroups[2].HasHitAnything())
					{
						if ((!bMoveAlongOldFacing) && g_sensorGroups[2].CompareWith(g_sensorGroups[0]))
						{
							// if the biped has reached a corner, stop it.
							vMovePos = m_vPos;
						}
						else
						{
							if (g_sensorGroups[2].IsPerfectWall())
							{
								Vector3 vPlanePoint = BlockCommon::ConvertToRealPosition(g_sensorGroups[0].m_block_pick_result.BlockX, g_sensorGroups[0].m_block_pick_result.BlockY, g_sensorGroups[0].m_block_pick_result.BlockZ, g_sensorGroups[0].m_block_pick_result.Side);
								vPlanePoint -= vMovePos;
								Plane wall_plane(vPlanePoint, g_sensorGroups[0].GetAvgImpactNorm());
								float fWallDist = wall_plane.getDistance(Vector3(0, 0, 0));

								float fReboundDistance = (0.23f - fWallDist); // 0.228f = sin(0.23)*g_blocksize
								if (fReboundDistance >= 0)
								{
									Vector3 vWallNormal = g_sensorGroups[0].GetAvgImpactNorm();
									Vector3 vFacing;
									vFacing = vWallNormal*fReboundDistance;
									vMovePos = vFacing + vMovePos;
								}
							}
							else
							{
								float fReboundDistance = (fRadius - g_sensorGroups[2].GetHitRaySensor().fDist - PENETRATION_DISTANCE);
								if (fReboundDistance >= 0)
								{
									Vector3 vFacing;
									vFacing = g_sensorGroups[2].GetHitRaySensor().vDir * (-fReboundDistance);
									vMovePos = vFacing + vMovePos;
								}
							}
						}
					}
				}
			}
			else
			{
				// if nothing is hit, simply move the character forward
				bCanMove = true;
				Vector3 vFacing;
				vFacing = vBipedFacing * float(fSpeed * dTimeDelta);
				vMovePos = vFacing + m_vPos;
			}

			if (bCanMove)
			{
			}
		}
	}

	/**
	* check to see if the character is in the air, or is climbing up stairs.
	* if so we will see the bReachPos to false, and allow the character to smoothly fall down or fly up.
	*/
	// calculate the vertical position of the character according to the larger of the terrain and physics height at its feet
	Vector3 orig = vMovePos;
	orig.y += GetPhysicsHeight();
	RayCastHitResult hit;

	// Get the closest shape
	float dist;
	IParaPhysicsActor* closestShape = CGlobals::GetPhysicsWorld()->GetPhysicsInterface()->RaycastClosestShape(
		CONVERT_PARAVECTOR3(orig), PARAVECTOR3(0, -1.f, 0), 0, hit, (int16)GetPhysicsGroupMask(), 10 * OBJ_UNIT);
	if (closestShape)
	{
		dist = hit.m_fDistance;
	}
	else
	{
		dist = MAX_DIST*OBJ_UNIT; // infinitely large
	}

	// set the object height to the higher of the two.
	float fTerrainHeight = CGlobals::GetGlobalTerrain()->GetElevation((float)vMovePos.x, (float)vMovePos.z);
	float fPhysicsHeight = orig.y - dist;

	bool isOnTerrain = false;
	bool isClimbableTerrain = true;

	Vector3 terrainNormal;

	if (!GetIsAlwaysAboveTerrain() && (fTerrainHeight > (m_vPos.y + GetPhysicsHeight())))
	{
		bIgnoreTerrain = true;
		bMaintainHeight = false;
		// the biped has gone into a terrain hole, we will subject the object to the physics object only.
		vMovePos.y = fPhysicsHeight;
		bUseGlobalTerrainNorm = false;
	}
	else
	{
		bIgnoreTerrain = false;

		/// take terrain in to account, we will adopt the higher of the two as the character's next height.
		if (fPhysicsHeight > fTerrainHeight)
		{
			// physics object is over the terrain objects.
			vMovePos.y = fPhysicsHeight;
			bUseGlobalTerrainNorm = false;
		}
		else
		{
			if (m_ignoreSlopeCollision || m_nMovementStyle == MOVESTYLE_HEIGHTONLY)
				isClimbableTerrain = true;
			else
				isClimbableTerrain = CGlobals::GetGlobalTerrain()->IsWalkable((float)vMovePos.x, (float)vMovePos.z, terrainNormal);
			/*
			if(m_vPos.y > (fTerrainHeight + GetPhysicsHeight()) )
			{
			// if player is well above the terrain, we will ignore the walkable area.
			isClimbableTerrain = true;
			}
			*/
			if (!isClimbableTerrain)
			{
				Vector3 destDir((float)(vMovePos.x - m_vPos.x), 0, (float)(vMovePos.z - m_vPos.z));
				Vector3	terrDir(terrainNormal.x, 0, terrainNormal.z);

				float dot = destDir.dotProduct(terrDir);
				//Going down to the hill
				if (dot >= 0)
					isClimbableTerrain = true;
				else
				{
					//climbing up
					Vector3 slideDir;
					Vector3 up(0, 1, 0);
					slideDir = terrDir.crossProduct(up);

					Vector3 invDestDir(-destDir.x, 0, -destDir.z);
					Vector3 sub;
					sub = invDestDir - terrDir;
					dot = sub.dotProduct(slideDir);

					if (dot > 0)
					{
						slideDir.x = -slideDir.x;
						slideDir.z = -slideDir.z;
					}

					Vector3 norSlideDir;
					norSlideDir = slideDir.normalisedCopy();

					double slideX = norSlideDir.x * 0.02f + m_vPos.x;
					double slideY = norSlideDir.z * 0.02f + m_vPos.z;
					float slideHeight = CGlobals::GetGlobalTerrain()->GetElevation((float)slideX, (float)slideY);
					if (slideHeight <= (float)m_vPos.y + 0.005)
					{
						fTerrainHeight = slideHeight;
						vMovePos.x = slideX;
						vMovePos.z = slideY;
					}
					else
					{
						fTerrainHeight = (float)m_vPos.y;
						vMovePos.x = m_vPos.x;
						vMovePos.z = m_vPos.z;
					}
				}
			}

			// the terrain is over the physics objects.
			vMovePos.y = fTerrainHeight;
			if (m_vPos.y < (vMovePos.y + GetPhysicsHeight()))
				/// if the object is not too far from the ground, it will subject to terrain surface norm.
				bUseGlobalTerrainNorm = true;
			else
				/// if the object is well above the ground(in air), it will not snap to terrain surface norm.
				bUseGlobalTerrainNorm = false;
		}

		if (bMaintainHeight && fMaintainHeight < vMovePos.y)
			fMaintainHeight = (float)vMovePos.y;
	}


	Vector3 vMinPos, vMaxPos;
	bool bUseMinMaxBox = CheckBlockWorld(vMovePos, vMinPos, vMaxPos, bUseGlobalTerrainNorm, bReachPos, vBipedFacing, (float)dTimeDelta);

	// TODO:  if there is neither physics object, nor terrain object below the biped. 
	// We will allow the biped to fall down to some value, and stay there.
	// we just suppose here that the lowest point in the world is LOWEST_WORLD = -1000.f
	// so the object will stop falling at that position.
	if (vMovePos.y < LOWEST_WORLD)
		vMovePos.y = LOWEST_WORLD;

	// move the object to the new location in the x,z plane
	m_vPos.x = vMovePos.x;
	m_vPos.z = vMovePos.z;

	/// animate the character vertically according to gravity.
	/// implement smooth fall down and jump up.
	if (m_vPos.y > vMovePos.y)
	{
		/** if the character is above the ground,  use the current vertical velocity.
		*/
		// if the object is standing on the ground in the last frame, and that it is suddenly in the air, 
		// we will give it a fall down speed which is the same as its current running speed.
		if (m_fSpeedVertical == 0.f)
			m_fSpeedVertical = -GetAbsoluteSpeed();

		float fLastSpeedVertical = m_fSpeedVertical;
		if (!bMaintainHeight)
		{
			m_fSpeedVertical -= fGravity*(float)dTimeDelta;
		}

		/** this ensures that flyable object will fly when in air. We do so, by setting the maximum vertical speed for flying object.
		maximum vertical speed  is proportional to the density. The closer to 0, the smaller the maximum vertical speed.
		*/
		if (m_fDensity <= FLY_DENSITY && m_fSpeedVertical < 0)
		{
			float MaxFlyVertical = m_fDensity / FLY_DENSITY*MAX_FLY_VERTICAL_SPEED;
			if (m_fSpeedVertical < -MaxFlyVertical)
				m_fSpeedVertical = -MaxFlyVertical;
		}

		if (!bMaintainHeight)
		{
			float dY = (float)dTimeDelta*(m_fSpeedVertical + fLastSpeedVertical) / 2.f;
			m_vPos.y += dY;
		}
		else
		{
			m_vPos.y = fMaintainHeight;
		}

		// if biped is in the air, this will ensure that it does not have a zero vertical speed.
		// this is because zero vertical speed stands for standing on the ground.
		if (m_fSpeedVertical == 0.f)
			m_fSpeedVertical = FLT_TOLERANCE;

		if (!bMaintainHeight)
		{
			if (m_vPos.y <= vMovePos.y)
			{
				if (bHeadUnderWater && m_fDensity > 0 && m_fDensity < 1.f)
				{
					// give some initial speed, so that character can float up. 
					m_fSpeedVertical = FLT_TOLERANCE;
				}
				else
				{
					m_fSpeedVertical = 0.f;
				}
				m_vPos.y = vMovePos.y;
				/** end jumping, if the biped is near the ground and has a downward vertical speed.*/
				{
					CBipedStateManager* pCharState = GetBipedStateManager();
					if (pCharState)
						pCharState->AddAction(CBipedStateManager::S_JUMP_END);
				}
			}
			else if (m_vPos.y > (vMovePos.y + GetPhysicsHeight()*0.25f) && m_fSpeedVertical < 0)
			{
				/** fall down, if the biped is well above the ground and has a downward vertical speed.*/
				{
					CBipedStateManager* pCharState = GetBipedStateManager();
					if (pCharState)
						pCharState->AddAction(CBipedStateManager::S_FALLDOWN);
				}
			}
		}
	}
	else if (m_vPos.y < vMovePos.y)
	{
		/** if the character is below the ground,  ensure that the character has at least max(CLIMBUP_SPEED,current_speed)
		* unless it already has a bigger vertical speed.
		*/
		// applying the vertical momentum, and check if it is jumping up.
		// a character is jumping up if it has a bigger vertical speed than the climbing up speed.
		// The climb up speed is defined as max(CLIMBUP_SPEED,current_speed)
		bool bIsJumpingUp = false;
		float fClimbUpSpeed = GetAbsoluteSpeed();
		if (fClimbUpSpeed < CLIMBUP_SPEED)
			fClimbUpSpeed = CLIMBUP_SPEED;
		if (m_fSpeedVertical > fClimbUpSpeed)
			fClimbUpSpeed = m_fSpeedVertical;

		float fLastSpeedVertical = m_fSpeedVertical;
		fClimbUpSpeed -= fGravity*(float)dTimeDelta;
		float dY = (float)dTimeDelta*(fClimbUpSpeed + fLastSpeedVertical) / 2.f;
		m_vPos.y += dY;

		// this is tricky: we will assume that the character is jumping if it has a jump up speed bigger then 0.2. 
		if (m_fSpeedVertical > 0.2)
		{
			bIsJumpingUp = true;
		}

		if (bIgnoreTerrain)
		{
			/* in case the biped is on physics object, we will ensure that the character does not leave the physical plane.
			* in other words, the character should stay below or on the physical plane.*/
			if (m_vPos.y >= vMovePos.y)
				m_vPos.y = vMovePos.y;
		}
		else
		{
			/* in case the biped is on the global terrain, we will ensure that the character is always over the ground.
			* in other words, the character should stay above or on the ground.*/
			if (m_vPos.y < vMovePos.y || (!bIsJumpingUp))
				m_vPos.y = vMovePos.y;
		}
		// TODO: this may lead the biped into standing state, perhaps a better way to do repair it, such as using an action state.
		if (!bIsJumpingUp)
		{
			if (bHeadUnderWater && m_fDensity > 0 && m_fDensity < 1.f)
			{
				// give some initial speed, so that character can float up. 
				m_fSpeedVertical = FLT_TOLERANCE;
			}
			else
			{
				m_fSpeedVertical = 0.f;
			}

			/** end jumping, if the biped is below the ground*/
			{
				CBipedStateManager* pCharState = GetBipedStateManager();
				if (pCharState)
					pCharState->AddAction(CBipedStateManager::S_JUMP_END);
			}
		}
	}
	else
	{
		/** if the character is on the ground,  make sure that it has a non-negative speed
		*/
		if (m_fSpeedVertical > 0.f)
		{
			float fLastSpeedVertical = m_fSpeedVertical;
			m_fSpeedVertical -= fGravity*(float)dTimeDelta;
			float dY = (float)dTimeDelta*(m_fSpeedVertical + fLastSpeedVertical) / 2.f;
			if (dY > 0.f)
			{
				m_vPos.y += dY;
			}
			else
			{
				m_fSpeedVertical = 0.f;
			}
		}
		else
		{
			if (bHeadUnderWater && m_fDensity > 0 && m_fDensity < 1.f)
			{
				// give some initial speed, so that character can float up. 
				m_fSpeedVertical = FLT_TOLERANCE;
			}
			else
			{
				m_fSpeedVertical = 0.f;
				CBipedStateManager* pCharState = GetBipedStateManager();
				if (pCharState && pCharState->IsFlying())
					pCharState->AddAction(CBipedStateManager::S_JUMP_END);
			}
		}
	}

	// Compute the norm (orientation) of the biped, and smoothly transform to it.
	{
		Vector3 vNorm;
		if (bUseGlobalTerrainNorm && !IsSkipTerrainNormal())
		{
			// get the global terrain norm
			vNorm = CBaseObject::GetNormal();
			// this normal value simulate a real biped on a slope
			if ((1.f - vNorm.y) > FLT_TOLERANCE)
			{
				Vector3 vAxis;
				vAxis = Vector3(0, 1, 0).crossProduct(vNorm).normalisedCopy();
				Vector3 vFacing;
				GetSpeedDirection(&vFacing);
				float fFactor = vAxis.dotProduct(vFacing);
				if (fabs(fFactor) < FLT_TOLERANCE)
					fFactor = 1;
				else
					fFactor = sqrt(1 - fFactor*fFactor);

				Matrix4 mx;
				ParaMatrixRotationAxis(&mx, vAxis, acos(vNorm.y)*fFactor);
				vNorm = Vector3(0, 1, 0)*mx;
			}
		}
		else
		{
			// For physics meshes. the norm is always (0,1,0)
			vNorm = Vector3::UNIT_Y;
		}

		m_vNormTarget = vNorm;
		if (m_vNorm != m_vNormTarget)
		{
			Math::SmoothMoveVec3(&m_vNorm, m_vNormTarget, m_vNorm, (float)(SPEED_NORM_TURN*dTimeDelta), 0);
			ParaVec3Normalize(&m_vNorm, &m_vNorm);
		}
		//Math::SmoothMoveVec3(&m_vNorm, vNorm, m_vNorm, (float)(0.2f*GetAbsoluteSpeed()*dTimeDelta), 0);
		//Math::SmoothMoveVec3(&m_vNorm, vNorm, m_vNorm, (float)(SPEED_NORM_TURN*dTimeDelta), 0);

	}

	if (pIsSlidingWall)
	{
		*pIsSlidingWall = bSlidingWall;
	}
	if (bUseMinMaxBox)
	{
		// this fixed a bug for walking on slab blocks
		if (m_vPos.y < vMinPos.y && m_fSpeedVertical < 0)
			m_fSpeedVertical = 0.f;
		BlockCommon::ConstrainPos(m_vPos, vMinPos, vMaxPos);
	}

	if (bReachPos && fabs(m_fSpeedVertical) > 0.1f)
	{
		bReachPos = false;
	}
	return bReachPos;
}

enum EnumBlockWalkState
{
	BLOCK_WALKSTATE_FREE = 0,
	BLOCK_WALKSTATE_OBSTRUCTION = 1,
	BLOCK_WALKSTATE_CLIMBABLE = 2,
	BLOCK_WALKSTATE_FENCE = 3,
};

#define IS_OBSTRUCTED(x)   ((x)>BLOCK_WALKSTATE_FREE)
#define IS_CLIMBABLE(x)   ((x) == BLOCK_WALKSTATE_CLIMBABLE)
//#define IS_CLIMBABLE_3(mat,x,y,z)   ((mat[x][y][z])==2 && ((mat[x][y+1][z]==2) || (mat[x][y-1][z]==2)) )
#define IS_CLIMBABLE_3(mat,x,y,z)   ((mat[x][y][z])==BLOCK_WALKSTATE_CLIMBABLE)
#define IS_FENCE(x)   ((x) == BLOCK_WALKSTATE_FENCE)

bool ParaEngine::CBipedObject::CheckBlockWorld(DVector3& vMovePos, Vector3& vMinPos, Vector3& vMaxPos, bool& bUseGlobalTerrainNorm, bool& bReachPos, Vector3& vBipedFacing, float fDeltaTime)
{
	bool bUseMinMaxBox = false;

	BlockWorldClient* pBlockWorldClient = BlockWorldClient::GetInstance();
	if (pBlockWorldClient && pBlockWorldClient->IsInBlockWorld())
	{
		// taking block world into consideration
		const float block_size = BlockConfig::g_blockSize;
		const float half_block_size = block_size*0.5f;
		const float quat_block_size = block_size*0.25f;

		uint16 block_x, block_y, block_z;
		BlockCommon::ConvertToBlockIndex((float)m_vPos.x, (float)m_vPos.y + quat_block_size, (float)m_vPos.z, block_x, block_y, block_z);
		int obstruction_matrix[3][4][3];

		Vector3 vFloorPos = BlockCommon::ConvertToRealPosition(block_x, block_y, block_z, 5);
		float fContainerBlockMinY = vFloorPos.y;
		// the min, max moving area. 
		vMinPos = vFloorPos;
		vMaxPos = vFloorPos;
		int block_x_tmp, block_y_tmp, block_z_tmp;
		vMaxPos.y += block_size*1.5f;
		Vector3 vInnerAABBMinPos = vFloorPos + Vector3(-quat_block_size, 0, -quat_block_size);
		Vector3 vInnerAABBMaxPos = vFloorPos + Vector3(quat_block_size, 0, quat_block_size);
		Vector3 vOuterAABBMinPos = vFloorPos + Vector3(-quat_block_size * 3, 0, -quat_block_size * 3);
		Vector3 vOuterAABBMaxPos = vFloorPos + Vector3(quat_block_size * 3, 0, quat_block_size * 3);

		bool bFallDown = false;

		/** head block index. if biped is 2-block high or 1 block high */
		int nBipedHeight = GetPhysicsHeight() > block_size ? 2 : 1;

		//
		// check the biped's nearby blocks and fill in obstruction matrix. obstruction_matrix[1][1][1] is where the biped's feet is in. 
		//
		for (int i = 0; i < 3; i++)
		{
			for (int j = 0; j < 3; j++)
			{
				for (int k = 0; k < 4; k++)
				{
					block_x_tmp = block_x + i - 1;
					block_y_tmp = block_y + k - 1;
					block_z_tmp = block_z + j - 1;
					if (block_y_tmp < 0 || block_y_tmp>255)
						obstruction_matrix[i][k][j] = 1;
					else
					{
						int result = BLOCK_WALKSTATE_FREE;
						uint16_t templateId = pBlockWorldClient->GetBlockTemplateIdByIdx((uint16)block_x_tmp, (uint16)block_y_tmp, (uint16)block_z_tmp);
						if (templateId > 0)
						{
							BlockTemplate* temp = pBlockWorldClient->GetBlockTemplate(templateId);
							// camera only collide with solid blocks rather than obstruction block. 
							if (temp)
							{
								if ((temp->GetAttFlag() & BlockTemplate::batt_obstruction) != 0)
								{
									if (temp->IsMatchAttribute(BlockTemplate::batt_solid))
									{
										result = BLOCK_WALKSTATE_OBSTRUCTION;
									}
									else if (temp->IsMatchAttribute(BlockTemplate::batt_blockcamera))
									{
										// detect slabs, etc that are only half block size. 
										result = BLOCK_WALKSTATE_OBSTRUCTION;
										if (k <= 1)
										{
											float fPhysicalHeight = temp->GetPhysicalHeight(pBlockWorldClient, (uint16)block_x_tmp, (uint16)block_y_tmp, (uint16)block_z_tmp);
											if (fPhysicalHeight > 0.01f && fPhysicalHeight < 0.6f)
											{
												// such as walking on slab that is only half the size of block.
												result = BLOCK_WALKSTATE_FREE;
												if (k == 1)
												{
													obstruction_matrix[i][k - 1][j] = BLOCK_WALKSTATE_OBSTRUCTION;
													if (i == 1 && j == 1)
													{
														float fDeltaHeightDist = (vFloorPos.y + fPhysicalHeight) - (float)m_vPos.y;
														// smoothly walk up to physical height. 
														if (fDeltaHeightDist > 0)
														{
															// walk up speed is 2 meter/sec
															if (fDeltaHeightDist > fDeltaTime*2.f)
															{
																fPhysicalHeight = (float)m_vPos.y + fDeltaTime*2.f - vFloorPos.y;
															}
														}

														vFloorPos.y += fPhysicalHeight;
														vMinPos.y += fPhysicalHeight;
														vMaxPos.y += fPhysicalHeight;
													}
												}
											}
										}
									}
									else
									{
										if ((temp->GetAttFlag() & BlockTemplate::batt_climbable) != 0)
										{
											// if obstruction and climbable. make the block beneath it obstruction(such as lily pad)
											if (k == 1)
												obstruction_matrix[i][k - 1][j] = BLOCK_WALKSTATE_OBSTRUCTION;
											result = BLOCK_WALKSTATE_FREE;
										}
										else
											result = BLOCK_WALKSTATE_FENCE;
									}
									bUseMinMaxBox = true;
								}
								else if ((i == 1 && j == 1) && (temp->GetAttFlag() & BlockTemplate::batt_climbable) != 0)
								{
									// if not obstruction and climbable, mostly ladder
									result = BLOCK_WALKSTATE_CLIMBABLE;
								}
							}

						}
						obstruction_matrix[i][k][j] = result;
					}
				}
			}
		}

		// if player is inside a ladder, making neighbor climbable. 
		for (int k = 0; k < 4; k++)
		{
			if (obstruction_matrix[1][k][1] == BLOCK_WALKSTATE_CLIMBABLE)
			{
				block_x_tmp = block_x;
				block_y_tmp = block_y + k - 1;
				block_z_tmp = block_z;

				// if climbable, but not obstruction. 
				uint32_t nData = pBlockWorldClient->GetBlockUserDataByIdx((uint16)block_x_tmp, (uint16)block_y_tmp, (uint16)block_z_tmp);
				if (nData == 1)
				{
					obstruction_matrix[1][k][0] = BLOCK_WALKSTATE_CLIMBABLE;
					if (obstruction_matrix[2][k][1] == BLOCK_WALKSTATE_FREE)
						obstruction_matrix[2][k][1] = BLOCK_WALKSTATE_OBSTRUCTION;
					if (obstruction_matrix[0][k][1] == BLOCK_WALKSTATE_FREE)
						obstruction_matrix[0][k][1] = BLOCK_WALKSTATE_OBSTRUCTION;
					bUseMinMaxBox = true;
				}
				else if (nData == 3)
				{
					obstruction_matrix[0][k][1] = BLOCK_WALKSTATE_CLIMBABLE;
					if (obstruction_matrix[1][k][0] == BLOCK_WALKSTATE_FREE)
						obstruction_matrix[1][k][0] = BLOCK_WALKSTATE_OBSTRUCTION;
					if (obstruction_matrix[1][k][2] == BLOCK_WALKSTATE_FREE)
						obstruction_matrix[1][k][2] = BLOCK_WALKSTATE_OBSTRUCTION;
					bUseMinMaxBox = true;
				}
				else if (nData == 4)
				{
					obstruction_matrix[2][k][1] = BLOCK_WALKSTATE_CLIMBABLE;
					if (obstruction_matrix[1][k][0] == BLOCK_WALKSTATE_FREE)
						obstruction_matrix[1][k][0] = BLOCK_WALKSTATE_OBSTRUCTION;
					if (obstruction_matrix[1][k][2] == BLOCK_WALKSTATE_FREE)
						obstruction_matrix[1][k][2] = BLOCK_WALKSTATE_OBSTRUCTION;
					bUseMinMaxBox = true;
				}
				else if (nData == 5)
				{
					obstruction_matrix[1][k][2] = BLOCK_WALKSTATE_CLIMBABLE;
					if (obstruction_matrix[0][k][1] == BLOCK_WALKSTATE_FREE)
						obstruction_matrix[0][k][1] = BLOCK_WALKSTATE_OBSTRUCTION;
					if (obstruction_matrix[2][k][1] == BLOCK_WALKSTATE_FREE)
						obstruction_matrix[2][k][1] = BLOCK_WALKSTATE_OBSTRUCTION;
					bUseMinMaxBox = true;
				}
				obstruction_matrix[1][k][1] = BLOCK_WALKSTATE_FREE;
			}
		}

		if (bUseMinMaxBox)
		{
			//
			// check the nearby x,z plane blocks 4,6,2,8 in numeric pad position with 2 blocks high and possibly 3 blocks high if biped is off-ground.
			//

			// suppose the character is 1.9 block high in block based world. 
			float max_off_ground_height = ((2 - 1.9f)*block_size) + vFloorPos.y;
			// whether biped is far from floor, which we will need to take the third floor into consideration.
			bool bIsOffGround = (m_vPos.y > max_off_ground_height);

			if (nBipedHeight >= 2)
			{
				// compute the moving area in x, z plane. 
				if (IS_OBSTRUCTED(obstruction_matrix[0][1][1]) || IS_OBSTRUCTED(obstruction_matrix[0][2][1]) || (bIsOffGround && IS_OBSTRUCTED(obstruction_matrix[0][3][1]))){
					vMinPos.x = vInnerAABBMinPos.x;
				}
				else{
					vMinPos.x = vOuterAABBMinPos.x;
				}
				if (IS_OBSTRUCTED(obstruction_matrix[2][1][1]) || IS_OBSTRUCTED(obstruction_matrix[2][2][1]) || (bIsOffGround && IS_OBSTRUCTED(obstruction_matrix[2][3][1]))){
					vMaxPos.x = vInnerAABBMaxPos.x;
				}
				else{
					vMaxPos.x = vOuterAABBMaxPos.x;
				}

				if (IS_OBSTRUCTED(obstruction_matrix[1][1][0]) || IS_OBSTRUCTED(obstruction_matrix[1][2][0]) || (bIsOffGround && IS_OBSTRUCTED(obstruction_matrix[1][3][0]))){
					vMinPos.z = vInnerAABBMinPos.z;
				}
				else{
					vMinPos.z = vOuterAABBMinPos.z;
				}
				if (IS_OBSTRUCTED(obstruction_matrix[1][1][2]) || IS_OBSTRUCTED(obstruction_matrix[1][2][2]) || (bIsOffGround && IS_OBSTRUCTED(obstruction_matrix[1][3][2]))){
					vMaxPos.z = vInnerAABBMaxPos.z;
				}
				else{
					vMaxPos.z = vOuterAABBMaxPos.z;
				}
			}
			else
			{
				// compute the moving area in x, z plane. 
				if (IS_OBSTRUCTED(obstruction_matrix[0][1][1]) || (bIsOffGround && IS_OBSTRUCTED(obstruction_matrix[0][2][1]))){
					vMinPos.x = vInnerAABBMinPos.x;
				}
				else{
					vMinPos.x = vOuterAABBMinPos.x;
				}
				if (IS_OBSTRUCTED(obstruction_matrix[2][1][1]) || (bIsOffGround && IS_OBSTRUCTED(obstruction_matrix[2][2][1]))){
					vMaxPos.x = vInnerAABBMaxPos.x;
				}
				else{
					vMaxPos.x = vOuterAABBMaxPos.x;
				}

				if (IS_OBSTRUCTED(obstruction_matrix[1][1][0]) || (bIsOffGround && IS_OBSTRUCTED(obstruction_matrix[1][2][0]))){
					vMinPos.z = vInnerAABBMinPos.z;
				}
				else{
					vMinPos.z = vOuterAABBMinPos.z;
				}
				if (IS_OBSTRUCTED(obstruction_matrix[1][1][2]) || (bIsOffGround && IS_OBSTRUCTED(obstruction_matrix[1][2][2]))){
					vMaxPos.z = vInnerAABBMaxPos.z;
				}
				else{
					vMaxPos.z = vOuterAABBMaxPos.z;
				}
			}

			//
			// check the ceiling floors
			// 
			if (IS_OBSTRUCTED(obstruction_matrix[1][1][1]))
			{
				// special case where current position is obstructed, but the one above it is not, we will move immediately to the above floor. 
				if (!IS_OBSTRUCTED(obstruction_matrix[1][2][1]))
				{
					vMinPos.y = vFloorPos.y + block_size;
					if (vMaxPos.y <= vMinPos.y)
						vMaxPos.y = vMinPos.y;
				}
			}
			else
			{
				if (m_vPos.y > max_off_ground_height)
				{
					// check for ceiling blocks and constrain area
					if (nBipedHeight >= 2)
					{
						if (IS_OBSTRUCTED(obstruction_matrix[1][3][1]))
						{
							vMaxPos.y = max_off_ground_height;
							// let the character fall down, if it has hit the ceiling. 
							bFallDown = true;
						}
					}
					else
					{
						if (IS_OBSTRUCTED(obstruction_matrix[1][2][1]))
						{
							vMaxPos.y = max_off_ground_height;
							// let the character fall down, if it has hit the ceiling. 
							bFallDown = true;
						}
					}
				}
			}
			//
			// check the under feet floors to see if we shall fall down 1 floor.
			// 
			bool bIsClimbing = false;
			if (!IS_OBSTRUCTED(obstruction_matrix[1][0][1]))
			{
				// check for under-feet blocks and constrain area
				if ((m_vPos.x<(vFloorPos.x - quat_block_size) && vMinPos.x < (vFloorPos.x - half_block_size) && IS_OBSTRUCTED(obstruction_matrix[0][0][1])) ||
					(m_vPos.x>(vFloorPos.x + quat_block_size) && vMaxPos.x >(vFloorPos.x + half_block_size) && IS_OBSTRUCTED(obstruction_matrix[2][0][1])) ||
					(m_vPos.z < (vFloorPos.z - quat_block_size) && vMinPos.z < (vFloorPos.z - half_block_size) && IS_OBSTRUCTED(obstruction_matrix[1][0][0])) ||
					(m_vPos.z>(vFloorPos.z + quat_block_size) && vMaxPos.z > (vFloorPos.z + half_block_size) && IS_OBSTRUCTED(obstruction_matrix[1][0][2]))
					)
				{
					// do not fall down since adjacent underground cell is solid. 
				}
				else
				{
					// let the character fall down, if it is in air.
					vMinPos.y = vFloorPos.y - block_size;

					// check if we can climb nearby blocks, so that we do not need to falldown
					if (IS_CLIMBABLE_3(obstruction_matrix, 1, 1, 0) || IS_CLIMBABLE_3(obstruction_matrix, 1, 1, 2) || IS_CLIMBABLE_3(obstruction_matrix, 2, 1, 1) || IS_CLIMBABLE_3(obstruction_matrix, 0, 1, 1))
					{
						bIsClimbing = true;
					}
				}
			}
			else if (GetSpeed() != 0.f)
			{
				// check if we can climb nearby blocks, so that we may climb up 
				if (IS_CLIMBABLE_3(obstruction_matrix, 1, 1, 0) || IS_CLIMBABLE_3(obstruction_matrix, 1, 1, 2) || IS_CLIMBABLE_3(obstruction_matrix, 2, 1, 1) || IS_CLIMBABLE_3(obstruction_matrix, 0, 1, 1))
				{
					bIsClimbing = true;
				}
			}

			//
			// Constrain the biped's new position(vMovePos) to min max box.
			//
			{
				DVector3 vNewPos = vMovePos;
				BlockCommon::ConstrainPos(vNewPos, vMinPos, vMaxPos);
				if (vMovePos.y <= vNewPos.y){
					bUseGlobalTerrainNorm = false;
				}
				else{
					float terrain_overrun = (float)vMovePos.y - vMaxPos.y;
					if (terrain_overrun > FLT_TOLERANCE && terrain_overrun < block_size)
					{
						// stop the biped in vertical y if the terrain is obstructed
						vMovePos.y = m_vPos.y;
						// stop the min/max, since the character position is not changed. 
						// bUseMinMaxBox = true;
						// this is tricky, we should disable vertical speed, otherwise the biped may still jump over the ceiling.
						m_fSpeedVertical = 0.f;
						bFallDown = false;
					}
					if (vNewPos.y < vMaxPos.y){
						// do not fall down if walking on terrain or physical object. 
						bFallDown = false;
					}
				}

				if (bUseMinMaxBox)
				{
					vMovePos = vNewPos;

					if (bFallDown)
						m_fSpeedVertical = -1.f;
				}
			}
			//
			// now we will check the corner block 7,9,1,3 in numeric pad position in x,z plane only.
			//
			if (bUseMinMaxBox)
			{
				if (vMovePos.x < vInnerAABBMinPos.x)
				{
					if (vMovePos.z < vInnerAABBMinPos.z)
					{
						if (IS_OBSTRUCTED(obstruction_matrix[0][1][0]) || IS_OBSTRUCTED(obstruction_matrix[0][2][0]) || (bIsOffGround && IS_OBSTRUCTED(obstruction_matrix[0][3][0]))){
							if ((vInnerAABBMinPos.z - vMovePos.z) < (vInnerAABBMinPos.x - vMovePos.x))
								vMovePos.z = vInnerAABBMinPos.z;
							else
								vMovePos.x = vInnerAABBMinPos.x;
						}
					}
					else if (vMovePos.z > vInnerAABBMaxPos.z)
					{
						if (IS_OBSTRUCTED(obstruction_matrix[0][1][2]) || IS_OBSTRUCTED(obstruction_matrix[0][2][2]) || (bIsOffGround && IS_OBSTRUCTED(obstruction_matrix[0][3][2]))){
							if ((vMovePos.z - vInnerAABBMaxPos.z) < (vInnerAABBMinPos.x - vMovePos.x))
								vMovePos.z = vInnerAABBMaxPos.z;
							else
								vMovePos.x = vInnerAABBMinPos.x;
						}
					}
				}
				else if (vMovePos.x > vInnerAABBMaxPos.x)
				{
					if (vMovePos.z < vInnerAABBMinPos.z)
					{
						if (IS_OBSTRUCTED(obstruction_matrix[2][1][0]) || IS_OBSTRUCTED(obstruction_matrix[2][2][0]) || (bIsOffGround && IS_OBSTRUCTED(obstruction_matrix[2][3][0]))){
							if ((vInnerAABBMinPos.z - vMovePos.z) < (vMovePos.x - vInnerAABBMaxPos.x))
								vMovePos.z = vInnerAABBMinPos.z;
							else
								vMovePos.x = vInnerAABBMaxPos.x;
						}
					}
					else if (vMovePos.z > vInnerAABBMaxPos.z)
					{
						if (IS_OBSTRUCTED(obstruction_matrix[2][1][2]) || IS_OBSTRUCTED(obstruction_matrix[2][2][2]) || (bIsOffGround && IS_OBSTRUCTED(obstruction_matrix[2][3][2]))){
							if ((vMovePos.z - vInnerAABBMaxPos.z) < (vMovePos.x - vInnerAABBMaxPos.x))
								vMovePos.z = vInnerAABBMaxPos.z;
							else
								vMovePos.x = vInnerAABBMaxPos.x;
						}
					}
				}
			}

			if (bIsClimbing)
			{
				m_fSpeedVertical = 0.f;
				if (bReachPos)
				{
					// fall down ladders if standing on ladders
					m_fSpeedVertical = -4.f;
				}
			}

			// if we have not reached position , we will move on. if there is a block in front of us, we will jump to on it automatically. 
			if (bUseMinMaxBox && bReachPos == false && GetSpeed() != 0.f)
			{
				if (m_bAutoWalkupBlock || (m_vPos.y - fContainerBlockMinY) > 0.5f)
				{
					bool bWalkUp = false;
					if (nBipedHeight >= 2)
					{
						if (vBipedFacing.x < 0)
						{
							if (vMinPos.x == vInnerAABBMinPos.x && !IS_OBSTRUCTED(obstruction_matrix[0][2][1]) && !IS_OBSTRUCTED(obstruction_matrix[0][3][1]) && !IS_FENCE(obstruction_matrix[0][1][1]))
							{
								bWalkUp = true;
							}
						}
						if (vBipedFacing.z < 0)
						{
							if (vMinPos.z == vInnerAABBMinPos.z && !IS_OBSTRUCTED(obstruction_matrix[1][2][0]) && !IS_OBSTRUCTED(obstruction_matrix[1][3][0]) && !IS_FENCE(obstruction_matrix[1][1][0]))
							{
								bWalkUp = true;
							}
						}
						if (vBipedFacing.x > 0)
						{
							if (vMaxPos.x == vInnerAABBMaxPos.x && !IS_OBSTRUCTED(obstruction_matrix[2][2][1]) && !IS_OBSTRUCTED(obstruction_matrix[2][3][1]) && !IS_FENCE(obstruction_matrix[2][1][1]))
							{
								bWalkUp = true;
							}
						}
						if (vBipedFacing.z > 0)
						{
							if (vMaxPos.z == vInnerAABBMaxPos.z && !IS_OBSTRUCTED(obstruction_matrix[1][2][2]) && !IS_OBSTRUCTED(obstruction_matrix[1][3][2]) && !IS_FENCE(obstruction_matrix[1][1][2]))
							{
								bWalkUp = true;
							}
						}
					}
					else
					{
						if (vBipedFacing.x < 0)
						{
							if (vMinPos.x == vInnerAABBMinPos.x && !IS_OBSTRUCTED(obstruction_matrix[0][2][1]) && !IS_FENCE(obstruction_matrix[0][1][1]))
							{
								bWalkUp = true;
							}
						}
						if (vBipedFacing.z < 0)
						{
							if (vMinPos.z == vInnerAABBMinPos.z && !IS_OBSTRUCTED(obstruction_matrix[1][2][0]) && !IS_FENCE(obstruction_matrix[1][1][0]))
							{
								bWalkUp = true;
							}
						}
						if (vBipedFacing.x > 0)
						{
							if (vMaxPos.x == vInnerAABBMaxPos.x && !IS_OBSTRUCTED(obstruction_matrix[2][2][1]) && !IS_FENCE(obstruction_matrix[2][1][1]))
							{
								bWalkUp = true;
							}
						}
						if (vBipedFacing.z > 0)
						{
							if (vMaxPos.z == vInnerAABBMaxPos.z && !IS_OBSTRUCTED(obstruction_matrix[1][2][2]) && !IS_FENCE(obstruction_matrix[1][1][2]))
							{
								bWalkUp = true;
							}
						}
					}


					if (bWalkUp)
					{
						if (!bFallDown)
						{
							// just give it a little vertical speed. 
							m_fSpeedVertical = 1.f;
						}
						vMovePos.y = m_vPos.y + GetSpeed()*fDeltaTime;
					}
				}

				if (bIsClimbing)
				{
					bool bIsClimbingUp = false;
					if (vMovePos.x == vInnerAABBMinPos.x && vMinPos.x == vInnerAABBMinPos.x)
					{
						if (IS_CLIMBABLE_3(obstruction_matrix, 0, 1, 1))
							bIsClimbingUp = true;
					}
					if (vMovePos.z == vInnerAABBMinPos.z && vMinPos.z == vInnerAABBMinPos.z)
					{
						if (IS_CLIMBABLE_3(obstruction_matrix, 1, 1, 0))
							bIsClimbingUp = true;
					}
					if (vMovePos.x == vInnerAABBMaxPos.x && vMaxPos.x == vInnerAABBMaxPos.x)
					{
						if (IS_CLIMBABLE_3(obstruction_matrix, 2, 1, 1))
							bIsClimbingUp = true;
					}
					if (vMovePos.z == vInnerAABBMaxPos.z && vMaxPos.z == vInnerAABBMaxPos.z)
					{
						if (IS_CLIMBABLE_3(obstruction_matrix, 1, 1, 2))
							bIsClimbingUp = true;
					}
					m_fSpeedVertical = (bIsClimbingUp) ? 4.f : -4.f;
				}
			}
		}
	}
	return bUseMinMaxBox;
}

bool CBipedObject::FlyTowards(double dTimeDelta, const DVector3& vPosTarget, float fStopDistance, bool * pIsSlidingWall)
{
	static SensorGroups g_sensorGroups[3];
	/** check to see if the maximum penetration distance allowed. In each simulation step, if the biped moves
	* longer than this step, it is further divided in to several evenly spaced steps.
	* the step length is determined by the biped's speed multiplied by the time delta.
	* See also NX_MIN_SEPARATION_FOR_PENALTY, and CBipedObject::MoveTowards()*/
	float fMaxPenetration = (float)(m_fSpeed * dTimeDelta);
	if (fMaxPenetration > PENETRATION_DISTANCE)
	{
		int nNumSteps = (int)ceil(fMaxPenetration / PENETRATION_DISTANCE);
		/// ignore PENETRATION_DISTANCE, if it takes longer than MAX_STEPS_PER_FRAME per frame.
		if (nNumSteps > MAX_STEPS_PER_FRAME)
		{

#ifdef ALLOW_HIGH_SPEED_PASSTHROUGH
			nNumSteps = MAX_STEPS_PER_FRAME;
			return MoveTowards_Linear(dTimeDelta, vPosTarget);
#else
			dTimeDelta = dTimeDelta * MAX_STEPS_PER_FRAME / nNumSteps;
			nNumSteps = MAX_STEPS_PER_FRAME;
#endif
		}
		double dT = dTimeDelta / nNumSteps;
		bool bReachedDestination = false;
		for (int i = 0; (i < nNumSteps) && (!bReachedDestination); i++)
		{
			bReachedDestination = FlyTowards(dT, vPosTarget, fStopDistance, pIsSlidingWall);
		}
		return bReachedDestination;
	}

	{
		// check to see whether the target is in movable region, if not, we only allow object to move if the object is already outside the movable region.
		const CShapeAABB * aabb = GetMovableRegion();
		if (aabb != 0)
		{
			Vector3 vMin = aabb->GetMin();
			Vector3 vMax = aabb->GetMax();
			// we only care about xz plane.
			if (vMin.x < vPosTarget.x && vMin.z<vPosTarget.z &&
				vMax.x>vPosTarget.x && vMax.z > vPosTarget.z)
			{
				// target is inside the movable region.
			}
			else
			{
				if (vMin.x <= m_vPos.x && vMin.z <= m_vPos.z &&
					vMax.x >= m_vPos.x && vMax.z >= m_vPos.z)
				{
					// biped is inside the movable region, we will stop it. 
					return true;
				}
			}
		}
	}

	bool bMaintainHeight = false;
	bool bSwimmingOnSurface = false;
	bool bFeetUnderWater, bHeadUnderWater;
	float fMaintainHeight = 0.f;
	//float fGravity = GRAVITY_CONSTANT;
	float fGravity = m_gravity;
	CBipedStateManager* pCharState = GetBipedStateManager();
	/** check whether the biped is under water. if so set water state.
	* the acceleration of biped under water is Gw= (1-e/D)*G -(V*V)*k
	* where G is gravity of the earth, D is the density of the object, V is the vertical speed of the biped.
	* e is the percentage of object body under water. It is 1 when object is completely underwater, and (0,1) when partially underwater, and 0 when not underwater.
	* k denotes water resistance, which is  0.1f.
	*/
	{
		// the height on the character body, when the character is floating on the water surface. 
		//	- this is feet position when biped density is larger than water density
		//	- this is head position when biped density is smaller than 0 (floating in the air)
		Vector3 vHeadPos = m_vPos;
		float fHeight = GetHeight();
		vHeadPos.y += fHeight;
		bFeetUnderWater = CGlobals::GetOceanManager()->IsPointUnderWater(m_vPos);
		bHeadUnderWater = CGlobals::GetOceanManager()->IsPointUnderWater(vHeadPos);
		if (m_fDensity > 0 && m_fDensity < 1.f && !bHeadUnderWater)
		{
			Vector3 vWaterLinePos = m_vPos;
			vWaterLinePos.y += fHeight * m_fDensity - 0.1f; // 0.1 is chosen arbitrary to make it smaller anyway. 
			bSwimmingOnSurface = CGlobals::GetOceanManager()->IsPointUnderWater(vWaterLinePos);
		}

		// Note: a biped is underwater only if the biped is below water level but ABOVE global terrain 
		if (bFeetUnderWater)
		{
			float fTerrainHeight = CGlobals::GetGlobalTerrain()->GetElevation((float)m_vPos.x, (float)m_vPos.z);
			if (m_vPos.y < (fTerrainHeight - 1.0f)){
				bFeetUnderWater = false;
				bHeadUnderWater = false;
			}
		}

		float e = 1.f;
		if (bFeetUnderWater)
		{
			if (bHeadUnderWater)
				e = 1.f;
			else
			{
				e = (CGlobals::GetOceanManager()->GetWaterLevel() - (float)m_vPos.y) / fHeight;
			}
		}
		else
			e = 0.f;
		if (bFeetUnderWater)
		{
			if (bSwimmingOnSurface || bHeadUnderWater)
			{
				// character is under water
				pCharState->AddAction(CBipedStateManager::S_IN_WATER);
			}
			else
			{
				// character is on its feet 
				pCharState->AddAction(CBipedStateManager::S_ON_WATER_SURFACE);
			}

			if (m_fDensity != 0)
				fGravity = (WATER_DENSITY - e / m_fDensity)*GRAVITY_CONSTANT;
			else
				fGravity = WATER_DENSITY*GRAVITY_CONSTANT;

			/** the larger the more resistance between [0,1]. */
#define UP_WATER_RESISTENCE_COEF	0.1f
#define DOWN_WATER_RESISTENCE_COEF	0.1f
			/** when an object is crossing the water/air surface with a speed less than WATER_SKIP_VELOCITY m/s, we will not allow the object
			* to escape to air or water, but rather floating at the height of m_fDensity*fHeight;
			*/
#define WATER_SKIP_VELOCITY			1.f

			if (bSwimmingOnSurface && m_fSpeedVertical == FLT_TOLERANCE)
			{
				// character is swimming on the water surface~
				bMaintainHeight = true;
				fMaintainHeight = CGlobals::GetOceanManager()->GetWaterLevel() - m_fDensity*fHeight;
			}
			else
			{
				if (m_fSpeedVertical > 0)
				{
					fGravity += m_fSpeedVertical*m_fSpeedVertical*UP_WATER_RESISTENCE_COEF;
				}
				else if (m_fSpeedVertical == 0.f)
				{
					if (bHeadUnderWater && m_fDensity > 0 && m_fDensity < 1.f)
					{
						// give some initial speed, so that character can float up. 
						m_fSpeedVertical = FLT_TOLERANCE;
					}
				}
				else
				{
					/** the fall down speed should be small.*/
					fGravity -= m_fSpeedVertical*m_fSpeedVertical*DOWN_WATER_RESISTENCE_COEF;
				}
				if (!bHeadUnderWater && m_fDensity < WATER_DENSITY)
				{
					/** when an object is crossing the water/air surface with a speed less than WATER_SKIP_VELOCITY m/s, we will not allow the object
					* to escape to air or water, but rather floating at the height of m_fDensity*fHeight;
					*/
					if (fabs(m_fSpeedVertical) < WATER_SKIP_VELOCITY)
					{
						bMaintainHeight = true;
						fMaintainHeight = CGlobals::GetOceanManager()->GetWaterLevel() - m_fDensity*fHeight;

						//if( fabs(e-m_fDensity)*fHeight <= FLT_TOLERANCE)
						{
							m_fSpeedVertical = FLT_TOLERANCE;
							fGravity *= 0.1f;
						}
					}
				}
			}
		}
		else
		{
			// character is on land or in air
			pCharState->AddAction(CBipedStateManager::S_ON_FEET);
		}
	}


	bool bReachPos = false;
	bool bSlidingWall = false; // whether the object is sliding along a wall.
	bool bUseGlobalTerrainNorm = false; // whether the biped will be oriented according to the global terrain 
	/// true to ignore global terrain physics. The object will fall through the global terrain. This is automatically
	/// set by the MoveTowards() method. If the height of the object is well below of the global terrain surface, it is assumed
	/// to be underground, such as in the cave or tunnel. The camera will also ignore collision with the terrain objects.
	bool bIgnoreTerrain = false;

	DVector3 vMovePos = m_vPos; // the position for the next frame without considering the height
	DVector3 vSub;
	float fDist;

	// get distance from target
	vSub = m_vPos - vPosTarget;
	fDist = (float)vSub.squaredLength();

	// check if we have already reached the position
	if (fStopDistance == 0)
	{
		if (fMaxPenetration * fMaxPenetration >= fDist)
		{
			// we're within reach
			bReachPos = true;
			// set the exact point
			vMovePos = vPosTarget;
		}
	}
	else if (fStopDistance > fDist)
	{
		/// if we're within reach, we will stop without reaching the exact target point
		bReachPos = true;
	}

	Vector3 vBipedFacing;
	Vector3 dir = vPosTarget - m_vPos;
	vBipedFacing = -vSub;
	vBipedFacing.normalise();

	// physical radius
	float fRadius = GetPhysicsRadius();

	// if we have not reached position , we will move on.
	if (bReachPos == false && GetSpeed() != 0.f)
	{
		/** -	Cast a group of n (n=3) rays (group 0) in front of the character, which covers a region of (n-2)/(n-1)*Pi radian.
		* if several sensor rays hit some obstacles within the radius of the object, we will see if the world
		* impact normals are roughly the same. If so, the character is considered to be blocked by
		* a single wall..In case of a single blocking wall,we will try slide the character along the wall;
		* otherwise the character is stopped.
		*/
		{
			// get origin
			Vector3 orig = m_vPos;
			float fSensorHeight = GetPhysicsHeight()*SENSOR_HEIGHT_RATIO;
			orig.y += fSensorHeight;
			// compute sensor group 0. 
			g_sensorGroups[0].ComputeSensorGroup(orig, vBipedFacing, fRadius, GetPhysicsGroupMask());
		}

		bool bCanMove = false; // whether the character can move either directly or sliding along wall.

		// move the character according to its impact forces
		if (g_sensorGroups[0].GetHitWallCount() > 1)
		{
			/** it has hit multiple things, we will not move the object */
			bCanMove = false;
		}
		else
		{
			if (g_sensorGroups[0].GetHitWallCount() == 1)
			{
				bCanMove = true;
				bSlidingWall = true;

				/**
				* we use the average impact norm to get a tentative point where the biped is most likely to be in the next frame.
				* we then cast another ray (group 1) from this tentative point to get another impact point. if there is no impact point
				* within the radius of (fRadius+m_fSpeed * dTimeDelta), the object will move using the old facing vector.
				* without further processing. Otherwise, from the two impact points(of group 0 and 1), we can calculate the wall direction vector,
				* which will be used for sliding wall.
				*/
				{
					// we will try sliding the character along the wall.The wall normal is given by impactNorm (the surface norm).
					Vector3 vWallNorm = g_sensorGroups[0].GetAvgImpactNorm(); // use only its projection on the y=0 plane.
					vWallNorm.y = 0;
					ParaVec3Normalize(&vWallNorm, &vWallNorm);

					Vector3 vTentativeFacing;
					/** we will compute a tentative sliding wall facing(not the final one) as below:
					* vFacing = vWallNorm (X) (vFacing (X) vWallNorm);
					* and get a tentative new position of the character.
					*/
					vTentativeFacing = vBipedFacing.crossProduct(vWallNorm);
					vTentativeFacing = vWallNorm.crossProduct(vTentativeFacing);
					ParaVec3Normalize(&vTentativeFacing, &vTentativeFacing); // just make it valid
					vTentativeFacing = vTentativeFacing*PENETRATION_DISTANCE;
					vTentativeFacing.y = 0;
					Vector3 vHitRayOrig = g_sensorGroups[0].m_vOrig + vTentativeFacing;
					Vector3 vHitRayDir = g_sensorGroups[0].GetHitRaySensor().vDir;
					float fSensorRange = fRadius + float(m_fSpeed * dTimeDelta);
					// compute sensor group 1. 
					g_sensorGroups[1].ComputeSensorGroup(vHitRayOrig, vHitRayDir, fSensorRange, GetPhysicsGroupMask(), 1);
				}
				bool bMoveAlongOldFacing = false;
				if (g_sensorGroups[1].HasHitAnything())
				{
					// check to see if the object needs to follow the wall.
					Vector3 vWallDir = (g_sensorGroups[1].GetHitRaySensor().impactPoint - g_sensorGroups[0].GetHitRaySensor().impactPoint);
					vWallDir.y = 0;
					// g_sensorGroups[1].GetHitRaySensor().fDist;
					if (vWallDir != Vector3(0, 0, 0))
					{
						ParaVec3Normalize(&vWallDir, &vWallDir);

						/**
						* check if the hit sensor ray and the biped facing vector are on the same side of the wall vector.
						* if so, it means that the biped is currently walking into the wall, otherwise it is leaving the wall.
						* If the biped facing vector (V), the wall direction(W), and the hit ray direction (R), satisfies the condition
						* ((R cross W) dot (V cross W))>=0, the character is still trying to walk into the wall, we will enforce wall sliding using
						* wall direction vector, otherwise, the character will be allowed to move away using old direction.
						*/
						Vector3 tmp1, tmp2;
						tmp1 = g_sensorGroups[1].GetHitRaySensor().vDir.crossProduct(vWallDir);
						tmp2 = vBipedFacing.crossProduct(vWallDir);
						if (tmp1.y * tmp2.y > 0)
						{
							// if the object is walking into the wall, we will slide along the wall.
							float speedScale = vBipedFacing.dotProduct(vWallDir);
							Vector3 vFacing;
							vFacing = vWallDir * float(m_fSpeed * dTimeDelta*speedScale);
							vMovePos = vFacing + m_vPos;
							if (speedScale < 0.5f)
							{
								// force stop, if we are sliding along a wall with too little speed(angle). 
								// NOTE: this is modified 2009.1.10
								//ForceStop();
								bReachPos = true;
							}
						}
						else
							bMoveAlongOldFacing = true;
					}
				}
				else
					bMoveAlongOldFacing = true;

				/// we permit walking along the old facing vector. This is the case when the character is walking away from the wall.
				if (bMoveAlongOldFacing)
				{
					// maintain the direction.
					Vector3 vFacing;
					vFacing = vBipedFacing * (float)(m_fSpeed * dTimeDelta);
					vMovePos = vFacing + m_vPos;
				}

				/** Finally, we will also move the character slightly out of the wall, if it has run too deep in to it
				* this is done by casting a third group of rays(group 2) from the newly computed position using
				* the old facing vector. At this stage, we can know if the biped is in a corner by comparing sensor group 0 and 2.
				* If the biped is cornered, the biped will be restored to its original position. */
				if (vMovePos != m_vPos)
				{
					{
						Vector3 vOrig = g_sensorGroups[0].m_vOrig + (vMovePos - m_vPos);
						// compute sensor group 2. 
						g_sensorGroups[2].ComputeSensorGroup(vOrig, vBipedFacing, fRadius, GetPhysicsGroupMask());
					}
					if (g_sensorGroups[2].HasHitAnything())
					{
						if ((!bMoveAlongOldFacing) && g_sensorGroups[2].CompareWith(g_sensorGroups[0]))
						{
							// if the biped has reached a corner, stop it.
							vMovePos = m_vPos;
						}
						else
						{
							float fReboundDistance = (fRadius - g_sensorGroups[2].GetHitRaySensor().fDist - PENETRATION_DISTANCE);
							if (fReboundDistance >= 0)
							{
								Vector3 vFacing;
								vFacing = g_sensorGroups[2].GetHitRaySensor().vDir *  (-fReboundDistance);
								vMovePos = vFacing + vMovePos;
							}
						}
					}
				}
			}
			else
			{
				// if nothing is hit, simply move the character forward
				bCanMove = true;
				Vector3 vFacing;
				vFacing = vBipedFacing * float(m_fSpeed * dTimeDelta);
				vMovePos = vFacing + m_vPos;
			}

			if (bCanMove)
			{
			}
		}
	}

	/**
	* check to see if the character is in the air, or is climbing up stairs.
	* if so we will see the bReachPos to false, and allow the character to smoothly fall down or fly up.
	*/
	// calculate the vertical position of the character according to the larger of the terrain and physics height at its feet
	Vector3 orig = vMovePos;
	orig.y += GetPhysicsHeight();
	RayCastHitResult hit;

	// Get the closest shape
	float dist;
	IParaPhysicsActor* closestShape = CGlobals::GetPhysicsWorld()->GetPhysicsInterface()->RaycastClosestShape(
		CONVERT_PARAVECTOR3(orig), PARAVECTOR3(0, -1.f, 0), 0, hit, (int16)GetPhysicsGroupMask(), 10 * OBJ_UNIT);
	if (closestShape)
	{
		dist = hit.m_fDistance;
	}
	else
	{
		dist = MAX_DIST*OBJ_UNIT; // infinitely large
	}

	// set the object height to the higher of the two.
	float fTerrainHeight = CGlobals::GetGlobalTerrain()->GetElevation((float)vMovePos.x, (float)vMovePos.z);
	float fPhysicsHeight = orig.y - dist;

	bool isOnTerrain = false;
	bool isClimbableTerrain = true;

	Vector3 terrainNormal;

	if (!m_readyToLanding)
	{
		float height = (fPhysicsHeight > fTerrainHeight) ? fPhysicsHeight : fTerrainHeight;
		if (vMovePos.y - height > 0.5f)
			m_readyToLanding = true;
	}

	float low = fTerrainHeight;
	if (closestShape)
	{
		if (fPhysicsHeight < fTerrainHeight)
			low = fPhysicsHeight;
	}
	if (vMovePos.y < low)
		vMovePos.y = low;

	//always above water
	if (CGlobals::GetOceanManager()->OceanEnabled())
	{
		float waterHeight = CGlobals::GetOceanManager()->GetWaterLevel();
		if (vMovePos.y <= waterHeight)
			vMovePos.y += 0.5f;
	}

	Vector3 vMinPos, vMaxPos;
	bool bUseMinMaxBox = CheckBlockWorld(vMovePos, vMinPos, vMaxPos, bUseGlobalTerrainNorm, bReachPos, vBipedFacing, (float)dTimeDelta);

	// TODO:  if there is neither physics object, nor terrain object below the biped. 
	// We will allow the biped to fall down to some value, and stay there.
	// we just suppose here that the lowest point in the world is LOWEST_WORLD = -1000.f
	// so the object will stop falling at that position.
	if (vMovePos.y < LOWEST_WORLD)
		vMovePos.y = LOWEST_WORLD;

	// move the object to the new location
	m_vPos.x = vMovePos.x;
	m_vPos.z = vMovePos.z;
	m_vPos.y = vMovePos.y;

	if (vMovePos.y > -LOWEST_WORLD)
		vMovePos.y = -LOWEST_WORLD;

	if (m_readyToLanding && ((fabs((vMovePos.y - fPhysicsHeight)) < 0.02f) || (fabs((vMovePos.y - fTerrainHeight)) < 0.02f)))
	{
		m_readyToLanding = false;

		if (!m_isAlwaysFlying)
		{
			m_isFlying = false;
			CBipedStateManager* pCharState = GetBipedStateManager();
			if (pCharState)
				pCharState->AddAction(CBipedStateManager::S_JUMP_END);
		}
	}

	/// animate the character vertically according to gravity.
	/// implement smooth fall down and jump up.

	if (m_fSpeedVertical > 0.f)
	{
		float fLastSpeedVertical = m_fSpeedVertical;
		m_fSpeedVertical -= fGravity*(float)dTimeDelta;
		if (m_fSpeedVertical < 0)
		{
			m_fSpeedVertical = 0;
		}
		float dY = (float)dTimeDelta*(m_fSpeedVertical + fLastSpeedVertical) / 2.f;
		m_vPos.y += dY;
	}


	if (pIsSlidingWall)
	{
		*pIsSlidingWall = bSlidingWall;
	}
	if (!m_isFlying)
	{
		m_fSpeedVertical = -GetAbsoluteSpeed();
	}
	if (bUseMinMaxBox)
	{
		BlockCommon::ConstrainPos(m_vPos, vMinPos, vMaxPos);
	}
	return bReachPos;
}

//-----------------------------------------------------------------------------
// Name: CBipedObject::AnimateMoving()
/// Desc: Here we try to figure out if we're moving and can keep moving, 
///       or if we're waiting / blocked and must keep waiting / blocked,
///       or if we have reached our destination.
/// bSharpTurning: when set this to true, the biped is first turned towards the destination
///  and then moves towards it, otherwise, the biped will move while turning.
//-----------------------------------------------------------------------------
void CBipedObject::AnimateMoving(double dTimeDelta, bool bSharpTurning)
{
	BipedWayPoint waypoint;
	if (GetWayPoint(&waypoint) == false) /* no target so no animation */
	{
		// action: the biped should be set to standing state
		//-----------------------------------------------------------
		// Default action:
		// when we have reached the position, play Loiter movie. 
		// automatically. This is a default action, it can be overridden 
		// by LLE, where the environment simulator detects that the object
		// has reached the point and assign it another job. This default
		// action just ensured that the object ceased after reaching the point.
		// Environment simulator can detect this, and immediately assign another
		// job. Also environment simulator can call loiter action explicitly.
		//-----------------------------------------------------------
		if (m_fSpeedVertical == 0.f)
		{
			ForceStop();
			if (m_vNorm != m_vNormTarget)
			{
				Math::SmoothMoveVec3(&m_vNorm, m_vNormTarget, m_vNorm, (float)(SPEED_NORM_TURN*dTimeDelta), 0);
				ParaVec3Normalize(&m_vNorm, &m_vNorm);
			}
		}
		else
			MoveTowards(dTimeDelta, GetPosition(), 1.f);

		// facing is still enabled in this mode
		if (m_fYaw != m_fFacingTarget)
			FacingTowards(dTimeDelta, m_fFacingTarget);

		return;
	}
	else if (waypoint.nType == BipedWayPoint::COMMAND_MOVING)
	{
		Vector3 vFacing;
		GetSpeedDirection(&vFacing);
		DVector3 vPos = GetPosition();
		vPos += vFacing;
		MoveTowards(dTimeDelta, vPos, 0.f);
	}
	else if (waypoint.nType == BipedWayPoint::COMMAND_POINT)
	{
		float fFacingTarget = 0.f;
		bool bReachedPos = false;
		if (Math::ComputeFacingTarget(waypoint.vPos, m_vPos, fFacingTarget))
		{

			// turn the speed angle immediately
			SetSpeedAngle(fFacingTarget);
			// turn the render model smoothly. 
			float fTmp = GetFacing();
			Math::SmoothMoveAngle1(fTmp, Math::ToStandardAngle(fFacingTarget), BODY_TURNING_SPEED*(float)dTimeDelta);
			SetFacing(fTmp);
			// TODO: set head turning angle using waypoint.vFacing
			// move the character to destination with the current speed angle and speed
			bReachedPos = MoveTowards(dTimeDelta, waypoint.vPos, 0);
		}
		else
		{
			if (waypoint.vPos.y != m_vPos.y)
			{
				bReachedPos = MoveTowards(dTimeDelta, waypoint.vPos, 0);
			}
			else
			{
				bReachedPos = true;
			}
		}
		if (bReachedPos)
		{
			if (waypoint.IsUseFacing())
			{
				RemoveWayPoint();
				FacingTarget(waypoint.fFacing); // facing target.
			}
			else
			{
				RemoveWayPoint();
			}
		}
	}
	else if (waypoint.nType == BipedWayPoint::COMMAND_MOUNT)
	{
		// rotate the character to face target
		bool bMounted = false;
		// here we assume the first reference object is the mount object. 
		//CBaseObject* pTarget = (CBaseObject*) GetRefObjectByName(waypoint.m_sTarget.c_str());
		CBaseObject* pTarget = NULL;
		RefListItem* item = GetRefObjectByTag(0);
		if (item != 0)
			pTarget = (CBaseObject*)item->m_object;

		if (pTarget)
		{
#ifdef USE_MOUNT_FACING
			/**
			* Set align norm and facing 
			*/
#ifdef SMOOTH_MOUNTING_FACING
			float fDeltaTime = (float)dTimeDelta;
			float fFacing = GetFacing();
			Math::SmoothMoveAngle1(fFacing, pTarget->GetFacing()+waypoint.fFacing, MOUNT_FACING_SPEED*fDeltaTime);
			SetFacing(fFacing); 
#else
			SetFacing(pTarget->GetFacing()+waypoint.fFacing);
#endif
#endif
			/**
			* Set world position from the mount point
			*/
			Matrix4 mxWorld;
			// Note: use the render matrix from last render frame is fine for simulation. 
			if (GetRenderMatrix(mxWorld))
			{
				Vector3 vMountRenderPos;
				ParaVec3TransformCoord(&vMountRenderPos, &Vector3::ZERO, &mxWorld);
				DVector3 vMountPos(vMountRenderPos + CGlobals::GetScene()->GetRenderOrigin());
#ifdef SMOOTH_MOUNTING_POS
				// Smooth move to that position, use some inertia to prevent camera jerking on horse back
				DVector3 vNewPos = GetPosition(&vNewPos);

				if((vNewPos- vMountPos).squaredLength()<=2.0f)
				{
					float fStep = max(MOUNT_POS_SPEED, pTarget->GetSpeed()) * fDeltaTime;
					Math::SmoothMoveFloat1(vNewPos.x, vMountPos.x, fStep);
					if(vMountPos.y > vNewPos.y)
						vNewPos.y = vMountPos.y;
					else
						Math::SmoothMoveFloat1(vNewPos.y, vMountPos.y, fStep);
					Math::SmoothMoveFloat1(vNewPos.z, vMountPos.z, fStep);
					SetPosition( vNewPos);
				}
				else
					SetPosition( vMountPos);
#else
				SetPosition(vMountPos);
#endif
				UpdateTileContainer();
			}
			bMounted = true;
		}
		if (!bMounted)
		{
			RemoveWayPoint();
			CBipedStateManager* pState = GetBipedStateManager();
			if (pState)
			{
				pState->SetMounted(false);
			}
			// TODO: shift to some other walking state to prevent staying in the air, such as jumping.
		}
	}
	else if (waypoint.nType == BipedWayPoint::COMMAND_FACING)
	{
		// rotate the character to face target
		if (FacingTowards(dTimeDelta, waypoint.fFacing))
		{
			RemoveWayPoint();
		}
	}
	else if (waypoint.nType == BipedWayPoint::PATHFINDING_POINT)
	{
		/// when it is a pathingfinding-point, we will implement simple movement
		/// in which biped is first turned and then moved.
		float fFacingTarget;
		if (Math::ComputeFacingTarget(waypoint.vPos, m_vPos, fFacingTarget))
			m_fFacingTarget = fFacingTarget;

		if (FacingTowards(dTimeDelta * 2 /*doubles turning speed*/, m_fFacingTarget))
		{
			if (MoveTowards(dTimeDelta, waypoint.vPos, 0.f))
			{
				RemoveWayPoint();
			}
		}
	}
	else if (waypoint.nType == BipedWayPoint::BLOCKED)
	{
		if (waypoint.fTimeLeft >= 0.0f)
			waypoint.fTimeLeft = max((float)(waypoint.fTimeLeft - dTimeDelta), 0.0f);
		if (waypoint.fTimeLeft == 0.0f)
		{
			RemoveWayPoint();
		}
		else
		{
			/// only facing the target when in blocked mode
			/// action: the biped should be set to standing state
			ForceStop();
			FacingTowards(dTimeDelta, m_fFacingTarget);
		}
	}
}

void CBipedObject::GetSpeedDirection(Vector3 *pV)
{
	Matrix4 m;
	if (!(m_isFlying && GetFlyUsingCameraDir()))
	{
		if (m_isFlying)
		{
			CBipedStateManager* pCharState = GetBipedStateManager();
			if (pCharState && pCharState->FindStateInMemory(CBipedStateManager::STATE_FLY_DOWNWARD) >= 0)
			{
				*pV = -Vector3::UNIT_Y;
				return;
			}
		}
		ParaVec3TransformCoord(pV, &Vector3::UNIT_X, ParaMatrixRotationY(&m, (GetSpeed() < 0.f) ? (-m_fSpeedAngle) : m_fSpeedAngle));
	}
	else
	{
		CBipedStateManager* pCharState = GetBipedStateManager();
		if (pCharState)
		{
			Vector3 vFlyingDir = m_flyingDir;
			bool bMovingForward = (pCharState->FindStateInMemory(CBipedStateManager::STATE_WALK_FORWORD) >= 0);
			bool bMovingBackward = (pCharState->FindStateInMemory(CBipedStateManager::STATE_WALK_BACKWORD) >= 0);
			if (bMovingBackward)
			{
				vFlyingDir = -vFlyingDir;
			}
			else if (pCharState->FindStateInMemory(CBipedStateManager::STATE_FLY_DOWNWARD) >= 0)
			{
				vFlyingDir.x = 0;
				vFlyingDir.y = -1.f;
				vFlyingDir.z = 0;
			}

			float fTurningAngle = Math::PI * 0.5f;
			if (bMovingForward)
				fTurningAngle *= 0.5f;
			else if (bMovingBackward)
				fTurningAngle *= -0.5f;

			if (pCharState->FindStateInMemory(CBipedStateManager::STATE_WALK_RIGHT) >= 0)
			{
				Vector3 temp(vFlyingDir);
				temp.y = 0;
				Vector3 flatDir;
				ParaVec3Normalize(&flatDir, &temp);
				ParaMatrixRotationY(&m, fTurningAngle);
				temp = flatDir * m;
				vFlyingDir = temp;
			}
			else if (pCharState->FindStateInMemory(CBipedStateManager::STATE_WALK_LEFT) >= 0)
			{
				Vector3 temp(vFlyingDir);
				temp.y = 0;
				Vector3 flatDir;
				ParaVec3Normalize(&flatDir, &temp);
				ParaMatrixRotationY(&m, -fTurningAngle);
				temp = flatDir*m;
				vFlyingDir = temp;
			}
			
			*pV = vFlyingDir;
		}
		else
		{
			*pV = m_flyingDir;
		}
	}

}

/**
* @params fFacingTarget: it  must be in the range [0, 2*pi]
*/
bool CBipedObject::FacingTowards(double dTimeDelta, float fFacingTarget)
{
	return Math::SmoothMoveAngle1(m_fYaw, fFacingTarget, SPEED_TURN * (float)dTimeDelta);
}

void CBipedObject::SetNextTargetPosition(const DVector3& v, bool bIgnoreHeight)
{
	if (bIgnoreHeight)
	{
		DVector3 vPos(v.x, m_vPos.y, v.z);
		if (m_vPosTarget.empty())
		{
			m_vPosTarget.push_front(BipedWayPoint(vPos));
		}
		else
		{
			m_vPosTarget.front().vPos = vPos;
		}
	}
	else
	{
		if (m_vPosTarget.empty())
		{
			m_vPosTarget.push_front(BipedWayPoint(v));
		}
		else
		{
			m_vPosTarget.front().vPos = v;
		}
	}
}

void CBipedObject::SetYaw(float fYaw)
{
	if (m_fYaw != fYaw || m_fFacingTarget != fYaw)
	{
		m_fYaw = m_fFacingTarget = fYaw;
		SetGeometryDirty(true);
	}
}


float ParaEngine::CBipedObject::GetRoll()
{
	return m_fRoll;
}

void ParaEngine::CBipedObject::SetRoll(float fValue)
{
	if (m_fRoll != fValue)
	{
		m_fRoll = fValue;
		SetGeometryDirty(true);
	}
}

float ParaEngine::CBipedObject::GetPitch()
{
	return m_fPitch;
}

void ParaEngine::CBipedObject::SetPitch(float fValue)
{
	if (m_fPitch != fValue)
	{
		m_fPitch = fValue;
		SetGeometryDirty(true);
	}
}

float ParaEngine::CBipedObject::GetAssetHeight()
{
	return m_fAssetHeight;
}

// TODO: update for roll, pitch, yaw in additional to size
// TODO: override GetAABB and GetOBB in IViewClippingObject to provide more accurate size due to bmax model.
void ParaEngine::CBipedObject::UpdateGeometry()
{
	SetGeometryDirty(false);
	CAnimInstanceBase* pAI = GetAnimInstance();
	if (pAI && m_pMultiAnimationEntity)
	{
		float fScale = GetSizeScale();
		pAI->SetSizeScale(fScale);
		float fRadius = 0.2f;
		if (m_pMultiAnimationEntity->GetType() == AssetEntity::parax)
		{
			CParaXModel* pModel = ((ParaXEntity*)m_pMultiAnimationEntity.get())->GetModel();
			if (pModel != 0)
			{
				fRadius = pModel->GetBoundingRadius();
				// we will use strict bounding box. 
				if (fRadius == 0.f)
				{
					SetRadius(0.f);
					m_fAssetHeight = 0.f;
				}
				else
				{
					Vector3 vMin = pModel->GetHeader().minExtent;
					Vector3 vMax = pModel->GetHeader().maxExtent;
					
					m_fAssetHeight = vMax.y*fScale;
					Matrix4 mat;
					GetLocalWorldTransform(mat);
					CShapeOBB obb(CShapeBox(vMin, vMax), mat);
					CShapeBox minmaxBox;
					minmaxBox.Extend(obb);
					SetAABB(&minmaxBox.GetMin(), &minmaxBox.GetMax());
				}
			}
			else
			{
				SetRadius(0.5f);
				m_fAssetHeight = 1.0f;
			}
		}
		/** uncomment to let the physics radius automatically scales */
		// SetPhysicsRadius(GetPhysicsRadius()*fScale/fOldScale);

		UnloadPhysics();
		if (m_dwPhysicsMethod == 0)
			m_dwPhysicsMethod = PHYSICS_LAZY_LOAD;
	}
	else
	{
		// just a default radius for debugging purposes only
		SetRadius(0.2f);
		m_fAssetHeight = 0.4f;
	}
}

void CBipedObject::SetHeadTurningAngle(float fFacing)
{
	CharModelInstance* pChar = GetCharModelInstance();
	if (pChar)
	{
		pChar->SetUpperBodyTurningAngle(Math::ToStandardAngle(fFacing));
	}
}

float CBipedObject::GetHeadTurningAngle()
{
	CharModelInstance* pChar = GetCharModelInstance();
	if (pChar)
		return pChar->GetCurrrentUpperBodyTurningAngle();
	return 0;
}


void ParaEngine::CBipedObject::SetHeadUpdownAngle(float fFacing)
{
	CharModelInstance* pChar = GetCharModelInstance();
	if (pChar)
	{
		pChar->SetUpperBodyUpdownAngle(Math::ToStandardAngle(fFacing));
	}
}

float ParaEngine::CBipedObject::GetHeadUpdownAngle()
{
	CharModelInstance* pChar = GetCharModelInstance();
	if (pChar)
		return pChar->GetCurrrentUpperBodyUpdownAngle();
	return 0;
}

float CBipedObject::GetHeadFacing()
{
	return GetFacing() + GetHeadTurningAngle();
}
void CBipedObject::FacingTargetImmediately()
{
	if (m_vPosTarget.empty())
		return;
	Math::ComputeFacingTarget(m_vPosTarget.front().vPos, m_vPos, m_fYaw);
	m_fFacingTarget = m_fYaw;
}

void CBipedObject::FacingTarget(float fTargetFacing)
{
	m_fFacingTarget = fTargetFacing;
}
void CBipedObject::FacingTarget(const Vector3 *pV)
{
	// facing target right away.
	if (IsStanding())
	{
		Math::ComputeFacingTarget(*pV, m_vPos, m_fFacingTarget);
	}
}

void CBipedObject::SetDensity(float fDensity)
{
	m_fDensity = fDensity;
}

float CBipedObject::GetDensity()
{
	return m_fDensity;
}

void CBipedObject::SetSpeedAngle(float fSpeedAngle)
{
	m_fSpeedAngle = fSpeedAngle;
}

float CBipedObject::GetSpeedAngle()
{
	return m_fSpeedAngle;
}

// speed cmd
float CBipedObject::GetSpeed()
{
	return m_fSpeed;
}
float CBipedObject::GetAbsoluteSpeed()
{
	if (m_fSpeed < 0)
		return -m_fSpeed;
	else
		return m_fSpeed;
}
void CBipedObject::SetSpeed(float fSpeed)
{
	m_fSpeed = fSpeed;
}

float CBipedObject::GetMaxSpeed()
{
	// stop whatever action the pObj is doing
	if (!m_pAI || m_pAI->GetValidAnimID(4) <= 0)
		return 0.f;
	float speed = 0.f;
	m_pAI->GetSpeedOf("Walk", &speed);
	return speed;
}

float CBipedObject::GetSpeedScale()
{
	return m_fSpeedScale;
}
void CBipedObject::SetSpeedScale(float fScale)
{
	m_fSpeedScale = fScale;

	if (m_pAI)
	{
		m_pAI->SetSpeedScale(fScale);
	}
}

float CBipedObject::GetSizeScale()
{
	return m_fSizeScale;
}

void CBipedObject::SetScaling(float fScale)
{
	SetSizeScale(fScale);
}

void CBipedObject::Rotate(float x, float y, float z)
{
	SetFacing(GetFacing() + y);
}

float CBipedObject::GetScaling()
{
	return GetSizeScale();
}

void CBipedObject::Reset()
{
	SetSizeScale(1.0f);
	SetSpeedScale(1.0f);
}

void CBipedObject::SetSizeScale(float fScale)
{
	if (m_fSizeScale != fScale)
	{
		m_fSizeScale = fScale;
		CAnimInstanceBase* pAI = GetAnimInstance();
		if (pAI && m_pMultiAnimationEntity)
		{
			pAI->SetSizeScale(fScale);
			SetGeometryDirty(true);
		}
		else
		{
			// just a default radius for debugging purposes only
			SetRadius(0.2f);
		}
	}
}

float CBipedObject::GetPhysicsRadius()
{
	return m_fPhysicsRadius;
}
void CBipedObject::SetPhysicsRadius(float fR)
{
	m_fPhysicsRadius = fR;
}

float CBipedObject::GetPhysicsHeight()
{
	return (m_fPhysicsHeight > 0.f) ? m_fPhysicsHeight : m_fPhysicsRadius * 4;
}

void CBipedObject::SetPhysicsHeight(float fH)
{
	m_fPhysicsHeight = fH;
}

CBaseObject* CBipedObject::GetPhysicsBBObj()
{
	static CTileObject g_objAABB;
	float fLen = GetPhysicsRadius() * 2;
	g_objAABB.SetBoundingBox(fLen, GetPhysicsHeight(), fLen, GetFacing());
	g_objAABB.SetPosition(GetPosition());
	return &g_objAABB;
}

void CBipedObject::SetWalkingState(const char* sName)
{
	if (IsStanding())
	{
		if (sName == NULL)
			PlayAnimation("f"); // default to forward animation
		else
			PlayAnimation(sName);
	}
}


/// If the biped is currently blocked. When in blocking mode.The biped is also in 
/// standing mode. But in rare cases, a biped has not been evaluated by the environment
/// simulator, the biped might be in walking state while in blocking mode. But this is
/// only for a very short moment between calls of environment simulation.
bool CBipedObject::IsBipedBlocked()
{
	BipedWayPoint waypoint;
	return (GetWayPoint(&waypoint) && (waypoint.nType == BipedWayPoint::BLOCKED));
}

/// TODO: may a movie track should be used
void CBipedObject::PlayDeathDecayAnim()
{
	PlayAnimation("Death");
	ForceStop();
}

void CBipedObject::SetStandingState()
{
	CBipedStateManager* pCharState = GetBipedStateManager();
	if (pCharState)
		GetBipedStateManager()->AddAction(CBipedStateManager::S_STANDING);
}

void CBipedObject::UpdateSpeed(float fNewSpeed)
{
	if (fNewSpeed < 0)
		fNewSpeed = -fNewSpeed;

	if (fNewSpeed == 0.f && m_fSpeed != 0.f)
	{
		m_fSpeed = fNewSpeed;
		ForceStop();
	}
	else if (fNewSpeed != 0.f && m_fSpeed == 0.f)
	{
		m_fSpeed = fNewSpeed;
		ForceMove();
	}
	else
		m_fSpeed = fNewSpeed;
}
void CBipedObject::UseSpeedFromAnimation(int nIndex)
{
	// TODO:
}
void CBipedObject::UseSpeedFromAnimation(const char* sName)
{
	if (m_pAI)
	{
		float fNewSpeed = m_fSpeed;
		m_pAI->GetSpeedOf(sName, &fNewSpeed);
		UpdateSpeed(fNewSpeed);
	}
}

void CBipedObject::PlayAnimation(DWORD nIndex, bool bUpdateSpeed, bool bAppend)
{
	if (m_pAI)
	{
		float fNewSpeed = m_fSpeed;
		m_pAI->LoadAnimation(nIndex, &fNewSpeed, bAppend);

		if (bUpdateSpeed)
			UpdateSpeed(fNewSpeed);
	}
}
void CBipedObject::PlayAnimation(const char* sName, bool bUpdateSpeed, bool bAppend)
{
	if (m_pAI)
	{
		float fNewSpeed = m_fSpeed;
		m_pAI->LoadAnimation(sName, &fNewSpeed, bAppend);

		if (bUpdateSpeed)
			UpdateSpeed(fNewSpeed);
	}
}
int CBipedObject::GetCurrentAnimation()
{
	if (m_pAI)
	{
		return m_pAI->GetCurrentAnimation();
	}
	return 0;
}


void ParaEngine::CBipedObject::SetAnimation(int nAnimID)
{
	PlayAnimation(nAnimID, false, false);
}

void ParaEngine::CBipedObject::SetUpperAnimation(int nAnimID)
{
	if (m_pAI)
	{
		m_pAI->SetUpperAnimation(nAnimID);
	}
}

int ParaEngine::CBipedObject::GetAnimation()
{
	return GetCurrentAnimation();
}

int ParaEngine::CBipedObject::GetUpperAnimation()
{
	if (m_pAI)
	{
		return m_pAI->GetUpperAnimation();
	}
	else
	{
		return -1;
	}
}


bool ParaEngine::CBipedObject::HasAnimation(int nAnimID)
{
	if (m_pAI)
	{
		return m_pAI->HasAnimation(nAnimID);
	}
	return false;
}


bool ParaEngine::CBipedObject::HasAlphaBlendedObjects()
{
	return GetOpacity() < 1.f || (m_pAI && m_pAI->HasAlphaBlendedObjects());
}

int ParaEngine::CBipedObject::GetNumReplaceableTextures()
{
	return 0;
}

TextureEntity* ParaEngine::CBipedObject::GetDefaultReplaceableTexture(int ReplaceableTextureID)
{
	return 0;
}

TextureEntity* ParaEngine::CBipedObject::GetReplaceableTexture(int ReplaceableTextureID)
{
	CharModelInstance* pChar = GetCharModelInstance();
	if (pChar)
	{
		return pChar->GetReplaceableTexture(ReplaceableTextureID);
	}
	return NULL;
}

bool ParaEngine::CBipedObject::SetReplaceableTexture(int ReplaceableTextureID, TextureEntity* pTextureEntity)
{
	CharModelInstance* pChar = GetCharModelInstance();
	if (pChar)
	{
		return pChar->SetReplaceableTexture(ReplaceableTextureID, pTextureEntity);
	}
	return true;
}

void ParaEngine::CBipedObject::SetNormal(const Vector3 & pNorm)
{
	m_vNorm = pNorm;
	m_vNormTarget = m_vNorm;
}

ParaEngine::Vector3 ParaEngine::CBipedObject::GetNormal()
{
	return m_vNorm;
}


DWORD ParaEngine::CBipedObject::GetPhysicsGroupMask()
{
	return m_dwPhysicsGroupMask;
}

float ParaEngine::CBipedObject::GetObjectToCameraDistance()
{
	return m_fObjectToCameraDistance;
}

void ParaEngine::CBipedObject::SetObjectToCameraDistance(float val)
{
	m_fObjectToCameraDistance = val;
}

void ParaEngine::CBipedObject::SetPhysicsShape(const char* shape)
{
	m_sPhysicsShape = shape;
}

const char* ParaEngine::CBipedObject::GetPhysicsShape()
{
	return m_sPhysicsShape.c_str();
}

void ParaEngine::CBipedObject::SetPhysicsProperty(const char* property)
{
	if (!m_dynamicPhysicsActor) {
		OUTPUT_LOG("warning: SetPhysicsProperty when actor does not exist.\n");
		return;
	}
		
	CGlobals::GetPhysicsWorld()->SetActorPhysicsProperty(m_dynamicPhysicsActor, property);
}

const char* ParaEngine::CBipedObject::GetPhysicsProperty()
{
	if (m_dynamicPhysicsActor)
	{
		CGlobals::GetPhysicsWorld()->GetActorPhysicsProperty(m_dynamicPhysicsActor);
	}
	return "";
}

void ParaEngine::CBipedObject::EnableDynamicPhysics(bool bEnable)
{
	if (IsPhysicsEnabled()) return ;

	if (bEnable) 
	{
		m_dwPhysicsMethod |= PHYSICS_FORCE_DYNAMIC;
		LoadDynamicPhysics();
	}
	else 
	{
		m_dwPhysicsMethod &= (~PHYSICS_FORCE_DYNAMIC);
		UnloadDynamicPhysics();
	}
}

bool ParaEngine::CBipedObject::IsDynamicPhysicsEnabled()
{
	return (m_dwPhysicsMethod & PHYSICS_FORCE_DYNAMIC) > 0;
}

void ParaEngine::CBipedObject::LoadDynamicPhysics()
{
	if (m_dynamicPhysicsActor == NULL)
	{
		m_dynamicPhysicsActor = CGlobals::GetPhysicsWorld()->CreateDynamicMesh(this);
	}
}

void ParaEngine::CBipedObject::UnloadDynamicPhysics()
{
	if (m_dynamicPhysicsActor != NULL)
	{
		CGlobals::GetPhysicsWorld()->ReleaseActor(m_dynamicPhysicsActor);
		m_dynamicPhysicsActor = NULL;
	}
}

void ParaEngine::CBipedObject::ApplyCentralImpulse(const Vector3& impulse)
{
	if (m_dynamicPhysicsActor != NULL)
	{
		m_dynamicPhysicsActor->ApplyCentralImpulse(PARAVECTOR3(impulse.x, impulse.y, impulse.z));
	}
}


bool ParaEngine::CBipedObject::CanHasPhysics()
{
	return IsPhysicsEnabled();
}

void ParaEngine::CBipedObject::LoadPhysics()
{
	if (m_dwPhysicsMethod > 0 && IsPhysicsEnabled() && (GetStaticActorCount() == 0) && GetParaXEntity() && GetParaXEntity()->IsLoaded())
	{
		CParaXModel* ppMesh = GetParaXEntity()->GetModel();
		if (ppMesh == 0 || ppMesh->GetHeader().maxExtent.x <= 0.f)
		{
			EnablePhysics(false); // disable physics forever, if failed loading physics data
			return;
		}

		// get world transform matrix
		Matrix4 mxWorld;
		GetWorldTransform(mxWorld);
		auto pAI = GetParaXAnimInstance();
		if (pAI)
		{
			pAI->UpdateWorldTransform(CGlobals::GetSceneState(), mxWorld, mxWorld);
		}

		IParaPhysicsActor* pActor = CGlobals::GetPhysicsWorld()->CreateStaticMesh(GetParaXEntity(), mxWorld, GetPhysicsGroup(), &m_staticActors, this);
		if (m_staticActors.empty())
		{
			// disable physics forever, if no physics actors are loaded. 
			EnablePhysics(false);
		}
	}
}

void ParaEngine::CBipedObject::UnloadPhysics()
{
	int nSize = (int)m_staticActors.size();
	if (nSize > 0)
	{
		for (int i = 0; i < nSize; ++i)
		{
			CGlobals::GetPhysicsWorld()->ReleaseActor(m_staticActors[i]);
		}
		m_staticActors.clear();
	}
}

void ParaEngine::CBipedObject::SetPhysicsGroup(int nGroup)
{
	PE_ASSERT(0 <= nGroup && nGroup < 32);
	if (m_nPhysicsGroup != nGroup)
	{
		m_nPhysicsGroup = nGroup;
		UnloadPhysics();
	}
}

int ParaEngine::CBipedObject::GetPhysicsGroup()
{
	return m_nPhysicsGroup;
}

void ParaEngine::CBipedObject::EnablePhysics(bool bEnable)
{
	if (IsDynamicPhysicsEnabled()) return;
	
	if (!bEnable){
		UnloadPhysics();
		m_dwPhysicsMethod |= PHYSICS_FORCE_NO_PHYSICS;
	}
	else
	{
		m_dwPhysicsMethod &= (~PHYSICS_FORCE_NO_PHYSICS);
		if ((m_dwPhysicsMethod&PHYSICS_ALWAYS_LOAD) > 0)
			LoadPhysics();
	}
}

bool ParaEngine::CBipedObject::IsPhysicsEnabled()
{
	return !((m_dwPhysicsMethod & PHYSICS_FORCE_NO_PHYSICS)>0);
}

int ParaEngine::CBipedObject::GetStaticActorCount()
{
	return (int)m_staticActors.size();
}

ParaEngine::ParaXEntity* ParaEngine::CBipedObject::GetParaXEntity()
{
	auto pAsset = GetPrimaryAsset();
	if (pAsset && pAsset->GetType() == AssetEntity::parax)
	{
		return (ParaXEntity*)pAsset;
	}
	return NULL;
}

void ParaEngine::CBipedObject::SetPhysicsGroupMask(DWORD dwValue)
{
	m_dwPhysicsGroupMask = dwValue;
}

void ParaEngine::CBipedObject::SetLastSpeed(float fSpeed)
{
	m_fLastSpeed = fSpeed;
}

float ParaEngine::CBipedObject::GetLastSpeed()
{
	return m_fLastSpeed;
}

void ParaEngine::CBipedObject::SetAccelerationDist(float fAccelerationDist /*= 0.f*/)
{
	m_fAccelerationDist = fAccelerationDist;
	if (fAccelerationDist == 0.f)
	{
		m_fLastSpeed = 0.f;
	}
}

float ParaEngine::CBipedObject::GetAccelerationDist()
{
	return m_fAccelerationDist;
}

void ParaEngine::CBipedObject::SetAutoWalkupBlock(bool bAutoWalkup)
{
	m_bAutoWalkupBlock = bAutoWalkup;
}

bool ParaEngine::CBipedObject::IsAutoWalkupBlock()
{
	return m_bAutoWalkupBlock;
}

void ParaEngine::CBipedObject::SetCanFly(bool canFly)
{
	m_canFly = canFly;
	if (!m_canFly)
	{
		m_isFlying = false;
		m_fSpeedVertical = -GetAbsoluteSpeed();
	}
}

bool ParaEngine::CBipedObject::GetIsFlying()
{
	return m_isFlying;
}

bool ParaEngine::CBipedObject::GetCanFly()
{
	return m_canFly;
}

void ParaEngine::CBipedObject::SetIgnoreSlopeCollision(bool ignoreSlope)
{
	m_ignoreSlopeCollision = ignoreSlope;
}

bool ParaEngine::CBipedObject::GetIgnoreSlopeCollision()
{
	return m_ignoreSlopeCollision;
}

void ParaEngine::CBipedObject::SetGravity(float gravity)
{
	m_gravity = gravity;
}

float ParaEngine::CBipedObject::GetGravity()
{
	return m_gravity;
}

void ParaEngine::CBipedObject::SetFlyingDirection(const Vector3 *dir)
{
	m_flyingDir = *dir;
}

void ParaEngine::CBipedObject::SetBootHeight(float fBootHeight)
{
	if (m_fBootHeight != fBootHeight)
	{
		m_fBootHeight = fBootHeight;
		SetGeometryDirty(true);
	}
}

float ParaEngine::CBipedObject::GetBootHeight()
{
	return m_fBootHeight;
}

bool ParaEngine::CBipedObject::IsControlledExternally() const
{
	return m_bIsControlledExternally;
}

void ParaEngine::CBipedObject::SetIsControlledExternally(bool val)
{
	m_bIsControlledExternally = val;
}

bool ParaEngine::CBipedObject::GetAlwaysFlying() const
{
	return m_isAlwaysFlying;
}

void ParaEngine::CBipedObject::SetAlwaysFlying(bool val)
{
	m_isAlwaysFlying = val;
}

void ParaEngine::CBipedObject::SetBlendingFactor(float fBlendingFactor)
{
	CAnimInstanceBase* pAI = GetAnimInstance();
	if (pAI)
	{
		pAI->SetBlendingFactor(fBlendingFactor);
	}
}

bool ParaEngine::CBipedObject::HasAnimId(int nAnimID)
{
	CAnimInstanceBase* pAI = GetAnimInstance();
	if (pAI)
	{
		return pAI->HasAnimId(nAnimID);
	}
	return false;
}

bool ParaEngine::CBipedObject::GetFlyUsingCameraDir() const
{
	return m_isFlyUsingCameraDir;
}

void ParaEngine::CBipedObject::SetFlyUsingCameraDir(bool val)
{
	m_isFlyUsingCameraDir = val;
}

void ParaEngine::CBipedObject::SetOpacity(float fOpacity)
{
	if (fOpacity < 1.f)
	{
		m_bCanAnimOpacity = false;
	}
	CTileObject::SetOpacity(fOpacity);
}

bool ParaEngine::CBipedObject::CanAnimOpacity() const
{
	return m_bCanAnimOpacity;
}

IAttributeFields* ParaEngine::CBipedObject::GetChildAttributeObject(const char * sName)
{
	return CBaseObject::GetChildAttributeObject(sName);
}

IAttributeFields* ParaEngine::CBipedObject::GetChildAttributeObject(int nRowIndex, int nColumnIndex /*= 0*/)
{
	if (nColumnIndex == 0)
		return CBaseObject::GetChildAttributeObject(nRowIndex, nColumnIndex);
	else if (nColumnIndex == 1)
	{
		// exposing primary asset and animation instance
		if (nRowIndex == 0)
			return GetPrimaryAsset();
		else if (nRowIndex == 1)
			return GetAnimInstance();
	}
	else if (nColumnIndex == 2)
	{
		return GetReplaceableTexture(nRowIndex);
	}
	return NULL;
}

int ParaEngine::CBipedObject::GetChildAttributeObjectCount(int nColumnIndex /*= 0*/)
{
	if (nColumnIndex == 0)
		return CBaseObject::GetChildAttributeObjectCount(nColumnIndex);
	else if (nColumnIndex == 1)
		// exposing primary asset and animation instance
		return 2;
	else if (nColumnIndex == 2)
		return GetNumReplaceableTextures();
	else
		return 0;
}

int ParaEngine::CBipedObject::GetChildAttributeColumnCount()
{
	return 3;
}

int ParaEngine::CBipedObject::ProcessObjectEvent(const ObjectEvent& event)
{
	// Process High level Event
	const string& sEvent = event.GetEventString();
	// whether we will delete this event 
	bool bCanDelete = true;
	int nPos = 0;
	HLEToken curToken;
	if (GetHLEToken(sEvent.c_str(), &curToken, &nPos) && curToken.myType == HLEToken::str)
	{
		DWORD curTokenCmd = curToken.GetCmd();
		if (curTokenCmd == HLEToken("mont").GetCmd())
		{
			// syntax: "mount objectname slot_id" 
			// desc: mount on another character 
			if (GetHLEToken(sEvent.c_str(), &curToken, &nPos))
			{
				std::string sObjName;
				curToken.GetString(sObjName, sEvent.c_str());
				if (!sObjName.empty())
				{
					CBaseObject* pBiped = CGlobals::GetScene()->GetGlobalObject(sObjName);

					if (pBiped && pBiped->IsBiped())
					{
						CBipedObject* pBipedObj = (CBipedObject*)pBiped;
						if (GetHLEToken(sEvent.c_str(), &curToken, &nPos))
						{
							int nMountSlotID = (int)curToken.dValue;

							int nListSize = sizeof(g_MountIDs) / sizeof(int);

							if (nMountSlotID >= 0 && nMountSlotID < nListSize)
							{
								if (pBipedObj->HasAttachmentPoint(g_MountIDs[nMountSlotID]))
								{
									MountOn(pBipedObj, g_MountIDs[nMountSlotID]);

									if (GetHLEToken(sEvent.c_str(), &curToken, &nPos))
									{
										if (curToken.GetType() == HLEToken::str)
										{
											std::string sMountAnimName;
											curToken.GetString(sMountAnimName, sEvent.c_str());

											if (!sMountAnimName.empty())
											{
												CBoneAnimProvider* pProvider = CBoneAnimProvider::CreateProvider(-1, sMountAnimName.c_str(), sMountAnimName.c_str(), false);
												if (pProvider)
												{
													SetHeadTurningAngle(0);
													SetHeadUpdownAngle(0.f);
													PlayAnimation(pProvider->GetAnimID());
												}
											}
										}
										else if (curToken.GetType() == HLEToken::value)
										{
											int nAnimID = (int)(curToken.dValue);
											SetHeadTurningAngle(0);
											SetHeadUpdownAngle(0.f);
											PlayAnimation(nAnimID);
										}
									}
								}
								else
								{
									bCanDelete = false;
								}
							}
						}
					}
					else
					{
						// if biped is not found, do not delete it. 
						bCanDelete = false;
					}
				}
			}
		}
		else if (curTokenCmd == HLEToken("umnt").GetCmd())
		{
			MountOn(NULL);
		}
		else if (curTokenCmd == HLEToken("walk").GetCmd())
		{
			double x, y;
			if (GetHLEToken(sEvent.c_str(), &curToken, &nPos)){
				x = curToken.dValue;
				if (GetHLEToken(sEvent.c_str(), &curToken, &nPos))
				{
					y = curToken.dValue;
					DVector3 v(x, 0, y);
					WalkTo(v);
				}
			}
		}
		else if (curTokenCmd == HLEToken("stop").GetCmd())
		{
			ForceStop();
		}
		else if (curTokenCmd == HLEToken("anim").GetCmd())
		{
			if (GetHLEToken(sEvent.c_str(), &curToken, &nPos)) {
				if (curToken.myType == HLEToken::str){
					string sValue;
					curToken.GetString(sValue, sEvent.c_str());
					PlayAnimation(sValue.c_str());
				}
				else{
					PlayAnimation((int)curToken.dValue);
				}
			}
		}
		else if (curTokenCmd == HLEToken("colr").GetCmd())
		{
			// set model color
			float value[3];
			for (int i = 0; i < 3; i++)
			{
				if (GetHLEToken(sEvent.c_str(), &curToken, &nPos) && curToken.myType == HLEToken::value)
				{
					value[i] = (float)curToken.dValue;
				}
			}
			CAnimInstanceBase* pAI = GetAnimInstance();
			if (pAI)
			{
				pAI->SetModelColor(LinearColor(value[0], value[1], value[2], 1.0f));
			}
		}
		else if (curTokenCmd == HLEToken("spds").GetCmd())
		{
			// set speed scale. default is 1.0
			float value;
			if (GetHLEToken(sEvent.c_str(), &curToken, &nPos) && curToken.myType == HLEToken::value)
			{
				value = (float)curToken.dValue;
				SetSpeedScale(value);
			}
		}
	}
	return 0;
}

void CBipedObject::WalkTo(const DVector3& v, bool bIgnoreHeight)
{
	if (!m_vPosTarget.empty())
		m_vPosTarget.clear();

	if (bIgnoreHeight)
		m_vPosTarget.push_front(BipedWayPoint(DVector3(v.x, m_vPos.y, v.z)));
	else
		m_vPosTarget.push_front(BipedWayPoint(v));
}

void CBipedObject::ForceMove()
{
	if (m_vPosTarget.size() == 1)
	{
		if (!m_vPosTarget.front().IsMoving())
			m_vPosTarget.front().SetPointType(BipedWayPoint::COMMAND_MOVING);
	}
	else
	{
		m_vPosTarget.clear();
		m_vPosTarget.push_front(BipedWayPoint(BipedWayPoint::COMMAND_MOVING));
	}
}

void CBipedObject::ForceStop()
{
	if (!m_vPosTarget.empty() && m_vPosTarget.back().GetPointType() != BipedWayPoint::COMMAND_MOUNT)
	{
		m_vPosTarget.clear();
	}

	m_fSpeed = 0.0f;
}

void CBipedObject::FallDown()
{
	// if biped is in the air, this will ensure that it does not have a zero vertical speed.
	// this is because zero vertical speed stands for standing on the ground.
	if (m_fSpeedVertical == 0.f)
		m_fSpeedVertical = FLT_TOLERANCE;
}

void CBipedObject::JumpUpward(float fVerticalSpeed, bool bJumpFromGround)
{
	if (bJumpFromGround && m_fSpeedVertical != 0.f)
	{
		return;
	}
	if (fVerticalSpeed > 0.f)
	{
		m_fSpeedVertical = fVerticalSpeed;
	}
	else
	{
		m_fSpeedVertical = max(JUMPUP_SPEED, GetAbsoluteSpeed() + 0.001f);
	}
}

float CBipedObject::GetVerticalSpeed()
{
	return m_fSpeedVertical;
}

void CBipedObject::SetVerticalSpeed(float fSpeed)
{
	m_fSpeedVertical = fSpeed;
}
void CBipedObject::ResetBipedPosition(const Vector3 *pV, bool bIgnoreHeight)
{
	m_vPosTarget.clear();
	m_vPos.x = pV->x;
	if (!bIgnoreHeight)
		m_vPos.y = pV->y;
	m_vPos.z = pV->z;
	ForceStop();
}

AssetEntity* ParaEngine::CBipedObject::GetPrimaryAsset()
{
	return m_pMultiAnimationEntity.get();
}

void CBipedObject::ResetBaseModel(AssetEntity* assetCharBaseModel)
{
	if (m_pMultiAnimationEntity != assetCharBaseModel)
	{
		// delete the old animation instance, this allows us to replace custom model with non-custom model, and vice versa. 
		// 2009.9.5: I delete old instance and create a new one. This will lose all CCS info in the old instance. So we will not be able to 
		// change from female to male model, etc without losing CCS like before. However, the new code is simpler and less error prone. 
		DeleteAnimInstance();
		m_pMultiAnimationEntity = assetCharBaseModel;

		// try create a new one
		CharModelInstance* pChar = GetCharModelInstance();

		if (m_pMultiAnimationEntity && (m_pMultiAnimationEntity->GetType() == AssetEntity::parax))
		{
			SetPrimaryTechniqueHandle(((ParaXEntity*)m_pMultiAnimationEntity.get())->GetPrimaryTechniqueHandle());
		}

		if (!SetParamsFromAsset())
		{
			// in case the asset is not available, we shall set a default scale to the biped. 
			// SetSizeScale(1.f);
		}
	}
}

void ParaEngine::CBipedObject::SetAssetFileName(const std::string& sFilename)
{
	ParaXEntity* pNewModel = CGlobals::GetAssetManager()->LoadParaX("", sFilename);
	if (pNewModel && m_pMultiAnimationEntity != pNewModel)
	{
		if (!m_pMultiAnimationEntity || (m_pMultiAnimationEntity->GetType() != AssetEntity::parax))
			ResetBaseModel(pNewModel);
		else
		{
			m_pMultiAnimationEntity = pNewModel;
			SetPrimaryTechniqueHandle(pNewModel->GetPrimaryTechniqueHandle());
			SetParamsFromAsset();

			// just reset asset file, without deleting old animation instance, 
			// all animation instance info like bone parameters, skin, etc are preserved. 
			CParaXAnimInstance* pInstance = GetParaXAnimInstance();
			if (pInstance)
			{
				pInstance->ResetBaseModel(pNewModel);
			}
		}
	}
}

// obsoleted function:
void CBipedObject::LoadStoredModel(int nModelSetID)
{
	CharModelInstance* pChar = GetCharModelInstance();
	if (pChar)
	{
		// pChar->loadSet(nModelSetID);
	}
}

void ParaEngine::CBipedObject::GetOBB(CShapeOBB* obb)
{
	obb->mCenter = GetObjectCenter();
	obb->mExtents = m_aabb.GetExtents();
	obb->mRot = Matrix3::IDENTITY;
}

IGameObject* CBipedObject::QueryIGameObject()
{
	return (IGameObject*)this;
}

int CBipedObject::On_EnterSentientArea()
{
	// TODO: 

	// call base interface, to activate script handler. 
	return IGameObject::On_EnterSentientArea();
}

int CBipedObject::On_LeaveSentientArea()
{
	// TODO: 

	// call base interface, to activate script handler. 
	return IGameObject::On_LeaveSentientArea();
}

int CBipedObject::On_Click(DWORD nMouseKey, DWORD dwParam1, DWORD dwParam2)
{
	// TODO: 

	// call base interface, to activate script handler. 
	return IGameObject::On_Click(nMouseKey, dwParam1, dwParam2);
}

int CBipedObject::On_Event(DWORD nEventType, DWORD dwParam1, DWORD dwParam2)
{
	// TODO: 

	// call base interface, to activate script handler. 
	return IGameObject::On_Event(nEventType, dwParam1, dwParam2);
}

int CBipedObject::On_Perception()
{
	// TODO: 

	// call base interface, to activate script handler. 
	return IGameObject::On_Perception();
}

int CBipedObject::On_FrameMove()
{
	// TODO: 

	// call base interface, to activate script handler. 
	return IGameObject::On_FrameMove();
}

int CBipedObject::On_Net_Send(DWORD dwNetType, DWORD dwParam1, DWORD dwParam2)
{
	// TODO: 

	// call base interface, to activate script handler. 
	return IGameObject::On_Net_Send(dwNetType, dwParam1, dwParam2);
}

int CBipedObject::On_Net_Receive(DWORD dwNetType, DWORD dwParam1, DWORD dwParam2)
{
	// TODO: 

	// call base interface, to activate script handler. 
	return IGameObject::On_Net_Receive(dwNetType, dwParam1, dwParam2);
}

bool CBipedObject::DumpBVHAnimations()
{
#ifdef USE_DIRECTX_RENDERER
	CharModelInstance* pChar = GetCharModelInstance();
	if (pChar != 0)
	{
		ParaXEntity* pEntity = pChar->GetBaseModel();
		if (pEntity != 0 && pEntity->GetModel() != 0)
		{
			string sFileName = pEntity->GetKey();
			OUTPUT_LOG("\r\n processing %s for BVH exportations\r\n", sFileName.c_str());

			string sExt = CParaFile::GetFileExtension(sFileName);
			sFileName = sFileName.substr(0, sFileName.size() - sExt.size() - 1);
			sFileName += ".bvh";

			// testing code: save current player's model to BVH file
			CBVHSerializer serializerBVH;
			if (serializerBVH.InitHierachy(*(pEntity->GetModel())))
			{
				return serializerBVH.SaveBVH(sFileName, "");
			}
		}
	}
#endif
	return false;
}

int CBipedObject::GetAnimFrame()
{
	CAnimInstanceBase * pAI = GetAnimInstance();
	if (pAI)
	{
		return pAI->GetAnimFrame();
	}
	return 0;
}

void CBipedObject::SetAnimFrame(int nFrame)
{
	CAnimInstanceBase * pAI = GetAnimInstance();
	if (pAI)
	{
		return pAI->SetAnimFrame(nFrame);
	}
}


void ParaEngine::CBipedObject::EnableAnim(bool bAnimated)
{
	CAnimInstanceBase * pAI = GetAnimInstance();
	if (pAI)
		return pAI->EnableAnimation(bAnimated);
}

bool ParaEngine::CBipedObject::IsAnimEnabled()
{
	CAnimInstanceBase * pAI = GetAnimInstance();
	if (pAI)
		return pAI->IsAnimationEnabled();
	return true;
}


int ParaEngine::CBipedObject::GetTime()
{
	CAnimInstanceBase * pAI = GetAnimInstance();
	return (pAI) ? pAI->GetTime() : 0;
}

void ParaEngine::CBipedObject::SetTime(int nTime)
{
	CAnimInstanceBase * pAI = GetAnimInstance();
	if(pAI)
		pAI->SetTime(nTime);
}

bool CBipedObject::MountOn(CBaseObject* pTarget, int nMountID)
{
	CBipedStateManager* pCharState = GetBipedStateManager();
	if (!pCharState)
		return false;

	CBipedStateManager* pTargetState = NULL;

	if (pTarget == 0)
	{
		// unmount the biped.  
		pCharState->SetMounted(false);
		RemoveWayPoint();
		DeleteAllRefs();
		return true;
	}
	else
	{
		pTargetState = pTarget->GetBipedStateManager();

		if (pTarget->IsBiped())
		{
			// if target is also mounted, unmount it first to prevent recursive calls. 
			if (pTargetState && pTargetState->IsMounted())
			{
				((CBipedObject*)pTarget)->MountOn(NULL);
			}
		}
	}

	BipedWayPoint ptnew(BipedWayPoint::COMMAND_MOUNT);

	if (pTarget)
	{
		if (nMountID == -1)
		{
			// find attachment point (nMountID)
			int nListSize = sizeof(g_MountIDs) / sizeof(int);
			Vector3 vPos2;
			DVector3 vPos = GetPosition();
			float fMinDistSq = FLOAT_POS_INFINITY;

			// check to see if it has an attachment ID 0, which is the default mount position.
			for (int i = 0; i < nListSize && pTarget->GetAttachmentPosition(vPos2, g_MountIDs[i]); ++i)
			{
				// here we compare distance between model origin and the attachment position. multiple attachment points on the same object are supported
				float fDistSq = (float)((vPos2 - vPos).squaredLength());
				if (fMinDistSq > fDistSq)
				{
					fMinDistSq = fDistSq;
					nMountID = g_MountIDs[i];
				}
			}

			// if no mount point id found on the target, use the default mount id 0. This is true for async loaded objects.
			if (nMountID < 0)
			{
				nMountID = 0;
				//return false;
			}
		}

		// if(pTargetState==0 || !pTargetState->IsMounted())
		{
			// add mount.
			ptnew.vPos = Vector3(0, 0, 0);
			ptnew.fFacing = 0;
			ptnew.m_nReserved0 = nMountID;
			RemoveWayPoint();
			AddWayPoint(ptnew);
			// remove previous mount target. is this the best way to do it?
			DeleteAllRefsByTag(0);
			AddReference(pTarget, 0);

			// Set mounted state.
			pCharState->SetMounted(true);
		}
	}
	return true;
}

int CBipedObject::InstallFields(CAttributeClass* pClass, bool bOverride)
{
	IGameObject::InstallFields(pClass, bOverride);


	pClass->AddField("Physics Radius", FieldType_Float, (void*)SetPhysicsRadius_s, (void*)GetPhysicsRadius_s, NULL, "", bOverride);
	pClass->AddField("PhysicsHeight", FieldType_Float, (void*)SetPhysicsHeight_s, (void*)GetPhysicsHeight_s, NULL, "", bOverride);

	pClass->AddField("Size Scale", FieldType_Float, (void*)SetSizeScale_s, (void*)GetSizeScale_s, NULL, "", bOverride);
	pClass->AddField("Density", FieldType_Float, (void*)SetDensity_s, (void*)GetDensity_s, NULL, "", bOverride);
	pClass->AddField("Gravity", FieldType_Float, (void*)SetGravity_s, (void*)GetGravity_s, NULL, "", bOverride);
	pClass->AddField("Speed Scale", FieldType_Float, (void*)SetSpeedScale_s, (void*)GetSpeedScale_s, NULL, "", bOverride);

	pClass->AddField("Animation ID", FieldType_Int, (void*)PlayAnimation_s, (void*)GetCurrentAnimation_s, NULL, "current animation ID", bOverride);

	pClass->AddField("IgnoreSlopeCollision", FieldType_Bool, (void*)SetIgnoreSlopeCollision_s, (void*)GetIgnoreSlopeCollision_s, NULL, "", bOverride);

	//#ifdef _DEBUG
	pClass->AddField("Dump BVH anims", FieldType_void, (void*)DumpBVHAnimations_s, (void*)DumpBVHAnimations_s, NULL, "dump all animations to BVH files", bOverride);
	//#endif
	pClass->AddField("HeadTurningAngle", FieldType_Float, (void*)SetHeadTurningAngle_s, (void*)GetHeadTurningAngle_s, NULL, "", bOverride);
	pClass->AddField("HeadUpdownAngle", FieldType_Float, (void*)SetHeadUpdownAngle_s, (void*)GetHeadUpdownAngle_s, NULL, "", bOverride);
	pClass->AddField("BootHeight", FieldType_Float, (void*)SetBootHeight_s, (void*)GetBootHeight_s, NULL, "", bOverride);

	pClass->AddField("MovementStyle", FieldType_Int, (void*)SetMovementStyle_s, (void*)GetMovementStyle_s, NULL, "", bOverride);
	pClass->AddField("WalkSpeed", FieldType_Float, NULL, (void*)GetMaxSpeed_s, NULL, "", bOverride);
	pClass->AddField("CurrentSpeed", FieldType_Float, (void*)SetSpeed_s, (void*)GetSpeed_s, NULL, "", bOverride);
	pClass->AddField("VerticalSpeed", FieldType_Float, (void*)SetVerticalSpeed_s, (void*)GetVerticalSpeed_s, NULL, "", bOverride);

	pClass->AddField("LastSpeed", FieldType_Float, (void*)SetLastSpeed_s, (void*)GetLastSpeed_s, NULL, "", bOverride);
	pClass->AddField("AccelerationDist", FieldType_Float, (void*)SetAccelerationDist_s, (void*)GetAccelerationDist_s, NULL, "", bOverride);


	pClass->AddField("PhysicsGroupSensorMask", FieldType_DWORD, (void*)SetPhysicsGroupSensorMask_s, (void*)GetPhysicsGroupSensorMask_s, NULL, "", bOverride);

	pClass->AddField("ForceStop", FieldType_void, (void*)ForceStop_s, NULL, NULL, NULL, bOverride);
	pClass->AddField("ForceMove", FieldType_void, (void*)ForceMove_s, NULL, NULL, NULL, bOverride);

	pClass->AddField("IsAlwaysAboveTerrain", FieldType_Bool, (void*)SetIsAlwaysAboveTerrain_s, (void*)GetIsAlwaysAboveTerrain_s, NULL, "", bOverride);
	pClass->AddField("IsAnimPaused", FieldType_Bool, (void*)SetIsAnimPaused_s, (void*)GetIsAnimPaused_s, NULL, "", bOverride);

	pClass->AddField("GetLastWayPointType", FieldType_Int, NULL, (void*)GetLastWayPointType_s, NULL, "", bOverride);
	pClass->AddField("GetLastWayPointPos", FieldType_Vector3, NULL, (void*)GetLastWayPointPos_s, NULL, "", bOverride);

	pClass->AddField("CanFly", FieldType_Bool, (void*)SetCanFly_s, (void*)GetCanFly_s, NULL, "", bOverride);
	pClass->AddField("AlwaysFlying", FieldType_Bool, (void*)SetAlwaysFlying_s, (void*)GetAlwaysFlying_s, NULL, "", bOverride);
	pClass->AddField("FlyUsingCameraDir", FieldType_Bool, (void*)SetFlyUsingCameraDir_s, (void*)GetFlyUsingCameraDir_s, NULL, "", bOverride);
	pClass->AddField("IsFlying", FieldType_Bool, NULL, (void*)GetIsFlying_s, NULL, "", bOverride);
	pClass->AddField("AutoWalkupBlock", FieldType_Bool, (void*)SetAutoWalkupBlock_s, (void*)IsAutoWalkupBlock_s, NULL, "", bOverride);
	pClass->AddField("IsControlledExternally", FieldType_Bool, (void*)SetIsControlledExternally_s, (void*)IsControlledExternally_s, NULL, "", bOverride);
	pClass->AddField("BlendingFactor", FieldType_Float, (void*)SetBlendingFactor_s, NULL, NULL, "", bOverride);
	return S_OK;
}

void CBipedObject::EnableAutoAnimation(bool enable)
{
	m_bAutoAnimation = enable;
}

//-----------------------------------------------------------------------------
// Class:	CBipedStateManager
// Authors:	Li, Xizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2005.10.8
// Revised: 2005.10.8
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "CustomCharCommon.h"
#include "ParaXAnimInstance.h"
#include "MovieCtrlers.h"
#include "KeyFrame.h"
#include "BipedObject.h"
#include "BipedStateManager.h"
#include "SceneObject.h"
#include "ShapeSphere.h"
#include "ParaXModel/AnimTable.h"
#include "ParaScriptingCharacter.h"

/** speed for upper body angle turning speed(in rads), when the biped is moving */
#define UPPERBODY_ANGLE_SPEED_MOVING	 4.0f
/** speed for upper body angle turning speed(in rads), when the biped is standing */
#define UPPERBODY_ANGLE_SPEED_STANDING	 3.0f

/** @def this is the maximum number of states that the state manager could store*/
#define MAX_STATE_MEMORY_SIZE	5

/** @def normally this will be greater than 8. see GetStateAnimName() */
#define MAX_ANIM_NAME_LENGTH	10

/** @def the minimum mount distance square. If a target(horse) is within this distance, it will be automatically mounted on */
#define MIN_MOUNT_DISTANCE_SQ 9.0f
using namespace ParaEngine;

namespace ParaEngine
{
	/** any object in the scene except. Usually for selection during scene editing.*/
	extern OBJECT_FILTER_CALLBACK g_fncPickingAll;
}

CBipedStateManager::CBipedStateManager(void)
	:m_pBiped(NULL), m_fTimer(0), m_bWalkOrRun(false),m_bRecording(false),m_bIsMounted(false),m_fAngleDelta(0), m_vPos(0,0,0), m_fJumpupSpeed(0.f)
{
	
}
CBipedStateManager::CBipedStateManager(CBipedObject * pBiped)
	:m_pBiped(pBiped), m_fTimer(0), m_bWalkOrRun(false),m_bRecording(false),m_bIsMounted(false),m_fAngleDelta(0), m_vPos(0,0,0), m_fJumpupSpeed(0.f)
{
}

CBipedStateManager::~CBipedStateManager(void)
{
}
bool CBipedStateManager::IsMounted()
{
	return m_bIsMounted;
}
void CBipedStateManager::SetMounted(bool bIsMounted)
{
	m_bIsMounted = bIsMounted;

	if (m_bIsMounted)
	{
		// this fix a bug when mounting while the character is turning.
		// remove all moving states from the memory
		for(list<BipedState>::iterator itCurCP = m_memory.begin(); itCurCP != m_memory.end(); )
		{
			if(IsMovingState(*itCurCP))
				itCurCP = m_memory.erase(itCurCP);
			else
				++itCurCP;
		}
	}
}

bool CBipedStateManager::IsRecording()
{
	return m_bRecording;
}
void  CBipedStateManager::SetRecording(bool bIsRecording)
{
	m_bRecording = bIsRecording;
}

bool CBipedStateManager::WalkingOrRunning()
{
	return m_bWalkOrRun;
}

void CBipedStateManager::SetWalkOrRun(bool bWalk)
{
	m_bWalkOrRun = bWalk;
}

bool CBipedStateManager::GetStateAnimName(CBipedStateManager::BipedState s, char* sName, int nNameSize)
{
	memset(sName, 0, nNameSize);
	bool bFound = true;
	switch(s) {
	case STATE_STANDING:
	case STATE_IN_WATER:
		bFound = false;
		break;
	case STATE_STAND:
		break;
	case STATE_WALK_FORWORD:
		sName[0] = WalkingOrRunning()? 'F' :'f';
		break;
	case STATE_WALK_BACKWORD:
		sName[0] = WalkingOrRunning()? 'B':'b';
		break;
	case STATE_SWIM:
		sName[0] = 's';
		break;
	case STATE_SWIM_FORWORD:
		sName[0] = 's';
		sName[1] = 'f';
		break;
	case STATE_SWIM_BACKWORD:
		sName[0] = 's';
		sName[1] = 'b';
		break;
	case STATE_SWIM_LEFT:
		sName[0] = 's';
		sName[1] = 'l';
		break;
	case STATE_SWIM_RIGHT:
		sName[0] = 's';
		sName[1] = 'r';
		break;
	case STATE_JUMP_IN_AIR:
		sName[0] = 'j';
		break;
	case STATE_JUMP_START:
		sName[0] = 'j';
		sName[1] = 's';
		break;
	case STATE_JUMP_END:
		sName[0] = 'j';
		sName[1] = 'e';
		break;
	case STATE_MOUNT:
		sName[0] = 'm';
		break;
	case STATE_ATTACK1:
		sName[0] = 'a';
		sName[1] = '1';
		break;
	case STATE_ATTACK2:
		sName[0] = 'a';
		sName[1] = '2';
		break;
	case STATE_DANCE:
		sName[0] = 'e'; // emotion
		sName[1] = '_';
		sName[2] = 'd';
		break;
	default:
		bFound = false;
		break;
	}
	return bFound;
}

CBipedStateManager::BipedState CBipedStateManager::GetLastAnimState()
{
	return m_nLastAnimState;
}

CBipedStateManager::BipedState CBipedStateManager::GetLastState()
{
	if(m_memory.empty())
		return STATE_STANDING;
	else
		return m_memory.back();
}

CBipedStateManager::BipedState CBipedStateManager::GetFirstState()
{
	if(m_memory.empty())
		return STATE_STANDING;
	else
		return m_memory.front();
}

void CBipedStateManager::ReplaceState(BipedState s)
{
	if(m_memory.empty())
		m_memory.push_back(s);
	else
		m_memory.back() = s;
}
void CBipedStateManager::CheckMemory()
{
	while(m_memory.size() > MAX_STATE_MEMORY_SIZE)
		m_memory.pop_back();
}

void CBipedStateManager::RemoveState(BipedState s)
{
	for(list<BipedState>::iterator itCurCP = m_memory.begin(); itCurCP != m_memory.end(); )
	{
		if((*itCurCP) == s)
			itCurCP = m_memory.erase(itCurCP);
		else
			++itCurCP;
	}
}
int CBipedStateManager::FindStateInMemory(BipedState s)
{
	int i=0;
	for(list<BipedState>::iterator itCurCP = m_memory.begin(); itCurCP != m_memory.end(); ++itCurCP,++i)
	{
		if((*itCurCP) == s)
			return i;
	}
	return -1;
}
void CBipedStateManager::SetUniqueState(BipedState s)
{
	int nOccurance=0;
	for(list<BipedState>::iterator itCurCP = m_memory.begin(); itCurCP != m_memory.end();)
	{
		if((*itCurCP) == s)
		{
			nOccurance++;
			if(nOccurance > 1)
				itCurCP = m_memory.erase(itCurCP);
			else
				++itCurCP;
		}
		else
			++itCurCP;
	}
}
void CBipedStateManager::PushState(CBipedStateManager::BipedState s)
{
	CheckMemory();
	m_memory.push_back(s);
}


void CBipedStateManager::PushUniqueState(BipedState s)
{
	// remove similar state
	RemoveState(s);
	PushState(s);
}

void CBipedStateManager::PrependState(BipedState s)
{
	CheckMemory();
	m_memory.push_front(s);
}

void CBipedStateManager::PrependUniqueState(BipedState s)
{
	RemoveState(s);
	PrependState(s);
}

bool CBipedStateManager::IsStandingState(CBipedStateManager::BipedState s)
{
	return (s >= STATE_STANDING);
}

bool CBipedStateManager::IsMovingState(BipedState s)
{
	return (s<STATE_STANDING) && (s>=STATE_MOVING);
}

bool CBipedStateManager::HasMovingState()
{
	list<BipedState>::iterator itEnd = m_memory.end();
	for(list<BipedState>::iterator itCurCP = m_memory.begin(); itCurCP != itEnd; ++itCurCP)
	{
		if(IsMovingState(*itCurCP))
			return true;
	}
	return false;
}

CBipedObject* CBipedStateManager::GetBiped()
{
	return m_pBiped;
}

void CBipedStateManager::SetBiped(CBipedObject* pBiped)
{
	m_pBiped = pBiped;
}

bool CBipedStateManager::IsSwimming()
{
	BipedState s = GetFirstState();
	return (s == STATE_IN_WATER);
}

bool CBipedStateManager::IsFlying()
{
	return FindStateInMemory(CBipedStateManager::STATE_JUMP_IN_AIR)>=0;
}

void CBipedStateManager::SetAngleDelta(float fAngleDelta)
{
	m_fAngleDelta = fAngleDelta;
}

float CBipedStateManager::GetAngleDelta()
{
	return m_fAngleDelta;
}

void CBipedStateManager::SetJumpupSpeed( float fSpeed )
{
	m_fJumpupSpeed = fSpeed;
}

float CBipedStateManager::GetJumpupSpeed()
{
	return m_fJumpupSpeed;
}

void CBipedStateManager::SetPos(const DVector3& v)
{
	m_vPos = v;
}

const DVector3& CBipedStateManager::GetPos()
{
	return m_vPos;
}

CBipedStateManager::BipedState CBipedStateManager::AddAction(ActionSymbols nAct, const void* pData)
{
	/// some looking back states in memory
	CBipedObject* pBiped = GetBiped();
	if(pBiped == NULL)
		return GetLastState();

	if(IsMounted())
	{
		if(nAct == S_JUMP_START || nAct == S_ACTIONKEY || nAct == S_MOUNT)
		{
		}
		else
		{
			return GetLastState();
		}
	}

	if(nAct == S_IN_WATER)
	{
		if(!IsSwimming())
			PrependUniqueState(STATE_IN_WATER);
	}
	else if(nAct == S_ON_FEET)
	{
		if(IsSwimming())
			RemoveState(STATE_IN_WATER);
	}
	else if(nAct == S_ON_WATER_SURFACE)
	{
		if(IsSwimming())
			RemoveState(STATE_IN_WATER);
		RemoveState(STATE_JUMP_START);
		RemoveState(STATE_JUMP_IN_AIR);
	}
	else
	{
		switch(nAct) {
		case S_STANDING:
			{
				// remove all moving states from the memory
				for(list<BipedState>::iterator itCurCP = m_memory.begin(); itCurCP != m_memory.end(); )
				{
					if(IsMovingState(*itCurCP))
						itCurCP = m_memory.erase(itCurCP);
					else
						++itCurCP;
				}
			}
			break;
		case POP_ACTION:
			break;
		case S_WALK_FORWORD:
			/// TODO: one may need to prevent the biped from changing speed direction, when it is in air. 
			/// this can be done by by examining the memory for STATE_JUMP_IN_AIR and STATE_IN_WATER.
			PushUniqueState(STATE_WALK_FORWORD);
			break;
		case S_WALK_BACKWORD:
			PushUniqueState(STATE_WALK_BACKWORD);
			pBiped->RemoveWayPoint();
			break;
		case S_WALK_RIGHT:
			PushUniqueState(STATE_WALK_RIGHT);
			break;
		case S_WALK_LEFT:
			PushUniqueState(STATE_WALK_LEFT);
			break;
		case S_WALK_POINT:
			PushUniqueState(STATE_WALK_POINT);
			break;
		case S_TURNING:
			PushUniqueState(STATE_TURNING);
			break;
		case S_JUMP_START:
			{
				/// TODO: you can prevent jumping multiple times on land by examining the memory for STATE_JUMP_IN_AIR.
				/// currently, it is allowed for debugging purposes only.
				if(IsMounted())
				{
					// TODO: group unmount action.
					SetMounted(false);
					pBiped->RemoveWayPoint();
					pBiped->DeleteAllRefs();
				}

				m_fTimer = 0;
				RemoveState(STATE_JUMP_END);
				RemoveState(STATE_JUMP_IN_AIR);
				float fJumpUpSpeed = 0.f;
				if(pData)
				{
					fJumpUpSpeed = (float)(*((double*)(pData)));
					SetJumpupSpeed(fJumpUpSpeed);
				}
				PushUniqueState(STATE_JUMP_START);
				if(IsRecording())
				{
					// record the state
					ParaScripting::ParaMovieCtrler movieCtler = (ParaScripting::ParaCharacter(pBiped)).GetMovieController();
					movieCtler.RecordNewAction("js"); // jump start
				}
			}
			break;
		case S_FALLDOWN:
			PushUniqueState(STATE_JUMP_IN_AIR);
			break;
		case S_JUMP_END:
			/// TODO: you can prevent jumping multiple times on land by examining the memory for STATE_JUMP_IN_AIR.
			/// currently, it is allowed for debugging purposes only.
			m_fTimer = 0;
			RemoveState(STATE_JUMP_START);
			if(FindStateInMemory(STATE_JUMP_IN_AIR)>=0)
			{
				RemoveState(STATE_JUMP_IN_AIR);
				PushUniqueState(STATE_JUMP_END);
			}
			break;
		case S_ACTIONKEY:
			/// perform the action in the action key. this is usually from a superimposed user action or the movie replaying.
			if(pData)
			{
				const ActionKey* actionKey = (const ActionKey*)pData;
				if(actionKey->IsJump())
					AddAction(CBipedStateManager::S_JUMP_START);
				else if(actionKey->IsToggleToRun())
					SetWalkOrRun(false);
				else if(actionKey->IsToggleToWalk())
					SetWalkOrRun(true);
				else
				{
					// play immediately the animation.
					int nAnimID = actionKey->ToAnimID();
					pBiped->PlayAnimation(nAnimID);
				}
#ifdef USE_DIRECTX_RENDERER
				if(IsRecording())
				{
					// record the state
					ParaScripting::ParaMovieCtrler movieCtler = (ParaScripting::ParaCharacter(pBiped)).GetMovieController();
					movieCtler.RecordNewAction(actionKey->ToString());
				}
#endif
			}
			break;
		case S_MOUNT:
			{
				// Deprecated 2008.6.19. 
				OUTPUT_LOG("warning: S_MOUNT action deprecated, but still used\n");
				BipedWayPoint ptnew(BipedWayPoint::COMMAND_MOUNT);
				ptnew.m_sTarget = (pData)?(const char*)pData:"";
				CBaseObject* pMountTargetObj=NULL;
				int nMountID = 0;
				if(!ptnew.m_sTarget.empty())
				{
					// use target if it is specified
					pMountTargetObj = CGlobals::GetScene()->GetGlobalObject(ptnew.m_sTarget);
					if(pMountTargetObj)
					{
						// check to see if it has an attachment ID 0, which is the default mount position.
						if(!pMountTargetObj->HasAttachmentPoint(0))
							pMountTargetObj = NULL;
					}
				}
				else
				{
					// if there is no target specified, just find the nearest one
					// we will mount on the closet mount point if the target is empty.
					OBJECT_FILTER_CALLBACK pFilterFunc = g_fncPickingAll;
					DVector3 vPos = pBiped->GetPosition();
					float fMinDistSq = FLOAT_POS_INFINITY;

					list<CBaseObject*> output;
					CShapeSphere sphere(vPos, MIN_MOUNT_DISTANCE_SQ);
					int nCount = CGlobals::GetScene()->GetObjectsBySphere(output, sphere, pFilterFunc);
					if(nCount>0)
					{
						static const int MountIDs [] = {0, ATT_ID_MOUNT1,ATT_ID_MOUNT2,ATT_ID_MOUNT3,ATT_ID_MOUNT4,ATT_ID_MOUNT5,ATT_ID_MOUNT6,ATT_ID_MOUNT7,ATT_ID_MOUNT8,ATT_ID_MOUNT9,
							ATT_ID_MOUNT10, ATT_ID_MOUNT11,ATT_ID_MOUNT12,ATT_ID_MOUNT13,ATT_ID_MOUNT14,ATT_ID_MOUNT15,ATT_ID_MOUNT16,ATT_ID_MOUNT17,ATT_ID_MOUNT18,ATT_ID_MOUNT19,ATT_ID_MOUNT20,};
						int nListSize = sizeof(MountIDs)/sizeof(int);
						Vector3 vPos2;

						list<CBaseObject*>::iterator itCur, itEnd = output.end();
						for(itCur = output.begin(); itCur!=itEnd; ++itCur)
						{
							CBaseObject* pObj = (*itCur);

							
							for (int i=0;i<nListSize && pObj->GetAttachmentPosition(vPos2, MountIDs[i]); ++i)
							{
								// here we compare distance between model origin and the attachment position. multiple attachment points on the same object are supported
								float fDistSq = (vPos2-vPos).squaredLength();
								if(fMinDistSq>fDistSq)
								{
									fMinDistSq = fDistSq;
									pMountTargetObj = pObj;
									nMountID = MountIDs[i];
								}
							}
						}
					}
				}
				
				if(pMountTargetObj!=NULL)
				{
					CBipedStateManager* pState = pMountTargetObj->GetBipedStateManager();
					if(pState==0 || !pState->IsMounted())
					{
						// add mount.
						ptnew.vPos = Vector3(0,0,0);
						ptnew.fFacing = 0;
						ptnew.m_nReserved0 = nMountID;
						pBiped->RemoveWayPoint();
						pBiped->AddWayPoint(ptnew);

						// remove previous mount target. is this the best way to do it?
						pBiped->DeleteAllRefsByTag(0);
						pBiped->AddReference(pMountTargetObj, 0);
						// Set mounted state.
						SetMounted(true);

						if(IsRecording())
						{
							// record the state
							ParaScripting::ParaMovieCtrler movieCtler = (ParaScripting::ParaCharacter(pBiped)).GetMovieController();
							movieCtler.RecordNewAction("_mount");
						}
					}
				}
				break;
			}
		case S_FLY_DOWNWARD:
			PushUniqueState(STATE_FLY_DOWNWARD);
			break;
		default:
			break;
		}
	}	
	return GetLastState();
}

void  CBipedStateManager::Update(float fTimeDelta)
{
	const float fHalfPI = MATH_PI*0.499f; // a little smaller than 0.5f to make SmoothMoveAngle1() right for transition from -0.5Pi->+0.5Pi

	/// some looking back states in memory
	CBipedObject* pBiped = GetBiped();
	if(pBiped == NULL)
		return;
	bool bIsSwimming = IsSwimming();
	BipedState stateAnim = bIsSwimming ? STATE_IN_WATER : STATE_STANDING;
	
	CParaXAnimInstance* pAI = pBiped->GetParaXAnimInstance();
	
	bool bIsJumpEnding = false; // whether the biped is ending the jump
	bool bIsJumping=false;  // whether the biped is starting|ending|during a jump
	char sAnimName[MAX_ANIM_NAME_LENGTH];
	bool bUpdateSpeed= true; // whether to update the biped's animation speed, when sAnimName is loaded
	bool bHasTurning = false; // whether the biped is turning 
	bool bUpdateModel = true; // update the model facing, speed direction and head facing
	bool bNewCommandPoint = false; // Has new command point?

	// process any jumping state and turning states
	{
		for(list<BipedState>::iterator itCurCP = m_memory.begin(); itCurCP != m_memory.end(); )
		{
			BipedState s = (*itCurCP);
			bool bErase = false;
			switch(s) {
				case STATE_JUMP_START:
					bIsJumping = true;
					if(bIsSwimming)
					{
						pBiped->JumpUpward(GetJumpupSpeed(),  false);
						(*itCurCP) = STATE_JUMP_IN_AIR; // this may introduce duplicates of state STATE_JUMP_IN_AIR
					}
					else
					{
						if(m_fTimer<0.2f) // the jump start animation will be played at least 0.2 seconds
						{
							if(m_fTimer == 0.f)
								pBiped->JumpUpward(GetJumpupSpeed(),  false);
							m_fTimer+=fTimeDelta;
						}
						else
						{
							m_fTimer = 0.f;
							(*itCurCP) = STATE_JUMP_IN_AIR; // this may introduce duplicates of state STATE_JUMP_IN_AIR
						}
						stateAnim = STATE_JUMP_START;
					}
						
					break;
				case STATE_JUMP_IN_AIR:
					bIsJumping = true;
					if(!bIsSwimming)
						stateAnim = STATE_JUMP_IN_AIR;
					break;
				case STATE_JUMP_END:
					bIsJumping = false;
					bIsJumpEnding = true;
					if(bIsSwimming)
					{ // play swimming
						stateAnim = STATE_SWIM; 
					}
					else
					{
						if(m_fTimer<0.2f) // the jump end animation will be played at least 0.2 seconds
						{
							m_fTimer+=fTimeDelta;
							stateAnim = STATE_JUMP_END;
						}
						else
						{
							m_fTimer = 0.f;
							bErase = true; // erase the jump state
							// play standing animation when the biped has finished with jumping
							stateAnim = STATE_STAND; 
						}
					}
					break;
				case STATE_TURNING:
					bHasTurning = true;
					bErase = true;
					break;
				case STATE_WALK_POINT:
					bNewCommandPoint = true;
					bErase = true;
					break;
				default:
					break;
			}
			if(bErase)
				itCurCP = m_memory.erase(itCurCP);
			else
				++ itCurCP;

		}
		SetUniqueState(STATE_JUMP_IN_AIR); // remove duplicates of state STATE_JUMP_IN_AIR
	}

	/** the new biped speed direction and model direction is mostly set according to the head facing. 
	* the old head facing is maintained the same. */
	float fHeadFacing=0,fSpeedFacing=0, fModelFacing=0;

	bool bWalkToPoint = false;
	if(bNewCommandPoint || pBiped->GetLastWayPoint().GetPointType() == BipedWayPoint::COMMAND_POINT)
		bWalkToPoint = true;

	bool bAccelerating = false;
	if(pBiped->GetAccelerationDist()>0.f && pBiped->GetLastSpeed() != pBiped->GetSpeed())
	{
		bAccelerating = true;
	}

	if( bWalkToPoint || HasMovingState() || bAccelerating)
	{
		BipedState oldStateAnim = stateAnim;
		
		if(bWalkToPoint)
		{
			/** way point*/
			// facing, speed direction and model direction are handled internally by the biped class, and it is always walking forwards
			bUpdateModel = false; 
			if(bNewCommandPoint)
			{
				BipedWayPoint& pt =  pBiped->GetLastWayPoint();
				BipedWayPoint ptnew(GetPos(), GetAngleDelta());
				if(pt.GetPointType() == BipedWayPoint::INVALID_WAYPOINT)
					pBiped->AddWayPoint(ptnew);
				else
				{
					pt.SetPointType(BipedWayPoint::COMMAND_POINT);
					pt.vPos = GetPos();
					pt.fFacing = GetAngleDelta();
					pt.bUseFacing = true;
				}
			}
			if(bIsSwimming)
				stateAnim = STATE_SWIM_FORWORD;
			else
				stateAnim = STATE_WALK_FORWORD;
		}
		else
		{
			/** handling object speed according to moving states, such as running, swimming, etc. */
			bool bDirs[4];

			memset(bDirs, 0, sizeof(bDirs));
			list<BipedState>::const_iterator itCurCP, itEndCP = m_memory.end();
			for( itCurCP = m_memory.begin(); itCurCP != itEndCP; ++ itCurCP)
			{
				BipedState s = (*itCurCP);
				switch(s) {
				case STATE_WALK_FORWORD:
					bDirs[0] = true;
					break;
				case STATE_WALK_BACKWORD:
					bDirs[1] = true;
					break;
				case STATE_WALK_LEFT:
					bDirs[2] = true;
					break;
				case STATE_WALK_RIGHT:
					bDirs[3] = true;
					break;
				default:
					break;
				}
			}

			if(bHasTurning)
				pBiped->SetFacing(Math::ToStandardAngle(pBiped->GetFacing()+GetAngleDelta()));
			//fHeadFacing = pBiped->GetFacing()+pBiped->GetHeadTurningAngle();
			fHeadFacing = GetAngleDelta();
			fSpeedFacing = fHeadFacing;
			fModelFacing = fHeadFacing;

			if(bDirs[0]){
				if(bDirs[2]){ // forward left
					fSpeedFacing += -MATH_PI*0.25f;
					if(bIsSwimming)
						stateAnim = STATE_SWIM_FORWORD;
					else
					{
						fModelFacing += -MATH_PI*0.25f;
						stateAnim = STATE_WALK_FORWORD;
					}
				}
				else if (bDirs[3]){ // forward right 
					fSpeedFacing += MATH_PI*0.25f;
					if(bIsSwimming)
						stateAnim = STATE_SWIM_FORWORD;
					else
					{
						fModelFacing += MATH_PI*0.25f;
						stateAnim = STATE_WALK_FORWORD;
					}
				}
				else{ // forward
					fSpeedFacing += 0;
					if(bIsSwimming)
						stateAnim = STATE_SWIM_FORWORD;
					else
					{
						fModelFacing += 0;
						stateAnim = STATE_WALK_FORWORD;
					}
				}
			}
			else if (bDirs[1]){
				if(bDirs[2]){ // back left
					fSpeedFacing += -MATH_PI*0.75f;
					if(bIsSwimming)
						stateAnim = STATE_SWIM_BACKWORD;
					else
						stateAnim = STATE_WALK_BACKWORD;
				}
				else if (bDirs[3]){ // back right 
					fSpeedFacing += MATH_PI*0.75f;
					if(bIsSwimming)
						stateAnim = STATE_SWIM_BACKWORD;
					else
						stateAnim = STATE_WALK_BACKWORD;
				}
				else{ // back
					fSpeedFacing += MATH_PI;
					if(bIsSwimming)
						stateAnim = STATE_SWIM_BACKWORD;
					else
						stateAnim = STATE_WALK_BACKWORD;
				}
			}
			else{
				if(bDirs[2]){ // left
					fSpeedFacing += -fHalfPI;
					if(bIsSwimming)
						stateAnim = STATE_SWIM_LEFT;
					else
					{
						fModelFacing += -fHalfPI;
						stateAnim = STATE_WALK_FORWORD;
					}
				}
				else if (bDirs[3]){ // right 
					fSpeedFacing += fHalfPI;
					if(bIsSwimming)
						stateAnim = STATE_SWIM_RIGHT;
					else
					{
						fModelFacing += fHalfPI;
						stateAnim = STATE_WALK_FORWORD;
					}
				}
				else{ // standing no speed
					fSpeedFacing += 0;
					if(bIsSwimming)
						stateAnim = STATE_IN_WATER;
					else
						stateAnim = STATE_STANDING;
				}
			}

			if(!bIsSwimming)
			{
				// turn the model smoothly. 
				float fTmp = pBiped->GetFacing();
				Math::SmoothMoveAngle1(fTmp, Math::ToStandardAngle(fModelFacing), UPPERBODY_ANGLE_SPEED_MOVING*fTimeDelta);
				fModelFacing = fTmp;
			}
		}
		
		if(bIsJumping )
		{ 
			if(!bIsSwimming)
			{
				// restore to jump animation if it is jumping and that it is not swimming.
				// however, we will update the animation speed for the moving animation.
				// NOTE: jumping in swimming mode is swim upwards,
				if(GetStateAnimName(stateAnim, sAnimName))
				{
					pBiped->UseSpeedFromAnimation(sAnimName);
				}
				stateAnim = oldStateAnim;
				bUpdateSpeed = false;
			}
		}
		else if( bIsJumpEnding )
		{
			// remove jump ending state, if the biped is moving.
			RemoveState(STATE_JUMP_END);
		}

		if(bAccelerating && pBiped->GetSpeed() == 0.f)
			bUpdateModel = false;

		pBiped->ForceMove();
	}
	else
	{
		/** the biped is at standing state, such as attacking, idling, dancing,etc. */
		/*if(bIsSwimming)
			stateAnim = STATE_IN_WATER;
		else if(!bIsJumping)
		{
			stateAnim = STATE_STANDING;
		}*/

		/*fHeadFacing = pBiped->GetFacing()+pBiped->GetHeadTurningAngle();
		fSpeedFacing = pBiped->GetSpeedAngle();
		fModelFacing = pBiped->GetFacing();*/
		if(bHasTurning)
		{
			/** the head turns first, then the body follows up.*/
			float fTotalTurning = GetAngleDelta()+pBiped->GetHeadTurningAngle();
			fHeadFacing = fTotalTurning+pBiped->GetFacing();
			if(fTotalTurning>fHalfPI)
				pBiped->SetFacing(fHeadFacing-fHalfPI);
			else if(fTotalTurning<-fHalfPI)
				pBiped->SetFacing(fHeadFacing+fHalfPI);
			fSpeedFacing=fHeadFacing;
			fModelFacing=fHeadFacing;

			// turn the model smoothly. 
			float fTmp = pBiped->GetFacing();
			Math::SmoothMoveAngle1(fTmp, Math::ToStandardAngle(fModelFacing), UPPERBODY_ANGLE_SPEED_STANDING*fTimeDelta);
			fModelFacing = fTmp;
		}
		else
			bUpdateModel = false;

		if(stateAnim == STATE_JUMP_IN_AIR)
		{
			// force no speed when no movement key in air
			pBiped->UpdateSpeed(0.f);
		}
	}

	/// set biped's model facing, head facing and speed facing.
	if(bUpdateModel)
	{
		pBiped->SetSpeedAngle(Math::ToStandardAngle(fSpeedFacing));
		if (! pBiped->GetIsFlying() )
		{
			pBiped->SetFacing(Math::ToStandardAngle(fModelFacing));
			if ( !(pBiped->HasAnimId(ANIM_RUN) && pBiped->GetAnimation() == ANIM_WALK) )
			{
				pBiped->SetHeadTurningAngle(Math::ToStandardAngle(fHeadFacing - fModelFacing));
				pBiped->SetHeadUpdownAngle(0.f);
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// if biped stops, we will force playing the proper standing animation.
	if(!pBiped->IsStanding() && !bAccelerating)
	{
		// if the biped is not in a standing state, we will play the default stand animation.
		if(stateAnim == STATE_STANDING)
			stateAnim = STATE_STAND;
		else if(stateAnim == STATE_IN_WATER)
			stateAnim = STATE_SWIM;
	}

	//////////////////////////////////////////////////////////////////////////
	// set the default idle animation. 
	if(pAI != 0)
	{
		if(stateAnim == STATE_IN_WATER || stateAnim == STATE_STANDING || stateAnim == STATE_JUMP_IN_AIR)
		{
			int nIdleAnimID = ANIM_STAND;
			if(stateAnim == STATE_IN_WATER)
				nIdleAnimID = ANIM_SWIMIDLE;
			else if(stateAnim == STATE_JUMP_IN_AIR)
				nIdleAnimID = ANIM_JUMP;

			if(pAI->GetIdleAnimationID() != nIdleAnimID)
			{
				pAI->SetIdleAnimationID(nIdleAnimID);
				// force playing the animation
				if(stateAnim == STATE_STANDING)
					stateAnim = STATE_STAND;
				else if(stateAnim == STATE_IN_WATER)
					stateAnim = STATE_SWIM;
			}
		}
	}


	if(FindStateInMemory(STATE_FLY_DOWNWARD)>=0)
	{
		pBiped->UpdateSpeed(7.0f);
		pBiped->ForceMove();
	}
	
	/** if the biped is mounted, play the mounted animation as the default standing animation.*/
	if(IsMounted())
	{
		//if(stateAnim == STATE_STAND)
		//{
		//	stateAnim = STATE_MOUNT;
		//	m_fTimer = 0.f;
		//}
		//else if(stateAnim == STATE_STANDING)
		//{
		//	// only set if the current animation is not mounting.
		//	int nAnimID = ANIM_STAND;
		//	if((pAI==NULL) || (nAnimID=pAI->GetCurrentAnimation()) != ANIM_MOUNT)
		//	{
		//		if(nAnimID== ANIM_STAND)
		//		{
		//			stateAnim = STATE_MOUNT;
		//			// remove old states
		//			m_memory.clear();
		//		}
		//	}
		//}
		//else
		//{
		//	stateAnim = STATE_MOUNT;
		//	m_memory.clear();
		//}

		// lxz 2008.6.20. This is just a quick fix: when character is commanded to be mounted while in air, 
		// the animation state should be changed to standing for later mount animation to be played at a later time. 

		// we will not play mount animation in C++, instead the scripting system should direct a property animation to play 
		// so we just set STATE_STANDING to retain the animation set by the scripting language. 
		stateAnim = STATE_STANDING;
		m_memory.clear();
	}
	
	if(stateAnim == STATE_JUMP_IN_AIR && m_nLastAnimState == stateAnim)
	{
	}
	else
	{
		/** play animation */
		if(GetStateAnimName(stateAnim, sAnimName))
			pBiped->PlayAnimation(sAnimName, bUpdateSpeed);
	}
	m_nLastAnimState = stateAnim;
}

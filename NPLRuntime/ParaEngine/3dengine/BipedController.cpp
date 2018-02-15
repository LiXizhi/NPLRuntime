//-----------------------------------------------------------------------------
// Class:	CBipedController
// Authors:	Li, Xizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2005.6.12
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "FileManager.h"
#include "BipedObject.h"
#include "AIBase.h"

#include "BipedController.h"
#include "AISimulator.h"
#include "BipedStateManager.h"
#include "memdebug.h"

using namespace ParaEngine;

/** @def if defined, when the biped in the follow controller is told to walk to a new destination, it will turn to that direction immediately with using an animation. */
#define IMMEDIATE_FOLLOW_FACING

/** @def a biped will catch up its target only if it is further away from the target than this value. */
#define MIN_FOLLOW_CATCHUP_DIST_SQUARE	9.0f


/************************************************************************/
/* CBipedController                                                     */
/************************************************************************/
CBipedController::CBipedController(CAIBase* pAI)
	:m_bSuspended(false),m_fTimeElapsed(0.f), m_pAI(pAI)
{
}
CBipedController::CBipedController(void)
	:m_bSuspended(false),m_fTimeElapsed(0.f), m_pAI(NULL)
{
}

CBipedController::~CBipedController(void)
{
}

bool CBipedController::IsActive()
{
	return !m_bSuspended;
}
void CBipedController::Suspend()
{
	m_bSuspended = true;
}
void CBipedController::Resume()
{
	m_bSuspended = false;
}
void CBipedController::SetTime(float fTime)
{
	m_fTimeElapsed = fTime;
}

float CBipedController::GetTime()
{
	return m_fTimeElapsed;
}

CBipedObject* CBipedController::GetBiped()
{
	if(m_pAI)
		return m_pAI->GetBiped();
	else
		return NULL;
}

void CBipedController::SetAI(CAIBase* pAI)
{
	m_pAI = pAI;
}

IGameObject*  CBipedController::GetClosestBiped(IGameObject* pInput)
{
	IGameObject*  pClosest = NULL;
	float fMinDist = FLOAT_POS_INFINITY;
	if(pInput)
	{
		int nNumPerceived = pInput->GetNumOfPerceivedObject();
		for(int i=0;i<nNumPerceived;++i)
		{
			IGameObject* pObj = pInput->GetPerceivedObject(i);
			if(pObj!=NULL)
			{
				float fDist = pInput->GetDistanceSq2D(pObj);
				if(fMinDist>fDist)
				{
					pClosest = pObj;
					fMinDist = fDist;
				}
			}
		}
	}
	return pClosest;
}

IGameObject* CBipedController::FindBiped(IGameObject* pInput, const std::string& searchString)
{
	if(pInput)
	{
		int nNumPerceived = pInput->GetNumOfPerceivedObject();
		for(int i=0;i<nNumPerceived;++i)
		{
			IGameObject* pObj = pInput->GetPerceivedObject(i);
			if(pObj!=NULL)
			{
				if( Math::MatchString(searchString, pObj->GetIdentifier()))
					return pObj;
			}
		}
	}
	return NULL;
}
void CBipedController::FrameMove(float fDeltaTime)
{
	if (IsActive()) {
		m_fTimeElapsed+=fDeltaTime;
	}	
}

/************************************************************************/
/* CFollowCtrler                                                        */
/************************************************************************/
CFollowCtrler::CFollowCtrler(CAIBase* pAI)
	:m_fAngleShift(MATH_PI),m_fRadius(2.5f),
	CBipedController(pAI)
{
}

CFollowCtrler::CFollowCtrler(void)
	:m_fAngleShift(MATH_PI),m_fRadius(2.5f),
	CBipedController()
{
}
CFollowCtrler::~CFollowCtrler(void)
{
}

void CFollowCtrler::SetFollowTarget(const std::string& obj)
{
	string::size_type nIndex;
	nIndex = obj.find_first_of(" ");
	m_sFollowTarget = obj.substr(0, nIndex);
	try
	{
		if( nIndex != string::npos )
		{
			string::size_type nFrom = nIndex+1;
			nIndex = obj.find_first_of(" ", nFrom);
			m_fRadius = (float)atof(obj.substr(nFrom, nIndex).c_str());
			
			if( nIndex != string::npos )
			{
				string::size_type nFrom = nIndex+1;
				nIndex = obj.find_first_of(" ", nFrom);
				m_fAngleShift = (float)atof(obj.substr(nFrom, nIndex).c_str());
			}
		}
	}
	catch (...) {
		m_fRadius = 2.5f;
		m_fAngleShift = MATH_PI;
	}
}
const std::string& CFollowCtrler::GetFollowTarget()
{
	return m_sFollowTarget;
}

void CFollowCtrler::FrameMove(float fDeltaTime)
{
	CBipedObject * pBiped = GetBiped();
	IGameObject* pPercevied = FindBiped(pBiped, m_sFollowTarget);
	if(pPercevied!=NULL && pBiped!=NULL && pPercevied->GetDistanceSq2D(pBiped)>MIN_FOLLOW_CATCHUP_DIST_SQUARE)
	{
		float fFacing = pPercevied->GetFacing() + m_fAngleShift;
		DVector3 vDest = pPercevied->GetPosition();

		vDest.x += cosf(fFacing)*m_fRadius;
		vDest.z += sinf(fFacing)*m_fRadius;

		CBipedStateManager* pState =  pBiped->GetBipedStateManager();
		if(pState)
		{
			DVector3 vSrc = pBiped->GetPosition();
			if(fabs(vSrc.x-vDest.x)>0.01f || fabs(vSrc.z-vDest.z)>0.01f)
			{
#ifdef IMMEDIATE_FOLLOW_FACING

				if(Math::ComputeFacingTarget(vDest, vSrc, fFacing))
					pBiped->SetFacing(fFacing);
#endif

				pState->SetPos(vDest);
				pState->SetAngleDelta(pPercevied->GetFacing());
				pState->AddAction(CBipedStateManager::S_WALK_POINT);
			}
		}
	}
}

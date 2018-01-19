//-----------------------------------------------------------------------------
// Class:	CSequenceCtler
// Authors:	Li, Xizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2006.5.12
// Revised:	2007.10.16
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "BipedObject.h"
#include "BipedStateManager.h"

#include "SceneObject.h"

#include "FaceTrackingCtrler.h"

using namespace ParaEngine;


/** @def speed for head turning angle (in rads) in face tracking mode */
#define HEAD_TRACKING_SPEED	 4.0f

/** @def only enable face tracking when the target object is within this distance */
#define MIN_FACE_TRACKING_DIST_SQUARE		100.0f

/************************************************************************/
/* CFaceTrackingCtrler                                                  */
/************************************************************************/
CFaceTrackingCtrler::CFaceTrackingCtrler(CAIBase* pAI)
:CBipedController(pAI),m_fTargetTimer(0), m_vTargetPoint(0,0,0)
{
	
}

CFaceTrackingCtrler::CFaceTrackingCtrler(void) : CBipedController(), m_fTargetTimer(0), m_vTargetPoint(0,0,0)
{
}

CFaceTrackingCtrler::~CFaceTrackingCtrler(void)
{
}

void CFaceTrackingCtrler::FrameMove(float fDeltaTime)
{
	CBipedObject * pBiped = GetBiped();
	if(pBiped ==0)
		return;
	DVector3 vPosTarget(0,0,0);

	bool bHasTarget = false;
	if(m_fTargetTimer > 0)
	{
		// face to a given target point
		vPosTarget = m_vTargetPoint;
		bHasTarget = true;
		m_fTargetTimer -= fDeltaTime;
	}
	else
	{
		// face to the nearest character

		IGameObject* pPercevied = GetClosestBiped(pBiped);
		if((pPercevied!=NULL) && pPercevied->GetDistanceSq2D(pBiped) <MIN_FACE_TRACKING_DIST_SQUARE)
		{
			vPosTarget = pPercevied->GetPosition();
			bHasTarget = true;
		}
	}
	
	if(bHasTarget)
	{
		float fTargetFacing=0;
		DVector3 vPos_src = pBiped->GetPosition();
		Math::ComputeFacingTarget(vPosTarget, vPos_src, fTargetFacing);
		float fHeadTurning = Math::ToStandardAngle(fTargetFacing-pBiped->GetFacing());
		if(fabs(fHeadTurning)>=MATH_PI*0.49f)
			fHeadTurning = 0.f;
		float fNewTurningAngle = pBiped->GetHeadTurningAngle();
		Math::SmoothMoveAngle1(fNewTurningAngle, fHeadTurning, HEAD_TRACKING_SPEED*fDeltaTime);
		pBiped->SetHeadTurningAngle(fNewTurningAngle);
	}
}

void CFaceTrackingCtrler::FaceTarget(const Vector3* pvPos, float fDuration)
{
	if(pvPos!=0)
		m_vTargetPoint = *pvPos;
	m_fTargetTimer = fDuration;
}
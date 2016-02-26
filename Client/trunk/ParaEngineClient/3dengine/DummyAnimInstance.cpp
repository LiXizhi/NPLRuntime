//-----------------------------------------------------------------------------
// Class:	DummyAnimInstance
// Authors:	Li, Xizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2014.9.21
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ParaXModel/AnimTable.h"
#include "DummyAnimInstance.h"
/** @def default walking speed when no model is found. */
#define DEFAULT_WALK_SPEED 4.f

using namespace ParaEngine;

ParaEngine::CDummyAnimInstance::CDummyAnimInstance()
{
	// since it is a singleton, we will never reference count it. 
	addref();
}

CDummyAnimInstance* ParaEngine::CDummyAnimInstance::GetInstance()
{
	static CDummyAnimInstance s_instance;
	return &s_instance;
}

void ParaEngine::CDummyAnimInstance::LoadAnimation(int nAnimID, float * fSpeed, bool bAppend /*= false*/)
{
	if (fSpeed)
	{
		if (nAnimID == ANIM_STAND)
			*fSpeed = 0.f;
		else if (nAnimID == ANIM_WALK)
			*fSpeed = DEFAULT_WALK_SPEED;
		else if (nAnimID == ANIM_RUN)
			*fSpeed = DEFAULT_WALK_SPEED*1.5f;
		else if (nAnimID == ANIM_FLY)
			*fSpeed = DEFAULT_WALK_SPEED*2.f;
		else
			*fSpeed = 0.f;
	}
}

bool ParaEngine::CDummyAnimInstance::HasAnimId(int nAnimID)
{
	return (nAnimID == ANIM_STAND || nAnimID == ANIM_WALK || nAnimID == ANIM_RUN || nAnimID == ANIM_FLY);
}

void ParaEngine::CDummyAnimInstance::GetSpeedOf(const char * sName, float * fSpeed)
{
	int nAnimID = CAnimTable::GetAnimIDByName(sName);
	if (fSpeed)
	{
		if (nAnimID == ANIM_STAND)
			*fSpeed = 0.f;
		else if (nAnimID == ANIM_WALK)
			*fSpeed = DEFAULT_WALK_SPEED;
		else if (nAnimID == ANIM_RUN)
			*fSpeed = DEFAULT_WALK_SPEED*1.5f;
		else if (nAnimID == ANIM_FLY)
			*fSpeed = DEFAULT_WALK_SPEED*2.f;
		else
			*fSpeed = 0.f;
	}
}


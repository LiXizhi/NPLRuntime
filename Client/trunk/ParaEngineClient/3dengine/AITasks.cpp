//-----------------------------------------------------------------------------
// Class:	AITasks
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2005
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include <string>
#include "AITasks.h"
#include <algorithm>
using namespace ParaEngine;


using std::string;

AITask_MovieTrack::Keyframe::Keyframe()
{
	sAnimation = "stand";
	vPos = Vector3(0,0,0);
	fFacing = 0;
	fDuration = 0;
}

AITask_MovieTrack::Keyframe::Keyframe(const char* anim, float x, float y, float z, float facing, float duration)
{
	sAnimation = anim;
	vPos.x = x;
	vPos.y = y;
	vPos.z = z;
	fFacing = facing;
	fDuration = duration;
}

AITask_MovieTrack::AITask_MovieTrack()
{
	fStartTime = 0;
	fPathFindingTime = 0;
}

/**
str[in]: a list of key
no spaces in the str are allowed.
[<string,number,number,number,number,number>]+
<const char* anim, float x, float y, float z, float facing, float duration>
*/
AITask_MovieTrack::AITask_MovieTrack(const char * str)
{
	fStartTime = 0;
	char anim[30];
	float x, y, z, facing, duration;

	/// replace , with ' ', so that the string can be parsed by sscanf
	string frameStr = str;
	int nLen = (int)frameStr.size();
	int nStart=0;
	for(int i=0; i<nLen;i++)
	{
		if(frameStr[i] == ',')
			frameStr[i] = ' ';
		else if(frameStr[i] == '<')
			nStart = i;
		else if(frameStr[i] == '>')
		{
			if(sscanf(frameStr.c_str()+nStart, "<%30s %f %f %f %f %f>", anim, &x, &y, &z, &facing, &duration) == 6)
			{
				listKeyframes.push_back(Keyframe(anim,x, y, z, facing, duration));
			}
		}
		
	}
}

AITask_Evade::AITask_Evade(const char* target, float distance1, float distance2)
{
	strcpy(m_sTarget, target);
	m_fDist1 = max(distance1, distance2);
	m_fDist2 = min(distance1, distance2);
}

AITask_Evade::AITask_Evade(const char *)
{
}

AITask_Follow::AITask_Follow(const char* target, float distance1, float distance2)
{
	strcpy(m_sTarget, target);
	m_fDist1 = min(distance1, distance2);
	m_fDist2 = max(distance1, distance2);
	m_pTarget = NULL;
}

AITask_Follow:: AITask_Follow(const char *)
{
	m_pTarget = NULL;
}

void ParaEngine::AITask::DestroyMe()
{
	if (pAITask == NULL) return;
	switch (m_nType)
	{
	case DieAndReborn:
		delete pAITask_DieAndReborn;
		break;
	case Evade:
		delete pAITask_Evade;
		break;
	case Follow:
		delete pAITask_Follow;
		break;
	case WanderNearby:
		delete pAITask_WanderNearby;
		break;
	case Movie:
		delete pAITask_Movie;
		break;
	default:
		// delete pAITask;
		break;
	}
}

ParaEngine::AITask::AITask(AITaskType type, void* pt)
{
	m_nType = type;
	pAITask = pAITask;
	m_bIsFinished = false;
}

ParaEngine::AITask::AITask()
{
	pAITask = NULL;
	m_bIsFinished = false;
}

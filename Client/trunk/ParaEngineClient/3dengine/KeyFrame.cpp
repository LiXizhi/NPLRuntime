//-----------------------------------------------------------------------------
// Class:	ActionKey
// Authors:	Li, Xizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2005
// Revised: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "KeyFrame.h"
#include "ParaXModel/AnimTable.h"
#include "memdebug.h"

using namespace ParaEngine;

/************************************************************************/
/* EffectKey                                                            */
/************************************************************************/

EffectKey::EffectKey(int effectid, const string& target)
{
	nEffectID = effectid;
	sTarget = target;
}

EffectKey::EffectKey(int effectid, const char* target)
{
	nEffectID = effectid;
	sTarget = target;
}
/************************************************************************/
/* ActionKey                                                            */
/************************************************************************/
ActionKey::ActionKey(const char* name)		
{
	sActionName = name;
}

ActionKey::ActionKey(const std::string& name)
{
	sActionName = name;
}

ActionKey::ActionKey(int nAnimID)
{
	char tmp[11];
	itoa(nAnimID, tmp, 10);
	tmp[10] = '\0';
	sActionName = tmp;
}

ActionKey::ActionKey(KeyID keyID)
{
	switch(keyID) {
	case JUMP:
		sActionName = "js";
		break;
	case TOGGLE_TO_WALK:
		sActionName = "_toggletowalk";
		break;
	case TOGGLE_TO_RUN:
		sActionName = "_toggletorun";
		break;
	case MOUNT:
		sActionName = "_mount";
		break;
	default:
		break;
	}
}

const char* ActionKey::ToString() const
{
	return sActionName.c_str();
}

int ActionKey::ToAnimID() const
{
	int nID = -1;
	if(sActionName[0]>='0' && sActionName[0]<='9')
	{
		try{
			nID = atoi(sActionName.c_str());
		}
		catch (...) {
			nID = -1;
		}
	}
	if(nID>=0)
		return nID;
	else
	{
		return CAnimTable::GetInstance()->GetAnimIDByName(ToString());
	}
}

bool ActionKey::IsJump() const
{
	return (sActionName== "js");
}

bool ActionKey::IsToggleToWalk() const
{
	return (sActionName == "_toggletowalk");
}

bool ActionKey::IsToggleToRun() const
{
	return (sActionName == "_toggletorun");
}
bool ActionKey::IsMount() const
{
	return (sActionName == "_mount");
}
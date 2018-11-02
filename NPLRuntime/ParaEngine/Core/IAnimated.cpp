//-----------------------------------------------------------------------------
// Class: IAnimated
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Date: 2015.9.2
// Notes:  
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "IAnimated.h"

int ParaEngine::IAnimated::GetNumKeys()
{
	return 0;
}

void ParaEngine::IAnimated::SetNumKeys(int nKeyCount)
{

}

int ParaEngine::IAnimated::GetNextKeyIndex(int nTime)
{
	return -1;
}

int ParaEngine::IAnimated::AddKey(int time, bool* isKeyExist)
{
	return 0;
}

void ParaEngine::IAnimated::SetTime(int nIndex, int nTime)
{

}

int ParaEngine::IAnimated::GetTime(int nIndex)
{
	return 0;
}

void ParaEngine::IAnimated::SetValue(int nIndex, const std::string& val)
{

}

bool ParaEngine::IAnimated::GetValue(int nIndex, int& val)
{
	return false;
}

bool ParaEngine::IAnimated::GetValueByTime(int nTime, std::string& val)
{
	return false;
}

bool ParaEngine::IAnimated::GetValueByTime(int nTime, int& val)
{
	return false;
}

bool ParaEngine::IAnimated::GetValueByTime(int nTime, float& val)
{
	return false;
}

bool ParaEngine::IAnimated::GetValueByTime(int nTime, double& val)
{
	return false;
}

bool ParaEngine::IAnimated::GetValueByTime(int nTime, Vector3& val)
{
	return false;
}

bool ParaEngine::IAnimated::GetValueByTime(int nTime, Vector2& val)
{
	return false;
}

bool ParaEngine::IAnimated::GetValueByTime(int nTime, Quaternion& val)
{
	return false;
}

bool ParaEngine::IAnimated::GetValue(int nIndex, float& val)
{
	return false;
}

void ParaEngine::IAnimated::SetValue(int nIndex, const int& val)
{

}

bool ParaEngine::IAnimated::GetValue(int nIndex, Vector2& val)
{
	return false;
}

void ParaEngine::IAnimated::SetValue(int nIndex, const float& val)
{

}

bool ParaEngine::IAnimated::GetValue(int nIndex, Vector3& val)
{
	return false;
}

void ParaEngine::IAnimated::SetValue(int nIndex, const Vector2& val)
{

}

void ParaEngine::IAnimated::SetValue(int nIndex, const Quaternion& val)
{

}

void ParaEngine::IAnimated::SetValue(int nIndex, const double& val)
{

}

bool ParaEngine::IAnimated::GetValue(int nIndex, std::string& val)
{
	return false;
}

void ParaEngine::IAnimated::SetValue(int nIndex, const Vector3& val)
{

}

bool ParaEngine::IAnimated::GetValue(int nIndex, Quaternion& val)
{
	return false;
}

bool ParaEngine::IAnimated::GetValue(int nIndex, double& val)
{
	return false;
}

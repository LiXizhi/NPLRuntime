//-----------------------------------------------------------------------------
// Class:	ParaConfig
// Authors:	Liu weili
// Company: ParaEngine
// Emails:	liuweili@21cn.com
// Date:	2006.3
// Desc: cross platformed by LiXizhi
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ic/ICConfigManager.h"

#include "ParaScriptingIC.h"
using namespace ParaScripting;
using namespace ParaInfoCenter;
using namespace ParaEngine;
bool ParaConfig::GetDoubleValueEx(const char* szName,double *value,int index)
{
	CICConfigManager* cm=CGlobals::GetICConfigManager();
	if (cm->GetDoubleValue(szName,value,index)==S_OK) {
		return true;
	}
	return false;
}
bool ParaConfig::GetDoubleValue(const char* szName,double *value)
{
	return GetDoubleValueEx(szName,value,0);
}

bool ParaConfig::GetIntValueEx(const char* szName,int *value,int index)
{
	CICConfigManager* cm=CGlobals::GetICConfigManager();
	if (cm->GetIntValue(szName,value,index)==S_OK) {
		return true;
	}
	return false;
}

bool ParaConfig::GetIntValue(const char* szName,int *value)
{
	return GetIntValueEx(szName,value,0);
}

string ParaConfig::GetTextValueEx(const char* szName,int index)
{
	string value;
	CICConfigManager* cm=CGlobals::GetICConfigManager();
	if (cm->GetTextValue(szName,value,index)==S_OK) {
		return value;
	}
	return value;
}

string ParaConfig::GetTextValue(const char* szName)
{
	return GetTextValueEx(szName,0);
}

bool ParaConfig::SetDoubleValueEx(const char* szName,double value,int index)
{
	CICConfigManager* cm=CGlobals::GetICConfigManager();
	if (cm->SetDoubleValue(szName,value,index)==S_OK) {
		return true;
	}
	return false;
}

bool ParaConfig::SetDoubleValue(const char* szName,double value)
{
	return SetDoubleValueEx(szName,value,0);
}

bool ParaConfig::SetIntValueEx(const char* szName,int value,int index)
{
	CICConfigManager* cm=CGlobals::GetICConfigManager();
	if (cm->SetIntValue(szName,value,index)==S_OK) {
		return true;
	}
	return false;
}

bool ParaConfig::SetIntValue(const char* szName,int value)
{
	return SetIntValueEx(szName,value,0);
}

bool ParaConfig::SetTextValueEx(const char* szName,const char* value,int index)
{
	CICConfigManager* cm=CGlobals::GetICConfigManager();
	if (cm->SetTextValue(szName,value,index)==S_OK) {
		return true;
	}
	return false;
}

bool ParaConfig::SetTextValue(const char* szName,const char* value)
{
	return SetTextValueEx(szName,value,0);
}

bool ParaConfig::AppendDoubleValue(const char* szName,double value)
{
	CICConfigManager* cm=CGlobals::GetICConfigManager();
	if (cm->AppendDoubleValue(szName,value)==S_OK) {
		return true;
	}
	return false;
}
bool ParaConfig::AppendIntValue(const char* szName,int value)
{
	CICConfigManager* cm=CGlobals::GetICConfigManager();
	if (cm->AppendIntValue(szName,value)==S_OK) {
		return true;
	}
	return false;
}
bool ParaConfig::AppendTextValue(const char* szName,const char* value)
{
	CICConfigManager* cm=CGlobals::GetICConfigManager();
	if (cm->AppendTextValue(szName,value)==S_OK) {
		return true;
	}
	return false;
}

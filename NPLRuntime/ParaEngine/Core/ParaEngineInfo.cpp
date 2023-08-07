//-----------------------------------------------------------------------------
// Title: ParaEngine Hard-coded strings.
// Authors:	Li,Xizhi
// Emails:	
// Date:	2006.1.18
// Desc: this functions defines strings embeded in ParaEngine executable or lib. 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ParaEngineInfo.h"
#include <string>
using namespace std;

#define PARAENGINE_MAJOR_VERSION	1
// 升级版本号 2023/8/17 minor ver -> 5
#define PARAENGINE_MINOR_VERSION	5

#define PRODUCT_VERSION_MAJOR	1
#define PRODUCT_VERSION_MINOR	0

namespace ParaEngineInfo
{
	string g_sVersion;
	//string g_sWaterMark = "Powered by ParaEngine Tech Studio";
	string g_sWaterMark = "Powered by ParaEngine - Demo Version";
	string g_sPublicKey = "PARAENGINE_LiXizhi";

	string g_sAuthorizedTo = "ParaEngine Dev Team";
	string g_sCopyright = "Copyright(c) 2005-2023";

	string CParaEngineInfo::GetVersion(){
		if(g_sVersion.empty())
		{
			char buf[100];
			snprintf(buf, 100, "%d.%d.%d.%d", PARAENGINE_MAJOR_VERSION, PARAENGINE_MINOR_VERSION, PRODUCT_VERSION_MAJOR,PRODUCT_VERSION_MINOR);
			g_sVersion = buf;
		}
		return g_sVersion;
	}

	int CParaEngineInfo::GetParaEngineMajorVersion()
	{
		return PARAENGINE_MAJOR_VERSION;
	}
	int CParaEngineInfo::GetParaEngineMinorVersion()
	{
		return PARAENGINE_MINOR_VERSION;

	}
	int CParaEngineInfo::GetProductMajorVersion()
	{
		return PRODUCT_VERSION_MAJOR;
	}
	int CParaEngineInfo::GetProductMinorVersion()
	{
		return PRODUCT_VERSION_MINOR;
	}

	string CParaEngineInfo::GetWaterMarkText(){
		return g_sWaterMark;
	}
	string CParaEngineInfo::GetPublicKey(){
		return g_sPublicKey;
	}
	string CParaEngineInfo::GetAuthorizedTo(){
		return g_sAuthorizedTo;
	}
	string CParaEngineInfo::GetCopyright(){
		return g_sCopyright;
	}
	void AuthorizationCheck()
	{
		
	}
}

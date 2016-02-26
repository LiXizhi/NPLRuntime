//-----------------------------------------------------------------------------
// Class: CWorldInfo	
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2006.6.5
// Desc: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"

#include "WorldInfo.h"
#include "memdebug.h"

using namespace ParaTerrain;
using namespace ParaEngine;

CWorldInfo::CWorldInfo(void)
{
}

CWorldInfo::~CWorldInfo(void)
{
}

CWorldInfo::CWorldInfo(const string& sWorldConfigFile)
:CWorldNameFactory(sWorldConfigFile)
{
		
}
CWorldInfo& CWorldInfo::GetSingleton()
{
	static CWorldInfo g_singleton("_emptyworld/_emptyworld.worldconfig.txt");
	return g_singleton;
}


void CWorldInfo::SetScriptSandBox(const char* sSandboxNeuronFile)
{
	if(sSandboxNeuronFile != NULL)
		m_sScriptSandbox = sSandboxNeuronFile;
	else
		m_sScriptSandbox = "";
}

const char* CWorldInfo::GetScriptSandBox()
{
	if(m_sScriptSandbox.empty())
		return NULL;
	else
		return m_sScriptSandbox.c_str();
}
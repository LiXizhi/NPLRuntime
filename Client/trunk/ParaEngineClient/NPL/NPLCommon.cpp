//-----------------------------------------------------------------------------
// Class:	NPL commons 
// Authors:	Li, Xizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2009.5.23
// Desc:  Cross platform for both server and client
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "NPLCommon.h"
#include "NPLRuntime.h"

using namespace NPL;

/** max allowed timer due time */
#define MAX_TIMER_DUE_TIME		10000000

NPLFileName::NPLFileName()
{
}


NPLFileName::NPLFileName(const char * sFilePath)
{
	FromString(sFilePath);
}

void NPLFileName::SetRelativePath(const char* sPath, int nCount)
{
	if(nCount <= 0)
	{
		sRelativePath = sPath;
		nCount = (int)sRelativePath.size();
	}
	else
	{
		sRelativePath.assign(sPath, nCount);
	}
	for(int i=0; i<nCount ;i++)
	{
		if(sRelativePath[i] == '\\')
			sRelativePath[i] = '/';
	}
}

void NPLFileName::ToString(string & output)
{
	output.clear();
	if(!sRuntimeStateName.empty())
	{
		output.append("(");
		output.append(sRuntimeStateName);
		output.append(")");
	}
	
	// the activation type
	if( ! sNID.empty() )
		output.append(sNID);
	if( ! sRelativePath.empty() )
		output.append(sRelativePath);
	if( ! sDNSServerName.empty() )
	{
		output.append("@");
		output.append(sDNSServerName);
	}
}

string NPLFileName::ToString()
{
	string output;
	ToString(output);
	return output;
}

void NPLFileName::FromString(const char* sFilePath)
{
	// for empty string, default to local Glia file.
	if(sFilePath[0] == '\0')
	{
		// FromString("(gl)script/empty.lua");
		sRuntimeStateName.clear();
		sNID.clear();
		sDNSServerName.clear();
		sRelativePath.clear();
		return;
	}

	int i = 0;
	int nNIDIndex = 0;
	int nRelativePathIndex;
	int nDNSServerIndex;

	bool bExplicitActivationType = false;
	/// get the activation type
	if(sFilePath[i] == '(')
	{
		bExplicitActivationType = true;
		i++;
		while( (sFilePath[i]!=')') && (sFilePath[i]!='\0'))
		{
			i++;
		}
		i++;
		if( !(i==4 && (sFilePath[1]=='g') && (sFilePath[2]=='l')) )
			sRuntimeStateName.assign(sFilePath+1, i-2);
		else
			sRuntimeStateName.clear();
		nNIDIndex = i;
	}

	/// get namespace
	while( (sFilePath[i]!=':') && (sFilePath[i]!='\0'))
		i++;
	
	if(sFilePath[i]=='\0')
	{
		sNID.clear();
		sRelativePath.assign(sFilePath+nNIDIndex, i-nNIDIndex);
		sDNSServerName.clear();
		return;
	}
	else 
	{
		if(i>nNIDIndex)
			sNID.assign(sFilePath+nNIDIndex, i-nNIDIndex);
		else
			sNID.clear();

		/// get relative path
		nRelativePathIndex = i+1;
		i++;
	}
	
	/// get relative path
	while( (sFilePath[i]!='@') && (sFilePath[i]!='\0') )
		i++;
	SetRelativePath(sFilePath+nRelativePathIndex, i - nRelativePathIndex);

	if(sFilePath[i]=='\0')
	{
		sDNSServerName.clear();
		return;
	}
	else 
		i++;

	/// get DNS server name
	nDNSServerIndex = i;

	while( sFilePath[i]!='\0')
		i++;
	sDNSServerName.assign(sFilePath+nDNSServerIndex, i - nDNSServerIndex);
}
//////////////////////////////////////////////////////////////////////////
//
// NPLTimer
//
//////////////////////////////////////////////////////////////////////////

NPLTimer::NPLTimer( const string& nplFile, const string& sCode, float fInterval )
	:m_nplFile(nplFile), m_sCode(sCode), m_nInterval((int)(fInterval/0.001f)),m_lastTick(0)
{
}

NPLTimer::NPLTimer( const string& nplFile, const string& sCode, DWORD dwInterval )
	:m_nplFile(nplFile), m_sCode(sCode), m_nInterval(dwInterval),m_lastTick(0)
{
}

void NPL::NPLTimer::Change( int dueTime, int period )
{
	ParaEngine::Lock lock_(m_mutex);
	m_nInterval = period;
	m_lastTick = ::GetTickCount() + dueTime-m_nInterval;
}

bool NPL::NPLTimer::Tick(NPLRuntimeState_ptr runtime_state, DWORD nTickCount)
{
	if(nTickCount == 0)
	{
		nTickCount = ::GetTickCount();
	}

	bool bActivate = false;
	{
		ParaEngine::Lock lock_(m_mutex);
		if( ((long long)nTickCount-(long long)m_lastTick)>=m_nInterval || 
			((nTickCount<m_lastTick) && (((int)nTickCount+(int)MAX_TIMER_DUE_TIME)<(long long)m_lastTick))) 
		{
			m_lastTick = nTickCount;
			bActivate = true;
		}
	}
	if(bActivate)
		ParaEngine::CGlobals::GetNPLRuntime()->NPL_Activate(runtime_state, m_nplFile.c_str(), m_sCode.c_str(), (int)m_sCode.size());
	return bActivate;
}
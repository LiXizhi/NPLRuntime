//-----------------------------------------------------------------------------
// Class: CEventHandler	
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Date:	2006.1.1
// Revised: 2006.8.16
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "Events_def.h"
#include "EventHandler.h"
#include "EventsCenter.h"
#include "AISimulator.h"

#include "util/StringHelper.h"

using namespace ParaEngine;


CEventHandler::CEventHandler(DWORD nType, const string& sID, const string& sScript)
: m_sID(sID),m_type(nType)
{
	SetScript(sScript);
}

CEventHandler::CEventHandler(const string& sID, const string& sScript)
:m_sID(sID),m_type(0)
{
	if(m_sID[0] == '_')
	{
		if(m_sID[1]=='m')
		{
			if(m_sID[2] == 'm')
				m_type |= EH_MOUSE_MOVE;
			else if(m_sID[2] == 'd')
				m_type |= EH_MOUSE_DOWN;
			else if(m_sID[2] == 'u')
				m_type |= EH_MOUSE_UP;
			else if(m_sID[2] == 'w')
				m_type |= EH_MOUSE_WHEEL;
			else
				m_type |= EH_MOUSE; // mouse click
		}
		else if(m_sID[1]=='k')
		{
			if(m_sID[2] == 'u')
				m_type |= EH_KEY_UP;
			else
				m_type |= EH_KEY;
		}
		else if(m_sID[1]=='n')
			m_type |= EH_NETWORK;
		else if(m_sID[1]=='s')
			m_type |= EH_SYSTEM;
		else if(m_sID[1]=='e')
			m_type |= EH_EDITOR;
		else if (m_sID[1] == 't')
			m_type |= EH_TOUCH;
		else if (m_sID[1] == 'a')
			m_type |= EH_ACCELEROMETER;
		else
			m_type |= 0xffff; // any type
	}
	else
		m_type |= 0xffff; // any type
	SetScript(sScript);
};

void CEventHandler::SetScript(const string& sScript) 
{
	StringHelper::DevideString(sScript, m_sFileName, m_sCode,';');
}

int CEventHandler::OnEvent(const IEvent* event, const string& sScriptCode)
{
	if (event != 0 && event->IsAsyncMode())
	{
		if (GetCode().empty())
		{
			return CGlobals::GetAISim()->NPLActivate(GetFileName().c_str(), sScriptCode.c_str());
		}
		else
		{
			string sCode = sScriptCode + GetCode();
			return CGlobals::GetAISim()->NPLActivate(GetFileName().c_str(), sCode.c_str());
		}
	}
	else
	{
		if (GetFileName().empty())
		{
			if (GetCode().empty())
			{
				CGlobals::GetAISim()->NPLDoString(sScriptCode.c_str(), sScriptCode.size());
			}
			else
			{
				string sCode = sScriptCode + GetCode();
				CGlobals::GetAISim()->NPLDoString(sCode.c_str(), sCode.size());
			}
		}
		else
		{
			OUTPUT_LOG("warn: event fired in non-async mode should not contain any NPL activation file: %s", GetFileName().c_str());
		}
		return 0;
	}
}
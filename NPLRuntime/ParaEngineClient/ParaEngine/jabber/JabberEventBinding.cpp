//-----------------------------------------------------------------------------
// Class:	jabber event
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2008.12.5
// Desc: partially based on ParaEngine::EventBinding class code. 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "JabberEventBinding.h"

using namespace std;
using namespace ParaEngine;

map<string,int> CJabberEventBinding::StringToEventTable;
map<int,string> CJabberEventBinding::EventToStringTable;
bool CJabberEventBinding::g_bStaticInited = false;

ParaEngine::CJabberEventBinding::CJabberEventBinding()
{
	m_pEventEnable.reset();
	m_pEventToScript.clear();
	InitEventMappingTable(false);
}

ParaEngine::CJabberEventBinding::~CJabberEventBinding()
{
	
}

#define BUILD_EVENT_MAPPING(a)  EventToStringTable[a]=""#a;StringToEventTable[""#a]=a;

void ParaEngine::CJabberEventBinding::StaticInit()
{
	// the following defines do the following 
	// EventToStringTable[JE_NONE]="JE_NONE";
	// StringToEventTable["JE_NONE"]=JE_NONE;
	
	BUILD_EVENT_MAPPING(JE_NONE)
	
	BUILD_EVENT_MAPPING(JE_OnPresence)
	BUILD_EVENT_MAPPING(JE_OnError)
	BUILD_EVENT_MAPPING(JE_OnRegistered)
	BUILD_EVENT_MAPPING(JE_OnRegisterInfo)
	BUILD_EVENT_MAPPING(JE_OnIQ)
	BUILD_EVENT_MAPPING(JE_OnMessage)
	BUILD_EVENT_MAPPING(JE_OnAuthError)
	BUILD_EVENT_MAPPING(JE_OnLoginRequired)
	
	BUILD_EVENT_MAPPING(JE_OnConnect)
	BUILD_EVENT_MAPPING(JE_OnAuthenticate)
	BUILD_EVENT_MAPPING(JE_OnDisconnect)
	BUILD_EVENT_MAPPING(JE_OnTLSConnect)

	BUILD_EVENT_MAPPING(JE_OnRoster)
	BUILD_EVENT_MAPPING(JE_OnRosterBegin)
	BUILD_EVENT_MAPPING(JE_OnRosterItem)
	BUILD_EVENT_MAPPING(JE_OnRosterEnd)
	BUILD_EVENT_MAPPING(JE_OnRosterError)
	BUILD_EVENT_MAPPING(JE_OnRosterPresence)
	
	BUILD_EVENT_MAPPING(JE_OnStanzaMessageChat)
	BUILD_EVENT_MAPPING(JE_OnStanzaMessageGroupChat)
	BUILD_EVENT_MAPPING(JE_OnStanzaMessageHeadline)
	BUILD_EVENT_MAPPING(JE_OnStanzaMessageNormal)
	BUILD_EVENT_MAPPING(JE_OnChatState)
	BUILD_EVENT_MAPPING(JE_OnMessageEvent)

	
	BUILD_EVENT_MAPPING(JE_OnResourceBindError)
	BUILD_EVENT_MAPPING(JE_OnSessionCreateError)

	BUILD_EVENT_MAPPING(JE_OnItemSubscribed)
	BUILD_EVENT_MAPPING(JE_OnItemAdded)
	BUILD_EVENT_MAPPING(JE_OnItemUnsubscribed)
	BUILD_EVENT_MAPPING(JE_OnItemRemoved)
	BUILD_EVENT_MAPPING(JE_OnItemUpdated)
	BUILD_EVENT_MAPPING(JE_OnPresence)

	BUILD_EVENT_MAPPING(JE_OnSelfPresence)
	BUILD_EVENT_MAPPING(JE_OnSubscriptionRequest)
	BUILD_EVENT_MAPPING(JE_OnUnsubscriptionRequest)
	BUILD_EVENT_MAPPING(JE_OnNonrosterPresence)
}

void ParaEngine::CJabberEventBinding::InitEventMappingTable( bool bDisable/*=false*/ )
{
	for (int i=0;i<JE_LAST;i++) 
	{
		m_pEventMappingTable[i].clear();
		m_pEventMappingTable[i].insert(i);
	}
	if (bDisable) 
		m_pEventEnable.reset();
	else
		m_pEventEnable.set();
}

void ParaEngine::CJabberEventBinding::EnableEvent( int eEvent )
{
	m_pEventEnable[eEvent]=true;
}

void ParaEngine::CJabberEventBinding::DisableEvent( int eEvent )
{
	m_pEventEnable[eEvent]=false;
}

void ParaEngine::CJabberEventBinding::MapEvent( int eSrcEvent, int eDestEvent )
{
	m_pEventMappingTable[eSrcEvent].insert(eDestEvent);
}

void ParaEngine::CJabberEventBinding::UnmapEvent( int eSrcEvent, int eDestEvent )
{
	m_pEventMappingTable[eSrcEvent].erase(eDestEvent);
}

bool ParaEngine::CJabberEventBinding::IsMapTo( int eSrcEvent, int eDestEvent ) const
{
	if (!m_pEventEnable.test(eSrcEvent)||!m_pEventEnable.test(eDestEvent)) 
		return false;
	if (eSrcEvent==eDestEvent) 
		return true;
	if (m_pEventMappingTable[eSrcEvent].find(eDestEvent)!=m_pEventMappingTable[eSrcEvent].end())
		return true;
	else
		return false;
}

void ParaEngine::CJabberEventBinding::MapEventToScript( int eEvent,const CScriptCallBack *script )
{
	if (script==NULL) 
		m_pEventToScript.erase(eEvent);
	else
		m_pEventToScript[eEvent]=*script;
}

void ParaEngine::CJabberEventBinding::UnmapEventToScript( int eEvent )
{
	if (m_pEventToScript.find(eEvent)!=m_pEventToScript.end()) 
		m_pEventToScript.erase(eEvent);
}


void ParaEngine::CJabberEventBinding::ClearAllScripts()
{
	m_pEventToScript.clear();
}

const CJabberEventBinding::CScriptCallBack * ParaEngine::CJabberEventBinding::GetEventScript( int eEvent ) const
{
	map<int,CScriptCallBack>::const_iterator iter;
	if ((iter=m_pEventToScript.find(eEvent))!=m_pEventToScript.end()) 
		return &iter->second;
	else
		return NULL;
}
bool ParaEngine::CJabberEventBinding::HasEventScript( int eEvent ) const
{
	return (m_pEventToScript.find(eEvent)!=m_pEventToScript.end());
}

int ParaEngine::CJabberEventBinding::StringToEventValue( const string &str )
{
	if(!g_bStaticInited)
		StaticInit();
	map<string,int>::iterator iter;
	if ((iter=StringToEventTable.find(str))!=StringToEventTable.end()) {
		return iter->second;
	}
	return 0;
}

const string& ParaEngine::CJabberEventBinding::EventValueToString( int value )
{
	if(!g_bStaticInited)
		StaticInit();

	map<int,string>::iterator iter;
	if ((iter=EventToStringTable.find(value))!=EventToStringTable.end()) {
		return iter->second;
	}

	return EventToStringTable[0];
}


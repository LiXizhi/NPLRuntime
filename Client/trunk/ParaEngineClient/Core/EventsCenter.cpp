//-----------------------------------------------------------------------------
// Class:	CEventsCenter
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2006.8.16
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "EventsCenter.h"
#include "util/StringHelper.h"
#include "AISimulator.h"
#include "Type.h"
#include "ObjectManager.h"
#include "2dengine/EventBinding.h"
#include "memdebug.h"

using namespace ParaEngine;

/** default size of the event pool */
#define DEFAULT_EVENT_POOL_SIZE	5000

/** @page events

@par Mouse related events
mouse_button="{left|middle|right|}";mouse_x=X;mouse_y=Y;

@par Key related events
keyboard_key={};

@par Editor related events
event_type=number;
*/

//////////////////////////////////////////////////////////////////////////
//
// event script helper functions
//
//////////////////////////////////////////////////////////////////////////

//finish refactoring 
namespace ParaEngine
{
	string GenerateOnMouseScript(int MouseState, int X, int Y)
	{
		char ctemp[30];
		string code;
		CObjectManager* pOm=&CSingleton<CObjectManager>::Instance();
		if (!pOm->IsExist("default_CEventBinding")) {
			CEventBinding::StaticInit();
		}
		const CEventBinding *eb=(CEventBinding*)pOm->GetObject("default_CEventBinding");
		if (eb->IsMapTo(MouseState,EM_MOUSE_MOVE)) 
		{
			code+="mouse_dx=";
			ParaEngine::StringHelper::fast_itoa(X,ctemp,30);
			code+=ctemp;code+=";";
			code+="mouse_dy=";
			ParaEngine::StringHelper::fast_itoa(Y,ctemp,30);
			code+=ctemp;code+=";";
		}
		else
		{
			if (eb->IsMapTo(MouseState,EM_MOUSE_LEFT)) 
				code="mouse_button=\"left\";";
			else if(eb->IsMapTo(MouseState,EM_MOUSE_RIGHT))
				code="mouse_button=\"right\";";
			else if(eb->IsMapTo(MouseState,EM_MOUSE_MIDDLE))
				code="mouse_button=\"middle\";";

			if(eb->IsMapTo(MouseState,EM_MOUSE_WHEEL))
			{
				code+="mouse_wheel=";
				ParaEngine::StringHelper::fast_itoa(X,ctemp,30);
				code+=ctemp;code+=";";
			}
			else
			{
				code+="mouse_x=";
				ParaEngine::StringHelper::fast_itoa(X,ctemp,30);
				code+=ctemp;code+=";";
				code+="mouse_y=";
				ParaEngine::StringHelper::fast_itoa(Y,ctemp,30);
				code+=ctemp;code+=";";
			}
		}
		
		return code;
	}

	string GenerateOnKeyboardScript(int option)
	{
		string code="keyboard_key={};";
		//DWORD a=0,b=0;
		//char ctemp[30];
		//string charseq="";
		//CDirectKeyboard *pKeyboard=CGUIRoot::Instance()->m_pKeyboard;
		//for (a=0;a<m_pKeyEvent.m_keyboard.Size;a++) {
		//	if(((m_pKeyEvent.m_keyboard.KeyEvents[a].dwData&0x80)!=0&&(option&CGUIEvent::IGNORE_DOWN_EVENTS)==0)|| \
		//	((m_pKeyEvent.m_keyboard.KeyEvents[a].dwData&0x80)==0&&(option&CGUIEvent::IGNORE_UP_EVENTS)==0)){			
		//		b++;
		//		::itoa(b,ctemp,10);
		//		code+="keyboard_key[";
		//		code+=ctemp;
		//		code+="]=";
		//		::itoa(m_pKeyEvent.m_keyboard.KeyEvents[a].dwOfs,ctemp,10);
		//		code+=ctemp;
		//		code+="];";
		//		charseq+=m_pKeyEvent.GetChar(pKeyboard->m_lastkeystate,m_pKeyEvent.m_keyboard.KeyEvents[a].dwOfs);
		//	}
		//}
		//code+="keyboard_character=\"";
		//code+=charseq;
		//code+="\";";
		return code;
	}
}//namespace ParaEngine

//////////////////////////////////////////////////////////////////////////
//
// CEventsCenter
//
//////////////////////////////////////////////////////////////////////////

CEventsCenter::CEventsCenter(void)
{
	InvalidateEventCounts();
	m_nMaxPoolSize = DEFAULT_EVENT_POOL_SIZE;
}

CEventsCenter::~CEventsCenter(void)
{
	UnregisterAllEvent();
}

CEventsCenter* CEventsCenter::GetInstance()
{
	static CEventsCenter g_instance;
	return &g_instance;
}

void CEventsCenter::Reset()
{
	UnregisterAllEvent();

	ParaEngine::Lock lock_(m_mutex);
	m_unhandledEventPool.clear();
}

CEventHandler* CEventsCenter::GetEventHandlerByID(const string& sID)
{
	EventHandler_List_t::iterator itCurCP, itEndCP = m_sEventHandlerList.end();

	for( itCurCP = m_sEventHandlerList.begin(); itCurCP != itEndCP; ++ itCurCP)
	{
		if((*itCurCP)->Equals(sID))
		{
			return (*itCurCP);
		}
	}
	return NULL;
}

void CEventsCenter::AddEventHandler(CEventHandler* pEventHandler)
{
	if(pEventHandler == 0)
		return;
	m_sEventHandlerList.push_back(pEventHandler);

	for(int i=0;i<EVENT_LAST;++i)
	{
		if(pEventHandler->IsEvent(i))
		{
			m_events[i].connect(boost::bind(&CEventHandler::OnEvent, 
				pEventHandler, _1, _2));
		}
	}
}

void CEventsCenter::RegisterEvent(DWORD nEventType, const string& sID, const string& sScript)
{
	CEventHandler* pEvent = GetEventHandlerByID(sID);
	if(pEvent==0)
	{
		AddEventHandler(new CEventHandler(nEventType, sID,sScript));
	}
	else
		pEvent->SetScript(sScript);
	InvalidateEventCounts();
}

void CEventsCenter::RegisterEvent(const string& sID, const string& sScript)
{
	CEventHandler* pEvent = GetEventHandlerByID(sID);
	if(pEvent==0)
	{
		AddEventHandler(new CEventHandler(sID,sScript));
	}
	else
		pEvent->SetScript(sScript);
	InvalidateEventCounts();
}

void CEventsCenter::UnregisterEvent(const string& sID)
{
	EventHandler_List_t::iterator itCurCP;

	for( itCurCP = m_sEventHandlerList.begin(); itCurCP != m_sEventHandlerList.end(); )
	{
		if((*itCurCP)->Equals(sID))
		{
			delete (*itCurCP);
			itCurCP = m_sEventHandlerList.erase(itCurCP);
		}
		else
			++ itCurCP;
	}
	InvalidateEventCounts();
}

int CEventsCenter::FireEvent(const IEvent& e)
{
	int nType = e.GetEventType();
	if(nType>=0 && nType<= EVENT_LAST)
	{
		string sScriptCode = e.ToScriptCode();
		m_events[nType](&e, sScriptCode);
		return S_OK;
	}
	else
	{
		return E_FAIL;
	}
}

void CEventsCenter::UnregisterAllEvent()
{
	EventHandler_List_t::iterator itCurCP, itEnd = m_sEventHandlerList.end();
	for( itCurCP = m_sEventHandlerList.begin(); itCurCP != itEnd; ++itCurCP)
	{
		delete (*itCurCP);
	}
	m_sEventHandlerList.clear();
}

void CEventsCenter::InvalidateEventCounts()
{
	for (int i=0;i<EVENT_LAST;++i)
	{
		m_eventCounts[i] = 0xffff;
	}
}

bool CEventsCenter::PostEvent(const Event& e, bool bUnique)
{
	ParaEngine::Lock lock_(m_mutex);
	if(bUnique && e.GetEventID()>=0)
	{
		// find if there is already an event with the same id,
		EventHandler_Pool_t::const_iterator itCurCP1, itEndCP1 = m_unhandledEventPool.end();
		for( itCurCP1 = m_unhandledEventPool.begin(); itCurCP1 != itEndCP1; ++ itCurCP1)
		{
			if ((*itCurCP1).GetEventID() == e.GetEventID())
			{
				// there is already an event with the same id, so return.
				return false;
			}
		}
	}

	if((int)m_unhandledEventPool.size()<m_nMaxPoolSize)
	{
		m_unhandledEventPool.push_back(e);
		return true;
	}
	else
		return false;
}

void CEventsCenter::FireAllUnhandledEvents()
{
	ParaEngine::Lock lock_(m_mutex);

	EventHandler_Pool_t::iterator itCurCP1, itEndCP1 = m_unhandledEventPool.end();
	for( itCurCP1 = m_unhandledEventPool.begin(); itCurCP1 != itEndCP1; ++ itCurCP1)
	{
		Event& e = (*itCurCP1);
		int nType = e.GetEventType();
		if(nType>=0 && nType<= EVENT_LAST)
		{
			string sScriptCode = e.ToScriptCode();

			// NOTE: fix me: There is potential dead lock situation, when the handler invokes another PostEvent(), 
			// however, since all handlers are asynchronous, this will not happen at the moment. 
			m_events[nType](&e, sScriptCode);
		}
	}
	m_unhandledEventPool.clear();
}
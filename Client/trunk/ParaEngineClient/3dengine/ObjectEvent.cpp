//-----------------------------------------------------------------------------
// Class:	ObjectEvent
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine Tech Studio
// Date:	2004.3.6
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ObjectEvent.h"

using namespace ParaEngine;

ObjectEvent::ObjectEvent()
{
	m_sEvent.clear();
	m_nEvent = 0;
}


ParaEngine::ObjectEvent::ObjectEvent(const char* str, int nEvent/*=0*/)
{
	SetEvent(str);
	m_nEvent = nEvent;
}

int ParaEngine::ObjectEvent::ParseEvent(const std::string& sEvent)
{
	return 0;
}

bool ParaEngine::ObjectEvent::operator==(const ObjectEvent& r) const
{
	return (m_nEvent == r.m_nEvent && m_sEvent == r.m_sEvent);
}

ObjectEvent::~ObjectEvent()
{

}

const std::string& ObjectEvent::GetEventString() const
{
	//if(! m_sEvent.empty())
	return m_sEvent;
}


void ObjectEvent::SetEvent(const char* str)
{
	m_sEvent.assign(str);
	m_nEvent = 0;
}

void ObjectEvent::SetEvent(int nEvent)
{
	m_sEvent.clear();
	m_nEvent = nEvent;
}



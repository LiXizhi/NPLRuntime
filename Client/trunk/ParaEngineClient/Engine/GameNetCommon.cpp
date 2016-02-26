//-----------------------------------------------------------------------------
// Class:	Game Network Common classes
// Authors:	Li, Xizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2007.3
//-----------------------------------------------------------------------------
#include "ParaEngine.h"

#include "GameNetCommon.h"

#include "memdebug.h"

using namespace ParaEngine;

CNormalUpdatePacket::CNormalUpdatePacket( void )
{

}

CNormalUpdatePacket::~CNormalUpdatePacket( void )
{

}

//////////////////////////////////////////////////////////////////////////
//
// ICharacterTrackerContainer
//
//////////////////////////////////////////////////////////////////////////

ICharacterTrackerContainer::ICharacterTrackerContainer()
{
	m_nTimeLag = 0;
	m_nLastTrackerGCTime = 0;
	ResetCharTrackers();
}

ICharacterTrackerContainer::~ICharacterTrackerContainer()
{

}

CCharacterTracker* ICharacterTrackerContainer::GetCharTracker( const char* sName/*=NULL*/ )
{
	if(sName == NULL)
	{
		return &m_charTracker;
	}
	else
	{

		map<string, CCharacterTracker>::iterator iter =  m_perceptionTracker.find(sName);
		if(iter!=m_perceptionTracker.end())
		{
			return &(iter->second);
		}
		else
			return NULL;
	}
}

CCharacterTracker* ICharacterTrackerContainer::CreateCharTracker( const char* sName)
{
	if(sName == 0)
	{
		// if this character.
		m_charTracker.Reset();
		return &m_charTracker;
	}

	map<string, CCharacterTracker>::iterator iter =  m_perceptionTracker.find(sName);
	if(iter!=m_perceptionTracker.end())
	{
		// reset old one if already exist
		(iter->second).Reset();
		return &(iter->second);
	}
	else
	{
		// create a new one
		m_perceptionTracker[sName] = CCharacterTracker();
		return &(m_perceptionTracker[sName]);
	}
}

bool ParaEngine::ICharacterTrackerContainer::RemoveCharTracker( const char* sName )
{
	if(sName == 0)
	{
		// if this character.
		m_charTracker.Reset();
		return true;
	}
	else
	{
		map<string, CCharacterTracker>::iterator iter =  m_perceptionTracker.find(sName);
		if(iter!=m_perceptionTracker.end())
		{
			m_perceptionTracker.erase(iter);
			return true;
		}
		else
			return false;
	}
}

int ICharacterTrackerContainer::RemoveCharTracker( int nBeforePerceivedTime )
{
	if(nBeforePerceivedTime<1)
		nBeforePerceivedTime = 1;

	int nCount = 0;
	if(m_charTracker.GetLastReceiveTime()<=nBeforePerceivedTime)
	{
		m_charTracker.Reset();
		++nCount;
	}
	map<string, CCharacterTracker>::iterator itCur, itEnd =  m_perceptionTracker.end();
	for (itCur = m_perceptionTracker.begin();itCur!=itEnd;++itCur)
	{
		if(itCur->second.GetLastReceiveTime()<=nBeforePerceivedTime)
		{
			itCur->second.Reset();
			++nCount;
		}
	}
	return nCount;
}

void ICharacterTrackerContainer::ResetCharTrackers()
{
	m_charTracker.Reset();
	m_perceptionTracker.clear();
}
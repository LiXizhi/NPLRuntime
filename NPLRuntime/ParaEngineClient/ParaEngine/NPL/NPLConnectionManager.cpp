//-----------------------------------------------------------------------------
// Class:	NPLConnectionManager
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2009.5.22
// Desc:  
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include <algorithm>
#include <list>
#include <boost/bind.hpp>
#include "NPLConnectionManager.h"

NPL::CNPLConnectionManager::CNPLConnectionManager()
{

}

NPL::CNPLConnectionManager::~CNPLConnectionManager()
{

}

void NPL::CNPLConnectionManager::start( NPLConnection_ptr c )
{
	{
		ParaEngine::Lock lock_(m_mutex);
		m_connections.insert(c);
	}
	c->start();
}

void NPL::CNPLConnectionManager::stop( NPLConnection_ptr c, int nReason)
{
	{
		ParaEngine::Lock lock_(m_mutex);
		m_connections.erase(c);
	}
	c->stop(false, nReason);
}

void NPL::CNPLConnectionManager::stop_all()
{
	/** the following code locks less time. 
	NPLConnection_ptr conn;
	NPLConnectionPool_Type::iterator iter;
	while(true)
	{
		{
			ParaEngine::Lock lock_(m_mutex);
			NPLConnectionPool_Type::iterator iter = m_connections.begin();
			if(iter!=m_connections.end())
			{
				conn = *iter;
				m_connections.erase(iter);
			}
			else   
				break;
		}
		conn->stop(false);
	}*/

	ParaEngine::Lock lock_(m_mutex);
	std::for_each(m_connections.begin(), m_connections.end(),
		boost::bind(&CNPLConnection::stop, _1, false, 0));
	m_connections.clear();
}

int NPL::CNPLConnectionManager::CheckIdleTimeout()
{
	std::list<NPLConnection_ptr> dead_connection_pool;

	int nCount = 0;
	{
		ParaEngine::Lock lock_(m_mutex);
		unsigned int nCurTime = GetTickCount();

		NPLConnectionPool_Type::iterator itCur, itEnd = m_connections.end();
		for (itCur = m_connections.begin(); itCur!=itEnd; ++itCur)
		{
			int nRes = (*itCur)->CheckIdleTimeout(nCurTime);
			if(nRes != 1)
			{
				 ++ nCount;
				if(nRes == 0)
				{
					// add to remove queue;
					dead_connection_pool.push_back((*itCur));
				}
			}
		}
	}
	if(!dead_connection_pool.empty())
	{
		std::list<NPLConnection_ptr>::iterator itCur, itEnd = dead_connection_pool.end();
		for (itCur = dead_connection_pool.begin(); itCur!=itEnd; ++itCur)
		{
			stop(*itCur);
		}
		dead_connection_pool.clear();
	}

	return nCount;
}

int NPL::CNPLConnectionManager::ForEachConnection( NPLConnectionCallBack* pCallback )
{
	ParaEngine::Lock lock_(m_mutex);

	int nCount = 0;
	NPLConnectionPool_Type::iterator itCur, itEnd = m_connections.end();
	for (itCur = m_connections.begin(); itCur!=itEnd; ++itCur)
	{
		++ nCount;
		if(pCallback->DoCallBack(*itCur) != 0)
			break;
	}
	return nCount;
}

void NPL::CNPLConnectionManager::add( NPLConnection_ptr c )
{
	ParaEngine::Lock lock_(m_mutex);
	m_connections.insert(c);
}

int NPL::CNPLConnectionManager::get_connection_count()
{
	ParaEngine::Lock lock_(m_mutex);
	return (int)(m_connections.size());
}
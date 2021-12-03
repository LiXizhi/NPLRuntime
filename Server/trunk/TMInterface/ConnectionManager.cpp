//-----------------------------------------------------------------------------
// Class:	NPLConnectionManager
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2009.5.22
// Desc:  
//-----------------------------------------------------------------------------
#include "TMInterface.h"
#include <algorithm>
#include <boost/bind.hpp>

#include "ConnectionManager.h"

#define MAX_WORKING_SOCKET_COUNT 2000


CConnectionManager::CConnectionManager()
{

}

CConnectionManager::~CConnectionManager()
{

}

CConnectionManager& CConnectionManager::GetSingleton()
{
	static CConnectionManager g_singleton;
	return g_singleton;
}

void CConnectionManager::start( connection_ptr c, const std::string& server,const std::string& port,const char * sendbuf,int iOutLength,int iProxyFlag,const char* sMsg, int iMsgLength)
{
	{
		boost::mutex::scoped_lock lock_(m_mutex);
		

		int nSize = (int)(m_connections.size());
		if(nSize > MAX_WORKING_SOCKET_COUNT)
		{
			fprintf(TMService::Instance().fp,"%s|working socket count=%d\n",TMService::Instance().GetLogFormatTime(),nSize);
			return ;
		}
		if(nSize > 10)
		{
			fprintf(TMService::Instance().fp,"%s|new TM connection started| current socket count=%d\n",TMService::Instance().GetLogFormatTime(),nSize);
		}
		// add new connection. 
		m_connections.insert(c);
	}
	c->start(server,port,sendbuf,iOutLength,iProxyFlag,sMsg, iMsgLength);
}

void CConnectionManager::stop( connection_ptr c )
{
	{
		boost::mutex::scoped_lock lock_(m_mutex);
		m_connections.erase(c);
	}
	c->stop();
}

void CConnectionManager::stop_all()
{
	/** the following code locks less time. 
	connection_ptr conn;
	ConnectionPool_Type::iterator iter;
	while(true)
	{
		{
			boost::mutex::scoped_lock lock_(m_mutex);
			ConnectionPool_Type::iterator iter = m_connections.begin();
			if(iter!=m_connections.end())
			{
				conn = *iter;
				m_connections.erase(iter);
			}
			else   
				break;
		}
		conn->stop();
	}*/

	boost::mutex::scoped_lock lock_(m_mutex);
	std::for_each(m_connections.begin(), m_connections.end(),
		boost::bind(&connection::stop, _1));
	m_connections.clear();
}

void CConnectionManager::add( connection_ptr c )
{
	boost::mutex::scoped_lock lock_(m_mutex);
	m_connections.insert(c);
}
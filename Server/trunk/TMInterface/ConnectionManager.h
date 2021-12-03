#pragma once
#include <set>
#include <boost/noncopyable.hpp>
#include "connection.hpp"


/**
* All incoming and outgoing connection sessions. 
* all functions are thread-safe.
*/
class CConnectionManager : private boost::noncopyable
{
public:
	CConnectionManager();
	~CConnectionManager();

	static CConnectionManager& GetSingleton();
	
	/// Add the specified connection to the manager and start it.
	void start(connection_ptr c, const std::string& server,const std::string& port,const char * sendbuf,int iOutLength,int iProxyFlag,const char* sMsg, int iMsgLength);

	/* Add the specified connection to the manager without starting it.  
	* the connection might have been started already or it will is connecting and starting by itself. 
	*/
	void add(connection_ptr c);

	/// Stop the specified connection.
	void stop(connection_ptr c);
	
	/// Stop all connections.
	void stop_all();

private:
	typedef std::set<connection_ptr, Connection_PtrOps> ConnectionPool_Type;

	/// The managed connections. It only keeps established connections, but it does not mean that the connection here is authenticated. 
	ConnectionPool_Type m_connections;

	boost::mutex m_mutex;
};

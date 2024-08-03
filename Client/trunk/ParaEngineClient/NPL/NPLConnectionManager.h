#pragma once
#include "util/mutex.h"
#include <set>
#include <boost/core/noncopyable.hpp>
#include "NPLConnection.h"

namespace NPL
{
	/**
	* All incoming and outgoing connection sessions. 
	* all functions are thread-safe.
	*/
	class CNPLConnectionManager : private boost::noncopyable
	{
	public:
		CNPLConnectionManager();
		~CNPLConnectionManager();
		
		/// Add the specified connection to the manager and start it.
		void start(NPLConnection_ptr c);

		/* Add the specified connection to the manager without starting it.  
		* the connection might have been started already or it will is connecting and starting by itself. 
		*/
		void add(NPLConnection_ptr c);

		/** Stop the specified connection.
		* @param nReason: 0 is normal disconnect. 1 is another user with same nid is authenticated. The server should 
		* send a message to tell the client about this. 
		*/
		void stop(NPLConnection_ptr c, int nReason = 0);
		
		/// Stop all connections.
		void stop_all();

		/** get the total number of connected connections. */
		int get_connection_count();

		/** used for various callback. */
		struct NPLConnectionCallBack
		{
			virtual int DoCallBack(const NPLConnection_ptr& c) {return 0;};
		};

		/**
		* it will iterate over all functions, and call NPLConnectionCallBack->DoCallBack(). that function should return 0 to continue or 1 to break.
		* @return the total number of connections
		*/
		int ForEachConnection(NPLConnectionCallBack* pCallback);

		/** for all active connections, check if any of them should be timed out. If so, close it or send keep alive message. 
		* This function is called by the NPLNetServer from an io service timer that periodically does the checking. 
		* @return the number of timed out connections.
		*/
		int CheckIdleTimeout();
	private:
		typedef std::set<NPLConnection_ptr, NPLConnection_PtrOps> NPLConnectionPool_Type;
		
		/// The managed connections. It only keeps established connections, but it does not mean that the connection here is authenticated. 
		NPLConnectionPool_Type m_connections;

		ParaEngine::mutex m_mutex;
	};
}
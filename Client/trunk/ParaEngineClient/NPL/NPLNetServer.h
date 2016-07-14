#pragma once
#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/asio/steady_timer.hpp>
#include "NPLConnectionManager.h"
#include "NPLDispatcher.h"

namespace NPL
{
	/**
	* Current NPL configuration settings. 
	* it just load from NPL XML config file. 
	*/
	class CNPLNetServer : private boost::noncopyable
	{
	public:
		CNPLNetServer();
		~CNPLNetServer();

		/**
		* start the server's io_service loop. This function returns immediately. it will spawn the accept and dispatcher thread.  
		* @param server: default to "127.0.0.1"
		* @param port: default to "60001"
		*/
		void start(const char* server=NULL, const char* port=NULL);

		/// Stop the server.
		void stop();

		/** clean up all resources*/
		void Cleanup();

		/**
		* NPLDispatcher serves as an interface between the low level socket interface and NPL message queues.  
		*/
		CNPLDispatcher& GetDispatcher(){return m_msg_dispatcher;};

		/** get the connection object in this server 
		*/
		CNPLConnectionManager& GetConnectionManager(){return m_connection_manager;};

		/**
		* Create a new connection with a remote server and immediately connect and start the connection. 
		* [Thread Safe]
		* @param pAddress: the target NPL runtime address to connect to. 
		* @return The function returns immediately, so the returned connection may not have been connected or authenticated yet.
		*/
		NPLConnection_ptr CreateConnection(NPLRuntimeAddress_ptr pAddress);

		/** System level Enable/disable SO_KEEPALIVE. 
		* one needs set following values in linux procfs or windows registry in order to work as expected. 
		* - tcp_keepalive_intvl (integer; default: 75) 
		* 	The number of seconds between TCP keep-alive probes. 
		* - tcp_keepalive_probes (integer; default: 9) 
		* 	The maximum number of TCP keep-alive probes to send before giving up and killing the connection if no response is obtained from the other end. 
		* - tcp_keepalive_time (integer; default: 7200) 
		* 	The number of seconds a connection needs to be idle before TCP begins sending out keep-alive probes. Keep-alives are only sent when the SO_KEEPALIVE socket option is enabled. The default value is 7200 seconds (2 hours). An idle connection is terminated after approximately an additional 11 minutes (9 probes an interval of 75 seconds apart) when keep-alive is enabled. 
		* 	Note that underlying connection tracking mechanisms and application timeouts may be much shorter. 
		* Use the default system level TCP keep alive setting for this socket. 
		* Please see TCP keep alive for more information. It can be used to solve the "half-open connection".
		* it is arguable whether to use protocol level keep alive or implement it in the application level. 
		* @param bEnable: true to enable. 
		*/
		void SetTCPKeepAlive(bool bEnable);

		/** whether SO_KEEPALIVE is enabled. 
		* @return bEnable: true to enable.
		*/
		bool IsTCPKeepAliveEnabled();

		/** enable application level keep alive. we will use a global idle timer to detect if a connection has been inactive for GetIdleTimeoutPeriod(),
		* if so, we may send the keep alive message. 
		* @param bEnable: enable keep alive will automatically enable EnableIdleTimeout()
		*/
		void SetKeepAlive(bool bEnable);
		bool IsKeepAliveEnabled();

		/** Enable idle timeout. This is the application level timeout setting. 
		* We will create a global timer which examines all send/receive time of all open connections, if a
		* connection is inactive (idle for GetIdleTimeoutPeriod()) we will 
		*	- if IsKeepAliveEnabled() is false, actively close the connection. This is the method used by HTTP, which is the only solution to detect broken connection without sending additional keep alive message. 
		*   - if IsKeepAliveEnabled() is true, send an empty message to the other end (keep alive messages) to more accurately detect dead connections (see SetKeepAlive). 
		*/
		void EnableIdleTimeout(bool bEnable);
		bool IsIdleTimeoutEnabled();

		/** how many milliseconds of inactivity to assume this connection should be timed out. if 0 it is never timed out. */
		void SetIdleTimeoutPeriod(int nMilliseconds);
		int GetIdleTimeoutPeriod();

		/** whether the first line of the NPL protocol is in ansi code page. 
		because NPL protocol is compatible with HTTP protocol in ansi mode, some interception web cache servers may cache request even the port number is not 80, 
		so client side applications are encouraged to disable ansi mode. */
		void EnableAnsiMode(bool bEnable);
		bool IsAnsiMode();

		/** queue size of the acceptor's queue. */
		int GetMaxPendingConnections() const;
		void SetMaxPendingConnections(int val);
	public:
		/** get extern IP address of this computer. */
		std::string GetExternalIP();

	private:
		/// handle resolve the current server address. 
		void handle_resolve_local(const boost::system::error_code& err, boost::asio::ip::tcp::resolver::iterator endpoint_iterator);

		/// Handle completion of an asynchronous accept operation.
		void handle_accept(const boost::system::error_code& e);

		/// Handle a request to stop the server.
		void handle_stop();

		/** Handle idle timer timeout.*/
		void handle_idle_timeout(const boost::system::error_code& err);

		/// The io_service for dispatching (send/receive) messages from TCP stack to NPL runtime states' message queues.
		boost::asio::io_service m_io_service_dispatcher;

		/** Thread used for running the m_io_service_dispatcher 's run loop for dispatching messages for all NPL Jabber Clients */
		boost::scoped_ptr<boost::thread> m_dispatcherThread;

		/// for address resolving.
		boost::asio::ip::tcp::resolver m_resolver;

		/// Acceptor used to listen for incoming connections.
		boost::asio::ip::tcp::acceptor m_acceptor;

		/// The connection manager which owns all live connections.
		CNPLConnectionManager m_connection_manager;

		/// The next connection to be accepted.
		NPLConnection_ptr m_new_connection;

		/// server address, default to "127.0.0.1"
		string m_strServer;
		/// server port. default to 60001
		string m_strPort;

		/** whether SO_KEEPALIVE is enabled.*/
		bool m_bTCPKeepAlive;

		/** whether application level keep alive is enabled.*/
		bool m_bKeepAlive;

		/** whether idle timeout is enabled. */
		bool m_bEnableIdleTimeout;

		/** how many milliseconds to assume time out, default to 2 mins. */
		int m_nIdleTimeoutMS;

		/** queue size of the acceptor's queue. */
		int m_nMaxPendingConnections;

		/** a slowly ticked timer which checks if any connection should be timed out. */
		typedef boost::asio::basic_waitable_timer<boost::chrono::steady_clock> timer_type;
		timer_type m_idle_timer;


		/** Work for the private m_io_service_dispatcher to perform. If we do not give the
		io_service some work to do then the io_service::run() function will exit immediately.*/
		boost::scoped_ptr<boost::asio::io_service::work> m_work_lifetime;

		/**
		* this class serves as an interface between the low level socket interface and NPL message queues.  
		*/
		CNPLDispatcher m_msg_dispatcher;
	};
}
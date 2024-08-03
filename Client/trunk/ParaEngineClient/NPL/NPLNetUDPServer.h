#pragma once
#include <boost/thread.hpp>
#include <boost/asio.hpp>
#include <boost/core/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/asio/steady_timer.hpp>
#include "NPLUDPRouteManager.h"
#include "NPLUDPDispatcher.h"

namespace NPL
{
	class CNPLNetUDPServer : private boost::noncopyable
	{
	public:
		/// @def default NPL UDP server port
		static const unsigned short NPL_DEFAULT_UDP_PORT = 8099;
		/** how many milliseconds to assume connection time out, default to 2 mins. */
		static const unsigned int DEFAULT_IDLE_TIMEOUT_MS = 120000;
		/** @def the number of milliseconds that checks all connections in the system about timeout. */
		static const unsigned int IDLE_TIMEOUT_TIMER_INTERVAL = 120000;
		// 
		static const size_t RECEIVE_BUFF_SIZE = 4096 * 2;
		
	public:
		CNPLNetUDPServer();
		~CNPLNetUDPServer();

		/**
		* start the server's io_service loop. This function returns immediately.
		* @param server: default to "127.0.0.1"
		* @param port: default to "8809"
		*/
		void start(const char* server = nullptr, unsigned short port = NPL_DEFAULT_UDP_PORT);
		///
		///void startAsClient();

		/// Stop the server.
		void stop();

		/** clean up all resources*/
		void Cleanup();


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


		/** get the host port of this NPL runtime */
		unsigned short GetHostPort();
		/** get the host IP of this NPL runtime */
		const std::string& GetHostIP();
		//
		bool IsServerStarted();

		///
		void SendTo(const char* buff, size_t size, const std::string& remoteIp, unsigned short remotePort);
		void SendTo(const char* buff, size_t size, const boost::asio::ip::udp::endpoint& ep);
		void SendTo(const char* buff, size_t size, NPLUDPAddress_ptr pAddress);
		void SendTo(const char* buff, size_t size, NPLUDPRoute_ptr pRoute);

		///
		boost::asio::io_context& GetIoService();

		///
		CNPLUDPDispatcher& GetDispatcher() { return m_msg_dispatcher; };

		/** get the route object in this server
		*/
		CNPLUDPRouteManager& GetRouteManager() { return m_route_manager; };

		//
		NPLUDPRoute_ptr CreateRoute(NPLUDPAddress_ptr pAddress);


		/* ping a host, if host is alive return delay time,  otherwise return -1

		*/
		static int Ping(const char* host, const char* port, unsigned int waitTime = 1000);

	private:
		/** Handle idle timer timeout.*/
		void handle_idle_timeout(const boost::system::error_code& err);
		//
		void handle_receive(const boost::system::error_code& error, std::size_t bytes_transferred);
		// 
		void handle_send(const boost::system::error_code& error, size_t bytes_transferred, const char* buff, size_t buff_size);
		/// Handle a request to stop the server.
		void handle_stop();

		/// The route manager which owns all live routes.
		CNPLUDPRouteManager m_route_manager;

		/// The io_service for dispatching (send)to/receive_from) messages from udp stack to NPL runtime states' message queues.
		boost::asio::io_context m_io_service_dispatcher;
		
		///
		boost::asio::ip::udp::socket m_udp;

		/** Thread used for running the m_io_service_dispatcher 's run loop for dispatching messages for all NPL Jabber Clients */
		boost::scoped_ptr<boost::thread> m_dispatcherThread;

		///
		boost::asio::ip::udp::endpoint m_remote_endpoint;

		/// server address, default to "127.0.0.1"
		std::string m_strServer;
		/// server port. default to 60001
		unsigned short m_nPort;


		/** a slowly ticked timer which checks if any connection should be timed out. */
		typedef boost::asio::basic_waitable_timer<boost::chrono::steady_clock> timer_type;
		timer_type m_idle_timer;

		/** Work for the private m_io_service_dispatcher to perform. If we do not give the
		io_service some work to do then the io_service::run() function will exit immediately.*/
		boost::scoped_ptr<boost::asio::io_service::work> m_work_lifetime;

		/** how many milliseconds to assume time out, default to 2 mins. */
		int m_nIdleTimeoutMS;

		/** whether idle timeout is enabled. */
		bool m_bEnableIdleTimeout;

		/** whether we are listening to a port.*/
		bool m_bIsServerStarted;


		//
		boost::array<char, RECEIVE_BUFF_SIZE> m_receive_buffer;

		/**
		* this class serves as an interface between the low level socket interface and NPL message queues.
		*/
		CNPLUDPDispatcher m_msg_dispatcher;
	};

} // namespace NPL
//-----------------------------------------------------------------------------
// Class:	NPLNetServer
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2009.5.22
// Desc:  
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include <boost/bind.hpp>
#include "NPLNetServer.h"

/// @def default NPL server IP or domain name
#define NPL_DEFAULT_SERVER		"127.0.0.1"

/// @def default NPL server port
#define NPL_DEFAULT_PORT		"60001"

/// @def default NPL server version
#define NPL_SERVER_VERSION		"0.1"

/** max pending connections for acceptors. default setting under linux is 128. we will usually make it larger for slow end-user connections. */
#define		DEFAULT_MAX_PENDING_CONNECTIONS		1000

/** how many milliseconds to assume connection time out, default to 2 mins. */
#define		DEFAULT_IDLE_TIMEOUT_MS		120000

/** @def the number of milliseconds that checks all connections in the system about timeout. */
#define IDLE_TIMEOUT_TIMER_INTERVAL 2000

NPL::CNPLNetServer::CNPLNetServer()
	: m_io_service_dispatcher(),
	m_acceptor(m_io_service_dispatcher),
	m_resolver(m_io_service_dispatcher),
	m_idle_timer(m_io_service_dispatcher),
	m_connection_manager(),
	m_msg_dispatcher(this), // TODO: this gives a warning. find a better way to pass this pointer.
	m_strServer(NPL_DEFAULT_SERVER),
	m_strPort(NPL_DEFAULT_PORT),
	m_nMaxPendingConnections(DEFAULT_MAX_PENDING_CONNECTIONS), m_bIsServerStarted(false),
	m_bTCPKeepAlive(false), m_bKeepAlive(false), m_bEnableIdleTimeout(true), m_nIdleTimeoutMS(DEFAULT_IDLE_TIMEOUT_MS), m_bNoDelay(false)
{
}

NPL::CNPLNetServer::~CNPLNetServer()
{
	stop();
}

void NPL::CNPLNetServer::SetTCPKeepAlive(bool bEnable)
{
	m_bTCPKeepAlive = bEnable;
	if (m_acceptor.is_open())
	{
		// Implements the SOL_SOCKET/SO_KEEPALIVE socket option. 
		boost::asio::socket_base::keep_alive option(bEnable);
		m_acceptor.set_option(option);
	}
}

bool NPL::CNPLNetServer::IsTCPKeepAliveEnabled()
{
	return m_bTCPKeepAlive;
}


void NPL::CNPLNetServer::SetTCPNoDelay(bool bEnable)
{
	m_bNoDelay = bEnable;
	if (m_acceptor.is_open())
	{
		boost::asio::ip::tcp::no_delay  option(bEnable);
		m_acceptor.set_option(option);
	}
}

bool NPL::CNPLNetServer::IsTcpNoDelay()
{
	return m_bNoDelay;
}

void NPL::CNPLNetServer::SetKeepAlive(bool bEnable)
{
	m_bKeepAlive = bEnable;

	if (m_bKeepAlive)
	{
		EnableIdleTimeout(true);
	}
}

bool NPL::CNPLNetServer::IsKeepAliveEnabled()
{
	return m_bKeepAlive;
}

void NPL::CNPLNetServer::EnableIdleTimeout(bool bEnable)
{
	m_bEnableIdleTimeout = bEnable;
}

bool NPL::CNPLNetServer::IsIdleTimeoutEnabled()
{
	return m_bEnableIdleTimeout;
}

void NPL::CNPLNetServer::SetIdleTimeoutPeriod(int nMilliseconds)
{
	m_nIdleTimeoutMS = nMilliseconds;
	if (m_nIdleTimeoutMS <= 0) {
		OUTPUT_LOG("warning: IdleTimeoutPeriod is 0, we will disable idle time out, otherwise connection will be closed immediately.\n");
		m_bEnableIdleTimeout = false;
	}
}

int NPL::CNPLNetServer::GetIdleTimeoutPeriod()
{
	return m_nIdleTimeoutMS;
}

void NPL::CNPLNetServer::handle_idle_timeout(const boost::system::error_code& err)
{
	if (!err)
	{
		if (IsIdleTimeoutEnabled())
		{
			m_connection_manager.CheckIdleTimeout();
		}

		// continue with next activation. 
		m_idle_timer.expires_from_now(boost::chrono::milliseconds(IDLE_TIMEOUT_TIMER_INTERVAL)); // GetIdleTimeoutPeriod()
		m_idle_timer.async_wait(boost::bind(&NPL::CNPLNetServer::handle_idle_timeout, this, boost::asio::placeholders::error));
	}
}

int NPL::CNPLNetServer::Ping(const char* host, const char* port, unsigned int waitTime/* = 1000*/)
{
	if (!host || !port)
		return -1;

	auto time = GetTickCount();
	boost::asio::io_context io_context;
	boost::asio::ip::tcp::resolver resolver(io_context);

	try
	{
		boost::asio::ip::tcp::resolver::results_type endpoints = resolver.resolve(host, port);
		boost::asio::ip::tcp::socket socket(io_context);
		boost::asio::connect(socket, endpoints);
		//OUTPUT_LOG("connectd\n");
	}
	catch (std::exception& /*e*/)
	{
		//OUTPUT_LOG("connect error : %s\n", e.what());
		return -1;
	}

	auto ret = GetTickCount() - time;

	OUTPUT_LOG("time : %d\n", ret);

	return ret;

}

void NPL::CNPLNetServer::start(const char* server/*=NULL*/, const char* port/*=NULL*/)
{
	if (m_dispatcherThread.get() != 0)
	{
		// One can only start the server once, unless we are listening to a new port
		if (m_strPort == "0" && port != 0)
		{
			if (strcmp(port, "0") != 0)
			{
				m_strPort = port;
				OUTPUT_LOG("NPL server %s is listening on %s:%s\n", NPL_SERVER_VERSION, m_strServer.c_str(), m_strPort.c_str());
				boost::asio::ip::tcp::resolver::query query(m_strServer, m_strPort);
				m_resolver.async_resolve(query,
					boost::bind(&CNPLNetServer::handle_resolve_local, this,
						boost::asio::placeholders::error,
						boost::asio::placeholders::iterator));
			}
		}
		return;
	}

	if (server != 0)
		m_strServer = server;
	if (port != 0)
		m_strPort = port;

	// add localhost to trusted runtime addresses, so that the local runtime can connect to itself. 
	// generally, this is used for testing loopback performance. 
	// please note: "localhost", "local", "127.0.0.1" are three internal names for the loop back socket interface, however they also present three different socket connections to the loopback NPL server.
	NPLRuntimeAddress_ptr address(new NPLRuntimeAddress(m_strServer, m_strPort, "localhost"));
	m_msg_dispatcher.AddNPLRuntimeAddress(address);
	address.reset(new NPLRuntimeAddress(m_strServer, m_strPort, "local"));
	m_msg_dispatcher.AddNPLRuntimeAddress(address);
	address.reset(new NPLRuntimeAddress(m_strServer, m_strPort, "127.0.0.1"));
	m_msg_dispatcher.AddNPLRuntimeAddress(address);

	// if the port is "0", we will not listen for incoming connections. 
	if (m_strPort != "0")
	{
		OUTPUT_LOG("NPL server %s is listening on %s:%s\n", NPL_SERVER_VERSION, m_strServer.c_str(), m_strPort.c_str());

		boost::asio::ip::tcp::resolver::query query(m_strServer, m_strPort);
		m_resolver.async_resolve(query,
			boost::bind(&CNPLNetServer::handle_resolve_local, this,
				boost::asio::placeholders::error,
				boost::asio::placeholders::iterator));
	}

	OUTPUT_LOG("TCPKeepAlive: %s\n", IsTCPKeepAliveEnabled() ? "true" : "false");
	OUTPUT_LOG("AppKeepAlive: %s\n", IsKeepAliveEnabled() ? "true" : "false");
	OUTPUT_LOG("IdleTimeout: %s\n", IsIdleTimeoutEnabled() ? "true" : "false");
	OUTPUT_LOG("TCPNoDelay: %s\n", IsTcpNoDelay() ? "true" : "false");
	OUTPUT_LOG("IdleTimeoutPeriod: %d\n", GetIdleTimeoutPeriod());

	OUTPUT_LOG("UseCompression(incoming): %s\n", GetDispatcher().IsUseCompressionIncomingConnection() ? "true" : "false");
	OUTPUT_LOG("CompressionLevel: %d\n", GetDispatcher().GetCompressionLevel());
	OUTPUT_LOG("CompressionThreshold: %d\n", GetDispatcher().GetCompressionThreshold());

	m_idle_timer.expires_from_now(boost::chrono::milliseconds(GetIdleTimeoutPeriod()));
	m_idle_timer.async_wait(boost::bind(&NPL::CNPLNetServer::handle_idle_timeout, this, boost::asio::placeholders::error));

	m_work_lifetime.reset(new boost::asio::io_service::work(m_io_service_dispatcher));

	m_dispatcherThread.reset(new boost::thread(boost::bind(&boost::asio::io_service::run, &m_io_service_dispatcher)));
}

void NPL::CNPLNetServer::stop()
{
	if (m_dispatcherThread.get() != 0)
	{
		// cancel timer
		m_idle_timer.cancel();

		try
		{
			// cancel resolver
			m_resolver.cancel();

			// stop incomming connections
			if (m_acceptor.is_open())
				m_acceptor.cancel();
		}
		catch (const std::exception& e)
		{
			OUTPUT_LOG("CNPLNetServer error: %s\n", e.what());
		}

		// Post a call to the stop function so that server::stop() is safe to call from any thread.
		m_io_service_dispatcher.post(boost::bind(&CNPLNetServer::handle_stop, this));

		// stop the work on dispatcher. 
		m_work_lifetime.reset();

		while (m_acceptor.is_open())
		{
			SLEEP(10);
		}
		m_io_service_dispatcher.stop();
		m_dispatcherThread->join();
		m_dispatcherThread.reset();
		
		Cleanup();
		m_io_service_dispatcher.reset();
		m_new_connection.reset();
		m_bIsServerStarted = false;
	}
}


void NPL::CNPLNetServer::handle_resolve_local(const boost::system::error_code& err, boost::asio::ip::tcp::resolver::iterator endpoint_iterator)
{
	if (!err)
	{
		try
		{
			// Attempt a connection to the first endpoint in the list. Each endpoint
			// will be tried until we successfully establish a connection.
			boost::asio::ip::tcp::endpoint endpoint = *endpoint_iterator;
			m_acceptor.open(endpoint.protocol());
			m_acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));

			// Implements a custom socket option that determines whether or not an accept operation is permitted to fail with boost::asio::error::connection_aborted. By default the option is false. 
			boost::asio::socket_base::enable_connection_aborted option(true);
			m_acceptor.set_option(option);
			m_acceptor.bind(endpoint);

			if (IsTCPKeepAliveEnabled())
			{
				// Implements the SOL_SOCKET/SO_KEEPALIVE socket option. 
				boost::asio::socket_base::keep_alive option(true);
				m_acceptor.set_option(option);
			}

			if (IsTcpNoDelay())
			{
				// Implements the SOL_SOCKET/SO_KEEPALIVE socket option. 
				boost::asio::ip::tcp::no_delay option(true);
				m_acceptor.set_option(option);
			}

			// QUESTION: shall we set the maximum length of the queue of pending connections. 
			m_acceptor.listen(m_nMaxPendingConnections);
			m_bIsServerStarted = true;

			OUTPUT_LOG("NPL max pending incoming connections allowed is %d\n", m_nMaxPendingConnections);
			// m_acceptor.listen();
			// OUTPUT_LOG("NPL max pending incoming connections allowed is %d\n", m_acceptor.max_connections);
		}
		catch (std::exception& e)
		{
			OUTPUT_LOG1("warning: unable to accept NPL connection, because %s\n", e.what());
			return;
		}
		catch (...)
		{
			OUTPUT_LOG1("warning: unable to accept NPL connection, because unknown error\n");
			return;
		}
		if (!m_new_connection) {
			m_new_connection.reset(new CNPLConnection(m_io_service_dispatcher, m_connection_manager, m_msg_dispatcher));
			m_acceptor.async_accept(m_new_connection->socket(), boost::bind(&CNPLNetServer::handle_accept, this, boost::asio::placeholders::error));
		}
	}
	else
	{
		string sError = err.message();
		OUTPUT_LOG("warning: CNPLNetServer unable to resolve TCP end point %s \n", sError.c_str());
	}
}

void NPL::CNPLNetServer::handle_accept(const boost::system::error_code& err)
{
	if (!err)
	{
		if (m_new_connection)
		{
			m_new_connection->EnableIdleTimeout(IsIdleTimeoutEnabled());
			m_new_connection->SetIdleTimeoutPeriod(GetIdleTimeoutPeriod());
			m_new_connection->SetKeepAlive(IsKeepAliveEnabled());
			m_new_connection->SetNoDelay(IsTcpNoDelay());

			m_connection_manager.start(m_new_connection);
			m_new_connection.reset(new CNPLConnection(m_io_service_dispatcher, m_connection_manager, m_msg_dispatcher));
			m_acceptor.async_accept(m_new_connection->socket(),
				boost::bind(&CNPLNetServer::handle_accept, this,
					boost::asio::placeholders::error));
		}
	}
	else if (err == boost::asio::error::connection_aborted)
	{
		string sError = err.message();
		OUTPUT_LOG("warning: CNPLNetServer aborted incoming connection, because: %s\n", sError.c_str());
		OUTPUT_LOG("current connections %d\n", m_connection_manager.get_connection_count());

		// QUESTION: is m_new_connection.reset() really needed here?
		/*m_new_connection.reset(new CNPLConnection(m_io_service_dispatcher,m_connection_manager,m_msg_dispatcher));
		m_acceptor.async_accept(m_new_connection->socket(),
		boost::bind(&CNPLNetServer::handle_accept, this,
		boost::asio::placeholders::error)); */
	}
	else
	{
		string sError = err.message();
		OUTPUT_LOG("error: CNPLNetServer unable to accept incoming connection, because: %s\n", sError.c_str());
		OUTPUT_LOG("current connections %d\n", m_connection_manager.get_connection_count());

		// QUESTION: is m_new_connection.reset() really needed here?
		/*m_new_connection.reset(new CNPLConnection(m_io_service_dispatcher,m_connection_manager,m_msg_dispatcher));
		m_acceptor.async_accept(m_new_connection->socket(),
		boost::bind(&CNPLNetServer::handle_accept, this,
		boost::asio::placeholders::error)); */
	}
}

void NPL::CNPLNetServer::handle_stop()
{
	// The server is stopped by canceling all outstanding asynchronous
	// operations. Once all operations have finished the io_service::run() call
	// will exit.
	boost::system::error_code ec;
	// m_acceptor.cancel(ec);
	m_acceptor.close(ec);
	if (ec)
	{
		// An error occurred.
		OUTPUT_LOG1("warning: m_acceptor.close() failed, because %s\n", ec.message().c_str());
	}
	/*
	// In some rare computer (win 7), async_accept can not be closed in any way, the following code does not help.
	if (m_new_connection){
		m_new_connection->stop(false);
		m_new_connection.reset();
	}
	*/
	m_connection_manager.stop_all();
}

std::string NPL::CNPLNetServer::GetBroadcastAddressList()
{
	using boost::asio::ip::tcp;
	std::string firstIP;
	try
	{
		boost::asio::io_service io_service;

		tcp::resolver resolver(io_service);
		tcp::resolver::query query(boost::asio::ip::host_name(), "");
		tcp::resolver::iterator it = resolver.resolve(query);

		while (it != tcp::resolver::iterator())
		{
			boost::asio::ip::address addr = (it++)->endpoint().address();
			if (addr.is_v4())
			{
				auto bytes = addr.to_v4().to_bytes();

				if ((bytes[0] == 10)
					|| (bytes[0] == 172 && (bytes[1] >= 16 && bytes[1] <= 31))
					|| (bytes[0] == 192 && bytes[1] == 168))
				{
					{
						// this is temporary algorithm,  broadcast address = ip | (~mask)
						bytes[3] = 0xff;
					}

					addr = boost::asio::ip::make_address_v4(bytes);

					if (firstIP.empty())
						firstIP = addr.to_string();
					else
						firstIP += "," + addr.to_string();
				}
			}
		}
	}
	catch (...)
	{
		OUTPUT_LOG1("warning: failed getting external ip in CNPLNetServer::GetExternalIPList()\n");
	}
	return firstIP;
}

std::string NPL::CNPLNetServer::GetExternalIPList()
{
	using boost::asio::ip::tcp;
	std::string firstIP;
	try
	{
		boost::asio::io_service io_service;

		tcp::resolver resolver(io_service);
		tcp::resolver::query query(boost::asio::ip::host_name(), "");
		tcp::resolver::iterator it = resolver.resolve(query);

		while (it != tcp::resolver::iterator())
		{
			boost::asio::ip::address addr = (it++)->endpoint().address();
			if (addr.is_v4())
			{
				if (firstIP.empty())
					firstIP = addr.to_string();
				else
					firstIP += "," + addr.to_string();
			}
		}

	}
	catch (...)
	{
		OUTPUT_LOG1("warning: failed getting external ip in CNPLNetServer::GetExternalIPList()\n");
	}
	return firstIP;
}

std::string NPL::CNPLNetServer::GetExternalIP()
{
	using boost::asio::ip::tcp;
	std::string firstIP;
	try
	{
		boost::asio::io_service io_service;

		tcp::resolver resolver(io_service);
		tcp::resolver::query query(boost::asio::ip::host_name(), "");
		tcp::resolver::iterator it = resolver.resolve(query);

		while (it != tcp::resolver::iterator())
		{
			boost::asio::ip::address addr = (it++)->endpoint().address();
			if (firstIP.empty() && addr.is_v4())
			{
				firstIP = addr.to_string().c_str();
			}
			OUTPUT_LOG("ip %s\n", addr.to_string().c_str());
		}
	}
	catch (...)
	{
		OUTPUT_LOG1("warning: failed getting external ip in CNPLNetServer::GetExternalIP()\n");
	}
	return firstIP;
}

const std::string& NPL::CNPLNetServer::GetHostPort()
{
	return m_strPort;
}

const std::string& NPL::CNPLNetServer::GetHostIP()
{
	return m_strServer;
}

bool NPL::CNPLNetServer::IsServerStarted()
{
	return m_bIsServerStarted;
}

void NPL::CNPLNetServer::Cleanup()
{
	m_msg_dispatcher.Cleanup();
}

NPL::NPLConnection_ptr NPL::CNPLNetServer::CreateConnection(NPLRuntimeAddress_ptr pAddress)
{
	NPLConnection_ptr pConnection(new CNPLConnection(m_io_service_dispatcher, m_connection_manager, m_msg_dispatcher));
	pConnection->SetNPLRuntimeAddress(pAddress);
	m_connection_manager.add(pConnection);
	pConnection->connect();
	return pConnection;
}

bool NPL::CNPLNetServer::IsAnsiMode()
{
	return CNPLMsgOut_gen::g_enable_ansi_mode;
}

void NPL::CNPLNetServer::EnableAnsiMode(bool bEnable)
{
	CNPLMsgOut_gen::g_enable_ansi_mode = bEnable;
}

void NPL::CNPLNetServer::SetMaxPendingConnections(int val)
{
	m_nMaxPendingConnections = val;
}

int NPL::CNPLNetServer::GetMaxPendingConnections() const
{
	return m_nMaxPendingConnections;
}

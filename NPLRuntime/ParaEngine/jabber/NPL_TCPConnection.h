#pragma once

#include "ParaRingBuffer.h"

#include <boost/bind.hpp>
#include <boost/asio.hpp>
#include <boost/thread.hpp>

// this is the gloox client
#include <gloox.h>
#include <connectionbase.h>
#include <logsink.h>

namespace ParaEngine
{
	using namespace gloox;
	using boost::asio::ip::tcp;

	/**
	* boost::asio is used for NPL jabber client's tcp connections. all NPL_TCPConnection (s) share a single dispatcher io service(one background thread) for 
	* DNS resolve, socket connections, and all send and receive message dispatching f. 
	*   - when the send message queue is full, new messages will be silently dropped. 
	*   - when the receive queue is full, socket connection will be closed. 
	*/
	class CNPLTCPConnection : public ConnectionBase
	{
	public:
		typedef CParaRingBuffer<std::string> RingBuffer_Type;

		/**
		* Constructs a new ConnectionTCPBase object.
		* @param logInstance The log target. Obtain it from ClientBase::logInstance().
		* @param server A server to connect to.
		* @param port The port to connect to. The default of -1 means that XMPP SRV records
		* will be used to find out about the actual host:port.
		* @note To properly use this object, you have to set a ConnectionDataHandler using
		* registerConnectionDataHandler(). This is not necessary if this object is
		* part of a 'connection chain', e.g. with ConnectionHTTPProxy.
		*/
		CNPLTCPConnection( boost::asio::io_service& io_service, const LogSink& logInstance, const std::string& server, int port = -1 );

		/**
		* Constructs a new ConnectionTCPBase object.
		* @param cdh An ConnectionDataHandler-derived object that will handle incoming data.
		* @param logInstance The log target. Obtain it from ClientBase::logInstance().
		* @param server A server to connect to.
		* @param port The port to connect to. The default of -1 means that SRV records will be used
		* to find out about the actual host:port.
		*/
		CNPLTCPConnection( boost::asio::io_service& io_service, ConnectionDataHandler *cdh, const LogSink& logInstance,
			const std::string& server, int port = -1 );

		/**
		* Virtual destructor
		*/
		virtual ~CNPLTCPConnection();

		// reimplemented from ConnectionBase
		virtual bool send( const std::string& data );

		// reimplemented from ConnectionBase
		virtual ConnectionError receive();

		// reimplemented from ConnectionBase
		virtual void disconnect();

		// reimplemented from ConnectionBase
		virtual void cleanup();

		// reimplemented from ConnectionBase
		virtual void getStatistics( long int  &totalIn, long int &totalOut );


		// reimplemented from ConnectionBase
		virtual ConnectionError recv( int timeout = -1 );

		// reimplemented from ConnectionBase
		virtual ConnectionError connect();

		// reimplemented from ConnectionBase
		virtual ConnectionBase* newInstance() const;

		/** this function can be called from the io thread to notify disconnection in the main thread. */
		void NotifyDisconnect(ConnectionError error);

		/** whether we have notification messages to send. */
		bool HasNotification();

		/** handle all notifications.
		* @return: the number of notifications returned. */
		int HandleNotification();

		/** reset this tcp connection such as changing server and port. */
		void Reset( const std::string& server, int port );
	protected:
		///////////////////////
		// IO event handlers
		///////////////////////

		void handle_resolve(const boost::system::error_code& err, tcp::resolver::iterator endpoint_iterator);
		void handle_connect(const boost::system::error_code& error, tcp::resolver::iterator endpoint_iterator);
		void handle_read(const boost::system::error_code& error, std::size_t bytes_transferred);
		void handle_write(const boost::system::error_code& error);
		void handle_disconnect();

	protected:
		CNPLTCPConnection &operator=( const CNPLTCPConnection & );
		void init( const std::string& server, int port );
		
		const LogSink& m_logInstance;
		
		// the receive buffer
		char *m_buf;
		const int m_bufsize;
		long int m_totalBytesIn;
		long int m_totalBytesOut;
		std::string m_strPort;

		tcp::socket m_socket;
		tcp::resolver m_resolver;
		/** dispatcher IO service object for the connection socket. */
		boost::asio::io_service * m_pIOService;

		/** true to notify disconnection */
		bool m_bNotifyDisconnect;
		/** disconnection error message. */
		ConnectionError m_nDisconnectError;

		ParaEngine::mutex m_mutex;

		/** the output message queue. a queue that is filled by the main thread and send out by a worker thread. */
		RingBuffer_Type m_queueOutput;

		/** the input message queue. a queue that is filled by incoming messages (the receiving thread) and processed by main thread at regular interval. */
		RingBuffer_Type m_queueInput;
		
	};
}
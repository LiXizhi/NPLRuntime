//-----------------------------------------------------------------------------
// Class:	TCP connection 
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2009.5.6
// Desc: it replaces the implementation of ConnectionTCPClient:ConnectionTCPBase:ConnectionBase
// instead, CNPLTCPConnection:ConnectionBase is used. 
// I use boost::asio for socket. only one dispatcher thread (io_service) is created for all NPL_TCPConnections. 
// all handle_XXX functions are callled from the io_service::run() thread. other functions like connection, send, recv, disconnect are called from the main game thread. 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include <gloox.h>
#include "NPL_TCPConnection.h"

#include <logsink.h>
#include <prep.h>

/** default receive buffer size in bytes. this no longer needs to be large since we use asio. */
#define DEFAULT_REV_BUFFER_SIZE		1024

/** the default maximum output message queue size. this is usually set to very big, such as 102400. 
the send message function will fail (service not available), if the queue is full */
#define DEFAULT_OUTPUT_QUEUE_SIZE		102400

/** the default maximum input message queue size. this is usually set to very big, such as 102400. 
the recv message function will not be called, if the queue is near full. */
#define DEFAULT_INPUT_QUEUE_SIZE	102400

/** the maximum number of messages to read from the incoming queue for processing in a single frame rate. 
this is usually smaller than DEFAULT_INPUT_QUEUE_SIZE */
#define MAX_MESSAGE_PROCESSED_PER_FRAME		5000

using namespace ParaEngine;

CNPLTCPConnection::CNPLTCPConnection( boost::asio::io_service& io_service, const LogSink& logInstance, const std::string& server, int port /*= -1 */ )
: m_pIOService(&io_service), m_socket(io_service), m_resolver(io_service), ConnectionBase( 0 ),
	m_logInstance( logInstance ), m_buf( 0 ), m_totalBytesIn( 0 ),
	m_bNotifyDisconnect(false), m_nDisconnectError(ConnIoError),
	m_totalBytesOut( 0 ), m_bufsize( DEFAULT_REV_BUFFER_SIZE ), m_queueOutput(DEFAULT_OUTPUT_QUEUE_SIZE), m_queueInput(DEFAULT_INPUT_QUEUE_SIZE)
{
	init( server, port );
}

CNPLTCPConnection::CNPLTCPConnection( boost::asio::io_service& io_service, ConnectionDataHandler *cdh, const LogSink& logInstance, const std::string& server, int port /*= -1 */ )
: m_pIOService(&io_service), m_socket(io_service), m_resolver(io_service), ConnectionBase( cdh ),
m_logInstance( logInstance ), m_buf( 0 ), m_totalBytesIn( 0 ),
m_bNotifyDisconnect(false), m_nDisconnectError(ConnIoError),
m_totalBytesOut( 0 ), m_bufsize( DEFAULT_REV_BUFFER_SIZE ), m_queueOutput(DEFAULT_OUTPUT_QUEUE_SIZE), m_queueInput(DEFAULT_INPUT_QUEUE_SIZE)
{
	init( server, port );
}

void ParaEngine::CNPLTCPConnection::init( const std::string& server, int port )
{
	// FIXME check return value?
	prep::idna( server, m_server );
	m_port = port;

	if(m_buf == 0)
	{
		m_buf = (char*)calloc( m_bufsize + 1, sizeof( char ) );
	}
}

void ParaEngine::CNPLTCPConnection::Reset( const std::string& server, int port )
{
	init(server, port);
}

ParaEngine::CNPLTCPConnection::~CNPLTCPConnection()
{
	cleanup();
	free( m_buf );
	m_buf = 0;
}

void CNPLTCPConnection::NotifyDisconnect(ConnectionError error)
{
	m_bNotifyDisconnect = true;
	m_nDisconnectError = error;
}

ConnectionBase* CNPLTCPConnection::newInstance() const
{
	m_logInstance.log( LogLevelError, LogAreaClassConnectionTCPClient, m_server + ": CNPLTCPConnection::newInstance() should never be called. ");
	return new CNPLTCPConnection(*m_pIOService,  m_handler, m_logInstance, m_server, m_port );
}

void CNPLTCPConnection::cleanup()
{
	// disconnect is called when client is released, so I need not call it on clean up code any more. 
	// disconnect(); 
	m_totalBytesIn = 0;
	m_totalBytesOut = 0;
}

void CNPLTCPConnection::disconnect()
{
	// Post a call to the stop function so that server::disconnect() is safe to call
	// from any thread.
	m_pIOService->post(boost::bind(&CNPLTCPConnection::handle_disconnect, this));
}

void CNPLTCPConnection::handle_disconnect()
{
	// The connection is stopped by cancelling all outstanding asynchronous
	// operations. Once all operations have finished the io_service::run() call
	// will exit.
	Lock lock_(m_mutex);
	boost::system::error_code ec;
	m_socket.close(ec);
	if (ec)
	{
		// An error occurred.
		OUTPUT_LOG1("warning: m_socket.close failed in CNPLTCPConnection::handle_disconnect, because %s\n", ec.message().c_str());
	}
	m_state = StateDisconnected;

	// notify disconnection, 
	// TODO: this is not accurate, since we error message is always ConnIoError
	NotifyDisconnect(ConnIoError);
}


bool CNPLTCPConnection::HasNotification()
{
	return m_bNotifyDisconnect;
}

ConnectionError CNPLTCPConnection::recv( int timeout )
{
	if (m_handler)
	{
		// process as many as possible. 
		string data;
		for (int i=0;i<MAX_MESSAGE_PROCESSED_PER_FRAME && m_queueInput.try_pop(data);++i)
		{
			m_handler->handleReceivedData( this, data );
		}

		// handle any pending notifications in the main thread anyway. 
		HandleNotification();
	}
	return ConnNoError;
}

int CNPLTCPConnection::HandleNotification()
{
	int nCount = 0;
	if(m_bNotifyDisconnect)
	{
		m_bNotifyDisconnect = false;
		nCount++;
		m_handler->handleDisconnect( this, m_nDisconnectError);
	}
	return nCount;
}
ConnectionError CNPLTCPConnection::receive()
{
	// this function should not be called. 
	m_logInstance.log( LogLevelError, LogAreaClassConnectionTCPClient,
		m_server + ": asio is used. no need to receive this function" );
	return ConnNotConnected;
}


bool CNPLTCPConnection::send( const std::string& data )
{
	if( data.empty() || ( m_state != StateConnected ) )
	{
		return false;
	}

	string * pFront = NULL;
	RingBuffer_Type::BufferStatus bufStatus =  m_queueOutput.try_push_get_front(data, &pFront);

	if(bufStatus == RingBuffer_Type::BufferFirst)
	{
		assert(pFront!=NULL);
		// LXZ: very tricky code to ensure thread-safety to the buffer.
		// only start the sending task when the buffer is empty, otherwise we will wait for previous send task. 
		// i.e. inside handle_write handler. 
		boost::asio::async_write(m_socket,
			boost::asio::buffer(*pFront),
			boost::bind(&CNPLTCPConnection::handle_write, this,
			boost::asio::placeholders::error));
	}
	else if(bufStatus == RingBuffer_Type::BufferOverFlow)
	{
		// too many messages to send.
		return false;
	}
	m_totalBytesOut += data.length();
	return true;
}

void CNPLTCPConnection::handle_write(const boost::system::error_code& error)
{
	if (!error)
	{
		// send more from the buffer, until the output queue is empty
		string* data=NULL;
		if(m_queueOutput.try_next(&data) && data!=NULL)
		{
			boost::asio::async_write(m_socket,
				boost::asio::buffer(*data),
				boost::bind(&CNPLTCPConnection::handle_write, this,
				boost::asio::placeholders::error));
		}
	}
	else
	{
		m_logInstance.log( LogLevelError, LogAreaClassConnectionTCPClient, m_server + ": send error, reason:"+error.message());
		disconnect();
	}
}

void CNPLTCPConnection::getStatistics( long int  &totalIn, long int &totalOut )
{
	totalIn = m_totalBytesIn;
	totalOut = m_totalBytesOut;
}

ConnectionError CNPLTCPConnection::connect()
{
	if( !m_handler )
	{
		return ConnNotConnected;
	}

	if( m_state > StateDisconnected )
	{
		return ConnNoError;
	}

	m_state = StateConnecting;

	try
	{
		// default to port 5222, if none is provided.
		if(m_port == -1)
		{
			m_port = 5222;
		}
		char temp[100];
		snprintf(temp, 100, "%d", m_port);
		m_strPort = temp;

		tcp::resolver::query query(m_server, m_strPort);

		m_resolver.async_resolve(query,
			boost::bind(&CNPLTCPConnection::handle_resolve, this,
			boost::asio::placeholders::error,
			boost::asio::placeholders::iterator));
	}
	catch(...)
	{
		m_logInstance.log( LogLevelError, LogAreaClassConnectionTCPClient, m_server + ": can not resolve m_server" + m_server);

		disconnect();
		return ConnDnsError;
	}
	return ConnNoError;
}

void CNPLTCPConnection::handle_resolve(const boost::system::error_code& err,tcp::resolver::iterator endpoint_iterator)
{
	if (!err)
	{
		// Attempt a connection to the first endpoint in the list. Each endpoint
		// will be tried until we successfully establish a connection.
		tcp::endpoint endpoint = *endpoint_iterator;
		m_socket.async_connect(endpoint,
			boost::bind(&CNPLTCPConnection::handle_connect, this,
			boost::asio::placeholders::error, ++endpoint_iterator));
	}
	else
	{
		m_logInstance.log( LogLevelError, LogAreaClassConnectionTCPClient,
			m_server + ": unable to resolve TCP end point"+err.message() );
		disconnect();
	}
}

void CNPLTCPConnection::handle_connect(const boost::system::error_code& error,
					tcp::resolver::iterator endpoint_iterator)
{
	Lock lock_(m_mutex);

	if (!error)
	{
		m_state = StateConnected;
		m_handler->handleConnect( this );

		// Read some from the server.
		m_socket.async_read_some(boost::asio::buffer(m_buf, m_bufsize),
			boost::bind(&CNPLTCPConnection::handle_read, this,
			boost::asio::placeholders::error, 
			boost::asio::placeholders::bytes_transferred));
	}
	else if (endpoint_iterator != tcp::resolver::iterator())
	{
		// That endpoint didn't work, try the next one.
		boost::system::error_code ec;
		m_socket.close(ec);
		if (ec)
		{
			// An error occurred.
			OUTPUT_LOG1("warning: m_socket.close failed in CNPLTCPConnection::handle_connect, because %s\n", ec.message().c_str());
		}

		tcp::endpoint endpoint = *endpoint_iterator;
		m_socket.async_connect(endpoint,
			boost::bind(&CNPLTCPConnection::handle_connect, this,
			boost::asio::placeholders::error, ++endpoint_iterator));
	}
	else
	{
		// unable to connect to any TCP endpoints
		m_logInstance.log( LogLevelError, LogAreaClassConnectionTCPClient,
			m_server + ": unable to connect to TCP end point" );
		disconnect();
	}
}

void CNPLTCPConnection::handle_read(const boost::system::error_code& error, std::size_t bytes_transferred)
{
	if (!error)
	{
		if( m_handler && bytes_transferred>0)
		{
			m_totalBytesIn += (int) bytes_transferred;

			if(m_queueInput.try_push(std::string( m_buf, bytes_transferred)) == RingBuffer_Type::BufferOverFlow)
			{
				// we are receiving too many packets than the CPU can handle, we will stop receiving, and close
				m_logInstance.log( LogLevelError, LogAreaClassConnectionTCPClient,
					m_server + ": we are receiving too many packets than the CPU can handle. Needs to check and restart the node" );
				disconnect();
			}
		}
		
		// Read some from the server.
		m_socket.async_read_some(boost::asio::buffer(m_buf, m_bufsize),
			boost::bind(&CNPLTCPConnection::handle_read, this,
			boost::asio::placeholders::error, 
			boost::asio::placeholders::bytes_transferred));
	}
	else
	{
		disconnect();
	}
}

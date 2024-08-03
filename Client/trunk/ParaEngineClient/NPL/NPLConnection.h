#pragma once
#include "NPLMessage.h"
#include "NPLCommon.h"
#include "NPLMsgIn.h"
#include "NPLMsgOut.h"
#include "NPLMsgIn_parser.h"
#include "NPLMessageQueue.h"
#include "WebSocket/WebSocketReader.h"
#include "WebSocket/WebSocketWriter.h"

#include <boost/asio.hpp>
#include <boost/array.hpp>
#include <boost/core/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

namespace NPL
{
	class CNPLDispatcher;

	/**
	* A incoming or outgoing connection. It does the following things
	* - with a newly established connection, authenticate it and create the NPLRuntimeAddress_ptr, add the address to the npl dispatcher. 
	* - close the connection, if authentication failed or connection times out, remove its NPL address from the npl dispatcher.
	* - when receives an NPL message, it calls the NPLDispatcher to put messages to the correct NPL runtime state's input queue and inform the handler thread. 
	* - When NPL runtime sends an NPL message, the NPLDispatcher will pop the messages from the NPL runtime state's output queue and send it out via the corresponding NPLConnection object. 
	* 
	* Please note that the handle_XXX functions are asynchronous callbacks from the dispatcher thread. 
	*/
	class CNPLConnection : 
		public boost::enable_shared_from_this<CNPLConnection>,
		private boost::noncopyable
	{
	public:
		enum ProtocolType
		{
			NPL = 0,
			WEBSOCKET = 1,
			TCP_CUSTOM = 2, // any custom protocol, like google protocol buffer
		};
		friend class CNPLDispatcher;
		typedef concurrent_ptr_queue<NPLMsgOut_ptr, dummy_condition_variable> RingBuffer_Type;
		typedef std::map<std::string, int>	StringMap_Type;

		~CNPLConnection();
		
		/// Construct a connection with the given io_service.
		explicit CNPLConnection(boost::asio::io_service& io_service,
			CNPLConnectionManager& manager, CNPLDispatcher& msg_dispatcher);

		/// Get the socket associated with the connection.
		boost::asio::ip::tcp::socket& socket();

		/** Start the first asynchronous operation for the connection. 
		*/
		void start();

		/** Stop all asynchronous operations associated with the connection. 
		* @param bRemoveConnection: if true(default), connection will be immediately removed from CNPLConnectionManager
		*  if false, one need to manually remove it from the CNPLConnectionManager
		* @param nReason: 0 is normal disconnect(it will abort pending read/write). 1 is another user with same nid is authenticated. The server should 
		* send a message to tell the client about this. -1 means close the connection when all pending data has been sent. 
		*/
		void stop(bool bRemoveConnection = true, int nReason = 0);

		/** close/stop the connection when all data has been sent */
		void CloseAfterSend();

		/** connect to the remote NPL runtime address. this function returns immediately. It will resolve host and then calls start().
		*/
		void connect();

		/// return the connection manager. 
		CNPLConnectionManager& GetConnectionManager() {return m_connection_manager;};

		/**
		* Send a message via this connection. 
		* @param file_name: the full qualified remote file name. Only relative path and runtime state name is used, other fields are ignored. 
		* @param code: it is a chunk of pure data table init code that would be transmitted to the destination file. 
		* @param nLength: the code length. if this is 0, length is determined from code by finding '\0', 
		* @param priority: if 0 it is normal priority. if 1 it will be inserted to the front of the message queue. 
		*/
		NPLReturnCode SendMessage(const NPLFileName& file_name, const char * code = NULL, int nLength=0, int priority=0);

		/**
		* Send a message via this connection. 
		* this is usually called for sending a special message.
		*/
		NPLReturnCode SendMessage(const NPLMessage& msg);

		/**
		* Send a message via this connection. 
		* this is usually called for sending a simple command message.
		*/
		NPLReturnCode SendMessage(const char* sCommandName, const char* sCommandData);

		/**
		* send a raw NPL output message to the stream. Message will fail if connection is not established, or queue is full. 
		* otherwise it will be added to the m_queueOutput
		* @param msg: the msg should be created just before it. 
		*/
		NPLReturnCode SendMessage(NPLMsgOut_ptr& msg);

		/** set the NPL runtime address that this connection connects to. */
		void SetNPLRuntimeAddress(NPLRuntimeAddress_ptr runtime_address);
	
	public:
		/** Get statistics about this connection. 
		* Function is not thread-safe, it is for debugging anyway. 
		*/
		virtual void GetStatistics( int &totalIn, int &totalOut );
	
		/**
		* Returns the current connection state.
		* Function is not thread-safe
		* @return The state of the connection.
		*/
		NPLConnectionState GetState() const { return m_state; }

		/** Get the NPL runtime address ID if any. 
		* [thread safe]
		*/
		const string& GetNID() const;

		/** Get the TCP IP address if any. 
		* [thread safe]
		*/
		string GetIP();

		/** Get the TCP IP port if any. 
		* [thread safe]
		*/
		string GetPort();

		/** set nid of this connection. If one renames a connection, such as changing from a temporary nid to authenticated nid.
		it will automatically update the nid to connection  mapping in the dispatcher. */
		bool SetNID(const char* sNID);

		/** set whether this connection is authenticated. */
		void SetAuthenticated(bool bAuthenticated);

		/** whether this connection is authenticated. Usually it is a the job of NPL runtime state to reject or authenticate
		a connection. The NPL server just accepts any incoming connections and assign a temporary nid. 
		it is the job of the NPL runtime state to assign an authenticated new nid to this connection. See 
		*/
		bool IsAuthenticated() const;

		/** whether the connection is established or not. */
		bool IsConnected() const;

		/** set whether to use compression for this connection. 
		* this is a candy function for SetCompressionLevel(). 
		*/
		void SetUseCompression(bool bUseCompression);

		/** get whether to use compression for this connection. */
		bool IsUseCompression();

		/** default to 0, which means no compression. Compression level, which is an integer in the range of -1 to 9. 
		* Lower compression levels result in faster execution, but less compression. Higher levels result in greater compression, 
		* but slower execution. The zlib constant Z_DEFAULT_COMPRESSION, equal to -1, provides a good compromise between compression 
		* and speed and is equivalent to level 6. Level 0 actually does no compression at all, and in fact expands the data slightly 
		* to produce the zlib format (it is not a byte-for-byte copy of the input). 
		*/
		void SetCompressionLevel(int nLevel);
		int GetCompressionLevel();

		
		/** when the message size is bigger than this number of bytes, we will use m_nCompressionLevel for compression. 
		* For message smaller than the threshold, we will not compress even m_nCompressionLevel is not 0. 
		*/
		void SetCompressionThreshold(int nThreshold);
		int GetCompressionThreshold();


		/** set the TCP protocol level keep alive. default value depends on whether it is listening sockets or not. 
		* see CNPLNetServer::SetTCPKeepAlive 
		* @note: it is advised to use application level timeout. see SetKeepAlive(). 
		*/
		void SetTCPKeepAlive(bool bEnable);

		/** enable application level keep alive. we will use a global idle timer to detect if a connection has been inactive for GetIdleTimeoutPeriod(),
		* if so, we may send the keep alive message. 
		* @param bEnable: enable keep alive will automatically enable EnableIdleTimeout()
		*/
		void SetKeepAlive(bool bEnable);
		bool IsKeepAliveEnabled();

		///
		void SetNoDelay(bool bEnable);
		bool IsNoDelay();

		/** Enable idle timeout. This is the application level timeout setting. 
		* We will create a global timer which examines all send/receive time of all open connections, if a
		* connection is inactive (idle for GetIdleTimeoutPeriod()) we will 
		*	- if IsKeepAliveEnabled() is false, actively close the connection. This is the method used by HTTP, which is the only solution to detect broken connection without sending additional keep alive message. 
		*   - if IsKeepAliveEnabled() is true, send an empty message to the other end (keep alive messages) to more accurately detect dead connections (see SetKeepAlive). 
		*/
		void EnableIdleTimeout(bool bEnable);
		bool IsIdleTimeoutEnabled();

		/** how many milliseconds of inactivity to assume this connection should be timed out.  if 0 it is never timed out. */
		void SetIdleTimeoutPeriod(int nMilliseconds);
		int GetIdleTimeoutPeriod();

		/** get the last time in milliseconds GetTickCount(), that a send/receive/connect message is transmitted via this connection. 
		* [thread safe only in dispatcher thread]
		*/
		unsigned int GetLastActiveTime();

		/** update the last send time. Internally, it just set the m_nLastActiveTime to GetTickCount(). 
		* [thread safe only in dispatcher thread]
		*/
		void TickSend();

		/** update the last receive or connect time. Internally, it just set the m_nLastActiveTime to GetTickCount(). 
		* [thread safe only in dispatcher thread]
		*/
		void TickReceive();

		/** if any of the connection should be timed out. If so, send keep alive message or the caller should close it. 
		* This function is called by a global service timer that periodically does the checking for all active connections. 
		* @return 1 if not timed out. 0 if timed out,the caller should close the connection. -1 if timed out and keep alive is sent.  
		*/
		int CheckIdleTimeout(unsigned int nCurTime);

		/** whether there is unsent data. */
		bool HasUnsentData();

		/** get global log level.*/
		int GetLogLevel();

		/** set transmission protocol, default value is 0. */
		void SetProtocol(ProtocolType protocolType = ProtocolType::NPL);
	public:
		//
		// In case, one wants to use a different connection data handler,  the following interface are provided. 
		//

		/**
		* This function is called whenever some data is received from the underlying transport.
		* the data is always in (m_buffer.data(), m_buffer.data()+bytes_transferred)
		* @param bytes_transferred The number of bytes received. 
		* @return true if succeed, and one should continue reading from the socket. and false if connection should be closed.  
		*/
		virtual bool handleReceivedData(int bytes_transferred);

		/**
		* This function is called when e.g. the raw TCP connection was established.
		*/
		virtual void handleConnect();

		/**
		* This connection is called when e.g. the raw TCP connection was closed.
		* @param reason The reason for the disconnect.
		*/
		virtual void handleDisconnect(int reason);

		/**
		* handle m_input_msg. After a complete message is read by the parser object, one should call this function. 
		* Once the message is handled, m_input_msg will be reset, so that more messages can be handled later on. 
		*/
		virtual bool handleMessageIn();

	private:
		/// try to parse websocket protocol
		bool handle_websocket_data(int bytes_transferred);
		/// try to parse TCP_CUSTOM protocol
		bool handle_tcp_custom_data(int bytes_transferred);
		//
		// boost io service call backs. 
		//

		/// Handle completion of a read operation.
		void handle_read(const boost::system::error_code& e,std::size_t bytes_transferred);

		/// Handle completion of a write operation.
		void handle_write(const boost::system::error_code& e);

		/// handle disconnection of this object
		void handle_stop();

		/// handle resolving host name to tcp endpoint
		void handle_resolve(const boost::system::error_code& err, boost::asio::ip::tcp::resolver::iterator endpoint_iterator);

		/// handle connection init. This is only used for active connection to server. 
		void handle_connect(const boost::system::error_code& error, boost::asio::ip::tcp::resolver::iterator endpoint_iterator);

		/// Socket for the connection.
		boost::asio::ip::tcp::socket m_socket;

	private:
		typedef boost::array<char, 8192> Buffer_Type;
		/** for address resolving. only created when needed. */
		boost::shared_ptr< boost::asio::ip::tcp::resolver > m_resolver;

		/// The manager for this connection.
		CNPLConnectionManager& m_connection_manager;

		/// this class serves as an interface between the low level socket interface and NPL message queues.  
		CNPLDispatcher& m_msg_dispatcher;

		/// the address of the NPL runtime, which this connection represents. The address is only available when the connection is authenticated and the npl runtime name is known.
		NPLRuntimeAddress_ptr m_address;
		///
		std::string m_resolved_address;

		/// Buffer for incoming data.
		Buffer_Type m_buffer;

		/// The incoming message that is being parsed.
		NPLMsgIn m_input_msg;

		/// The parser for the incoming message.
		NPLMsgIn_parser m_parser;

		/** the output message queue. a queue that is filled by SendMessage() function */
		RingBuffer_Type m_queueOutput;

		/** the NPL connection state*/
		NPLConnectionState m_state;

		/** this mutex is only used for determine the state. */
		ParaEngine::mutex m_mutex;

		/** a mapping from filename to file id. The file map is established at runtime. */
		StringMap_Type m_filename_id_map;

		/// for statistics, number of bytes received
		uint32 m_totalBytesIn;
		/// for statistics, number of bytes sent
		uint32 m_totalBytesOut;
		
		/** default to false, if true, it will dump all send and received data to output. */
		bool m_bDebugConnection;

		/** default to 0, which means no compression. Compression level, which is an integer in the range of -1 to 9. 
		* Lower compression levels result in faster execution, but less compression. Higher levels result in greater compression, 
		* but slower execution. The zlib constant Z_DEFAULT_COMPRESSION, equal to -1, provides a good compromise between compression 
		* and speed and is equivalent to level 6. Level 0 actually does no compression at all, and in fact expands the data slightly 
		* to produce the zlib format (it is not a byte-for-byte copy of the input). 
		*/
		int32 m_nCompressionLevel;

		/** when the message size is bigger than this number of bytes, we will use m_nCompressionLevel for compression. 
		* For message smaller than the threshold, we will not compress even m_nCompressionLevel is not 0. 
		*/
		int32 m_nCompressionThreshold;

		/** whether SO_KEEPALIVE is enabled.*/
		bool m_bKeepAlive;

		///
		bool m_bNoDelay;

		/** whether idle timeout is enabled. */
		bool m_bEnableIdleTimeout;

		/** number of async_send calls */
		uint32 m_nSendCount;
		/** number of finished async_send calls. if m_nSendCount == m_nFinishedCount, it means that all messages are sent */
		uint32 m_nFinishedCount;

		/** close connection when all data is sent*/
		bool m_bCloseAfterSend;

		/** how many milliseconds to assume time out, default to 2 mins. 0 is never time out. */
		uint32 m_nIdleTimeoutMS;

		/** Get last active time. */
		uint32 m_nLastActiveTime;

		/** when does this connection started */
		uint32 m_nStartTime;

		/** why is this connection stopped */
		int32 m_nStopReason;

		WebSocket::WebSocketReader m_websocket_reader;
		WebSocket::WebSocketWriter m_websocket_writer;
		std::vector<byte> m_websocket_input_data;
		std::vector<byte> m_websocket_out_data;

		ProtocolType m_protocolType;
	};
	

	/** a compare class connection ptr */
	struct NPLConnection_PtrOps
	{	// functor for operator<
		bool operator()(const NPLConnection_ptr& _Left, const NPLConnection_ptr& _Right) const
		{	// apply operator< to operands
			return (_Left.get()<_Right.get());
		}
	};
}

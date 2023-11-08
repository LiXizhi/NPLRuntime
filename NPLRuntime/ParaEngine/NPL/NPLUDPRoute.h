#pragma once
#include "NPLMessage.h"
#include "NPLCommon.h"
#include "NPLMsgIn.h"
#include "NPLMsgOut.h"
#include "NPLMsgIn_parser.h"
#include "NPLMessageQueue.h"
#ifndef EMSCRIPTEN_SINGLE_THREAD
#include <boost/asio.hpp>
#endif
#include <boost/array.hpp>
#include <boost/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>



namespace NPL
{
	class CNPLUDPDispatcher;
	class CNPLUDPRouteManager;
	class CNPLNetUDPServer;

	struct NPLUDPAddress :
		public boost::enable_shared_from_this<NPLUDPAddress>,
		private boost::noncopyable
	{
#ifndef EMSCRIPTEN_SINGLE_THREAD
		NPLUDPAddress(const string& sHost, unsigned short port, const string& sNID): m_ep(boost::asio::ip::make_address_v4(sHost), port), m_sNID(sNID)
#else			
		NPLUDPAddress(const string& sHost, unsigned short port, const string& sNID): m_sNID(sNID)
#endif
			
		{

		}

#ifndef EMSCRIPTEN_SINGLE_THREAD
		NPLUDPAddress(unsigned int host, unsigned short port, const string& sNID): m_ep(boost::asio::ip::address_v4(host), port), m_sNID(sNID)
#else	
		NPLUDPAddress(unsigned int host, unsigned short port, const string& sNID): m_sNID(sNID)
#endif
		{

		}


#ifndef EMSCRIPTEN_SINGLE_THREAD
		NPLUDPAddress(const boost::asio::ip::udp::endpoint& ep, const string& sNID) 
			: m_ep(ep)
			, m_sNID(sNID)
		{
		}
#endif


		inline const std::string& GetNID() const { return m_sNID; }
		inline void SetNID(const char* nid) { m_sNID = nid; }
		inline void SetNID(const std::string& nid) { m_sNID = nid; }
#ifndef EMSCRIPTEN_SINGLE_THREAD
		inline const std::string GetHost() const { return m_ep.address().to_string(); }
		inline unsigned short GetPort() const { return m_ep.port(); }
		inline const boost::asio::ip::udp::endpoint&  GetEndPoint() const {return m_ep; }
		inline unsigned long long GetHash() const { return ComputerHash(m_ep); }

		static unsigned long long ComputerHash(const boost::asio::ip::udp::endpoint& ep)
		{
			unsigned long long ip = (unsigned long long)ep.address().to_v4().to_uint();
			unsigned long long port = (unsigned long long)ep.port();

			return (ip << 32) | port;
		}
#else
		inline unsigned long long GetHash() const { return 0; }
		inline const std::string GetHost() const { return "0.0.0.0"; }
		inline unsigned short GetPort() const { return 0; }
#endif

	private:
		//
		// ip address and port are const, because NPL udp address are immutable.
		//
#ifndef EMSCRIPTEN_SINGLE_THREAD
		boost::asio::ip::udp::endpoint m_ep;
#endif

		// nid may be changed during the course of authentication. 
		std::string m_sNID;
	};

	typedef boost::shared_ptr<NPLUDPAddress> NPLUDPAddress_ptr;


	class CNPLUDPRoute :
		public boost::enable_shared_from_this<CNPLUDPRoute>,
		private boost::noncopyable
	{

	public:
		friend class CNPLUDPDispatcher;

		~CNPLUDPRoute();

		/// Construct a route with the given io_service.
		explicit CNPLUDPRoute(CNPLNetUDPServer& udp_server, CNPLUDPRouteManager& manager, CNPLUDPDispatcher& msg_dispatcher);


		/** Start the first asynchronous operation for the route.
		*/
		void start();
		///
		void start_broadcast(unsigned short port);
		///
		void start_send(const char* ip, unsigned short port);

		/** Stop all asynchronous operations associated with the route.
		* @param bRemoveConnection: if true(default), route will be immediately removed from CNPLUDPRouteManager
		*  if false, one need to manually remove it from the CNPLUDPRouteManager
		* @param nReason: 0 is normal disconnect(it will abort pending read/write). 1 is another user with same nid is authenticated. The server should
		* send a message to tell the client about this. -1 means close the route when all pending data has been sent.
		*/
		void stop(bool bRemoveConnection = true);

		/** close/stop the route when all data has been sent */
		void CloseAfterSend();

		/// return the route manager. 
		CNPLUDPRouteManager& GetRouteManager() { return m_route_manager; };

		/**
		* Send a message via this route.
		* @param file_name: the full qualified remote file name. Only relative path and runtime state name is used, other fields are ignored.
		* @param code: it is a chunk of pure data table init code that would be transmitted to the destination file.
		* @param nLength: the code length. if this is 0, length is determined from code by finding '\0',
		* @param priority: if 0 it is normal priority. if 1 it will be inserted to the front of the message queue.
		*/
		NPLReturnCode SendMessage(const NPLFileName& file_name, const char * code = nullptr, int nLength = 0, int priority = 0);

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


		/** Get the NPL runtime address ID if any.
		* [thread safe]
		*/
		const string& GetNID() const;

		/** Get the TCP/IP address if any.
		* [thread safe]
		*/
		string GetIP();

		/** Get the TCP/IP port if any.
		* [thread safe]
		*/
		unsigned short GetPort();

	
		/** set nid of this route. If one renames a route, such as changing from a temporary nid to authenticated nid.
		it will automatically update the nid to route  mapping in the dispatcher. */
		bool SetNID(const char* sNID);
		

		/** set the NPL udp address that this route connects to. */
		void SetNPLUDPAddress(NPLUDPAddress_ptr udp_address);

		//
		NPLUDPAddress_ptr GetNPLUDPAddress() const { return m_address;  }

		/** set whether to use compression for this route.
		* this is a candy function for SetCompressionLevel().
		*/
		void SetUseCompression(bool bUseCompression);

		/** get whether to use compression for this route. */
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

		/** Enable idle timeout. This is the application level timeout setting.
		* We will create a global timer which examines all send/receive time of all open routes, if a
		* route is inactive (idle for GetIdleTimeoutPeriod()) we will
		*/
		void EnableIdleTimeout(bool bEnable);
		bool IsIdleTimeoutEnabled();

		/** how many milliseconds of inactivity to assume this route should be timed out.  if 0 it is never timed out. */
		void SetIdleTimeoutPeriod(int nMilliseconds);
		int GetIdleTimeoutPeriod();

		/** get the last time in milliseconds GetTickCount(), that a send/receive message is transmitted via this route.
		* [thread safe only in dispatcher thread]
		*/
		unsigned int GetLastActiveTime();

		/** update the last send time. Internally, it just set the m_nLastActiveTime to GetTickCount().
		* [thread safe only in dispatcher thread]
		*/
		void TickSend();

		/** update the last receive time. Internally, it just set the m_nLastActiveTime to GetTickCount().
		* [thread safe only in dispatcher thread]
		*/
		void TickReceive();

		/** if any of the route should be timed out. If so, send keep alive message or the caller should close it.
		* This function is called by a global service timer that periodically does the checking for all active connections.
		* @return 1 if not timed out. 0 if timed out,the caller should close the route. -1 if timed out and keep alive is sent.
		*/
		int CheckIdleTimeout(unsigned int nCurTime);

		/** whether there is unsent data. */
		bool HasUnsentData();

		/** get global log level.*/
		int GetLogLevel();


		/**
		* This function is called whenever some data is received from the underlying transport.
		* the data is always in (m_buffer.data(), m_buffer.data()+bytes_transferred)
		* @param bytes_transferred The number of bytes received.
		* @return true if succeed, and one should continue reading from the socket. and false if route should be closed.
		*/
		bool handleReceivedData(const char* buff, size_t bytes_transferred);


		/**
		* handle m_input_msg. After a complete message is read by the parser object, one should call this function.
		* Once the message is handled, m_input_msg will be reset, so that more messages can be handled later on.
		*/
		bool handleMessageIn();

		///
		unsigned long long getHash();

		//
#ifndef EMSCRIPTEN_SINGLE_THREAD
		void handle_send(const boost::system::error_code& error, size_t bytes_transferred, const char* buff, size_t buff_size);
#endif
	private:

		/// handle disconnection of this object
		void handle_stop();

		//typedef boost::array<char, 8192> Buffer_Type;

		/// The manager for this route.
		CNPLUDPRouteManager& m_route_manager;

		/// this class serves as an interface between the low level socket interface and NPL message queues.  
		CNPLUDPDispatcher& m_msg_dispatcher;

		///
		CNPLNetUDPServer& m_udp_server;


		/// 
		NPLUDPAddress_ptr m_address;

		/// Buffer for incoming data.
		//Buffer_Type m_buffer;
		
		/// The incoming message that is being parsed.
		NPLMsgIn m_input_msg;

		/// The parser for the incoming message.
		NPLMsgIn_parser m_parser;


		/** this mutex is only used for determine the state. */
		ParaEngine::mutex m_mutex;

		/// for statistics, number of bytes received
		uint32 m_totalBytesIn;
		/// for statistics, number of bytes sent
		uint32 m_totalBytesOut;

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

			/** whether idle timeout is enabled. */
			bool m_bEnableIdleTimeout;

			/** number of async_send calls */
			uint32 m_nSendCount;
			/** number of finished async_send calls. if m_nSendCount == m_nFinishedCount, it means that all messages are sent */
			uint32 m_nFinishedCount;

			/** close route when all data is sent*/
			bool m_bCloseAfterSend;

			/** how many milliseconds to assume time out, default to 2 mins. 0 is never time out. */
			uint32 m_nIdleTimeoutMS;

			/** Get last active time. */
			uint32 m_nLastActiveTime;

			/** when does this route started */
			uint32 m_nStartTime;

	}; // class NPLUDPRoute

} // namespace NPL
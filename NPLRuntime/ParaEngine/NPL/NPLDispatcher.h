#pragma once

#include <boost/bimap.hpp>
#ifndef EMSCRIPTEN_SINGLE_THREAD
#include <boost/thread/shared_mutex.hpp>
#endif
#include <string>

#include "NPLCommon.h"
#include "NPLMessageQueue.h"
#include "NPLConnection.h"
#include "NPLRuntimeState.h"
#include "util/mutex.h"

namespace NPL
{
	/**
	* this class serves as an interface between the low level socket interface and NPL message queues.  
	* - when a socket connection receives an NPL message, it calls the NPLDispatcher to put messages to the correct NPL runtime state's input queue and inform the handler thread. 
	* - When the local NPL runtime sends an NPL message, the NPLDispatcher will create get a connection object that represents the target and then sends out the message. 
	* - when a NPL server accepts a new connection, it will add a new connection to the dispatcher with their sNID. 
	*
	* all handler_xxx functions are called from the dispatcher thread. the dispatching thread is an asynchronous io service for dispatching all messages of all sockets. 
	*/
	class CNPLDispatcher
	{
	public:
		CNPLDispatcher(CNPLNetServer* server);
		~CNPLDispatcher();

		/** the NPL Net server object. */
		CNPLNetServer* GetNetServer() { return m_pServer; }

		/** clean up all resources*/
		void Cleanup();

		/**
		* Get the NPL connection object by a given NID. 
		* if the NID does not match a connection,a new connection object will be created and immediately returned. 
		* Please note that this function will always return a NPLConnection object. For newly created connection object, it
		* may not even been connected or authenticated and all messages to it are cached in the output (pending) queue.
		* [thread safe]
		* @param sNID: this is the sNID in NPLFileName. Please see NPLFileName for more details. 
		* It usually represents a network endpoint (IP/port). 
		* However, instead of using IP address "192.168.0.111/60001", we usually use email like addresses, like "1001@paraengine.com"
		* the email address will latter be resolved to IP/port. if empty, it means the local runtime environment.
		* Specially, "all@local" means all remote NIDs connecting to this local machine.
		*
		* @note: it is possible that multiple forms of sNID matches to the same NPL connection. 
		*/
		NPLConnection_ptr CreateGetNPLConnectionByNID(const string& sNID);

		/**
		* same as CreateGetNPLConnectionByNID(), except that the returned connection object may be null, if no sNID matches. 
		* [thread safe]
		*/
		NPLConnection_ptr GetNPLConnectionByNID(const string& sNID);

		/**
		* A a new NPL connection to the m_active_connection_map. it also removes from the pending connection map if any. 
		* if there is already a connection with the same name, the previous one will be silently removed. 
		* This function is called by the ConnectionManager, when it has accepted a new incoming connection. 
		* [thread safe]
		* @param sNID: the sNID of the connection. 
		* @param pConnection: if pConnection is not connected, this function does nothing. 
		*/
		void AddNPLConnection(const string& sNID, NPLConnection_ptr pConnection);

		/** remove a given connection object from the active or pending connection map if any.
		* This function is called automatically by the NPLConnection object when the connection is closed. 
		* [thread safe]
		* @return true if connection is found and removed. false if connection is not found. 
		*/
		bool RemoveNPLConnection(NPLConnection_ptr pConnection);

		/** accept a given connection. The connection will be regarded as authenticated once accepted. 
		* [thread safe]
		* @param tid: the temporary id or NID of the connection to be accepted. usually it is from msg.tid or msg.nid. 
		* @param nid: if this is not nil, tid will be renamed to nid after accepted. 
		*/
		void NPL_accept(const char* tid, const char* nid = NULL);

		/** set transmission protocol, default value is 0. */
		void NPL_SetProtocol(const char* nid, CNPLConnection::ProtocolType protocolType = CNPLConnection::ProtocolType::NPL);

		/** rename a given connection. this is called by SetNid() and its function is same as NPL_accept.
		* [thread safe]
		* @param tid: the temporary id or NID of the connection to be accepted. usually it is from msg.tid or msg.nid. 
		* @param nid: if this is not nil, tid will be renamed to nid after accepted. 
		*/
		void RenameConnection(NPLConnection_ptr pConnection, const char* sNid);

		/**
		* Activate the specified remote file. This function is called by CNPLRuntime::NPL_Activate
		* @param runtime_state: the runtime environment
		* @param file_name: the full qualified remote file name 
		* @param code: it is a chunk of pure data table init code that would be transmitted to the destination file. 
		* @param nLength: the code length. if this is 0, length is determined from code by finding '\0', 
		* @param priority: if 0 it is normal priority. if 1 it will be inserted to the front of the message queue. 
		* @return if failed, such as the runtime state does not exist, etc. 
		*/
		NPLReturnCode Activate_Async(const NPLFileName& file_name, const char * code = NULL, int nLength=0, int priority=0);

		/**
		* Dispatch a message from a given socket connection to a local NPL runtime state. This function is called by the connection object's data handler
		* whenever a message is received from the network layer.
		* This function just parse and insert message to a proper NPL runtime state's input queue and returns immediately.
		* @note: this function is similar to Activate_Async, except that it takes input from a network message and always dispatched to a local runtime state. 
		* @return if failed, such as the runtime state does not exist, etc. 
		*/
		NPLReturnCode DispatchMsg(NPLMsgIn& msg);
		

		/**
		* add a given NPL runtime address to the fully trusted server addresses list. 
		* [thread safe]
		*/
		bool AddNPLRuntimeAddress(NPLRuntimeAddress_ptr pAddress);

		/**
		* check to see if a given NID is trusted by the dispatcher. if so, return its address. 
		* [thread safe]
		*/
		NPLRuntimeAddress_ptr GetNPLRuntimeAddress(const string& sNID);


		/** add a nID, filename pair to the public file list. 
		* we only allow remote NPL runtime to activate files in the public file list. 
		* Each public file has a user defined ID. The NPL network layer always try to use its ID for transmission to minimize bandwidth. 
		* There are some negotiations between sender and receiver to sync the string to ID map before they use it. 
		* [thread safe]
		* @param nID: the integer to encode the string. it is usually small and positive number.
		* @param sString: the string for the id. if input is empty, it means removing the mapping of nID. 
		*/
		void AddPublicFile(const string& filename, int nID);

		/** clear all public files, so that the NPL server will be completely private. 
		* [thread safe]
		*/
		void ClearPublicFiles();

		/** get a public file name by its id in the public filename map 
		* [thread safe]
		* @return false if not found
		*/
		bool GetPubFileNameByID(string& filename, int nID);

		/** return true if either filename or nID is in the public file list. 
		* [thread safe]
		*/
		bool CheckPubFile(string& filename, int& nID);

		/** get id by filename string in the public filename map 
		* [thread safe]
		* @return -1 if not found
		*/
		int GetIDByPubFileName(const string& sString);

		/** post network event 
		* It will post network message as {code=nNPLNetworkCode, nid=string, msg=string}
		* @param nNPLNetworkCode: one of the NPLReturnCode.
		* @param sNid: connection nid that sends this request. Can be NULL.
		* @param sMsg: a string message. Can be NULL.
		*/
		int PostNetworkEvent(NPLReturnCode nNPLNetworkCode, const char * sNid=NULL, const char* sMsg=NULL);

		/** whether to use compression on transport layer for incoming connections
		* @param bCompressIncoming: if true, compression is used for all incoming connections. default to false.
		*/
		void SetUseCompressionIncomingConnection(bool bCompressIncoming);
		bool IsUseCompressionIncomingConnection();

		/** whether to use compression on transport layer for outgoing connections
		* @param bCompressIncoming: if true, compression is used for all outgoing connections. default to false.
		*/
		void SetUseCompressionOutgoingConnection(bool bCompressOutgoing);
		bool IsUseCompressionOutgoingConnection();

		/** Set the zlib compression level to use in case compresssion is enabled. 
		* default to 0, which means no compression. Compression level, which is an integer in the range of -1 to 9. 
		* Lower compression levels result in faster execution, but less compression. Higher levels result in greater compression, 
		* but slower execution. The zlib constant Z_DEFAULT_COMPRESSION, equal to -1, provides a good compromise between compression 
		* and speed and is equivalent to level 6. Level 0 actually does no compression at all, and in fact expands the data slightly 
		* to produce the zlib format (it is not a byte-for-byte copy of the input). 
		*/
		void SetCompressionLevel(int nLevel);
		int GetCompressionLevel();


		/** set the default compression threshold for all connections on this machine. 
		* when the message size is bigger than this number of bytes, we will use m_nCompressionLevel for compression. 
		* For message smaller than the threshold, we will not compress even m_nCompressionLevel is not 0. 
		*/
		void SetCompressionThreshold(int nThreshold);
		int GetCompressionThreshold();

	protected:
		/**
		* Create a new connection with a remote server and immediately connect and start the connection. 
		* [Thread Safe]
		* @param pAddress: the target NPL runtime address to connect to. 
		* @return The function returns immediately, so the returned connection may not have been connected or authenticated yet.
		*/
		NPLConnection_ptr CreateConnection(NPLRuntimeAddress_ptr pAddress);

		/** not thread safe. please see RenameConnection(). */
		void RenameConnectionImp(NPLConnection_ptr pConnection, const char* sNid);

	protected:
		typedef std::map<string, NPLConnection_ptr> ActiveConnectionMap_Type;
		typedef std::map<string, NPLRuntimeAddress_ptr> ServerAddressMap_Type;
		typedef boost::bimap<int, std::string>	StringBimap_Type;
		
		/** a mapping from the authenticated NPL runtime id (NID) to its associated NPL connection. */
		ActiveConnectionMap_Type m_active_connection_map;

		/** a mapping from the not connected or authenticated NPL runtime id (NID) to its associated NPL connection. */
		ActiveConnectionMap_Type m_pending_connection_map;

		/** a mapping from the NPL runtime id (NID) to its full NPLRuntimeAddress. 
		* NPL runtime addresses in this set are fully trusted by the dispatcher. 
		* The dispatcher will automatically establish connections to those addresses if any no existing connection is available. 
		*/
		ServerAddressMap_Type m_server_address_map;

		/** provide thread safe access to shared data members in this class. 
		* @note: a simple critical section mutex is better than boost::shared_mutex (read_write_lock),  since we only 
		* lock a few cycles within spin count.
		*/
		ParaEngine::mutex m_mutex;
		
		/** whether to use compression on transport layer for incoming connections */
		bool m_bUseCompressionIncomingConnection;

		/** whether to use compression on transport layer for outgoing connections */
		bool m_bUseCompressionOutgoingConnection;

		/** zlib compression level to use in case compresssion is enabled. */
		int m_nCompressionLevel;

		/** when msg is larger than this, we will compress it. */
		int m_nCompressionThreshold;

		/**
		* only files in the public file map can be activated remotely. 
		* bidirectional map between file id and filename. 
		*/
		StringBimap_Type m_public_filemap;

		/** the server object */
		CNPLNetServer* m_pServer;
	};
}
#pragma once

#include <boost/bimap.hpp>
#ifndef EMSCRIPTEN_SINGLE_THREAD
#include <boost/thread/shared_mutex.hpp>
#endif
#include <string>

#include "NPLCommon.h"
#include "NPLMessageQueue.h"
#include "NPLUDPRoute.h"
#include "NPLRuntimeState.h"
#include "util/mutex.h"

namespace NPL
{
	class CNPLUDPDispatcher
	{
	public:
		CNPLUDPDispatcher(CNPLNetUDPServer& server, CNPLUDPRouteManager& manager);
		~CNPLUDPDispatcher();

		/** the NPL Net udp server object. */
		CNPLNetUDPServer& GetNetServer() { return m_server; }

		/** clean up all resources*/
		void Cleanup();


		NPLUDPRoute_ptr CreateGetNPLRouteByNID(const std::string& sNID);

		///
		NPLReturnCode DispatchMsg(NPLMsgIn& msg);

		/**
		* add a given NPL runtime address to the fully trusted server addresses list.
		* [thread safe]
		*/
		bool AddNPLUDPAddress(NPLUDPAddress_ptr pAddress);

		// it will same to rename route
		void RenameAddress(NPLUDPAddress_ptr udp_address, const char* sNID);

		/// 
		void RemoveUDPAddress(NPLUDPAddress_ptr pAddress);


		/**
		* check to see if a given NID is trusted by the dispatcher. if so, return its address.
		* [thread safe]
		*/
		NPLUDPAddress_ptr GetNPLUDPAddress(const string& sNID);

		/** post network event
		* It will post network message as {code=nNPLNetworkCode, nid=string, msg=string}
		* @param nNPLNetworkCode: one of the NPLReturnCode.
		* @param sNid: route nid that sends this request. Can be NULL.
		* @param sMsg: a string message. Can be NULL.
		*/
		int PostNetworkEvent(NPLReturnCode nNPLNetworkCode, const char * sNid = nullptr, const char* sMsg = nullptr);

		/** whether to use compression on transport layer for incoming/outgoing routes
		* @param bCompress: if true, compression is used for all incoming/outgoing routes. default to false.
		*/
		void SetUseCompressionRoute(bool bCompress);
		bool IsUseCompressionRoute();



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

		/**
		* Activate the specified remote file. This function is called by CNPLRuntime::NPL_Activate
		* @param runtime_state: the runtime environment
		* @param file_name: the full qualified remote file name
		* @param code: it is a chunk of pure data table init code that would be transmitted to the destination file.
		* @param nLength: the code length. if this is 0, length is determined from code by finding '\0',
		* @param priority: if 0 it is normal priority. if 1 it will be inserted to the front of the message queue.
		* @return if failed, such as the runtime state does not exist, etc.
		*/
		NPLReturnCode Activate_Async(const NPLFileName& file_name, const char * code = nullptr, int nLength = 0, int priority = 0);
		NPLReturnCode Activate_Async2(const NPLFileName& file_name, const char* ip, unsigned short port, const char * code = nullptr, int nLength = 0, int priority = 0);

		NPLReturnCode Broadcast_Async(const NPLFileName& file_name, unsigned short port, const char * code = nullptr, int nLength = 0, int priority = 0);
	
	protected:

	protected:
		typedef std::map<string, NPLUDPAddress_ptr> ServerAddressMap_Type;

		///
		ServerAddressMap_Type m_server_address_map;

		/** provide thread safe access to shared data members in this class.
		* @note: a simple critical section mutex is better than boost::shared_mutex (read_write_lock),  since we only
		* lock a few cycles within spin count.
		*/
		ParaEngine::mutex m_mutex;


		/** whether to use compression on transport layer for incoming/outgoing routes */
		bool m_bUseCompressionRoute;

		/** zlib compression level to use in case compresssion is enabled. */
		int m_nCompressionLevel;

		/** when msg is larger than this, we will compress it. */
		int m_nCompressionThreshold;

		///
		NPLUDPRoute_ptr m_broadcast_route;
	
		/** the server object */
		CNPLNetUDPServer& m_server;
		///
		CNPLUDPRouteManager& m_route_manager;
	};

} // namespace NPL
#pragma once
//-----------------------------------------------------------------------------
// Copyright (C) 2007 - 2010 ParaEngine Corporation, All Rights Reserved.
// Author: LiXizhi
// Date: 2010.2.26
// Description:	API for NPL interface 
//-----------------------------------------------------------------------------
#include "baseinterface.h"
#include <string>
#include "INPLRuntimeState.h"

namespace ParaEngine
{
	class INPLJabberClient;
	class CURLRequestTask;
}
namespace NPL 
{
	class INPLRuntimeState;

	/**
	* NPL Runtime Environment interface. 
	* NPL runtime hosts multiple NPLRuntimeStates. Each NPLRuntimeState usually runs in a separate thread 
	* Multiple languages, like C++, NPL, .Net can be used to program inside a NPL runtime state. 
	* A NPLRuntimeState runs scripts by dynamically load INPLScriptingState(script VM) according to file extension. 
	* One can implement new language extension by implementing the INPLScriptingState interface. C++ dll file is natively supported and do not use the INPLRuntimeState interface. 
	* 
	* There is a special NPL state called (main) state, which can be configured to run in the main thread or in a separate thread like other normal states. 
	*/
	class INPLRuntime : ParaEngine::BaseInterface
	{
	public:
		/** initialize NPL runtime environment */
		virtual void Init() = 0;

		/**
		* call this function regularly in the main game thread to process packages. 
		* This function also dispatches messages for the (main) runtime state if it is configured so. 
		* @param bToEnd: if true, the function will only return when there is no more input packages in the queue
		*/
		virtual void Run(bool bToEnd = true) = 0;

		/** clean up the NPL runtime environment */
		virtual void Cleanup() = 0;

		/** whether we will process messages in the main threads in the frame move function. 
		* It is default to true;  
		* However, it is possible for server to set to false, if one wants to have a more responsive main state on the server. 
		* For example, it does high-frequency dispatcher jobs, instead of monitoring. 
		* But client application, it is only advised to set to true, otherwise the scripting and render modules will be run in different threads, leading to complexity and bugs. 
		* @NOTE: One can only call this function once to set to false. This function is only used by the ParaEngineServer
		*/
		virtual void SetHostMainStatesInFrameMove(bool bHostMainStatesInFrameMove) = 0;

		/** create a new runtime state.
		* this function is thread safe 
		* @param name: if "", it is an anonymous runtime state. otherwise it should be a unique name. 
		* @param type_: the runtime state type. 
		* @return the newly created state is returned. If an runtime state with the same non-empty name already exist, the old one is returned. 
		*/
		virtual INPLRuntimeState* CreateState(const char* name, NPLRuntimeStateType type_=NPLRuntimeStateType_NPL) = 0;

		/** get a runtime state with an explicit name.
		* this function is thread safe 
		* @param name: the name of the runtime state. if NULL or "main", the main runtime state is returned. 
		*/
		virtual INPLRuntimeState* GetState(const char* name = NULL) = 0;

		/** it get runtime state first, if none exist, it will create one and add it to the main threaded state */
		virtual INPLRuntimeState* CreateGetState(const char* name, NPLRuntimeStateType type_=NPLRuntimeStateType_NPL) = 0;

		/** create a given runtime state.
		* this function is thread safe */
		virtual bool DeleteState(INPLRuntimeState* pRuntime_state) = 0;

		/** get the default (main) runtime state.*/
		virtual INPLRuntimeState* GetMainState() = 0;

		/** add a given runtime state to the main game thread. 
		* this function is thread safe 
		*/
		virtual bool AddToMainThread(INPLRuntimeState* runtime_state) = 0;

		/** whether to use compression on transport layer for incoming and outgoing connections
		* @param bCompressIncoming: if true, compression is used for all incoming connections. default to false.
		* @param bCompressIncoming: if true, compression is used for all outgoing connections. default to false.
		*/
		virtual void SetUseCompression(bool bCompressIncoming, bool bCompressOutgoing) = 0;

		/**
		* set the compression method of incoming the outgoing messages. 
		* If this is not called, the default internal key is used for message encoding. 
		* [Not Thread Safe]: one must call this function before sending or receiving any encoded messages. 
		* so it is usually called when the game engine starts. 
		* @param sKey: the byte array of key. the generic key that is used for encoding/decoding
		* @param nSize: size in bytes of the sKey. default is 64 bytes
		* @param nUsePlainTextEncoding: default to 0. 
		* if 0, the key is used as it is. 
		* if 1, the input key will be modified so that the encoded message looks like plain text(this can be useful to pass some firewalls). 
		* if -1, the input key will be modified so that the encoded message is binary. 
		*/
		virtual void SetCompressionKey(const byte* sKey=0, int nSize=0, int nUsePlainTextEncoding = 0) = 0;

		/** Set the zlib compression level to use in case compresssion is enabled. 
		* default to 0, which means no compression. Compression level, which is an integer in the range of -1 to 9. 
		* Lower compression levels result in faster execution, but less compression. Higher levels result in greater compression, 
		* but slower execution. The zlib constant Z_DEFAULT_COMPRESSION, equal to -1, provides a good compromise between compression 
		* and speed and is equivalent to level 6. Level 0 actually does no compression at all, and in fact expands the data slightly 
		* to produce the zlib format (it is not a byte-for-byte copy of the input). 
		*/
		virtual void SetCompressionLevel(int nLevel) = 0;
		virtual int GetCompressionLevel() = 0;

		/** set the default compression threshold for all connections on this machine. 
		* when the message size is bigger than this number of bytes, we will use m_nCompressionLevel for compression. 
		* For message smaller than the threshold, we will not compress even m_nCompressionLevel is not 0. 
		*/
		virtual void SetCompressionThreshold(int nThreshold) = 0;
		virtual int GetCompressionThreshold() = 0;

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
		virtual void SetTCPKeepAlive(bool bEnable) = 0;

		/** whether SO_KEEPALIVE is enabled. 
		* @return bEnable: true to enable.
		*/
		virtual bool IsTCPKeepAliveEnabled() = 0;

		/** enable application level keep alive. we will use a global idle timer to detect if a connection has been inactive for GetIdleTimeoutPeriod(),
		* if so, we may send the keep alive message. 
		* @param bEnable: enable keep alive will automatically enable EnableIdleTimeout()
		*/
		virtual void SetKeepAlive(bool bEnable) = 0;
		virtual bool IsKeepAliveEnabled() = 0;

		/** Enable idle timeout. This is the application level timeout setting. 
		* We will create a global timer which examines all send/receive time of all open connections, if a
		* connection is inactive (idle for GetIdleTimeoutPeriod()) we will 
		*	- if IsKeepAliveEnabled() is false, actively close the connection. This is the method used by HTTP, which is the only solution to detect broken connection without sending additional keep alive message. 
		*   - if IsKeepAliveEnabled() is true, send an empty message to the other end (keep alive messages) to more accurately detect dead connections (see SetKeepAlive). 
		*/
		virtual void EnableIdleTimeout(bool bEnable) = 0;
		virtual bool IsIdleTimeoutEnabled() = 0;

		/** how many milliseconds of inactivity to assume this connection should be timed out. if 0 it is never timed out. */
		virtual void SetIdleTimeoutPeriod(int nMilliseconds) = 0;
		virtual int GetIdleTimeoutPeriod() = 0;

		/**
		* start the NPL net server's io_service loop. This function returns immediately. it will spawn the accept and dispatcher thread.  
		* call this function only once per process.
		* @param server: default to "127.0.0.1"
		* @param port: default to "60001"
		*/
		virtual void StartNetServer(const char* server=NULL, const char* port=NULL) = 0;

		/** stop the net server */
		virtual void StopNetServer() = 0;

		/** add a nID, filename pair to the public file list. 
		* we only allow remote NPL runtime to activate files in the public file list. 
		* Each public file has a user defined ID. The NPL network layer always try to use its ID for transmission to minimize bandwidth. 
		* There are some negotiations between sender and receiver to sync the string to ID map before they use it. 
		* [thread safe]
		* @param nID: the integer to encode the string. it is usually small and positive number.
		* @param sString: the string for the id. if input is empty, it means removing the mapping of nID. 
		*/
		virtual void AddPublicFile(const std::string& filename, int nID) = 0;

		/** clear all public files, so that the NPL server will be completely private. 
		* [thread safe]
		*/
		virtual void ClearPublicFiles() = 0;

		/** get the ip address of given NPL connection. 
		* this function is usually called by the server for connected clients. 
		* @param nid: nid or tid. 
		* @param pOutPut: it must be at least [256] bytes big, that receives the output. 
		* @return: the ip address in dot format. empty string is returned if connection can not be found. 
		*/
		virtual void GetIP(const char* nid, char* pOutput) = 0;


		/** accept a given connection. The connection will be regarded as authenticated once accepted. 
		* [thread safe]
		* @param tid: the temporary id or NID of the connection to be accepted. usually it is from msg.tid or msg.nid. 
		* @param nid: if this is not nil, tid will be renamed to nid after accepted. 
		*/
		virtual void accept(const char* tid, const char* nid = NULL) = 0;

		/** reject and close a given connection. The connection will be closed once rejected. 
		* [thread safe]
		* @param nid: the temporary id or NID of the connection to be rejected. usually it is from msg.tid or msg.nid. 
		* @param nReason: default to 0. if 1 it means connection overriden
		*/
		virtual void reject(const char* nid, int nReason = 0) = 0;


		//////////////////////////////////////////////////////////////////////////
		//
		// jabber client functions
		//
		//////////////////////////////////////////////////////////////////////////

		/**
		* get an existing jabber client instance interface by its JID.
		* If the client is not created using CreateJabberClient() before, function may return NULL.
		* @param sJID: such as "lixizhi@paraweb3d.com"
		*/
		virtual ParaEngine::INPLJabberClient* GetJabberClient(const char* sJID) = 0;
		/**
		* Create a new jabber client instance with the given jabber client ID. It does not open a connection immediately.
		* @param sJID: such as "lixizhi@paraweb3d.com"
		*/
		virtual ParaEngine::INPLJabberClient* CreateJabberClient(const char* sJID) = 0;

		/**
		* close a given jabber client instance. Basically, there is no need to close a web service, 
		* unless one wants to reopen it with different credentials
		* @param sJID: such as "lixizhi@paraweb3d.com"
		*/
		virtual bool CloseJabberClient(const char* sJID) = 0;

		//////////////////////////////////////////////////////////////////////////
		//
		// new libcUrl interface. 
		//
		//////////////////////////////////////////////////////////////////////////

		/** Append URL request to a pool. 
		* @param pUrlTask: must be new CURLRequestTask(), the ownership of the task is transfered to the manager. so the caller should never delete the pointer. 
		* @param sPoolName: the request pool name. If the pool does not exist, it will be created. If null, the default pool is used. 
		*/
		virtual bool AppendURLRequest(ParaEngine::CURLRequestTask* pUrlTask, const char* sPoolName = NULL) = 0;

		/**
		* There is generally no limit to the number of requests sent. However, each pool has a specified maximum number of concurrent worker slots. 
		*  the default number is 1. One can change this number with this function. 
		*/
		virtual bool ChangeRequestPoolSize(const char* sPoolName, int nCount) = 0;


		//////////////////////////////////////////////////////////////////////////
		//
		// Downloader functions
		//
		//////////////////////////////////////////////////////////////////////////

		/**
		* Asynchronously download a file from the url.
		* @param callbackScript: script code to be called, a global variable called msg is assigned, as below
		*  msg = {DownloadState=""|"complete"|"terminated", totalFileSize=number, currentFileSize=number, PercentDone=number}
		*/
		virtual void AsyncDownload(const char* url, const char* destFolder, const char* callbackScript, const char* DownloaderName) = 0;


		/**
		* cancel all asynchronous downloads that matches a certain downloader name pattern
		* @param DownloaderName:regular expression. such as "proc1", "proc1.*", ".*"
		*/
		virtual void CancelDownload(const char* DownloaderName) = 0;

		/**
		* Synchronous call of the function AsyncDownload(). This function will not return until download is complete or an error occurs. 
		* this function is rarely used. AsyncDownload() is used. 
		* @return:1 if succeed, 0 if fail
		*/
		virtual int Download(const char* url, const char* destFolder, const char* callbackScript, const char* DownloaderName) = 0;

		/**
		* add a DNS server record to the current NPL runtime.
		* DNS server record is a mapping from name to (IP:port)
		* if one maps several IP:port to the same name, the former ones will be overridden.
		* @param sDNSName: the DNS server name. the DNS name "_world" is used for the current 
		*	world DNS server. It is commonly used as a DNS reference to the current world that 
		*	the user is exploring.
		* @param sAddress: "IP:port". e.g. "192.168.1.10:4000"
		*/
		virtual void NPL_AddDNSRecord(const char * sDNSName, const char* sAddress) = 0;

		/**
		* Set the default channel ID, default value is 0. Default channel is used when NPL.activate() call¡¯s does not contain the channel property.
		* @param channel_ID It can be a number in [0,15].default is 0
		*/
		virtual void NPL_SetDefaultChannel(int channel_ID) = 0;
		/**
		* Get the default channel ID, default value is 0. Default channel is used when NPL.activate() call¡¯s does not contain the channel property.
		* @return channel_ID It can be a number in [0,15].default is 0
		*/
		virtual int NPL_GetDefaultChannel() = 0;

		/**
		* Messages can be sent via predefined channels. There are 16 channels from 0 to 15 to be used. 0 is the default channel. 
		* This method sets the channel property for a given channel. The default channel property is given in table.
		The following table shows the default NPL channel properties. It is advised for users to stick to this default mapping when developing their own applications. 
		Table 1. 	Default NPL channel properties
		channel_ID	Priority	Reliability				Usage
		0		med			RELIABLE_ORDERED		System message
		1		med			UNRELIABLE_SEQUENCED	Character positions
		2		med			RELIABLE_ORDERED		Large Simulation Object transmission, such as terrain height field.
		4		med			RELIABLE_ORDERED		Chat message
		14		med			RELIABLE				files transmission and advertisement
		15		med			RELIABLE_SEQUENCED		Voice transmission
		11-15	med			RELIABLE_ORDERED	

		* @param channel_ID 
		* @param priority 
		* @param reliability 
		*/
		virtual void NPL_SetChannelProperty(int channel_ID, int priority, int reliability) = 0;
		/**
		* reset all 16 predefined channel properties. according to table1. Default NPL channel properties. see also NPL_SetChannelProperty
		The following table shows the default NPL channel properties. It is advised for users to stick to this default mapping when developing their own applications. 
		Table 1. 	Default NPL channel properties
		channel_ID	Priority	Reliability				Usage
		0		med			RELIABLE_ORDERED		System message
		1		med			UNRELIABLE_SEQUENCED	Character positions
		2		med			RELIABLE_ORDERED		Large Simulation Object transmission, such as terrain height field.
		4		med			RELIABLE_ORDERED		Chat message
		14		med			RELIABLE				files transmission and advertisement
		15		med			RELIABLE_SEQUENCED		Voice transmission
		11-15	med			RELIABLE_ORDERED	
		*/
		virtual void NPL_ResetChannelProperties() = 0;

		/**
		* see also NPL_SetChannelProperty
		* @param channel_ID 
		* @param priority [out]
		* @param reliability [out]
		*/
		virtual void NPL_GetChannelProperty(int channel_ID, int* priority, int* reliability) = 0;

		//////////////////////////////////////////////////////////////////////////
		//
		// Global activation functions
		//
		//////////////////////////////////////////////////////////////////////////

		/**
		* activate the specified file. It can either be local or remote file. 
		* [thread safe] This function is thread safe, if and only if pRuntimeState is still valid
		* @param pRuntimeState: the source runtime state that initiated this activation. If pState is NULL, the main runtime state is used.
		* @param sNPLFileName: 
		* a globally unique name of a NPL file name instance. 
		* The string format of an NPL file name is like below. 
		* [(sRuntimeStateName|gl)][sNID:]sRelativePath[@sDNSServerName]
		* 
		* the following is a list of all valid file name combinations: 
		*	"user001@paraengine.com:script/hello.lua"	-- a file of user001 in its default gaming thread
		*	"(world1)server001@paraengine.com:script/hello.lua"		-- a file of server001 in its thread world1
		*	"(worker1)script/hello.lua"			-- a local file in the thread worker1
		*	"(gl)script/hello.lua"			-- a glia (local) file in the current runtime state's thread
		*	"script/hello.lua"			-- a file in the current thread. For a single threaded application, this is usually enough.
		* 
		* @param code: it is a chunk of pure data table init code that would be transmitted to the destination file. 
		* @param nLength: the code length. if this is 0, length is determined from code by finding '\0', 
		*	but, it must not exceed 4096 bytes. If length is explicitly specified, there is no such a limit. 
		* @param channel:On which channel to send the package. It can be a number in [0,15]. In case it is nil, the default channel (0) is used. 
		* @param priority: From high to low.If this is nil, medium priority(0) is used. 
		following enumerations are used to describe when packets are delivered.
		enum PacketPriority
		{
		SYSTEM_PRIORITY,   /// internally Used to send above-high priority messages.
		HIGH_PRIORITY,   /// High priority messages are send before medium priority messages.
		MEDIUM_PRIORITY,   /// Medium priority messages are send before low priority messages.
		LOW_PRIORITY,   /// Low priority messages are only sent when no other messages are waiting.
		};
		* @param reliability:From unreliable to reliable sequenced. 0 stands for unreliable. If this is nil, RELIABLE_ORDERED(3) is used. 
		following enumerations are used to describe how packets are delivered.
		enum PacketReliability
		{
		UNRELIABLE,   /// Same as regular UDP, except that it will also discard duplicate datagrams.  It adds (6 to 17) + 21 bits of overhead, 16 of which is used to detect duplicate packets and 6 to 17 of which is used for message length.
		UNRELIABLE_SEQUENCED,  /// Regular UDP with a sequence counter.  Out of order messages will be discarded.  This adds an additional 13 bits on top what is used for UNRELIABLE.
		RELIABLE,   /// The message is sent reliably, but not necessarily in any order.  Same overhead as UNRELIABLE.
		RELIABLE_ORDERED,   /// This message is reliable and will arrive in the order you sent it.  Messages will be delayed while waiting for out of order messages.  Same overhead as UNRELIABLE_SEQUENCED.
		RELIABLE_SEQUENCED /// This message is reliable and will arrive in the sequence you sent it.  Out or order messages will be dropped.  Same overhead as UNRELIABLE_SEQUENCED.
		};
		* @note: pure data table is defined as table consisting of only string, number and other table of the above type. 
		*   NPL.activate function also accepts ParaFileObject typed message data type. ParaFileObject will be converted to base64 string upon transmission. There are size limit though of 10MB.
		*   one can also programmatically check whether a script object is pure date by calling NPL.SerializeToSCode() function. Please note that data types that is not pure data in sCode will be ignored instead of reporting an error.
		*/
		virtual int Activate(INPLRuntimeState* pRuntimeState, const char * sNeuronFile, const char * code = NULL,int nLength=0,  int channel=0, int priority=2, int reliability=3) = 0;
	};
}

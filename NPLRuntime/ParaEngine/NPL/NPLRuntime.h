#pragma once
#include <set>
struct lua_State;

// include NPLScriptingState
#include "ParaScripting.h"
#include "NPLRuntimeState.h"
#include "INPLRuntime.h"
/* internal data structure used by NPL runtime */
#include "NPLCommon.h"
#include "IAttributeFields.h"

#include <boost/scoped_ptr.hpp>

#ifndef EMSCRIPTEN_SINGLE_THREAD
#include <boost/thread/shared_mutex.hpp>
#endif 

namespace ParaEngine
{
	class INPLWebServiceClient;
	class INPLJabberClient;
#ifndef EMSCRIPTEN_SINGLE_THREAD
	class CNPLNetClient;
#endif
	class CURLRequestTask;
}

/**
* @ingroup NPL
* Neural Parallel Language Runtime environment implementation is in this namespace
*/
namespace NPL
{
	class CNPLNetServer;
	class CNPLNetUDPServer;
	using namespace ParaScripting;
	class INPLStimulationPipe;
	class CNPLNameSpaceBinding;
	struct NPLPacket;


	/**
	* default channel property
	*/
	struct ChannelProperty
	{
	public:
		int m_nPriority;
		int m_nReliability;
		/**
		* default to m_nPriority = NPL::MEDIUM_PRIORITY, m_nReliability = NPL::RELIABLE_ORDERED
		*/
		ChannelProperty() :m_nPriority(2), m_nReliability(3) {};
		ChannelProperty(int nPriority, int nReliability) :m_nPriority(nPriority), m_nReliability(nReliability) {};

	public:
		/** set properties */
		void Set(int nPriority, int nReliability)
		{
			m_nPriority = nPriority;
			m_nReliability = nReliability;
		};
	};

	/**
	* CNPLRuntime is a wrapper of the NPL application programming interface (NPL API).
	* it is normally used as a singleton class.
	*/
	class CNPLRuntime : public ParaEngine::IAttributeFields, public INPLRuntime
	{
	public:
		CNPLRuntime();
		virtual ~CNPLRuntime(void);

		/** the singleton instance */
		static CNPLRuntime* GetInstance();
		/** if one wants to change the default runtime state, such as using external luastate, one need to call this function before GetInstance(). */
		static void SetDefaultNPLStateType(NPL::NPLRuntimeStateType val);

		/** initialize NPL runtime environment */
		virtual void Init();

		/**
		* call this function regularly, to process packages.
		* @param bToEnd: if true, the function will only return when there is no more input packages in the queue
		*/
		virtual void Run(bool bToEnd = true);

		/** clean up the NPL runtime environment */
		virtual void Cleanup();

		//////////////////////////////////////////////////////////////////////////
		// implementation of IAttributeFields

		/** attribute class ID should be identical, unless one knows how overriding rules work.*/
		virtual int GetAttributeClassID() { return ParaEngine::ATTRIBUTE_CLASSID_CNPLRuntime; }
		/** a static string, describing the attribute class object's name */
		virtual const char* GetAttributeClassName() { static const char name[] = "CNPLRuntime"; return name; }
		/** a static string, describing the attribute class object */
		virtual const char* GetAttributeClassDescription() { static const char desc[] = ""; return desc; }
		/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		virtual int InstallFields(ParaEngine::CAttributeClass* pClass, bool bOverride);
		/** get the number of child objects (row count) in the given column. please note different columns can have different row count. */
		virtual int GetChildAttributeObjectCount(int nColumnIndex = 0);
		virtual ParaEngine::IAttributeFields* GetChildAttributeObject(int nRowIndex, int nColumnIndex = 0);

		ATTRIBUTE_METHOD1(CNPLRuntime, IsServerStarted_s, bool*) { *p1 = cls->IsServerStarted(); return S_OK; }
		ATTRIBUTE_METHOD1(CNPLRuntime, GetHostIP_s, const char**) { *p1 = cls->GetHostIP().c_str(); return S_OK; }
		ATTRIBUTE_METHOD1(CNPLRuntime, GetHostPort_s, const char**) { *p1 = cls->GetHostPort().c_str(); return S_OK; }

		ATTRIBUTE_METHOD1(CNPLRuntime, IsUDPServerStarted_s, bool*) { *p1 = cls->IsUDPServerStarted(); return S_OK; }
		ATTRIBUTE_METHOD1(CNPLRuntime, GetUDPHostIP_s, const char**) { *p1 = cls->GetUDPHostIP().c_str(); return S_OK; }
		ATTRIBUTE_METHOD1(CNPLRuntime, GetUDPHostPort_s, int*) { *p1 = cls->GetUDPHostPort(); return S_OK; }

		ATTRIBUTE_METHOD1(CNPLRuntime, IsTCPKeepAliveEnabled_s, bool*) { *p1 = cls->IsTCPKeepAliveEnabled(); return S_OK; }
		ATTRIBUTE_METHOD1(CNPLRuntime, SetTCPKeepAlive_s, bool) { cls->SetTCPKeepAlive(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CNPLRuntime, IsTCPNoDelay_s, bool*) { *p1 = cls->IsTCPNoDelay(); return S_OK; }
		ATTRIBUTE_METHOD1(CNPLRuntime, SetTCPNoDelay_s, bool) { cls->SetTCPNoDelay(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CNPLRuntime, IsKeepAliveEnabled_s, bool*) { *p1 = cls->IsKeepAliveEnabled(); return S_OK; }
		ATTRIBUTE_METHOD1(CNPLRuntime, SetKeepAlive_s, bool) { cls->SetKeepAlive(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CNPLRuntime, IsIdleTimeoutEnabled_s, bool*) { *p1 = cls->IsIdleTimeoutEnabled(); return S_OK; }
		ATTRIBUTE_METHOD1(CNPLRuntime, EnableIdleTimeout_s, bool) { cls->EnableIdleTimeout(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CNPLRuntime, IsUDPIdleTimeoutEnabled_s, bool*) { *p1 = cls->IsUDPIdleTimeoutEnabled(); return S_OK; }
		ATTRIBUTE_METHOD1(CNPLRuntime, EnableUDPIdleTimeout_s, bool) { cls->EnableUDPIdleTimeout(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CNPLRuntime, SetUDPUseCompression_s, bool) { cls->SetUDPUseCompression(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CNPLRuntime, GetIdleTimeoutPeriod_s, int*) { *p1 = cls->GetIdleTimeoutPeriod(); return S_OK; }
		ATTRIBUTE_METHOD1(CNPLRuntime, SetIdleTimeoutPeriod_s, int) { cls->SetIdleTimeoutPeriod(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CNPLRuntime, GetUDPIdleTimeoutPeriod_s, int*) { *p1 = cls->GetUDPIdleTimeoutPeriod(); return S_OK; }
		ATTRIBUTE_METHOD1(CNPLRuntime, SetUDPIdleTimeoutPeriod_s, int) { cls->SetUDPIdleTimeoutPeriod(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CNPLRuntime, GetCompressionLevel_s, int*) { *p1 = cls->GetCompressionLevel(); return S_OK; }
		ATTRIBUTE_METHOD1(CNPLRuntime, SetCompressionLevel_s, int) { cls->SetCompressionLevel(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CNPLRuntime, GetUDPCompressionLevel_s, int*) { *p1 = cls->GetUDPCompressionLevel(); return S_OK; }
		ATTRIBUTE_METHOD1(CNPLRuntime, SetUDPCompressionLevel_s, int) { cls->SetUDPCompressionLevel(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CNPLRuntime, GetCompressionThreshold_s, int*) { *p1 = cls->GetCompressionThreshold(); return S_OK; }
		ATTRIBUTE_METHOD1(CNPLRuntime, SetCompressionThreshold_s, int) { cls->SetCompressionThreshold(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CNPLRuntime, GetUDPCompressionThreshold_s, int*) { *p1 = cls->GetUDPCompressionThreshold(); return S_OK; }
		ATTRIBUTE_METHOD1(CNPLRuntime, SetUDPCompressionThreshold_s, int) { cls->SetUDPCompressionThreshold(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CNPLRuntime, IsAnsiMode_s, bool*) { *p1 = cls->IsAnsiMode(); return S_OK; }
		ATTRIBUTE_METHOD1(CNPLRuntime, EnableAnsiMode_s, bool) { cls->EnableAnsiMode(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CNPLRuntime, GetMaxPendingConnections_s, int*) { *p1 = cls->GetMaxPendingConnections(); return S_OK; }
		ATTRIBUTE_METHOD1(CNPLRuntime, SetMaxPendingConnections_s, int) { cls->SetMaxPendingConnections(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CNPLRuntime, GetLogLevel_s, int*) { *p1 = cls->GetLogLevel(); return S_OK; }
		ATTRIBUTE_METHOD1(CNPLRuntime, SetLogLevel_s, int) { cls->SetLogLevel(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CNPLRuntime, EnableUDPServer_s, int) { cls->NPL_StartNetUDPServer(nullptr, p1); return S_OK; }
		ATTRIBUTE_METHOD(CNPLRuntime, DisableUDPServer_s) { cls->NPL_StopNetUDPServer(); return S_OK; }

		ATTRIBUTE_METHOD1(CNPLRuntime, GetExternalIPList_s, const char**) { *p1 = CNPLRuntime::GetExternalIPList().c_str(); return S_OK; }
		ATTRIBUTE_METHOD1(CNPLRuntime, GetBroadcastAddressList_s, const char**) { *p1 = CNPLRuntime::GetBroadcastAddressList().c_str(); return S_OK; }
	public:
		/** whether to use compression on transport layer for incoming and outgoing connections
		* @param bCompressIncoming: if true, compression is used for all incoming connections. default to false.
		* @param bCompressIncoming: if true, compression is used for all outgoing connections. default to false.
		*/
		virtual void SetUseCompression(bool bCompressIncoming, bool bCompressOutgoing);
		void SetUDPUseCompression(bool bCompress);


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
		virtual void SetCompressionKey(const unsigned char* sKey = 0, int nSize = 0, int nUsePlainTextEncoding = 0);

		/** Set the zlib compression level to use in case compression is enabled.
		* default to 0, which means no compression. Compression level, which is an integer in the range of -1 to 9.
		* Lower compression levels result in faster execution, but less compression. Higher levels result in greater compression,
		* but slower execution. The zlib constant Z_DEFAULT_COMPRESSION, equal to -1, provides a good compromise between compression
		* and speed and is equivalent to level 6. Level 0 actually does no compression at all, and in fact expands the data slightly
		* to produce the zlib format (it is not a byte-for-byte copy of the input).
		*/
		virtual void SetCompressionLevel(int nLevel);
		virtual int GetCompressionLevel();
		void SetUDPCompressionLevel(int nLevel);
		int GetUDPCompressionLevel();

		/** set the default compression threshold for all connections on this machine.
		* when the message size is bigger than this number of bytes, we will use m_nCompressionLevel for compression.
		* For message smaller than the threshold, we will not compress even m_nCompressionLevel is not 0.
		*/
		virtual void SetCompressionThreshold(int nThreshold);
		virtual int GetCompressionThreshold();
		void SetUDPCompressionThreshold(int nThreshold);
		int GetUDPCompressionThreshold();

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
		virtual void SetTCPKeepAlive(bool bEnable);

		/** whether SO_KEEPALIVE is enabled.
		* @return bEnable: true to enable.
		*/
		virtual bool IsTCPKeepAliveEnabled();

		/** enable application level keep alive. we will use a global idle timer to detect if a connection has been inactive for GetIdleTimeoutPeriod(),
		* if so, we may send the keep alive message.
		* @param bEnable: enable keep alive will automatically enable EnableIdleTimeout()
		*/
		virtual void SetKeepAlive(bool bEnable);
		virtual bool IsKeepAliveEnabled();

		/** Enable idle timeout. This is the application level timeout setting.
		* We will create a global timer which examines all send/receive time of all open connections, if a
		* connection is inactive (idle for GetIdleTimeoutPeriod()) we will
		*	- if IsKeepAliveEnabled() is false, actively close the connection. This is the method used by HTTP, which is the only solution to detect broken connection without sending additional keep alive message.
		*   - if IsKeepAliveEnabled() is true, send an empty message to the other end (keep alive messages) to more accurately detect dead connections (see SetKeepAlive).
		*/
		virtual void EnableIdleTimeout(bool bEnable);
		virtual bool IsIdleTimeoutEnabled();
		void EnableUDPIdleTimeout(bool bEnable);
		bool IsUDPIdleTimeoutEnabled();

		/** how many milliseconds of inactivity to assume this connection should be timed out. if 0 it is never timed out. */
		virtual void SetIdleTimeoutPeriod(int nMilliseconds);
		virtual int GetIdleTimeoutPeriod();
		void SetUDPIdleTimeoutPeriod(int nMilliseconds);
		int GetUDPIdleTimeoutPeriod();

		/** whether the first line of the NPL protocol is in ansi code page.
		because NPL protocol is compatible with HTTP protocol in ansi mode, some interception web cache servers may cache request even the port number is not 80,
		so client side applications are encouraged to disable ansi mode. */
		virtual void EnableAnsiMode(bool bEnable);
		virtual bool IsAnsiMode();

		/** queue size of the server acceptor's queue. */
		virtual int GetMaxPendingConnections();
		virtual void SetMaxPendingConnections(int val);


		/** get the host port of this NPL runtime */
		virtual const std::string& GetHostPort();
		/** get the host IP of this NPL runtime */
		virtual const std::string& GetHostIP();
		/** whether the NPL runtime's http server is started. */
		virtual bool IsServerStarted();

		/** get the host port of this NPL runtime */
		unsigned short GetUDPHostPort();
		/** get the host IP of this NPL runtime */
		const std::string& GetUDPHostIP();
		/** whether the NPL runtime's udp server is started. */
		bool IsUDPServerStarted();

		void SetTCPNoDelay(bool bEnable);
		bool IsTCPNoDelay();

		/** get extern IP address lsit of this computer. use ',' to separate */
		static const std::string& GetExternalIPList();
		/* get broadcast address lsit of this computer. use ',' to separate */
		static const std::string& GetBroadcastAddressList();

		//////////////////////////////////////////////////////////////////////////
		//
		// NPL core functions
		//
		//////////////////////////////////////////////////////////////////////////

		/**
		* load a file (in the specified runtime state) without running it. If the file is already loaded,
		* it will not be loaded again unless bReload is true.
		* IMPORTANT: this function is synchronous; unlike the asynchronous activation function.
		* LoadFile is more like "include in C++".When the function returns, contents in the file is loaded to memory.
		* For source file: when loading an NPL file, we will first find if there is an up to date compiled version in the bin directory. if there is,
		* we will load the compiled version, otherwise we will use the text version.  use bin version, if source version does not exist; use bin version, if source and bin versions are both on disk (instead of zip) and that bin version is newer than the source version.
		* e.g. we can compile source to bin directory with file extension ".o", e.g. "script/abc.lua" can be compiled to "bin/script/abc.o", The latter will be used if available and up-to-date.
		* @note: This function must be called in the same thread that hosts the specified runtime state (pState).
		* @param pState: In which runtime state to load the file. If pState is NULL, the main runtime state is used.
		* @param filePath: the local relative file path. If the file extension is ".dll", it will be treated as a plug-in. if the filepath is "*.dll", it means all DLLs in that directory.
		* @param bReload: if true, the file will be reloaded even if it is already loaded.
		*    otherwise, the file will only be loaded if it is not loaded yet.
		* @remark: one should be very careful when calling with bReload set to true, since this may lead to recursive
		*	reloading of the same file. If this occurs, it will generate C Stack overflow error message.
		*/
		void NPL_LoadFile(NPLRuntimeState_ptr runtime_state, const char* filePath, bool bReload);

		/**
		* activate the specified file. It can either be local or remote file.
		*
		* @param pState: the source runtime state that initiated this activation. If pState is NULL, the main runtime state is used.
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
		int NPL_Activate(NPLRuntimeState_ptr runtime_state, const char * sNeuronFile, const char * code = NULL, int nLength = 0, int channel = 0, int priority = 2, int reliability = 3);

		/** same as NPL_Activate */
		virtual int Activate(INPLRuntimeState* pRuntimeState, const char * sNeuronFile, const char * code = NULL, int nLength = 0, int channel = 0, int priority = 2, int reliability = 3);

		/** activate immediately for the local file (may be in different thread).
		*@return 0 or the last return value from code or neuron file
		*/
		virtual int ActivateLocalNow(const char * sNeuronFile, const char * code = NULL, int nLength = 0);

		//////////////////////////////////////////////////////////////////////////
		//
		// NPL Net Server functions
		//
		//////////////////////////////////////////////////////////////////////////

		/**
		* start the NPL net server's io_service loop. This function returns immediately. it will spawn the accept and dispatcher thread.
		* call this function only once per process.
		* @param server: default to "127.0.0.1"
		* @param port: default to "60001"
		*/
		void NPL_StartNetServer(const char* server = NULL, const char* port = NULL);
		///
		void NPL_StartNetUDPServer(const char* server = nullptr, unsigned short port = 8099);

		/** stop the net server */
		void NPL_StopNetServer();
		void NPL_StopNetUDPServer();

		/** add a nID, filename pair to the public file list.
		* we only allow remote NPL runtime to activate files in the public file list.
		* Each public file has a user defined ID. The NPL network layer always try to use its ID for transmission to minimize bandwidth.
		* There are some negotiations between sender and receiver to sync the string to ID map before they use it.
		* [thread safe]
		* @param nID: the integer to encode the string. it is usually small and positive number.
		* @param sString: the string for the id. if input is empty, it means removing the mapping of nID.
		*/
		void NPL_AddPublicFile(const string& filename, int nID);

		/** clear all public files, so that the NPL server will be completely private.
		* [thread safe]
		*/
		void NPL_ClearPublicFiles();

		/** get the ip address of given NPL connection.
		* this function is usually called by the server for connected clients.
		* @param nid: nid or tid.
		* @return: the ip address in dot format. empty string is returned if connection can not be found.
		*/
		string NPL_GetIP(const char* nid);


		/** accept a given connection. The connection will be regarded as authenticated once accepted.
		* [thread safe]
		* @param tid: the temporary id or NID of the connection to be accepted. usually it is from msg.tid or msg.nid.
		* @param nid: if this is not nil, tid will be renamed to nid after accepted.
		*/
		void NPL_accept(const char* tid, const char* nid = NULL);

		/** set transmission protocol, default value is 0. */
		void NPL_SetProtocol(const char* nid, int protocolType = 0);

		/** reject and close a given connection. The connection will be closed once rejected.
		* [thread safe]
		* @param nid: the temporary id or NID of the connection to be rejected. usually it is from msg.tid or msg.nid.
		* @param nReason: default to 0.
		* - 0 or positive value is forcibly reset/disconnect (it will abort pending read/write immediately).
		* - 1 is another user with same nid is authenticated. The server should send a message to tell the client about this.
		* - -1 or negative value means gracefully close the connection when all pending data has been sent.
		*/
		void NPL_reject(const char* nid, int nReason = 0);

		virtual void StartNetServer(const char* server = NULL, const char* port = NULL);
		virtual void StopNetServer();
		virtual void AddPublicFile(const string& filename, int nID);
		virtual void ClearPublicFiles();
		virtual void GetIP(const char* nid, char* pOutput);
		virtual void accept(const char* tid, const char* nid = NULL);
		virtual void reject(const char* nid, int nReason = 0);

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
		virtual ParaEngine::INPLJabberClient* GetJabberClient(const char* sJID);
		/**
		* Create a new jabber client instance with the given jabber client ID. It does not open a connection immediately.
		* @param sJID: such as "lixizhi@paraweb3d.com"
		*/
		virtual ParaEngine::INPLJabberClient* CreateJabberClient(const char* sJID);

		/**
		* close a given jabber client instance. Basically, there is no need to close a web service,
		* unless one wants to reopen it with different credentials
		* @param sJID: such as "lixizhi@paraweb3d.com"
		*/
		virtual bool CloseJabberClient(const char* sJID);

		//////////////////////////////////////////////////////////////////////////
		//
		// new libcUrl interface. 
		//
		//////////////////////////////////////////////////////////////////////////

		/** Append URL request to a pool.
		* @param pUrlTask: must be new CURLRequestTask(), the ownership of the task is transfered to the manager. so the caller should never delete the pointer.
		* @param sPoolName: the request pool name. If the pool does not exist, it will be created. If null, the default pool is used.
		*/
		virtual bool AppendURLRequest(ParaEngine::CURLRequestTask* pUrlTask, const char* sPoolName = NULL);

		/**
		* There is generally no limit to the number of requests sent. However, each pool has a specified maximum number of concurrent worker slots.
		*  the default number is 1. One can change this number with this function.
		*/
		virtual bool ChangeRequestPoolSize(const char* sPoolName, int nCount);


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
		virtual void AsyncDownload(const char* url, const char* destFolder, const char* callbackScript, const char* DownloaderName);


		/**
		* cancel all asynchronous downloads that matches a certain downloader name pattern
		* @param DownloaderName:regular expression. such as "proc1", "proc1.*", ".*"
		*/
		virtual void CancelDownload(const char* DownloaderName);

		/**
		* Synchronous call of the function AsyncDownload(). This function will not return until download is complete or an error occurs.
		* this function is rarely used. AsyncDownload() is used.
		* @return:1 if succeed, 0 if fail
		*/
		virtual int Download(const char* url, const char* destFolder, const char* callbackScript, const char* DownloaderName);


		//////////////////////////////////////////////////////////////////////////
		//
		// Web Service functions
		//
		//////////////////////////////////////////////////////////////////////////

		/**
		* this method will associate a web service (method) with either a sCode, which will be called when the web service returned.
		The returned message, if any, will be passed via a global parameter called msg. For example:
		Function callbackFunc1()
		If(msg.username~=nil) then end
		End
		NPL.RegisterWSCallBack("http://paraengine.com/login.aspx",callbackFunc1);
		NPL.activate("http://paraengine.com/login.aspx", {username=��lxz��});
		* @param sWebServiceFile URL of the web service
		* @param sCode code to be executed when the web service is called. When a two-way web service call is invoked;
		*  it internally will create a thread for the returning message. Please refer to .Net 3.0 network communication architecture.
		*/
		void NPL_RegisterWSCallBack(const char * sWebServiceFile, const char * sCode);
		/**
		* unregister web service call back for a given file.
		* @param sWebServiceFile if this is nil or "", all call backs will be unregistered.
		*/
		void NPL_UnregisterWSCallBack(const char * sWebServiceFile);

		//////////////////////////////////////////////////////////////////////////
		//
		// Timer functions
		//
		//////////////////////////////////////////////////////////////////////////

		/** creates a timer with the specified time-out value
		* [thread safe]
		* @param runtime_state: in which runtime state the timer is. Timer ID is unique only within a given runtime state.
		* @param nIDEvent: Specifies a positive timer identifier. For nIDEvent<=0, they are reserved for internal uses.
		* If the NPL runtime already has a timer with the value nIDEvent,
		* then the existing timer is replaced by the new timer. When SetTimer replaces a timer, the timer is reset.
		* @param fElapse: Specifies the time-out value, in seconds. Please note that a timer will not be repeatedly activated if
		*		its timeout is shorter than the frame rate of the NPL simulation pipeline .
		* @param sNeuronFile: The NPL file to be activated when the time-out value elapses. For more information about the file name
		*  See NPL.activate().
		* @return: true if succeeds.An application can pass the value of the nIDEvent parameter to the NPL.KillTimer function to destroy the timer.
		*/
		bool NPL_SetTimer(NPLRuntimeState_ptr runtime_state, int nIDEvent, float fElapse, const char* sNeuronFile);

		/**
		* Destroys the specified timer
		* [thread safe]
		* @param runtime_state: in which runtime state the timer is. Timer ID is unique only within a given runtime state.
		* @param nIDEvent: Specifies the timer to be destroyed.For nIDEvent<=0, they are reserved for internal uses can not be killed by this function.
		* This value must be the same as the nIDEvent value passed to the SetTimer function that created the timer.
		* @return : If the function succeeds, the return value is true
		*/
		bool NPL_KillTimer(NPLRuntimeState_ptr runtime_state, int nIDEvent);

		/**
		* Changes the start time and the interval between method invocations for a timer, using 32-bit signed integers to measure time intervals.
		* [thread safe]
		* @param runtime_state: in which runtime state the timer is. Timer ID is unique only within a given runtime state.
		* @param nIDEvent: Specifies the timer to be destroyed.For nIDEvent<=0, they are reserved for internal uses can not be killed by this function.
		* This value must be the same as the nIDEvent value passed to the SetTimer function that created the timer.
		* @param dueTime: The amount of time to delay before the invoking the callback method specified when the Timer was constructed, in milliseconds. Specify zero (0) to restart the timer immediately.
		*  however, the current implementation does not accept dueTime that is larger than MAX_TIMER_DUE_TIME	10000000, which is 10000 seconds.
		* @param period:The time interval between invocations of the callback method specified when the Timer was constructed, in milliseconds.
		* @return : If the function succeeds, the return value is true
		*/
		bool NPL_ChangeTimer(NPLRuntimeState_ptr runtime_state, int nIDEvent, int dueTime, int period);

		//////////////////////////////////////////////////////////////////////////
		//
		// UDP networking functions (RakNet in NPL network layer, currently obsoleted)
		//
		//////////////////////////////////////////////////////////////////////////

		/// Get the network layer pipe
		/// @return: INPLStimulationPipe* m_pNetPipe
		static INPLStimulationPipe* GetNetworkLayerPipe();

		/**
		* add a DNS server record to the current NPL runtime.
		* DNS server record is a mapping from name to (IP:port)
		* if one maps several IP:port to the same name, the former ones will be overridden.
		* @param sDNSName: the DNS server name. the DNS name "_world" is used for the current
		*	world DNS server. It is commonly used as a DNS reference to the current world that
		*	the user is exploring.
		* @param sAddress: "IP:port". e.g. "192.168.1.10:4000"
		*/
		virtual void NPL_AddDNSRecord(const char * sDNSName, const char* sAddress);

		/**
		* Set the default channel ID, default value is 0. Default channel is used when NPL.activate() call��s does not contain the channel property.
		* @param channel_ID It can be a number in [0,15].default is 0
		*/
		virtual void NPL_SetDefaultChannel(int channel_ID);
		/**
		* Get the default channel ID, default value is 0. Default channel is used when NPL.activate() call��s does not contain the channel property.
		* @return channel_ID It can be a number in [0,15].default is 0
		*/
		virtual int NPL_GetDefaultChannel();

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
		virtual void NPL_SetChannelProperty(int channel_ID, int priority, int reliability);
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
		virtual void NPL_ResetChannelProperties();

		/**
		* see also NPL_SetChannelProperty
		* @param channel_ID
		* @param priority [out]
		* @param reliability [out]
		*/
		virtual void NPL_GetChannelProperty(int channel_ID, int* priority, int* reliability);

		///
		static int NPL_Ping(const char* host, const char* port, unsigned int waitTime, bool bTcp);

		/**
		* OBSOLETED: Enable the network, by default the network layer is disabled.
		* calling this function multiple time with different CenterName will restart the network layer with a different center name.
		* @param bEnable true to enable, false to disable.If this is false, the CenterName and Password are ignored.
		* @param CenterName the local nerve center name. it is also the user name
		*  which local receptor will use in the credentials to login in other NPL runtime.
		* @param Password
		* @return true if succeeded.
		*/
		void NPL_EnableNetwork(bool bEnable, const char* CenterName, const char* password);

		/** get the current activation's source name. Each NPL activation has a source name which started the activation.
		* This is usually "" for local activation and some kind of "name@server" for network activation.*/
		const string& NPL_GetSourceName();
		/**
		* Set the current activation's source name. Each NPL activation has a source name which started the activation.
		* This function is called automatically when a new activation occurs.So only call this function if one wants to override the old one for special code logics.
		* @param sName This is usually "" for local activation and some kind of "name@server" for network activation.
		*/
		void NPL_SetSourceName(const char* sName);

		/* default to 1, set to 0 to silence some connection verbose log. */
		int GetLogLevel() const;

		/* default to 1, set to 0 to silence some connection verbose log. */
		void SetLogLevel(int val);

	private: // methods
			 /** load the web service plug-in. */
		bool LoadWebServicePlugin();

		/**
		* .Net web service file
		* @param sWebserviceFile URL such as "http://paraengine.com/login.aspx"
		* @param code
		*/
		void STI_DoWebServiceFile(const char* sWebserviceFile, const char * code);

		/** from NPL::PacketPriority to internal priority */
		int TranslatePriorityValue(int priority);
	public:
		virtual INPLRuntimeState* CreateState(const char* name, NPLRuntimeStateType type_ = NPLRuntimeStateType_NPL);
		virtual INPLRuntimeState* GetState(const char* name);
		virtual INPLRuntimeState* CreateGetState(const char* name, NPLRuntimeStateType type_ = NPLRuntimeStateType_NPL);
		virtual bool DeleteState(INPLRuntimeState* pRuntime_state);
		virtual INPLRuntimeState* GetMainState();
		virtual bool AddToMainThread(INPLRuntimeState* runtime_state);

		/** create a new runtime state.
		* this function is thread safe
		* @param name: if "", it is an anonymous runtime state. otherwise it should be a unique name.
		* @param type_: the runtime state type.
		* @return the newly created state is returned. If an runtime state with the same non-empty name already exist, the old one is returned.
		*/
		NPLRuntimeState_ptr CreateRuntimeState(const string& name, NPLRuntimeStateType type_ = NPLRuntimeStateType_NPL);

		/** get a runtime state with an explicit name.
		* this function is thread safe
		* @param name: the name of the runtime state. if empty or "main", the main runtime state is returned.
		*/
		NPLRuntimeState_ptr GetRuntimeState(const string& name);

		/** it get runtime state first, if none exist, it will create one and add it to the main threaded state */
		NPLRuntimeState_ptr CreateGetRuntimeState(const string& name, NPLRuntimeStateType type_ = NPLRuntimeStateType_NPL);

		/** create a given runtime state.
		* this function is thread safe */
		bool DeleteRuntimeState(NPLRuntimeState_ptr runtime_state);

		/** get the default (main) runtime state.*/
		NPLRuntimeState_ptr GetMainRuntimeState();

		/** add a given runtime state to the main game thread.
		* this function is thread safe
		*/
		bool AddStateToMainThread(NPLRuntimeState_ptr runtime_state);

		/** remove a given runtime state from the main game thread.
		* this function is thread safe
		*/
		bool RemoveStateFromMainThread(NPLRuntimeState_ptr runtime_state);

		/** get the NPL net server */
		CNPLNetServer* GetNetServer() { return m_net_server.get(); };
		CNPLNetUDPServer* GetNetUDPServer() { return m_net_udp_server.get(); };

#ifndef EMSCRIPTEN_SINGLE_THREAD
		/** get the Net client implementation.*/
		ParaEngine::CNPLNetClient* GetNetClient();
#endif
		/** whether we will process messages in the main threads in the frame move function.
		* It is default to true;
		* However, it is possible for server to set to false, if one wants to have a more responsive main state on the server.
		* For example, it does high-frequency dispatcher jobs, instead of monitoring.
		* But client application, it is only advised to set to true, otherwise the scripting and render modules will be run in different threads, leading to complexity and bugs.
		* @NOTE: One can only call this function once to set to false. This function is only used by the ParaEngineServer
		*/
		void SetHostMainStatesInFrameMove(bool bHostMainStatesInFrameMove);

	private:
		/** the DNS server stack */
		stack <string> m_stackDNS;

		/** Network layer */
		INPLStimulationPipe* m_pNetPipe;
		CNPLNameSpaceBinding* m_pNPLNamespaceBinding;

		/** default channel. default value is 0*/
		int m_nDefaultChannel;
		/** an array of channel property. m_channelProperties[channelID]. The m_channelProperties is initialized with 16 channels.*/
		std::vector<ChannelProperty> m_channelProperties;

		/** a mapping from web service URL to its callback sCode */
		std::map <string, string> m_mapWebServiceCallBacks;

		/** a manager interface for NPL web service client.*/
		ParaEngine::INPLWebServiceClient* m_pWebServiceClient;

		/** whether we will process messages in the main threads in the frame move function.
		* It is default to true;
		* However, it is possible for server to set to false, if one wants to have a more responsive main state on the server.
		* For example, it does high-frequency dispatcher jobs, instead of monitoring.
		* But client application, it is only advised to set to true, otherwise the scripting and render modules will be run in different threads, leading to complexity and bugs.
		*/
		bool m_bHostMainStatesInFrameMove;

		/* default to 1, set to 0 to silence some connection verbose log. */
		int m_nLogLevel;
	private:
		typedef std::set<NPLRuntimeState_ptr, NPLRuntimeState_PtrOps>	NPLRuntime_Pool_Type;
		typedef std::vector<NPLRuntimeState_ptr>	NPLRuntime_Temp_Pool_Type;
		typedef std::map<std::string, NPLRuntimeState_ptr> ActiveStateMap_Type;

		/// the network server, the main NPL networking implementation. 
		boost::scoped_ptr<CNPLNetServer> m_net_server;

		/// the network udp server, 
		boost::scoped_ptr<CNPLNetUDPServer> m_net_udp_server;

		/// all NPL runtime states in the NPL runtime
		NPLRuntime_Pool_Type m_runtime_states;

		/// all NPL runtime states with at least one timer in it. 
		NPLRuntime_Pool_Type m_runtime_states_with_timers;

		/** a mapping from the runtime state name to runtime state instance pointer */
		ActiveStateMap_Type m_active_state_map;

		/// a group of state that runs in the main game thread
		NPLRuntime_Pool_Type m_runtime_states_main_threaded;
		/// temporary run time states queue
		NPLRuntime_Temp_Pool_Type m_temp_rts_pool;

		/// the default (main) NPL runtime state. 
		NPLRuntimeState_ptr m_runtime_state_main;

		/// protecting this data member
		ParaEngine::mutex m_mutex;

		static NPLRuntimeStateType m_defaultNPLStateType;

		static std::string m_tempString;
	};

}

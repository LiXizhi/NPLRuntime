//-----------------------------------------------------------------------------
// Copyright (C) 2004 - 2006 ParaEngine Dev Studio, All Rights Reserved.
// Date:	2004.5
// Description:	API for NPL. 
//-----------------------------------------------------------------------------
#pragma once
#include <boost/shared_ptr.hpp>
#include "ParaScriptingGlobal.h"

namespace ParaScripting
{
	using namespace luabind;
	using namespace std;

	/**
	* @ingroup NPL
	* A runtime state contains the scripting runtime stack and can be run in a single thread. 
	* 
	* An NPL runtime state is message driven, however we also support timer and several other event callbacks. 
	* Normally we only use this class to start a new runtime, or get statistics about a runtime 
	*/
	class ParaNPLRuntimeState
	{
	public:
		ParaNPLRuntimeState();
		ParaNPLRuntimeState(NPL::CNPLRuntimeState* rts_);
		ParaNPLRuntimeState(NPL::NPLRuntimeState_ptr rts_);
		~ParaNPLRuntimeState();

		NPL::CNPLRuntimeState* m_rts;

		/** if this is a valid state. */
		bool IsValid();

		/** return the name of this runtime state. if "", it is considered an anonymous name */
		const char* GetName() const;

		/** start this runtime state in a worker thread 
		* @return the number of threads that are currently working on the runtime state. normally this is 1. 
		* only dll runtime state may have more than one worker thread. 
		*/
		int Start();

		/**
		* Stop the worker thread. this will stop processing messages in the thread. 
		*/
		bool Stop();

		/**
		* it is like starting a fresh new runtime state. All memory, tables, timers, pending messages are removed. 
		* this function only takes effect on the next message loop. So there can be other clean up code following this function. 
		* @param onResetScode: the code to be executed immediately after runtime state is reset. default to NULL. 
		*/
		void Reset();
		void Reset1(const char* onResetScode);

		/**
		* TODO: get statistics about this runtime environment. 
		*/
		object GetStats(const object& inout);

		/** Get the current number of messages in the input message queue. Sometimes, a monitor or dispatcher logics may need to know the queue size of all NPL runtime states. 
		* and a dispatcher will usually need to add new messages to the NPL state with smallest queue size. 
		* This function will lock the message queue to retrieve the queue size, so do not call it very often, but use a timer to query it on some interval. 
		* [thread safe]
		*/
		int GetCurrentQueueSize();

		/** the total number of message processed by this runtime state since start. If this number does not increase,
		* perhaps the processing is blocking somewhere, and we should take actions. 
		* [Not thread safe] in most cases, it will return correct result even in multi-threaded environment, but since we do not use lock, 
		* unexpected result may return. This function is usually used for stat printing and monitoring. 
		*/
		int GetProcessedMsgCount();

		/** get the message queue size. default to 500. For busy server side, we can set this to something like 5000
		* [thread safe]
		*/
		int GetMsgQueueSize();

		/** set the message queue size. default to 500. For busy server side, we can set this to something like 5000
		* [thread safe]
		*/
		void SetMsgQueueSize(int nSize = 500);

		/** simply wait for the next message to arrive.
		*/
		void WaitForMessage();

		/**
		* @param nMessageCount: if not negative, this function will immediately return when the message queue size is bigger than this value.
		*/
		void WaitForMessage2(int nMessageCount);

		/**
		* @param inout: this should be a table {filename=true, code=true, msg=true}, specify which part of the message to retrieve in return value. 
		* {filename=true} will only retrieve the filename, because it is faster if code is big. 
		* @return: msg table {filename, code, msg} if exist, or nil.
		*/
		object PeekMessage(int nIndex, const object& inout);

		/** pop message at given index. usually we need to call peek() first.
		* @param inout: this should be a table {filename=true, code=true, process=true}, specify which part of the message to retrieve in return value.
		* {filename=true} will only retrieve the filename, because it is faster if code is big.
		* if inout.process == true, we will pop and process the message via standard activation. 
		* if inout.process == false, we will pop without processing the message, in which case the caller may need to process it manually
		* @return: msg table {filename, code, msg, result} if exist, or filename will be false. result is only available if process is true
		*/
		object PopMessageAt(int nIndex, const object& inout);
	};

	/** 
	* @ingroup NPL
	* Neural Parallel Language functions are in this namespace.
	*/
	class PE_CORE_DECL CNPL
	{
	public:
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
		*	"(worker1)NPLRouter.dll"			-- activate a C++ or C# dll. Please note that, in windows, it looks for NPLRonter.dll; in linux, it looks for ./libNPLRouter.so 
		*	"plugin/libNPLRouter.dll"			-- almost same as above, it is recommented to remove the heading 'lib' when loading. In windows, it looks for plugin/NPLRonter.dll; in linux, it looks for ./plugin/libNPLRouter.so
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
		* @return: NPLReturnCode. 0 means succeed. 
		*/
		static int activate(const object& sNPLFilename, const object& sCode);
		static int activate1(const object& sNPLFilename);
		static int activate3(const object& sNPLFilename, const object& sCode, int channel);
		static int activate5(const object& sNPLFilename, const object& sCode, int channel, int priority, int reliability);
		/** this function is only called by .Net API.*/
		static int activate2_(const char * sNPLFilename, const char* sCode);
		/** this function is only called by .Net API.*/
		static int activate_(const char * sNPLFilename, const char* sCode, int channel, int priority, int reliability);

		/**
		* This function is used to activate a local file synchronously in the current runtime state. Use activate() if you need an asynchronous activation. 
		* for more information, please see activate();
		*/
		static void call(const object& sNPLFilename, const object& sCode);
		/** this function is only called by .Net API.*/
		static void call_(const char * sNPLFilename, const char* sCode);

		/**
		* return the NPL file name that is being loaded. Only call this function when the file is being initialized. i.e. at the root level. 
		* Note: calling this function inside other functions will lead to unexpected result.
		*/
		static const char* GetFileName();

		/**
		* NOTE: the function name is "this" in NPL, not "this_". 
		* associate a user defined function as the activation function of this file.
		* add the current file name to the __act table.
		* create the activate table, if it does not exist.
		* @param funcActivate: the function pointer to the activation function. It can either be local or global.
		* @param params: nil or a table {[PreemptiveCount=number,] [MsgQueueSize=number,] [filename|name=string,]}
		* - PreemptiveCount: if PreemptiveCount is omitted (default), the activate function will 
		* run non-preemptive (it is the programmer's job to let the function finish in short time). 
		* If PreemptiveCount > 0, the activate function will be preemptive (yield) after this number of virtual instructions.
		* which allows us to run tens of thousands of jobs concurrently. Each job has its own stack and but the programmer 
		* should pay attention when making changes to shared data.
		* - MsgQueueSize: Max message queue size of this file, if not specified it is same as the NPL thread's message queue size. 
		* - filename|name: virtual filename, if not specified, the current file being loaded is used. 
		* - clear: clear all memory used by the file, including its message queue. Normally one never needs to clear. 
		*  A neuron file without messages takes less than 100 bytes of memory (mostly depending on the length's of its filename)
		*/
		static void this_(const object& funcActivate);
		static void this2_(const object& funcActivate, const object& params);

		/** get the attribute object. This function return a clone of this object. */
		static ParaAttributeObject GetAttributeObject();

		
		/**
		* load a new file (in the current runtime state) without activating it. If the file is already loaded,
		* it will not be loaded again unless bReload is true. 
		* IMPORTANT: this function is synchronous; unlike the asynchronous activation function. 
		* LoadFile is more like "include in C++".When the function returns, contents in the file is loaded to memory. 
		* @note: in NPL/lua, function is first class object, so loading a file means executing the code chunk in protected mode with pcall, 
		* in most cases,  this means injecting new code to the global table. Since there may be recursions (such as A load B, while B also load A), 
		* your loading code should not reply on the loading order to work. You need to follow basic code injection principles.
		* For example, commonlib.gettable("") is the the commended way to inject new code to the current thread's global table. 
		* Be careful not to pollute the global table too much, use nested table/namespace. 
		* Different NPL applications may have their own sandbox environments, which have their own dedicated global tables, for example all `*.page` files use a separate global table per URL request in NPL Web Server App.
		* @note: when loading an NPL file, we will first find if there is an up to date compiled version in the bin directory. if there is, 
		* we will load the compiled version, otherwise we will use the text version.  use bin version, if source version does not exist; use bin version, if source and bin versions are both on disk (instead of zip) and that bin version is newer than the source version. 
		* e.g. we can compile source to bin directory with file extension ".o", e.g. "script/abc.lua" can be compiled to "bin/script/abc.o", The latter will be used if available and up-to-date. 
		* @param filePath: the local relative file path. 
		* If the file extension is ".dll", it will be treated as a plug-in. Examples:
		*	"NPLRouter.dll"			-- load a C++ or C# dll. Please note that, in windows, it looks for NPLRonter.dll; in linux, it looks for ./libNPLRouter.so 
		*	"plugin/libNPLRouter.dll"			-- almost same as above, it is reformatted to remove the heading 'lib' when loading. In windows, it looks for plugin/NPLRonter.dll; in linux, it looks for ./plugin/libNPLRouter.so
		* @param bReload: if true, the file will be reloaded even if it is already loaded.
		*    otherwise, the file will only be loaded if it is not loaded yet. 
		* @remark: one should be very careful when calling with bReload set to true, since this may lead to recursive 
		*	reloading of the same file. If this occurs, it will generate C Stack overflow error message.
		*/
		static void load(const object& filePath, bool bReload);
		/** for NPL managed only.*/
		static void load_(const char* filePath, bool bReload);
		/** same as NPL.load(filepath, false); */
		static void load1(const object& filePath);

		/**
		* execute a given string immediately in protected mode. 
		* @param sCode : the code to run. the code can not be longer than some internally defined value. 
		* @remark:caution: there may be a security issue. 
		*/
		static void DoString(const object& sCode);

		/**
		* execute a given string immediately in protected mode. 
		* @param sCode : the code to run. the code can not be longer than some internally defined value. 
		* @param sFilename: can be nil, a filename to be associated with the chunk of code for debuggin purposes. 
		* @remark:caution: there may be a security issue. 
		*/
		static void DoString2(const object& sCode, const char* sFilename);

		/** only for NPL managed only */
		static void DoString_(const char* sCode);

		/** this function should be removed in release build. it just run NPL C++ test cases 
		* @param input: which test case to run. 
		*/
		static void test(const object& input);

		/** creates a timer with the specified time-out value
		* @param nIDEvent: Specifies a positive timer identifier. For nIDEvent<=0, they are reserved for internal uses.
		* If the NPL runtime already has a timer with the value nIDEvent, 
		* then the existing timer is replaced by the new timer. When SetTimer replaces a timer, the timer is reset. 
		* @param fElapse: Specifies the time-out value, in seconds. Please note that a timer will not be repeatedly activated if
		*		its timeout is shorter than the frame rate of the NPL simulation pipeline .
		* @param sNeuronFile: The NPL file to be activated when the time-out value elapses. it can also carry sCode. 
		*  e.g. "script/hello.lua;funcABC();", ";funcABC();", "(gl)script/hello.lua";
		* @return: true if succeeds.An application can pass the value of the nIDEvent parameter to the NPL.KillTimer function to destroy the timer.
		*/
		static bool SetTimer(const object& nIDEvent, float fElapse, const object& sNeuronFile);

		/**
		* Destroys the specified timer
		* @param nIDEvent: Specifies the timer to be destroyed.For nIDEvent<=0, they are reserved for internal uses can not be killed by this function.
		* This value must be the same as the nIDEvent value passed to the SetTimer function that created the timer.
		* @return : If the function succeeds, the return value is true
		*/
		static bool KillTimer(const object& nIDEvent);

		/**
		* Changes the start time and the interval between method invocations for a timer, using 32-bit signed integers to measure time intervals. 
		* @param nIDEvent: Specifies the timer to be destroyed.For nIDEvent<=0, they are reserved for internal uses can not be killed by this function.
		* This value must be the same as the nIDEvent value passed to the SetTimer function that created the timer.
		* @param dueTime: The amount of time to delay before the invoking the callback method specified when the Timer was constructed, in milliseconds. Specify zero (0) to restart the timer immediately.
		*  however, the current implementation does not accept dueTime that is larger than MAX_TIMER_DUE_TIME	10000000, which is 10000 seconds. 
		* @param period:The time interval between invocations of the callback method specified when the Timer was constructed, in milliseconds. 
		* @return : If the function succeeds, the return value is true
		*/
		static bool ChangeTimer(const object& nIDEvent, int dueTime, int period);

		/**
		* serialize a luabind object into sCode. The object could be a value, string or a table of the above type. 
		* input also accepts ParaFileObject typed data. ParaFileObject will be converted to base64 string internally.There are size limit though of 10MB.
		* @param sStorageVar if this is "", the scode contains only the object. otherwise an assignment is made,
		* by prefixing the scode with  "[sStorageVar = ".
		* @param input input luabind object
		* @return sCode the output scode
		*/
		static const string& SerializeToSCode(const char* sStorageVar, const object& input);

		/** verify the script code. it returns true if the script code contains pure msg data or table. 
		* this function is used to verify scode received from the network. So that the execution of a pure data in the local runtime is harmless. 
		*/
		static bool IsSCodePureData(const char* sCode);

		/** it will return true if input string is "false", "true", NUMBER, STRING, and {table} */
		static bool IsPureData(const char* sCode);

		/** it will return true if input string is a {table} containing only "false", "true", NUMBER, STRING, and other such {table} */
		static bool IsPureTable(const char* sCode);

		/** load a table from string. */
		static luabind::object LoadTableFromString(const object& input);
		/** load a table,string, anything from string. */
		static luabind::object LoadObjectFromString(const object& input);


		/**
		* start the NPL net server's io_service loop. This function returns immediately. it will spawn the accept and dispatcher thread.  
		* call this function only once per process.
		* @param server: if nil, it defaults to "127.0.0.1"
		* @param port: if nil, it defaults to "60001"; if "0", we will not listen or accept incoming connections. This is usually the setting for pure client application. 
		*/
		static void StartNetServer(const object& server, const object& port);

		/** stop the net server */
		static void StopNetServer();

		/** add a nID, filename pair to the public file list. 
		* we only allow remote NPL runtime to activate files in the public file list. 
		* Each public file has a user defined ID. The NPL network layer always try to use its ID for transmission to minimize bandwidth. 
		* There are some negotiations between sender and receiver to sync the string to ID map before they use it. 
		* [thread safe]
		* @param nID: the integer to encode the string. it is usually small and positive number.
		* @param sString: the string for the id. if input is empty, it means removing the mapping of nID. 
		*/
		static void AddPublicFile(const string& filename, int nID);

		/** register a network event handler 
		* @param nType: reserved, this is always 0.
		* @param sID: a string identifier of the network event handler.
		* @param sScript: the script to be executed when the event is triggered.This is usually a function call in NPL.
		*	sScript should be in the following format "{NPL filename};{sCode};". this is the same format in the UI event handler. 
		*	Please note, it is slightly faster when there is no sCode, and we can register the callback script to be in a different NPL state(thread), such as
		*   "(gateway)script/apps/GameServer/GSL_NPLEvent.lua". 
		*/
		static void RegisterEvent(int nType, const char* sID, const char* sScript);

		/** unregister a network event handler 
		* @param nType: reserved, this is always 0.
		*/
		static void UnregisterEvent(int nType, const char* sID);

		/**
		* get statistics about this runtime environment. 
		* e.g. local stats = NPL.GetStats({connection_count = true, nids_str=true, nids = true});
		* @param input: this should be a table containing mapping from string to true. function will return a new table by replacing true with the actual data. such as {["connection_count"] = true, ["nids_str"] = true }, supported fields are 
		*  "connection_count" : total connection. 
		*  "nids_str": commar separated list of nids.  
		*  "nids": a table array of nids
		* @return {connection_count = 10, nids_str="101,102,"}
		*/
		static object GetStats(const object& inout);

		/** clear all public files, so that the NPL server will be completely private. 
		* [thread safe]
		*/
		static void ClearPublicFiles();

		/**
		* add a given NPL runtime address to the fully trusted server addresses list. 
		* whenever an address is added, one can activate any public files in that runtime. Please note that connection is only established on first activation. 
		* In most cases, one should add all trusted servers at start up time. 
		* @param npl_address: this should be a table of 
		{
		host = "127.0.0.1",
		port = "60001",
		nid = "MyServer",
		}
		* [thread safe]
		* @return: true if successfully added. 
		*/
		static bool AddNPLRuntimeAddress(const object& npl_address);

		/** get the ip address of given NPL connection. 
		* this function is usually called by the server for connected clients. 
		* @param nid: nid or tid. 
		* @return: the ip address in dot format. empty string is returned if connection can not be found. 
		*/
		static string GetIP(const char* nid);

		/** accept a given connection. The connection will be regarded as authenticated once accepted. 
		* [thread safe]
		* @param tid: the temporary id or NID of the connection to be accepted. usually it is from msg.tid or msg.nid. 
		* @param nid: if this is not nil, tid will be renamed to nid after accepted. 
		*/
		static void accept(const object& tid, const object& nid);

		/** this function is used by C++ API interface. */
		static void accept_(const char* tid, const char* nid);

		/** reject and close a given connection. The connection will be closed once rejected. 
		* [thread safe]
		* @param nid: the temporary id or NID of the connection to be rejected. usually it is from msg.tid or msg.nid. 
		* it can also be {nid=number|string, reason=0|1|-1}
		* reason: 
		* - 0 or positive value is forcibly reset/disconnect (it will abort pending read/write immediately). 
		* - 1 is another user with same nid is authenticated. The server should send a message to tell the client about this. 
		* - -1 or negative value means gracefully close the connection when all pending data has been sent.
		*/
		static void reject(const object& nid);

		/** this function is used by C++ API interface. */
		static void reject_(const char* nid, int nReason = 0);

		/** whether to use compression on transport layer for incoming and outgoing connections
		* @param bCompressIncoming: if true, compression is used for all incoming connections. default to false.
		* @param bCompressIncoming: if true, compression is used for all outgoing connections. default to false.
		*/
		static void SetUseCompression(bool bCompressIncoming, bool bCompressOutgoing);
	
		/**
		* set the compression method of incoming the outgoing messages. 
		* If this is not called, the default internal key is used for message encoding. 
		* [Not Thread Safe]: one must call this function before sending or receiving any encoded messages. 
		* so it is usually called when the game engine starts. 
		* @param input: a table, such as { key = "", size = 100, UsePlainTextEncoding = 1}, or {UsePlainTextEncoding = 1}
		* input.key: the byte array of key. the generic key that is used for encoding/decoding
		* input.size: size in bytes of the sKey. default is 64 bytes
		* input.UsePlainTextEncoding: default to 0. 
		* if 0, the key is used as it is. 
		* if 1, the input key will be modified so that the encoded message looks like plain text(this can be useful to pass some firewalls). 
		* if -1, the input key will be modified so that the encoded message is binary. 
		*/
		static void SetCompressionKey(const object& input);

		//////////////////////////////////////////////////////////////////////////
		//
		// old network implementations. functions in this categories are obsoleted at the moment. 
		//
		//////////////////////////////////////////////////////////////////////////

		/** get the current activation's source name. Each NPL activation has a source name which started the activation. 
		* This is usually "" for local activation and some kind of "name@server" for network activation.*/
		static const char* GetSourceName();
		/**
		* Set the current activation's source name. Each NPL activation has a source name which started the activation. 
		* This function is called automatically when a new activation occurs.So only call this function if one wants to override the old one for special code logics.
		* @param sName This is usually "" for local activation and some kind of "name@server" for network activation.
		*/
		static void SetSourceName(const char* sName);

		/**
		* Enable the network, by default the network layer is disabled.
		* calling this function multiple time with different CenterName will restart the network layer with a different center name.
		* @param bEnable true to enable, false to disable.If this is false, the CenterName and Password are ignored. 
		* @param CenterName the local nerve center name. it is also the user name 
		*  which local receptor will use in the credentials to login in other NPL runtime.
		* @param Password 
		* @return true if succeeded.
		*/
		static void EnableNetwork(bool bEnable, const char* CenterName, const char* password);

		/**
		* add a DNS server record to the current NPL runtime.
		* DNS server record is a mapping from name to (IP:port)
		* if one maps several IP:port to the same name, the former ones will be overridden.
		* @param sDNSName: the DNS server name. the DNS name "_world" is used for the current 
		*	world DNS server. It is commonly used as a DNS reference to the current world that 
		*	the user is exploring.
		* @param sAddress: "IP:port". e.g. "192.168.1.10:4000"
		*/
		static void AddDNSRecord(const char * sDNSName, const char* sAddress);

		/**
		* Set the default channel ID, default value is 0. Default channel is used when NPL.activate() call¡¯s does not contain the channel property.
		* @param channel_ID It can be a number in [0,15].default is 0
		*/
		static void SetDefaultChannel(int channel_ID);

		/**
		* Get the default channel ID, default value is 0. Default channel is used when NPL.activate() call¡¯s does not contain the channel property.
		* @return channel_ID It can be a number in [0,15].default is 0
		*/
		static int GetDefaultChannel();

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
		static void SetChannelProperty(int channel_ID, int priority, int reliability);

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
		static void ResetChannelProperties();

		/**
		* see also NPL_SetChannelProperty
		* @param channel_ID 
		* @param priority [out]
		* @param reliability [out]
		*/
		static void GetChannelProperty(int channel_ID, int* priority, int* reliability);

		/**
		* this method will associate a web service (method) with either a sCode, which will be called when the web service returned. 
		The returned message, if any, will be passed via a global parameter called msg. 
		If msg == nil, it always means that there is an error during processing, such as HTTP 404 not found.
		the error code either string or number will be returned in a global variable called msgerror.
		For example:
		function callbackFunc1()
		if(msg~=nil) then 
		log(msg.username);  -- error code in msgerror
		else
		log(tostring(msgerror)); -- error code in msgerror
		end
		end
		NPL.RegisterWSCallBack("http://paraengine.com/login.aspx",callbackFunc1);
		NPL.activate("http://paraengine.com/login.aspx", {username=¡±lxz¡±});
		* @param sWebServiceFile URL of the web service
		* @param sCode code to be executed when the web service is called. When a two-way web service call is invoked; 
		*  it internally will create a thread for the returning message. Please refer to .Net 3.0 network communication architecture.
		*/
		static void RegisterWSCallBack(const char * sWebServiceFile, const char * sCode);
		/**
		* unregister web service call back for a given file.
		* @param sWebServiceFile if this is nil or "", all call backs will be unregistered.
		*/
		static void UnregisterWSCallBack(const char * sWebServiceFile);

		//////////////////////////////////////////////////////////////////////////
		//
		// downloader functions
		//
		//////////////////////////////////////////////////////////////////////////

		/**
		* Asynchronously download a file or an HTTP web page from the url.
		* @param destFolder:folder path or file path. if the destFolder contains a file extension, we will save the downloaded file as the destFolder
		*  otherwise, we will save the file as the combination of the destFolder and the filename returned from the remote target.
		* @param callbackScript: script code to be called, a global variable called msg is assigned, as below
		* if url is a file: 
		*   msg = {DownloadState=""|"complete"|"terminated", totalFileSize=number, currentFileSize=number, PercentDone=number}
		* if url is a web page: 
		*   msg = {DownloadState=""|"complete"|"terminated", 
		*    ContentType=string that contains "text/html", Headers=string of {name:value\r\n}, StatusCode=int,  StatusDescription=string, ResponseUri=string of actual url that is responding. 
		*    totalFileSize=number, currentFileSize=number, PercentDone=number}
		*/
		static void AsyncDownload(const char* url, const char* destFolder, const char* callbackScript, const char* DownloaderName);

		/**
		* cancel all asynchronous downloads that matches a certain downloader name pattern
		* @param DownloaderName:regular expression. such as "proc1", "proc1.*", ".*"
		*/
		static void CancelDownload(const char* DownloaderName);

		/**
		* Synchronous call of the function AsyncDownload(). This function will not return until download is complete or an error occurs. 
		* this function is rarely used. AsyncDownload() is used. 
		* @return:1 if succeed, 0 if fail
		*/
		static int Download(const char* url, const char* destFolder, const char* callbackScript, const char* DownloaderName);

		//////////////////////////////////////////////////////////////////////////
		//
		// neuron file function
		//
		//////////////////////////////////////////////////////////////////////////

		/** create a new runtime state.
		* this function is thread safe 
		* @param name: if "", it is an anonymous runtime state. otherwise it should be a unique name. 
		* @param type_: NPLRuntimeStateType,  the runtime state type. 
		enum NPLRuntimeStateType
		{
		/// the default NPL runtime state, with all NPL and ParaEngine functions loaded. it will consume about 1MB memory at start up. 
		NPLRuntimeStateType_NPL = 0,
		/// the light-weighter NPL runtime state, with only NPL and very limited functions loaded. 
		NPLRuntimeStateType_NPL_LITE,
		/// it consumes nothing. and is usually used with DLL plugins. 
		NPLRuntimeStateType_DLL,
		};
		* @return the newly created state is returned. If an runtime state with the same non-empty name already exist, the old one is returned. 
		*/
		static ParaNPLRuntimeState CreateRuntimeState(const string& name, int type_);

		/** get a runtime state with an explicit name.
		* this function is thread safe 
		*/
		static ParaNPLRuntimeState GetRuntimeState(const string& name);

		/** it get runtime state first, if none exist, it will create one and add it to the main threaded state */
		static ParaNPLRuntimeState CreateGetRuntimeState(const string& name, int type_);

		/** create a given runtime state.
		* this function is thread safe */
		static bool DeleteRuntimeState(ParaNPLRuntimeState runtime_state);

		/** compile source 
		* The main advantages of precompiling chunks are: faster loading, protecting source code from accidental user changes, and off-line syntax checking. 
		* Precompiling does not imply faster execution because in npl chunks are always compiled into bytecodes before being executed. 
		* compiling simply allows those bytecodes to be saved in a file for later execution. 
		* compiling can produce a single output file containing the bytecodes for all source files given. By default, the output file is named luac.out, but you can change this with the -o option. 
		* e.g. NPL.Compile("-p -o bin/script/config.o script/config.lua");
		* @param arguments: "%s [options] [filenames]"
		"Available options are:\n"
		"  -        process stdin\n"
		"  -l       list\n"
		"  -o name  output to file " LUA_QL("name") " (default is \"%s\")\n"
		"  -p       parse only\n"
		"  -s       strip debug information\n"
		"  -v       show version information\n"
		"  --       stop handling options\n",
		*/
		static void Compile(const char* arguments);

		//////////////////////////////////////////////////////////////////////////
		//
		// new libcUrl interface. 
		//
		//////////////////////////////////////////////////////////////////////////

		/** Append URL request to a pool. 
		* @param pUrlTask: must be new CURLRequestTask(), the ownership of the task is transfered to the manager. so the caller should never delete the pointer. 
		* @param sPoolName: the request pool name. If the pool does not exist, it will be created. If null, the default pool is used. 
		*/
		//static bool AppendURLRequest(CURLRequestTask* pUrlTask, const char* sPoolName);

		/**
		* Append URL request to a pool. 
		* HTTP Get: NPL.AppendURLRequest("paraengine.com", "callbackFunc()", {"name1", "value1", "name2", "value2", }, "r")
		* HTTP Post: NPL.AppendURLRequest("paraengine.com", "callbackFunc()", {name1="value1", name2="value2"}, "r")
		* @param url: usually a rest url. 
		* @param sCallback: a string callback function. it may begin with (runtime_state_name) such as "(main)my_function()", 
		* if no runtime state is provided, it is the main state(Not the calling thread). This prevents the user to use multiple threads to download to the same file location by mistake. 
		* the callback function to be called. a global msg={data, header, code, rcode} contains the result. 
		*	the msg.data contains the response data, and msg.header contains the http header, and msg.code contains the return code from libcurl, msg.rcode contains HTTP/FTP status code(200 if succeed)
		* @param sForm: if it contains name value pairs, HTTP POST is used, otherwise HTTP GET is used. 
		*  note that if it contains an array of name, value, name, value, ..., they will be regarded as url parameters can inserted to url automatically. This applies regardless of whether http get or post is used. 
		*  one can also post a file, like belows
		*		{name = {file="/tmp/test.txt",	type="text/plain"}}
		*		{name = {file="dummy.html",	data="<html><bold>bold</bold></html>, type="text/html"}}
		*  some predefined field name in sForm is 
		*		request_timeout: milliseconds of request timeout e.g. {request_timeout=50000,}
		* @param sPoolName: the request pool name. If the pool does not exist, it will be created. If null, the default pool is used. 
		*  there are some reserved pool names used by ParaEngine. They are: 
		*    - "d": download pool. default size is 2, for downloading files. 
		*    - "r": rest pool. default size is 5, for REST like HTTP get/post calls.
		*    - "w": web pool. default size is 5, for web based requests.
		*/
		static bool AppendURLRequest1(const object& url, const char* sCallback, const object& sForm, const char* sPoolName);

		/** build a use query using base url plus additional query parameters. 
		* NPL.BuildURLQuery("paraengine.com", {"name1", "value1", "name2", "value2", })
		* [thread safe]
		* @param sParams; an array table, where odd index is name, even index is value. 
		* @return: result is like "paraengine.com?name1=value1&name2=value2", they already in encoded form. 
		*/
		static string EncodeURLQuery(const char * baseUrl, const object& sParams);

		/** get extern IP address of this computer. */
		static std::string GetExternalIP();

		/**
		* There is generally no limit to the number of requests sent. However, each pool has a specified maximum number of concurrent worker slots. 
		*  the default number is 1. One can change this number with this function. 
		*/
		static bool ChangeRequestPoolSize(const char* sPoolName, int nCount);

		/** convert json string to NPL object. Internally TinyJson is used.  
		* @param sJson: the json code to parse. the first level must be array or table. otherwise, false is returned. 
		* @param output: [in|out] it must be a table. and usually empty table. the output is written to this table. 
		* @return true if succeed. false if parsing failed. 
		*/
		static bool FromJson(const char* sJson, const object& output);

		/** convert from NPL/lua table object to json string. /uXXXX encoding is recognized in string. */
		static const string& ToJson(const object& output);
		static const string& ToJson2(const object& output, bool bUseEmptyArray);

		/** compress using zlib/gzip, etc
		* @param output: {method="zlib|gzip", content=string, [level=number, windowBits=number,] result=string}
		* @return return true if success. 
		*/
		static bool Compress(const object& output);

		/** compress using zlib/gzip, etc
		* @param output: {method="zlib|gzip", content=string, [level=number, windowBits=number,] result=string}
		*/
		static bool Decompress(const object& output);
	};
}

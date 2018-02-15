---+ NPL Network Layer 
| author | LiXizhi |
| date| 2009.6.30 |
%TOC%

This file just reminds me how code is orgnized

---++ code call graph

---+++ incoming connections
<verbatim>
incoming connection:  acceptor/dispatcher thread
	-> CNPLNetServer::handle_accept 
		-> CNPLNetServer.m_connection_manager.start(a new CNPLConnection object) : a new CNPLConnection object is added to CNPLConnectionManager 
			-> CNPLConnection.start() 
				-> automatically assign a temporary nid to this unauthenticated connection, temp connection is called ~XXX.
					-> CNPLDispatcher::AddNPLConnection(temporary nid, CNPLConnection)
				-> begin reading the connection sockets
				-> LATER: Once authenticated, the scripting interface can call (CNPLConnection)NPL.GetConnection(nid)::rename(nid, bAuthenticated) 
</verbatim>
if any messages are sent via the new incomming connection, the message field should contain msg.nid and msg.tid, 
where tid is temporaray nid. If there is no nid, it means that the connection is not yet authenticated. 
In NPL, one can call NPL.accept(tid, nid)


---+++ incoming messages
<verbatim>
incoming message
	-> CNPLConnection::handle_read 
		-> CNPLConnection::handleReceivedData
			-> CNPLConnection::m_parser.parse until a complete message is read
				-> CNPLConnection::handleMessageIn()
					-> CNPLConnection::m_msg_dispatcher.DispatchMsg((NPLMsgIn)this->m_input_msg)
						-> CNPLRuntimeState = GetNPLRuntime()->GetRuntimeState(NPLMsgIn::m_rts_name)
						-> if CNPLDispatcher::CheckPubFile(NPLMsgIn::m_filename)
							-> new NPLMessage(from NPLMsgIn)
							-> CNPLRuntimeState::Activate_async(NPLMessage);
								-> CNPLRuntimeState::SendMessage(NPLMessage) -> insert message to CNPLRuntimeState.m_input_queue
						-> or return access denied	
</verbatim>
---+++ outgoing messages
<verbatim>
from NPL script NPL.activate()
	-> CNPLRuntime::NPL_Activate
		-> if local activation, get the runtime state.
			-> CNPLRuntimeState::Activate_async(NPLMessage) -> CNPLRuntimeState::SendMessage(NPLMessage) -> insert message to CNPLRuntimeState.m_input_queue
		-> if remote activation with nid, send via dispatcher	
			-> CNPLDispatcher::Activate_Async(NPLFileName, code)
				-> CNPLConnection = CNPLDispatcher::CreateGetNPLConnectionByNID(nid) 
					-> if found in CNPLDispatcher::m_active_connection_map(nid, CNPLConnection), return
					-> or if found in CNPLDispatcher::m_pending_connection_map(nid, CNPLConnection), return
					-> or if found in CNPLDispatcher::m_server_address_map(host, port, nid): we will actively establish a new connection to it. 
						-> CNPLDispatcher::CreateConnection(NPLRuntimeAddress)
							-> CNPLNetServer::CreateConnection(NPLRuntimeAddress)
								-> new CNPLConnection() 
								-> CNPLConnection::SetNPLRuntimeAddress(NPLRuntimeAddress) 
								-> (CNPLNetServer::m_connection_manager as CNPLConnectionManager)::add(CNPLConnection)
								-> CNPLConnection::connect() -> CNPLConnection::m_resolver->async_resolve -> 
									-> CNPLConnection::handle_resolve -> CNPLConnection::m_socket.async_connect 
									-> CNPLConnection::handle_connect 
										-> CNPLConnection::handleConnect() : for custom behavior
										-> CNPLConnection::start()
											-> CNPLDispatcher::AddNPLConnection(CNPLConnection.m_address.nid, CNPLConnection)
												-> add nid to CNPLDispatcher::m_active_connection_map(nid, CNPLConnection), remove nid from m_pending_connection_map(nid, CNPLConnection)
											-> begin reading the connection sockets
							-> Add to CNPLDispatcher::m_pending_connection_map if not connected when returned, return the newly created connection
					-> or return null.	
				-> CNPLConnection::SendMessage(NPLFilename, code)
					-> new NPLMsgOut( from NPLFilename, code)
					-> push to CNPLConnection::m_queueOutput() and start sending first one if queue is previously empty.
</verbatim>
---++ NPL Message Format
More information is in NPLMsgIn_parser.h

*quick sample*
<verbatim>
A (g1)script/hello.lua NPL/1.0
rts:r1
User-Agent:NPL
16:{"hello world!"}
</verbatim>

---++ NPL quick user guide

| sample file | "script/test/script/test/network/TestSimpleServer(Client).lua" | 
| level | easy |

In NPL runtime, there can be one or more runtime threads called NPL runtime states. 
Each runtime state has its own name, stack, thread local memory manager, and message queue. 
The default NPL state is called (main), which is also the thread for rendering 3D graphics. It is also the thread to spawn other worker threads.

To make a NPL runtime accessible by other NPL runtimes, one must call 
<verbatim>
	NPL.StartNetServer("127.0.0.1", "60001");
</verbatim>

To make files within NPL runtime accessible by other NPL runtimes, one must add those files to public file lists by calling 
<verbatim>
	NPL.AddPublicFile("script/test/network/TestSimpleClient.lua", 1);
	NPL.AddPublicFile("script/test/network/TestSimpleServer.lua", 2);
</verbatim>
the second parameter is id of the file. The file name to id map must be the same for all communicating computers. This presumption may be removed in futhure versions.  

To create additional worker threads (NPL states) for handling messages, one can call
<verbatim>
	for i=1, 10 do
		local worker = NPL.CreateRuntimeState("some_runtime_state"..i, 0);
		worker:Start();
	end
</verbatim>

Each NPL runtime on the network (either LAN/WAN) is identified by a globally unique nid string. 
NPL itself does not verify or authenticate nid, it is the job of application programmers. 

To add a trusted server address to nid map, one can call 
<verbatim>
	NPL.AddNPLRuntimeAddress({host="127.0.0.1", port="60001", nid="this_is_server_nid"})
</verbatim>
so that NPL runtime knows how to connection to the nid via TCP endpoints. host can be ip or domain name.
	
To activate a file on a remote NPL runtime(identified by a nid), one can call 
<verbatim>
	while( NPL.activate("(some_runtime_state)this_is_server_nid:script/test/network/TestSimpleServer.lua", {some_data_table}) ~=0 ) do end
</verbatim>
Please note that NPL.activate() function will return false if no connection is established for the target, 
but it will immediately try to establish a new connection if the target address of server nid is known. 
The above sample code simply loop until activate succeed; in real world, one should use a timer with timeout. 

When a file is activated, its activation function will be called, and data is inside the global msg table. 
<verbatim>
	local function activate()
		if(msg.nid) then
		elseif(msg.tid) then
		end
		NPL.activate(string.format("%s:script/test/network/TestSimpleClient.lua", msg.nid or msg.tid), {some_date_sent_back})
	end
	NPL.this(activate)
</verbatim>
msg.nid contains the nid of the source NPL runtime. msg.nid is nil, if connection is not authenticated or nid is not known. 
In such cases, msg.tid is always available; it contains the local temporary id. When the connection is authenticated, one can call following function to reassign nid to a connection or reject it.
<verbatim>
	NPL.accept(tid, nid)  -- to rename tid to nid, or 
	NPL.reject(tid) -- to close the connection. 
</verbatim>

__Note__: only the main thread support non-thread safe functions, such as those with rendering. For worker thread(runtime state), please
only use NPL functions that are explicitly marked as thread-safe in the documentation. 


---++ NPL Extensibility and Scripting Performance
NPL provides three extensibility modes: (1) Lua scripting runtime states (2) Mono .Net runtimes (3) C++ plugin interface
All of them can be used simultanously to create logics for game applications. All modes support cross-platform and multi-threaded computing. 

Lua runtime is natively supported and has the most extensive ParaEngine API exposed. It is both extremely light weighted and having a good thread local memory manager. 
It is suitable for both client and server side scripting. It is recommended to use only lua scripts on the client side. 

Mono .Net runtimes is supported via NPLMono.dll (which in turn is a C++ plugin dll). The main advantage of using .Net scripting is its rich libraries, popular language support(C#,Java,VB and their IDE) and compiled binary code performance.
.Net scripting runtime is recommended to use on the server side only, since deploying .Net on the client requires quite some disk space. And .Net (strong typed language) is less effective than lua when coding for client logics. 

C++ plugins allows us to treat dll file as a single script file. It has the best performance among others, but also is the most difficult to code. 
We would only want to use it for performance critical tasks or functions that make extensive use of other third-party C/C++ libraries. For example, NPLRouter.dll is a C++ plugin for routing messages on the server side. 

---+++ Cross-Runtime Communication
A game project may have thousands of lua scripts on the client, mega bytes of C# code contained in several .NET dll assemblies on the server,  and a few C++ plugin dlls on both client and server. 
All these extensible codes are connected to the ParaEngine Runtime and have full access to the exposed ParaEngine API.
Hence, source code written in different language runtimes can use the ParaEngine API to communicate with each other as well as the core game engine module.  

More over, NPL.activate() is the single most versatile and effective communication function used in all NPL extensibility modes. 
In NPL, a file is the smallest communication entity. Each file can receive messages either from the local or remote files. 
   * In NPL lua runtime, each script file with a activate() function assigned can receive messages from other scripts. 
   * In NPL .Net runtime, each C# file with a activate() function defined inside a class having the same name as the file name can receive messages from other scripts. (namespace around class name is also supported, see the example)
   * In NPL C++ plugins, each dll file must expose a activate() function to receive messages from other scripts. 

Following are example scripts and NPL.activate() functions to send messages to them. 
<verbatim>
---------------------------------------
File name:	script/HelloWorld.lua  (NPL script file)
activate:	NPL.activate("script/HelloWorld.lua", {data})
---------------------------------------
local function activate()
end
NPL.this(activate)

---------------------------------------
File name:	HelloWorld.cs inside C# mono/MyMonoLib.dll
activate:	NPL.activate("mono/MyMonoLib.dll/HelloWorld.cs", {data})  
			NPL.activate("mono/MyMonoLib.dll/ParaMono.HelloWorld.cs", {data}) 
---------------------------------------
class HelloWorld
{
	public static void activate(ref int type, ref string msg)
	{
	}
}
namespace ParaMono
{
	class HelloWorld
	{
		public static void activate(ref int type, ref string msg)
		{
		}
	}
}
---------------------------------------
File name:	HelloWorld.cpp inside C++ MyPlugin.dll
activate:	NPL.activate("MyPlugin.dll", {data})
---------------------------------------
CORE_EXPORT_DECL void LibActivate(int nType, void* pVoid)
{
	if(nType == ParaEngine::PluginActType_STATE)
	{
	}
}
</verbatim>

The NPL.activate() can use the file extension (*.lua, *.cs, *.dll) to distinguish the file type.

All NPL.activate() communications are asynchronous and each message must go through message queues between the sending and receiving endpoints. 
The performance of NPL.activate() is therefore not as efficient as local function calls, so they are generally only used to dispatch work tasks to different worker threads or communicating between client and server.  
Generally, 30000 msgs per second can be processed for 100 bytes message on 2.5G quad core CPU machine. 
For pure client side game logics, we rarely use NPL.activate(), instead we use local function calls whereever possible. 

---+++ High Level Networking Programming Libraries
In real world applications, we usually need to build a network architecture on top of NPL, which usually contains the following tool sets and libraries.
   * gateway servers and game(application) servers where the client connections are kept. 
   * routers for dispaching messagings between game(app) servers and database servers. 
   * database servers that performances business logics and query the underlying databases(My SQLs)
   * memory cache servers that cache data for database servers to minimize access to database.
   * Configuration files and management tools (shell scripts, server deployingment tools, monitoring tools, etc)
   * backup servers for all above and DNS servers.
   
For that, ParaEngine provides a scalable and extensive network progamming libraries built on top of NPL. 
It is suitable for MMO games or other applications with large (unlimited) number of concurrent users, and where the server logics changes frequently. 
We use the most effective language(lua, C++, .Net C#) for different NPL programming libraries. And we encourage programmers to do so as well when extending it.

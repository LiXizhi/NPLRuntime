#pragma once
#include "util/mutex.h"
#include "NPLMemPool.h"
#include "NPLTypes.h"

#include <boost/core/noncopyable.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>

using namespace std;

namespace NPL
{
	/** NPL String can be used instead of std::string, for strings which are created and deleted very regularly. 
	* iternally it uses memory pool CNPLPool_Char_alloc for memory allocation. 
	* NPL String uses SSO(small string optimization 16 bytes) as std::string; for string smaller or equal to 2048 Bytes, it uses one of its internal memory pools,
	* for even bigger string, standard malloc is used(default to std::string behavior). 
	*/
	typedef std::basic_string<char, std::char_traits<char>, ParaEngine::CNPLPool_Char_alloc<> > NPLString;	

	/**
	* a globally unique name of a NPL file name instance. 
	* The string format of an NPL file name is like below. 
	* [(sRuntimeStateName|gl)][sNID:]sRelativePath[@sDNSServerName]
	*/
	struct NPLFileName
	{
		/** the runtime state name that the file belongs to. It usually specifies which thread the file instance is running in. 
		If empty or "gl", it means the current or default runtime state. 
		It must be a string with only alphabetic letters. 
		*/
		string sRuntimeStateName;
		/** the NPL runtime server ID that the file belongs to. It usually represents a network endpoint (IP/port). 
		* However, instead of using IP address "192.168.0.111/60001", we usually use email like addresses, like "1001@paraengine.com"
		* the email address will latter be resolved to IP/port. if empty, it means the local runtime environment.
		* Specially, "all@local" means all remote NIDs connecting to this local machine.
		*/
		string sNID;
		/** the relative file path of the NPL file in the Runtime's file system. it uses forward slashes such as "script/sample.lua" 
		* if empty, it defaults to "script/empty.lua"
		*/
		string sRelativePath;
		/** the DNS file name. this is a global server where the sNID can be authenticated and converted to IP/port.  
		* if empty, it first defaults to the server part of sNID, if that is empty too, default to the current one in the local runtime environment. 
		* we rarely need to explicitly specify DNS server.
		*/ 
		string sDNSServerName;

	public:
		NPLFileName();

		/**
		* create the NPL file name object from a string. 
		* The string is of the following file format
		* @param filename: [(sRuntimeStateName|gl)][sNID:]sRelativePath[@sDNSServerName]
		* the following is a list of all valid file name combinations: 
		*	"user001@paraengine.com:script/hello.lua"	-- a file of user001 in its default gaming thread
		*	"(world1)server001@paraengine.com:script/hello.lua"		-- a file of server001 in its thread world1
		*	"(worker1)script/hello.lua"			-- a local file in the thread worker1
		*	"(gl)script/hello.lua"			-- a glia (local) file in the current runtime state's thread
		*	"script/hello.lua"			-- a file in the current thread. For a single threaded application, this is usually enough.
		*/
		NPLFileName(const char * filename);

		/** set the relaive_Path 
		* @param sPath: where the path string is.
		* @param nCount: the number of characters to copy from sPath. If this is negative. the whole sPath will be read.
		*/
		void SetRelativePath(const char* sPath, int nCount=-1);

	public:
		/** convert to string of the following format
		* [(sRuntimeStateName|gl)][sNID:]sRelativePath[@sDNSServerName]
		*/
		string ToString();
		void ToString(string & output);

		/** reset from string
		* @see NPLFileName() for details.
		*/
		void FromString(const char* filename);
	};

	/**
	* The globally unique address of NPL runtime. 
	* Usually it is IP:port, which is also mapped to an email like string, called NPL ID or NID. 
	* The NID is obtained when an IP:port is authenticated and retrieved the NID from a NPL name server.
	* The name server can be implemented via a jabber or web server.
	*
	* Always create using shared pointer like this: 
	*	NPLRuntimeAddress_ptr address(new NPLRuntimeAddress_ptr(...));
	*/
	struct NPLRuntimeAddress: 
		public boost::enable_shared_from_this<NPLRuntimeAddress>,
		private boost::noncopyable
	{
	public:
		NPLRuntimeAddress(const string& sHost, const string& sPort, const string& sNID)
			:m_sHost(sHost), m_sPort(sPort), m_sNID(sNID){};

		inline const std::string& GetHost() const {return m_sHost;} 
		inline const std::string& GetPort() const {return m_sPort;} 
		inline const std::string& GetNID() const {return m_sNID;}
		inline void SetNID(const char* nid) {m_sNID = nid;}
		inline void SetNID(const std::string& nid) {m_sNID = nid;}

	private:
		//
		// ip address and port are const, because NPL runtime address are immutable.
		//
		const std::string m_sHost;
		const std::string m_sPort;

		// nid may be changed during the course of authentication. 
		std::string m_sNID;
	};


	/**
	* Obsoleted: the address of NPL runtime environment. Normally, it is IP+port. 
	*/
	struct NPLAddress
	{
		string m_sAddress;
	public:
		NPLAddress(){m_sAddress = "";};
		NPLAddress(const char* sAddress){m_sAddress = sAddress;};

	public:
		/** whether the address is the local address */
		bool IsLocal(){return m_sAddress.empty();}
		/** TODO: get the IP address component in the address */
		string GetIP(){return "";};
		/** TODO: get the port component in the address */
		string GetPort(){return "";};
		/** convert to string. e.g. "192.168.0.10:4000" */
		string ToString(){return m_sAddress;}
	};

	/**
	* NOT USED YET: a list of NPL runtime address. This is from my 2005 design. 
	*/
	class UIReceivers
	{
	private:
		list <NPLAddress> m_listReceivers;
	public:
		UIReceivers(){m_listReceivers.clear();}

		/** returns true if the receiver is empty. */
		bool empty(){return m_listReceivers.empty();};

		/** add a new receiver from a string */
		void AddReceiver(const char* sAddress){
			if(sAddress!=0)
				m_listReceivers.push_back(NPLAddress(sAddress));
		};
		/** clean up*/
		void Cleanup(){
			m_listReceivers.clear();
		};
		/** convert to string. separated by ";". e.g. 
		* "192.168.0.10:4000;192.168.0.11:4001;" */
		string ToString(){
			string sReceivers = "";
			list<NPLAddress>::iterator itCurCP, itEndCP = m_listReceivers.end();
			for( itCurCP = m_listReceivers.begin(); itCurCP != itEndCP; ++ itCurCP){
				sReceivers += (*itCurCP).ToString() + ";";
			}
			return sReceivers;
		}
		/** append UI receivers from string. 
		* @param sReceivers: it can be of the string returned by UIReceivers.ToString(). */
		void FromString(const char* sReceivers){
			int nTo=0;
			int nFrom = 0;
			while( sReceivers[nTo]!='\0'){
				if(sReceivers[nTo] == ';'){
					string sAddress;
					sAddress.assign(sReceivers+nFrom, nTo-nFrom);
					AddReceiver(sAddress.c_str());
					nFrom = nTo+1;
				}
				nTo++;
			}
		}
	};

	/** timer struct */
	struct NPLTimer : public boost::enable_shared_from_this<NPLTimer>,
		private boost::noncopyable
	{
		const string m_nplFile;
		const string m_sCode;

	private:
		// interval in milliseconds
		DWORD  m_nInterval;
		// value returned from last activation call of ::timeGetTime()
		DWORD  m_lastTick;
		// mutex
		ParaEngine::mutex m_mutex;

	public:
		/**
		@param fInterval: interval in seconds
		*/
		NPLTimer(const string& nplFile, const string& sCode, float fInterval);

		/**
		@param nInterval: interval in milliseconds
		*/
		NPLTimer(const string& nplFile, const string& sCode, DWORD dwInterval);

	
		/**
		* Changes the start time and the interval between method invocations for a timer, using 32-bit signed integers to measure time intervals. 
		* [thread safe]
		* @param nIDEvent: Specifies the timer to be destroyed.For nIDEvent<=0, they are reserved for internal uses can not be killed by this function.
		* This value must be the same as the nIDEvent value passed to the SetTimer function that created the timer.
		* @param dueTime: The amount of time to delay before the invoking the callback method specified when the Timer was constructed, in milliseconds. Specify zero (0) to restart the timer immediately.
		* @param period:The time interval between invocations of the callback method specified when the Timer was constructed, in milliseconds. 
		*/
		void Change(int dueTime, int period);

		/** set the tick count. it will return true, if the timer is activated. 
		* call this function regularly with new tick count. 
		* [thread safe]
		* @param runtime_state: the NPL runtime state in which this is called. 
		* @param nTickCount: it should be ::GetTickCount() in millisecond. if 0, we will call the system ::GetTickCount() to get the current tick count. 
		* @return true if timer is activated. 
		*/
		bool Tick(NPLRuntimeState_ptr runtime_state, DWORD nTickCount = 0);
	};

	/**
	* Information of the local or a remote server. Such information is usually published publicly, such as
	* NID, IP, port of the server, whether the server can be trusted. 
	* @note: we will only establish connections with trusted servers. One can tell the NPL runtime which servers are trusted. 
	* callers normally add a number of trusted server infos before making any activation calls. 
	*/
	class NPLServerInfo : public boost::enable_shared_from_this<NPLServerInfo>
	{
	public:
		NPLServerInfo(const string& sNID, const string& sIP, const string& sPort, bool bTrusted=true)
			:m_sNID(sNID), m_sIP(sIP), m_sPort(sPort), m_bTrusted(bTrusted){};
		NPLServerInfo():m_bTrusted(true){};
	public:
		/// NID of the NPL server
		string m_sNID;
		/// this can be IP addresses or domain name, such as "localhost", "server.paraengine.com"
		string m_sIP;
		/// port of the server. 
		string m_sPort;

		/// whether we trust this server, if true, we will automatically connect to it when there is a NPL activation call to it. 
		bool m_bTrusted;
	};
}

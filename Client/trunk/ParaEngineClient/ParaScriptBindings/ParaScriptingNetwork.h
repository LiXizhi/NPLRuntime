//-----------------------------------------------------------------------------
// Copyright (C) 2004 - 2006 ParaEngine Dev Studio, All Rights Reserved.
// Date:	2006.8
// Description:	API for NPL Network Layer
//-----------------------------------------------------------------------------
#pragma once

#include <string>

using namespace std;

namespace NPL
{
	struct NPLPacket;
}
namespace luabind
{
	namespace adl{
		class object;
	}
	using adl::object;	
}

namespace ParaScripting
{
	using namespace luabind;

#ifdef _OBSOLETED
	/**
	* @ingroup ParaNetwork
	* NPL Network packet
	*/
	class PE_CORE_DECL ParaPacket
	{
	public:
		NPL::NPLPacket * m_pObj;

	public:
		ParaPacket():m_pObj(NULL){};
		ParaPacket(NPL::NPLPacket* obj){m_pObj=obj;};

		/// check if the object is valid
		bool IsValid(){return m_pObj!=NULL;}
		/// not used in script
		NPL::NPLPacket * GetNPLPacket(){return m_pObj;}
		/// release this packet, call this function when a packet is not needed. 
		void Release();
		/// same as Release();
		void ReleasePacketData();
		/** make this packet invalid.*/
		void InvalidateMe();
		
		/**
		* packet type
		* @return 
		*/
		int GetType() const;

		/**
		* @param type usually 103, which is a ID_NPL_CHAT
		*/
		void SetType( int type );

		string GetDestName() const;
		void SetDestName( const char * destName);
		string GetSrcName() const;
		void SetSrcName( const char * srcName);
		unsigned long GetSize() const;
		string GetData() const;
		void SetDataAndSize( const char * data, unsigned long size);
	};
#endif

#define MAP_STRING_BUFFER_LENGTH 1024
	/**
	* @ingroup ParaNetwork
	* API wrapper for NPL Network Layer functions
	*/
	class PE_CORE_DECL ParaNetwork
	{
	public:

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
		* whether network layer is running or not.
		* @return 
		*/
		static bool IsNetworkLayerRunning();


		/** set the IP and port for the nerve center: call this function before you enable the network (EnableNetwork) for it to take effect. 
		* @param pAddress: such as "127.0.0.1:60001", or ":60001" (if you want to use any available IP on the computer)
		*/
		static void SetNerveCenterAddress(const char* pAddress); 

		/** set the IP and port for the nerve receptor: call this function before you enable the network (EnableNetwork) for it to take effect. 
		* @param pAddress: such as "127.0.0.1:60000", or ":60000" (if you want to use any available IP on the computer)
		*/
		static void SetNerveReceptorAddress(const char* pAddress); 

#ifdef _OBSOLETED
		

		/**
		* local IP address. 
		* @return 
		*/
		static const char * GetLocalIP();
		static unsigned short GetLocalNerveCenterPort();
		static unsigned short GetLocalNerveReceptorPort();
		static const char * GetLocalNerveCenterName();

		/**
		* the current account name by which this computer is connected to a remote server
		* @return 
		*/
		static const char * GetLocalNerveReceptorAccountName();
		/**
		* the server world name that this computer is connected with. 
		* @return 
		*/
		static const char * GetLocalNerveReceptorConnectWorldName();

		/**
		* Not implemented
		* all connected remote client names separated by ';'
		* @return 
		*/
		static const char * GetNerveCenterUserMapAsString();
		/**
		* Not implemented
		* all connected remote server names separated by ';'
		* @return 
		*/
		static const char * GetNerveReceptorUserMapAsString();

		/** create a new packet. if a packet is not sent, it must be destroyed manually. using DestroyParaPacket().*/
		static ParaPacket CreateParaPacket();
		/** only used in API exportation. */
		static ParaPacket* CreateParaPacket_();

		/** destroy a packet. One must call this function or call a packet's release function if it is a received packet. */
		static void DestroyParaPacket(ParaPacket& packet);
		
		/**
		* send a packet to network. once a packet is sent, it will be automatically released by the network layer. 
		* the packet will be an invalid one upon return. 
		* @param packet 
		*/
		static void PushPacket(ParaPacket& packet);

		/**
		* set whether all network packets are written to net.text log file.
		*/
		static void EnableLog(bool bEnable);

		/**
		* get whether all network packets are written to net.text log file.
		*/
		static bool IsLogEnabled();

		/**
		* receive or pop a message from the current received message pool. 
		* @remark: one must manually release the received packet by calling Release() method when it is no longer needed. 
		* @return the returned packet is invalid if there are no longer any waiting packets. 
		*/
		static ParaPacket PopPacket();
		/** only used in API exportation. */
		static ParaPacket* PopPacket_();

		/// Insert a new name space binding record.
		/// @params [in] name: name space name, e.g. "Paraworld_Kids"
		/// @params [in] IP: address IP, e.g. "127.0.0.1"
		/// @params [in] port: address Port, e.g. "60000"
		/// @return: the index of the record in database, -1 if fail
		static int64 AddNamespaceRecord(const char * name, const char * IP, unsigned short port);
		/// this is only used in the scripting interface, since int64 is not a valid return type in the scripting interface.
		static int AddNamespaceRecord_s(const char * name, const char * IP, unsigned short port);

		/// Remove name space binding by address
		/// @param name: name space name
		/// @return: true if success, false if fail
		static bool RemoveNamespaceRecord(const char* name);

		/// account functions
		/// insert new account record
		static int64 InsertAccountRecord(const char * name, const char * password);

		/// verify account record using the account name and password
		static bool VerifyAccountRecord(const char * name, const char * password);

		/// establish a connection with a remote NPL runtime center
		/// make sure there is a name/IP binding before this function
		/// @param nameSpaceName: target NPL name space name
		static void ConnectToCenter(const char * nameSpaceName);

		/**
		* Close all connections (sending notification messages) and reconnect predefined ones using the current namespace mapping. 
		* All user connections and connection with the previous server can not be restored. So call this function, as if restarting the computer. 
		*/
		static void Restart();

		/** Close the connection to another host (if we initiated the connection it will disconnect, if they did it will kick them out).
		* @param [in] name Which system to close the connection to.*/
		static void CloseConnection( const char * name);

		/** Bans a player from connecting.  Banned players persist between connections but are not saved on shutdown nor loaded on startup.
		* @param [in] IP Dotted IP address. Can use * as a wildcard, such as 128.0.0.* will ban all IP addresses starting with 128.0.0
		* @param [in] milliseconds how many ms for a temporary ban.  Use 0 for a permanent ban */
		static void AddToBanList( const char *IP, unsigned int milliseconds);

		/** Allows a previously banned IP to connect. 
		* @param[in] Dotted IP address. Can use * as a wildcard, such as 128.0.0.* will banAll IP addresses starting with 128.0.0 */
		static void RemoveFromBanList( const char *IP );

		/** Allows all previously banned IPs to connect.*/
		static void ClearBanList( void );

		/// Returns true or false indicating if a particular IP is banned.
		/// \param[in] IP - Dotted IP address.
		/// \return true if IP matches any IPs in the ban list, accounting for any wildcards. False otherwise.
		static bool IsBanned( const char *IP );

		/**
		* get the user IP of a user.
		* @remark: not thread-safe.
		* @param name 
		* @return IP address without port. Not thread-safe
		*/
		static const char* GetUserIP(const char * name);

		/// Sets how many incoming connections are allowed. If this is less than the number of players currently connected,
		/// no more players will be allowed to connect.  If this is greater than the maximum number of peers allowed,
		/// it will be reduced to the maximum number of peers allowed.  Defaults to 0.
		/// \param[in] numberAllowed Maximum number of incoming connections allowed.
		static void SetMaximumIncomingConnections( unsigned short numberAllowed );

		/// Returns the value passed to SetMaximumIncomingConnections()
		/// \return the maximum number of incoming connections, which is always <= maxConnections
		static unsigned short GetMaximumIncomingConnections( void );

		/** 
		* get all the systems we are connected to in a array. in the following format {name="IP"}
		* e.g. local con_list = ParaNetwork.GetConnectionList({}); or local num = ParaNetwork.GetConnectionList(0)
		* @param remoteSystems: an empty table or 0. If it is 0, the number of remote systems that are currently connected to the current nerve center server. 
		*/
		static object GetConnectionList( const object& remoteSystems);
		
		/// Set the time, in MS, to use before considering ourselves disconnected after not being able to deliver a reliable message.
		/// Default time is 10,000 or 10 seconds in release and 30,000 or 30 seconds in debug.
		/// \param[in] timeMS Time, in MS
		/// \param[in] target Which system to do this for
		static void SetTimeoutTime( unsigned int timeMS, const char* target );

		/// Return the unique address identifier that represents you on the the network and is based on your local IP / port.
		/// \return the identifier of your system internally, which may not be how other systems see if you if you are behind a NAT or proxy
		/// this is not thread-safe.
		static const char* GetInternalID( void );

		/// Return the unique address identifier that represents you on the the network and is based on your externalIP / port
		/// (the IP / port the specified player uses to communicate with you)
		/// \param[in] target Which remote system you are referring to for your external ID.  Usually the same for all systems, unless you have two or more network cards.
		/// this is not thread-safe.
		static const char* GetExternalID( const char* target );

		/**
		* get a table containing a large set of network statistics for the specified system.
		* this is useful for monitoring the system.
		* e.g. local data = ParaNetwork.GetStatistics("LXZ",{});
		*/
		static object GetStatistics( const char* target, const object& data);
		static object GetReceptorStatistics( const char* target, const object& data);

		/// Verbosity level currently supports 0 (low), 1 (medium), 2 (high)
		/// \param[in] target remote or local system name. it can be ""  or nil.
		/// \param[in] verbosityLevel 
		/// 0 low
		/// 1 medium 
		/// 2 high 
		/// @note: function is not thread safe, the returned string is in a static buffer. Use it immediately, but do not preserve it.
		/// @return: it may return nil.
		static const char* GetStatisticsAsString( const char* target, int verbosityLevel);
		static const char* GetReceptorStatisticsAsString( const char* target, int verbosityLevel);
#endif		
	};
}

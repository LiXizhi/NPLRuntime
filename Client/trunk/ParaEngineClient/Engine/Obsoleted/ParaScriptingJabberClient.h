#pragma once
namespace ParaEngine
{
	class INPLJabberClient;
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
	/**
	* @ingroup JabberClient
	* NPL Jabber Client class
	@par Class Properties
	- ("User",&NPLJabberClient::GetUser,&NPLJabberClient::SetUser)
	- ("Priority",&NPLJabberClient::GetPriority,&NPLJabberClient::SetPriority)
	- ("Password",&NPLJabberClient::GetPassword,&NPLJabberClient::SetPassword)
	- ("AutoLogin",&NPLJabberClient::GetAutoLogin,&NPLJabberClient::SetAutoLogin)
	- ("AutoRoster",&NPLJabberClient::GetAutoRoster,&NPLJabberClient::SetAutoRoster)
	- ("AutoIQErrors",&NPLJabberClient::GetAutoIQErrors,&NPLJabberClient::SetAutoIQErrors)
	- ("Resource",&NPLJabberClient::GetAutoPresence,&NPLJabberClient::SetAutoPresence)
	- ("Resource",&NPLJabberClient::GetResource,&NPLJabberClient::SetResource)
	//- ("IsAuthenticated",&NPLJabberClient::GetIsAuthenticated,&NPLJabberClient::SetIsAuthenticated)
	- ("Server",&NPLJabberClient::GetServer,&NPLJabberClient::SetServer)
	- ("SetNetworkHost",&NPLJabberClient::GetNetworkHost,&NPLJabberClient::SetNetworkHost)
	- ("Port",&NPLJabberClient::GetPort,&NPLJabberClient::SetPort)
	- ("PlaintextAuth",&NPLJabberClient::GetPlaintextAuth,&NPLJabberClient::SetPlaintextAuth)
	- ("SetSSL",&NPLJabberClient::GetSSL,&NPLJabberClient::SetSSL)
	- ("AutoStartTLS",&NPLJabberClient::GetAutoStartTLS,&NPLJabberClient::SetAutoStartTLS)
	- ("AutoStartCompression",&NPLJabberClient::GetAutoStartCompression,&NPLJabberClient::SetAutoStartCompression)
	- ("KeepAlive",&NPLJabberClient::GetKeepAlive,&NPLJabberClient::SetKeepAlive)
	- ("AutoReconnect",&NPLJabberClient::GetAutoReconnect,&NPLJabberClient::SetAutoReconnect)
	- ("RequiresSASL",&NPLJabberClient::GetRequiresSASL,&NPLJabberClient::SetRequiresSASL)
	*/
	class PE_CORE_DECL NPLJabberClient
	{
	public:
		NPLJabberClient():m_pProxy(NULL){}
		NPLJabberClient(ParaEngine::INPLJabberClient* pProxy):m_pProxy(pProxy){}

		/**
		* check if object is invalid. .
		* @return 
		*/
		bool IsValid() {return m_pProxy!=0;};

		//////////////////////////////////////////////////////////////////////////
		//
		// attribute functions
		//
		//////////////////////////////////////////////////////////////////////////

		/** The username to connect as.*/
		void SetUser(const char* User);

		/** The username to connect as.*/
		const char* GetUser() const;

		/** Priority for this connection.*/
		void SetPriority(int nPriority);

		/** Priority for this connection.*/
		int GetPriority() const;

		/** The password to use for connecting.
		This may be sent across the wire plaintext, if the
		server doesn't support digest and PlaintextAuth is true.*/
		void SetPassword(const char* Password);

		/** The password to use for connecting.
		This may be sent across the wire plaintext, if the
		server doesn't support digest and PlaintextAuth is true.*/
		const char* GetPassword() const;

		/** Automatically log in on connection.*/
		void SetAutoLogin(bool AutoLogin);

		/** Automatically log in on connection.*/
		bool GetAutoLogin() const;

		/** Automatically retrieve roster on connection.*/
		void SetAutoRoster(bool AutoRoster);

		/** Automatically retrieve roster on connection.*/
		bool GetAutoRoster() const;

		/** Automatically send back 501/feature-not-implemented to IQs that have not been handled.*/
		void SetAutoIQErrors(bool AutoIQErrors);

		/** Automatically send back 501/feature-not-implemented to IQs that have not been handled.*/
		bool GetAutoIQErrors() const;

		/** Automatically send presence on connection.*/
		void SetAutoPresence(bool AutoPresence);

		/** Automatically send presence on connection.*/
		bool GetAutoPresence() const;


		/** The connecting resource. Used to identify a unique connection.*/
		void SetResource(const char* Resource);

		/** The connecting resource. Used to identify a unique connection.*/
		const char* GetResource() const;

		/**Are we currently connected?*/
		void SetIsAuthenticated(bool IsAuthenticated);

		/** Are we currently connected? */
		bool GetIsAuthenticated() const;

		/** The name of the server to connect to.*/
		void SetServer(const char* Server);

		/** The name of the server to connect to. */
		const char* GetServer() const;

		/** The address to use on the "to" attribute of the stream:stream.
		/// You can put the network hostname or IP address of the server to connect to.
		/// If none is specified, the Server will be used.
		/// Eventually, when SRV is supported, this will be deprecated.*/
		void SetNetworkHost(const char* NetworkHost);

		/** The address to use on the "to" attribute of the stream:stream.
		/// You can put the network hostname or IP address of the server to connect to.
		/// If none is specified, the Server will be used.
		/// Eventually, when SRV is supported, this will be deprecated.*/
		const char* GetNetworkHost() const;

		/** Automatically send presence on connection.*/
		void SetPort(int Port);

		/** The TCP port to connect to. default to 5222*/
		int GetPort() const;


		/** Allow plaintext authentication? default to false*/
		void SetPlaintextAuth(bool PlaintextAuth);

		/** Allow plaintext authentication? default to false*/
		bool GetPlaintextAuth() const;

		/** Is the current connection SSL/TLS protected? */
		bool IsSSLon() const;

		/** Is the current connection XEP-138 compressed? */
		bool IsCompressionOn() const;

		/**Do SSL3/TLS1 on startup*/
		void SetSSL(bool SSL);

		/** Do SSL3/TLS1 on startup*/
		bool GetSSL() const;

		/** Allow Start-TLS on connection, if the server supports it*/
		void SetAutoStartTLS(bool AutoStartTLS);

		/** Allow Start-TLS on connection, if the server supports it*/
		bool GetAutoStartTLS() const;

		/** Allow start compression on connection, if the server supports it*/
		void SetAutoStartCompression(bool AutoStartCompression);

		/** Allow start compression on connection, if the server supports it*/
		bool GetAutoStartCompression() const;

		/** Time, in seconds, between keep-alive spaces. default to 20f*/
		void SetKeepAlive(float KeepAlive);

		/** Time, in seconds, between keep-alive spaces. default to 20f*/
		float GetKeepAlive() const;

		/** Seconds before automatically reconnecting if the connection drops.  -1 to disable, 0 for immediate. Default to 30 seconds */
		void SetAutoReconnect(float AutoReconnect);

		/** Seconds before automatically reconnecting if the connection drops.  -1 to disable, 0 for immediate. Default to 30 seconds */
		float GetAutoReconnect() const;


		/** The version string returned in the server's open stream element. */
		const char* GetServerVersion() const;

		/** Is SASL required?  This will default to true in the future.*/
		void SetRequiresSASL(bool RequiresSASL);

		/** Is SASL required?  This will default to true in the future. */
		bool GetRequiresSASL() const;

		/** Returns a StatisticsStruct containing byte and stanza counts for the current active connection. 
		* @param inout: the input|output table. usually this is an empty table. 
		* @return: a struct containing the current connection's statistics. 
		* e.g. local stats = jc:GetStatistics({}); stats
		stats fields: 
		int totalBytesSent;             // < The total number of bytes sent over the wire. This does @b not
										* include the TLS handshake nor any TLS-related overhead, but it does
										* include anything sent before compression was switched on. 
		int totalBytesReceived;         // < The total number of bytes received over the wire. This does @b not
										* include the TLS handshake nor any TLS-related overhead, but it does
										* include anything sent before compression was switched on. 
		int compressedBytesSent;        // < Total number of bytes sent over the wire after compression was
										* applied. 
		int compressedBytesReceived;    // < Total number of bytes received over the wire before decompression
										* was applied. 
		int uncompressedBytesSent;      // < Total number of bytes sent over the wire before compression was
										* applied. 
		int uncompressedBytesReceived;  // < Total number of bytes received over the wire after decompression
										* was applied. 
		int totalStanzasSent;           // < The total number of Stanzas sent. 
		int totalStanzasReceived;       // < The total number of Stanzas received. 
		int iqStanzasSent;              // < The total number of IQ Stanzas sent. 
		int iqStanzasReceived;          // < The total number of IQ Stanzas received. 
		int messageStanzasSent;         // < The total number of Message Stanzas sent. 
		int messageStanzasReceived;     // < The total number of Message Stanzas received. 
		int s10nStanzasSent;            // < The total number of Subscription Stanzas sent. 
		int s10nStanzasReceived;        // < The total number of Subscription Stanzas received. 
		int presenceStanzasSent;        // < The total number of Presence Stanzas sent. 
		int presenceStanzasReceived;    // < The total number of Presence Stanzas received. 
		bool encryption;                // < Whether or not the connection (to the server) is encrypted. 
		bool compression;               // < Whether or not the stream (to the server) gets compressed. 
		*/
		object GetStatistics( const object& inout );

		//////////////////////////////////////////////////////////////////////////
		//
		// basic functions
		//
		//////////////////////////////////////////////////////////////////////////

		/** Connect to the server.  This happens asynchronously, and
		could take a couple of seconds to get the full handshake
		completed.  This will auth, send presence, and request
		roster info, if the Auto* properties are set.
		*/
		bool Connect();

		/** Close down the connection, as gracefully as possible.*/
		void Close();

		/**
		* activate or sending a message to a target This function is similar to NPL.activate(), except that it only accept Jabber ID as destination. .
		* 
		* @param sDestination: format: JID[:neuron_filepath]
		* JID or jabber ID is in the format:  username@servername
		*  e.g.
		*	"lixizhi@paraengine.com:script/network/client.lua". the target NPL runtime's neuron file  will receive message by its activation function.
		*	"lixizhi@paraengine.com" if no neuron file is specified. the sCode will be regarded as an ordinary Jabber:XMPP:Chat message body. 
		* @Note: please Node that offline message is NOT supported when a neuron_filepath is specified. 
		*  TODO: when a neuron file receives a msg. The msg table automatically has following addition fields filled with valid values
		*    msg.sender: the sender's JID
		*    msg.time: the time at which the sender send a message. 
		* @param sCode: If it is a string, it is regarded as a chunk of secure msg code that should be executed in the destination neuron file's runtime. 
		*	If this is a table or number, it will be transmitted via a internal variable called "msg". When activating neurons on a remote network, only pure data table is allowed in the sCode.
		* @note: pure data table is defined as table consisting of only string, number and other table of the above type. 
		*   NPL.activate function also accepts ParaFileObject typed message data type. ParaFileObject will be converted to base64 string upon transmission. There are size limit though of 10MB.
		*   one can also programmatically check whether a script object is pure date by calling NPL.SerializeToSCode() function. Please note that data types that is not pure data in sCode will be ignored instead of reporting an error.
		* @return: if true, message is put to the output queue. if output queue is full, the function will return false. And one should possible report service unavailable. 
		*/
		bool activate(const char * sDestination, const object& sCode);

		/** Send a NPL message. 
		* @param to: JID such as lxz@paraengine.com
		* @param neuronfile: a NPL table converted to secure code.
		* @param sCode: must be pure msg data, such as "msg = {x=0}"
		*/
		bool WriteNPLMessage(const char* to, const char* neuronfile,  const char* sCode);

		/** Send raw string. */
		bool WriteRawString(const char* rawstring);

		/** Initiate the auth process.*/
		bool Login();

		/**
		/// Send a presence packet to the server
		/// <param name="t">What kind? 
		public enum PresenceType
		{
		/// None specified
		available = -1,
		/// May I subscribe to you?
		subscribe,
		/// Yes, you may subscribe.
		subscribed,
		/// Unsubscribe from this entity.
		unsubscribe,
		/// No, you may not subscribe.
		unsubscribed,
		/// Offline
		unavailable,
		/// server-side only.
		probe,
		/// A presence error.
		error,
		/// Invisible presence: we're unavailable to them, but still see theirs.
		invisible
		}
		</param>
		/// <param name="status">How to show us?</param>
		/// <param name="show">away, dnd, etc.</param>
		/// <param name="priority">How to prioritize this connection.
		/// Higher number mean higher priority.  0 minumum, 127 max.  
		/// -1 means this is a presence-only connection.</param>
		*/
		bool SetPresence(int t,const char* status,const char* show,int priority);

		/**
		/// Send a message packet to another user
		/// <param name="t">What kind?
		public enum MessageType
		{
		/// Normal message
		normal = -1,
		/// Error message
		error,
		/// Chat (one-to-one) message
		chat,
		/// Groupchat
		groupchat,
		/// Headline
		headline
		}
		</param>
		/// <param name="to">Who to send it to?</param>
		/// <param name="body">The message.</param>
		*/
		bool Message(int t, const char* to, const char* body);

		/// Send a message packet to another user
		/// <param name="to">Who to send it to?</param>
		/// <param name="body">The message.</param>
		bool Message2(const char* to, const char* body);

		/** Get a full roster 
		* @return: the returned string is NPL table of the following 
		{
		{jid=string:jid, name=string,subscription=int:SubscriptionEnum, online=true, groups={string, string, ...}, resources={name={presence=int, priority=int, message=string}, ...},},
		{jid=string:jid, name=string,subscription=int:SubscriptionEnum, online=true, groups={string, string, ...}, resources={name={presence=int, priority=int, message=string}, ...},},
		{jid=string:jid, name=string,subscription=int:SubscriptionEnum, groups={string, string, ...}, resources={name={presence=int, priority=int, message=string}, ...},},
		...
		}
		In most cases, each jid has only one resource, but multiple is supported. Item presence info is in resources[name] table.
		{presence=int, priority=int, message=string}, where priority is resource priority, presence is the Presence Enum, message is the user message after its name. 
		*/
		const char* GetRoster();

		/**
		* Use this function to subscribe to a new JID. The contact is added to the roster automatically
		* (by compliant servers, as required by RFC 3921).
		* @param jid The address to subscribe to.
		* @param name The displayed name of the contact.
		* @param groups A list of groups the contact belongs to. separated by ";". Currently only one group is supported. 
		* @param msg A message sent along with the request.
		*/
		bool Subscribe(const char* jid, const char* name, const char* groups, const char* msg);

		/**
		* Use this function to unsubscribe from a contact's presence. You will no longer
		* receive presence from this contact.
		* This will have the side-effect of bi-directionally unsubscribing to/from the user.
		* @param to: The JID to remove
		* @param msg A message to send along with the request.
		*/
		void Unsubscribe(const char* to, const char* msg);


		/**
		* this function is called in OnSubscription method to confirm or refuse a subscription request from another user. 
		* @param to: the JID
		* @param bAllow: true to allow subscription, false to deny
		*/
		bool AllowSubscription(const char* to, bool bAllow);

		//////////////////////////////////////////////////////////////////////////
		// OBSOLETED methods
		//////////////////////////////////////////////////////////////////////////

		/** OBSOLETED:
		* Request a list of agents from the server
		*/
		void GetAgents();


		/** OBSOLETED: 
		* in Jabber, a contact list is called a roster
		/// Remove an item from the roster.  
		/// This will have the side-effect of bi-directionally unsubscribing to/from the user.
		/// <param name="to">The JID to remove</param>
		*/
		void RemoveRosterItem(const char* to, const char* msg);


		/// OBSOLETED: Attempt to register a new user.  This will fire OnRegisterInfo to retrieve
		/// information about the new user, and OnRegistered when the registration is complete or failed.
		/// <param name="jid">The user to register</param>
		bool Register(const char* jid);


		/// OBSOLETED: Retrieve details of a given roster item.
		/// <param name="jid"></param>
		/// <returns>
		/// null or string. the string is an NPL table of the following format
		/// "{ [1]={NickName=string, presenceType=number(-1 means available, positive values means one of the unavailable type),groupname=string, presenceStatus = "string", presenceShow = string(a short string about detailed available state, such as busy, chatty, etc)}, [2]=...}"
		/// Usually a roster item only belongs to one group. so the table usually contains only one item at index 1.
		/// </returns>
		const char* GetRosterItemDetail(const char* jid);

		/** OBSOLETED: 
		* The group names for the roster as a string. each group name is separated by ;
		* such as "general;friends;relatives;"
		*/
		const char* GetRosterGroups();

		/** OBSOLETED
		* The Roster Item names as a string. each name is separated by ;
		* such as "Xizhi;Andy;Clayman;"
		*/
		const char* GetRosterItems();

		/** to create an account:
		// - create the Client instance by passing it the server only, e.g. j = new Client( "example.net" );
		// - in onConnect() (or some similar place) call Registration::fetchRegistrationFields()
		// - in RegistrationHandler::handleRegistrationFields() set up an RegistrationFields struct
		//   with the desired username/password and call Registration::createAccount()
		*/
		void RegisterCreateAccount(const char* username, const char* password);
		/** to change an account's password:
		// - connect to the server as usual
		// - in onConnect() or at any later time call Registration::changePassword()
		*/
		void RegisterChangePassword(const char* password);

		/** to delete an account:
		// - connect to the server as usual
		// - in onConnect() or at any later time call Registration::removeAccount()
		*/
		void RegisterRemoveAccount();

		//////////////////////////////////////////////////////////////////////////
		//
		// event functions
		//
		//////////////////////////////////////////////////////////////////////////

		/**
		/// <summary>
		/// add a NPL call back script to a given even listener
		/// </summary>
		/// <param name="nListenerType">type of message listener. The user can register NPL callback script with any one listener type. 
		enum JABBERLISTENER_TYPE
		{
			Jabber_OnPresence = 0,//We received a presence packet.
			Jabber_OnError,
			Jabber_OnRegistered,//After calling Register(), the registration succeeded or failed.
			Jabber_OnRegisterInfo,//after calling Register, information about the user is required.  Fill in the given IQ with the requested information.
			Jabber_OnIQ,//We received an IQ packet.
			Jabber_OnMessage,//We received a message packet.
			Jabber_OnAuthError,//Authentication failed.  The connection is not terminated if there is an auth error and there is at least one event handler for this event.
			Jabber_OnLoginRequired,//AutoLogin is false, and it's time to log in.

			Jabber_OnConnect, // The connection is connected, but no stream:stream has been sent, yet.
			Jabber_OnAuthenticate, // The connection is complete, and the user is authenticated.
			Jabber_OnDisconnect, // The connection is disconnected

			Jabber_OnRosterEnd, //Fired when a roster result is completed being processed.
			Jabber_OnRosterBegin, // Fired when a roster result starts, before any OnRosterItem events fire.
			Jabber_OnRosterItem, //event for new roster items. A roster may belong to multiple groups
		};
		</param>
		/// <param name="callbackScript">the script to be called when the listener event is raised. Usually parameters are stored in a NPL parameter called "msg".</param>
		*/
		void AddEventListener(int nListenerType, const char*  callbackScript);
		void AddEventListener1(const char* sListenerType, const char*  callbackScript);

		/**
		/// <summary>
		/// remove a NPL call back script from a given even listener
		/// </summary>
		/// <param name="nListenerType">type of message listener. The user can register NPL callback script with any one listener type. 
		enum JABBERLISTENER_TYPE
		{
			Jabber_OnPresence = 0,//We received a presence packet.
			Jabber_OnError,
			Jabber_OnRegistered,//After calling Register(), the registration succeeded or failed.
			Jabber_OnRegisterInfo,//after calling Register, information about the user is required.  Fill in the given IQ with the requested information.
			Jabber_OnIQ,//We received an IQ packet.
			Jabber_OnMessage,//We received a message packet.
			Jabber_OnAuthError,//Authentication failed.  The connection is not terminated if there is an auth error and there is at least one event handler for this event.
			Jabber_OnLoginRequired,//AutoLogin is false, and it's time to log in.

			Jabber_OnConnect, // The connection is connected, but no stream:stream has been sent, yet.
			Jabber_OnAuthenticate, // The connection is complete, and the user is authenticated.
			Jabber_OnDisconnect, // The connection is disconnected

			Jabber_OnRosterEnd, //Fired when a roster result is completed being processed.
			Jabber_OnRosterBegin, // Fired when a roster result starts, before any OnRosterItem events fire.
			Jabber_OnRosterItem, //event for new roster items. A roster may belong to multiple groups
		};
		</param>
		/// <param name="callbackScript">the script to be called when the listener event is raised. Usually parameters are stored in a NPL parameter called "msg".</param>
		*/
		void RemoveEventListener(int nListenerType, const char*  callbackScript);
		void RemoveEventListener1(const char* sListenerType, const char*  callbackScript);

		/// <summary>
		/// clear all NPL call back script from a given even listener
		/// </summary>
		/// <param name="nListenerType">type of message listener. The user can register NPL callback script with any one listener type. </param>
		void ClearEventListener(int nListenerType);
		void ClearEventListener1(const char* sListenerType);

		/// <summary>
		/// clear all registered event listeners
		/// </summary>
		void ResetAllEventListeners();
	public:
		ParaEngine::INPLJabberClient* m_pProxy;
	};


	/** 
	* @ingroup JabberClient
	* jabber client manager or factory
	*/
	class PE_CORE_DECL JabberClientManager
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		//
		// jabber functions
		//
		//////////////////////////////////////////////////////////////////////////

		/**
		* get an existing jabber client instance interface by its JID.
		* If the client is not created using CreateJabberClient() before, function may return NULL.
		* @param sJID: such as "lixizhi@paraweb3d.com"
		*/
		static NPLJabberClient GetJabberClient(const char* sJID);

		/**
		* Create a new jabber client instance with the given jabber client ID. It does not open a connection immediately.
		* @param sJID: such as "lixizhi@paraweb3d.com"
		*/
		static NPLJabberClient CreateJabberClient(const char* sJID);

		/**
		* close a given jabber client instance. Basically, there is no need to close a web service, 
		* unless one wants to reopen it with different credentials
		* @param sJID: such as "lixizhi@paraweb3d.com", if this is "", it will close all jabber clients.
		*/
		static bool CloseJabberClient(const char* sJID);

		/** add string a string mapping. We will automatically encode NPL filename string if it is in this string map. It means shorter message sent over the network. 
		* use AddStringMap whenever you want to add a string to the map. Please note, that the sender and the receiver must maintain the same string map in memory in order to have consistent string translation result.
		* the function is static, it will apply to all client instances. 
		* @param nID: the integer to encode the string. it is usually positive. 
		* @param sString: the string for the id. if input is NULL, it means removing the mapping of nID. 
		*/
		static void AddStringMap(int nID, const char* sString);
		/** clear all string mapping */
		static void ClearStringMap();
	};
}
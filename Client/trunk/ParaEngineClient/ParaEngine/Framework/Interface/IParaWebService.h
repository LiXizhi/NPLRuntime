//-----------------------------------------------------------------------------
// Copyright (C) 2004 - 2007 ParaEngine Corporation, All Rights Reserved.
// Author: LiXizhi
// Email: LiXizhi@yeah.net
//-----------------------------------------------------------------------------
#pragma once

#define WEBSERVICECLIENT_CLASS_ID		0x100A20

namespace ParaEngine
{
	class CURLRequestTask;

	/**
	* A structure describing the current jabber connection statistics.
	*/
	struct StatisticsJabber
	{
		long int totalBytesSent;             /**< The total number of bytes sent over the wire. This does @b not
										* include the TLS handshake nor any TLS-related overhead, but it does
										* include anything sent before compression was switched on. */
		long int totalBytesReceived;         /**< The total number of bytes received over the wire. This does @b not
										* include the TLS handshake nor any TLS-related overhead, but it does
										* include anything sent before compression was switched on. */
		int compressedBytesSent;        /**< Total number of bytes sent over the wire after compression was
										* applied. */
		int compressedBytesReceived;    /**< Total number of bytes received over the wire before decompression
										* was applied. */
		int uncompressedBytesSent;      /**< Total number of bytes sent over the wire before compression was
										* applied. */
		int uncompressedBytesReceived;  /**< Total number of bytes received over the wire after decompression
										* was applied. */
		int totalStanzasSent;           /**< The total number of Stanzas sent. */
		int totalStanzasReceived;       /**< The total number of Stanzas received. */
		int iqStanzasSent;              /**< The total number of IQ Stanzas sent. */
		int iqStanzasReceived;          /**< The total number of IQ Stanzas received. */
		int messageStanzasSent;         /**< The total number of Message Stanzas sent. */
		int messageStanzasReceived;     /**< The total number of Message Stanzas received. */
		int s10nStanzasSent;            /**< The total number of Subscription Stanzas sent. */
		int s10nStanzasReceived;        /**< The total number of Subscription Stanzas received. */
		int presenceStanzasSent;        /**< The total number of Presence Stanzas sent. */
		int presenceStanzasReceived;    /**< The total number of Presence Stanzas received. */
		bool encryption;                /**< Whether or not the connection (to the server) is encrypted. */
		bool compression;               /**< Whether or not the stream (to the server) gets compressed. */
	};

	/**
	* NPL interface of a XML Web service client proxy 
	*/
	class INPLWebService
	{
	public:
		/**
		* Invokes an XML Web service method synchronously using SOAP.
		* @param OperationName: if this is NULL or "", the name "Activate" is assumed.
		* @param msg: NPL sCode, which must be a "msg" variable. e.g. msg={op="add", x=1, y=1};
		* @return: NPL sCode, which is a "msg" variable. e.g. msg={result=2};"
		*/
		virtual const char* Invoke(const char* OperationName, const char* msg)=0;
		/**
		* Invokes the specified method asynchronously. 
		* @param OperationName: if this is NULL or "", the name "Activate" is assumed.
		* @param msg: NPL sCode, which must be a "msg" variable. e.g. msg={op="add", x=1, y=1};
		* @param sCallBackCode: code to be executed upon return. The returning NPL code 
		*	from the web service is executed before the sCallBackCode.
		*/
		virtual void InvokeAsync(const char* OperationName, const char* msg, const char* sCallBackCode)=0;

		/** Gets or sets the base URL of the XML Web service the client is requesting. */
		virtual const char* GetURL()=0;

		/**
		* Abort cancels a synchronous XML Web service request. Since a synchronous request 
		* will block the thread until the response has been processed you must call Abort from a separate thread. 
		*/
		virtual void Abort()=0;
	};

	/** JABBERLISTENER_TYPE */
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

		Jabber_OnSubscription, // a new users subscribes to this user
		Jabber_OnUnsubscription, // a user unsubscribes from this user

		NPL_OnNPLMessage,//We received a NPL message. NPL message has no listeners
	};

	/**
	* NPL interface of a Jabber-XMPP client proxy 
	*/
	class INPLJabberClient
	{
	public:
		//////////////////////////////////////////////////////////////////////////
		//
		// attribute functions
		//
		//////////////////////////////////////////////////////////////////////////

		/** The username to connect as.*/
		virtual void SetUser(const char* User) = 0;

		/** The username to connect as.*/
		virtual const char* GetUser() = 0;

		/** Priority for this connection.*/
		virtual void SetPriority(int nPriority) = 0;

		/** Priority for this connection.*/
		virtual int GetPriority() = 0;

		/** The password to use for connecting.
		This may be sent across the wire plaintext, if the
		server doesn't support digest and PlaintextAuth is true.*/
		virtual void SetPassword(const char* Password) = 0;

		/** The password to use for connecting.
		This may be sent across the wire plaintext, if the
		server doesn't support digest and PlaintextAuth is true.*/
		virtual const char* GetPassword() = 0;

		/** Automatically log in on connection.*/
		virtual void SetAutoLogin(bool AutoLogin) = 0;

		/** Automatically log in on connection.*/
		virtual bool GetAutoLogin() = 0;

		/** Automatically retrieve roster on connection.*/
		virtual void SetAutoRoster(bool AutoRoster) = 0;

		/** Automatically retrieve roster on connection.*/
		virtual bool GetAutoRoster() = 0;

		/** Automatically send back 501/feature-not-implemented to IQs that have not been handled.*/
		virtual void SetAutoIQErrors(bool AutoIQErrors) = 0;

		/** Automatically send back 501/feature-not-implemented to IQs that have not been handled.*/
		virtual bool GetAutoIQErrors() = 0;

		/** Automatically send presence on connection.*/
		virtual void SetAutoPresence(bool AutoPresence) = 0;

		/** Automatically send presence on connection.*/
		virtual bool GetAutoPresence() = 0;


		/** The connecting resource. Used to identify a unique connection.*/
		virtual void SetResource(const char* Resource) = 0;

		/** The connecting resource. Used to identify a unique connection.*/
		virtual const char* GetResource() = 0;

		/**Are we currently connected?*/
		virtual void SetIsAuthenticated(bool IsAuthenticated) = 0;

		/** Are we currently connected? */
		virtual bool GetIsAuthenticated() = 0;

		/** The name of the server to connect to.*/
		virtual void SetServer(const char* Server) = 0;

		/** The name of the server to connect to. */
		virtual const char* GetServer() = 0;

		/** The address to use on the "to" attribute of the stream:stream.
		/// You can put the network hostname or IP address of the server to connect to.
		/// If none is specified, the Server will be used.
		/// Eventually, when SRV is supported, this will be deprecated.*/
		virtual void SetNetworkHost(const char* NetworkHost) = 0;

		/** The address to use on the "to" attribute of the stream:stream.
		/// You can put the network hostname or IP address of the server to connect to.
		/// If none is specified, the Server will be used.
		/// Eventually, when SRV is supported, this will be deprecated.*/
		virtual const char* GetNetworkHost() = 0;

		/** Automatically send presence on connection.*/
		virtual void SetPort(int Port) = 0;

		/** The TCP port to connect to. default to 5222*/
		virtual int GetPort() = 0;


		/** Allow plaintext authentication? default to false*/
		virtual void SetPlaintextAuth(bool PlaintextAuth) = 0;

		/** Allow plaintext authentication? default to false*/
		virtual bool GetPlaintextAuth() = 0;

		/** Is the current connection SSL/TLS protected? */
		virtual bool IsSSLon() = 0;

		/** Is the current connection XEP-138 compressed? */
		virtual bool IsCompressionOn() = 0;

		/**Do SSL3/TLS1 on startup*/
		virtual void SetSSL(bool SSL) = 0;

		/** Do SSL3/TLS1 on startup*/
		virtual bool GetSSL() = 0;

		/** Allow Start-TLS on connection, if the server supports it*/
		virtual void SetAutoStartTLS(bool AutoStartTLS) = 0;

		/** Allow Start-TLS on connection, if the server supports it*/
		virtual bool GetAutoStartTLS() = 0;

		/** Allow start compression on connection, if the server supports it*/
		virtual void SetAutoStartCompression(bool AutoStartCompression) = 0;

		/** Allow start compression on connection, if the server supports it*/
		virtual bool GetAutoStartCompression() = 0;

		/** Time, in seconds, between keep-alive spaces. default to 20f*/
		virtual void SetKeepAlive(float KeepAlive) = 0;

		/** Time, in seconds, between keep-alive spaces. default to 20f*/
		virtual float GetKeepAlive() = 0;

		/** Seconds before automatically reconnecting if the connection drops.  -1 to disable, 0 for immediate. Default to 30 seconds */
		virtual void SetAutoReconnect(float AutoReconnect) = 0;

		/** Seconds before automatically reconnecting if the connection drops.  -1 to disable, 0 for immediate. Default to 30 seconds */
		virtual float GetAutoReconnect() = 0;


		/** The version string returned in the server's open stream element. */
		virtual const char* GetServerVersion() = 0;

		/** Is SASL required?  This will default to true in the future.*/
		virtual void SetRequiresSASL(bool RequiresSASL) = 0;

		/** Is SASL required?  This will default to true in the future. */
		virtual bool GetRequiresSASL() = 0;

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
		virtual bool Connect() = 0;

		/** Close down the connection, as gracefully as possible.*/
		virtual void Close() = 0;

		/** Send a NPL message. 
		* @param msg: a NPL table converted to secure code.
		*/
		virtual bool WriteNPLMessage(const char* to, const char* neuronfile,  const char* sCode) = 0;

		/** Send raw string. */
		virtual bool WriteRawString(const char* rawstring) = 0;

		/** Initiate the auth process.*/
		virtual bool Login() = 0;

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
		virtual bool SetPresence(int t,const char* status,const char* show,int priority) = 0;

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
		virtual bool SendMessage(int t, const char* to, const char* body) = 0;

		/// Send a message packet to another user
		/// <param name="to">Who to send it to?</param>
		/// <param name="body">The message.</param>
		virtual bool SendMessage(const char* to, const char* body) = 0;

		/** Request a new copy of the roster.*/
		virtual const char* GetRoster() = 0;

		/**
		* in Jabber, a contact list is called a roster
		/// Send a presence subscription request and update the roster for a new roster contact.
		/// <param name="to">The JID of the contact (required)</param>
		/// <param name="nickname">The nickname to show for the user.</param>
		/// <param name="groups">A list of groups to put the contact in. Groups are separated by ';'. May be null.  Hint: "foo;bar;" or "foo"</param>
		*/
		virtual bool Subscribe(const char* to, const char* nickname, const char* groups, const char* msg) = 0;

		/**
		* in Jabber, a contact list is called a roster
		/// Remove an item from the roster.  
		/// This will have the side-effect of bi-directionally unsubscribing to/from the user.
		/// <param name="to">The JID to remove</param>
		*/
		virtual void Unsubscribe(const char* to, const char* msg) = 0;
		virtual void RemoveRosterItem(const char* to, const char* msg) = 0;


		/**
		* this function is called in OnSubscription method to confirm or refuse a subscription request from another user. 
		* @param to: the JID
		* @param bAllow: true to allow subscription, false to deny
		*/
		virtual bool AllowSubscription(const char* to, bool bAllow) = 0;

		/**
		* Request a list of agents from the server
		*/
		virtual void GetAgents() = 0;


		/// Attempt to register a new user.  This will fire OnRegisterInfo to retrieve
		/// information about the new user, and OnRegistered when the registration is complete or failed.
		/// <param name="jid">The user to register</param>
		virtual bool Register(const char* jid) = 0;


		/// Retrieve details of a given roster item.
		/// <param name="jid"></param>
		/// <returns>
		/// null or string. the string is an NPL table of the following format
		/// "{ [1]={text=[[NickName(status)]], presence=number,groupname=[[name]]}, [2]=...}"
		/// Usually a roster item only belongs to one group. so the table usually contains only one item at index 1.
		/// </returns>
		virtual const char* GetRosterItemDetail(const char* jid) = 0;

		/**
		* The group names for the roster as a string. each group name is separated by ;
		* such as "general;friends;relatives;"
		*/
		virtual const char* GetRosterGroups() = 0;

		/**
		* The Roster Item names as a string. each name is separated by ;
		* such as "Xizhi;Andy;Clayman;"
		*/
		virtual const char* GetRosterItems() = 0;

		/** to create an account:
		// - create the Client instance by passing it the server only, e.g. j = new Client( "example.net" );
		// - in onConnect() (or some similar place) call Registration::fetchRegistrationFields()
		// - in RegistrationHandler::handleRegistrationFields() set up an RegistrationFields struct
		//   with the desired username/password and call Registration::createAccount()
		*/
		virtual void RegisterCreateAccount(const char* username, const char* password)=0;
		/** to change an account's password:
		// - connect to the server as usual
		// - in onConnect() or at any later time call Registration::changePassword()
		*/
		virtual void RegisterChangePassword(const char* password)=0;
		/** to delete an account:
		// - connect to teh server as usual
		// - in onConnect() or at any later time call Registration::removeAccount()
		*/
		virtual void RegisterRemoveAccount()=0;

		/**
		* Returns a StatisticsJabber containing byte and stanza counts for the current active connection. 
		* @return: a struct containing the current connection's statistics. 
		*/
		virtual StatisticsJabber GetStatistics() = 0;

		//////////////////////////////////////////////////////////////////////////
		//
		// event functions
		//
		//////////////////////////////////////////////////////////////////////////

		/// <summary>
		/// add a NPL call back script to a given even listener
		/// </summary>
		/// <param name="nListenerType">type of message listener. The user can register NPL callback script with any one listener type. </param>
		/// <param name="callbackScript">the script to be called when the listener event is raised. Usually parameters are stored in a NPL parameter called "msg".</param>
		virtual void AddEventListener(int nListenerType, const char*  callbackScript) = 0;
		virtual void AddEventListener1(const char* sListenerType, const char*  callbackScript) = 0;

		/// <summary>
		/// remove a NPL call back script from a given even listener
		/// </summary>
		/// <param name="nListenerType">type of message listener. The user can register NPL callback script with any one listener type. </param>
		/// <param name="callbackScript">the script to be called when the listener event is raised. Usually parameters are stored in a NPL parameter called "msg".</param>
		virtual void RemoveEventListener(int nListenerType, const char*  callbackScript) = 0;
		virtual void RemoveEventListener1(const char* sListenerType, const char*  callbackScript) = 0;

		/// <summary>
		/// clear all NPL call back script from a given even listener
		/// </summary>
		/// <param name="nListenerType">type of message listener. The user can register NPL callback script with any one listener type. </param>
		virtual void ClearEventListener(int nListenerType) = 0;
		virtual void ClearEventListener1(const char* sListenerType) = 0;

		/// <summary>
		/// clear all registered event listeners
		/// </summary>
		virtual void ResetAllEventListeners() = 0;
	};

	/**
	* interface of NPL web service client.
	* this class is usually implemented by .Net plug-in. See ParaEnginePlugin/NPLWebServiceClient
	* class ID is WEBSERVICECLIENT_CLASS_ID
	*/
	class INPLWebServiceClient
	{
	public:
		/**
		* get the web service interface by its URL, if the web service does not exists it will return NULL, in which case 
		* one needs to call OpenWebService() 
		*/
		virtual INPLWebService* GetWebService(const char* sURL)=0;
		/**
		* open a new web service with optional credentials
		* @param sURL: url of the web service, such as http://paraengine.com/test.asmx
		* @param sUserName the user name associated with the credentials
		* @param sPassword the password for the user name associated with the credentials.
		* @param sDomain the domain or computer name that verifies the credentials
		*/
		virtual INPLWebService* OpenWebService(const char* sURL, const char* sUserName, const char* sPassword, const char* sDomain)=0;

		/**
		* close a given web service. Basically, there is no need to close a web service, 
		* unless one wants to reopen it with different credentials
		*/
		virtual bool CloseWebService(const char* sURL)=0;

		/**
		* get an existing jabber client instance interface by its JID.
		* If the client is not created using CreateJabberClient() before, function may return NULL.
		* @param sJID: such as "lixizhi@paraweb3d.com"
		*/
		virtual INPLJabberClient* GetJabberClient(const char* sJID)=0;
		/**
		* Create a new jabber client instance with the given jabber client ID. It does not open a connection immediately.
		* @param sJID: such as "lixizhi@paraweb3d.com"
		*/
		virtual INPLJabberClient* CreateJabberClient(const char* sJID)=0;

		/**
		* close a given jabber client instance. Basically, there is no need to close a web service, 
		* unless one wants to reopen it with different credentials
		* @param sJID: such as "lixizhi@paraweb3d.com"
		*/
		virtual bool CloseJabberClient(const char* sJID)=0;

		/**
		* Asynchronously download a file from the url.
		* @param callbackScript: script code to be called, a global variable called msg is assigned, as below
		*  msg = {DownloadState=""|"complete"|"terminated", totalFileSize=number, currentFileSize=number, PercentDone=number}
		*/
		virtual void AsyncDownload(const char* url, const char* destFolder, const char* callbackScript, const char* DownloaderName)=0;


		/**
		* cancel all asynchronous downloads that matches a certain downloader name pattern
		* @param DownloaderName:regular expression. such as "proc1", "proc1.*", ".*"
		*/
		virtual void CancelDownload(const char* DownloaderName)=0;

		/**
		* Synchronous call of the function AsyncDownload(). This function will not return until download is complete or an error occurs. 
		* this function is rarely used. AsyncDownload() is used. 
		* @return:1 if succeed, 0 if fail
		*/
		virtual int Download(const char* url, const char* destFolder, const char* callbackScript, const char* DownloaderName)=0;

		/** this function is called once on each frame to polling all results and calling the callbacks in scripting interface 
		* @note: this function must be called from the main NPL runtime thread,since it will execute NPL code internally.
		* @return the number of asynchronous messages processed are returned.
		*/
		virtual int ProcessResults()=0;

		/** this function is called once on each frame to polling all results and calling the callbacks in scripting interface 
		* @note: this function must be called from the main NPL runtime thread,since it will execute NPL code internally.
		* @return the number of asynchronous download processed are returned.
		*/
		virtual int ProcessDownloaderResults() = 0;

		/** delete this object when it is no longer needed.*/
		virtual void DeleteThis()=0;

		//////////////////////////////////////////////////////////////////////////
		//
		// new libcUrl interface. 
		//
		//////////////////////////////////////////////////////////////////////////

		/** Append URL request to a pool. connections in the same pool are reused as much as possible. 
		* There is generally no limit to the number of requests sent. However, each pool has a specified maximum number of concurrent worker slots. 
		*  the default number is 1. One can change this number with ChangeRequestPoolSize. 
		* @param pUrlTask: must be new CURLRequestTask(), the ownership of the task is transfered to the manager. so the caller should never delete the pointer. 
		* @param sPoolName: the request pool name. If the pool does not exist, it will be created. If null, the default pool is used. 
		*/
		virtual bool AppendURLRequest(CURLRequestTask* pUrlTask, const char* sPoolName = NULL) = 0;

		/**
		* There is generally no limit to the number of requests sent. However, each pool has a specified maximum number of concurrent worker slots. 
		*  the default number is 1. One can change this number with this function. 
		*/
		virtual bool ChangeRequestPoolSize(const char* sPoolName, int nCount) = 0;

		/** this function is called once on each frame to polling all results and calling the callbacks in scripting interface 
		* @note: this function must be called from the main NPL runtime thread,since it will execute NPL code internally.
		* @return the number of asynchronous messages processed are returned.
		*/
		virtual int ProcessUrlRequests() = 0;
	};
}
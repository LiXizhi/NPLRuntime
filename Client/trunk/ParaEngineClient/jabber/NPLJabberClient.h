#pragma once
#include "IParaWebService.h"

#include "JC_Options.h"
#include "JabberEventBinding.h"
#include "ParaRingBuffer.h"

#include "util/mutex.h"
#include <boost/bimap.hpp>
#include <boost/thread.hpp>
#include <boost/asio.hpp>

// in gloox src folder.
#include <gloox.h>
#include <client.h>
#include <message.h>
#include <messagesessionhandler.h>
#include <messageeventhandler.h>
#include <messageeventfilter.h>
#include <chatstatehandler.h>
#include <chatstatefilter.h>
#include <connectionlistener.h>
#include <disco.h>
#include <lastactivity.h>
#include <loghandler.h>
#include <logsink.h>
#include <connectiontcpclient.h>
#include <connectionsocks5proxy.h>
#include <connectionhttpproxy.h>
#include <messagehandler.h>
#include <rostermanager.h>
#include <registration.h>

using namespace gloox;

namespace ParaEngine
{
	using namespace std;

	/** NPL jabber client
	*/
	class CNPLJabberClient : public MessageSessionHandler, ConnectionListener, LogHandler, RegistrationHandler,
		MessageEventHandler, MessageHandler, ChatStateHandler, RosterListener, INPLJabberClient/*, MUCRoomHandler*/
	{
	public:
		typedef boost::bimap<int, std::string>	StringMap_Type;
		
		/**
		* Create a new jabber client instance with the given jabber client ID. It does not open a connection immediately.
		* @param sJID: such as "lixizhi@paraengine.com"
		*/
		CNPLJabberClient(boost::asio::io_service& io_service, const char* sJID);
		virtual ~CNPLJabberClient(void);

	public:
		/** Use this periodically to receive data from the socket and to feed the parser. 
		* @param timeout:  The timeout in microseconds to use for select. Default of -1 means blocking until data was available.  
		* @return: whether there is a connection error.  0 if connection is not established. 
		*/
		int ProcessMessage(int timeout = 10);

		//
		// handlers 
		//
		virtual void handleLog( LogLevel level, LogArea area, const std::string& message );

		virtual void onConnect();
		virtual void onDisconnect( ConnectionError e );
		virtual bool onTLSConnect( const CertInfo& info );
		virtual void handleMessage( const gloox::Message & stanza, MessageSession * /*session*/ );
		virtual void handleMessageEvent( const JID& from, MessageEventType event );
		virtual void handleChatState( const JID& from, ChatStateType state );
		virtual void handleMessageSession( MessageSession *session );


		virtual void onResourceBindError( ResourceBindError error );
		virtual void onSessionCreateError( SessionCreateError error );
		virtual void handleItemSubscribed( const JID& jid );
		virtual void handleItemAdded( const JID& jid );
		virtual void handleItemUnsubscribed( const JID& jid );
		virtual void handleItemRemoved( const JID& jid );
		virtual void handleItemUpdated( const JID& jid );
		virtual void handleRoster( const Roster& roster );
		virtual void handleRosterError( const gloox::IQ & /*stanza*/ );
		virtual void handleRosterPresence( const RosterItem& item, const std::string& resource,
			gloox::Presence::PresenceType presence, const std::string& /*msg*/ );
		virtual void handleSelfPresence( const RosterItem& item, const std::string& resource,
			gloox::Presence::PresenceType presence, const std::string& /*msg*/ );
		virtual bool handleSubscriptionRequest( const JID& jid, const std::string& /*msg*/ );
		virtual bool handleUnsubscriptionRequest( const JID& jid, const std::string& /*msg*/ );
		virtual void handleNonrosterPresence( const gloox::Presence & );

		virtual void handleRegistrationFields( const JID& /*from*/, int fields, std::string instructions );
		virtual void handleRegistrationResult( const JID& /*from*/, RegistrationResult result );
		virtual void handleAlreadyRegistered(const JID& /*from*/ );
		virtual void handleDataForm( const JID& /*from*/, const DataForm& /*form*/ );
		virtual void handleOOB( const JID& /*from*/, const OOB& oob );
	public:
		//
		// exposed functions
		//

		/// <summary>
		/// Connect to the server.  This happens asynchronously, and
		/// could take a couple of seconds to get the full handshake
		/// completed.  This will auth, send presence, and request
		/// roster info, if the Auto* properties are set.
		/// </summary>
		bool Connect();

		/** Close down the connection, as gracefully as possible.*/
		void Close();

		/** add string a string mapping. We will automatically encode NPL filename string if it is in this string map. It means shorter message sent over the network. 
		* use AddStringMap whenever you want to add a string to the map. Please note, that the sender and the receiver must maintain the same string map in memory in order to have consistent string translation result.
		* the function is static, it will apply to all client instances. 
		* @param nID: the integer to encode the string. it is usually positive. 
		* @param sString: the string for the id. if input is NULL, it means removing the mapping of nID. 
		*/
		static void AddStringMap(int nID, const char* sString);

		/** clear all string mapping */
		static void ClearStringMap();

		/** Send a NPL message. The message is queued in a output message queue and this function returns immediately.  
		* @param msg: a NPL table converted to secure code.
		* @return true if message is sent to output queue. if the queue is full, the function returns false. 
		*	and the caller typically reports a service unavailable message.
		*/
		bool WriteNPLMessage(const char* to, const char* neuronfile,  const char* sCode);

		/** Send raw string. */
		bool WriteRawString(const char* rawstring);

		/**
		/// <summary>
		/// Send a message packet to another user
		/// </summary>
		/// <param name="t">type of MessageType</param>
		/// <param name="to">Who to send it to?</param>
		/// <param name="body">The message.</param>
		*/
		bool SendMessage(int t, const char* to, const char* body);

		/// <summary>
		/// Send a message packet to another user
		/// </summary>
		/// <param name="to">Who to send it to?</param>
		/// <param name="body">The message.</param>
		bool SendMessage(const char* to, const char* body);

		/** Initiate the auth process.*/
		bool Login();

		/**
		/// <summary>
		/// Send a presence packet to the server
		/// </summary>
		/// <param name="t">What kind? 
		public enum PresenceType
		{
		/// <summary>
		/// None specified
		/// </summary>
		available = -1,
		/// <summary>
		/// May I subscribe to you?
		/// </summary>
		subscribe,
		/// <summary>
		/// Yes, you may subscribe.
		/// </summary>
		subscribed,
		/// <summary>
		/// Unsubscribe from this entity.
		/// </summary>
		unsubscribe,
		/// <summary>
		/// No, you may not subscribe.
		/// </summary>
		unsubscribed,
		/// <summary>
		/// Offline
		/// </summary>
		unavailable,
		/// <summary>
		/// server-side only.
		/// </summary>
		probe,
		/// <summary>
		/// A presence error.
		/// </summary>
		error,
		/// <summary>
		/// Invisible presence: we're unavailable to them, but still see
		/// theirs.
		/// </summary>
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

		/** message type*/
		enum MessageType
		{
			/// <summary>
			/// Normal message
			/// </summary>
			MsgType_normal = -1,
			/// <summary>
			/// Error message
			/// </summary>
			MsgType_error,
			/// <summary>
			/// Chat (one-to-one) message
			/// </summary>
			MsgType_chat,
			/// <summary>
			/// Groupchat
			/// </summary>
			MsgType_groupchat,
			/// <summary>
			/// Headline
			/// </summary>
			MsgType_headline
		};

		/** Get a full roster 
		* @return: the returned string is NPL table of the following 
		{
		{jid=string:jid, name=string,subscription=int:SubscriptionEnum, groups={string, string, ...}, resources={string, string, ...},},
		{jid=string:jid, name=string,subscription=int:SubscriptionEnum, groups={string, string, ...}, resources={string, string, ...},},
		{jid=string:jid, name=string,subscription=int:SubscriptionEnum, groups={string, string, ...}, resources={string, string, ...},},
		...
		}
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


		/** to create an account:
		// - create the Client instance by passing it the server only, e.g. j = new Client( "example.net" );
		// - in onConnect() (or some similar place) call Registration::fetchRegistrationFields()
		// - in RegistrationHandler::handleRegistrationFields() set up an RegistrationFields struct
		//   with the desired username/password and call Registration::createAccount()
		*/
		virtual void RegisterCreateAccount(const char* username, const char* password);
		/** to change an account's password:
		// - connect to the server as usual
		// - in onConnect() or at any later time call Registration::changePassword()
		*/
		virtual void RegisterChangePassword(const char* password);
		/** to delete an account:
		// - connect to teh server as usual
		// - in onConnect() or at any later time call Registration::removeAccount()
		*/
		virtual void RegisterRemoveAccount();


		/**
		* Returns a StatisticsStruct containing byte and stanza counts for the current active connection. 
		* @return: a struct containing the current connection's statistics. 
		*/
		virtual StatisticsJabber GetStatistics();

		//////////////////////////////////////////////////////////////////////////
		// following are obsoleted methods
		//////////////////////////////////////////////////////////////////////////

		/** OBSOLETED */
		void RemoveRosterItem(const char* to, const char* msg);

		/**
		* this function is called in OnSubscription method to confirm or refuse a subscription request from another user. 
		* @param to: the JID
		* @param bAllow: true to allow subscription, false to deny
		*/
		bool AllowSubscription(const char* to, bool bAllow);


		/**
		* OBSOLETED: Request a list of agents from the server
		*/
		void GetAgents();

		/// <summary>
		/// obsoleted: Attempt to register a new user.  This will fire OnRegisterInfo to retrieve
		/// information about the new user, and OnRegistered when the registration is complete or failed.
		/// </summary>
		/// <param name="jid">The user to register</param>
		bool Register(const char* jid);


		/// <summary>
		/// OBSOLETED: Retrieve details of a given roster item.
		/// </summary>
		/// <param name="jid"></param>
		/// <returns>
		/// null or string. the string is an NPL table of the following format
		/// "{ [1]={text=[[NickName]], presence=number,groupname=[[name]]}, [2]=...}"
		///  Presence Values: OFFLINE = 0;ONLINE = 1; AWAY = 2;XA = 3;DND = 4;CHATTY = 5;
		/// Usually a roster item only belongs to one group. so the table usually contains only one item at index 1.
		/// </returns>
		const char* GetRosterItemDetail(const char* jid);

		/** OBSOLETED
		* The group names for the roster as a string. each group name is separated by ;
		* such as "general;friends;relatives;"
		*/
		const char* GetRosterGroups();

		/** OBSOLETED
		* The Roster Item jids as a string. each name is separated by ;
		* such as "lixizhi@pala5.cn;Andy@pala5.cn;"
		*/
		const char* GetRosterItems();


		//////////////////////////////////////////////////////////////////////////
		//
		// event functions
		//
		//////////////////////////////////////////////////////////////////////////
		/** 
		* @note although the interface supports multiple listeners for the same event, the current implementation supports only one 
		* which makes the RemoveEventListener and ClearEventListener virtually the same. 
		* @param nListenerType: enum from JabberEvents
		* @param callbackScript: an NPL callback script.
		*/ 
		void AddEventListener(int nListenerType, const char*  callbackScript);
		void AddEventListener1(const char* sListenerType, const char*  callbackScript);
		void RemoveEventListener(int nListenerType, const char*  callbackScript);
		void RemoveEventListener1(const char* sListenerType, const char*  callbackScript);
		void ClearEventListener(int nListenerType);
		void ClearEventListener1(const char* sListenerType);
		void ResetAllEventListeners();

		//////////////////////////////////////////////////////////////////////////
		//
		// misc 
		//
		//////////////////////////////////////////////////////////////////////////

		/**
		* Creates a string which is unique in the current instance and can be used as an ID for queries. 
		* @return: A unique string suitable for query IDs. 
		*/
		const std::string GetID();

		/** Returns the current status message */
		const string& GetStatus() const;

		/** This describes the possible states of a stream. 
		*@return : 
		* 0: StateDisconnected  The client is in disconnected state.  
		* 1: StateConnecting  The client is currently trying to establish a connection.  
		* 2: StateConnected  The client is connected to the server but authentication is not (yet) done. 
		*/
		int GetConnectionStatus() const;

		/** Use this function to retrieve the type of the authentication error after it occurs and you received a ConnectionError of type ConnAuthenticationFailed from the ConnectionListener
		* @return: 		Enumeration values: 
		* 0: AuthErrorUndefined  No error occurred, or error condition is unknown.  
		* 1: SaslAborted  The receiving entity acknowledges an <abort/> element sent by the initiating entity; sent in reply to the <abort/> element.  
		* 2: SaslIncorrectEncoding  The data provided by the initiating entity could not be processed because the [BASE64] encoding is incorrect (e.g., because the encoding does not adhere to the definition in Section 3 of [BASE64]); sent in reply to a <response/> element or an <auth/> element with initial response data.  
		* 3: SaslInvalidAuthzid  The authzid provided by the initiating entity is invalid, either because it is incorrectly formatted or because the initiating entity does not have permissions to authorize that ID; sent in reply to a <response/> element or an <auth/> element with initial response data.  
		* 4: SaslInvalidMechanism  The initiating entity did not provide a mechanism or requested a mechanism that is not supported by the receiving entity; sent in reply to an <auth/> element.  
		* 5: SaslMechanismTooWeak  The mechanism requested by the initiating entity is weaker than server policy permits for that initiating entity; sent in reply to a <response/> element or an <auth/> element with initial response data.  
		* 6: SaslNotAuthorized  The authentication failed because the initiating entity did not provide valid credentials (this includes but is not limited to the case of an unknown username); sent in reply to a <response/> element or an <auth/> element with initial response data.  
		* 7: SaslTemporaryAuthFailure  The authentication failed because of a temporary error condition within the receiving entity; sent in reply to an <auth/> element or <response/> element.  
		* 8: NonSaslConflict  XEP-0078: Resource Conflict  
		* 9: NonSaslNotAcceptable  XEP-0078: Required Information Not Provided  
		* 10: NonSaslNotAuthorized  XEP-0078: Incorrect Credentials  
		*/
		int GetAuthError()  const;


		/** Sends a whitespace ping to the server.  */
		void  WhitespacePing (); 

		/** Sends a XMPP Ping (XEP-0199) to the given JID. There is currently no way to know whether the remote entity answered (other than registering an IQ handler for the urn:xmpp:ping namespace). */
		void  XmppPing (const string & sTo);

		/** Retrieves the value of the xml:lang attribute of the initial stream. Default is 'en', i.e. if not changed by a call to setXmlLang(). */
		const std::string &  GetXmlLang () const ;
		void  SetXmlLang (const std::string &xmllang);


		/**
		* Use this function to set the user's certificate and private key. The certificate will
		* be presented to the server upon request and can be used for SASL EXTERNAL authentication.
		* The user's certificate file should be a bundle of more than one certificate in PEM format.
		* The first one in the file should be the user's certificate, each cert following that one
		* should have signed the previous one.
		* @note These certificates are not necessarily the same as those used to verify the server's
		* certificate.
		* @param clientKey The absolute path to the user's private key in PEM format.
		* @param clientCerts A path to a certificate bundle in PEM format.
		*/
		void  SetClientCert (const std::string &clientKey, const std::string &clientCerts);

	public:
		//
		// properties
		//

		/** The username to connect as.*/
		const char* GetUser();
		void SetUser(const char* sValue);

		/** Priority for this connection.*/
		int GetPriority();
		void SetPriority(int nValue){m_Options.PRIORITY = nValue;}

		/** The password to use for connecting.
		This may be sent across the wire plaintext, if the
		server doesn't support digest and PlaintextAuth is true.*/
		const char* GetPassword();
		void SetPassword(const char* sValue);


		/** Automatically log in on connection.*/
		bool GetAutoLogin() {return m_Options.AUTO_LOGIN;}
		void SetAutoLogin(bool nValue){m_Options.AUTO_LOGIN = nValue;}


		/** Automatically retrieve roster on connection.*/
		bool GetAutoRoster(){return m_Options.AUTO_ROSTER;}
		void SetAutoRoster(bool nValue){m_Options.AUTO_ROSTER = nValue;}

		/** Automatically send back 501/feature-not-implemented to IQs that have not been handled.*/
		bool GetAutoIQErrors() {return m_Options.AUTO_IQ_ERRORS;}
		void SetAutoIQErrors(bool nValue){m_Options.AUTO_IQ_ERRORS = nValue;}

		/** Automatically send presence on connection.*/
		bool GetAutoPresence(){return m_Options.AUTO_PRESENCE;}
		void SetAutoPresence(bool nValue){m_Options.AUTO_PRESENCE = nValue;}

		/** The connecting resource. Used to identify a unique connection.*/
		const char* GetResource();
		void SetResource(const char* sValue){m_Options.RESOURCE = sValue;}

		/**Are we currently connected?*/
		bool GetIsAuthenticated();
		void SetIsAuthenticated(bool bValue);

		/** The name of the server to connect to.*/
		const char* GetServer();
		void SetServer(const char* Server);

		/** The address to use on the "to" attribute of the stream:stream.
		You can put the network hostname or IP address of the server to connect to.
		If none is specified, the Server will be used.
		Eventually, when SRV is supported, this will be deprecated.*/
		const char* GetNetworkHost(){return m_Options.NETWORK_HOST.c_str();}
		void SetNetworkHost(const char* sValue){m_Options.NETWORK_HOST = sValue;}

		/** Automatically send presence on connection.*/
		int GetPort();
		void SetPort(int nValue);

		/** Allow plaintext authentication? default to false*/
		bool GetPlaintextAuth(){return m_Options.PLAINTEXT;}
		void SetPlaintextAuth(bool bValue){m_Options.PLAINTEXT = bValue;}

		/** Is the current connection SSL/TLS protected? */
		bool IsSSLon(){return m_Options.SSL;}

		/** Is the current connection XEP-138 compressed? */
		bool IsCompressionOn(){return m_Options.AUTO_COMPRESS;}

		/**Do SSL3/TLS1 on startup*/
		bool GetSSL(){return m_Options.SSL;}
		void SetSSL(bool bValue){m_Options.SSL = bValue;}

		/** Allow Start-TLS on connection, if the server supports it*/
		bool GetAutoStartTLS(){return m_Options.AUTO_TLS;}
		void SetAutoStartTLS(bool bValue);

		/** Allow start compression on connection, if the server supports it*/
		bool GetAutoStartCompression();
		void SetAutoStartCompression(bool bValue);

		/** Time, in seconds, between keep-alive spaces. default to 20f*/
		float GetKeepAlive(){return m_fKeepAlive;	}
		void SetKeepAlive(float KeepAlive){m_fKeepAlive = KeepAlive;}

		/** Seconds before automatically reconnecting if the connection drops.  -1 to disable, 0 for immediate. Default to 30 seconds */
		float GetAutoReconnect(){return m_fAutoReconnect;}
		void SetAutoReconnect(float AutoReconnect){m_fAutoReconnect = AutoReconnect;}

		/** The version string returned in the server's open stream element. */
		const char* GetServerVersion() { return GLOOX_VERSION.c_str();}

		/** Is SASL required?  This will default to true in the future.*/
		bool GetRequiresSASL(){return m_Options.SASL;}
		void SetRequiresSASL(bool bValue);

	protected:
		/**
		* Activate the given code of a type of event
		* The activated script will send to the script buffer and wait for the script engine to execute it.
		* The function will return immediately. So the script execution is asynchronous.
		* @param nEventType: event type
		* @param precode: the script to be activated before the user-defined script code. 
		* @param postcode: the script to be activated after the user-defined script code.
		* @return: true if there is a callback
		*/ 
		virtual bool AddEvent(int nEventType, const char* precode=NULL, const char* postcode=NULL);
		/**
		* call the given code of a type of event
		* It is similar to AddEvent, except that the callback functions are activated immediately and return after the callback returns. 
		* The function will return immediately. So the script execution is asynchronous.
		* @param nEventType: event type
		* @param precode: the script to be activated before the user-defined script code. 
		* @param postcode: the script to be activated after the user-defined script code.
		* @return: true if there is a callback
		*/ 
		virtual bool CallEvent(int nEventType, const char* precode=NULL, const char* postcode=NULL);

		/** get a string by id in the string map 
		* @return NULL if not found
		*/
		const char* GetStringByID(int nID);
		/** get id by string in the string map 
		* @return -1 if not found
		*/
		int GetIDByString(const char* sString);

		/** this is the unique key of this JC instance. This is always the same as the bare JID of this instance.
		* this key is set automatically, when an JC instance is created. 
		*/
		const string& GetJIDKey() {return m_sThisJIDKey;}
		/** never call this function, unless you know why. */
		void SetJIDKey(const string& sJIDKey) {m_sThisJIDKey = sJIDKey;}

	protected:
		Client *m_client;
		MessageSession *m_session;
		MessageEventFilter *m_messageEventFilter;
		ChatStateFilter *m_chatStateFilter;
		Registration *m_reg;

		JCOptions m_Options;
		bool m_bIsAuthenticated;
		float m_fKeepAlive;
		float m_fAutoReconnect;
		CJabberEventBinding m_eventbinding;

		/** only used for creating a new user account. */
		string m_sUsername;
		string m_sPassword;
		/** presence status*/
		string m_sStatus;
		/** this is the unique key of this JC instance. This is always the same as the bare JID of this instance.
		* this key is set automatically, when an JC instance is created. 
		*/
		string m_sThisJIDKey;
		/** we will automatically encode NPL filename string if it is in this string map. It means shorter message sent over the network. 
		* use AddStringMap whenever you want to add a string to the map. Please note, that the sender and the receiver must maintain the same string map in memory in order to have consistent string translation result.
		*/
		static StringMap_Type g_mapStringMap;

		/** dispatcher IO service object for the connection socket. */
		boost::asio::io_service * m_pIOService;

		ParaEngine::mutex m_mutex;
	};

}

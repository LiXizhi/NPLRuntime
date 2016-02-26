//-----------------------------------------------------------------------------
// Class:	NPL Jabber Client Exposed to Scripting Interface
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Date:	2007.9.18
// Note: code is cross platformed
//-----------------------------------------------------------------------------
#ifdef USE_JABBERCLIENT
#include "ParaEngine.h"

extern "C"
{
#include <lua.h>
}
#include <luabind/luabind.hpp>
#include <luabind/object.hpp>
using namespace luabind;

#include "NPLRuntime.h"
#include "NPLHelper.h"
#include "IParaWebService.h"
#include "jabber/NPLJabberClient.h"
#include "ParaScriptingJabberClient.h"

#ifdef PARAENGINE_CLIENT
#include "memdebug.h"
#endif




namespace ParaScripting
{
	NPLJabberClient JabberClientManager::GetJabberClient( const char* sJID )
	{
		return NPLJabberClient(NPL::CNPLRuntime::GetInstance()->GetJabberClient(sJID));

	}

	NPLJabberClient JabberClientManager::CreateJabberClient( const char* sJID )
	{
		return NPLJabberClient(NPL::CNPLRuntime::GetInstance()->CreateJabberClient(sJID));

	}

	bool JabberClientManager::CloseJabberClient( const char* sJID )
	{
		return NPL::CNPLRuntime::GetInstance()->CloseJabberClient(sJID);
	}

	void JabberClientManager::AddStringMap( int nID, const char* sString )
	{
		ParaEngine::CNPLJabberClient::AddStringMap(nID, sString);
	}

	void JabberClientManager::ClearStringMap()
	{
		ParaEngine::CNPLJabberClient::ClearStringMap();
	}
	
	//////////////////////////////////////////////////////////////////////////
	//
	// jabber client
	//
	//////////////////////////////////////////////////////////////////////////

	/** The username to connect as.*/
	void NPLJabberClient::SetUser(const char* User)
	{
		if(m_pProxy!=0) m_pProxy->SetUser(User);
	}

	/** The username to connect as.*/
	const char* NPLJabberClient::GetUser() const
	{
		return (m_pProxy!=0) ? m_pProxy->GetUser() : NULL;
	}

	/** Priority for this connection.*/
	void NPLJabberClient::SetPriority(int nPriority)
	{
		if(m_pProxy!=0) m_pProxy->SetPriority(nPriority);
	}

	/** Priority for this connection.*/
	int NPLJabberClient::GetPriority() const
	{
		return (m_pProxy!=0) ? m_pProxy->GetPriority() : 0;
	}

	/** The password to use for connecting.
	This may be sent across the wire plaintext, if the
	server doesn't support digest and PlaintextAuth is true.*/
	void NPLJabberClient::SetPassword(const char* Password)
	{
		if(m_pProxy!=0) m_pProxy->SetPassword(Password);
	}

	/** The password to use for connecting.
	This may be sent across the wire plaintext, if the
	server doesn't support digest and PlaintextAuth is true.*/
	const char* NPLJabberClient::GetPassword() const
	{
		return (m_pProxy!=0) ? m_pProxy->GetPassword() : NULL;
	}

	/** Automatically log in on connection.*/
	void NPLJabberClient::SetAutoLogin(bool AutoLogin)
	{
		if(m_pProxy!=0) m_pProxy->SetAutoLogin(AutoLogin);
	}

	/** Automatically log in on connection.*/
	bool NPLJabberClient::GetAutoLogin() const
	{
		return (m_pProxy!=0) ? m_pProxy->GetAutoLogin() : false;
	}

	/** Automatically retrieve roster on connection.*/
	void NPLJabberClient::SetAutoRoster(bool AutoRoster)
	{
		if(m_pProxy!=0) m_pProxy->SetAutoRoster(AutoRoster);
	}

	/** Automatically retrieve roster on connection.*/
	bool NPLJabberClient::GetAutoRoster() const
	{
		return (m_pProxy!=0) ? m_pProxy->GetAutoRoster() : false;
	}

	/** Automatically send back 501/feature-not-implemented to IQs that have not been handled.*/
	void NPLJabberClient::SetAutoIQErrors(bool AutoIQErrors)
	{
		if(m_pProxy!=0) m_pProxy->SetAutoIQErrors(AutoIQErrors);
	}

	/** Automatically send back 501/feature-not-implemented to IQs that have not been handled.*/
	bool NPLJabberClient::GetAutoIQErrors() const
	{
		return (m_pProxy!=0) ? m_pProxy->GetAutoIQErrors() : false;
	}

	/** Automatically send presence on connection.*/
	void NPLJabberClient::SetAutoPresence(bool AutoPresence)
	{
		if(m_pProxy!=0) m_pProxy->SetAutoPresence(AutoPresence);
	}

	/** Automatically send presence on connection.*/
	bool NPLJabberClient::GetAutoPresence() const
	{
		return (m_pProxy!=0) ? m_pProxy->GetAutoPresence() : false;
	}


	/** The connecting resource. Used to identify a unique connection.*/
	void NPLJabberClient::SetResource(const char* Resource)
	{
		if(m_pProxy!=0) m_pProxy->SetResource(Resource);
	}

	/** The connecting resource. Used to identify a unique connection.*/
	const char* NPLJabberClient::GetResource() const
	{
		return (m_pProxy!=0) ? m_pProxy->GetResource() : NULL;
	}

	/**Are we currently connected?*/
	void NPLJabberClient::SetIsAuthenticated(bool IsAuthenticated)
	{
		if(m_pProxy!=0) m_pProxy->SetIsAuthenticated(IsAuthenticated);
	}

	/** Are we currently connected? */
	bool NPLJabberClient::GetIsAuthenticated() const
	{
		return (m_pProxy!=0) ? m_pProxy->GetIsAuthenticated() : false;
	}

	/** The name of the server to connect to.*/
	void NPLJabberClient::SetServer(const char* Server)
	{
		if(m_pProxy!=0) m_pProxy->SetServer(Server);
	}

	/** The name of the server to connect to. */
	const char* NPLJabberClient::GetServer() const
	{	
		return (m_pProxy!=0) ? m_pProxy->GetServer() : NULL;

	}

	/** The address to use on the "to" attribute of the stream:stream.
	/// You can put the network hostname or IP address of the server to connect to.
	/// If none is specified, the Server will be used.
	/// Eventually, when SRV is supported, this will be deprecated.*/
	void NPLJabberClient::SetNetworkHost(const char* NetworkHost)
	{
		if(m_pProxy!=0) m_pProxy->SetNetworkHost(NetworkHost);
	}

	/** The address to use on the "to" attribute of the stream:stream.
	/// You can put the network hostname or IP address of the server to connect to.
	/// If none is specified, the Server will be used.
	/// Eventually, when SRV is supported, this will be deprecated.*/
	const char* NPLJabberClient::GetNetworkHost() const
	{
		return (m_pProxy!=0) ? m_pProxy->GetNetworkHost() : NULL;
	}

	/** Automatically send presence on connection.*/
	void NPLJabberClient::SetPort(int Port)
	{
		if(m_pProxy!=0) m_pProxy->SetPort(Port);
	}

	/** The TCP port to connect to. default to 5222*/
	int NPLJabberClient::GetPort() const
	{
		return (m_pProxy!=0) ? m_pProxy->GetPort() : 5222;
	}


	/** Allow plaintext authentication? default to false*/
	void NPLJabberClient::SetPlaintextAuth(bool PlaintextAuth)
	{
		if(m_pProxy!=0) m_pProxy->SetPlaintextAuth(PlaintextAuth);
	}

	/** Allow plaintext authentication? default to false*/
	bool NPLJabberClient::GetPlaintextAuth() const
	{
		return (m_pProxy!=0) ? m_pProxy->GetPlaintextAuth() : false;
	}

	/** Is the current connection SSL/TLS protected? */
	bool NPLJabberClient::IsSSLon() const
	{
		return (m_pProxy!=0) ? m_pProxy->IsSSLon() : false;
	}

	/** Is the current connection XEP-138 compressed? */
	bool NPLJabberClient::IsCompressionOn() const
	{
		return (m_pProxy!=0) ? m_pProxy->IsCompressionOn() : false;
	}

	/**Do SSL3/TLS1 on startup*/
	void NPLJabberClient::SetSSL(bool SSL)
	{
		if(m_pProxy!=0) m_pProxy->SetSSL(SSL);
	}

	/** Do SSL3/TLS1 on startup*/
	bool NPLJabberClient::GetSSL() const
	{
		return (m_pProxy!=0) ? m_pProxy->GetSSL() : false;
	}

	/** Allow Start-TLS on connection, if the server supports it*/
	void NPLJabberClient::SetAutoStartTLS(bool AutoStartTLS)
	{
		if(m_pProxy!=0) m_pProxy->SetAutoStartTLS(AutoStartTLS);
	}

	/** Allow Start-TLS on connection, if the server supports it*/
	bool NPLJabberClient::GetAutoStartTLS() const
	{
		return (m_pProxy!=0) ? m_pProxy->GetAutoStartTLS() : false;
	}

	/** Allow start compression on connection, if the server supports it*/
	void NPLJabberClient::SetAutoStartCompression(bool AutoStartCompression)
	{
		if(m_pProxy!=0) m_pProxy->SetAutoStartCompression(AutoStartCompression);
	}

	/** Allow start compression on connection, if the server supports it*/
	bool NPLJabberClient::GetAutoStartCompression() const
	{	
		return (m_pProxy!=0) ? m_pProxy->GetAutoStartCompression() : false;
	}

	/** Time, in seconds, between keep-alive spaces. default to 20f*/
	void NPLJabberClient::SetKeepAlive(float KeepAlive)
	{
		if(m_pProxy!=0) m_pProxy->SetKeepAlive(KeepAlive);
	}

	/** Time, in seconds, between keep-alive spaces. default to 20f*/
	float NPLJabberClient::GetKeepAlive() const
	{
		return (m_pProxy!=0) ? m_pProxy->GetKeepAlive() : 20.f;
	}

	/** Seconds before automatically reconnecting if the connection drops.  -1 to disable, 0 for immediate. Default to 30 seconds */
	void NPLJabberClient::SetAutoReconnect(float AutoReconnect)
	{
		if(m_pProxy!=0) m_pProxy->SetAutoReconnect(AutoReconnect);
	}

	/** Seconds before automatically reconnecting if the connection drops.  -1 to disable, 0 for immediate. Default to 30 seconds */
	float NPLJabberClient::GetAutoReconnect() const
	{
		return (m_pProxy!=0) ? m_pProxy->GetAutoReconnect() : 30.f;
	}


	/** The version string returned in the server's open stream element. */
	const char* NPLJabberClient::GetServerVersion() const
	{
		return (m_pProxy!=0) ? m_pProxy->GetServerVersion() : NULL;
	}

	/** Is SASL required?  This will default to true in the future.*/
	void NPLJabberClient::SetRequiresSASL(bool RequiresSASL)
	{
		if(m_pProxy!=0) m_pProxy->SetRequiresSASL(RequiresSASL);
	}

	/** Is SASL required?  This will default to true in the future. */
	bool NPLJabberClient::GetRequiresSASL() const
	{
		return (m_pProxy!=0) ? m_pProxy->GetRequiresSASL() : false;
	}
	/** Connect to the server.  This happens asynchronously, and
	could take a couple of seconds to get the full handshake
	completed.  This will auth, send presence, and request
	roster info, if the Auto* properties are set.
	*/
	bool NPLJabberClient::Connect()
	{
		return (m_pProxy!=0) ? m_pProxy->Connect() : false;
	}

	/** Close down the connection, as gracefully as possible.*/
	void NPLJabberClient::Close()
	{
		if(m_pProxy!=0) m_pProxy->Close();
	}

	bool NPLJabberClient::activate(const char * sDestination, const object& input)
	{
		// when sending a message, no serious validation(security) is performed. It is generally considered the receiver's job.
		ParaEngine::StringBuilder sCode;
		int nType = type(input);

		if(nType == LUA_TSTRING)
		{
			// arbitrary code is allowed in sCode
			sCode = object_cast<const char*>(input);
			// check if data is secure
			if(!NPL::NPLHelper::IsPureData(sCode.c_str(), (int)(sCode.size())))
			{
				OUTPUT_LOG("warning: unsecure scode detected in JC:activate()\n");
				return false;
			}
		}
		else
		{
			// it is the receiver's responsibility to validate the scode according to its source.
			NPL::NPLHelper::SerializeToSCode(NULL, input, sCode);
		}
		sCode.append('\0');

		NPL::NPLFileName FullName(sDestination);
		if(!FullName.sNID.empty())
		{
			if(FullName.sRelativePath.empty())
			{
				// if no neuron file is specified. the sCode will be regarded as an ordinary Jabber:XMPP:Chat message body. 
				return Message2(FullName.sNID.c_str(), sCode.c_str());
			}
			else
			{
				// an NPL activation message will be sent
				if(m_pProxy!=0)
					return m_pProxy->WriteNPLMessage(FullName.sNID.c_str(), FullName.sRelativePath.c_str(), sCode.c_str());
			}
		}
		return false;
	}

	/** Send a NPL message. 
	* @param msg: a NPL table converted to secure code.
	*/
	bool NPLJabberClient::WriteNPLMessage(const char* to, const char* neuronfile,  const char* sCode)
	{
		return (m_pProxy!=0) ? m_pProxy->WriteNPLMessage(to, neuronfile, sCode) : false;
	}

	/** Send raw string. */
	bool NPLJabberClient::WriteRawString(const char* rawstring)
	{
		return (m_pProxy!=0) ? m_pProxy->WriteRawString(rawstring) : false;
	}

	/** Initiate the auth process.*/
	bool NPLJabberClient::Login()
	{
		return (m_pProxy!=0) ? m_pProxy->Login() : false;
	}

	bool NPLJabberClient::SetPresence(int t,const char* status,const char* show,int priority)
	{
		return (m_pProxy!=0) ? m_pProxy->SetPresence(t,status, show, priority) : false;
	}

	
	bool NPLJabberClient::Message(int t, const char* to, const char* body)
	{
		return (m_pProxy!=0) ? m_pProxy->SendMessage(t,to,body) : false;
	}

	/// Send a message packet to another user
	/// <param name="to">Who to send it to?</param>
	/// <param name="body">The message.</param>
	bool NPLJabberClient::Message2(const char* to, const char* body)
	{
		return (m_pProxy!=0) ? m_pProxy->SendMessage(to, body) : false;
	}

	/** Request a new copy of the roster.*/
	const char* NPLJabberClient::GetRoster()
	{
		if(m_pProxy!=0) 
			return m_pProxy->GetRoster();
		else
			return NULL;
	}

	/**
	* in Jabber, a contact list is called a roster
	/// Send a presence subscription request and update the roster for a new roster contact.
	/// <param name="to">The JID of the contact (required)</param>
	/// <param name="nickname">The nickname to show for the user.</param>
	/// <param name="groups">A list of groups to put the contact in. Groups are separated by ';'. May be null.  Hint: "foo;bar;" or "foo"</param>
	*/
	bool NPLJabberClient::Subscribe(const char* to, const char* nickname, const char* groups, const char* msg)
	{
		return (m_pProxy!=0) ? m_pProxy->Subscribe(to, nickname, groups, msg) : false;
	}

	/**
	* in Jabber, a contact list is called a roster
	/// Remove an item from the roster.  
	/// This will have the side-effect of bi-directionally unsubscribing to/from the user.
	/// <param name="to">The JID to remove</param>
	*/
	void NPLJabberClient::Unsubscribe( const char* to, const char* msg )
	{
		if(m_pProxy!=0) m_pProxy->Unsubscribe(to, msg);
	}

	/**
	* in Jabber, a contact list is called a roster
	/// Remove an item from the roster.  
	/// This will have the side-effect of bi-directionally unsubscribing to/from the user.
	/// <param name="to">The JID to remove</param>
	*/
	void NPLJabberClient::RemoveRosterItem(const char* to, const char* msg)
	{
		if(m_pProxy!=0) m_pProxy->RemoveRosterItem(to, msg);
	}

	/**
	* Request a list of agents from the server
	*/
	void NPLJabberClient::GetAgents()
	{
		if(m_pProxy!=0) m_pProxy->GetAgents();
	}


	/// Attempt to register a new user.  This will fire OnRegisterInfo to retrieve
	/// information about the new user, and OnRegistered when the registration is complete or failed.
	/// <param name="jid">The user to register</param>
	bool NPLJabberClient::Register(const char* jid)
	{
		return (m_pProxy!=0) ? m_pProxy->Register(jid) : false;
	}


	/// Retrieve details of a given roster item.
	/// <param name="jid"></param>
	/// <returns>
	/// null or string. the string is an NPL table of the following format
	/// "{ [1]={text=[[NickName(status)]], presence=number,groupname=[[name]]}, [2]=...}"
	/// Usually a roster item only belongs to one group. so the table usually contains only one item at index 1.
	/// </returns>
	const char* NPLJabberClient::GetRosterItemDetail(const char* jid)
	{
		return (m_pProxy!=0) ? m_pProxy->GetRosterItemDetail(jid) : NULL;
	}

	/**
	* The group names for the roster as a string. each group name is separated by ;
	* such as "general;friends;relatives;"
	*/
	const char* NPLJabberClient::GetRosterGroups()
	{
		return (m_pProxy!=0) ? m_pProxy->GetRosterGroups() : NULL;
	}

	/**
	* The Roster Item names as a string. each name is separated by ;
	* such as "Xizhi;Andy;Clayman;"
	*/
	const char* NPLJabberClient::GetRosterItems()
	{
		return (m_pProxy!=0) ? m_pProxy->GetRosterItems() : NULL;
	}

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
	void NPLJabberClient::AddEventListener(int nListenerType, const char*  callbackScript)
	{
		if(m_pProxy!=0) m_pProxy->AddEventListener(nListenerType, callbackScript);
	}

	/// <summary>
	/// remove a NPL call back script from a given even listener
	/// </summary>
	/// <param name="nListenerType">type of message listener. The user can register NPL callback script with any one listener type. </param>
	/// <param name="callbackScript">the script to be called when the listener event is raised. Usually parameters are stored in a NPL parameter called "msg".</param>
	void NPLJabberClient::RemoveEventListener(int nListenerType, const char*  callbackScript)
	{
		if(m_pProxy!=0) m_pProxy->RemoveEventListener(nListenerType, callbackScript);
	}

	/// <summary>
	/// clear all NPL call back script from a given even listener
	/// </summary>
	/// <param name="nListenerType">type of message listener. The user can register NPL callback script with any one listener type. </param>
	void NPLJabberClient::ClearEventListener(int nListenerType)
	{
		if(m_pProxy!=0) m_pProxy->ClearEventListener(nListenerType);
	}

	/// <summary>
	/// clear all registered event listeners
	/// </summary>
	void NPLJabberClient::ResetAllEventListeners()
	{
		if(m_pProxy!=0) m_pProxy->ResetAllEventListeners();
	}

	bool NPLJabberClient::AllowSubscription( const char* to, bool bAllow )
	{
		if(m_pProxy!=0) 
			return m_pProxy->AllowSubscription(to, bAllow);
		return false;
	}

	void NPLJabberClient::AddEventListener1( const char* sListenerType, const char* callbackScript )
	{
		if(m_pProxy!=0) 
			m_pProxy->AddEventListener1(sListenerType, callbackScript);
	}

	void NPLJabberClient::RemoveEventListener1( const char* sListenerType, const char* callbackScript )
	{
		if(m_pProxy!=0) 
			m_pProxy->RemoveEventListener1(sListenerType, callbackScript);
	}

	void NPLJabberClient::ClearEventListener1( const char* sListenerType )
	{
		if(m_pProxy!=0) 
			m_pProxy->ClearEventListener1(sListenerType);
	}

	void NPLJabberClient::RegisterCreateAccount( const char* username, const char* password )
	{
		if(m_pProxy!=0) 
			m_pProxy->RegisterCreateAccount(username, password);
	}

	void NPLJabberClient::RegisterChangePassword( const char* password )
	{
		if(m_pProxy!=0) 
			m_pProxy->RegisterChangePassword(password);
	}

	void NPLJabberClient::RegisterRemoveAccount()
	{
		if(m_pProxy!=0) 
			m_pProxy->RegisterRemoveAccount();
	}

	luabind::object NPLJabberClient::GetStatistics(const object& data )
	{
		if(type(data) == LUA_TTABLE && m_pProxy!=0)
		{
			using namespace ParaEngine;
			StatisticsJabber stats = m_pProxy->GetStatistics();
			data["totalBytesSent"] = stats.totalBytesSent;
			data["totalBytesReceived"] = stats.totalBytesReceived;

			data["compressedBytesSent"] = stats.compressedBytesSent;
			data["compressedBytesReceived"] = stats.compressedBytesReceived;
			data["uncompressedBytesSent"] = stats.uncompressedBytesSent;
			data["uncompressedBytesReceived"] = stats.uncompressedBytesReceived;
			data["totalStanzasSent"] = stats.totalStanzasSent;
			data["totalStanzasReceived"] = stats.totalStanzasReceived;
			data["iqStanzasSent"] = stats.iqStanzasSent;
			data["iqStanzasReceived"] = stats.iqStanzasReceived;
			data["messageStanzasSent"] = stats.messageStanzasSent;
			data["messageStanzasReceived"] = stats.messageStanzasReceived;
			data["s10nStanzasSent"] = stats.s10nStanzasSent;
			data["s10nStanzasReceived"] = stats.s10nStanzasReceived;
			data["presenceStanzasSent"] = stats.presenceStanzasSent;
			data["presenceStanzasReceived"] = stats.presenceStanzasReceived;
			data["encryption"] = stats.encryption;
			data["compression"] = stats.compression;
		}
		return object(data);
	}

}
#endif
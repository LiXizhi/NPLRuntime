//-----------------------------------------------------------------------------
// Class:	CNPLJabberClient
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2008.11.28
//-----------------------------------------------------------------------------
#include "ParaEngine.h"

#include "AISimulator.h"
#include "NPLHelper.h"
#include "NPLJabberClient.h"
#include "NPL_TCPConnection.h"
#include "util/StringHelper.h"

#ifdef PARAENGINE_CLIENT
#include "memdebug.h"
#endif

/** Whether we want to use a thread for the connection and send/receive from socket. */ 
#define NPL_JABBER_USE_THREADING

/**  Whether we will need to use message session. */
// #define HANDLE_MESSAGE_SESSION

using namespace ParaEngine;

ParaEngine::CNPLJabberClient::StringMap_Type ParaEngine::CNPLJabberClient::g_mapStringMap;

CNPLJabberClient::CNPLJabberClient(boost::asio::io_service& io_service, const char* sJID)
:m_pIOService(&io_service), m_client(NULL), m_bIsAuthenticated(false), m_session(NULL), m_messageEventFilter(NULL), m_chatStateFilter(NULL), m_reg(NULL), 
	m_fKeepAlive(0.f), m_fAutoReconnect(20.f)
{
	if(sJID ==NULL)
		return;
	try
	{
		string strJID = sJID;
		SetJIDKey(strJID);
		if( strJID.find_first_of("@") == string::npos)
		{
			// for registration only
			m_client = new Client(sJID);
			
			m_reg = new Registration( m_client );
			m_reg->registerRegistrationHandler( this );
		}
		else
		{
			string jidStr = sJID;
			size_t slash = jidStr.find( "/", 0 );
			// use default resource "/pe",  if none is found in jid
			if(slash == std::string::npos )
			{
				jidStr += "/pe";
			}
			JID jid(jidStr.c_str());
			m_client = new Client(jid, "");
			
			m_client->registerConnectionListener( this );
#ifdef HANDLE_MESSAGE_SESSION
			m_client->registerMessageSessionHandler( this, 0 );
#else
			m_client->registerMessageHandler( this);
#endif
			m_client->rosterManager()->registerRosterListener( this );
			m_client->disco()->setVersion( "npl", GLOOX_VERSION);
			m_client->disco()->setIdentity( "client", "bot" );

			m_client->disco()->addFeature( XMLNS_CHAT_STATES );	
		}

#ifdef _DEBUG
		m_client->logInstance().registerLogHandler( LogLevelDebug, LogAreaAll, this );
#else
		m_client->logInstance().registerLogHandler( LogLevelError, LogAreaAll, this );
#endif

		/*StringList ca;
		ca.push_back( "/path/to/cacert.crt" );
		m_client->setCACerts( ca );*/
	}
	catch(...)
	{
		OUTPUT_LOG("Failed: creating CNPLJabberClient\n");
	}
}

CNPLJabberClient::~CNPLJabberClient(void)
{
	SAFE_DELETE(m_reg);
	SAFE_DELETE(m_client);
	// OUTPUT_LOG("NPL_JC: jabber destructor called\n");
}


void ParaEngine::CNPLJabberClient::SetIsAuthenticated( bool bValue )
{
	m_bIsAuthenticated = bValue;
	if (m_bIsAuthenticated)
	{
		/*if (AutoRoster)
		GetRoster();
		if (AutoPresence)
		Presence(PresenceType.available,
		"online", null, Priority);*/
	}
}

//void ParaEngine::CNPLJabberClient::JabberThreadFunc::operator()()
//{
//	if(m_pJC == 0)
//		return;
//	
//	if(m_pJC->m_client->connect(false))
//	{
//		NPL::CNPLWriter writer;
//		writer.WriteName("msg");
//		writer.BeginTable();
//		writer.WriteName("jckey");
//		writer.WriteValue(m_pJC->GetJIDKey().c_str());
//		writer.EndTable();
//		writer.WriteParamDelimiter();
//		m_pJC->AddEvent(JE_OnConnect, writer.ToString().c_str());
//
//		// now receive from the sockets until stream closed. 
//		if(m_pJC->m_client->connectionImpl())
//		{
//			// create the sender thread.
//			m_pJC->SetClosing(false);
//			m_pJC->m_SendingThread = boost::thread(JabberSendThreadFunc(m_pJC));
//
//			// this function only returns when connection is terminated
//			ConnectionError err = m_pJC->m_client->connectionImpl()->receive();
//			// OUTPUT_LOG("NPL_JC: Connection is closed with code %d\n", (err == ConnNoError) ? ConnNotConnected : err);
//
//			// set closing to true to let the sender thread close gracefully. 
//			m_pJC->SetClosing(true);
//		}
//	}
//}

bool ParaEngine::CNPLJabberClient::Connect()
{
	if(m_client)
	{
		if(m_client->connectionImpl() == 0)
		{
			if(m_pIOService == 0)
				return false;
			// 2009.5.8 LXZ, we will use our own tcp connection object, since we use a different socket send/receive buffer size
			CNPLTCPConnection* pConnection = new CNPLTCPConnection(*m_pIOService, m_client, m_client->logInstance(),m_client->server(), m_client->port());
			m_client->setConnectionImpl(pConnection);
		}
		else
		{
			CNPLTCPConnection* pConnection = (CNPLTCPConnection*)(m_client->connectionImpl());
			pConnection->Reset(m_client->server(), m_client->port());
		}

		// using create a thread
		if(m_client->connect(false))
		{
			NPL::CNPLWriter writer;
			writer.WriteName("msg");
			writer.BeginTable();
			writer.WriteName("jckey");
			writer.WriteValue(GetJIDKey().c_str());
			writer.EndTable();
			writer.WriteParamDelimiter();
			AddEvent(JE_OnConnect, writer.ToString().c_str());
			return true;
		}
	}
	return false;
}

void ParaEngine::CNPLJabberClient::Close()
{
	if(m_client)
	{
		m_client->disconnect();
	}
}

bool ParaEngine::CNPLJabberClient::Login()
{
	if(m_client)
	{
		return m_client->login();
	}
	return false;
}

int ParaEngine::CNPLJabberClient::GetPriority()
{
	if(m_client)
	{
		return m_client->priority();
	}
	return m_Options.PRIORITY;
}

const char* ParaEngine::CNPLJabberClient::GetResource()
{
	if(m_client)
	{
		return m_client->resource().c_str();
	}
	return m_Options.RESOURCE.c_str();
}

bool ParaEngine::CNPLJabberClient::SetPresence( int t,const char* status,const char* show,int priority )
{
	if(m_client)
	{
		if(show)
			m_sStatus = show;
		SetPriority(priority);
		m_client->setPresence((gloox::Presence::PresenceType)t, priority, show);
	}
	return false;
}

const string& ParaEngine::CNPLJabberClient::GetStatus() const
{
	if(m_client)
	{
		// return m_client->presence().status();
		return m_sStatus;
	}
	return CGlobals::GetString();
}

void ParaEngine::CNPLJabberClient::SetRequiresSASL( bool bValue )
{
	m_Options.SASL = bValue;
	if(m_client)
	{
		m_client->setSasl(bValue);
	}
}

void ParaEngine::CNPLJabberClient::SetAutoStartCompression( bool bValue )
{
	m_Options.AUTO_COMPRESS = bValue;
	if(m_client)
	{
		m_client->setCompression(bValue);
	}
}

void ParaEngine::CNPLJabberClient::SetPort( int nValue )
{
	m_Options.PORT = nValue;
	if(m_client)
	{
		m_client->setPort(nValue);
	}
}

void ParaEngine::CNPLJabberClient::SetAutoStartTLS( bool bValue )
{
	m_Options.AUTO_TLS = bValue;
	if(m_client)
	{
		if(bValue)
			m_client->setTls(TLSPolicy(TLSOptional));
		else
			m_client->setTls(TLSPolicy(TLSDisabled));
	}
}

void ParaEngine::CNPLJabberClient::SetPassword( const char* sValue )
{
	m_Options.PASSWORD = sValue;
	if(m_client)
	{
		m_client->setPassword(sValue);
	}
}

void ParaEngine::CNPLJabberClient::SetServer( const char* Server )
{
	m_Options.SERVER = Server;
	if(m_client)
	{
		m_client->setServer(Server);
	}
}

const char* ParaEngine::CNPLJabberClient::GetUser()
{
	if(m_client)
	{
		return m_client->username().c_str();
	}
	return m_Options.USER.c_str();
}

void ParaEngine::CNPLJabberClient::SetUser( const char* sValue )
{
	m_Options.USER = sValue;
}

const char* ParaEngine::CNPLJabberClient::GetPassword()
{
	if(m_client)
	{
		return m_client->password().c_str();
	}
	return m_Options.PASSWORD.c_str();
}

const char* ParaEngine::CNPLJabberClient::GetServer()
{
	if(m_client)
	{
		return m_client->server().c_str();
	}
	return m_Options.SERVER.c_str();
}

int ParaEngine::CNPLJabberClient::GetPort()
{
	if(m_client)
	{
		return m_client->port();
	}
	return m_Options.PORT;
}

bool ParaEngine::CNPLJabberClient::GetAutoStartCompression()
{
	if(m_client)
	{
		return m_client->compression();
	}
	return m_Options.AUTO_COMPRESS;
}

const std::string ParaEngine::CNPLJabberClient::GetID()
{
	if(m_client)
	{
		return m_client->getID();
	}
	return "";
}

StatisticsJabber ParaEngine::CNPLJabberClient::GetStatistics()
{
	if(m_client)
	{
		StatisticsStruct src = m_client->getStatistics();
		StatisticsJabber stat;
		stat.totalBytesSent = src.totalBytesSent;
		stat.compressedBytesReceived = src.compressedBytesReceived;
		stat.totalBytesReceived = src.totalBytesReceived;
		stat.compressedBytesSent = src.compressedBytesSent;
		stat.uncompressedBytesSent = src.uncompressedBytesSent;
		stat.uncompressedBytesReceived = src.uncompressedBytesReceived;
		stat.totalStanzasSent = src.totalStanzasSent;
		stat.totalStanzasReceived = src.totalStanzasReceived;
		stat.iqStanzasSent = src.iqStanzasSent;
		stat.iqStanzasReceived = src.iqStanzasReceived;
		stat.messageStanzasSent = src.messageStanzasSent;
		stat.messageStanzasReceived = src.messageStanzasReceived;
		stat.s10nStanzasSent = src.s10nStanzasSent;
		stat.s10nStanzasReceived = src.s10nStanzasReceived;
		stat.presenceStanzasSent = src.presenceStanzasSent;
		stat.presenceStanzasReceived = src.presenceStanzasReceived;
		stat.encryption = src.encryption;
		stat.compression = src.compression;
		if(m_client->connectionImpl())
		{
			m_client->connectionImpl()->getStatistics(stat.totalBytesReceived, stat.totalBytesSent);
		}
		return stat;
	}
	return StatisticsJabber();
}

int ParaEngine::CNPLJabberClient::GetConnectionStatus() const
{
	if(m_client)
	{
		return (int)(m_client->state());
	}
	return 0;
}

bool ParaEngine::CNPLJabberClient::GetIsAuthenticated()
{
	if(m_client)
	{
		return m_client->authed();
	}
	return false;
}

int ParaEngine::CNPLJabberClient::GetAuthError() const
{
	if(m_client)
	{
		return (int) (m_client->authError());
	}
	return 0;
}

void ParaEngine::CNPLJabberClient::WhitespacePing()
{
	if(m_client)
	{
		m_client->whitespacePing();
	}
}

void ParaEngine::CNPLJabberClient::XmppPing( const string & sTo )
{
	if(m_client)
	{
		JID jid( sTo );
		m_client->xmppPing(jid, NULL);
	}
}

const std::string & ParaEngine::CNPLJabberClient::GetXmlLang() const
{
	if(m_client)
	{
		return m_client->xmlLang();
	}
	return CGlobals::GetString();
}

void ParaEngine::CNPLJabberClient::SetXmlLang( const std::string &xmllang )
{
	if(m_client)
	{
		m_client->setXmlLang(xmllang);
	}
}

void ParaEngine::CNPLJabberClient::handleLog( LogLevel level, LogArea area, const std::string& message )
{
	OUTPUT_LOG("NPL_JC: level: %d, area: %d, %s\n", level, area, message.c_str() );
}

void ParaEngine::CNPLJabberClient::SetClientCert( const std::string &clientKey, const std::string &clientCerts )
{
	if(m_client)
	{
		m_client->setClientCert(clientKey, clientCerts);
	}
}

int ParaEngine::CNPLJabberClient::ProcessMessage( int timeout /*= 10*/ )
{
	if(m_client )
	{
		if(/*m_client->state() == StateConnected && */
			(m_client->recv(timeout) == ConnNoError))
		{
			return 1;
		}
		else if(m_client->connectionImpl() != 0)
		{
			((CNPLTCPConnection*)m_client->connectionImpl())->HandleNotification();
			return 1;
		}
	}
	
	return 0;
}

void ParaEngine::CNPLJabberClient::onConnect()
{
	OUTPUT_LOG( "NPL_JC: connection established and user %s authenticated\n", GetUser() );
	NPL::CNPLWriter writer;
	writer.WriteName("msg");
	writer.BeginTable();
	writer.WriteName("jckey");
	writer.WriteValue(GetJIDKey().c_str());
	writer.EndTable();
	writer.WriteParamDelimiter();
	AddEvent(JE_OnAuthenticate, writer.ToString().c_str());
}

// this function is from the io_service dispatcher thread. Implementation must be thread safe. 
void ParaEngine::CNPLJabberClient::onDisconnect( ConnectionError errorcode )
{
	NPL::CNPLWriter writer;
	writer.WriteName("msg");
	writer.BeginTable();
	writer.WriteName("jckey");
	writer.WriteValue(GetJIDKey().c_str());
	writer.WriteName("errorcode");
	writer.WriteValue(errorcode);

	if(errorcode == ConnStreamError) 
	{
		writer.WriteName("streamError");
		writer.WriteValue(m_client->streamError());
		writer.WriteName("streamErrorText");
		writer.WriteValue(m_client->streamErrorText());
	}

	writer.EndTable();
	writer.WriteParamDelimiter();
	AddEvent(JE_OnDisconnect, writer.ToString().c_str());

	// Threading: the only situation that this function is called from the JabberClientThreadFunc is that 
	// the TCP connection establishment(not auth) failed. 
	// For all other situations, we can consider it is always called from the main thread via the message processor. 
	OUTPUT_LOG( "NPL_JC: disconnected: %d\n", errorcode);
	if( errorcode == ConnAuthenticationFailed )
	{
		OUTPUT_LOG( "NPL_JC: auth failed. reason: %d\n", m_client->authError());

		NPL::CNPLWriter writer;
		writer.WriteName("msg");
		writer.BeginTable();
		writer.WriteName("jckey");
		writer.WriteValue(GetJIDKey().c_str());
		writer.WriteName("reason");
		writer.WriteValue(m_client->authError());
		writer.EndTable();
		writer.WriteParamDelimiter();
		AddEvent(JE_OnAuthError, writer.ToString().c_str());
	}
}

// this function is from the io_service dispatcher thread. Implementation must be thread safe. 
bool ParaEngine::CNPLJabberClient::onTLSConnect( const CertInfo& info )
{
	OUTPUT_LOG( "NPL_JC: status: %d\nissuer: %s\npeer: %s\nprotocol: %s\nmac: %s\ncipher: %s\ncompression: %s\n"
		"from: %s\nto: %s\n",
		info.status, info.issuer.c_str(), info.server.c_str(),
		info.protocol.c_str(), info.mac.c_str(), info.cipher.c_str(),
		info.compression.c_str(), ctime( (const time_t*)&info.date_from ),
		ctime( (const time_t*)&info.date_to ) );

	NPL::CNPLWriter writer;
	writer.WriteName("msg");
	writer.BeginTable();
	writer.WriteName("jckey");
	writer.WriteValue(GetJIDKey().c_str());
	writer.WriteName("status");
	writer.WriteValue(info.status);
	writer.WriteName("issuer");
	writer.WriteValue(info.issuer.c_str());
	writer.WriteName("peer");
	writer.WriteValue(info.server.c_str());
	writer.WriteName("protocol");
	writer.WriteValue(info.protocol.c_str());
	writer.WriteName("mac");
	writer.WriteValue(info.mac.c_str());
	writer.WriteName("cipher");
	writer.WriteValue(info.cipher.c_str());
	writer.WriteName("compression");
	writer.WriteValue(info.compression.c_str());
	writer.WriteName("from");
	writer.WriteValue(ctime( (const time_t*)&info.date_from ));
	writer.WriteName("to");
	writer.WriteValue(ctime( (const time_t*)&info.date_to ));
	writer.EndTable();
	writer.WriteParamDelimiter();

	AddEvent(JE_OnTLSConnect, writer.ToString().c_str());
	return true;
}

bool ParaEngine::CNPLJabberClient::WriteRawString( const char* rawstring )
{
	OUTPUT_LOG( "NPL_JC: WriteRawString obsoleted. \n");
	return false;
}


bool ParaEngine::CNPLJabberClient::SendMessage( int msgType, const char* to, const char* body )
{
	if(GetIsAuthenticated() && to!=NULL)
	{
		Message::MessageType t = Message::Chat; 
		if(msgType == MsgType_normal)
			t = Message::Normal;
		else if(msgType == MsgType_chat)
			t = Message::Chat;
		else if(msgType == MsgType_groupchat)
			t = Message::Groupchat;
		else if(msgType == MsgType_headline)
			t = Message::Headline;
		else if(msgType == MsgType_error)
			t = Message::Error;

		string sBody;
		if(body)
		{
			sBody = body;
			#ifdef PARAENGINE_CLIENT
				StringHelper::removeInValidXMLChars(sBody);
			#endif
		}
		m_client->send( gloox::Message( t, JID(to), sBody) );
		return true;
	}
	return false;
}

bool ParaEngine::CNPLJabberClient::SendMessage( const char* to, const char* body )
{
	return SendMessage(MsgType_chat, to, body);
}
bool ParaEngine::CNPLJabberClient::WriteNPLMessage( const char* to, const char* neuronfile, const char* sCode )
{
	if(to==NULL || neuronfile == NULL || sCode ==NULL)
		return false;

	int nStringID = GetIDByString(neuronfile);

	/** The current NPL message is sent via a special normal chat message
	subtype: MsgType_normal
	subject: NI:id or NF:string
	body: msg table scode
	*/
	string subject;
	if(nStringID != -1)
	{
		char temp[20];
		itoa(nStringID,temp,10);
		// NPL string map Integer
		subject = "NI:";
		subject += temp;
	}
	else
	{
		// NPL File name
		subject = "NF:";
		subject += neuronfile;
	}
	// TODO: use a special message handler, currently I use MsgType_normal for all NPL messages. 
	bool bSucceed = true;
	string sBody;
	if(sCode)
	{
		sBody = sCode;
#ifdef PARAENGINE_CLIENT
		StringHelper::removeInValidXMLChars(sBody);
#endif
	}
	m_client->send(gloox::Message( Message::Normal, JID(to), sBody, subject ));
	return bSucceed;
}
void ParaEngine::CNPLJabberClient::handleMessage( const gloox::Message & stanza, MessageSession * /*session*/ )
{
	/*OUTPUT_LOG( "NPL_JC: type: %d, subject: %s, message: %s, thread id: %s\n", stanza.subtype(),
	stanza.subject().c_str(), stanza.body().c_str(), stanza.thread().c_str() );

	std::string msg = "You said:\n> " + stanza.body() + "\nI like that statement.";
	std::string sub;
	if( !stanza.subject().empty() )
	sub = "Re: " +  stanza.subject();

	m_messageEventFilter->raiseMessageEvent( MessageEventDisplayed );
	Sleep( 1000 );
	m_messageEventFilter->raiseMessageEvent( MessageEventComposing );
	m_chatStateFilter->setChatState( ChatStateComposing );
	Sleep( 2000 );
	m_session->send( msg, sub );*/

	
	Message::MessageType subtype = stanza.subtype();
	const string body = stanza.body();
	const string subject = stanza.subject();
	/*if(  body == "quit" )
	m_client->disconnect();
	else if( body == "subscribe" )
	m_client->rosterManager()->subscribe( stanza.from() );
	else if( body == "unsubscribe" )
	m_client->rosterManager()->unsubscribe( stanza.from() );
	else if( body == "cancel" )
	m_client->rosterManager()->cancel( stanza.from() );
	else if( body == "remove" )
	m_client->rosterManager()->remove( stanza.from() );
	else*/
	{

#if DEBUG
		OUTPUT_LOG( "NPL_JC: handleMessage: type: %d, subject: %s, message: %s, thread id: %s\n", subtype, subject.c_str(), body.c_str(), stanza.thread().c_str() );
#endif

		if (subtype == Message::Normal && 
			(subject.size()>=4 && subject[0]=='N' && (subject[1]=='I' ||  subject[1]=='F') && subject[2]==':'))
		{
			// for NPL messages
			string sNeuronFile;
			if(subject[1] == 'I')
			{

				int nStringID = atoi(&(subject[3]));
				const char* sString = GetStringByID(nStringID);
				if(sString)
					sNeuronFile = sString;
			}
			else
			{
				// TODO: for security reasons: shall we only allow remote to call trusted (string mapped) neuron file?
				// currently, we allow any activation. 
				sNeuronFile = subject.substr(3);
			}

			if(!sNeuronFile.empty())
			{
				if(NPL::NPLHelper::IsPureTable(body.c_str(), (int)(body.size())))
				{
					NPL::CNPLWriter writer(body.size()+100);
					writer.Append("msg=");
					writer.Append(body);
					writer.Append(";msg.from=");
					writer.WriteValue(stanza.from().bare());
					writer.Append(";msg.jckey=");
					writer.WriteValue(GetJIDKey().c_str());
					
					CGlobals::GetAISim()->NPLActivate(sNeuronFile.c_str(), writer.ToString().c_str(), (int)(writer.ToString().size()));
				}
#ifdef _DEBUG
				else
				{
					OUTPUT_LOG( "NPL_JC: warning: unsecure code detected in npl activation call : type: %d, subject: %s, message: %s, thread id: %s\n", subtype, subject.c_str(), body.c_str(), stanza.thread().c_str() );
				}
#endif
			}
#ifdef _DEBUG
			else
			{

				OUTPUT_LOG("NPL_JC: warning: unknown sNeuronFile is found. are we missing the string map?\n");

			}
#endif
		}
		else
		{
			NPL::CNPLWriter writer((int)(subject.size()+body.size()+100));
			writer.WriteName("msg");
			writer.BeginTable();
			writer.WriteName("jckey");
			writer.WriteValue(GetJIDKey().c_str());
			writer.WriteName("from");
			writer.WriteValue(stanza.from().bare().c_str());
			writer.WriteName("subtype");
			writer.WriteValue((int)subtype);
			writer.WriteName("subject");
			writer.WriteValue(subject.c_str());

			writer.WriteName("body");
			writer.WriteValue(body.c_str());
			if(subtype == Message::Chat)
			{
			}
			else if (subtype == Message::Error)
			{
			}
			else if (subtype == Message::Groupchat)
			{
			}
			else if (subtype == Message::Headline)
			{
			}
			else if (subtype == Message::Normal)
			{
			}
			writer.EndTable();
			writer.WriteParamDelimiter();

			AddEvent(JE_OnMessage, writer.ToString().c_str());
		}
	}
}

void ParaEngine::CNPLJabberClient::handleMessageEvent( const JID& from, MessageEventType event )
{
#if DEBUG
	OUTPUT_LOG( "NPL_JC:received event: %d from: %s\n", event, from.full().c_str() );
#endif
	NPL::CNPLWriter writer;
	writer.WriteName("msg");
	writer.BeginTable();
	writer.WriteName("jckey");
	writer.WriteValue(GetJIDKey().c_str());
	writer.WriteName("from");
	writer.WriteValue(from.bare().c_str());
	writer.WriteName("event");
	writer.WriteValue((int)event);
	writer.EndTable();
	writer.WriteParamDelimiter();

	AddEvent(JE_OnMessageEvent, writer.ToString().c_str());
}

void ParaEngine::CNPLJabberClient::handleChatState( const JID& from, ChatStateType state )
{
#if DEBUG
	OUTPUT_LOG( "NPL_JC:received state: %d from: %s\n", state, from.full().c_str() );
#endif
	NPL::CNPLWriter writer;
	writer.WriteName("msg");
	writer.BeginTable();
	writer.WriteName("jckey");
	writer.WriteValue(GetJIDKey().c_str());
	writer.WriteName("from");
	writer.WriteValue(from.bare().c_str());
	writer.WriteName("state");
	writer.WriteValue((int)state);
	writer.EndTable();
	writer.WriteParamDelimiter();

	AddEvent(JE_OnChatState, writer.ToString().c_str());
}

void ParaEngine::CNPLJabberClient::handleMessageSession( MessageSession *session )
{
#ifdef HANDLE_MESSAGE_SESSION
	OUTPUT_LOG( "NPL_JC: got new session\n");

	// this example can handle only one session. so we get rid of the old session
	m_client->disposeMessageSession( m_session );
	m_session = session;
	m_session->registerMessageHandler( this );
	m_messageEventFilter = new MessageEventFilter( m_session );
	m_messageEventFilter->registerMessageEventHandler( this );
	m_chatStateFilter = new ChatStateFilter( m_session );
	m_chatStateFilter->registerChatStateHandler( this );
#endif
}

bool ParaEngine::CNPLJabberClient::Subscribe( const char* to, const char* nickname, const char* groups , const char* msg)
{
	RosterManager* pRosterManager = m_client->rosterManager();
	if(pRosterManager)
	{
		StringList listGroups;
		if(groups)
		{
			// TODO: add multiple groups. 
			listGroups.push_back(groups);
		}
		pRosterManager->subscribe(JID(to), nickname, listGroups, msg);
		return true;
	}
	return false;
}

void ParaEngine::CNPLJabberClient::Unsubscribe( const char* to , const char* msg)
{
	RosterManager* pRosterManager = m_client->rosterManager();
	if(pRosterManager)
	{
		pRosterManager->unsubscribe(JID(to), msg);
	}
}

bool ParaEngine::CNPLJabberClient::AllowSubscription( const char* to, bool bAllow )
{
	RosterManager* pRosterManager = m_client->rosterManager();
	if(pRosterManager)
	{
		pRosterManager->ackSubscriptionRequest(JID(to), bAllow);
	}
	return false;
}

void ParaEngine::CNPLJabberClient::RemoveRosterItem( const char* to , const char* msg)
{
	OUTPUT_LOG( "NPL_JC: RemoveRosterItem obsoleted. Use Unsubscribe instead \n");
}


void ParaEngine::CNPLJabberClient::GetAgents()
{
	OUTPUT_LOG( "NPL_JC: GetAgents obsoleted. \n");
}

bool ParaEngine::CNPLJabberClient::Register( const char* jid )
{
	OUTPUT_LOG( "NPL_JC: Register obsoleted. \n");
	return false;
}

const char* ParaEngine::CNPLJabberClient::GetRosterItemDetail( const char* jid )
{
	OUTPUT_LOG("GetRosterGroups() obsoleted. Use GetRoster() instead\n ");
	return CGlobals::GetString().c_str();
}

const char* ParaEngine::CNPLJabberClient::GetRosterGroups()
{
	OUTPUT_LOG("GetRosterGroups() obsoleted. Use GetRoster() instead\n ");
	return CGlobals::GetString().c_str();
}

const char* ParaEngine::CNPLJabberClient::GetRosterItems()
{
	OUTPUT_LOG("GetRosterGroups() obsoleted. Use GetRoster() instead\n ");
	static string g_str;
	g_str.clear();
	RosterManager* pRosterManager = m_client->rosterManager();
	if(pRosterManager)
	{
		Roster* pRoster = pRosterManager->roster();
		if(pRoster)
		{
			Roster::const_iterator it = pRoster->begin();
			for( ; it != pRoster->end(); ++it )
			{
				g_str += (*it).second->jid().c_str();
				g_str += ";";
			}
		}
	}
	return CGlobals::GetString().c_str();
}

const char* ParaEngine::CNPLJabberClient::GetRoster()
{
	NPL::CNPLWriter writer;
	writer.BeginTable();

	RosterManager* pRosterManager = m_client->rosterManager();
	if(pRosterManager && pRosterManager->roster())
	{
		Roster& roster = *(pRosterManager->roster());

		Roster::const_iterator it = roster.begin();

		for( ; it != roster.end(); ++it )
		{
			writer.BeginTable();
			writer.WriteName("jid");
			writer.WriteValue((*it).second->jid().c_str());
			writer.WriteName("name");
			writer.WriteValue((*it).second->name().c_str());
			writer.WriteName("subscription");
			writer.WriteValue((*it).second->subscription());

			if((*it).second->online())
			{
				writer.WriteName("online");
				writer.WriteValue(true);
			}

			writer.WriteName("groups");
			writer.BeginTable();
			StringList g = (*it).second->groups();
			StringList::const_iterator it_g = g.begin();
			for( ; it_g != g.end(); ++it_g )
			{
				writer.WriteValue((*it_g).c_str());
			}
			writer.EndTable();

			writer.WriteName("resources");
			writer.BeginTable();
			RosterItem::ResourceMap::const_iterator rit = (*it).second->resources().begin();
			for( ; rit != (*it).second->resources().end(); ++rit )
			{
				writer.WriteName((*rit).first.c_str());
				writer.BeginTable();

				writer.WriteName("presence");
				writer.WriteValue((int)((*rit).second->presence()));
				writer.WriteName("message");
				writer.WriteValue((*rit).second->message());
				writer.WriteName("priority");
				writer.WriteValue((*rit).second->priority());

				writer.EndTable();
			}
			writer.EndTable();

			writer.EndTable();
		}
	}
	writer.EndTable();

	static string g_str;
	g_str = writer.ToString();
	return g_str.c_str();
}

void ParaEngine::CNPLJabberClient::onResourceBindError( ResourceBindError error )
{
	OUTPUT_LOG( "NPL_JC: onResourceBindError: %d\n", error );
	NPL::CNPLWriter writer;
	writer.WriteName("msg");
	writer.BeginTable();
	writer.WriteName("jckey");
	writer.WriteValue(GetJIDKey().c_str());
	writer.WriteName("error");
	writer.WriteValue((int)error);
	writer.EndTable();
	writer.WriteParamDelimiter();
	AddEvent(JE_OnResourceBindError, writer.ToString().c_str());
}

void ParaEngine::CNPLJabberClient::onSessionCreateError( SessionCreateError error )
{
	OUTPUT_LOG( "NPL_JC: onSessionCreateError: %d\n", error );
	NPL::CNPLWriter writer;
	writer.WriteName("msg");
	writer.BeginTable();
	writer.WriteName("jckey");
	writer.WriteValue(GetJIDKey().c_str());
	writer.WriteName("error");
	writer.WriteValue((int)error);
	writer.EndTable();
	writer.WriteParamDelimiter();
	AddEvent(JE_OnSessionCreateError, writer.ToString().c_str());
}

void ParaEngine::CNPLJabberClient::handleItemSubscribed( const JID& jid )
{
#if DEBUG
	OUTPUT_LOG( "NPL_JC: subscribed %s\n", jid.bare().c_str() );
#endif
	NPL::CNPLWriter writer;
	writer.WriteName("msg");
	writer.BeginTable();
	writer.WriteName("jckey");
	writer.WriteValue(GetJIDKey().c_str());
	writer.WriteName("jid");
	writer.WriteValue(jid.bare().c_str());
	writer.EndTable();
	writer.WriteParamDelimiter();
	AddEvent(JE_OnItemSubscribed, writer.ToString().c_str());
}

void ParaEngine::CNPLJabberClient::handleItemAdded( const JID& jid )
{
#if DEBUG
	OUTPUT_LOG( "NPL_JC: added %s\n", jid.bare().c_str() );
#endif
	NPL::CNPLWriter writer;
	writer.WriteName("msg");
	writer.BeginTable();
	writer.WriteName("jckey");
	writer.WriteValue(GetJIDKey().c_str());
	writer.WriteName("jid");
	writer.WriteValue(jid.bare().c_str());
	writer.EndTable();
	writer.WriteParamDelimiter();
	AddEvent(JE_OnItemAdded, writer.ToString().c_str());
}

void ParaEngine::CNPLJabberClient::handleItemUnsubscribed( const JID& jid )
{
#if DEBUG
	OUTPUT_LOG( "NPL_JC: unsubscribed %s\n", jid.bare().c_str() );
#endif
	NPL::CNPLWriter writer;
	writer.WriteName("msg");
	writer.BeginTable();
	writer.WriteName("jckey");
	writer.WriteValue(GetJIDKey().c_str());
	writer.WriteName("jid");
	writer.WriteValue(jid.bare().c_str());
	writer.EndTable();
	writer.WriteParamDelimiter();
	AddEvent(JE_OnItemUnsubscribed, writer.ToString().c_str());
}

void ParaEngine::CNPLJabberClient::handleItemRemoved( const JID& jid )
{
#if DEBUG
	OUTPUT_LOG( "NPL_JC: removed %s\n", jid.bare().c_str() );
#endif
	NPL::CNPLWriter writer;
	writer.WriteName("msg");
	writer.BeginTable();
	writer.WriteName("jckey");
	writer.WriteValue(GetJIDKey().c_str());
	writer.WriteName("jid");
	writer.WriteValue(jid.bare().c_str());
	writer.EndTable();
	writer.WriteParamDelimiter();
	AddEvent(JE_OnItemRemoved, writer.ToString().c_str());
}

void ParaEngine::CNPLJabberClient::handleItemUpdated( const JID& jid )
{
#if DEBUG
	OUTPUT_LOG( "NPL_JC: updated %s\n", jid.bare().c_str() );
#endif
	NPL::CNPLWriter writer;
	writer.WriteName("msg");
	writer.BeginTable();
	writer.WriteName("jckey");
	writer.WriteValue(GetJIDKey().c_str());
	writer.WriteName("jid");
	writer.WriteValue(jid.bare().c_str());
	writer.EndTable();
	writer.WriteParamDelimiter();
	AddEvent(JE_OnItemUpdated, writer.ToString().c_str());
}

void ParaEngine::CNPLJabberClient::handleRoster( const Roster& roster )
{
	NPL::CNPLWriter writer;
	writer.WriteName("msg");
	writer.BeginTable();
	writer.WriteName("jckey");
	writer.WriteValue(GetJIDKey().c_str());

	Roster::const_iterator it = roster.begin();
	//OUTPUT_LOG( "roster arrived\n");
	for( ; it != roster.end(); ++it )
	{
		writer.BeginTable();
		writer.WriteName("jid");
		writer.WriteValue((*it).second->jid().c_str());
		writer.WriteName("name");
		writer.WriteValue((*it).second->name().c_str());
		writer.WriteName("subscription");
		writer.WriteValue((*it).second->subscription());

		if((*it).second->online())
		{
			writer.WriteName("online");
			writer.WriteValue(true);
		}

		writer.WriteName("groups");
		writer.BeginTable();
		//OUTPUT_LOG( "jid: %s, name: %s, subscription: %d\n",(*it).second->jid().c_str(), (*it).second->name().c_str(),(*it).second->subscription() );
		StringList g = (*it).second->groups();
		StringList::const_iterator it_g = g.begin();
		for( ; it_g != g.end(); ++it_g )
		{
			//OUTPUT_LOG( "\tgroup: %s\n", (*it_g).c_str() );
			writer.WriteValue((*it_g).c_str());
		}
		writer.EndTable();

		writer.WriteName("resources");
		writer.BeginTable();
		RosterItem::ResourceMap::const_iterator rit = (*it).second->resources().begin();
		for( ; rit != (*it).second->resources().end(); ++rit )
		{
			writer.WriteName((*rit).first.c_str());
			writer.BeginTable();

			writer.WriteName("presence");
			writer.WriteValue((int)((*rit).second->presence()));
			writer.WriteName("message");
			writer.WriteValue((*rit).second->message());
			writer.WriteName("priority");
			writer.WriteValue((*rit).second->priority());

			writer.EndTable();
		}
		writer.EndTable();

		writer.EndTable();
	}

	writer.EndTable();
	writer.WriteParamDelimiter();

	AddEvent(JE_OnRoster, writer.ToString().c_str());
}

void ParaEngine::CNPLJabberClient::handleRosterError( const gloox::IQ & stanza)
{
	OUTPUT_LOG( "an roster-related error occured\n" );
}

void ParaEngine::CNPLJabberClient::handleRosterPresence( const RosterItem& item, const std::string& resource, Presence::PresenceType presence, const std::string& /*msg*/ )
{
#if DEBUG
	OUTPUT_LOG( "NPL_JC: presence received: %s/%s -- %d\n", item.jid().c_str(), resource.c_str(), presence );
#endif
	NPL::CNPLWriter writer;
	writer.WriteName("msg");
	writer.BeginTable();
	writer.WriteName("jckey");
	writer.WriteValue(GetJIDKey().c_str());
	writer.WriteName("jid");
	writer.WriteValue(item.jid().c_str());
	writer.WriteName("resource");
	writer.WriteValue(resource.c_str());
	writer.WriteName("presence");
	writer.WriteValue((int)presence);
	writer.EndTable();
	writer.WriteParamDelimiter();
	AddEvent(JE_OnRosterPresence, writer.ToString().c_str());
}

void ParaEngine::CNPLJabberClient::handleSelfPresence( const RosterItem& item, const std::string& resource, gloox::Presence::PresenceType presence, const std::string& /*msg*/ )
{
#if DEBUG
	OUTPUT_LOG( "NPL_JC: self presence received: %s/%s -- %d\n", item.jid().c_str(), resource.c_str(), presence );
#endif
	NPL::CNPLWriter writer;
	writer.WriteName("msg");
	writer.BeginTable();
	writer.WriteName("jckey");
	writer.WriteValue(GetJIDKey().c_str());
	writer.WriteName("jid");
	writer.WriteValue(item.jid().c_str());
	writer.WriteName("resource");
	writer.WriteValue(resource.c_str());
	writer.WriteName("presence");
	writer.WriteValue((int)presence);
	writer.EndTable();
	writer.WriteParamDelimiter();
	AddEvent(JE_OnSelfPresence, writer.ToString().c_str());
}

bool ParaEngine::CNPLJabberClient::handleSubscriptionRequest( const JID& jid, const std::string& msg )
{
#if DEBUG
	OUTPUT_LOG( "NPL_JC: subscription: %s\n", jid.bare().c_str() );
#endif
	NPL::CNPLWriter writer;
	writer.WriteName("msg");
	writer.BeginTable();
	writer.WriteName("jckey");
	writer.WriteValue(GetJIDKey().c_str());
	writer.WriteName("jid");
	writer.WriteValue(jid.bare().c_str());
	writer.WriteName("msg");
	writer.WriteValue(msg.c_str());
	writer.EndTable();
	writer.WriteParamDelimiter();
	AddEvent(JE_OnSubscriptionRequest, writer.ToString().c_str());

	//StringList groups;
	//JID id( jid );
	//m_client->rosterManager()->subscribe( id, "", groups, "" );
	return true;
}

bool ParaEngine::CNPLJabberClient::handleUnsubscriptionRequest( const JID& jid, const std::string& msg )
{
#if DEBUG
	OUTPUT_LOG( "NPL_JC: unsubscription: %s\n", jid.bare().c_str() );
#endif
	NPL::CNPLWriter writer;
	writer.WriteName("msg");
	writer.BeginTable();
	writer.WriteName("jckey");
	writer.WriteValue(GetJIDKey().c_str());
	writer.WriteName("jid");
	writer.WriteValue(jid.bare().c_str());
	writer.WriteName("msg");
	writer.WriteValue(msg.c_str());
	writer.EndTable();
	writer.WriteParamDelimiter();
	AddEvent(JE_OnUnsubscriptionRequest, writer.ToString().c_str());
	return true;
}

void ParaEngine::CNPLJabberClient::handleNonrosterPresence( const gloox::Presence & stanza )
{
#if DEBUG
	OUTPUT_LOG( "NPL_JC: received presence from entity not in the roster: %s\n", stanza.from().full().c_str() );
#endif
	NPL::CNPLWriter writer;
	writer.WriteName("msg");
	writer.BeginTable();
	writer.WriteName("jckey");
	writer.WriteValue(GetJIDKey().c_str());
	writer.WriteName("jid");
	writer.WriteValue(stanza.from().full().c_str());
	writer.EndTable();
	writer.WriteParamDelimiter();
	AddEvent(JE_OnNonrosterPresence, writer.ToString().c_str());
}

//////////////////////////////////////////////////////////////////////////
//
// event listener
//
//////////////////////////////////////////////////////////////////////////
void ParaEngine::CNPLJabberClient::AddEventListener( int nListenerType, const char* callbackScript )
{
	CJabberEventBinding::CScriptCallBack scriptCallback(callbackScript);
	m_eventbinding.MapEventToScript(nListenerType, &scriptCallback);
}

void ParaEngine::CNPLJabberClient::AddEventListener1( const char* sListenerType, const char* callbackScript )
{
	int nListenerType = m_eventbinding.StringToEventValue(sListenerType);
	if(nListenerType!=0)
	{
		AddEventListener(nListenerType, callbackScript);
	}
}

void ParaEngine::CNPLJabberClient::RemoveEventListener( int nListenerType, const char* callbackScript )
{
	// although the interface supports multiple listeners for the same event, the current implementation supports only one 
	// which makes the RemoveEventListener and ClearEventListener virtually the same. 
	m_eventbinding.MapEventToScript(nListenerType, NULL);
}

void ParaEngine::CNPLJabberClient::RemoveEventListener1( const char* sListenerType, const char* callbackScript )
{
	int nListenerType = m_eventbinding.StringToEventValue(sListenerType);
	if(nListenerType!=0)
	{
		RemoveEventListener(nListenerType, callbackScript);
	}
}
void ParaEngine::CNPLJabberClient::ClearEventListener( int nListenerType )
{
	m_eventbinding.MapEventToScript(nListenerType, NULL);
}

void ParaEngine::CNPLJabberClient::ClearEventListener1( const char* sListenerType )
{
	int nListenerType = m_eventbinding.StringToEventValue(sListenerType);
	if(nListenerType!=0)
	{
		ClearEventListener(nListenerType);
	}
}
void ParaEngine::CNPLJabberClient::ResetAllEventListeners()
{
	m_eventbinding.ClearAllScripts();
}

bool ParaEngine::CNPLJabberClient::AddEvent( int nEventType, const char* precode/*=NULL*/, const char* postcode/*=NULL*/ )
{
	const CJabberEventBinding::CScriptCallBack *pCallBackScript=m_eventbinding.GetEventScript(nEventType);
	if (pCallBackScript) 
	{
		string sCode;
		if(precode)
			sCode = precode;
		else
			sCode = NPL::CNPLWriter::GetEmptyMessage();
		sCode += pCallBackScript->m_sCallbackScript;
		if(postcode)
			sCode += postcode;
		CGlobals::GetAISim()->AddNPLCommand(sCode);
		return true;
	}
	return false;
}

bool ParaEngine::CNPLJabberClient::CallEvent( int nEventType, const char* precode/*=NULL*/, const char* postcode/*=NULL*/ )
{
	const CJabberEventBinding::CScriptCallBack *pCallBackScript=m_eventbinding.GetEventScript(nEventType);
	if (pCallBackScript) 
	{
		string sCode;
		if(precode)
			sCode = precode;
		else
			sCode = NPL::CNPLWriter::GetEmptyMessage();
		sCode += pCallBackScript->m_sCallbackScript;
		if(postcode)
			sCode += postcode;
		CGlobals::GetAISim()->NPLDoString(sCode.c_str());
		return true;
	}
	return false;
}

void ParaEngine::CNPLJabberClient::handleRegistrationFields( const JID& /*from*/, int fields, std::string instructions )
{
	if(m_reg)
	{
		OUTPUT_LOG( "NPL_JC: fields: %d\ninstructions: %s\n", fields, instructions.c_str() );
		RegistrationFields vals;
		vals.username = m_sUsername;
		vals.password = m_sPassword;
		m_reg->createAccount( fields, vals );
	}
}

void ParaEngine::CNPLJabberClient::handleRegistrationResult( const JID& /*from*/, RegistrationResult result )
{
	OUTPUT_LOG( "NPL_JC: registration result: %d\n", result );
	//m_client->disconnect();
}

void ParaEngine::CNPLJabberClient::handleAlreadyRegistered( const JID& /*from*/ )
{
	OUTPUT_LOG( "NPL_JC: the account already exists.\n" );
}

void ParaEngine::CNPLJabberClient::handleOOB( const JID& /*from*/, const OOB& oob )
{
	OUTPUT_LOG( "NPL_JC: OOB registration requested. %s: %s\n", oob.desc().c_str(), oob.url().c_str() );
}

void ParaEngine::CNPLJabberClient::handleDataForm( const JID& /*from*/, const DataForm& /*form*/ )
{
	OUTPUT_LOG( "NPL_JC: datForm received\n" );
}

void ParaEngine::CNPLJabberClient::RegisterCreateAccount( const char* username, const char* password )
{
	// requesting reg fields
	if(m_reg)
	{
		m_sUsername = username;
		m_sPassword = password;
		m_reg->fetchRegistrationFields();
	}
}

void ParaEngine::CNPLJabberClient::RegisterChangePassword( const char* password )
{
	if(m_reg==0)
	{
		m_reg = new Registration( m_client );
		m_reg->registerRegistrationHandler( this );
	}
	if(m_reg)
	{
		m_reg->changePassword( m_client->username(), password );
	}
}

void ParaEngine::CNPLJabberClient::RegisterRemoveAccount()
{
	if(m_reg==0)
	{
		m_reg = new Registration( m_client );
		m_reg->registerRegistrationHandler( this );
	}
	if(m_reg)
	{
		m_reg->removeAccount();
	}
}

void ParaEngine::CNPLJabberClient::AddStringMap( int nID, const char* sString )
{
	if(sString && nID!=-1)
	{
		g_mapStringMap.insert(StringMap_Type::value_type(nID, sString));
	}
	else if (sString == NULL)
	{
		StringMap_Type::left_map::iterator iter = g_mapStringMap.left.find(nID);
		if (iter != g_mapStringMap.left.end())
		{
			g_mapStringMap.left.erase(iter);
		}
	}
}

const char* ParaEngine::CNPLJabberClient::GetStringByID( int nID )
{
	StringMap_Type::left_map::iterator iter = g_mapStringMap.left.find(nID);
	if (iter != g_mapStringMap.left.end())
	{
		return iter->second.c_str();
	}
	return NULL;
}

int ParaEngine::CNPLJabberClient::GetIDByString( const char* sString )
{
	if(sString!=0)
	{
		StringMap_Type::right_map::iterator iter = g_mapStringMap.right.find(sString);
		if(iter!=g_mapStringMap.right.end())
		{
			return iter->second;
		}
	}
	return -1;
}

void ParaEngine::CNPLJabberClient::ClearStringMap()
{
	g_mapStringMap.clear();

}
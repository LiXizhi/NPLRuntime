//-----------------------------------------------------------------------------
// Class:	CNPLScriptingState
// Authors:	Li, Xizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2009.4.9
// Desc:  Cross platform for both server and client
//-----------------------------------------------------------------------------
#include "ParaEngine.h"

#include "FileManager.h"
#include "NPLRuntime.h"
#include "PluginManager.h"
#include "PluginAPI.h"
#include "EventsCenter.h"
#include "IParaWebService.h"
#include "NPLHelper.h"
#include "IParaEngineApp.h"
#include "NPLNetClient.h"
#include "NPLNetServer.h"
#include "NPLNetUDPServer.h"

#include "NPLCodec.h"
#include "FileSystemWatcher.h"
#include <time.h>

#include "AsyncLoader.h"
#include "UrlLoaders.h"

#ifdef PARAENGINE_CLIENT
#include "memdebug.h"
#endif

#include <regex>

/** define this to enable debugging of NPL code in visual studio */
//#define DEBUG_NPL_GLIA_FILE

/** whether to use .Net plugin for networking functionalities. If not, the C++ version is used. */
//#define USE_DOTNET_CLIENT

namespace NPL
{
/* list of files */
#define FILE_SHARE							0
#define FILE_NPLWEBSERVICECLIENT			1
	const char* NPLBaseFiles[] =
	{
		"script/share.lua",
		"NPLWebServiceClient.dll"
	};

	int CNPLRuntime::GetLogLevel() const
	{
		return m_nLogLevel;
	}

	void CNPLRuntime::SetLogLevel(int val)
	{
		m_nLogLevel = val;
	}

	NPL::NPLRuntimeStateType CNPLRuntime::m_defaultNPLStateType = NPLRuntimeStateType_NPL;
}
using namespace NPL;
using namespace ParaScripting;

std::string CNPLRuntime::m_tempString;

/**
* command line: local="config/local.ini"
*/
CNPLRuntime::CNPLRuntime(void)
	: m_nDefaultChannel(0)
	, m_pWebServiceClient(0)
	, m_net_udp_server(new CNPLNetUDPServer())
	, m_net_server(new CNPLNetServer())
	, m_pNetPipe(0)
	, m_pNPLNamespaceBinding(0)
	, m_bHostMainStatesInFrameMove(true)
	, m_nLogLevel(1)
{
	curl_global_init(CURL_GLOBAL_ALL);
	Init();
}

CNPLRuntime::~CNPLRuntime(void)
{
	Cleanup();
	curl_global_cleanup();
}

void CNPLRuntime::Init()
{
	if(m_runtime_state_main.get() == 0)
	{
		// SetCompressionKey(NULL, 0, 1);
		// the default "main" runtime state
		m_runtime_state_main = CreateRuntimeState("main", m_defaultNPLStateType);
		AddStateToMainThread(m_runtime_state_main);
	}
}

void CNPLRuntime::Cleanup()
{
	if (m_net_server) {
		OUTPUT_LOG("shutdown network server.\n");
		m_net_server->stop();
	}
	
	if (m_net_udp_server) {
		OUTPUT_LOG("shutdown udp server.\n");
		m_net_udp_server->stop();
	}

	// web service clients shall be cleaned up prior to the NPL runtime states.
	if(m_pWebServiceClient)
	{
		OUTPUT_LOG("shutdown web service client.\n");
		m_pWebServiceClient->DeleteThis();
		m_pWebServiceClient = NULL;
	}

	ParaEngine::CNPLNetClient::ReleaseInstance();

	while(true)
	{
		NPLRuntimeState_ptr rts;
		{
			ParaEngine::Lock lock_(m_mutex);
			NPLRuntime_Pool_Type::iterator iter = m_runtime_states.begin();
			if(iter != m_runtime_states.end())
			{
				rts = (*iter);
				m_runtime_states.erase(iter);
			}
		}
		if(rts.get() != 0)
		{
			rts->Stop();
		}
		else
			break;
	}

	m_runtime_states_with_timers.clear();
	m_runtime_states_main_threaded.clear();
	m_runtime_state_main.reset();
	m_runtime_states.clear();
	m_active_state_map.clear();
	m_net_server.reset();
	m_net_udp_server.reset();
}


void CNPLRuntime::SetDefaultNPLStateType(NPL::NPLRuntimeStateType val)
{
	m_defaultNPLStateType = val;
}

CNPLRuntime* CNPLRuntime::GetInstance()
{
	static CNPLRuntime singleton;
	return &singleton;
}


int CNPLRuntime::ActivateLocalNow(const char * sNeuronFile, const char * code /*= NULL*/, int nLength /*= 0*/)
{
	NPLFileName FullName(sNeuronFile);
	if (FullName.sNID.empty())
	{
		// local activation between local npl runtime state.
		if (!FullName.sRuntimeStateName.empty())
		{
			NPLRuntimeState_ptr rts = GetRuntimeState(FullName.sRuntimeStateName);
			if (rts.get() != 0)
			{
				if (rts->ActivateFile(FullName.sRelativePath, code, nLength) == NPL_OK)
				{
					return rts->GetLastReturnValue();
				}
			}
			else
			{
				OUTPUT_LOG("warning: runtime state %s does not exist\n", FullName.sRuntimeStateName.c_str());
				return -1;
			}
		}
		else
		{
			if (GetMainRuntimeState()->ActivateFile(FullName.sRelativePath, code, nLength) == NPL_OK)
			{
				return GetMainRuntimeState()->GetLastReturnValue();
			}
		}
	}
	return S_OK;
}


int CNPLRuntime::Activate( INPLRuntimeState* pRuntimeState, const char * sNeuronFile, const char * code /*= NULL*/,int nLength/*=0*/, int channel/*=0*/, int priority/*=2*/, int reliability/*=3*/ )
{
	if(sNeuronFile == NULL)
	{
		sNeuronFile = "";
	}

	priority = TranslatePriorityValue(priority);

	NPLFileName FullName(sNeuronFile);

	// use Dispatcher to dispatch to a proper local runtime state or a remote one.
	if(pRuntimeState == 0)
	{
		// default to main state.
		return m_runtime_state_main->Activate_async(FullName.sRelativePath, code, nLength, priority);
	}
	else
	{
		if(FullName.sNID.empty())
		{
			// local activation between local npl runtime state.
			if(!FullName.sRuntimeStateName.empty())
			{
				NPLRuntimeState_ptr rts = GetRuntimeState(FullName.sRuntimeStateName);
				if(rts.get() != 0)
				{
					return rts->Activate_async(FullName.sRelativePath, code, nLength, priority);
				}
				else
				{
					OUTPUT_LOG("warning: runtime state %s does not exist\n", FullName.sRuntimeStateName.c_str());
					return -1;
				}
			}
			else
			{
				return pRuntimeState->Activate_async(FullName.sRelativePath, code, nLength, priority);
			}
		}
		else
		{
			if (reliability == UNRELIABLE)
			{
				if (FullName.sNID[0] == '*')
				{
					unsigned short port;
					if (FullName.sNID.size() >= 2)
					{
						port = (unsigned short)atoi(FullName.sNID.c_str() + 1);
					}
					else
					{
						port = m_net_udp_server->GetHostPort() == 0 ? CNPLNetUDPServer::NPL_DEFAULT_UDP_PORT : m_net_udp_server->GetHostPort();
					}

					return m_net_udp_server->GetDispatcher().Broadcast_Async(FullName, port, code, nLength, priority);
				}
				else if (FullName.sNID[0] == '\\')
				{
					std::regex reg("\\\\\\\\(\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}) (\\d{1,5})");
					std::smatch m;
					auto bMatch = std::regex_match(FullName.sNID, m, reg);


					if (bMatch)
					{
						auto ip = m.str(1);
						auto port = atoi(m.str(2).c_str());


						return m_net_udp_server->GetDispatcher().Activate_Async2(FullName, ip.c_str(), port, code, nLength, priority);
					}
					else
					{
						return m_net_server->GetDispatcher().Activate_Async(FullName, code, nLength, priority);
					}
				}
				else
				{
					return m_net_udp_server->GetDispatcher().Activate_Async(FullName, code, nLength, priority);
				}
			}
			else
			{
				// send via dispatcher if a (remote) NID is found in file name.
				return m_net_server->GetDispatcher().Activate_Async(FullName, code, nLength, priority);
			}
		}
	}
}

int CNPLRuntime::TranslatePriorityValue(int priority)
{
	return (priority >= NPL::MEDIUM_PRIORITY) ? 0 : 1;
}

int CNPLRuntime::NPL_Activate(NPLRuntimeState_ptr runtime_state, const char * sNeuronFile, const char * code, int nLength, int channel, int priority, int reliability)
{
	if (sNeuronFile == NULL)
	{
		sNeuronFile = "";
	}

	if(sNeuronFile[0] == 'h' && sNeuronFile[1] == 't' && sNeuronFile[2] == 't' && sNeuronFile[3] == 'p')
	{
		//////////////////////////////////////////////////////////////////////////
		// for SOAP web service files. NPL may support XML-RPC in future
		//////////////////////////////////////////////////////////////////////////
		STI_DoWebServiceFile(sNeuronFile, code);
		return 0;
	}

	NPLFileName FullName(sNeuronFile);

	priority = TranslatePriorityValue(priority);


	// use Dispatcher to dispatch to a proper local runtime state or a remote one.
	if(runtime_state.get() == 0)
	{
		// default to main state.
		return m_runtime_state_main->Activate_async(FullName.sRelativePath, code, nLength, priority);
	}
	else
	{
		if(FullName.sNID.empty())
		{
			// local activation between local npl runtime state.
			if(!FullName.sRuntimeStateName.empty())
			{
				NPLRuntimeState_ptr rts = GetRuntimeState(FullName.sRuntimeStateName);
				if(rts.get() != 0)
				{
					return rts->Activate_async(FullName.sRelativePath, code, nLength, priority);
				}
				else
				{
					OUTPUT_LOG("warning: runtime state %s does not exist\n", FullName.sRuntimeStateName.c_str());
					return -1;
				}
			}
			else
			{
				return runtime_state->Activate_async(FullName.sRelativePath, code, nLength, priority);
			}
		}
		else
		{
			if (reliability == UNRELIABLE)
			{
				if (FullName.sNID[0] == '*')
				{
					unsigned short port;
					if (FullName.sNID.size() >= 2)
					{
						port = (unsigned short)atoi(FullName.sNID.c_str() + 1);
					}
					else
					{
						port = m_net_udp_server->GetHostPort() == 0 ? CNPLNetUDPServer::NPL_DEFAULT_UDP_PORT : m_net_udp_server->GetHostPort();
					}

					return m_net_udp_server->GetDispatcher().Broadcast_Async(FullName, port, code, nLength, priority);
				}
				else if (FullName.sNID[0] == '\\')
				{
					std::regex reg("\\\\\\\\(\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}\\.\\d{1,3}) (\\d{1,5})");
					std::smatch m;
					auto bMatch = std::regex_match(FullName.sNID, m, reg);


					if (bMatch)
					{
						auto ip = m.str(1);
						auto port = atoi(m.str(2).c_str());


						return m_net_udp_server->GetDispatcher().Activate_Async2(FullName, ip.c_str(), port, code, nLength, priority);
					}
					else
					{
						return m_net_server->GetDispatcher().Activate_Async(FullName, code, nLength, priority);
					}
				}
				else
				{
					return m_net_udp_server->GetDispatcher().Activate_Async(FullName, code, nLength, priority);
				}
			}
			else
			{
				// send via dispatcher if a (remote) NID is found in file name.
				return m_net_server->GetDispatcher().Activate_Async(FullName, code, nLength, priority);
			}
		}
	}
}

void CNPLRuntime::NPL_LoadFile(NPLRuntimeState_ptr runtime_state, const char* filePath, bool bReload)
{
	NPLFileName FullName(filePath);
	if(runtime_state.get() == 0)
		m_runtime_state_main->LoadFile_any(FullName.sRelativePath, bReload);
	else
		runtime_state->LoadFile_any(FullName.sRelativePath, bReload);
}

void CNPLRuntime::NPL_StartNetServer(const char* server, const char* port)
{
	m_net_server->start(server, port);
}



void CNPLRuntime::NPL_StartNetUDPServer(const char* server, unsigned short port)
{
	m_net_udp_server->start(server, port);
}

void CNPLRuntime::NPL_StopNetServer()
{
	m_net_server->stop();
}

void CNPLRuntime::NPL_StopNetUDPServer()
{
	m_net_udp_server->stop();
}

int CNPLRuntime::NPL_Ping(const char* host, const char* port, unsigned int waitTime, bool bTcp)
{
	if (bTcp)
		return CNPLNetServer::Ping(host, port, waitTime);
	else
		return CNPLNetUDPServer::Ping(host, port, waitTime);
}

void CNPLRuntime::NPL_AddPublicFile( const string& filename, int nID )
{
	m_net_server->GetDispatcher().AddPublicFile(filename, nID);
}

void CNPLRuntime::NPL_ClearPublicFiles()
{
	m_net_server->GetDispatcher().ClearPublicFiles();
}


const string& CNPLRuntime::NPL_GetSourceName()
{
	// TODO
	return ParaEngine::CGlobals::GetString(0);
}

void CNPLRuntime::NPL_SetSourceName( const char* sName )
{
	// TODO
}


void CNPLRuntime::StartNetServer( const char* server/*=NULL*/, const char* port/*=NULL*/ )
{
	NPL_StartNetServer(server, port);
}

void CNPLRuntime::StopNetServer()
{
	NPL_StopNetServer();
}

void CNPLRuntime::AddPublicFile( const string& filename, int nID )
{
	AddPublicFile(filename, nID);
}

void CNPLRuntime::ClearPublicFiles()
{
	return ClearPublicFiles();
}

void CNPLRuntime::GetIP( const char* nid, char* pOutput )
{
	string sIP = NPL_GetIP(nid);
	if(pOutput && sIP.size()<256)
	{
		strcpy(pOutput, sIP.c_str());
	}
}

void CNPLRuntime::accept( const char* tid, const char* nid /*= NULL*/ )
{
	NPL_accept(tid, nid);
}

void CNPLRuntime::reject( const char* nid, int nReason )
{
	NPL_reject(nid, nReason);
}

INPLStimulationPipe* CNPLRuntime::GetNetworkLayerPipe()
{
	return GetInstance()->m_pNetPipe;
}

bool CNPLRuntime::LoadWebServicePlugin()
{
	if(m_pWebServiceClient==0)
	{
#ifdef USE_DOTNET_CLIENT
		using namespace ParaEngine;
		//////////////////////////////////////////////////////////////////////////
		// load from plug-in
		//////////////////////////////////////////////////////////////////////////
		const char * sFileName = NPLBaseFiles[FILE_NPLWEBSERVICECLIENT];
		DLLPlugInEntity* pPluginEntity = CGlobals::GetPluginManager()->GetPluginEntity(sFileName);
		if(pPluginEntity==0)
		{
			// load the plug-in if it has never been loaded before.
			pPluginEntity = CGlobals::GetPluginManager()->LoadDLL("",sFileName);
		}
		if(pPluginEntity!=0)
		{
			// the plug-in must contain at least one implementation
			if(pPluginEntity->GetNumberOfClasses()>0)
			{
				// assume it is the first class
				ClassDescriptor* pDesc = pPluginEntity->GetClassDescriptor(0);

				// the super class must be of type WEBSERVICECLIENT_CLASS_ID, so that we can safely cast.
				if(pDesc && pDesc->SuperClassID() == WEBSERVICECLIENT_CLASS_ID)
				{
					m_pWebServiceClient = (INPLWebServiceClient*)pDesc->Create();
				}
			}
		}
#else
		m_pWebServiceClient = ParaEngine::CNPLNetClient::GetInstance();
#endif
	}
	if(m_pWebServiceClient==0)
	{
		OUTPUT_LOG("error: unable to load web service plug-in. .Net framework must be installed in order to use web service plug-in.\r\n");
	}
	return m_pWebServiceClient!=0;
}

void CNPLRuntime::AsyncDownload( const char* url, const char* destFolder, const char* callbackScript, const char* DownloaderName )
{
	// we need to download from the web server.
	if(url == 0)
		return;
	using namespace ParaEngine;
	CAsyncLoader* pAsyncLoader = &(CAsyncLoader::GetSingleton());
	string sTmp = string("NPL.AsyncDownload Started:") + string(url) + "\n";
	pAsyncLoader->log(sTmp);
	CUrlLoader* pLoader = new CUrlLoader();
	CUrlProcessor* pProcessor = new CUrlProcessor();

	pLoader->SetUrl(url);
	pProcessor->SetUrl(url);
	pProcessor->SetScriptCallback(callbackScript);
	pProcessor->SetSaveToFile(destFolder);
	if(pAsyncLoader->AddWorkItem( pLoader, pProcessor, NULL, NULL,ResourceRequestID_Asset) != S_OK)
	{
		string sTmp = string("NPL.AsyncDownload Failed:") + string(url) + "\n";
		pAsyncLoader->log(sTmp);
	}

	// obsoleted: we used UrlLoader now
	/*if(m_pWebServiceClient==0)
	{
		if(!LoadWebServicePlugin())
		{
			return;
		}
	}
	m_pWebServiceClient->AsyncDownload(url, destFolder, callbackScript, DownloaderName);*/
}

void CNPLRuntime::CancelDownload( const char* DownloaderName )
{
	OUTPUT_LOG("obsoleted call to NPL.CancelDownload: we used UrlLoader now\n");
	// obsoleted: we used UrlLoader now
	/*if(m_pWebServiceClient==0)
	{
		if(!LoadWebServicePlugin())
		{
			return;
		}
	}
	m_pWebServiceClient->CancelDownload(DownloaderName);*/
}

int CNPLRuntime::Download( const char* url, const char* destFolder, const char* callbackScript, const char* DownloaderName )
{
	OUTPUT_LOG("obsoleted call to NPL.lDownload: we used UrlLoader now\n");
	//if(m_pWebServiceClient==0)
	//{
	//	if(!LoadWebServicePlugin())
	//	{
	//		return 0;
	//	}
	//}
	//return m_pWebServiceClient->Download(url, destFolder, callbackScript, DownloaderName);
	return 1;
}


ParaEngine::INPLJabberClient* CNPLRuntime::GetJabberClient( const char* sJID )
{
	if(m_pWebServiceClient==0)
	{
		if(!LoadWebServicePlugin())
		{
			return NULL;
		}
	}
	return m_pWebServiceClient->GetJabberClient(sJID);
}

ParaEngine::INPLJabberClient* CNPLRuntime::CreateJabberClient( const char* sJID )
{
	ParaEngine::INPLJabberClient* jc = GetJabberClient(sJID);
	if(jc != 0)
		return jc;
	else
		return m_pWebServiceClient->CreateJabberClient(sJID);
}

bool CNPLRuntime::CloseJabberClient( const char* sJID )
{
	if(m_pWebServiceClient==0)
	{
		if(!LoadWebServicePlugin())
		{
			return false;
		}
	}
	return m_pWebServiceClient->CloseJabberClient(sJID);
}

void CNPLRuntime::STI_DoWebServiceFile( const char* sWebserviceFile, const char * code )
{
	if(m_pWebServiceClient==0)
	{
		if(!LoadWebServicePlugin())
		{
			return;
		}
	}
	// get the current call back script code
	const char* sCallBackScript = NULL;
	map<string, string>::iterator iter = m_mapWebServiceCallBacks.find(sWebserviceFile);
	if(iter!=m_mapWebServiceCallBacks.end())
	{
		sCallBackScript = iter->second.c_str();
	}

	using namespace ParaEngine;
	INPLWebService* pProxy = m_pWebServiceClient->GetWebService(sWebserviceFile);
	if(pProxy==0)
	{
		pProxy = m_pWebServiceClient->OpenWebService(sWebserviceFile, NULL, NULL, NULL);
		if(pProxy == 0)
		{
			OUTPUT_LOG("error: unable to activate web service file, because following web service file can not be opened: %s\r\n", sWebserviceFile);
			// output error to the scripting interface's default runtime state
			// TODO: whether we support callbacks in other custom runtime states.
			m_runtime_state_main->DoString("msg=nil;msgerror=1;");

			// call the call back script code, if any
			if (sCallBackScript != NULL)
			{
				m_runtime_state_main->DoString(sCallBackScript);
			}
			return;
		}
	}

	pProxy->InvokeAsync(NULL, code, sCallBackScript);
}


void CNPLRuntime::NPL_AddDNSRecord(const char * sDNSName, const char* sAddress)
{
	/**
	* TODO: add security: we cannot allow every one to change this as they wish. The user must be informed before this can be changed.
	*/
	//m_pNetPipe->AddDNSRecord(sDNSName, sAddress);
}
void CNPLRuntime::NPL_EnableNetwork(bool bEnable, const char* CenterName, const char* password)
{
	//
}

void CNPLRuntime::NPL_SetDefaultChannel( int channel_ID )
{
	m_nDefaultChannel = channel_ID;
}

int CNPLRuntime::NPL_GetDefaultChannel()
{
	return m_nDefaultChannel;
}

void CNPLRuntime::NPL_ResetChannelProperties()
{
	/*
	Table 1. 	Default NPL channel properties
	channel_ID	Priority	Reliability				Usage
	0		med			RELIABLE_ORDERED		System message
	1		med			UNRELIABLE_SEQUENCED	Character positions
	2		med			RELIABLE_ORDERED		Large Simulation Object transmission, such as terrain height field.
	3		high        RELIABLE_ORDERED		High order System message
	4		med			RELIABLE_ORDERED		Chat message
	14		med			RELIABLE				files transmission and advertisement
	15		med			RELIABLE_SEQUENCED		Voice transmission
	11-15	med			RELIABLE_ORDERED
	*/
	m_channelProperties.resize(16, ChannelProperty());
	m_channelProperties[0].Set(NPL::MEDIUM_PRIORITY, NPL::RELIABLE_ORDERED);
	m_channelProperties[1].Set(NPL::MEDIUM_PRIORITY, NPL::UNRELIABLE_SEQUENCED);
	m_channelProperties[3].Set(NPL::HIGH_PRIORITY, NPL::RELIABLE_ORDERED);
	m_channelProperties[14].Set(NPL::MEDIUM_PRIORITY, NPL::RELIABLE);
	m_channelProperties[15].Set(NPL::MEDIUM_PRIORITY, NPL::RELIABLE_SEQUENCED);
}

void CNPLRuntime::NPL_SetChannelProperty( int channel_ID, int priority, int reliability )
{
	if((int)m_channelProperties.size()>channel_ID)
	{
		m_channelProperties[channel_ID].m_nPriority = priority;
		m_channelProperties[channel_ID].m_nReliability = reliability;
	}
}

void CNPLRuntime::NPL_GetChannelProperty( int channel_ID, int* priority, int* reliability )
{
	if((int)m_channelProperties.size()>channel_ID)
	{
		if(priority)
		{
			*priority = m_channelProperties[channel_ID].m_nPriority;
		}
		if(reliability)
		{
			*reliability = m_channelProperties[channel_ID].m_nReliability;
		}
	}
}

void CNPLRuntime::NPL_RegisterWSCallBack( const char * sWebServiceFile, const char * sCode )
{
	m_mapWebServiceCallBacks[sWebServiceFile] = sCode;
}

void CNPLRuntime::NPL_UnregisterWSCallBack(const char * sWebServiceFile)
{
	if(sWebServiceFile== NULL || sWebServiceFile[0] == '\0')
	{
		m_mapWebServiceCallBacks.clear();
	}
	else
	{
		map<string, string>::iterator iter = m_mapWebServiceCallBacks.find(sWebServiceFile);
		if(iter!=m_mapWebServiceCallBacks.end())
		{
			m_mapWebServiceCallBacks.erase(iter);
		}
	}
}

bool CNPLRuntime::NPL_KillTimer(NPLRuntimeState_ptr runtime_state, int nIDEvent)
{
	return runtime_state->KillTimer(nIDEvent);
}

bool CNPLRuntime::NPL_SetTimer(NPLRuntimeState_ptr runtime_state, int nIDEvent, float fElapse, const char* sNeuronFile)
{
	{
		ParaEngine::Lock lock(m_mutex);
		NPLRuntime_Pool_Type::iterator iter = m_runtime_states_with_timers.find(runtime_state);
		if(iter == m_runtime_states_with_timers.end())
		{
			m_runtime_states_with_timers.insert(runtime_state);
		}
	}
	return runtime_state->SetTimer(nIDEvent, fElapse, sNeuronFile);
}

bool CNPLRuntime::NPL_ChangeTimer(NPLRuntimeState_ptr runtime_state, int nIDEvent, int dueTime, int period )
{
	return runtime_state->ChangeTimer(nIDEvent, dueTime, period);
}

string CNPLRuntime::NPL_GetIP(const char* nid)
{
	NPLConnection_ptr pConnection = NPL::CNPLRuntime::GetInstance()->GetNetServer()->GetDispatcher().GetNPLConnectionByNID(nid);
	if(pConnection)
	{
		return pConnection->GetIP();
	}
	return "";
}

void CNPLRuntime::NPL_SetProtocol(const char* nid, int protocolType /*= 0*/)
{
	NPL::CNPLRuntime::GetInstance()->GetNetServer()->GetDispatcher().NPL_SetProtocol(nid, (NPL::CNPLConnection::ProtocolType)protocolType);

}

void CNPLRuntime::NPL_accept(const char* sTID, const char* sNID)
{
	if(sTID!=0)
	{
		/* old method
		NPLConnection_ptr pConnection = NPL::CNPLRuntime::GetInstance()->GetNetServer()->GetDispatcher().GetNPLConnectionByNID(sTID);
		if(pConnection)
		{
			pConnection->SetAuthenticated(true);

			if(sNID!=0 && strcmp(sTID, sNID)!=0 )
			{
				pConnection->SetNID(sNID);
			}
		}
		*/
		// this is the new more atomic function to accept connection.
		NPL::CNPLRuntime::GetInstance()->GetNetServer()->GetDispatcher().NPL_accept(sTID, sNID);
	}
}

void CNPLRuntime::NPL_reject(const char* sNID, int nReason)
{
	if(sNID!=0)
	{
		NPLConnection_ptr pConnection = NPL::CNPLRuntime::GetInstance()->GetNetServer()->GetDispatcher().GetNPLConnectionByNID(sNID);
		if(pConnection)
		{
			NPL::CNPLRuntime::GetInstance()->GetNetServer()->GetConnectionManager().stop(pConnection, nReason);
		}
	}
}

bool CNPLRuntime::AppendURLRequest( ParaEngine::CURLRequestTask* pUrlTask, const char* sPoolName /*= NULL*/ )
{
	if(m_pWebServiceClient==0)
	{
		if(!LoadWebServicePlugin())
		{
			return false;
		}
	}
	return m_pWebServiceClient->AppendURLRequest(pUrlTask, sPoolName);
}

bool CNPLRuntime::ChangeRequestPoolSize( const char* sPoolName, int nCount )
{
	if(m_pWebServiceClient==0)
	{
		if(!LoadWebServicePlugin())
		{
			return false;
		}
	}
	return m_pWebServiceClient->ChangeRequestPoolSize(sPoolName, nCount);
}

void CNPLRuntime::SetHostMainStatesInFrameMove(bool bHostMainStatesInFrameMove)
{
	if(m_bHostMainStatesInFrameMove != bHostMainStatesInFrameMove)
	{
		if(bHostMainStatesInFrameMove!=false)
		{
			OUTPUT_LOG("warning: SetHostMainStatesInFrameMove can only be called once with false at the moment. \n");
			return;
		}
		else
		{
			OUTPUT_LOG("(main) state is now running in a separate thread. It is more responsive now. \n");
		}

		m_bHostMainStatesInFrameMove = bHostMainStatesInFrameMove;

		if(!m_bHostMainStatesInFrameMove)
		{
			// we need to start all main states in separate threads.
			ParaEngine::Lock lock_(m_mutex);
			NPLRuntime_Pool_Type::const_iterator iter, iter_end = m_runtime_states_main_threaded.end();
			for(iter = m_runtime_states_main_threaded.begin(); iter!=iter_end; ++iter)
			{
				(*iter)->Run_Async();
			}
		}
	}
}

void CNPLRuntime::Run(bool bToEnd)
{
	/** dispatch events in NPL. */
	#if !defined(PARAENGINE_MOBILE)
	ParaEngine::CFileSystemWatcherService::GetInstance()->DispatchEvents();
    #endif

	/**
	* activate all timers
	*/
	DWORD TimeTicks = ::GetTickCount();
	{
		// in case the structure is modified by other threads or during processing, we will first dump to a temp queue and then process from the queue.
		ParaEngine::Lock lock_(m_mutex);
		NPLRuntime_Pool_Type::const_iterator iter, iter_end = m_runtime_states_with_timers.end();
		for(iter = m_runtime_states_with_timers.begin(); iter!=iter_end; ++iter)
		{
			m_temp_rts_pool.push_back(*iter);
		}
	}

	{
		NPLRuntime_Temp_Pool_Type::iterator itCur, itEnd = m_temp_rts_pool.end();
		for (itCur = m_temp_rts_pool.begin(); itCur != itEnd; ++itCur)
		{
			if((*itCur)->TickTimers(TimeTicks) == 0)
			{
				m_runtime_states_with_timers.erase((*itCur));
			}
		}
		{
			// frame move ticks
			ParaEngine::Lock lock_(m_mutex);
			for (auto pState : m_runtime_states)
			{
				pState->SendTick();
			}
		}
		m_temp_rts_pool.clear();
	}


	/**
	* process results from NPL web service client
	*/
	if(m_pWebServiceClient)
	{
		PERF1("NPL_NET_REV");
		m_pWebServiceClient->ProcessResults();
		//m_pWebServiceClient->ProcessDownloaderResults();
		m_pWebServiceClient->ProcessUrlRequests();
	}

	PERF1("NPL_PROC");

	if(m_bHostMainStatesInFrameMove)
	{
		// the main runtime state is processed in the main game thread.
		{
			// in case the structure is modified by other threads or during processing, we will first dump to a temp queue and then process from the queue.
			ParaEngine::Lock lock_(m_mutex);
			NPLRuntime_Pool_Type::const_iterator iter, iter_end = m_runtime_states_main_threaded.end();
			for(iter = m_runtime_states_main_threaded.begin(); iter!=iter_end; ++iter)
			{
				m_temp_rts_pool.push_back(*iter);
			}
		}

		NPLRuntime_Temp_Pool_Type::iterator itCur, itEnd = m_temp_rts_pool.end();
		for (itCur = m_temp_rts_pool.begin(); itCur != itEnd; ++itCur)
		{
			(*itCur)->Process();
		}
		m_temp_rts_pool.clear();
	}
}

INPLRuntimeState* CNPLRuntime::CreateState(const char* name, NPLRuntimeStateType type_)
{
	return CreateRuntimeState(name, type_).get();
}

INPLRuntimeState* CNPLRuntime::GetState(const char* name)
{
	if( name == NULL)
		return m_runtime_state_main.get();

	return GetRuntimeState(name).get();
}

INPLRuntimeState* CNPLRuntime::CreateGetState(const char* name, NPLRuntimeStateType type_)
{
	return CreateGetRuntimeState(name, type_).get();
}

bool CNPLRuntime::DeleteState(INPLRuntimeState* pRuntime_state)
{
	if(pRuntime_state)
	{
		return DeleteRuntimeState(((CNPLRuntimeState*)pRuntime_state)->shared_from_this());
	}
	return false;
}

INPLRuntimeState* CNPLRuntime::GetMainState()
{
	return m_runtime_state_main.get();
}

bool CNPLRuntime::AddToMainThread(INPLRuntimeState* pRuntime_state)
{
	if(pRuntime_state)
	{
		return AddStateToMainThread(((CNPLRuntimeState*)pRuntime_state)->shared_from_this());
	}
	return false;
}

NPL::NPLRuntimeState_ptr CNPLRuntime::GetMainRuntimeState()
{
	return m_runtime_state_main;
}

NPL::NPLRuntimeState_ptr CNPLRuntime::GetRuntimeState( const string& name )
{
	if( name.empty() )
		return m_runtime_state_main;

	ParaEngine::Lock lock_(m_mutex);
	ActiveStateMap_Type::iterator iter = m_active_state_map.find(name);

	if( iter != m_active_state_map.end())
	{
		return iter->second;
	}
	return NPLRuntimeState_ptr();
}

NPL::NPLRuntimeState_ptr CNPLRuntime::CreateGetRuntimeState(const string& name, NPLRuntimeStateType type_)
{
	NPLRuntimeState_ptr runtimestate =  GetRuntimeState(name);
	if(runtimestate.get() == 0)
	{
		// create the state and run it in the main thread.
		runtimestate =  CreateRuntimeState(name, type_);
		AddStateToMainThread(runtimestate);
	}
	return runtimestate;
}

NPL::NPLRuntimeState_ptr CNPLRuntime::CreateRuntimeState(const string& name, NPLRuntimeStateType type_)
{
	NPLRuntimeState_ptr runtimestate = GetRuntimeState(name);
	if(runtimestate.get() == 0)
	{
		runtimestate.reset(new CNPLRuntimeState(name, type_));
		runtimestate->Init();
		ParaEngine::Lock lock_(m_mutex);
		m_runtime_states.insert(runtimestate);
		// PE_ASSERT(m_runtime_states.find(runtimestate)!= m_runtime_states.end());
		if(!name.empty())
			m_active_state_map[name] = runtimestate;
	}
	return runtimestate;
}

bool CNPLRuntime::DeleteRuntimeState( NPLRuntimeState_ptr runtime_state)
{
	if(runtime_state.get() == 0)
		return true;
	ParaEngine::Lock lock_(m_mutex);
	NPLRuntime_Pool_Type::iterator iter = m_runtime_states.find(runtime_state);
	if(iter != m_runtime_states.end())
	{
		(*iter)->Stop_Async();
		m_runtime_states.erase(iter);
		return true;
	}
	NPLRuntime_Pool_Type::iterator iter1 = m_runtime_states_main_threaded.find(runtime_state);
	if(iter1 != m_runtime_states_main_threaded.end())
	{
		m_runtime_states_main_threaded.erase(iter1);
	}
	if( ! runtime_state->GetName().empty() )
	{
		m_active_state_map.erase(runtime_state->GetName());
	}
	return false;
}

bool CNPLRuntime::AddStateToMainThread( NPLRuntimeState_ptr runtime_state )
{
	if(runtime_state.get() == 0)
		return true;
	ParaEngine::Lock lock_(m_mutex);
	if(m_runtime_states_main_threaded.find(runtime_state) == m_runtime_states_main_threaded.end())
	{
		runtime_state->Stop_Async();
		m_runtime_states_main_threaded.insert(runtime_state);
	}
	return true;
}

bool CNPLRuntime::RemoveStateFromMainThread( NPLRuntimeState_ptr runtime_state )
{
	if(runtime_state.get() == 0)
		return true;
	ParaEngine::Lock lock_(m_mutex);
	NPLRuntime_Pool_Type::iterator iter = m_runtime_states_main_threaded.find(runtime_state);
	if(iter != m_runtime_states_main_threaded.end())
	{
		m_runtime_states_main_threaded.erase(iter);
	}
	return true;
}

void CNPLRuntime::SetUDPUseCompression(bool bCompress)
{
	GetNetUDPServer()->GetDispatcher().SetUseCompressionRoute(bCompress);
}

void CNPLRuntime::SetUseCompression(bool bCompressIncoming, bool bCompressOutgoing)
{
	NPL::CNPLRuntime::GetInstance()->GetNetServer()->GetDispatcher().SetUseCompressionIncomingConnection(bCompressIncoming);
	NPL::CNPLRuntime::GetInstance()->GetNetServer()->GetDispatcher().SetUseCompressionOutgoingConnection(bCompressOutgoing);
}

void CNPLRuntime::SetCompressionKey(const byte* sKey, int nSize, int nUsePlainTextEncoding)
{
	if(sKey != 0)
	{
		NPL::NPLCodec::SetGenericKey(sKey, nSize);
	}
	if(nUsePlainTextEncoding != 0)
	{
		NPL::NPLCodec::UsePlainTextEncoding(nUsePlainTextEncoding  == 1);
	}
}

void CNPLRuntime::SetUDPCompressionLevel(int nLevel)
{
	GetNetUDPServer()->GetDispatcher().SetCompressionLevel(nLevel);
}
int CNPLRuntime::GetUDPCompressionLevel()
{
	return GetNetUDPServer()->GetDispatcher().GetCompressionLevel();
}

void CNPLRuntime::SetCompressionLevel(int nLevel)
{
	NPL::CNPLRuntime::GetInstance()->GetNetServer()->GetDispatcher().SetCompressionLevel(nLevel);
}
int CNPLRuntime::GetCompressionLevel()
{
	return NPL::CNPLRuntime::GetInstance()->GetNetServer()->GetDispatcher().GetCompressionLevel();
}

void CNPLRuntime::SetCompressionThreshold(int nThreshold)
{
	NPL::CNPLRuntime::GetInstance()->GetNetServer()->GetDispatcher().SetCompressionThreshold(nThreshold);
}

int CNPLRuntime::GetCompressionThreshold()
{
	return NPL::CNPLRuntime::GetInstance()->GetNetServer()->GetDispatcher().GetCompressionThreshold();
}

void CNPLRuntime::SetUDPCompressionThreshold(int nThreshold)
{
	GetNetUDPServer()->GetDispatcher().SetCompressionThreshold(nThreshold);
}

int CNPLRuntime::GetUDPCompressionThreshold()
{
	return GetNetUDPServer()->GetDispatcher().GetCompressionThreshold();
}

void CNPLRuntime::SetTCPKeepAlive(bool bEnable)
{
	NPL::CNPLRuntime::GetInstance()->GetNetServer()->SetTCPKeepAlive(bEnable);
}

bool CNPLRuntime::IsTCPKeepAliveEnabled()
{
	return NPL::CNPLRuntime::GetInstance()->GetNetServer()->IsTCPKeepAliveEnabled();
}

void CNPLRuntime::SetTCPNoDelay(bool bEnable)
{
	GetNetServer()->SetTCPNoDelay(bEnable);
}

bool CNPLRuntime::IsTCPNoDelay()
{
	return GetNetServer()->IsTcpNoDelay();
}

void CNPLRuntime::SetKeepAlive(bool bEnable)
{
	NPL::CNPLRuntime::GetInstance()->GetNetServer()->SetKeepAlive(bEnable);
}

bool CNPLRuntime::IsKeepAliveEnabled()
{
	return NPL::CNPLRuntime::GetInstance()->GetNetServer()->IsKeepAliveEnabled();
}

void CNPLRuntime::EnableIdleTimeout(bool bEnable)
{
	NPL::CNPLRuntime::GetInstance()->GetNetServer()->EnableIdleTimeout(bEnable);
}

bool CNPLRuntime::IsIdleTimeoutEnabled()
{
	return NPL::CNPLRuntime::GetInstance()->GetNetServer()->IsIdleTimeoutEnabled();
}


void CNPLRuntime::EnableUDPIdleTimeout(bool bEnable)
{
	GetNetUDPServer()->EnableIdleTimeout(bEnable);
}

bool CNPLRuntime::IsUDPIdleTimeoutEnabled()
{
	return GetNetUDPServer()->IsIdleTimeoutEnabled();
}

void CNPLRuntime::SetIdleTimeoutPeriod(int nMilliseconds)
{
	NPL::CNPLRuntime::GetInstance()->GetNetServer()->SetIdleTimeoutPeriod(nMilliseconds);
}

int CNPLRuntime::GetIdleTimeoutPeriod()
{
	return NPL::CNPLRuntime::GetInstance()->GetNetServer()->GetIdleTimeoutPeriod();
}

void CNPLRuntime::SetUDPIdleTimeoutPeriod(int nMilliseconds)
{
	GetNetUDPServer()->SetIdleTimeoutPeriod(nMilliseconds);
}

int CNPLRuntime::GetUDPIdleTimeoutPeriod()
{
	return GetNetUDPServer()->GetIdleTimeoutPeriod();
}


int CNPLRuntime::GetMaxPendingConnections()
{
	return NPL::CNPLRuntime::GetInstance()->GetNetServer()->GetMaxPendingConnections();
}

void CNPLRuntime::SetMaxPendingConnections(int val)
{
	NPL::CNPLRuntime::GetInstance()->GetNetServer()->SetMaxPendingConnections(val);
}

const std::string& NPL::CNPLRuntime::GetHostPort()
{
	return NPL::CNPLRuntime::GetInstance()->GetNetServer()->GetHostPort();
}


/** get the host port of this NPL runtime */
unsigned short NPL::CNPLRuntime::GetUDPHostPort()
{
	return GetNetUDPServer()->GetHostPort();
}

/** get the host IP of this NPL runtime */
const std::string& NPL::CNPLRuntime::GetUDPHostIP()
{
	return GetNetUDPServer()->GetHostIP();
}

/** whether the NPL runtime's udp server is started. */
bool NPL::CNPLRuntime::IsUDPServerStarted()
{
	return GetNetUDPServer()->IsServerStarted();
}

const std::string& NPL::CNPLRuntime::GetHostIP()
{
	return NPL::CNPLRuntime::GetInstance()->GetNetServer()->GetHostIP();
}

bool NPL::CNPLRuntime::IsServerStarted()
{
	return NPL::CNPLRuntime::GetInstance()->GetNetServer()->IsServerStarted();
}

void CNPLRuntime::EnableAnsiMode( bool bEnable )
{
	NPL::CNPLRuntime::GetInstance()->GetNetServer()->EnableAnsiMode(bEnable);
}

bool CNPLRuntime::IsAnsiMode()
{
	return NPL::CNPLRuntime::GetInstance()->GetNetServer()->IsAnsiMode();
}

int CNPLRuntime::GetChildAttributeObjectCount(int nColumnIndex /*= 0*/)
{
	ParaEngine::Lock lock_(m_mutex);
	return (int)m_active_state_map.size();
}

ParaEngine::IAttributeFields* CNPLRuntime::GetChildAttributeObject(int nRowIndex, int nColumnIndex /*= 0*/)
{
	ParaEngine::Lock lock_(m_mutex);
	int nIndex = 0;
	for (auto iter = m_active_state_map.begin(); iter != m_active_state_map.end(); iter++, nIndex++)
	{
		if (nIndex == nRowIndex)
			return iter->second.get();
	}
	return NULL;
}

const std::string& CNPLRuntime::GetExternalIPList()
{
	m_tempString = CNPLNetServer::GetExternalIPList();
	return m_tempString;
}

const std::string& CNPLRuntime::GetBroadcastAddressList()
{
	m_tempString = CNPLNetServer::GetBroadcastAddressList();
	return m_tempString;
}


int CNPLRuntime::InstallFields(ParaEngine::CAttributeClass* pClass, bool bOverride)
{
	using namespace ParaEngine;

	// install parent fields if there are any. Please replace __super with your parent class name.
	ParaEngine::IAttributeFields::InstallFields(pClass, bOverride);

	PE_ASSERT(pClass!=NULL);
	pClass->AddField("TCPKeepAlive",FieldType_Bool, (void*)SetTCPKeepAlive_s, (void*)IsTCPKeepAliveEnabled_s, NULL, NULL, bOverride);
	pClass->AddField("KeepAlive", FieldType_Bool, (void*)SetKeepAlive_s, (void*)IsKeepAliveEnabled_s, NULL, NULL, bOverride);
	pClass->AddField("TCPNoDelay",FieldType_Bool, (void*)SetTCPNoDelay_s, (void*)IsTCPNoDelay_s, NULL, NULL, bOverride);
	pClass->AddField("IdleTimeout",FieldType_Bool, (void*)EnableIdleTimeout_s, (void*)IsIdleTimeoutEnabled_s, NULL, NULL, bOverride);
	pClass->AddField("IdleTimeoutPeriod",FieldType_Int, (void*)SetIdleTimeoutPeriod_s, (void*)GetIdleTimeoutPeriod_s, NULL, NULL, bOverride);
	pClass->AddField("CompressionThreshold",FieldType_Int, (void*)SetCompressionThreshold_s, (void*)GetCompressionThreshold_s, NULL, NULL, bOverride);
	pClass->AddField("CompressionLevel",FieldType_Int, (void*)SetCompressionLevel_s, (void*)GetCompressionLevel_s, NULL, NULL, bOverride);
	pClass->AddField("MaxPendingConnections", FieldType_Int, (void*)SetMaxPendingConnections_s, (void*)GetMaxPendingConnections_s, NULL, NULL, bOverride);
	pClass->AddField("LogLevel", FieldType_Int, (void*)SetLogLevel_s, (void*)GetLogLevel_s, NULL, NULL, bOverride);
	pClass->AddField("EnableAnsiMode",FieldType_Bool, (void*)EnableAnsiMode_s, (void*)IsAnsiMode_s, NULL, NULL, bOverride);
	pClass->AddField("IsServerStarted", FieldType_Bool, (void*)0, (void*)IsServerStarted_s, NULL, NULL, bOverride);
	pClass->AddField("HostIP", FieldType_String, (void*)0, (void*)GetHostIP_s, NULL, NULL, bOverride);
	pClass->AddField("HostPort", FieldType_String, (void*)0, (void*)GetHostPort_s, NULL, NULL, bOverride);

	pClass->AddField("UDPIdleTimeout", FieldType_Bool, (void*)EnableUDPIdleTimeout_s, (void*)IsUDPIdleTimeoutEnabled_s, NULL, NULL, bOverride);
	pClass->AddField("UDPIdleTimeoutPeriod", FieldType_Int, (void*)SetUDPIdleTimeoutPeriod_s, (void*)GetUDPIdleTimeoutPeriod_s, NULL, NULL, bOverride);
	pClass->AddField("UDPCompressionThreshold", FieldType_Int, (void*)SetUDPCompressionThreshold_s, (void*)GetUDPCompressionThreshold_s, NULL, NULL, bOverride);
	pClass->AddField("UDPCompressionLevel", FieldType_Int, (void*)SetUDPCompressionLevel_s, (void*)GetUDPCompressionLevel_s, NULL, NULL, bOverride);
	pClass->AddField("IsUDPServerStarted", FieldType_Bool, (void*)0, (void*)IsUDPServerStarted_s, NULL, NULL, bOverride);
	pClass->AddField("UDPHostIP", FieldType_String, (void*)0, (void*)GetUDPHostIP_s, NULL, NULL, bOverride);
	pClass->AddField("UDPHostPort", FieldType_Int, (void*)0, (void*)GetUDPHostPort_s, NULL, NULL, bOverride);
	pClass->AddField("UDPUseCompression", FieldType_Bool, (void*)SetUDPUseCompression_s, nullptr, NULL, NULL, bOverride);
	pClass->AddField("EnableUDPServer", FieldType_Int, (void*)EnableUDPServer_s, nullptr, NULL, NULL, bOverride);
	pClass->AddField("DisableUDPServer", FieldType_void, (void*)DisableUDPServer_s, NULL, NULL, NULL, bOverride);

	pClass->AddField("ExternalIPList", FieldType_String, (void*)0, (void*)GetExternalIPList_s, NULL, NULL, bOverride);
	pClass->AddField("BroadcastAddressList", FieldType_String, (void*)0, (void*)GetBroadcastAddressList_s, NULL, NULL, bOverride);
	return S_OK;
}

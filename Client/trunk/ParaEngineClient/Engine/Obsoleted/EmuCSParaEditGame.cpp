//-----------------------------------------------------------------------------
// Class:	CEmuCSParaEditGame
// Authors:	Li, Xizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2006.10.7
// Desc: 
//-----------------------------------------------------------------------------
#include "DxStdAfx.h"
#include "ParaEngine.h"

#include "SceneObject.h"
#include "MathLib.h"
#include "BipedObject.h"
#include "OPCBiped.h"
#include "AutoCamera.h"

#include "EnvironmentSim.h"
#include "EnvSimServer.h"
#include "EnvSimClient.h"
#include "FrameRateController.h"
#include "terrain/GlobalTerrain.h"
#include "terrain/TTerrain.h"
#include "EmuCSParaEditGame.h"
#include "NPLHelper.h"
#include "memdebug.h"

using namespace ParaEngine;
namespace ParaEngine
{
	extern void LoadFRCServerMode();
	extern void LoadFRCNormal();
}

/** @def set how many terrain tile(including height fields, etc) are cached in memory. Generally server mode needs to cache more terrain tiles.*/
#define TERRAIN_CACHE_SIZE_STAND_ALONE		18
#define TERRAIN_CACHE_SIZE_CLIENT			18
#define TERRAIN_CACHE_SIZE_SERVER			1000

/** if defined, normal packet will not be sent, and it may be easier to track and debug other network packets.*/
// #define DISABLE_NORMAL_PACKET

/** @def server will only send a normal update to client if it has received a normal update from the client at least 3 frames ago.*/
#define SERVER_NORMAL_UPDATE_RESPONSE_TIME	(SERVER_NORMAL_UPDATE_RATE*3)

/** @def if the server does not receive anything in a good period of time, the server will disconnect from the client.*/
#define SERVER_NORMAL_UPDATE_TIMEOUT_TIME	(SERVER_NORMAL_UPDATE_RATE*40)

/** @def whether the rule defined by SERVER_NORMAL_UPDATE_TIMEOUT_TIME is applied. if this is not defined, we will not remove the OPC as long as there is a connection maintained. */
#define ENABLE_SERVER_NORMAL_UPDATE_TIMEOUT

/** max height map grid size that could be transmitted on the network.*/
#define MAX_HEIGHTMAP_SIZE	128

namespace ParaEngine
{
	struct ChannelProperty
	{
		int nChannel;
		NPL::PacketPriority priority;
		NPL::PacketReliability reliability;
	};

	ChannelProperty g_characterNormalUpdateChannelClient = {1, NPL::MEDIUM_PRIORITY, NPL::UNRELIABLE_SEQUENCED};
	ChannelProperty g_characterNormalUpdateChannelServer = {1, NPL::MEDIUM_PRIORITY, NPL::RELIABLE_SEQUENCED};
	//ChannelProperty g_characterNormalUpdateChannelClient = {6, NPL::HIGH_PRIORITY, NPL::RELIABLE_ORDERED};
	//ChannelProperty g_characterNormalUpdateChannelServer = {5, NPL::HIGH_PRIORITY, NPL::RELIABLE_ORDERED};
	ChannelProperty g_terrainChannel = {2, NPL::MEDIUM_PRIORITY, NPL::RELIABLE_ORDERED};
}


CEmuCSParaEditGame::CEmuCSParaEditGame(void)
{
	m_clientTimer.SetType(CFrameRateController::FRC_CONSTANT);
	m_clientTimer.m_fMinDeltaTime = (1.f/CLIENT_NORMAL_UPDATE_RATE);
	m_clientTimer.m_fConstDeltaTime = m_clientTimer.m_fMinDeltaTime;

	m_serverTimer.SetType(CFrameRateController::FRC_CONSTANT);
	m_serverTimer.m_fMinDeltaTime = (1.f/SERVER_NORMAL_UPDATE_RATE);
	m_serverTimer.m_fConstDeltaTime = m_serverTimer.m_fMinDeltaTime;
}

CEmuCSParaEditGame::~CEmuCSParaEditGame(void)
{
}

CEmuCSParaEditGame& CEmuCSParaEditGame::GetInstance()
{
	static CEmuCSParaEditGame g_instance;
	return g_instance;
}

CEmuCSParaEditGame::CurrentState CEmuCSParaEditGame::GetState()
{
	if(CGlobals::GetEnvSim()==NULL)
		return STATE_STAND_ALONE;

	IEnvironmentSim::SIMULATOR_TYPE type = CGlobals::GetEnvSim()->GetSimulatorType();
	switch(type)
	{
	case IEnvironmentSim::SIM_TYPE_STANDALONE:
		return STATE_STAND_ALONE;
		break;
	case IEnvironmentSim::SIM_TYPE_SERVER:
		return STATE_SERVER;
		break;
	case IEnvironmentSim::SIM_TYPE_CLIENT:
		return STATE_CLIENT;
	    break;
	default:
		return STATE_STAND_ALONE;
	    break;
	}
}

bool CEmuCSParaEditGame::SetState(CurrentState state)
{
	if(GetState() == state)
		return true;
	switch(state)
	{
	case STATE_STAND_ALONE:
		CGlobals::GetGlobalTerrain()->SetMaxTileCacheSize(TERRAIN_CACHE_SIZE_STAND_ALONE);
		CGlobals::GetScene()->SetEnvironmentSim(new CEnvironmentSim());
		LoadFRCNormal();
		break;
	case STATE_SERVER:
		CGlobals::GetGlobalTerrain()->SetMaxTileCacheSize(TERRAIN_CACHE_SIZE_SERVER);
		CGlobals::GetScene()->SetEnvironmentSim(new CEnvSimServer());
		LoadFRCServerMode();
		break;
	case STATE_CLIENT:
		CGlobals::GetGlobalTerrain()->SetMaxTileCacheSize(TERRAIN_CACHE_SIZE_CLIENT);
		CGlobals::GetScene()->SetEnvironmentSim(new CEnvSimClient());
		LoadFRCNormal();
		break;
	default:
		return false;
		break;
	}
	return true;
}

bool CEmuCSParaEditGame::OnReceiveNPLPacket(NPL::NPLPacket* p)
{
	assert(p!=0);
	CurrentState nCurrentState = GetState();
	if(nCurrentState == STATE_STAND_ALONE)
		return false;
	
	if(p->size <= 0)
		return true;
	NPL::CBitStream s((unsigned char*)(p->data), (unsigned int)p->size, false);
	int nType = 0;
	s.Read(nType);
	switch(nType)
	{
	case NPL::CS_NORMAL_UPDATE:
		{
			if(nCurrentState == STATE_CLIENT )
			{
				CEnvSimClient* pEnv = GetEnvClient();
				if(	pEnv)
				{
					pEnv->OnReceiveNormalUpdate("", &s);
				}
			}
			else if(nCurrentState == STATE_SERVER)
			{
				CEnvSimServer* pEnv = GetEnvServer();
				if(	pEnv)
				{
					NPL::NPLEmailAddress address(p->srcName);
					if(pEnv->OnReceiveNormalUpdate(address.UserName, &s))
					{
					}
				}
			}
			break;
		}
	case NPL::CS_TERRAIN_UPDATE:
		{
			OnReceiveTerrainUpdate(p);
			break;
		}
	case NPL::CS_NORMAL_CHAT:
		{
			// TODO:
			break;
		}
	default:
		return false;
		break;
	}
	return true;
}

CEnvSimServer* CEmuCSParaEditGame::GetEnvServer()
{
	if(	CGlobals::GetScene()->GetEnvSim()->GetSimulatorType() == IEnvironmentSim::SIM_TYPE_SERVER)
	{
		return (CEnvSimServer*)(CGlobals::GetScene()->GetEnvSim());
	}
	else
	{
		OUTPUT_LOG("unmatched environment simulator and state\r\n");
		return NULL;
	}
}

CEnvSimClient* CEmuCSParaEditGame::GetEnvClient()
{
	if(	CGlobals::GetScene()->GetEnvSim()->GetSimulatorType() == IEnvironmentSim::SIM_TYPE_CLIENT)
	{
		return (CEnvSimClient*)(CGlobals::GetScene()->GetEnvSim());
	}
	else
	{
		OUTPUT_LOG("unmatched environment simulator and state\r\n");
		return NULL;
	}
}

void CEmuCSParaEditGame::FrameMove()
{
#ifdef DISABLE_NORMAL_PACKET
	return;
#endif
	
	CurrentState nCurrentState = GetState();	
	if(nCurrentState == STATE_STAND_ALONE)
		return;
	else if(nCurrentState == STATE_CLIENT)
	{
		// give the network layer thread some CPU
		Sleep(0); 

		if(m_clientTimer.FrameMove(CGlobals::GetFrameRateController(FRC_GAME)->GetTime()))
		{
			NPL::INPLStimulationPipe* pPipe = CGlobals::GetNetworkLayer();
			if(!pPipe->IsNetworkLayerRunning())
				return;
			const char* sLocalUserName = pPipe->GetLocalNerveReceptorAccountName(); 
			const char* sDestWorldName = pPipe->GetLocalNerveReceptorConnectWorldName(); 
			if(sDestWorldName==NULL || sDestWorldName[0] == '\0') // not connected.
				return;

			CEnvSimClient* pEnv = GetEnvClient();
			if(	pEnv)
			{
				// increase the time
				pEnv->GetNetTime()->FrameMove();
				pEnv->HideTimeoutOPC();

				static NPL::CBitStream s;
				s.Reset();

				//////////////////////////////////////////////////////////////////////////
				// send normal update of the current player to server
				s.Write((int)NPL::CS_NORMAL_UPDATE);
				pEnv->SendNormalUpdate(sLocalUserName, &s);

				// client to server
				NPL::NPLPacket * pPacket = new NPL::NPLPacket();
				pPacket->SetType(NPL::ID_NPL_CHAT);
				pPacket->SetDestinationName(NULL, true, sDestWorldName);
				pPacket->size = s.GetNumberOfBytesUsed();
				s.CopyData( (unsigned char**)(&(pPacket->data)) );
				pPipe->PushPacket(pPacket, g_characterNormalUpdateChannelClient.nChannel, g_characterNormalUpdateChannelClient.priority, g_characterNormalUpdateChannelClient.reliability);
			}
		}
	}
	else if(nCurrentState == STATE_SERVER)
	{
		// give the network layer thread some CPU
		Sleep(0); 
		if(m_serverTimer.FrameMove(CGlobals::GetFrameRateController(FRC_GAME)->GetTime()))
		{
			CEnvSimServer* pEnv = GetEnvServer();
			if(	pEnv)
			{
				// increase the time
				pEnv->GetNetTime()->FrameMove();
				int nNetTime = pEnv->GetNetTime()->GetTime();
				// if the OPC is not updated after this time, there will be no normal update message for this OPC.
				int nResponseTime = max(1, nNetTime-SERVER_NORMAL_UPDATE_RESPONSE_TIME);

				//////////////////////////////////////////////////////////////////////////
				// loop all clients and send update. 
				static NPL::CBitStream out;
				NPL::INPLStimulationPipe* pPipe = CGlobals::GetNetworkLayer();
				if(!pPipe->IsNetworkLayerRunning())
					return;

				char name[MAX_NAME_LEN+1];
				name[MAX_NAME_LEN] = '\0';
				int nSize = MAX_NAME_LEN;
				
				list <string> UnconnectedOPC; // a list of unconnected client
				list <COPCBiped*>& listClients = CGlobals::GetScene()->GetOPCList();
				list <COPCBiped*>::iterator itCur, itEnd = listClients.end();
				for (itCur=listClients.begin();itCur!=itEnd; ++itCur)
				{
					COPCBiped* pOPC = *itCur;
					CCharacterTracker* pCharTracker = pOPC->GetCharTracker();
					if(pCharTracker->GetLastReceiveTime()>nResponseTime)
					{
						// only send a normal update to client, if we have just received one from the client just a while ago
						bool bFinished = false;
						while(!bFinished)
						{
							out.Reset();
							out.Write((int)NPL::CS_NORMAL_UPDATE);
							bFinished = pEnv->SendNormalUpdate(pOPC, &out);

							// server to client
							NPL::NPLPacket * pPacket = new NPL::NPLPacket();
							pPacket->SetType(NPL::ID_NPL_CHAT);
							pPacket->SetDestinationName(pOPC->GetIdentifier().c_str(), true, "local");
							pPacket->size = out.GetNumberOfBytesUsed();
							out.CopyData( (unsigned char**)(&(pPacket->data)) );
							HRESULT res = pPipe->PushPacket(pPacket, g_characterNormalUpdateChannelServer.nChannel, g_characterNormalUpdateChannelServer.priority, g_characterNormalUpdateChannelServer.reliability);
							if(res != S_OK)
							{
								UnconnectedOPC.push_back(pOPC->GetIdentifier());
							}
						}
					}
					else
					{
	#ifdef ENABLE_SERVER_NORMAL_UPDATE_TIMEOUT
						// the OPC is not sending normal update message in a good amount of time, so we will not send anything to client if the client does not send anything to us.
						if(pCharTracker->GetLastReceiveTime()<(nNetTime-SERVER_NORMAL_UPDATE_TIMEOUT_TIME))
						{
							// we may disconnect the OPC, if the client does not send anything to use in a very long period of time (SERVER_NORMAL_UPDATE_TIMEOUT_TIME)
							UnconnectedOPC.push_back(pOPC->GetIdentifier());
						}
	#endif
					}
				}
				
				// if there are any OPC that should be removed in this pass, do it here. 
				if(!UnconnectedOPC.empty())
				{
					list <string>::iterator itCur, itEnd = UnconnectedOPC.end();
					for (itCur = UnconnectedOPC.begin(); itCur!=itEnd; ++itCur)
					{
						CGlobals::GetScene()->DeleteOPCByID(*itCur);
					}
				}
			}
		}
	}
}

// currently it only support integer resolution such as 1,2,3,4
bool CEmuCSParaEditGame::SendTerrainUpdate( const string& sDestination, const D3DXVECTOR3& vCenter, float fRadius, float fResolution/*=1.f*/ )
{
	// TODO: use another channel to send the packet, using reliable sequenced packet, and ensure that the last packet is not discarded.
	NPL::INPLStimulationPipe* pPipe = CGlobals::GetNetworkLayer();
	if(pPipe==NULL)
		return false;
	static NPL::CBitStream s;
	s.Reset();
	s.Write((int)NPL::CS_TERRAIN_UPDATE);
	// write position
	float x = vCenter.x,z = vCenter.z;
	CGlobals::GetGlobalTerrain()->SnapPointToVertexGrid(vCenter.x, vCenter.z, &x, &z);
	s.Write(x);
	s.Write(vCenter.y);
	s.Write(z);
	s.Write(fRadius);

	// write the terrain block's height map
	TTerrain TerrainBuf;
	// TODO: to snap to terrain grid at any resolution, we need more work here, current it only support integer resolution such as 1,2,3,4 
	float vertex_spacing = CGlobals::GetGlobalTerrain()->GetVertexSpacing()*fResolution;
	fRadius = CMath::Round(fRadius/vertex_spacing)*vertex_spacing;
	TerrainBuf.CreatePlane(CMath::Round(fRadius*2/vertex_spacing)+1, 0.f, vertex_spacing);
	CGlobals::GetGlobalTerrain()->GetHeightmapOf( x, z, fRadius*2, &TerrainBuf);
	int nHeightmapSize = TerrainBuf.GetGridSize();
	// TODO: compress float array using deviation from average height.
	s.Write(nHeightmapSize);
	s.Write((const char*)TerrainBuf.GetHeightmap(), nHeightmapSize*nHeightmapSize*sizeof(float));
	
	// client to server
	NPL::NPLPacket * pPacket = new NPL::NPLPacket();
	pPacket->SetType(NPL::ID_NPL_CHAT);
	pPacket->SetDestinationName(sDestination.c_str());
	
	pPacket->size = s.GetNumberOfBytesUsed();
	s.CopyData( (unsigned char**)(&(pPacket->data)) );
	pPipe->PushPacket(pPacket, g_terrainChannel.nChannel, g_terrainChannel.priority, g_terrainChannel.reliability);
	return true;
}

bool ParaEngine::CEmuCSParaEditGame::OnReceiveTerrainUpdate( NPL::NPLPacket* p )
{
	if(p->size <= 0)
		return true;
	NPL::CBitStream s((unsigned char*)(p->data), (unsigned int)p->size, false);
	int nType = 0;
	s.Read(nType);
	assert(nType == NPL::CS_TERRAIN_UPDATE);
	// TODO: Perform further validation here, so that we know that its source is authorized and that the data is OK. 
	
	D3DXVECTOR3 vCenter(0,0,0);
	float fRadius=0;
	// read position
	s.Read(vCenter.x);
	s.Read(vCenter.y);
	s.Read(vCenter.z);
	s.Read(fRadius);

	// read the terrain block's height map
	int nHeightmapSize = 0;
	s.Read(nHeightmapSize);
	TTerrain TerrainBuf;
	if(nHeightmapSize>0 && nHeightmapSize<MAX_HEIGHTMAP_SIZE)
	{
		TerrainBuf.CreatePlane(nHeightmapSize, 0, fRadius*2.f/nHeightmapSize);
		s.Read((char*)TerrainBuf.GetHeightmap(), nHeightmapSize*nHeightmapSize*sizeof(float));
		CGlobals::GetGlobalTerrain()->SetHeightmapOf(vCenter.x, vCenter.z, fRadius*2, &TerrainBuf);

		// update the scene
		CGlobals::GetGlobalTerrain()->Update(true);
		CGlobals::GetScene()->OnTerrainChanged(vCenter, fRadius);
	}
	else
		return false;
	return true;
}

bool ParaEngine::CEmuCSParaEditGame::OnCenterUserLost( const char* sUserName )
{
	CurrentState nCurrentState = GetState();	
	if(nCurrentState == STATE_STAND_ALONE)
		return false;
	else if(nCurrentState == STATE_CLIENT)
	{
	}
	else if(nCurrentState == STATE_SERVER)
	{
		// delete the OPC by ID. 
		CGlobals::GetScene()->DeleteOPCByID(sUserName);
	}
	return true;
}

bool ParaEngine::CEmuCSParaEditGame::OnReceptorUserLost( const char* sUserName )
{
	return true;
}
//-----------------------------------------------------------------------------
// Class:	CEnvSimServer
// Authors:	Li, Xizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2006.10.3
// Desc: 
/** see also CEnvSimClient
*/
//-----------------------------------------------------------------------------
#include "DxStdAfx.h"
#include "ParaEngine.h"
#include "ParaWorldAsset.h"
#include "SceneObject.h"
#include "BipedObject.h"
#include "OPCBiped.h"
#include "TerrainTile.h"
#include "AIBase.h"
#include "EngineUtil.h"
#include "FrameRateController.h"

#include "TerrainTile.h"
#include "AutoCamera.h"
#include "MeshPhysicsObject.h"
#include "MissileObject.h"
#include "BipedStateManager.h"
#include "ParaXAnimInstance.h"

#include "EnvSimServer.h"
#include <stack>
#include <queue>
#include "memdebug.h"

using namespace ParaEngine;

#ifdef _DEBUG
/** define this to log all messages to log file.*/
#define LOG_NET_MESSAGE
#endif


/**
* @def even if an object is detected to be constant, the server will still resend it using the interval defined below, such as 1 seconds
*/
#define SERVER_GROUP0_SEND_INTERVAL		(1*SERVER_NORMAL_UPDATE_RATE)
/**
* @def even if an object is detected to be constant, the server will still resend it using the interval defined below, such as 3 seconds
*/
#define SERVER_GROUP1_SEND_INTERVAL		(3*SERVER_NORMAL_UPDATE_RATE)
/**
* @def even if an object is detected to be constant, the server will still resend it using the interval defined below, such as 6 seconds
*/
#define SERVER_GROUP2_SEND_INTERVAL		(6*SERVER_NORMAL_UPDATE_RATE)

/**
* @def max number of characters to send in a normal update packet. 
* NOTE: The packet size can exceed the MTU size of network layer (which is 570 bytes for a modem connection)
* otherwise, there will severe lag. It is better to send many small packets, and let the network layer concartenate them in one MTU-sized packet.
*/
#define MAX_CHAR_PER_UPDATE		4

/** @def how many frames to wait until a garbage collection is performed on an OPC's character tackers. 
* in case it is 3 frames per second, this is 150 seconds.
*/
#define TRACKER_GCTIME_INVERVAL		(150*SERVER_NORMAL_UPDATE_RATE)

/** an OPC will no longer track its perceived characters, if any of them has not been perceived for more than this number of frames
* in case it is 3 frames per second, this is 100 seconds.
*/
#define CHAR_TRACKER_TIMEOUT_FRAME		(100*SERVER_NORMAL_UPDATE_RATE)

/** @def smaller than which character will be moved without animation.*/
#define NO_MOVE_DISTANCE	0.5f

/** larger than which the character will immediately shift to the position.*/
#define MAX_NETWORK_PLAYER_LAG_DISTANCE		10.f

/** a fixed opc ID length in the transmission. */
#define OPC_ID_LENGTH	20

namespace ParaEngine
{
	extern CFrameRateController g_gameTime;
	#ifdef LOG_NET_MESSAGE
		bool IEnvironmentSim::m_bWriteLog = true;
	#else
		bool IEnvironmentSim::m_bWriteLog = false;
	#endif
}

/**
@def The simulation time delta advance must be smaller than this value.
currently it is 1 second lag. Greater than this value, the environment
simulator will only advance MAX_SIM_STEP
*/
#define MAX_SIM_LAG		1.0f

/**
@def this defines the worst case frame rate of the simulator
normally 25FPS works fine.
*/
#define MAX_SIM_STEP		(1.f/25.f)

/** for simulation tag */
#define SIM_TAG_START		0
#define SIM_TAG_BASIC		1
#define SIM_TAG_FINISHED	0xff

CEnvSimServer::CEnvSimServer(void)
{
}

CEnvSimServer::~CEnvSimServer(void)
{
	m_listActiveTerrain.clear();

	list< ActiveBiped* >::iterator itCurCP, itEndCP = m_listActiveBiped.end();
	for( itCurCP = m_listActiveBiped.begin(); itCurCP != itEndCP; ++ itCurCP)
	{
		ActiveBiped* pBiped = *itCurCP;
		delete pBiped;
	}
	m_listActiveBiped.clear();
	m_listVIPBipeds.clear();
}

void CEnvSimServer::Release()
{
	delete this;
}

void CEnvSimServer::Animate( double dTimeDelta )
{
	if(dTimeDelta<=0)
		return;
	if(dTimeDelta  > MAX_SIM_LAG)
	{
		/// time lag more than MAX_SIM_LAG(1) second will be detected by the environment simulator
		//OUTPUT_DEBUG("time lag more than 1 second detected by the environment simulator\n");
		dTimeDelta  = MAX_SIM_STEP;
	}


	CSceneObject* pScene = CGlobals::GetScene();
	if((pScene == NULL) || pScene->IsScenePaused() || (!(pScene->IsSceneEnabled())) )
		return;

	/** advance the game time */
	g_gameTime.FrameMoveDelta((float)dTimeDelta);

	// advance time of day
	pScene->GetSunLight().AdvanceTimeOfDay((float)dTimeDelta);

	/** Check load physics around the current player and the camera position
	* this is very game specific. It only ensures that physics object around the current player and camera is loaded.
	*/
	CBipedObject* pPlayer = pScene->GetCurrentPlayer();
	if(pPlayer)
	{
		CheckLoadPhysics(pPlayer->GetPosition(), pPlayer->GetPhysicsRadius()*2.f);
	}
	if(pScene->GetCurrentCamera())
	{
		CheckLoadPhysics(pScene->GetCurrentCamera()->GetEyePosition(), pScene->GetCurrentCamera()->GetNearPlane()*2);
	}

	UpdateGameObjects(dTimeDelta);

	{
		list_IGameObjectPtr_Type::iterator itCur, itEnd = pScene->GetSentientObjects().end();

		for (itCur = pScene->GetSentientObjects().begin(); itCur!=itEnd;++itCur)
		{
			IGameObject* pObj = (*itCur);
			if(pObj->GetSimTag() != SIM_TAG_FINISHED)
			{
				pScene->SetCurrentActor((CBaseObject*)pObj);

				if(pObj->GetSimTag() == SIM_TAG_START)
				{
					// generate way points
					pObj->PathFinding(dTimeDelta);
					// move the biped according to way point commands
					pObj->AnimateBiped(dTimeDelta);
				}

				// apply AI controller
				if(pObj->GetAIModule())
					pObj->GetAIModule()->FrameMove((float)dTimeDelta);
				if(!pObj->GetPerceiveList().empty())
					pObj->On_Perception();
				// call the frame move script if any.
				pObj->On_FrameMove();
				pObj->SetSimTag(SIM_TAG_FINISHED);

				if(pObj->HasReferences())
				{
					RefList::iterator itCur, itEnd = pObj->GetRefList().end();
					for (itCur = pObj->GetRefList().begin(); itCur!=itEnd; ++itCur)
					{
						if(itCur->m_tag == 0)
						{
							IGameObject* pRefObj = ((CBaseObject*)((*itCur).m_object))->QueryIGameObject();
							if(pRefObj!=0 && !pRefObj->IsSentient() && pRefObj->IsGlobal() && (pRefObj->GetSimTag() == SIM_TAG_START))
							{
								//////////////////////////////////////////////////////////////////////////
								// update reference object in the terrain tile according to its current position
								D3DXVECTOR3 vPos;
								pRefObj->GetPosition(&vPos);
								CTerrainTile * pTile = pScene->GetRootTile().GetTileByPoint(vPos.x, vPos.z);
								if(pTile != NULL)
								{
									if(pRefObj->GetTileContainer()!=pTile){
										if(pRefObj->GetTileContainer()!=NULL)
											pRefObj->GetTileContainer()->RemoveVisitor(pRefObj);
										pTile->AddVisitor(pRefObj);
									}
								}

								//////////////////////////////////////////////////////////////////////////
								// basic simulation: path finding, etc.
								pScene->SetCurrentActor((CBaseObject*)pRefObj);
								// generate way points
								pRefObj->PathFinding(dTimeDelta);
								// move the biped according to way point commands
								pRefObj->AnimateBiped(dTimeDelta);
								pRefObj->SetSimTag(SIM_TAG_BASIC);
							}
						}
					}
				}
			}
		}

		if(CGlobals::WillGenReport())
		{
			CGlobals::GetReport()->SetValue("sentient objects", (int)pScene->GetSentientObjects().size());
		}
	}
	/// frame move each unexploded missile objects.
	{
		list< CMissileObject* >::iterator itCurCP,itEndCP = pScene->GetMissiles().end();
		for( itCurCP = pScene->GetMissiles().begin(); itCurCP != itEndCP; ++ itCurCP)
		{
			CMissileObject* pMissile = (*itCurCP);
			if( !pMissile->IsExploded() )
			{
				pMissile->Animate(dTimeDelta);
			}
		}
	}
}

void CEnvSimServer::CheckLoadPhysics(const D3DXVECTOR3& vCenter, float fRadius)
{
	queue_CTerrainTilePtr_Type queueTiles;
	CTerrainTile* pTile = (CTerrainTile*)(&CGlobals::GetScene()->GetRootTile());
	/// breadth first transversing the scene(the root tile is ignored)
	/// pTile is now the root tile. object attached to it are never rendered directly
	bool bQueueTilesEmpty = false;
	while(bQueueTilesEmpty == false)
	{
		/// add other tiles
		for(int i=0; i<MAX_NUM_SUBTILE; i++)
		{
			if(pTile->m_subtiles[i])
			{
				/// rough culling algorithm using the quad tree terrain tiles
				/// test against a sphere round the eye
				if(pTile->m_subtiles[i]->TestCollisionSphere(& vCenter, fRadius))
				{
					queueTiles.push( pTile->m_subtiles[i] );
				}		
			}
		}

		/// go down the quad tree terrain tile to render objects
		if(queueTiles.empty())
		{
			/// even we know that the tile is empty, we still need to see if there is anything in the queueNode for rendering
			/// so when both queue are empty, we can exit the main rendering transversing loop
			bQueueTilesEmpty = true;
		}
		else
		{
			pTile = queueTiles.front();
			queueTiles.pop();
			/// For each mesh physics object in the tile, load its physics.
			{
				list< CBaseObject* >::iterator itCurCP, itEndCP = pTile->m_listFreespace.end();

				for( itCurCP = pTile->m_listFreespace.begin(); itCurCP != itEndCP; ++ itCurCP)
				{
					if((*itCurCP)->GetType()==CBaseObject::MeshPhysicsObject)
					{
						CBaseObject* pViewClippingObject = (*itCurCP)->GetViewClippingObject();
						if(pViewClippingObject->TestCollisionSphere(& vCenter, fRadius,2))
						{
							((CMeshPhysicsObject*)(*itCurCP))->LoadPhysics();
						}
					}
				}
			}
		}
	}//while(!queueTiles.empty())
}

void CEnvSimServer::UpdateGameObjects(double dTimeDelta)
{
	//	PERF1("Environment Sim");
	CSceneObject* pScene = CGlobals::GetScene();

	CTerrainTile* duplicateTiles[9];

	list_IGameObjectPtr_Type::iterator itCur;

	for (itCur = pScene->GetSentientObjects().begin(); itCur!=pScene->GetSentientObjects().end();)
	{
		// for each sentient game objects (sentientObj) in the scene
		IGameObject* sentientObj = (*itCur);
		sentientObj->SetSentientObjCount(0);
		sentientObj->GetPerceiveList().clear();

		D3DXVECTOR3 vPos;
		sentientObj->GetPosition(&vPos);
		D3DXVECTOR2 vec2Pos(vPos.x, vPos.z);
		// update itself in the terrain tile according to its current position:
		CTerrainTile * pTile = pScene->GetRootTile().GetTileByPoint(vPos.x, vPos.z);
		if(pTile != NULL)
		{
			if(sentientObj->GetTileContainer()!=pTile){
				if(sentientObj->GetTileContainer()!=NULL)
					sentientObj->GetTileContainer()->RemoveVisitor(sentientObj);
				pTile->AddVisitor(sentientObj);
			}

			for(int k=0;k<9;++k)
				duplicateTiles[k]=NULL;

			//for each valid terrain tiles(9 or more) in the neighborhood of sentientObj
			for (int i=0; i<9;++i)
			{
				CTerrainTile* pTileAdjacent = pScene->GetRootTile().GetAdjacentTile(vPos, (CTerrainTileRoot::DIRECTION)i);

				if(pTileAdjacent!=NULL)
				{
					// check if pTile is a new tile that has not been processed by the current sentient object.
					bool bNewAdjacentTile = true;

					if((pTileAdjacent==sentientObj->GetTileContainer()) || pTileAdjacent->m_fRadius != sentientObj->GetTileContainer()->m_fRadius)
					{
						for(int k=0;k<9 && (duplicateTiles[k]!=NULL);++k)
						{
							if(duplicateTiles[k] == pTileAdjacent){
								bNewAdjacentTile = false;
								break;
							}
						}
					}
					if(bNewAdjacentTile)
					{
						{
							list< IGameObject* >::iterator itCurCP, itEndCP = pTileAdjacent->m_listVisitors.end();

							for( itCurCP = pTileAdjacent->m_listVisitors.begin(); itCurCP != itEndCP; ++ itCurCP)
							{
								IGameObject* AnotherObj = (*itCurCP);
								if(AnotherObj!=sentientObj)
								{
									D3DXVECTOR3 vPosAnother;
									AnotherObj->GetPosition(&vPosAnother);

									D3DXVECTOR2 vec2PosAnother(vPosAnother.x, vPosAnother.z);
									float fDistSq = D3DXVec2LengthSq(&(vec2PosAnother-vec2Pos));

									if( ! AnotherObj->IsSentient() )
									{
										//if sentientObj falls inside the sentient area of AnotherObj
										if(AnotherObj->IsSentientWith(sentientObj) && fDistSq<= AnotherObj->GetSentientRadius()*AnotherObj->GetSentientRadius()){
											pScene->AddSentientObject(AnotherObj);
										}
									}

									//if AnotherObj falls inside the sentient area of sentientObj
									if(sentientObj->IsSentientWith(AnotherObj) &&  fDistSq<= sentientObj->GetSentientRadius()*sentientObj->GetSentientRadius())
									{
										sentientObj->SetSentientObjCount(sentientObj->GetSentientObjCount()+1);

										// if AnotherObj falls inside the perceptive area of sentientObj
										if(fDistSq<= sentientObj->GetPerceptiveRadius()*sentientObj->GetPerceptiveRadius())
										{
											sentientObj->GetPerceiveList().push_back(AnotherObj->GetIdentifier());
										}
									}
								}
							}
						}
						// skip solid object on the root tile, since they are all global objects, not static objects. 
						if(pTileAdjacent!=(&pScene->GetRootTile()))
						{
							// check collision with other static solid objects.
							list< CBaseObject* >::iterator itCurCP, itEndCP = pTileAdjacent->m_listSolidObj.end();

							for( itCurCP = pTileAdjacent->m_listSolidObj.begin(); itCurCP != itEndCP; ++ itCurCP)
							{
								IGameObject* AnotherObj = (*itCurCP)->QueryIGameObject();
								if(AnotherObj!=NULL && AnotherObj!=sentientObj && sentientObj->IsSentientWith(AnotherObj) )
								{
									D3DXVECTOR3 vPosAnother;
									AnotherObj->GetPosition(&vPosAnother);

									D3DXVECTOR2 vec2PosAnother(vPosAnother.x, vPosAnother.z);
									float fDistSq = D3DXVec2LengthSq(&(vec2PosAnother-vec2Pos));

									//if AnotherObj falls inside the sentient area of sentientObj
									if(fDistSq<= sentientObj->GetSentientRadius()*sentientObj->GetSentientRadius())
									{
										sentientObj->SetSentientObjCount(sentientObj->GetSentientObjCount()+1);

										// if AnotherObj falls inside the perceptive area of sentientObj
										if(fDistSq<= sentientObj->GetPerceptiveRadius()*sentientObj->GetPerceptiveRadius())
										{
											sentientObj->GetPerceiveList().push_back(AnotherObj->GetIdentifier());
										}
									}
								}
							}
						}
						for(int k=0;k<9;++k)
						{
							if(duplicateTiles[k]==NULL)
							{
								duplicateTiles[k] = pTileAdjacent;
								break;
							}
						}
					}//if(bNewAdjacentTile)
				}
			}
		}

		if(sentientObj->GetSentientObjCount()>0 || sentientObj->IsAlwaysSentient())
		{
			++itCur;

			{ // set the simulation tag of all sentient objects and their referenced objects to SIM_TAG_START state.
				sentientObj->SetSimTag(SIM_TAG_START);
				if(sentientObj->HasReferences())
				{
					RefList::iterator itCur, itEnd = sentientObj->GetRefList().end();
					for (itCur = sentientObj->GetRefList().begin(); itCur!=itEnd; ++itCur)
					{
						if(itCur->m_tag == 0)
						{
							IGameObject* pRefObj = ((CBaseObject*)((*itCur).m_object))->QueryIGameObject();
							if(pRefObj!=0)
							{
								pRefObj->SetSimTag(SIM_TAG_START);
							}
						}
					}
				}
			}
			// call on perceived now or in the next pass. 
		}
		else
		{
			sentientObj->On_LeaveSentientArea();
			itCur = pScene->GetSentientObjects().erase(itCur);
		}
	}
}

bool CEnvSimServer::SendCharacterUpdate(CBipedObject* pChar, CCharacterTracker* pCharTracker, NPL::CBitStream* s, const char* sIdentifier)
{
	// set update time
	assert(pCharTracker);
	int nNetTime = GetNetTime()->GetTime();

	CParaXAnimInstance* pAnimInstance = pChar->GetParaXAnimInstance(); 
	CharModelInstance* pCharModel =  pChar->GetCharModelInstance();
	if(pAnimInstance == 0 || pCharModel==0)
	{
		// sending character with no model, this is not allowed.
		return false;
	}

	
	//////////////////////////////////////////////////////////////////////////
	// gather information for group 1: position, facing, speed and animation
	// desc: group one is sent at high frequency at every frame
	//////////////////////////////////////////////////////////////////////////

	// get position
	D3DXVECTOR3 vPos;
	pChar->GetPosition(&vPos);
	// get facing
	float fFacing = pChar->GetFacing();
	// get vertical speed
	float fVerticalSpeed = pChar->GetVerticalSpeed();
	// get speed
	float fSpeed = pChar->GetSpeed();
	// get the current animation ID
	int nAnimID = pAnimInstance->GetCurrentAnimation();
	// get whether walking or running
	bool bRunning, bWalking;
	CBipedStateManager* pState = pChar->GetBipedStateManager(false);
	if(pState!=0 && !pChar->IsStanding())
	{
		bool bWalkOrRun = pState->WalkingOrRunning();
		bRunning = !bWalkOrRun;// whether running
		bWalking = bWalkOrRun;// whether walking
	}
	else
	{
		bRunning = !pChar->IsStanding();// whether running
		bWalking = false;// disable walking
	}
	// get whether global: 
	// @note: global attribute is considered unimportant, and does not contribute to bGroup1Changed
	bool bIsGlobal = pChar->IsGlobal();

	// check whether the current player has different group 1 info than the one in the tracker.
	bool bGroup1Changed = !(pCharTracker->m_vPos.CompareWith(vPos) &&
		pCharTracker->m_fFacing.CompareWith(fFacing) && 
		pCharTracker->m_fVerticalSpeed.CompareWith(fVerticalSpeed) && 
		pCharTracker->m_fSpeed.CompareWith(fSpeed) && 
		pCharTracker->m_nAnimID.CompareWith(nAnimID) && 
		pCharTracker->m_bRunning.CompareWith(bRunning) && 
		pCharTracker->m_bWalking.CompareWith(bWalking) );

	//////////////////////////////////////////////////////////////////////////
	// gather information for group 2: appearance and attribute
	// desc: group2 is only sent at a very low frequency or any of its content is different from 
	// the one in the character tracker. 
	//////////////////////////////////////////////////////////////////////////
	// get the size scale
	float fSizeScale = pChar->GetSizeScale();
	// get the base model
	ParaXEntity* pBaseModel = pCharModel->GetBaseModel();
	string sBaseModel;
	if(pBaseModel != NULL)
		sBaseModel = pBaseModel->GetKey().ToString();
	// get the skin index
	int nSkinIndex = pCharModel->GetSkin();

	bool bIsCustomModel = pCharModel->IsCustomModel();
	static CNetByteArray CustomModelData;
	if(bIsCustomModel)
	{
		byte CustomData[256];
		int nCustomDataSize = pCharModel->save(CustomData, 256);
		CustomModelData.SetData(CustomData, nCustomDataSize);
	}

	// check whether the current player has different group 2 info than the one in the tracker.
	bool bGroup2Changed = !(pCharTracker->m_fSizeScale.CompareWith(fSizeScale) && 
		pCharTracker->m_sBasemodel.CompareWith(sBaseModel) && pCharTracker->m_bIsCustomModel.CompareWith(bIsCustomModel));
	if(!bGroup2Changed)
	{
		if(bIsCustomModel)
		{
			bGroup2Changed = !(pCharTracker->m_arrayCustomModelData.CompareWith(CustomModelData));
		}
		else
		{
			bGroup2Changed = !(pCharTracker->m_nSkinIndex.CompareWith(nSkinIndex));
		}
	}
	

	/**
	Header or group0: send if any of the following is true:
	-	either group 1 or group2 is included.
	-	The last send time of group0 is older than SERVER_GROUP0_INTERVAL
	*/
	if ((bGroup1Changed || bGroup2Changed) || pCharTracker->GetLastSendTime(0)<=(nNetTime-SERVER_GROUP0_SEND_INTERVAL))
	{
		NPL::NPLStreamWriteBlock b(s); 
		
		// update send time for group 0
		pCharTracker->SetLastSendTime(nNetTime, 0);

		// write character ID
		char sID[OPC_ID_LENGTH+1];
		memset(sID, 0, sizeof(sID));
		if(sIdentifier!=NULL)
			strncpy(sID, sIdentifier, OPC_ID_LENGTH);
		else
			strncpy(sID, pChar->GetIdentifier().c_str(), OPC_ID_LENGTH);
		s->WriteAlignedBytes((const unsigned char*)sID, OPC_ID_LENGTH);

		bool bIncludeGroup1 = (bGroup1Changed || pCharTracker->GetLastSendTime(1)<=(nNetTime-SERVER_GROUP1_SEND_INTERVAL));
		bool bIncludeGroup2 = (bGroup2Changed || pCharTracker->GetLastSendTime(2)<=(nNetTime-SERVER_GROUP2_SEND_INTERVAL));
		// write booleans up to 8 bits (one byte)
		s->Write((bool)(bIncludeGroup1)); // has group1
		s->Write((bool)(bIncludeGroup2)); // has group2

		s->Write((bool)(bRunning));
		s->Write((bool)(bWalking)); 
		s->Write((bool)(bIsGlobal));
		s->AlignWriteToByteBoundary();
		
		/**
		Group1: send if any of the following is true:
		-	The current player has different group 1 info than the one in the tracker.
		-	The last send time of group1 is older than SERVER_GROUP1_INTERVAL
		Update group 1 tracker info of the player, such as group1 send time and all value trackers. 
		*/
		if(bIncludeGroup1)
		{
			// update send time in tracker for group 1
			pCharTracker->SetLastSendTime(nNetTime, 1);

			// write position
			s->WriteVector(vPos.x,vPos.y,vPos.z);
			// write facing
			s->Write((float)fFacing);
			// write vertical speed
			s->Write((float)fVerticalSpeed);
			// write speed
			s->Write((float)fSpeed);
			// write the current animation ID
			s->Write((int)nAnimID);

			//////////////////////////////////////////////////////////////////////////
			// update tracker for group 1

			// update tracker position
			pCharTracker->m_vPos.Push(nNetTime, vPos);
			// update tracker facing
			pCharTracker->m_fFacing.Push(nNetTime, fFacing);
			// update tracker vertical speed
			pCharTracker->m_fVerticalSpeed.Push(nNetTime, fVerticalSpeed);
			// update tracker speed
			pCharTracker->m_fSpeed.Push(nNetTime, fSpeed);
			// update tracker animation
			pCharTracker->m_nAnimID.Push(nNetTime, nAnimID);
			// update tracker whether walking or running
			pCharTracker->m_bRunning.Push(nNetTime, bRunning);
			pCharTracker->m_bWalking.Push(nNetTime, bWalking);
			// update tracker whether global.
			pCharTracker->m_bIsGlobal.Push(nNetTime, bIsGlobal);
		}
		/**
		Group2: send if any of the following is true:
		-	The current player has different group 2 info than the one in the tracker.
		-	The last send time of group2 is older than SERVER_GROUP2_INTERVAL
		Update group 2 tracker info of the player, such as group 2 send time and all value trackers. 
		*/
		if(bIncludeGroup2)
		{
			// update send time for group 2
			pCharTracker->SetLastSendTime(nNetTime, 2);

			// write the size of the character
			s->Write((float)fSizeScale);
			// write appearance: base model name and skin ID.
			{
				// write appearances
				NPL::NPLStreamWriteBlock Appearance(s);
				s->Write((short)sBaseModel.size());
				s->Write(sBaseModel.c_str(), (int)sBaseModel.size());

				s->Write((bool)bIsCustomModel);
				if(bIsCustomModel)
				{
					// write custom character appearance
					char CustomData[256];
					int nCustomDataSize = pCharModel->save((byte*)CustomData, 256);
					s->Write((short)nCustomDataSize);
					s->WriteAlignedBytes((const unsigned char*)CustomData, nCustomDataSize);
				}
				else
				{
					// only write skins
					s->Write(nSkinIndex);
				}
			}

			//////////////////////////////////////////////////////////////////////////
			// update tracker 
			pCharTracker->m_fSizeScale.Push(nNetTime, fSizeScale);
			pCharTracker->m_sBasemodel.Push(nNetTime, sBaseModel);
			pCharTracker->m_bIsCustomModel.Push(nNetTime, bIsCustomModel);
			if(bIsCustomModel)
			{
				pCharTracker->m_arrayCustomModelData.Push(nNetTime, CustomModelData);
			}
			else
			{
				pCharTracker->m_nSkinIndex.Push(nNetTime, nSkinIndex);
			}
		}

		if(IsLogEnabled())
		{
			OUTPUT_LOG("Server->Send to %s :",sID); 
			if(bIncludeGroup1)
				OUTPUT_LOG("Group1 vPos(%f, %f, %f) ", vPos.x, vPos.y, vPos.z);
			if(bIncludeGroup2)
				OUTPUT_LOG("Group2 %s ", sBaseModel.c_str());
			OUTPUT_LOG("\r\n");
		}

		return true;
	}
	return false;
}

bool CEnvSimServer::NetworkMoveCharacter(CBipedObject* pBiped, const D3DXVECTOR3& vDest, float fFacing, float fSpeed, float fVerticalSpeed, bool bForceRun, bool bForceWalk)
{
	CBipedStateManager* pState = NULL;
	if(pBiped==NULL || (pState=pBiped->GetBipedStateManager())== NULL)
		return false;
	D3DXVECTOR3 vOldPos(0,0,0);
	pBiped->GetPosition(&vOldPos);
	
	//float fDistSq = D3DXVec3LengthSq(&(vDest-vOldPos));
	float fDistSq = D3DXVec2LengthSq(&D3DXVECTOR2(vDest.x-vOldPos.x, vDest.z-vOldPos.z));
	float fDeltaY = vDest.y - vOldPos.y;

	if(fDistSq>MAX_NETWORK_PLAYER_LAG_DISTANCE)
	{
		pBiped->SetPosition(&vDest);
		return true;
	}
	else if (fSpeed == 0.f)
	{
		if( fDistSq< (NO_MOVE_DISTANCE*NO_MOVE_DISTANCE) )
		{
			if(pBiped->IsStanding())
			{
				// immediately move the character if the shift is very small.
#define  DETECT_JUMP_ANOTHER_DISTANCE		0.5f
#define  MAX_VERTICAL_DISTANCE		5.f
				if(fabs(fDeltaY) > MAX_VERTICAL_DISTANCE)
				{
					pBiped->SetPosition(&vDest);
				}
				else
				{
					if(fVerticalSpeed > 0.f)
					{
						if(pBiped->GetVerticalSpeed() <= 0.f || fDeltaY>DETECT_JUMP_ANOTHER_DISTANCE)
						{
							pBiped->SetPosition(&D3DXVECTOR3(vDest.x, vOldPos.y, vDest.z));
							pState->AddAction(CBipedStateManager::S_JUMP_START);
						}
						else
						{
							pBiped->SetPosition(&vDest);
						}
					}
					else if(fVerticalSpeed < 0.f)
					{
						if(pBiped->GetVerticalSpeed() != 0.f)
						{
							pBiped->SetPosition(&vDest);
						}
						else
						{
							pBiped->FallDown();
							pBiped->SetPosition(&vDest);
						}
					}
					else if(fVerticalSpeed == 0.f)
					{
						pBiped->SetPosition(&vDest);
					}
				}
			}
			else
			{
				// reset the target position in the destination command point to the new location.
				CBipedObject::BipedWayPoint& waypoint = pBiped->GetLastWayPoint();
				if( waypoint.nType == CBipedObject::BipedWayPoint::COMMAND_POINT)
				{
					waypoint.vPos = vDest;
				}
				else
				{
					pBiped->SetPosition(&vDest);
				}
			}
			return true;
		}
		else
		{
			bForceRun = true;
		}
	}

	
	pState->SetPos(vDest);
	pState->SetAngleDelta(fFacing);

	if(bForceWalk)
		pState->SetWalkOrRun(true);
	else if(bForceRun)
		pState->SetWalkOrRun(false);

	pState->AddAction(CBipedStateManager::S_WALK_POINT, (const void*)1);
	return true;
}


IGameObject* CEnvSimServer::ReceiveCharacterUpdate(NPL::CBitStream* s,bool& bIsDifferentUpdate, const char* sIdentifier)
{
	NPL::NPLStreamReadBlock b(s);
	
	//////////////////////////////////////////////////////////////////////////
	// read header or group 0:
	//////////////////////////////////////////////////////////////////////////

	// read the character ID
	char sID[OPC_ID_LENGTH+1];
	memset(sID, 0, sizeof(sID));
	s->ReadAlignedBytes((unsigned char*)sID, OPC_ID_LENGTH);
	if (sIdentifier!=0)
	{
		strncpy(sID, sIdentifier, OPC_ID_LENGTH);
	}
	bIsDifferentUpdate = true;

	CSceneObject* pScene = CGlobals::GetScene();
	CBipedObject * pObj = NULL;
	COPCBiped * pOPC = NULL;
	CBaseObject * pSceneObj = NULL;
	CCharacterTracker* pCharTracker = NULL;

	// search in OPC
	pOPC = pScene->GetOPCByID(sID);
	if(pOPC!=NULL)
		pObj = pOPC;
	
	bool bHasGroup1 = false;
	s->Read(bHasGroup1);
	bool bHasGroup2 = false;
	s->Read(bHasGroup2);
	// read whether walking or running
	bool bRunning = false;
	s->Read(bRunning);
	bool bWalking = false;
	s->Read(bWalking);
	// read whether the character is global
	bool bIsGlobal = false;
	s->Read(bIsGlobal);
	s->AlignReadToByteBoundary();

	if(IsLogEnabled())
		OUTPUT_LOG("Server->receive %s ", sID);

	//////////////////////////////////////////////////////////////////////////
	// read group 1: position, facing, speed and animation
	// desc: group one is received at high frequency at every frame
	//////////////////////////////////////////////////////////////////////////
	if(bHasGroup1)
	{
		/**
		Group1 if any: 
		-	Update OPC in the scene (validation may be included)
		*/
		
		// read position
		D3DXVECTOR3 vPos(0,0,0);
		s->ReadVector(vPos.x,vPos.y,vPos.z);
		// read facing
		float fFacing=0;
		s->Read(fFacing);
		// read vertical speed
		float fVerticalSpeed = 0.f;
		s->Read(fVerticalSpeed);
		// read speed
		float fSpeed = 0.f;
		s->Read(fSpeed);
		// read the current animation ID
		int nAnimID = 0;
		s->Read(nAnimID);

		if(pOPC==NULL)
		{
			//////////////////////////////////////////////////////////////////////////
			// add the OPC if it does not exist
			//////////////////////////////////////////////////////////////////////////

			// TODO: what happens, when a user illegally sends another player name to update the server, instead of its own. 
			// we may use the sIdentifier parameter of ReceiveCharacterUpdate() to force impersonation of the packet sender.
			// here we will assume all clients are not exploiting this logic.
			pOPC = new COPCBiped();
			/// set perceptive radius for the character
			pOPC->SetAttribute(OBJ_VOLUMN_PERCEPTIVE_RADIUS);
			pOPC->SetAttribute(OBJ_VOLUMN_FREESPACE);

			pOPC->SetIdentifier(sID);

			pOPC->SetPosition(&vPos);
			pOPC->SetFacing(fFacing);
			pScene->AddOPC(pOPC);
			pObj = pOPC;
		}

		//////////////////////////////////////////////////////////////////////////
		// move the character according to group 1 information and the tracker
		if(pObj)
		{
			if(true)
			{
				// use state manager to animate moving. 
				// Position change : use state manager to animate moving. 
				NetworkMoveCharacter(pObj, vPos, fFacing, fSpeed, fVerticalSpeed, bRunning, bWalking);
				pObj->UpdateTileContainer();
				pObj->SetFacing(fFacing);
				CParaXAnimInstance* pAnimInstance = pObj->GetParaXAnimInstance(); 
				if(pAnimInstance!=0 && pAnimInstance->GetCurrentAnimation() != nAnimID)
				{
					// TODO:  check whether the animation ID can be played. here just play anything above 46.
					if(nAnimID>46)
					{
						pObj->PlayAnimation(nAnimID);
					}
				}
			}
			else
			{
				// TODO: use a more light-weighted method to move character to the destination, like something below. 
				pObj->SetPosition(&vPos);
				pObj->UpdateTileContainer();
				pObj->SetFacing(fFacing);
				CBipedStateManager* pState = pObj->GetBipedStateManager();
				if(pState!= NULL)
				{
					if(bRunning)
						pState->SetWalkOrRun(true);
					else if(bWalking)
						pState->SetWalkOrRun(false);
				}
				pObj->PlayAnimation(nAnimID);
				pObj->SetSpeed(fSpeed);
				pObj->SetVerticalSpeed(fVerticalSpeed);
			}
		}
		if(IsLogEnabled())
			OUTPUT_LOG("group 1:vPos(%f, %f, %f) ", vPos.x, vPos.y, vPos.z);
	}
	
	//////////////////////////////////////////////////////////////////////////
	// group 2: appearance and attribute
	// desc: group2 is only sent at a very low frequency or any of its content is different from 
	// the one in the character tracker. 
	//////////////////////////////////////////////////////////////////////////
	if(bHasGroup2 && pObj!=0)
	{
		/**
		Group2 if any: 
		-	Update OPC in the scene
		*/

		// whether there is a group 2 in the rest of the packet stream.
		bool bIncludeGroup2 = false;
		float fScaling = 1.f;
		char sBaseModel[MAX_PATH];
		sBaseModel[0] = '\0';
		int nSkinIndex=0;
		bool bIsCustomModel = false;
		static CNetByteArray CustomModelData;
		
		//////////////////////////////////////////////////////////////////////////
		// read all information from group 2
		// write the size of the character
		s->Read(fScaling);
		{
			// read appearances
			NPL::NPLStreamReadBlock Appearance(s);
			if(Appearance.GetLength()>0)
			{
				short nStrSize=0;
				s->Read(nStrSize);

				if(nStrSize<MAX_PATH)
				{
					s->Read(sBaseModel, nStrSize);
					sBaseModel[nStrSize] = '\0';

					s->Read(bIsCustomModel);
					if(bIsCustomModel)
					{
						//CustomModelData.
						short nCustomDataSize=0;
						s->Read(nCustomDataSize);
						CustomModelData.resize(nCustomDataSize);
						s->ReadAlignedBytes(&CustomModelData[0], (int)nCustomDataSize);
					}
					else
					{
						s->Read(nSkinIndex);
					}
				}
			}
		}

		///////////////////////////////////////////////////////////////////////////
		// modify the character according to group 2 information and the tracker
		if(!bIsGlobal){
			// TODO: how about local character object. currently, we will just ignore them. 
		}
		
		// modify appearance
		CParaXAnimInstance* pAnimInstance = pObj->GetParaXAnimInstance(); 
		CharModelInstance * pCharModel = pObj->GetCharModelInstance();
		if(sBaseModel[0]!='\0')
		{
			bool bRefreshBaseModel = false;
			ParaXEntity* pBaseModel = NULL;
			if(pCharModel==0 /** in case character model has never been set before */|| 
				((pBaseModel = pCharModel->GetBaseModel())!=NULL && (pBaseModel->GetKey().ToString()!= sBaseModel)) )
			{
				bRefreshBaseModel = true;
				AssetEntity* pMAE = CGlobals::GetAssetManager()->LoadParaX("", sBaseModel);
				pObj->ResetBaseModel(pMAE);
				
				// since the pointer will be invalid after reseting the base model, we will refresh them here.
				pAnimInstance = pObj->GetParaXAnimInstance(); 
				pCharModel = pObj->GetCharModelInstance();
			}
			if(pCharModel!=0)
			{
				if(bIsCustomModel)
				{
					byte CustomData[256];
					int nCustomDataSize = pCharModel->save(CustomData, 256);
					if(!CustomModelData.EqualsTo(CustomData, nCustomDataSize))
					{
						pCharModel->load(&CustomModelData[0], (int)CustomModelData.size());
						bRefreshBaseModel = true;
					}
					if(bRefreshBaseModel)
					{
						pCharModel->RefreshEquipment();
						pCharModel->RefreshModel();
					}
				}
				else
				{
					if(pCharModel->GetSkin() != nSkinIndex)
					{
						pCharModel->SetSkin(nSkinIndex);
					}
				}
			}
		}

		// modify scaling
		if(pObj->GetSizeScale() != fScaling)
			pObj->SetSizeScale(fScaling);

		if(IsLogEnabled())
			OUTPUT_LOG("group 2:%s", sBaseModel);
	}

	//////////////////////////////////////////////////////////////////////////
	// Update received time of character tracker of the client OPC
	if(pOPC)
	{
		pCharTracker = pOPC->GetCharTracker();
		if(pCharTracker!=0)
		{
			int nNetTime = GetNetTime()->GetTime();
			// update tracker time, so that we know when we have received a normal update from client.
			if(pCharTracker->GetLastReceiveTime() >=nNetTime)
				bIsDifferentUpdate = false;
			pCharTracker->SetLastReceiveTime(nNetTime);
		}
	}
	
	if(IsLogEnabled())
		OUTPUT_LOG("\r\n");
	return pObj;
}

bool CEnvSimServer::SendNormalUpdate(COPCBiped * pOPC, NPL::CBitStream* s)
{
	assert(pOPC!=NULL);
	NPL::NPLStreamWriteBlock b(s);

	if(IsLogEnabled())
	{
		OUTPUT_LOG("CEnvSimServer::SendNormal Update begins:=================================\r\n");
	}
	int m_nCount = 0;
	CCharacterTracker* pCharTracker = NULL;
	int nNetTime = GetNetTime()->GetTime();
	int nNumPerceived = pOPC->GetNumOfPerceivedObject();
	pCharTracker = pOPC->GetCharTracker();
	if(pCharTracker && pCharTracker->GetLastSendTime()<nNetTime)
	{
		++m_nCount;
		SendCharacterUpdate(pOPC, pCharTracker, s);

		if((nNetTime-pOPC->GetLastTrackerGCTime())>=TRACKER_GCTIME_INVERVAL)
		{
			pOPC->SetLastTrackerGCTime(nNetTime);
			pOPC->RemoveCharTracker( nNetTime - CHAR_TRACKER_TIMEOUT_FRAME);
		}
	}

	for(int i=0;i<nNumPerceived;++i)
	{
		IGameObject* pObj = pOPC->GetPerceivedObject(i);
		if(pObj!=NULL && pObj->IsBiped())
		{
			pCharTracker = pOPC->GetCharTracker(pObj->GetIdentifier().c_str());
			if(pCharTracker==0)
			{
				// create if not exist
				pCharTracker = pOPC->CreateCharTracker(pObj->GetIdentifier().c_str());
			}
			if(pCharTracker)
			{
				pCharTracker->SetLastReceiveTime(nNetTime);

				if(pCharTracker->GetLastSendTime()<nNetTime)
				{
					if(m_nCount++<MAX_CHAR_PER_UPDATE)
					{
						if(SendCharacterUpdate((CBipedObject*)pObj,pCharTracker, s))
						{

						}
						else
						{
							--m_nCount;
						}
					}
					else
						return false; // the packet is full, request another Normal Update packet by returning false;
				}
			}
		}
		else
		{
			// TODO: how about other objects? send them by write block
		}
	}
	if(IsLogEnabled())
	{
		OUTPUT_LOG("CEnvSimServer::SendNormal Update ends:=================================\r\n\r\n\r\n");
	}
	return true;
}

bool CEnvSimServer::SendNormalUpdate(const string& id, NPL::CBitStream* s)
{
	CSceneObject* pScene = CGlobals::GetScene();
	COPCBiped * pOPC = NULL;
	if(pScene && (pOPC = pScene->GetOPCByID(id))!=0)
	{
		SendNormalUpdate(pOPC, s);
	}
	return true;
}

bool CEnvSimServer::OnReceiveNormalUpdate(const string& id, NPL::CBitStream* s)
{
	if(IsLogEnabled())
	{
		OUTPUT_LOG("CEnvSimServer::OnReceiveNormal Update begins:=================================\r\n");
	}
	NPL::NPLStreamReadBlock b(s);
	
	bool bDifferentUpdate = true;
	if( b.HasData()) // can be while( b.HasData()) if we allow the client to sent multiple characters
	{
		ReceiveCharacterUpdate(s, bDifferentUpdate);
	}

	if(IsLogEnabled())
	{
		OUTPUT_LOG("CEnvSimServer::OnReceiveNormal Update ends:=================================\r\n\r\n\r\n");
	}
	return bDifferentUpdate;
}

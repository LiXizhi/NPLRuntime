//----------------------------------------------------------------------
// Class:	CEnvironmentSim
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Date:	2004.3.8
// Revised: 2006.3.10
//
// desc: 
/** 
2006.3.10 sentient algorithm: good for both client and server side computing. 
@change 2006.8.23: whether an object is sentient has nothing to do with whether it is referenced by other objects. 
	However, if an object is sentient, all its referenced objects are at least simulated by the basic path-finding algorithm. 
	and automatically updated in the container tile when their location changes.  

// Compute next scene for all game objects and generate the vicinity list. 
CEnvironmentSim::Simulate(dTimeDelta){
	
	// Load physics around the current player and the camera position. 
	// This is very game specific. Usually, it only ensures that physics object 
	// around the current player and camera eye position is loaded.
	Call CheckLoadPhysics(player?position, player?radius*2)
	Call CheckLoadPhysics(camera?position, camera?near_plane*2)

	// Pass 1:Update Game Objects: building perceived object lists for each sentient object
	for each sentient game objects (sentientObj) in the scene{
		sentientObj.m_nSentientObjCount=0;
		sentientObj.m_PerceivedList.clear();

		update itself in the terrain tile according to its current position: 

		for each valid terrain tiles(9 or more) in the neighbourhood of sentientObj{
			for each object (AnotherObj) in the terrain tile{
				if(AnotherObj is not a sentient object){
					if sentientObj falls inside the sentient area of any other game object(AnotherObj){
						wake up AnotherObj, add it to the back of the sentient object list.
						AnotherObj.OnEnterSentientArea();
					}
				}
				if AnotherObj falls inside the sentient area of sentientObj{
					sentientObj.m_nSentientObjCount++;
					if AnotherObj falls inside the perceptive area of sentientObj{
						sentientObj.m_PerceivedList.push_back(AnotherObj.name());
					}
				}
			}
		}
		if(sentientObj.m_nSentientObjCount==0){
			sentientObj.OnLeaveSentientArea();
			remove sentientObj from the sentient object list.
		}else{
			// call game AI now or in the next pass, we advise a second pass, 
			// so it gives equal chance to each character
			// sentientObj.OnPerceived();
		}
	}

	// Pass 2 (It can be combined with Pass 1): call game AI of sentient objects
	for each sentient game objects (pObj) in the scene{
		pObj->m_nSimTag = SIM_TAG_START;
	}
	for each sentient game objects (pObj) in the scene{
			// generate way points
			pObj->PathFinding(dTimeDelta);
			// move the biped according to way point commands
			pObj->AnimateBiped(dTimeDelta);
			// apply AI controllers
			if(pObj->GetAIModule())
				pObj->GetAIModule()->FrameMove((float)dTimeDelta);
			if(!pObj->m_PerceivedList.empty())
				pObj->On_Perception();
			// call the frame move script if any.
			pObj->On_FrameMove();
		}
	}

	// Animate all global particles
	for each particle system (pObj) in the scene{
		if( !pObj->IsExploded() )
			pObj->Animate(dTimeDelta);
	}
}
*/
//-----------------------------------------------------------------------
#include "ParaEngine.h"
#include "ParaWorldAsset.h"
#include "SceneObject.h"
#include "MissileObject.h"
#include "BipedObject.h"
#include "TerrainTile.h"
#include "AIBase.h"
#include "FrameRateController.h"
#include "PhysicsWorld.h"
#include "TerrainTile.h"
#include "AutoCamera.h"
#include "MeshPhysicsObject.h"
#include "TerrainTileRoot.h"
#include "SunLight.h"
#include "EnvironmentSim.h"

#include "memdebug.h"

using namespace ParaEngine;

namespace ParaEngine
{
	extern CFrameRateController g_gameTime;
}

/**
@def The simulation time delta advance must be smaller than this value.
currently it is 1 second lag. Greater than this value, the environment
simulator will only advance MAX_SIM_STEP
*/
#define MAX_SIM_LAG		1.0f

/**
@def this defines the worst case frame rate of the simulator
normally 15FPS works fine.
*/
#define MAX_SIM_STEP		(1.f/15.f)

/** for simulation tag */
#define SIM_TAG_START		0
#define SIM_TAG_BASIC		1
#define SIM_TAG_FINISHED	0xff


CEnvironmentSim::CEnvironmentSim(void)
{
	
}

CEnvironmentSim::~CEnvironmentSim(void)
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

void CEnvironmentSim::Release()
{
	delete this;
}

// 2007.8.27: logic changes: the current player is always sentient to all other objects and all other objects are sentient to the current player. 
void CEnvironmentSim::UpdateGameObjects(double dTimeDelta)
{
	PERF1("EnvSim::UpdateGameObjects");
	CSceneObject* pScene = CGlobals::GetScene();

	CTerrainTile* duplicateTiles[9];

	for (int i = 0;i < (int)pScene->GetSentientObjects().size(); )
	{
		// for each sentient game objects (sentientObj) in the scene
		IGameObject* sentientObj = pScene->GetSentientObjects()[i];
		if (!sentientObj)
		{
			pScene->GetSentientObjects().erase(i);
			OUTPUT_LOG("warn: invalid weak ref found in pScene->GetSentientObjects()\n");
			continue;
		}

		sentientObj->SetSentientObjCount(0);
		sentientObj->GetPerceiveList().clear();
		
		Vector3 vPos = sentientObj->GetPosition();
		Vector2 vec2Pos(vPos.x, vPos.z);
		// update itself in the terrain tile according to its current position:
		CTerrainTile * pTile = pScene->GetRootTile()->GetTileByPoint(vPos.x, vPos.z);
		if(pTile != NULL)
		{
			sentientObj->SetTileContainer(pTile);

			for(int k=0;k<9;++k)
				duplicateTiles[k]=NULL;

			//for each valid terrain tiles(9 or more) in the neighborhood of sentientObj
			for (int i=0; i<9;++i)
			{
				CTerrainTile* pTileAdjacent = pScene->GetRootTile()->GetAdjacentTile(vPos, (CTerrainTileRoot::DIRECTION)i);

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
							VisitorList_type::iterator itCurCP, itEndCP = pTileAdjacent->m_listVisitors.end();

							for( itCurCP = pTileAdjacent->m_listVisitors.begin(); itCurCP != itEndCP;)
							{
								IGameObject* AnotherObj = (*itCurCP);
								if (AnotherObj != nullptr)
								{
									if (AnotherObj != sentientObj)
									{
										Vector3 vPosAnother = AnotherObj->GetPosition();

										Vector2 vec2PosAnother(vPosAnother.x, vPosAnother.z);
										float fDistSq = (vec2PosAnother - vec2Pos).squaredLength();

										if (!AnotherObj->IsSentient())
										{
											//if sentientObj falls inside the sentient area of AnotherObj
											if ((AnotherObj->IsSentientWith(sentientObj)) && fDistSq <= AnotherObj->GetSentientRadius()*AnotherObj->GetSentientRadius()){
												pScene->AddSentientObject(AnotherObj);
											}
										}

										//if AnotherObj falls inside the sentient area of sentientObj
										if ((sentientObj->IsSentientWith(AnotherObj)) && fDistSq <= sentientObj->GetSentientRadius()*sentientObj->GetSentientRadius())
										{
											sentientObj->SetSentientObjCount(sentientObj->GetSentientObjCount() + 1);

											// if AnotherObj falls inside the perceptive area of sentientObj
											if (fDistSq <= sentientObj->GetPerceptiveRadius()*sentientObj->GetPerceptiveRadius())
											{
												sentientObj->GetPerceiveList().push_back(AnotherObj->GetIdentifier());
											}
										}
									}
									++itCurCP;
								}
								else
									itCurCP = pTileAdjacent->m_listVisitors.erase(itCurCP);
							}
						}
						// skip solid object on the root tile, since they are all global objects, not static objects. 
						if(pTileAdjacent!=pScene->GetRootTile())
						{
							// check collision with other static solid objects.

							for (auto pObject : pTileAdjacent->m_listSolidObj)
							{
								IGameObject* AnotherObj = pObject->QueryIGameObject();
								if(AnotherObj!=NULL && AnotherObj!=sentientObj && sentientObj->IsSentientWith(AnotherObj) )
								{
									Vector3 vPosAnother = AnotherObj->GetPosition();

									Vector2 vec2PosAnother(vPosAnother.x, vPosAnother.z);
									float fDistSq = (vec2PosAnother-vec2Pos).squaredLength();

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
			++i;

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
			pScene->GetSentientObjects().erase(i);
		}
	}
}

void CEnvironmentSim::Simulate(double dTimeDelta)
{
	if(dTimeDelta<=0)
		return;
	CSceneObject* pScene = CGlobals::GetScene();
	if((pScene == NULL) || pScene->IsScenePaused() || (!(pScene->IsSceneEnabled())) )
		return;

	// physics engine frame move. 
	CGlobals::GetPhysicsWorld()->StepSimulation(dTimeDelta);

	/** advance the game time */
	g_gameTime.FrameMoveDelta((float)dTimeDelta);

	// advance time of day
	pScene->GetSunLight().AdvanceTimeOfDay((float)dTimeDelta);

	/** Check load physics around the current player and the camera position
	* this is very game specific. It only ensures that physics object around the current player and camera is loaded.
	*/
	CBipedObject* pPlayer = pScene->GetCurrentPlayer();
	int nPointCount = 0;
	CShapeSphere points[2];
	if(pPlayer)
	{
		points[nPointCount].Set(pPlayer->GetPosition(), pPlayer->GetPhysicsRadius()*2.f);
		nPointCount++;
	}
	if(pScene->GetCurrentCamera())
	{
		points[nPointCount].Set(pScene->GetCurrentCamera()->GetEyePosition(), pScene->GetCurrentCamera()->GetNearPlane()*2);
		nPointCount++;
	}
	CheckLoadPhysics(points, nPointCount);

	UpdateGameObjects(dTimeDelta);

	{
		PERF1("EnvSim::Frame Move Sentient Objects");

		for (auto itCur = pScene->GetSentientObjects().begin(); itCur != pScene->GetSentientObjects().end();)
		{
			IGameObject* pObj = (*itCur);
			if (!pObj)
			{
				itCur = pScene->GetSentientObjects().erase(itCur);
				OUTPUT_LOG("warn: invalid weak ref found in pScene->GetSentientObjects()\n");
				continue;
			}
			else
			{
				itCur++;
			}
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
								Vector3 vPos = pRefObj->GetPosition();
								CTerrainTile * pTile = pScene->GetRootTile()->GetTileByPoint(vPos.x, vPos.z);
								if(pTile != NULL)
								{
									pRefObj->SetTileContainer(pTile);
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
		for (auto pMissile : pScene->GetMissiles())
		{
			if( !pMissile->IsExploded() )
			{
				pMissile->Animate(dTimeDelta);
			}
		}
	}
}

/** the scene must be loaded before calling this function*/
void CEnvironmentSim::CheckLoadPhysics(CShapeSphere* points, int nPointCount)
{
	if(nPointCount<=0)
		return;
	queue_CTerrainTilePtr_Type queueTiles;
	CTerrainTile* pTile = CGlobals::GetScene()->GetRootTile();
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
/** since 2009.8, all physics objects are attached to the leaf node, so we need to cull tile using a larger radius rather than the input fRadius. */
#define PhysicsMinDistance		50.f
				/// rough culling algorithm using the quad tree terrain tiles
				/// test against a sphere round the eye
				for (int j=0;j<nPointCount;++j)
				{
					if(pTile->m_subtiles[i]->TestCollisionSphere(&(points[j].GetCenter()), PhysicsMinDistance))
					{
						queueTiles.push( pTile->m_subtiles[i] );
						break;
					}		
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
			
			{
				// For each mesh physics object in the tile, load its physics.
				for (auto pObj : pTile->m_listFreespace)
				{
					if (pObj && pObj->CanHasPhysics())
					{
						IViewClippingObject* pViewClippingObject = pObj->GetViewClippingObject();
						for(int j=0;j<nPointCount; ++j)
						{
							if(pViewClippingObject->TestCollisionSphere(&(points[j].GetCenter()), points[j].GetRadius(), 2))
							{
								pObj->LoadPhysics();
								break;
							}
						}
					}
				}

				// for visiting bipeds
				for (auto pObj : pTile->m_listVisitors)
				{
					if (pObj && pObj->CanHasPhysics())
					{
						IViewClippingObject* pViewClippingObject = pObj->GetViewClippingObject();
						for (int j = 0; j < nPointCount; ++j)
						{
							if (pViewClippingObject->TestCollisionSphere(&(points[j].GetCenter()), points[j].GetRadius(), 2))
							{
								pObj->LoadPhysics();
								break;
							}
						}
					}
				}
			}
		}
	}//while(!queueTiles.empty())
}

//-----------------------------------------------------------------
// name: Animate
/**
// desc: Environment collision detection and response
// call this function at every frame move. It's up to this simulator to decide
// how much computation is done each call. In other words, some computational
// extensive checking is done every few seconds and in multiple calls. In other
// words, the function will guarantee that the amount of computation for each call
// is balanced. Collision pairs are saved in m_pCollisionPairs, which may be used
// for AI simulation or game interface
// 
[RIGHT: persistent collision] the ideal collision is like this: Once in collision 
always in collision, unless the user or sim has moved object away. Collision in this 
sense is better interpreted as touched against one other, like player and NPC.

[WRONG: collision only occurs once, then disapears]Collision is handled like this:
we will implement one step look ahead method. Suppose initially, a player is not in 
collision with any object. By taking commands from the user, we will need to move 
the player to a new position. So we save the player's old state, and move the player
one frame ahead. Then we will generate collision .
*/
//-----------------------------------------------------------------
void CEnvironmentSim::Animate( double dTimeDelta )
{
	if(dTimeDelta  > MAX_SIM_LAG)
	{
		/// time lag more than MAX_SIM_LAG(1) second will be detected by the environment simulator
		//OUTPUT_DEBUG("time lag more than 1 second detected by the environment simulator\n");
		dTimeDelta  = MAX_SIM_STEP;
	}

	CSceneObject* pScene = CGlobals::GetScene();

	/**
	// the simulation is divided in to sub steps. 
	int nStepCount = (int)ceil(dTimeDelta/MAX_SIM_STEP);
	double fSubStepDeltaTime = dTimeDelta;
	if(nStepCount<=1)
	nStepCount = 1;
	else
	fSubStepDeltaTime /= nStepCount;
	// global biped simulation in multiple steps
	for (int i =0; i<nStepCount;i++)
	{
	//CGlobals::GetPhysicsWorld()->StartPhysics(fSubStepDeltaTime);
	// CGlobals::GetPhysicsWorld()->GetPhysicsResults();
	Simulate(fSubStepDeltaTime);
	}
	*/
	Simulate(dTimeDelta);

	/// TODO: simulate other physical object here.
}

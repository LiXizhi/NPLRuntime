//-----------------------------------------------------------------------------
// Class:	CSceneObject
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2006.1.1
// desc: ray picking implementation
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "BipedObject.h"
#include "AutoCamera.h"
#include "SelectionManager.h"
#include "terrain/GlobalTerrain.h"
#include "PhysicsWorld.h"
#include "MeshPhysicsObject.h"
#include "MeshObject.h"
#include "RayCollider.h"

#include "2dengine/GUIRoot.h"
#include "ViewportManager.h"
#include "SceneObjectPicking.h"
#include "TerrainTileRoot.h"
#ifdef EMSCRIPTEN_SINGLE_THREAD
// #define auto_ptr unique_ptr
#include "AutoPtr.h"
#endif
#include <luabind/object.hpp>

#include "memdebug.h"

namespace ParaEngine
{
	bool PickingAll(CBaseObject* obj);
	bool PickingFilterMesh(CBaseObject* obj);
	bool PickingFilterBiped(CBaseObject* obj);
	bool PickingFilterNotPlayer(CBaseObject* obj);
	bool PickingFilterLights(CBaseObject* obj);
	bool PickingFilterAnyObject(CBaseObject* obj);
	bool PickingFilterGlobal(CBaseObject* obj);
	bool PickingActionMesh(CBaseObject* obj);
	bool PickingByObjectTypes(CBaseObject* obj);
	bool PickingByPhysicsGroup(CBaseObject* obj);
	
	OBJECT_FILTER_CALLBACK g_fncPickingMesh = PickingFilterMesh;
	OBJECT_FILTER_CALLBACK g_fncPickingNPCMesh = PickingFilterMesh; // obsoleted.
	OBJECT_FILTER_CALLBACK g_fncPickingBiped = PickingFilterBiped;
	OBJECT_FILTER_CALLBACK g_fncPickingNotPlayer = PickingFilterNotPlayer;
	OBJECT_FILTER_CALLBACK g_fncPickingAll = PickingAll;
	OBJECT_FILTER_CALLBACK g_fncPickingLight = PickingFilterLights;
	OBJECT_FILTER_CALLBACK g_fncPickingGlobal = PickingFilterGlobal;
	OBJECT_FILTER_CALLBACK g_fncPickingAnyObject = PickingFilterAnyObject;
	OBJECT_FILTER_CALLBACK g_fncPickingActionMesh = PickingActionMesh;
	// call SetPickingFilter(CBaseObject::MeshObject | CBaseObject::MeshPhysicsObject)
	// to set the current picking function.
	OBJECT_FILTER_CALLBACK g_fncPickingByObjectTypes = PickingByObjectTypes; 
	OBJECT_FILTER_CALLBACK g_fncPickingByPhysicsGroup = PickingByPhysicsGroup; 
	

	bool PickingAll(CBaseObject* obj){return true;}

	bool PickingActionMesh(CBaseObject* obj)
	{
		int nType = (int)obj->GetType();
		bool Result = false;
		if(nType == CBaseObject::MeshPhysicsObject)
		{
			CMeshObject* pMesh = ((CMeshPhysicsObject*)obj)->GetMeshObject();
			Result = (pMesh && pMesh->GetXRefScriptCount()>0);
		}
		else if(nType == CBaseObject::MeshObject)
		{
			CMeshObject* pMesh = (CMeshObject*)obj;
			Result = (pMesh && pMesh->GetXRefScriptCount()>0);
		}
		return Result;
	}

	bool PickingFilterLights(CBaseObject* obj)
	{
		return ( obj->GetType()== CBaseObject::LightObject);
	}
	
	bool PickingFilterMesh(CBaseObject* obj)
	{
		const static DWORD filters = CBaseObject::MeshObject | CBaseObject::MeshPhysicsObject;
		return ( (filters & obj->GetType()) != 0 );
	}
	bool PickingFilterBiped(CBaseObject* obj)
	{
		const static DWORD filters = CBaseObject::BipedObject | CBaseObject::RPGBiped;
		return ( (filters & obj->GetType()) != 0 );
	}

	bool PickingFilterAnyObject(CBaseObject* obj)
	{
		const static DWORD filters = CBaseObject::MeshObject | CBaseObject::MeshPhysicsObject | CBaseObject::OPCBiped | CBaseObject::RPGBiped | CBaseObject::BipedObject;
		return ( (filters & obj->GetType()) != 0 );
	}

	bool PickingFilterGlobal(CBaseObject* obj)
	{
		return ( obj->IsGlobal() );
	}

	bool PickingFilterNotPlayer(CBaseObject* obj)
	{
		const static DWORD filters = CBaseObject::MeshObject | CBaseObject::MeshPhysicsObject | CBaseObject::OPCBiped | CBaseObject::RPGBiped | CBaseObject::BipedObject;
		return ( (filters & obj->GetType()) != 0 ) && (CGlobals::GetScene()->GetCurrentPlayer() != obj);
	}

	static DWORD g_pickingObjectFilter = 0;
	bool PickingByObjectTypes(CBaseObject* obj)
	{
		return ( (g_pickingObjectFilter & obj->GetType()) != 0 );
	}
	/** Set the picking filter to be used with PickingByObjectTypes function filter
	* @param filter: bitwise fields of _SceneObjectType, such as CBaseObject::ZoneNode | CBaseObject::PortalNode
	*/
	void SetPickingFilter(DWORD filter)
	{
		g_pickingObjectFilter = filter;
	}

	static DWORD g_pickingPhysicsFilter = 0;
	bool PickingByPhysicsGroup(CBaseObject* obj)
	{
		return ( (g_pickingPhysicsFilter & (0x1<<(obj->GetPhysicsGroup()))) != 0 );
	}
	/** Set the picking filter to be used with PickingByPhysicsGroup function filter
	* @param filter: bitwise fields of group 0-31. so that 0xffffffff means matches any physics group. 
	*/
	void SetPickingPhysicsFilter(DWORD filter)
	{
		g_pickingPhysicsFilter = filter;
	}

	

	//-----------------------------------------------------------------------------
	// Name: RayRectIntersect()
	/// Desc: Check the ray intersection with a rect in the y=0 plane, specified by the 
	///       rect origin and width 2*fX, depth 2*fZ
	//-----------------------------------------------------------------------------
	bool RayRectIntersect(Vector3& vPickRayOrig, Vector3& vPickRayDir,
		Vector3& vPos, FLOAT fX, FLOAT fZ, Vector3* vIntersectPos)
	{
		/// the rect plane function is Ax + By + Cz + D = 0, where A=0,B=1,C=0,D=0
		/// check if it's parallel by checking A Dx + B Dy + C Dz is near zero
		if(fabs(vPickRayDir.y) < 0.0001f)
		{
			return false;
		}
		FLOAT fT = -vPickRayOrig.y / vPickRayDir.y;
		/// give the intersection point with the plane
		(*vIntersectPos) = vPickRayOrig + Vector3(fT*vPickRayDir.x, -vPickRayOrig.y, fT*vPickRayDir.z );
		/// check if the intersection point is in the rect
		if(vIntersectPos->x<=(vPos.x+fX) && vIntersectPos->x>=(vPos.x-fX) &&
			vIntersectPos->z<=(vPos.z+fZ) && vIntersectPos->z>=(vPos.z-fZ))
			return true;
		else
			return false;
	}

	//-----------------------------------------------------------------------------
	// Name: RaySphereIntersect()
	/// Desc: Check the ray intersection with a Sphere specified by its
	///       origin and radius fR
	//-----------------------------------------------------------------------------
	bool RaySphereIntersect(Vector3& vPickRayOrig, Vector3& vPickRayDir,
		Vector3& vPos, FLOAT fR, Vector3* vIntersectPos)
	{
		return true;	
	}


	bool Pick(int mouseX, int mouseY, float &pickedX, float &pickedY, float &pickedZ)
	{
#ifdef DIRECTX_PICK

		float depth[1];

		D3DMATRIX modelm;
		D3DMATRIX projm;
		D3DVIEWPORT9  view;
		Settings::GetInstance()->GettD3DDevice()->GetTransform(D3DTS_PROJECTION, &projm);
		Settings::GetInstance()->GettD3DDevice()->GetTransform(D3DTS_VIEW, &modelm);
		Settings::GetInstance()->GettD3DDevice()->GetViewport(&view);


		LPDIRECT3DSURFACE9 pZBuffer;
		Settings::GetInstance()->GettD3DDevice()->GetDepthStencilSurface( &pZBuffer );
		D3DLOCKED_RECT pLockedRect;

		RECT rect;
		int nRaidus = 1;
		rect.left = mouseX-nRaidus;
		rect.top = mouseY-nRaidus;
		rect.right = mouseX+nRaidus;
		rect.bottom = mouseY+nRaidus;
		pZBuffer->LockRect(&pLockedRect, &rect, D3DLOCK_READONLY|D3DLOCK_NOOVERWRITE);
		//glReadPixels(mouseX, mouseY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, depth);
		pZBuffer->UnlockRect();

		//gluUnProject(mouseX, mouseY, depth[0], modelm, projm, view, &pos[0], &pos[1], &pos[2]);

		pickedX = (float)pos[0];
		pickedY = (float)pos[1];
		pickedZ = (float)pos[2];

#endif
		return true;
	}

	float CSceneObject::PickClosest(int nScreenX, int nScreenY, CBaseObject** pPickedObject, Vector3* vIntersectPos, Vector3* vImpactNormal, bool bTestObject, float fMaxDistance, DWORD dwGroupMask, bool bEnableGlobalTerrain)
	{
		if (!CGlobals::GetViewportManager())
		{
			return -1.f;
		}
		Matrix4 matWorld;
		matWorld = Matrix4::IDENTITY;

		POINT ptCursor;
		float fScaleX = 1.f, fScaleY = 1.f; 
		CGlobals::GetGUI()->GetUIScale(&fScaleX, &fScaleY);
		ptCursor.x = (fScaleX == 1.f) ? nScreenX : (int)(nScreenX*fScaleX);
		ptCursor.y = (fScaleY == 1.f) ? nScreenY : (int)(nScreenY*fScaleY);

		Vector3 vPickRayOrig, vPickRayDir;

		int x = ptCursor.x;
		int y = ptCursor.y;
		int nWidth, nHeight;
		CGlobals::GetViewportManager()->GetPointOnViewport(x, y, &nWidth, &nHeight);
		ptCursor.x = x;
		ptCursor.y = y;
		GetCurrentCamera()->GetMouseRay(vPickRayOrig, vPickRayDir, ptCursor,nWidth, nHeight, &matWorld);

		return PickClosest(vPickRayOrig+GetRenderOrigin(), vPickRayDir, pPickedObject, vIntersectPos, vImpactNormal, bTestObject, fMaxDistance, dwGroupMask, bEnableGlobalTerrain);
	}

	float CSceneObject::PickClosest(const Vector3& vPickRayOrig, const Vector3& vPickRayDir, CBaseObject** pPickedObject, Vector3* vIntersectPos, Vector3* vImpactNormal, bool bTestObject, float fMaxDistance, DWORD dwGroupMask, bool bEnableGlobalTerrain)
	{
		if(fMaxDistance<=0)
		{
			fMaxDistance = m_sceneState->fViewCullingRadius;
		}
		bool bPicked = false;
		// test the global terrain, fDist may be negative
		float fDist = bEnableGlobalTerrain ? m_globalTerrain->IntersectRay(vPickRayOrig.x, vPickRayOrig.y, vPickRayOrig.z, vPickRayDir.x, vPickRayDir.y, vPickRayDir.z, vIntersectPos->x, vIntersectPos->y, vIntersectPos->z, fMaxDistance) : -1.f;

		// test all geometry in the physical mesh
		if(dwGroupMask!=0)
		{
			RayCastHitResult hit;
			//OUTPUT_LOG("scene pick: raycastClosestShape %f %f %f", vPickRayDir.x, vPickRayDir.y, vPickRayDir.z);
			IParaPhysicsActor* closestShape = CGlobals::GetPhysicsWorld()->GetPhysicsInterface()->RaycastClosestShape(
				(const PARAVECTOR3&)(vPickRayOrig), (const PARAVECTOR3&)(vPickRayDir), 0, hit, (int16)dwGroupMask, fMaxDistance);
			//OUTPUT_LOG(" end\n");
			if (closestShape)
			{
				if(fDist > hit.m_fDistance || fDist<0)
				{
					// the ray has hit physical world geometry before it hit the terrain.
					vIntersectPos->x = hit.m_vHitPointWorld.x;
					vIntersectPos->y = hit.m_vHitPointWorld.y;
					vIntersectPos->z = hit.m_vHitPointWorld.z;
					fDist = hit.m_fDistance;
					if(vImpactNormal)
					{
						vImpactNormal->x = hit.m_vHitNormalWorld.x;
						vImpactNormal->y = hit.m_vHitNormalWorld.y;
						vImpactNormal->z = hit.m_vHitNormalWorld.z;
					}
				}
				if(pPickedObject)
					*pPickedObject = (CBaseObject*)(closestShape->GetUserData());
			}
			if(fDist>0 && fDist<(fMaxDistance+100.f))
				bPicked = true;
		}
		
		if(bTestObject)
		{
			/** 
			* for simplicity, we will find the nearest biped object in 
			* sceneState.listPRBiped (a list of biped in the view), instead of transversing the scene graph to find them.
			*/
			CBipedObject* touchedObj = NULL;
			SceneState::List_PostRenderObject_Type::iterator itCurCP, itEndCP = m_sceneState->listPRBiped.end();
			bool bCollided = false;
			float fLastDist = 100000.f;
			for( itCurCP = m_sceneState->listPRBiped.begin(); itCurCP != itEndCP; ++ itCurCP)
			{
				CBipedObject* pObj = (CBipedObject*)((*itCurCP).m_pRenderObject);
				if(pObj!=0 && pObj->CanPick())
				{
					float fDistTmp;
					if( pObj->TestCollisionRay(vPickRayOrig,vPickRayDir, &fDistTmp) 
						// added LXZ 2009.11.10: this is a special requirement, we will only pick a biped object if the camera distance is larger than 1.f (2 times camera near plane). 
						&& fDistTmp > (1.f))
					{
						if(fDistTmp < fLastDist)
						{
							fLastDist = fDistTmp;
							touchedObj = pObj;
							bCollided = true;
						}
					}
				}
			}
			if(bCollided)
			{
				// we have found one.
				if (vIntersectPos)
					*vIntersectPos = touchedObj->GetPosition();
				*pPickedObject = touchedObj;
				bPicked = true;
				if(fLastDist < fDist)
				{
					fDist = fLastDist;
				}
			}
		}
		if(bPicked)
			return fDist;
		else 
			return -1.f;
	}

	bool CSceneObject::PickObject(int nScreenX, int nScreenY, CBaseObject** pTouchedObject, float fMaxDistance, OBJECT_FILTER_CALLBACK pFnctFilter)
	{
		Matrix4 matWorld;
		matWorld = Matrix4::IDENTITY;

		float fScaleX = 1.f, fScaleY = 1.f; 
		CGlobals::GetGUI()->GetUIScale(&fScaleX, &fScaleY);
		POINT ptCursor;
		ptCursor.x = (fScaleX == 1.f) ? nScreenX : (int)(nScreenX*fScaleX);
		ptCursor.y = (fScaleY == 1.f) ? nScreenY : (int)(nScreenY*fScaleY);
		
		int x = ptCursor.x;
		int y = ptCursor.y;
		int nWidth, nHeight;
		
		auto pManager = CGlobals::GetViewportManager();
		if(pManager)
			pManager->GetPointOnViewport(x, y, &nWidth, &nHeight);
		ptCursor.x = x;
		ptCursor.y = y;
		Vector3 vPickRayOrig, vPickRayDir;
		auto pCamera = GetCurrentCamera();
		if(pCamera)
			pCamera->GetMouseRay(vPickRayOrig, vPickRayDir, ptCursor,nWidth, nHeight, &matWorld);

		return PickObject(CShapeRay(vPickRayOrig+GetRenderOrigin(), vPickRayDir), pTouchedObject, fMaxDistance,pFnctFilter);
	}

	bool CSceneObject::PickObject(const CShapeRay& ray, CBaseObject** pTouchedObject, float fMaxDistance, OBJECT_FILTER_CALLBACK pFnctFilter)
	{
		if(pFnctFilter == 0)
			pFnctFilter = g_fncPickingAll;

		if(fMaxDistance<=0)
			fMaxDistance = m_sceneState->fViewCullingRadius;

		// the object picked.
		PickedObject lastObj; 

		static CMeshPhysicsObject g_dummy;
		if(pFnctFilter(&g_dummy))
		{
			/** pick object in the physics engine. This is accurate. */
			Vector3 vIntersectPt;
			CBaseObject* pObj=NULL;
			float fDist = PickClosest(ray.mOrig, ray.mDir, &pObj, &vIntersectPt,NULL, false, fMaxDistance, GetPhysicsGroupMask());
			if(fDist>0 && pObj!=0 && pFnctFilter(pObj) && pObj->CanPick())
			{
				lastObj.m_pObj = pObj;
				lastObj.m_fRayDist = fDist;
				IViewClippingObject* pViewClippingObject = pObj->GetViewClippingObject();
				// further testing using bounding box
				CShapeOBB obb;
				pViewClippingObject->GetOBB(&obb);
				lastObj.m_fMinObjExtent = Math::MinVec3(obb.mExtents);
			}
		}
		

		CRayCollider rayCollider;
		rayCollider.SetMaxDist(fMaxDistance);

		Vector3 vRenderOrigin=GetRenderOrigin();

		// the ray in the view space, shifted to the render origin.
		CShapeRay ray_view(ray.mOrig-vRenderOrigin, ray.mDir);

		queue_CBaseObjectPtr_Type queueNodes;
		queue_CTerrainTilePtr_Type queueTiles;
		CTerrainTile* pTile = m_pTileRoot.get();
		

		float fViewRadius = fMaxDistance/2;
		Vector3 vViewCenter = ray.mOrig + ray.mDir*fViewRadius;

		/// select object on root tile
		{
			// pTile is now the root tile
			if(pTile->m_listVisitors.empty() == false)
			{
				/// add all visitor biped objects, only the last time will have this list non-empty
				
				for( auto pObject : pTile->m_listVisitors)
				{
					if (pObject && pObject->CanPick())
						queueNodes.push(pObject);
				}
			}
		}

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
					if(pTile->m_subtiles[i]->TestCollisionSphere(& (vViewCenter), fViewRadius))
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
				/// We will not push objects in the current terrain tile to a queue object for further view clipping.

				pTile = queueTiles.front();
				queueTiles.pop();
				{
					/// add all solid objects to the queue for further testing
					for (auto& pObject : pTile->m_listSolidObj)
					{
						if (pObject->CanPick())
							queueNodes.push(pObject);
					}
					/// add all visitor biped objects to the queue.
					for (auto pObject : pTile->m_listVisitors)
					{
						if (pObject && (pObject)->CanPick())
							queueNodes.push(pObject);
					}
				}
				{
					/// add all free space objects to the queue
					for (auto& pObject : pTile->m_listFreespace)
					{
						if (pObject->CanPick())
							queueNodes.push(pObject);
					}
				}
			}

			/// For any potentially visible objects in the queue, 
			/// perform further object-level clipping test, and draw them if the test passes.
			while(!queueNodes.empty())
			{
				// pop up one object
				CBaseObject* pObj = queueNodes.front();
				queueNodes.pop();
				if( ( !pFnctFilter(pObj) )/* filter object*/ || 
					(pObj == lastObj.m_pObj) /* already selected */ || 
					( (pObj->GetType() == CBaseObject::MeshPhysicsObject) && (((CMeshPhysicsObject*)pObj)->GetStaticActorCount()>0) ) /* already picked by physics engine*/)
				{
					// we no longer need to pick any loaded physics object, since they are already picked
					continue;
				}
				IViewClippingObject* pViewClippingObject = pObj->GetViewClippingObject();
				
				Vector3 vObjCenter = pViewClippingObject->GetObjectCenter();
				vObjCenter -= vRenderOrigin;

				// float fR = pViewClippingObject->GetRadius();
				// rough testing using bounding sphere
				if(pViewClippingObject->TestCollisionSphere(& (vViewCenter), fViewRadius,1) )
				{
					// further testing using bounding box
					CShapeOBB obb;
					pViewClippingObject->GetOBB(&obb);
					obb.mCenter-=vRenderOrigin;

					float fDist;
					if(rayCollider.Intersect(ray_view, obb, &fDist) && fDist<=fMaxDistance &&
						// added LXZ 2009.11.10: this is a special requirement, we will only pick a biped object if the camera distance is larger than 1.f (2 times camera near plane). 
						!(pObj->GetMyType() >=_Biped && fDist<1.f))
					{
						// add to collision list
						float fMinExtent = Math::MinVec3(obb.mExtents);
						bool bUseCurrent = false;

						if(lastObj.IsValid()==false)
							bUseCurrent = true;
						else
						{
							bool bDetermined = false;
							
							if(lastObj.m_pObj->GetMyType()>=_Biped)
							{
								if(pObj->GetMyType()<_Biped)
								{
									// if the last object is a character and the current one is a non-physical mesh object,
									// we will give character a little priority over smaller mesh object by 
									//  (1) enlarging the character by its radius
									//  (2) always select the character if the camera to character distance is less than 2 times the character radius
									float fRadius = lastObj.m_pObj->GetRadius();
									if(fRadius>fabs(lastObj.m_fRayDist -fDist) 
										|| lastObj.m_fRayDist<fRadius*2)
									{
										// use last character
										bDetermined = true;
									}
								}
								else
								{
									// if the last object and current candidates are both biped. 
									// and that the their ray distance only differs by the sum of the bound radius of the two objects, we will pick according to their render importance.  
									if( (lastObj.m_pObj->GetRenderImportance()!=pObj->GetRenderImportance()) 
										&& fabs(lastObj.m_fRayDist - fDist) < (lastObj.m_pObj->GetRadius()+pObj->GetRadius()) )
									{
										if(lastObj.m_pObj->GetRenderImportance()>pObj->GetRenderImportance())
										{
											// use last character
											bDetermined = true;
										}
										else
										{
											bDetermined = true;
											bUseCurrent = true;
										}
									}
								}
							}
							if(!bDetermined)
							{
								if(lastObj.m_fRayDist<=fDist)
								{
									if( (lastObj.m_pObj->GetType() != CBaseObject::MeshPhysicsObject) || (((CMeshPhysicsObject*)lastObj.m_pObj)->GetStaticActorCount()==0) )
									{
										// if the last object's intersection point is in front of the current object's  
										if(lastObj.m_fMinObjExtent > fMinExtent && 
											(fDist-lastObj.m_fRayDist) <= lastObj.m_fMinObjExtent*2)
											// if the last object is NOT entirely in front of the current object, this is just an approximate
											// we will adopt the current one
											bUseCurrent = true;
									}
								}
								else
								{
									// if the last object's intersection point is behind the current object's  
									if(lastObj.m_fMinObjExtent < fMinExtent && 
										(lastObj.m_fRayDist-fDist) <= fMinExtent*2)
									{
										// if the last object is a smaller object
										// we will adopt the last object
									}
									else
										bUseCurrent = true;
								}
							}
						}
						if(bUseCurrent)
						{
							lastObj.m_pObj = pObj;
							lastObj.m_fMinObjExtent = fMinExtent;
							lastObj.m_fRayDist = fDist;
						}
					}
				}
			}
		}
		CGlobals::GetReport()->SetValue("picking", (double)lastObj.m_fRayDist);

		if(pTouchedObject!=0 )
		{
			if(lastObj.IsValid())
			{
				*pTouchedObject = lastObj.m_pObj;
			}
			else
				*pTouchedObject = NULL;
		}
		
		return false;
	}

	int CSceneObject::SelectObject(int nGroupIndex, const CShapeOBB& box, OBJECT_FILTER_CALLBACK pFnctFilter)
	{
		int nCount = 0;

		Vector3 vRenderOrigin=GetRenderOrigin();

		// the ray in the view space, shifted to the render origin.
		
		queue_CBaseObjectPtr_Type queueNodes;
		queue_CTerrainTilePtr_Type queueTiles;
		CTerrainTile* pTile = m_pTileRoot.get();

		Vector3 vViewCenter = box.mCenter;
		CShapeOBB viewOBB = box;
		viewOBB.mCenter -= vRenderOrigin;
		float fViewRadius = sqrt(box.mExtents.x*box.mExtents.x + box.mExtents.z*box.mExtents.z);
		
		/// select object on root tile
		{
			// pTile is now the root tile
			if(!pTile->m_listVisitors.empty())
			{
				/// add all visitor biped objects, only the last time will have this list non-empty
				for (auto pObject : pTile->m_listVisitors)
				{
					if (pObject)
						queueNodes.push(pObject);
				}
			}
		}

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
					if(pTile->m_subtiles[i]->TestCollisionSphere(& (vViewCenter), fViewRadius))
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
				/// We will not push objects in the current terrain tile to a queue object for further view clipping.

				pTile = queueTiles.front();
				queueTiles.pop();
				{
					/// add all solid objects to the queue for further testing
					for (auto& pObject : pTile->m_listSolidObj)
						queueNodes.push(pObject);
					/// add all visitor biped objects to the queue.
					for (auto pObject : pTile->m_listVisitors)
					{
						if (pObject)
							queueNodes.push(pObject);
					}
				}
				{
					/// add all free space objects to the queue
					for (auto& pObject : pTile->m_listFreespace)
						queueNodes.push(pObject);
				}
			}

			/// For any potentially visible objects in the queue, 
			/// perform further object-level clipping test, and draw them if the test passes.
			while(!queueNodes.empty())
			{
				// pop up one object
				CBaseObject* pObj = queueNodes.front();
				queueNodes.pop();
				if( ! pObj->CanPick() || ( !(pFnctFilter(pObj)) )/* filter object*/ )
				{
					// filter test failed, skip this object
					continue;
				}
				IViewClippingObject* pViewClippingObject = pObj->GetViewClippingObject();

				Vector3 vObjCenter = pViewClippingObject->GetObjectCenter();
				vObjCenter -= vRenderOrigin;

				// rough testing using bounding sphere
				if(pViewClippingObject->TestCollisionSphere(& (vViewCenter), fViewRadius,1) )
				{
					// further testing using bounding box
					CShapeOBB obb;
					pViewClippingObject->GetOBB(&obb);
					obb.mCenter-=vRenderOrigin;

					if( ! obb.IsInside(viewOBB) )
					{
						// add to selection group list
						nCount++;
						CGlobals::GetSelectionManager()->AddObject(pObj, nGroupIndex);
					}
				}
			}
		}
		return nCount;
	}

	int CSceneObject::SelectObject(int nGroupIndex, const CShapeSphere& circle, OBJECT_FILTER_CALLBACK pFnctFilter)
	{
		int nCount = 0;

		Vector3 vRenderOrigin=GetRenderOrigin();

		// the ray in the view space, shifted to the render origin.

		queue_CBaseObjectPtr_Type queueNodes;
		queue_CTerrainTilePtr_Type queueTiles;
		CTerrainTile* pTile = m_pTileRoot.get();

		float fViewRadius = circle.mRadius;
		Vector3 vViewCenter = circle.mCenter;

		/// select object on root tile
		{
			// pTile is now the root tile
			if(!pTile->m_listVisitors.empty())
			{
				/// add all visitor biped objects, only the last time will have this list non-empty
				for (auto pObject : pTile->m_listVisitors)
				{
					if (pObject)
						queueNodes.push(pObject);
				}
			}
		}

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
					if(pTile->m_subtiles[i]->TestCollisionSphere(& (vViewCenter), fViewRadius))
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
				/// We will not push objects in the current terrain tile to a queue object for further view clipping.

				pTile = queueTiles.front();
				queueTiles.pop();
				{
					/// add all solid objects to the queue for further testing
					for (auto& pObject : pTile->m_listSolidObj)
						queueNodes.push(pObject);
					/// add all visitor biped objects to the queue.
					for (auto pObject : pTile->m_listVisitors)
					{
						if (pObject)
							queueNodes.push(pObject);
					}
				}
				{
					/// add all free space objects to the queue
					for (auto& pObject : pTile->m_listFreespace)
						queueNodes.push(pObject);
				}
			}

			/// For any potentially visible objects in the queue, 
			/// perform further object-level clipping test, and draw them if the test passes.
			while(!queueNodes.empty())
			{
				// pop up one object
				CBaseObject* pObj = queueNodes.front();
				queueNodes.pop();
				if( ! pObj->CanPick() || !pFnctFilter(pObj)/* filter object*/ )
				{
					// filter test failed, skip this object
					continue;
				}
				IViewClippingObject* pViewClippingObject = pObj->GetViewClippingObject();

				Vector3 vObjCenter = pViewClippingObject->GetObjectCenter();
				vObjCenter -= vRenderOrigin;

				// rough testing using bounding sphere
				if(pViewClippingObject->TestCollisionSphere(& (vViewCenter), fViewRadius,1) )
				{
					// add to selection group list
					nCount++;
					CGlobals::GetSelectionManager()->AddObject(pObj, nGroupIndex);
				}
			}
		}
		return nCount;
	}

}//namespace ParaEngine

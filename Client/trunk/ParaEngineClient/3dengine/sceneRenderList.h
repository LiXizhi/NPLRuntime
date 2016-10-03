#pragma once

/** @def if there are over this number of mesh objects in a single frame, we will use instancing to draw them in a single draw call. */
#define AUTO_INSTANCING_THRESHOLD_NUM	10

namespace ParaEngine
{
	extern bool g_bShaderVersion3;

	/** performance hardware occlusion test for all objects in the renderlist and set the m_bOccluded property. 
	* @param renderlist: any render list that implement the m_bOccluded property (such as AlphaPostRenderObject). it will set the m_bOccluded property. 
	* @param nMinEnableCount: only bother to use hardware occlusion testing if the number of objects is larger than this value. defaults to 25. 
	* @param nPixelsCount: pixels under which object will be considered occluded. this must be 0.
	* @return: currently it just returns the renderlist size. 
	*/
	template <class T>
		int PerformOcclusionTest(T& renderlist, SceneState& sceneState, int nMinEnableCount = 25, int nPixelsCount = 0)
	{
#ifdef USE_DIRECTX_RENDERER
		LPDIRECT3DDEVICE9 pd3dDevice = sceneState.m_pd3dDevice;
		int nObjectNum = (int)renderlist.size();

		// only bother to use hardware occlusion testing if the number of small object is larger than 20.
		if(	nObjectNum>nMinEnableCount )
		{
			// PERF1("small_mesh");
			LatentOcclusionQueryBank* pQueries = NULL;
			if(CGlobals::GetScene()->IsOcclusionQueryEnabled() && ((pQueries = CGlobals::GetAssetManager()->GetOcclusionQueryBank())!=0))
			{
				//////////////////////////////////////////////////////////////////////////
				//
				// Issue multiple occlusion queries in a batch pass: 
				// 
				//////////////////////////////////////////////////////////////////////////
				/* parallelism is given by the following example. More parallelism can be achieve by using the occlusion result from the last render frame.
				* Suppose there are 34 objects, and 32 query objects. 
				* Q1-Q32 denotes the 32 parallel query objects. o1-o34 denotes 34 objects
				* Issue(Q1) for o1,..., Issue(Q32) for o32, retrieve(Q1) for o1, Issue(Q1) for o33, retrieve(Q2) for o2, Issue(Q2) for o34,
				* retrieve(ALL),
				* Render(o1-o34)
				*/

				//////////////////////////////////////////////////////////////////////////
				// issue multiple queries up to nQueryNum in parallel for all objects
				// set up query render state: 
				AlphaPostRenderObject * pObj = NULL;
				DWORD pixelsVisible = 0;
				CGlobals::GetEffectManager()->BeginEffect(TECH_OCCLUSION_TEST, &(sceneState.m_pCurrentEffect));
				typename T::const_iterator itCurCP, itEnd = renderlist.end();
				for(itCurCP = renderlist.begin(); itCurCP !=itEnd ; ++itCurCP)
				{
					// wait on the result of the first query, to ensure there is always an unused query in the bank.
					if(! pQueries->HasUnusedQuery())
					{
						pObj = NULL;
						if((pQueries->WaitForFirstResult(&pixelsVisible, (void**)(&pObj)) == S_OK) && pObj!=NULL)
						{
							if(pObj!=NULL)
							{
								pObj->m_nOccluded = (pixelsVisible == 0) ? 1: -1;
								pObj = NULL;
							}
						}
						else
							break;
					}
					IViewClippingObject* pViewClippingObject = (*itCurCP).m_pRenderObject->GetViewClippingObject();
					if(pQueries->BeginNewQuery((void*)(&(*itCurCP))) == S_OK)
					{
						pViewClippingObject->DrawOcclusionObject(&sceneState);
						if(pQueries->EndNewQuery() != S_OK)
						{
							break;
						}
					}
					else
						break;
				}
				// fetch the remaining result
				pObj = NULL;
				while(pQueries->WaitForFirstResult(&pixelsVisible, (void**)(&pObj)) == S_OK)
				{
					if(pObj!=0)
					{
						pObj->m_nOccluded = (pixelsVisible == 0) ? 1: -1;
						pObj = NULL;
					}
				}

				CGlobals::GetEffectManager()->EndEffect(); // end effects for all query objects
			}
			else
			{
				CGlobals::GetScene()->EnableOcclusionQuery(false);
			}
		}
		return nObjectNum;
#else
		return 0;
#endif
	}
	/** Render object in the given list, using batch rendering, but not using hardware occlusion testing.
	e.g. nObjCount += RenderList(sceneState.listPRSmallObject, sceneState);
	return the number of object rendered.*/
	template <class T>
	int RenderList(T& renderlist, SceneState& sceneState)
	{
		int nObjCount = 0;
		if(renderlist.empty())
			return nObjCount;
		// check whether use instancing
		CEffectFile* pEntity = CGlobals::GetEffectManager()->GetEffectByHandle(TECH_SIMPLE_MESH_NORMAL_INSTANCED); 
#ifdef USE_DIRECTX_RENDERER
		bool bUseInstancing = CGlobals::GetScene()->IsInstancingEnabled() && g_bShaderVersion3 && (pEntity!=0) && pEntity->IsValid();
#else
		bool bUseInstancing = false;
#endif

		RenderDevicePtr pd3dDevice = CGlobals::GetRenderDevice();

		//////////////////////////////////////////////////////////////////////////
		// render without hardware occlusion testing
		int nLastHandle = CGlobals::GetEffectManager()->GetCurrentTechHandle();
		AssetEntity * pLastEntity = NULL;
		typename T::const_iterator itCurCP, itEnd = renderlist.end();
		int nNum = 0;
		for(itCurCP = renderlist.begin(); itCurCP !=itEnd; ++itCurCP,++nNum)
		{
			if((*itCurCP).IsOccluded() || ! sceneState.CheckAddRenderCount((*itCurCP).m_pRenderObject->GetRenderImportance()))
				continue;

			int nCurTechHandle = (*itCurCP).m_pRenderObject->GetPrimaryTechniqueHandle();
			AssetEntity * pEntity = (*itCurCP).m_pRenderObject->GetPrimaryAsset();
#ifdef USE_DIRECTX_RENDERER
			if(bUseInstancing && (nNum==0 || nCurTechHandle!=nLastHandle || pLastEntity!=pEntity))
			{
				// check to see if we can use instancing to draw the mesh in one draw call. 
				// currently, instancing only works with static mesh with normals and without replaceable textures and without xrefs. 
				if(nCurTechHandle == TECH_SIMPLE_MESH_NORMAL)
				{
					if(pEntity!=0 && pEntity->GetType() == AssetEntity::mesh 
						&& ((MeshEntity*)pEntity)->GetMesh() && (((MeshEntity*)pEntity)->GetMesh()->GetNumReplaceableTextures()==0) && (((MeshEntity*)pEntity)->GetMesh()->GetXRefObjectsCount()==0))
					{
						int nInstanceCount=0;
						for(typename T::const_iterator itCur1 = itCurCP; itCur1!=itEnd; ++itCur1)
						{
							if(nCurTechHandle == (*itCur1).m_pRenderObject->GetPrimaryTechniqueHandle() 
								&& pEntity == (*itCur1).m_pRenderObject->GetPrimaryAsset())
							{
								++nInstanceCount;
							}
							else
								break;
						}
						if(nInstanceCount>AUTO_INSTANCING_THRESHOLD_NUM)
						{
							// use instancing.
							// Update the instance streams with the per instance data.
							MatrixVertex* pVertices = NULL;
							DynamicVertexBufferEntity* pBuffer = CGlobals::GetAssetManager()->GetDynamicBuffer(DVB_MATRIX);
							if(pBuffer!=0 && pBuffer->Lock(nInstanceCount, (VOID**)&pVertices) == nInstanceCount)
							{
								for(int i=0;i<nInstanceCount;i++)
								{	
									Matrix4 matrix;
									(*itCurCP).m_pRenderObject->GetRenderMatrix(matrix);
									// since no projection, encode the 4x3 part in a 3x4(shader will decode)
									pVertices[i].r1 = Vector4(matrix._11,matrix._21,matrix._31,matrix._41);
									pVertices[i].r2 = Vector4(matrix._12,matrix._22,matrix._32,matrix._42);
									pVertices[i].r3 = Vector4(matrix._13,matrix._23,matrix._33,matrix._43);
									if(i<(nInstanceCount-1))
										++itCurCP;
								}
								pBuffer->Unlock();

								if(CGlobals::GetEffectManager()->BeginEffect(TECH_SIMPLE_MESH_NORMAL_INSTANCED, &(sceneState.m_pCurrentEffect)))
								{
									// Set the first stream to be the indexed data and render N instances
									pd3dDevice->SetStreamSourceFreq(0,(D3DSTREAMSOURCE_INDEXEDDATA | nInstanceCount));
									// Set the second stream to be per instance data and iterate once per instance
									pd3dDevice->SetStreamSourceFreq(1,(D3DSTREAMSOURCE_INSTANCEDATA | 1));
									pd3dDevice->SetStreamSource(1, pBuffer->GetBuffer(), pBuffer->m_dwBase*pBuffer->m_nUnitSize, pBuffer->m_nUnitSize);

									sceneState.SetCameraToCurObjectDistance((*itCurCP).m_fObjectToCameraDistance);
									(*itCurCP).m_pRenderObject->Draw(&sceneState);
									nLastHandle = TECH_SIMPLE_MESH_NORMAL_INSTANCED;
									pLastEntity = pEntity;
									nObjCount += nInstanceCount;
								}
								continue;
							}
						}
					}
				}
			}
#endif
			pLastEntity = pEntity;
			if(nCurTechHandle!=nLastHandle)
			{
				// this is a new bunch of objects. 
				nLastHandle = nCurTechHandle;
				CGlobals::GetEffectManager()->BeginEffect(nCurTechHandle, &(sceneState.m_pCurrentEffect));
			}
			sceneState.SetCameraToCurObjectDistance((*itCurCP).m_fObjectToCameraDistance);
			(*itCurCP).m_pRenderObject->Draw(&sceneState);
			++nObjCount;
		}

		return nObjCount;
	}

	/** Render object, using batch rendering, but not using hardware occlusion testing.
	e.g. nObjCount += RenderList(sceneState.listPRSmallObject);
	return the number of object rendered.*/
	template <class T>
	int RenderShadowList(T& renderlist, SceneState& sceneState)
	{
		int nObjCount = 0;
		if(renderlist.empty())
			return nObjCount;
#ifdef USE_DIRECTX_RENDERER
		sceneState.m_nCurRenderGroup = RENDER_SHADOWMAP;

		LPDIRECT3DDEVICE9 pd3dDevice = CGlobals::GetRenderDevice();
		// check whether use instancing
		CEffectFile* pEntity = CGlobals::GetEffectManager()->GetEffectByHandle(TECH_SIMPLE_MESH_NORMAL_INSTANCED); 
		bool bUseInstancing = CGlobals::GetScene()->IsInstancingEnabled() && g_bShaderVersion3 && (pEntity!=0) && pEntity->IsValid();

		//////////////////////////////////////////////////////////////////////////
		// render without hardware occlusion testing
		int nLastHandle = CGlobals::GetEffectManager()->GetCurrentTechHandle();
		AssetEntity * pLastEntity = NULL;
		typename T::const_iterator itCurCP, itEnd = renderlist.end();
		int nNum = 0;
		CBipedObject* pPlayer = CGlobals::GetScene()->GetCurrentPlayer();
		bool bIsShowMainPlayer = CGlobals::GetScene()->IsShowMainPlayer();
		for(itCurCP = renderlist.begin(); itCurCP !=itEnd; ++itCurCP,++nNum)
		{
			if ((*itCurCP).m_pRenderObject == pPlayer && !bIsShowMainPlayer)
				continue;

			int nCurTechHandle = (*itCurCP).m_pRenderObject->GetPrimaryTechniqueHandle();
			AssetEntity * pEntity = (*itCurCP).m_pRenderObject->GetPrimaryAsset();

			if(bUseInstancing && (nNum==0 || nCurTechHandle!=nLastHandle || pLastEntity!=pEntity))
			{
				// check to see if we can use instancing to draw the mesh in one draw call. 
				// currently, instancing only works with static mesh with normals. 
				if(nCurTechHandle == TECH_SIMPLE_MESH_NORMAL)
				{
					if(pEntity->GetType() == AssetEntity::mesh)
					{
						int nInstanceCount=0;
						for(typename T::const_iterator itCur1 = itCurCP; itCur1!=itEnd; ++itCur1)
						{
							if(nCurTechHandle == (*itCur1).m_pRenderObject->GetPrimaryTechniqueHandle() 
								&& pEntity == (*itCur1).m_pRenderObject->GetPrimaryAsset())
							{
								++nInstanceCount;
							}
							else
								break;
						}
						if(nInstanceCount>AUTO_INSTANCING_THRESHOLD_NUM)
						{
							// use instancing.
							// Update the instance streams with the per instance data.
							MatrixVertex* pVertices = NULL;
							DynamicVertexBufferEntity* pBuffer = CGlobals::GetAssetManager()->GetDynamicBuffer(DVB_MATRIX);
							if(pBuffer!=0 && pBuffer->Lock(nInstanceCount, (VOID**)&pVertices) == nInstanceCount)
							{
								for(int i=0;i<nInstanceCount;i++)
								{	
									Matrix4 matrix;
									(*itCurCP).m_pRenderObject->GetRenderMatrix(matrix);
									// since no projection, encode the 4x3 part in a 3x4(shader will decode)
									pVertices[i].r1 = Vector4(matrix._11,matrix._21,matrix._31,matrix._41);
									pVertices[i].r2 = Vector4(matrix._12,matrix._22,matrix._32,matrix._42);
									pVertices[i].r3 = Vector4(matrix._13,matrix._23,matrix._33,matrix._43);
									if(i<(nInstanceCount-1))
										++itCurCP;
								}
								pBuffer->Unlock();

								if(CGlobals::GetEffectManager()->BeginEffect(TECH_SIMPLE_MESH_NORMAL_INSTANCED, &(sceneState.m_pCurrentEffect)))
								{
									// Set the first stream to be the indexed data and render N instances
									pd3dDevice->SetStreamSourceFreq(0,(D3DSTREAMSOURCE_INDEXEDDATA | nInstanceCount));
									// Set the second stream to be per instance data and iterate once per instance
									pd3dDevice->SetStreamSourceFreq(1,(D3DSTREAMSOURCE_INSTANCEDATA | 1));
									pd3dDevice->SetStreamSource(1, pBuffer->GetBuffer(), pBuffer->m_dwBase*pBuffer->m_nUnitSize, pBuffer->m_nUnitSize);

									sceneState.SetCameraToCurObjectDistance((*itCurCP).m_fObjectToCameraDistance);
									(*itCurCP).m_pRenderObject->Draw(&sceneState);
									nLastHandle = TECH_SIMPLE_MESH_NORMAL_INSTANCED;
									pLastEntity = pEntity;
									nObjCount += nInstanceCount;
								}
								continue;
							}
						}
					}
				}
			}
			pLastEntity = pEntity;

			CEffectFile* pEffect = CGlobals::GetEffectManager()->GetEffectByHandle(nCurTechHandle);
			if(pEffect!=0 && pEffect->IsValid() && pEffect->GetCurrentTechniqueDesc()->nCategory == 
				CEffectFile::TechCategory_GenShadowMap)
			{
				/** only render shadow. */
				if(nCurTechHandle!=nLastHandle){
					nLastHandle = nCurTechHandle;
					CGlobals::GetEffectManager()->BeginEffect(nCurTechHandle, &(sceneState.m_pCurrentEffect));
					// turn off alpha blending to enable early-Z on modern graphic cards. 
					CGlobals::GetRenderDevice()->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ONE);
					CGlobals::GetRenderDevice()->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_ZERO);
					CGlobals::GetRenderDevice()->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
				}

				sceneState.SetCameraToCurObjectDistance((*itCurCP).m_fObjectToCameraDistance);
				(*itCurCP).m_pRenderObject->Draw(&sceneState);
				++nObjCount;
			}
		}
#endif
		return nObjCount;
	}
}
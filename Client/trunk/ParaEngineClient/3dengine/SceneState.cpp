//-----------------------------------------------------------------------------
// Class:	Scene state
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2004.5
// Revised: 2005.11.12
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "TextureEntity.h"
#include "ViewCullingObject.h"
#include "IScene.h"
#include "SortedFaceGroups.h"
#include "BaseObject.h"
#include "ParaXModel/particle.h"
#include "LightObject.h"
#include "SceneState.h"

/** reference type in scene state */
#define REFERENCE_TYPE_SCENE_STATE		12

namespace ParaEngine
{
	CBaseObject* SceneState::m_pCurrentSceneObject = NULL;
	extern int64_t globalTime;

	SceneState::SceneState(IScene * pScene)
		:fAlphaFactor(1.0f), bIsBatchRender(false), m_bCameraMoved(true), m_bDeferredShading(false), m_nCurrentRenderPipeline(0),
		m_pCurrentEffect(NULL), m_nRenderCount(0), m_pScene(pScene), m_nOccluded(0), m_pBatchedDrawer(NULL), m_renderState(RenderState_Standard),
		m_bEnableTranslucentFaceSorting(false), m_fShadowMapCameraToObjectDistance(500.f), m_bIsShadowPass(false), m_bOuterWorldZoneVisible(true), m_bIgnoreTransparent(false), m_nCurRenderGroup(0), m_bUseLocalMaterial(false), m_bGlobalLOD(true)
	{
		m_pCurrentSceneObject = NULL;
		fViewCullingRadius = OBJ_UNIT*20; // default to 20 meters round the eye
		pTouchedObject = NULL;
		m_pd3dDevice = NULL;
		
		ZeroMemory( &m_matGlobalMaterial, sizeof(m_matGlobalMaterial) );
		m_matGlobalMaterial.Diffuse.r = m_matGlobalMaterial.Ambient.r = 0.3f;
		m_matGlobalMaterial.Diffuse.g = m_matGlobalMaterial.Ambient.g = 0.3f;
		m_matGlobalMaterial.Diffuse.b = m_matGlobalMaterial.Ambient.b = 0.3f;
		m_matGlobalMaterial.Diffuse.a = m_matGlobalMaterial.Ambient.a = 0.3f;

		listPostRenderingObjects.reserve(16);
		listPRBiped.reserve(16);
		listPRTransparentBiped.reserve(4);
		listPRSolidObject.reserve(16);
		listPRTransparentObject.reserve(16);
		listPRSmallObject.reserve(16);
		listShadowReceivers.reserve(16);
		listShadowCasters.reserve(16);
		listParticleSystems.reserve(32);

		Cleanup();
	};

	void SceneState::Cleanup()
	{
		CleanupSceneState();
		
		listParticleSystems.clear();

		mxView = Matrix4::IDENTITY;
		mxProj = Matrix4::IDENTITY;
		vEye = Vector3(0,0,0);
		vLookAt = Vector3(0,0,0);
		vEye = Vector3(0,0,0);
		pTouchedObject = NULL;
		m_nRenderCount = 0;
	}

	/// clean up all objects in post rendering lists.  call this when you finished rendering one render frame.
	void SceneState::CleanupSceneState()
	{
		listPostRenderingObjects.clear();
#ifdef USE_DIRECTX_RENDERER
		listPRSprite.clear();
#endif
		listPRMissiles.clear();
		listPRBiped.clear();
		listPRTransparentBiped.clear();
		listPRSolidObject.clear();
		listPRTransparentObject.clear();
		listPRSmallObject.clear();
		listHeadonDisplayObject.clear();
		listSolidOverlayObject.clear();
		listShadowReceivers.clear();
		listShadowCasters.clear();
		listDeferredLightObjects.clear();

		m_SortedFaceGroups.Clear();
		//listParticleSystems.clear();
		listZones.clear();
		listPortals.clear();
		listBoundingBox.clear();
		listSelections.clear();
		listOwnerDrawObjs.clear();

		bIsBatchRender = false;
		m_bEnableTranslucentFaceSorting = false;
		fAlphaFactor = 1.0f;
		m_dwD3D_CULLMODE = D3DCULL_CCW;
		m_pCurrentEffect = NULL;
		m_bSkipOcean = false;
		m_bSkipTerrain = false;
		m_bSkipSky = false;
		m_bOuterWorldZoneVisible = true;
		m_bIgnoreTransparent = false;
		m_bUseLocalMaterial = false;
		m_bDeferredShading = false;
		EnableGlobalLOD(true);
		m_curLightStrength = Vector3(1.f,0,0);
		m_renderState = RenderState_Standard;
		{
			int nCount = (int)m_RenderCount.size();
			for(int i=0; i<nCount;++i)
				m_RenderCount[i] = 0;
		}
		m_nGlobalTime = globalTime;
	};

	CSortedFaceGroups* SceneState::GetFaceGroups() 
	{
		return  &m_SortedFaceGroups;
	}

	PostRenderObject::PostRenderObject(CBaseObject* pRenderObject, float fObjectToCameraDistance, int nOccluded)
		:m_pRenderObject(pRenderObject), m_fObjectToCameraDistance(fObjectToCameraDistance), m_nOccluded(nOccluded)
	{
		
	}

	AlphaPostRenderObject::AlphaPostRenderObject(CBaseObject* pRenderObject, float fObjectToCameraDistance,int nOccluded, float fAlpha, ALPHA_ANIM nAlphaAnimMethod)
		:m_fAlpha(fAlpha), m_nAlphaAnimMethod(ALPHA_NO_ANIM), PostRenderObject(pRenderObject, fObjectToCameraDistance, nOccluded)
	{	
	}

	CBaseObject* SceneState::GetCurrentSceneObject()
	{
		return m_pCurrentSceneObject;
	}

	void  SceneState::SetCurrentSceneObject(CBaseObject* pObj)
	{
		m_pCurrentSceneObject = pObj;
	}
	void SceneState::AddParticleSystem(ParticleSystem* ps)
	{
		if(ps)
		{
			SceneState::List_ParticleSystemPtr_Type::const_iterator iter, iterEnd = listParticleSystems.end();
			for(iter = listParticleSystems.begin(); iter!= iterEnd; ++iter)
			{
				if((*iter).get()==ps)
				{
					return; // return if found.
				}
			}
			listParticleSystems.push_back(ParticleSystemRef(ps));
		}
		
	}

	HRESULT SceneState::DeleteDeviceObjects()
	{
		listParticleSystems.clear();
		return S_OK;
	}

	void SceneState::List_PostRenderObject_TrackRef_Type::push_back(PostRenderObject& item)
	{
		AddReference( item.m_pRenderObject, REFERENCE_TYPE_SCENE_STATE);
		List_PostRenderObject_Type::push_back(item);
	}

	void SceneState::List_PostRenderObject_TrackRef_Type::clear()
	{
		List_PostRenderObject_Type::clear();
		DeleteAllRefs();
	}

	void SceneState::List_PostRenderObject_TrackRef_Type::OnRefDeleted(IRefObject* rm)
	{
		if(DeleteReference(rm) == REF_SUCCEED && !empty())
		{
			SceneState::List_PostRenderObject_Type::iterator itCurCP, itEndCP = end();
			for( itCurCP = begin(); itCurCP != itEndCP; ++ itCurCP )
			{
				IRefObject* pObj = (IRefObject*)((*itCurCP).m_pRenderObject);
				if( pObj == rm )
				{
					// clear it, instead of delete it. 
					(*itCurCP).m_pRenderObject = NULL;
				}
			}
		}
	}

	bool SceneState::CheckAddRenderCount(int nRenderImportance)
	{
		if(nRenderImportance>=0 && nRenderImportance < (int)m_RenderCount.size() &&  nRenderImportance < (int)m_MaxRenderCount.size())
		{
			if(m_RenderCount[nRenderImportance] < m_MaxRenderCount[nRenderImportance])
			{
				m_RenderCount[nRenderImportance] ++;
				return true;
			}
		}
		else
			return true;
		return false;
	}

	void SceneState::SetMaxRenderCount(int nRenderImportance, int nCount)
	{
		if(nRenderImportance >=(int)m_RenderCount.size() )
		{
			m_RenderCount.resize(nRenderImportance+1, 0);
		}
		if(nRenderImportance >= (int)m_MaxRenderCount.size())
		{
			m_MaxRenderCount.resize(nRenderImportance+1, 65535);
		}
		m_RenderCount[nRenderImportance] = nCount;
		m_MaxRenderCount[nRenderImportance] = nCount;
	}

	void SceneState::SetGlobalTime( int nGlobalTime )
	{
		m_nGlobalTime = nGlobalTime;
	}

	void SceneState::SetIgnoreTransparent( bool bIgnore )
	{
		m_bIgnoreTransparent = bIgnore;
	}

	void SceneState::SetCameraToCurObjectDistance( float fDist )
	{
		m_fCameraToObjectDistance = fDist;
	}

	float SceneState::GetCameraToCurObjectDistance()
	{
		return m_bIsShadowPass ? m_fShadowMapCameraToObjectDistance : m_fCameraToObjectDistance;
	}

	bool SceneState::IsLODEnabled()
	{
		return IsGlobalLODEnabled() && (m_pCurrentSceneObject && m_pCurrentSceneObject->IsLODEnabled());
	}

	void SceneState::SetShadowMapCameraToCurObjectDistance( float fDist )
	{
		m_fShadowMapCameraToObjectDistance = fDist;
	}

	float SceneState::GetShadowMapCameraToCurObjectDistance()
	{
		return m_fShadowMapCameraToObjectDistance;
	}

	void SceneState::SetShadowPass( bool bShadowPass )
	{
		m_bIsShadowPass = bShadowPass;
	}

	bool SceneState::IsShadowPass()
	{
		return m_bIsShadowPass;
	}

	void SceneState::SetBatchedElementDrawer(IBatchedElementDraw* pDebugDraw)
	{
		m_pBatchedDrawer = pDebugDraw;
	}

	IBatchedElementDraw* SceneState::GetBatchedElementDrawer()
	{
		return m_pBatchedDrawer;
	}

	int SceneState::GetGlobalTime()
	{
		return m_nGlobalTime;
	}

	ParaMaterial& SceneState::GetGlobalMaterial()
	{
		return m_matGlobalMaterial;
	}

	ParaMaterial& SceneState::GetLocalMaterial()
	{
		return m_matLocalMaterial;
	}

	void SceneState::EnableLocalMaterial( bool bEnable /*= true*/ )
	{
		m_bUseLocalMaterial = bEnable;
	}

	ParaMaterial& SceneState::GetCurrentMaterial()
	{
		return m_bUseLocalMaterial ? m_matLocalMaterial : m_matGlobalMaterial;
	}

	bool SceneState::HasLocalMaterial()
	{
		return m_bUseLocalMaterial;
	}

	Vector3& SceneState::GetCurrentLightStrength()
	{
		return m_curLightStrength;
	}

	HRESULT SceneState::InitDeviceObjects()
	{
		return S_OK;
	}

	HRESULT SceneState::RestoreDeviceObjects()
	{
		return S_OK;
	}

	HRESULT SceneState::InvalidateDeviceObjects()
	{
		return S_OK;
	}

	bool SceneState::IsDeferredShading() const
	{
		return m_bDeferredShading;
	}

	void SceneState::SetDeferredShading(bool val)
	{
		m_bDeferredShading = val;
	}

	CBaseCamera* SceneState::GetCamera()
	{
		return GetScene()->GetCurrentCamera();
	}

	int SceneState::GetCurrentRenderPipeline() const
	{
		return m_nCurrentRenderPipeline;
	}

	void SceneState::SetCurrentRenderPipeline(int val)
	{
		m_nCurrentRenderPipeline = val;
	}

	void SceneState::AddToDeferredLightPool(CLightObject * lightObject)
	{
		listDeferredLightObjects.push_back(lightObject);
	}

	bool SceneState::IsGlobalLODEnabled() const
	{
		return m_bGlobalLOD;
	}

	void SceneState::EnableGlobalLOD(bool val)
	{
		m_bGlobalLOD = val;
	}

	ParaEngine::SceneStateRenderState SceneState::GetRenderState() const
	{
		return m_renderState;
	}

	void SceneState::SetRenderState(SceneStateRenderState val)
	{
		m_renderState = val;
	}

	CPushRenderState::CPushRenderState(SceneState* pState, SceneStateRenderState state) : m_pState(pState)
	{
		m_lastState = pState->GetRenderState();
		pState->SetRenderState(state);
	}

	CPushRenderState::CPushRenderState(SceneState* pState) : m_pState(pState)
	{
		m_lastState = pState->GetRenderState();
	}

	CPushRenderState::~CPushRenderState()
	{
		m_pState->SetRenderState(m_lastState);
	}

	int SceneState::InstallFields(CAttributeClass* pClass, bool bOverride)
	{
		IAttributeFields::InstallFields(pClass, bOverride);

		pClass->AddField("CurrentRenderPipeline", FieldType_Int, (void*)SetCurrentRenderPipeline_s, (void*)GetCurrentRenderPipeline_s, NULL, NULL, bOverride);
		pClass->AddField("RenderState", FieldType_Int, (void*)SetRenderState_s, (void*)GetRenderState_s, NULL, NULL, bOverride);
		pClass->AddField("GlobalTime", FieldType_Int, (void*)SetGlobalTime_s, (void*)GetGlobalTime_s, NULL, NULL, bOverride);
		pClass->AddField("CameraToCurObjectDistance", FieldType_Float, (void*)SetCameraToCurObjectDistance_s, (void*)GetCameraToCurObjectDistance_s, NULL, NULL, bOverride);
		return S_OK;
	}

}//namespace ParaEngine



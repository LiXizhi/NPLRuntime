//-----------------------------------------------------------------------------
// Class:	CSceneObject
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2004.3.8
// Revised: 2006.7.22
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#ifdef USE_DIRECTX_RENDERER
#include "DirectXEngine.h"
#include "SpriteObject.h"
#include "ShadowMap.h"
#include "GlowEffect.h"
#include "DataProviderManager.h"
#include "NpcDatabase.h"
#include "LightObject.h"
#include "MirrorSurface.h"
#include "RpgCharacter.h"
#include "BVHSerializer.h"
#include "DropShadowRenderer.h"
#include "OcclusionQueryBank.h"
#include "WaveEffect.h"
#elif defined(USE_OPENGL_RENDERER)
#include "ShadowMap.h"
#endif
#include "PaintEngine/Painter.h"
#include "ContainerObject.h"
#include "AttributeModelProxy.h"
#include "LightManager.h"
#include "BlockEngine/BlockWorldClient.h"
#include "OceanManager.h"
#include "BipedStateManager.h"
#include "EnvironmentSim.h"
#include "BipedObject.h"
#include "AutoCamera.h"
#include "SkyMesh.h"
#include "ParaWorldAsset.h"
#include "terrain/GlobalTerrain.h"
#include "FrameRateController.h"
#include "PhysicsWorld.h"
#include "ManagedLoader.h"
#include "MissileObject.h"
#include "ParaXModel/particle.h"
#include "RayCollider.h"
#include "2dengine/GUIRoot.h"
#include "2dengine/GUIDirectInput.h"
#include "ParaXSerializer.h"
#include "SceneWriter.h"
#include "ParaScriptingScene.h"
#include "2dengine/EventBinding.h"
#include "ParaXAnimInstance.h"
#include "SelectionManager.h"
#include "EventsCenter.h"
#include "MiniSceneGraph.h"
#include "sceneRenderList.h"
#include "SortedFaceGroups.h"
#include "ShapeAABB.h"
#include "ShapeSphere.h"
#include "PortalNode.h"
#include "ZoneNode.h"
#include "PortalFrustum.h"
#include "ic/ICDBManager.h"
#include "BatchedElementDraw.h"
#include "NPLRuntime.h"
#include "ViewportManager.h"
#include "TerrainTileRoot.h"
#include "SceneObject.h"
#include "SunLight.h"
#include "BufferPicking.h"
#include "memdebug.h"
#include "StringHelper.h"

/** @def shadow radius around the eye, larger than which shadows will not be considered.  */
#define SHADOW_RADIUS	100.f

/**@def sun angle larger than which the shadow will not be drawn. 1.57 means 90.  */
#define NO_SHADOW_SUN_ANGLE		1.5f

/** interval between two ripples from the same source in seconds*/
#define MAX_RIPPLE_INTERVAL		0.10f

/**@def  maximum number of mirror surfaces in the scene. */
#define MAX_MIRRORSURFACES_NUM	3

/** @default distance within which onclick event will be fired. 
the mouse ray picking distance for onclick event. usually 30-100 meters.*/
#define DEFAULT_ON_CLICK_DISTANCE	50.f

/** some default view culling parameters. please see the header file for more information.
* typically increase MINPOPUPDISTANCE_DEFAULT and ALWAYS_DRAW_RADIUS if one want to draw more objects in the distance.*/
#define ALWAYS_DRAW_RADIUS				15.0f
/** the tangent(angle). If the view angle of the object is larger than this value, it will be put to front to back post rendering list.
* otherwise, it will be put to back to front rendering list. Very careful with this value.*/
#define ALWAYS_DRAW__TAN_ANGLE			0.1f
#define MINPOPUPDISTANCE_DEFAULT		50.f
#define DEFAULT_CULLING_PIXELS_HEIGHT			20.f
#define FOG_START_DEFAULT				50.0f
#define FOG_END_DEFAULT					100.0f
#define FOG_DENSITY_DEFAULT				0.66f

/** the default distance from the eye, within which shadow map will be used. This should match the value in shadow map shader. */
#define DEFAULT_SHADOW_RADIUS		40.f

#ifdef DEBUG_SELECTION_GROUP
	#ifdef CHECK_SELECTION
	#undef CHECK_SELECTION
	const DWORD m_dwRenderGroupMask = RENDER_MESH_FRONT_TO_BACK | RENDER_MESH_BACK_TO_FRONT;
	#define CHECK_SELECTION(x) ((dwSelection&(x)&m_dwRenderGroupMask)>0)
	#endif
#endif

namespace ParaEngine
{
	bool g_bShaderVersion3 = false;
	
	extern int64_t globalTime;
	
	/** any object in the scene except. Usually for selection during scene editing.*/
	extern OBJECT_FILTER_CALLBACK g_fncPickingAll;

	/** whether force export mesh physics */
	extern bool g_bForceExportMeshPhysics;

	CSceneObject* CSceneObject::g_pRootscene = NULL;
}

static const string g_sPSSceneName = "_ps";

using namespace ParaTerrain;
using namespace ParaEngine;

/** default physics group mask. */
#define DEFAULT_PHYSICS_GROUP_MASK 0xffffffff


CSceneObject::CSceneObject()
:m_event(NULL),m_pickObj(NULL),m_bPickFlag(false),m_pActor(NULL), m_pEnvironmentSim(NULL),
m_bModified(false),m_bShowLocalLightMesh(false),m_bShowHeadOnDisplay(false), 
m_fMaxHeadOnDisplayDistance(50.f), m_bEnablePostProcessing(false),m_bEnablePortalZone(true),m_bUseWireFrame(false),
m_bShowPortalSystem(false), m_fShadowRadius(DEFAULT_SHADOW_RADIUS), m_bIsPersistent(true), m_bBlockInput(false), m_nCursorHotSpotX(-1),m_nCursorHotSpotY(-1),
#ifdef USE_DIRECTX_RENDERER
m_dropShadowRenderer(new DropShadowRenderer()),
#endif
m_globalTerrain(new ParaTerrain::CGlobalTerrain()),
m_sceneState(new ParaTerrain::SceneState()),
m_dwPhysicsGroupMask(DEFAULT_PHYSICS_GROUP_MASK), m_renderDropShadow(false), m_bShowMainPlayer(true), m_bCanShowMainPlayer(true), m_fPostRenderQueueOrder(100.f)
{
#ifdef _DEBUG
	// test local light
	m_bShowLocalLightMesh = true;
	m_bShowPortalSystem = true;
#endif
	g_pRootscene = this;

	m_pBatchedElementDraw = new CBatchedElementDraw();
	m_pPhysicsWorld = new CPhysicsWorld();		/// physics world
	m_pSunLight = new CSunLight();
	SetEnvironmentSim(new CEnvironmentSim());
	m_pBlockWorldClient = new BlockWorldClient();
	m_pTileRoot = new CTerrainTileRoot();

	/// game status
	m_bGamePaused = false;
	m_bInitialized = false;
	m_bGameEnabled = false;

	m_dwBackGroundColor = 0xffffffff;
	m_bEnableOcclusionQuery = false;
	m_bUseInstancing = false;
	m_dwEnableMouseEvent = 0; //0 to disable all and 0xffff to enable all.
	m_FogColorFactor = 0xffffffff;

	m_bAutoPlayerRipple = true;
	m_nMaxNumShadowCasters = 50;
	m_nMaxNumShadowReceivers = 0;

	m_nMaxCharTriangles = 50000;
	m_scripts.init();
	
	m_bCandrag=false;
	m_event=new CGUIEvent();
	m_event->SetBinding(this);
	CEventBinding *pBinding=new CEventBinding();
	pBinding->InitEventMappingTable();
	pBinding->EnableKeyboard();
	pBinding->EnableMouse();
	pBinding->DefaultMap_Mouse();
	pBinding->DefaultMap_Text();
	m_event->m_eventbinding=EventBinding_cow_type(pBinding);
	LoadDefaultEventBinding();

	m_fFullscreenGlowIntensity = 0.8f; 
	m_fFullscreenGlowBlendingFactor = 2;
	m_bEnableMiniScenegraph = true;

	/// set fog
	m_bRenderFog = false;
	m_dwFogColor = 0xff00bfff;
	m_fFogStart = FOG_START_DEFAULT;
	m_fFogEnd = FOG_END_DEFAULT;
	m_fFogDensity = FOG_DENSITY_DEFAULT;
	m_dwObjCullingMethod =  CENTER_ON_FRUSTUM | VIEW_RADIUS_FRUSTUM;// CENTER_ON_CAMERA|VIEW_RADIUS_FOG;
	m_fCullingPixelsHeight = DEFAULT_CULLING_PIXELS_HEIGHT;
	m_fMinPopUpDistance = MINPOPUPDISTANCE_DEFAULT;

	/// Default view culling radius
	/// view-culling: render objects within 30 meters from the eye position
	m_sceneState->fViewCullingRadius = OBJ_UNIT*40;

	m_bSnapToTerrain = true;
	m_bRenderMeshShadow = false;
	m_bEnableLighting = true;
	m_bEnableSunLight = true;

	/// turn on sky rendering, if there is any available sky meshes
	m_bRenderSky = true;
#ifdef _DEBUG
	m_bShowBoundingBox = true;
	m_bGenerateReport = true;
#else
	m_bShowBoundingBox = false;
	m_bGenerateReport = false;
#endif

	m_fOnClickDistance = DEFAULT_ON_CLICK_DISTANCE;

	m_sceneState->m_pScene = this;
	m_sceneState->SetBatchedElementDrawer(GetBatchedElementDrawer());
	// reset to a blank scene
	ResetScene();
	CreateAttributeModels();
}

CSceneObject::~CSceneObject(void)
{
	m_attribute_models.clear();
	SAFE_RELEASE(m_pEnvironmentSim);
	SAFE_DELETE(m_event);
	SAFE_DELETE(m_pBatchedElementDraw);
}

CSceneObject* CSceneObject::GetInstance()
{
	return g_pRootscene;
}

void CSceneObject::CreateAttributeModels()
{
	// all attribute models are in second column of child attribute object. 
	m_attribute_models.clear();
	m_attribute_models.reserve(16);
	// TILE_ROOT_INDEX = 0
	m_attribute_models.push_back(m_pTileRoot.get());
	// BLOCK_WORLD_INDEX = 1
	m_attribute_models.push_back(m_pBlockWorldClient.get());
	// PHYSICS_WORLD_INDEX = 2
	m_attribute_models.push_back(m_pPhysicsWorld.get());
	// SUNLIGHT_INDEX = 3
	m_attribute_models.push_back(m_pSunLight.get());
	// GLOBALTERRAIN_INDEX = 4
	m_attribute_models.push_back(m_globalTerrain.get());
	// SCENESTATE_INDEX = 5
	m_attribute_models.push_back(m_sceneState.get());

	m_attribute_models.push_back(new CArrayAttributeProxy< CameraPool_type >(&m_cameras, "cameras"));
	m_attribute_models.push_back(new CArrayAttributeProxy< SkyMeshPool_type >(&m_skymeshes, "skymeshes"));
	m_attribute_models.push_back(new CArrayAttributeProxy< MiniSceneGraphPool_type >(&m_miniSceneGraphs, "miniSceneGraphs"));

	m_attribute_models.push_back(new CArrayAttributeProxy< list_IObjectWeakPtr_Type >(&m_sentientGameObjects, "sentientGameObjects"));
	m_attribute_models.push_back(new CMapAttributeProxy< map <string, CZoneNode*> >(&m_zones, "zones"));
	m_attribute_models.push_back(new CMapAttributeProxy< map <string, CPortalNode*> >(&m_portals, "portals"));
	m_attribute_models.push_back(new CMapAttributeProxy< map <string, CManagedLoader*> >(&m_managedloader, "managedloader"));
	m_attribute_models.push_back(new CArrayAttributeProxy< MissileObjectPool_Type >(&m_missiles, "missiles"));

#ifdef PARAENGINE_CLIENT
	m_attribute_models.push_back(new CArrayAttributeProxy< MirrorSufacePool_Type >(&m_mirrorSurfaces, "mirrorSurfaces"));
#endif
	
}


void CSceneObject::ResetScene()
{
	Cleanup();

	if(m_pPhysicsWorld)
		m_pPhysicsWorld->ResetPhysics();

	m_currentplayer = NULL;
	
	/// create a default camera
	/// Camera for interactive character navigation
	CAutoCamera * pCamera = new CAutoCamera(); 
	m_pCurrentCamera = pCamera;
	m_cameras.push_back(pCamera);

	/// default tile root
	m_pTileRoot->ResetTerrain( 1280.f, 7); 
	
	m_vRenderOrigin = Vector3(0,0,0);
	ClearConsoleStr();
	if(IsInitialized())
	{
		// restore the aspect ratio of camera and reset fog parameters.
		ResetCameraAndFog();
	}
#ifdef PARAENGINE_CLIENT
	// two reserved mirrors at a time
	if(m_mirrorSurfaces.size()<2)
	{
		m_mirrorSurfaces.push_back(NULL);
		m_mirrorSurfaces.push_back(NULL);
	}
#endif
	m_bShowMainPlayer = true;
	// unregister all events
	// CGlobals::GetEventsCenter()->UnregisterAllEvent();
}

/** set the environment simulator to be used with this world. */
void CSceneObject::SetEnvironmentSim(IEnvironmentSim* pSim)
{
	if(m_pEnvironmentSim!=pSim)
	{
		SAFE_RELEASE(m_pEnvironmentSim);
		m_pEnvironmentSim = pSim;
	}
}

IParaDebugDraw* CSceneObject::GetDebugDrawer()
{
	return m_pBatchedElementDraw->GetDebugDrawInterface();
}

IBatchedElementDraw* CSceneObject::GetBatchedElementDrawer()
{
	return (IBatchedElementDraw*)m_pBatchedElementDraw;
}

CBaseObject* CSceneObject::GetCurrentActor()
{
	return m_pActor;
}

void CSceneObject::SetCurrentActor(CBaseObject* pActor)
{
	m_pActor = pActor;
}

const CEventBinding * CSceneObject::GetEventBinding()
{
	return m_event->GetConstEventBindingObj();
}

void CSceneObject::LoadDefaultEventBinding()
{
	if(GetEventBinding()==NULL)
		return;
	CEventBinding *pBinding=m_event->GetEventBindingObj();
	pBinding->MapEvent(EM_KEY_P,EM_GM_PAUSE);
}

void CSceneObject::SetMaxCharTriangles( int nNum )
{
	m_nMaxCharTriangles = nNum;
}

int  CSceneObject::GetMaxCharTriangles()
{
	return m_nMaxCharTriangles;
}

int CSceneObject::GetMaximumNumShadowReceivers()
{
	return m_nMaxNumShadowReceivers;
}

void CSceneObject::SetMaximumNumShadowReceivers(int nNum)
{
	m_nMaxNumShadowReceivers = nNum;
}

int CSceneObject::GetMaximumNumShadowCasters()
{
	return m_nMaxNumShadowCasters;
}

void CSceneObject::SetMaximumNumShadowCasters(int nNum)
{
	if(nNum<=0)
	{
		m_nMaxNumShadowCasters = 0;
		SetShadow(false);
	}
	else
		m_nMaxNumShadowCasters = nNum;
}

CZoneNode * CSceneObject::CreateGetZoneNode( const char* sName )
{
	if(sName == 0)
		return NULL;
	map <string, CZoneNode*>::iterator iter = m_zones.find(sName);
	if(iter != m_zones.end())
	{
		return (*iter).second;
	}
	else
	{
		CZoneNode* pObj = new CZoneNode();
		pObj->addref();
		m_zones[sName] = pObj;
		return pObj;
	}
}

CZoneNode * CSceneObject::GetZoneNode( const char* sName )
{
	if(sName == 0)
		return NULL;
	map <string, CZoneNode*>::iterator iter = m_zones.find(sName);
	return (iter != m_zones.end()) ? (*iter).second : NULL;
}

CPortalNode * CSceneObject::CreateGetPortalNode( const char* sName )
{
	if(sName == 0)
		return NULL;
	map <string, CPortalNode*>::iterator iter = m_portals.find(sName);
	if(iter != m_portals.end())
	{
		return (*iter).second;
	}
	else
	{
		CPortalNode* pObj = new CPortalNode();
		pObj->addref();
		m_portals[sName] = pObj;
		return pObj;
	}
}

CPortalNode * CSceneObject::GetPortalNode( const char* sName )
{
	if(sName == 0)
		return NULL;
	map <string, CPortalNode*>::iterator iter = m_portals.find(sName);
	return (iter != m_portals.end()) ? (*iter).second : NULL;
}


void CSceneObject::ShowPortalSystem( bool bEnable )
{
	m_bShowPortalSystem = bEnable;
}

bool CSceneObject::IsPortalSystemShown()
{
	return m_bShowPortalSystem;
}

CMirrorSurface* CSceneObject::GetMirrorSurface(int nIndex)
{
#ifdef USE_DIRECTX_RENDERER
	if(nIndex>=(int)m_mirrorSurfaces.size())
	{
		if(nIndex>MAX_MIRRORSURFACES_NUM)
			return NULL;
		m_mirrorSurfaces.resize(nIndex+1,NULL);
	}
	if(m_mirrorSurfaces[nIndex]==0)
	{
		m_mirrorSurfaces[nIndex] = new CMirrorSurface();
	}
	return m_mirrorSurfaces[nIndex];
#else
	return NULL;
#endif
}

bool CSceneObject::IsShowMainPlayer()
{
	return m_bShowMainPlayer;
}

void CSceneObject::ShowMainPlayer( bool bEnable )
{
	m_bShowMainPlayer = bEnable;
}

bool CSceneObject::CanShowMainPlayer() const
{
	return m_bCanShowMainPlayer && m_bShowMainPlayer;
}

void CSceneObject::SetCanShowMainPlayer(bool val)
{
	m_bCanShowMainPlayer = val;
}

bool CSceneObject::IsSunLightEnabled()
{
	return m_bEnableSunLight;
}
void CSceneObject::EnableSunLight(bool bEnable)
{
	m_bEnableSunLight = bEnable;
}

bool CSceneObject::IsLightEnabled()
{
	return m_bEnableLighting;
}

void CSceneObject::EnableLight(bool bEnable)
{
	m_bEnableLighting = bEnable;
}

void CSceneObject::SetMaxLocalLightsNum(int nNum)
{
#ifdef USE_DIRECTX_RENDERER
	CGlobals::GetEffectManager()->SetMaxLocalLightsNum(nNum);
#endif
}

int CSceneObject::GetMaxLocalLightsNum()
{
#ifdef USE_DIRECTX_RENDERER
	return CGlobals::GetEffectManager()->GetMaxLocalLightsNum();
#else
	return 0;
#endif
}

bool CSceneObject::IsShowLocalLightMesh()
{
	return m_bShowLocalLightMesh;
}

void CSceneObject::ShowLocalLightMesh(bool bShow)
{
	m_bShowLocalLightMesh = bShow;
}


DWORD CSceneObject::GetPhysicsGroupMask()
{
	return m_dwPhysicsGroupMask;
}

void CSceneObject::SetPhysicsGroupMask( DWORD dwValue )
{
	m_dwPhysicsGroupMask = dwValue;
}

void CSceneObject::ResetCameraAndFog()
{
}

ParaTerrain::CGlobalTerrain*	CSceneObject::GetGlobalTerrain()
{
	return m_globalTerrain.get();
}

CBaseObject* CSceneObject::FindObjectByNameAndType(const std::string& sName, const std::string& sClassName)
{
	CBaseObject* pParent = GetChildByName(sClassName);
	if (pParent){
		return pParent->GetChildByName(sName, false);
	}
	return NULL;
}

IObject* CSceneObject::AttachObject(CBaseObject * pObject)
{
	IObject * pParentObj = NULL;
	switch (pObject->GetType())
	{
	case CBaseObject::ManagedLoader:
		{
			CManagedLoader* pLoader = GetManagedLoader(pObject->GetIdentifier());
			if(pLoader)
			{
				pLoader->AttachObjectsToScene();
				pParentObj = m_pTileRoot.get();
			}
			break;
		}
	default:
		{
			if (pObject->IsTileObject())
			{
				if (pObject->IsGlobal())
				{
					pParentObj = m_pTileRoot->AttachGlobalObject(pObject);
				}
				else
				{
					pParentObj = m_pTileRoot->AttachLocalObject(pObject);
				}
			}
			else
			{
				// attach it to container of its type. 
				CBaseObject* pParent = GetChildByName(pObject->GetAttributeClassName());
				if (!pParent){
					OUTPUT_LOG("info: Object container automatically created for class: %s \n", pObject->GetAttributeClassName());
					pParent = CContainerObject::Create();
					pParent->SetIdentifier(pObject->GetAttributeClassName());
					AddChild(pParent);
				}
				if (pParent)
					pParent->AddChild(pObject);
				pParentObj = pParent;
			}
		}
	}
	return pParentObj;
}

void CSceneObject::AddSentientObject(IGameObject* pObj,bool bCheckDuplicate)
{
	if(pObj!=NULL)
	{
		++(pObj->m_nSentientObjCount);
		if(!bCheckDuplicate)
		{
			m_sentientGameObjects.push_back(pObj->GetWeakReference());
		}
		else
		{
			// check duplicates
			list_IObjectWeakPtr_Type::iterator itCur, itEnd = m_sentientGameObjects.end();
			for (itCur = m_sentientGameObjects.begin();itCur!=itEnd;++itCur)
			{
				if((*itCur)==pObj){
					return;
				}
			}
			m_sentientGameObjects.push_back(pObj->GetWeakReference());
		}
		pObj->On_EnterSentientArea();
	}
}

bool CSceneObject::DeleteSentientObject(IGameObject* pObject)
{
	if(pObject != NULL)
	{
		// delete object from the sentient object list.
		list_IObjectWeakPtr_Type::iterator itCur, itEnd = m_sentientGameObjects.end();
		for (itCur = m_sentientGameObjects.begin();itCur!=itEnd;++itCur)
		{
			if ((*itCur) == pObject)
			{
				pObject->On_LeaveSentientArea();
				m_sentientGameObjects.erase(itCur);
				return true;
			}
		}
	}
	return false;
}

bool CSceneObject::DetachObject(CBaseObject * pObject)
{
	bool bDetached = false;

	switch(pObject->GetType())
	{
	case CBaseObject::ManagedLoader:
		{
			CManagedLoader* pLoader = GetManagedLoader(pObject->GetIdentifier());
			if(pLoader)
			{
				pLoader->DetachObjectsFromScene();
			}
			break;
		}
	default:
		{
			/// compress object, so that physics and resource assets will be released.
			pObject->CompressObject();
			if (pObject->IsTileObject())
			{
				bDetached = m_pTileRoot->DetachObject(pObject);
				IGameObject* pGameObject = pObject->QueryIGameObject();
				if (pGameObject != NULL)
					DeleteSentientObject(pGameObject);
			}
			else
			{
				CBaseObject* pParent = GetChildByName(pObject->GetAttributeClassName());
				if (pParent)
				{
					if (pParent->RemoveChild(pObject) > 0)
					{
						bDetached = true;
					}
				}
			}
		}
	}
	return bDetached;
}

CBaseObject* CSceneObject::GetGlobalObject(const string& sName)
{
	return m_pTileRoot->GetGlobalObject(sName);
}

CBaseObject* CSceneObject::GetLocalObject(const string& sName, const Vector3& vPos)
{
	return m_pTileRoot->GetLocalObject(sName, vPos);
}

CBaseObject* CSceneObject::GetObject(const string& sName, const Vector3& vPos, bool bGlobal)
{
	return bGlobal? GetGlobalObject(sName) : GetLocalObject(sName, vPos);
}

CBaseObject* CSceneObject::GetLocalObject(const Vector3& vPos, float fEpsilon)
{
	return m_pTileRoot->GetLocalObject(vPos, fEpsilon);
}

CBaseObject* CSceneObject::GetLocalObject( const Vector3& vPos, const std::string& sName, float fEpsilon/*=0.01f*/ )
{
	return m_pTileRoot->GetLocalObject(vPos, sName, fEpsilon);
}

CBaseObject* CSceneObject::GetObjectByViewBox( const CShapeAABB& viewbox )
{
	return m_pTileRoot->GetObjectByViewBox(viewbox);
}

bool CSceneObject::DeleteObject(CBaseObject * pObject)
{
	switch(pObject->GetType())
	{
	case CBaseObject::ManagedLoader:
		DeleteManagedLoader(pObject->GetIdentifier());
		break;
	default:
		if(pObject->GetType() == CBaseObject::ZoneNode)
		{
			map <string, CZoneNode*>::iterator itCurCP, itEndCP = m_zones.end();
			for( itCurCP = m_zones.begin(); itCurCP != itEndCP;)
			{
				if(((*itCurCP).second) == pObject)
				{
					pObject->AddToAutoReleasePool();
					pObject->Release();
					itCurCP = m_zones.erase(itCurCP);
				}
				itCurCP++;
			}
		}
		else if(pObject->GetType() == CBaseObject::PortalNode)
		{
			map <string, CPortalNode*>::iterator itCurCP, itEndCP = m_portals.end();
			for( itCurCP = m_portals.begin(); itCurCP != itEndCP; ++ itCurCP)
			{
				if(((*itCurCP).second) == pObject)
				{
					pObject->AddToAutoReleasePool();
					pObject->Release();
					itCurCP = m_portals.erase(itCurCP);
				}
				itCurCP++;
			}
		}

		DetachObject(pObject);

		m_scripts.DeleteScript(pObject);
		break;
	}
	return true;
}

void CSceneObject::UnloadObjectResources(CBaseObject * pObject)
{

}

void CSceneObject::DestroyObjectByName(const char * id)
{
	if(strcmp(id, "\\"))
	{
		Cleanup();
	}
	else
	{
		m_pTileRoot->DestroyObjectByName(id);
	}
}

CManagedLoader* CSceneObject::GetManagedLoader(string sName)
{
	map <string, CManagedLoader*>::iterator	iter = m_managedloader.find(sName);
	if(iter!=m_managedloader.end())
	{
		return (*iter).second;
	}
	return NULL;
}

bool CSceneObject::DeleteManagedLoader(string sName)
{
	map <string, CManagedLoader*>::iterator	iter = m_managedloader.find(sName);
	if(iter!=m_managedloader.end())
	{
		iter->second->Release();
		m_managedloader.erase(iter);
		return true;
	}
	return false;
}

CManagedLoader* CSceneObject::CreateManagedLoader(string sName)
{
	CManagedLoader* pLoader = GetManagedLoader(sName);
	if(pLoader==NULL)
	{
		pLoader = new CManagedLoader();
		pLoader->SetIdentifier(sName);
		pLoader->addref();
		pair<map <string, CManagedLoader*>::iterator, bool> res = 
			m_managedloader.insert(pair<string, CManagedLoader*>(sName, pLoader));
		PE_ASSERT(res.second);
	}
	return pLoader;
}
HRESULT CSceneObject::InitDeviceObjects()
{
	HRESULT hr = S_OK;
	CBaseObject::InitDeviceObjects();

	CGlobals::GetEffectManager()->SetScene(this);
	m_sceneState->m_pd3dDevice = CGlobals::GetRenderDevice();

	m_globalTerrain->InitDeviceObjects();
	CGlobals::GetOceanManager()->InitDeviceObjects();

	m_pBlockWorldClient->InitDeviceObjects();

	{
		SkyMeshPool_type::iterator itCurCP, itEndCP = m_skymeshes.end();
		for( itCurCP = m_skymeshes.begin(); itCurCP != itEndCP; ++ itCurCP)
		{
			(*itCurCP)->InitDeviceObjects();
		}
	}
#ifdef USE_DIRECTX_RENDERER
	for (int i=0;i<(int)m_mirrorSurfaces.size();++i)
	{
		if(m_mirrorSurfaces[i]!=0)
			m_mirrorSurfaces[i]->InitDeviceObjects();
	}
	
	{
		MiniSceneGraphPool_type::iterator itCur, itEnd = m_miniSceneGraphs.end();
		for (itCur = m_miniSceneGraphs.begin(); itCur!=itEnd; ++itCur)
		{
			(*itCur)->InitDeviceObjects();
		}
	}
#endif
	
	return hr;
}

HRESULT CSceneObject::RestoreDeviceObjects()
{
	CBaseObject::RestoreDeviceObjects();
#ifdef USE_DIRECTX_RENDERER
	g_bShaderVersion3 = CGlobals::GetDirectXEngine().m_d3dCaps.VertexShaderVersion >= D3DVS_VERSION(3,0);
#endif
	ResetCameraAndFog();

	CGlobals::GetOceanManager()->RestoreDeviceObjects();
#ifdef USE_DIRECTX_RENDERER
	for (int i=0;i<(int)m_mirrorSurfaces.size();++i)
	{
		if(m_mirrorSurfaces[i]!=0)
			m_mirrorSurfaces[i]->RestoreDeviceObjects();
	}

	{
		MiniSceneGraphPool_type::iterator itCur, itEnd = m_miniSceneGraphs.end();
		for (itCur = m_miniSceneGraphs.begin(); itCur!=itEnd; ++itCur)
		{
			(*itCur)->RestoreDeviceObjects();
		}
	}
#endif
	m_globalTerrain->RestoreDeviceObjects();
	m_pBlockWorldClient->RestoreDeviceObjects();
	m_bInitialized = true;
	return S_OK;
}

HRESULT CSceneObject::InvalidateDeviceObjects()
{
	CBaseObject::InvalidateDeviceObjects();
	HRESULT hr = S_OK;
	m_globalTerrain->InvalidateDeviceObjects();
	CGlobals::GetOceanManager()->InvalidateDeviceObjects();

	m_pBlockWorldClient->InvalidateDeviceObjects();
#ifdef USE_DIRECTX_RENDERER
	for (int i=0;i<(int)m_mirrorSurfaces.size();++i)
	{
		if(m_mirrorSurfaces[i]!=0)
			m_mirrorSurfaces[i]->InvalidateDeviceObjects();
	}
#endif

	{
		MiniSceneGraphPool_type::iterator itCur, itEnd = m_miniSceneGraphs.end();
		for (itCur = m_miniSceneGraphs.begin(); itCur!=itEnd; ++itCur)
		{
			(*itCur)->InvalidateDeviceObjects();
		}
	}
	
	return hr;
}


HRESULT CSceneObject::RendererRecreated()
{
	CBaseObject::RendererRecreated();
	m_pBlockWorldClient->RendererRecreated();
	for (auto& skymesh : m_skymeshes)
	{
		skymesh->RendererRecreated();
	}
	return S_OK;
}


HRESULT CSceneObject::DeleteDeviceObjects()
{
	CBaseObject::DeleteDeviceObjects();
	HRESULT hr = S_OK;
	m_globalTerrain->DeleteDeviceObjects();
	
	m_pBlockWorldClient->DeleteDeviceObjects();
	{
		SkyMeshPool_type::iterator itCurCP, itEndCP = m_skymeshes.end();
		for( itCurCP = m_skymeshes.begin(); itCurCP != itEndCP; ++ itCurCP)
		{
			(*itCurCP)->DeleteDeviceObjects();
		}
	}
#ifdef USE_DIRECTX_RENDERER
	m_dropShadowRenderer->DeleteDeviceObjects();
	CGlobals::GetOceanManager()->DeleteDeviceObjects();

	for (int i=0;i<(int)m_mirrorSurfaces.size();++i)
	{
		if(m_mirrorSurfaces[i]!=0)
			m_mirrorSurfaces[i]->DeleteDeviceObjects();
	}
#endif
	{
		MiniSceneGraphPool_type::iterator itCur, itEnd = m_miniSceneGraphs.end();
		for (itCur = m_miniSceneGraphs.begin(); itCur!=itEnd; ++itCur)
		{
			(*itCur)->DeleteDeviceObjects();
		}
	}
	m_bInitialized = false;

	return hr;
}
void CSceneObject::AddConsoleStr(const char * pStr)
{
	OUTPUT_LOG("%s\n", pStr);
}
	
void CSceneObject::ClearConsoleStr()
{
	m_sConsoleString[0] = '\0';
}

const char * ParaEngine::CSceneObject::GetConsoleString() const
{
	return m_sConsoleString;
}

void CSceneObject::SetBackGroundColor( const LinearColor& bgColor )
{
	m_dwBackGroundColor = bgColor;
}

LinearColor CSceneObject::GetBackGroundColor()
{
	LinearColor color(m_dwBackGroundColor);
	color.a = 1.0f;
	return color;
}


ParaEngine::LinearColor CSceneObject::GetClearColor()
{
	return 0x00ffffff & (DWORD)(IsSceneEnabled() ? GetFogColor() : GetBackGroundColor());
}

LinearColor CSceneObject::GetFogColor()
{
	LinearColor color(m_dwFogColor);
	color *= m_fFogDensity;
	color.a = 1.0f;
	return color;
}

void CSceneObject::SetFogColor(const LinearColor& fogColor)
{
	m_dwFogColor = fogColor;
}
void CSceneObject::SetFogStart(float fFogStart)
{
	m_fFogStart = fFogStart;
}

float CSceneObject::GetFogStart()
{
	return m_fFogStart;
}
void CSceneObject::SetFogEnd(float fFogEnd)
{
	m_fFogEnd = fFogEnd;
}
float CSceneObject::GetFogEnd()
{
	return m_fFogEnd;
}
void CSceneObject::SetFogDensity(float fFogDensity)
{
	m_fFogDensity = fFogDensity;
}
float CSceneObject::GetFogDensity()
{
	return m_fFogDensity;
}

void CSceneObject::EnableFog(bool bEnableFog)
{
	m_bRenderFog = bEnableFog;
}

bool CSceneObject::IsFogEnabled()
{
	return m_bRenderFog;
}

// just for backward compatibilities
void CSceneObject::SetAndRestoreFog(bool bEnableFog, DWORD dwFogColor, FLOAT fFogStart, FLOAT fFogEnd, FLOAT fFogDensity)
{
	m_bRenderFog = bEnableFog;
	m_dwFogColor = dwFogColor;
	m_fFogStart = fFogStart;
	m_fFogEnd = fFogEnd;
	m_fFogDensity = fFogDensity;
}

//-----------------------------------------------------------------------------
// Name: Cleanup()
/// Desc: Called before the app exits, this function gives the app the chance
///       to cleanup after itself.Animate the scene : Environment simulation 
///       is carried out by the scene
//-----------------------------------------------------------------------------
void CSceneObject::Cleanup()
{
	if (m_pBatchedElementDraw)
		m_pBatchedElementDraw->ClearAll();
	if (m_pBlockWorldClient)
		m_pBlockWorldClient->Cleanup();
	SetPersistent(true);
	EnableScene(false);

	RemoveDeadObjects();
	m_dead_objects.shrink_to_fit();

	/// remove all portals
	{
		map <string, CPortalNode*>::iterator itCurCP, itEndCP = m_portals.end();

		for( itCurCP = m_portals.begin(); itCurCP != itEndCP; ++ itCurCP)
		{
			((*itCurCP).second)->Release();
		}
		m_portals.clear();
	}
	/// remove all zones
	{
		map <string, CZoneNode*>::iterator itCurCP, itEndCP = m_zones.end();

		for( itCurCP = m_zones.begin(); itCurCP != itEndCP; ++ itCurCP)
		{
			((*itCurCP).second)->Release();
		}
		m_zones.clear();
	}

	// clear all selections
	CGlobals::GetSelectionManager()->ClearGroup(-1);

	m_scripts.ClearAll();
#ifdef USE_DIRECTX_RENDERER
	m_dropShadowRenderer->Cleanup();
#endif
	m_globalTerrain->Cleanup();

	/*if(m_pEnvironmentSim)
		m_pEnvironmentSim->CleanupImtermediateData();*/

	// clean scene state
	m_sceneState->Cleanup();
	m_missiles.clear();
	/// remove all loaders
	{
		map <string, CManagedLoader*>::iterator itCurCP, itEndCP = m_managedloader.end();

		for( itCurCP = m_managedloader.begin(); itCurCP != itEndCP; ++ itCurCP)
		{
			((*itCurCP).second)->Release();
		}
		m_managedloader.clear();
	}

	m_sentientGameObjects.clear();
	/// delete m_cameras
	{
		m_cameras.clear();
		m_pCurrentCamera.reset();
	}

	// delete all mini scene graph 
	{
		for (auto pScene : m_miniSceneGraphs)
		{
			pScene->Reset();
		}
		m_miniSceneGraphs.clear();
	}

	/// delete Sky meshes
	{
		DeleteSkyBox("");
	}
	{/// delete children
		DestroyChildren();	//recursively
	}
	m_pTileRoot->Cleanup();

	{/// delete shadow volumes
		ShadowVolumePool_Type::iterator itCurCP, itEndCP = m_ShadowVolumes.end();
		
		for( itCurCP = m_ShadowVolumes.begin(); itCurCP != itEndCP; ++ itCurCP)
		{
			delete *itCurCP;
		}
		m_ShadowVolumes.clear();
	}

	/// reset physics environment
	if(m_pPhysicsWorld)
		m_pPhysicsWorld->ExitPhysics();
#ifdef USE_DIRECTX_RENDERER
	for (int i=0;i<(int)m_mirrorSurfaces.size();++i)
	{
		if(m_mirrorSurfaces[i]!=0)
		{
			m_mirrorSurfaces[i]->InvalidateDeviceObjects();
			m_mirrorSurfaces[i]->DeleteDeviceObjects();
			m_mirrorSurfaces[i]->Cleanup();
			SAFE_DELETE(m_mirrorSurfaces[i]);
		}
	}
#endif
}

void CSceneObject::AutoGenPlayerRipple(float fTimeDelta)
{
	BlockWorldClient* pBlockWorldClient = BlockWorldClient::GetInstance();
	if(m_bAutoPlayerRipple)
	{
		static float g_fLastRippleTime=MAX_RIPPLE_INTERVAL;

		CBipedObject* pPlayer = GetCurrentPlayer();
		if(pPlayer!=0)
		{
			Vector3 vPos = pPlayer->GetPosition();
			float fWaterLevel = pBlockWorldClient->GetWaterLevel(vPos.x, vPos.y+pPlayer->GetHeight(), vPos.z, 2);
			
			/** only add a ripple if (1) the character is moving, (2) character is on or below water surface, (3) character's head is above water surface */
			if( (!pPlayer->IsStanding() || fabs(pPlayer->GetVerticalSpeed())>0.1f) && vPos.y<=fWaterLevel && (fWaterLevel-vPos.y)<(pPlayer->GetPhysicsHeight()*0.78f) )
			{
				if(g_fLastRippleTime>=MAX_RIPPLE_INTERVAL)
				{
					g_fLastRippleTime = 0;

					// add a ripple at the feet of the character, but adjusted to the height of the water surface.  
					vPos.y = fWaterLevel;
					//vPos.y = m_underwater ? fWaterLevel-0.1f: fWaterLevel+0.1f;

					CGlobals::GetOceanManager()->AddRipple(vPos);
				}
				else
				{
					g_fLastRippleTime +=fTimeDelta;
				}
			}
			else
			{
				g_fLastRippleTime = MAX_RIPPLE_INTERVAL;
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Name: Animate()
/// Desc: Called once per frame, the call is the entry point for animating
///       the scene. Animate the camera. This should match the speed of 
///       rendering, otherwise the scene will not seem to move.
//-----------------------------------------------------------------------------
void CSceneObject::Animate( double dTimeDelta, int nRenderNumber )
{
	RemoveDeadObjects();
	if( !m_bGameEnabled || !m_bInitialized)
		return;
	m_pBlockWorldClient->OnFrameMove();

	/// Update the camera 
	GetCurrentCamera()->FrameMove( (FLOAT)dTimeDelta );

	// when game time is paused, also pause the scene delta time
	if (CGlobals::GetFrameRateController(FRC_GAME)->IsPaused() && dTimeDelta > 0.f)
		dTimeDelta = CGlobals::GetFrameRateController(FRC_GAME)->GetElapsedTime();

	if (m_bGamePaused)
		return;

	AutoGenPlayerRipple((float)dTimeDelta);
	
	if(CGlobals::WillGenReport())
	{
		/// report current player position and facing
		if(GetCurrentPlayer())
		{
			char sDesc[256];
			Vector3 vPlayerPos = GetCurrentPlayer()->GetPosition();
			snprintf( sDesc,255, " time: %2.f pos<%2.1f,%2.1f, %2.1f> %2.1f", 
				(float)(CGlobals::GetGameTime()), vPlayerPos.x, vPlayerPos.y, vPlayerPos.z, GetCurrentPlayer()->GetFacing() );
			CGlobals::GetReport()->SetString("PInfo", sDesc);
		}
	}
	
	for (CBaseObject* pChild : GetChildren())
	{
		pChild->Animate(dTimeDelta, nRenderNumber);
	}
}

HRESULT CSceneObject::Draw(SceneState * sceneState)
{
	return S_OK;
}

bool CSceneObject::IsInitialized()
{
	return m_bInitialized;
}

int ClipZone(SceneState& sceneState, PostRenderObject& zone, CPortalFrustum& portalFrustum );

template <class T>
int RemovePortalOccludedObjects(T& renderlist)
{
	typename T::iterator itCur, itEnd =  renderlist.end();
	for(itCur = renderlist.begin();itCur!=itEnd;)
	{
		if((*itCur).m_nOccluded>=0)
			itCur = renderlist.erase(itCur);
		else
			++itCur;
	}
	return 0;
}

template <class T>
int RemoveZoneObjects(T& renderlist)
{
	typename T::iterator itCur, itEnd =  renderlist.end();
	for(itCur = renderlist.begin();itCur!=itEnd;)
	{
		CBaseObject* pObj = (*itCur).m_pRenderObject;
		if(pObj->GetHomeZone())
			itCur = renderlist.erase(itCur);
		else
			++itCur;
	}
	return 0;
}
bool CSceneObject::PrepareRenderObject(CBaseObject* pObj, CBaseCamera* pCamera, SceneState& sceneState)
{
	IViewClippingObject* pViewClippingObject = pObj->GetViewClippingObject();
	// fNewViewRadius is usually the camera far plane distance, however, it can be set to a smaller value according to the size of the object. 
	float fNewViewRadius;
	float fR = pViewClippingObject->GetRadius();
	if(fR == 0.f && pObj->CheckVolumnField(OBJ_BIG_STATIC_OBJECT))
	{
		// This is tricky: due to asynchronous loading of mesh entity, we do not know how big the associated mesh is when game world is loaded. 
		// so in case, the object is marked as big static object, we will use the radius of the object instead of mesh entity for view culling. 
		pViewClippingObject = pObj;
		fR = pViewClippingObject->GetRadius();
	}
	
	float fRenderDistance = pObj->GetRenderDistance();
	if(fRenderDistance <= 0.f)
	{
		fNewViewRadius = (fR>0.2f) ? m_fCoefK*fR : m_fCoefN;
		
		if(fNewViewRadius<m_fMinPopUpDistance)
			fNewViewRadius = m_fMinPopUpDistance;
	}
	else
	{
		fNewViewRadius = fRenderDistance;
	}
	if(fNewViewRadius > m_fFogEnd)
		fNewViewRadius = m_fFogEnd;

	bool bDrawObj = false;
	
	/*  the intersection of two spheres: one sphere with center at the eye position; another at the camera frustum's bounding volumne origin. 
	bool bEyeRoughTestPassed = bRoughTestPassed = pViewClippingObject->TestCollisionSphere(& (sceneState.vEye), fNewViewRadius,1);
	if(!bEyeCenteredTest && bRoughTestPassed)
	{
		bRoughTestPassed = pViewClippingObject->TestCollisionSphere(& vViewCenter, fViewRadius,1);
	}*/
	
	///  rough test is performed first followed by precise bounding box test, with the camera's view frustum.
	if( pViewClippingObject->TestCollisionSphere(pCamera, fNewViewRadius)
		&& pViewClippingObject->TestCollision(pCamera))
	{
		bDrawObj = true;
		
		ObjectType oType = pObj->GetMyType();
		if(oType ==  _PC_Zone)
		{
			bDrawObj = false;
			sceneState.listZones.push_back(PostRenderObject(pObj, 0));
			return true;
		}
		else if(oType ==  _PC_Portal)
		{
			sceneState.listPortals.push_back(PostRenderObject(pObj, 0));
			bDrawObj = false;
			return true;
		}
	}
	
	bool bIsShadowCaster = false;
	
	//////////////////////////////////////////////////////////////////////////
	// test if this is shadow caster or shadow receiver
	//  it is only a shadow receiver if it is visible. 
	bool bIsShadowReceiver = bDrawObj && pObj->IsShadowReceiver();
	
	// if a mesh is shadow receiver, it will not be caster caster at the same time, so...
	// if an object has a home zone, it must not be shadow caster
	// if object is visible, it is shadow caster already.
	Vector3 vSunDir = GetSunLight().GetSunDirection();
	bIsShadowCaster = ( m_bRenderMeshShadow && pObj->IsShadowCaster() && pObj->GetHomeZone()==0)
		&& ((pViewClippingObject->TestCollisionSphere(& (sceneState.vEye), GetShadowRadius(),1) 
					&& pViewClippingObject->TestShadowSweptSphere(pCamera, &(vSunDir))));
	
	/// draw this object
	if(bDrawObj || bIsShadowCaster)
	{
		float fObjectToCameraDist = (pObj->GetObjectToCameraDistance()<=0.f) ?  pViewClippingObject->GetObjectToPointDistance(& (sceneState.vEye)) : pObj->GetObjectToCameraDistance();
		if(bDrawObj)
		{
			if(pObj->IsShowBoundingBox())
			{
				DWORD dwColor = COLOR_ARGB(255,255,0,0);
				pViewClippingObject->DrawBoundingBox(&sceneState, dwColor);
			}
			if(pObj->GetSelectGroupIndex() >= 0)
			{
				sceneState.listSelections.push_back(PostRenderObject(pObj,fObjectToCameraDist));
			}
		}
		/** To which post rendering list the object goes to. this is only for debugging purposes. 
		* the bounding box of object from different render list are drawn using different colors.
		* 0 means biped post rendering list.
		* 1 means solid object rendering list.
		* 2 means transparent or small rendering list.
		* */
		//int nPostRenderType = 0;
		if(pObj->IsBiped())
		{
			// this is a biped object
			if(bDrawObj)
			{
				PostRenderObject o(pObj, fObjectToCameraDist, sceneState.m_nOccluded);
				if (pObj->GetRenderOrder() >= m_fPostRenderQueueOrder)
					sceneState.listPostRenderingObjects.push_back(o);
				else if (!pObj->HasAlphaBlendedObjects())
					sceneState.listPRBiped.push_back(o);
				else
					sceneState.listPRTransparentBiped.push_back(o); 
			}
			if(bIsShadowCaster && pObj->GetOpacity()>0.f)
			{
				PostRenderObject o(pObj, fObjectToCameraDist);
				sceneState.listShadowCasters.push_back(o);
			}
			//////////////////////////////////////////////////////////////////////////
			// animate the visible biped
			((CBipedObject*)pObj)->Animate(sceneState.dTimeDelta, sceneState.GetRenderFrameCount());
		}
		else
		{
			/** for ordinary mesh objects, we will check its distance from the camera eye to its surface. 
			* if */
			float fObjRenderTangentAngle=0;
			if(pObj->IsTransparent() && bDrawObj)
			{
				pObj->AutoSelectTechnique();
				sceneState.listPRTransparentObject.push_back(AlphaPostRenderObject(pObj, fObjectToCameraDist, sceneState.m_nOccluded));
				//nPostRenderType = 2;
			}
			else if( fObjectToCameraDist<=ALWAYS_DRAW_RADIUS || (fObjRenderTangentAngle = pViewClippingObject->GetRadius()/fObjectToCameraDist)>ALWAYS_DRAW__TAN_ANGLE)
			{
				/// if the mesh is very near the camera, or it has a very big view angle, we will draw it as solid mesh,
				/// and thus it shall go to solid list.
				pObj->AutoSelectTechnique();
				if(bDrawObj)
				{
					if (pObj->GetRenderOrder() >= m_fPostRenderQueueOrder){
						PostRenderObject o(pObj, fObjectToCameraDist, sceneState.m_nOccluded);
						sceneState.listPostRenderingObjects.push_back(o);
					}
					else if(!pObj->HasAlphaBlendedObjects())
						sceneState.listPRSolidObject.push_back(PostRenderObject(pObj, fObjectToCameraDist, sceneState.m_nOccluded));
					else
					{
						sceneState.listPRTransparentObject.push_back(AlphaPostRenderObject(pObj, fObjectToCameraDist, sceneState.m_nOccluded));
					}
				}
				if (pObj->IsVisible())
				{
					if (bIsShadowCaster && pObj->GetOpacity()>0.f)
					{
						sceneState.listShadowCasters.push_back(PostRenderObject(pObj, fObjectToCameraDist));
					}
					else if (bIsShadowReceiver)
					{
						// only take the bounding box of the shadow receiver in to consideration, 
						// when the receiver object is either close to the camera or has a big viewing angle.
						sceneState.listShadowReceivers.push_back(PostRenderObject(pObj, fObjectToCameraDist));
					}
				}
				//nPostRenderType = 1;
			}
			else if(bDrawObj)
			{
				/// if the mesh is far away from the camera, and it has a small view angle, we will draw it as transparent mesh,
				float fAlpha = 1.0f;
				if(fObjectToCameraDist > m_fFogStart)
					fAlpha = 0.5f;
				// TODO: fAlpha animation should be enabled, according to whether a object is drawn in last frame.
				pObj->AutoSelectTechnique();
				if (pObj->GetRenderOrder() >= m_fPostRenderQueueOrder){
					PostRenderObject o(pObj, fObjectToCameraDist, sceneState.m_nOccluded);
					sceneState.listPostRenderingObjects.push_back(o);
				}
				else
					sceneState.listPRSmallObject.push_back(AlphaPostRenderObject(pObj, fObjectToCameraDist, sceneState.m_nOccluded, fAlpha));
				//nPostRenderType = 2;
			}
		}
	}
	if (!pObj->GetChildren().empty())
	{
		for (CBaseObject* pChild : pObj->GetChildren())
		{
			if (pChild->IsVisible())
			{
				pChild->ViewTouch();
				pChild->PrepareRender(pCamera, &sceneState);
			}
		}
	}
	return bDrawObj;
}

void CSceneObject::PrepareTileObjects(CBaseCamera* pCamera, SceneState &sceneState)
{
	Vector3 vViewCenter(0, 0, 0);
	queue_CTerrainTilePtr_Type queueTiles;
	CTerrainTile* pTile = m_pTileRoot.get();

	// they are for rough object-level culling.
	float fViewRadius;
	bool bEyeCenteredTest = false; // whether to use auto radius

	// fNewViewRadius = (fR>0.2f) ? m_fCoefK*fR : m_fCoefN;
	m_fCoefF = m_fFogEnd;
	m_fCoefN = m_fFogStart;
#ifdef USE_DIRECTX_RENDERER
	uint32 nViewHeight = CGlobals::GetDirectXEngine().GetBackBufferHeight();
#else
	uint32 nViewHeight = 560;
#endif
	m_fCoefK = 1 / tanf(m_fCullingPixelsHeight / nViewHeight * (pCamera->GetFieldOfView() / pCamera->GetAspectRatio()));

	// set rough testing view's center
	if (CheckObjectCullingMethod(CENTER_ON_PLAYER))
	{
		/**
		* set view-culling radius center.
		* we will use the current camera followed character as the view center
		* if the character does not exist, we will use the camera eye position as the view center
		*/
		if (pCamera->GetFollowTarget() && (pCamera->GetCameraMode() < CameraCamera))
			vViewCenter = pCamera->GetFollowTarget()->GetPosition();
		else
			vViewCenter = pCamera->GetEyePosition();
	}
	else if (CheckObjectCullingMethod(CENTER_ON_FRUSTUM))
	{
		vViewCenter.x = pCamera->GetObjectFrustum()->v2DCircleCenter.x;
		vViewCenter.y = sceneState.vEye.y;
		vViewCenter.z = pCamera->GetObjectFrustum()->v2DCircleCenter.y;
	}
	else //if(CheckObjectCullingMethod(CENTER_ON_CAMERA))
	{
		bEyeCenteredTest = true;
		vViewCenter = pCamera->GetEyePosition();
	}

	// set rough testing view's radius
	if (CheckObjectCullingMethod(VIEW_RADIUS_FRUSTUM))
	{
		fViewRadius = pCamera->GetObjectFrustum()->v2DCircleRadius;
	}
	else if (CheckObjectCullingMethod(VIEW_RADIUS_FOG))
	{
		fViewRadius = m_fFogEnd;
	}
	else
	{
		fViewRadius = sceneState.fViewCullingRadius;
	}

	/// render object on root tile
	{
		// pTile is now the root tile
		if (pTile->m_listVisitors.empty() == false)
		{
			/// add all visitor biped objects, only the last time will have this list non-empty
			for (auto itCurCP1 = pTile->m_listVisitors.begin(); itCurCP1 != pTile->m_listVisitors.end();)
			{
				if ((*itCurCP1))
				{
					(*itCurCP1)->PrepareRender(pCamera, &sceneState);
					itCurCP1++;
				}
				else
					itCurCP1 = pTile->m_listVisitors.erase(itCurCP1);
			}
		}
	}

	// add all objects on mini scene graph
	if (IsMiniSceneGraphEnabled())
	{
		// mini scenegraph objects are always not occluded. so that zones and portals have no effects on them. 
		sceneState.m_nOccluded = -1;
		MiniSceneGraphPool_type::iterator itCur, itEnd = m_miniSceneGraphs.end();
		for (itCur = m_miniSceneGraphs.begin(); itCur != itEnd; ++itCur)
		{
			CMiniSceneGraph* pObj = (*itCur);
			if (pObj && pObj->IsVisible() && !(pObj->IsCameraEnabled()))
			{
				for (CBaseObject* pChild : pObj->GetChildren())
				{
					if (pChild->IsVisible())
					{
						pChild->ViewTouch();
						pChild->PrepareRender(pCamera, &sceneState);
					}
				}
			}
		}
		sceneState.m_nOccluded = 0;
	}

	/// breadth first transversing the scene(the root tile is ignored)
	/// pTile is now the root tile. object attached to it are never rendered directly
	bool bQueueTilesEmpty = false;
	while (bQueueTilesEmpty == false)
	{
		/// add other tiles
		for (int i = 0; i < MAX_NUM_SUBTILE; i++)
		{
			if (pTile->m_subtiles[i])
			{
				/// rough culling algorithm using the quad tree terrain tiles
				/// test against a sphere round the eye
				if (pTile->m_subtiles[i]->TestCollisionSphere(&(vViewCenter), fViewRadius))
				{
					queueTiles.push(pTile->m_subtiles[i]);
				}
				else if (m_bRenderMeshShadow &&
					!bEyeCenteredTest && pTile->m_subtiles[i]->TestCollisionSphere(&(sceneState.vEye), SHADOW_RADIUS))
				{
					// ensure all sub tiles with potential shadow casters are enabled.
					queueTiles.push(pTile->m_subtiles[i]);
				}
			}
		}

		/// go down the quad tree terrain tile to render objects
		if (queueTiles.empty())
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
				/// add all solid objects to the queue for further view clipping test
				for (auto& pObj : pTile->m_listSolidObj)
				{
					/// We will not render Biped that can perceive its surroundings.
					/// since they will be in the visitor biped list. 
					if (pObj->IsRenderable() && pObj->CheckAttribute(OBJ_VOLUMN_TILE_VISITOR) == false)
						pObj->PrepareRender(pCamera, &sceneState);
					else
					{
						if (pObj->IsVisible())
							pObj->ViewTouch();
					}
				}
				if (!pTile->m_listVisitors.empty())
				{
					/// add all visitor biped objects to the queue.
					for (auto itCurCP1 = pTile->m_listVisitors.begin(); itCurCP1 != pTile->m_listVisitors.end();)
					{
						IGameObject* pObj = (*itCurCP1);
						if (pObj)
						{
							if (pObj->IsRenderable())
								pObj->PrepareRender(pCamera, &sceneState);
							else
							{
								if (pObj->IsVisible())
									pObj->ViewTouch();
							}
							itCurCP1++;
						}
						else
							itCurCP1 = pTile->m_listVisitors.erase(itCurCP1);
					}
				}
			}
			{
				/// add all free space objects to the queue
				for (auto& pObj : pTile->m_listFreespace)
				{
					if (pObj->IsRenderable())
						pObj->PrepareRender(pCamera, &sceneState);
					else
					{
						if (pObj->IsVisible())
							pObj->ViewTouch();
					}
				}
			}
		}
	}//while(!queueTiles.empty())
}


void CSceneObject::PrepareMissileObjects(CBaseCamera* pCamera, SceneState &sceneState)
{
	MissileObjectPool_Type::iterator itCurCP, itEndCP = m_missiles.end();

	for (itCurCP = m_missiles.begin(); itCurCP != itEndCP; ++itCurCP)
	{
		CMissileObject* pMissile = (*itCurCP);
		IViewClippingObject* pViewObj = pMissile->GetViewClippingObject();
		if (!pMissile->IsExploded() && pCamera->CanSeeObject(pViewObj))
		{
			float fObjectToCameraDist = pViewObj->GetObjectToPointDistance(&(sceneState.vEye));
			sceneState.listPRMissiles.push_back(AlphaPostRenderObject(pMissile, fObjectToCameraDist));
		}
	}
}

void CSceneObject::PreparePortalsAndZones(CBaseCamera* pCamera, SceneState &sceneState)
{
#ifdef USE_DIRECTX_RENDERER
	if (!sceneState.listZones.empty() && IsPortalZoneEnabled())
	{
		// we will further remove potentially visible objects according to portals and zones

		// Find the rootZone where camera eye position is, by testing all zones in zonePool.
		PostRenderObject* rootZone = NULL;
		{
			SceneState::List_PostRenderObject_Type::iterator itCurCP, itEndCP = sceneState.listZones.end();
			for (itCurCP = sceneState.listZones.begin(); itCurCP != itEndCP; ++itCurCP)
			{
				CZoneNode* pZone = (CZoneNode*)((*itCurCP).m_pRenderObject);
				if (pZone && pZone->Contains(sceneState.vEye))
				{
					rootZone = &(*itCurCP);
					break;
				}
			}
		}

		// set the home zones of all mobile characters. 
		UpdateMovableObjectZone(sceneState, sceneState.listPRBiped);
		UpdateMovableObjectZone(sceneState, sceneState.listPRTransparentBiped);

		// further cull objects by zones. 
		PostRenderObject* outerWorldZone = NULL;
		sceneState.listZones.push_back(PostRenderObject(NULL, 0));
		outerWorldZone = &(*(sceneState.listZones.rbegin()));

		if (rootZone == 0)
		{
			rootZone = outerWorldZone;
		}
		else
		{
			sceneState.m_bOuterWorldZoneVisible = false;
		}

		ClipZone(sceneState, *rootZone, *(pCamera->GetPortalFrustum()));

		// remove objects that are not visible from the queue. 
		RemovePortalOccludedObjects(sceneState.listPRBiped);
		RemovePortalOccludedObjects(sceneState.listPRTransparentBiped);
		RemovePortalOccludedObjects(sceneState.listPRSolidObject);
		RemovePortalOccludedObjects(sceneState.listPRSmallObject);
		// objects that are inside zones are automatically removed from shadow caster list, this is done in previous stage
		// RemoveZoneObjects(sceneState.listShadowCasters);

		if (!sceneState.m_bOuterWorldZoneVisible)
		{
			// the outer world is occluded, we need not render ocean, sky and terrain. 
			sceneState.m_bSkipOcean = true;
			sceneState.m_bSkipTerrain = true;
			sceneState.m_bSkipSky = true;
		}

		PE_ASSERT(CGlobals::GetWorldMatrixStack().size() == 1);
		PE_ASSERT(CGlobals::GetViewMatrixStack().size() == 1);
		PE_ASSERT(CGlobals::GetProjectionMatrixStack().size() == 1);
	}
#endif
}

void CSceneObject::PrepareCameraState(CBaseCamera* pCamera, SceneState &sceneState)
{
	// update the culling planes according to view and projection matrix on top of the global stack
	pCamera->UpdateFrustum();

	// check if camera moves
	static Vector3 s_vLastCameraEyePos(0, 0, 0);
	if ((sceneState.mxView != *(pCamera->GetViewMatrix())) || (s_vLastCameraEyePos != sceneState.vEye) || (sceneState.mxProj != *(pCamera->GetProjMatrix())))
	{
		sceneState.mxView = *(pCamera->GetViewMatrix());
		sceneState.mxProj = *(pCamera->GetProjMatrix());
		s_vLastCameraEyePos = sceneState.vEye;
		sceneState.m_bCameraMoved = true;
		m_globalTerrain->SetCameraPosition(sceneState.vEye.x, sceneState.vEye.z);

		//update block world
		if (m_pBlockWorldClient->IsInBlockWorld())
		{
			CBipedObject* camTarget = GetCurrentPlayer();
			if (camTarget == nullptr)
				m_pBlockWorldClient->OnViewCenterMove(sceneState.vEye.x, sceneState.vEye.y, sceneState.vEye.z);
			else{
				DVector3 vPos = camTarget->GetPosition();
				m_pBlockWorldClient->OnViewCenterMove((float)vPos.x, (float)vPos.y, (float)vPos.z);
			}
				
		}
	}
	else
	{
		sceneState.m_bCameraMoved = false;
	}
}


void CSceneObject::PrepareRenderAllChildren(CBaseCamera* pCamera, SceneState& sceneState)
{
	for (CBaseObject* pChild : GetChildren())
	{
		if (pChild->IsVisible())
		{
			pChild->ViewTouch();
			pChild->PrepareRender(pCamera, &sceneState);
		}
	}
}

int CSceneObject::PrepareRender(CBaseCamera* pCamera, SceneState* pSceneState)
{
	PERF1("RebuildSceneState");
	SceneState& sceneState = *pSceneState;
	///////////////////////////////////////////////////////////////////////////
	//
	// Set the scene state and transformations for the current frame rendering
	//
	//////////////////////////////////////////////////////////////////////////
	sceneState.CleanupSceneState();
	sceneState.EnableGlobalLOD(IsLODEnabled());
	sceneState.vEye = pCamera->GetEyePosition();
	sceneState.vLookAt = pCamera->GetLookAtPosition();
	
	/// Set up a rotation matrix to orient the billboard towards the camera.
	sceneState.BillBoardInfo().UpdateBillBoardInfo(&sceneState);
	
	///////////////////////////////////////////////////////////////////////////
	// update the default material
	sceneState.GetGlobalMaterial().Ambient = GetSunLight().GetSunAmbientHue();
	sceneState.GetGlobalMaterial().Diffuse = GetSunLight().GetSunColor();


	//////////////////////////////////////////////////////////////////////////
	// set initial transformations for both the fixed and programmable pipeline

	// set up transformation stack
	while(! CGlobals::GetWorldMatrixStack().empty())
		CGlobals::GetWorldMatrixStack().pop();
	while(! CGlobals::GetViewMatrixStack().empty())
		CGlobals::GetViewMatrixStack().pop();
	while(! CGlobals::GetProjectionMatrixStack().empty())
		CGlobals::GetProjectionMatrixStack().pop();

	CGlobals::GetWorldMatrixStack().push(*CGlobals::GetIdentityMatrix());
	CGlobals::GetViewMatrixStack().push(*(pCamera->GetViewMatrix()));
	CGlobals::GetProjectionMatrixStack().push(*(pCamera->GetProjMatrix()));

#ifdef USE_DIRECTX_RENDERER
	CGlobals::GetEffectManager()->UpdateD3DPipelineTransform(true,true, true);
#endif
	PrepareCameraState(pCamera, sceneState);

	// always clean up local lights, because all local lights will be rebuilt when rebuilding the scene states.
	CGlobals::GetLightManager()->CleanupLights();

	PrepareTileObjects(pCamera, sceneState);
	PrepareMissileObjects(pCamera, sceneState);
	PreparePortalsAndZones(pCamera, sceneState);
	PrepareRenderAllChildren(pCamera, sceneState);


	//////////////////////////////////////////////////////////////////////////
	//
	// sorting objects in the pool by effects, textures, states, camera distances, etc
	//
	//////////////////////////////////////////////////////////////////////////
	/*
	To reduce the number of state changes and associated overhead, applications can sort and batch rendered objects 
	by shaders or rather "effects" that include combination of shaders, textures and other states. 
	On the other hand it might be beneficial to sort objects by distance for HYPER Z optimizations. 
	Sorting objects by effect/state quite often conflicts with sorting by distance used in front to back rendering.

	Shader switching is one of the most expensive state changes. Batching rendering by vertex shader is always a good idea. 
	When switches between shaders are inevitable, try limiting frequent switches only to recently used smaller shaders 
	as driver and hardware can more effectively cache them. Switching between fixed function and programmable pipeline 
	is in most cases more expensive that switching between equivalent shaders because of the extra driver overhead.
	*/

	/** sort by the camera-object distance: front to back.*/
	std::sort(sceneState.listPRSolidObject.begin(), sceneState.listPRSolidObject.end(), LessPostRenderObj<PostRenderObject>());
	/** sort by the camera-object distance: back to front*/
	std::sort(sceneState.listPRSmallObject.begin(), sceneState.listPRSmallObject.end(), GreaterPostRenderObj<AlphaPostRenderObject>());
	/** sort by the camera-object distance: back to front*/
	std::sort(sceneState.listHeadonDisplayObject.begin(), sceneState.listHeadonDisplayObject.end(), GreaterPostRenderObj<PostRenderObjectWeakPtr>());
	/** for transparent objects */
	std::sort(sceneState.listPRTransparentObject.begin(), sceneState.listPRTransparentObject.end(), GreaterPostRenderObj<AlphaPostRenderObject>());
#ifdef USE_DIRECTX_RENDERER
	/** sort by the primary asset : in this case it's texture */
	std::sort(sceneState.listPRSprite.begin(), sceneState.listPRSprite.end(), CmpBaseObjectAsset<CSpriteObject*>());
#endif
	/** sort by the camera to object distance asset : from front to back. this allows us to skip some if triangle count is too big */
	//std::sort(sceneState.listPRBiped.begin(), sceneState.listPRBiped.end(), LessPostRenderObj_NoTechBatch<PostRenderObject>());
	// Note: use following one will sort by render importance as well. 
	std::sort(sceneState.listPRBiped.begin(), sceneState.listPRBiped.end(), LessPostRenderObj_BipedSort<PostRenderObject>());
	// transparent sort	
	std::sort(sceneState.listPRTransparentBiped.begin(), sceneState.listPRTransparentBiped.end(), GreaterPostRenderObj_NoTechBatch<PostRenderObject>());

	/** sort by distance: front to back */
	if( m_nMaxNumShadowCasters <(int)sceneState.listShadowCasters.size())
	{
		// remove shadows casters that are far from the eye position.
		std::sort(sceneState.listShadowCasters.begin(), sceneState.listShadowCasters.end(), LessPostRenderObj_NoTechBatch<PostRenderObject>());
		int nRemoveNum = (int)sceneState.listShadowCasters.size() - m_nMaxNumShadowCasters;
		SceneState::List_PostRenderObject_Type::iterator itFrom = sceneState.listShadowCasters.end();
		for (int i=0;i<nRemoveNum;++i)
			--itFrom;
		sceneState.listShadowCasters.erase(itFrom, sceneState.listShadowCasters.end());
	}
	// only for effect batch
	std::sort(sceneState.listShadowCasters.begin(), sceneState.listShadowCasters.end(), LessPostRenderObj<PostRenderObject>());
	/** sort by the camera-object distance: back to front*/
	std::sort(sceneState.listPRMissiles.begin(), sceneState.listPRMissiles.end(), GreaterPostRenderObj<AlphaPostRenderObject>());
	sceneState.bIsBatchRender = true;

	std::sort(sceneState.listPostRenderingObjects.begin(), sceneState.listPostRenderingObjects.end(), GreaterPostRenderObj_ByOrder<PostRenderObject>());
	

	if(CGlobals::WillGenReport())
	{
		/** number of shadow casters and receivers are now reported to the console in the format:
		*/
		if(m_bRenderMeshShadow)
		{
			static char sNumReport[50];
			snprintf(sNumReport, 50, "casters:%d receivers:%d", (int)sceneState.listShadowCasters.size(), (int)sceneState.listShadowReceivers.size());
			CGlobals::GetReport()->SetString("shadow:", sNumReport);
		}
	}

	// update the GUI's view projection matrix for calculating 3d GUI object position
	{
		Matrix4 matViewProj;
		ParaMatrixMultiply(&matViewProj, (const Matrix4*)&sceneState.mxView, (const Matrix4*)&sceneState.mxProj);
		CGlobals::GetGUI()->Set3DViewProjMatrix(matViewProj);
	}
	return 0;
}


void CSceneObject::UpdateMovableObjectZone(SceneState &sceneState, SceneState::List_PostRenderObject_Type& listPRBiped)
{
	SceneState::List_PostRenderObject_Type::iterator itCur, itEnd = listPRBiped.end();
	for (itCur = listPRBiped.begin(); itCur != itEnd; ++itCur)
	{
		// we will only use the center of the character to decide which zone it belongs. 
		CBaseObject* pObj = (*itCur).m_pRenderObject;
		if (pObj != 0)
		{
			Vector3 vPos = pObj->GetObjectCenter();
			CZoneNode* pZone = pObj->GetHomeZone();
			if (!(pZone && pZone->Contains(vPos)))
			{
				// if the home zone of the mobile character changes, find the new zone. 
				pObj->SetHomeZone(NULL);
				SceneState::List_PostRenderObject_Type::iterator itCurCP, itEndCP = sceneState.listZones.end();
				for (itCurCP = sceneState.listZones.begin(); itCurCP != itEndCP; ++itCurCP)
				{
					pZone = (CZoneNode*)((*itCurCP).m_pRenderObject);
					if (pZone && pZone->Contains(vPos))
					{
						pObj->SetHomeZone(pZone);
						break;
					}
				}
			}
		}
	}
}


template <class T>
int PerformPortalOcclusionTest(T& renderlist,PostRenderObject& zone,  CPortalFrustum& portalFrustum)
{
	typename T::iterator itCur, itEnd =  renderlist.end();
	for(itCur = renderlist.begin();itCur!=itEnd;++itCur)
	{
		CBaseObject* pObj = (*itCur).m_pRenderObject;
		if((*itCur).m_nOccluded>=0 && pObj->GetHomeZone() == zone.m_pRenderObject)
		{
			IViewClippingObject* pViewClippingObject = pObj->GetViewClippingObject();
			(*itCur).m_nOccluded = (portalFrustum.CanSeeObject_PortalOnly(pViewClippingObject)) ? -1 : 1;
		}
	}
	return 0;
}

int ClipZone(SceneState& sceneState, PostRenderObject& zone, CPortalFrustum& portalFrustum )
{
	// cull objects by the give zone
	PerformPortalOcclusionTest(sceneState.listPRBiped, zone, portalFrustum);
	PerformPortalOcclusionTest(sceneState.listPRSolidObject, zone, portalFrustum);
	PerformPortalOcclusionTest(sceneState.listPRSmallObject, zone, portalFrustum);

	zone.m_nOccluded = -1;
	// this will prevent recursion 
	if(zone.m_pRenderObject)
	{
		CZoneNode* pZone =  (CZoneNode*)(zone.m_pRenderObject);
		RefList::const_iterator itCur, itEnd = pZone->GetRefList().end();
		for(itCur = pZone->GetRefList().begin();itCur!=itEnd && ((*itCur).m_tag == -1);++itCur)
		{
			CPortalNode* pPortal = (CPortalNode*)((*itCur).m_object);
			pPortal->CheckFrameNumber(sceneState.m_nRenderCount); // ensure that derived data are updated. 

			if(portalFrustum.isVisible(pPortal))
			{
				CZoneNode* pTargetZone =  pPortal->GetZone(pZone);
				PostRenderObject* pZoneObj =  sceneState.FindObject(sceneState.listZones, pTargetZone);
				if(pZoneObj && pZoneObj->m_nOccluded == 0)
				{
					portalFrustum.AddPortalCullingPlanes(pPortal);
					ClipZone(sceneState, *pZoneObj, portalFrustum);
					portalFrustum.RemovePortalCullingPlanes(pPortal);
				}
			}
		}
		
	}
	else
	{
		// this will make the outer world zone visible. 
		sceneState.m_bOuterWorldZoneVisible = true;

		// this is the outer world zone. we will add all portals to it.
		SceneState::List_PostRenderObject_Type::iterator itCurCP, itEndCP = sceneState.listZones.end();
		for(itCurCP = sceneState.listZones.begin(); itCurCP !=itEndCP; ++itCurCP)
		{
			CZoneNode* pZone = (CZoneNode*)((*itCurCP).m_pRenderObject);
			// if zone is not the outer world and that its occlusion are undetermined. 
			if(pZone && (*itCurCP).m_nOccluded==0)
			{
				RefList::const_iterator itCur, itEnd = pZone->GetRefList().end();
				for(itCur = pZone->GetRefList().begin();itCur!=itEnd && ((*itCur).m_tag == -1);++itCur)
				{
					CPortalNode* pPortal = (CPortalNode*)((*itCur).m_object);
					pPortal->CheckFrameNumber(sceneState.m_nRenderCount); // ensure that derived data are updated. 

					// if a zone has portal that connect to the outer world and that the portal is not faced away from the frustum
					if(pPortal->GetZoneCount()==1 && portalFrustum.isVisible(pPortal, false))
					{
						// this algorithm is limited in that the camera can see into a room via at most one portal
						portalFrustum.AddPortalCullingPlanes(pPortal);
						ClipZone(sceneState, (*itCurCP), portalFrustum);
						portalFrustum.RemovePortalCullingPlanes(pPortal);
					}
				}
			}
		}
	}
	zone.m_nOccluded = 0;
	return 0;
}

void CSceneObject::UpdateOcean()
{
	CGlobals::GetOceanManager()->ForceUpdateOcean();
}

//-----------------------------------------------------------------------------
// Name: AdvanceScene()
/// Desc: Advances the local animation time by dTimeDelta, and render 
///       it. Basic view culling is also done here.
/// Note: Note that for any multianimation object, it's important that 
///		 AdvanceTime() and Draw() must be called in succession.
//-----------------------------------------------------------------------------
HRESULT CSceneObject::AdvanceScene(double dTimeDelta, int nPipelineOrder)
{
	if(!m_bInitialized)
		return E_FAIL;

	// when game time is paused, also pause the scene delta time
	if (CGlobals::GetFrameRateController(FRC_GAME)->IsPaused() && dTimeDelta > 0.f)
		dTimeDelta = CGlobals::GetFrameRateController(FRC_GAME)->GetElapsedTime();

	globalTime =  (int)(CGlobals::GetGameTime()*1000);
	SceneState& sceneState = *(m_sceneState.get());
	
	sceneState.SetCurrentRenderPipeline(nPipelineOrder);
	RenderDevicePtr pd3dDevice = sceneState.m_pd3dDevice;

#ifdef USE_DIRECTX_RENDERER
	pd3dDevice->SetRenderTarget(1,NULL);
	pd3dDevice->SetRenderTarget(2,NULL);
	pd3dDevice->SetRenderTarget(3,NULL);
#endif
	if(nPipelineOrder == PIPELINE_POST_UI_3D_SCENE)
	{
		//////////////////////////////////////////////////////////////////////////
		// 
		// for mini scene graphs that have their own camera and render order is Post UI. 
		//
		//////////////////////////////////////////////////////////////////////////
		if(IsMiniSceneGraphEnabled())
		{
			int nCount = 0;
			MiniSceneGraphPool_type::iterator itCur, itEnd = m_miniSceneGraphs.end();
			for (itCur = m_miniSceneGraphs.begin(); itCur!=itEnd; ++itCur)
			{
				CMiniSceneGraph* pObj = (*itCur);
				if(pObj && (pObj->GetRenderPipelineOrder() == PIPELINE_POST_UI_3D_SCENE) && pObj->IsVisible() && pObj->IsCameraEnabled() && pObj->IsDirty())
				{
					if(nCount == 0)
					{
						// clear z buffer
						pd3dDevice->Clear( 0L, NULL, D3DCLEAR_ZBUFFER, 0, 1.0f, 0L );
					}
					pObj->Draw((float)dTimeDelta);
					nCount++;
				}
			}
		}
		return S_OK;
	}

	//////////////////////////////////////////////////////////////////////////
	// 
	// for mini scene graphs that have their own render target and camera.
	//
	//////////////////////////////////////////////////////////////////////////
	if(IsMiniSceneGraphEnabled())
	{
		int nCount = 0;
		MiniSceneGraphPool_type::iterator itCur, itEnd = m_miniSceneGraphs.end();
		for (itCur = m_miniSceneGraphs.begin(); itCur!=itEnd; ++itCur)
		{
			CMiniSceneGraph* pObj = (*itCur);
			if(pObj && pObj->IsNeedUpdate() &&  pObj->IsVisible() && pObj->IsCameraEnabled() && pObj->IsDirty() && (pObj->GetRenderPipelineOrder()<PIPELINE_UI))
			{
				pObj->Draw((float)dTimeDelta);
				nCount++;
			}
		}
		// end previous effects, since they may serve different IScene instance.
		if(nCount>0)
			CGlobals::GetEffectManager()->EndEffect();
	}


	if (!m_bGameEnabled || (GetCurrentPlayer() == NULL))
	{
		// this ensures that char in miniscenegraph and owner draw objects are drawn when scene is not enabled. 
		RenderHeadOnDisplay(0);
		{
			sceneState.CleanupSceneState();
			PrepareRenderAllChildren(GetCurrentCamera(), sceneState);
			RenderSelection(RENDER_OWNER_DRAW);
		}
		return E_FAIL;
	}

	//////////////////////////////////////////////////////////////////////////
	//
	// Render objects in the scene state
	//
	//////////////////////////////////////////////////////////////////////////
	
	CAutoCamera* pCamera = (CAutoCamera*)GetCurrentCamera();
	sceneState.dTimeDelta = dTimeDelta;
	++ sceneState.m_nRenderCount; 
	SetFrameNumber(sceneState.m_nRenderCount);
	
	PrepareRender(pCamera, &sceneState);

	//////////////////////////////////////////////////////////////////////////
	// Set some initial states for the normal pipeline
	UpdateFogColor();

	CGlobals::GetEffectManager()->EnableFog(IsFogEnabled());
	CGlobals::GetEffectManager()->SetD3DFogState();
#ifdef USE_DIRECTX_RENDERER
	pd3dDevice->SetMaterial((D3DMATERIAL9*)&(sceneState.GetGlobalMaterial()));


	if(IsUseWireFrame())
	{
		CGlobals::GetRenderDevice()->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );
	}
	else
	{
		CGlobals::GetRenderDevice()->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
	}
#endif
	/////////////////////////////////////////////////////////////////
	// still renderings

	// this ensures that particles are not animated or created in canvas's coordinate system.
	double oldDelta = m_sceneState->dTimeDelta;
	m_sceneState->dTimeDelta = 0; 

	// like render target object
	RenderSelection(RENDER_OWNER_DRAW);

	//////////////////////////////////////////////////////////////////////////
	// end previous effects, since they may serve different IScene instance.
	CGlobals::GetEffectManager()->EndEffect();

	//////////////////////////////////////////////////////////////////////////
	//save scene fog param
	LinearColor defaultAmbColor = GetSunLight().GetSunAmbient();
	LinearColor defaultFogColor = GetFogColor();
	float defaultFogStart = GetFogStart();
	float defaultFogEnd = GetFogEnd();

#ifdef USE_DIRECTX_RENDERER
	//////////////////////////////////////////////////////////////////////////
	// render ocean reflection image to texture
	COceanManager* pOcean = CGlobals::GetOceanManager();
	bool bSimulateOcean = pOcean->UpdateReflectionTexture(sceneState);

	//replace ambient params for under water scene;
	if(bSimulateOcean && pOcean->IsUnderWater() && !pOcean->IsScreenSpaceFog())
	{
		ApplyWaterFogParam();
	}
	
	//////////////////////////////////////////////////////////////////////////
	// render all reflection map in the scene. 
	if(sceneState.m_bCameraMoved || pOcean->IsRedrawStillReflection())
	{
		for (int i=0;i<(int)m_mirrorSurfaces.size();++i)
		{
			if(m_mirrorSurfaces[i]!=0)
			{
				m_mirrorSurfaces[i]->RenderReflectionTexture();
			}
		}
	}
#endif	
	// terrain tessellation goes here. This is CPU time consuming.
	if(!(sceneState.m_bSkipTerrain) && m_globalTerrain->TerrainRenderingEnabled())
		m_globalTerrain->Update();

	//////////////////////////////////////////////////////////////////////////
	// render the shadow map
	if( IsShadowMapEnabled() && (!(sceneState.m_bSkipTerrain) || BlockWorldClient::GetInstance()->GetBlockRenderMethod() == BLOCK_RENDER_FANCY_SHADER))
	{
		sceneState.GetFaceGroups()->Clear();
		sceneState.m_bEnableTranslucentFaceSorting = false;
		RenderShadowMap();
	}

	m_sceneState->dTimeDelta = oldDelta;
	
	//////////////////////////////////////////////////////////////////////////
	//
	// the normal pipeline: terrain, mesh, shadows, bipeds, missiles, sky, ocean, semi-transparent, particles, bounding boxes, etc.
	// TODO: we may put these info a pipeline xml for adjustment. 
	//
	//////////////////////////////////////////////////////////////////////////
	
	BlockWorldClient::GetInstance()->DrawMultiFrameBlockWorld();
	// prepare render target first for deferred shading if fancy graphics is enabled in block world.
	BlockWorldClient::GetInstance()->PrepareAllRenderTargets();
	
	//////////////////////////////////////////////////////////////////////////
	/// draw the terrain
	if(!(sceneState.m_bSkipTerrain))
		RenderSelection(RENDER_GLOBAL_TERRAIN);

	m_pBlockWorldClient->PreRender();
	m_pBlockWorldClient->Render(BlockRenderPass_Opaque);
	m_pBlockWorldClient->Render(BlockRenderPass_AlphaTest);
	if (m_pBlockWorldClient->GetBlockRenderMethod() == BLOCK_RENDER_FANCY_SHADER)
	{
		// we will render sky and fog in post processor. Current sky shader does not render to multiple render target, it will cause glitches. 
		// sceneState.m_bSkipSky = true;
		// then do the deferred lighting
	}
	
	// so that mesh and character that contains translucent faces are sorted. 
	if(!sceneState.GetFaceGroups()->IsEmpty())
	{
		sceneState.GetFaceGroups()->Clear();
	}
	sceneState.m_bEnableTranslucentFaceSorting = true;

	/**
	All potentially visible scene objects in the scene state are put to three major post rendering lists for rendering.
	Each list is sorted according to object-to-camera distance. And then they will be rendered in the following order:
	- solid object list: sorted from front to back. In mesh report, it will display "F->B:%d". Their bounding boxes will be rendered as red boxes.
	- transparent object list: sorted from back to front. In mesh report, it will display "B->F:%d(%d1)".
		where %d is the total number of meshes rendered and that %d1 is the number of meshes that has been occluded during occlusion testing.
		Their bounding boxes will be rendered as blue boxes.
	- Biped list: sorted by primary asset. In mesh report, it will display "Biped:%d". Their bounding boxes will be rendered as green boxes.
	*/
	int nMeshRendered = 0; 
	{
		PERF1("Render_Mesh");
		// object num in potentially visible set from current view point
		
		//////////////////////////////////////////////////////////////////////////
		/// draw solid object rendering list from front to back.
		if(!sceneState.listPRSolidObject.empty())
		{
			RenderSelection(RENDER_MESH_FRONT_TO_BACK);
		}

		//////////////////////////////////////////////////////////////////////////
		// draw smaller object rendering list from back to front.
		if(!sceneState.listPRSmallObject.empty())
		{
			nMeshRendered = RenderSelection(RENDER_MESH_BACK_TO_FRONT);
		}
	}
#ifdef USE_DIRECTX_RENDERER
	//////////////////////////////////////////////////////////////////////////
	/// Sprite post-rendering list
	if(!sceneState.listPRSprite.empty())
	{
		RenderSelection(RENDER_SPRITES);
	}

	//////////////////////////////////////////////////////////////////////////
	/// animate the local animation of the biped object
	
	//////////////////////////////////////////////////////////////////////////
	/// render shadow volumes
	// RenderShadows();
#endif

	//////////////////////////////////////////////////////////////////////////
	/// Draw queued bipeds
	/// we draw biped after its shadow, because we do not want biped to cast shadows on its own
	int nCharacterRendered = 0;
	if(!sceneState.listPRBiped.empty())
	{
		nCharacterRendered += RenderSelection(RENDER_CHARACTERS);
	}

	m_pBlockWorldClient->RenderDeferredLighting();

	RenderSelection(RENDER_SELECTION);

	// draw solid head on
	RenderHeadOnDisplay(3);

	//////////////////////////////////////////////////////////////////////////
	/// Draw SkyBox : we shall render sky after terrain but before transparent meshes, to make z-buffer work better. 
	if (!(sceneState.m_bSkipSky))
		RenderSelection(RENDER_SKY_BOX);
	// this function should be called after sky box is drawn.
	BlockWorldClient::GetInstance()->DrawMultiFrameBlockWorldOnSky();

	//////////////////////////////////////////////////////////////////////////
	/// draw missile objects
	if(!sceneState.listPRMissiles.empty())
	{
		RenderSelection(RENDER_MISSILES);
	}

	//////////////////////////////////////////////////////////////////////////
	// deferred shading so far. 
	m_pBlockWorldClient->Render(BlockRenderPass_ReflectedWater);
	m_pBlockWorldClient->DoPostRenderingProcessing(BlockRenderPass_Opaque);
	

	// draw overlays solid
	RenderHeadOnDisplay(2);
	
	// draw transparent particles
	m_pBatchedElementDraw->DrawBatchedParticles(true);
	
	// alpha blended objects should be rendered after deferred shading with normal rendering order. 
	m_pBlockWorldClient->Render(BlockRenderPass_AlphaBlended);

	if (!sceneState.listPRTransparentBiped.empty())
	{
		nCharacterRendered += RenderSelection(RENDER_TRANSPARENT_CHARACTERS);
	}

	m_pBlockWorldClient->DoPostRenderingProcessing(BlockRenderPass_AlphaBlended);

	// draw the head on display GUI
	RenderHeadOnDisplay(0);

	if(CGlobals::WillGenReport())
	{
		// print the mesh report.
		static char sMeshReport[50];
		snprintf(sMeshReport, 50, "F->B:%d B->F:%d(%d) char:%d(%d) post:%d", 
			(int)sceneState.listPRSolidObject.size(), (int)sceneState.listPRSmallObject.size(), nMeshRendered, (int)(sceneState.listPRBiped.size() + sceneState.listPRTransparentBiped.size()), nCharacterRendered, (int)sceneState.listPRTransparentObject.size());
		sMeshReport[49] = '\0';
		CGlobals::GetReport()->SetString("meshes", sMeshReport);
	}

#ifdef USE_DIRECTX_RENDERER
	//////////////////////////////////////////////////////////////////////////
	/// draw ocean if any
	if(bSimulateOcean){
		pOcean->Render(&sceneState);
	}
	else{
		if(CGlobals::WillGenReport())
			CGlobals::GetReport()->SetValue("ocean tile", 0);
	}
#endif

	sceneState.m_bEnableTranslucentFaceSorting = false;

	//////////////////////////////////////////////////////////////////////////
	// draw transparent object rendering list from back to front.it is rendered after ocean to prevent the ocean color over the particles. 
	if(!sceneState.listPRTransparentObject.empty())
	{
		RenderSelection(RENDER_MESH_TRANSPARENT);
	}

	//////////////////////////////////////////////////////////////////////////
	// render translucent face groups.
	if(!sceneState.GetFaceGroups()->IsEmpty())
	{
		RenderSelection(RENDER_TRANSLUCENT_FACE_GROUPS);
		sceneState.GetFaceGroups()->Clear();
	}

	//////////////////////////////////////////////////////////////////////////
	// render post rendering with render order
	if (!sceneState.listPostRenderingObjects.empty())
	{
		RenderSelection(RENDER_POST_RENDER_LIST);
	}

	//////////////////////////////////////////////////////////////////////////
	/// render all particle system instances after most of the scene have been rendered.
	/// it is rendered after ocean to prevent the ocean color over the particles. since all particles are z write disabled.
	{
		int nParticlesCount = RenderSelection(RENDER_PARTICLES, dTimeDelta);
		if(CGlobals::WillGenReport()){
			/** the particle system instance count and the total particle counts are now reported to the console in the format:
			* nParticles: %2     
			* where %2 is the total particle counts */
			static char sParticleReport[50];
			snprintf(sParticleReport, 50, "%d", nParticlesCount);
			CGlobals::GetReport()->SetString("particles", sParticleReport);
		}
	}

#ifdef USE_DIRECTX_RENDERER
	//////////////////////////////////////////////////////////////////////////
	// call the post processor if any. 
	if(IsPostProcessingEnabled())
	{
		// post processing in the scripting interface 
		if(!m_sPostPorcessorCallbackScript.empty())
		{
			CGlobals::GetNPLRuntime()->GetMainRuntimeState()->DoString(m_sPostPorcessorCallbackScript.c_str(), (int)m_sPostPorcessorCallbackScript.size());
		}
	}
	else if(BlockWorldClient::GetInstance()->GetBlockRenderMethod() != BLOCK_RENDER_FANCY_SHADER)
	{
		//////////////////////////////////////////////////////////////////////////
		// full screen glow effect
		if(IsUsingFullScreenGlow())
		{
			if(!pOcean->IsUnderWater())
			{
				RenderFullScreenGlowEffect();				
			}
			else if(!IsUsingScreenWaveEffect())
			{
				RenderFullScreenGlowEffect();
			}
		}
	}

	if(bSimulateOcean && IsUsingScreenWaveEffect() && pOcean->IsUnderWater())
	{
		RenderScreenWaveEffect();
	}

	if(bSimulateOcean && !pOcean->IsScreenSpaceFog())
	{
		GetSunLight().SetSunAmbient(defaultAmbColor);
		SetFogColor(defaultFogColor);
		SetFogStart(defaultFogStart);
		SetFogEnd(defaultFogEnd);
	}
#endif
	/////////////////////////////////////////////////////////////////////////
	// render the physics debug mode
	if(GetPhysicsDebugDrawMode() != 0)
	{
		PARAVECTOR3 vOffset( -m_vRenderOrigin.x, -m_vRenderOrigin.y, -m_vRenderOrigin.z);
		GetBatchedElementDrawer()->SetRenderOffset(vOffset);
		CGlobals::GetPhysicsWorld()->GetPhysicsInterface()->DebugDrawWorld();
		GetBatchedElementDrawer()->SetRenderOffset(PARAVECTOR3(0,0,0));
	}

	//////////////////////////////////////////////////////////////////////////
	// bounding box
	RenderSelection(RENDER_BOUNDINGBOX);

	if(m_bShowPortalSystem)
		RenderSelection(RENDER_PORTAL_SYSTEM);

	// draw overlays transparent 
	RenderHeadOnDisplay(1);

	//////////////////////////////////////////////////////////////////////////
	// TODO: draw dummy objects

	//////////////////////////////////////////////////////////////////////////
	// TODO: draw debugging objects
	
	//////////////////////////////////////////////////////////////////////////
	// Now begins the GUI
	
	RemoveDeadObjects();

	// make picking result dirty when scene is redrawn.
	auto entity = BufferPickingManager::GetInstance().GetEntity("backbuffer");
	if (entity)
		entity->SetResultDirty();
	return S_OK;
}

template <class T>
void RenderHeadOnDisplayList(T& renderlist, int& nObjCount, SceneState* pSceneState, CGUIText** ppObjUITextDefault, bool bZEnable = true, bool b3DText = false, bool bZWriteEnable = false, bool bRenderSolid = false)
{
	typename T::const_iterator itCurCP, itEnd = renderlist.end();
	for(itCurCP = renderlist.begin(); itCurCP !=itEnd; ++itCurCP)
	{
		CBaseObject* pObj = (*itCurCP).m_pRenderObject;
		if (pObj && pObj->HasHeadOnDisplay() && (pObj->IsHeadOnSolid() == bRenderSolid)  && !IHeadOn3D::DrawHeadOnUI(pObj, nObjCount, pSceneState, ppObjUITextDefault, bZEnable, b3DText, bZWriteEnable))
			break;
	}
}

int CSceneObject::RenderHeadOnDisplay(int nPass)
{
	if(!IsHeadOnDisplayShown())
		return 0;
	SceneState& sceneState = *(m_sceneState.get());
	
	
	CGUIText* pObjUITextDefault = NULL;
	int nTotalCount = 0;
	
	
	float fScaleX, fScaleY;
	CGUIRoot::GetInstance()->GetUIScale(&fScaleX, &fScaleY);
	CGUIRoot::GetInstance()->SetUIScale(1, 1, true, true, false);
	{
		// render mesh object with 3d text. 
		bool bUsePointTexture = CGUIRoot::GetInstance()->GetUsePointTextureFiltering();
		if(!bUsePointTexture)
		{
			CGUIRoot::GetInstance()->SetUsePointTextureFiltering(true);
		}
		int nObjCount = 0;
		if (nPass == 0)
		{
			if (!sceneState.listPRSolidObject.empty())
				RenderHeadOnDisplayList(sceneState.listPRSolidObject, nObjCount, &sceneState, &pObjUITextDefault, true, true);
			if (!sceneState.listPRTransparentObject.empty())
				RenderHeadOnDisplayList(sceneState.listPRTransparentObject, nObjCount, &sceneState, &pObjUITextDefault, true, true);
			if (!sceneState.listPRSmallObject.empty())
				RenderHeadOnDisplayList(sceneState.listPRSmallObject, nObjCount, &sceneState, &pObjUITextDefault, true, true);
			if (!sceneState.listPRBiped.empty())
				RenderHeadOnDisplayList(sceneState.listPRBiped, nObjCount, &sceneState, &pObjUITextDefault, true, true);
			if (!sceneState.listPRTransparentBiped.empty())
				RenderHeadOnDisplayList(sceneState.listPRTransparentBiped, nObjCount, &sceneState, &pObjUITextDefault, true, false);
			IHeadOn3D::DrawHeadOnUI(NULL, nObjCount, &sceneState);
		}
		else if (nPass == 1)
		{
			// draw overlays
			if (!sceneState.listHeadonDisplayObject.empty())
			{
				CPushRenderState state(&sceneState);
				if (sceneState.GetCurrentRenderPipeline() == PIPELINE_COLOR_PICKING)
				{
					nObjCount = 0;
					CGUIRoot::GetInstance()->GetPainter()->Flush();
					sceneState.SetRenderState(RenderState_Overlay_Picking);
					RenderHeadOnDisplayList(sceneState.listHeadonDisplayObject, nObjCount, &sceneState, &pObjUITextDefault, false, true);
					IHeadOn3D::DrawHeadOnUI(NULL, nObjCount, &sceneState);
				}
				else
				{
					nObjCount = 0;
					CGUIRoot::GetInstance()->GetPainter()->Flush();
					sceneState.SetRenderState(RenderState_Overlay_ZPass);
					RenderHeadOnDisplayList(sceneState.listHeadonDisplayObject, nObjCount, &sceneState, &pObjUITextDefault, false, true);
					IHeadOn3D::DrawHeadOnUI(NULL, nObjCount, &sceneState);
					nObjCount = 0;
					sceneState.SetRenderState(RenderState_Overlay);
					RenderHeadOnDisplayList(sceneState.listHeadonDisplayObject, nObjCount, &sceneState, &pObjUITextDefault, true, true);
					IHeadOn3D::DrawHeadOnUI(NULL, nObjCount, &sceneState);
				}
			}
		}
		else if (nPass == 2)
		{
			// draw solid overlays, which is rendered before transparent objects. 
			if (!sceneState.listSolidOverlayObject.empty())
			{
				CPushRenderState state(&sceneState);
				nObjCount = 0;
				CGUIRoot::GetInstance()->GetPainter()->Flush();
				sceneState.SetRenderState(RenderState_Overlay);
				RenderHeadOnDisplayList(sceneState.listSolidOverlayObject, nObjCount, &sceneState, &pObjUITextDefault, true, true, true);
				IHeadOn3D::DrawHeadOnUI(NULL, nObjCount, &sceneState);
			}
		}
		else if (nPass == 3)
		{
			if (!sceneState.listPRSolidObject.empty())
				RenderHeadOnDisplayList(sceneState.listPRSolidObject, nObjCount, &sceneState, &pObjUITextDefault, true, true, false, true);
			if (!sceneState.listPRTransparentObject.empty())
				RenderHeadOnDisplayList(sceneState.listPRTransparentObject, nObjCount, &sceneState, &pObjUITextDefault, true, true, false, true);
			if (!sceneState.listPRSmallObject.empty())
				RenderHeadOnDisplayList(sceneState.listPRSmallObject, nObjCount, &sceneState, &pObjUITextDefault, true, true, false, true);
			if (!sceneState.listPRBiped.empty())
				RenderHeadOnDisplayList(sceneState.listPRBiped, nObjCount, &sceneState, &pObjUITextDefault, true, true, false, true);
			if (!sceneState.listPRTransparentBiped.empty())
				RenderHeadOnDisplayList(sceneState.listPRTransparentBiped, nObjCount, &sceneState, &pObjUITextDefault, true, false, false, true);
			IHeadOn3D::DrawHeadOnUI(NULL, nObjCount, &sceneState);
		}
		nTotalCount+=nObjCount;
		if(!bUsePointTexture)
		{
			CGUIRoot::GetInstance()->SetUsePointTextureFiltering(bUsePointTexture);
		}
	}
	if (nPass == 0)
	{
		// Draw all character's with 2d text facing the camera. 
		int nObjCount = 0;
		if(!sceneState.listPRBiped.empty())
			RenderHeadOnDisplayList(sceneState.listPRBiped, nObjCount, &sceneState, &pObjUITextDefault, true, false);
		if (!sceneState.listPRTransparentBiped.empty())
			RenderHeadOnDisplayList(sceneState.listPRTransparentBiped, nObjCount, &sceneState, &pObjUITextDefault, true, false);
		IHeadOn3D::DrawHeadOnUI(NULL, nObjCount, &sceneState);
		nTotalCount+=nObjCount;
	}
	CGUIRoot::GetInstance()->SetUIScale(fScaleX, fScaleY, true, true, false);
	return nTotalCount;
}

CBaseCamera* ParaEngine::CSceneObject::GetCurrentCamera()
{
	return m_pCurrentCamera.get();
}

void ParaEngine::CSceneObject::SetCurrentCamera(CBaseCamera* pCamera)
{
	m_pCurrentCamera = pCamera;
}

CSkyMesh* CSceneObject::GetCurrentSky()
{	
	if(m_skymeshes.empty())
		return NULL;
	else
		return m_skymeshes.back();
}
bool CSceneObject::CreateSkyBox(const string& strObjectName, AssetEntity& MeshAsset, float fScaleX, float fScaleY,float fScaleZ, float fHeightOffset)
{
	// if the object already exists, just select it as the current sky.
	SkyMeshPool_type::iterator itCurCP, itEndCP = m_skymeshes.end();

	for( itCurCP = m_skymeshes.begin(); itCurCP != itEndCP; ++ itCurCP)
	{
		if((*itCurCP)->GetIdentifier() == strObjectName)
		{
			ref_ptr<CSkyMesh> pTmp(*itCurCP);
			m_skymeshes.erase(itCurCP);
			m_skymeshes.push_back(pTmp.get());
			// sky selected
			return true;
		}
	}

	// if the sky name does not exist, we will create it and select it as the current sky mesh. 
	if(!MeshAsset.IsValid())
	{
		return false;
	}
	else
	{
		CSkyMesh * pSky = new CSkyMesh();
		if( pSky == NULL )
			return false;
		pSky->CreateSkyStaticMesh(strObjectName,MeshAsset,fScaleX, fScaleY, fScaleZ, fHeightOffset );
		m_skymeshes.push_back(pSky);
		// sky added and selected as current.
		return true; 
	}

	return false;
}

bool CSceneObject::DeleteSkyBox(const string& strObjectName)
{
	if(strObjectName=="")
	{
		m_skymeshes.clear();
	}
	return true;
}

CSunLight& CSceneObject::GetSunLight()
{	
	/** only used when there is no sky created. */
	return *m_pSunLight;
}

//--------------------------------------------------------------------------
// Render the shadows for bipeds objects only
// this function must be called by the advances time() function. it assumes
// that sunlight in sky mesh, camera, post rendering bipeds have all been saved
// in the current sceneState. It then performs step 2 of the rendering pipeline(see below)
	/** Shadow volume rendering
	1. Render the portion of the scene which will receive shadows.
	2. For each active light in the scene. { 
		2.1. Build shadow volume for each enabled shadow caster, with regard to the current light.
		2.2. Render the shadow volumes to stencil buffer twice to get the shaded region.
		2.3. Render the shadow (i.e. alpha blending a big quad of the shadow color to the screen), with regard to the stencil buffer
	}
	3. Render the portion of the scene that does not receive shadows.
	*/
//--------------------------------------------------------------------------
void CSceneObject::RenderShadows()
{
#ifdef USE_DIRECTX_RENDERER
	SceneState& sceneState = *(m_sceneState.get());
	CAutoCamera* pCamera =  (CAutoCamera*)GetCurrentCamera();
	LPDIRECT3DDEVICE9 pd3dDevice = sceneState.m_pd3dDevice;

	if(m_bRenderMeshShadow)
	{// for each light, we cast shadows
		//LPDIRECT3DSTATEBLOCK9 pStateBlock;
		//pd3dDevice->CreateStateBlock( D3DSBT_ALL, &pStateBlock );
		//pStateBlock->Capture();
		ShadowVolume* pShadowVolume = NULL;
		if(m_ShadowVolumes.empty())
		{
			m_ShadowVolumes.push_back(new ShadowVolume());
		}
		pShadowVolume = m_ShadowVolumes.front();
		pShadowVolume->Reset();

		pd3dDevice->SetVertexShader(NULL);
		pd3dDevice->SetPixelShader(NULL);
		pd3dDevice->SetTexture(0, NULL);

		/** update the shadow volume global information, which is shared by all shadow volumes in the current frame */
		{ 
			Matrix4 matModelview;
			Matrix4 matProjection;
			ParaViewport  viewport;
			pd3dDevice->GetTransform(D3DTS_PROJECTION, matProjection.GetConstPointer());
			pd3dDevice->GetTransform(D3DTS_VIEW, matModelview.GetConstPointer());
			pd3dDevice->GetViewport(reinterpret_cast<D3DVIEWPORT9*> (&viewport));

			pShadowVolume->UpdateProjectionInfo(&viewport, &matProjection, &matModelview);
		}

		/** for each light source that cast shadows */
		{
			// currently, we only have one light, i.e. the Sun
			LightParams vShadowLight;
			Para3DLight* pLight =  GetSunLight().GetD3DLight();
			vShadowLight.Position = pLight->Position;
			vShadowLight.Direction = pLight->Direction;
			vShadowLight.Diffuse = pLight->Diffuse;
			vShadowLight.Range = pLight->Range;
			vShadowLight.bIsDirectional = true;
			
			/// update shadow volume information for this light
			pShadowVolume->SetLightParams(&vShadowLight);
			pShadowVolume->ReCalculateOcclusionPyramid(pCamera);
			
			// set world transform as identity, since all volume shadows are in world coordinates
			pd3dDevice->SetTransform(D3DTS_WORLD, Matrix4::IDENTITY.GetConstPointer());
								
			/// clear stencil buffer
			pd3dDevice->Clear( 0L, NULL, D3DCLEAR_STENCIL, 0, 0, 0L );

			/** set up render states */
			// Disable z-buffer writes (note: z-testing still occurs), and enable the
			// stencil-buffer
			pd3dDevice->SetRenderState( D3DRS_ZWRITEENABLE,  FALSE );
			pd3dDevice->SetRenderState( D3DRS_STENCILENABLE, TRUE );

			// Make sure that no pixels get drawn to the frame buffer
			pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
			pd3dDevice->SetRenderState( D3DRS_ALPHATESTENABLE, FALSE);
			pd3dDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_ZERO );
			pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_ONE );
			
			// Do not bother with interpolating color
			//pd3dDevice->SetRenderState( D3DRS_SHADEMODE,     D3DSHADE_FLAT );

			// Set up stencil compare function, reference value, and masks.
			// Stencil test passes if ((ref & mask) cmpfn (stencil & mask)) is true.
			pd3dDevice->SetRenderState( D3DRS_STENCILFUNC,  D3DCMP_ALWAYS );
			pd3dDevice->SetRenderState( D3DRS_STENCILREF,       0x1 );			// 0x0
			pd3dDevice->SetRenderState( D3DRS_STENCILMASK,      0xffffffff );
			pd3dDevice->SetRenderState( D3DRS_STENCILWRITEMASK, 0xffffffff );

			/// Build and render shadow volume for these bipeds
			if(!sceneState.listShadowCasters.empty())
			{
				SceneState::List_PostRenderObject_Type::iterator itCurCP, itEndCP = sceneState.listShadowCasters.end();
				
				for( itCurCP = sceneState.listShadowCasters.begin(); itCurCP != itEndCP; ++ itCurCP)
				{
					if((*itCurCP).m_pRenderObject->IsBiped())
					{
						// use shadow volume for biped shadow casters
						CBipedObject* pBiped = (CBipedObject*)((*itCurCP).m_pRenderObject);
						// clear shadow volume
						pShadowVolume->Reset();
						// build shadow volume
						pBiped->BuildShadowVolume(&sceneState, pShadowVolume, &vShadowLight);
						// render shadow volume
						if(pShadowVolume->m_nNumVertices>0)
						{
							if(pShadowVolume->m_shadowMethod == ShadowVolume::SHADOW_Z_PASS)
							{
								/// Z-Pass 
								if(( CGlobals::GetDirectXEngine().m_d3dCaps.StencilCaps & D3DSTENCILCAPS_TWOSIDED ) != 0 )
								{
									// With 2-sided stencil, we can avoid rendering twice:
									pd3dDevice->SetRenderState( D3DRS_STENCILFAIL,  D3DSTENCILOP_KEEP );
									pd3dDevice->SetRenderState( D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP );
									pd3dDevice->SetRenderState( D3DRS_STENCILPASS,      D3DSTENCILOP_INCR );

									pd3dDevice->SetRenderState( D3DRS_TWOSIDEDSTENCILMODE, TRUE );
									pd3dDevice->SetRenderState( D3DRS_CCW_STENCILFUNC,  D3DCMP_ALWAYS );
									pd3dDevice->SetRenderState( D3DRS_CCW_STENCILZFAIL, D3DSTENCILOP_KEEP );
									pd3dDevice->SetRenderState( D3DRS_CCW_STENCILFAIL,  D3DSTENCILOP_KEEP );
									pd3dDevice->SetRenderState( D3DRS_CCW_STENCILPASS, D3DSTENCILOP_DECR );

									pd3dDevice->SetRenderState( D3DRS_CULLMODE,  D3DCULL_NONE );

									// Draw both sides of shadow volume in stencil/z only
									pShadowVolume->Render( &sceneState );

									pd3dDevice->SetRenderState( D3DRS_TWOSIDEDSTENCILMODE, FALSE );
								}
								else
								{
									// render front faces on z pass
									pd3dDevice->SetRenderState( D3DRS_CULLMODE,  D3DCULL_CCW );
									pd3dDevice->SetRenderState( D3DRS_STENCILFAIL,  D3DSTENCILOP_KEEP );
									pd3dDevice->SetRenderState( D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP );
									pd3dDevice->SetRenderState( D3DRS_STENCILPASS,      D3DSTENCILOP_INCR );

									// Draw front-side of shadow volume in stencil/z only
									pShadowVolume->Render( &sceneState );

									// Now reverse cull order so back sides of shadow volume are written.
									pd3dDevice->SetRenderState( D3DRS_CULLMODE,   D3DCULL_CW );
									pd3dDevice->SetRenderState( D3DRS_STENCILPASS, D3DSTENCILOP_DECR );

									// Draw back-side of shadow volume in stencil/z only
									pShadowVolume->Render( &sceneState );
								}
							}
							else if(pShadowVolume->m_shadowMethod == ShadowVolume::SHADOW_Z_FAIL)
							{
								/// Z-Fail
								{
									// render front faces on z pass
									pd3dDevice->SetRenderState( D3DRS_CULLMODE,  D3DCULL_CW );
									pd3dDevice->SetRenderState( D3DRS_STENCILFAIL,  D3DSTENCILOP_KEEP );
									pd3dDevice->SetRenderState( D3DRS_STENCILZFAIL, D3DSTENCILOP_INCR );
									pd3dDevice->SetRenderState( D3DRS_STENCILPASS,  D3DSTENCILOP_KEEP);

									// Draw front-side of shadow volume in stencil/z only
									pShadowVolume->Render( &sceneState );

									// Now reverse cull order so back sides of shadow volume are written.
									pd3dDevice->SetRenderState( D3DRS_CULLMODE,   D3DCULL_CCW );
									pd3dDevice->SetRenderState( D3DRS_STENCILZFAIL, D3DSTENCILOP_DECR );

									// Draw back-side of shadow volume in stencil/z only
									pShadowVolume->Render( &sceneState );
								}
							}
						}
					}
					
				}
			}
			/** Draw the big square as shadow.
			*/
			// Set render states (disable z-buffering, enable stencil, disable fog, and
			// turn on alpha blending)
			pd3dDevice->SetRenderState( D3DRS_ZENABLE,          FALSE );
			pd3dDevice->SetRenderState( D3DRS_STENCILENABLE,    TRUE );
			pd3dDevice->SetRenderState( D3DRS_FOGENABLE,        FALSE );
			pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, TRUE );
			pd3dDevice->SetRenderState( D3DRS_SRCBLEND,  D3DBLEND_SRCALPHA );
			pd3dDevice->SetRenderState( D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA );
			//pd3dDevice->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_GOURAUD );
			pd3dDevice->SetRenderState( D3DRS_CULLMODE,  D3DCULL_CCW );

			pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG1, D3DTA_TEXTURE );
			pd3dDevice->SetTextureStageState( 0, D3DTSS_COLORARG2, D3DTA_DIFFUSE );
			pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_MODULATE );
			pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG1, D3DTA_TEXTURE );
			pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAARG2, D3DTA_DIFFUSE );
			pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_MODULATE );

			// Only write where stencil val >= 1 (count indicates # of shadows that
			// overlap that pixel)
			pd3dDevice->SetRenderState( D3DRS_STENCILWRITEMASK,      0x0 );// do not write to stencil buffer
			pd3dDevice->SetRenderState( D3DRS_STENCILFUNC, D3DCMP_LESSEQUAL ); // D3DCMP_EQUAL
			pd3dDevice->SetRenderState( D3DRS_STENCILREF,  0x1 );	// 0x0
			pd3dDevice->SetRenderState( D3DRS_STENCILPASS, D3DSTENCILOP_KEEP );
			pd3dDevice->SetRenderState( D3DRS_STENCILFAIL,  D3DSTENCILOP_KEEP );
			pd3dDevice->SetRenderState( D3DRS_STENCILZFAIL, D3DSTENCILOP_KEEP );

			// Draw a big, gray square
			pd3dDevice->SetFVF( SHADOWVERTEX::FVF );
			pd3dDevice->SetMaterial((D3DMATERIAL9*)&(sceneState.GetGlobalMaterial()));
			pd3dDevice->SetStreamSource( 0, sceneState.pAssetManager->GetShadowSquareVB(), 0, sizeof(SHADOWVERTEX) );
			RenderDevice::DrawPrimitive( pd3dDevice, RenderDevice::DRAW_PERF_TRIANGLES_MESH,  D3DPT_TRIANGLESTRIP, 0, 2 );

			// Restore render states
			pd3dDevice->SetRenderState( D3DRS_ZENABLE,          TRUE );
			pd3dDevice->SetRenderState( D3DRS_STENCILENABLE,    FALSE );
			pd3dDevice->SetRenderState( D3DRS_FOGENABLE,        TRUE );
			pd3dDevice->SetRenderState( D3DRS_ALPHABLENDENABLE, FALSE );
		}//for each light 
		//pStateBlock->Apply();
		//SAFE_RELEASE(pStateBlock);
	}//if(m_bRenderMeshShadow)
#endif
}

//---------------------------------------------------------------------
/// desc: GetCurrentPlayer, the first element in the m_player list.
/// if the list is empty, NULL is returned
//---------------------------------------------------------------------
CBipedObject*	CSceneObject::GetCurrentPlayer()
{	
	return m_currentplayer;
}

CBipedObject*	CSceneObject::GetNextPlayer(CBipedObject* pLast)
{
	if(pLast)
	{
		auto itEndCP = m_pTileRoot->m_listSolidObj.end();
		bool bFound = false;
		int i=0;
		for(auto itCurCP = m_pTileRoot->m_listSolidObj.begin(); itCurCP != itEndCP && i<2;)
		{
			if(bFound && ((*itCurCP)->IsBiped()))
			{
				if((*itCurCP)!=pLast)
					return (CBipedObject*)(*itCurCP);
				else
					break;
			}
			if((*itCurCP) == pLast)
				bFound = true;
			if((++itCurCP) == itEndCP)
			{
				if(!bFound)
					break;
				itCurCP = m_pTileRoot->m_listSolidObj.begin();
				++i;
			}
		}
		/// the NULL is returned if the given character is not found.
		return NULL;
	}
	else
		return GetCurrentPlayer();
}

void CSceneObject::PauseScene(bool bEnable)
{
	if(m_bGamePaused != bEnable)
	{
		m_bGamePaused = bEnable;
	}
}

void CSceneObject::EnableScene(bool bEnable)
{
	if(m_bGameEnabled != bEnable)
	{
		m_bGameEnabled = bEnable;
	}
}

Vector3 CSceneObject::GetRenderOrigin()
{
	return m_vRenderOrigin;
}

void CSceneObject::RegenerateRenderOrigin(const Vector3& vPos)
{
	// TODO: use another way, such as (vPos/1000*1000+vPos%1000);
	m_vRenderOrigin.x = (float)((int)vPos.x);
	m_vRenderOrigin.y = (float)((int)vPos.y);
	m_vRenderOrigin.z = (float)((int)vPos.z);

	//m_vRenderOrigin.x = round(vPos.x);
	//m_vRenderOrigin.y = round(vPos.y);
	//m_vRenderOrigin.z = round(vPos.z);
}
CMissileObject* CSceneObject::NewMissile()
{
	CMissileObject* pMissile = NULL;
	// TODO: to crease performance, we can move all exploded missile to the end of the list.
	// and search from the back first.
	/** find in memory if there is any exploded missile which can be reused.*/
	MissileObjectPool_Type::const_iterator itCurCP, itEndCP = m_missiles.end();

	for( itCurCP = m_missiles.begin(); itCurCP != itEndCP; ++ itCurCP)
	{
		pMissile = (*itCurCP);
		if(pMissile->IsExploded())
		{
			return pMissile; // found exploded missile
		}
	}
	// if there is no exploded missile, we will create a new one
	pMissile = new CMissileObject();
	if(pMissile)
		m_missiles.push_back(pMissile);
	return pMissile;

}

bool CSceneObject::HandleUserInput()
{
	if(!m_bInitialized || IsBlockInput())
		return false;
	bool bHasKeyEvent = false;

	MouseEvent mouse_down_event(0,0,0,-1);
	MouseEvent mouse_up_event(0,0,0,-1);
	MouseEvent mouse_move_event(0,0,0,-1);
	MouseEvent mouse_click_event(0,0,0, -1);
	MouseEvent mouse_wheel_event(0, 0, 0, -1);
	KeyEvent key_event(0,0);

	CDirectMouse* pMouse=CGlobals::GetGUI()->m_pMouse;
	if (pMouse && CGlobals::GetGUI()->m_events.size()>0)
	{
		GUIMsgEventList_type::iterator iter=CGlobals::GetGUI()->m_events.begin(),iterend=CGlobals::GetGUI()->m_events.end();
		MSG *pMsg;
		for (;iter!=iterend;++iter)
		{
			pMsg=&(*iter);
			//////////////////////////////////////////////////////////////////////////
			//
			// handle mouse event
			//
			//////////////////////////////////////////////////////////////////////////
			if ( !m_bGamePaused && m_event->IsMapTo(pMsg->message,EM_MOUSE))
			{
				static bool g_lastMouseLocked = false;
				g_lastMouseLocked |= pMouse->IsLocked();
				static MouseEvent g_lastMouseDown(0,0,0);
				if ((mouse_down_event.m_nEventType==-1) && m_event->IsMapTo(pMsg->message,EM_MOUSE_DOWN))
				{
					g_lastMouseDown.m_MouseState = pMsg->message;
					g_lastMouseDown.m_x = pMsg->pt.x;
					g_lastMouseDown.m_y = pMsg->pt.y;
					g_lastMouseLocked = pMouse->IsLocked();
					
					mouse_down_event.m_MouseState = pMsg->message;
					mouse_down_event.m_x = pMsg->pt.x;
					mouse_down_event.m_y = pMsg->pt.y;
					mouse_down_event.m_nEventType = EVENT_MOUSE_DOWN;
					//OUTPUT_LOG("MouseDown %d %d \n", mouse_move_event.m_x, mouse_move_event.m_y);
				}
				else if((mouse_move_event.m_nEventType==-1) && m_event->IsMapTo(pMsg->message,EM_MOUSE_MOVE))
				{
					mouse_move_event.m_MouseState = pMsg->message;
					
					// please note: mouse move is delta value. 
					mouse_move_event.m_x = pMouse->GetMouseXDeltaSteps();
					mouse_move_event.m_y = pMouse->GetMouseYDeltaSteps();
					
					mouse_move_event.m_nEventType = EVENT_MOUSE_MOVE;
					//OUTPUT_LOG("MouseMove %d %d \n", mouse_move_event.m_x, mouse_move_event.m_y);
				}
				else if(mouse_up_event.m_nEventType == -1 && m_event->IsMapTo(pMsg->message,EM_MOUSE_UP))
				{
					mouse_up_event.m_MouseState = pMsg->message;
					mouse_up_event.m_x = pMsg->pt.x;
					mouse_up_event.m_y = pMsg->pt.y;
					mouse_up_event.m_nEventType = EVENT_MOUSE_UP;

					if ((mouse_click_event.m_nEventType==0) && !g_lastMouseLocked/** this ensures that the mouse is not locked during the mouse down and mouse up event*/ )
					{
						// Note: mouse click is rarely used, since it can be produced in NPL via mouse down move and up. However, lazy NPL programmer can still use it if they do not like to write other mouse handlers in NPL.
						/** @def 3d scene will not handle mouse drag. Only the distance between the mouse down position and mouse up position is smaller 
						than this value is considered a mouse click, and is sent to the scripting interface. 
						Note: in the Auto camera class, we have locked the mouse when a mouse drag operation is performed, hence the delta will always be 0 or 1
						*/
						// TODO: we should use mouse drag distance in mouse move function to determine whether it is a click event, like what I does in the NPL. However this is also fine. 
#define MOUSE_DRAG_INSTANCE_IN_PIXEL	1
						if((abs(g_lastMouseDown.m_x-pMsg->pt.x)+abs(g_lastMouseDown.m_y-pMsg->pt.y))< MOUSE_DRAG_INSTANCE_IN_PIXEL)
						{
							mouse_click_event.m_MouseState = pMsg->message;
							mouse_click_event.m_x = pMsg->pt.x;
							mouse_click_event.m_y = pMsg->pt.y;
							mouse_click_event.m_nEventType = EVENT_MOUSE;
						}
					}
				}
				else if ((mouse_wheel_event.m_nEventType == -1) && m_event->IsMapTo(pMsg->message, EM_MOUSE_WHEEL))
				{
					mouse_wheel_event.m_MouseState = pMsg->message;

					// please note: mouse move is delta value. 
					int nDelta = ((int32)(pMsg->lParam)) / 120;
					if (nDelta == 0)
						nDelta = ((int32)(pMsg->lParam)) > 0 ? 1 : -1;
					mouse_wheel_event.m_x = nDelta;

					mouse_wheel_event.m_nEventType = EVENT_MOUSE_WHEEL;
				}
				
			}
			//////////////////////////////////////////////////////////////////////////
			//
			// handle key events
			//
			//////////////////////////////////////////////////////////////////////////
			else if (m_event->IsMapTo(pMsg->message,EM_KEY))
			{
				bool bIsKeyDown = IS_KEYDOWN(pMsg->lParam);
				if(bIsKeyDown)
				{
					key_event.m_KeyState = (DWORD)pMsg->lParam;
					key_event.m_nKey = pMsg->message;
					key_event.m_nEventType = EVENT_KEY;
					CGlobals::GetEventsCenter()->FireEvent(key_event);
				}
				else
				{
					key_event.m_KeyState = (DWORD)pMsg->lParam;
					key_event.m_nKey = pMsg->message;
					key_event.m_nEventType = EVENT_KEY_UP;
					CGlobals::GetEventsCenter()->FireEvent(key_event);
				}
			}
		}
	}

	if(m_dwEnableMouseEvent == 0xffff && (mouse_click_event.m_nEventType>0))
	{
		//////////////////////////////////////////////////////////////////////////
		// scene game object's on click event is handled here
		ParaScripting::ParaObject obj = ParaScripting::ParaScene::MousePick(GetOnClickDistance(), "biped");
		if(obj.IsCharacter())
		{
			IGameObject* gameobj = obj.m_pObj->QueryIGameObject();
			if(gameobj!=NULL)
			{
				gameobj->On_Click(mouse_click_event.m_MouseState, mouse_click_event.m_x, mouse_click_event.m_y);
			}
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// user registered custom mouse events are handled here: in the order mouse down, move, up, click
	
	// call mouse down event handlers
	if(mouse_down_event.m_nEventType>=0) 
		CGlobals::GetEventsCenter()->FireEvent(mouse_down_event);
	// call mouse click event handlers
	if(mouse_move_event.m_nEventType>=0) {
		CGlobals::GetEventsCenter()->FireEvent(mouse_move_event);
	}
	else{
		if(pMouse->IsLocked() && (pMouse->GetMouseXDeltaSteps()!=0 || pMouse->GetMouseYDeltaSteps()!=0) && CGlobals::GetGUI()->IsActive())
		{
			mouse_move_event.m_MouseState = 0;

			// please note: mouse move is delta value. 
			mouse_move_event.m_x = pMouse->GetMouseXDeltaSteps();
			mouse_move_event.m_y = pMouse->GetMouseYDeltaSteps();

			mouse_move_event.m_nEventType = EVENT_MOUSE_MOVE;
			CGlobals::GetEventsCenter()->FireEvent(mouse_move_event);
		}
	}
	// call mouse up event handlers
	if(mouse_up_event.m_nEventType>=0) 
		CGlobals::GetEventsCenter()->FireEvent(mouse_up_event);
	// call mouse click event handlers
	if(mouse_click_event.m_nEventType>=0) 
		CGlobals::GetEventsCenter()->FireEvent(mouse_click_event);
	if (mouse_wheel_event.m_nEventType >= 0)
		CGlobals::GetEventsCenter()->FireEvent(mouse_wheel_event);
	//if(bHasKeyEvent)
	//{ 
	//	// key handlers
	//	CGlobals::GetEventsCenter()->FireKeyEvents(key_event);
	//}
	return true;
}

int CSceneObject::RenderCharacters(SceneState& sceneState, SceneState::List_PostRenderObject_TrackRef_Type& listPRBiped)
{
	int nObjCount = 0;
	if (!listPRBiped.empty())
	{
		CBaseObject* pPlayer = GetCurrentPlayer();
#ifdef USE_DIRECTX_RENDERER
		//render drop shadow
		if (!IsShadowMapEnabled() && !m_bRenderMeshShadow && m_renderDropShadow && !sceneState.IsShadowPass())
		{
			m_dropShadowRenderer->BeginFrame();

			SceneState::List_PostRenderObject_Type::iterator itCurCP, itEndCP = listPRBiped.end();
			for (itCurCP = listPRBiped.begin(); itCurCP != itEndCP; ++itCurCP)
			{
				//Add drop shadow for each character instance,however,biped objects may contain special effect
				//which don't cast shadow. IsShadowCaster is true by default for all biped, so we use GetMaxSpeed()
				//as a reference for a temporary solution. If biped doesn't have "walk" animation,then we consider it
				//as special effect.
				if (itCurCP->m_pRenderObject != 0 && !((*itCurCP).IsOccluded())
					&& sceneState.CheckAddRenderCount((*itCurCP).m_pRenderObject->GetRenderImportance())
					&& itCurCP->m_pRenderObject->IsShadowCaster())
				{
					if (itCurCP->m_pRenderObject->IsBiped())
					{
						CBipedObject* pBiped = (CBipedObject*)itCurCP->m_pRenderObject;
						if (pBiped->GetMaxSpeed() <= 0 || pBiped->GetOpacity()==0.f)
							continue;
					}

					if ((*itCurCP).m_pRenderObject == pPlayer && !CanShowMainPlayer())
						continue;

					CBipedStateManager* pSm = itCurCP->m_pRenderObject->GetBipedStateManager();
					if (pSm && pSm->IsMounted())
						continue;

					Vector3 worldPos = (*itCurCP).m_pRenderObject->GetPosition();
					Vector3 renderPos = (*itCurCP).m_pRenderObject->GetRenderOffset();

					CShapeAABB aabb;
					(*itCurCP).m_pRenderObject->GetAABB(&aabb);
					float width = aabb.GetWidth();
					float depth = aabb.GetDepth();
					float radius = (width > depth) ? depth : width;

					m_dropShadowRenderer->AddInstance(worldPos, renderPos, radius*1.8f);
				}
			}
			m_dropShadowRenderer->Render();
		}
#endif
		// run the occlusion test pass, if there are more than 8 small objects to render. 
		// PerformOcclusionTest(sceneState.listPRBiped, sceneState, 8);
		int nStartCharCount = RenderDevice::GetPerfCount(RenderDevice::DRAW_PERF_TRIANGLES_CHARACTER);

		CGlobals::GetEffectManager()->BeginEffect(TECH_CHARACTER, &(sceneState.m_pCurrentEffect));

		bool bHasPlayer = false;
		/** sort by the primary asset : in this case it's MA */
		for (auto& item : listPRBiped)
		{
			if (item.m_pRenderObject != 0 && !(item.IsOccluded()) && sceneState.CheckAddRenderCount(item.m_pRenderObject->GetRenderImportance()))
			{
				if (item.m_pRenderObject == pPlayer && !CanShowMainPlayer())
					continue;
				if (RenderDevice::GetPerfCount(RenderDevice::DRAW_PERF_TRIANGLES_CHARACTER) < (nStartCharCount + GetMaxCharTriangles()))
				{
					sceneState.SetCameraToCurObjectDistance(item.m_fObjectToCameraDistance);
					item.m_pRenderObject->Draw(&sceneState);
					++nObjCount;
				}
			}
		}
	}
	return nObjCount;
}

int CSceneObject::RenderSelection(DWORD dwSelection, double dTimeDelta)
{
	SceneState& sceneState = *(m_sceneState.get());
	int nObjCount = 0;

	sceneState.m_nCurRenderGroup = dwSelection;
	if (CHECK_SELECTION(RENDER_SKY_BOX))
	{
		CSkyMesh * pSky = GetCurrentSky();
		if (pSky != 0 && IsRenderSky())
		{
			pSky->Draw(&sceneState);
			++nObjCount;
		}
	}
	if (CHECK_SELECTION(RENDER_GLOBAL_TERRAIN) && m_globalTerrain->TerrainRenderingEnabled())
	{
		if (m_globalTerrain->IsTerrainEngineEnabled() && m_globalTerrain->TerrainRenderingEnabled())
		{
			CGlobals::GetEffectManager()->BeginEffect(TECH_TERRAIN, &(sceneState.m_pCurrentEffect));
			m_globalTerrain->Render();
			++nObjCount;
		}
	}
#ifdef USE_DIRECTX_RENDERER
	if(CHECK_SELECTION(RENDER_GEN_SM_TERRAIN) && m_globalTerrain->TerrainRenderingEnabled())
	{
		if (m_globalTerrain->IsTerrainEngineEnabled() && m_globalTerrain->TerrainRenderingEnabled())
		{
			// this is very tricky. the draw sequence must be: RENDER_GLOBAL_TERRAIN(reflection), RENDER_GEN_SM_TERRAIN, RENDER_GLOBAL_TERRAIN(normal)
			// otherwise,the tessellation may use wrong camera settings.
			if(CGlobals::GetEffectManager()->BeginEffect(TECH_TERRAIN, &(sceneState.m_pCurrentEffect)))
			{
				if(sceneState.m_pCurrentEffect)
				{
					m_globalTerrain->Render();
					++nObjCount;
				}
			}
		}
	}
#endif 
	if(CHECK_SELECTION(RENDER_MESH_FRONT_TO_BACK))
	{
		/// draw solid object rendering list from front to back.
		if(!sceneState.listPRSolidObject.empty())
		{
			sceneState.fAlphaFactor=1.0f;
			nObjCount += RenderList(sceneState.listPRSolidObject, sceneState);
		}
	}
	if(CHECK_SELECTION(RENDER_MESH_TRANSPARENT))
	{
		//////////////////////////////////////////////////////////////////////////
		// render without hardware occlusion testing
		nObjCount += RenderList(sceneState.listPRTransparentObject, sceneState);
	}
	if(CHECK_SELECTION(RENDER_MESH_BACK_TO_FRONT))
	{
		// draw transparent object rendering list from back to front.
		if(!sceneState.listPRSmallObject.empty())
		{
			// run the occlusion test pass, if there are more than 25 small objects to render. 
			PerformOcclusionTest(sceneState.listPRSmallObject, sceneState, 25);
			
			//////////////////////////////////////////////////////////////////////////
			// render normally
			nObjCount += RenderList(sceneState.listPRSmallObject, sceneState);
			sceneState.fAlphaFactor=1.0f;
		}
	}

	//if(CHECK_SELECTION(RENDER_PLAYER))
	//{
	//	//////////////////////////////////////////////////////////////////////////
	//	// OBSOLETED: we can use more advanced effects for the main player, such as specular lighting and normal mapping, cartoon rendering, etc.
	//	// currently, it is the same as the normal character
	//	//////////////////////////////////////////////////////////////////////////
	//	CBipedObject* pPlayer = GetCurrentPlayer();
	//	if(pPlayer!=0 && CanShowMainPlayer() && pPlayer->IsVisible())
	//	{
	//		DWORD techHandle = TECH_CHARACTER;
	//		CAutoCamera* pCamera = (CAutoCamera*)GetCurrentCamera();
	//		
	//		Vector3 vEye = sceneState.vEye;
	//		Vector3 vCharPos = pPlayer->GetPosition();
	//		if(vEye.y>(vCharPos.y+pPlayer->GetPhysicsHeight()))
	//			vCharPos.y = vCharPos.y+pPlayer->GetPhysicsHeight();
	//		else if(vEye.y>vCharPos.y)
	//			vCharPos.y = vEye.y;

	//		float fDistToChar = (vEye-vCharPos).length();

	//		if(pCamera != NULL && pCamera->GetCameraMode() != CAutoCamera::CameraCameraFirstPerson)
	//		{
	//			// if the player is very close to the camera, we will draw it transparent.
	//			if(fDistToChar<(pPlayer->GetPhysicsRadius()+pCamera->GetNearPlane()))
	//			{
	//				techHandle = TECH_SIMPLE_MESH_NORMAL_TRANSPARENT;
	//			}
	//		}

	//		CGlobals::GetEffectManager()->BeginEffect(techHandle, &(sceneState.m_pCurrentEffect));
	//		sceneState.SetCameraToCurObjectDistance(fDistToChar);
	//		pPlayer->Draw(&sceneState);
	//		++nObjCount;
	//	}
	//}
	if(CHECK_SELECTION(RENDER_CHARACTERS))
	{
		/// Draw all characters including the player itself.
		nObjCount += RenderCharacters(sceneState, sceneState.listPRBiped);
	}
	if (CHECK_SELECTION(RENDER_TRANSPARENT_CHARACTERS))
	{
		/// Draw all characters including the player itself.
		nObjCount += RenderCharacters(sceneState, sceneState.listPRTransparentBiped);
	}
	if(CHECK_SELECTION(RENDER_SELECTION))
	{
		if(!sceneState.listSelections.empty())
		{
			SceneState::List_PostRenderObject_Type::iterator itCurCP, itEndCP = sceneState.listSelections.end();
			for(itCurCP = sceneState.listSelections.begin(); itCurCP !=itEndCP; ++itCurCP)
			{
				sceneState.SetCameraToCurObjectDistance((*itCurCP).m_fObjectToCameraDistance);
				(*itCurCP).m_pRenderObject->Draw(&sceneState);
				++nObjCount;
			}
		}
	}
#ifdef USE_DIRECTX_RENDERER
	if(CHECK_SELECTION(RENDER_SPRITES))
	{
		/// -- Sprite post-rendering list
		LPDIRECT3DDEVICE9 pd3dDevice = sceneState.m_pd3dDevice;
		if(!sceneState.listPRSprite.empty())
		{
			pd3dDevice->SetVertexShader( NULL );			// always set this
			pd3dDevice->SetFVF( SPRITEVERTEX::FVF );

			AssetEntity* pLastAsset = NULL;
			CSpriteObject* pObj;

			SceneState::List_CSpriteObjectPtr_Type::iterator itCurCP, itEndCP = sceneState.listPRSprite.end();
			for( itCurCP = sceneState.listPRSprite.begin(); itCurCP != itEndCP; ++ itCurCP)
			{
				pObj = (*itCurCP);
				if(pObj->GetPrimaryAsset() != pLastAsset)
				{
					/* beginning of a new bunch with the same texture*/
					pLastAsset = pObj->GetPrimaryAsset();

					/* set vertex buffer, texture once and for all object in the bunch*/
					pd3dDevice->SetStreamSource( 0, pObj->m_pSprite->GetVB(), 0, sizeof(SPRITEVERTEX) );
					if(pObj->m_ppTexture)
						pd3dDevice->SetTexture(0, pObj->m_ppTexture->GetTexture());
					pObj->SetRenderState(pd3dDevice);
				}

				pObj->Draw(&sceneState);
				++nObjCount;
			}
			pObj->RestoreRenderState(pd3dDevice); /* use a universal restore*/

		}
	}
	if(CHECK_SELECTION(RENDER_PORTAL_SYSTEM))
	{
		// Turn off effect: TODO: this should be removed
		if(!sceneState.listZones.empty() || !sceneState.listPortals.empty())
		{
			CGlobals::GetEffectManager()->BeginEffect(TECH_NONE, &(sceneState.m_pCurrentEffect));
			LPDIRECT3DDEVICE9 pd3dDevice = sceneState.m_pd3dDevice;

			// draw zones
			{
				SceneState::List_PostRenderObject_Type::iterator itCurCP, itEndCP = sceneState.listZones.end();
				for(itCurCP = sceneState.listZones.begin(); itCurCP !=itEndCP; ++itCurCP)
				{
					if((*itCurCP).m_pRenderObject)
					{
						(*itCurCP).m_pRenderObject->Draw(&sceneState);
						++nObjCount;
					}
				}
			}
			// draw portals
			{
				SceneState::List_PostRenderObject_Type::iterator itCurCP, itEndCP = sceneState.listPortals.end();
				for(itCurCP = sceneState.listPortals.begin(); itCurCP !=itEndCP; ++itCurCP)
				{
					(*itCurCP).m_pRenderObject->Draw(&sceneState);
					++nObjCount;
				}
			}
		}
	}
#endif
	if(CHECK_SELECTION(RENDER_MISSILES))
	{
		/// -- Draw all characters including the player itself.
		if(!sceneState.listPRMissiles.empty())
		{
			CGlobals::GetEffectManager()->BeginEffect(TECH_CHARACTER, &(sceneState.m_pCurrentEffect));

			SceneState::List_AlphaPostRenderObject_Type::iterator itCurCP, itEndCP = sceneState.listPRMissiles.end();
			for( itCurCP = sceneState.listPRMissiles.begin(); itCurCP != itEndCP; ++ itCurCP)
			{
				sceneState.SetCameraToCurObjectDistance((*itCurCP).m_fObjectToCameraDistance);
				(*itCurCP).m_pRenderObject->Draw(&sceneState);
				++nObjCount;
			}
		}
	}
	if(CHECK_SELECTION(RENDER_PARTICLES))
	{
		bool bEffectSet = false;

		Vector3 vRenderOrigin = CGlobals::GetScene()->GetRenderOrigin();

		int nPSInstancesCount = 0;	// statistic:number of particles instances 
		int nParticlesCount = 0;	// statistic:number of particles 
		
		for (auto itCurCP = sceneState.listParticleSystems.begin(); itCurCP != sceneState.listParticleSystems.end();)
		{
			ParticleSystem* PS = (*itCurCP).get();

			map <void*, ParticleList*>& instances =  PS->m_instances;
			map<void*, ParticleList*>::iterator iter;
			bool bHasInstance = false;
			for (iter = instances.begin(); iter!=instances.end();)
			{
				ParticleList* instancePS = iter->second;
				if(instancePS->m_pSceneState == (&sceneState))
				{
					bHasInstance = true;
				}
				else
				{
					++iter;
					continue;
				}

				if(instancePS->m_bUpdated == false)
				{
					if(instancePS->m_bUseAbsCord)
					{ /// for globally un-updated instances, animate the remaining particles
						Vector3 vRenderOriginOffset = (Vector3)(instancePS->m_vLastRenderOrigin) - vRenderOrigin;
						instancePS->m_vLastRenderOrigin = *((Vector3*)(&vRenderOrigin));// update render origin
						if(PS->AnimateExistingParticles((float)dTimeDelta, vRenderOriginOffset, instancePS))
						{
							if(!bEffectSet){
								CGlobals::GetEffectManager()->BeginEffect(TECH_PARTICLES, &(sceneState.m_pCurrentEffect));
								bEffectSet = true;
							}
							PS->drawInstance(instancePS);
							if(CGlobals::WillGenReport())
								nParticlesCount += (int)instancePS->particles.size();
							++iter;
						}
						else
						{
							// delete the particle system instance, if there is no particle instances left.
							SAFE_DELETE(instancePS) ;
							iter = instances.erase(iter);
						}
					}
					else
					{
						/// for local un-updated instances, delete the instance.
						SAFE_DELETE(instancePS) ;
						iter = instances.erase(iter);
					}
				}
				else
				{
					instancePS->m_bUpdated = false;
					if(instancePS->m_bUseAbsCord)
					{ /// for globally updated instances, just draw it.
						if(instancePS->m_bRender)
						{
							if(!bEffectSet){
								CGlobals::GetEffectManager()->BeginEffect(TECH_PARTICLES, &(sceneState.m_pCurrentEffect));
								bEffectSet = true;
							}
							PS->drawInstance(instancePS);
							instancePS->m_bRender = false;

							if(CGlobals::WillGenReport())
								nParticlesCount += (int)instancePS->particles.size();
						}
						++iter;
					}
					else
					{
						/// for local updated instances, ignore it, since it has already been draw with the model to which it is attached.
						++iter;

						if(CGlobals::WillGenReport())
							nParticlesCount += (int)instancePS->particles.size();
					}
				}
			}
			// erase particle system from the list if there is no instance left.
			if (!bHasInstance)
				itCurCP = sceneState.listParticleSystems.erase(itCurCP);
			else
			{
				++itCurCP;
				if(CGlobals::WillGenReport())
					nPSInstancesCount += (int)instances.size();
			}
		}
		nObjCount = nParticlesCount;
	}
	if(CHECK_SELECTION(RENDER_BOUNDINGBOX))
	{
		if (m_bShowBoundingBox && GetBatchedElementDrawer())
		{
			// draw the oriented bounding box for debugging and editing.
			{
				GetBatchedElementDrawer()->SetDefaultColor(PARAVECTOR3(1.f, 0, 0));
				SceneState::List_PostRenderObject_Type::iterator itCurCP, itEndCP = sceneState.listPRSolidObject.end();
				for (itCurCP = sceneState.listPRSolidObject.begin(); itCurCP != itEndCP; ++itCurCP)
				{
					if (!((*itCurCP).m_pRenderObject->CheckAttribute(OBJ_CUSTOM_RENDERER)))
					{
						IViewClippingObject* pViewClippingObject = (*itCurCP).m_pRenderObject->GetViewClippingObject();
						pViewClippingObject->DebugDraw(GetBatchedElementDrawer());
						++nObjCount;
					}
				}
			}
			{
				GetBatchedElementDrawer()->SetDefaultColor(PARAVECTOR3(0, 0, 1.f));
				DWORD dwColor = COLOR_ARGB(255, 0, 0, 255);
				SceneState::List_AlphaPostRenderObject_Type::iterator itCurCP, itEndCP = sceneState.listPRSmallObject.end();
				for (itCurCP = sceneState.listPRSmallObject.begin(); itCurCP != itEndCP; ++itCurCP)
				{
					if (!((*itCurCP).m_pRenderObject->CheckAttribute(OBJ_CUSTOM_RENDERER)))
					{
						IViewClippingObject* pViewClippingObject = (*itCurCP).m_pRenderObject->GetViewClippingObject();
						pViewClippingObject->DebugDraw(GetBatchedElementDrawer());
						++nObjCount;
					}
				}
			}
			{
				GetBatchedElementDrawer()->SetDefaultColor(PARAVECTOR3(0, 0, 1.f));
				SceneState::List_AlphaPostRenderObject_Type::iterator itCurCP, itEndCP = sceneState.listPRMissiles.end();
				for(itCurCP = sceneState.listPRMissiles.begin(); itCurCP !=itEndCP; ++itCurCP)
				{
					if (!((*itCurCP).m_pRenderObject->CheckAttribute(OBJ_CUSTOM_RENDERER)))
					{
						IViewClippingObject* pViewClippingObject = (*itCurCP).m_pRenderObject->GetViewClippingObject();
						pViewClippingObject->DebugDraw(GetBatchedElementDrawer());
						++nObjCount;
					}
				}
			}
			{
				SceneState::List_PostRenderObject_Type::iterator itCurCP, itEndCP = sceneState.listPRBiped.end();
				for( itCurCP = sceneState.listPRBiped.begin(); itCurCP != itEndCP; ++ itCurCP)
				{
					if (itCurCP->m_pRenderObject != 0 && !((*itCurCP).m_pRenderObject->CheckAttribute(OBJ_CUSTOM_RENDERER)))
					{
						GetBatchedElementDrawer()->SetDefaultColor(PARAVECTOR3(0, 0, 1.f));
						((CBipedObject*)((*itCurCP).m_pRenderObject))->DebugDraw(GetBatchedElementDrawer());
						GetBatchedElementDrawer()->SetDefaultColor(PARAVECTOR3(0, 1.f, 0));
						((CBipedObject*)((*itCurCP).m_pRenderObject))->GetPhysicsBBObj()->DebugDraw(GetBatchedElementDrawer());
						++nObjCount;
					}
				}
			}
		}

		if (!m_pBatchedElementDraw->IsEmpty())
		{
			m_pBatchedElementDraw->DrawAll(true);
		}
	}
	if(CHECK_SELECTION(RENDER_TRANSLUCENT_FACE_GROUPS))
	{
		if(!sceneState.GetFaceGroups()->IsEmpty())
		{
			if(!sceneState.GetFaceGroups()->IsSorted())
			{
				sceneState.GetFaceGroups()->Sort(CGlobals::GetScene()->GetCurrentCamera()->GetEyePosition()-GetRenderOrigin());
			}

			CGlobals::GetEffectManager()->BeginEffect(TECH_SIMPLE_MESH_NORMAL);
			CEffectFile* pEffectFile = CGlobals::GetEffectManager()->GetCurrentEffectFile();
			//if(pEffectFile)
			{
				sceneState.GetFaceGroups()->Render();
			}
		}
	}
	if (CHECK_SELECTION(RENDER_OWNER_DRAW))
	{
		/// -- Draw all characters including the player itself.
		if (!sceneState.listOwnerDrawObjs.empty())
		{
			for (auto& obj : sceneState.listOwnerDrawObjs)
			{
				if (obj)
				{
					((CBaseObject*)(obj.get()))->Draw(&sceneState);
					++nObjCount;
				}
			}
		}
	}
	if (CHECK_SELECTION(RENDER_POST_RENDER_LIST))
	{
		/// post rendering objects
		if (!sceneState.listPostRenderingObjects.empty())
		{
			nObjCount += RenderList(sceneState.listPostRenderingObjects, sceneState);
		}
	}
	// end the effect anyway.
	// CGlobals::GetEffectManager()->EndEffect();
	return nObjCount;
}
CSceneScripts&	CSceneObject::GetScripts()
{
	return m_scripts;
}

void CSceneObject::EnableMouseEvent(bool bEnable, DWORD dwEvents)
{
	if(bEnable)
		m_dwEnableMouseEvent = 0xffff;
	else
		m_dwEnableMouseEvent = 0;
}

void CSceneObject::SetCurrentPlayer(CBipedObject* pPlayer)
{
	if(pPlayer!=NULL)
	{
		if(m_currentplayer!=pPlayer)
		{
			// note: no need to disable always sentient when player control changes, since AlwaysSentient is completely controlled by scripting interface. 
			/*if(m_currentplayer!=NULL)
			{
				m_currentplayer->SetAlwaysSentient(false);
			}*/

			if (m_currentplayer)
			{
				CGlobals::GetScene()->SetCanShowMainPlayer(true);
				// restore old settings.
				CParameterBlock* pParam = m_currentplayer->GetEffectParamBlock();
				if (pParam)
				{
					pParam->SetParameter("g_opacity", 1.f);
				}
			}

			m_currentplayer=pPlayer;
			// note: always make sentient if we take control of a player. this may not be the default behavior. we shall move this in future. 
			pPlayer->SetAlwaysSentient(true);
			pPlayer->MakeSentient(true);
		}
		
	}
}
void CSceneObject::SetShadowMapTexelSizeLevel(int nLevel)
{
#if defined(USE_DIRECTX_RENDERER)||defined(USE_OPENGL_RENDERER)
	if(CGlobals::GetRenderDevice())
	{
		CShadowMap* pShadowMap = CGlobals::GetEffectManager()->GetShadowMap();
		if(pShadowMap!=0) 
		{
			pShadowMap->SetShadowMapTexelSizeLevel(nLevel);
		}
	}
	else
	{
		CShadowMap::SetDefaultShadowMapTexelSizeLevel(nLevel);
	}
#endif
}

void CSceneObject::SetShadowMapTexelSize(int nWidth, int nHeight)
{
#if defined(USE_DIRECTX_RENDERER)||defined(USE_OPENGL_RENDERER)
	CShadowMap* pShadowMap = CGlobals::GetEffectManager()->GetShadowMap();
	if(pShadowMap!=0) 
	{
		pShadowMap->SetShadowMapTexelSize(nWidth, nHeight);
	}
#endif
}

/// call this function turn on/off shadow rendering once for all
void CSceneObject::SetShadow(bool bRenderShadow)
{
	m_bRenderMeshShadow = bRenderShadow;
#if defined(USE_DIRECTX_RENDERER)||defined(USE_OPENGL_RENDERER)
	if(bRenderShadow)
	{
		// never allow shadow rendering, if the shader version is below 20. 
		if(CGlobals::GetEffectManager()->GetDefaultEffectMapping()<20)
		{
			m_bRenderMeshShadow = false;
		}
	}

	CGlobals::GetEffectManager()->EnableUsingShadowMap(m_bRenderMeshShadow);
	if(m_bRenderMeshShadow)
	{
		SetShadowMapTexelSizeLevel(0);
	}
#endif
}

bool CSceneObject::IsShadowMapEnabled()
{
	if(!m_bRenderMeshShadow)
		return m_bRenderMeshShadow;
	else
	{
		return fabs(GetSunLight().GetSunAngle())<NO_SHADOW_SUN_ANGLE;
	}
}
void CSceneObject::RenderShadowMap()
{
#if defined(USE_DIRECTX_RENDERER)||defined(USE_OPENGL_RENDERER)
	// render a blank image even if sceneState.listShadowCasters.empty() 
	
	SceneState& sceneState = *(m_sceneState.get());
	CShadowMap* pShadowMap = CGlobals::GetEffectManager()->GetShadowMap();
	if((pShadowMap!=0) && SUCCEEDED(pShadowMap->BeginShadowPass()))
	{
		sceneState.SetShadowPass(true);
		double oldDelta = sceneState.dTimeDelta;
		sceneState.dTimeDelta = 0; // this ensures that particles are not animated another time during shadow map pass.

		// render the terrain 
		if(pShadowMap->UsingBlurredShadowMap())
		{
			RenderSelection(RENDER_GEN_SM_TERRAIN);
		}

		BlockWorldClient::GetInstance()->RenderShadowMap();

		// Drawing Render list with Instancing
		RenderShadowList(sceneState.listShadowCasters, sceneState);

		sceneState.dTimeDelta = oldDelta;
		sceneState.SetShadowPass(false);
		pShadowMap->EndShadowPass();
	}
#endif
}

BOOL CSceneObject::IsOnObject(int x,int y, int z)
{
	/*if(m_bPickFlag){
		ParaScripting::ParaObject obj=ParaScripting::ParaScene::MousePick(30, "");
		if (m_pickObj&&m_pickObj==obj.m_pObj){
			return true;
		}
	}
	return false;*/
	return TRUE;
}

bool CSceneObject::IsPersistent()
{
	return m_bIsPersistent;
}

void CSceneObject::SetPersistent(bool bPersistent)
{
	m_bIsPersistent = bPersistent;
}

int CSceneObject::RemoveCharacterFromDB(IGameObject* pObj)
{
#ifdef USE_DIRECTX_RENDERER
	CNpcDatabase* pDB = CGlobals::GetDataProviderManager()->GetNpcDB();
	if(pObj!=NULL && pObj->GetType() == CBaseObject::RPGBiped)
	{
		// only save persistent RPG character. 
		CRpgCharacter* pChar = (CRpgCharacter*)(pObj);
		
		if(!pChar->IsPersistent() && pChar->GetCharacterID()>0)
		{
			if(pDB->DeleteNPCByID(pChar->GetCharacterID()))
			{
				return S_OK;
			}
			else
			{
				OUTPUT_LOG("error removing character from db\r\n");
			}
		}
	}
#endif
	return E_FAIL;
}

int CSceneObject::SaveCharacterToDB(IGameObject* pObj)
{
#ifdef USE_DIRECTX_RENDERER
	CNpcDatabase* pDB = CGlobals::GetDataProviderManager()->GetNpcDB();
	if(pObj!=NULL && pObj->GetType() == CBaseObject::RPGBiped)
	{
		// only save persistent RPG character. 
		CRpgCharacter* pChar = (CRpgCharacter*)(pObj);
		if(/*pChar->IsModified() &&*/ pChar->IsPersistent())
		{
			CNpcDbItem npc;
			if(pChar->ToNpcDbItem(npc))
			{
				if(npc.m_nID > 0)
				{
					// update
					if(pDB->UpdateNPC(npc, CNpcDbItem::ALL_ATTRIBUTES))
					{
						
					}
					else
					{
						OUTPUT_LOG("error saving character\r\n");
					}
				}
				else
				{
					// insert
					if(pDB->InsertNPC(npc))
					{
						pChar->SetCharacterID(npc.m_nID);
					}
					else
					{
						OUTPUT_LOG("error saving character\r\n");
					}
				}
			}
			pChar->SetModified(false);
			return S_OK;
		}
	}
#endif
	return E_FAIL;
}

int CSceneObject::SaveAllCharacters()
{
#ifdef USE_DIRECTX_RENDERER
	CNpcDatabase* pDB = CGlobals::GetDataProviderManager()->GetNpcDB();
	// use transactions to speed up. 
	if(pDB->GetDBEntity())
		pDB->GetDBEntity()->ExecuteSqlScript("BEGIN");

	int nCount = 0;
	int nErrorCount = 0;
	for (auto& pObject : m_pTileRoot->m_listSolidObj)
	{
		if (pObject->GetType() == CBaseObject::RPGBiped)
		{
			// only save persistent RPG character. 
			CRpgCharacter* pChar = (CRpgCharacter*)pObject;
			if(pChar->IsPersistent())
			{
				if(SaveCharacterToDB(pChar->QueryIGameObject()) == S_OK)
				{
					nCount++;
				}
			}
		}
	}
	if(pDB->GetDBEntity())
		pDB->GetDBEntity()->ExecuteSqlScript("END");
	return nCount;
#else
	return 0;
#endif
}

int CSceneObject::SaveLocalCharacters()
{
	if(IsModified() == false)
		return 0;
#ifdef USE_DIRECTX_RENDERER
	CNpcDatabase* pDB = CGlobals::GetDataProviderManager()->GetNpcDB();
	int nCount = 0;
	int nErrorCount = 0;
	for (auto& pObject : m_pTileRoot->m_listSolidObj)
	{
		if (pObject->GetType() == CBaseObject::RPGBiped)
		{
			// only save persistent RPG character. 
			CRpgCharacter* pChar = (CRpgCharacter*)pObject;
			if(pChar->IsModified() && pChar->IsPersistent())
			{
				CNpcDbItem npc;
				if(pChar->ToNpcDbItem(npc))
				{
					if(npc.m_nID > 0)
					{
						// update
						if(pDB->UpdateNPC(npc, CNpcDbItem::ALL_ATTRIBUTES))
						{
							++nCount;
						}
						else
						{
							++nErrorCount;
						}
					}
					else
					{
						// insert
						if(pDB->InsertNPC(npc))
						{
							++nCount;
							pChar->SetCharacterID(npc.m_nID);
						}
						else
						{
							++nErrorCount;
						}
					}
				}
				pChar->SetModified(false);
			}
		}		
	}	
	return nCount;
#else
	return 0;
#endif
}

void CSceneObject::SetModified(bool bModified)
{
	m_bModified = bModified;
}

bool CSceneObject::IsModified()
{
	return m_bModified;
}


bool CSceneObject::DB_LoadNPCByID(int nNPCID, CRpgCharacter** pOut, bool bReload)
{
#ifdef USE_DIRECTX_RENDERER
	CNpcDatabase* pDB = CGlobals::GetDataProviderManager()->GetNpcDB();	
	CNpcDbItem npc;
	if(pDB->SelectNPCByID(nNPCID, npc))
	{
		return LoadNPCByNPCDbItem(npc, pOut, bReload);
	}
#endif
	return false;
}
bool CSceneObject::DB_LoadNPCByName(const string& sName, CRpgCharacter** pOut, bool bReload)
{
#ifdef USE_DIRECTX_RENDERER
	CNpcDatabase* pDB = CGlobals::GetDataProviderManager()->GetNpcDB();	
	CNpcDbItem npc;
	if(pDB->SelectNPCByName(sName, npc))
	{
		return LoadNPCByNPCDbItem(npc, pOut, bReload);
	}
#endif
	return false;
}
int CSceneObject::DB_LoadNPCsByRegion(const Vector3& vMin, const Vector3& vMax, bool bReload)
{
	int nCount = 0;
#ifdef USE_DIRECTX_RENDERER
	CNpcDatabase* pDB = CGlobals::GetDataProviderManager()->GetNpcDB();	
	NPCDBItemList listNpc;
	if(pDB->SelectNPCListByRegion(&listNpc, vMin, vMax))
	{
		NPCDBItemList::iterator itCur, itEnd = listNpc.end();
		for ( itCur = listNpc.begin(); itCur!=itEnd; ++itCur)
		{
			CRpgCharacter* pChar=NULL;
			if(LoadNPCByNPCDbItem(*itCur, &pChar, bReload))
			{
				++nCount;
			}
		}
		
	}
#endif
	return nCount;
}


bool CSceneObject::LoadNPCByNPCDbItem(CNpcDbItem& npc,CRpgCharacter** pOut, bool bReload)
{
#ifdef USE_DIRECTX_RENDERER
	CRpgCharacter * pObj = NULL;
	CBaseObject* OldObj=NULL;
	if((OldObj = GetObject(npc.m_sName, npc.m_vPos, npc.m_bIsGlobal)) == NULL)
	{
		if(CGlobals::WillGenReport())
		{
			/// check for invalid objects
			AssetEntity* pMAE = NULL;
			if(!npc.m_sAssetName.empty())
			{
				pMAE = CGlobals::GetAssetManager()->LoadParaX("", npc.m_sAssetName);
				if(pMAE)
				{
					if(!pMAE->IsValid())
						pMAE = NULL;
				}
			}
			if(pMAE==NULL)
			{
				OUTPUT_LOG("error: model asset not found for %s. character not created.\r\n", npc.m_sName.c_str());
				return false;
			}
		}
		
		pObj = new CRpgCharacter();
		if(pObj==NULL)
			return false;
	}
	else
	{
		if(bReload)
		{
			if(OldObj->GetType() == CBaseObject::RPGBiped)
			{
				pObj = (CRpgCharacter*)OldObj;
				// TODO: reset base model if they are different.
				// pObj->ResetBaseModel(pMAE);
				DetachObject(pObj);
			}
			else
			{
				OUTPUT_LOG("warning: biped type does not match, NPC %s can not be reloaded. \r\n", npc.m_sName.c_str());
				return false;
			}
		}
		else
			return false;
	}
	
	pObj->UpdateFromNPCDbItem(npc, CNpcDbItem::ALL_ATTRIBUTES);
	
	// make it unmodified, since it is just loaded
	pObj->SetModified(false);
	// make it persistent, since it is loaded from a persistent database.
	pObj->SetPersistent(true);

	AttachObject(pObj);
#endif
	return true;
}

bool CSceneObject::IsUsingFullScreenGlow()
{
#ifdef USE_DIRECTX_RENDERER
	return CGlobals::GetEffectManager()->IsUsingFullScreenGlow();
#else
	return false;
#endif
}

void CSceneObject::EnableFullScreenGlow( bool bEnable)
{
#ifdef USE_DIRECTX_RENDERER
	CGlobals::GetEffectManager()->EnableFullScreenGlow(bEnable);
#endif 
}

void CSceneObject::SetFullscreenGlowIntensity(float fGlowIntensity) 
{
	m_fFullscreenGlowIntensity = fGlowIntensity;
}
float CSceneObject::GetFullscreenGlowIntensity()
{
	return m_fFullscreenGlowIntensity;
}
void CSceneObject::SetFullscreenGlowBlendingFactor(float fGlowFactor)
{
	m_fFullscreenGlowBlendingFactor = fGlowFactor;
}

float CSceneObject::GetFullscreenGlowBlendingFactor()
{
	return m_fFullscreenGlowBlendingFactor;
}

void CSceneObject::SetGlowness( const Vector4& Glowness )
{
#ifdef USE_DIRECTX_RENDERER
	CGlobals::GetEffectManager()->SetGlowness(Glowness);
#endif
}

const Vector4& CSceneObject::GetGlowness()
{
#ifdef USE_DIRECTX_RENDERER
	return CGlobals::GetEffectManager()->GetGlowness();
#else
	return Vector4::ZERO;
#endif
}

void CSceneObject::RenderFullScreenGlowEffect()
{
#ifdef USE_DIRECTX_RENDERER
	// render a blank image even if sceneState.listShadowCasters.empty() 
	if(!IsUsingFullScreenGlow()) 
		return;
	CGlowEffect* pGlowEffect= CGlobals::GetEffectManager()->GetGlowEffect();
	if(pGlowEffect!=0)
	{
		if(CGlobals::GetOceanManager()->OceanEnabled() && CGlobals::GetOceanManager()->IsUnderWater())
			pGlowEffect->Render(0.3f, 1.1f);
		else
			pGlowEffect->Render( m_fFullscreenGlowIntensity,m_fFullscreenGlowBlendingFactor);
	}
#endif
}

bool CSceneObject::IsUsingScreenWaveEffect()
{
#ifdef USE_DIRECTX_RENDERER
	return CGlobals::GetEffectManager()->IsUsingScreenWaveEffect();
#else
	return 0;
#endif
}

void CSceneObject::EnableScreenWaveEffect(bool value)
{
#ifdef USE_DIRECTX_RENDERER
	COceanManager* pOcean = CGlobals::GetOceanManager();
	if(pOcean != NULL)
		pOcean->EnableScreenSpaceFog(!value);
	return CGlobals::GetEffectManager()->EnableScreenWaveEffect(value);
#endif
}

void CSceneObject::RenderScreenWaveEffect()
{
#ifdef USE_DIRECTX_RENDERER
	if(!IsUsingScreenWaveEffect())
		return;

	WaveEffect* pWaveEffect = CGlobals::GetEffectManager()->GetScreenWaveEffect();
	if(pWaveEffect!=0)
	{
		pWaveEffect->Render();
	}
#endif
}

void CSceneObject::ApplyWaterFogParam()
{
#ifdef USE_DIRECTX_RENDERER
	LinearColor waterAmb(0,0.749f,0.902f,0);
	GetSunLight().SetSunAmbient(waterAmb);

	LinearColor waterFogColor(0.686f,1,1,0);
	SetFogColor(waterFogColor);
	SetFogStart(60);
	SetFogEnd(210);
	SetFogDensity(0.65f);
#endif
}

void CSceneObject::RestoreSceneFogParam()
{
	GetSunLight().SetSunAmbient(m_defaultAmbColor);
	SetFogColor(m_defaultFogColor);
	SetFogStart(m_defaultFogStart);
	SetFogEnd(m_defaultFogEnd);
}

bool CSceneObject::ScreenShotReflection()
{
#ifdef USE_DIRECTX_RENDERER
	// render the ocean's X flipped reflection map.
	static unsigned short i=0;
	string filename = "Screen Shots\\reflectionmap0.jpg";
	filename[filename.size()-5] = '0'+(char)(++i);
#if WIN32&&defined(DEFAULT_FILE_ENCODING)
	std::wstring filename16 = StringHelper::MultiByteToWideChar(filename.c_str(), DEFAULT_FILE_ENCODING);
	D3DXSaveTextureToFileW(filename16.c_str(), D3DXIFF_JPG, CGlobals::GetOceanManager()->m_waveReflectionTexture, NULL);
#else 
	D3DXSaveTextureToFile(filename.c_str(), D3DXIFF_JPG, CGlobals::GetOceanManager()->m_waveReflectionTexture, NULL);
#endif
#endif
	return true;
}

bool CSceneObject::ScreenShotShadowMap()
{
#ifdef USE_DIRECTX_RENDERER
	CShadowMap* pShadowMap= CGlobals::GetEffectManager()->GetShadowMap();
	if(pShadowMap!=0)
	{
		static unsigned short i=0;
		string filename = "Screen Shots\\shadowmap0.jpg";
		filename[filename.size()-5] = '0'+(char)(++i);
		return pShadowMap->SaveShadowMapToFile(filename);
	}
#endif
	return false;
}

bool CSceneObject::ScreenShotGlowMap()
{
#ifdef USE_DIRECTX_RENDERER
	// out glow textures
	CGlowEffect* pGlowEffect= CGlobals::GetEffectManager()->GetGlowEffect();
	if(pGlowEffect!=0)
	{
		static unsigned short i=0;
		string filename = "Screen Shots\\glowmap0.jpg";
		filename[filename.size()-5] = '0'+(char)(++i);
		pGlowEffect->SaveGlowTexturesToFile(filename);
		return true;
	}
#endif
	return false;
}

CBaseObject* CSceneObject::TogglePlayer()
{
	CBipedObject* pLast = GetCurrentPlayer();
	if(pLast!=0)
	{
		RefListItem* item = pLast->GetRefObjectByTag(0);
		if(item!=0 && ((CBaseObject*)item->m_object)->IsGlobal() &&  ((CBaseObject*)item->m_object)->IsBiped() )
		{
			SetCurrentPlayer((CBipedObject*)(item->m_object));
			CAutoCamera* pCamera =  (CAutoCamera*)GetCurrentCamera();
			pCamera->FollowBiped( GetCurrentPlayer(), CameraFollowThirdPerson, 0);
		}
		else
		{
			auto itEndCP = m_pTileRoot->m_listSolidObj.end();
			bool bFound = false;
			int i=0;
			for(auto itCurCP = m_pTileRoot->m_listSolidObj.begin(); itCurCP!=itEndCP  && i<2;)
			{
				if(bFound && ((*itCurCP)->IsBiped()))
				{
					SetCurrentPlayer((CBipedObject*)(*itCurCP));
					CAutoCamera* pCamera =  (CAutoCamera*)GetCurrentCamera();
					pCamera->FollowBiped( GetCurrentPlayer(), CameraFollowThirdPerson, 0);
					break;
				}
				if((*itCurCP) == pLast)
					bFound = true;

				if((++itCurCP) == itEndCP)
				{
					if(!bFound)
						break;
					itCurCP = m_pTileRoot->m_listSolidObj.begin();
					++i;
				}
			}
		}
	}
	return GetCurrentPlayer();
}

void CSceneObject::OnTerrainChanged(const Vector3& vCenter, float fRadius)
{
	// TODO: in case there are too many characters, we may utilize the scene graph. 
	// At the moment, we will just linearly traverse all characters in the scene. 
	for (auto& pObject : m_pTileRoot->m_listSolidObj)
	{
		if (pObject->IsBiped())
		{
			CBipedObject * pBiped = (CBipedObject*)pObject;
			
			if(pBiped->TestCollisionSphere(&vCenter, fRadius, 1))
			{
				Vector3 vPos = pBiped->GetPosition();
				float fElev = CGlobals::GetGlobalTerrain()->GetElevation(vPos.x, vPos.z);
				if(fElev<vPos.y)
				{
					// let the biped fall to the ground
					pBiped->FallDown();
				}
				else
				{
					// terrain is above character, we need to bring the biped up
					// TODO: this will go wrong for underground bipeds.
					pBiped->SnapToTerrainSurface(false);
				}
			}
		}
	}
}

void CSceneObject::SetUseWireFrame(bool UseWireFrame )
{
	m_bUseWireFrame = UseWireFrame;
}

template <class T>
int FilterScreenObjectToList(T& renderlist, list<CBaseObject*>& output, CPortalFrustum* pFrustum, OBJECT_FILTER_CALLBACK pFnctFilter)
{
	int nCount = 0;
	typename T::iterator itCur, itEnd =  renderlist.end();
	for(itCur = renderlist.begin();itCur!=itEnd;++itCur)
	{
		CBaseObject* pObject = (*itCur).m_pRenderObject;
		if(pObject && pFrustum->CanSeeObject_CompleteCull(pObject->GetViewClippingObject(), 4))
		{
			if(pFnctFilter(pObject))
			{
				output.push_back(pObject);
				++nCount;
			}
		}
	}
	return nCount;
}

int CSceneObject::GetObjectsByScreenRect( list<CBaseObject*>& output, const RECT& rect, OBJECT_FILTER_CALLBACK pFnctFilter/*=NULL*/, float fMaxDistance)
{
	auto pViewportManager = CGlobals::GetViewportManager();
	if(rect.top >= rect.bottom || rect.left >= rect.right || pViewportManager==0)
	{
		return 0;
	}
	if(pFnctFilter==0)
		pFnctFilter = g_fncPickingAll;
	CBaseCamera* pCamera = GetCurrentCamera();
	static CPortalFrustum g_frustum;
	CPortalFrustum* pFrustum = &g_frustum;
	
	const Matrix4& matWorld = Matrix4::IDENTITY;

	POINT ptCursor[5];
	Vector3 vPickRayOrig;
	Vector3 vPickRayDir[5];

	int x = rect.left;
	int y = rect.top;
	int nWidth, nHeight;
	pViewportManager->GetPointOnViewport(x, y, &nWidth, &nHeight);
	ptCursor[0].x = x;
	ptCursor[0].y = y;
	pCamera->GetMouseRay(vPickRayOrig, vPickRayDir[0], ptCursor[0], nWidth, nHeight, &matWorld);

	x = rect.right;
	y = rect.top;
	pViewportManager->GetPointOnViewport(x, y, &nWidth, &nHeight);
	ptCursor[1].x = x;
	ptCursor[1].y = y;
	pCamera->GetMouseRay(vPickRayOrig, vPickRayDir[1], ptCursor[1], nWidth, nHeight, &matWorld);

	x = rect.right;
	y = rect.bottom;
	pViewportManager->GetPointOnViewport(x, y, &nWidth, &nHeight);
	ptCursor[2].x = x;
	ptCursor[2].y = y;
	pCamera->GetMouseRay(vPickRayOrig, vPickRayDir[2], ptCursor[2], nWidth, nHeight, &matWorld);

	x = rect.left;
	y = rect.bottom;
	pViewportManager->GetPointOnViewport(x, y, &nWidth, &nHeight);
	ptCursor[3].x = x;
	ptCursor[3].y = y;
	pCamera->GetMouseRay(vPickRayOrig, vPickRayDir[3], ptCursor[3], nWidth, nHeight, &matWorld);

	x = 0;
	y = 0;
	pViewportManager->GetPointOnViewport(x, y, &nWidth, &nHeight);
	ptCursor[4].x = x;
	ptCursor[4].y = y;
	pCamera->GetMouseRay(vPickRayOrig, vPickRayDir[4], ptCursor[4], nWidth, nHeight, &matWorld);

	// add the four side planes 
	for (int i=0;i<4;++i)
	{
		Vector3 vNormal;
		auto nextIndex = (i + 1) % 4;
		vNormal = vPickRayDir[nextIndex].crossProduct(vPickRayDir[i]);
		vNormal.normalise();
		Plane plane(vNormal, vPickRayOrig);
		pFrustum->AddCullingPlane(plane);
	}
	// add the near and far plane at index 4 and 5, this is tricky, since we will use CanSeeObject_CompleteCull for testing
	{
		Vector3& vNormal = vPickRayDir[4];
		vNormal.normalise();

		{
			// near plane
			Plane plane(vNormal, vPickRayOrig + (pCamera->GetNearPlane() * vNormal));
			pFrustum->AddCullingPlane(plane, false);
		}
		{
			// far plane
			auto t1 = ((fMaxDistance > 0.f) ? fMaxDistance : pCamera->GetObjectFrustum()->GetViewDepth()) * vNormal;
			Plane plane(-vNormal, vPickRayOrig + t1);
			pFrustum->AddCullingPlane(plane, false);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	/// select object on root tile
	int nCount = 0;

	SceneState& sceneState = *(m_sceneState.get());
	nCount += FilterScreenObjectToList(sceneState.listPRSolidObject, output, pFrustum, pFnctFilter);
	nCount += FilterScreenObjectToList(sceneState.listPRSmallObject, output, pFrustum, pFnctFilter);
	nCount += FilterScreenObjectToList(sceneState.listPRBiped, output, pFrustum, pFnctFilter);
	nCount += FilterScreenObjectToList(sceneState.listZones, output, pFrustum, pFnctFilter);
	nCount += FilterScreenObjectToList(sceneState.listPortals, output, pFrustum, pFnctFilter);

	pFrustum->RemoveAllExtraCullingPlanes();
	return nCount;
}

int CSceneObject::GetObjectsBySphere( list<CBaseObject*>& output, const CShapeSphere& sphere, OBJECT_FILTER_CALLBACK pFnctFilter, int nMethod/*=0*/ )
{
	if(pFnctFilter==0)
		pFnctFilter = g_fncPickingAll;
	
	Vector3 vCenter = sphere.GetCenter();
	float fRadius = sphere.GetRadius();
	int nCount = 0;

	queue_CTerrainTilePtr_Type queueTiles;
	CTerrainTile* pTile = GetRootTile();

	/// select object on root tile

	// pTile is now the root tile
	if(pTile->m_listVisitors.empty() == false)
	{
		/// add all visitor biped objects, only the last time will have this list non-empty
		VisitorList_type::iterator itCurCP, itEndCP = pTile->m_listVisitors.end();

		for( itCurCP = pTile->m_listVisitors.begin(); itCurCP != itEndCP; ++ itCurCP)
		{
			IGameObject* pObject = (*itCurCP);
			if (pObject)
			{
				IViewClippingObject* pViewClippingObject = pObject->GetViewClippingObject();
				if (pViewClippingObject->TestCollisionSphere(&vCenter, fRadius, 2))
				{
					if (pFnctFilter(pObject))
					{
						output.push_back(pObject);
						++nCount;
					}
				}
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

			/// For each free space object in the tile
			{
				for (auto& pObject : pTile->m_listFreespace)
				{
					IViewClippingObject* pViewClippingObject = pObject->GetViewClippingObject();
					if(pViewClippingObject->TestCollisionSphere(& vCenter, fRadius,2))
					{
						if (pFnctFilter(pObject))
						{
							output.push_back(pObject);
							++nCount;
						}
					}
				}
			}
			{
				/// add all solid objects to the queue for further testing
				for (auto& pObject : pTile->m_listSolidObj)
				{
					IViewClippingObject* pViewClippingObject = pObject->GetViewClippingObject();
					if(pViewClippingObject->TestCollisionSphere(& vCenter, fRadius,2))
					{
						if (pFnctFilter(pObject))
						{
							output.push_back(pObject);
							++nCount;
						}
					}
				}
			}
			{
				/// add all visitor biped objects to the queue.
				VisitorList_type::iterator itCurCP, itEndCP = pTile->m_listVisitors.end();
				for( itCurCP = pTile->m_listVisitors.begin(); itCurCP != itEndCP; ++ itCurCP)
				{
					IGameObject* pObject = *itCurCP;
					if (pObject)
					{
						IViewClippingObject* pViewClippingObject = pObject->GetViewClippingObject();
						if (pViewClippingObject->TestCollisionSphere(&vCenter, fRadius, 2))
						{
							if (pFnctFilter(pObject))
							{
								output.push_back(pObject);
								++nCount;
							}
						}
					}
				}
			}
		}
	}//while(!queueTiles.empty())
	return nCount;
}

CMiniSceneGraph* CSceneObject::GetPostProcessingScene()
{
	return GetMiniSceneGraph(g_sPSSceneName);
}

void CSceneObject::EnablePostProcessing( bool bEnable, const char* sCallbackScript)
{
	m_bEnablePostProcessing = bEnable;
	m_sPostPorcessorCallbackScript = sCallbackScript;
}

bool CSceneObject::IsPostProcessingEnabled()
{
	return m_bEnablePostProcessing;
}

void CSceneObject::EnableMiniSceneGraph( bool bEnable )
{
	m_bEnableMiniScenegraph = bEnable;
}

bool CSceneObject::IsMiniSceneGraphEnabled()
{
	return m_bEnableMiniScenegraph;
}

const std::string& CSceneObject::GetAllMiniSceneGraphNames()
{
	m_sMiniSceneGraphNames.clear();
	MiniSceneGraphPool_type::iterator itCur, itEnd = m_miniSceneGraphs.end();
	for (itCur = m_miniSceneGraphs.begin(); itCur!=itEnd; ++itCur)
	{
		m_sMiniSceneGraphNames += (*itCur)->GetIdentifier();
		m_sMiniSceneGraphNames += ",";
	}
	return m_sMiniSceneGraphNames;
}

CMiniSceneGraph* CSceneObject::GetMiniSceneGraph( const string& name )
{
	MiniSceneGraphPool_type::iterator itCur, itEnd = m_miniSceneGraphs.end();
	for (itCur = m_miniSceneGraphs.begin(); itCur!=itEnd; ++itCur)
	{
		if((*itCur)->GetIdentifier() == name)
		{
			return (*itCur);
		}
	}

	//////////////////////////////////////////////////////////////////////////
	// create a new one if no mini scene graph is found
	//////////////////////////////////////////////////////////////////////////
	CMiniSceneGraph* pGraph = new CMiniSceneGraph();
	pGraph->SetIdentifier(name);
	m_miniSceneGraphs.push_back(pGraph);

	// if this is a post processing scene. 
	if(name == g_sPSSceneName)
	{
		pGraph->SetVisibility(false);
	}
	return pGraph;
}

int CSceneObject::DeleteMiniSceneGraph( const string& name )
{
	int nCount = 0;
	for (MiniSceneGraphPool_type::iterator itCur = m_miniSceneGraphs.begin(); itCur!=m_miniSceneGraphs.end(); )
	{
		if(name=="" || name == "*" || (*itCur)->GetIdentifier() == name)
		{
			itCur = m_miniSceneGraphs.erase(itCur);
			++nCount;
		}
		else
			++itCur;
	}
	return nCount;
}

void CSceneObject::SetForceExportPhysics(bool bWhetherPhysicsAreAlwaysExported)
{
	g_bForceExportMeshPhysics = bWhetherPhysicsAreAlwaysExported;
}

bool CSceneObject::IsForceExportPhysics()
{
	return g_bForceExportMeshPhysics;
}

void CSceneObject::SetMaxRenderCount(int nRenderImportance, int nCount)
{
	if(nRenderImportance >= (int)m_MaxRenderCount.size())
	{
		m_MaxRenderCount.resize(nRenderImportance+1, 65535);
	}
	m_MaxRenderCount[nRenderImportance] = nCount;

	// set the scene state. 
	m_sceneState->SetMaxRenderCount(nRenderImportance, nCount);
}

int CSceneObject::GetMaxRenderCount(int nRenderImportance)
{
	if(nRenderImportance < (int)m_MaxRenderCount.size())
	{
		return m_MaxRenderCount[nRenderImportance];
	}
	return 0;
}

void CSceneObject::SetPhysicsDebugDrawMode( int nMode )
{
	if(GetBatchedElementDrawer())
	{
		CGlobals::GetPhysicsWorld()->GetPhysicsInterface()->SetDebugDrawMode(nMode);
	}
}

int CSceneObject::GetPhysicsDebugDrawMode()
{
	return (GetBatchedElementDrawer()) ? CGlobals::GetPhysicsWorld()->GetPhysicsInterface()->GetDebugDrawMode() : 0;
}

void CSceneObject::SetShadowRadius( float fShadowRadius /*= 50.f*/ )
{
	m_fShadowRadius = fShadowRadius;
}


void CSceneObject::ClearParticles()
{
	m_missiles.clear();
	{
		SceneState::List_ParticleSystemPtr_Type::iterator itCurCP, iterEnd = m_sceneState->listParticleSystems.end();
		for( itCurCP = m_sceneState->listParticleSystems.begin(); itCurCP != iterEnd; itCurCP++)
		{
			ParticleSystem* PS = (*itCurCP).get();
			if(PS)
			{
				PS->ClearAllInstances();
			}
		}
		m_sceneState->listParticleSystems.clear();
	}
}

void CSceneObject::SetCursor( const char* szCursorFile, int nHotSpotX /*= -1*/, int nHotSpotY /*= -1*/ )
{
	if(szCursorFile)
		m_sCursorFile = szCursorFile;
	else
		m_sCursorFile.clear();
	m_nCursorHotSpotX = nHotSpotX;
	m_nCursorHotSpotY = nHotSpotY;
}

const std::string& CSceneObject::GetCursor( int* pnHotSpotX /*= 0*/, int* pnHotSpotY /*= 0*/ ) const
{
	if(pnHotSpotX)
	{
		*pnHotSpotX = m_nCursorHotSpotX;
	}
	if(pnHotSpotY)
	{
		*pnHotSpotY = m_nCursorHotSpotY;
	}
	return m_sCursorFile;
}

bool CSceneObject::UnLoadFlashTextures()
{
#ifdef USE_FLASH_MANAGER
	CGlobals::GetAssetManager()->GetFlashManager().UnloadAsset();
#endif
	return true;
}


Vector3 CSceneObject::GetFogColor(float fCelestialAngle)
{
	float cosAngle = Math::Cos(fCelestialAngle * (float)Math::PI * 2.0f) * 2.0f + 0.5f;
	if (cosAngle < 0.0f)
		cosAngle = 0.0f;
	if (cosAngle > 1.0f)
		cosAngle = 1.0f;
	float r = 0.7529412f;
	float g = 0.84705883f;
	float b = 1.0f;
	r *= cosAngle * 0.94f + 0.06f;
	g *= cosAngle * 0.94f + 0.06f;
	b *= cosAngle * 0.91f + 0.09f;
	return Vector3(r, g, b);
}


void CSceneObject::UpdateFogColor()
{
	CSkyMesh * pSky = GetCurrentSky();
	if (pSky != 0 && IsRenderSky() && pSky->IsSimulatedSkyEnabled())
	{
		float fSunAngle = GetSunLight().GetCelestialAngle();
		LinearColor fogColor = GetFogColor(fSunAngle);
		Vector3 vLookat = GetCurrentCamera()->GetWorldAhead();
		Vector3 vSunPos = - GetSunLight().GetSunDirection();
		float fDotAngle = vLookat.dotProduct(vSunPos);
		if (fDotAngle < 0.f)
			fDotAngle = 0.f;
		if (fDotAngle > 0.f)
		{
			// merge with sunset/sunrise color
			LinearColor* pvSunColor = GetSunLight().CalcSunriseSunsetColors(fSunAngle);
			if (pvSunColor)
			{
				// apply blending
				float fBlending = pvSunColor->a;
				pvSunColor->r = fogColor.r*(1.f - fBlending) + fBlending * pvSunColor->r;
				pvSunColor->g = fogColor.g*(1.f - fBlending) + fBlending * pvSunColor->g;
				pvSunColor->b = fogColor.b*(1.f - fBlending) + fBlending * pvSunColor->b;

				// apply camera look direction
				fogColor.r = fogColor.r*(1.f - fDotAngle) + fDotAngle * pvSunColor->r;
				fogColor.g = fogColor.g*(1.f - fDotAngle) + fDotAngle * pvSunColor->g;
				fogColor.b = fogColor.b*(1.f - fDotAngle) + fDotAngle * pvSunColor->b;
			}
		}
		// TODO: shall we take rain, block lighting in to consideration here? 
		// reference: EntityRenderer  updateFogColor();
		SetFogColor(fogColor);
	}
	else
	{
		LinearColor fogColor = GetSunLight().ComputeFogColor();
		fogColor.r *= GetFogColorFactor().r;
		fogColor.g *= GetFogColorFactor().g;
		fogColor.b *= GetFogColorFactor().b;
		SetFogColor(fogColor);
	}
}

void CSceneObject::AddToDeadObjectPool(CBaseObject* pObject)
{
	if (pObject && (pObject->GetParent() || pObject->GetTileContainer()))
		m_dead_objects.push_back(pObject);
}

void CSceneObject::RemoveDeadObjects()
{
	for (auto& pObject: m_dead_objects)
	{
		if (pObject->IsDead())
		{
			// needs to verify again, just in case the object is reused again. 
			auto pParent = pObject->GetParent();
			if (pParent)
			{
				pParent->RemoveChild(pObject);
				pObject->SetParent(NULL);
			}
			else
			{
				DeleteObject(pObject);
			}
		}
	}
	m_dead_objects.clear();
}


IAttributeFields* CSceneObject::GetChildAttributeObject(const char * sName)
{
	if (std::string(sName) == "SceneState")
	{
		return m_sceneState.get();
	}
	return CBaseObject::GetChildAttributeObject(sName);
}

IAttributeFields* CSceneObject::GetChildAttributeObject(int nRowIndex, int nColumnIndex /*= 0*/)
{
	if (nColumnIndex == 0)
		return (nRowIndex < (int)m_children.size()) ? m_children.at(nRowIndex) : NULL;
	else if (nColumnIndex == 1)
		return (nRowIndex < (int)m_attribute_models.size()) ? m_attribute_models.at(nRowIndex) : NULL;
	else
		return 0;
}

int CSceneObject::GetChildAttributeObjectCount(int nColumnIndex /*= 0*/)
{
	if (nColumnIndex == 0)
		return (int)m_children.size();
	else if (nColumnIndex == 1)
		return (int)m_attribute_models.size();
	else
		return 0;
}

int CSceneObject::GetChildAttributeColumnCount()
{
	return 2;
}

int CSceneObject::InstallFields(CAttributeClass* pClass, bool bOverride)
{
	CBaseObject::InstallFields(pClass, bOverride);

	pClass->AddField("IsModified", FieldType_Bool, (void*)SetModified_s, (void*)IsModified_s, NULL, NULL, bOverride);
	pClass->AddField("FullScreenGlow", FieldType_Bool, (void*)EnableFullScreenGlow_s, (void*)IsUsingFullScreenGlow_s, NULL, NULL, bOverride);
	pClass->AddField("GlowIntensity", FieldType_Float, (void*)SetFullscreenGlowIntensity_s, (void*)GetFullscreenGlowIntensity_s, NULL, NULL, bOverride);
	pClass->AddField("GlowFactor", FieldType_Float, (void*)SetFullscreenGlowBlendingFactor_s, (void*)GetFullscreenGlowBlendingFactor_s, NULL, NULL, bOverride);
	pClass->AddField("Glowness", FieldType_Vector4, (void*)SetGlowness_s, (void*)GetGlowness_s, NULL, NULL, bOverride);

	pClass->AddField("EnableSunLight", FieldType_Bool, (void*)EnableSunLight_s, (void*)IsSunLightEnabled_s, NULL, NULL, bOverride);
	pClass->AddField("EnableLight", FieldType_Bool, (void*)EnableLight_s, (void*)IsLightEnabled_s, NULL, NULL, bOverride);
	pClass->AddField("ShowLights", FieldType_Bool, (void*)ShowLocalLightMesh_s, (void*)IsShowLocalLightMesh_s, NULL, NULL, bOverride);
	pClass->AddField("MaxLightsNum", FieldType_Int, (void*)SetMaxLocalLightsNum_s, (void*)GetMaxLocalLightsNum_s, CAttributeField::GetSimpleSchemaOfInt(0, 4), NULL, bOverride);

	pClass->AddField("SetShadow", FieldType_Bool, (void*)SetShadow_s, (void*)IsShadowMapEnabled_s, NULL, NULL, bOverride);
	pClass->AddField("MaxNumShadowCaster", FieldType_Int, (void*)SetMaximumNumShadowCasters_s, (void*)GetMaximumNumShadowCasters_s, NULL, NULL, bOverride);
	pClass->AddField("MaxNumShadowReceiver", FieldType_Int, (void*)SetMaximumNumShadowReceivers_s, (void*)GetMaximumNumShadowReceivers_s, NULL, NULL, bOverride);
	pClass->AddField("MaxCharTriangles", FieldType_Int, (void*)SetMaxCharTriangles_s, (void*)GetMaxCharTriangles_s, NULL, NULL, bOverride);
	
	pClass->AddField("BackgroundColor", FieldType_Vector3, (void*)SetBackGroundColor_s, (void*)GetBackGroundColor_s, CAttributeField::GetSimpleSchema(SCHEMA_RGB), NULL, bOverride);
	pClass->AddField("EnableFog", FieldType_Bool, (void*)EnableFog_s, (void*)IsFogEnabled_s, NULL, NULL, bOverride);
	pClass->AddField("FogColor", FieldType_Vector3, (void*)SetFogColor_s, (void*)GetFogColor_s, CAttributeField::GetSimpleSchema(SCHEMA_RGB), NULL, bOverride);
	pClass->AddField("FogStart", FieldType_Float, (void*)SetFogStart_s, (void*)GetFogStart_s, NULL, NULL, bOverride);
	pClass->AddField("FogEnd", FieldType_Float, (void*)SetFogEnd_s, (void*)GetFogEnd_s, NULL, NULL, bOverride);
	pClass->AddField("FogDensity", FieldType_Float, (void*)SetFogDensity_s, (void*)GetFogDensity_s, NULL, NULL, bOverride);
	pClass->AddField("MinPopUpDistance", FieldType_Float, (void*)SetMinPopUpDistance_s, (void*)GetMinPopUpDistance_s, NULL, NULL, bOverride);
	pClass->AddField("OnClickDistance", FieldType_Float, (void*)SetOnClickDistance_s, (void*)GetOnClickDistance_s, NULL, NULL, bOverride);

	pClass->AddField("ShowSky", FieldType_Bool, (void*)EnableSkyRendering_s, (void*)IsRenderSky_s, NULL, NULL, bOverride);
	pClass->AddField("PasueScene", FieldType_Bool, (void*)PauseScene_s, (void*)IsScenePaused_s, NULL, NULL, bOverride);
	pClass->AddField("PauseScene", FieldType_Bool, (void*)PauseScene_s, (void*)IsScenePaused_s, NULL, NULL, bOverride);
	pClass->AddField("EnableScene", FieldType_Bool, (void*)EnableScene_s, (void*)IsSceneEnabled_s, NULL, NULL, bOverride);
	pClass->AddField("ShowBoundingBox", FieldType_Bool, (void*)ShowBoundingBox_s, (void*)IsShowBoundingBox_s, NULL, NULL, bOverride);
	pClass->AddField("ShowPortalSystem", FieldType_Bool, (void*)ShowPortalSystem_s, (void*)IsPortalSystemShown_s, NULL, NULL, bOverride);
	pClass->AddField("ShowMainPlayer", FieldType_Bool, (void*)ShowMainPlayer_s, (void*)IsShowMainPlayer_s, NULL, NULL, bOverride);

	pClass->AddField("EnablePortalZone", FieldType_Bool, (void*)EnablePortalZone_s, (void*)IsPortalZoneEnabled_s, NULL, NULL, bOverride);

	pClass->AddField("GenerateReport", FieldType_Bool, (void*)SetGenerateReport_s, (void*)IsGenerateReport_s, NULL, NULL, bOverride);
	pClass->AddField("AutoPlayerRipple", FieldType_Bool, (void*)EnableAutoPlayerRipple_s, (void*)IsAutoPlayerRippleEnabled_s, NULL, NULL, bOverride);
	pClass->AddField("ShowHeadOnDisplay", FieldType_Bool, (void*)ShowHeadOnDisplay_s, (void*)IsHeadOnDisplayShown_s, NULL, NULL, bOverride);
	pClass->AddField("UseWireFrame", FieldType_Bool, (void*)SetUseWireFrame_s, (void*)IsUseWireFrame_s, NULL, NULL, bOverride);
	pClass->AddField("ForceExportPhysics", FieldType_Bool, (void*)SetForceExportPhysics_s, (void*)IsForceExportPhysics_s, NULL, NULL, bOverride);
	pClass->AddField("MaxHeadOnDisplayDistance", FieldType_Float, (void*)SetMaxHeadOnDisplayDistance_s, (void*)GetMaxHeadOnDisplayDistance_s, NULL, NULL, bOverride);
	pClass->AddField("UseInstancing", FieldType_Bool, (void*)EnableInstancing_s, (void*)IsInstancingEnabled_s, NULL, NULL, bOverride);

	pClass->AddField("persistent", FieldType_Bool, (void*)SetPersistent_s, (void*)IsPersistent_s, NULL, NULL, bOverride);
	pClass->AddField("PhysicsDebugDrawMode", FieldType_Int, (void*)SetPhysicsDebugDrawMode_s, (void*)GetPhysicsDebugDrawMode_s, NULL, NULL, bOverride);
	pClass->AddField("BlockInput", FieldType_Bool, (void*)SetBlockInput_s, (void*)IsBlockInput_s, NULL, NULL, bOverride);
	pClass->AddField("AllMiniSceneGraphNames", FieldType_String, NULL, (void*)GetAllMiniSceneGraphNames_s, NULL, NULL, bOverride);
	pClass->AddField("ShadowRadius", FieldType_Float, (void*)SetShadowRadius_s, (void*)GetShadowRadius_s, NULL, NULL, bOverride);
	pClass->AddField("ClearParticles", FieldType_void, (void*)ClearParticles_s, NULL, NULL, NULL, bOverride);

	pClass->AddField("UseDropShadow", FieldType_Bool, (void*)SetDropShadow_s, (void*)IsDropShadow_s, NULL, NULL, bOverride);
	pClass->AddField("UseScreenWaveEffect", FieldType_Bool, (void*)SetEnableScreenWaveEffect_s, (void*)IsUsingScreenWaveEffect_s, NULL, NULL, bOverride);

	pClass->AddField("UnLoadFlashTextures", FieldType_void, (void*)UnLoadFlashTextures_s, NULL, NULL, NULL, bOverride);
#ifdef _DEBUG
	// just for taking pictures for writing my book on the game engine. 
	pClass->AddField("Save ReflectionMap", FieldType_void, (void*)ScreenShotReflection_s, NULL, NULL, NULL, bOverride);
	pClass->AddField("Save ShadowMap", FieldType_void, (void*)ScreenShotShadowMap_s, NULL, NULL, NULL, bOverride);
	pClass->AddField("Save GlowMap", FieldType_void, (void*)ScreenShotGlowMap_s, NULL, NULL, NULL, bOverride);
#endif
	pClass->AddField("ShadowMapSize", FieldType_Int, (void*)SetShadowMapTexelSize_s, NULL, NULL, NULL, bOverride);

	return S_OK;
}

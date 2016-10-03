//-----------------------------------------------------------------------------
// Class:	CMiniSceneGraph
// Authors:	Li, Xizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2007.7.18
// Revised: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#ifdef USE_DIRECTX_RENDERER
#include "DirectXEngine.h"
#endif
#include "SkyMesh.h"
#include "ParaWorldAsset.h"
#include "CanvasCamera.h"
#include "EffectHelper.h"
#include "SceneObject.h"
#include "MiniSceneGraph.h"
#include "BipedObject.h"
#include "2dengine/GUIRoot.h"
#include "SceneObjectPicking.h"
#include "RayCollider.h"
#include "sceneRenderList.h"
#include "MeshPhysicsObject.h"
#include "ParaXModel/particle.h"
#include "SortedFaceGroups.h"
#include "BatchedElementDraw.h"
#include "2dengine/GUIRoot.h"
#include "SunLight.h"
#include "ViewportManager.h"


#include "memdebug.h"
#include <list>


using namespace std;
using namespace ParaEngine;

#define FOG_START_DEFAULT				50.0f
#define FOG_END_DEFAULT					100.0f
#define FOG_DENSITY_DEFAULT				0.66f

CMiniSceneGraph::CMiniSceneGraph(void)
	: m_bShowHeadOnDisplay(false), m_fMaxHeadOnDisplayDistance(50.f), m_nRenderPipelineOrder(PIPELINE_3D_SCENE), m_bUsePointTexture(false)
{
	m_bIsDirty = false;
	m_pCamera = NULL;
	m_pActor = NULL;
	m_bUseLocalCamera = false;
	
	SetPosition(DVector3(0,0,0));
	m_bEnabled = true;
	m_vRenderOrigin = Vector3::ZERO;

	m_bEnableLighting = true;
	m_bEnableSunLight = true;

	/// set fog
	m_FogColorFactor = 0xffffffff;
	m_bRenderFog = false; // no fog by default as for miniscenegraph
	m_dwFogColor = 0xff00bfff;
	m_fFogStart = FOG_START_DEFAULT;
	m_fFogEnd = FOG_END_DEFAULT;
	m_fFogDensity = FOG_DENSITY_DEFAULT;

	/// turn on sky rendering, if there is any available sky meshes
	m_bRenderSky = false;

	m_sceneState.m_pScene = this;

	m_pBatchedElementDraw = new CBatchedElementDraw();
	m_sceneState.SetBatchedElementDrawer(GetBatchedElementDrawer());
}

CMiniSceneGraph::~CMiniSceneGraph(void)
{
	SAFE_DELETE(m_pCamera);
	SAFE_DELETE(m_pBatchedElementDraw);
}

void CMiniSceneGraph::Cleanup()
{
	Reset();

	/// delete Sky meshes
	{
		DeleteSkyBox("");
	}
	CRenderTarget::Cleanup();

	SAFE_DELETE(m_pCamera);
}


CCanvasCamera* CMiniSceneGraph::GetCamera()
{
	if(m_pCamera==NULL)
		m_pCamera = new CCanvasCamera();
	return m_pCamera;
}

void CMiniSceneGraph::SetMaskTexture(TextureEntity* pTexture)
{
	if(m_pMask!=pTexture)
	{
		m_pMask = pTexture;
	}
}

void ParaEngine::CMiniSceneGraph::EnableCamera( bool bEnable )
{
	m_bUseLocalCamera = bEnable;
	if(m_bUseLocalCamera)
	{
		CCanvasCamera* pCamera = GetCamera();
		if(pCamera)
		{
			if(GetRenderPipelineOrder() == PIPELINE_POST_UI_3D_SCENE)
			{
				// set aspect ratio of the camera to that of the main scene. 
				pCamera->UpdateProjParams(CGlobals::GetScene()->GetCurrentCamera()->GetAspectRatio());
			}
			else
			{
				// set aspect ratio of the camera to that of texture surface. 
				pCamera->UpdateProjParams((float)m_nTextureWidth/(float)m_nTextureHeight);
			}
		}

		if(GetRenderPipelineOrder()<PIPELINE_POST_UI_3D_SCENE &&  m_pCanvasTexture.get() == 0)
		{
			InitWithWidthAndHeight(m_nTextureWidth, m_nTextureHeight);
		}
	}
}

bool ParaEngine::CMiniSceneGraph::IsCameraEnabled()
{
	return m_bUseLocalCamera;
}

CBaseObject* ParaEngine::CMiniSceneGraph::GetObject( const char* name , bool bRecursive)
{
	string sName = name;

	if(!bRecursive)
	{
		ObjectNameMap_Type::iterator iter =  m_name_map.find(sName);
		if(iter!=m_name_map.end())
		{
			return iter->second;
		}
		return NULL;
	}
	else
	{
		// we will first perform a non recursive search on the level 1, since we want to make GetObject() faster on level one child. 
		CBaseObject * pChild = GetChildByName(sName, false);

		if(pChild ==NULL)
		{
			// if objects is not on level 1 child, we will perform a depth first search for each of the level 1 child. 
			for (auto pNode : m_children)
			{
				pChild = pNode->GetChildByName(sName, true);

				if(pChild!=NULL)
				{
					break;
				}
			}
		}
		return pChild;
	}
}

int ParaEngine::CMiniSceneGraph::DestroyObject( const char* name, bool bRecursive )
{
	string sName = name;
	if(!bRecursive)
	{
		// for non-recursive, we will only destroy if we can find the name on the map.  
		ObjectNameMap_Type::iterator iter =  m_name_map.find(sName);
		if(iter != m_name_map.end())
		{
			m_name_map.erase(iter);
			return DestroyChildByName(sName, false);
		}
		return 0;
	}
	else
	{
		// we will first perform a non recursive search on the level 1, since we want to make it faster on level one child. 
		int nCount = DestroyChildByName(sName, false);

		if(nCount > 0)
		{
			ObjectNameMap_Type::iterator iter =  m_name_map.find(sName);
			if(iter!=m_name_map.end())
				m_name_map.erase(iter);
		}
		else
		{
			// if objects is not on level 1 child, we will perform a depth first search for each of the level 1 child. 
			for (auto pChild : m_children)
			{
				nCount = pChild->DestroyChildByName(sName, true);

				if(nCount>0)
				{
					break;
				}
			}
		}
		return nCount;
	}
}

int ParaEngine::CMiniSceneGraph::DestroyObject(CBaseObject* pObj)
{
	int nCount = RemoveObject(pObj);
	return nCount;
}

int ParaEngine::CMiniSceneGraph::RemoveObject(const char* name, bool bRecursive)
{
	string sName = name;

	if(!bRecursive)
	{
		// for non-recursive, we will only destroy if we can find the name on the map.  
		ObjectNameMap_Type::iterator iter =  m_name_map.find(sName);
		if(iter != m_name_map.end())
		{
			m_name_map.erase(iter);
			return RemoveChildByName(sName, false);
		}
		return 0;
	}
	else
	{
		// we will first perform a non recursive search on the level 1, since we want to make it faster on level one child. 
		int nCount = RemoveChildByName(sName, false);

		if(nCount >0)
		{
			ObjectNameMap_Type::iterator iter =  m_name_map.find(sName);
			if(iter!=m_name_map.end())
				m_name_map.erase(iter);
		}
		else
		{
			// if objects is not on level 1 child, we will perform a depth first search for each of the level 1 child. 
			for (auto pChild : m_children)
			{
				nCount = pChild->RemoveChildByName(sName, true);

				if(nCount>0)
				{
					break;
				}
			}
		}
		return nCount;
	}
}

int ParaEngine::CMiniSceneGraph::RemoveObject( CBaseObject* pObj )
{
	if(pObj == 0)
		return 0;

	pObj->AddToAutoReleasePool();
	// we will first perform a non recursive search on the level 1, since we want to make it faster on level one child. 
	int nCount = RemoveChild(pObj, false);

	if(nCount > 0)
	{
		ObjectNameMap_Type::iterator iter =  m_name_map.find(pObj->GetName());
		if(iter!=m_name_map.end())
			m_name_map.erase(iter);
	}
	else
	{
		// if objects is not on level 1 child, we will perform a depth first search for each of the level 1 child. 
		for (auto pChild : m_children)
		{
			nCount = pChild->RemoveChild(pObj, true);

			if(nCount>0)
			{
				break;
			}
		}
	}
	
	return nCount;
}


void ParaEngine::CMiniSceneGraph::Reset()
{
	EnableCamera(false);
	m_pActor = NULL;
	DestroyChildren();
	m_name_map.clear();
	m_sceneState.Cleanup();
}

void ParaEngine::CMiniSceneGraph::AddChild( CBaseObject * pObject )
{
	if(pObject)
	{
		CBaseObject::AddChild(pObject);
		// for level 1 child, use a string map to accelerate map hash. 
		m_name_map[pObject->GetName()] = pObject;
	}
}

CBaseObject* GetObject_recursive(CBaseObject* pObj, float x, float y, float z, float fEpsilon)
{
	for (auto pChild : pObj->GetChildren())
	{
		Vector3 vPos = pChild->GetPosition();

		if((fabs(vPos.x-x)+fabs(vPos.y-y) +fabs(vPos.z -z))<fEpsilon)
		{
			return pChild;
		}
	}
	return NULL;
}

CBaseObject* ParaEngine::CMiniSceneGraph::GetObject(float x, float y, float z, float fEpsilon)
{
	return GetObject_recursive(this, x,y,z, fEpsilon);
}

void CMiniSceneGraph::SetActor(CBaseObject * pActor)
{
	m_pActor = pActor;
}

CBaseObject * CMiniSceneGraph::GetActor()
{
	return m_pActor;
}

const char* ParaEngine::CMiniSceneGraph::GetName()
{
	return GetIdentifier().c_str();
}

void ParaEngine::CMiniSceneGraph::SetName( const char* sName )
{
	SetIdentifier(sName);
}

void CMiniSceneGraph::Draw_Internal(float fDeltaTime)
{
	PERF1("MiniSceneGraph_Draw");
	SceneState& sceneState = m_sceneState;
	m_sceneState.m_pd3dDevice = CGlobals::GetRenderDevice();
	RenderDevicePtr pd3dDevice = m_sceneState.m_pd3dDevice;

	EffectManager* pEffectManager = CGlobals::GetEffectManager();
	bool old_IsLocalLightEnabled = pEffectManager->IsLocalLightingEnabled();

	IScene * pOldScene = pEffectManager->GetScene();
	pEffectManager->SetScene(this);

	//////////////////////////////////////////////////////////////////////////
	// Set some initial states for the normal pipeline
	LinearColor fogColor = GetSunLight().ComputeFogColor();
	fogColor.r *= GetFogColorFactor().r;
	fogColor.g *= GetFogColorFactor().g;
	fogColor.b *= GetFogColorFactor().b;
	SetFogColor(fogColor);
	pEffectManager->EnableFog(IsFogEnabled());
	pEffectManager->SetD3DFogState();
#ifdef USE_DIRECTX_RENDERER
	pd3dDevice->SetMaterial((D3DMATERIAL9*)&(sceneState.GetGlobalMaterial()));
#elif defined(USE_OPENGL_RENDERER)
	if (!IsSunLightEnabled())
	{
		// sun light can not be disabled in shader, so we will fake it here. Remove this when shader support turn off global sun light. 
		GetSunLight().SetSunAmbient(LinearColor::White);
		GetSunLight().SetSunDiffuse(LinearColor::Black);
	}
#endif
	
	// disable local lights on bipeds
	if(old_IsLocalLightEnabled)
		pEffectManager->EnableLocalLighting(false);

	// end previous effects, since they may serve different IScene instance.
	pEffectManager->EndEffect();

	// copy the main scene's state
	sceneState.m_pCurrentEffect = pEffectManager->GetCurrentEffectFile();
	
	// rebuild scene state
	sceneState.dTimeDelta = fDeltaTime;
	PrepareRender(m_pCamera, &sceneState);
	
	{
		// all render code goes here

		/**
		All potentially visible scene objects in the scene state are put to three major post rendering lists for rendering.
		Each list is sorted according to object-to-camera distance. And then they will be rendered in the following order:
		- solid object list: sorted from front to back. In mesh report, it will display "F->B:%d". Their bounding boxes will be rendered as red boxes.
		- transparent object list: sorted from back to front. In mesh report, it will display "B->F:%d(%d1)".
		where %d is the total number of meshes rendered and that %d1 is the number of meshes that has been occluded during occlusion testing.
		Their bounding boxes will be rendered as blue boxes.
		- Biped list: sorted by primary asset. In mesh report, it will display "Biped:%d". Their bounding boxes will be rendered as green boxes.
		*/
		{
			// so that mesh and character that contains translucent faces are sorted and rendered later on. 
			if(!sceneState.GetFaceGroups()->IsEmpty())
			{
				sceneState.GetFaceGroups()->Clear();
			}
			sceneState.m_bEnableTranslucentFaceSorting = true;

			// object num in potentially visible set from current view point
			int nPVSCount = 0; 

			//////////////////////////////////////////////////////////////////////////
			/// Draw SkyBox 
			RenderSelection(RENDER_SKY_BOX);

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
				nPVSCount = RenderSelection(RENDER_MESH_BACK_TO_FRONT);
			}
			
			//////////////////////////////////////////////////////////////////////////
			/// Draw queued bipeds
			/// we draw biped after its shadow, because we do not want biped to cast shadows on its own
			if(!sceneState.listPRBiped.empty())
			{
				RenderSelection(RENDER_CHARACTERS);
			}

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
				// translucent faces are ignored. 
				RenderSelection(RENDER_TRANSLUCENT_FACE_GROUPS);
				sceneState.GetFaceGroups()->Clear();
			}

			//////////////////////////////////////////////////////////////////////////
			/// render all particle system instances after most of the scene have been rendered.
			/// it is rendered after ocean to prevent the ocean color over the particles. since all particles are z write disabled.
			{
				int nParticlesCount = RenderSelection(RENDER_PARTICLES, fDeltaTime);
			}

			//////////////////////////////////////////////////////////////////////////
			// bounding box
			if(CGlobals::GetScene()->IsShowBoundingBox())
				RenderSelection(RENDER_BOUNDINGBOX);

			//////////////////////////////////////////////////////////////////////////
			// portal 
			if(CGlobals::GetScene()->IsPortalSystemShown())
				RenderSelection(RENDER_PORTAL_SYSTEM);
		}
	}
#ifdef USE_DIRECTX_RENDERER
	if(m_pMask!=0 && pEffectManager->BeginEffect(TECH_GUI, &(m_sceneState.m_pCurrentEffect)))
	{
		/** draw the mask square in front of the screen. */
		CEffectFile* pEffectFile = CGlobals::GetEffectManager()->GetCurrentEffectFile();
		if(pEffectFile == 0)
		{
			//////////////////////////////////////////////////////////////////////////
			// fixed programming pipeline
			float sx = (float)m_nTextureWidth,sy = (float)m_nTextureHeight;

			DXUT_SCREEN_VERTEX v[10];
			v[0].x = 0;  v[0].y = sy; v[0].tu = 0;  v[0].tv = 1.0f;
			v[1].x = 0;  v[1].y = 0;  v[1].tu = 0;  v[1].tv = 0;
			v[2].x = sx; v[2].y = sy; v[2].tu = 1.f; v[2].tv = 1.f;
			v[3].x = sx; v[3].y = 0;  v[3].tu = 1.f; v[3].tv = 0;

			DWORD dwColor = LinearColor(1.f,1.f,1.f,1.f);
			int i;
			for(i=0;i<4;i++)
			{
				v[i].color = dwColor;
				v[i].z = 0;
				v[i].h = 1.0f;
			}
			//pd3dDevice->SetTextureStageState( 0, D3DTSS_COLOROP,   D3DTOP_SELECTARG1 );		// modulate operation
			//pd3dDevice->SetTextureStageState( 0, D3DTSS_ALPHAOP,   D3DTOP_SELECTARG2 );
			//
			//pd3dDevice->SetRenderState( D3DRS_COLORWRITEENABLE,
			//	D3DCOLORWRITEENABLE_RED  | D3DCOLORWRITEENABLE_GREEN |
			//	D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_ALPHA );

			pd3dDevice->SetRenderState( D3DRS_COLORWRITEENABLE, D3DCOLORWRITEENABLE_ALPHA );

			pd3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, TRUE);	
			pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_ZERO);
			pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_SRCALPHA);

			/*pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP,D3DTOP_SELECTARG2);
			pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1,D3DTA_TEXTURE);
			pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2,D3DTA_CURRENT);

			pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP,D3DTOP_SELECTARG1);
			pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1,D3DTA_TEXTURE);
			pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2,D3DTA_CURRENT);*/

			pd3dDevice->SetTexture(0, m_pMask->GetTexture());
			RenderDevice::DrawPrimitiveUP( pd3dDevice, RenderDevice::DRAW_PERF_TRIANGLES_MESH, D3DPT_TRIANGLESTRIP, 2, v, sizeof(DXUT_SCREEN_VERTEX) );

			/*pd3dDevice->SetTextureStageState(0, D3DTSS_COLOROP,D3DTOP_MODULATE);
			pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG1,D3DTA_TEXTURE);
			pd3dDevice->SetTextureStageState(0, D3DTSS_COLORARG2,D3DTA_CURRENT);

			pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAOP,D3DTOP_SELECTARG1);
			pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG1,D3DTA_TEXTURE);
			pd3dDevice->SetTextureStageState(0, D3DTSS_ALPHAARG2,D3DTA_CURRENT);*/

			pd3dDevice->SetRenderState(D3DRS_SRCBLEND, D3DBLEND_SRCALPHA);
			pd3dDevice->SetRenderState(D3DRS_DESTBLEND, D3DBLEND_INVSRCALPHA);

			pd3dDevice->SetRenderState( D3DRS_COLORWRITEENABLE,
					D3DCOLORWRITEENABLE_RED  | D3DCOLORWRITEENABLE_GREEN |
					D3DCOLORWRITEENABLE_BLUE | D3DCOLORWRITEENABLE_ALPHA );
		}
	}
#endif

	// draw the head on display GUI
	RenderHeadOnDisplay();

	//////////////////////////////////////////////////////////////////////////
	// Restore state for the normal pipeline

	pEffectManager->SetScene(pOldScene);

	// enable fog
	pEffectManager->EnableFog(CGlobals::GetScene()->IsFogEnabled());

	// restore the main scene's state
	CGlobals::GetSceneState()->m_pCurrentEffect = sceneState.m_pCurrentEffect;

	if(old_IsLocalLightEnabled)
		pEffectManager->EnableLocalLighting(old_IsLocalLightEnabled);
}

template <class T>
void RenderHeadOnDisplayList(T& renderlist, int& nObjCount, SceneState* pSceneState, CGUIText** ppObjUITextDefault)
{
	typename T::const_iterator itCurCP, itEnd = renderlist.end();
	for(itCurCP = renderlist.begin(); itCurCP !=itEnd; ++itCurCP)
	{
		CBaseObject* pObj = (*itCurCP).m_pRenderObject;
		if (pObj != 0 && !IHeadOn3D::DrawHeadOnUI(pObj, nObjCount, pSceneState, ppObjUITextDefault))
			break;
	}
}

int CMiniSceneGraph::RenderHeadOnDisplay()
{
	if(!IsHeadOnDisplayShown())
		return 0;
	SceneState& sceneState = m_sceneState;
	int nObjCount = 0;
	CGUIText* pObjUITextDefault = NULL;

	// update the GUI's view projection matrix for calculating 3d GUI object position
	Matrix4 matViewProj_old;
	Matrix4 matViewProj;
	ParaMatrixMultiply(&matViewProj, (const Matrix4*)&sceneState.mxView, (const Matrix4*)&sceneState.mxProj);
	matViewProj_old = *(CGlobals::GetGUI()->Get3DViewProjMatrix());
	CGlobals::GetGUI()->Set3DViewProjMatrix(matViewProj);
	
	/// -- Draw all characters' head on display including the player itself.
	if(!sceneState.listPRBiped.empty())
		RenderHeadOnDisplayList(sceneState.listPRBiped, nObjCount, &sceneState, &pObjUITextDefault);
	if(!sceneState.listPRSolidObject.empty())
		RenderHeadOnDisplayList(sceneState.listPRSolidObject, nObjCount, &sceneState, &pObjUITextDefault);
	if(!sceneState.listPRTransparentObject.empty())
		RenderHeadOnDisplayList(sceneState.listPRTransparentObject, nObjCount, &sceneState, &pObjUITextDefault);
	if(!sceneState.listPRSmallObject.empty())
		RenderHeadOnDisplayList(sceneState.listPRSmallObject, nObjCount, &sceneState, &pObjUITextDefault);
	
	IHeadOn3D::DrawHeadOnUI(NULL, nObjCount);

	CGlobals::GetGUI()->Set3DViewProjMatrix(matViewProj_old);
	return nObjCount;
}

void ParaEngine::CMiniSceneGraph::Draw(float fDeltaTime)
{
	if(!IsCameraEnabled() || m_pCamera==0)
	{
		OUTPUT_LOG("warning: one can not call CMiniSceneGraph::draw() function when camera is disabled.\n");
		return;
	}

	bool bRenderToTexture = (GetRenderPipelineOrder() < PIPELINE_UI );
	
	m_sceneState.m_pd3dDevice = CGlobals::GetRenderDevice();

	if (IsDirty())
		SetDirty(false);

	if(bRenderToTexture)
	{
		if (!Begin())
			return;
		/** we will allow alpha in render target in case the device support it, if we have fog, the alpha does not take effect. */
		DWORD dwColor = 0xffffffff&(DWORD)(IsFogEnabled()? GetFogColor() : GetBackGroundColor());
		Clear(LinearColor(dwColor));
	}

	// animate the camera and push result to transformation stack
	m_pCamera->FrameMove(fDeltaTime);
	
	if(IsCameraEnabled() || CGlobals::GetWorldMatrixStack().empty())
		CGlobals::GetWorldMatrixStack().push(*CGlobals::GetIdentityMatrix());
	CGlobals::GetProjectionMatrixStack().push(*m_pCamera->GetProjMatrix());
	CGlobals::GetViewMatrixStack().push(*m_pCamera->GetViewMatrix());

	EffectManager* pEffectManager = CGlobals::GetEffectManager();
	pEffectManager->UpdateD3DPipelineTransform(true, true, true);

	// update the culling planes according to view and projection matrix on top of the global stack
	m_pCamera->UpdateFrustum(); 

	m_vRenderOrigin = CGlobals::GetScene()->GetRenderOrigin();

	{
		EffectTextureStateStack tex_state_(pEffectManager, IsUsePointTexture());
		
		/// the actual draw call is here.
		Draw_Internal(fDeltaTime);
	}
	

	//////////////////////////////////////////////////////////////////////////
	// Restore state for the normal pipeline
	if(bRenderToTexture)
	{
		DoPaint();
		End();
	}
	
	// restore transformations
	if(IsCameraEnabled())
		CGlobals::GetWorldMatrixStack().pop();
	CGlobals::GetProjectionMatrixStack().pop();
	CGlobals::GetViewMatrixStack().pop();
	CGlobals::GetEffectManager()->UpdateD3DPipelineTransform(true,true, true);

}


void ParaEngine::CMiniSceneGraph::CameraZoom( float fAmount )
{
	GetCamera()->Zoom(fAmount);
}

void ParaEngine::CMiniSceneGraph::CameraZoomSphere( const CShapeSphere& sphere )
{
	GetCamera()->ZoomSphere(sphere);
}

void ParaEngine::CMiniSceneGraph::CameraRotate( float dx, float dy, float dz )
{
	GetCamera()->Rotate(dx,dy,dz);
}

void ParaEngine::CMiniSceneGraph::CameraPan( float dx, float dy )
{
	GetCamera()->Pan(dx,dy);
}

void ParaEngine::CMiniSceneGraph::CameraSetLookAtPos( float x, float y, float z )
{
	GetCamera()->SetLookAtPos(x,y,z);
}


void ParaEngine::CMiniSceneGraph::CameraGetLookAtPos( float* x, float* y, float* z )
{
	Vector3 vPos = GetCamera()->GetLookAtPosition();	
	*x = vPos.x;
	*y = vPos.y;
	*z = vPos.z;
}

void ParaEngine::CMiniSceneGraph::CameraSetEyePosByAngle( float fRotY, float fLiftupAngle, float fCameraObjectDist)
{
	GetCamera()->SetEyePosByAngle(fRotY,fLiftupAngle,fCameraObjectDist);
}

void ParaEngine::CMiniSceneGraph::CameraGetEyePosByAngle( float* fRotY, float* fLiftupAngle, float* fCameraObjectDist )
{
	GetCamera()->GetEyePosByAngle(fRotY, fLiftupAngle, fCameraObjectDist);	
}

void ParaEngine::CMiniSceneGraph::CameraSetEyePos(double x, double y, double z)
{
	DVector3 vEye(x, y, z);
	GetCamera()->SetViewParams(vEye, GetCamera()->GetLookAtPosition());
}

void ParaEngine::CMiniSceneGraph::CameraGetEyePos(double* x, double* y, double* z)
{
	DVector3 vPos = GetCamera()->GetEyePosition();
	*x = vPos.x;
	*y = vPos.y;
	*z = vPos.z;
}


bool ParaEngine::CMiniSceneGraph::PrepareRenderObject(CBaseObject* pObj, CBaseCamera* pCamera, SceneState& sceneState)
{
	ObjectType oType = pObj->GetMyType();
	IViewClippingObject* pViewClippingObject = pObj->GetViewClippingObject();
	bool bDrawObj = false;
	//////////////////////////////////////////////////////////////////////////
	//
	// [stage passed:] rough object-level culling goes here:it uses the intersection of the bounding sphere of 
	// the frustum and the eye centered fog sphere for rough view testing
	//
	//////////////////////////////////////////////////////////////////////////

	///  if no rough test is performed, further cull object with the camera's view frustum.
	if (pCamera->CanSeeObject(pViewClippingObject))
	{
		bDrawObj = true;
		if (oType == _PC_Zone)
		{
			bDrawObj = false;
			sceneState.listZones.push_back(PostRenderObject(pObj, 0));
			return true;
		}
		else if (oType == _PC_Portal)
		{
			sceneState.listPortals.push_back(PostRenderObject(pObj, 0));
			bDrawObj = false;
			return true;
		}
	}

	/// draw this object
	if (bDrawObj)
	{
		float fObjectToCameraDist = pViewClippingObject->GetObjectToPointDistance(&(sceneState.vEye));
		/** To which post rendering list the object goes to. this is only for debugging purposes.
		* the bounding box of object from different render list are drawn using different colors.
		* 0 means biped post rendering list.
		* 1 means solid object rendering list.
		* 2 means transparent or small rendering list.
		* */
		//int nPostRenderType = 0;
		if (pObj->IsBiped())
		{
			if (bDrawObj)
			{
				PostRenderObject o(pObj, fObjectToCameraDist);
				sceneState.listPRBiped.push_back(o); // this is a biped object
			}
			//////////////////////////////////////////////////////////////////////////
			// animate the visible biped
			((CBipedObject*)pObj)->Animate(sceneState.dTimeDelta);
		}
		else
		{
			/** for ordinary mesh objects, we will check its distance from the camera eye to its surface.
			* if */
			float fObjRenderTangentAngle = 0;
			if (pObj->IsTransparent() && bDrawObj)
			{
				pObj->AutoSelectTechnique();
				sceneState.listPRTransparentObject.push_back(AlphaPostRenderObject(pObj, fObjectToCameraDist));
				//nPostRenderType = 2;
			}
			else
			{
				/// if the mesh is very near the camera, or it has a very big view angle, we will draw it as solid mesh,
				sceneState.listPRSolidObject.push_back(PostRenderObject(pObj, fObjectToCameraDist));
			}
		}
	}

	/// push its child objects to the queue
	for (auto pChild : pObj->GetChildren())
	{
		if (pChild->IsVisible())
		{
			pChild->ViewTouch();
			pChild->PrepareRender(pCamera, &sceneState);
		}
	}
	return bDrawObj;
}

int CMiniSceneGraph::PrepareRender(CBaseCamera* pCamera, SceneState* pSceneState)
{
	SceneState& sceneState = *pSceneState;
	// update sun parameters
	GetSunLight().AdvanceTimeOfDay(0.f);

	///////////////////////////////////////////////////////////////////////////
	//
	// Set the scene state and transformations for the current frame rendering
	//
	//////////////////////////////////////////////////////////////////////////
	sceneState.CleanupSceneState();

	///////////////////////////////////////////////////////////////////////////
	// update the default material
	sceneState.GetGlobalMaterial().Ambient = GetSunLight().GetSunAmbientHue();
	sceneState.GetGlobalMaterial().Diffuse = GetSunLight().GetSunColor();

	sceneState.vEye = pCamera->GetEyePosition();
	sceneState.vLookAt = pCamera->GetLookAtPosition();
	sceneState.mxView = *(pCamera->GetViewMatrix());
	sceneState.mxProj = *(pCamera->GetProjMatrix());

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
	/** for transparent objects */
	std::sort(sceneState.listPRTransparentObject.begin(), sceneState.listPRTransparentObject.end(), GreaterPostRenderObj<AlphaPostRenderObject>());
	/** sort by the primary asset : in this case it's MA from front to back. */
	std::sort(sceneState.listPRBiped.begin(), sceneState.listPRBiped.end(), LessPostRenderObj_NoTechBatch<PostRenderObject>());
	
	sceneState.bIsBatchRender = true;
	return 0;
}

int CMiniSceneGraph::RenderSelection(DWORD dwSelection, double dTimeDelta)
{
	SceneState& sceneState = m_sceneState;

	sceneState.m_nCurRenderGroup = dwSelection;
	int nObjCount = 0;
	if(CHECK_SELECTION(RENDER_SKY_BOX))
	{
		CSkyMesh * pSky = GetCurrentSky();
		if(pSky!=0)
		{
			// TODO: use culling algorithms
			pSky->Draw(&sceneState);
			++nObjCount;
		}		
	}
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
		if(!sceneState.listPRTransparentObject.empty())
			nObjCount += RenderList(sceneState.listPRTransparentObject, sceneState);
	}
	if(CHECK_SELECTION(RENDER_MESH_BACK_TO_FRONT))
	{
		// draw transparent object rendering list from back to front.
		if(!sceneState.listPRSmallObject.empty())
		{
			//////////////////////////////////////////////////////////////////////////
			// render without hardware occlusion testing
			nObjCount += RenderList(sceneState.listPRSmallObject, sceneState);
			sceneState.fAlphaFactor=1.0f;
		}
	}
	if(CHECK_SELECTION(RENDER_CHARACTERS))
	{
		/// -- Draw all characters including the player itself.
		if(!sceneState.listPRBiped.empty())
		{
			CGlobals::GetEffectManager()->BeginEffect(TECH_CHARACTER, &(sceneState.m_pCurrentEffect));
			
			/** sort by the primary asset : in this case it's MA */
			SceneState::List_PostRenderObject_Type::iterator itCurCP, itEndCP = sceneState.listPRBiped.end();
			for( itCurCP = sceneState.listPRBiped.begin(); itCurCP != itEndCP; ++ itCurCP)
			{
				if(itCurCP->m_pRenderObject!=0)
				{
					sceneState.SetCameraToCurObjectDistance((*itCurCP).m_fObjectToCameraDistance);
					((*itCurCP).m_pRenderObject)->Draw(&sceneState);
					++nObjCount;
				}
			}
		}
	}
	if(CHECK_SELECTION(RENDER_BOUNDINGBOX))
	{
		if(GetBatchedElementDrawer())
		{
			// draw the oriented bounding box for debugging and editing.
			{
				GetBatchedElementDrawer()->SetDefaultColor(PARAVECTOR3(1.f,0,0));
				SceneState::List_PostRenderObject_Type::iterator itCurCP, itEndCP = sceneState.listPRSolidObject.end();
				for(itCurCP = sceneState.listPRSolidObject.begin(); itCurCP !=itEndCP; ++itCurCP)
				{
					IViewClippingObject* pViewClippingObject = (*itCurCP).m_pRenderObject->GetViewClippingObject();
					pViewClippingObject->DebugDraw(GetBatchedElementDrawer());
					++nObjCount;
				}
			}
			{
				GetBatchedElementDrawer()->SetDefaultColor(PARAVECTOR3(0, 0, 1.f));
				DWORD dwColor = COLOR_ARGB(255,0,0,255);
				SceneState::List_AlphaPostRenderObject_Type::iterator itCurCP, itEndCP = sceneState.listPRSmallObject.end();
				for(itCurCP = sceneState.listPRSmallObject.begin(); itCurCP !=itEndCP; ++itCurCP)
				{
					IViewClippingObject* pViewClippingObject = (*itCurCP).m_pRenderObject->GetViewClippingObject();
					pViewClippingObject->DebugDraw(GetBatchedElementDrawer());
					++nObjCount;
				}
			}
			{
				SceneState::List_PostRenderObject_Type::iterator itCurCP, itEndCP = sceneState.listPRBiped.end();
				for( itCurCP = sceneState.listPRBiped.begin(); itCurCP != itEndCP; ++ itCurCP)
				{
					if(itCurCP->m_pRenderObject!=0)
					{
						GetBatchedElementDrawer()->SetDefaultColor(PARAVECTOR3(0, 0, 1.f));
						((CBipedObject*)((*itCurCP).m_pRenderObject))->DebugDraw(GetBatchedElementDrawer());
						GetBatchedElementDrawer()->SetDefaultColor(PARAVECTOR3(0, 1.f, 0));
						((CBipedObject*)((*itCurCP).m_pRenderObject))->GetPhysicsBBObj()->DebugDraw(GetBatchedElementDrawer());
						++nObjCount;
					}
				}
			}
			if(! m_pBatchedElementDraw->IsEmpty())
			{
				// Turn off effect: 
				CGlobals::GetEffectManager()->BeginEffect(TECH_NONE, &(sceneState.m_pCurrentEffect));
				m_pBatchedElementDraw->DrawAll();
			}
		}
	}
	if(CHECK_SELECTION(RENDER_TRANSLUCENT_FACE_GROUPS))
	{
		if(!sceneState.GetFaceGroups()->IsEmpty())
		{
			if(!sceneState.GetFaceGroups()->IsSorted())
			{
				sceneState.GetFaceGroups()->Sort(GetCurrentCamera()->GetEyePosition()-GetRenderOrigin());
			}

			CGlobals::GetEffectManager()->BeginEffect(TECH_SIMPLE_MESH_NORMAL);
			CEffectFile* pEffectFile = CGlobals::GetEffectManager()->GetCurrentEffectFile();
			//if(pEffectFile)
			{
				sceneState.GetFaceGroups()->Render();
			}
		}
	}
	if(CHECK_SELECTION(RENDER_PARTICLES))
	{
		bool bEffectSet = false;

		Vector3 vRenderOrigin = GetRenderOrigin();

		int nPSInstancesCount = 0;	// statistic:number of particles instances 
		int nParticlesCount = 0;	// statistic:number of particles 
		SceneState::List_ParticleSystemPtr_Type::iterator itCurCP;
		for (itCurCP = sceneState.listParticleSystems.begin(); itCurCP != sceneState.listParticleSystems.end();)
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
						Vector3 vRenderOriginOffset = (instancePS->m_vLastRenderOrigin) - vRenderOrigin;
						instancePS->m_vLastRenderOrigin = (vRenderOrigin);// update render origin
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
			if(!bHasInstance)
				itCurCP = sceneState.listParticleSystems.erase(itCurCP);
			else
			{
				++ itCurCP;
				if(CGlobals::WillGenReport())
					nPSInstancesCount += (int)instances.size();
			}
		}
		nObjCount = nParticlesCount;
	}
	return nObjCount;
}

Vector3 ParaEngine::CMiniSceneGraph::GetRenderOrigin()
{
	return IsCameraEnabled() ? m_vRenderOrigin : CGlobals::GetScene()->GetRenderOrigin();
}

bool ParaEngine::CMiniSceneGraph::PickObject( int nScreenX, int nScreenY, CBaseObject** pTouchedObject, float fMaxDistance/*=0*/, OBJECT_FILTER_CALLBACK pFnctFilter/*=NULL*/ )
{
	Matrix4 matWorld;
	matWorld = Matrix4::IDENTITY;

	POINT ptCursor;
	float fScaleX = 1.f, fScaleY = 1.f; 
	CGlobals::GetGUI()->GetUIScale(&fScaleX, &fScaleY);
	ptCursor.x = (fScaleX == 1.f) ? nScreenX : (int)(nScreenX*fScaleX);
	ptCursor.y = (fScaleY == 1.f) ? nScreenY : (int)(nScreenY*fScaleY);

	Vector3 vPickRayOrig, vPickRayDir;
	if(IsCameraEnabled())
	{
		GetCamera()->GetMouseRay(vPickRayOrig, vPickRayDir, ptCursor,m_nTextureWidth, m_nTextureHeight, &matWorld);
		return PickObject(CShapeRay(vPickRayOrig+GetRenderOrigin(), vPickRayDir), pTouchedObject, fMaxDistance,pFnctFilter);
	}
	else
	{
		int x = ptCursor.x;
		int y = ptCursor.y;
		int nWidth, nHeight;
		CGlobals::GetViewportManager()->GetPointOnViewport(x, y, &nWidth, &nHeight);
		ptCursor.x = x;
		ptCursor.y = y;
		CGlobals::GetScene()->GetCurrentCamera()->GetMouseRay(vPickRayOrig, vPickRayDir, ptCursor,nWidth, nHeight, &matWorld);
		return PickObject(CShapeRay(vPickRayOrig+GetRenderOrigin(), vPickRayDir), pTouchedObject, fMaxDistance,pFnctFilter);
	}
}

bool ParaEngine::CMiniSceneGraph::PickObject( const CShapeRay& ray, CBaseObject** pTouchedObject, float fMaxDistance/*=0*/, OBJECT_FILTER_CALLBACK pFnctFilter/*=NULL*/ )
{
	bool result = false;
	// the object picked.
	PickedObject lastObj; 

	// save old render origin
	Vector3 vOldRenderOrigin = CGlobals::GetScene()->GetRenderOrigin();
	if(IsCameraEnabled())
	{
		vOldRenderOrigin = CGlobals::GetScene()->GetRenderOrigin();
		CGlobals::GetScene()->RegenerateRenderOrigin(GetRenderOrigin());
	}
	

	CRayCollider rayCollider;
	rayCollider.SetMaxDist(fMaxDistance);

	Vector3 vRenderOrigin=GetRenderOrigin();

	// the ray in the view space, shifted to the render origin.
	CShapeRay ray_view(ray.mOrig-vRenderOrigin, ray.mDir);

	float fViewRadius = fMaxDistance/2;
	Vector3 vViewCenter = ray.mOrig + ray.mDir*fViewRadius;

	{
		// TODO: picking code here
		queue_CBaseObjectPtr_Type queueNodes;

		/// add all children 
		{
			for (auto pChild : m_children)
			{
				if (pChild->IsVisible())
					queueNodes.push(pChild);
			}
		}

		/// For any potentially visible objects in the queue, 
		/// perform further object-level clipping test, and draw them if the test passes.
		while(!queueNodes.empty())
		{
			// pop up one object
			CBaseObject* pObj = queueNodes.front();
			
			queueNodes.pop();
			ObjectType oType =  pObj->GetMyType();
			//////////////////////////////////////////////////////////////////////////
			//
			// [stage passed:] rough object-level culling goes here:it uses the intersection of the bounding sphere of 
			// the frustum and the eye centered fog sphere for rough view testing
			//
			//////////////////////////////////////////////////////////////////////////

			///  if no rough test is performed, further cull object with the camera's view frustum.
			if( !IsVisible() || !(pFnctFilter==NULL || pFnctFilter(pObj)) /* filter object*/)
			{
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
				if(rayCollider.Intersect(ray_view, obb, &fDist) && fDist<=fMaxDistance )
				{
					// add to collision list
					float fMinExtent = Math::MinVec3(obb.mExtents);
					bool bUseCurrent = false;

					if(lastObj.IsValid()==false)
						bUseCurrent = true;
					else
					{
						bool bDetermined = false;

						if(lastObj.m_pObj->GetMyType()>=_Biped && pObj->GetMyType()<_Biped)
						{
							// if the last object is a character and the current one is a non-physical mesh object,
							// we will give character a little priority over smaller mesh object by 
							//  (1) enlarging the character by its radius
							//  (2) always select the character if the camera to character distance is less than 4 times the character radius
							float fRadius = lastObj.m_pObj->GetRadius();
							if(fRadius>fabs(lastObj.m_fRayDist -fDist) 
								|| lastObj.m_fRayDist<fRadius*4)
							{
								// use last character
								bDetermined = true;
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

			/// push its child objects to the queue
			for (auto pChild : pObj->GetChildren())
			{
				if (pChild->IsVisible())
					queueNodes.push(pChild);
			}
			
		} //while(!queueNodes.empty())
	}


	if(pTouchedObject!=0 )
	{
		if(lastObj.IsValid())
		{
			result = true;
			*pTouchedObject = lastObj.m_pObj;
		}
		else
			*pTouchedObject = NULL;
	}
	
	// restore render origin
	if(IsCameraEnabled())
		CGlobals::GetScene()->RegenerateRenderOrigin(vOldRenderOrigin);

	return result;
}

void ParaEngine::CMiniSceneGraph::SetRenderPipelineOrder( int nOrder )
{
	m_nRenderPipelineOrder = nOrder;
}

int ParaEngine::CMiniSceneGraph::GetRenderPipelineOrder()
{
	return m_nRenderPipelineOrder;
}

IBatchedElementDraw* ParaEngine::CMiniSceneGraph::GetBatchedElementDrawer()
{
	return (IBatchedElementDraw*)m_pBatchedElementDraw;
}

bool ParaEngine::CMiniSceneGraph::IsUsePointTexture() const
{
	return m_bUsePointTexture;
}

void ParaEngine::CMiniSceneGraph::SetUsePointTexture(bool val)
{
	m_bUsePointTexture = val;
}

void ParaEngine::CMiniSceneGraph::SetBackGroundColor(const LinearColor& bgColor)
{
	SetClearColor(bgColor);
}

ParaEngine::LinearColor ParaEngine::CMiniSceneGraph::GetBackGroundColor()
{
	return GetClearColor();
}

bool CMiniSceneGraph::IsNeedUpdate()
{
	if(m_pCanvasTexture)
	{
		return m_pCanvasTexture->GetHitCount()>0;
	}
	return false;
}

bool CMiniSceneGraph::IsLightEnabled()
{
	return m_bEnableLighting;
}

void CMiniSceneGraph::EnableLight(bool bEnable)
{
	m_bEnableLighting = bEnable;
}

LinearColor CMiniSceneGraph::GetFogColor()
{
	LinearColor color(m_dwFogColor);
	color *= m_fFogDensity;
	color.a = 1.0f;
	return color;
}

void CMiniSceneGraph::SetFogColor(const LinearColor& fogColor)
{
	m_dwFogColor = fogColor;
}
void CMiniSceneGraph::SetFogStart(float fFogStart)
{
	m_fFogStart = fFogStart;
}

float CMiniSceneGraph::GetFogStart()
{
	return m_fFogStart;
}
void CMiniSceneGraph::SetFogEnd(float fFogEnd)
{
	m_fFogEnd = fFogEnd;
}
float CMiniSceneGraph::GetFogEnd()
{
	return m_fFogEnd;
}
void CMiniSceneGraph::SetFogDensity(float fFogDensity)
{
	m_fFogDensity = fFogDensity;
}
float CMiniSceneGraph::GetFogDensity()
{
	return m_fFogDensity;
}

void CMiniSceneGraph::EnableFog(bool bEnableFog)
{
	m_bRenderFog = bEnableFog;
}

bool CMiniSceneGraph::IsFogEnabled()
{
	return m_bRenderFog;
}

CSkyMesh* CMiniSceneGraph::GetCurrentSky()
{	
	if(m_skymeshes.empty())
		return NULL;
	else
		return m_skymeshes.back();
}
bool CMiniSceneGraph::CreateSkyBox(const string& strObjectName, AssetEntity& MeshAsset, float fScaleX, float fScaleY,float fScaleZ, float fHeightOffset)
{
	// if the object already exists, just select it as the current sky.
	for (auto itCurCP = m_skymeshes.begin(); itCurCP != m_skymeshes.end(); ++itCurCP)
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

bool CMiniSceneGraph::DeleteSkyBox(const string& strObjectName)
{
	if(strObjectName=="")
	{
		m_skymeshes.clear();
	}
	return true;
}

CSunLight& CMiniSceneGraph::GetSunLight()
{	
	/** only used when there is no sky created. */
	return m_SunLight;
}

bool CMiniSceneGraph::IsSunLightEnabled()
{
	return m_bEnableSunLight;
}
void CMiniSceneGraph::EnableSunLight(bool bEnable)
{
	m_bEnableSunLight = bEnable;
}

int CMiniSceneGraph::InstallFields( CAttributeClass* pClass, bool bOverride )
{
	CRenderTarget::InstallFields(pClass, bOverride);

	pClass->AddField("EnableLight", FieldType_Bool, (void*)EnableLight_s, (void*)IsLightEnabled_s, NULL, NULL, bOverride);
	pClass->AddField("EnableSunLight", FieldType_Bool, (void*)EnableSunLight_s, (void*)IsSunLightEnabled_s, NULL, NULL, bOverride);
	pClass->AddField("BackgroundColor", FieldType_Vector3, (void*)SetBackGroundColor_s, (void*)GetBackGroundColor_s, CAttributeField::GetSimpleSchema(SCHEMA_RGB), NULL, bOverride);

	pClass->AddField("EnableFog", FieldType_Bool, (void*)EnableFog_s, (void*)IsFogEnabled_s, NULL, NULL, bOverride);
	pClass->AddField("FogColor", FieldType_Vector3, (void*)SetFogColor_s, (void*)GetFogColor_s, CAttributeField::GetSimpleSchema(SCHEMA_RGB), NULL, bOverride);
	pClass->AddField("FogStart", FieldType_Float, (void*)SetFogStart_s, (void*)GetFogStart_s, NULL, NULL, bOverride);
	pClass->AddField("FogEnd", FieldType_Float, (void*)SetFogEnd_s, (void*)GetFogEnd_s, NULL, NULL, bOverride);
	pClass->AddField("FogDensity", FieldType_Float, (void*)SetFogDensity_s, (void*)GetFogDensity_s, NULL, NULL, bOverride);
	
	pClass->AddField("ShowSky", FieldType_Bool, (void*)EnableSkyRendering_s, (void*)IsRenderSky_s, NULL, NULL, bOverride);
	pClass->AddField("UsePointTexture", FieldType_Bool, (void*)SetUsePointTexture_s, (void*)IsUsePointTexture_s, NULL, NULL, bOverride);
	pClass->AddField("ShowHeadOnDisplay", FieldType_Bool, (void*)ShowHeadOnDisplay_s, (void*)IsHeadOnDisplayShown_s, NULL, NULL, bOverride);
	pClass->AddField("MaxHeadOnDisplayDistance", FieldType_Float, (void*)SetMaxHeadOnDisplayDistance_s, (void*)GetMaxHeadOnDisplayDistance_s, NULL, NULL, bOverride);

	pClass->AddField("RenderPipelineOrder", FieldType_Int, (void*)SetRenderPipelineOrder_s, (void*)GetRenderPipelineOrder_s, NULL, NULL, bOverride);
		
	return S_OK;
}

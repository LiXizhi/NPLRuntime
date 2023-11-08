#pragma once
#include "BaseObject.h"
#include "ShadowVolume.h"
#include "ShapeRay.h"
#include "SceneState.h"
#include "ObjScriptPool.h"
#include "IScene.h"
#include "IObjectDrag.h"
#include "util/ParaMemPool.h"
#include "util/unordered_array.hpp"
#include <list>
#include <map>
#include <queue>

namespace ParaTerrain
{
	class CGlobalTerrain;
}

namespace ParaEngine
{
	class IParaDebugDraw;
	class IBatchedElementDraw;
	class CBatchedElementDraw;
	class BlockWorldClient;
	class CTerrainTileRoot;
	class DropShadowRenderer;
	class CSunLight;
	typedef ObjectScriptPool<CBaseObject> CSceneScripts;

	typedef CTerrainTile* CTerrainTilePtr;
	typedef FixedSizedAllocator<CTerrainTilePtr>	DL_Allocator_CTerrainTilePtr;
	typedef std::deque<CTerrainTilePtr, DL_Allocator_CTerrainTilePtr >	deque_CTerrainTilePtr_Type;
	typedef std::queue<CTerrainTilePtr, deque_CTerrainTilePtr_Type >	queue_CTerrainTilePtr_Type;

	typedef CBaseObject* CBaseObjectPtr;
	typedef FixedSizedAllocator<CBaseObjectPtr>	DL_Allocator_CBaseObjectPtr;
	typedef std::deque<CBaseObjectPtr, DL_Allocator_CBaseObjectPtr >	deque_CBaseObjectPtr_Type;
	typedef std::queue<CBaseObjectPtr, deque_CBaseObjectPtr_Type >	queue_CBaseObjectPtr_Type;

	typedef unordered_array<WeakPtr>	list_IObjectWeakPtr_Type;

	/** call back function type for ray picking. return true if the obj is accepted, false otherwise. */
	typedef bool (*OBJECT_FILTER_CALLBACK)(CBaseObject* obj);
	
	/**
	* The top level scene management class.
	* This is the single most important class in the game engine. It manages all 
	* game objects in a tree hierarchy. The tree is flat in the its root node, but deep
	* on some of its child nodes. For examples, on the root of the scene object is a flat
	* list of engine objects, such as the global terrain, the camera, the global bipeds, 
	* the physics, the asset manager, the render state, the sky boxes, 
	* the quad tree for holding all 3D scene objects, the AI simulator, 2D GUI, etc.
	*
	* The scene object class also controls most global states of the 3D scene, such as fog, shadow and
	* some debugging information.
	*/
	class CSceneObject :
		public CBaseObject,public IObjectDrag, public IScene
	{
	public:
		typedef unordered_ref_array <CAutoCamera*> CameraPool_type;
		typedef unordered_ref_array <CMiniSceneGraph*> MiniSceneGraphPool_type;
		typedef unordered_ref_array <CSkyMesh*> SkyMeshPool_type;
		typedef std::list <ShadowVolume*> ShadowVolumePool_Type;
		typedef unordered_ref_array<CMissileObject*> MissileObjectPool_Type;
#ifdef PARAENGINE_CLIENT
		typedef std::vector <CMirrorSurface*> MirrorSufacePool_Type;
#endif
		CSceneObject();
		virtual ~CSceneObject(void);

		virtual CBaseObject::_SceneObjectType GetType(){return CBaseObject::SceneRoot;};
		static CSceneObject* g_pRootscene;
		/** this function may return NULL, if scene object is never created. */
		static CSceneObject* GetInstance();

		/************************************************************************/
		/* Object-level culling method                                          */
		/************************************************************************/
		/** a bit wise field for object level culling parameters. 
		All method will Automatically adjust the view radius according to the fog near and far plane as well as the size of the object.
		The following formula is used to decide the view radius. 
		Let R be the radius of the object. Let fNear be the near plane distance of the fog, and fFar be the far plane distance of the fog
		The view radius ViewRadius is given be the formula:
		ViewRadius(R) = max((fFar-Pow2(fNear*tanf(m_fCullingAngle))*density*(fFar-fNear)/(R*R)), fNear);
		The formula ensures that the pixel changes of any newly popped out object are a roughly a constant. 
		Small objects will only be drawn when they are very close to the near fog plane, whereas large object will be drawn
		as soon as they are within the far fog plane.
		*/
		enum OBJECT_LEVEL_CULLING_METHOD
		{
			/// the view center is always on camera eye position
			CENTER_ON_CAMERA = 1,
			/// the view center is on current player. if no player selected. it is on the current camera eye position
			CENTER_ON_PLAYER = 0x1<<1,
			/// the view center is the 2d center of the view frustum
			CENTER_ON_FRUSTUM = 0x1<<2,
			/// Use fog far plane distance as the view radius.
			VIEW_RADIUS_FOG = 0x1<<3,
			/// the view frustum's bounding radius
			VIEW_RADIUS_FRUSTUM = 0x1<<4,
		};
	public:

		//////////////////////////////////////////////////////////////////////////
		// implementation of IAttributeFields

		/** attribute class ID should be identical, unless one knows how overriding rules work.*/
		virtual int GetAttributeClassID(){return ATTRIBUTE_CLASSID_CSceneObject;}
		/** a static string, describing the attribute class object's name */
		virtual const char* GetAttributeClassName(){static const char name[] = "CSceneObject"; return name;}
		/** a static string, describing the attribute class object */
		virtual const char* GetAttributeClassDescription(){static const char desc[] = ""; return desc;}
		/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);

		/** get attribute by child object. used to iterate across the attribute field hierarchy. */
		virtual IAttributeFields* GetChildAttributeObject(const char * sName);
		/** get the number of child objects (row count) in the given column. please note different columns can have different row count. */
		virtual int GetChildAttributeObjectCount(int nColumnIndex = 0);
		/** we support multi-dimensional child object. by default objects have only one column. */
		virtual int GetChildAttributeColumnCount();
		virtual IAttributeFields* GetChildAttributeObject(int nRowIndex, int nColumnIndex = 0);

		ATTRIBUTE_METHOD1(CSceneObject, IsModified_s, bool*)	{*p1 = cls->IsModified(); return S_OK;}
		ATTRIBUTE_METHOD1(CSceneObject, SetModified_s, bool)	{cls->SetModified(p1); return S_OK;}

		ATTRIBUTE_METHOD1(CSceneObject, IsUsingFullScreenGlow_s, bool*)	{*p1 = cls->IsUsingFullScreenGlow(); return S_OK;}
		ATTRIBUTE_METHOD1(CSceneObject, EnableFullScreenGlow_s, bool)	{cls->EnableFullScreenGlow(p1); return S_OK;}
		

		ATTRIBUTE_METHOD1(CSceneObject, GetFullscreenGlowIntensity_s, float*)	{*p1 = cls->GetFullscreenGlowIntensity(); return S_OK;}
		ATTRIBUTE_METHOD1(CSceneObject, SetFullscreenGlowIntensity_s, float)	{cls->SetFullscreenGlowIntensity(p1); return S_OK;}

		ATTRIBUTE_METHOD1(CSceneObject, GetFullscreenGlowBlendingFactor_s, float*)	{*p1 = cls->GetFullscreenGlowBlendingFactor(); return S_OK;}
		ATTRIBUTE_METHOD1(CSceneObject, SetFullscreenGlowBlendingFactor_s, float)	{cls->SetFullscreenGlowBlendingFactor(p1); return S_OK;}

		ATTRIBUTE_METHOD1(CSceneObject, GetGlowness_s, Vector4*)		{*p1 = *(Vector4*)(&cls->GetGlowness()); return S_OK;}
		//ATTRIBUTE_METHOD1(CSceneObject, SetGlowness_s, Vector4)		{LinearColor c(p1.x, p1.y,p1.z,1); cls->SetGlowness(c); return S_OK;}
		//change glowness type to Vector4  --clayman 2011.7.19
		ATTRIBUTE_METHOD1(CSceneObject, SetGlowness_s, Vector4)		{ cls->SetGlowness(p1); return S_OK;}

		ATTRIBUTE_METHOD1(CSceneObject, IsSunLightEnabled_s, bool*)	{*p1 = cls->IsSunLightEnabled(); return S_OK;}
		ATTRIBUTE_METHOD1(CSceneObject, EnableSunLight_s, bool)	{cls->EnableSunLight(p1); return S_OK;}

		ATTRIBUTE_METHOD1(CSceneObject, IsLightEnabled_s, bool*)	{*p1 = cls->IsLightEnabled(); return S_OK;}
		ATTRIBUTE_METHOD1(CSceneObject, EnableLight_s, bool)	{cls->EnableLight(p1); return S_OK;}

		ATTRIBUTE_METHOD1(CSceneObject, IsShowLocalLightMesh_s, bool*)	{*p1 = cls->IsShowLocalLightMesh(); return S_OK;}
		ATTRIBUTE_METHOD1(CSceneObject, ShowLocalLightMesh_s, bool)	{cls->ShowLocalLightMesh(p1); return S_OK;}

		ATTRIBUTE_METHOD1(CSceneObject, GetMaxLocalLightsNum_s, int*)	{*p1 = cls->GetMaxLocalLightsNum(); return S_OK;}
		ATTRIBUTE_METHOD1(CSceneObject, SetMaxLocalLightsNum_s, int)	{cls->SetMaxLocalLightsNum(p1); return S_OK;}

		ATTRIBUTE_METHOD1(CSceneObject, IsShadowMapEnabled_s, bool*)	{*p1 = cls->IsShadowMapEnabled(); return S_OK;}
		ATTRIBUTE_METHOD1(CSceneObject, SetShadow_s, bool)	{cls->SetShadow(p1); return S_OK;}
		
		ATTRIBUTE_METHOD1(CSceneObject, GetMaximumNumShadowCasters_s, int*)	{*p1 = cls->GetMaximumNumShadowCasters(); return S_OK;}
		ATTRIBUTE_METHOD1(CSceneObject, SetMaximumNumShadowCasters_s, int)	{cls->SetMaximumNumShadowCasters(p1); return S_OK;}

		ATTRIBUTE_METHOD1(CSceneObject, GetMaximumNumShadowReceivers_s, int*)	{*p1 = cls->GetMaximumNumShadowReceivers(); return S_OK;}
		ATTRIBUTE_METHOD1(CSceneObject, SetMaximumNumShadowReceivers_s, int)	{cls->SetMaximumNumShadowReceivers(p1); return S_OK;}

		ATTRIBUTE_METHOD1(CSceneObject, GetMaxCharTriangles_s, int*)	{*p1 = cls->GetMaxCharTriangles(); return S_OK;}
		ATTRIBUTE_METHOD1(CSceneObject, SetMaxCharTriangles_s, int)	{cls->SetMaxCharTriangles(p1); return S_OK;}

		ATTRIBUTE_METHOD1(CSceneObject, GetBackGroundColor_s, Vector3*)		{ *p1 = cls->GetBackGroundColor().ToVector3(); return S_OK;}
		ATTRIBUTE_METHOD1(CSceneObject, SetBackGroundColor_s, Vector3)		{LinearColor c(p1.x, p1.y,p1.z,1); cls->SetBackGroundColor(c); return S_OK;}

		ATTRIBUTE_METHOD1(CSceneObject, IsFogEnabled_s, bool*)	{*p1 = cls->IsFogEnabled(); return S_OK;}
		ATTRIBUTE_METHOD1(CSceneObject, EnableFog_s, bool)	{cls->EnableFog(p1); return S_OK;}

		ATTRIBUTE_METHOD1(CSceneObject, GetFogColor_s, Vector3*)		{ *p1 = cls->GetFogColorFactor().ToVector3(); return S_OK; }
		ATTRIBUTE_METHOD1(CSceneObject, SetFogColor_s, Vector3)		{LinearColor c(p1.x, p1.y,p1.z,1); cls->SetFogColorFactor(c); return S_OK;}

		ATTRIBUTE_METHOD1(CSceneObject, GetFogStart_s, float*)	{*p1 = cls->GetFogStart(); return S_OK;}
		ATTRIBUTE_METHOD1(CSceneObject, SetFogStart_s, float)	{cls->SetFogStart(p1); return S_OK;}

		ATTRIBUTE_METHOD1(CSceneObject, GetFogEnd_s, float*)	{*p1 = cls->GetFogEnd(); return S_OK;}
		ATTRIBUTE_METHOD1(CSceneObject, SetFogEnd_s, float)	{cls->SetFogEnd(p1); return S_OK;}

		ATTRIBUTE_METHOD1(CSceneObject, GetFogDensity_s, float*)	{*p1 = cls->GetFogDensity(); return S_OK;}
		ATTRIBUTE_METHOD1(CSceneObject, SetFogDensity_s, float)	{cls->SetFogDensity(p1); return S_OK;}
		
		ATTRIBUTE_METHOD1(CSceneObject, GetMinPopUpDistance_s, float*)	{*p1 = cls->GetMinPopUpDistance(); return S_OK;}
		ATTRIBUTE_METHOD1(CSceneObject, SetMinPopUpDistance_s, float)	{cls->SetMinPopUpDistance(p1); return S_OK;}

		ATTRIBUTE_METHOD1(CSceneObject, GetOnClickDistance_s, float*)	{*p1 = cls->GetOnClickDistance(); return S_OK;}
		ATTRIBUTE_METHOD1(CSceneObject, SetOnClickDistance_s, float)	{cls->SetOnClickDistance(p1); return S_OK;}

		ATTRIBUTE_METHOD1(CSceneObject, IsRenderSky_s, bool*)	{*p1 = cls->IsRenderSky(); return S_OK;}
		ATTRIBUTE_METHOD1(CSceneObject, EnableSkyRendering_s, bool)	{cls->EnableSkyRendering(p1); return S_OK;}

		ATTRIBUTE_METHOD1(CSceneObject, IsScenePaused_s, bool*)	{*p1 = cls->IsScenePaused(); return S_OK;}
		ATTRIBUTE_METHOD1(CSceneObject, PauseScene_s, bool)	{cls->PauseScene(p1); return S_OK;}

		ATTRIBUTE_METHOD1(CSceneObject, IsSceneEnabled_s, bool*)	{*p1 = cls->IsSceneEnabled(); return S_OK;}
		ATTRIBUTE_METHOD1(CSceneObject, EnableScene_s, bool)	{cls->EnableScene(p1); return S_OK;}

		ATTRIBUTE_METHOD1(CSceneObject, IsShowBoundingBox_s, bool*)	{*p1 = cls->IsShowBoundingBox(); return S_OK;}
		ATTRIBUTE_METHOD1(CSceneObject, ShowBoundingBox_s, bool)	{cls->ShowBoundingBox(p1); return S_OK;}

		ATTRIBUTE_METHOD1(CSceneObject, IsGenerateReport_s, bool*)	{*p1 = cls->IsGenerateReport(); return S_OK;}
		ATTRIBUTE_METHOD1(CSceneObject, SetGenerateReport_s, bool)	{cls->SetGenerateReport(p1); return S_OK;}

		ATTRIBUTE_METHOD1(CSceneObject, IsPortalSystemShown_s, bool*)	{*p1 = cls->IsPortalSystemShown(); return S_OK;}
		ATTRIBUTE_METHOD1(CSceneObject, ShowPortalSystem_s, bool)	{cls->ShowPortalSystem(p1); return S_OK;}

		ATTRIBUTE_METHOD1(CSceneObject, IsAutoPlayerRippleEnabled_s, bool*)	{*p1 = cls->IsAutoPlayerRippleEnabled(); return S_OK;}
		ATTRIBUTE_METHOD1(CSceneObject, EnableAutoPlayerRipple_s, bool)	{cls->EnableAutoPlayerRipple(p1); return S_OK;}

		ATTRIBUTE_METHOD1(CSceneObject, IsHeadOnDisplayShown_s, bool*)	{*p1 = cls->IsHeadOnDisplayShown(); return S_OK;}
		ATTRIBUTE_METHOD1(CSceneObject, ShowHeadOnDisplay_s, bool)	{cls->ShowHeadOnDisplay(p1); return S_OK;}

		ATTRIBUTE_METHOD1(CSceneObject, IsPortalZoneEnabled_s, bool*)	{*p1 = cls->IsPortalZoneEnabled(); return S_OK;}
		ATTRIBUTE_METHOD1(CSceneObject, EnablePortalZone_s, bool)	{cls->EnablePortalZone(p1); return S_OK;}

		ATTRIBUTE_METHOD1(CSceneObject, IsUseWireFrame_s, bool*)	{*p1 = cls->IsUseWireFrame(); return S_OK;}
		ATTRIBUTE_METHOD1(CSceneObject, SetUseWireFrame_s, bool)	{cls->SetUseWireFrame(p1); return S_OK;}

		ATTRIBUTE_METHOD1(CSceneObject, IsForceExportPhysics_s, bool*)	{*p1 = cls->IsForceExportPhysics(); return S_OK;}
		ATTRIBUTE_METHOD1(CSceneObject, SetForceExportPhysics_s, bool)	{cls->SetForceExportPhysics(p1); return S_OK;}

		ATTRIBUTE_METHOD1(CSceneObject, IsInstancingEnabled_s, bool*)	{*p1 = cls->IsInstancingEnabled(); return S_OK;}
		ATTRIBUTE_METHOD1(CSceneObject, EnableInstancing_s, bool)	{cls->EnableInstancing(p1); return S_OK;}

		ATTRIBUTE_METHOD1(CSceneObject, GetMaxHeadOnDisplayDistance_s, float*)	{*p1 = cls->GetMaxHeadOnDisplayDistance(); return S_OK;}
		ATTRIBUTE_METHOD1(CSceneObject, SetMaxHeadOnDisplayDistance_s, float)	{cls->SetMaxHeadOnDisplayDistance(p1); return S_OK;}
		
		ATTRIBUTE_METHOD1(CSceneObject, IsPersistent_s, bool*)		{*p1 = cls->IsPersistent(); return S_OK;}
		ATTRIBUTE_METHOD1(CSceneObject, SetPersistent_s, bool)		{cls->SetPersistent(p1); return S_OK;}

		ATTRIBUTE_METHOD(CSceneObject, ScreenShotReflection_s)	{return cls->ScreenShotReflection() ? S_OK:E_FAIL;}
		ATTRIBUTE_METHOD(CSceneObject, ScreenShotShadowMap_s)	{return cls->ScreenShotShadowMap() ? S_OK:E_FAIL;}
		ATTRIBUTE_METHOD(CSceneObject, ScreenShotGlowMap_s)	{return cls->ScreenShotGlowMap() ? S_OK:E_FAIL;}
		ATTRIBUTE_METHOD(CSceneObject, ClearParticles_s)	{cls->ClearParticles(); return  S_OK;}
		ATTRIBUTE_METHOD(CSceneObject, UnLoadFlashTextures_s)	{return cls->UnLoadFlashTextures() ? S_OK:E_FAIL;}
		
		ATTRIBUTE_METHOD1(CSceneObject, GetPhysicsDebugDrawMode_s, int*)	{*p1 = cls->GetPhysicsDebugDrawMode(); return S_OK;}
		ATTRIBUTE_METHOD1(CSceneObject, SetPhysicsDebugDrawMode_s, int)	{cls->SetPhysicsDebugDrawMode(p1); return S_OK;}

		ATTRIBUTE_METHOD1(CSceneObject, IsBlockInput_s, bool*)	{*p1 = cls->IsBlockInput(); return S_OK;}
		ATTRIBUTE_METHOD1(CSceneObject, SetBlockInput_s, bool)	{cls->SetBlockInput(p1); return S_OK;}

		ATTRIBUTE_METHOD1(CSceneObject, GetAllMiniSceneGraphNames_s, const char**)	{*p1 = cls->GetAllMiniSceneGraphNames().c_str(); return S_OK;}
		
		ATTRIBUTE_METHOD1(CSceneObject, GetShadowRadius_s, float*)	{*p1 = cls->GetShadowRadius(); return S_OK;}
		ATTRIBUTE_METHOD1(CSceneObject, SetShadowRadius_s, float)	{cls->SetShadowRadius(p1); return S_OK;}

		ATTRIBUTE_METHOD1(CSceneObject, IsDropShadow_s, bool*)	{*p1 = cls->IsDropShadow(); return S_OK;}
		ATTRIBUTE_METHOD1(CSceneObject, SetDropShadow_s, bool)	{cls->SetDropShadow(p1); return S_OK;}

		ATTRIBUTE_METHOD1(CSceneObject, IsUsingScreenWaveEffect_s, bool*)	{*p1 = cls->IsUsingScreenWaveEffect(); return S_OK;}
		ATTRIBUTE_METHOD1(CSceneObject, SetEnableScreenWaveEffect_s, bool)	{cls->EnableScreenWaveEffect(p1); return S_OK;}

		ATTRIBUTE_METHOD1(CSceneObject, IsShowMainPlayer_s, bool*)	{*p1 = cls->IsShowMainPlayer(); return S_OK;}
		ATTRIBUTE_METHOD1(CSceneObject, ShowMainPlayer_s, bool)	{cls->ShowMainPlayer(p1); return S_OK;}

		ATTRIBUTE_METHOD1(CSceneObject, SetShadowMapTexelSize_s, int)	{ cls->SetShadowMapTexelSize(p1, p1); return S_OK; }
	public:
		/** show or hide all scene's objects' head on display*/
		void ShowHeadOnDisplay(bool bShow){m_bShowHeadOnDisplay = bShow;};

		/** whether all scene's objects' head on display*/
		bool IsHeadOnDisplayShown(){return m_bShowHeadOnDisplay;};
		
		/** show or hide all scene's objects' head on display*/
		void SetMaxHeadOnDisplayDistance(float fDist){m_fMaxHeadOnDisplayDistance = fDist;};

		/** whether all scene's objects' head on display*/
		float GetMaxHeadOnDisplayDistance(){return m_fMaxHeadOnDisplayDistance;};

		/** take a screen shot of the current reflection map and save it to ./screenshot directory */
		bool ScreenShotReflection();
		/** take a screen shot of the current shadow map and save it to ./screenshot directory */
		bool ScreenShotShadowMap();
		/** take a screen shot of the current glow texture and save it to ./screenshot directory */
		bool ScreenShotGlowMap();

		/** unload all flash textures. this is sometimes used to close music and delete all flash windows. */
		bool UnLoadFlashTextures();

		/** clear all particles. useful for switching scenes. */
		void ClearParticles();

		/** set whether physics of MeshPhysicsObject is always exported regardless of per object settings. */
		void SetForceExportPhysics(bool bWhetherPhysicsAreAlwaysExported);

		/** get whether physics of MeshPhysicsObject is always exported regardless of per object settings. */
		bool IsForceExportPhysics();

		/**  all objects must be drawn when they are within this radius. Default value is 15 meters	*/
		float GetMinPopUpDistance(){return m_fMinPopUpDistance;};
		void SetMinPopUpDistance(float fDist){m_fMinPopUpDistance = fDist;}
		
		/** whether snap to terrain */
		bool IsSnapToTerrain(){return m_bSnapToTerrain;};

		/** whether to draw bounding box of scene objects. */
		bool IsShowBoundingBox(){return m_bShowBoundingBox;};
		void ShowBoundingBox(bool bEnable){m_bShowBoundingBox = bEnable;}

		/** whether to use generate scene report. */
		bool IsGenerateReport(){return m_bGenerateReport;};
		void SetGenerateReport(bool bEnable){m_bGenerateReport = bEnable;}
		
		/** whether to use occlusion query algorithm for rendering. */
		bool IsOcclusionQueryEnabled(){return m_bEnableOcclusionQuery;};
		void EnableOcclusionQuery(bool bEnable){m_bEnableOcclusionQuery = bEnable;}

		/** whether to use instancing. It is disabled by default. */
		bool IsInstancingEnabled(){return m_bUseInstancing;};
		void EnableInstancing(bool bEnable){m_bUseInstancing = bEnable;}

		/** whether to show main player. default to true.  */
		bool IsShowMainPlayer();
		void ShowMainPlayer(bool bEnable);

		/** whether current camera allows showing the main player */
		bool CanShowMainPlayer() const;
		void SetCanShowMainPlayer(bool val);

		/** when a scene is paused, all animation will be frozen.*/
		bool IsScenePaused(){return m_bGamePaused;};
		void PauseScene(bool bEnable);

		/** whether 3D scene is enabled or not. a disabled scene is not visible no matter what*/
		bool IsSceneEnabled(){return m_bGameEnabled;};
		void EnableScene(bool bEnable);
		
		/** set whether scene is modified */
		void SetModified(bool bModified);
		/** Get whether scene is modified */
		bool IsModified();

		/** get light enable state */
		bool IsSunLightEnabled();
		/** whether use light */
		void EnableSunLight(bool bEnable);

		/** get light enable state */
		bool IsLightEnabled();
		/** whether use light */
		void EnableLight(bool bEnable);

		/**
		* set maximum number of local lights per object. 
		* @param nNum must be smaller than some internal value, currently its upper limit is 8
		*/
		void SetMaxLocalLightsNum(int nNum);

		/**
		* get maximum number of local lights per object. 
		* @return 
		*/
		int GetMaxLocalLightsNum();

		/** whether show the arrow mesh associated with the light object.*/
		bool IsShowLocalLightMesh();
		/** show the arrow mesh associated with the light object */
		void ShowLocalLightMesh(bool bShow);

		/** whether rendering the sky */
		bool IsRenderSky(){ return m_bRenderSky; }
		void EnableSkyRendering(bool bEnable){m_bRenderSky = bEnable;};

		/** set the color of the scene ground when it is not enabled.When scene is enabled, the background color is always the fog color. */
		void SetBackGroundColor(const LinearColor& bgColor);
		/** Get the color of the scene ground when it is not enabled.When scene is enabled, the background color is always the fog color. */
		LinearColor GetBackGroundColor();

		/** Get the color of the scene ground when it is not enabled.When scene is enabled, the background color is always the fog color. */
		LinearColor GetClearColor();

		/** fog parameters */
		void SetFogColorFactor(const LinearColor& fogColorFactor){m_FogColorFactor = fogColorFactor;};
		const LinearColor& GetFogColorFactor(){return m_FogColorFactor;};
		void SetFogColor(const LinearColor& fogColor);
		LinearColor GetFogColor();
		void SetFogStart(float fFogStart);
		float GetFogStart();
		void SetFogEnd(float fFogEnd);
		float GetFogEnd();
		void SetFogDensity(float fFogDensity);
		float GetFogDensity();
		void EnableFog(bool bEnableFog);
		bool IsFogEnabled();
		void ResetCameraAndFog();
		void SetAndRestoreFog(bool bEnableFog, DWORD dwFogColor, FLOAT fFogStart, FLOAT fFogEnd, FLOAT fFogDensity);
		
		/** whether render shadow */
		void SetShadow(bool bRenderShadow);
		/** whether render using shadow map */
		bool IsShadowMapEnabled();

		/** get the maximum number of shadow casters. */
		int GetMaximumNumShadowCasters();

		/** set the maximum number of shadow casters. */
		void SetMaximumNumShadowCasters(int nNum);

		/** get the maximum number of shadow receivers. */
		int GetMaximumNumShadowReceivers();

		/** set the maximum number of shadow receivers. */
		void SetMaximumNumShadowReceivers(int nNum);

		/** set max number of character triangles allowed. we render characters from front to back. and if the previously drawn character count exceed this value
		* we will stop rendering the rest of the characters. this value defaults to 50000*/
		void SetMaxCharTriangles(int nNum);

		/** get max number of character triangles allowed. we render characters from front to back. and if the previously drawn character count exceed this value
		* we will stop rendering the rest of the characters. this value defaults to 50000*/
		int  GetMaxCharTriangles();

		/** set the max number of objects to be rendered of a given render importance. 
		* One can set the render importance of any objects by setting its "RenderImportance" property
		* @param nRenderImportance: the render importance to set. All objects are by default set with render importance 0. 
		* @param nCount: the max number to set for the above render importance. 
		*/
		void SetMaxRenderCount(int nRenderImportance, int nCount);
		/** Get the max number of objects to be rendered of a given render importance. 
		* One can set the render importance of any objects by setting its "RenderImportance" property
		* @param nRenderImportance: the render importance to set. All objects are by default set with render importance 0. 
		* @return: the max number to set for the above render importance. 
		*/
		int GetMaxRenderCount(int nRenderImportance);
		
		/** whether full screen glow effect is used. */
		bool IsUsingFullScreenGlow();

		/** whether full screen glow effect is used. */
		void EnableFullScreenGlow( bool bEnable);

		bool IsUsingScreenWaveEffect();
		void EnableScreenWaveEffect(bool value);

		/**
		* Set/Get cursor file when mouse is over it. If empty, the parent cursor file is used. 
		**/
		void SetCursor(const char* szCursorFile, int nHotSpotX = -1, int nHotSpotY = -1);
		const std::string& GetCursor(int* pnHotSpotX = 0, int* pnHotSpotY = 0) const;
		
		//now glowness contains 4 control value
		//glowness.x : glow saturate
		//glowness.y : glow brightness/intensity
		//glowness.z : original scene saturate
		//glowness.w : original scene brightness/intensity  --clayman 2011.7.19

		void SetGlowness(const Vector4& Glowness);
		const Vector4& GetGlowness();


		/** Set the size level of the shadow map.
		@param nLevel: 0 means 1024 texels; 1 means 1536 texels. default is level 0.*/
		void SetShadowMapTexelSizeLevel(int nLevel);
		/** Set the size of the shadow map.*/
		void SetShadowMapTexelSize(int nWidth, int nHeight);

	public:
		/** Create a new managed missile object.*/
		CMissileObject* NewMissile();
		/** 
		* @param dwMethod: a bit wise combination of OBJECT_LEVEL_CULLING_METHOD 
		*/
		bool CheckObjectCullingMethod(DWORD dwMethod){
			return ((dwMethod & m_dwObjCullingMethod)==dwMethod); 
		};
		/** get the managed loader by its name 
		* @return: NULL is returned if object not found. */
		CManagedLoader* GetManagedLoader(string sName);
		/** delete a managed loader */
		bool DeleteManagedLoader(string sName);
		/** create a new managed loader. if the loader already exists, the old one is returned. 
		* this function will never return NULL.*/
		CManagedLoader* CreateManagedLoader(string sName);

		void AddConsoleStr(const char * pStr);
		void ClearConsoleStr();
		const char* GetConsoleString() const;
				
		/** when the device used by the scene have been initialized and ready for drawing. */
		bool IsInitialized();

		/**
		* This function is called manually to update characters in a square region. So that when the terrain heightfield is changed,
		* the characters within the region will act accordingly, either falling down or snap to terrain surface. 
		* @param vCenter : center of the terrain region being modified. 
		* @param fRadius : radius of the terrain region being modified. 
		*/
		void OnTerrainChanged(const Vector3& vCenter, float fRadius);

		/**
		* reset the scene to blank. 
		*/
		void ResetScene();
		/** get the render origin in the rendering coordinate system.rendering coordinate system
		* is a 3D coordinate system parallel to the world coordinate system, but with a different origin
		* called render origin.Render origin is an arbitrary 3D point chosen near the current camera 
		* eye position. All scene objects and the camera view are transformed to the rendering coordinate system
		* for rendering. This is because if we use the world coordinate system for rendering, the components of 
		* transform matrix that is used for object manipulation will be inconsistent (i.e. of different magnitudes.) 
		* By doing so, it will correct floating point calculation imprecisions, due to large numbers in object's 
		* coordinates, such as the simple addition of (20000.01f+0.01234f). The result of using world 
		* coordinate system for rendering is that we will get increasingly jerky object on screen as 
		* its global position shifts away from the origin of the world coordinate system.(this is usually the case for games
		* with a very large map. One may consider using double type to store the global position of scene object in world units.) 
		* In the rendering coordinate system, however, all components in the matrix will be comparatively small 
		* and consistent. Hence objects can be positioned more precisely near the render origin, which is 
		* near the camera eye position. */
		Vector3 GetRenderOrigin();
		/** generate the render origin, so that it is near the given point in the world units 
		* this function is called by the camera class. As a general rule, whenever the camera moves,
		* the render origin should be regenerated.
		* @param vPos: this is usually the current camera eye position in the world unit. 
		* @see GetRenderOrigin() */
		void RegenerateRenderOrigin(const Vector3& vPos);

		/** clean up all objects in the delete list. This function is called at the beginning of each frame,
		* during the rest of the frame, objects may be added to the delete list for deletion in the next frame.*/
		// void CleanDeleteList();

		/** the global terrain object */
		ParaTerrain::CGlobalTerrain*	GetGlobalTerrain();

		/** return true if the object is found and deleted. */
		bool DeleteSentientObject(IGameObject* pObj);
		/** add a new sentient object to the back of the sentient object list. The sentient count of the newly added object is automatically increased by 1
		* @param bCheckDuplicate: if true, If the object is already in the list, it will not be reinserted. */
		void AddSentientObject(IGameObject* pObj, bool bCheckDuplicate = false);

		/** get the root terrain tile of the quad tree.*/
		inline CTerrainTileRoot* GetRootTile()	{return m_pTileRoot.get();};

		/** get current camera*/
		CBaseCamera* GetCurrentCamera();
		void SetCurrentCamera(CBaseCamera* pCamera);


		/** return NULL if no sky defined.*/
		CSkyMesh*				GetCurrentSky();
		/** create a sky box and add it to the current list, and select it as the current skybox. 
		* sky box with the same name will not be recreated,but will be selected as the current sky box. 
		* @param strObjectName: this can be "". 
		* return true if succeeded
		*/
		bool CreateSkyBox(const string& strObjectName, AssetEntity& MeshAsset, float fScaleX, float fScaleY,float fScaleZ, float fHeightOffset);
		/** delete a name sky box. 
		* @param strObjectName: if this is "", all sky boxes will be deleted.*/
		bool DeleteSkyBox(const string& strObjectName);

		CSunLight&				GetSunLight();

		/*
		* @absolete: this function is no longer used. 
		* detach a specified object from the scene and destroy it. A destroyed object can no longer be used.
		* @param id: if id == "\\", the entire scene is deleted and game is put into unloaded state and paused. 
		*	asset is not released by this call.
		*/
		void DestroyObjectByName(const char * id);
		/**
		* Automatically attach a scene object to the scene graph according to its type and position. 
		* The object can be a manager loader, a global object or any ordinary scene object.
		*    - For tiled object, it is added to the smallest CTerrainTile in the quad-tree
		*    - For global tiled object, it is added to the root CTerrainTile
		*    - For non-tiled object, it is added to an automatically created CContainerObject whose name is the class identifier name of the object. 
		*       hence objects are automatically grouped by class type on the root scene's child nodes. 
		*       To explicitly add an object to a specified parent, use AddChild() method on the parent node. 
		* @return: parent object is returned if successfully attached. 
		* For tiled object, this is the smallest terrain tile that contains the object.
		* For non-tiled object, this is the automatically created CContainerObject that.
		* @note: If the object has already been attached to the scene, it will be removed and reattached. 
		* In most cases, a strong reference of the object is kept by its parent.  
		*/
		IObject* AttachObject(CBaseObject * pObject);

		/**
		* detach an object from the scene graph. 
		* The object can be a manager loader, a global object or any ordinary scene object.
		* Do not delete the object immediately after detaching it from the scene, unless
		* you are sure that there are no references to it elsewhere.
		* currently, the object's type and location is used to locate the object in the scene graph in a very fast way. 
		* It will only search for objects which are attached to terrain tile. If an object is attached to another scene object
		* it will not be found. the function will return once the first matching object is found and detached
		* @param pObject: the object to be detached.
		* @return: true if the object has been found and detached.
		*/
		bool DetachObject(CBaseObject * pObject);
		
		/** get the global object by its name. If there have been several objects with the same name,
		* the most recently attached object is returned.
		* @param sName: exact name of the object
		*/
		virtual CBaseObject* GetGlobalObject(const string& sName);

		/**
		* Get an object. Call this function to see if object with a given identifier is already attached.
		* @param sName name of the object, if this is "", the function always returns NULL
		* @param vPos: the location to search for the object. 
		*/
		CBaseObject* GetLocalObject(const string& sName, const Vector3& vPos);
		
		/**
		* get the first local object,whose position is very close to vPos. This function will search for the first (local mesh) object throughout the hierachy of the scene.
		* this function is kind of slow, please do not call on a per frame basis. Use GetObjectByViewBox() to get an object faster. 
		* @param vPos: world position of the local mesh object
		* @param fEpsilon: if a mesh is close enough to vPos within this value. 
		* @return: NULL if not found
		*/
		CBaseObject* GetLocalObject(const Vector3& vPos, float fEpsilon=0.01f);
		CBaseObject* GetLocalObject(const Vector3& vPos, const std::string& sName, float fEpsilon=0.01f);

		/**
		* this function is just a short cut for (bGlobal ? GetGlobalObject():GetLocalObject())
		* @param sName 
		* @param vPos 
		* @param bGlobal if this is true, vPos can be any value.
		* @return NULL if not found
		*/
		CBaseObject* GetObject(const string& sName, const Vector3& vPos, bool bGlobal);

		/** find a named object by its name. It is usually used to search for automatically attached objects, such as RenderTargets.  
		* this function is not fast, since it will search linearly. 
		*/
		CBaseObject* FindObjectByNameAndType(const std::string& sName, const std::string& sClassName);

		/**
		* get an object(usually a static mesh object) by a given view box. 
		* @param viewbox: One can get the view box by getting the view culling object. see example in the ParaObject:GetViewBox().
		* @return: return the object with the closest match with the bounding box. NULL is returned if there is no match. 
		*/
		CBaseObject* GetObjectByViewBox(const CShapeAABB& viewbox);

		/**
		* Get objects inside or intersect with a sphere. By default it just uses a rough testing between spheres.
		* This function is usually used for finding other static mesh objects near a certain character. 
		* @param output: list to get the result
		* @param sphere: sphere, if this is big, it could be time consuming.
		* @param pFnctFilter: a callback function to further filter selected object. if it is NULL, any scene object could be selected.
		* @param nMethod: testing method, 0 for rough. 
		* @return: return the number of objects in sphere.
		*/
		int GetObjectsBySphere(list<CBaseObject*>& output, const CShapeSphere& sphere, OBJECT_FILTER_CALLBACK pFnctFilter=NULL, int nMethod=0);

		/**
		* Get objects inside or intersect with a screen rect. screen rect is translated to a 3d cone from the camera eye position to a plane fMaxDistance away.
		* This function is usually used for finding other static mesh objects near a certain character. 
		* please note that: objects must be completely inside the near and far planes in order to pass the test. 
		* @param output: list to get the result
		* @param rect: the rect in screen space coordinates
		* @param fMaxDistance: the world unit distance inside which we will test possible candidates. if negative, it will get all visible ones.
		* @param pFnctFilter: a callback function to further filter selected object. if it is NULL, any scene object could be selected.
		* @return: return the number of objects in sphere.
		*/
		int GetObjectsByScreenRect(list<CBaseObject*>& output, const RECT& rect,OBJECT_FILTER_CALLBACK pFnctFilter=NULL, float fMaxDistance = -1.f);

		/** get the post processing mini scene graph. 
		*@return: this is same as GetMiniSceneGraph("_ps")*/
		CMiniSceneGraph* GetPostProcessingScene();

		/** set whether post processing is enabled. 
		* @param bEnable: bool
		* @param sCallbackScript: if bEnabled is true, this is a callback script to be called per frame at which one can process scene after the main 3d scene is rendered. 
		* if this is NULL, the old callback script is used. 
		*/
		void EnablePostProcessing(bool bEnable, const char* sCallbackScript);

		/** get whether post processing is enabled.*/
		bool IsPostProcessingEnabled();

		/** If the mini scene graph does not exist, it will be created */
		CMiniSceneGraph*  GetMiniSceneGraph(const string& name);
			
		/** Delete mini scene graphs by name.  if name is "" or "*", all mini scene graphs will be deleted.
		* @return: the number of graphs deleted is returned. 
		*/
		int DeleteMiniSceneGraph(const string& name);

		/** enable to render all mini scene graphs */
		void EnableMiniSceneGraph(bool bEnable);

		/** enable to render all mini scene graphs */
		bool IsMiniSceneGraphEnabled();

		/**get commar separated list of mini scene graph names, such as "name1,name2,"*/
		const std::string& GetAllMiniSceneGraphNames();

		/** 
		* delete a object from the scene. 
		* The object can be a manager loader, a global object or any ordinary scene object
		* It will first find the object in the scene, detach it and delete it. 
		* Please be very careful with this function. the object can no longer be used once it has been deleted 
		* @param pObject: the object to be deleted.
		* @return: true if the object has been found and deleted.
		*/
		bool DeleteObject(CBaseObject * pObject);

		/** set whether portal zone and portals are rendered. 
		*/
		void ShowPortalSystem(bool bEnable);

		/** get whether portal zone and portals are rendered. */
		bool IsPortalSystemShown();         

		/**
		* unload the resources used by the specified object. If it is a physics object, it will be removed from the physics world.
		*/
		void UnloadObjectResources(CBaseObject * pObject);

		/// -- For device and input handling --
		virtual HRESULT InitDeviceObjects();
		virtual HRESULT DeleteDeviceObjects();
		virtual HRESULT RestoreDeviceObjects();
		virtual HRESULT InvalidateDeviceObjects();
		virtual HRESULT RendererRecreated();
		virtual void Cleanup();

		/**
		* Checks if mouse point hits any physical geometry in the current scene. We will tranverse the last render queue to find a match instead of traverse the entire scene.
		* Here physical geometry may be the global terrain or any loaded physical objects. 
		* Both the hit object and the hit point will be returned

		* @params vPickRayOrig: the mouse ray's origin in the world view.
		* @params vPickRayDir: the mouse ray's direction in the world view.(must be normalized)
		* @params pPickedObject: [out] the scene object that collide with the mouse ray. 
		*		This may be NULL, if no object is found.
		* @params vIntersectPos: [out] this is the hit point in world coordinate system.
		*		if a scene object is picked, this will be the object's world position
		* @params vImpactNormal: Impact normal. If normal can not be retrieved, this value will be set to (0,0,0). So always compare with 0 before using the normal. 
		* @params bTestObject: if this is false, we will only test the global terrain and level mesh in the scene.
		*   ignoring all scene objects (pPickedObject is always set to NULL). if this is true, scene object
		*	is also tested. 
		* @params fMaxDistance: the longest distance from the ray origin to check for collision. If the value is 0 or negative, the view culling radius is used 
		*	as the fMaxDistance.
		* @params dwGroupMask: groups Mask used to filter physics objects, default to 0xFFFFffff
		* @return : return the distance from the interaction point to the ray origin. A negative value (-1) is returned if it hits nothing.
		*/
		float PickClosest(const Vector3& vPickRayOrig, const Vector3& vPickRayDir , CBaseObject** pTouchedObject, Vector3* vIntersectPos, Vector3* vImpactNormal = NULL, bool bTestObject=false, float fMaxDistance = 0, DWORD dwGroupMask=0xffffffff, bool bEnableGlobalTerrain = true);

		/** picking using a screen point instead of a ray */
		float PickClosest(int nScreenX, int nScreenY, CBaseObject** pPickedObject, Vector3* vIntersectPos, Vector3* vImpactNormal = NULL, bool bTestObject=false, float fMaxDistance=0, DWORD dwGroupMask=0xffffffff, bool bEnableGlobalTerrain = true);
		/**
		* Pick object using view clipping object. 
		* pick the smallest intersected object which is un-occluded by any objects. Object A is considered occluded by object B only if 
		* (1) both A and B intersect with the hit ray. 
		* (2) both A and B do not intersect with each other. 
		* (3) B is in front of A, with regard to the ray origin.
		* 
		* this function will ray-pick any loaded scene object(biped & mesh, but excluding the terrain) using their oriented bounding box. 
		* a filter function may be provided to further filter selected object. 
		* this function will transform all objects to near-camera coordinate system. 
		* This will remove some floating point inaccuracy near the camera position.Hence this function is most suitable for 
		* testing object near the camera eye position. This function does not rely on the physics engine to perform ray-picking. 
		* @see Pick(). 
		* @param ray: the ray in world space
		* @params pPickedObject: [out] the scene object that collide with the mouse ray. 
		*		This may be NULL, if no object is found.
		* @params fMaxDistance: the longest distance from the ray origin to check for collision. If the value is 0 or negative, the view culling radius is used 
		*	as the fMaxDistance.
		* @params dwGroupMask: groups Mask used to filter physics objects, default to 0xFFFFffff
		* @return :true if an object is picked. 
		*/
		bool PickObject(const CShapeRay& ray, CBaseObject** pTouchedObject, float fMaxDistance=0, OBJECT_FILTER_CALLBACK pFnctFilter=NULL);
		/** @see PickObject() above */
		bool PickObject(int nScreenX, int nScreenY, CBaseObject** pTouchedObject, float fMaxDistance=0, OBJECT_FILTER_CALLBACK pFnctFilter=NULL);

		/**
		* select select objects within a given region into a given group. 
		* @param nGroupIndex: which group to select to. One can get the result from CSelectionManager. In most cases, select to group 1; since group 0 is reserved for current selection. 
		* @param box: region in world position
		* @param pFnctFilter: a callback function to further filter selected object. if it is NULL, any scene object could be selected.
		* @return: the total number of selected objects is returned. 
		*/
		int SelectObject(int nGroupIndex, const CShapeOBB& box, OBJECT_FILTER_CALLBACK pFnctFilter=NULL);
		int SelectObject(int nGroupIndex, const CShapeSphere& circle, OBJECT_FILTER_CALLBACK pFnctFilter=NULL);

		/** @see same as PauseScene() */
		void PauseGame(){PauseScene(true);};
		/** @see same as EnableScene(false) */
		void DisableGame(){	EnableScene(false);};
		
		
		// -- For rendering and frame move --
		virtual void Animate( double dTimeDelta, int nRenderNumber=0);

		/** build the render list, and render the entire scene. 
		* @param dTimeDelta: fAnimation delta time in seconds.
		* @param nPipelineOrder: the current pipeline order, default to PIPELINE_3D_SCENE, which is anything before UI. 
		* specify over PIPELINE_POST_UI_3D_SCENE for anything after UI is drawn. 
		*/
		HRESULT AdvanceScene( double dTimeDelta, int nPipelineOrder = PIPELINE_3D_SCENE);

		void UpdateFogColor();
		/** get fog color based on current sun angle. */
		Vector3 GetFogColor(float fAngle);

		/** render objects in the post rendering list. This function can be called to render to texture render targets
		* if multiple objects are in selection, it will draw them in internally preferred order.
		* it will assume that the camera and world transform has already been set. It will just call the render method of respective scene objects.
		* @param dwSelection: any bit combination of RENDER_GROUP, default is render all scene object.
		* @return:  return the number of object rendered
		*/
		int RenderSelection(DWORD dwSelection=0xffffffff, double dTimeDelta=0);

		/** render characters */
		int RenderCharacters(SceneState& sceneState, SceneState::List_PostRenderObject_TrackRef_Type& listPRBiped);

		/** render the head on display, return the number of objects rendered
		* @param nPass: 0 means standard scene object headon display. 1 means overlay headon display. 
		*/
		int RenderHeadOnDisplay(int nPass = 0);

		/**
		* It will create the mirror surface if not exist.
		* @param nIndex index of surface. 
		* @return NULL is not able to create.
		*/
		CMirrorSurface* GetMirrorSurface(int nIndex);
		
		// -- Used mostly internally
		virtual HRESULT Draw(SceneState * sceneState);
		void RenderShadows();	/// render the shadow volume
		/// render the shadow map
		void RenderShadowMap();	

		/// render the full screen glow effect
		void RenderFullScreenGlowEffect();	

		//render screen wave effect
		void RenderScreenWaveEffect();

		void ApplyWaterFogParam();
		void RestoreSceneFogParam();

		/** set the current player */
		void SetCurrentPlayer(CBipedObject* pPlayer);

		/** get the current player */
		CBipedObject*			GetCurrentPlayer();
		/**
		* get the next scene object. 
		* @param pObj: the object whose next object is retrieved.
		* @return: return the next object. NULL is returned if the given character is not found.
		*/
		CBipedObject*			GetNextPlayer(CBipedObject* pObj);

		/** handle user input 
		* return true if there is a handler
		*/
		bool HandleUserInput();

		/** get script interface for reading and writing. */
		CSceneScripts&	GetScripts();

		/** Whether to generate mouse events for the scene objects. 
		* @param bEnable: true to enable, false to disable. 
		* @param dwEvents: what kind of mouse events are affected. Use 0xffff to affect all.*/
		void EnableMouseEvent(bool bEnable, DWORD dwEvents=0xffff);

		/** Get the current event binding object. this may return NULL. */
		const CEventBinding * GetEventBinding();

		/** load the default camera key and mouse event binding.*/
		void LoadDefaultEventBinding();

		/** get the actor that is being processed by the AI module or a script call back. The validity of the pointer is not guaranteed. */
		CBaseObject* GetCurrentActor();
		/** Set the actor that is being processed by the AI module or a script call back. The pointer can be NULL. */
		void SetCurrentActor(CBaseObject* pActor);

		/** call this function, when the ocean has changed or the environment has changed. This will cause 
		* the reflection map of the ocean surface to redraw regardless of whether the camera moves or not. */
		void UpdateOcean();

		/**
		* save all modified and unsaved local characters in the scene to the current NPC database. 
		* this function is usually called automatically when the terrain modification bit is set.  
		* @return the number of saved characters are returned.
		*/
		int SaveLocalCharacters();
		
		/**
		* save all local characters in the scene to the current NPC database regardless of whether they are modified or not
		* this function is usually called manually in some very rare cases. In most cases, call SaveLocalCharacters() instead.
		* @return the number of saved characters are returned.
		*/
		int SaveAllCharacters();

		/** save the given character to current database no matter it is modified or not. 
		* it will only if the character is persistent.
		@return : HRESULT */
		int SaveCharacterToDB(IGameObject* pObj);
		/**
		* remove a character from DB. 
		* @param pObj  object to remove
		* @return HRESULT
		*/
		int RemoveCharacterFromDB(IGameObject* pObj);

		/** get whether the scene can be automatically marked as modified */
		bool IsPersistent();

		/** set whether the scene can be automatically marked as modified */
		void SetPersistent(bool bPersistent);

		/** implementing IObjectDrag Interface */
		virtual void SetCandrag(bool bCandrag){m_bCandrag=bCandrag;}
		virtual bool GetCandrag()const{return m_bCandrag;}
		virtual void BeginDrag(int nEvent, int x, int y, int z){}
		virtual void EndDrag(int nEvent, int x, int y, int z){}
		virtual BOOL IsOnObject(int x,int y, int z);
		bool IsPickingObject(){return m_bPickFlag;};

		/**
		* It changes the current player to the next player in the scene. 
		* this function is mostly for testing and game purpose. 
		* if the object has a reference object, the reference object will become the current object. 
		* return : the new current player is returned. 
		*/
		CBaseObject* TogglePlayer();

		/**
		* load a character from the database. This function is usually called internally by DB_Load*() functions. 
		* @param npc the npc database item.
		* @param pOut : the structure will be filled with data in the database
		* @param bReload: if the character already exists in the scene and it is true, the character will be updated with the parameter in the database
		* @return true if succeed
		*/
		bool LoadNPCByNPCDbItem(CNpcDbItem& npc,CRpgCharacter** pOut, bool bReload = false);

		/**
		* load a character from the database
		* @param nNPCID character ID in the database
		* @param pOut : the structure will be filled with data in the database
		* @param bReload: if the character already exists in the scene and it is true, the character will be updated with the parameter in the database
		* @return true if succeed
		*/
		bool DB_LoadNPCByID(int nNPCID, CRpgCharacter** pOut, bool bReload = false);
		/**
		* load a character from the database
		* @param sName character name
		* @param pOut : the structure will be filled with data in the database
		* @param bReload: if the character already exists in the scene and it is true, the character will be updated with the parameter in the database
		* @return true if succeed
		*/
		bool DB_LoadNPCByName(const string& sName, CRpgCharacter** pOut, bool bReload = false);

		/**
		* return all NPC whose position is within a AABB region
		* @param pOut : the list will be filled with data in the database that meat the criterion.
		* @param vMin min point in world coordinate system, y component is ignored
		* @param vMax max point in world coordinate system, y component is ignored 
		* @param bReload: if the character already exists in the scene and it is true, the character will be updated with the parameter in the database
		* @return the number of NPC is returned.
		*/
		int DB_LoadNPCsByRegion(const Vector3& vMin, const Vector3& vMax, bool bReload = false);
		
		/** full screen glow effect is parameterized by the blending weight of the source image and the glow image. 
		* the blending weights are given by 
		* - source image weight: m_fFullscreenGlowIntensity*(1-m_fFullscreenGlowBlendingFactor)
		* - source glow image weight: m_fFullscreenGlowIntensity*m_fFullscreenGlowBlendingFactor 
		* @param fGlowIntensity : default value is 2
		*/
		void SetFullscreenGlowIntensity(float fGlowIntensity);
		float GetFullscreenGlowIntensity();

		/** full screen glow effect is parameterized by the blending weight of the source image and the glow image. 
		* the blending weights are given by 
		* - source image weight: m_fFullscreenGlowIntensity*(1-m_fFullscreenGlowBlendingFactor)
		* - source glow image weight: m_fFullscreenGlowIntensity*m_fFullscreenGlowBlendingFactor 
		* @param fGlowFactor : default value is 0.5
		*/
		void SetFullscreenGlowBlendingFactor(float fGlowFactor);
		float GetFullscreenGlowBlendingFactor();

		/**the current player will automatically generate ripples near the water surface*/
		bool IsAutoPlayerRippleEnabled(){return m_bAutoPlayerRipple;};
		/**the current player will automatically generate ripples near the water surface*/
		void EnableAutoPlayerRipple(bool bEnable){m_bAutoPlayerRipple = bEnable;};

		/** enable portal and zone rendering. so that we can see the effect before and after portal rendering.*/
		bool IsPortalZoneEnabled(){return m_bEnablePortalZone;};

		/** enable portal and zone rendering. so that we can see the effect before and after portal rendering.*/
		void EnablePortalZone(bool bEnable){m_bEnablePortalZone = bEnable;};
		
		/** whether to use wire frame */
		bool IsUseWireFrame(){return m_bUseWireFrame;};

		/** set whether to use wire frame */
		void SetUseWireFrame(bool UseWireFrame );
		

		/** get the list of sentient objects. */
		inline list_IObjectWeakPtr_Type&	GetSentientObjects(){ return m_sentientGameObjects; };

		/** return a list of missile objects. Missile objects will always be tested 
		* for rendered on a per-frame basis.*/
		inline MissileObjectPool_Type& GetMissiles(){return m_missiles;};

		/** set the environment simulator to be used with this world. */
		void SetEnvironmentSim(IEnvironmentSim* pSim);

		/** get the scene state information. The scene state contains information about the current simulation
		* and render states.Such as which scene object is being animated or rendered, etc*/
		SceneState* GetSceneState(){return m_sceneState.get();};
		inline CPhysicsWorld* GetPhysicsWorld() {return m_pPhysicsWorld.get();} ;
		inline IEnvironmentSim* GetEnvSim() {return m_pEnvironmentSim;};

		/** deprecated: use GetBatchedElementDrawer() */
		IParaDebugDraw* GetDebugDrawer();
		
		IBatchedElementDraw* GetBatchedElementDrawer();

		/** set the physics debug draw mode. it is a bitwise field of PhysicsDebugDrawModes 
		* @param nMode: 0 to disable it. -1(0xffff) to enable full debug mode drawing for current physics. */
		void SetPhysicsDebugDrawMode(int nMode);

		/** set the physics debug draw mode. it is a bitwise field of PhysicsDebugDrawModes 
		* @param nMode: 0 to disable it. -1(0xffff) to enable full debug mode drawing for current physics. */
		int GetPhysicsDebugDrawMode();

		/** the mouse ray picking distance for onclick event. usually 30-100 meters*/
		void SetOnClickDistance(float fDist){ m_fOnClickDistance = fDist;}
		float GetOnClickDistance(){ return m_fOnClickDistance;}

		/** get a zone node by its name. it will create an empty one if it does not exist */
		CZoneNode * CreateGetZoneNode(const char* sName);
		/** get a zone node by its name. it will return NULL if it does not exist */
		CZoneNode * GetZoneNode(const char* sName);
		/** get a portal node by its name. it will create an empty one if it does not exist */
		CPortalNode * CreateGetPortalNode(const char* sName);
		/** get a portal node by its name. it will return NULL if it does not exist */
		CPortalNode * GetPortalNode(const char* sName);

		/** get the distance from the eye, within which shadow map will be used. This should match the value in shadow map shader.
		*/
		virtual float GetShadowRadius() { return m_fShadowRadius; }
		void SetShadowRadius(float fShadowRadius);

		/** whether to block all user input (both key and mouse). Input is usually blocked as a game logics, such as we do not want the user to control the player anymore. */
		bool IsBlockInput(){return m_bBlockInput;}
		void SetBlockInput(bool bBlockInput){m_bBlockInput = bBlockInput;}

		/** set groups Mask used to filter physics objects, default to 0xffffffff*/
		virtual void SetPhysicsGroupMask(DWORD dwValue); 

		/** get groups Mask used to filter physics objects, default to 0xffffffff*/
		virtual DWORD GetPhysicsGroupMask(); 

		bool IsDropShadow(){return m_renderDropShadow;}
		void SetDropShadow(bool useDropShadow){m_renderDropShadow = useDropShadow;}

		/** When a child object wants to remove itself, it is usually not safe to do so immediately, instead
		* the object add itself to dead object pool for the root scene to safely remove it at the end of the frame. */
		void AddToDeadObjectPool(CBaseObject* pObject);
		void RemoveDeadObjects();

		/** this function is called at the beginning of each frame to build the scene state from the specified camera
		* Objects which are potentially visible are put to predefined queues of the scene states. */
		virtual int PrepareRender(CBaseCamera* pCamera, SceneState* pSceneState);

	private:

		/** whether to block all user input (both key and mouse). Input is usually blocked as a game logics, such as we do not want the user to control the player anymore. */
		bool m_bBlockInput;

		/** whether post processing is enabled.*/
		bool m_bEnablePostProcessing;
		/** the script to be executed per frame when doing the post processing. One can control the post processing
		* via the GetPostProcessingScene() function */
		string m_sPostPorcessorCallbackScript;

		/** whether to render object's head on display text */
		bool m_bShowHeadOnDisplay;

		/** even if m_bShowHeadOnDisplay is shown, */
		float m_fMaxHeadOnDisplayDistance;

		/** enable to render all mini scene graphs */
		bool m_bEnableMiniScenegraph;

		/** the current player will automatically generate ripples near the water surface*/
		bool m_bAutoPlayerRipple;
		/**
		* the current object level method used. The default value is 
		* CENTER_ON_CAMERA|VIEW_RADIUS_AUTO
		*/
		DWORD m_dwObjCullingMethod;
		/**
		* we will allow an object having this number of vertical pixels on screen to be shown.
		*/
		float	m_fCullingPixelsHeight;
		/** 
		* all objects must be drawn when they are within this radius. Default value is 15 meters
		*/
		float	m_fMinPopUpDistance;
		
		/** the managed loader list for scene object loading and unloading. */
		map <string, CManagedLoader*> m_managedloader;

		bool					m_bRenderFog;
		/// original fog color, it will be multiplied by m_fFogDensity to attain the real fog color.
		DWORD					m_dwFogColor;
		LinearColor				m_FogColorFactor;
		FLOAT					m_fFogStart;
		FLOAT					m_fFogEnd;
		FLOAT					m_fFogDensity;
		
		/** the distance from the eye, within which shadow map will be used. */
		float m_fShadowRadius;
		
		/// used when rendering
		DWORD					m_dwBackGroundColor;
		bool					m_bSnapToTerrain; /// if true, all objects will be rendered with the terrain height added to it, including the camera. default value is true
		bool					m_bShowBoundingBox;
		bool					m_bGenerateReport;

		/// whether to use occlusion query algorithm for rendering. 
		bool					m_bEnableOcclusionQuery;

		/// game paused
		bool					m_bGamePaused;		
		/// 3D scene is enabled or not. a disabled scene is not visible no matter what
		bool					m_bGameEnabled;		
		
		/// shadow support
		ShadowVolumePool_Type	m_ShadowVolumes;
		bool					m_bRenderMeshShadow;
		
		/// available cameras, the first one in the list is current
		CameraPool_type		m_cameras;
		ref_ptr<CBaseCamera> m_pCurrentCamera;
		SkyMeshPool_type		m_skymeshes;
		MiniSceneGraphPool_type		m_miniSceneGraphs;
		
		/// whether to render the sky box/dome/plane 
		bool					m_bRenderSky;

		/** whether scene is modified*/
		bool m_bModified;

		/** event binding */
		CGUIEvent *m_event;

		/** render origin */
		Vector3 m_vRenderOrigin;

		/** The scene state contains information about the current simulation
		* and render states.Such as which scene object is being animated or rendered, etc*/
		ref_ptr<SceneState>			    m_sceneState;

		/** the maximum number of shadow casters. */
		int						m_nMaxNumShadowCasters;
		int						m_nMaxNumShadowReceivers;

		/** max number of character triangles allowed. we render characters from front to back. and if the previously drawn character count exceed this value
		* we will stop rendering the rest of the characters. this value defaults to 50000*/
		int						m_nMaxCharTriangles;


		/** full screen glow effect is parameterized by the blending weight of the source image and the glow image. 
		* the blending weights are given by m_fFullscreenGlowIntensity*(1-m_fFullscreenGlowBlendingFactor) and m_fFullscreenGlowIntensity*m_fFullscreenGlowBlendingFactor */
		float m_fFullscreenGlowIntensity;
		float m_fFullscreenGlowBlendingFactor;

		/** dynamic global terrain object.*/
		ref_ptr<ParaTerrain::CGlobalTerrain>	m_globalTerrain;
		
		/** all queued scripts to be processed in the next frame.*/
		CSceneScripts			m_scripts;
		/// game device is loaded
		bool					m_bInitialized;

		/** a bit wise field which tells what kind of mouse events will be generated for the scene objects. 
		* currently it supports only 0 and 0xffff, the latter means enable all. */
		DWORD m_dwEnableMouseEvent;
		/** the current player */
		CBipedObject*			m_currentplayer;

		/** whether light is enabled in the scene rendering.*/
		bool					m_bEnableLighting;
		/** whether sun light is enabled. */
		bool					m_bEnableSunLight;

		/** the global sunlight parameters.*/
		ref_ptr<CSunLight> m_pSunLight;

		CBaseObject* m_pickObj;
		/** the actor that is being processed by the AI module or a script call back. The validity of the pointer is not guaranteed. */
		CBaseObject* m_pActor;

		/** show the arrow mesh associated with the light object*/
		bool m_bShowLocalLightMesh;

		bool m_bPickFlag;


		/** enable portal and zone rendering. so that we can see the effect before and after portal rendering.*/
		bool m_bEnablePortalZone;

		/** whether to rendering portal system.*/
		bool m_bShowPortalSystem;

		/** render with wire frame.default to false */
		bool m_bUseWireFrame;


		/// some coefficients during the auto radius calculation.
		float m_fCoefF, m_fCoefN, m_fCoefK;

		/** decide whether pObj is visible by the pCamera, if so, it will add it to the proper render queue in sceneState. 
		* @note: this function is only used by RebuildSceneState
		* @note: This function may be recursive if child node of pObj needs processing. 
		* @return: if false, we should break from further processing.
		*/
		virtual bool PrepareRenderObject(CBaseObject* pObj, CBaseCamera* pCamera, SceneState& sceneState);
		
		void PrepareRenderAllChildren(CBaseCamera* pCamera, SceneState& sceneState);

		void PrepareCameraState(CBaseCamera* pCamera, SceneState &sceneState);

		/** portal rendering based render queue culling, if we have zones in the render queue */
		void PreparePortalsAndZones(CBaseCamera* pCamera, SceneState &sceneState);

		/** for global missile objects, select potentially visible ones to list. */
		void PrepareMissileObjects(CBaseCamera* pCamera, SceneState &sceneState);

		/** transverse the scene's quad-tree hierarchy and build potentially visible objects in to corresponding lists
		*/
		void PrepareTileObjects(CBaseCamera* pCamera, SceneState &sceneState);

		void UpdateMovableObjectZone(SceneState &sceneState, SceneState::List_PostRenderObject_Type& listPRBiped);

		/** auto generate player ripple for the current player. */
		void AutoGenPlayerRipple(float fTimeDelta);

		/** all attribute models are in second column of child attribute object. 
		* exposing everything in the scene as abstract data models and can be accessed by scripting interface, etc. 
		*/
		void CreateAttributeModels();

#ifdef USE_DIRECTX_RENDERER
		std::unique_ptr<DropShadowRenderer> m_dropShadowRenderer;
#endif
		bool m_renderDropShadow;

		LinearColor m_defaultAmbColor;
		LinearColor m_defaultFogColor;
		float m_defaultFogStart;
		float m_defaultFogEnd;


		//
		ref_ptr<BlockWorldClient> m_pBlockWorldClient;
		
	protected:
		IEnvironmentSim*		m_pEnvironmentSim;		/// current physics engine and simulation engine
		ref_ptr<CPhysicsWorld>			m_pPhysicsWorld;		/// physics world
		/** for drawing line based debug object. */
		CBatchedElementDraw*			m_pBatchedElementDraw;

		/** it keeps a reference to all active (sentient) game objects in the scene.*/
		list_IObjectWeakPtr_Type		m_sentientGameObjects;
		/** a list of missile objects. Missile objects will always be tested 
		* for rendered on a per-frame basis. */
		MissileObjectPool_Type	m_missiles;
#ifdef PARAENGINE_CLIENT
		/** managed mirror surfaces. */
		MirrorSufacePool_Type m_mirrorSurfaces;
#endif
		/** store all zones in a map from zone name to its pointer */
		map <string, CZoneNode*> m_zones;
		/** store all zones in a map from zone name to its pointer */
		map <string, CPortalNode*> m_portals;

		/** a quad tree object that hold most static (regional mobile) objects according to its spatial parameters*/
		ref_ptr<CTerrainTileRoot>	m_pTileRoot;
		/** dead objects pool. */
		unordered_ref_array<CBaseObject*>  m_dead_objects;

		/// console string
		/// the console panel string
		char					m_sConsoleString[200];	
		
		/** the mouse ray picking distance for onclick event. usually 30-100 meters*/
		float m_fOnClickDistance;

		/** whether to use instancing during scene rendering. it is disabled by default. */
		bool m_bUseInstancing;

		/** whether the scene can be automatically marked as modified */
		bool m_bIsPersistent;

		/** whether to show the main player */
		bool m_bShowMainPlayer;

		/** whether current camera allows showing the main player */
		bool m_bCanShowMainPlayer;

		/** m_MaxRenderCount[RenderImportance] is the max number of objects to be rendered of a given render importance */
		vector<int> m_MaxRenderCount;

		/** comma separated list of mini scene graphs */
		std::string m_sMiniSceneGraphNames;

		/** the cursor to be displayed when mouse over it. If this is empty, the parent cursor is used. */
		std::string m_sCursorFile;
		/** default to -1, which is using current hardware value.  */
		int16 m_nCursorHotSpotX;
		/** default to -1, which is using current hardware value. */
		int16 m_nCursorHotSpotY;

		/** groups Mask used to filter physics objects, default to 0xffffffff*/
		DWORD m_dwPhysicsGroupMask;

		/** this is always 100.f*/
		float m_fPostRenderQueueOrder;
		/** attribute models in the second column*/
		unordered_ref_array<IAttributeFields*> m_attribute_models;
		friend class CParaEngineApp;
	};

}
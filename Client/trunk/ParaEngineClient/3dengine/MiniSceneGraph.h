#pragma once
#include "RenderTarget.h"
#include "SceneState.h"
#include "IScene.h"
#include "SunLight.h"
#include <map>

namespace ParaEngine
{
	// forward declare
	class CCanvasCamera;
	class CBaseCamera;
	struct TextureEntity;
	class CShapeRay;
	class IBatchedElementDraw;
	class CBatchedElementDraw;

	/** call back function type for ray picking. return true if the obj is accepted, false otherwise. */
	typedef bool (*OBJECT_FILTER_CALLBACK)(CBaseObject* obj);

	/**
	* Mini scene graph does not use a spatial partitions for its stored objects, instead it just implement a simple parent/child relationships. Most scene objects can be attached to mini scene graph. We can add and delete, show or hide objects just like the 2D objects. Contents on mini scene graphs are rendered after the main scene is rendered. 
	* Mini scene graph is so designed that it can be used to display interactive 3D helper objects in the scene. In most cases, we will want to keep only a small number of objects on a visible mini scene graph node. For example, some rotation handles or attachment points on mesh or helper 3D handles can be created and displayed via mini scene graph. Objects on mini scene graph are not persistent, so they are usually used to display temporary 3D objects. 
	* Following is a list of possible usage of mini scene graph
	*  -	Simple snow or rain effects, where we create new particle system around the current camera position every few seconds, while deleting all particle systems that are created a few seconds ago. This can be done either in the scripting interface or in the game engine, so long as a timer is used to update the particles.
	*  -	Displaying mount point or attachment point on mesh objects. For example, when a player approaches a mount point, we may display some 3D icon to inform the user that it can interact with it. This is usually done via the scripting interface. 
	*  -	Displaying a special 3D cursor, such as a 3D object during mouse move or click. For instance, we can display the situation of dragging a 3D object across the scene. 
	*/
	class CMiniSceneGraph : public CRenderTarget, public IScene
	{
	public:
		typedef ParaEngine::weak_ptr<IObject, CMiniSceneGraph> WeakPtr_type;
		CMiniSceneGraph(void);
		virtual ~CMiniSceneGraph(void);
		virtual CBaseObject::_SceneObjectType GetType(){return CBaseObject::MiniSceneGraph;};
		
		//////////////////////////////////////////////////////////////////////////
		// implementation of IAttributeFields
		/** attribute class ID should be identical, unless one knows how overriding rules work.*/
		virtual int GetAttributeClassID(){return ATTRIBUTE_CLASSID_MiniSceneGraph;}
		/** a static string, describing the attribute class object's name */
		virtual const char* GetAttributeClassName(){static const char name[] = "CMiniSceneGraph"; return name;}
		/** a static string, describing the attribute class object */
		virtual const char* GetAttributeClassDescription(){static const char desc[] = ""; return desc;}
		/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);

		ATTRIBUTE_METHOD1(CMiniSceneGraph, IsLightEnabled_s, bool*)	{*p1 = cls->IsLightEnabled(); return S_OK;}
		ATTRIBUTE_METHOD1(CMiniSceneGraph, EnableLight_s, bool)	{cls->EnableLight(p1); return S_OK;}

		ATTRIBUTE_METHOD1(CMiniSceneGraph, GetBackGroundColor_s, Vector3*)		{ *p1 = cls->GetBackGroundColor().ToVector3(); return S_OK; }
		ATTRIBUTE_METHOD1(CMiniSceneGraph, SetBackGroundColor_s, Vector3)		{ LinearColor c(p1.x, p1.y, p1.z, 1); cls->SetBackGroundColor(c); return S_OK; }

		ATTRIBUTE_METHOD1(CMiniSceneGraph, IsSunLightEnabled_s, bool*)	{*p1 = cls->IsSunLightEnabled(); return S_OK;}
		ATTRIBUTE_METHOD1(CMiniSceneGraph, EnableSunLight_s, bool)	{cls->EnableSunLight(p1); return S_OK;}

		ATTRIBUTE_METHOD1(CMiniSceneGraph, IsFogEnabled_s, bool*)	{*p1 = cls->IsFogEnabled(); return S_OK;}
		ATTRIBUTE_METHOD1(CMiniSceneGraph, EnableFog_s, bool)	{cls->EnableFog(p1); return S_OK;}

		ATTRIBUTE_METHOD1(CMiniSceneGraph, GetFogColor_s, Vector3*)		{*p1 = cls->GetFogColorFactor().ToVector3(); return S_OK;}
		ATTRIBUTE_METHOD1(CMiniSceneGraph, SetFogColor_s, Vector3)		{LinearColor c(p1.x, p1.y,p1.z,1); cls->SetFogColorFactor(c); return S_OK;}

		ATTRIBUTE_METHOD1(CMiniSceneGraph, GetFogStart_s, float*)	{*p1 = cls->GetFogStart(); return S_OK;}
		ATTRIBUTE_METHOD1(CMiniSceneGraph, SetFogStart_s, float)	{cls->SetFogStart(p1); return S_OK;}

		ATTRIBUTE_METHOD1(CMiniSceneGraph, GetFogEnd_s, float*)	{*p1 = cls->GetFogEnd(); return S_OK;}
		ATTRIBUTE_METHOD1(CMiniSceneGraph, SetFogEnd_s, float)	{cls->SetFogEnd(p1); return S_OK;}

		ATTRIBUTE_METHOD1(CMiniSceneGraph, GetFogDensity_s, float*)	{*p1 = cls->GetFogDensity(); return S_OK;}
		ATTRIBUTE_METHOD1(CMiniSceneGraph, SetFogDensity_s, float)	{cls->SetFogDensity(p1); return S_OK;}

		ATTRIBUTE_METHOD1(CMiniSceneGraph, IsRenderSky_s, bool*)	{*p1 = cls->IsRenderSky(); return S_OK;}
		ATTRIBUTE_METHOD1(CMiniSceneGraph, EnableSkyRendering_s, bool)	{cls->EnableSkyRendering(p1); return S_OK;}

		ATTRIBUTE_METHOD1(CMiniSceneGraph, IsHeadOnDisplayShown_s, bool*)	{*p1 = cls->IsHeadOnDisplayShown(); return S_OK;}
		ATTRIBUTE_METHOD1(CMiniSceneGraph, ShowHeadOnDisplay_s, bool)	{cls->ShowHeadOnDisplay(p1); return S_OK;}

		ATTRIBUTE_METHOD1(CMiniSceneGraph, IsUsePointTexture_s, bool*)	{ *p1 = cls->IsUsePointTexture(); return S_OK; }
		ATTRIBUTE_METHOD1(CMiniSceneGraph, SetUsePointTexture_s, bool)	{ cls->SetUsePointTexture(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CMiniSceneGraph, GetMaxHeadOnDisplayDistance_s, float*)	{*p1 = cls->GetMaxHeadOnDisplayDistance(); return S_OK;}
		ATTRIBUTE_METHOD1(CMiniSceneGraph, SetMaxHeadOnDisplayDistance_s, float)	{cls->SetMaxHeadOnDisplayDistance(p1); return S_OK;}

		ATTRIBUTE_METHOD1(CMiniSceneGraph, GetRenderPipelineOrder_s, int*)	{*p1 = cls->GetRenderPipelineOrder(); return S_OK;}
		ATTRIBUTE_METHOD1(CMiniSceneGraph, SetRenderPipelineOrder_s, int)	{cls->SetRenderPipelineOrder(p1); return S_OK;}
		
			
		/** whether rendering the sky */
		bool IsRenderSky(){ return m_bRenderSky; }
		void EnableSkyRendering(bool bEnable){m_bRenderSky = bEnable;};

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

		/** get light enable state */
		bool IsLightEnabled();
		/** whether use light */
		void EnableLight(bool bEnable);

		/** get light enable state */
		bool IsSunLightEnabled();
		/** whether use light */
		void EnableSunLight(bool bEnable);

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

		/** get sun light object*/
		CSunLight& GetSunLight();
	public:
		/**
		* set the actor: The camera always focuses on actor, so 
		* this actor can be used to control the current camera position.
		* @param pActor: it must be a valid object. 
		*/
		void SetActor(CBaseObject * pActor);

		/**get the current actor */
		CBaseObject * GetActor();

		/** clean up all resource objects */
		virtual void Cleanup();

		/** get the camera object. create if the camera does not exist.*/
		CCanvasCamera* GetCamera();

		/** get the current camera*/
		CBaseCamera*	GetCurrentCamera() {return (CBaseCamera*)GetCamera();};

		/**
		* Zoom the camera
		* @param fAmount 
		*/
		void CameraZoom(float fAmount);

		/**
		* reset the camera parameters to view the entire sphere at best (default) distance
		* @param sphere 
		*/
		void CameraZoomSphere(const CShapeSphere& sphere);

		/**
		* set the camera look at position
		*/
		void CameraSetLookAtPos(float x, float y, float z);
		void CameraGetLookAtPos(float* x, float* y, float* z);

		/**
		* set the camera eye position
		* @param fRotY rotation of the camera around the Y axis, in the world coordinate. 
		* @param fLiftupAngle lift up angle of the camera.
		* @param fCameraObjectDist the distance from the camera eye to the object being followed.
		*/
		void CameraSetEyePosByAngle(float fRotY, float fLiftupAngle, float fCameraObjectDist);
		void CameraGetEyePosByAngle(float* fRotY, float* fLiftupAngle, float* fCameraObjectDist);
		void CameraSetEyePos(double x, double y, double z);
		void CameraGetEyePos(double* x, double* y, double* z);

		/**
		* rotate the camera round the object on canvas
		* @param dx 
		* @param dy relative amount in radian. 
		* @param dz 
		*/
		void CameraRotate(float dx, float dy, float dz);

		/**
		* pan the camera
		* @param dx relative amount in pixels
		* @param dy relative amount in pixels
		*/
		void CameraPan(float dx, float dy);

		/** this function is called at the beginning of each frame to build the scene state from the specified camera
		* Objects which are potentially visible are put to predefined queues of the scene states. */
		int PrepareRender(CBaseCamera* pCamera, SceneState* pSceneState);

		/** decide whether pObj is visible by the pCamera, if so, it will add it to the proper render queue in sceneState.
		* @note: this function is only used by RebuildSceneState
		* @note: This function may be recursive if child node of pObj needs processing.
		* @return: if false, we should break from further processing.
		*/
		virtual bool PrepareRenderObject(CBaseObject* pObj, CBaseCamera* pCamera, SceneState& sceneState);

		/** render objects in the post rendering list. This function can be called to render to texture render targets
		* if multiple objects are in selection, it will draw them in internally preferred order.
		* it will assume that the camera and world transform has already been set. It will just call the render method of respective scene objects.
		* @param dwSelection: any bit combination of RENDER_GROUP, default is render all scene object.
		* @return:  return the number of object rendered
		*/
		int RenderSelection(DWORD dwSelection=0xffffffff, double dTimeDelta=0);

		/** render the head on display, return the number of objects rendered*/
		int RenderHeadOnDisplay();

		/** draw the content of the scene graph to the current render target. If EnableActiveRendering is enabled, this function will be called each render frame after the main scene graph. 
		* however, if EnableActiveRendering is disabled, one can call this function to render a single frame to the render target on demand. 
		*/
		void Draw(float fDeltaTime);

		/** this is an optional 2D mask, which is drawn over the entire canvas after scene is rendered in to it. 
		* @param pTexture 
		*/
		void SetMaskTexture(TextureEntity* pTexture);


		/** get name */
		const char* GetName();

		/** set the object name. this function can be used to rename this object */
		void SetName(const char* sName);

		/** enable or disable a given camera */
		void EnableCamera(bool bEnable);

		/** whether camera is enabled. A dedicated render target for this miniscenegraph will be automatically created when the scene graph claims to enable its own camera. 
		* the render target size should be set before calling this function at SetRenderTargetSize().
		* it is disabled by default. it is much faster to disable camera, because it will use the main scene's render pipeline and effects. 
		* Otherwise it will be rendered after the main scene is rendered, since the camera is different.
		* TODO: currently mini scene graph is only rendered when its camera is disabled. local camera is not supported at the moment
		*/
		bool IsCameraEnabled();
		
		/** even when active rendering is on, the mini scene graph will not update itself, if need update is false. Need update will be set to true whenever GetTexture is called. 
		* and that whenever draw method is called. Need update will become false. */
		bool IsNeedUpdate();

		/** get object by name, if there are multiple objects with the same name, the last added one is inserted. 
		* @note: This function will traverse the scene to search the object. So there might be some performance penalty. 
		* @param name: 
		* @param bRecursive: true to search recursively(may be slow). default to false. false only search the first level child from a fast hash map. 
		*/
		CBaseObject* GetObject(const char* name, bool bRecursive = false);

		/** get the first object that matches the position. fEpsilon is 0.01f by default*/
		CBaseObject* GetObject(float x, float y, float z, float fEpsilon = 0.01f);

		/** remove an object from this scene graph but do not destroy it. This function will search the scene recursively
		* this function can be used to move a node from one scene graph to another */
		int RemoveObject(const char* name, bool bRecursive = false);


		/** remove an object from this scene graph but do not destroy it.This function will search the scene recursively
		* this function can be used to move a node from one scene graph to another */
		int RemoveObject(CBaseObject* pObj);

		/** destroy all objects with the given name. This function will search the scene recursively
		* the current version will only destroy the first met child with the given name. 
		* @return the number of objects deleted. 
		*/
		int DestroyObject(const char* name, bool bRecursive = false);

		/** delete an object from this scene graph but do not destroy it.This function will search the scene recursively
		* this function can be used to move a node from one scene graph to another 
		* Note: this is like calling RemoveObject(pObj) and then delete the object. 
		* @param pObj: 
		*/
		int DestroyObject(CBaseObject* pObj);

		
		/** clear the entire scene graph */
		void Reset();

		virtual void AddChild(CBaseObject * pObject);

		/** get the scene state information. The scene state contains information about the current simulation
		* and render states.Such as which scene object is being animated or rendered, etc*/
		SceneState* GetSceneState(){return &m_sceneState;};

		/** */
		bool PickObject(const CShapeRay& ray, CBaseObject** pTouchedObject, float fMaxDistance=0, OBJECT_FILTER_CALLBACK pFnctFilter=NULL);
		bool PickObject(int nScreenX, int nScreenY, CBaseObject** pTouchedObject, float fMaxDistance=0, OBJECT_FILTER_CALLBACK pFnctFilter=NULL);

		/** show or hide all scene's objects' head on display*/
		void ShowHeadOnDisplay(bool bShow){m_bShowHeadOnDisplay = bShow;};

		/** whether all scene's objects' head on display*/
		bool IsHeadOnDisplayShown(){return m_bShowHeadOnDisplay;};

		/** show or hide all scene's objects' head on display*/
		void SetMaxHeadOnDisplayDistance(float fDist){m_fMaxHeadOnDisplayDistance = fDist;};

		/** whether all scene's objects' head on display*/
		float GetMaxHeadOnDisplayDistance(){return m_fMaxHeadOnDisplayDistance;};

		bool IsUsePointTexture() const;
		void SetUsePointTexture(bool val);
	
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
		

		/** get debug draw */
		IBatchedElementDraw* GetBatchedElementDrawer();;

		////////////////////////////////////////////////////////////////////////
		//
		// following functions are just for post processing effects that are called per render frame move. 
		//
		////////////////////////////////////////////////////////////////////////


		/** set the current effect */
		bool SetEffect(CEffectFile* pEffect);

		/** default to PIPELINE_3D_SCENE. please see RENDER_PIPELINE_ORDER enum type 
		- if 0,  it will be rendered before main scene 
		- if 1,  it will be rendered during the main scene 
		- if 51,  it will be rendered after all UI objects. 
		*/
		void SetRenderPipelineOrder(int nOrder);

		/** default to PIPELINE_3D_SCENE. please see RENDER_PIPELINE_ORDER enum type
		- if 0,  it will be rendered before main scene 
		- if 1,  it will be rendered during the main scene 
		- if 51,  it will be rendered after all UI objects. 
		*/
		int GetRenderPipelineOrder();

		void SetBackGroundColor(const LinearColor& bgColor);

		LinearColor GetBackGroundColor();
	private:
		typedef std::map<std::string, CBaseObject*> ObjectNameMap_Type;
		ObjectNameMap_Type m_name_map;

		/** camera */
		CCanvasCamera* m_pCamera;

		/** if true, it will use the m_pCamera. otherwise it will use the main scene's camera by default.*/
		bool m_bUseLocalCamera;

		/** whether to use point filtering for all objects rendered. */
		bool m_bUsePointTexture;

		bool m_bEnabled;

		/** this is an optional 2D mask, which is drawn over the entire canvas after scene is rendered in to it. */
		asset_ptr<TextureEntity> m_pMask;

		/// the actor on stage
		CBaseObject *	m_pActor;

		/** The scene state contains information about the current simulation
		* and render states.Such as which scene object is being animated or rendered, etc*/
		SceneState			    m_sceneState;

		Vector3 m_vRenderOrigin;

		/** whether to render object's head on display text */
		bool m_bShowHeadOnDisplay;

		/** even if m_bShowHeadOnDisplay is shown, */
		float m_fMaxHeadOnDisplayDistance;

		/// available cameras, the first one in the list is current
		unordered_ref_array<CSkyMesh*>		m_skymeshes;
		/// whether to render the sky box/dome/plane 
		bool					m_bRenderSky;

		// render fog
		bool					m_bRenderFog;
		/// original fog color, it will be multiplied by m_fFogDensity to attain the real fog color.
		DWORD					m_dwFogColor;
		LinearColor				m_FogColorFactor;
		FLOAT					m_fFogStart;
		FLOAT					m_fFogEnd;
		FLOAT					m_fFogDensity;

		/** whether sun light is enabled. */
		bool					m_bEnableLighting;

		/** whether sun light is enabled. */
		bool					m_bEnableSunLight;

		/** the global sunlight parameters.*/
		CSunLight m_SunLight;

		/** default to 0 
		- if 0,  it will be rendered before main scene 
		- if 1,  it will be rendered during the main scene 
		- if 51,  it will be rendered after all UI objects. 
		*/
		int		m_nRenderPipelineOrder;

		/** for drawing line based debug object. */
		CBatchedElementDraw*			m_pBatchedElementDraw;
	private:
		void Draw_Internal(float fDeltaTime);

	};

}

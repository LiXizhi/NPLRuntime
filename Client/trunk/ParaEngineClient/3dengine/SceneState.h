#pragma once

#include "util/unordered_array.hpp"
#include "SortedFaceGroups.h"
#include "util/ParaMemPool.h"
#include "IRefObject.h"
#include "ParaXModel/ParticleSystemRef.h"
#include "BillBoardViewInfo.h"
#include "BaseObject.h"

using namespace std;

namespace ParaEngine
{
	class CBaseObject;
	class IBatchedElementDraw;
	class CParaWorldAsset;
	class CContainerObject;
	class CSpriteObject;
	class CBipedObject;
	struct GUIState;
	class ParticleSystem;
	class CEffectFile;
	class CSortedFaceGroups;
	class IScene;
	class CLightObject;

	struct SceneState;

	enum RENDER_GROUP{
		RENDER_SKY_BOX = 1,
		RENDER_GLOBAL_TERRAIN = 0x1 << 1,
		RENDER_MESH_FRONT_TO_BACK = 0x1 << 2,
		RENDER_MESH_BACK_TO_FRONT = 0x1 << 3,
		RENDER_MESH_TRANSPARENT = 0x1 << 4,
		RENDER_CHARACTERS = 0x1 << 5,
		RENDER_PLAYER = 0x1 << 6,
		RENDER_SPRITES = 0x1 << 7,
		RENDER_MISSILES = 0x1 << 8,
		RENDER_PARTICLES = 0x1 << 9,
		RENDER_BOUNDINGBOX = 0x1 << 10,
		RENDER_GEN_SM_TERRAIN = 0x1 << 11, // generate shadow map for global terrain object.
		RENDER_TRANSLUCENT_FACE_GROUPS = 0x1 << 12,
		RENDER_PORTAL_SYSTEM = 0x1 << 13,
		RENDER_SELECTION = 0x1 << 14,
		RENDER_SHADOWMAP = 0x1 << 15,
		RENDER_TRANSPARENT_CHARACTERS = 0x1 << 16,
		RENDER_OWNER_DRAW = 0x1 << 17,
	};

#define CHECK_SELECTION(x) ((dwSelection&(x))>0)

	/** general render pipeline order. The order may be changed, however, this is the general order.  */
	enum RENDER_PIPELINE_ORDER{
		PIPELINE_ORDER_PRE_SCENE = 0,
		PIPELINE_3D_SCENE = 1,
		// TODO: add more here. 
		PIPELINE_UI = 50,
		PIPELINE_POST_UI_3D_SCENE = 51,
		PIPELINE_COLOR_PICKING = 100,
	};

	/** for post rendering Object.
	* such is usually for solid object, which will be drawn from front to back according to
	* its object-camera distance*/
	struct PostRenderObjectWeakPtr
	{
	public:
		float m_fObjectToCameraDistance;
		// this will lead to crash if used cross frame. 
		CBaseObject::WeakPtr_type m_pRenderObject;
		/// whether this object is occluded. default to 0, which means undetermined. 
		int m_nOccluded;
	public:
		/** create a new post render object with the Object surface to camera distance.*/
		PostRenderObjectWeakPtr(CBaseObject* pRenderObject, float fObjectToCameraDistance, int nOccluded = 0)
			:m_pRenderObject(pRenderObject), m_fObjectToCameraDistance(fObjectToCameraDistance), m_nOccluded(nOccluded) {};

		/** return true if occluded */
		bool IsOccluded() const { return m_nOccluded > 1; }
	};

	/** for post rendering Object.
	* such is usually for solid object, which will be drawn from front to back according to
	* its object-camera distance*/
	struct PostRenderObject
	{
	public:
		float m_fObjectToCameraDistance;
		// this will lead to crash if used cross frame. 
		CBaseObject* m_pRenderObject;
		/// whether this object is occluded. default to 0, which means undetermined. 
		int m_nOccluded;
	public:
		/** create a new post render object. The Object surface to camera distance will be automatically computed.*/
		//PostRenderObject(CBaseObject* pRenderObject);
		/** create a new post render object with the Object surface to camera distance.*/
		PostRenderObject(CBaseObject* pRenderObject, float fObjectToCameraDistance, int nOccluded = 0);

		//PostRenderObject& operator = (const PostRenderObject& _right);

		/** return true if occluded */
		bool IsOccluded() const { return m_nOccluded > 1; }
	};

	/** for post rendering object with alpha.
	* such is usually for transparent objects, which will be drawn from back to front according to
	* its object-camera distance */
	struct AlphaPostRenderObject : PostRenderObject
	{
	public:
		// the alpha value: 0-1.0f
		float m_fAlpha;
		/** alpha animation*/
		enum ALPHA_ANIM{
			ALPHA_INCREASE_ANIM,	/// increase the alpha value, if it is 1.0f, it will be removed from the list.
			ALPHA_DECREASE_ANIM,		/// decrease the alpha value, if it is 0.0f, it will be removed from the list. 
			ALPHA_NO_ANIM,			/// no animation
		};
		/// alpha animation method.
		ALPHA_ANIM m_nAlphaAnimMethod;
	public:
		AlphaPostRenderObject(CBaseObject* pRenderObject, float fObjectToCameraDistance, int nOccluded = 0, float fAlpha = 1.f, ALPHA_ANIM nAlphaAnimMethod = ALPHA_NO_ANIM);
	};

	/**
	* compare two post rendering object by its object to camera distance
	* Type must be pointer to PostRenderObject derived class
	*/
	template<class Type>
	struct LessPostRenderObj
	{
		bool operator()(
			const Type& _Left,
			const Type& _Right
			) const
		{
			int nLeftTech = _Left.m_pRenderObject->GetPrimaryTechniqueHandle();
			int nRightTech = _Right.m_pRenderObject->GetPrimaryTechniqueHandle();

#ifdef DONOT_SORT_BY_ASSET
			return ( (nLeftTech < nRightTech) || 
				((nLeftTech == nRightTech) && ((_Left.m_fObjectToCameraDistance) < (_Right.m_fObjectToCameraDistance))) );
#else	
			AssetEntity* LeftAsset = _Left.m_pRenderObject->GetPrimaryAsset();
			AssetEntity* RightAsset = _Right.m_pRenderObject->GetPrimaryAsset();
			return ((nLeftTech < nRightTech) ||
				((nLeftTech == nRightTech) && (LeftAsset < RightAsset)) ||
				((nLeftTech == nRightTech) && (LeftAsset == RightAsset) && ((_Left.m_fObjectToCameraDistance) < (_Right.m_fObjectToCameraDistance))));
#endif
		};
	};

	template<class Type>
	struct LessPostRenderObj_NoTechBatch
	{
		inline bool operator()(
			const Type& _Left,
			const Type& _Right
			) const
		{
			return (_Left.m_fObjectToCameraDistance) < (_Right.m_fObjectToCameraDistance);
		};
	};

	/** first sort from large to small by render importance and if render importance are same from near camera to far from camera.*/
	template<class Type>
	struct LessPostRenderObj_BipedSort
	{
		inline bool operator()(
			const Type& _Left,
			const Type& _Right
			) const
		{
			int nLeftRenderImportance = _Left.m_pRenderObject->GetRenderImportance();
			int nRightRenderImportance = _Right.m_pRenderObject->GetRenderImportance();
			return (nLeftRenderImportance > nRightRenderImportance) ||
				((nLeftRenderImportance == nRightRenderImportance) && ((_Left.m_fObjectToCameraDistance) < (_Right.m_fObjectToCameraDistance)));
		};
	};

	template<class Type>
	struct GreaterPostRenderObj
	{
		bool operator()(
			const Type& _Left,
			const Type& _Right
			) const
		{
			int nLeftTech = _Left.m_pRenderObject->GetPrimaryTechniqueHandle();
			int nRightTech = _Right.m_pRenderObject->GetPrimaryTechniqueHandle();

#ifdef DONOT_SORT_BY_ASSET
			return ( (nLeftTech < nRightTech) || 
				((nLeftTech == nRightTech) && ((_Left.m_fObjectToCameraDistance) > (_Right.m_fObjectToCameraDistance))) );
#else	
			AssetEntity* LeftAsset = _Left.m_pRenderObject->GetPrimaryAsset();
			AssetEntity* RightAsset = _Right.m_pRenderObject->GetPrimaryAsset();
			return ((nLeftTech < nRightTech) ||
				((nLeftTech == nRightTech) && (LeftAsset < RightAsset)) ||
				((nLeftTech == nRightTech) && (LeftAsset == RightAsset) && ((_Left.m_fObjectToCameraDistance) > (_Right.m_fObjectToCameraDistance))));
#endif
		};
	};

	template<class Type>
	struct GreaterPostRenderObj_NoTechBatch
	{
		inline bool operator()(
			const Type& _Left,
			const Type& _Right
			) const
		{
			return (_Left.m_fObjectToCameraDistance) > (_Right.m_fObjectToCameraDistance);
		};
	};

	/** compare two object's primary asset */
	template<class Type> /** Type must be pointer to CBaseobject derived class */
	struct CmpBaseObjectAsset
	{
		bool operator()(
			const Type& _Left,
			const Type& _Right
			) const
		{
			int nLeftTech = _Left->GetPrimaryTechniqueHandle();
			int nRightTech = _Right->GetPrimaryTechniqueHandle();
			return ((nLeftTech < nRightTech) ||
				((nLeftTech == nRightTech) && (_Left->GetPrimaryAsset()) < (_Right->GetPrimaryAsset())));
		};
	};

	/** which render state the current render pipeline is in. */
	enum SceneStateRenderState
	{
		RenderState_Standard,
		RenderState_3D,
		RenderState_Shadowmap,
		RenderState_Headon,
		RenderState_Overlay_ZPass,
		RenderState_Overlay,
		RenderState_Overlay_Picking,
		RenderState_Debug,
		RenderState_GUI,
	};

	/** push a render state */
	class CPushRenderState
	{
	public:
		CPushRenderState(SceneState* pState, SceneStateRenderState state);
		CPushRenderState(SceneState* pState);
		~CPushRenderState();
	private:
		SceneState* m_pState;
		SceneStateRenderState m_lastState;
	};

	/**
	It's used as parameter to Draw method of each scene object. Its only
	instance is in the RootScene object.
	It keeps track of all the intermediate results  generated during game rendering.
	It might be used by game interface, environment simulator or AI, since most of
	the time we are only interested of computing object that is in the view.
	*/
	struct SceneState : public IAttributeFields
	{
	public:
		// constructor
		SceneState(IScene * pScene = NULL);

		ATTRIBUTE_DEFINE_CLASS(SceneState);
		/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);

		ATTRIBUTE_METHOD1(SceneState, GetCurrentRenderPipeline_s, int*)	{ *p1 = cls->GetCurrentRenderPipeline(); return S_OK; }
		ATTRIBUTE_METHOD1(SceneState, SetCurrentRenderPipeline_s, int)	{ cls->SetCurrentRenderPipeline(p1); return S_OK; }

		ATTRIBUTE_METHOD1(SceneState, GetRenderState_s, int*)	{ *p1 = (int)cls->GetRenderState(); return S_OK; }
		ATTRIBUTE_METHOD1(SceneState, SetRenderState_s, int)	{ cls->SetRenderState((SceneStateRenderState)p1); return S_OK; }

		ATTRIBUTE_METHOD1(SceneState, GetCameraToCurObjectDistance_s, float*)	{ *p1 = cls->GetCameraToCurObjectDistance(); return S_OK; }
		ATTRIBUTE_METHOD1(SceneState, SetCameraToCurObjectDistance_s, float)	{ cls->SetCameraToCurObjectDistance(p1); return S_OK; }

		ATTRIBUTE_METHOD1(SceneState, GetGlobalTime_s, int*)	{ *p1 = cls->GetGlobalTime(); return S_OK; }
		ATTRIBUTE_METHOD1(SceneState, SetGlobalTime_s, int)	{ cls->SetGlobalTime(p1); return S_OK; }

	public:
		/** get the current scene object that is being processed. the object may not be
		* a valid scene object, unless one understands well when it is being assigned.
		* the current scene object is assigned by the scene object itself in
		* its animate or draw method.
		*/
		CBaseObject* GetCurrentSceneObject();
		/** set the current scene object. see GetCurrentSceneObject() */
		void  SetCurrentSceneObject(CBaseObject* pObj);

		/** set camera to current object distance. this is automatically set by the pipeline before drawing a certain object.  */
		void SetCameraToCurObjectDistance(float fDist);

		/** get camera to current object distance. this is automatically set by the pipeline before drawing a certain object.  */
		float GetCameraToCurObjectDistance();

		/** set shadow map camera to current object distance. This is usually a fixed value for all shadow casters, such as 500 meters */
		void SetShadowMapCameraToCurObjectDistance(float fDist);

		/** get shadow map camera to current object distance. This is usually a fixed value for all shadow casters, such as 500 meters  */
		float GetShadowMapCameraToCurObjectDistance();;

		SceneStateRenderState GetRenderState() const;
		void SetRenderState(SceneStateRenderState val);

		/** true if it is a shadow pass. Currently it only affects the GetCameraToCurObjectDistance() for auto LOD mesh picking.  */
		void SetShadowPass(bool bShadowPass);
		/** true if it is a shadow pass. Currently it only affects the GetCameraToCurObjectDistance() for auto LOD mesh picking.  */
		bool IsShadowPass();
		
		/** set the debug drawing object. */
		void SetBatchedElementDrawer(IBatchedElementDraw* pDebugDraw);

		/** get the debug drawing object. */
		IBatchedElementDraw* GetBatchedElementDrawer();

		/** get the global time. */
		int GetGlobalTime();
		void SetGlobalTime(int nGlobalTime);

		/** ignore all globally transparent and particle object */
		inline bool IsIgnoreTransparent() { return m_bIgnoreTransparent; }
		void SetIgnoreTransparent(bool bIgnore);

		/** this is the usually the global sun light material.*/
		ParaMaterial& GetGlobalMaterial();

		/** this is the usually the material local to a given 3d position.*/
		ParaMaterial& GetLocalMaterial();

		/** this is the main function used by effect file to apply material to shaders.*/
		ParaMaterial& GetCurrentMaterial();

		/** Whether to use local material as current material.*/
		void EnableLocalMaterial(bool bEnable = true);

		/** Whether to use local material.*/
		bool HasLocalMaterial();

		/** x is the sun light strength, y is local light strength. z is reserved.*/
		Vector3& GetCurrentLightStrength();

		RenderDevicePtr GetRenderDevice(){ return m_pd3dDevice; };

		BillBoardViewInfo& BillBoardInfo() { return m_BillBoardInfo; }

		/** whether we are currently using deferred shading */
		bool IsDeferredShading() const;

		/** whether we are currently using deferred shading */
		void SetDeferredShading(bool val);

		/** get current camera */
		CBaseCamera* GetCamera();

		/** one of the enum RENDER_PIPELINE_ORDER */
		int GetCurrentRenderPipeline() const;
		/** one of the enum RENDER_PIPELINE_ORDER */
		void SetCurrentRenderPipeline(int val);

		void AddToDeferredLightPool(CLightObject * lightObject);

	public:
		IScene* GetScene(){ return m_pScene; }

		RenderDevicePtr m_pd3dDevice;	/// rendering device pointer
		// scene associated with the state
		IScene*			m_pScene;
		/// the GUI rendering state
		GUIState*		  pGUIState;
		/// pointer to the asset manager
		CParaWorldAsset*  pAssetManager;

		/** one of the enum RENDER_PIPELINE_ORDER */
		int m_nCurrentRenderPipeline;

		/// time advance delta
		double			dTimeDelta;
		/// default material
		ParaMaterial	m_matGlobalMaterial;
		ParaMaterial	m_matLocalMaterial;
		bool			m_bUseLocalMaterial : 1;
		/// true if the it's currently performing batch rendering
		bool			bIsBatchRender : 1;
		bool			m_bCameraMoved : 1;
		/// translucent face sorting is enabled. 
		bool			m_bEnableTranslucentFaceSorting;
		/** how many times the scene has been rendered since its creation. If this is 0, we may be pre-render stage or rendering is disabled.*/
		int				m_nRenderCount;
		/** the frame number that this object is last accessed.*/
		//int m_nFrameNumber;

		float			fAlphaFactor;   /// the material alpha should be multiplied by this factor. For opaque object, this is 1.0f.
		/// the pipeline required cull mode, mostly for reflection rendering during fixed function pipeline, where the cull mode is reversed. 
		/// objects who will be rendered as reflections, should set the cull mode, according to this setting, rather then use what they think is correct.
		/// CULLNONE is an exception to this rule.
		DWORD			  m_dwD3D_CULLMODE;

		/// the default value of Occluded property when new objects are added to it. 
		int m_nOccluded;

		// -- camera and view
		Matrix4 mxView;				/// view transform of the current camera
		Matrix4 mxProj;				/// projection transform of the current camera
		Vector3 vEye;				/// camera position
		Vector3 vLookAt;			/// camera look at position
		BillBoardViewInfo m_BillBoardInfo;

		FLOAT	   fViewCullingRadius;  /// the radius of the circle with the center at vEye: 
		/// contents outside are culled out when rendering
		CBaseObject* pTouchedObject;    /// the object that has been touched by the mouse ray.
		/// current effect object, this may be NULL
		CEffectFile* m_pCurrentEffect;
		/** x is the sun light strength, y is local light strength. z is reserved.*/
		Vector3	m_curLightStrength;

		enum TOUCH_METHOD{ LButton, RButton, None }touchMethod; // how the above object is being touched

		/// At what stage is the scene currently being processed, ascending order
		enum ProcessingStage
		{
			/* stages during frame move*/
			stage_framemove = 0,
			stage_UserInputHandling = 1,
			stage_EnvironmentSim = 2,
			stage_AISimulator = 3,
			stage_RootSceneAnimate = 4,
			stage_RootGUIAnimate = 5,

			/* stages during rendering*/
			stage_SetCamera = 101,
			stage_AdvanceScene = 102,
			stage_AdvanceGUI = 103
		}CurrentStage;

		/** bitwise fields of RENDER_GROUP */
		DWORD	m_nCurRenderGroup;
	public:
		typedef std::vector<PostRenderObject>	List_PostRenderObject_Type;
		typedef std::vector<CSpriteObject*>	List_CSpriteObjectPtr_Type;
		typedef unordered_array<ParticleSystemRef>	List_ParticleSystemPtr_Type;
		typedef std::vector<AlphaPostRenderObject>	List_AlphaPostRenderObject_Type;
		typedef std::vector<PostRenderObjectWeakPtr>	List_HeadOnRenderObject_Type;
		typedef std::vector<CLightObject*>				List_LightObject_Type;
		

		/** Same as IRefObject except that it tracks references of the object. */
		class List_PostRenderObject_TrackRef_Type : public IRefObject, public List_PostRenderObject_Type
		{
		public:
			/** add an object */
			void push_back(PostRenderObject& item);

			/** clear all object */
			void clear();

			/** This is called after another object deleted this object from its reference list.
			* If this object needs to know, it can override it.
			* @param rm:
			* @remark: one can call DeleteReference in the OnRefDeleted, to delete bi-directional reference.
			* By default, a bi-directional reference is deleted in this function
			*/
			virtual void OnRefDeleted(IRefObject* rm);
		};

		// -- for batched rendering optimization such as sprites objects
		/**
		listPostRenderingObjects: this is the general post rendering list. Objects in this list are
		sorted only by primary asset entity name. Texture entity is usually considered as the primary
		asset entity, since texture switch is most slow on GPU. The process goes like this: scan the
		list. For each items with the same texture as the first element in the list, render and remove
		it from the list. This goes on until the list is blank. When inserting items into the list,
		you can also sort while inserting.
		listPRSprite: Not only sorted by texture, but also use transparent key | alpha channel for all objects in the list.
		listPRBiped: biped, sorted by primary asset entity, this is always the MA asset.
		Note: post rendering are performed in the order listed above.
		*/
		SceneState::List_PostRenderObject_Type		listPostRenderingObjects;   /// list of general post rendering object
#ifdef USE_DIRECTX_RENDERER
		SceneState::List_CSpriteObjectPtr_Type	listPRSprite;				/// list of Post Rendering object: Sprite
#endif
		SceneState::List_PostRenderObject_TrackRef_Type		listPRBiped;				/// list of Post Rendering object: biped
		SceneState::List_PostRenderObject_TrackRef_Type		listPRTransparentBiped;		/// list of Post Rendering object: biped with alpha blending

		/// solid meshes (object) are usually rendered from front to back(near to far).
		SceneState::List_PostRenderObject_Type	listPRSolidObject;
		SceneState::List_PostRenderObject_Type	listShadowCasters;
		SceneState::List_PostRenderObject_Type	listShadowReceivers;
		/// smaller objects are rendered after bigger objects are drawn for better occlusion.They are rendered from back to front.  
		SceneState::List_AlphaPostRenderObject_Type	listPRSmallObject;
		SceneState::List_HeadOnRenderObject_Type	listHeadonDisplayObject;

		/// transparent meshes (object) are usually rendered from back to front(far to near).
		/// and they are rendered after all solid meshes have already been rendered. 
		/// with "z-buffer write" disabled.
		SceneState::List_AlphaPostRenderObject_Type	listPRTransparentObject;
		SceneState::List_AlphaPostRenderObject_Type	listPRMissiles;
		SceneState::List_ParticleSystemPtr_Type	listParticleSystems;

		SceneState::List_PostRenderObject_Type		listZones;
		SceneState::List_PostRenderObject_Type		listPortals;
		SceneState::List_PostRenderObject_Type		listBoundingBox;
		SceneState::List_PostRenderObject_Type		listSelections;
		SceneState::List_LightObject_Type			listDeferredLightObjects;
		// like the render target 
		unordered_array<WeakPtr>		listOwnerDrawObjs;

		bool m_bSkipOcean;
		bool m_bSkipTerrain;
		bool m_bSkipSky;
		// whether the outer world is visible. this is usually true, unless camera is inside a zone. 
		// if this is false, ocean terrain sky are escaped. 
		bool m_bOuterWorldZoneVisible;
		/** whether we are currently using deferred shading */
		bool m_bDeferredShading;

		/** m_MaxRenderCount[RenderImportance] is the max number of objects to be rendered of a given render importance */
		vector<int> m_MaxRenderCount;

		/** m_RenderCount[RenderImportance] is the max number of objects that has already been rendered of a given render importance */
		vector<int> m_RenderCount;

		/** the max number of objects to be rendered of a given render importance. This is usually called at the beginning of a frame. */
		void SetMaxRenderCount(int nRenderImportance, int nCount);

		/**  Add one to the render count of a given render importance. return true if it has not reached MAX rendered count. */
		bool CheckAddRenderCount(int nRenderImportance);

		/** get the current frame render number */
		inline int GetRenderFrameCount() { return m_nRenderCount; }

		template <class T>
		typename T::value_type* FindObject(T& renderlist, CBaseObject* pObject)
		{
			typename T::iterator itCur, itEnd = renderlist.end();
			for (itCur = renderlist.begin(); itCur != itEnd; ++itCur)
			{
				if (itCur->m_pRenderObject == pObject)
				{
					return &(*itCur);
				}
			}
			return NULL;
		}

		/// face groups.
		CSortedFaceGroups m_SortedFaceGroups;

		/// clean up all objects in post rendering lists.  call this when you finished rendering one framework.
		void CleanupSceneState();
		/** this function must be called, when the entire scene is reset or cleaned up.*/
		void Cleanup();

		/** add a particle system to the scene state.
		* during each frame, the scene manager will render all absolute instances in the particle system list.
		*/
		void AddParticleSystem(ParticleSystem* ps);

		/** get sorted face groups. */
		CSortedFaceGroups* GetFaceGroups();

		HRESULT InitDeviceObjects();	// device independent
		HRESULT RestoreDeviceObjects(); // device dependent
		HRESULT InvalidateDeviceObjects();
		HRESULT DeleteDeviceObjects();

	private:
		/** the current scene object that is being processed.*/
		static CBaseObject*		m_pCurrentSceneObject;

		/** for debug object drawing. */
		IBatchedElementDraw* m_pBatchedDrawer;

		/** camera to current object distance. this is automatically set by the pipeline before drawing a certain object.  */
		float m_fCameraToObjectDistance;

		/** when shadow pass is enabled, we will return this distance for GetCameraToCurObjectDistance(). This is usually a fixed value for all shadow casters.
		* the value is usually set so that almost all objects uses the lowest lod. */
		float m_fShadowMapCameraToObjectDistance;

		/** ignore all globally transparent and particle object */
		bool m_bIgnoreTransparent;

		/** if we are rendering shadow map */
		bool m_bIsShadowPass;

		/** the global time in milliseconds. ticking from 0*/
		int m_nGlobalTime;

		/** which render state the current render pipeline is in. */
		SceneStateRenderState m_renderState;
	};
}
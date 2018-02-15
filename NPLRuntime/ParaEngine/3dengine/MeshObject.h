#pragma once
#include "TileObject.h"


namespace ParaEngine
{
	class CViewCullingObject;
	/** static mesh scene object.
	represent a static object in the scene such as a terrain mesh, floor, stone, houses, etc
	currently it only support X file asset entity, and no animation is allowed,
	object can be lighted and textured.
	*/
	class CMeshObject : public CTileObject
	{
	public:
		virtual CBaseObject::_SceneObjectType GetType(){return CBaseObject::MeshObject;};
		CMeshObject(void);
		virtual ~CMeshObject(void);

		virtual void SetPosition(const DVector3 & v);

		/** Rotate the object.This only takes effects on objects having 3D orientation, such as
		* static mesh and physics mesh. The orientation is computed in the following way: first rotate around x axis, 
		* then around y, finally z axis.
		* @param x: rotation around the x axis.
		* @param y: rotation around the y axis.
		* @param z: rotation around the z axis.
		*/
		virtual void Rotate(float x, float y, float z);

		/** set the scale of the object. This function takes effects on both character object and mesh object. 
		* @param s: scaling applied to all axis.1.0 means original size. */
		virtual void SetScaling(float s);
		/**get scaling */
		virtual float GetScaling();

		/** reset the object to its default settings.*/
		virtual void Reset();

		/** set rotation using a quaternion. */
		virtual void SetRotation(const Quaternion& quat);
		/** get rotation using a quaternion */
		virtual void GetRotation(Quaternion* quat);
	
		/** Set the current animation id
		* @param nAnimID: 0 is default standing animation. 4 is walking, 5 is running. more information, please see AnimationID */
		virtual void SetAnimation(int nAnimID);

		/** get the scaling. */
		virtual int GetAnimation();

		/** set the current animation frame number relative to the beginning of current animation. 
		* @param nFrame: 0 means beginning. if nFrame is longer than the current animation length, it will wrap (modulate the length).
		*/
		virtual void SetAnimFrame(int nFrame);

		/** get the current animation frame number relative to the beginning of current animation.  */
		virtual int GetAnimFrame();

		/** this function is called, when the object is in view range. we may need to load the primary asset to update the bounding box, etc. 
		* @return true if the object is ready to be rendered. 
		*/
		virtual bool ViewTouch();

		/** whether to use global time to sync animation. Default to false. 
		if true, all characters plays exactly the same frames if they are using the same animation file at all times, 
		if false, each character advances its time frame separately according to their visibility in the scene. 
		@note: animation is always looped once use global time
		*/
		virtual void SetUseGlobalTime(bool bUseGlobalTime);

		/** whether to use global time to sync animation. Default to false. 
		if true, all characters plays exactly the same frames if they are using the same animation file at all times, 
		if false, each character advances its time frame separately according to their visibility in the scene. 
		@note: animation is always looped once use global time
		*/
		virtual bool IsUseGlobalTime();

		/** whether the object is persistent in the world. If an object is persistent, it will be saved to the world's database.
		if it is not persistent it will not be saved when the world closes. Player, OPC, some temporary movie actors may 
		by non-persistent; whereas NPC are usually persistent to the world that it belongs.*/
		virtual bool IsPersistent();

		/** whenever a persistent object is made non-persistent, the SaveToDB() function will actually removed it from the database and the action can not be recovered.
		* so special caution must be given when using this function to prevent accidentally losing information. 
		@see IsPersistent() */
		virtual void SetPersistent(bool bPersistent);

	public:

		//////////////////////////////////////////////////////////////////////////
		// implementation of IAttributeFields

		/** attribute class ID should be identical, unless one knows how overriding rules work.*/
		virtual int GetAttributeClassID(){return ATTRIBUTE_CLASSID_CMeshObject;}
		/** a static string, describing the attribute class object's name */
		virtual const char* GetAttributeClassName(){static const char name[] = "CMeshObject"; return name;}
		/** a static string, describing the attribute class object */
		virtual const char* GetAttributeClassDescription(){static const char desc[] = ""; return desc;}
		/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);
		ATTRIBUTE_SUPPORT_CREATE_FACTORY(CMeshObject);
		ATTRIBUTE_METHOD1(CMeshObject, IsFaceCullingDisabled_s, bool*)		{*p1 = cls->IsFaceCullingDisabled(); return S_OK;}
		ATTRIBUTE_METHOD1(CMeshObject, SetFaceCullingDisabled_s, bool)		{cls->SetFaceCullingDisabled(p1); return S_OK;}
	public:
		// -- for init and delete
		HRESULT InitObject(MeshEntity* ppMesh, TextureEntity* ppTexture, Vector3 vPos);
		virtual HRESULT InitDeviceObjects();
		HRESULT RestoreDeviceObjects();
		HRESULT InvalidateDeviceObjects();
		virtual void Cleanup();

		// animate the object
		virtual void Animate( double dTimeDelta, int nRenderNumber=0 );

		// -- for rendering
		virtual HRESULT Draw( SceneState * sceneState);
	
		/**it is the same as draw, except that it will use the current transform, instead of applying by the mesh object's position and transform. 
		* @param pMxWorld: if NULL, the current one will be used. 
		* @param materialParams: overwrite local effect parameter. 
		*/
		HRESULT DrawInner( SceneState * sceneState, const Matrix4* pMxWorld=0, float fCameraToObjectDist=0.f, CParameterBlock* materialParams = NULL);

		/**
		* return the world matrix of the object for rendering
		* @param out: the output.
		* @param nRenderNumber: if it is bigger than current calculated render number, the value will be recalculated. 
		* @return: same as out. or NULL if not exists.
		*/
		virtual Matrix4* GetRenderMatrix( Matrix4& out, int nRenderNumber=0);

		/** Get the specified attachment matrix of the current model.
		* this is usually for getting the mount point on a certain model, such as horses. It also works for static mesh with xrefed mountable objects. 
		* @param pOut: output result 
		* @param nAttachmentID: see ATTACHMENT_ID. default to 0, which is general mount point. ATT_ID_MOUNT1-9(20-28) is another mount points
		* @param nRenderNumber: if it is bigger than current calculated render number, the value will be recalculated. 
		* @return: NULL if not attachment found, otherwise it is pOut.
		*/
		virtual Matrix4* GetAttachmentMatrix(Matrix4& pOut, int nAttachmentID=0, int nRenderNumber=0);

		virtual AssetEntity* GetPrimaryAsset(){return (AssetEntity*)m_ppMesh.get();};				// used as KEY for batch rendering

		virtual float GetCtorPercentage();
		virtual void SetCtorPercentage(float fPercentage);

		/** return true if the object contains alpha blended render pass. This function determines which render pipeline stage the object is rendered.
		* generally speaking, if deferred shading is used, we will render alpha blended objects last.
		*/
		virtual bool HasAlphaBlendedObjects();

		/**
		* whether the object contains transparent material. Ideally, we should perform polygon-level sorting for transparent faces;
		* however, at the moment, we just draw them last, after all solid objects are drawn.
		* @return true if it is partially or completely transparent.
		*/
		virtual bool IsTransparent(){return m_bIsTransparent;};

		/**
		* Set whether the object is transparent. 
		* @param bIsTransparent 
		*/
		virtual void SetTransparent(bool bIsTransparent){m_bIsTransparent = bIsTransparent;};

		/** set local transform directly */
		virtual void SetLocalTransform(const Matrix4& mXForm);
		/** get local transform*/
		virtual void GetLocalTransform(Matrix4* localTransform);

		/** set local transform by first uniform scale, then rotate around Z, X, Y axis sequentially. */
		void SetLocalTransform(float fScale, float fRotX, float fRotY, float fRotZ);
		/** set local transform by first uniform scale, then rotate using a quaternion. */
		void SetLocalTransform(float fScale, const Quaternion& quat);
		/** get local transform*/
		void GetLocalTransform(float* fScale, Quaternion* quat);
	
		virtual IViewClippingObject* GetViewClippingObject();

		/**
		* whether force rendering without local or global lighting effect. 
		* @param bNoLight 
		*/
		void ForceNoLight(bool bNoLight);
		/**
		* @return whether force rendering without local or global lighting effect. 
		*/
		bool IsNoLight();

		/**
		* get the total number of replaceable textures, which is the largest replaceable texture ID. 
		* but it does not mean that all ID contains valid replaceable textures.  
		* This function can be used to quickly decide whether the model contains replaceable textures. 
		* Generally we allow 32 replaceable textures per model.
		* @note: This function will cause the mesh entity to be initialized.
		* @return 0 may be returned if no replaceable texture is used by the model.
		*/
		virtual int GetNumReplaceableTextures();

		/**
		* get the default replaceable texture by its ID. 
		* The default replaceable texture is the main texture exported from the 3dsmax exporter.
		* @note: This function will cause the mesh entity to be initialized.
		* @param ReplaceableTextureID usually [0-32)
		*	generally speaking, replaceable ID 0 is used for general purpose replaceable texture, ID 1 is for user defined. ID 2 is for custom skins.
		* @return this may return NULL, if replaceable texture is not set before or ID is invalid.
		*/
		virtual TextureEntity* GetDefaultReplaceableTexture(int ReplaceableTextureID); 

		/**
		* get the current replaceable texture by its ID.
		* if no replaceable textures is set before, this will return the same result as GetNumReplaceableTextures(). 
		* @note: This function will cause the mesh entity to be initialized.
		* @param ReplaceableTextureID usually [0-32)
		*	generally speaking, replaceable ID 0 is used for general purpose replaceable texture, ID 1 is for user defined. ID 2 is for custom skins.
		* @return this may return NULL, if replaceable texture is not set before or ID is invalid.
		*/
		virtual TextureEntity* GetReplaceableTexture(int ReplaceableTextureID); 

		/**
		* set the replaceable texture at the given index with a new texture. 
		* this function will succeed regardless whether the mesh is initialized. Hence it can be used at loading time. 
		* because default instance of the mesh may use different replaceable texture set. 
		* @param ReplaceableTextureID usually [0-32)
		*	generally speaking, replaceable ID 0 is used for general purpose replaceable texture, ID 1 is for user defined. ID 2 is for custom skins.
		* @param pTextureEntity The reference account of the texture entity will be automatically increased by one. 
		* @return true if succeed. if ReplaceableTextureID exceed the total number of replaceable textures, this function will return false.
		*/
		virtual bool  SetReplaceableTexture(int ReplaceableTextureID, TextureEntity* pTextureEntity); 

		/// search the associated mesh entity for any XReference object
		void SearchXRefObject();

		/// get the number of the animation X reference instances
		int GetXRefInstanceCount();

		/// return animation xref instance by index
		XRefObject* GetXRefInstanceByIndex(int nIndex);

		/// remove and clean up all anim xref references
		void CleanupXRefInstances();

		///// get the number of the animation X reference instances
		//int GetXRefMeshCount();

		///// return mesh xref instance by index
		//XRefObject* GetXRefMeshByIndex(int nIndex);

		///// remove and clean up all mesh xref references
		//void CleanupXRefMesh();

	
		/// get the number of the script X reference instances
		int GetXRefScriptCount();

		/// return xref script file path by index
		const char* GetXRefScript(int nIndex);

		/**
		* get the 3D position in world space of the script object's origin
		*/
		Vector3 GetXRefScriptPosition(int nIndex);

		/**
		* get the scaling of the object in both x,y,z directions
		*/
		Vector3 GetXRefScriptScaling(int nIndex);

		/**
		* get the facing of the object in xz plane
		*/
		float GetXRefScriptFacing(int nIndex);

		/**
		* get the local transform of the script object. It contains scale and rotation only
		* the string returned by this function must be used at once and should not be saved. This function is also not thread-safe. 
		* @return: "11,12,13, 21,22,23, 31,32,33,41,42,43" ,where 41,42,43 are always 0. It may return NULL, if the local matrix does not exist.
		*/
		const char* GetXRefScriptLocalMatrix(int nIndex);


		/** whether we will turn off any material level face culling setting. but use the current face culling setting in the effect manager. */
		void SetFaceCullingDisabled(bool bDisableFaceCulling);

		/** whether we will turn off any material level face culling setting. but use the current face culling setting in the effect manager. */
		bool IsFaceCullingDisabled();
	private:

		/** return true if asset effect type is known. and false in case the asset is not loaded yet. */
		bool SetParamsFromAsset();

	protected:
		CViewCullingObject* m_pViewClippingObject;

		/// mesh local world xform: center in the origin
		Matrix4            m_mxLocalTransform;

		/// mesh geometry
		asset_ptr<MeshEntity>       m_ppMesh;
		vector< asset_ptr<TextureEntity> >		m_ReplaceableTextures;

		/// xref animation instances in the model.
		vector<XRefObject*>	m_XRefObjects;
	
		/// texture
		asset_ptr<TextureEntity>		  m_ppTexture;					

		/** construction progress of the mesh */
		float		m_fCtorPercentage;

		/** animation id for xref animated object. default to 0. */
		int m_nAnimID;

		/** animation frame for xref animated object. default to 0. */
		int m_nAnimFrame;

		/** whether to use global time for all animation. */
		bool m_bUseGlobalTime;

		/** whether the object contains transparent material. Ideally, we should perform polygon-level sorting for transparent face;
		however, at the moment, we just draw it last, after all solid meshes are drawn. */
		bool m_bIsTransparent;

		/** force rendering without local or global lighting effect. */
		bool m_bForceNoLight;

		/** if true, we will disable facing culling when rendering the object. */
		bool m_bDisableFaceCulling;

		/// whether view clipping object should be updated. 
		bool m_bViewClippingUpdated;

		/// whether we have searched the XRef object, we will use late evaluation and only load XRef when they are drawn. 
		bool m_XRefSearched;

		/** whether it is persistent in the world. If an object is persistent, it will be saved to the world's database.
		if it is not persistent it will not be saved when the world closes. Player, OPC, some temporary movie actors may 
		by non-persistent; whereas NPC are usually persistent to the world that it belongs.*/
		bool m_bIsPersistent;

		friend class CMeshPhysicsObject;
		friend class CSkyMesh;
	};
}
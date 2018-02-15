#pragma once

namespace ParaEngine
{
	class CMeshObject;
	struct AssetEntity;
	class CParaXAnimInstance;
	class CharModelInstance;
	struct SceneState;


	/** base class for all xref objects */
	class XRefObject
	{
	public:
		XRefObject(CMeshObject* pParent, AssetEntity* pModel, const Vector3& vOrigin, const Matrix4& mxLocalTransform);
		XRefObject();
		virtual ~XRefObject(void);

		/** Get the parent object associated with this object*/
		CMeshObject* GetParent(){return m_pParent;};

		/** return the model asset associated with this object */
		AssetEntity* GetModelAsset(){return m_pModelEntity.get();};

		/** return true if asset effect type is known. and false in case the asset is not loaded yet. */
		virtual bool SetParamsFromAsset();

		/** animate the model */
		virtual void Animate( double dTimeDelta, int nRenderNumber=0 ){};

		/** draw the model*/
		virtual HRESULT Draw( SceneState * sceneState);

		// it is the same as draw, except that it will use the current transform, instead of applying by the mesh object's position and transform. 
		// @param pMxWorld: if NULL, the current one will be used. 
		virtual HRESULT DrawInner( SceneState * sceneState, const Matrix4* pMxWorld=0, float fCameraToObjectDist=0.f);

		/**
		* set the replaceable texture at the given index with a new texture. 
		* this function will succeed regardless whether the mesh is initialized. Hence it can be used at loading time. 
		* because default instance of the mesh may use different replaceable texture set. 
		* @param ReplaceableTextureID usually [0-32)
		*	generally speaking, replaceable ID 0 is used for general purpose replaceable texture, ID 1 is for user defined. ID 2 is for custom skins.
		* @param pTextureEntity The reference account of the texture entity will be automatically increased by one. 
		* @return true if succeed. if ReplaceableTextureID exceed the total number of replaceable textures, this function will return false.
		*/
		virtual bool  SetReplaceableTexture(int ReplaceableTextureID, TextureEntity* pTextureEntity){return true;}; 

		/** get matrix for rendering 
		* @param mxWorld: out 
		* @param pMxParent: in, if NULL, parent render world matrix is used. 
		* @param nRenderNumber: if it is bigger than current calculated render number, the value will be recalculated. If 0, it will not recalculate
		*/
		Matrix4* GetRenderMatrix(Matrix4& mxWorld, const Matrix4* pMxParent=NULL, int nRenderNumber=0);

		/** Get the specified attachment matrix of the current model.
		* this is usually for getting the mount point on a certain model, such as horses. It also works for static mesh with xrefed mountable objects. 
		* @param pOut: output result 
		* @param nAttachmentID: see ATTACHMENT_ID. default to 0, which is general mount point. ATT_ID_MOUNT1-9(20-28) is another mount points
		* @param nRenderNumber: if it is bigger than current calculated render number, the value will be recalculated. If 0, it will not recalculate
		* @return: NULL if not attachment found, otherwise it is pOut.
		*/
		virtual Matrix4* GetAttachmentMatrix(Matrix4& pOut, int nAttachmentID=0, int nRenderNumber=0){return NULL;};

		/** Set the current animation id
		* @param nAnimID: 0 is default standing animation. 4 is walking, 5 is running. more information, please see AnimationID */
		virtual void SetAnimation(int nAnimID){};

		/** get the scaling. */
		virtual int GetAnimation(){return 0;};

		/** set the current animation frame number relative to the beginning of current animation. 
		* @param nFrame: 0 means beginning. if nFrame is longer than the current animation length, it will wrap (modulate the length).
		*/
		virtual void SetAnimFrame(int nFrame){};

		/** get the current animation frame number relative to the beginning of current animation.  */
		virtual int GetAnimFrame(){return 0;};

		/** whether to use global time to sync animation. Default to false. 
		if true, all characters plays exactly the same frames if they are using the same animation file at all times, 
		if false, each character advances its time frame separately according to their visibility in the scene. 
		@note: animation is always looped once use global time
		*/
		virtual void SetUseGlobalTime(bool bUseGlobalTime){};

		/** whether to use global time to sync animation. Default to false. 
		if true, all characters plays exactly the same frames if they are using the same animation file at all times, 
		if false, each character advances its time frame separately according to their visibility in the scene. 
		@note: animation is always looped once use global time
		*/
		virtual bool IsUseGlobalTime(){return false;};

		/** get local matrix*/
		Matrix4& GetLocalMatrix() {return m_mxLocalTransform;};

		/** get matrix relative to a parent matrix */
		Matrix4* GetMatrix(Matrix4& mxWorld, const Matrix4* pMatParent);


		/** when batch-rendering a group of objects, objects are usually sorted by their render techniques and then by their primary asset. 
		* One can access the effect file currently associated with the handle in the asset manager. Please note that, the game engine may change the mapping
		* from technique handle to the effect file, secretly at runtime. Effect files capable to be assigned to the same technique handle must 
		* also share the same (vertex) input declaration in order to be swapped secretly at runtime. 
		* Note: the Draw() function of object will dynamically retrieve the effect file for each call. If the effect file is not valid, it will try to render 
		* using directx 9's fixed programming pipeline. 
		* @see TechniqueHandle
		*/
		virtual int GetPrimaryTechniqueHandle() {return m_nTechniqueHandle;};

		/**
		* Set a new render technique handle. If the effect file associated with the handle is invalid or do not share the same input declaration as 
		* the object at the time of drawing, the object will not be drawn or will be drawn improperly.
		* This function rarely needs to be called by users. Technique handles are usually automatically assigned by the game engine when the asset file is loaded.
		* Of course, users can use special effect files for the rendering of special objects; then this function needs to be called. 
		* @param nHandle: @see TechniqueHandle
		*/
		virtual void SetPrimaryTechniqueHandle(int nHandle){m_nTechniqueHandle = nHandle;};
	protected:
		// asset entity
		asset_ptr<AssetEntity>			m_pModelEntity;
		
		/// the xrefed model's origin relative to its parent model. 
		/// this is usually a const read from the model file
		Vector3 m_vOrigin;


		/// animated local transform: this is usually a const read from the model file
		Matrix4            m_mxLocalTransform;

		/** the primary technique handle*/
		int m_nTechniqueHandle;

		/// parent object
		CMeshObject* m_pParent;
	};

	/** mesh instance referenced in model object.*/
	class XRefMeshObject : public XRefObject
	{
	public:
		XRefMeshObject(CMeshObject* pParent, AssetEntity* pModel, const Vector3& vOrigin, const Matrix4& mxLocalTransform);
		XRefMeshObject();
		virtual ~XRefMeshObject(void);

		/** return true if asset effect type is known. and false in case the asset is not loaded yet. */
		virtual bool SetParamsFromAsset();

	public:
		/** draw the model*/
		virtual HRESULT Draw( SceneState * sceneState);
		// it is the same as draw, except that it will use the current transform, instead of applying by the mesh object's position and transform. 
		// @param pMxWorld: if NULL, the current one will be used. 
		virtual HRESULT DrawInner( SceneState * sceneState, const Matrix4* pMxWorld=0, float fCameraToObjectDist=0.f);

		/** reset replaceable texture */
		virtual bool  SetReplaceableTexture(int ReplaceableTextureID, TextureEntity* pTextureEntity); 
	};

	/**
	* animated animation instance referenced in model object.
	*  Parent(StaticMesh)-->Several Child(CParaXAnimInstance)
	*		V                       V
	*  MeshObject---------->Several Child(XRefAnimInstance)
	*/
	class XRefAnimInstance : public XRefObject
	{
	public:
		XRefAnimInstance(CMeshObject* pParent, AssetEntity* pModel, const Vector3& vOrigin, const Matrix4& mxLocalTransform);
		~XRefAnimInstance();

		/** return true if asset effect type is known. and false in case the asset is not loaded yet. */
		virtual bool SetParamsFromAsset();

		/** get the animation instance.*/
		CAnimInstanceBase* GetAnimInstance();

		/** get the ParaX animation instance. This function will return NULL, if it is not a ParaX AnimInstance
		* one can then check whether the model has mount point.
		*/
		CParaXAnimInstance* GetParaXAnimInstance();

		/** delete AnimInstance*/
		void DeleteAnimInstance();

		/** get the character model instance. This function will return NULL, if there is not a character 
		* model instance associated with this node*/
		CharModelInstance* GetCharModelInstance();

		/** animate the model */
		virtual void Animate( double dTimeDelta, int nRenderNumber=0  );

		/** draw the model*/
		virtual HRESULT Draw( SceneState * sceneState);

		// it is the same as draw, except that it will use the current transform, instead of applying by the mesh object's position and transform. 
		// @param pMxWorld: if NULL, the current one will be used. 
		virtual HRESULT DrawInner( SceneState * sceneState, const Matrix4* pMxWorld=0, float fCameraToObjectDist=0.f);

		/** reset replaceable texture */
		virtual bool  SetReplaceableTexture(int ReplaceableTextureID, TextureEntity* pTextureEntity); 

		/** Get the specified attachment matrix of the current model.
		* this is usually for getting the mount point on a certain model, such as horses. It also works for static mesh with xrefed mountable objects. 
		* @param pOut: output result 
		* @param nAttachmentID: see ATTACHMENT_ID. default to 0, which is general mount point. ATT_ID_MOUNT1-9(20-28) is another mount points
		* @param nRenderNumber: if it is bigger than current calculated render number, the value will be recalculated. 
		* @return: NULL if not attachment found, otherwise it is pOut.
		*/
		virtual Matrix4* GetAttachmentMatrix(Matrix4& pOut, int nAttachmentID=0, int nRenderNumber=0);

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
	private:
		/// animation instance for m_pModelEntity Animated character model object
		ref_ptr<CAnimInstanceBase>	m_pAI;
	};
}

#pragma once
#include "TileObject.h"

namespace ParaEngine
{
	class CMeshObject;
	struct IParaPhysicsActor;
	class CPhysicsWorld;
	struct MeshEntity;

	/**
	* A static mesh based physical object in the scene
	* please note that it keeps the static mesh object as its member rather than inheriting from it.
	* in future, the mesh physics object may contain different kinds of mesh objects.
	*/
	class CMeshPhysicsObject : public CTileObject
	{
	public:
		virtual CBaseObject::_SceneObjectType GetType(){ return CBaseObject::MeshPhysicsObject; };
		virtual std::string ToString(DWORD nMethod);
		CMeshPhysicsObject(void);
		virtual ~CMeshPhysicsObject(void);

	private:
		/** Create the mesh object for rendering, which shares the same mesh with the physics object	*/
		void CreateMeshObject();

		void SetParamsFromAsset();

		/// Pointer to the static physics actor in physics engine
		vector<IParaPhysicsActor*> m_staticActors;

		/** whether it is persistent in the world. If an object is persistent, it will be saved to the world's database.
		if it is not persistent it will not be saved when the world closes. Player, OPC, some temporary movie actors may
		by non-persistent; whereas NPC are usually persistent to the world that it belongs.*/
		bool m_bIsPersistent;

	public:

		//////////////////////////////////////////////////////////////////////////
		// implementation of IAttributeFields

		/** attribute class ID should be identical, unless one knows how overriding rules work.*/
		virtual int GetAttributeClassID(){ return ATTRIBUTE_CLASSID_CMeshPhysicsObject; }
		/** a static string, describing the attribute class object's name */
		virtual const char* GetAttributeClassName(){ static const char name[] = "CMeshPhysicsObject"; return name; }
		/** a static string, describing the attribute class object */
		virtual const char* GetAttributeClassDescription(){ static const char desc[] = ""; return desc; }
		/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);

		ATTRIBUTE_SUPPORT_CREATE_FACTORY(CMeshPhysicsObject);
		ATTRIBUTE_METHOD1(CMeshPhysicsObject, IsFaceCullingDisabled_s, bool*)		{ *p1 = cls->IsFaceCullingDisabled(); return S_OK; }
		ATTRIBUTE_METHOD1(CMeshPhysicsObject, SetFaceCullingDisabled_s, bool)		{ cls->SetFaceCullingDisabled(p1); return S_OK; }

	public:
		/** whether we will turn off any material level face culling setting. but use the current face culling setting in the effect manager. */
		void SetFaceCullingDisabled(bool bDisableFaceCulling);

		/** whether we will turn off any material level face culling setting. but use the current face culling setting in the effect manager. */
		bool IsFaceCullingDisabled();

		/** set the physics group ID to which this object belongs to
		default to 0, must be smaller than 32.
		please see groups Mask used to filter shape objects.
		- group 0 means physics object that will block the camera and player, such as building walls, big tree trunks, etc.
		- group 1 means physics object that will block the player, but not the camera, such as small stones, thin posts, trees, etc.
		*/
		virtual void SetPhysicsGroup(int nGroup);
		/** Get the physics group ID to which this object belongs to
		default to 0, must be smaller than 32.
		please see groups Mask used to filter shape objects.
		- group 0 means physics object that will block the camera and player, such as building walls, big tree trunks, etc.
		- group 1 means physics object that will block the player, but not the camera, such as small stones, thin posts, trees, etc.
		*/
		virtual int GetPhysicsGroup();

		/** get the number of physics actors. If physics is not loaded, the returned value is 0. */
		int GetStaticActorCount();

		/** if the object may contain physics*/
		virtual bool CanHasPhysics();

		/**
		* mesh asset file name ending with _a or _b (such as "tree_b.x" )will be forced no physics initially.
		* It will override the bApplyPhysics input parameter.
		* @param ppMesh mesh asset entity
		* @param fOBB_X
		* @param fOBB_Y
		* @param fOBB_Z
		* @param bApplyPhysics whether to apply physics initially.
		* @param localTransform
		* @return
		*/
		HRESULT InitObject(MeshEntity* ppMesh, float fOBB_X, float fOBB_Y, float fOBB_Z, bool bApplyPhysics, const Matrix4& localTransform);

		/** clean up */
		virtual void Cleanup();

		/** Rotate the object.This only takes effects on objects having 3D orientation, such as
		* static mesh and physics mesh. The orientation is computed in the following way: first rotate around x axis,
		* then around y, finally z axis.
		* @param x: rotation around the x axis.
		* @param y: rotation around the y axis.
		* @param z: rotation around the z axis.
		*/
		virtual void Rotate(float x, float y, float z);

		/** set rotation using a quaternion. */
		virtual void SetRotation(const Quaternion& quat);
		/** get rotation using a quaternion */
		virtual void GetRotation(Quaternion* quat);

		/** set the scale of the object. This function takes effects on both character object and mesh object.
		* @param s: scaling applied to all axis.1.0 means original size. */
		virtual void SetScaling(float s);
		/** get the size scale */
		virtual float GetScaling();

		/** reset the object to its default settings.*/
		virtual void Reset();

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

		/**
		* update the local transform of the mesh object. This function will not automatically update the physical scene.
		* Once need to manually reload the physics object.
		* @param localTransform: the new transform
		*/
		virtual void SetLocalTransform(const Matrix4& mXForm);
		virtual void GetLocalTransform(Matrix4* localTransform);

		virtual void SetBoundingBox(float fOBB_X, float fOBB_Y, float fOBB_Z, float fFacing);
		virtual void SetBoundRect(FLOAT fWidth, FLOAT fHeight, FLOAT fFacing);
		virtual void SetRadius(FLOAT fRadius);

		virtual Matrix4* GetRenderMatrix(Matrix4& out, int nRenderNumber = 0);

		/** Get the specified attachment matrix of the current model.
		* this is usually for getting the mount point on a certain model, such as horses. It also works for static mesh with xrefed mountable objects.
		* @param pOut: output result
		* @param nAttachmentID: see ATTACHMENT_ID. default to 0, which is general mount point. ATT_ID_MOUNT1-9(20-28) is another mount points
		* @return: NULL if not attachment found, otherwise it is pOut.
		*/
		virtual Matrix4* GetAttachmentMatrix(Matrix4& pOut, int nAttachmentID = 0, int nRenderNumber = 0);

		/** whether it has an attachment point
		* @param nAttachmentID: see ATTACHMENT_ID. default to 0, which is general mount point. ATT_ID_MOUNT1-9(20-28) is another mount points
		*/
		virtual bool HasAttachmentPoint(int nAttachmentID = 0);

		/** whether the object is persistent in the world. If an object is persistent, it will be saved to the world's database.
		if it is not persistent it will not be saved when the world closes. Player, OPC, some temporary movie actors may
		by non-persistent; whereas NPC are usually persistent to the world that it belongs.*/
		virtual bool IsPersistent();
		/** whenever a persistent object is made non-persistent, the SaveToDB() function will actually removed it from the database and the action can not be recovered.
		* so special caution must be given when using this function to prevent accidentally losing information.
		@see IsPersistent() */
		virtual void SetPersistent(bool bPersistent);

		/** get the mesh object associated with the object*/
		CMeshObject* GetMeshObject();
		/**
		* load the physics objects.
		* Once the physics object is created, its position and orientation can no longer be changed
		* Currently this function will be automatically called when it is inserted to the scene root.
		* with the ParaScene.Attach HAPI. So be sure, the position and facing has been set properly
		* before this.
		* Multiple calls to this function will not take effects. If the changing of the position is really
		* needed, one must explicitly call UnloadPhysics(), update the position and then call LoadPhysics()
		*
		* Even though operations such as changing the position, adding more shapes, or even deleting the static
		* actor are not explicitly forbidden, they are not recommended for two reasons: First, the
		* ParaEngine assumes that static actors are indeed static,and does all sorts of optimizations that
		* rely on this property.Second, the collision detection and object clipping code is also written
		* assuming that the position will not be changed during the game play.
		*/
		virtual void LoadPhysics();
		/**
		* load the physics object
		*/
		virtual void UnloadPhysics();

		/** by default physics is lazy-load when player walk into its bounding box, setting this to false will always load the physics.
		* Please note, one must EnablePhysics(true) before this one takes effect.
		*/
		virtual void SetAlwaysLoadPhysics(bool bEnable);

		/** this function is called, when the object is in view range. we may need to load the primary asset to update the bounding box, etc.
		* @return true if the object is ready to be rendered.
		*/
		virtual bool ViewTouch();

		/** this function will turn on or off the physics of the object. */
		virtual void EnablePhysics(bool bEnable);
		virtual bool IsPhysicsEnabled();

		/** unload physics.*/
		virtual void CompressObject(CompressOption option = (CompressOption)0xffff);
		virtual void SetPosition(const DVector3 & v);
		virtual void SetYaw(float fFacing);

		// animate the object
		virtual void Animate(double dTimeDelta, int nRenderNumber = 0);
		virtual HRESULT Draw(SceneState * sceneState);
		// used as KEY for batch rendering
		virtual AssetEntity* GetPrimaryAsset();
		virtual IViewClippingObject* GetViewClippingObject();
		virtual int GetPrimaryTechniqueHandle();
		virtual void SetPrimaryTechniqueHandle(int nHandle);
		/** get effect parameter block with this object.
		* @param bCreateIfNotExist:
		*/
		virtual CParameterBlock* GetEffectParamBlock(bool bCreateIfNotExist = false);

		virtual float GetCtorPercentage();
		virtual void SetCtorPercentage(float fPercentage);
		virtual void AutoSelectTechnique();

		/** set the callback scipt whenever the primary asset is successfully loaded. */
		virtual void SetOnAssetLoaded(const char* sCallbackScript);

		/**
		* called when an object is selected.
		* @param nGroupID the group ID in to which the object is selected.
		*/
		virtual void OnSelect(int nGroupID);
		/** called whenever an object is un-selected.*/
		virtual void OnDeSelect();

		/**
		* whether the object contains transparent material. Ideally, we should perform polygon-level sorting for transparent faces;
		* however, at the moment, we just draw them last, after all solid objects are drawn.
		* @return true if it is partially or completely transparent.
		*/
		virtual bool IsTransparent();

		/**
		* Set whether the object is transparent.
		* @param bIsTransparent
		*/
		virtual void SetTransparent(bool bIsTransparent);


		/** return true if the object contains alpha blended render pass. This function determines which render pipeline stage the object is rendered.
		* generally speaking, if deferred shading is used, we will render alpha blended objects last.
		*/
		virtual bool HasAlphaBlendedObjects();

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

	protected:
		// any bit wise combination of PHYSICS_METHOD
		DWORD m_dwPhysicsMethod;

		unsigned int m_nPhysicsGroup;
		/// pointer to a mesh object.
		union{
			CMeshObject * m_pMeshObject;
		};

	};
}
#pragma once
#include "IAttributeFields.h"

namespace ParaEngine
{
	class CanvasAttachment;
	struct LightParams;

	/**
	* A ParaX Model canvas is just a hierarchy of parax Models.
	* In a model canvas, there is a base model, all other child models may be attached to slots
	* on the base model. Slots are defined internally in the ParaX model file of the base model.
	* In ParaEngine, Model canvas is usually used for displaying a character biped model with
	* different body attachments, where the base model is the base character body model. The attachments
	* for the character model are static models such as armory, hats, etc.
	*/
	class ParaXModelCanvas : public IAttributeFields
	{
	public:
		ParaXModelCanvas();
		virtual ~ParaXModelCanvas();

		ATTRIBUTE_DEFINE_CLASS(ParaXModelCanvas);
		/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);

		/** get attribute by child object. used to iterate across the attribute field hierarchy. */
		virtual IAttributeFields* GetChildAttributeObject(const std::string& sName);
		/** get the number of child objects (row count) in the given column. please note different columns can have different row count. */
		virtual int GetChildAttributeObjectCount(int nColumnIndex = 0);
		/** we support multi-dimensional child object. by default objects have only one column. */
		virtual int GetChildAttributeColumnCount();
		virtual IAttributeFields* GetChildAttributeObject(int nRowIndex, int nColumnIndex = 0);

	public:
		/** call this to recalculate the bone transform matrices according to the current frame time
		* the caller must set the model's local time through other means previously.
		* @param pPose: it will override the poses defined in the model.
		* @param nAnimID: the animation id for attached models. By default, it is ANIM_STAND(0).
		* @return true if bones are animated
		*/
		bool Animate(SceneState * sceneState, CharacterPose* pPose, int nAnimID = 0, IAttributeFields* pAnimInstance = NULL);
		/** draw the canvas with all models on it. Be sure to call animate() before calling the draw method.*/
		void Draw(SceneState * sceneState, CParameterBlock* materialParams = NULL);
		/** build the shadow volume */
		void BuildShadowVolume(SceneState * sceneState, ShadowVolume * pShadowVolume, LightParams* pLight, Matrix4* mxWorld);

		/** set this model as automatic character model. Automatic character model will generate some of its own
		* bone matrix, such as the upper body turning angle.*/
		void SetAutoCharacterModel(bool bAutoCharacterModel);
		/**
		* reset the model canvas, and initialize with a base model without any attachments.
		* @param pModel: the model asset for the base model.
		* @return : true if success.
		*/
		bool InitBaseModel(ParaXEntity * pModel);

		/** mount the current model on another model, such as a horse.
		*/
		void MountOnModel(ParaXEntity *model, float fScale = 1.0f);
		/** unmount the current model.*/
		void UnmountModel();
		/** the base object is mounted on another model */
		bool IsMounted();
		/** get the base model, where body attachments are attached.Base model is usually the main character model.*/
		ParaXEntity* GetBaseModel();
		/** get the animation model. this is the base model if it is not mounted; otherwise it is the mount model.
		* When animating a model, such as walking, attacking, etc, only animated model are animated.
		*/
		ParaXEntity* GetAnimModel();
		/** add an attachment to the character
		* @param model: model to be attached
		* @param id: this is ATTACHMENT_ID which is defined in ParaXModel.h.
		*	one can specified, through this value, the position of the attachment point,
		*   such as head, shoulder, left/right hand, etc.
		* @param slot: an identifier of the object. One can later delete attachment by slot.
		*   there can be many objects attached to the same slots. some slots are reserved. see CharSlots enum.
		* @param fScale: the scale of attached object. default to 1.0
		*/
		CanvasAttachment* addAttachment(ParaXEntity *model, int id, int slot, float fScale = 1.0f);
		CanvasAttachment* addAttachment(MeshEntity *model, int id, int slot, float fScale = 1.0f);

		/** delete all attachment to this character. */
		void clearAttachments();
		/** delete everything on a specified slot of the character
		* @param slot: an identifier of the object. One can later delete attachment by slot.
		*   there can be many objects attached to the same slots. some slots are reserved. see CharSlots enum.
		*/
		void deleteSlot(int slot);

		/** get the first attachment matching the id*/
		CanvasAttachment* GetChild(int id, int slot = -1);

		/** delete all attachments which is attached to the position specified by the attachment ID
		* @param nAttachmentID: this is ATTACHMENT_ID which is defined in ParaXModel.h.
		*	one can specified, through this value, the position of the attachment point,
		*   such as head, shoulder, left/right hand, etc.
		*/
		void deleteAttachment(int nAttachmentID);
	
	public:
		enum _MODEL_LIGHT{
			LT_AMBIENT,
			LT_DIRECTIONAL,
			LT_MODEL_ONLY
		};
		_MODEL_LIGHT m_lightType;

		/** root canvas */
		ref_ptr<CanvasAttachment> m_root;

		/// whether the model is a automatic character model, bone matrix calculation for automatic model 
		/// is given externally. Such as the upper bottom turning angle. The default value is false.
		bool m_bIsAutoCharacter : 1;

	private:
		/// the main model in the canvas, all child models may reference into this model.
		asset_ptr<ParaXEntity> m_baseModel;
		/** the based model can be mounted on another object called Mount Model, such as horse, birds, etc.*/
		asset_ptr<ParaXEntity> m_MountModel;
	};

	typedef ref_ptr<CanvasAttachment> CanvasAttachmentPtr;

	/**
	* Attachment on parax model canvas
	*/
	class CanvasAttachment : public IAttributeFields
	{
	public:
		CanvasAttachment();
		CanvasAttachment(CanvasAttachment *parent, ParaXEntity *model, int id, int slot, float scale = 1.0f);
		CanvasAttachment(CanvasAttachment *parent, MeshEntity *model, int id, int slot, float scale = 1.0f);
		virtual ~CanvasAttachment();

		ATTRIBUTE_DEFINE_CLASS(CanvasAttachment);

		/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);

		/** get attribute by child object. used to iterate across the attribute field hierarchy. */
		virtual IAttributeFields* GetChildAttributeObject(const std::string& sName);
		/** get the number of child objects (row count) in the given column. please note different columns can have different row count. */
		virtual int GetChildAttributeObjectCount(int nColumnIndex = 0);
		/** we support multi-dimensional child object. by default objects have only one column. */
		virtual int GetChildAttributeColumnCount();
		virtual IAttributeFields* GetChildAttributeObject(int nRowIndex, int nColumnIndex = 0);
	
	public:
		void SetReplaceableTexture(TextureEntity* pTex);
		void SetModel(ParaXEntity* pModel);
		void SetModel(MeshEntity* pModel);

		/** set offset */
		void SetOffset(float x, float y, float z);

		/** set up the transform, so that this attached model can be drawn in its local space.
		* @param fCameraToCurObjectDistance: this is only used for LOD
		*/
		bool SetupParantTransform(float fCameraToCurObjectDistance = 0.f);

		/**
		* add a new model to this node.
		* @param model: the parax model entity
		* @param id: model id, which is used to find the attach point(transform) in the parent model.
		* @param slot: slot is a number which designate a predefined position on the base mode, to which
		*	another model can be attached.
		* @param scale: the scale applies to all x,y,z of the attached model. default value is 1.0
		*/
		CanvasAttachment* addChild(ParaXEntity *model, int id, int slot, float scale = 1.0f);
		CanvasAttachment* addChild(MeshEntity *model, int id, int slot, float scale = 1.0f);

		/** get the first attachment matching the id*/
		CanvasAttachment* GetChild(int id, int slot = -1);

		/** get the mesh object's attribute object. */
		IAttributeFields * GetAttributeObject();

		/** attach a new child to this attachment.  The new child will be deleted automatically.
		*/
		void AttachChild(CanvasAttachment* att);

		void delSlot(int slot);
		void delChildren();
		void delChild(CanvasAttachment* child);
		/** delete a child by its ID */
		void delChildByID(int nID);

		/** this is a self destruction method. it delete this;
		* Note: use with caution.
		*/
		void release();
		/** recalculate all bones of this attachment and all its children.
		* @param pPose: it will override the poses defined in the model.
		* @param bUseGlobal: if true, the standing animation will be played using the global timer. This is usually for attached the objects.
		* @param nAnimID: the animation id to play. It defaults to ANIM_STAND(0)
		* @return true if bones are animated
		*/
		bool animate(SceneState * sceneState, CharacterPose* pPose, bool bUseGlobal = false, int nAnimID = 0, IAttributeFields* pAnimInstance = NULL);

		/** draw this attachment and all its children. */
		void draw(SceneState * sceneState, ParaXModelCanvas *c, CParameterBlock* materialParams = NULL);
		/** build the shadow volume */
		void BuildShadowVolume(SceneState * sceneState, ShadowVolume * pShadowVolume, LightParams* pLight, Matrix4* mxWorld);

	public:
		/** child models */
		std::vector< CanvasAttachmentPtr > children;

		/** weak reference to parent */
		CanvasAttachment * parent;

		// model associated with this attachment
		asset_ptr<ParaXEntity> model;
		// static mesh entity, for backward compatibility
		// asset_ptr<MeshEntity> m_pMeshEntity;

		// static mesh object
		ref_ptr<CMeshObject> m_pMeshObject;

		// a replaceable texture for use with the model on this attachment.
		asset_ptr<TextureEntity> texReplaceable;
		

		/// this id will be used to index into the parent model's attachment lookup table to find the
		/// actual attachment point in the parent model.
		int id;
		/// the slot ID used to identify the logical position of this attachment
		int slot;
		/// this is a model scale which applies to x,y,z axis uniformly.
		float scale;
		Vector3 m_vOffset;
		/// whether the model is a automatic character model, bone matrix calculation for automatic model 
		/// is given externally. Such as the upper bottom turning angle. The default value is false.
		bool m_bIsAutoCharacter : 1;
	};

}
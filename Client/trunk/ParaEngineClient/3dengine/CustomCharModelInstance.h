#pragma once
#include "IAttributeFields.h"
#include "CustomCharCommon.h"
#include "CustomCharFace.h"

namespace ParaEngine
{
	class CartoonFace;
	class ParaXModelCanvas;
	class CParaXModel;
	struct LightParams;
	struct AnimIndex;

	/**
	* character animation instance: it includes both the animation and model display information.
	*/
	class CharModelInstance : public IAttributeFields
	{
	public:
		CharModelInstance();
		virtual ~CharModelInstance();

		ATTRIBUTE_DEFINE_CLASS(CharModelInstance);

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
		/** the cartoon faces that needs to recomposed as soon as possible. possibly in a batch. */
		static std::list <CharModelInstance*> g_listPendingCartoonFaces;

		/** the body textures that needs to recomposed as soon as possible. possibly in a batch. */
		static std::list <CharModelInstance*> g_listPendingBodyTextures;

		/** Compose pending cartoon faces in a batch. Possibly after rendering all other objects. It uses the same effect file as the GUI.
		* when this function returns the all pending cartoon faces are cleared from the list even if some are not updated.
		* @param nMaxNumber: max number of cartoon faces to be processed in this frame.
		*/
		static void ProcessPendingTextures(int nMaxNumber = 20);

		/** add an instance to the global list */
		static void AddPendingCartoonFace(CharModelInstance* pendingInstance);
		static bool HasPendingCartoonFace(CharModelInstance* pendingInstance);
		static bool RemovePendingCartoonFace(CharModelInstance* pendingInstance);

		/** add an instance to the global list */
		static void AddPendingBodyTexture(CharModelInstance* pendingInstance);
		static bool HasPendingBodyTexture(CharModelInstance* pendingInstance);
		static bool RemovePendingBodyTexture(CharModelInstance* pendingInstance);


		/**
		* the model will be rendered with the character's upper body turning m_fUpperBodyFacing rads
		* this is done by rotating (-m_fUpperBodyFacing) of the model origin, than rotate the upper body
		* by m_fUpperBodyFacing rads. This will allow the rendered character to be always facing forwards.
		* Note: this is only valid for FULL character with predefined bone structures.
		* TODO: in future version, bone information for body turning will be read from the ParaX file format.
		*/
		void SetUpperBodyTurningAngle(float fAngle);
		/** return the angle which will be used for rendering in the next frame. */
		float GetCurrrentUpperBodyTurningAngle();

		void SetUpperBodyUpdownAngle(float fAngle);
		/** return the angle which will be used for rendering in the next frame. */
		float GetCurrrentUpperBodyUpdownAngle();

		/** set the replaceable skin according to the skin database.
		* this only applies to non-customizable characters.if the index exceeds, it will use the default one.
		* the default skin is at index 0.
		* @param nIndex: the skin index.
		*/
		void SetSkin(int nIndex = 0);

		/** return the current skin. */
		int GetSkin();


		/** the color mask to be applied to cartoon face and ccs base layer. It can be used to fine tune skin color
		* on top of exiting base skin and face textures. Default value is 0xffffffff. setting this to a different value
		* will degrade the performance a little.
		*/
		void SetSkinColorMask(DWORD dwColor);

		/** the color mask to be applied to cartoon face and ccs base layer. It can be used to fine tune skin color
		* on top of exiting base skin and face textures. Default value is 0xffffffff. setting this to a different value
		* will degrade the performance a little.
		*/
		DWORD GetSkinColorMask();

		/**
		* check whether this character is customizable.
		*/
		bool IsCustomModel();

		/** set the model or geoset ID of a specified character slot.
		* @param nSlotID:
		* @param nItemID: the ID of the item to be put into the character slot. The default value for all slots is 0. One
		* may empty a certain slots by setting its nItemID to 0.*/
		void SetCharacterSlot(int nSlotID, int nItemID);

		/** return the item id at the given slot_id*/
		int  GetCharacterSlot(int nSlotID);

		/**
		* Set the character body parameters.
		* Need to call RefreshModel() after finished with the settings.
		* All body parameters are integer ranging from 0 to maxType, where maxType is the maximum number of types of
		* of a certain body parameter. For each body parameter, one can specify -1 to retain its current value. This
		* is useful, when the caller only wants to change one or several of the body parameters. The default body parameters
		* is (0,0,0,0,0).
		*/
		void SetBodyParams(int skinColor, int faceType, int hairColor, int hairStyle, int facialHair);

		int GetBodyParams(int type);

		/** Set the display options for the character.
		*/
		void SetDisplayOptions(int bShowUnderwear, int bShowEars, int bShowHair);

		/**
		* Get the display option parameters.
		*/
		bool GetDisplayOptions(int type);

		/** given an item type, return the slot index to which it should be attached.
		* @param nItemType: the item type
		* @param nSlotIndex: [out] the slot index which will be filled with the slot index upon returning.
		* @return: true, if a mapping has been found.
		*/
		bool MapItemTypeToSlot(int nItemType, int& nSlotIndex);

		/** whether an item can be inserted in to a certain slot */
		bool CanItemInSlot(int nItemType, int nSlotIndex);

		/** refresh model resource entities according to the display information */
		void SetModifiedModel();
		/** refresh all attached items (equipment) */
		void SetModifiedEquipment();
		/** set both model and equipment modified. */
		void SetModified();

		/** update the base model according to a given equipment.
		* equipments on the base model is displayed as textures and visibility of some meshes.*/
		void UpdateBaseModelByEquipment(int slot, int itemnum, int layer, CharTexture &tex);

		/** refresh an item on the canvas */
		void RefreshItem(int slot);


		/** Animate ParaX model, so that it is ready to be drawn. The animation time (frame number) is set, the visibilities of geoset is set,
		* the model textures are set, and the bone matrices are calculated.
		* @return true if model is animation. this function may return false if the model is not loaded and no bones to animate.
		* Ususally, one should only call Draw() method when the return value is true.
		*/
		bool AnimateModel(SceneState * sceneState, const AnimIndex& CurrentFrame, const AnimIndex& NextAnim, const AnimIndex& BlendingAnim, float blendingFactor, IAttributeFields* pAnimInstance = NULL);

		/** draw the model instance.
		* Note: call AnimateModel() before calling this function.
		*/
		void Draw(SceneState * sceneState, CParameterBlock* materialParams = NULL);

		/** build the shadow volume, call AnimateModel() before calling this function.*/
		void BuildShadowVolume(SceneState * sceneState, ShadowVolume * pShadowVolume, LightParams* pLight, Matrix4* mxWorld);

		/** advance by a delta time*/
		void AdvanceTime(float fDeltaTime);

		/** init base model. The race and gender are determined by the model name */
		bool InitBaseModel(ParaXEntity * pModel);

		/** get the base model for the current character.*/
		ParaXEntity* GetBaseModel();
		/** get the Anim model for the current character.*/
		ParaXEntity* GetAnimModel();

		void reset();

		/** cast a magic effect by the effect ID.
		* @param nEffectID: effect ID in the effect database
		* if effect id is negative, the effect will be removed from the object.
		* @param nAttachmentID: to which part of the character, the effect model is attached.
		0	ATT_ID_SHIELD,
		1	ATT_ID_HAND_RIGHT,
		2	ATT_ID_HAND_LEFT,		-- default value
		5	ATT_ID_SHOULDER_RIGHT,
		6	ATT_ID_SHOULDER_LEFT,
		11	ATT_ID_HEAD,
		* @param nSlotID: the slot id of the effect, if there is already an effect with the same ID
		*	it will be replaced with this new one.
		*/
		void CastEffect(int nEffectID, int nAttachmentID = ATT_ID_HAND_LEFT, int nSlotID = -1);
		/**
		* @param pModelEntity the model to be attached. if NULL, the old model at the slot will be removed.
		* @param nAttachmentID to which part of the character, the effect model is attached.
		0	ATT_ID_SHIELD,
		1	ATT_ID_HAND_RIGHT,
		2	ATT_ID_HAND_LEFT,		-- default value
		5	ATT_ID_SHOULDER_RIGHT,
		6	ATT_ID_SHOULDER_LEFT,
		11	ATT_ID_HEAD,
		* @param nSlotID the slot id of the effect. default value is -1.  if there is already an effect with the same ID
		*	it will be replaced with this new one.
		*/
		void AddAttachment(ParaXEntity* pModelEntity, int nAttachmentID = ATT_ID_HAND_LEFT, int nSlotID = -1, float fScaling = 1.0, TextureEntity* pReplaceableTexture = NULL);
		void AddAttachment(MeshEntity* pModelEntity, int nAttachmentID = ATT_ID_HAND_LEFT, int nSlotID = -1, float fScaling = 1.0, TextureEntity* pReplaceableTexture = NULL);

		/** get attribute fields of the given attachment ID */
		IAttributeFields * GetAttachmentAttObj(int nAttachmentID);

		/**
		* get the current replaceable texture by its ID.
		* if no replaceable textures is set before, this will return the same result as GetNumReplaceableTextures().
		* @note: This function will cause the mesh entity to be initialized.
		* @param ReplaceableTextureID usually [0-32)
		*	generally speaking, replaceable ID 0 is used for general purpose replaceable texture, ID 1 is for user defined. ID 2 is for custom skins.
		* @return this may return NULL, if replaceable texture is not set before or ID is invalid.
		*/
		TextureEntity* GetReplaceableTexture(int ReplaceableTextureID);

		/**
		* set the replaceable texture at the given index with a new texture.
		* this function will succeed regardless whether the mesh is initialized. Hence it can be used at loading time.
		* because default instance of the mesh may use different replaceable texture set.
		* @param ReplaceableTextureID usually [0-32)
		*	generally speaking, replaceable ID 0 is used for general purpose replaceable texture, ID 1 is for user defined. ID 2 is for custom skins.
		* @param pTextureEntity The reference account of the texture entity will be automatically increased by one.
		* @return true if succeed. if ReplaceableTextureID exceed the total number of replaceable textures, this function will return false.
		*/
		bool  SetReplaceableTexture(int ReplaceableTextureID, TextureEntity* pTextureEntity);

		/** get the cartoon face associated with this object.
		* @param bCreateIfNotExist: if this is true, a new one will be created if the cartoon is never created before.
		*/
		CartoonFace* GetCartoonFace(bool bCreateIfNotExist = true);

		/** check whether the associated cartoon model supports cartoon face. */
		bool IsSupportCartoonFace();

	public:
		static bool IsStaticMeshAsset(const std::string& filename);

	private:
		/** make the replaceable texture.
		@param texfn: base texture, usually the default texture path stored in the ParaX file.
		@param skin: texture name which is assumed to be inside the same directory as the base texture path*/
		std::string makeSkinTexture(const char *texfn, const char *skin);

		/** make character item texture according to the following rules.
		- if name does not contains '/', the region parameter is used to find the parent directory.
		- if name contains '/', region parameter is ignored.
		- if name does not have file extensions, "dds" is used
		- if name ends with "_U", it is considered a uni-sex texture. and ".dds" is appended.
		- if name does not end with "_U", "_M", "_F", it will first search for a file with "_M" or "_F" appended (according to gender), if not found, it will append "_U".
		@param region: see enum CharRegions
		@param name: either of the following are valid. Please see above for more details.
		- "sleaves00_TU"  -- this is useful when we have two textures for different sex.
		- "sleaves00_TU_U" -- this is useful when we have a single texture for both sex.
		- "sleaves00_TU_M"
		- "sleaves00_TU_F"
		- "Item/TextureComponents/ArmUpperTexture/sleaves00_TU_U.dds" -- full path is used.
		- "Item/TextureComponents/ArmUpperTexture/sleaves00_TU_U" -- full path is used.
		*/
		std::string makeItemTexture(int region, const char *name);

		/**
		* similar to makeItemTexture, except that .x is appended instead of .dds.
		*/
		std::string makeItemModel(int region, const char *name);

		bool slotHasModel(int i);
		void adjustHairVisibility();
	
		/** the character database entity */
		//static DatabaseEntity* m_pDbCharacters;

		/** this function is internal and implementation of RefreshModel(). */
		void RefreshModel_imp();

		/** this function is internal and implementation of RefreshModel(). */
		void RefreshEquipment_imp();

		/** update geosets to model*/
		void UpdateGeosetsToModel(CParaXModel* pModel);
		void UpdateTexturesToModel(CParaXModel* pModel);

		/* check whether the model has custom geometry sets */
		bool HasCustomGeosets(CParaXModel* pModel);

	public:
		static const size_t NumItems = 8;

		enum CharSections
		{
			SkinType = 0,
			FaceType = 1,
			FacialHairType = 2,
			HairType = 3,
			UnderwearType = 4
		};

		/** model resource entity*/
		enum {
			CHAR_TEX = 0,
			HAIR_TEX,
			FUR_TEX,
			CAPE_TEX,
			WING_TEX,
			CARTOON_FACE_TEX,
			NUM_TEX
		};
		asset_ptr<TextureEntity> m_textures[NUM_TEX];

		enum CharGeosets {
			CSET_HAIR = 0,
			CSET_FACIAL_HAIR1 = 1,
			CSET_FACIAL_HAIR2 = 2,
			CSET_FACIAL_HAIR3 = 3,
			CSET_GLOVES = 4,
			CSET_BOOTS = 5,
			CSET_EARS = 7,
			CSET_ARM_SLEEVES = 8,
			CSET_PANTS = 9,
			CSET_WINGS = 10, // newly added 2007.7.7
			CSET_TABARD = 12,
			CSET_ROBE = 13,
			CSET_SKIRT = 14,// newly added 2007.7.8
			CSET_CAPE = 15,
			// TOTAL
			NUM_CHAR_GEOSETS = 16
		};
		int geosets[NUM_CHAR_GEOSETS];

		/** a base model and its attachments.*/
		ref_ptr<ParaXModelCanvas> m_pModelCanvas;

		/* model display information */
		int nSetID; /// -1 stands for non-standard set

		int skinColor; // for customizable character
		int m_skinIndex; // for non-customizable character

		int faceType;
		int hairColor;
		int hairStyle;
		int facialHair;

		//int maxHairStyle, maxHairColor, maxSkinColor, maxFaceType, maxFacialHair;

		int race, gender;

		/** whether this is a customizable model.
		* As a rule, a customizable model's asset file is stored in a directory called "Character".
		*/
		bool m_bIsCustomModel : 1;
		/** whether the model has geosets that can be toggled. */
		bool m_bHasCustomGeosets : 1;

		bool showUnderwear, showEars, showHair, showFacialHair:1;
		bool hadRobe : 1;
		bool showAriesPantByItem, showAriesHandByItem, showAriesFootByItem, showAriesGlassByItem:1;
		bool autoHair : 1;

		int equipment[NUM_CHAR_SLOTS];

		/** the color mask to be applied to cartoon face and ccs base layer. It can be used to fine tune skin color
		* on top of exiting base skin and face textures. Default value is 0xffffffff. setting this to a different value
		* will degrade the performance a little.
		*/
		DWORD m_dwSkinColorMask;

		/** data structure for the cartoon face*/
		CartoonFace * m_cartoonFace;
		/** data structure for the body textures */
		CharTexture* m_bodyTexture;

		// temp character skin texture name
		std::string m_sSkinTextureName;

		// temp face texture name
		std::string m_sFaceTextureName;

		/** load the global characters.db database if it has not been loaded.*/
		//static void LoadCharacterDatabase();
		/** return the character database entity */
		//DatabaseEntity* GetCharDB();

		/* model pose information */
		float m_fUpperBodyTurningAngle;
		float m_fUpperBodyUpDownAngle;
	private:
		/** whether the char model needs to be refreshed. we only refresh when it is both modified and visible */
		bool m_bNeedRefreshModel;
		/** whether the equipment needs to be refreshed. we only refresh when it is both modified and visible */
		bool m_bNeedRefreshEquipment;
		/** whether we need to recompose the body texture. since the body texture component may be remote textures, there may be delay for body texture composing.
		* usually we need to recompose body texture, whenever char model is changed.
		*/
		bool m_bNeedComposeBodyTexture;
	};
}
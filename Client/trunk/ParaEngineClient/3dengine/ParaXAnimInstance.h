#pragma once
#include "AnimInstanceBase.h"

namespace ParaEngine
{
	class CParaXModel;
	class ParaXModelCanvas;
	class CharModelInstance;

	/** this is primary animation instance used for normal animated character or advanced customizable character
	* External animation are stored in dynamic fields using bone names as key.
	*/
	class CParaXAnimInstance : public CAnimInstanceBase
	{
	public:
		CParaXAnimInstance(void);
		virtual ~CParaXAnimInstance(void);

		ATTRIBUTE_DEFINE_CLASS(CParaXAnimInstance);

		/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);
		
		/** get attribute by child object. used to iterate across the attribute field hierarchy. */
		virtual IAttributeFields* GetChildAttributeObject(const std::string& sName);
		/** get the number of child objects (row count) in the given column. please note different columns can have different row count. */
		virtual int GetChildAttributeObjectCount(int nColumnIndex = 0);
		/** we support multi-dimensional child object. by default objects have only one column. */
		virtual int GetChildAttributeColumnCount();
		virtual IAttributeFields* GetChildAttributeObject(int nRowIndex, int nColumnIndex = 0);

		ATTRIBUTE_METHOD1(CParaXAnimInstance, GetIdleAnimationID_s, int*)		{ *p1 = cls->GetIdleAnimationID(); return S_OK; }
		ATTRIBUTE_METHOD1(CParaXAnimInstance, SetIdleAnimationID_s, int)		{ cls->SetIdleAnimationID(p1); return S_OK; }
		ATTRIBUTE_METHOD(CParaXAnimInstance, UpdateModel_s)		{ cls->UpdateModel(); return S_OK; }
		
	public:
		struct AttachmentMat
		{
			Matrix4 m_mat;
			int m_nRenderNumber;
		};
		typedef map<int, AttachmentMat> ATTACHMENT_MATRIX_POOL_TYPE;
		
		/** defines the model type that this instance holds*/
		enum ModelType{ 
			/** the character model is a fully customizable model for human shaped animation model
			* one can change the skin, attachments, clothings, hair and facial styles, etc */
			CharacterModel=0, 
			/** Fixed model is a model which is not customizable, but can contain a list of animations to play 
			* such as a flying flag, etc.*/
			FixedModel=1 
		};
	public:
		/** get the current local time in case it is animated in milli seconds frames. */
		virtual int GetTime();
		virtual void SetTime(int nTime);


		/** whether the character has a mount point at the given ID. Such characters are usually cars, horses, planes, etc. */
		bool HasMountPoint(int nMountPointID = 0);

		/** Get the character model instance. If the model instance is not of character type, 
		* then NULL will be returned. But it does not mean that there is not a valid model instance of other types */
		CharModelInstance * GetCharModel();
		/** get the animation model*/
		ParaXEntity* GetAnimModel();
		/** init the animation instance. associate it with the ParaX model*/
		void Init(ParaXEntity * pModel);

		/** reset the base model */
		bool ResetBaseModel(ParaXEntity * pModel);
				
		/** reset the animation and blending animation. this function must be called when the base model (CharModelInstance) is changed. 
		*/
		virtual void ResetAnimation();

		/** Render the model with its current settings. */
		virtual HRESULT Draw( SceneState * sceneState, const Matrix4* mxWorld,CParameterBlock* params=NULL);

		/** animate the model. One can call this function multiple times in a single render frame, but only the first time is calculated.  
		* this function will call AdvanceTime internally. 
		*/
		virtual void Animate( double dTimeDelta, int nRenderNumber=0 );


		/** update world transform by applying local size scale. 
		* return true if succeed
		*/
		virtual bool UpdateWorldTransform(SceneState * sceneState, Matrix4& out, const Matrix4& mxWorld);

		/** update model according to current animation data and time*/
		bool UpdateModel(SceneState * sceneState = NULL);

		/**
		* @return the current idle animation ID. default value is 0
		*/
		int GetIdleAnimationID();
		/**
		* the current idle animation ID. default value is 0
		* @param nID 
		*/
		void SetIdleAnimationID(int nID);
		
		/** shadow volume */
		virtual void BuildShadowVolume(SceneState * sceneState,ShadowVolume * pShadowVolume, LightParams* pLight, Matrix4* mxWorld);

		/** Advance the animation by a time delta in second */
		virtual void AdvanceTime( double dTimeDelta );
		/** Get the specified attachment matrix of the current model.
		* this is usually for getting the mount point on a certain model, such as horses.
		* @param nRenderNumber: if it is bigger than current calculated render number, the value will be recalculated. If 0, it will not recalculate
		* @return: NULL if not successful, otherwise it is pOut.
		*/
		Matrix4* GetAttachmentMatrix(Matrix4* pOut, int nAttachmentID, int nRenderNumber);

		/** Get the specified attachment matrix of the current model.
		* this is usually for getting the mount point on a certain model, such as horses.
		* @param nAttachmentID: NULL if not successful, otherwise it is pOut.
		*/
		bool HasAttachmentMatrix(int nAttachmentID);

		/**
		* Play or append the specified animation.
		* If the current animation is the same as the specified animation, it will loop on the 
		* current animation infinitely until a different animation is specified; otherwise,
		* the current animation will be stopped, the specified animation played and the next animation emptied.
		* @param fSpeed: [out] the speed of the animation after applying scaling is returned.
		* @param bAppend: if true, the new animation will be appended to the current animation according to the following rules:
		*		- if the current animation is non-looping, the new animation will be played after the end of the current animation.
		*		- if the current animation is looping, the new animation will be played immediately.
		*/
		virtual void LoadAnimation(const char * sName, float * fSpeed,bool bAppend = false);
		virtual void LoadAnimation(int nAnimID, float * fSpeed,bool bAppend = false);
		virtual void LoadAnimationByIndex(const AnimIndex& nAnimIndex, bool bAppend = false);
		virtual void LoadDefaultStandAnim(float * fSpeed);
		virtual void LoadDefaultWalkAnim(float * fSpeed);

		/** check to see if the underlying model has a given animation id. */
		virtual bool HasAnimId(int nAnimID);

		/** get the ID of the current animation.
		* 0 is default standing animation.*/
		virtual int GetCurrentAnimation();
		/// normally this will read the radius of the current animation
		/// and calculate the correct size after scaling
		virtual void GetCurrentRadius(float* fRadius);
		/// normally this will read the radius of the current animation
		/// and calculate the correct size after scaling
		virtual void GetCurrentSize(float * fWidth, float * fDepth);
		/// normally this will read the move speed of the current animation
		/// and calculate the correct(scaled) speed
		virtual void GetCurrentSpeed (float* fSpeed);
		/// normally this will read the move speed of the specified animation
		/// and calculate the correct(scaled) speed
		virtual void GetSpeedOf(const char * sName, float * fSpeed);

		/** whether to use global time to sync animation. Default to false. 
		if true, all characters plays exactly the same frames if they are using the same animation file at all times, 
		if false, each character advances its time frame separately according to their visibility in the scene. */
		virtual void SetUseGlobalTime(bool bUseGlobalTime);

		/** whether to use global time to sync animation. Default to false. 
		if true, all characters plays exactly the same frames if they are using the same animation file at all times, 
		if false, each character advances its time frame separately according to their visibility in the scene. */
		virtual bool IsUseGlobalTime();

		/** set the current animation frame number relative to the beginning of current animation. 
		* @param nFrame: 0 means beginning. if nFrame is longer than the current animation length, it will wrap (modulate the length).
		*/
		virtual void SetAnimFrame(int nFrame);

		/** get the current animation frame number relative to the beginning of current animation.  */
		virtual int GetAnimFrame();

		/** set the blending factor between the current and blend animation. 
		* @param fBlendingFactor: from [0,1]. where 0 means no blending. 
		*/
		virtual void SetBlendingFactor(float fBlendingFactor);

		/** whether an animation id exist. this function may have different return value when asset is async loaded. 
		* @param nAnimID: predefined id.
		*/
		virtual bool HasAnimation(int nAnimID);

		/** return true if the object contains alpha blended render pass. This function determines which render pipeline stage the object is rendered.
		* generally speaking, if deferred shading is used, we will render alpha blended objects last.
		*/
		virtual bool HasAlphaBlendedObjects();
		/** look for a valid animation ID for nAnimID. return -1 if no animation is found. e.g. if nAnimId 4 not found, 0 will be returned. */
		virtual int GetValidAnimID(int nAnimID);
		
	public:
		/** current animation index, this is different from sequence ID
		* an absolute ParaX frame number denoting the current animation frame. It is always within
		* the range of the current animation sequence's start and end frame number. */
		AnimIndex m_CurrentAnim;

		/** the next animation index.if it is -1, the next animation will
		* depends on the loop property of the current sequenc; otherwise, the animation specified
		* by the ID will be played next, after the current animation reached the end. */
		AnimIndex m_NextAnim;

		/** the animation sequence with which the current animation should be blended.
		* an absolute ParaX frame number denoting the blending animation frame. It is always within
		* the range of the blending animation sequence's start and end frame number. */
		AnimIndex m_BlendingAnim;

		/** by how much the blending frame should be blended with the current frame.
		* 1.0 will use solely the blending frame, whereas 0.0 will use only the current frame.
		* [0,1), blendingFrame*(blendingFactor)+(1-blendingFactor)*currentFrame */
		float m_fBlendingFactor;

		/** the current idle animation ID. [1] is largest alternative standing animation ID. if 0, it means not available.*/
		unsigned short m_nCurrentIdleAnimationID;

		/** the number of custom animations. if -1, it is undefined. if 0, it means there is no such animations. custom standing animations are at ANIM_CUSTOM0 - ANIM_CUSTOM3. */
		short m_nCustomStandingAnimCount;

		/** whether to use global time to sync animation. Default to false.
		if true, all characters plays exactly the same frames if they are using the same animation file at all times,
		if false, each character advances its time frame separately according to their visibility in the scene. */
		bool m_bUseGlobalTime;

		/** current time for dynamic fields. */
		int m_curTime;
	private:
		/// the type of the model 
		ModelType m_modelType;
		ref_ptr<CharModelInstance> m_pCharModel;

		/** pre-calculated attachment matrix that can be used in scene rendering at a later time.
		* mapping from attachment ID to matrix. These matrix will be calculated on first use and when the character is drawn on to the screen. */
		ATTACHMENT_MATRIX_POOL_TYPE m_AttachmentMatrices;
	};


	
}

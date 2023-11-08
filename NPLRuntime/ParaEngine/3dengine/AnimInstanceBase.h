#pragma once
#include "IAttributeFields.h"
#include <map>

namespace ParaEngine
{
	struct SceneState;
	class ShadowVolume;
	struct LightParams;
	using namespace std;

	/** it presents a given in bone animation providers or parax local model bone animation pools*/
	struct AnimIndex
	{
	public:
		AnimIndex() :nIndex(-1), Provider(0), nCurrentFrame(0), nStartFrame(0), nEndFrame(0), loopType(0), nAnimID(0), nFramePlayed(0) {}
		AnimIndex(int nIndex_) :nIndex(nIndex_), Provider(0), nCurrentFrame(0), nStartFrame(0), nEndFrame(0), loopType(0), nAnimID(0), nFramePlayed(0) {}
		AnimIndex(int nIndex_, unsigned char nProvider_) :nIndex(nIndex_), Provider(nProvider_), nCurrentFrame(0), nStartFrame(0), nEndFrame(0), loopType(0), nAnimID(0), nFramePlayed(0) {}
		AnimIndex(int nIndex_, unsigned char nProvider_, int nStartFrame_, int nEndFrame_, unsigned char loopType_) :nIndex(nIndex_), Provider(nProvider_), nCurrentFrame(nStartFrame_), nStartFrame(nStartFrame_), nEndFrame(nEndFrame_), loopType(loopType_), nAnimID(0), nFramePlayed(0) {}
		AnimIndex(int nIndex_, unsigned char nProvider_, int nStartFrame_, int nEndFrame_, unsigned char loopType_, int nAnimID) :nIndex(nIndex_), Provider(nProvider_), nCurrentFrame(nStartFrame_), nStartFrame(nStartFrame_), nEndFrame(nEndFrame_), loopType(loopType_), nAnimID(nAnimID), nFramePlayed(0) {}

		/** whether index is valid*/
		bool IsValid() const { return nIndex != -1; }

		/** two index are considered equal if nIndex and Provider are the same.*/
		bool operator==(const AnimIndex& right) const { return (this->nIndex == right.nIndex) && (this->Provider == right.Provider) && (this->loopType == right.loopType); }
		bool operator!=(const AnimIndex& right) const { return (this->nIndex != right.nIndex) || (this->Provider != right.Provider) || ((this->loopType != right.loopType)); }

		/** whether use looping */
		bool IsLooping() const { return loopType == 0; };

		/** if animation is undetermined, we should refresh this structure from asset, possibly because the animation is not loaded previously. */
		bool IsUndetermined() const { return loopType == 2; };

		/** set the current frame to the start frame*/
		void Reset() { nCurrentFrame = nStartFrame; nFramePlayed = 0; };

		/** make this instance invalid.*/
		void MakeInvalid() { nIndex = -1; Provider = 0; };

		/** add total frames to nFramePlayed.*/
		void AddCycle() { nFramePlayed += (nEndFrame - nStartFrame); }
	public:
		// bone animation provider: 0 for local model bone animation pool; 1 for global bone animation provider
		unsigned char Provider;
		// 0 for looping, 1 for non-looping, 2 for undeterminted(the animation needs to be refreshed upon finish, such as during async-loading). 
		unsigned char loopType;
		// current index, -1 if invalid
		int nIndex;
		// the animation ID as seen by outsiders
		int nAnimID;
		// current frame
		int nCurrentFrame;
		// start frame
		int nStartFrame;
		// end frame
		int nEndFrame;
		// total number of frames played
		int nFramePlayed;
	};

	/**
	abstract class for all animation instance implementation.
	so that there can be several implementations such as MDX animation and X file animation support.
	*/
	class CAnimInstanceBase : public IAttributeFields
	{
	public:
		CAnimInstanceBase();
		virtual ~CAnimInstanceBase() {};

		ATTRIBUTE_DEFINE_CLASS(CAnimInstanceBase);

		/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);

		ATTRIBUTE_METHOD1(CAnimInstanceBase, GetRenderCount_s, int*) { *p1 = cls->GetRenderCount(); return S_OK; }
		ATTRIBUTE_METHOD1(CAnimInstanceBase, SetRenderCount_s, int) { cls->SetRenderCount(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CAnimInstanceBase, IsAnimIDMapEnabled_s, bool*) { *p1 = cls->IsAnimIDMapEnabled(); return S_OK; }
		ATTRIBUTE_METHOD1(CAnimInstanceBase, EnableAnimIDMap_s, bool) { cls->EnableAnimIDMap(p1); return S_OK; }

		ATTRIBUTE_METHOD(CAnimInstanceBase, ResetAnimation_s) { cls->ResetAnimation(); return S_OK; }

		ATTRIBUTE_METHOD1(CAnimInstanceBase, GetCurrentAnimation_s, int*) { *p1 = cls->GetCurrentAnimation(); return S_OK; }

		ATTRIBUTE_METHOD1(CAnimInstanceBase, ShowMesh_s, DWORD) { cls->ShowMesh(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CAnimInstanceBase, GetSizeScale_s, float*) { *p1 = cls->GetSizeScale(); return S_OK; }
		ATTRIBUTE_METHOD1(CAnimInstanceBase, SetSizeScale_s, float) { cls->SetSizeScale(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CAnimInstanceBase, GetSpeedScale_s, float*) { *p1 = cls->GetSpeedScale(); return S_OK; }
		ATTRIBUTE_METHOD1(CAnimInstanceBase, SetSpeedScale_s, float) { cls->SetSpeedScale(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CAnimInstanceBase, IsUseGlobalTime_s, bool*) { *p1 = cls->IsUseGlobalTime(); return S_OK; }
		ATTRIBUTE_METHOD1(CAnimInstanceBase, SetUseGlobalTime_s, bool) { cls->SetUseGlobalTime(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CAnimInstanceBase, GetAnimFrame_s, int*) { *p1 = cls->GetAnimFrame(); return S_OK; }
		ATTRIBUTE_METHOD1(CAnimInstanceBase, SetAnimFrame_s, int) { cls->SetAnimFrame(p1); return S_OK; }

		ATTRIBUTE_METHOD1(CAnimInstanceBase, IsAnimationEnabled_s, bool*) { *p1 = cls->IsAnimationEnabled(); return S_OK; }
		ATTRIBUTE_METHOD1(CAnimInstanceBase, EnableAnimation_s, bool) { cls->EnableAnimation(p1); return S_OK; }

	protected:
		// object status primitive
		/// color of this model
		LinearColor			m_modelColor;
		//bool				m_bLoopAnim;
		//DWORD				m_dwLoiterIndex;

		/// scale parameters
		float	m_fSizeScale;
		float 	m_fSpeedScale;
		/** the last render frame count that this object is rendered. */
		int m_nRenderCount;

		/**
		* a mapping from one animation ID to another ID. This mapping is usually not used (empty). However, it is used when we can
		* want to secretly replace animation used by this animation instance, by adding a ID to ID map.
		* e.g. we can replace female walk animation ID with an external female walk animation ID.
		*/
		map<int, int> m_mapAnimIDs;

		/** whether the m_mapAnimIDs will be used. Disabled by default*/
		bool m_bEnableAnimIDMap;
		/** whether animation is enabled, true by default. */
		bool m_bEnableAnimation;

	public:
		/** this function is called to update the render frame number of this object.
		* please note that, the frame number increases by 1 every frame. This function is called inside the Draw() function
		* to prevent the same animation to be advanced multiple times in a single render frame.
		*/
		virtual int GetRenderCount();
		virtual void SetRenderCount(int nCount);

		/**
		* set whether the m_mapAnimIDs will be used. Disabled by default
		*/
		void EnableAnimIDMap(bool bEnable);

		/**
		* get whether the m_mapAnimIDs will be used.Disabled by default
		*/
		bool IsAnimIDMapEnabled();

		/** one can use the returned map to set and get the animation ID mapping.
		* animation ID mapping is a mapping from one animation ID to another ID. This mapping is usually not used (empty). However, it is used when we can
		* want to secretly replace animation used by this animation instance, by adding a ID to ID map.
		* e.g. we can replace female walk animation ID with an external female walk animation ID.
		* @note: remember to EnableAnimIDMap() in order for the mapping to take effect.
		*/
		map<int, int>* GetAnimIDMap();

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
		virtual void LoadAnimation(const char* sName, float* fSpeed, bool bAppend = false);
		virtual void LoadAnimation(int nAnimID, float* fSpeed, bool bAppend = false);
		virtual void LoadDefaultStandAnim(float* fSpeed);
		virtual void LoadDefaultWalkAnim(float* fSpeed);

		/** check to see if the underlying model has a given animation id. */
		virtual bool HasAnimId(int nAnimID);

		/** get the ID of the current animation.
		* 0 is default standing animation.*/
		virtual int GetCurrentAnimation();

		/** reset the animation and blending animation. this function must be called when the base model (CharModelInstance) is changed.
		*/
		virtual void ResetAnimation();

		/// set model color, different race might have different color
		virtual void SetModelColor(LinearColor modelColor);
		/// a model might has several meshes, you can show or hide them by setting
		/// the bit field of a DWORD. the lowest bit is the first mesh.
		virtual void ShowMesh(BOOL bShow, int nIndex);
		virtual void ShowMesh(DWORD dwBitfields);

		/// Draw() method will call this function automatically
		virtual void AdvanceTime(double dTimeDelta);

		/** animate the model. One can call this function multiple times in a single render frame, but only the first time is calculated.
		* this function will call AdvanceTime internally.
		*/
		virtual void Animate(double dTimeDelta, int nRenderNumber = 0);

		//Add third parameter, pass per instance material parameters   --clayman 2012.7.4
		/** simply call this function to draw and advance time. set dTimeDelta to 0 if you want to render only static model.*/
		virtual HRESULT Draw(SceneState* sceneState, const Matrix4* mxWorld, CParameterBlock* paramBlock = NULL);

		virtual void BuildShadowVolume(SceneState* sceneState, ShadowVolume* pShadowVolume, LightParams* pLight, Matrix4* mxWorld);

		/* new set */
		/// get the size scale scale normally this is 1.0f
		virtual void SetSizeScale(float fScale);
		/** get the size scale*/
		virtual float GetSizeScale();

		/// normally this will read the radius of the current animation
		/// and calculate the correct size after scaling
		virtual void GetCurrentRadius(float* fRadius) {};
		/// normally this will read the radius of the current animation
		/// and calculate the correct size after scaling
		virtual void GetCurrentSize(float* fWidth, float* fDepth);
		/// set the scale factor that will be pre-multiplied  to the loaded model and animation
		/// the animation speed is changed, one should later call GetCurrentSpeed to update 
		/// the animation's displacement speed.
		virtual void SetSpeedScale(float fScale);
		/**
		* Get the scale factor that will be pre-multiplied  to the loaded model and animation
		* @return : speed scale. 1.0 is the original speed.
		*/
		virtual float GetSpeedScale();
		/// normally this will read the move speed of the current animation
		/// and calculate the correct(scaled) speed
		virtual void GetCurrentSpeed(float* fSpeed);
		/// normally this will read the move speed of the specified animation
		/// and calculate the correct(scaled) speed
		virtual void GetSpeedOf(const char* sName, float* fSpeed);

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

		/** set the current animation frame number relative to the beginning of current animation.
		* @param nFrame: 0 means beginning. if nFrame is longer than the current animation length, it will wrap (modulate the length).
		*/
		virtual void SetAnimFrame(int nFrame);

		/** get the current animation frame number relative to the beginning of current animation.  */
		virtual int GetAnimFrame();

		/** set the blending factor between the current and blend animation.
		* @param bBlendingFactor: from [0,1]. where 0 means no blending.
		*/
		virtual void SetBlendingFactor(float fBlendingFactor);

		/** whether an animation id exist*/
		virtual bool HasAnimation(int nAnimID);

		/** return true if the object contains alpha blended render pass. This function determines which render pipeline stage the object is rendered.
		* generally speaking, if deferred shading is used, we will render alpha blended objects last.
		*/
		virtual bool HasAlphaBlendedObjects();

		/** look for a valid animation ID for nAnimID. return -1 if no animation is found. e.g. if nAnimId 4 not found, 0 will be returned. */
		virtual int GetValidAnimID(int nAnimID);

		/** whether animation is enabled. by default this is true. During movie editing, we may disable animation, set animation frame explicitly by editor logics. */
		void EnableAnimation(bool bAnimated);
		bool IsAnimationEnabled() const;

		virtual void SetUpperAnimation(int nAnimID) {}
		virtual int GetUpperAnimation() { return -1; }
	};
}

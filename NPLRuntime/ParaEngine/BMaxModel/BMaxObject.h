#pragma once
#include "MeshEntity.h"
#include "ParaXEntity.h"
#include "TileObject.h"
namespace ParaEngine
{
	class CParaXModel;
	class BMaxModel;
	struct SceneState;
	struct IParaPhysicsActor;
	class CPhysicsWorld;

	/* render with color and material. */
	class BMaxObject : public CTileObject
	{
	public:
		BMaxObject(void);
		virtual ~BMaxObject(void);

		ATTRIBUTE_DEFINE_CLASS(BMaxObject);
		ATTRIBUTE_SUPPORT_CREATE_FACTORY(BMaxObject);

		/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);

		/** get the number of child objects (row count) in the given column. please note different columns can have different row count. */
		virtual int GetChildAttributeObjectCount(int nColumnIndex = 0);
		/** we support multi-dimensional child object. by default objects have only one column. */
		virtual int GetChildAttributeColumnCount();
		virtual IAttributeFields* GetChildAttributeObject(int nRowIndex, int nColumnIndex = 0);

		ATTRIBUTE_METHOD(BMaxObject, UpdateModel_s) { cls->UpdateModel(); return S_OK; }
	public:
		IAttributeFields* GetAnimInstanceFields();

		virtual HRESULT Draw(SceneState * sceneState);

		void ApplyBlockLighting(SceneState * sceneState);

		virtual AssetEntity* GetPrimaryAsset();
		virtual void SetAssetFileName(const std::string& sFilename);

		virtual Matrix4* GetAttachmentMatrix(Matrix4& pOut, int nAttachmentID = 0, int nRenderNumber = 0);

		virtual void GetLocalTransform(Matrix4* localTransform);
		virtual void UpdateGeometry();

		/** set the scale of the object. This function takes effects on both character object and mesh object.
		* @param s: scaling applied to all axis.1.0 means original size. */
		virtual void SetScaling(float s);

		/** get the scaling. */
		virtual float GetScaling();

		/** rotation related */
		virtual float GetPitch();
		virtual void SetPitch(float fValue);

 		virtual float GetRoll();
		virtual void SetRoll(float fValue);

		/**
		* return the world matrix of the object for rendering
		* @param out: the output.
		* @param nRenderNumber: if it is bigger than current calculated render number, the value will be recalculated. If 0, it will not recalculate
		* @return: same as out. or NULL if not exists.
		*/
		virtual Matrix4* GetRenderMatrix(Matrix4& out, int nRenderNumber = 0);
		
		/** if the object may contain physics*/
		virtual bool CanHasPhysics();
		virtual void LoadPhysics();
		/** by default physics is lazy-load when player walk into its bounding box, setting this to false will always load the physics.
		* Please note, one must EnablePhysics(true) before this one takes effect.
		*/
		virtual void SetAlwaysLoadPhysics(bool bEnable);
		virtual void UnloadPhysics();
		virtual void SetPhysicsGroup(int nGroup);
		virtual int GetPhysicsGroup();
		virtual void EnablePhysics(bool bEnable);
		virtual bool IsPhysicsEnabled();
		virtual TextureEntity* GetReplaceableTexture(int ReplaceableTextureID)override;
		virtual bool  SetReplaceableTexture(int ReplaceableTextureID,TextureEntity* pTextureEntity)override;

		/** whether animation is enabled. by default this is true. During movie editing, we may disable animation, set animation frame explicitly by editor logics. */
		virtual void EnableAnim(bool bAnimated);
		virtual bool IsAnimEnabled();

		/** get the current local time in case it is animated in milli seconds frames. */
		virtual int GetTime();
		virtual void SetTime(int nTime);

		/** set the current animation frame number relative to the beginning of current animation.
		* @param nFrame: 0 means beginning. if nFrame is longer than the current animation length, it will wrap (modulate the length).
		*/
		virtual void SetAnimFrame(int nFrame);

		/** get the current animation frame number relative to the beginning of current animation.  */
		virtual int GetAnimFrame();

		/** get the number of physics actors. If physics is not loaded, the returned value is 0. */
		int GetStaticActorCount();
		
		/** update model according to current animation data and time*/
		bool UpdateModel(SceneState * sceneState = NULL);

		/** ignore selected state for bmaxobject
		virtual void OnSelect(int nGroupID)override{}
		virtual void OnDeSelect()override{}
		*/

	private:
		/** size scale */
		float	m_fScale;

		/** rotation parameters */
		float m_fPitch;
        float m_fRoll;

		AnimIndex m_CurrentAnim;
		ref_ptr<ParaXEntity>      m_pAnimatedMesh;

		/** all static physics actors in physics engine */ 
		vector<IParaPhysicsActor*> m_staticActors;

		// any bit wise combination of PHYSICS_METHOD
		DWORD m_dwPhysicsMethod;
		uint32 m_nPhysicsGroup;

		/** a value between [0,1). last block light. */
		float m_fLastBlockLight;
		/** a hash to detect if the containing block position of this biped changed. */
		DWORD m_dwLastBlockHash;

		/** current time for dynamic fields. */
		int m_curTime;
		/** whether to enable animation in asset file. */
		bool m_bEnableAnim;

		std::map<uint32, TextureEntity*> mReplaceTextures;
	};
}

#pragma once
#include "animated.h"
#include "IAttributeFields.h"

namespace ParaEngine
{
	class CBVHSerializer;
	class CBoneAnimProvider;

	/** a single animated bone,  it contains both the bone instance data and all animation data of the bone. 
	There are three ways to calculate final bone matrix. 
	1. using each bone's pivot point and SRT transforms relative to the current bone
	2. using offset matrix (which transform from mesh to local bone space) and SRT transforms relative to the parent bone
	3. the bone has no animation and a static transform is used to transform from current bone space to its parent bone space. 
	*/
	class Bone : public IAttributeFields
	{
	public:
		Bone();
		virtual ~Bone();

		ATTRIBUTE_DEFINE_CLASS(Bone);

		/** this class should be implemented if one wants to add new attribute. This function is always called internally.*/
		virtual int InstallFields(CAttributeClass* pClass, bool bOverride);
		
		ATTRIBUTE_METHOD1(Bone, GetTag_s, const char**) { *p1 = cls->GetTag().c_str(); return S_OK; }
		ATTRIBUTE_METHOD1(Bone, GetRotName_s, const char**)		{ *p1 = cls->GetRotName().c_str(); return S_OK; }
		ATTRIBUTE_METHOD1(Bone, GetTransName_s, const char**)		{ *p1 = cls->GetTransName().c_str(); return S_OK; }
		ATTRIBUTE_METHOD1(Bone, GetScaleName_s, const char**)		{ *p1 = cls->GetScaleName().c_str(); return S_OK; }
		ATTRIBUTE_METHOD1(Bone, GetTimeName_s, const char**) { *p1 = cls->GetTimeName().c_str(); return S_OK; }

		ATTRIBUTE_METHOD1(Bone, IsBillBoarded_s, bool*)		{ *p1 = cls->IsBillBoarded(); return S_OK; }
		ATTRIBUTE_METHOD1(Bone, IsPivotBone_s, bool*)		{ *p1 = cls->IsPivotBone(); return S_OK; }
		ATTRIBUTE_METHOD1(Bone, IsOffsetMatrixBone_s, bool*){ *p1 = cls->IsOffsetMatrixBone(); return S_OK; }
		ATTRIBUTE_METHOD1(Bone, IsTransformationNode_s, bool*)	{ *p1 = cls->IsTransformationNode(); return S_OK; }
		ATTRIBUTE_METHOD1(Bone, IsStaticTransform_s, bool*) { *p1 = cls->IsStaticTransform(); return S_OK; }
		ATTRIBUTE_METHOD1(Bone, IsDummyNode_s, bool*) { *p1 = cls->IsDummyNode(); return S_OK; }
		ATTRIBUTE_METHOD1(Bone, IsAnimated_s, bool*)		{ *p1 = cls->IsAnimated(); return S_OK; }

		ATTRIBUTE_METHOD1(Bone, SetOffsetMatrix_s, const Matrix4&)		{ cls->SetOffsetMatrix(p1); return S_OK; }
		ATTRIBUTE_METHOD1(Bone, SetStaticTransform_s, const Matrix4&)		{ cls->SetStaticTransform(p1); return S_OK; }
		
		ATTRIBUTE_METHOD(Bone, RemoveRedundentKeys_s)		{ cls->RemoveRedundentKeys(); return S_OK; }
		
		ATTRIBUTE_METHOD1(Bone, GetFinalMatrix_s, Matrix4*)		{ *p1 = cls->GetFinalMatrix(); return S_OK; }
		ATTRIBUTE_METHOD1(Bone, GetFinalRotMatrix_s, Matrix4*)		{ *p1 = cls->GetFinalRotMatrix(); return S_OK; }
		ATTRIBUTE_METHOD1(Bone, GetPivotRotMatrix_s, Matrix4*)		{ *p1 = cls->GetPivotRotMatrix(); return S_OK; }
		
		ATTRIBUTE_METHOD1(Bone, GetPivotPoint_s, Vector3*)		{ *p1 = cls->GetPivotPoint(); return S_OK; }
		ATTRIBUTE_METHOD1(Bone, GetAnimatedPivotPoint_s, Vector3*)		{ *p1 = cls->GetAnimatedPivotPoint(); return S_OK; }

		ATTRIBUTE_METHOD1(Bone, SetFinalRot_s, Quaternion)		{ cls->SetFinalRot(p1); return S_OK; }
		ATTRIBUTE_METHOD1(Bone, GetFinalRot_s, Quaternion*)		{ *p1 = cls->GetFinalRot(); return S_OK; }

		ATTRIBUTE_METHOD1(Bone, SetFinalTrans_s, Vector3)		{ cls->SetFinalTrans(p1); return S_OK; }
		ATTRIBUTE_METHOD1(Bone, GetFinalTrans_s, Vector3*)		{ *p1 = cls->GetFinalTrans(); return S_OK; }

		ATTRIBUTE_METHOD1(Bone, SetFinalScaling_s, Vector3)		{ cls->SetFinalScaling(p1); return S_OK; }
		ATTRIBUTE_METHOD1(Bone, GetFinalScaling_s, Vector3*)		{ *p1 = cls->GetFinalScaling(); return S_OK; }

		ATTRIBUTE_METHOD1(Bone, GetParentIndex_s, int*)		{ *p1 = cls->GetParentIndex(); return S_OK; }
		ATTRIBUTE_METHOD1(Bone, GetBoneIndex_s, int*)		{ *p1 = cls->GetBoneIndex(); return S_OK; }
		
		ATTRIBUTE_METHOD1(Bone, GetBoneID_s, int*)		{ *p1 = cls->GetBoneID(); return S_OK; }
		ATTRIBUTE_METHOD1(Bone, SetBoneID_s, int)		{ cls->SetBoneID(p1); return S_OK; }
		
	public:
		/** get the name or identifier. */
		virtual const std::string& GetIdentifier();
		virtual void SetIdentifier(const std::string& sID);

		/** whether the bone is billboarded*/
		bool IsBillBoarded() const { return (flags & BONE_BILLBOARDED) != 0; };
		/** whether calculate bone matrix using each bone's pivot point and SRT transforms relative to the current bone.*/
		bool IsPivotBone() const { return (flags & BONE_OFFSET_MATRIX) == 0; };
		/** whether calculate bone matrix using offset matrix (which transform from mesh to local bone space) and SRT transforms relative to the parent bone */
		bool IsOffsetMatrixBone() const { return (flags & BONE_OFFSET_MATRIX) != 0; };
		/** whether the bone has no animation and a static transform is used to transform from current bone space to its parent bone space. */
		bool IsStaticTransform() const { return (flags & BONE_STATIC_TRANSFORM) != 0; };
		/** whether the bone is transformation node */
		bool IsTransformationNode() const { return (flags & BONE_TRANSFORMATION_NODE) != 0; };
		/** whether the bone and all of its parent bones are static and transformation node. Dummy node is only calculated once */
		bool IsDummyNode() const { return m_bIsDummyNode; };
		

		/** calling this function means that you want to use BONE_OFFSET_MATRIX for final bone matrix calculation. */
		void SetOffsetMatrix(const Matrix4& mat);
		
		/** calling this function means that you want to use BONE_STATIC_TRANSFORM for final bone matrix calculation. */
		void SetStaticTransform(const Matrix4& mat);

		/** whether the bone contains animation data. */
		bool CheckHasAnimation();

		/** whether the bone contains bone rotation, scaling or translation animation data. */
		inline bool HasBoneAnimation() {
			return rot.used || scale.used || trans.used || IsBillBoarded();
		}

		const std::string& GetName() const;
		void SetName(const std::string& val);

		/** automatically set bone id from bone name. */
		void AutoSetBoneInfoFromName();

		/** whether bone is animated. */
		bool IsAnimated();
		/**
		* calc bone matrix and all of its parent bones.
		* @param nCurrentAnim: current animation sequence ID
		* @param currentFrame: an absolute ParaX frame number denoting the current animation frame. It is always within
		* the range of the current animation sequence's start and end frame number.
		* @param nBlendingAnim: the animation sequence with which the current animation should be blended.
		* @param blendingFrame: an absolute ParaX frame number denoting the blending animation frame. It is always within
		* the range of the blending animation sequence's start and end frame number.
		* @param blendingFactor: by how much the blending frame should be blended with the current frame.
		* 1.0 will use solely the blending frame, whereas 0.0 will use only the current frame.
		* [0,1), blendingFrame*(blendingFactor)+(1-blendingFactor)*currentFrame
		* @param pAnimInstance: if specified, we will use rotation variable in the animation instance if any.
		*/
		bool calcMatrix(Bone* allbones, const AnimIndex & CurrentAnim, const AnimIndex & BlendingAnim, float blendingFactor, IAttributeFields* pAnimInstance = NULL);
		/** for static bones */
		void calcMatrix(Bone* allbones);

		/** get external rot quaternion from animation instance. 
		* @return true, if rotation is found, otherwise the one in the bone should be used. 
		*/
		bool GetExternalRot(IAttributeFields* pAnimInstance, Quaternion& outQuat);
		bool GetExternalTranslation(IAttributeFields* pAnimInstance, Vector3& outTrans);
		bool GetExternalScaling(IAttributeFields* pAnimInstance, Vector3& outScaling);

		/**  Always call this function after loading data from files.*/
		void RemoveRedundentKeys();

		/* rotation and offset matrix are processed */
		void PostCalculateBoneMatrix();
		
		const ParaEngine::Matrix4& GetFinalMatrix();
		/** return the pivot point in binding space */
		const Vector3& GetPivotPoint();

		/** return the current pivot point */
		Vector3 GetAnimatedPivotPoint();

		int GetParentIndex() const { return parent; }
		int GetBoneIndex() const { return nIndex; }
		int GetBoneID() const { return nBoneID; }
		int IsAttachment() const;
		/** return -1 if it is not attachment id. of it is non-negative attachment id */
		int GetAttachmentId() const;
		void SetBoneID(int val) { nBoneID = val; }

		const ParaEngine::Quaternion& GetFinalRot() const;
		void SetFinalRot(const ParaEngine::Quaternion& val);

		const Vector3 & GetFinalTrans() const;
		void SetFinalTrans(const Vector3 &val);

		const Vector3& GetFinalScaling() const;
		void SetFinalScaling(const Vector3& val);

		const ParaEngine::Matrix4& GetFinalRotMatrix() const;
		/** similar to GetFinalRotMatrix(), except that it will remove rotation in its offset matrix. */
		Matrix4 GetPivotRotMatrix();
		
		/** mark this bone as un-calculated bone. 
		* @param bForce: if false(default), Static and transformation node are never dirty. 
		*/
		void MakeDirty(bool bForce = false);

		friend class CBVHSerializer;

		Bone& operator=(const Bone& other);

		/** call this before save to disk to compress the size of the file, if it is loaded from FBX files */
		void RemoveUnusedAnimKeys();
	
		/** find the matching bone in the provider. */
		Bone* FindMatchingBoneInProvider(CBoneAnimProvider* pProvider);

	public:
		enum BONE_FLAGS
		{
			/** calculate bone matrix using each bone's pivot point and SRT transforms relative to the current bone.
			* offset matrix is not used.
			*/
			BONE_USE_PIVOT = 0,
			/** whether the bone is billboarded */
			BONE_BILLBOARDED = (0x1 << 3),
			/** calculate bone matrix using offset matrix (which transform from mesh to local bone space) and SRT transforms relative to the parent bone
			* pivot point is not used.
			*/
			BONE_OFFSET_MATRIX = (0x1 << 4),
			/** the bone has no animation and a static transform is used to transform from current bone space to its parent bone space.  */
			BONE_STATIC_TRANSFORM = (0x1 << 5),
			/* the bone is the transformation node */
			BONE_TRANSFORMATION_NODE = (0x1 << 6),
			/* the bone and all of its parent bones are static and tranformation node*/
			BONE_DUMMY_NODE = (0x1 << 7),
		};
		std::string	m_sIdentifer;
		std::string	m_sTag;

		std::string	m_sRotName;
		std::string	m_sTransName;
		std::string	m_sScaleName;
		std::string	m_sTimeName;
		
		const std::string& GetTag();
		const std::string& GetRotName();
		const std::string& GetTransName();
		const std::string& GetScaleName();
		const std::string& GetTimeName();

		Animated<Vector3> trans;
		Animated<Quaternion> rot;
		Animated<Vector3> scale;

		// pivot point in binding pos
		Vector3 pivot;
		
		// offset matrix that transforms from mesh space to the current bone space. 
		Matrix4 matOffset;
		// the bone has no animation and a static transform is used to transform from current bone space to its parent bone space. 
		Matrix4 matTransform;

		// index of the parent bone
		int parent;
		
		// the predefined bone ID. it is 0 for unknown bones, positive for known bones. one of the KNOWN_BONE_NODES
		int nBoneID;

		// index of this bone
		int nIndex;
		
		// bitwise fields of bone attributes. BONE_BILLBOARDED for bill boarded. 
		DWORD flags;
	
		// temporary final matrix
		Matrix4 mat;
		
		// temporary final rotation matrix
		Matrix4 mrot;
		
		// temporary final m_finalRot used in composing the final mat
		Quaternion m_finalRot;

		// temporary final m_finalTrans used in composing the final mat
		Vector3 m_finalTrans;

		// temporary final m_finalScaling used in composing the final mat
		Vector3 m_finalScaling;
		
		bool calc;
		bool bUsePivot;

		bool mIsUpper;
		bool m_bIsDummyNode;

		/** max number of bones per vertex, currently this is 4. */
		const static int s_MaxBonesPerVertex = 4;
	};
}

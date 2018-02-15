#pragma once

namespace ParaEngine
{
	class Bone;
	struct AnimIndex;

	// find calculate the middle joint.
	// pelvis(20)->waist(21)->chest(23)->necklow(43)->neck(61) 
	//           ->but(22)
	/** a chain of bones which will be rotated with some constraint.*/
	struct CBoneChain
	{
		struct BoneItem {
			int nBoneID;
			float fWeight;
		};
		BoneItem m_boneChain[4];
		int m_nBoneCount;
	public:
		CBoneChain(int bBoneCount = 4);;
		/** set the start bone index. We will therefore build a chain of bones by recursively get the parent bone.
		* please note that the first bone is the parent of nStart.
		* @param allBones:
		* @param nStart: the index of the start bone's child.Normally, nStart is the HEAD, so the bone chain will be
		*	NECK->SHOULDER->CHEST->WAIST
		*/
		void SetStartBone(Bone* allBones, int nStart, const int* m_boneLookup = NULL);
		/** rotate a chain of bones around the Y axis by a given radius.
		* @param allBones:
		* @param: nMaxBoneNum: the size of allBones
		* @param fAngle: in rads. */
		void RotateBoneChain(const Vector3& vAxis, Bone* allBones, int nMaxBoneNum, float fAngle, const AnimIndex& CurrentAnim, const AnimIndex& BlendingAnim, float blendingFactor, IAttributeFields* pAnimInstance = NULL);
	};
}
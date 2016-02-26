//-----------------------------------------------------------------------------
// Class:	BoneChain
// Authors:	Li, Xizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2005.10.8
// Revised: 2005.10.8, 2014.8
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ParaXModel.h"
#include "ParaXBone.h"
#include "BoneChain.h"

using namespace ParaEngine;

void ParaEngine::CBoneChain::RotateBoneChain(const Vector3& vAxis, Bone* allBones, int nMaxBoneNum, float fAngle, const AnimIndex& CurrentAnim, const AnimIndex& BlendingAnim, float blendingFactor, IAttributeFields* pAnimInstance /*= NULL*/)
{
	for (int i = 0; i < m_nBoneCount; i++)
	{
		int nBoneID = m_boneChain[i].nBoneID;
		if (nBoneID < nMaxBoneNum && nBoneID >= 0)
		{
			Bone & bone = allBones[nBoneID];
			bone.calcMatrix(allBones, CurrentAnim, BlendingAnim, blendingFactor, pAnimInstance);
			// just rotate one bone at most. 
			Matrix4 mAfterRot(Quaternion(vAxis, fAngle));
			if (bone.bUsePivot)
			{
				Matrix4 M;
				// use pivot point
				M.makeTrans(bone.pivot*-1.0f);
				mAfterRot = M.Multiply4x3(mAfterRot);
				mAfterRot.offsetTrans(bone.pivot);
			}
			bone.mat = mAfterRot.Multiply4x3(bone.mat);
			bone.mrot = mAfterRot.Multiply4x3(bone.mrot);
			break;
		}
	}
}

void ParaEngine::CBoneChain::SetStartBone(Bone* allBones, int nStart, const int* m_boneLookup /*= NULL*/)
{
	for (int i = m_nBoneCount - 1; i >= 0; i--)
	{
		if (nStart >= 0)
		{
			m_boneChain[i].nBoneID = nStart;
			nStart = allBones[nStart].parent; // get its parent
		}
		else
			m_boneChain[i].nBoneID = -1;
	}
}

ParaEngine::CBoneChain::CBoneChain(int bBoneCount /*= 4*/)
{
	m_nBoneCount = bBoneCount;
	float weight = 1.f / bBoneCount;
	for (int i = m_nBoneCount - 1; i >= 0; i--)
	{
		m_boneChain[i].nBoneID = -1;
		m_boneChain[i].fWeight = weight;
	}
}

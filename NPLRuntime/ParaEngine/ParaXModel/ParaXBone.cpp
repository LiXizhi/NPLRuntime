//-----------------------------------------------------------------------------
// Class:	ParaXBone
// Authors:	Li, Xizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2005.10.8
// Revised: 2005.10.8, 2014.8
// Note: some logics is based on the open source code of WOWMAPVIEW
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "FileManager.h"
#include "BoneAnimProvider.h"
#include "DynamicAttributeField.h"
#include "CustomCharCommon.h"
#include "ParaXModel.h"
#include "StringHelper.h"
#include "ParaXBone.h"

using namespace ParaEngine;

Bone::Bone()
	:bUsePivot(true), nBoneID(0), nIndex(0), parent(-1), flags(0), calc(false), m_bIsDummyNode(false), pivot(0.f, 0.f, 0.f), matTransform(Matrix4::IDENTITY), matOffset(Matrix4::IDENTITY), m_finalTrans(0, 0, 0), m_finalScaling(1.f, 1.f, 1.f), mIsUpper(false)
{
}

void Bone::RemoveRedundentKeys()
{
	if (IsPivotBone())
		bUsePivot = (pivot != Vector3(0, 0, 0));

#ifdef ONLY_REMOVE_EQUAL_KEYS
	scale.SetConstantKey(Vector3(1.f, 1.f, 1.f));
	trans.SetConstantKey(Vector3(0, 0, 0));
#else
	scale.SetConstantKey(Vector3(1.f, 1.f, 1.f), 0.0001f);
	trans.SetConstantKey(Vector3(0, 0, 0), 0.00001f);
#endif

}
void Bone::calcMatrix(Bone* allbones)
{
	if (calc)
		return;

	if (parent >= 0) {
		allbones[parent].calcMatrix(allbones);
		mat = allbones[parent].mat;
	}
	else
		mat.identity();

	// compute transformation matrix (mrot) for normal vectors 
	if (parent >= 0)
		mrot = allbones[parent].mrot;
	else
		mrot.identity();
}

/**
for shared animations to take effect,bones must be named properly in 3dsmax (the default biped naming of character studio in 3dsmax 7,8,9 is compatible with us).
Here is the list of known bone names (case insensitive, maybe prefixed with any characters like biped01,but not suffixed):
Root(any name which is parent of pelvis), Pelvis,Spine,	L Thigh,L Calf,	L Foot,	R Thigh,R Calf,	R Foot,	L Clavicle,	L UpperArm,	L Forearm,	L Hand,	R Clavicle,	R UpperArm,	R Forearm,R Hand,Head,Neck,	L Toe0,	R Toe0,		R Finger0,	L Finger0,	Spine1,	Spine2,	Spine3,
The parent of the pelvis bone is always regarded as the root bone (Root).  All predefined bones must have pivot points,otherwise external animation will not be applied.
only the Bone_Root and facial bone's translation animation (which is also scaled automatically according to the local model) will be applied to the local model, for all other predefined bones, only rotation is applied.
This conforms to the BVH file format, where only the root node has translation and rotation animation, where all other nodes contains only rotation animation.
This allows the same animation data to be applied to different models with different bone lengths, but the same topology.
*/
bool Bone::calcMatrix(Bone* allbones, const AnimIndex& CurrentAnim, const AnimIndex& BlendingAnim, float blendingFactor, IAttributeFields* pAnimInstance)
{
	if (calc)
		return true;
	calc = true;
	if (!CurrentAnim.IsValid())
	{
		if (rot.used || scale.used || trans.used)
			return false;
	}
	if (IsStaticTransform() && IsTransformationNode())
	{
		if (parent >= 0) {
			allbones[parent].calcMatrix(allbones, CurrentAnim, BlendingAnim, blendingFactor, pAnimInstance);
			mat = matTransform * allbones[parent].mat;
			if (allbones[parent].IsDummyNode() && !IsDummyNode())
			{
				PostCalculateBoneMatrix();
				m_bIsDummyNode = true;
			}
		}
		else
		{
			mat = matTransform;
			if (!IsDummyNode())
			{
				PostCalculateBoneMatrix();
				m_bIsDummyNode = true;
			}
		}
		return true;
	}

	if (!BlendingAnim.IsValid())
		blendingFactor = 0.f;

	auto current_blending_factor = blendingFactor;
	auto& current_anim = CurrentAnim;
	auto& current_blending_anim = BlendingAnim;

	CBoneAnimProvider* pCurProvider = NULL;
	// the bone in the external bone provider that corresponding to the current bone. 
	Bone* pCurBone = NULL;
	if (current_anim.Provider != 0)
	{
		pCurProvider = CBoneAnimProvider::GetProviderByID(current_anim.nIndex);
		if (pCurProvider)
		{
			pCurBone = FindMatchingBoneInProvider(pCurProvider);
		}
	}
	CBoneAnimProvider* pBlendingProvider = NULL;
	// the bone in the external bone provider that corresponding to the current bone. 
	Bone* pBlendBone = NULL;
	if (current_blending_anim.Provider != 0 && current_blending_anim.IsValid())
	{
		pBlendingProvider = CBoneAnimProvider::GetProviderByID(current_blending_anim.nIndex);
		if (pBlendingProvider)
		{
			pBlendBone = FindMatchingBoneInProvider(pBlendingProvider);
		}
	}
	Quaternion q;
	Vector3 tr(0, 0, 0);
	Vector3 sc(1.f, 1.f, 1.f);


	if (pCurBone == NULL && pBlendBone == NULL)
	{
		//////////////////////////////////////////////////////////////////////////
		// Both current and blending anim are local
		Matrix4 m, mLocalRot;
		int nCurrentAnim = current_anim.nIndex;
		int currentFrame = current_anim.nCurrentFrame;
		int nBlendingAnim = current_blending_anim.nIndex;
		int blendingFrame = current_blending_anim.nCurrentFrame;

		if (IsStaticTransform())
		{
			if (GetExternalRot(pAnimInstance, q))
			{
				// use pivot point
				if (IsPivotBone())
				{
					m.makeTrans(pivot * -1.0f);
					if (GetExternalScaling(pAnimInstance, sc))
					{
						m.m[0][0] = sc.x;
						m.m[1][1] = sc.y;
						m.m[2][2] = sc.z;
						m.m[3][0] *= sc.x;
						m.m[3][1] *= sc.y;
						m.m[3][2] *= sc.z;
					}
					mLocalRot = Matrix4(q);
					m = m.Multiply4x3(mLocalRot);
					m.offsetTrans(pivot);
				}
				else
				{
					if (GetExternalScaling(pAnimInstance, sc))
					{
						m.makeScale(sc);
						mLocalRot = Matrix4(q);
						m *= mLocalRot;
					}
					else
					{
						mLocalRot = Matrix4(q);
						m = mLocalRot;
					}
				}
				if (GetExternalTranslation(pAnimInstance, tr))
				{
					m.offsetTrans(tr);
				}
				m = m * matTransform;
			}
			else
			{
				if (GetExternalScaling(pAnimInstance, sc))
				{
					m.makeScale(sc);
					if (GetExternalTranslation(pAnimInstance, tr))
					{
						m.offsetTrans(tr);
					}
					m = m * matTransform;
				}
				else if (GetExternalTranslation(pAnimInstance, tr))
				{
					m.makeTrans(tr);
					m = m * matTransform;
				}
				else
					m = matTransform;
			}
		}
		else if (HasBoneAnimation())
		{
			// #define PERFOAMRNCE_TEST_calcMatrix
#ifdef PERFOAMRNCE_TEST_calcMatrix
			PERF1("calcMatrix");
#endif
			// Compute transform matrix from SRT keys and the pivot point. m[column] = (pivot)*T*R*S*(-pivot)
			if (bUsePivot)
			{
				// use pivot point
				m.makeTrans(pivot * -1.0f);

				if (GetExternalScaling(pAnimInstance, sc) ||
					(scale.used && (sc = scale.getValue(nCurrentAnim, currentFrame/*, nBlendingAnim, blendingFrame, blendingFactor*/)) != Vector3::UNIT_SCALE))
				{
					m.m[0][0] = sc.x;
					m.m[1][1] = sc.y;
					m.m[2][2] = sc.z;
					m.m[3][0] *= sc.x;
					m.m[3][1] *= sc.y;
					m.m[3][2] *= sc.z;
				}
				if (GetExternalRot(pAnimInstance, q))
				{
					mLocalRot = Matrix4(q);
					m = m.Multiply4x3(mLocalRot);
				}
				else if (rot.used) {
					q = rot.getValue(nCurrentAnim, currentFrame, nBlendingAnim, blendingFrame, current_blending_factor);
					mLocalRot = Matrix4(q.invertWinding());
					m = m.Multiply4x3(mLocalRot);
				}

				if (GetExternalTranslation(pAnimInstance, tr))
				{
					m.offsetTrans(tr);
				}
				else if (trans.used) {
					tr = trans.getValue(nCurrentAnim, currentFrame, nBlendingAnim, blendingFrame, current_blending_factor);
					m.offsetTrans(tr);
				}
				m.offsetTrans(pivot);
			}
			else
			{
				// no pivot point is used.
				if (GetExternalScaling(pAnimInstance, sc) ||
					(scale.used && (sc = scale.getValue(nCurrentAnim, currentFrame/*, nBlendingAnim, blendingFrame, blendingFactor*/)) != Vector3::UNIT_SCALE))

				{
					m.makeScale(sc);
					if (GetExternalRot(pAnimInstance, q))
					{
						mLocalRot = Matrix4(q);
						m = m.Multiply4x3(mLocalRot);
					}
					else if (rot.used) {
						q = rot.getValue(nCurrentAnim, currentFrame, nBlendingAnim, blendingFrame, current_blending_factor);
						mLocalRot = Matrix4(q.invertWinding());
						m = m.Multiply4x3(mLocalRot);
					}
				}
				else
				{
					if (GetExternalRot(pAnimInstance, q))
					{
						mLocalRot = Matrix4(q);
						m = mLocalRot;
					}
					else if (rot.used) {
						q = rot.getValue(nCurrentAnim, currentFrame, nBlendingAnim, blendingFrame, current_blending_factor);
						mLocalRot = Matrix4(q.invertWinding());
						m = mLocalRot;
					}
					else
						m.identity();
				}

				if (GetExternalTranslation(pAnimInstance, tr))
				{
					m.offsetTrans(tr);
				}
				else if (trans.used) {
					tr = trans.getValue(nCurrentAnim, currentFrame, nBlendingAnim, blendingFrame, current_blending_factor);
					m.offsetTrans(tr);
				}
			}
		}
		else
		{
			m.identity();
		}

		if (parent >= 0) {
			allbones[parent].calcMatrix(allbones, CurrentAnim, BlendingAnim, blendingFactor, pAnimInstance);
			mat = m * allbones[parent].mat;
		}
		else
			mat = m;
	}
	else
	{
		//////////////////////////////////////////////////////////////////////////
		// either current anim or the blending animation references an external animation provider.
		Matrix4 m, mLocalRot;
		mLocalRot.identity();
		m.identity();
		int nCurrentAnim = current_anim.nIndex;
		int currentFrame = current_anim.nCurrentFrame;
		int nBlendingAnim = current_blending_anim.nIndex;
		int blendingFrame = current_blending_anim.nCurrentFrame;

		// Compute transform matrix from SRT keys and the pivot point. m[column] = (pivot)*T*R*S*(-pivot)
		if (bUsePivot)
		{
			// use pivot point
			m.makeTrans(pivot * -1.0f);
			if (scale.used)
			{
				// always uses local scale
				sc = scale.getValue(current_anim);

				m.m[0][0] = sc.x;
				m.m[1][1] = sc.y;
				m.m[2][2] = sc.z;
				m.m[3][0] *= sc.x;
				m.m[3][1] *= sc.y;
				m.m[3][2] *= sc.z;
			}
			//if (rot.used) 
			if (GetExternalRot(pAnimInstance, q))
			{
				mLocalRot = Matrix4(q);
				m = m.Multiply4x3(mLocalRot);
			}
			else
			{
				//////////////////////////////////////////////////////////////////////////
				// external animation may affect the rotations, but not scale or translation, except the root and facial bone.
				Quaternion currentValue;
				Quaternion blendValue;
				if (pCurBone != NULL)
				{
					if (pCurBone->rot.used)
						currentValue = pCurBone->rot.getValue(pCurProvider->GetSubAnimID(), current_anim.nCurrentFrame);
				}
				else if (rot.used)
				{
					if (current_anim.Provider == 0)
						currentValue = rot.getValue(current_anim);
					else
						currentValue = rot.getDefaultValue();
				}
				if (current_blending_factor != 0.f)
				{
					if (pBlendBone != NULL)
					{
						if (pBlendBone->rot.used)
							blendValue = pBlendBone->rot.getValue(pBlendingProvider->GetSubAnimID(), current_blending_anim.nCurrentFrame);
					}
					else if (rot.used)
					{
						if (current_blending_anim.Provider == 0)
							blendValue = rot.getValue(current_blending_anim);
						else
							blendValue = currentValue;
					}
					if (pCurBone == NULL && current_anim.Provider != 0)
					{
						currentValue = blendValue;
					}
				}
				q = rot.BlendValues(currentValue, blendValue, current_blending_factor);
				mLocalRot = Matrix4(q.invertWinding());
				m = m * mLocalRot;
			}
			//if (trans.used) 
			{
				if (!(nBoneID == Bone_Root || (nBoneID >= Bone_forehand && nBoneID <= Bone_chin)))
				{
					// this is any other bone, except the root bone and facial animation bone.
					tr = trans.getValue(current_anim, current_blending_anim, current_blending_factor);
					m.offsetTrans(tr);
				}
				else
				{
					// only root node and facial animation may contain translation animation, in shared animation mode? such as BVH files.
					// more over, translation animation is scaled uniformly according to the pivot point Y values in the external animation file vs local file.
					Vector3 currentValue;
					Vector3 blendValue;

					if (pCurBone != NULL)
					{
						currentValue = pCurBone->trans.getValue(pCurProvider->GetSubAnimID(), current_anim.nCurrentFrame);
						if (nBoneID == Bone_Root)
						{
							// translation animation is scaled uniformly according to the pivot point Y values in the external animation file vs local file.
							currentValue *= (pivot.y / pCurBone->pivot.y);
						}
					}
					else
					{
						if (current_anim.Provider == 0)
							currentValue = trans.getValue(current_anim);
						else
							currentValue = trans.getDefaultValue();
					}

					if (current_blending_factor != 0.f)
					{
						if (pBlendBone != NULL)
						{
							blendValue = pBlendBone->trans.getValue(pBlendingProvider->GetSubAnimID(), current_blending_anim.nCurrentFrame);
							if (nBoneID == Bone_Root)
							{
								// translation animation is scaled uniformly according to the pivot point Y values in the external animation file vs local file.
								blendValue *= (pivot.y / pBlendBone->pivot.y);
							}
						}
						else
						{
							if (current_blending_anim.Provider == 0)
								blendValue = trans.getValue(current_blending_anim);
							else
								blendValue = currentValue;
						}
						if (pCurBone == NULL && current_anim.Provider != 0)
						{
							currentValue = blendValue;
						}
					}

					tr = trans.BlendValues(currentValue, blendValue, current_blending_factor);
					m.offsetTrans(tr);
				}
			}
			m.offsetTrans(pivot);
		}
		else if (IsStaticTransform() && (pCurBone == NULL || pCurBone->IsStaticTransform()) && (pBlendBone == NULL || pBlendBone->IsStaticTransform()))
		{
			if (GetExternalRot(pAnimInstance, q))
			{
				// use pivot point
				if (IsPivotBone())
				{
					m.makeTrans(pivot * -1.0f);
					mLocalRot = Matrix4(q);
					m = m.Multiply4x3(mLocalRot);
					m.offsetTrans(pivot);
				}
				else
				{
					mLocalRot = Matrix4(q);
					m = mLocalRot;
				}
			}
			else
			{
				m = matTransform;
			}
		}
		else
		{
			// external animation is not applied when there is no pivot point in the bone
			if ((pCurBone && pCurBone->scale.used) || (pBlendBone && pBlendBone->scale.used))
			{
				Vector3 currentValue(0, 0, 0);
				Vector3 blendValue(0, 0, 0);

				if (pCurBone != NULL)
				{
					currentValue = pCurBone->scale.getValue(pCurProvider->GetSubAnimID(), current_anim.nCurrentFrame);
					// currentValue *= (pivot.y / pCurBone->pivot.y);
				}
				else if (scale.used)
				{
					if (current_anim.Provider == 0)
						currentValue = scale.getValue(current_anim);
					else
						currentValue = scale.getDefaultValue();
				}

				if (current_blending_factor != 0.f)
				{
					if (pBlendBone != NULL)
					{
						blendValue = pBlendBone->scale.getValue(pBlendingProvider->GetSubAnimID(), current_blending_anim.nCurrentFrame);
						// blendValue *= (pivot.y / pBlendBone->pivot.y);
					}
					else if (scale.used)
					{
						if (current_blending_anim.Provider == 0)
							blendValue = scale.getValue(current_blending_anim);
						else
							blendValue = currentValue;
					}
					if (pCurBone == NULL && current_anim.Provider != 0)
					{
						currentValue = blendValue;
					}
				}
				sc = trans.BlendValues(currentValue, blendValue, current_blending_factor);
				m.makeScale(sc);
			}
			else if (scale.used)
			{
				sc = scale.getValue(current_anim);
				m.makeScale(sc);
			}
			if (GetExternalRot(pAnimInstance, q))
			{
				mLocalRot = Matrix4(q);
				m = m.Multiply4x3(mLocalRot);
			}
			else
			{
				Quaternion currentValue;
				Quaternion blendValue;
				if (pCurBone != NULL)
				{
					if (pCurBone->rot.used)
						currentValue = pCurBone->rot.getValue(pCurProvider->GetSubAnimID(), current_anim.nCurrentFrame);
				}
				else if (rot.used)
				{
					if (current_anim.Provider == 0)
						currentValue = rot.getValue(current_anim);
					else
						currentValue = rot.getDefaultValue();
				}
				if (current_blending_factor != 0.f)
				{
					if (pBlendBone != NULL)
					{
						if (pBlendBone->rot.used)
							blendValue = pBlendBone->rot.getValue(pBlendingProvider->GetSubAnimID(), current_blending_anim.nCurrentFrame);
					}
					else if (rot.used)
					{
						if (current_blending_anim.Provider == 0)
							blendValue = rot.getValue(current_blending_anim);
						else
							blendValue = currentValue;
					}
					if (pCurBone == NULL && current_anim.Provider != 0)
					{
						currentValue = blendValue;
					}
				}
				q = rot.BlendValues(currentValue, blendValue, current_blending_factor);
				mLocalRot = Matrix4(q.invertWinding());
				m = m.Multiply4x3(mLocalRot);
			}

			// translation is only applied to root bone like Hips
			if ((pCurBone && pCurBone->trans.used && pCurBone->GetBoneID() == Bone_Root) || (pBlendBone && pBlendBone->trans.used && pBlendBone->GetBoneID() == Bone_Root))
			{
				// TODO: translation animation is scaled uniformly according to root bone height ratio in external vs local model.
				Vector3 currentValue(0, 0, 0);
				Vector3 blendValue(0, 0, 0);

				if (pCurBone != NULL)
				{
					currentValue = pCurBone->trans.getValue(pCurProvider->GetSubAnimID(), current_anim.nCurrentFrame);
					// currentValue *= (pivot.y / pCurBone->pivot.y);
				}
				else if (trans.used)
				{
					if (current_anim.Provider == 0)
						currentValue = trans.getValue(current_anim);
					else
						currentValue = trans.getDefaultValue();
				}

				if (current_blending_factor != 0.f)
				{
					if (pBlendBone != NULL)
					{
						blendValue = pBlendBone->trans.getValue(pBlendingProvider->GetSubAnimID(), current_blending_anim.nCurrentFrame);
						// blendValue *= (pivot.y / pBlendBone->pivot.y);
					}
					else if (trans.used)
					{
						if (current_blending_anim.Provider == 0)
							blendValue = trans.getValue(current_blending_anim);
						else
							blendValue = currentValue;
					}
					if (pCurBone == NULL && current_anim.Provider != 0)
					{
						currentValue = blendValue;
					}
				}
				tr = trans.BlendValues(currentValue, blendValue, current_blending_factor);
				m.offsetTrans(tr);
			}
			else if (trans.used) {
				tr = trans.getValue(current_anim, current_blending_anim, current_blending_factor);
				m.offsetTrans(tr);
			}
		}


		if (parent >= 0) {
			allbones[parent].calcMatrix(allbones, CurrentAnim, BlendingAnim, blendingFactor, pAnimInstance);
			mat = m * allbones[parent].mat;
		}
		else
			mat = m;
	}

	if (IsBillBoarded())
	{
		//////////////////////////////////////////////////////////////////////////
		// billboarding should be applied after the entire bone transformation is done.

		Matrix4 mtrans = CGlobals::GetViewMatrixStack().SafeGetTop();
		Matrix4 mtransWorld = CGlobals::GetWorldMatrixStack().SafeGetTop();
		mtransWorld = mat * mtransWorld; // added 

		// convert everything to model space, so that we have look, up, right vector in model space.  
		mtrans = mtransWorld * mtrans;
		mtrans.invert();
		Vector3 camera = Vector3(0, 0, 0) * mtrans;
		Vector3 look = (camera - pivot).normalisedCopy();
		Vector3 up = ((mtrans * Vector3(0, 1, 0)) - camera).normalisedCopy();
		Vector3 right = (up % look).normalisedCopy();
		up = (look % right).normalisedCopy();

		// calculate a billboard matrix
		Matrix4 mbb;
		mbb.identity();

		mbb.m[2][0] = right.x;
		mbb.m[2][1] = right.y;
		mbb.m[2][2] = right.z;
		mbb.m[1][0] = up.x;
		mbb.m[1][1] = up.y;
		mbb.m[1][2] = up.z;
		mbb.m[0][0] = look.x;
		mbb.m[0][1] = look.y;
		mbb.m[0][2] = look.z;

		// fixed pivot LXZ 2008.12.3. 
		if (bUsePivot)
		{
			mtrans.makeTrans(pivot * -1.0f);
			mtrans = mtrans * mbb;
			mtrans.offsetTrans(pivot);
			mbb = mtrans;
		}

		mat = mbb * mat;
	}
	SetFinalRot(q);
	SetFinalTrans(tr);
	SetFinalScaling(sc);
	return true;
}

ParaEngine::Bone::~Bone()
{

}

const std::string& ParaEngine::Bone::GetIdentifier()
{
	if (m_sIdentifer.empty())
	{
		if (GetBoneID() > 0)
		{
			KNOWN_BONE_NODES boneNames = (KNOWN_BONE_NODES)GetBoneID();
			switch (boneNames)
			{
			case ParaEngine::Bone_Root:
				SetName("Root");
				break;
			case ParaEngine::Bone_Pelvis:
				SetName("Pelvis");
				break;
			case ParaEngine::Bone_Spine:
				SetName("Spine");
				break;
			case ParaEngine::Bone_L_Thigh:
				SetName("L_Thigh");
				break;
			case ParaEngine::Bone_L_Calf:
				SetName("L_Calf");
				break;
			case ParaEngine::Bone_L_Foot:
				SetName("L_Foot");
				break;
			case ParaEngine::Bone_R_Thigh:
				SetName("R_Thigh");
				break;
			case ParaEngine::Bone_R_Calf:
				SetName("R_Calf");
				break;
			case ParaEngine::Bone_R_Foot:
				SetName("R_Foot");
				break;
			case ParaEngine::Bone_L_Clavicle:
				SetName("L_Clavicle");
				break;
			case ParaEngine::Bone_L_UpperArm:
				SetName("L_UpperArm");
				break;
			case ParaEngine::Bone_L_Forearm:
				SetName("L_Forearm");
				break;
			case ParaEngine::Bone_L_Hand:
				SetName("L_Hand");
				break;
			case ParaEngine::Bone_R_Clavicle:
				SetName("R_Clavicle");
				break;
			case ParaEngine::Bone_R_UpperArm:
				SetName("R_UpperArm");
				break;
			case ParaEngine::Bone_R_Forearm:
				SetName("R_Forearm");
				break;
			case ParaEngine::Bone_R_Hand:
				SetName("R_Hand");
				break;
			case ParaEngine::Bone_Head:
				SetName("Head");
				break;
			case ParaEngine::Bone_Neck:
				SetName("Neck");
				break;
			case ParaEngine::Bone_L_Toe0:
				SetName("L_Toe0");
				break;
			case ParaEngine::Bone_R_Toe0:
				SetName("R_Toe0");
				break;
			case ParaEngine::Bone_R_Finger0:
				SetName("R_Finger0");
				break;
			case ParaEngine::Bone_L_Finger0:
				SetName("L_Finger0");
				break;
			case ParaEngine::Bone_Spine1:
				SetName("Spine1");
				break;
			case ParaEngine::Bone_Spine2:
				SetName("Spine2");
				break;
			case ParaEngine::Bone_Spine3:
				SetName("Spine3");
				break;
			case ParaEngine::Bone_forehand:
				SetName("forehand");
				break;
			case ParaEngine::Bone_L_eyelid:
				SetName("L_eyelid");
				break;
			case ParaEngine::Bone_R_eyelid:
				SetName("R_eyelid");
				break;
			case ParaEngine::Bone_L_eye:
				SetName("L_eye");
				break;
			case ParaEngine::Bone_R_eye:
				SetName("R_eye");
				break;
			case ParaEngine::Bone_B_eyelid:
				SetName("B_eyelid");
				break;
			case ParaEngine::Bone_upper_lip:
				SetName("upper_lip");
				break;
			case ParaEngine::Bone_L_lip:
				SetName("L_lip");
				break;
			case ParaEngine::Bone_R_lip:
				SetName("R_lip");
				break;
			case ParaEngine::Bone_B_lip:
				SetName("B_lip");
				break;
			case ParaEngine::Bone_chin:
				SetName("chin");
				break;
			case ParaEngine::Bone_R_Finger01:
				SetName("R_Finger01");
				break;
			case ParaEngine::Bone_L_Finger01:
				SetName("RL_Finger01oot");
				break;
			case ParaEngine::Bone_R_Finger1:
				SetName("R_Finger1");
				break;
			case ParaEngine::Bone_L_Finger1:
				SetName("L_Finger1");
				break;
			case ParaEngine::Bone_R_Finger11:
				SetName("R_Finger11");
				break;
			case ParaEngine::Bone_L_Finger11:
				SetName("L_Finger11");
				break;
			case ParaEngine::Bone_R_Finger2:
				SetName("R_Finger2");
				break;
			case ParaEngine::Bone_L_Finger2:
				SetName("L_Finger2");
				break;
			case ParaEngine::Bone_R_Finger21:
				SetName("R_Finger21");
				break;
			case ParaEngine::Bone_L_Finger21:
				SetName("L_Finger21");
				break;
			default:
			{
				CVariable var;
				var = nIndex;
				SetName(std::string("Unknown") + (const std::string&)var);
				break;
			}
			}
		}
		else
		{
			CVariable var;
			var = nIndex;
			SetName((const std::string&)var);
		}
	}
	return GetName();
}

void ParaEngine::Bone::SetName(const std::string& val)
{
	m_sIdentifer = val;

	// extract tag from the name in {}
	auto nFromPos = val.find_first_of('{');
	if (nFromPos != string::npos)
	{
		auto nToPos = val.find_last_of('}');
		if (nToPos != string::npos)
		{
			m_sTag = val.substr(nFromPos, nToPos - nFromPos + 1);
			return;
		}
	}
	m_sTag.clear();
}

void ParaEngine::Bone::AutoSetBoneInfoFromName()
{
	if (m_sIdentifer.empty() || GetBoneID() > 0)
		return;
	std::string sName = m_sIdentifer;
	// remove the tag from the name
	auto nFromPos = sName.find_first_of('{');
	if (nFromPos != string::npos)
	{
		sName = sName.substr(0, nFromPos);
	}

	StringHelper::make_lower(sName);

	// check special meaning ending names
	{
		int nSize = sName.size();
		int nMarkIndex = nSize - 2;// Index of the character '_'

		while (nMarkIndex >= 0 && sName[nMarkIndex] == '_')
		{
			char symbol = sName[nMarkIndex + 1];
			if (symbol == 'b')
			{
				// the bone is billboarded. always facing the camera. 
				flags |= 0x8;
			}
			else if (symbol == 'u')
			{
				// the bone is billboarded, but up axis is fixed. always facing the camera. 
				flags |= 0x4;
			}
			nMarkIndex -= 2;
		}
	}
	// Check the bone name for some predefined attachment points
	if ((int)sName.length() > 4 && sName[0] == 'a' && sName[1] == 't' && sName[2] == 't' && sName[3] == '_')
	{
		// if the name begins with "att". 
		string sAttName = sName.substr(4);
		if (sAttName == "lefthand")
		{
			SetBoneID(-ATT_ID_HAND_LEFT);
			mIsUpper = true;
		}
		else if (sAttName == "righthand")
		{
			SetBoneID(-ATT_ID_HAND_RIGHT);
			mIsUpper = true;
		}
		else if (sAttName == "head")
		{
			SetBoneID(-ATT_ID_HEAD);
			mIsUpper = true;
		}
		else if (sAttName == "text")
		{
			SetBoneID(-ATT_ID_TEXT);
		}
		else if (sAttName == "ground")
		{
			SetBoneID(-ATT_ID_GROUND);
		}
		else if (sAttName == "leftshoulder")
		{
			SetBoneID(-ATT_ID_SHOULDER_LEFT);
		}
		else if (sAttName == "rightshoulder")
		{
			SetBoneID(-ATT_ID_SHOULDER_RIGHT);
		}
		else if (sAttName == "boots")
		{
			SetBoneID(-ATT_ID_BOOTS);
			mIsUpper = true;
		}
		else if (sAttName == "neck")
		{
			SetBoneID(-ATT_ID_NECK);
			mIsUpper = true;
		}
		else if (sAttName == "mouth")
		{
			SetBoneID(-ATT_ID_MOUTH);
		}
		else if (sAttName == "face" || sAttName == "overhead")
		{
			SetBoneID(-ATT_ID_FACE_ADDON);
		}
		else if (sAttName == "leftear")
		{
			SetBoneID(-ATT_ID_EAR_LEFT_ADDON);
		}
		else if (sAttName == "rightear")
		{
			SetBoneID(-ATT_ID_EAR_RIGHT_ADDON);
		}
		else if (sAttName == "back")
		{
			SetBoneID(-ATT_ID_BACK_ADDON);
		}
		else if (sAttName == "waist")
		{
			SetBoneID(-ATT_ID_WAIST);
		}
	}
	else if (StringHelper::StrEndsWithWord(sName, "mount") || StringHelper::StrEndsWithWord(sName, "shield"))
	{
		// Default Mount or shield position
		SetBoneID(-ATT_ID_MOUNT00);
	}
	else if (StringHelper::StrEndsWithWord(sName, "mount?") || StringHelper::StrEndsWithWord(sName, "mount??"))
	{
		// additional mount position
		int nMountBoneIndex = 0;
		char c = sName[sName.size() - 1];
		if (c >= '0' && c <= '9')
			nMountBoneIndex = c - '0';
		c = sName[sName.size() - 2];
		if (c >= '0' && c <= '9')
			nMountBoneIndex = (c - '0') * 10 + nMountBoneIndex;

		int nAttID = ATT_ID_MOUNT00;
		if (nMountBoneIndex >= 1 && nMountBoneIndex <= 99)
		{
			nAttID = ATT_ID_MOUNT1 + (nMountBoneIndex - 1);
		}
		SetBoneID(-nAttID);

		if (nMountBoneIndex >= 1 && nMountBoneIndex <= 7)
			mIsUpper = true;
	}
	else
	{
		std::string::size_type nPos = -1;
		if (StringHelper::StrEndsWithWord(sName, "r?hand"))
		{
			SetBoneID(Bone_R_Hand);
			SetIdentifier("");
			mIsUpper = true;
		}
		else if (StringHelper::StrEndsWithWord(sName, "l?hand"))
		{
			SetBoneID(Bone_L_Hand);
			SetIdentifier("");
			mIsUpper = true;
		}
		else if (StringHelper::StrEndsWithWord(sName, "l?foot"))
		{
			SetBoneID(Bone_L_Foot);
			SetIdentifier("");
		}
		else if (StringHelper::StrEndsWithWord(sName, "r?foot"))
		{
			SetBoneID(Bone_R_Foot);
			SetIdentifier("");
		}
		else if (StringHelper::StrEndsWithWord(sName, "l?upperarm"))
		{
			SetBoneID(Bone_L_UpperArm);
			SetIdentifier("");
			mIsUpper = true;
		}
		else if (StringHelper::StrEndsWithWord(sName, "r?upperarm"))
		{
			SetBoneID(Bone_R_UpperArm);
			SetIdentifier("");
			mIsUpper = true;
		}
		else if (StringHelper::StrEndsWithWord(sName, "pelvis"))
		{
			SetBoneID(Bone_Pelvis);
			SetIdentifier("");
		}
		else if (StringHelper::StrEndsWithWord(sName, "spine"))
		{
			SetBoneID(Bone_Spine);
			SetIdentifier("");
			mIsUpper = true;
		}
		else if (StringHelper::StrEndsWithWord(sName, "spine1"))
		{
			SetBoneID(Bone_Spine1);
			SetIdentifier("");
			mIsUpper = true;
		}
		else if (StringHelper::StrEndsWithWord(sName, "spine2"))
		{
			SetBoneID(Bone_Spine2);
			SetIdentifier("");
		}
		else if (StringHelper::StrEndsWithWord(sName, "spine3"))
		{
			SetBoneID(Bone_Spine3);
			SetIdentifier("");
		}
		else if (StringHelper::StrEndsWithWord(sName, "l?thigh"))
		{
			SetBoneID(Bone_L_Thigh);
			SetIdentifier("");
		}
		else if (StringHelper::StrEndsWithWord(sName, "r?thigh"))
		{
			SetBoneID(Bone_R_Thigh);
			SetIdentifier("");
		}
		else if (StringHelper::StrEndsWithWord(sName, "l?calf"))
		{
			SetBoneID(Bone_L_Calf);
			SetIdentifier("");
		}
		else if (StringHelper::StrEndsWithWord(sName, "r?calf"))
		{
			SetBoneID(Bone_R_Calf);
			SetIdentifier("");
		}
		else if (StringHelper::StrEndsWithWord(sName, "l?forearm"))
		{
			SetBoneID(Bone_L_Forearm);
			SetIdentifier("");
			mIsUpper = true;
		}
		else if (StringHelper::StrEndsWithWord(sName, "r?forearm"))
		{
			SetBoneID(Bone_R_Forearm);
			SetIdentifier("");
			mIsUpper = true;
		}
		else if (StringHelper::StrEndsWithWord(sName, "l?clavicle"))
		{
			SetBoneID(Bone_L_Clavicle);
			SetIdentifier("");
			mIsUpper = true;
		}
		else if (StringHelper::StrEndsWithWord(sName, "r?clavicle"))
		{
			SetBoneID(Bone_R_Clavicle);
			SetIdentifier("");
			mIsUpper = true;
		}
		else if (StringHelper::StrEndsWithWord(sName, "l?toe0"))
		{
			SetBoneID(Bone_L_Toe0);
			SetIdentifier("");
		}
		else if (StringHelper::StrEndsWithWord(sName, "r?toe0"))
		{
			SetBoneID(Bone_R_Toe0);
			SetIdentifier("");
		}
		else if (StringHelper::StrEndsWithWord(sName, "neck"))
		{
			SetBoneID(Bone_Neck);
			SetIdentifier("");
			mIsUpper = true;
		}
		else if (StringHelper::StrEndsWithWord(sName, "l?finger0"))
		{
			SetBoneID(Bone_L_Finger0);
			SetIdentifier("");
			mIsUpper = true;
		}
		else if (StringHelper::StrEndsWithWord(sName, "r?finger0"))
		{
			SetBoneID(Bone_R_Finger0);
			SetIdentifier("");
			mIsUpper = true;
		}
		else if (StringHelper::StrEndsWithWord(sName, "l?finger1"))
		{
			SetBoneID(Bone_L_Finger1);
			SetIdentifier("");
			mIsUpper = true;
		}
		else if (StringHelper::StrEndsWithWord(sName, "r?finger1"))
		{
			SetBoneID(Bone_R_Finger1);
			SetIdentifier("");
			mIsUpper = true;
		}
		else if ((nPos = sName.find("head")) != string::npos)
		{
			if ((nPos == (sName.size() - 4)) &&
				(nPos == 0 || sName[nPos - 1] == ' ' || sName[nPos - 1] == '_'))
			{
				SetBoneID(Bone_Head);
				SetIdentifier("");
				mIsUpper = true;
			}
		}
		else if (StringHelper::StrEndsWithWord(sName, "upper"))
		{
			mIsUpper = true;
		}
	}
}

void ParaEngine::Bone::SetIdentifier(const std::string& sID)
{
	SetName(sID);
}

void ParaEngine::Bone::SetOffsetMatrix(const Matrix4& mat)
{
	matOffset = mat;
	flags |= BONE_OFFSET_MATRIX;
	bUsePivot = false;
}

const std::string& ParaEngine::Bone::GetName() const
{
	return m_sIdentifer;
}

void ParaEngine::Bone::SetStaticTransform(const Matrix4& mat)
{
	matTransform = mat;
	flags |= BONE_STATIC_TRANSFORM;
	bUsePivot = false;
}

bool ParaEngine::Bone::CheckHasAnimation()
{
	return !(IsStaticTransform() || (!scale.CheckIsAnimated() && !trans.CheckIsAnimated() && !rot.CheckIsAnimated()));
}

bool ParaEngine::Bone::IsAnimated()
{
	return !(IsStaticTransform() || (!scale.used && !trans.used && !rot.used));
}

bool ParaEngine::Bone::GetExternalRot(IAttributeFields* pAnimInstance, Quaternion& outQuat)
{
	if (pAnimInstance && pAnimInstance->GetDynamicFieldCount() > 0)
	{
		CDynamicAttributeField* pVarField = pAnimInstance->GetDynamicField(GetRotName());
		if (pVarField)
		{
			int nTime = pAnimInstance->GetTime();
			CDynamicAttributeField* pTimeField = pAnimInstance->GetDynamicField(GetTimeName());
			if (pTimeField != 0) {
				nTime = (int)((double)(*pTimeField));
				if (nTime < 0)
				{
					if (nTime <= -1000)
						return false;
					else
						nTime = pAnimInstance->GetTime();
				}
			}
			return pVarField->GetValueByTime(nTime, outQuat);
		}
	}
	return false;
}

bool ParaEngine::Bone::GetExternalTranslation(IAttributeFields* pAnimInstance, Vector3& outTrans)
{
	if (pAnimInstance && pAnimInstance->GetDynamicFieldCount() > 0)
	{
		CDynamicAttributeField* pVarField = pAnimInstance->GetDynamicField(GetTransName());
		if (pVarField)
		{
			int nTime = pAnimInstance->GetTime();
			CDynamicAttributeField* pTimeField = pAnimInstance->GetDynamicField(GetTimeName());
			if (pTimeField != 0) {
				nTime = (int)((double)(*pTimeField));
				if (nTime < 0)
				{
					if (nTime <= -1000)
						return false;
					else
						nTime = pAnimInstance->GetTime();
				}
			}
			return pVarField->GetValueByTime(nTime, outTrans);
		}
	}
	return false;
}

bool ParaEngine::Bone::GetExternalScaling(IAttributeFields* pAnimInstance, Vector3& outScaling)
{
	if (pAnimInstance && pAnimInstance->GetDynamicFieldCount() > 0)
	{
		CDynamicAttributeField* pVarField = pAnimInstance->GetDynamicField(GetScaleName());
		if (pVarField)
		{
			int nTime = pAnimInstance->GetTime();
			CDynamicAttributeField* pTimeField = pAnimInstance->GetDynamicField(GetTimeName());
			if (pTimeField != 0) {
				nTime = (int)((double)(*pTimeField));
				if (nTime < 0)
				{
					if (nTime <= -1000)
						return false;
					else
						nTime = pAnimInstance->GetTime();
				}
			}
			return pVarField->GetValueByTime(nTime, outScaling);
		}
	}
	return false;
}

void ParaEngine::Bone::PostCalculateBoneMatrix()
{
	if (!IsDummyNode())
	{
		if (IsOffsetMatrixBone() && !IsAttachment())
		{
			mat = matOffset * mat;
		}
		// compute transformation matrix (mrot) for normal vectors 
		{
			// Quaternion q = mat.extractQuaternion();
			// q.ToRotationMatrix(mrot, Vector3::ZERO);
			mrot = mat.RemoveTranslation();
			mrot.RemoveScaling();
		}
	}
}

const ParaEngine::Matrix4& ParaEngine::Bone::GetFinalMatrix()
{
	return mat;
}

const Vector3& ParaEngine::Bone::GetPivotPoint()
{
	return pivot;
}


ParaEngine::Vector3 ParaEngine::Bone::GetAnimatedPivotPoint()
{
	if (IsOffsetMatrixBone() || IsPivotBone())
	{
		Vector3 vPivot = pivot * mat;
		return vPivot;
	}
	else
	{
		return Vector3(mat._41, mat._42, mat._43);
	}
}

int ParaEngine::Bone::IsAttachment() const
{
	return nBoneID < 0;
}

int ParaEngine::Bone::GetAttachmentId() const
{
	return nBoneID < 0 ? ((-nBoneID == ATT_ID_MOUNT00) ? 0 : -nBoneID) : -1;
}

const ParaEngine::Quaternion& ParaEngine::Bone::GetFinalRot() const
{
	return m_finalRot;
}

void ParaEngine::Bone::SetFinalRot(const ParaEngine::Quaternion& val)
{
	m_finalRot = val;
}

const Vector3& ParaEngine::Bone::GetFinalTrans() const
{
	return m_finalTrans;
}

void ParaEngine::Bone::SetFinalTrans(const Vector3& val)
{
	m_finalTrans = val;
}

const Vector3& ParaEngine::Bone::GetFinalScaling() const
{
	return m_finalScaling;
}

void ParaEngine::Bone::SetFinalScaling(const Vector3& val)
{
	m_finalScaling = val;
}

const ParaEngine::Matrix4& ParaEngine::Bone::GetFinalRotMatrix() const
{
	return mrot;
}

ParaEngine::Matrix4 ParaEngine::Bone::GetPivotRotMatrix()
{
	if (IsOffsetMatrixBone())
	{
		Matrix4 mat = matOffset.RemoveTranslation();
		mat.RemoveScaling();
		mat.invert();
		mat = mat * GetFinalRotMatrix();
		return mat;
	}
	else
	{
		return GetFinalRotMatrix();
	}
}

void ParaEngine::Bone::MakeDirty(bool bForce)
{
	if (!IsDummyNode())
		calc = false;
}

const std::string& ParaEngine::Bone::GetTag()
{
	return m_sTag;
}

const std::string& ParaEngine::Bone::GetRotName()
{

	if (m_sRotName.empty())
	{
		m_sRotName = GetIdentifier() + "_rot";
	}
	return m_sRotName;
}

const std::string& ParaEngine::Bone::GetTransName()
{
	if (m_sTransName.empty())
	{
		m_sTransName = GetIdentifier() + "_trans";
	}
	return m_sTransName;
}

const std::string& ParaEngine::Bone::GetScaleName()
{
	if (m_sScaleName.empty())
	{
		m_sScaleName = GetIdentifier() + "_scale";
	}
	return m_sScaleName;
}

const std::string& ParaEngine::Bone::GetTimeName()
{
	if (m_sTimeName.empty())
	{
		m_sTimeName = GetIdentifier() + "_time";
	}
	return m_sTimeName;
}

int ParaEngine::Bone::InstallFields(CAttributeClass* pClass, bool bOverride)
{
	IAttributeFields::InstallFields(pClass, bOverride);

	pClass->AddField("Tag", FieldType_String, (void*)0, (void*)GetTag_s, NULL, "", bOverride);
	pClass->AddField("RotName", FieldType_String, (void*)0, (void*)GetRotName_s, NULL, "", bOverride);
	pClass->AddField("TransName", FieldType_String, (void*)0, (void*)GetTransName_s, NULL, "", bOverride);
	pClass->AddField("ScaleName", FieldType_String, (void*)0, (void*)GetScaleName_s, NULL, "", bOverride);
	pClass->AddField("TimeName", FieldType_String, (void*)0, (void*)GetTimeName_s, NULL, "", bOverride);

	pClass->AddField("IsBillBoarded", FieldType_Bool, (void*)0, (void*)IsBillBoarded_s, NULL, "", bOverride);
	pClass->AddField("IsPivotBone", FieldType_Bool, (void*)0, (void*)IsPivotBone_s, NULL, "", bOverride);
	pClass->AddField("IsOffsetMatrixBone", FieldType_Bool, (void*)0, (void*)IsOffsetMatrixBone_s, NULL, "", bOverride);
	pClass->AddField("IsStaticTransform", FieldType_Bool, (void*)0, (void*)IsStaticTransform_s, NULL, "", bOverride);
	pClass->AddField("IsTransformationNode", FieldType_Bool, (void*)0, (void*)IsTransformationNode_s, NULL, "", bOverride);
	pClass->AddField("IsDummyNode", FieldType_Bool, (void*)0, (void*)IsDummyNode_s, NULL, "", bOverride);

	pClass->AddField("IsAnimated", FieldType_Bool, (void*)0, (void*)IsAnimated_s, NULL, "", bOverride);
	pClass->AddField("OffsetMatrix", FieldType_Matrix4, (void*)SetOffsetMatrix_s, (void*)0, NULL, "", bOverride);
	pClass->AddField("SetStaticTransform", FieldType_Matrix4, (void*)SetStaticTransform_s, (void*)0, NULL, "", bOverride);
	pClass->AddField("FinalMatrix", FieldType_Matrix4, (void*)0, (void*)GetFinalMatrix_s, NULL, "", bOverride);
	pClass->AddField("FinalRotMatrix", FieldType_Matrix4, (void*)0, (void*)GetFinalRotMatrix_s, NULL, "", bOverride);
	pClass->AddField("PivotRotMatrix", FieldType_Matrix4, (void*)0, (void*)GetPivotRotMatrix_s, NULL, "", bOverride);

	pClass->AddField("PivotPoint", FieldType_Vector3, (void*)0, (void*)GetPivotPoint_s, NULL, "", bOverride);
	pClass->AddField("AnimatedPivotPoint", FieldType_Vector3, (void*)0, (void*)GetAnimatedPivotPoint_s, NULL, "", bOverride);
	pClass->AddField("FinalRot", FieldType_Quaternion, (void*)SetFinalRot_s, (void*)GetFinalRot_s, NULL, "", bOverride);
	pClass->AddField("FinalTrans", FieldType_Vector3, (void*)SetFinalTrans_s, (void*)GetFinalTrans_s, NULL, "", bOverride);
	pClass->AddField("FinalScaling", FieldType_Vector3, (void*)SetFinalScaling_s, (void*)GetFinalScaling_s, NULL, "", bOverride);

	pClass->AddField("ParentIndex", FieldType_Int, (void*)0, (void*)GetParentIndex_s, NULL, "", bOverride);
	pClass->AddField("BoneIndex", FieldType_Int, (void*)0, (void*)GetBoneIndex_s, NULL, "", bOverride);
	pClass->AddField("BoneID", FieldType_Int, (void*)SetBoneID_s, (void*)GetBoneID_s, NULL, "", bOverride);

	return S_OK;
}

Bone& ParaEngine::Bone::operator=(const Bone& other)
{
	m_sIdentifer = other.m_sIdentifer;
	m_sRotName = other.m_sRotName;
	m_sTransName = other.m_sTransName;
	m_sScaleName = other.m_sScaleName;

	trans = other.trans;
	rot = other.rot;
	scale = other.scale;

	pivot = other.pivot;
	matOffset = other.matOffset;
	matTransform = other.matTransform;

	parent = other.parent;
	nBoneID = other.nBoneID;
	nIndex = other.nIndex;

	flags = other.flags;

	mat = other.mat;

	mrot = other.mrot;

	m_finalRot = other.m_finalRot;

	m_finalTrans = other.m_finalTrans;

	m_finalScaling = other.m_finalScaling;

	calc = other.calc;
	bUsePivot = other.bUsePivot;

	mIsUpper = other.mIsUpper;

	return *this;
}

void ParaEngine::Bone::RemoveUnusedAnimKeys()
{
	rot.RemoveUnusedAnimKeys();
	trans.RemoveUnusedAnimKeys();
	scale.RemoveUnusedAnimKeys();
}

Bone* ParaEngine::Bone::FindMatchingBoneInProvider(CBoneAnimProvider* pProvider)
{
	Bone* pCurBone = 0;
	if (this->nBoneID > 0)
	{
		// if the bone is one of the known biped bones, we will find in the external provider.
		// if no such a bone in the provider, we will use the default value (0). 
		pCurBone = pProvider->GetBone((KNOWN_BONE_NODES)(this->nBoneID));
	}
	else
	{
		const auto& sIdentifier = GetIdentifier();
		if (!sIdentifier.empty())
		{
			// since we are assigning the bone index to the identifier when identifier is empty, we will skip the digit identifier.
			bool isDigitIdentifier = true;
			for (size_t i = 0; i < sIdentifier.size(); i++)
			{
				if (!isdigit(sIdentifier[i]))
				{
					isDigitIdentifier = false;
					break;
				}
			}
			if (!isDigitIdentifier)
			{
				pCurBone = pProvider->GetBoneByName(sIdentifier);
			}
		}

		if (pCurBone == 0)
		{
			// if the bone is one of the unknown biped bones, both locally and externally. we will try to use external animation by matching bone index. 
			// if no unknown bone in the external provider with the same index and parent index, we will use the default animation (0). 
			Bone* bone_ = pProvider->GetBoneByIndex(this->nIndex);
			if (bone_ != 0 && bone_->nBoneID <= 0 && this->parent == bone_->parent)
			{
				pCurBone = bone_;
			}
		}
	}
	return pCurBone;
}


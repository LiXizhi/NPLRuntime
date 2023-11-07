//-----------------------------------------------------------------------------
// Class:	CParaXAnimInstance
// Authors:	Li, Xizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2005.9.18
// Revised: 2005.10.8
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ParaWorldAsset.h"
#include "ParaXModel/ParaXModel.h"
#include "ParaXModel/AnimTable.h"
#include "ParaXModel/BoneAnimProvider.h"
#include "EffectManager.h"
#include "SceneState.h"
#include "CustomCharSettings.h"
#include "CustomCharModelInstance.h"
#include "ParaXModelCanvas.h"
#include "ParaXAnimInstance.h"

/** @def default walking animation speed if model does not exist or does not contain a walk animation. */
#define DEFAULT_WALK_SPEED 4.0f

namespace ParaEngine
{
	// global time for global sequences
	extern int64_t globalTime;
}

using namespace ParaEngine;


CParaXAnimInstance::CParaXAnimInstance(void)
	:m_modelType(CharacterModel), m_bUseGlobalTime(false), m_curTime(0), m_fBlendingFactor(0), m_nCurrentIdleAnimationID(ANIM_STAND), m_nCustomStandingAnimCount(-1), mUpperBlendingFactor(0)
{
	// Maybe init from a another function, instead of constructor
	if (m_modelType == CharacterModel)
	{
		m_pCharModel.reset(new CharModelInstance());
	}
}

CParaXAnimInstance::~CParaXAnimInstance(void)
{
}

int CParaXAnimInstance::GetTime()
{
	return m_curTime;
}

void CParaXAnimInstance::SetTime(int nTime)
{
	m_curTime = nTime;
}

void CParaXAnimInstance::SetBlendingFactor(float fBlendingFactor)
{
	m_fBlendingFactor = fBlendingFactor;
}


void CParaXAnimInstance::SetUseGlobalTime(bool bUseGlobalTime)
{
	m_bUseGlobalTime = bUseGlobalTime;
}

bool CParaXAnimInstance::IsUseGlobalTime()
{
	return m_bUseGlobalTime;
}

void CParaXAnimInstance::SetAnimFrame(int nFrame)
{
	if (m_CurrentAnim.IsValid() && !m_CurrentAnim.IsUndetermined())
	{
		int nLength = m_CurrentAnim.nEndFrame - m_CurrentAnim.nStartFrame;
		if (nLength >= 0 && nFrame >= 0)
		{
			if (nFrame <= nLength)
			{
				m_CurrentAnim.nCurrentFrame = m_CurrentAnim.nStartFrame + nFrame;
			}
			else
			{
				m_CurrentAnim.nCurrentFrame = m_CurrentAnim.nStartFrame + ((nLength > 0) ? (nFrame % nLength) : 0);
			}
			m_fBlendingFactor = 0.f;
		}
	}

}

int CParaXAnimInstance::GetAnimFrame()
{
	return m_CurrentAnim.nCurrentFrame - m_CurrentAnim.nStartFrame;
}

int CParaXAnimInstance::GetIdleAnimationID()
{
	return m_nCurrentIdleAnimationID;

}

void CParaXAnimInstance::SetIdleAnimationID(int nID)
{
	m_nCurrentIdleAnimationID = nID;
}

bool CParaXAnimInstance::HasMountPoint(int nMountPointID /*= 0*/)
{
	return HasAttachmentMatrix(nMountPointID);
}

CharModelInstance* CParaXAnimInstance::GetCharModel()
{
	if (m_modelType == CharacterModel)
	{
		return m_pCharModel.get();
	}
	return NULL;
}

void CParaXAnimInstance::Init(ParaXEntity* pModel)
{
	ResetBaseModel(pModel);
}

int CParaXAnimInstance::GetValidAnimID(int nAnimID)
{
	if (m_modelType == CharacterModel)
	{
		ParaXEntity* pModelEnity = GetAnimModel();
		CParaXModel* pModel = (pModelEnity != NULL) ? pModelEnity->GetModel() : NULL;
		if (pModel)
		{
			int nAnimIndex = 0;
			do {
				nAnimIndex = pModel->GetAnimIndexByID(nAnimID).nIndex;
				if (nAnimIndex >= 0 || nAnimID == 0)
					break;
				nAnimID = CAnimTable::GetDefaultAnimIDof(nAnimID);
			} while (true);
			return nAnimID;
		}
	}
	return -1;
}

void ParaEngine::CParaXAnimInstance::SetUpperAnimation(int nAnimID)
{
	if (m_modelType == CharacterModel)
	{
		ParaXEntity* pModelEnity = GetAnimModel();
		CParaXModel* pModel = (pModelEnity != NULL) ? pModelEnity->GetModel() : NULL;
		if (pModel == NULL)
		{
			return;
		}

		if (IsAnimIDMapEnabled())
		{
			map<int, int>::iterator it = GetAnimIDMap()->find(nAnimID);
			if (it != GetAnimIDMap()->end())
			{
				nAnimID = (*it).second;
			}
		}

		if (nAnimID < 0)
		{
			mUpperAnim.MakeInvalid();
			mUpperBlendingAnim.MakeInvalid();
			mUpperBlendingFactor = 0;
		}
		else
		{
			AnimIndex IndexAnim(0);
			bool bHasWalkAnim = false;
			if (nAnimID < 1000)
			{
				// load local model animation
				do {
					if (nAnimID == ANIM_WALK)
						bHasWalkAnim = true;
					IndexAnim = pModel->GetAnimIndexByID(nAnimID);
					if (IndexAnim.IsValid() || nAnimID == 0)
						break;
					nAnimID = CAnimTable::GetDefaultAnimIDof(nAnimID);
				} while (true);
			}
			else
			{
				// load an external animation.
				CBoneAnimProvider* pProvider = CBoneAnimProvider::GetProviderByID(nAnimID);
				if (pProvider)
				{
					IndexAnim = pProvider->GetAnimIndex(pProvider->GetSubAnimID());
				}
			}
			if (IndexAnim.IsValid() && IndexAnim.Provider == 0)
			{
				// scale speed properly
				auto pModelAnim = pModel->GetModelAnimByIndex(IndexAnim.nIndex);
				if (pModelAnim)
				{
					float moveSpeed = pModelAnim->moveSpeed;
					if (bHasWalkAnim && moveSpeed == 0.f)
					{
						// default to 4 meters/seconds in case walk animation is not inside the file. 
						moveSpeed = DEFAULT_WALK_SPEED;
					}
				}
			}
			IndexAnim.nAnimID = nAnimID; // enforce the same ID

			if (mUpperAnim != IndexAnim)
			{
				/// If the current animation is looping and bAppend is false, play immediately from the beginning of the new animation.
				// turn on motion warping

				mUpperBlendingAnim = mUpperAnim;
				if (!mUpperBlendingAnim.IsValid())
					mUpperBlendingAnim = m_CurrentAnim;
				mUpperBlendingFactor = 1.0;

				mUpperAnim = IndexAnim;
				mUpperAnim.Reset();
			}
			else
			{
				mUpperAnim.Reset();
				mUpperBlendingFactor = 0;
			}
		}
	}
	else
	{
		// TODO: other model type goes here
	}
}

int ParaEngine::CParaXAnimInstance::GetUpperAnimation()
{
	return mUpperAnim.IsValid() ? mUpperAnim.nAnimID : -1;
}

bool CParaXAnimInstance::HasAnimId(int nAnimID)
{
	if (m_modelType == CharacterModel)
	{
		ParaXEntity* pModelEnity = GetAnimModel();
		CParaXModel* pModel = (pModelEnity != NULL) ? pModelEnity->GetModel() : NULL;
		if (pModel)
		{
			return pModel->GetAnimIndexByID(nAnimID).IsValid();
		}
	}
	return false;
}

void CParaXAnimInstance::ResetAnimation()
{
	m_CurrentAnim.MakeInvalid();
	m_NextAnim.MakeInvalid();
	m_BlendingAnim.MakeInvalid();
	m_fBlendingFactor = 0;
	m_nCurrentIdleAnimationID = ANIM_STAND;
	m_nCustomStandingAnimCount = -1;
	m_AttachmentMatrices.clear();
	// external animation are stored in dynamic fields using bone names as key. 
	RemoveAllDynamicFields();
	m_curTime = 0;

	mUpperAnim.MakeInvalid();
}

bool CParaXAnimInstance::ResetBaseModel(ParaXEntity* pModel)
{
	if (m_modelType == CharacterModel)
	{
		// animation info are preserved, so never call ResetAnimation() here. 
		// ResetAnimation();
		m_pCharModel->InitBaseModel(pModel);
		ParaXEntity* pModelEnity = GetAnimModel();
		CParaXModel* pModel = (pModelEnity != NULL) ? pModelEnity->GetModel() : NULL;
		if (pModel)
			m_CurrentAnim = pModel->GetAnimIndexByID(GetIdleAnimationID());
		else
			m_CurrentAnim.MakeInvalid();
		m_NextAnim.MakeInvalid();
		m_BlendingAnim.MakeInvalid();
		m_fBlendingFactor = 0;
		m_nCurrentIdleAnimationID = ANIM_STAND;
		m_nCustomStandingAnimCount = -1;

		mUpperAnim.MakeInvalid();
		return true;
	}
	return false;
}

void CParaXAnimInstance::LoadAnimationByIndex(const AnimIndex& IndexAnim, bool bAppend)
{
	if (!IndexAnim.IsValid())
		return;
	if (m_modelType == CharacterModel)
	{
		if (m_CurrentAnim != IndexAnim)
		{
			/// if the animation is different, we will play a new one.

			if ((!bAppend) || m_CurrentAnim.IsLooping())
			{
				/// If the current animation is looping and bAppend is false, play immediately from the beginning of the new animation.
				// turn on motion warping

				m_BlendingAnim = m_CurrentAnim;
				m_fBlendingFactor = 1.0;

				m_CurrentAnim = IndexAnim;
				m_CurrentAnim.Reset();

				/// set the next animation to default
				m_NextAnim.MakeInvalid();
			}
			else
			{
				/// if the current animation is non-looping, we will set the next animation to play to the new animation.
				m_NextAnim = IndexAnim;
				m_NextAnim.Reset();
			}
		}
		else
		{
			m_NextAnim = IndexAnim;
		}
	}
	else
	{
		// TODO: other model type goes here
	}
}

/** get the ID of the current animation.*/
int CParaXAnimInstance::GetCurrentAnimation()
{
	int nID = 0;
	if (m_modelType == CharacterModel)
	{
		nID = m_CurrentAnim.nAnimID;
		if (nID < 0)
			nID = 0;
	}
	else
	{
		// TODO: other model type goes here
	}
	return nID;
}

bool CParaXAnimInstance::HasAlphaBlendedObjects()
{
	if (m_modelType == CharacterModel)
	{
		ParaXEntity* pModelEnity = GetAnimModel();
		CParaXModel* pModel = (pModelEnity != NULL) ? pModelEnity->GetModel() : NULL;
		if (pModel)
		{
			return pModel->HasAlphaBlendedObjects();
		}
	}
	return false;
}

bool CParaXAnimInstance::HasAnimation(int nAnimID)
{
	if (m_modelType == CharacterModel)
	{
		ParaXEntity* pModelEnity = GetAnimModel();
		CParaXModel* pModel = (pModelEnity != NULL) ? pModelEnity->GetModel() : NULL;
		if (pModel == NULL)
			return false;

		if (IsAnimIDMapEnabled())
		{
			map<int, int>::iterator it = GetAnimIDMap()->find(nAnimID);
			if (it != GetAnimIDMap()->end())
			{
				nAnimID = (*it).second;
			}
		}

		if (nAnimID < 0)
			return false;

		AnimIndex IndexAnim(0);

		if (nAnimID < 1000)
		{
			// load local model animation
			IndexAnim = pModel->GetAnimIndexByID(nAnimID);
			if (IndexAnim.IsValid() || nAnimID == 0)
				return true;
		}
		else
		{
			// load an external animation.
			CBoneAnimProvider* pProvider = CBoneAnimProvider::GetProviderByID(nAnimID);
			if (pProvider)
			{
				IndexAnim = pProvider->GetAnimIndex(pProvider->GetSubAnimID());
				if (IndexAnim.IsValid())
				{
					return true;
				}
			}
		}
	}
	else
	{
		// TODO: other model type goes here
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////
// major implementation of loading a given animation ID.
void CParaXAnimInstance::LoadAnimation(int nNextAnimID, float* fSpeed, bool bAppend)
{
	if (m_modelType == CharacterModel)
	{
		ParaXEntity* pModelEnity = GetAnimModel();
		CParaXModel* pModel = (pModelEnity != NULL) ? pModelEnity->GetModel() : NULL;
		if (pModel == NULL)
		{
			if (fSpeed)
			{
				if (CAnimTable::IsWalkAnimation(nNextAnimID))
					*fSpeed = m_fSpeedScale * m_fSizeScale * DEFAULT_WALK_SPEED;
				else
					*fSpeed = 0.f;
			}
			return;
		}

		if (IsAnimIDMapEnabled())
		{
			map<int, int>::iterator it = GetAnimIDMap()->find(nNextAnimID);
			if (it != GetAnimIDMap()->end())
			{
				nNextAnimID = (*it).second;
			}
		}

		if (nNextAnimID < 0)
			return;

		AnimIndex IndexAnim(0);
		bool bHasWalkAnim = false;
		if (nNextAnimID < 1000)
		{
			// load local model animation
			do {
				if (nNextAnimID == ANIM_WALK)
					bHasWalkAnim = true;
				IndexAnim = pModel->GetAnimIndexByID(nNextAnimID);
				if (IndexAnim.IsValid() || nNextAnimID == 0)
					break;
				nNextAnimID = CAnimTable::GetDefaultAnimIDof(nNextAnimID);
			} while (true);
		}
		else
		{
			// load an external animation.
			CBoneAnimProvider* pProvider = CBoneAnimProvider::GetProviderByID(nNextAnimID);
			if (pProvider)
			{
				IndexAnim = pProvider->GetAnimIndex(pProvider->GetSubAnimID());
				if (IndexAnim.IsValid())
				{
					pProvider->GetAnimMoveSpeed(fSpeed, pProvider->GetSubAnimID());
					*fSpeed *= m_fSpeedScale * m_fSizeScale;
				}
			}
		}
		if (IndexAnim.IsValid() && IndexAnim.Provider == 0)
		{
			// scale speed properly
			auto pModelAnim = pModel->GetModelAnimByIndex(IndexAnim.nIndex);
			if (pModelAnim)
			{
				float moveSpeed = pModelAnim->moveSpeed;
				if (bHasWalkAnim && moveSpeed == 0.f)
				{
					// default to 4 meters/seconds in case walk animation is not inside the file. 
					moveSpeed = DEFAULT_WALK_SPEED;
				}
				if (fSpeed)
					*fSpeed = m_fSpeedScale * m_fSizeScale * moveSpeed;
			}
			else
			{
				if (fSpeed)
					*fSpeed = (bHasWalkAnim) ? m_fSpeedScale * m_fSizeScale * DEFAULT_WALK_SPEED : 0.f;
			}
		}
		else
		{
			if (fSpeed)
				*fSpeed = (bHasWalkAnim) ? m_fSpeedScale * m_fSizeScale * DEFAULT_WALK_SPEED : 0.f;
		}
		IndexAnim.nAnimID = nNextAnimID; // enforce the same ID
		LoadAnimationByIndex(IndexAnim, bAppend);
	}
	else
	{
		// TODO: other model type goes here
	}
}

void CParaXAnimInstance::LoadAnimation(const char* sName, float* fSpeed, bool bAppend)
{
	LoadAnimation(CAnimTable::GetAnimIDByName(sName), fSpeed, bAppend);
}

void CParaXAnimInstance::LoadDefaultStandAnim(float* fSpeed)
{
	LoadAnimation(ANIM_STAND, fSpeed);
}
void CParaXAnimInstance::LoadDefaultWalkAnim(float* fSpeed)
{
	LoadAnimation(ANIM_RUN, fSpeed);
}

Matrix4* CParaXAnimInstance::GetAttachmentMatrix(Matrix4* pOut, int nAttachmentID, int nRenderNumber)
{
	ParaXEntity* pModelEnity = GetAnimModel();
	CParaXModel* pModel = (pModelEnity != NULL) ? pModelEnity->GetModel() : NULL;
	if (pModel == NULL)
		return NULL;

	ATTACHMENT_MATRIX_POOL_TYPE::iterator iter = m_AttachmentMatrices.find(nAttachmentID);

	if (iter != m_AttachmentMatrices.end())
	{
		// return the pre-calculated matrix. 
		if (nRenderNumber == 0 || iter->second.m_nRenderNumber == nRenderNumber)
		{
			(*pOut) = iter->second.m_mat;
			return pOut;
		}
	}
	if (pModel->GetAttachmentMatrix(pOut, nAttachmentID, m_CurrentAnim, m_BlendingAnim, m_fBlendingFactor, mUpperAnim, mUpperBlendingAnim, mUpperBlendingFactor))
	{
		// save the attachment matrix. 
		AttachmentMat mat;
		mat.m_mat = *pOut;
		mat.m_nRenderNumber = nRenderNumber;
		m_AttachmentMatrices[nAttachmentID] = mat;
	}
	else
		return NULL;
	return pOut;
}

bool CParaXAnimInstance::HasAttachmentMatrix(int nAttachmentID)
{
	ParaXEntity* pModelEnity = GetAnimModel();
	CParaXModel* pModel = (pModelEnity != NULL) ? pModelEnity->GetModel() : NULL;
	if (pModel == NULL)
		return false;
	return (pModel->HasAttachmentMatrix(nAttachmentID));
}

void CParaXAnimInstance::AdvanceTime(double dTimeDelta)
{
	//if(dTimeDelta<0.000001)
	//	return;
	/// Speed up the animation rendering by a factor of m_fSpeedScale
	dTimeDelta *= m_fSpeedScale;

	if (m_modelType == CharacterModel)
	{
		CharModelInstance* pChar = GetCharModel();
		if (pChar)
			pChar->AdvanceTime(float(dTimeDelta));
		ParaXEntity* pModelEnity = GetAnimModel();
		CParaXModel* pModel = (pModelEnity != NULL) ? pModelEnity->GetModel() : NULL;
		if (pModel == NULL)
			return;
		if (m_CurrentAnim.IsValid())
		{
			if (m_CurrentAnim.nCurrentFrame < (int)m_CurrentAnim.nStartFrame)
				m_CurrentAnim.nCurrentFrame = m_CurrentAnim.nStartFrame;
			if (m_CurrentAnim.nCurrentFrame > (int)m_CurrentAnim.nEndFrame)
				m_CurrentAnim.nCurrentFrame = m_CurrentAnim.nEndFrame;
			int nToDoFrame = m_CurrentAnim.nCurrentFrame + (int)(dTimeDelta * 1000);
			if (m_bUseGlobalTime)
			{
				nToDoFrame = m_CurrentAnim.nStartFrame + ParaEngine::globalTime % (m_CurrentAnim.nEndFrame - m_CurrentAnim.nStartFrame);
				if (m_CurrentAnim.nCurrentFrame > nToDoFrame)
				{
					// looping
					if (m_CurrentAnim.IsUndetermined())
					{
						// if current animation is undetermined, possibly because it is still being loaded, we shall try loading it again. 
						float fSpeed;
						LoadAnimation(m_CurrentAnim.nAnimID, &fSpeed);
					}
				}
			}
			//int nSegLength = m_CurrentAnim.nEndFrame - m_CurrentAnim.nStartFrame;

			// blending factor is decreased
			if (m_fBlendingFactor > 0)
			{
#ifdef _DEBUG
				if (dTimeDelta > 0.033)
				{
					dTimeDelta = 0.033;
				}
#endif
				m_fBlendingFactor -= (float)(dTimeDelta / pModel->fBlendingTime); // BLENDING_TIME blending time
				if (m_fBlendingFactor < 0)
					m_fBlendingFactor = 0;
			}
			// check if we have reached the end frame of the current animation
			if (nToDoFrame > (int)m_CurrentAnim.nEndFrame)
			{
				nToDoFrame = (nToDoFrame - m_CurrentAnim.nStartFrame) % (m_CurrentAnim.nEndFrame - m_CurrentAnim.nStartFrame + 1) + m_CurrentAnim.nStartFrame; // wrap to the beginning

				if (m_NextAnim.IsValid())
				{///  if there is a queued animation, we will play that one.
					if (m_NextAnim == m_CurrentAnim)
					{// if the next animation is the same as the current one,force looping on the current animation
						m_CurrentAnim.nCurrentFrame = nToDoFrame;
					}
					else
					{// play the next animation with motion blending with the current one.
						if (!m_CurrentAnim.IsLooping())
						{
							m_CurrentAnim.nCurrentFrame = m_CurrentAnim.nEndFrame; // this is not necessary ?
						}
						LoadAnimationByIndex(m_NextAnim);
						// m_CurrentAnim.nCurrentFrame = m_CurrentAnim.nStartFrame;// this is not necessary ?
					}
					/// empty the queue
					m_NextAnim.MakeInvalid();
				}
				else
				{///  if there is NO queued animation, we will play the default one.
					if (!m_CurrentAnim.IsLooping())
					{/// non-looping, play the default idle animation
						if (!m_CurrentAnim.IsUndetermined())
						{
							AnimIndex IdleAnimIndex = pModel->GetAnimIndexByID(GetIdleAnimationID());
							if (m_CurrentAnim == IdleAnimIndex)
							{
								m_CurrentAnim.nCurrentFrame = nToDoFrame;
								m_CurrentAnim.AddCycle();
							}
							else
							{
								LoadAnimationByIndex(IdleAnimIndex);
							}
						}
						else
						{
							// if current animation is undetermined, possibly because it is still being loaded, we shall try loading it again. 
							float fSpeed;
							LoadAnimation(m_CurrentAnim.nAnimID, &fSpeed);
						}
					}
					else
					{/// looping on the current animation
						m_CurrentAnim.nCurrentFrame = nToDoFrame;
						m_CurrentAnim.AddCycle();
						if (m_nCustomStandingAnimCount != 0)
						{
							// the character may have alternative standing animations. A character must have the standing animation ANIM_STAND(0), 
							// it may also have up to 4 custom standing animations, in the id range ANIM_CUSTOM0-ANIM_CUSTOM3
							// we will play the custom animations, if any, randomly after the default standing animation is played after some set time(such as 10 seconds).
							if ((m_CurrentAnim.nAnimID == ANIM_STAND && m_CurrentAnim.nFramePlayed > 7000) || (m_CurrentAnim.nAnimID >= ANIM_CUSTOM0 && m_CurrentAnim.nAnimID <= ANIM_CUSTOM3))
							{
								// if it has played over 10 seconds. we will choose one of the custom animations if any.
								if (m_nCustomStandingAnimCount == -1)
								{
									m_nCustomStandingAnimCount = 0;
									for (int k = 0; k < 3; k++)
									{
										AnimIndex IdleAnimIndex = pModel->GetAnimIndexByID(ANIM_CUSTOM0 + k);
										if (IdleAnimIndex.IsValid())
											++m_nCustomStandingAnimCount;
										else
											break;
									}
								}
								if (m_nCustomStandingAnimCount > 0)
								{
									// select a random number [0,m_nCustomStandingAnimCount]
									int nCustomIndex = 0;
									if (pChar == 0 || fabs(pChar->GetCurrrentUpperBodyTurningAngle()) < 0.01)
									{
										nCustomIndex = (rand()) % (m_nCustomStandingAnimCount + 1);
										if (nCustomIndex == m_nCustomStandingAnimCount)
											nCustomIndex = ANIM_STAND;
										else
										{
											nCustomIndex = ANIM_CUSTOM0 + nCustomIndex;
											if (m_CurrentAnim.nAnimID == nCustomIndex)
												nCustomIndex = ANIM_STAND;
										}
									}
									else
									{
										nCustomIndex = ANIM_STAND;
									}

									if (m_CurrentAnim.nAnimID != nCustomIndex)
									{
										AnimIndex IdleAnimIndex = pModel->GetAnimIndexByID(nCustomIndex);
										if (IdleAnimIndex.IsValid())
										{
											LoadAnimationByIndex(IdleAnimIndex);
										}
									}
								}
							}
						}
					}
				}
			}
			else
			{
				m_CurrentAnim.nCurrentFrame = nToDoFrame;
			}
		}

		if (mUpperAnim.IsValid())
		{
			if (mUpperAnim.nCurrentFrame < (int)mUpperAnim.nStartFrame)
				mUpperAnim.nCurrentFrame = mUpperAnim.nStartFrame;
			if (mUpperAnim.nCurrentFrame > (int)mUpperAnim.nEndFrame)
				mUpperAnim.nCurrentFrame = mUpperAnim.nEndFrame;
			int nToDoFrame = mUpperAnim.nCurrentFrame + (int)(dTimeDelta * 1000);
			if (m_bUseGlobalTime)
			{
				nToDoFrame = mUpperAnim.nStartFrame + ParaEngine::globalTime % (mUpperAnim.nEndFrame - mUpperAnim.nStartFrame);
				if (mUpperAnim.nCurrentFrame > nToDoFrame)
				{
					// looping
					if (mUpperAnim.IsUndetermined())
					{
						// if current animation is undetermined, possibly because it is still being loaded, we shall try loading it again. 
						assert(false);
					}
				}
			}
			if (mUpperBlendingFactor > 0)
			{
#ifdef _DEBUG
				if (dTimeDelta > 0.033)
				{
					dTimeDelta = 0.033;
				}
#endif
				mUpperBlendingFactor -= (float)(dTimeDelta / pModel->fBlendingTime); // BLENDING_TIME blending time
				if (mUpperBlendingFactor < 0)
					mUpperBlendingFactor = 0;
			}
			// check if we have reached the end frame of the current animation
			if (nToDoFrame > (int)mUpperAnim.nEndFrame)
			{
				nToDoFrame -= (mUpperAnim.nEndFrame - mUpperAnim.nStartFrame); // wrap to the beginning

				///  if there is NO queued animation, we will play the default one.
				if (!mUpperAnim.IsLooping())
				{/// non-looping, play the default idle animation
					mUpperAnim.MakeInvalid();
				}
				else
				{/// looping on the current animation
					mUpperAnim.nCurrentFrame = nToDoFrame;
					mUpperAnim.AddCycle();
				}
			}
			else
			{
				mUpperAnim.nCurrentFrame = nToDoFrame;
			}
		}
	}
	else
	{
		// TODO: other model type goes here
	}
}

void CParaXAnimInstance::BuildShadowVolume(SceneState* sceneState, ShadowVolume* pShadowVolume, LightParams* pLight, Matrix4* mxWorld)
{
#ifdef USE_DIRECTX_RENDERER
	LPDIRECT3DDEVICE9  pd3dDevice = CGlobals::GetRenderDevice();

	// scale the model
	Matrix4 mat, matScale;
	ParaMatrixScaling(&matScale, m_fSizeScale, m_fSizeScale, m_fSizeScale);
	mat = matScale * (*mxWorld);
	CGlobals::GetWorldMatrixStack().push(mat);

	// draw model
	m_pCharModel->AnimateModel(sceneState, m_CurrentAnim, m_NextAnim, m_BlendingAnim, m_fBlendingFactor, mUpperAnim, mUpperBlendingAnim, mUpperBlendingFactor, NULL);
	m_pCharModel->BuildShadowVolume(sceneState, pShadowVolume, pLight, &mat);

	// pop matrix
	CGlobals::GetWorldMatrixStack().pop();
#endif
}

void CParaXAnimInstance::Animate(double dTimeDelta, int nRenderNumber/*=0 */)
{
	if (GetRenderCount() < nRenderNumber || nRenderNumber == 0)
	{
		if (nRenderNumber != 0)
			SetRenderCount(nRenderNumber);
		if (IsAnimationEnabled())
			AdvanceTime(dTimeDelta);
	}
}

bool CParaXAnimInstance::UpdateModel(SceneState* sceneState)
{
	if (m_modelType == CharacterModel)
	{
		// draw model
		if (m_pCharModel->AnimateModel(sceneState, m_CurrentAnim, m_NextAnim, m_BlendingAnim, m_fBlendingFactor, mUpperAnim, mUpperBlendingAnim, mUpperBlendingFactor, this))
		{
			// update the attachment matrix
			if (!m_AttachmentMatrices.empty() && sceneState)
			{
				int nRenderNumber = sceneState->GetRenderFrameCount();

				ParaXEntity* pModelAsset = m_pCharModel->GetBaseModel();
				if (pModelAsset)
				{
					int nIndex = sceneState->IsLODEnabled() ? pModelAsset->GetLodIndex(sceneState->GetCameraToCurObjectDistance()) : 0;
					CParaXModel* pModel = pModelAsset->GetModel(nIndex);

					if (pModel)
					{
						// update attachment matrix. 
						ATTACHMENT_MATRIX_POOL_TYPE::iterator itCur, itEnd = m_AttachmentMatrices.end();
						for (itCur = m_AttachmentMatrices.begin(); itCur != itEnd; ++itCur)
						{
							Matrix4 maxOut;
							if (itCur->second.m_nRenderNumber != nRenderNumber &&
								pModel->GetAttachmentMatrix(&maxOut, itCur->first, m_CurrentAnim, m_BlendingAnim, m_fBlendingFactor, mUpperAnim, mUpperBlendingAnim, mUpperBlendingFactor, false))
							{
								// save the attachment matrix. 
								itCur->second.m_mat = maxOut;
								itCur->second.m_nRenderNumber = nRenderNumber;
							}
						}
					}
				}
			}
			return true;
		}
	}
	return false;
}


bool CParaXAnimInstance::UpdateWorldTransform(SceneState* sceneState, Matrix4& out, const Matrix4& mxWorld)
{
	if (m_modelType == CharacterModel)
	{
		if (UpdateModel(sceneState))
		{
			out = mxWorld;
			return true;
		}
	}
	return false;
}

HRESULT CParaXAnimInstance::Draw(SceneState* sceneState, const Matrix4* mxWorld, CParameterBlock* materialParams)
{
	if (m_modelType == CharacterModel)
	{
		Matrix4 mat;
		if (UpdateWorldTransform(sceneState, mat, *mxWorld))
		{
			CGlobals::GetWorldMatrixStack().push(mat);
			m_pCharModel->Draw(sceneState, materialParams);
			CGlobals::GetWorldMatrixStack().pop();
		}
	}
	else
	{
		// TODO: other model type goes here
	}
	return S_OK;
}

/// normally this will read the radius of the current animation
/// and calculate the correct size after scaling
void CParaXAnimInstance::GetCurrentRadius(float* fRadius)
{
	if (m_modelType == CharacterModel)
	{
		ParaXEntity* pModelEnity = GetAnimModel();
		CParaXModel* pModel = (pModelEnity != NULL) ? pModelEnity->GetModel() : NULL;
		if (pModel == NULL)
			return;
		int nIndex = (m_CurrentAnim.Provider == 0 && (int)(pModel->GetObjectNum().nAnimations) > m_CurrentAnim.nIndex) ? m_CurrentAnim.nIndex : 0;
		auto pModelAnim = pModel->GetModelAnimByIndex(nIndex);
		if (pModelAnim) {
			float boundsRadius = pModelAnim->rad;
			*fRadius = m_fSizeScale * boundsRadius;
		}
	}
	else
	{
		// TODO: other type goes here
	}
}
/// normally this will read the radius of the current animation
/// and calculate the correct size after scaling
void CParaXAnimInstance::GetCurrentSize(float* fWidth, float* fDepth)
{
	if (m_modelType == CharacterModel)
	{
		ParaXEntity* pModelEnity = GetAnimModel();
		CParaXModel* pModel = (pModelEnity != NULL) ? pModelEnity->GetModel() : NULL;
		if (pModel == NULL)
			return;
		int nIndex = (m_CurrentAnim.Provider == 0 && (int)(pModel->GetObjectNum().nAnimations) > m_CurrentAnim.nIndex) ? m_CurrentAnim.nIndex : 0;
		auto pModelAnim = pModel->GetModelAnimByIndex(nIndex);
		if (pModelAnim) {
			Vector3 box = pModelAnim->boxA - pModelAnim->boxB;
			*fWidth = fabs(m_fSizeScale * box.x);
			*fDepth = fabs(m_fSizeScale * box.y);
		}
	}
	else
	{
		// TODO: other type goes here
	}
}

/// normally this will read the move speed of the current animation
/// and calculate the correct(scaled) speed
void CParaXAnimInstance::GetCurrentSpeed(float* fSpeed)
{
	if (m_modelType == CharacterModel)
	{
		ParaXEntity* pModelEnity = GetAnimModel();
		CParaXModel* pModel = (pModelEnity != NULL) ? pModelEnity->GetModel() : NULL;
		if (pModel == NULL)
			return;
		int nIndex = (m_CurrentAnim.Provider == 0 && (int)(pModel->GetObjectNum().nAnimations) > m_CurrentAnim.nIndex) ? m_CurrentAnim.nIndex : 0;
		auto pModelAnim = pModel->GetModelAnimByIndex(nIndex);
		if (pModelAnim)
		{
			float moveSpeed = pModelAnim->moveSpeed;
			if (fSpeed)
				*fSpeed = m_fSpeedScale * m_fSizeScale * moveSpeed;
		}
	}
	else
	{
		// TODO: other type goes here
	}
}

/// normally this will read the move speed of the specified animation
/// and calculate the correct(scaled) speed
void CParaXAnimInstance::GetSpeedOf(const char* sName, float* fSpeed)
{
	// TODO: find a way to get it by name from database.
	int nAnimID = CAnimTable::GetAnimIDByName(sName);
	if (m_modelType == CharacterModel)
	{
		ParaXEntity* pModelEnity = GetAnimModel();
		CParaXModel* pModel = (pModelEnity != NULL) ? pModelEnity->GetModel() : NULL;
		if (pModel == NULL)
		{
			if (fSpeed)
			{
				if (CAnimTable::IsWalkAnimation(nAnimID))
					*fSpeed = m_fSpeedScale * m_fSizeScale * DEFAULT_WALK_SPEED;
				else
					*fSpeed = 0.f;
			}
			return;
		}


		int nAnimIndex = -1;
		bool bHasWalkAnim = false;
		do {
			if (nAnimID == ANIM_WALK)
				bHasWalkAnim = true;
			nAnimIndex = pModel->GetAnimIndexByID(nAnimID).nIndex;
			if (nAnimIndex >= 0 || nAnimID == 0)
				break;
			nAnimID = CAnimTable::GetDefaultAnimIDof(nAnimID);
		} while (true);

		if (nAnimIndex >= 0)
		{
			auto pModelAnim = pModel->GetModelAnimByIndex(nAnimIndex);
			if (pModelAnim)
			{
				float moveSpeed = pModelAnim->moveSpeed;
				if (bHasWalkAnim && moveSpeed == 0.f)
				{
					// default to 4 meters/seconds in case walk animation is not inside the file. 
					moveSpeed = DEFAULT_WALK_SPEED;
				}
				if (fSpeed)
					*fSpeed = m_fSpeedScale * m_fSizeScale * moveSpeed;
			}
		}
		else
		{
			if (fSpeed)
				*fSpeed = (bHasWalkAnim) ? m_fSpeedScale * m_fSizeScale * DEFAULT_WALK_SPEED : 0.f;
		}
	}
	else
	{
		// TODO: other type goes here
	}

}

ParaXEntity* CParaXAnimInstance::GetAnimModel()
{
	CharModelInstance* pCharInst = GetCharModel();
	return (pCharInst != NULL) ? pCharInst->GetAnimModel() : NULL;
}


IAttributeFields* CParaXAnimInstance::GetChildAttributeObject(const char * sName)
{
	return GetCharModel();
}

IAttributeFields* CParaXAnimInstance::GetChildAttributeObject(int nRowIndex, int nColumnIndex /*= 0*/)
{
	if (nRowIndex == 0 && nColumnIndex == 0)
		return GetCharModel();
	else if (nColumnIndex == 1)
	{
		// bones 
		ParaXEntity* pModel = GetAnimModel();
		if (pModel)
		{
			CParaXModel* pXModel = pModel->GetModel();
			if (pXModel && nRowIndex < (int)pXModel->GetObjectNum().nBones)
			{
				return &(pXModel->bones[nRowIndex]);
			}
		}
	}
	return NULL;
}

int CParaXAnimInstance::GetChildAttributeObjectCount(int nColumnIndex /*= 0*/)
{
	if (nColumnIndex == 0)
	{
		// animation instance
		return 1;
	}
	else if (nColumnIndex == 1)
	{
		// bones
		ParaXEntity* pModel = GetAnimModel();
		if (pModel && pModel->GetModel())
		{
			return pModel->GetModel()->GetObjectNum().nBones;
		}
	}
	return 0;
}

int CParaXAnimInstance::GetChildAttributeColumnCount()
{
	return 2;
}

int CParaXAnimInstance::InstallFields(CAttributeClass* pClass, bool bOverride)
{
	CAnimInstanceBase::InstallFields(pClass, bOverride);

	pClass->AddField("IdleAnimationID", FieldType_Int, (void*)SetIdleAnimationID_s, (void*)GetIdleAnimationID_s, NULL, "", bOverride);
	pClass->AddField("UpdateModel", FieldType_void, (void*)UpdateModel_s, (void*)0, NULL, "", bOverride);

	return S_OK;
}

//-----------------------------------------------------------------------------
// Class:	CMissileObject
// Authors:	Li, Xizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2005.11.12
// Revised: 2005.11.12
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ParaWorldAsset.h"
#include "ParaXModel/ParaXModel.h"
#include "SceneObject.h"

#include "MissileObject.h"
#include "memdebug.h"

using namespace ParaEngine;

CMissileObject::CMissileObject(void)
:m_fModelScale(1.0f), m_pParaXModel(NULL),
m_fTime(0), m_bFired(false), m_fExplotionTime(5.0f),
m_fSpeed(10.0f), m_sDestObjectName(""),m_vTargetPos(0,0,0), m_vCurrentPos(0,0,0)
{
	m_CurrentAnim.MakeInvalid();
	SetPrimaryTechniqueHandle(TECH_CHARACTER);
}

CMissileObject::~CMissileObject(void)
{
}

HRESULT CMissileObject::InitObject(ParaXEntity *model, const string& sDestObject, float fSpeed, float fModelScale)
{
	m_pParaXModel = model;

	// set target
	m_sDestObjectName = sDestObject;

	m_fSpeed = fSpeed;
	m_fModelScale = fModelScale;
	m_bFired = false;
	m_bExploded = false;
	m_fTime = 0;
	if(m_pParaXModel && m_pParaXModel->GetModel())
	{
		SetRadius(0);
	}
	return S_OK;
}

HRESULT CMissileObject::InitObject(ParaXEntity *model, const Vector3& vTargetPos, float fSpeed, float fModelScale)
{
	m_pParaXModel = model;
	
	// set target
	m_sDestObjectName = "";

	m_vTargetPos.x = vTargetPos.x;
	m_vTargetPos.y = vTargetPos.y;
	m_vTargetPos.z = vTargetPos.z;

	m_fSpeed = fSpeed;
	m_fModelScale = fModelScale;
	m_bFired = false;
	m_bExploded = false;
	m_fTime = 0;
	if(m_pParaXModel && m_pParaXModel->GetModel())
	{
		SetRadius(0);
	}
	return S_OK;
}

HRESULT CMissileObject::Draw( SceneState * sceneState)
{
	// for models with particle systems
	sceneState->SetCurrentSceneObject(this);

	// only draw if it is not exploded yet.
	CParaXModel * pModel = NULL;
	if( IsFired()&& !IsExploded() && m_pParaXModel && ((pModel=m_pParaXModel->GetModel())!=NULL))
	{
		// push matrix
		Matrix4  mxWorld, mx;
		// set model facing
		ParaMatrixRotationY( & mxWorld, GetFacing() );

		// set scaling.
		if(m_fModelScale!=1.0f)
		{
			mxWorld._11 *= m_fModelScale;
			mxWorld._22 *= m_fModelScale;
			mxWorld._33 *= m_fModelScale;
		}
		// world translation
		Vector3 vPos = GetRenderOffset();
		mxWorld._41 += vPos.x;
		mxWorld._42 += vPos.y;
		mxWorld._43 += vPos.z;
		CGlobals::GetWorldMatrixStack().push(mxWorld);
		
		/**
		* copy the current instance states to ParaX model
		*/
		pModel->m_CurrentAnim = m_CurrentAnim;
		pModel->m_NextAnim.MakeInvalid();
		pModel->m_BlendingAnim = m_CurrentAnim;
		pModel->blendingFactor = 0;
		pModel->animate(sceneState, NULL);
		pModel->draw(sceneState);
		// pop
		CGlobals::GetWorldMatrixStack().pop();
	}
	return S_OK;
}

void CMissileObject::Animate( double dTimeDelta, int nRenderNumber )
{
	// for models with particle systems
	CGlobals::GetSceneState()->SetCurrentSceneObject(this);

	if(IsExploded())
		return;

	CParaXModel* pModel =  m_pParaXModel->GetModel();
	
	if(pModel && !m_CurrentAnim.IsValid())
	{
		m_CurrentAnim = pModel->GetAnimIndexByID(0);
	}

	// first missile if not yet.
	if(!IsFired())
	{
		m_bFired = true;
		m_fTime = 0;
		// TODO: maybe we should allow user to specified the animation, rather than using the default one.
		if(pModel)
		{
			m_CurrentAnim = pModel->GetAnimIndexByID(0);
		}
	}
	/**
	* check out the animation time.
	*/
	m_fTime += (float)dTimeDelta;
	
	if(m_fTime>= m_fExplotionTime)
	{
		Explode();
		return;
	}
	/**
	* compute the current animation frame
	* it will just loop on the Standing animation.
	*/
	
	if(pModel)
	{
		if(m_CurrentAnim.IsValid())
		{
			//ModelAnimation &animInfo = pModel->anims[m_nCurrentAnim];
			if(m_CurrentAnim.nCurrentFrame<(int)m_CurrentAnim.nStartFrame)
				m_CurrentAnim.nCurrentFrame = m_CurrentAnim.nStartFrame;
			if(m_CurrentAnim.nCurrentFrame>(int)m_CurrentAnim.nEndFrame)
				m_CurrentAnim.nCurrentFrame = m_CurrentAnim.nEndFrame;
			int nToDoFrame = m_CurrentAnim.nCurrentFrame+(int)(dTimeDelta*1000);
			//int nSegLength = m_CurrentAnim.nEndFrame - m_CurrentAnim.nStartFrame;

			// blending factor is decreased
			if(m_blendingFactor >0 )
			{
				m_blendingFactor -= (float)(dTimeDelta/pModel->fBlendingTime); // BLENDING_TIME blending time
				if(m_blendingFactor<0)
					m_blendingFactor = 0;
			}
			// check if we have reached the end frame of the current animation
			if(nToDoFrame>(int)m_CurrentAnim.nEndFrame)
			{
				nToDoFrame -= (m_CurrentAnim.nEndFrame - m_CurrentAnim.nStartFrame); // wrap to the beginning
				/// looping on the current animation
				m_CurrentAnim.nCurrentFrame = nToDoFrame;
			}
			else
			{
				m_CurrentAnim.nCurrentFrame = nToDoFrame;
			}
		}
	}
	/**
	* compute facing and move the missile forward
	*/
	bool bHasTarget = false;
	DVector3 vDest;
	if(m_sDestObjectName.empty())
	{
		vDest = m_vTargetPos;
		bHasTarget = true;
	}
	else
	{
		CBaseObject* pTarget =  CGlobals::GetScene()->GetGlobalObject(m_sDestObjectName);
		if(pTarget)
		{
			// TODO: find a better way to find out the target position, rather than using the object center, which is really weak measures.
			Vector3 vDest_ = pTarget->GetObjectCenter();
			vDest.x = vDest_.x;
			vDest.y = vDest_.y;
			vDest.z = vDest_.z;
			bHasTarget = true;
		}
	}
	if(bHasTarget)
	{
		Vector3 vDir;
		vDir.x = (float)(vDest.x-m_vCurrentPos.x);
		vDir.y = (float)(vDest.y-m_vCurrentPos.y);
		vDir.z = (float)(vDest.z-m_vCurrentPos.z);
		Vector3 vOrigin(0,0,0);
		if(!Math::ComputeFacingTarget(vDir, vOrigin, m_fYaw))
		{
			m_fYaw = 0;
		}
		
		float fStep = m_fSpeed*(float)dTimeDelta;
		if(vDir.squaredLength() <= (fStep*fStep))
		{
			// reached the target, so we shall explode.
			Explode();
		}
		else
		{
			ParaVec3Normalize(&vDir, &vDir);
			vDir = vDir*fStep;
			m_vCurrentPos.x += vDir.x;
			m_vCurrentPos.y += vDir.y;
			m_vCurrentPos.z += vDir.z;

			m_vPos = m_vCurrentPos;
		}
	}
	else
	{
		OUTPUT_LOG("missile target not found.");
		Explode();
	}
}

void CMissileObject::Explode()
{
	// TODO: explode, and delete this missile object. 
	m_bExploded = true;

	// reset the default parameters.
	m_fModelScale = 1.0f;
	m_fTime = 0;
	m_bFired = false;
	m_fExplotionTime = 5.0f;
	m_vPos = Vector3(0,0,0);
	m_vCurrentPos.x = 0;
	m_vCurrentPos.y = 0;
	m_vCurrentPos.z = 0;
	m_fSpeed = 10.0f;
	m_sDestObjectName = "";
	m_CurrentAnim.Reset();
}

IViewClippingObject* CMissileObject::GetViewClippingObject()
{
	return this;
}

void ParaEngine::CMissileObject::SetPosition(const DVector3 & v)
{
	m_vPos = v;
	m_vCurrentPos.x = m_vPos.x;
	m_vCurrentPos.y = m_vPos.y;
	m_vCurrentPos.z = m_vPos.z;
}

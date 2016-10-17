//-----------------------------------------------------------------------------
// Class:	
// Authors:	Clayman@paraengine.com
// Emails:	
// Date:	2007.11
// Revised: 2007.3
// Note: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ParaWorldAsset.h"
#include "SceneObject.h"
#include "AutoCamera.h"
#include "effect_file.h"

#include "CadModelNode.h"



namespace ParaEngine
{
	CadModelNode::CadModelNode()
	{
	}

	CadModelNode::~CadModelNode()
	{

	}

	HRESULT CadModelNode::InitObject(CadModel* pModel)
	{
		m_pModel = pModel;

		if(GetPrimaryTechniqueHandle() < 0)
		{
			SetParamsFromAsset();
		}
		return S_OK;
	}

	void CadModelNode::Cleanup()
	{
	}

	HRESULT CadModelNode::Draw(SceneState *sceneState)
	{
		if(m_pModel == NULL || !m_pModel->IsValid())
			return E_FAIL;

		if(!m_pModel->IsLoaded())
		{
			SetParamsFromAsset();
			m_pModel->LoadAsset();
			return E_FAIL;
		}

		sceneState->SetCurrentSceneObject(this);
		Matrix4 worldMat;
		GetRenderMatrix(worldMat);
		
		CEffectFile* pEffectFile = CGlobals::GetEffectManager()->GetCurrentEffectFile();
		if(pEffectFile != 0)
		{
			
			CGlobals::GetWorldMatrixStack().push(worldMat);
			//sceneState->pd3dDevice->SetRenderState(D3DRS_FILLMODE,D3DFILL_WIREFRAME);
			int currentPass = -1;
			pEffectFile->begin(true);
			//pEffectFile->BeginPass(0);
			        
			for(size_t i=0;i<m_pModel->m_modelParts.size();i++)
			{
				//CadModelPart* modelPart = m_pModel->m_modelParts[i];
				CadModelPart* modelPart = m_pModel->m_modelParts[i];
				if(currentPass != modelPart->GetEffectPassId())
				{
					if(currentPass > -1)
					{
						pEffectFile->EndPass();
					}
					currentPass = modelPart->GetEffectPassId();
					pEffectFile->BeginPass(currentPass);
				}

				pEffectFile->setParameter(CEffectFile::k_diffuseMaterialColor,(const float*)&(m_pModel->m_modelParts[i]->GetDiffuseColor()));	
				pEffectFile->CommitChanges();
				modelPart->SubmitGeometry(sceneState->m_pd3dDevice);
			}
			pEffectFile->EndPass();

			
			CGlobals::GetWorldMatrixStack().pop();
			pEffectFile->end();
		}

		return S_OK;
	}

	void CadModelNode::SetPosition(const DVector3 & v)
	{
		if(m_vPos != v)
		{
			m_vPos = v;
			m_bViewClippingUpdated = false;
		}
	}

	void CadModelNode::SetLocalTransform(const Matrix4& mXForm)
	{
		m_mxLocalTransform = mXForm;
		m_bViewClippingUpdated = false;
	}

	void CadModelNode::SetLocalTransform(float fScale, float fRotX, float fRotY, float fRotZ)
	{
		Matrix4 mx;
		ParaMatrixScaling(&m_mxLocalTransform, fScale, fScale, fScale);
		ParaMatrixRotationZ(&mx, fRotZ);
		m_mxLocalTransform = m_mxLocalTransform*mx; 
		ParaMatrixRotationX(&mx, fRotX);
		m_mxLocalTransform = m_mxLocalTransform*mx; 
		ParaMatrixRotationY(&mx, fRotY);
		m_mxLocalTransform = m_mxLocalTransform*mx;
	}

	void CadModelNode::SetLocalTransform(float fScale, const Quaternion& quat)
	{
		Matrix4 mx;
		ParaMatrixScaling(&m_mxLocalTransform, fScale, fScale, fScale);
		quat.ToRotationMatrix(mx, Vector3::ZERO);
		m_mxLocalTransform = m_mxLocalTransform*mx; 
		m_bViewClippingUpdated = false;
	}
	
	void CadModelNode::GetLocalTransform(Matrix4* localTransform)
	{
		*localTransform = m_mxLocalTransform;
		m_bViewClippingUpdated = false;
	}

	void CadModelNode::GetLocalTransform(float* fScale, Quaternion* quat)
	{
		Vector3 vScale(1,1,1);
		Vector3 vTrans(0,0,0);
		// assume uniform scale
		*fScale = vScale.x; 
		ParaMatrixDecompose(&vScale, quat, &vTrans, &m_mxLocalTransform);
		m_bViewClippingUpdated = false;
	}

	void CadModelNode::SetRotation(const Quaternion& quat)
	{
		Vector3 vScale(1,1,1);
		Vector3 vTrans(0,0,0);
		Quaternion oldQuat;
		if(ParaMatrixDecompose(&vScale, &oldQuat, &vTrans, &m_mxLocalTransform))
		{
			// assume uniform scale by using vScale.x
			ParaMatrixAffineTransformation(&m_mxLocalTransform, vScale.x, NULL, &quat, &vTrans);
		}
		m_bViewClippingUpdated = false;
	}

	void CadModelNode::GetRotation(Quaternion* quat)
	{
		Vector3 vScale(1,1,1);
		Vector3 vTrans(0,0,0);
		ParaMatrixDecompose(&vScale, quat, &vTrans, &m_mxLocalTransform);
		m_bViewClippingUpdated = false;
	}

	void CadModelNode::Rotate(float x, float y, float z)
	{
		Matrix4 mat;
		GetLocalTransform(&mat);
		Matrix4 mat1;
		if(x!=0.f)
			mat = (*ParaMatrixRotationX(&mat1, x))*mat;
		if(y!=0.f)
			mat = (*ParaMatrixRotationY(&mat1, y))*mat;
		if(z!=0.f)
			mat = (*ParaMatrixRotationZ(&mat1, z))*mat;

		SetLocalTransform(mat);
	}

	void CadModelNode::SetScaling(float s)
	{
		Matrix4 mat;
		GetLocalTransform(&mat);
		float fScalingX,fScalingY,fScalingZ;
		Math::GetMatrixScaling(mat, &fScalingX,&fScalingY,&fScalingZ);
		Matrix4 mat1;
		ParaMatrixScaling(&mat1, s/fScalingX,s/fScalingY,s/fScalingZ);
		mat = mat1*mat;
		SetLocalTransform(mat);
	}

	float CadModelNode::GetScaling()
	{
		Matrix4 mat;
		GetLocalTransform(&mat);
		float fScalingX,fScalingY,fScalingZ;
		Math::GetMatrixScaling(mat, &fScalingX,&fScalingY,&fScalingZ);

		return max(max(fScalingX,fScalingY), fScalingZ);
	}

	void CadModelNode::Reset()
	{
		SetLocalTransform(*CGlobals::GetIdentityMatrix());
	}

	Matrix4* CadModelNode::GetRenderMatrix( Matrix4& mxWorld, int nRenderNumber)
	{
		// render offset
		Vector3 vPos = GetRenderOffset();

		// get world transform matrix
		Matrix4 mx;
		float fFacing = GetFacing();

		if(IsBillboarded())
		{
			// TODO: how about in the reflection pass?
			Vector3 vDir = m_vPos - CGlobals::GetScene()->GetCurrentCamera()->GetEyePosition();
			if( vDir.x > 0.0f )
				fFacing += -atanf(vDir.z/vDir.x)+MATH_PI/2;
			else
				fFacing += -atanf(vDir.z/vDir.x)-MATH_PI/2;
		}
		if(fFacing != 0.f)
		{
			ParaMatrixRotationY(&mxWorld, fFacing);/** set facing and rotate local matrix round y axis*/
			mxWorld = m_mxLocalTransform*mxWorld;
		}
		else
		{
			mxWorld = m_mxLocalTransform;
		}

		// world translation
		mxWorld._41 += vPos.x;
		mxWorld._42 += vPos.y;
		mxWorld._43 += vPos.z;
		return &mxWorld;
	}

	bool CadModelNode::SetParamsFromAsset()
	{
		if(m_pModel.get() != 0 && m_pModel->GetPrimaryTechniqueHandle() > 0)
		{
			SetPrimaryTechniqueHandle(TECH_SIMPLE_CAD_MODEL);	
			//SetPrimaryTechniqueHandle(m_pModel->GetPrimaryTechniqueHandle());
			return true;
		}
		SetPrimaryTechniqueHandle(TECH_SIMPLE_CAD_MODEL);
		return false;
	}
	
	
	bool CadModelNode::ViewTouch()
	{
		if(m_pModel == NULL || m_pModel->IsValid())
			return false;
		
		if(GetPrimaryTechniqueHandle()<0)
		{
			SetParamsFromAsset();
			m_pModel->LoadAsset();
			return false;
		}
		return true;
	}


}
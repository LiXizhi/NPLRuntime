//-----------------------------------------------------------------------------
// Class:	ViewCullingObject
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine Tech Studio
// Date:	2004.3.6
// Revised: 2010.7.28 OBB and culling object is cached. 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"

#include "SceneObject.h"
#include "ShapeOBB.h"
#include "ViewCullingObject.h"

#include "memdebug.h"

using namespace ParaEngine;

#ifndef min 
#define min(a,b)  (((a) < (b)) ? (a) : (b))
#endif
#ifndef max
#define max(a,b)  (((a) > (b)) ? (a) : (b))
#endif

CViewCullingObject::CViewCullingObject(void)
:m_vMin(0,0,0), m_vMax(0,0,0), m_vPos(0,0,0), m_fRadius(0.f), m_bound_fY(0.f), m_pShapeOBB(NULL), m_pShapeAABB(NULL)
{
	m_mWorldTransform = Matrix4::IDENTITY;
}

CViewCullingObject::~CViewCullingObject(void)
{
	SAFE_DELETE(m_pShapeOBB);
	SAFE_DELETE(m_pShapeAABB);
}

DVector3 CViewCullingObject::GetPosition()
{
	return m_vPos;
}
void CViewCullingObject::SetAABB(const Vector3 *vMin, const Vector3 *vMax)
{
	m_vMin = *vMin;
	m_vMax = *vMax;
}

void CViewCullingObject::SetTransform(Matrix4* pLocalTransform, const DVector3& vGlobalPos, float* pRotation)
{
	/// get world transform matrix
	Matrix4  mx;
	if(*pRotation == 0.f)
	{
		m_mWorldTransform = *pLocalTransform;
	}
	else
	{
		ParaMatrixRotationY(&m_mWorldTransform, *pRotation);
		m_mWorldTransform = (*pLocalTransform)*m_mWorldTransform;
	}
	/// get global position of the center of the view object 
	Vector3 vMin, vMax;
	ParaVec3TransformCoord( &vMin, &m_vMin, &m_mWorldTransform);
	ParaVec3TransformCoord( &vMax, &m_vMax, &m_mWorldTransform);
	m_vPos = (vMin+vMax)*0.5f;
	m_vPos += vGlobalPos;

	m_mWorldTransform._41 += (float)vGlobalPos.x;
	m_mWorldTransform._42 += (float)vGlobalPos.y;
	m_mWorldTransform._43 += (float)vGlobalPos.z;

	/// get the radius of the view object, we assume that a uniform scale is applied in pLocalTransform.
	float fRadius = (vMax-vMin).squaredLength();

	if(fRadius>0.001f)
		fRadius = sqrt(fRadius)*0.5f;
	// sphere shape is assumed at this stage.
	SetRadius( fRadius ); 

	/// offset the global position to the bottom of the view object.
	m_vPos.y -= GetHeight()*0.5f; 

	///tricky code:  Make OBB invalid, so that it is recomputed next time. 
	if(m_pShapeOBB)
		m_pShapeOBB->mExtents.x = -1.f;
	if(m_pShapeAABB)
		m_pShapeAABB->SetInvalid();
}


void CViewCullingObject::GetRenderVertices(Vector3 * pVertices, int* nNumber)
{
	int nNum = 8;
	*nNumber = nNum;

	pVertices[0].x = m_vMin.x;
	pVertices[0].y = m_vMin.y;
	pVertices[0].z = m_vMin.z;

	pVertices[1].x = m_vMin.x;
	pVertices[1].y = m_vMin.y;
	pVertices[1].z = m_vMax.z;

	pVertices[2].x = m_vMax.x;
	pVertices[2].y = m_vMin.y;
	pVertices[2].z = m_vMax.z;

	pVertices[3].x = m_vMax.x;
	pVertices[3].y = m_vMin.y;
	pVertices[3].z = m_vMin.z;

	pVertices[4].x = m_vMin.x;
	pVertices[4].y = m_vMax.y;
	pVertices[4].z = m_vMin.z;

	pVertices[5].x = m_vMin.x;
	pVertices[5].y = m_vMax.y;
	pVertices[5].z = m_vMax.z;

	pVertices[6].x = m_vMax.x;
	pVertices[6].y = m_vMax.y;
	pVertices[6].z = m_vMax.z;

	pVertices[7].x = m_vMax.x;
	pVertices[7].y = m_vMax.y;
	pVertices[7].z = m_vMin.z;
	
	Matrix4 mat = m_mWorldTransform;

	/// this will make the components of world transform matrix consistent.(i.e. of the same magnitude.)
	/// hence it will correct floating point calculation imprecisions.
	Vector3 vOrig = CGlobals::GetScene()->GetRenderOrigin();
	mat._41 -= vOrig.x;
	mat._42 -= vOrig.y;
	mat._43 -= vOrig.z;

	ParaVec3TransformCoordArray( pVertices,sizeof(Vector3), pVertices, sizeof(Vector3), &mat,nNum);
}

void CViewCullingObject::GetOBB(CShapeOBB* obb)
{
	if(m_pShapeOBB!=0 && m_pShapeOBB->IsValid())
	{
		*obb = * m_pShapeOBB;
	}
	else
	{
		/// finally we will update the OBB. 
		UpdateOBB();
		if(m_pShapeOBB)
		{
			*obb = * m_pShapeOBB;
		}
	}
}


void ParaEngine::CViewCullingObject::SetRadius( FLOAT fRadius )
{
	m_fRadius = fRadius;
	m_bound_fY = fRadius * 2.f;
}

void ParaEngine::CViewCullingObject::UpdateOBB()
{
	if(m_pShapeOBB == 0)
	{
		m_pShapeOBB = new CShapeOBB();
	}
	Matrix4 rotMat = m_mWorldTransform;
	/** Get the scaling factor from globalMat.
	* since we need to create separate physics mesh with different scaling factors even for the same mesh model.
	* it is assumed that components of globalMat satisfies the following equation:
	* |(globalMat._11, globalMat._12, globalMat._13)| = 1;
	* |(globalMat._21, globalMat._22, globalMat._23)| = 1;
	* |(globalMat._31, globalMat._32, globalMat._33)| = 1;
	*/
	float fScalingX,fScalingY,fScalingZ;
	{
		// remove scaling and translation from the transform matrix
		fScalingX = Vector3(rotMat._11, rotMat._12, rotMat._13).length();
		fScalingY = Vector3(rotMat._21, rotMat._22, rotMat._23).length();
		fScalingZ = Vector3(rotMat._31, rotMat._32, rotMat._33).length();
		rotMat._11/=fScalingX;
		rotMat._12/=fScalingX;
		rotMat._13/=fScalingX;
		rotMat._21/=fScalingY;
		rotMat._22/=fScalingY;
		rotMat._23/=fScalingY;
		rotMat._31/=fScalingZ;
		rotMat._32/=fScalingZ;
		rotMat._33/=fScalingZ;
		rotMat._41=0;
		rotMat._42=0;
		rotMat._43=0;
	}

	Vector3 vExtents((m_vMax.x-m_vMin.x)/2, (m_vMax.y-m_vMin.y)/2, (m_vMax.z-m_vMin.z)/2);
	
	vExtents.x *= fScalingX;
	vExtents.y *= fScalingY;
	vExtents.z *= fScalingZ;

	m_pShapeOBB->mCenter = GetObjectCenter();
	m_pShapeOBB->mExtents = vExtents;
	m_pShapeOBB->mRot = rotMat;
}

void ParaEngine::CViewCullingObject::GetAABB( CShapeAABB* aabb )
{
	if(m_pShapeAABB!=0 && m_pShapeAABB->IsValid())
	{
		*aabb = * m_pShapeAABB;
	}
	else
	{
		/// finally we will update the OBB. 
		UpdateAABB();
		if(m_pShapeAABB)
			*aabb = *m_pShapeAABB;
	}
}

Matrix4* ParaEngine::CViewCullingObject::GetWorldTransform(Matrix4& pOut, int nRenderNumber /*= 0*/)
{
	pOut = m_mWorldTransform;
	return &pOut;
}

void ParaEngine::CViewCullingObject::UpdateAABB()
{
	if(m_pShapeAABB == 0)
	{
		m_pShapeAABB= new CShapeAABB();
	}

	Vector3 pVertices[8];

	pVertices[0].x = m_vMin.x;
	pVertices[0].y = m_vMin.y;
	pVertices[0].z = m_vMin.z;

	pVertices[1].x = m_vMin.x;
	pVertices[1].y = m_vMin.y;
	pVertices[1].z = m_vMax.z;

	pVertices[2].x = m_vMax.x;
	pVertices[2].y = m_vMin.y;
	pVertices[2].z = m_vMax.z;

	pVertices[3].x = m_vMax.x;
	pVertices[3].y = m_vMin.y;
	pVertices[3].z = m_vMin.z;

	pVertices[4].x = m_vMin.x;
	pVertices[4].y = m_vMax.y;
	pVertices[4].z = m_vMin.z;

	pVertices[5].x = m_vMin.x;
	pVertices[5].y = m_vMax.y;
	pVertices[5].z = m_vMax.z;

	pVertices[6].x = m_vMax.x;
	pVertices[6].y = m_vMax.y;
	pVertices[6].z = m_vMax.z;

	pVertices[7].x = m_vMax.x;
	pVertices[7].y = m_vMax.y;
	pVertices[7].z = m_vMin.z;

	Matrix4 mat = m_mWorldTransform;
	ParaVec3TransformCoordArray( pVertices,sizeof(Vector3), pVertices, sizeof(Vector3), &mat,8);

	Vector3 vMin = pVertices[0];
	Vector3 vMax = pVertices[6];
	for(int i=0;i<8;++i)
	{
		const Vector3& v = pVertices[i];
		if(v.x < vMin.x)
			vMin.x = v.x;
		if(v.x > vMax.x)
			vMax.x = v.x;

		if(v.y < vMin.y)
			vMin.y = v.y;
		if(v.y > vMax.y)
			vMax.y = v.y;

		if(v.z < vMin.z)
			vMin.z = v.z;
		if(v.z > vMax.z)
			vMax.z = v.z;
	}
	m_pShapeAABB->SetMinMax(vMin, vMax);
}

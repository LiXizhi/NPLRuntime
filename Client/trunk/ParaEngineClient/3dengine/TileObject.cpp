//-----------------------------------------------------------------------------
// Class:	TileObject
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2015.1.22
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "math/ShapeOBB.h"
#include "TileObject.h"

using namespace ParaEngine;

ParaEngine::CTileObject::CTileObject()
	:m_vPos(0, 0, 0), m_fRadius(0), m_fYaw(0), m_aabb(Vector3::ZERO, Vector3::ZERO)
{

}

ParaEngine::CTileObject::~CTileObject()
{

}

void CTileObject::SetBoundingBox(float fOBB_X, float fOBB_Y, float fOBB_Z, float fFacing)
{
	SetObjectShape(_ObjectShape_Box);
	m_aabb.SetWidth(fOBB_X);
	m_aabb.SetHeight(fOBB_Y);
	m_aabb.SetDepth(fOBB_Z);
	m_aabb.GetCenter().y = fOBB_Y * 0.5f;
	m_fRadius = m_aabb.GetRadius();
	m_fYaw = fFacing;
}

void CTileObject::GetBoundingBox(float* fOBB_X, float* fOBB_Y, float* fOBB_Z, float* fFacing)
{
	*fOBB_X = m_aabb.GetWidth();
	*fOBB_Y = m_aabb.GetHeight();
	*fOBB_Z = m_aabb.GetDepth();
	*fFacing = m_fYaw;
}

void CTileObject::SetBoundRect(float fWidth, float fHeight, float fFacing)
{
	SetObjectShape(_ObjectShape_Rectangular);
	m_aabb.SetWidth(fWidth);
	m_aabb.SetHeight(0.f);
	m_aabb.SetDepth(fHeight);
	m_aabb.GetCenter().y = 0;
	m_fRadius = m_aabb.GetRadius();
	m_fYaw = fFacing;
}

void CTileObject::GetBoundRect(float* fWidth, float* fHeight, float* fFacing)
{
	*fWidth = m_aabb.GetWidth();
	*fHeight = m_aabb.GetHeight();
	*fFacing = m_fYaw;
}

void CTileObject::SetRadius(float fRadius)
{
	SetObjectShape(_ObjectShape_Sphere);
	m_aabb.SetExtents(Vector3(fRadius, fRadius, fRadius));
	m_aabb.GetCenter().y = fRadius;
	m_fRadius = fRadius;
}

float CTileObject::GetRadius()
{
	return m_fRadius;
}

float CTileObject::GetHeight()
{
	return m_aabb.GetHeight();
}

void ParaEngine::CTileObject::SetHeight(float fHeight)
{
	SetObjectShape(_ObjectShape_Box);
	m_aabb.SetHeight(fHeight);
	m_aabb.GetCenter().y = fHeight * 0.5f;
	m_fRadius = m_aabb.GetRadius();
}

float ParaEngine::CTileObject::GetWidth()
{
	return m_aabb.GetWidth();
}

void ParaEngine::CTileObject::SetWidth(float fWidth)
{
	SetObjectShape(_ObjectShape_Box);
	m_aabb.SetWidth(fWidth);
	m_fRadius = m_aabb.GetRadius();
}

float ParaEngine::CTileObject::GetDepth()
{
	return m_aabb.GetDepth();
}

void ParaEngine::CTileObject::SetDepth(float fDepth)
{
	SetObjectShape(_ObjectShape_Box);
	m_aabb.SetDepth(fDepth);
	m_fRadius = m_aabb.GetRadius();
}

DVector3 ParaEngine::CTileObject::GetPosition()
{
	return m_vPos;
}

void ParaEngine::CTileObject::SetPosition(const DVector3& v)
{
	m_vPos = v;
}

void ParaEngine::CTileObject::SetAABB(const Vector3 *vMin, const Vector3 *vMax)
{
	m_aabb.SetMinMax(*vMin, *vMax);
	m_fRadius = m_aabb.GetRadius();
}

ParaEngine::Vector3 ParaEngine::CTileObject::GetLocalAABBCenter()
{
	return m_aabb.GetCenter();
}

void ParaEngine::CTileObject::GetOBB(CShapeOBB* obb)
{
	obb->mCenter = GetObjectCenter();
	obb->mExtents = m_aabb.GetExtents();
	obb->mRot = Matrix3::IDENTITY;
}

void ParaEngine::CTileObject::GetAABB(CShapeAABB* aabb)
{
	aabb->SetCenterExtents(GetPosition() + m_aabb.GetCenter(), m_aabb.GetExtents());
}

ParaEngine::DVector3 ParaEngine::CTileObject::GetObjectCenter()
{
	DVector3 v = GetPosition();
	v += m_aabb.GetCenter();
	return v;
}

void ParaEngine::CTileObject::SetObjectCenter(const DVector3 & v)
{
	DVector3 vPos = v;
	vPos.y -= m_aabb.GetExtents(1);
	SetPosition(vPos);
}

void ParaEngine::CTileObject::GetVerticesWithOrigin(const Vector3* vOrigin, Vector3 * pVertices, int* pnNumber)
{
	if (GetObjectShape() == _ObjectShape_Box && GetRadius() > 0.f)
	{
		Vector3 vPos = *vOrigin + m_aabb.GetCenter();
		int nNumber = 8;
		const Vector3& vExtents = m_aabb.GetExtents();
		float fBX = vExtents.x;
		float fBY = vExtents.y; 
		float fBZ = vExtents.z; 
		
		pVertices[0].x = vPos.x + fBX;
		pVertices[0].y = vPos.y - fBY;
		pVertices[0].z = vPos.z + fBZ;

		pVertices[1].x = vPos.x - fBX;
		pVertices[1].y = vPos.y - fBY;
		pVertices[1].z = vPos.z + fBZ;

		pVertices[2].x = vPos.x - fBX;
		pVertices[2].y = vPos.y - fBY;
		pVertices[2].z = vPos.z - fBZ;

		pVertices[3].x = vPos.x + fBX;
		pVertices[3].y = vPos.y - fBY;
		pVertices[3].z = vPos.z - fBZ;

		pVertices[4].x = vPos.x + fBX;
		pVertices[4].y = vPos.y + fBY;
		pVertices[4].z = vPos.z + fBZ;

		pVertices[5].x = vPos.x - fBX;
		pVertices[5].y = vPos.y + fBY;
		pVertices[5].z = vPos.z + fBZ;

		pVertices[6].x = vPos.x - fBX;
		pVertices[6].y = vPos.y + fBY;
		pVertices[6].z = vPos.z - fBZ;

		pVertices[7].x = vPos.x + fBX;
		pVertices[7].y = vPos.y + fBY;
		pVertices[7].z = vPos.z - fBZ;

		*pnNumber = nNumber;
	}
	else
		CBaseObject::GetVerticesWithOrigin(vOrigin, pVertices, pnNumber);
}

float CTileObject::GetYaw()
{
	return m_fYaw;
};
void CTileObject::SetYaw(float fValue)
{
	if (m_fYaw != fValue)
	{
		m_fYaw = fValue;
		SetGeometryDirty(true);
	}
};

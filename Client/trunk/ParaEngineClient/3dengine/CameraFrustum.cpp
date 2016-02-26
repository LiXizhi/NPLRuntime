//-----------------------------------------------------------------------------
// Class:	CCameraFrustum
// Authors:	Li,Xizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2015.12.1
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "SceneObject.h"
#include "ShapeAABB.h"
#include "CameraFrustum.h"

using namespace ParaEngine;


void ParaEngine::CCameraFrustum::UpdateFrustum(const Matrix4* matViewProj, bool bInversedMatrix/*=false*/, float fNearPlane /*= 0.f*/, float fFarPlane /*= 1.f*/)
{
	CShapeFrustum::UpdateFrustum(matViewProj, bInversedMatrix, fNearPlane, fFarPlane);

	Vector3 vNearCenter = (vecFrustum[0] + vecFrustum[3]) / 2;
	m_vEyeOrigin = vNearCenter;
	Vector3 vFarCenter = (vecFrustum[4] + vecFrustum[7]) / 2;
	Vector3 vCenter = vFarCenter - vNearCenter;
	// compute the 2D bounding circle: ignore height.only suitable for quad tree transversal
	m_fViewDepth = vCenter.length();
	vCenter = vCenter / m_fViewDepth;
	// this is related to field of view, just assume a standard 60 degree field of view. 
	v2DCircleRadius = m_fViewDepth * 0.677f;
	// tricky: we will move 10 meters closer to the eye position, so that objects near the eye are more thoroughly covered.
	vCenter = vNearCenter + vCenter*(v2DCircleRadius - 10);
	vCenter += CGlobals::GetScene()->GetRenderOrigin();
	v2DCircleCenter = Vector2(vCenter.x, vCenter.z);
}

bool  ParaEngine::CCameraFrustum::TestBox_sphere_far_plane(const CShapeAABB* box) const
{
	bool intersect = false;

	Vector3 minPt, maxPt;
	box->GetMin(minPt);
	box->GetMax(maxPt);
	// only test 5 planes, except the far plane
	for (int i = 0; i < 5; i++)
	{
		int nV = nVertexLUT[i];
		// pVertex is diagonally opposed to nVertex
		Vector3 nVertex((nV & 1) ? minPt.x : maxPt.x, (nV & 2) ? minPt.y : maxPt.y, (nV & 4) ? minPt.z : maxPt.z);

		if (planeFrustum[i].PlaneDotCoord(nVertex) < 0.f)
			return false;
	}
	// test far plane as if it is a spherical plane

	// the following code is based on ParaMath::intersect(sphere, aabb)
	// Use splitting planes
	Vector3 center = m_vEyeOrigin;
	float radius = m_fViewDepth;
	// Arvo's algorithm
	float s, d = 0;
	for (int k = 0; k < 3; ++k)
	{
		if (center[k] < minPt[k])
		{
			s = center[k] - minPt[k];
			d += s * s;
		}
		else if (center[k] > maxPt[k])
		{
			s = center[k] - maxPt[k];
			d += s * s;
		}
	}
	if (d > radius * radius)
	{
		return false;
	}

	return true;
}

int ParaEngine::CCameraFrustum::TestBoxInsideFarPlane(const CShapeAABB* box) const
{
	bool intersect = false;

	Vector3 minPt, maxPt;
	box->GetMin(minPt);
	box->GetMax(maxPt);

	// test the far plane first, the object must be completely inside far plane
	{
		int i = 5;

		int nV = nVertexLUT[i];
		// pVertex is diagonally opposed to nVertex
		Vector3 nVertex((nV & 1) ? minPt.x : maxPt.x, (nV & 2) ? minPt.y : maxPt.y, (nV & 4) ? minPt.z : maxPt.z);
		Vector3 pVertex((nV & 1) ? maxPt.x : minPt.x, (nV & 2) ? maxPt.y : minPt.y, (nV & 4) ? maxPt.z : minPt.z);

		if (planeFrustum[i].PlaneDotCoord(nVertex) < 0.f)
			return 0;
		if (planeFrustum[i].PlaneDotCoord(pVertex) < 0.f)
		{
			// intersect = true;
			return 0;
		}
	}

	// testing the other 5 planes as usual.
	for (int i = 0; i < 5; i++)
	{
		int nV = nVertexLUT[i];
		// pVertex is diagonally opposed to nVertex
		Vector3 nVertex((nV & 1) ? minPt.x : maxPt.x, (nV & 2) ? minPt.y : maxPt.y, (nV & 4) ? minPt.z : maxPt.z);
		Vector3 pVertex((nV & 1) ? maxPt.x : minPt.x, (nV & 2) ? maxPt.y : minPt.y, (nV & 4) ? maxPt.z : minPt.z);

		if (planeFrustum[i].PlaneDotCoord(nVertex) < 0.f)
			return 0;
		if (planeFrustum[i].PlaneDotCoord(pVertex) < 0.f)
			intersect = true;
	}

	return (intersect) ? 2 : 1;
}
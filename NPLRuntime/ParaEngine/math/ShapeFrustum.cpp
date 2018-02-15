//-----------------------------------------------------------------------------
// Class:	CShapeFrustum
// Authors:	Li, Xizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2006.4
// Note: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ShapeAABB.h"
#include "ShapeSphere.h"

#include "ShapeFrustum.h"

using namespace ParaEngine;

CShapeFrustum::CShapeFrustum()
{
}

CShapeFrustum::CShapeFrustum( const Matrix4* matrix )
{
	UpdateFrustum(matrix);
}

void CShapeFrustum::UpdateFrustum( const Matrix4* matViewProj, bool bInversedMatrix/*=false*/, float fNearPlane /*= 0.f*/, float fFarPlane /*= 1.f*/ )
{
	Matrix4 mat;
	if(bInversedMatrix){
		mat = *matViewProj;
	}else{
		mat = matViewProj->inverse();
	}
#ifdef USE_OPENGL_RENDERER
	// opengl, z axis is in range -1,1
	fNearPlane = fNearPlane * 2.f - 1.f;
#endif
	vecFrustum[0] = Vector3(-1.0f, -1.0f,  fNearPlane); // xyz
	vecFrustum[1] = Vector3( 1.0f, -1.0f,  fNearPlane); // Xyz
	vecFrustum[2] = Vector3(-1.0f,  1.0f,  fNearPlane); // xYz
	vecFrustum[3] = Vector3( 1.0f,  1.0f,  fNearPlane); // XYz
	vecFrustum[4] = Vector3(-1.0f, -1.0f,  fFarPlane); // xyZ
	vecFrustum[5] = Vector3( 1.0f, -1.0f,  fFarPlane); // XyZ
	vecFrustum[6] = Vector3(-1.0f,  1.0f,  fFarPlane); // xYZ
	vecFrustum[7] = Vector3( 1.0f,  1.0f,  fFarPlane); // XYZ


	for (int i = 0; i < 8; i++)
		ParaVec3TransformCoord(&vecFrustum[i], &vecFrustum[i], &mat);
	
	planeFrustum[0].redefine(vecFrustum[0], vecFrustum[1], vecFrustum[2] ); // Near
	planeFrustum[5].redefine(vecFrustum[6], vecFrustum[7], vecFrustum[5] ); // Far
	planeFrustum[1].redefine(vecFrustum[2], vecFrustum[6], vecFrustum[4] ); // Left
	planeFrustum[2].redefine(vecFrustum[7], vecFrustum[3], vecFrustum[5] ); // Right
	planeFrustum[4].redefine(vecFrustum[2], vecFrustum[3], vecFrustum[6] ); // Top
	planeFrustum[3].redefine(vecFrustum[1], vecFrustum[0], vecFrustum[4] ); // Bottom

	//  build a bit-field that will tell us the indices for the nearest and farthest vertices from each plane...
	for (int i=0; i<6; i++)
		nVertexLUT[i] = ((planeFrustum[i].a()<0.f) ? 1 : 0) | ((planeFrustum[i].b()<0.f) ? 2 : 0) | ((planeFrustum[i].c()<0.f) ? 4 : 0);
}

//partially based on nvidia sdk sample code:PracticalPSM
bool CShapeFrustum::SweptSpherePlaneIntersect(float& t0, float& t1, const Plane* plane, const CShapeSphere* sphere, const Vector3* sweepDir)
{
	float b_dot_n = plane->PlaneDotCoord(sphere->mCenter);
	float d_dot_n = plane->PlaneDotNormal(*sweepDir);

	if (d_dot_n == 0.f)
	{
		if (b_dot_n <= sphere->mRadius)
		{
			//  effectively infinity
			t0 = 0.f;
			t1 = 1e32f;
			return true;
		}
		else
			return false;
	}
	else
	{
		float tmp0 = ( sphere->mRadius - b_dot_n) / d_dot_n;
		float tmp1 = (-sphere->mRadius - b_dot_n) / d_dot_n;
		t0 = min(tmp0, tmp1);
		t1 = max(tmp0, tmp1);
		return true;
	}
}


bool CShapeFrustum::TestSweptSphere(const CShapeSphere *sphere, const Vector3 *sweepDir) const
{
	//  algorithm -- get all 12 intersection points of the swept sphere with the view frustum
	//  for all points >0, displace sphere along the sweep direction.  if the displaced sphere
	//  is inside the frustum, return TRUE.  else, return FALSE
	float displacements[12];
	int cnt = 0;
	float a, b;
	bool inFrustum = false;

	for (int i=0; i<6; i++)
	{
		if (SweptSpherePlaneIntersect(a, b, &planeFrustum[i], sphere, sweepDir))
		{
			if (a>=0.f)
				displacements[cnt++] = a;
			if (b>=0.f)
				displacements[cnt++] = b;
		}
	}

	for (int i=0; i<cnt; i++)
	{
		CShapeSphere displacedSphere(*sphere);
		displacedSphere.mCenter += (*sweepDir)*displacements[i];
		displacedSphere.mRadius *= 1.1f;
		inFrustum |= TestSphere(&displacedSphere);
	}
	return inFrustum;
}

bool CShapeFrustum::TestSphere(const CShapeSphere* sphere) const
{
	bool inside = true;
	float radius = sphere->mRadius;

	for (int i=0; (i<6) && inside; i++)
		inside &= ((planeFrustum[i].PlaneDotCoord(sphere->mCenter) + radius) >= 0.f);

	return inside;
}

int CShapeFrustum::TestBox( const CShapeAABB* box ) const
{
	bool intersect = false;

	Vector3 minPt,maxPt;
	box->GetMin(minPt);
	box->GetMax(maxPt);
	for (int i=0; i<6; i++)
	{
		int nV = nVertexLUT[i];
		// pVertex is diagonally opposed to nVertex
		Vector3 nVertex( (nV&1)?minPt.x:maxPt.x, (nV&2)?minPt.y:maxPt.y, (nV&4)?minPt.z:maxPt.z );
		Vector3 pVertex( (nV&1)?maxPt.x:minPt.x, (nV&2)?maxPt.y:minPt.y, (nV&4)?maxPt.z:minPt.z );

		if (planeFrustum[i].PlaneDotCoord(nVertex) < 0.f)
			return 0;
		if (planeFrustum[i].PlaneDotCoord(pVertex) < 0.f)
			intersect = true;
	}

	return (intersect)?2 : 1;
}

bool ParaEngine::CShapeFrustum::CullPointWithPlane( int iPlane, const Vector3* vPos )
{
	return (planeFrustum[iPlane].PlaneDotCoord(*vPos) < 0.f);
}

bool ParaEngine::CShapeFrustum::CullPointsWithFrustum( Vector3* pVecBounds, int nCount )
{
#ifdef INSIDE_FIRST
	// Check boundary vertices against all 6 frustum planes, 
	// and store result (1 if outside) in a bit field
	for( int iPoint = 0; iPoint < nCount; iPoint++ )
	{
		bool bInside = true;
		for( int iPlane = 0; iPlane < 6; iPlane++ )
		{
			if( planeFrustum[iPlane].PlaneDotCoord((pVecBounds[iPoint])) < 0.f)
			{
				// the point is outside
				bInside = false;
				break;
			}
		}
		if( bInside )
			return true;
	}
	return false;
#else // OUTSIDE_FIRST

	// Check boundary vertices against all 6 frustum planes, 
	// and store result (1 if outside) in a bit field
	for( int iPlane = 0; iPlane < 6; iPlane++ )
	{
		int iPoint;
		for( iPoint = 0; iPoint < nCount; iPoint++ )
		{
			if (planeFrustum[iPlane].PlaneDotCoord((pVecBounds[iPoint])) > 0.f)
			{
				// the point is inside the plane
				break;
			}
		}
		if( iPoint>= nCount)
			return false;
	}
	return true;
#endif
}

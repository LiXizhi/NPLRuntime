//-----------------------------------------------------------------------------
// Class:	CShapeSphere
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Date:	2006.4
// Desc: Note:Added the bounding sphere from point array constructor
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ShapeAABB.h"
#include "ShapeSphere.h"

using namespace ParaEngine;


CShapeSphere::CShapeSphere(int nb_verts, const Vector3* verts)
{
	Compute(nb_verts, verts);
}

void	CShapeSphere::Compute(int nb_verts, const Vector3* verts)
{
	PE_ASSERT(nb_verts> 0);
	mRadius = 0.f;
	mCenter = verts[0];

	for (int i=0;i<nb_verts; ++i)
	{
		const Vector3& tmp = verts[i];
		Vector3 cVec = tmp - mCenter;
		float d = cVec.dotProduct(cVec);
		if ( d > mRadius*mRadius )
		{
			d = sqrtf(d);
			float r = 0.5f * (d+mRadius);
			float scale = (r-mRadius) / d;
			mCenter = mCenter + scale*cVec;
			mRadius = r;
		}
	}
}

inline BOOL CShapeSphere::Contains(const CShapeAABB& aabb)	const
{
	// I assume if all 8 box vertices are inside the sphere, so does the whole box.
	// Sounds ok but maybe there's a better way?
	float R2 = mRadius * mRadius;
	Vector3 Max; aabb.GetMax(Max);
	Vector3 Min; aabb.GetMin(Min);
	Vector3 p;
	p.x=Max.x; p.y=Max.y; p.z=Max.z;	if((mCenter-p).squaredLength()>=R2)	return FALSE;
	p.x=Min.x;							if((mCenter-p).squaredLength()>=R2)	return FALSE;
	p.x=Max.x; p.y=Min.y;				if((mCenter-p).squaredLength()>=R2)	return FALSE;
	p.x=Min.x;							if((mCenter-p).squaredLength()>=R2)	return FALSE;
	p.x=Max.x; p.y=Max.y; p.z=Min.z;	if((mCenter-p).squaredLength()>=R2)	return FALSE;
	p.x=Min.x;							if((mCenter-p).squaredLength()>=R2)	return FALSE;
	p.x=Max.x; p.y=Min.y;				if((mCenter-p).squaredLength()>=R2)	return FALSE;
	p.x=Min.x;							if((mCenter-p).squaredLength()>=R2)	return FALSE;

	return TRUE;
}

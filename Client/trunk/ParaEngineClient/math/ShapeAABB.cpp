//-----------------------------------------------------------------------------
// Class:	CShapeAABB and CShapeBox
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Date:	2005.12.21
// Desc: CShapeAABB is partially based on Pierre Terdiman's work in the ODE physics engine.
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "math/ParaMath.h"
#include "ShapeOBB.h"
#include "FPU.h"
#include "ShapeAABB.h"
using namespace ParaEngine;

#define FLT_AS_DW(F) (*(DWORD*)&(F))
#define ALMOST_ZERO(F) ((FLT_AS_DW(F) & 0x7f800000L)==0)

CShapeAABB::CShapeAABB(const CShapeBox& box)
{
	SetMinMax(box.m_Min, box.m_Max);
}


CShapeAABB::CShapeAABB(const Vector3& center, const Vector3& extents)
:mCenter(center), mExtents(extents)
{
}

CShapeAABB::CShapeAABB( const Vector3* points, int nSize)
{
	SetPointAABB(points[0]);
	ExtendByPointList(&(points[1]), nSize-1);
}

CShapeAABB::CShapeAABB(const CShapeAABB* boxes, int nSize)
{
	SetMinMax(boxes[0].GetMin(), boxes[0].GetMax());
	ExtendByAABBList(&(boxes[1]), nSize-1);
}

CShapeAABB::~CShapeAABB(void)
{
}

float CShapeAABB::GetSize() const
{ 
	return Math::MaxVec3(mExtents);
}

BOOL CShapeAABB::Intersect(const CShapeAABB& a) const
{
	float tx = mCenter.x - a.mCenter.x;	float ex = a.mExtents.x + mExtents.x;	if(AIR(tx) > IR(ex))	return FALSE;
	float ty = mCenter.y - a.mCenter.y;	float ey = a.mExtents.y + mExtents.y;	if(AIR(ty) > IR(ey))	return FALSE;
	float tz = mCenter.z - a.mCenter.z;	float ez = a.mExtents.z + mExtents.z;	if(AIR(tz) > IR(ez))	return FALSE;
	return TRUE;
}

bool CShapeAABB::GomezIntersect(const CShapeAABB& a)
{
	Vector3	T = mCenter - a.mCenter;	// Vector from A to B
	return	((fabsf(T.x) <= (a.mExtents.x + mExtents.x))
		&& (fabsf(T.y) <= (a.mExtents.y + mExtents.y))
		&& (fabsf(T.z) <= (a.mExtents.z + mExtents.z)));
}

BOOL CShapeAABB::Intersect(const CShapeAABB& a, DWORD axis)	const
{
	float t = mCenter[axis] - a.mCenter[axis];
	float e = a.mExtents[axis] + mExtents[axis];
	if(AIR(t) > IR(e))	return FALSE;
	return TRUE;
}

void		CShapeAABB::SetEmpty()
{ 
	mCenter=Vector3(0,0,0); 
	mExtents = Vector3(MIN_FLOAT, MIN_FLOAT, MIN_FLOAT);
}

void CShapeAABB::Rotate(const Matrix4& mtx, CShapeAABB& aabb)	const
{
	// Compute new center
	aabb.mCenter = mCenter * mtx;
	
	// Compute new extents. FPU code & CPU code have been interleaved for improved performance.
	Vector3 Ex(mtx.m[0][0] * mExtents.x, mtx.m[0][1] * mExtents.x, mtx.m[0][2] * mExtents.x);
	IR(Ex.x)&=0x7fffffff;	IR(Ex.y)&=0x7fffffff;	IR(Ex.z)&=0x7fffffff;

	Vector3 Ey(mtx.m[1][0] * mExtents.y, mtx.m[1][1] * mExtents.y, mtx.m[1][2] * mExtents.y);
	IR(Ey.x)&=0x7fffffff;	IR(Ey.y)&=0x7fffffff;	IR(Ey.z)&=0x7fffffff;

	Vector3 Ez(mtx.m[2][0] * mExtents.z, mtx.m[2][1] * mExtents.z, mtx.m[2][2] * mExtents.z);
	IR(Ez.x)&=0x7fffffff;	IR(Ez.y)&=0x7fffffff;	IR(Ez.z)&=0x7fffffff;

	aabb.mExtents.x = Ex.x + Ey.x + Ez.x;
	aabb.mExtents.y = Ex.y + Ey.y + Ez.y;
	aabb.mExtents.z = Ex.z + Ey.z + Ez.z;
}

BOOL CShapeAABB::IsValid()	const
{
	// Consistency condition for (Center, Extents) boxes: Extents >= 0
	if(IS_NEGATIVE_FLOAT(mExtents.x))	return FALSE;
	if(IS_NEGATIVE_FLOAT(mExtents.y))	return FALSE;
	if(IS_NEGATIVE_FLOAT(mExtents.z))	return FALSE;
	return TRUE;
}

void CShapeAABB::ExtendByPointList( const Vector3* points, int nSize)
{
	for ( int i=0; i<nSize; i++ )
		Extend( points[i] );
}

void	CShapeAABB::SetInvalid()
{
	mExtents = Vector3(-1.f,-1.f,-1.f);
}

void CShapeAABB::ExtendByAABBList( const CShapeAABB* boxes, int nSize )
{
	for (int i=0; i<nSize; i++) 
	{
		Vector3 Max = boxes[i].GetMax();
		Vector3 Min = boxes[i].GetMin();

		Extend(Max);
		Extend(Min);
	}
}

void ParaEngine::CShapeAABB::Extend(const Vector3& p)
{
	Vector3 Max = mCenter + mExtents;
	Vector3 Min = mCenter - mExtents;

	if (p.x > Max.x)	Max.x = p.x;
	if (p.x < Min.x)	Min.x = p.x;

	if (p.y > Max.y)	Max.y = p.y;
	if (p.y < Min.y)	Min.y = p.y;

	if (p.z > Max.z)	Max.z = p.z;
	if (p.z < Min.z)	Min.z = p.z;

	SetMinMax(Min, Max);
}

void ParaEngine::CShapeAABB::Extend(const CShapeOBB& obb)
{
	Vector3 points[8];
	obb.ComputePoints(points);
	ExtendByPointList(points, 8);
}

float ParaEngine::CShapeAABB::GetRadius()
{
	return mExtents.length();
}

void ParaEngine::CShapeAABB::SetWidth(float fWidth)
{
	mExtents.x = fWidth * 0.5f;
}

void ParaEngine::CShapeAABB::SetHeight(float fHeight)
{
	mExtents.y = fHeight * 0.5f;
}

void ParaEngine::CShapeAABB::SetDepth(float fDepth)
{
	mExtents.z = fDepth * 0.5f;
}

bool ParaEngine::CShapeAABB::IntersectOutside(float* hitDist, const Vector3* origPt, const Vector3* dir, int* pHitSide /*= NULL*/)
{
	Vector3 minPt = GetMin();
	Vector3 maxPt = GetMax();
	Plane sides[6] = { 
		Plane(1, 0, 0, -minPt.x), Plane(-1, 0, 0, maxPt.x),
		Plane(0, 0, 1, -minPt.z), Plane(0, 0, -1, maxPt.z),
		Plane( 0, 1, 0,-minPt.y), Plane( 0,-1, 0, maxPt.y),
	};

	*hitDist = 0.f;  // safe initial value
	Vector3 hitPt = *origPt;

	bool inside = false;

	for ( int i=0; i<6; i++ )
	{
		float cosTheta = sides[i].PlaneDotNormal(*dir);
		float dist = sides[i].PlaneDotCoord(*origPt);

		//  if we're nearly intersecting, just punt and call it an intersection
		if ( ALMOST_ZERO(dist) )
		{
			if(pHitSide)
				*pHitSide = i;
			return true;
		}
		//  skip nearly (&actually) parallel rays, or rays inside the AABB
		if (dist>=-0.00015 || ALMOST_ZERO(cosTheta) ) 
			continue;
		//  only interested in intersections along the ray, not before it.
		*hitDist = -dist / cosTheta;
		if ( *hitDist < 0.f ) continue;

		hitPt = (*hitDist)*(*dir) + (*origPt);

		inside = true;

		for ( int j=0; (j<6) && inside; j++ )
		{
			if ( j==i )
				continue;
			float d = sides[j].PlaneDotCoord(hitPt);

			inside = ((d + 0.00015) >= 0.f);
		}
		if(inside)
		{
			if(pHitSide)
				*pHitSide = i;
			break;
		}
	}

	return inside;
}


bool CShapeAABB::Intersect(float* hitDist, const Vector3* origPt, const Vector3* dir, int* pHitSide )
{
	Vector3 minPt = GetMin();
	Vector3 maxPt = GetMax();
	Plane sides[6] = { 
		Plane( 1, 0, 0,-minPt.x), Plane(-1, 0, 0, maxPt.x),
		Plane( 0, 0, 1,-minPt.z), Plane( 0, 0,-1, maxPt.z),
		Plane( 0, 1, 0,-minPt.y), Plane( 0,-1, 0, maxPt.y),
	};

	*hitDist = 0.f;  // safe initial value
	Vector3 hitPt = *origPt;

	bool inside = false;

	for ( int i=0; i<6; i++ )
	{
		float cosTheta = sides[i].PlaneDotNormal(*dir);
		float dist = sides[i].PlaneDotCoord(*origPt);

		//  if we're nearly intersecting, just punt and call it an intersection
		if ( ALMOST_ZERO(dist) )
		{
			if(pHitSide)
				*pHitSide = i;
			return true;
		}
		//  skip nearly (&actually) parallel rays
		if ( ALMOST_ZERO(cosTheta) ) continue;
		//  only interested in intersections along the ray, not before it.
		*hitDist = -dist / cosTheta;
		if ( *hitDist < 0.f ) continue;

		hitPt = (*hitDist)*(*dir) + (*origPt);

		inside = true;

		for ( int j=0; (j<6) && inside; j++ )
		{
			if ( j==i )
				continue;
			float d = sides[j].PlaneDotCoord(hitPt);

			inside = ((d + 0.00015) >= 0.f);
		}
		if(inside)
		{
			if(pHitSide)
				*pHitSide = i;
			break;
		}
	}

	return inside;        
}

//////////////////////////////////////////////////////////////////////////
//
// CShapeBox
//
//////////////////////////////////////////////////////////////////////////
CShapeBox::CShapeBox(const CShapeBox* boxes, int nSize)
{

}

CShapeBox::CShapeBox( const Vector3* points, int nSize)
{

}

void	CShapeBox::SetEmpty()
{
	m_Min = Vector3(1e33f, 1e33f, 1e33f);
	m_Max = Vector3(-1e33f, -1e33f, -1e33f);
}
bool		CShapeBox::IsValid()
{
	return (m_Min.x<=m_Max.x) && (m_Min.y<=m_Max.y) && (m_Min.z<=m_Max.z);
}
void	CShapeBox::SetCenterExtents(const Vector3& c, const Vector3& e)
{
	m_Min = c-e; 
	m_Max = c+e;
}
void CShapeBox::Translate(const Vector3& offset)
{
	m_Min += offset;
	m_Max += offset;
}
void CShapeBox::Extend(const Vector3& vec)
{
	m_Min.x = min(m_Min.x, vec.x);
	m_Min.y = min(m_Min.y, vec.y);
	m_Min.z = min(m_Min.z, vec.z);
	m_Max.x = max(m_Max.x, vec.x);
	m_Max.y = max(m_Max.y, vec.y);
	m_Max.z = max(m_Max.z, vec.z);
}

void CShapeBox::Extend( const Vector3* points, int nSize)
{
	for ( int i=0; i<nSize; i++ )
		Extend( points[i] );
}

void CShapeBox::Extend( const CShapeBox* boxes, int nSize )
{
	for (int i=0; i<nSize; i++) 
	{
		Extend(boxes[i].GetMax());
		Extend(boxes[i].GetMin());
	}	
}

bool CShapeBox::Intersect(float* hitDist, const Vector3* origPt, const Vector3* dir) const
{
	Plane sides[6] = { Plane( 1, 0, 0,-m_Min.x), Plane(-1, 0, 0, m_Max.x),
		Plane( 0, 1, 0,-m_Min.y), Plane( 0,-1, 0, m_Max.y),
		Plane( 0, 0, 1,-m_Min.z), Plane( 0, 0,-1, m_Max.z) };

	*hitDist = 0.f;  // safe initial value
	Vector3 hitPt = *origPt;

	bool inside = false;

	for ( int i=0; (i<6) && !inside; i++ )
	{
		float cosTheta = sides[i].PlaneDotNormal(*dir);
		float dist = sides[i].PlaneDotCoord(*origPt);

		//  if we're nearly intersecting, just punt and call it an intersection
		if ( ALMOST_ZERO(dist) ) return true;
		//  skip nearly (&actually) parallel rays
		if ( ALMOST_ZERO(cosTheta) ) continue;
		//  only interested in intersections along the ray, not before it.
		*hitDist = -dist / cosTheta;
		if ( *hitDist < 0.f ) continue;

		hitPt = (*hitDist)*(*dir) + (*origPt);

		inside = true;

		for ( int j=0; (j<6) && inside; j++ )
		{
			if ( j==i )
				continue;
			float d = sides[j].PlaneDotCoord(hitPt);

			inside = ((d + 0.00015) >= 0.f);
		}
	}

	return inside;        
}

ParaEngine::CShapeBox::CShapeBox(const CShapeAABB& other) : m_Min(other.GetMin()), m_Max(other.GetMax())
{

}

void ParaEngine::CShapeBox::Extend(const CShapeOBB& obb)
{
	Vector3 points[8];
	obb.ComputePoints(points);
	Extend(points, 8);
}

bool ParaEngine::CShapeBox::Intersect(const CShapeSphere& sphere) const
{
	return Math::intersects(sphere, *this);
}

float ParaEngine::CShapeBox::DistanceToPoint( const Vector3& p, Vector3* where ) const
{
	Vector3 center = 0.5f*(m_Min+m_Max);
	Vector3 length = 0.5f*(m_Max-m_Min);
	// compute coordinates of point in aabb coordinate system
	Vector3 diff = p - center;

	// project test point onto aabb
	float distance = 0.f;
	float delta;

	for( int i=0; i<3; i++ )
	{
		if ( diff[i] < -length[i] )
		{
			delta = diff[i] + length[i];
			distance += delta*delta;
			diff[i] = -length[i];
		}
		else if ( diff[i] > length[i] )
		{
			delta = diff[i] - length[i];
			distance += delta*delta;
			diff[i] = length[i];
		}
	}
	if ( where )
		*where = diff;

	return distance;
}

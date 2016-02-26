//-----------------------------------------------------------------------------
// Class:	CShapePlane
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2005.12.21
// Revised: 2006.4, 2014.8
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "ShapeAABB.h"
#include "ShapePlane.h"

using namespace ParaEngine;

CShapePlane::CShapePlane(float nx, float ny, float nz, float d)
{ 
	Set(nx, ny, nz, d);
}

CShapePlane::CShapePlane(const Vector3& p, const Vector3& n)
{ 
	Set(p, n);
}

CShapePlane::CShapePlane(const Vector3& p0, const Vector3& p1, const Vector3& p2)
{ 
	Set(p0, p1, p2);
}

CShapePlane::CShapePlane(const Vector3& _n, float _d)
{
	n = _n; d = _d;
}

CShapePlane::CShapePlane(const CShapePlane& plane) : 
	n(plane.n), d(plane.d)
{
}

CShapePlane&	CShapePlane::Zero()
{ 
	n = Vector3(0,0,0); 
	d = 0.0f;
	return *this;	
}

CShapePlane&	CShapePlane::Set(float nx, float ny, float nz, float _d)
{
	n = Vector3(nx, ny, nz); 
	d = _d;	
	return *this;
}

CShapePlane& CShapePlane::Set(const Vector3& p0, const Vector3& p1, const Vector3& p2)
{
	Vector3 Edge0 = p1 - p0;
	Vector3 Edge1 = p2 - p0;

	n = Edge0.crossProduct(Edge1);
	n.normalise();
	
	d = - p0.dotProduct(n);
	return	*this;
}

CShapePlane&	CShapePlane::Set(const Vector3& p, const Vector3& _n)
{
	n = _n; 
	d = (-p).dotProduct(n);
	return *this;
}

float	CShapePlane::Distance(const Vector3& p)			const
{ 
	return p.dotProduct(n) + d;
}

bool	CShapePlane::Belongs(const Vector3& p)				const
{ 
	return fabsf(Distance(p)) < PLANE_EPSILON;	
}

void	CShapePlane::Normalize()
{
	float Denom = 1.0f / n.length();
	n.x	*= Denom;
	n.y	*= Denom;
	n.z	*= Denom;
	d	*= Denom;
}

CShapePlane::operator Vector3()	const				
{ 
	return n;
}

CShapePlane::operator Vector4() const
{ 
	return Vector4(n, d);
}

CShapePlane	CShapePlane::operator*(const Matrix4& m)		const
{
	CShapePlane Ret(*this);
	return Ret *= m;
}

CShapePlane&	CShapePlane::operator*=(const Matrix4& m)
{
	Vector3 n2;
	Vector4 tmp;
	tmp = Vector4(n,0)*m;
	n2 = (Vector3)tmp;
	
	tmp = Vector4(-d*n, 1.0f)*m;

	d = - ((Vector3) tmp).dotProduct(n2);
	n = n2;
	return *this;
}

// some code from ogre plane class
CShapePlane::Side ParaEngine::CShapePlane::getSide( const Vector3& rkPoint ) const
{
	float fDistance = Distance(rkPoint);

	if ( fDistance < 0.0 )
		return CShapePlane::NEGATIVE_SIDE;

	if ( fDistance > 0.0 )
		return CShapePlane::POSITIVE_SIDE;

	return CShapePlane::NO_SIDE;
}

ParaEngine::CShapePlane::Side ParaEngine::CShapePlane::getSide( const CShapeAABB& box ) const
{
	if (!box.IsValid()) 
		return NO_SIDE;
	/*if (box.isInfinite())
		return BOTH_SIDE;*/

	Vector3 center, extents;
	box.GetCenter(center);
	box.GetExtents(extents);
	return getSide(center, extents);
}

ParaEngine::CShapePlane::Side ParaEngine::CShapePlane::getSide( const Vector3& centre, const Vector3& halfSize ) const
{
	// Calculate the distance between box centre and the plane
	float dist = Distance(centre);

	// Calculate the maximise allows absolute distance for
	// the distance between box centre and plane
	float maxAbsDist = fabsf(n.x * halfSize.x) + fabsf(n.y * halfSize.y) + fabsf(n.z * halfSize.z);

	if (dist < -maxAbsDist)
		return CShapePlane::NEGATIVE_SIDE;

	if (dist > +maxAbsDist)
		return CShapePlane::POSITIVE_SIDE;

	return CShapePlane::BOTH_SIDE;
}
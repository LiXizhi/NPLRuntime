/** 
author: LiXizhi
date: 2009.4.15
company:ParaEngine
desc: ParaEngine used to use D3DX math library. however, when porting to Linux, I need a cross platform GNU LGPL math library. 
The following one is chosen with some modifications so that they work well with other parts of ParaEngine. 
*/
#include "ParaEngine.h"
#include "ParaPlane.h"
#include "ParaMatrix3.h"
#include "ParaAxisAlignedBox.h" 
#include "ShapeAABB.h" 

namespace ParaEngine {
	//-----------------------------------------------------------------------
	Plane::Plane ()
	{
		normal = Vector3::ZERO;
		d = 0.0;
	}
	//-----------------------------------------------------------------------
	Plane::Plane (const Plane& rhs)
	{
		normal = rhs.normal;
		d = rhs.d;
	}
	//-----------------------------------------------------------------------
	Plane::Plane (const Vector3& rkNormal, float fConstant)
	{
		normal = rkNormal;
		d = -fConstant;
	}
	//---------------------------------------------------------------------
	Plane::Plane (float a, float b, float c, float _d)
		: normal(a, b, c), d(_d)
	{
	}
	//-----------------------------------------------------------------------
	Plane::Plane (const Vector3& rkNormal, const Vector3& rkPoint)
	{
		redefine(rkNormal, rkPoint);
	}
	//-----------------------------------------------------------------------
	Plane::Plane (const Vector3& rkPoint0, const Vector3& rkPoint1,
		const Vector3& rkPoint2)
	{
		redefine(rkPoint0, rkPoint1, rkPoint2);
	}
	//-----------------------------------------------------------------------
	float Plane::getDistance (const Vector3& rkPoint) const
	{
		return normal.dotProduct(rkPoint) + d;
	}
	//-----------------------------------------------------------------------
	Plane::Side Plane::getSide (const Vector3& rkPoint) const
	{
		float fDistance = getDistance(rkPoint);

		if ( fDistance < 0.0f )
			return Plane::NEGATIVE_SIDE;

		if ( fDistance > 0.0f )
			return Plane::POSITIVE_SIDE;

		return Plane::NO_SIDE;
	}


	//-----------------------------------------------------------------------
	Plane::Side Plane::getSide (const AxisAlignedBox& box) const
	{
		if (box.isNull()) 
			return NO_SIDE;
		if (box.isInfinite())
			return BOTH_SIDE;

        return getSide(box.getCenter(), box.getHalfSize());
	}
    //-----------------------------------------------------------------------
    Plane::Side Plane::getSide (const Vector3& centre, const Vector3& halfSize) const
    {
        // Calculate the distance between box centre and the plane
        float dist = getDistance(centre);

        // Calculate the maximise allows absolute distance for
        // the distance between box centre and plane
        float maxAbsDist = normal.absDotProduct(halfSize);

        if (dist < -maxAbsDist)
            return Plane::NEGATIVE_SIDE;

        if (dist > +maxAbsDist)
            return Plane::POSITIVE_SIDE;

        return Plane::BOTH_SIDE;
    }

	Plane::Side Plane::getSide(const CShapeAABB& box) const
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

	//-----------------------------------------------------------------------
	void Plane::redefine(const Vector3& rkPoint0, const Vector3& rkPoint1,
		const Vector3& rkPoint2)
	{
		Vector3 kEdge1 = rkPoint1 - rkPoint0;
		Vector3 kEdge2 = rkPoint2 - rkPoint0;
		normal = kEdge1.crossProduct(kEdge2);
		normal.normalise();
		d = -normal.dotProduct(rkPoint0);
	}
	//-----------------------------------------------------------------------
	void Plane::redefine(const Vector3& rkNormal, const Vector3& rkPoint)
	{
		normal = rkNormal;
		d = -rkNormal.dotProduct(rkPoint);
	}
	//-----------------------------------------------------------------------
	Vector3 Plane::projectVector(const Vector3& p) const
	{
		// We know plane normal is unit length, so use simple method
		Matrix3 xform;
		xform[0][0] = 1.0f - normal.x * normal.x;
		xform[0][1] = -normal.x * normal.y;
		xform[0][2] = -normal.x * normal.z;
		xform[1][0] = -normal.y * normal.x;
		xform[1][1] = 1.0f - normal.y * normal.y;
		xform[1][2] = -normal.y * normal.z;
		xform[2][0] = -normal.z * normal.x;
		xform[2][1] = -normal.z * normal.y;
		xform[2][2] = 1.0f - normal.z * normal.z;
		return xform * p;
	}
	//-----------------------------------------------------------------------
    float Plane::normalise(void)
    {
        float fLength = normal.length();

        // Will also work for zero-sized vectors, but will change nothing
        if (fLength > 1e-08f)
        {
            float fInvLength = 1.0f / fLength;
            normal *= fInvLength;
            d *= fInvLength;
        }

        return fLength;
    }

	Plane::operator Vector3() const
	{
		return normal;
	}

	Plane::operator Vector4() const
	{
		return Vector4(normal, d);
	}

	ParaEngine::Plane Plane::operator*(const Matrix4& m) const
	{
		Plane Ret(*this);
		return Ret *= m;
	}

	Plane& Plane::operator*=(const Matrix4& m)
	{
		Vector3 n2;
		Vector4 tmp;
		tmp = Vector4(normal, 0)*m;
		n2 = (Vector3)tmp;

		tmp = Vector4(-d*normal, 1.0f)*m;

		d = -((Vector3)tmp).dotProduct(n2);
		normal = n2;
		return *this;
	}

	ParaEngine::Plane Plane::PlaneTransform(const Matrix4& M) const
	{
		Plane plane;
		plane.normal.x = M.m[0][0] * normal.x + M.m[1][0] * normal.y + M.m[2][0] * normal.z + M.m[3][0] * d;
		plane.normal.y = M.m[0][1] * normal.x + M.m[1][1] * normal.y + M.m[2][1] * normal.z + M.m[3][1] * d;
		plane.normal.z = M.m[0][2] * normal.x + M.m[1][2] * normal.y + M.m[2][2] * normal.z + M.m[3][2] * d;
		plane.d		   = M.m[0][3] * normal.x + M.m[1][3] * normal.y + M.m[2][3] * normal.z + M.m[3][3] * d;
		return plane;
	}

	ParaEngine::Plane Plane::operator-() const
	{
		return Plane(-normal, -d);
	}

	Plane::operator const float*() const
	{
		return reinterpret_cast<const float*>(this);
	}

	Plane::operator float*()
	{
		return reinterpret_cast<float*>(this);
	}

	//-----------------------------------------------------------------------
	std::ostream& operator<< (std::ostream& o, const Plane& p)
	{
		o << "Plane(normal=" << p.normal << ", d=" << p.d << ")";
		return o;
	}
} // namespace ParaEngine

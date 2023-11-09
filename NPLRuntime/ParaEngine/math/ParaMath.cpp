/** 
author: LiXizhi
date: 2009.4.15, refactored 2014.8
company:ParaEngine
desc: ParaEngine used to use D3DX math library. however, when porting to Linux, I need a cross platform GNU LGPL math library. 
The following one is chosen with some modifications so that they work well with other parts of ParaEngine. 
*/
#include "ParaEngine.h"
#include "asm_math.h"
#include "ParaAxisAlignedBox.h"
#include "ParaRay.h"
#include "ShapeAABB.h"
#include "ShapeSphere.h"
#include "ParaMath.h"

using namespace ParaEngine;

const float Math::POS_INFINITY = std::numeric_limits<float>::infinity();
const float Math::NEG_INFINITY = -std::numeric_limits<float>::infinity();
const float Math::PI = float( 4.0f * atan( 1.0f ) );
const float Math::TWO_PI = float( 2.0f * PI );
const float Math::HALF_PI = float( 0.5 * PI );
const float Math::fDeg2Rad = PI / float(180.0);
const float Math::fRad2Deg = float(180.0) / PI;

int Math::mTrigTableSize;
Math::AngleUnit Math::msAngleUnit;

float  Math::mTrigTableFactor;
float *Math::mSinTable = NULL;
float *Math::mTanTable = NULL;

// create an instance for static tables.
Math s_math;

//-----------------------------------------------------------------------
Math::Math( unsigned int trigTableSize )
{
	msAngleUnit = AU_DEGREE;

	mTrigTableSize = trigTableSize;
	mTrigTableFactor = mTrigTableSize / Math::TWO_PI;

	mSinTable = new float[mTrigTableSize];
	mTanTable = new float[mTrigTableSize];

	buildTrigTables();
}

//-----------------------------------------------------------------------
Math::~Math()
{
	delete [](mSinTable);
	delete [](mTanTable);
}

//-----------------------------------------------------------------------
void Math::buildTrigTables(void)
{
	// Build trig lookup tables
	// Could get away with building only PI sized Sin table but simpler this 
	// way. Who cares, it'll ony use an extra 8k of memory anyway and I like 
	// simplicity.
	float angle;
	for (int i = 0; i < mTrigTableSize; ++i)
	{
		angle = Math::TWO_PI * i / mTrigTableSize;
		mSinTable[i] = sin(angle);
		mTanTable[i] = tan(angle);
	}
}
//-----------------------------------------------------------------------	
float Math::SinTable (float fValue)
{
	// Convert range to index values, wrap if required
	int idx;
	if (fValue >= 0)
	{
		idx = int(fValue * mTrigTableFactor) % mTrigTableSize;
	}
	else
	{
		idx = mTrigTableSize - (int(-fValue * mTrigTableFactor) % mTrigTableSize) - 1;
	}

	return mSinTable[idx];
}
//-----------------------------------------------------------------------
float Math::TanTable (float fValue)
{
	// Convert range to index values, wrap if required
	int idx = int(fValue *= mTrigTableFactor) % mTrigTableSize;
	return mTanTable[idx];
}
//-----------------------------------------------------------------------
int Math::ISign (int iValue)
{
	return ( iValue > 0 ? +1 : ( iValue < 0 ? -1 : 0 ) );
}
//-----------------------------------------------------------------------
Radian Math::ACos (float fValue)
{
	if ( -1.0f < fValue )
	{
		if ( fValue < 1.0f )
			return Radian(acos(fValue));
		else
			return Radian(0.0);
	}
	else
	{
		return Radian(PI);
	}
}
//-----------------------------------------------------------------------
Radian Math::ASin (float fValue)
{
	if ( -1.0f < fValue )
	{
		if ( fValue < 1.0f )
			return Radian(asin(fValue));
		else
			return Radian(HALF_PI);
	}
	else
	{
		return Radian(-HALF_PI);
	}
}
//-----------------------------------------------------------------------
float Math::Sign (float fValue)
{
	if ( fValue > 0.0f )
		return 1.0;

	if ( fValue < 0.0f )
		return -1.0;

	return 0.0;
}
//-----------------------------------------------------------------------
float Math::InvSqrt(float fValue)
{
	return float(asm_rsq(fValue));
}
//-----------------------------------------------------------------------
float Math::UnitRandom ()
{
	return asm_rand() / asm_rand_max();
}

//-----------------------------------------------------------------------
float Math::RangeRandom (float fLow, float fHigh)
{
	return (fHigh-fLow)*UnitRandom() + fLow;
}

//-----------------------------------------------------------------------
float Math::SymmetricRandom ()
{
	return 2.0f * UnitRandom() - 1.0f;
}

//-----------------------------------------------------------------------
void Math::setAngleUnit(Math::AngleUnit unit)
{
	msAngleUnit = unit;
}
//-----------------------------------------------------------------------
Math::AngleUnit Math::getAngleUnit(void)
{
	return msAngleUnit;
}
//-----------------------------------------------------------------------
float Math::AngleUnitsToRadians(float angleunits)
{
	if (msAngleUnit == AU_DEGREE)
		return angleunits * fDeg2Rad;
	else
		return angleunits;
}

//-----------------------------------------------------------------------
float Math::RadiansToAngleUnits(float radians)
{
	if (msAngleUnit == AU_DEGREE)
		return radians * fRad2Deg;
	else
		return radians;
}

//-----------------------------------------------------------------------
float Math::AngleUnitsToDegrees(float angleunits)
{
	if (msAngleUnit == AU_RADIAN)
		return angleunits * fRad2Deg;
	else
		return angleunits;
}

//-----------------------------------------------------------------------
float Math::DegreesToAngleUnits(float degrees)
{
	if (msAngleUnit == AU_RADIAN)
		return degrees * fDeg2Rad;
	else
		return degrees;
}

//-----------------------------------------------------------------------
bool Math::pointInTri2D(const Vector2& p, const Vector2& a, 
	const Vector2& b, const Vector2& c)
{
	// Winding must be consistent from all edges for point to be inside
	Vector2 v1, v2;
	float dot[3];
	bool zeroDot[3];

	v1 = b - a;
	v2 = p - a;

	// Note we don't care about normalisation here since sign is all we need
	// It means we don't have to worry about magnitude of cross products either
	dot[0] = v1.crossProduct(v2);
	zeroDot[0] = Math::RealEqual(dot[0], 0.0f, 1e-3f);


	v1 = c - b;
	v2 = p - b;

	dot[1] = v1.crossProduct(v2);
	zeroDot[1] = Math::RealEqual(dot[1], 0.0f, 1e-3f);

	// Compare signs (ignore colinear / coincident points)
	if(!zeroDot[0] && !zeroDot[1] 
	&& Math::Sign(dot[0]) != Math::Sign(dot[1]))
	{
		return false;
	}

	v1 = a - c;
	v2 = p - c;

	dot[2] = v1.crossProduct(v2);
	zeroDot[2] = Math::RealEqual(dot[2], 0.0f, 1e-3f);
	// Compare signs (ignore colinear / coincident points)
	if((!zeroDot[0] && !zeroDot[2] 
	&& Math::Sign(dot[0]) != Math::Sign(dot[2])) ||
		(!zeroDot[1] && !zeroDot[2] 
	&& Math::Sign(dot[1]) != Math::Sign(dot[2])))
	{
		return false;
	}


	return true;
}
//-----------------------------------------------------------------------
bool Math::pointInTri3D(const Vector3& p, const Vector3& a, 
	const Vector3& b, const Vector3& c, const Vector3& normal)
{
	// Winding must be consistent from all edges for point to be inside
	Vector3 v1, v2;
	float dot[3];
	bool zeroDot[3];

	v1 = b - a;
	v2 = p - a;

	// Note we don't care about normalisation here since sign is all we need
	// It means we don't have to worry about magnitude of cross products either
	dot[0] = v1.crossProduct(v2).dotProduct(normal);
	zeroDot[0] = Math::RealEqual(dot[0], 0.0f, 1e-3f);


	v1 = c - b;
	v2 = p - b;

	dot[1] = v1.crossProduct(v2).dotProduct(normal);
	zeroDot[1] = Math::RealEqual(dot[1], 0.0f, 1e-3f);

	// Compare signs (ignore colinear / coincident points)
	if(!zeroDot[0] && !zeroDot[1] 
	&& Math::Sign(dot[0]) != Math::Sign(dot[1]))
	{
		return false;
	}

	v1 = a - c;
	v2 = p - c;

	dot[2] = v1.crossProduct(v2).dotProduct(normal);
	zeroDot[2] = Math::RealEqual(dot[2], 0.0f, 1e-3f);
	// Compare signs (ignore colinear / coincident points)
	if((!zeroDot[0] && !zeroDot[2] 
	&& Math::Sign(dot[0]) != Math::Sign(dot[2])) ||
		(!zeroDot[1] && !zeroDot[2] 
	&& Math::Sign(dot[1]) != Math::Sign(dot[2])))
	{
		return false;
	}


	return true;
}
//-----------------------------------------------------------------------
bool Math::RealEqual( float a, float b, float tolerance )
{
	if (fabs(b-a) <= tolerance)
		return true;
	else
		return false;
}

//-----------------------------------------------------------------------
std::pair<bool, float> Math::intersects(const Ray& ray, const Plane& plane)
{

	float denom = plane.normal.dotProduct(ray.getDirection());
	if (Math::Abs(denom) < std::numeric_limits<float>::epsilon())
	{
		// Parallel
		return std::pair<bool, float>(false, 0.f);
	}
	else
	{
		float nom = plane.normal.dotProduct(ray.getOrigin()) + plane.d;
		float t = -(nom/denom);
		return std::pair<bool, float>(t >= 0, t);
	}

}
//-----------------------------------------------------------------------
std::pair<bool, float> Math::intersects(const Ray& ray, 
	const std::vector<Plane>& planes, bool normalIsOutside)
{
	std::list<Plane> planesList;
	for (std::vector<Plane>::const_iterator i = planes.begin(); i != planes.end(); ++i)
	{
		planesList.push_back(*i);
	}
	return intersects(ray, planesList, normalIsOutside);
}
//-----------------------------------------------------------------------
std::pair<bool, float> Math::intersects(const Ray& ray, 
	const std::list<Plane>& planes, bool normalIsOutside)
{
	std::list<Plane>::const_iterator planeit, planeitend;
	planeitend = planes.end();
	bool allInside = true;
	std::pair<bool, float> ret;
	std::pair<bool, float> end;
	ret.first = false;
	ret.second = 0.0f;
	end.first = false;
	end.second = 0;


	// derive side
	// NB we don't pass directly since that would require Plane::Side in 
	// interface, which results in recursive includes since Math is so fundamental
	Plane::Side outside = normalIsOutside ? Plane::POSITIVE_SIDE : Plane::NEGATIVE_SIDE;

	for (planeit = planes.begin(); planeit != planeitend; ++planeit)
	{
		const Plane& plane = *planeit;
		// is origin outside?
		if (plane.getSide(ray.getOrigin()) == outside)
		{
			allInside = false;
			// Test single plane
			std::pair<bool, float> planeRes = 
				ray.intersects(plane);
			if (planeRes.first)
			{
				// Ok, we intersected
				ret.first = true;
				// Use the most distant result since convex volume
				ret.second = max(ret.second, planeRes.second);
			}
			else
			{
				ret.first =false;
				ret.second=0.0f;
				return ret;
			}
		}
		else
		{
			std::pair<bool, float> planeRes = 
				ray.intersects(plane);
			if (planeRes.first)
			{
				if( !end.first )
				{
					end.first = true;
					end.second = planeRes.second;
				}
				else
				{
					end.second = min( planeRes.second, end.second );
				}

			}

		}
	}

	if (allInside)
	{
		// Intersecting at 0 distance since inside the volume!
		ret.first = true;
		ret.second = 0.0f;
		return ret;
	}

	if( end.first )
	{
		if( end.second < ret.second )
		{
			ret.first = false;
			return ret;
		}
	}
	return ret;
}
//-----------------------------------------------------------------------
std::pair<bool, float> Math::intersects(const Ray& ray, const Sphere& sphere, 
	bool discardInside)
{
	const Vector3& raydir = ray.getDirection();
	// Adjust ray origin relative to sphere center
	const Vector3& rayorig = ray.getOrigin() - sphere.getCenter();
	float radius = sphere.getRadius();

	// Check origin inside first
	if (rayorig.squaredLength() <= radius*radius && discardInside)
	{
		return std::pair<bool, float>(true, 0.f);
	}

	// Mmm, quadratics
	// Build coeffs which can be used with std quadratic solver
	// ie t = (-b +/- sqrt(b*b + 4ac)) / 2a
	float a = raydir.dotProduct(raydir);
	float b = 2 * rayorig.dotProduct(raydir);
	float c = rayorig.dotProduct(rayorig) - radius*radius;

	// Calc determinant
	float d = (b*b) - (4 * a * c);
	if (d < 0)
	{
		// No intersection
		return std::pair<bool, float>(false, 0.f);
	}
	else
	{
		// BTW, if d=0 there is one intersection, if d > 0 there are 2
		// But we only want the closest one, so that's ok, just use the 
		// '-' version of the solver
		float t = ( -b - Math::Sqrt(d) ) / (2 * a);
		if (t < 0)
			t = ( -b + Math::Sqrt(d) ) / (2 * a);
		return std::pair<bool, float>(true, t);
	}


}
//-----------------------------------------------------------------------
std::pair<bool, float> Math::intersects(const Ray& ray, const AxisAlignedBox& box)
{
	if (box.isNull()) return std::pair<bool, float>(false, 0.f);
	if (box.isInfinite()) return std::pair<bool, float>(true, 0.f);

	float lowt = 0.0f;
	float t;
	bool hit = false;
	Vector3 hitpoint;
	const Vector3& min = box.getMinimum();
	const Vector3& max = box.getMaximum();
	const Vector3& rayorig = ray.getOrigin();
	const Vector3& raydir = ray.getDirection();

	// Check origin inside first
	if ( rayorig > min && rayorig < max )
	{
		return std::pair<bool, float>(true, 0.f);
	}

	// Check each face in turn, only check closest 3
	// Min x
	if (rayorig.x <= min.x && raydir.x > 0)
	{
		t = (min.x - rayorig.x) / raydir.x;
		if (t >= 0)
		{
			// Substitute t back into ray and check bounds and dist
			hitpoint = rayorig + raydir * t;
			if (hitpoint.y >= min.y && hitpoint.y <= max.y &&
				hitpoint.z >= min.z && hitpoint.z <= max.z &&
				(!hit || t < lowt))
			{
				hit = true;
				lowt = t;
			}
		}
	}
	// Max x
	if (rayorig.x >= max.x && raydir.x < 0)
	{
		t = (max.x - rayorig.x) / raydir.x;
		if (t >= 0)
		{
			// Substitute t back into ray and check bounds and dist
			hitpoint = rayorig + raydir * t;
			if (hitpoint.y >= min.y && hitpoint.y <= max.y &&
				hitpoint.z >= min.z && hitpoint.z <= max.z &&
				(!hit || t < lowt))
			{
				hit = true;
				lowt = t;
			}
		}
	}
	// Min y
	if (rayorig.y <= min.y && raydir.y > 0)
	{
		t = (min.y - rayorig.y) / raydir.y;
		if (t >= 0)
		{
			// Substitute t back into ray and check bounds and dist
			hitpoint = rayorig + raydir * t;
			if (hitpoint.x >= min.x && hitpoint.x <= max.x &&
				hitpoint.z >= min.z && hitpoint.z <= max.z &&
				(!hit || t < lowt))
			{
				hit = true;
				lowt = t;
			}
		}
	}
	// Max y
	if (rayorig.y >= max.y && raydir.y < 0)
	{
		t = (max.y - rayorig.y) / raydir.y;
		if (t >= 0)
		{
			// Substitute t back into ray and check bounds and dist
			hitpoint = rayorig + raydir * t;
			if (hitpoint.x >= min.x && hitpoint.x <= max.x &&
				hitpoint.z >= min.z && hitpoint.z <= max.z &&
				(!hit || t < lowt))
			{
				hit = true;
				lowt = t;
			}
		}
	}
	// Min z
	if (rayorig.z <= min.z && raydir.z > 0)
	{
		t = (min.z - rayorig.z) / raydir.z;
		if (t >= 0)
		{
			// Substitute t back into ray and check bounds and dist
			hitpoint = rayorig + raydir * t;
			if (hitpoint.x >= min.x && hitpoint.x <= max.x &&
				hitpoint.y >= min.y && hitpoint.y <= max.y &&
				(!hit || t < lowt))
			{
				hit = true;
				lowt = t;
			}
		}
	}
	// Max z
	if (rayorig.z >= max.z && raydir.z < 0)
	{
		t = (max.z - rayorig.z) / raydir.z;
		if (t >= 0)
		{
			// Substitute t back into ray and check bounds and dist
			hitpoint = rayorig + raydir * t;
			if (hitpoint.x >= min.x && hitpoint.x <= max.x &&
				hitpoint.y >= min.y && hitpoint.y <= max.y &&
				(!hit || t < lowt))
			{
				hit = true;
				lowt = t;
			}
		}
	}

	return std::pair<bool, float>(hit, lowt);

} 
//-----------------------------------------------------------------------
bool Math::intersects(const Ray& ray, const AxisAlignedBox& box,
	float* d1, float* d2)
{
	if (box.isNull())
		return false;

	if (box.isInfinite())
	{
		if (d1) *d1 = 0;
		if (d2) *d2 = Math::POS_INFINITY;
		return true;
	}

	const Vector3& min = box.getMinimum();
	const Vector3& max = box.getMaximum();
	const Vector3& rayorig = ray.getOrigin();
	const Vector3& raydir = ray.getDirection();

	Vector3 absDir;
	absDir[0] = Math::Abs(raydir[0]);
	absDir[1] = Math::Abs(raydir[1]);
	absDir[2] = Math::Abs(raydir[2]);

	// Sort the axis, ensure check minimise floating error axis first
	int imax = 0, imid = 1, imin = 2;
	if (absDir[0] < absDir[2])
	{
		imax = 2;
		imin = 0;
	}
	if (absDir[1] < absDir[imin])
	{
		imid = imin;
		imin = 1;
	}
	else if (absDir[1] > absDir[imax])
	{
		imid = imax;
		imax = 1;
	}

	float start = 0, end = Math::POS_INFINITY;

#define _CALC_AXIS(i)                                       \
do {                                                    \
float denom = 1 / raydir[i];                         \
float newstart = (min[i] - rayorig[i]) * denom;      \
float newend = (max[i] - rayorig[i]) * denom;        \
if (newstart > newend) std::swap(newstart, newend); \
if (newstart > end || newend < start) return false; \
if (newstart > start) start = newstart;             \
if (newend < end) end = newend;                     \
} while(0)

	// Check each axis in turn

	_CALC_AXIS(imax);

	if (absDir[imid] < std::numeric_limits<float>::epsilon())
	{
		// Parallel with middle and minimise axis, check bounds only
		if (rayorig[imid] < min[imid] || rayorig[imid] > max[imid] ||
			rayorig[imin] < min[imin] || rayorig[imin] > max[imin])
			return false;
	}
	else
	{
		_CALC_AXIS(imid);

		if (absDir[imin] < std::numeric_limits<float>::epsilon())
		{
			// Parallel with minimise axis, check bounds only
			if (rayorig[imin] < min[imin] || rayorig[imin] > max[imin])
				return false;
		}
		else
		{
			_CALC_AXIS(imin);
		}
	}
#undef _CALC_AXIS

	if (d1) *d1 = start;
	if (d2) *d2 = end;

	return true;
}
//-----------------------------------------------------------------------
std::pair<bool, float> Math::intersects(const Ray& ray, const Vector3& a,
	const Vector3& b, const Vector3& c, const Vector3& normal,
	bool positiveSide, bool negativeSide)
{
	//
	// Calculate intersection with plane.
	//
	float t;
	{
		float denom = normal.dotProduct(ray.getDirection());

		// Check intersect side
		if (denom > + std::numeric_limits<float>::epsilon())
		{
			if (!negativeSide)
				return std::pair<bool, float>(false, 0.f);
		}
		else if (denom < - std::numeric_limits<float>::epsilon())
		{
			if (!positiveSide)
				return std::pair<bool, float>(false, 0.f);
		}
		else
		{
			// Parallel or triangle area is close to zero when
			// the plane normal not normalised.
			return std::pair<bool, float>(false, 0.f);
		}

		t = normal.dotProduct(a - ray.getOrigin()) / denom;

		if (t < 0)
		{
			// Intersection is behind origin
			return std::pair<bool, float>(false, 0.f);
		}
	}

	//
	// Calculate the largest area projection plane in X, Y or Z.
	//
	size_t i0, i1;
	{
		float n0 = Math::Abs(normal[0]);
		float n1 = Math::Abs(normal[1]);
		float n2 = Math::Abs(normal[2]);

		i0 = 1; i1 = 2;
		if (n1 > n2)
		{
			if (n1 > n0) i0 = 0;
		}
		else
		{
			if (n2 > n0) i1 = 0;
		}
	}

	//
	// Check the intersection point is inside the triangle.
	//
	{
		float u1 = b[i0] - a[i0];
		float v1 = b[i1] - a[i1];
		float u2 = c[i0] - a[i0];
		float v2 = c[i1] - a[i1];
		float u0 = t * ray.getDirection()[i0] + ray.getOrigin()[i0] - a[i0];
		float v0 = t * ray.getDirection()[i1] + ray.getOrigin()[i1] - a[i1];

		float alpha = u0 * v2 - u2 * v0;
		float beta  = u1 * v0 - u0 * v1;
		float area  = u1 * v2 - u2 * v1;

		// epsilon to avoid float precision error
		const float EPSILON = 1e-6f;

		float tolerance = - EPSILON * area;

		if (area > 0)
		{
			if (alpha < tolerance || beta < tolerance || alpha+beta > area-tolerance)
				return std::pair<bool, float>(false, 0.f);
		}
		else
		{
			if (alpha > tolerance || beta > tolerance || alpha+beta < area-tolerance)
				return std::pair<bool, float>(false, 0.f);
		}
	}

	return std::pair<bool, float>(true, t);
}
//-----------------------------------------------------------------------
std::pair<bool, float> Math::intersects(const Ray& ray, const Vector3& a,
	const Vector3& b, const Vector3& c,
	bool positiveSide, bool negativeSide)
{
	Vector3 normal = calculateBasicFaceNormalWithoutNormalize(a, b, c);
	return intersects(ray, a, b, c, normal, positiveSide, negativeSide);
}
//-----------------------------------------------------------------------
bool Math::intersects(const Sphere& sphere, const AxisAlignedBox& box)
{
	if (box.isNull()) return false;
	if (box.isInfinite()) return true;

	// Use splitting planes
	const Vector3& center = sphere.getCenter();
	float radius = sphere.getRadius();
	const Vector3& min = box.getMinimum();
	const Vector3& max = box.getMaximum();

	// Arvo's algorithm
	float s, d = 0;
	for (int i = 0; i < 3; ++i)
	{
		if (center.ptr()[i] < min.ptr()[i])
		{
			s = center.ptr()[i] - min.ptr()[i];
			d += s * s; 
		}
		else if(center.ptr()[i] > max.ptr()[i])
		{
			s = center.ptr()[i] - max.ptr()[i];
			d += s * s; 
		}
	}
	return d <= radius * radius;

}
bool Math::intersects(const CShapeSphere& sphere, const CShapeBox& box)
{
	// Use splitting planes
	const Vector3& center = (const Vector3&)sphere.GetCenter();
	float radius = sphere.GetRadius();
	const Vector3& min = (const Vector3&)box.GetMin();
	const Vector3& max = (const Vector3&)box.GetMax();

	// Arvo's algorithm
	float s, d = 0;
	for (int i = 0; i < 3; ++i)
	{
		if (center.ptr()[i] < min.ptr()[i])
		{
			s = center.ptr()[i] - min.ptr()[i];
			d += s * s; 
		}
		else if(center.ptr()[i] > max.ptr()[i])
		{
			s = center.ptr()[i] - max.ptr()[i];
			d += s * s; 
		}
	}
	return d <= radius * radius;
}

//-----------------------------------------------------------------------
bool Math::intersects(const Plane& plane, const AxisAlignedBox& box)
{
	return (plane.getSide(box) == Plane::BOTH_SIDE);
}
//-----------------------------------------------------------------------
bool Math::intersects(const Sphere& sphere, const Plane& plane)
{
	return (
		Math::Abs(plane.getDistance(sphere.getCenter()))
		<= sphere.getRadius() );
}
//-----------------------------------------------------------------------
Vector3 Math::calculateTangentSpaceVector(
	const Vector3& position1, const Vector3& position2, const Vector3& position3,
	float u1, float v1, float u2, float v2, float u3, float v3)
{
	//side0 is the vector along one side of the triangle of vertices passed in, 
	//and side1 is the vector along another side. Taking the cross product of these returns the normal.
	Vector3 side0 = position1 - position2;
	Vector3 side1 = position3 - position1;
	//Calculate face normal
	Vector3 normal = side1.crossProduct(side0);
	normal.normalise();
	//Now we use a formula to calculate the tangent. 
	float deltaV0 = v1 - v2;
	float deltaV1 = v3 - v1;
	Vector3 tangent = deltaV1 * side0 - deltaV0 * side1;
	tangent.normalise();
	//Calculate binormal
	float deltaU0 = u1 - u2;
	float deltaU1 = u3 - u1;
	Vector3 binormal = deltaU1 * side0 - deltaU0 * side1;
	binormal.normalise();
	//Now, we take the cross product of the tangents to get a vector which 
	//should point in the same direction as our normal calculated above. 
	//If it points in the opposite direction (the dot product between the normals is less than zero), 
	//then we need to reverse the s and t tangents. 
	//This is because the triangle has been mirrored when going from tangent space to object space.
	//reverse tangents if necessary
	Vector3 tangentCross = tangent.crossProduct(binormal);
	if (tangentCross.dotProduct(normal) < 0.0f)
	{
		tangent = -tangent;
		binormal = -binormal;
	}

	return tangent;

}
//-----------------------------------------------------------------------
Matrix4 Math::buildReflectionMatrix(const Plane& p)
{
	return Matrix4(
		-2 * p.normal.x * p.normal.x + 1,   -2 * p.normal.x * p.normal.y,       -2 * p.normal.x * p.normal.z,       -2 * p.normal.x * p.d, 
		-2 * p.normal.y * p.normal.x,       -2 * p.normal.y * p.normal.y + 1,   -2 * p.normal.y * p.normal.z,       -2 * p.normal.y * p.d, 
		-2 * p.normal.z * p.normal.x,       -2 * p.normal.z * p.normal.y,       -2 * p.normal.z * p.normal.z + 1,   -2 * p.normal.z * p.d, 
		0,                                  0,                                  0,                                  1);
}
//-----------------------------------------------------------------------
Vector4 Math::calculateFaceNormal(const Vector3& v1, const Vector3& v2, const Vector3& v3)
{
	Vector3 normal = calculateBasicFaceNormal(v1, v2, v3);
	// Now set up the w (distance of tri from origin
	return Vector4(normal.x, normal.y, normal.z, -(normal.dotProduct(v1)));
}
//-----------------------------------------------------------------------
Vector3 Math::calculateBasicFaceNormal(const Vector3& v1, const Vector3& v2, const Vector3& v3)
{
	Vector3 normal = (v2 - v1).crossProduct(v3 - v1);
	normal.normalise();
	return normal;
}
//-----------------------------------------------------------------------
Vector4 Math::calculateFaceNormalWithoutNormalize(const Vector3& v1, const Vector3& v2, const Vector3& v3)
{
	Vector3 normal = calculateBasicFaceNormalWithoutNormalize(v1, v2, v3);
	// Now set up the w (distance of tri from origin)
	return Vector4(normal.x, normal.y, normal.z, -(normal.dotProduct(v1)));
}
//-----------------------------------------------------------------------
Vector3 Math::calculateBasicFaceNormalWithoutNormalize(const Vector3& v1, const Vector3& v2, const Vector3& v3)
{
	Vector3 normal = (v2 - v1).crossProduct(v3 - v1);
	return normal;
}
//-----------------------------------------------------------------------
float Math::gaussianDistribution(float x, float offset, float scale)
{
	float nom = Math::Exp(
		-Math::Sqr(x - offset) / (2 * Math::Sqr(scale)));
	float denom = scale * Math::Sqrt(2 * Math::PI);

	return nom / denom;

}
//---------------------------------------------------------------------
Matrix4 Math::makeViewMatrix(const Vector3& position, const Quaternion& orientation, 
	const Matrix4* reflectMatrix)
{
	Matrix4 viewMatrix;

	// View matrix is:
	//
	//  [ Lx  Uy  Dz  Tx  ]
	//  [ Lx  Uy  Dz  Ty  ]
	//  [ Lx  Uy  Dz  Tz  ]
	//  [ 0   0   0   1   ]
	//
	// Where T = -(Transposed(Rot) * Pos)

	// This is most efficiently done using 3x3 Matrices
	Matrix3 rot;
	orientation.ToRotationMatrix(rot);

	// Make the translation relative to new axes
	Matrix3 rotT = rot.Transpose();
	Vector3 trans = -rotT * position;

	// Make final matrix
	viewMatrix = Matrix4::IDENTITY;
	viewMatrix = rotT; // fills upper 3x3
	viewMatrix[0][3] = trans.x;
	viewMatrix[1][3] = trans.y;
	viewMatrix[2][3] = trans.z;

	// Deal with reflections
	if (reflectMatrix)
	{
		viewMatrix = viewMatrix * (*reflectMatrix);
	}

	return viewMatrix;

}

int Math::log2 (unsigned int x)
{
	static const unsigned char log_2[256] = {
		0,1,2,2,3,3,3,3,4,4,4,4,4,4,4,4,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,5,
		6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,6,
		7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
		7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,
		8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
		8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
		8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,
		8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8,8
	};
	int l = -1;
	while (x >= 256) { l += 8; x >>= 8; }
	return l + log_2[x];
}


static int32 GSRandSeed = 1234567;

void Math::SRandInit(int32 Seed)
{
	GSRandSeed = Seed;
}

float Math::SRand()
{
	GSRandSeed = (GSRandSeed * 196314165) + 907633515;
	union { float f; int32 i; } Result;
	union { float f; int32 i; } Temp;
	const float SRandTemp = 1.0f;
	Temp.f = SRandTemp;
	Result.i = (Temp.i & 0xff800000) | (GSRandSeed & 0x007fffff);
	return Fractional(Result.f);
}


bool Math::SmoothMoveFloat1(float& src, const float dest, const float fMaxStep)
{
	if (fabs(src - dest) <= fMaxStep)
	{
		src = dest;
		return true;
	}
	else if (src>dest)
	{
		src -= fMaxStep;
	}
	else if (src<dest)
	{
		src += fMaxStep;
	}
	return false;
}
bool Math::SmoothMoveAngle1(float& src, const float dest, const float fMaxStep)
{
	float fDif = ToStandardAngle(src - dest);
	if (fabs(fDif) <= fMaxStep)
	{
		src = dest;
		return true;
	}
	else if (fDif>0)
	{
		src -= fMaxStep;
	}
	else// if(fDif<0)
	{
		src += fMaxStep;
	}
	return false;
}

bool Math::SmoothMoveVec3(Vector3* result, const Vector3& vPosTarget, const Vector3& vPos, FLOAT fIncrease, FLOAT fTolerance/*=0*/)
{
	Vector3 vSub;
	float fDist;

	// get distance from target
	vSub = vPosTarget - vPos;
	fDist = vSub.squaredLength();
	if (fDist>0)
		fDist = sqrt(fDist);

	if (fIncrease >= fDist || fTolerance >= fDist)
	{
		// we're within reach; set the exact point
		(*result) = vPosTarget;
		return true;
	}
	else
	{
		// moving forward
		vSub *= fIncrease / fDist;
		*result = vPos + vSub;
		return false;
	}
}

double ParaEngine::Math::ToStandardAngle(double fAngle)
{
	if (fAngle > 0)
	{
		fAngle = fAngle - (MATH_PI * 2)*((int)(fAngle / (MATH_PI * 2)));
		if (fAngle > MATH_PI)
			fAngle -= MATH_PI * 2;
	}
	else
	{
		fAngle = fAngle - (MATH_PI * 2)*((int)(fAngle / (MATH_PI * 2)));
		if (fAngle < -MATH_PI)
			fAngle += MATH_PI * 2;
	}
	return fAngle;
}

float Math::ToStandardAngle(float fAngle)
{
	if (fAngle>0)
	{
		fAngle = fAngle - (MATH_PI * 2)*((int)(fAngle / (MATH_PI * 2)));
		if (fAngle>MATH_PI)
			fAngle -= MATH_PI * 2;
	}
	else
	{
		fAngle = fAngle - (MATH_PI * 2)*((int)(fAngle / (MATH_PI * 2)));
		if (fAngle<-MATH_PI)
			fAngle += MATH_PI * 2;
	}

	/*while(fAngle<=-MATH_PI)
	fAngle+=MATH_PI*2;
	while(fAngle>MATH_PI)
	fAngle-=MATH_PI*2;*/
	return fAngle;
}

bool Math::MatchString(const std::string& matchPattern_, const std::string& matchStr_)
{
	bool bMatched = true;
	char s;
	int nameIndex, searchStrIndex;
	const char* matchPattern = matchPattern_.c_str();
	const char* matchStr = matchStr_.c_str();
	for (nameIndex = 0, searchStrIndex = 0; matchStr[nameIndex] != '\0'; nameIndex++)
	{
		s = matchPattern[searchStrIndex];
		if ((matchStr[nameIndex] != s && s != '*') || (s == '\0'))
		{
			bMatched = false;
			break;
		}
		if (s != '*')
			searchStrIndex++;

	}
	s = matchPattern[searchStrIndex];
	if (bMatched && ((s == '*') || (s == '\0')))
		return true;
	else
		return false;
}

bool Math::ComputeFacingTarget(const Vector3& target, const Vector3& source, FLOAT& fFacing)
{
	Vector2 vDiff;
	vDiff.x = target.x - source.x;
	vDiff.y = source.z - target.z;

	if ((vDiff.x == 0.f) && (vDiff.y == 0.f))
		return false;
	vDiff.normalise();

	if (vDiff.y>1)	vDiff.y = 1;
	if (vDiff.y<-1)	vDiff.y = -1;

	if (vDiff.x>0)
	{
		fFacing = asinf(vDiff.y);
		if (fFacing < 0)
		{
			fFacing = 2 * MATH_PI + fFacing;
		}
	}
	else
	{
		fFacing = MATH_PI - asinf(vDiff.y);
	}
	return true;
}


bool ParaEngine::Math::ComputeFacingTarget(const DVector3& target, const DVector3& source, float& fFacing)
{
	Vector2 vDiff;
	vDiff.x = (float)(target.x - source.x);
	vDiff.y = (float)(source.z - target.z);

	if ((vDiff.x == 0.f) && (vDiff.y == 0.f))
		return false;
	vDiff.normalise();

	if (vDiff.y > 1)	vDiff.y = 1;
	if (vDiff.y < -1)	vDiff.y = -1;

	if (vDiff.x > 0)
	{
		fFacing = asinf(vDiff.y);
		if (fFacing < 0)
		{
			fFacing = 2 * MATH_PI + fFacing;
		}
	}
	else
	{
		fFacing = MATH_PI - asinf(vDiff.y);
	}
	return true;
}


float Math::MinVec3(const Vector3& v)
{
	return min(v.x, min(v.y, v.z));
}

float Math::MaxVec3(const Vector3& v)
{
	return max(v.x, max(v.y, v.z));
}

void Math::GetMatrixScaling(const Matrix4& globalMat, float* fScalingX, float* fScalingY, float* fScalingZ)
{
	if (fScalingX != 0)
		*fScalingX = Vector3(globalMat._11, globalMat._12, globalMat._13).length();
	if (fScalingY != 0)
		*fScalingY = Vector3(globalMat._21, globalMat._22, globalMat._23).length();
	if (fScalingZ != 0)
		*fScalingZ = Vector3(globalMat._31, globalMat._32, globalMat._33).length();
}


Matrix4* ParaEngine::Math::CreateBillboardMatrix(Matrix4* pOut, const Matrix4* matModelview, const Vector3* pvBillboardPos, bool bAxisAligned /*= false*/)
{
	Vector3 vBillboardPos(0, 0, 0);
	if (pvBillboardPos)
		vBillboardPos = *pvBillboardPos;
	Matrix4 mtrans, mtransWorld;
	if (matModelview)
	{
		mtrans = *matModelview;
	}
	else
	{
		mtrans = CGlobals::GetViewMatrixStack().SafeGetTop();
		mtransWorld = CGlobals::GetWorldMatrixStack().SafeGetTop();
		mtrans = mtransWorld * mtrans;
	}
	mtrans = mtrans.inverse();

	// convert everything to model space, so that we have look, up, right vector in model space.  
	Vector3 camera;
	ParaVec3TransformCoord(&camera, &Vector3::ZERO, &mtrans);
	Vector3 look = (camera - vBillboardPos);
	ParaVec3Normalize(&look, &look);

	Vector3 up;
	ParaVec3TransformCoord(&up, &Vector3::UNIT_Y, &mtrans);
	up -= camera;
	ParaVec3Normalize(&up, &up);

	Vector3 right;
	right = up.crossProduct(look);
	ParaVec3Normalize(&right, &right);

	up = look.crossProduct(right);
	ParaVec3Normalize(&up, &up);

	// calculate a billboard matrix
	Matrix4 mbb(Matrix4::IDENTITY);

	mbb._11 = right.x;
	mbb._12 = right.y;
	mbb._13 = right.z;
	mbb._21 = up.x;
	mbb._22 = up.y;
	mbb._23 = up.z;
	mbb._31 = look.x;
	mbb._32 = look.y;
	mbb._33 = look.z;

	// TODO: 
	if (bAxisAligned)
	{
	}

	// fixed pivot LXZ 2008.12.3. 
	mtrans.makeTrans(-vBillboardPos.x, -vBillboardPos.y, -vBillboardPos.z);
	mbb = mtrans * mbb;
	mbb._41 += vBillboardPos.x;
	mbb._42 += vBillboardPos.y;
	mbb._43 += vBillboardPos.z;

	if (pOut)
	{
		*pOut = mbb;
	}
	return pOut;
}

void ParaEngine::Math::CameraRotMatrixYawPitch(Matrix4& out, float fYaw, float fPitch)
{
	/** LXZ: 2009.6.26. for unknown reasons, ParaMatrixRotationYawPitchRoll sometimes yields invalid matrix. So I need to emulate it with standard multiplication.
	The order of transformations is roll first, then pitch, then yaw, where:
	- Yaw around the y-axis, in radians.
	- Pitch around the x-axis, in radians.
	- Roll around the z-axis, in radians.
	ParaMatrixRotationYawPitchRoll( &mCameraRot, m_fCameraYawAngle, m_fCameraPitchAngle, 0 );
	*/
	Matrix4 matPitch, matYaw;
	ParaMatrixRotationX(&matPitch, fPitch);
	ParaMatrixRotationY(&matYaw, fYaw);
	out = matPitch * matYaw;
}

int ParaEngine::Math::NextPowerOf2(int x)
{
	x = x - 1;
	x = x | (x >> 1);
	x = x | (x >> 2);
	x = x | (x >> 4);
	x = x | (x >> 8);
	x = x | (x >> 16);
	return x + 1;
}

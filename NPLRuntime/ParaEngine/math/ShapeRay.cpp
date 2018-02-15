//-----------------------------------------------------------------------------
// Class:	CShapeRay
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Date:	2005.12.21
// Revised: 2006.4
// Desc: 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"

#include "ShapeAABB.h"
#include "ShapeRay.h"

using namespace ParaEngine;

CShapeRay::CShapeRay(void)
{
}

CShapeRay::~CShapeRay(void)
{
}

float CShapeRay::Distance(const Vector3& point, float* t) const 
{ 
	return sqrtf(SquareDistance(point, t));
}

float CShapeRay::SquareDistance(const Vector3& point, float* t)	const
{
	Vector3 Diff = point - mOrig;
	
	float fT = Diff.dotProduct(mDir);

	if(fT<=0.0f)
	{
		fT = 0.0f;
	}
	else
	{
		fT /= mDir.squaredLength();
		Diff -= fT*mDir;
	}

	if(t) *t = fT;

	return Diff.squaredLength();
}

std::pair<bool, float> CShapeRay::intersects(const CShapeAABB& box, const Matrix4* world) const
{
	Vector3 rayorig,raydir;
	// Compute ray in local space
	// The (Origin/Dir) form is needed for the ray-triangle test anyway (even for segment tests)
	if(world)
	{
		Matrix3 InvWorld(*world);
		raydir = InvWorld * mDir;

		Matrix4 World = world->InvertPRMatrix();
		rayorig = mOrig * World;
	}
	else
	{
		raydir = mDir;
		rayorig = mOrig;
	}

	float lowt = 0.0f;
	float t;
	bool hit = false;
	Vector3 hitpoint;
	Vector3 min,max;
	box.GetMin(min);
	box.GetMax(max);

	// Check origin inside first
	if ( rayorig > min && rayorig < max )
	{
		return std::pair<bool, float>(true, 0.f);
	}

	// Check each face in turn, only check closest 3
	// Min x
	if (rayorig.x < min.x && raydir.x > 0)
	{
		t = (min.x - rayorig.x) / raydir.x;
		if (t > 0)
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
	if (rayorig.x > max.x && raydir.x < 0)
	{
		t = (max.x - rayorig.x) / raydir.x;
		if (t > 0)
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
	if (rayorig.y < min.y && raydir.y > 0)
	{
		t = (min.y - rayorig.y) / raydir.y;
		if (t > 0)
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
	if (rayorig.y > max.y && raydir.y < 0)
	{
		t = (max.y - rayorig.y) / raydir.y;
		if (t > 0)
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
	if (rayorig.z < min.z && raydir.z > 0)
	{
		t = (min.z - rayorig.z) / raydir.z;
		if (t > 0)
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
	if (rayorig.z > max.z && raydir.z < 0)
	{
		t = (max.z - rayorig.z) / raydir.z;
		if (t > 0)
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

int CShapeRay::IntersectPlane(const Plane * plane, Vector3 * point, float *distance) const
{
	float vd, vo, PnDOTRo, t;

	vd = plane->PlaneDotNormal(mDir);
	if (vd == 0.0)
		// The plane is parallel to the ray. I've never seen this happen but someday it will . . .
		return -1;
	if (vd > 0.0)
	{
		// The plane is facing away from the ray so no intersection occurs.
		return -2;
	}
	PnDOTRo = plane->PlaneDotNormal(mOrig);
	vo = -1.0f * (PnDOTRo + plane->d);
	t = vo / vd;
	if (t < 0.0f)
		// The intersection occurs behind the ray's origin.
		return -3;
	point->x = mOrig.x + mDir.x * t;
	point->y = mOrig.y + mDir.y * t;
	point->z = mOrig.z + mDir.z * t;
	if (distance != NULL)
		*distance = t;
	return 1;
}

int CShapeRay::IntersectBox(const CShapeBox * box, Vector3* point, float *distance) const
{
	float tnear, tfar, t1, t2;

	tnear = -FLOAT_POS_INFINITY;
	tfar = FLOAT_POS_INFINITY;

	// Find intersection with x-aligned planes of box.
	// If the ray is parallel to the box and not within the planes of the box it misses.
	if (mDir.x == 0.0)
		if ((mOrig.x < box->m_Min.x) && (mOrig.x > box->m_Max.x))
			return 0;
	// Calculate intersection distance with the box's planes.
	t1 = (box->m_Min.x - mOrig.x) / mDir.x;
	t2 = (box->m_Max.x - mOrig.x) / mDir.x;
	if (t1 > t2)
	{
		float tmp = t1;
		t1 = t2;
		t2 = tmp;
	}
	if (t1 > tnear)
		tnear = t1;
	if (t2 < tfar)
		tfar = t2;
	if (tnear > tfar)
		return 0;
	if (tfar < 0.0)
		return 0;
	// Find intersection with y-aligned planes of box.
	// If the ray is parallel to the box and not within the planes of the box it misses.
	if (mDir.y == 0.0)
		if ((mOrig.y < box->m_Min.y) && (mOrig.y > box->m_Max.y))
			return 0;
	// Calculate intersection distance with the box's planes.
	t1 = (box->m_Min.y - mOrig.y) / mDir.y;
	t2 = (box->m_Max.y - mOrig.y) / mDir.y;
	if (t1 > t2)
	{
		float tmp = t1;
		t1 = t2;
		t2 = tmp;
	}
	if (t1 > tnear)
		tnear = t1;
	if (t2 < tfar)
		tfar = t2;
	if (tnear > tfar)
		return 0;
	if (tfar < 0.0)
		return 0;
	// Find intersection with z-aligned planes of box.
	// If the ray is parallel to the box and not within the planes of the box it misses.
	if (mDir.z == 0.0)
		if ((mOrig.z < box->m_Min.z) && (mOrig.z > box->m_Max.z))
			return 0;
	// Calculate intersection distance with the box's planes.
	t1 = (box->m_Min.z - mOrig.z) / mDir.z;
	t2 = (box->m_Max.z - mOrig.z) / mDir.z;
	if (t1 > t2)
	{
		float tmp = t1;
		t1 = t2;
		t2 = tmp;
	}
	if (t1 > tnear)
		tnear = t1;
	if (t2 < tfar)
		tfar = t2;
	if (tnear > tfar)
		return 0;
	if (tfar < 0.0)
		return 0;
	// If we survived all of the tests, the box is hit.
	if (point != NULL)
	{
		point->x = mOrig.x + tnear * mDir.x;
		point->y = mOrig.y + tnear * mDir.y;
		point->z = mOrig.z + tnear * mDir.z;
	}
	if(distance!=NULL)
		*distance = tnear;
	return 1;
}


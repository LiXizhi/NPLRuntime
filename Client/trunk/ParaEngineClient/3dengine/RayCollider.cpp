//-----------------------------------------------------------------------------
// Class:	CRayCollider
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2005.12.21
// Desc: Based on Pierre Terdiman's work in the ODE physics engine.
// and the paper "A Cross-Platform Framework for Interactive Ray Tracing" by Markus Geimer and Stefan 
//-----------------------------------------------------------------------------
#include "ParaEngine.h"
#include "FPU.h"

#include "RayCollider.h"

using namespace ParaEngine;

/** @def use SSE_RAY_BOX_INTERSECTION for ray box collision detection. It does not work at the moment*/
// #define SSE_RAY_BOX_INTERSECTION

#ifdef SSE_RAY_BOX_INTERSECTION
#include <xmmintrin.h>

	/* local struct for ray_box_intersect() */
	struct vec_t { 
		float x,y,z,pad; 
		vec_t& operator=(const Vector3& v){x=v.x;y=v.y;z=v.z; return (*this);};
	};
	struct aabb_t { 
		vec_t	min;
		vec_t	max;
	};
	struct ray_t {
		vec_t	pos;
		vec_t	inv_dir;
	};

	/** the distance from the ray origin to the near and far intersection point*/
	struct ray_segment_t {
		float	t_near,t_far;
	};
	bool ray_box_intersect(const aabb_t &box, const ray_t &ray, ray_segment_t &rs);
#endif

CRayCollider::CRayCollider(void):
	mCulling			(true),
	mMaxDist			(MAX_FLOAT)
{
}

CRayCollider::~CRayCollider(void)
{
}

void CRayCollider::SetCulling(bool flag)
{ 
	mCulling		= flag;
}

void CRayCollider::SetMaxDist(float max_dist)	
{ 
	mMaxDist		= max_dist;	
}

bool CRayCollider::InitQuery(const CShapeRay& world_ray, const Matrix4* world)
{
	// Reset stats & contact status
	Collider::InitQuery();
	
	// Compute ray in local space
	// The (Origin/Dir) form is needed for the ray-triangle test anyway (even for segment tests)
	if(world)
	{
		Matrix3 InvWorld(*world);
		mDir = InvWorld * world_ray.mDir;
		
		Matrix4 World = world->InvertPRMatrix();
		ParaVec3TransformCoord(&mOrigin, &world_ray.mOrig, &World);
	}
	else
	{
		mDir	= world_ray.mDir;
		mOrigin	= world_ray.mOrig;
	}

	// Precompute data (moved after temporal coherence since only needed for ray-AABB)
	if(IR(mMaxDist)!=IEEE_MAX_FLOAT)
	{
		// For Segment-AABB overlap
		mData = 0.5f * mDir * mMaxDist;
		mData2 = mOrigin + mData;

		// Precompute mFDir;
		mFDir.x = fabsf(mData.x);
		mFDir.y = fabsf(mData.y);
		mFDir.z = fabsf(mData.z);
	}
	else
	{
		// For Ray-AABB overlap
		
		// Precompute mFDir;
		mFDir.x = fabsf(mDir.x);
		mFDir.y = fabsf(mDir.y);
		mFDir.z = fabsf(mDir.z);
	}

	return false;
}

bool CRayCollider::SegmentAABBOverlap(const Vector3& center, const Vector3& extents)
{
	float Dx = mData2.x - center.x;		if(fabsf(Dx) > extents.x + mFDir.x)	return false;
	float Dy = mData2.y - center.y;		if(fabsf(Dy) > extents.y + mFDir.y)	return false;
	float Dz = mData2.z - center.z;		if(fabsf(Dz) > extents.z + mFDir.z)	return false;

	float f;
	f = mData.y * Dz - mData.z * Dy;	if(fabsf(f) > extents.y*mFDir.z + extents.z*mFDir.y)	return false;
	f = mData.z * Dx - mData.x * Dz;	if(fabsf(f) > extents.x*mFDir.z + extents.z*mFDir.x)	return false;
	f = mData.x * Dy - mData.y * Dx;	if(fabsf(f) > extents.x*mFDir.y + extents.y*mFDir.x)	return false;

	return true;
}

bool CRayCollider::RayAABBOverlap(const Vector3& center, const Vector3& extents)
{
	float Dx = mOrigin.x - center.x;	if(GREATER(Dx, extents.x) && Dx*mDir.x>=0.0f)	return false;
	float Dy = mOrigin.y - center.y;	if(GREATER(Dy, extents.y) && Dy*mDir.y>=0.0f)	return false;
	float Dz = mOrigin.z - center.z;	if(GREATER(Dz, extents.z) && Dz*mDir.z>=0.0f)	return false;

	float f;
	f = mDir.y * Dz - mDir.z * Dy;		if(fabsf(f) > extents.y*mFDir.z + extents.z*mFDir.y)	return false;
	f = mDir.z * Dx - mDir.x * Dz;		if(fabsf(f) > extents.x*mFDir.z + extents.z*mFDir.x)	return false;
	f = mDir.x * Dy - mDir.y * Dx;		if(fabsf(f) > extents.x*mFDir.y + extents.y*mFDir.x)	return false;

	return true;
}

bool CRayCollider::Collide(const CShapeRay& world_ray, const CShapeAABB& world_AABB, const Matrix4* world)
{
	// Init collision query
	// Basically this is only called to initialize precomputed data
	if(InitQuery(world_ray, world))	return true;

	Vector3 vCenter, vExtents;
	world_AABB.GetCenter(vCenter);
	world_AABB.GetExtents(vExtents);

	// Perform stabbing query
	if(IR(mMaxDist)!=IEEE_MAX_FLOAT)
	{
		return SegmentAABBOverlap(vCenter, vExtents);
	}
	else
	{
		return RayAABBOverlap(vCenter, vExtents);
	}

	return true;
}

bool CRayCollider::Collide(const CShapeRay& world_ray, const CShapeOBB& world_oob)
{
	CShapeRay ray(world_ray);
	ray.mOrig -= world_oob.GetCenter();
	return Collide(ray, CShapeAABB(Vector3(0,0,0), world_oob.GetExtents()), &world_oob.GetRot());
}

bool CRayCollider::Intersect(const CShapeRay& world_ray, const CShapeAABB& world_AABB, float * pDist, const Matrix4* world)
{
#ifdef SSE_RAY_BOX_INTERSECTION
	/*TODO: it always throw an exception. so I have to use another intersection method */
	ray_segment_t rs;
	// Compute ray in local space
	// The (Origin/Dir) form is needed for the ray-triangle test anyway (even for segment tests)
	if(world)
	{
		Matrix3x3 InvWorld(*world);
		mDir = InvWorld * world_ray.mDir;

		Matrix4 World = world->InvertPRMatrix();
		ParaVec3TransformCoord(&mOrigin, &world_ray.mOrig, &World);
	}
	else
	{
		mDir	= world_ray.mDir;
		mOrigin	= world_ray.mOrig;
	}
	aabb_t aabb;
	world_AABB.GetMin(*(Vector3*)(&aabb.min));
	world_AABB.GetMax(*(Vector3*)(&aabb.max));
	ray_t ray_;
	ray_.pos = mOrigin;
	ray_.inv_dir = mDir;
	if(pDist)
		(*pDist) = rs.t_near;
	return ray_box_intersect(aabb, ray_, rs);
#else
	std::pair<bool, float> rs = world_ray.intersects(world_AABB, world);
	if(pDist && rs.first)
		(*pDist) = rs.second;
	return rs.first;
#endif
}

bool CRayCollider::Intersect(const CShapeRay& world_ray, const CShapeOBB& world_oob, float * pDist)
{
	CShapeRay ray(world_ray);
	ray.mOrig -= world_oob.GetCenter();
	return Intersect(ray, CShapeAABB(Vector3(0,0,0), world_oob.GetExtents()), pDist, &world_oob.GetRot());
}

#ifdef SSE_RAY_BOX_INTERSECTION
	/************************************************************************/
	/* A robust and branchless SSE ray/box intersection test. 

	In their excellent paper "A Cross-Platform Framework for Interactive Ray Tracing"[1] Markus Geimer 
	and Stefan Müller propose a branchless SIMD friendly variation of the slab test. It's extremely fast 
	but there's a catch, under some conditions (say (box_min-pos) == 0 while inv_dir = inf) a NaN is produced 
	and due to the way SSE min/max work, it ends up with a bogus result. Here's an attempt to fix that corner 
	case while keeping things tight. It also works with flat voxels and take around 33 cycles on a good day 
	(compared to the 17 cycles of the original version), if you have a decent compiler (wink wink, nudge nudge). 

	Thierry Berger-Perrin. 

	PS: if you don't need the intersection points, just discard them.
	PPS: scheduling is left as an exercise for the compiler. 
	*/
	/************************************************************************/
	// turn those verbose intrinsics into something readable.
	#define loadps(mem)		_mm_load_ps((const float * const)(mem))
	#define storess(ss,mem)		_mm_store_ss((float * const)(mem),(ss))
	#define minss			_mm_min_ss
	#define maxss			_mm_max_ss
	#define minps			_mm_min_ps
	#define maxps			_mm_max_ps
	#define mulps			_mm_mul_ps
	#define subps			_mm_sub_ps
	#define rotatelps(ps)		_mm_shuffle_ps((ps),(ps), 0x39)	// a,b,c,d -> b,c,d,a
	#define muxhps(low,high)	_mm_movehl_ps((low),(high))	// low{a,b,c,d}|high{e,f,g,h} = {c,d,g,h}


	static const float flt_plus_inf = -logf(0);	// let's keep C and C++ compilers happy.
	static const float _MM_ALIGN16
	ps_cst_plus_inf[4]	= {  flt_plus_inf,  flt_plus_inf,  flt_plus_inf,  flt_plus_inf },
	ps_cst_minus_inf[4]	= { -flt_plus_inf, -flt_plus_inf, -flt_plus_inf, -flt_plus_inf };

	bool ray_box_intersect(const aabb_t &box, const ray_t &ray, ray_segment_t &rs) 
	{
		// you may already have those values hanging around somewhere
		const __m128
			plus_inf	= loadps(ps_cst_plus_inf),
			minus_inf	= loadps(ps_cst_minus_inf);

		// use whatever appropriate to load.
		const __m128
			box_min	= loadps(&box.min),
			box_max	= loadps(&box.max),
			pos	= loadps(&ray.pos),
			inv_dir	= loadps(&ray.inv_dir);

		// use a div if inverted directions aren't available
		const __m128 l1 = mulps(subps(box_min, pos), inv_dir);
		const __m128 l2 = mulps(subps(box_max, pos), inv_dir);

		// the order we use for those min/max is vital to filter out
		// NaNs that happens when an inv_dir is +/- inf and
		// (box_min - pos) is 0. inf * 0 = NaN
		const __m128 filtered_l1a = minps(l1, plus_inf);
		const __m128 filtered_l2a = minps(l2, plus_inf);

		const __m128 filtered_l1b = maxps(l1, minus_inf);
		const __m128 filtered_l2b = maxps(l2, minus_inf);

		// now that we're back on our feet, test those slabs.
		__m128 lmax = maxps(filtered_l1a, filtered_l2a);
		__m128 lmin = minps(filtered_l1b, filtered_l2b);

		// unfold back. try to hide the latency of the shufps & co.
		const __m128 lmax0 = rotatelps(lmax);
		const __m128 lmin0 = rotatelps(lmin);
		lmax = minss(lmax, lmax0);
		lmin = maxss(lmin, lmin0);

		const __m128 lmax1 = muxhps(lmax,lmax);
		const __m128 lmin1 = muxhps(lmin,lmin);
		lmax = minss(lmax, lmax1);
		lmin = maxss(lmin, lmin1);

		const bool ret = _mm_comige_ss(lmax, _mm_setzero_ps()) & _mm_comige_ss(lmax,lmin);

		storess(lmin, &rs.t_near);
		storess(lmax, &rs.t_far);

		return  ret;
	}

	void checkpointcharlie() {
		// let's keep things simple.
		// the ray is right on the edge, aimed straight into Z
		const aabb_t	_MM_ALIGN16 box = { -1,-1,-1, 0, 1,1,1, 0 };
		const ray_t	_MM_ALIGN16 ray = { -1,-1,-1, 0, flt_plus_inf,flt_plus_inf,-1, 0 };

		ray_segment_t rs;
		const bool rc = ray_box_intersect(box, ray, rs);
	}
#endif

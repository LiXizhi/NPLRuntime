#pragma once

#include "Collider.h"
#include "ShapeAABB.h"
#include "ShapeRay.h"
#include "ShapeOBB.h"

namespace ParaEngine
{
	/**
	*	Contains a ray-vs-shape collider.
	*	This class performs a stabbing query on an AABB, OOB, i.e. does a ray-mesh collision.
	*   Note: this is a reduced version for OPCODE - Optimized Collision Detection in ODE.
	* 
	*	HIGHER DISTANCE BOUND:
	*
	*		If P0 and P1 are two 3D points, let's define:
	*		- d = distance between P0 and P1
	*		- Origin	= P0
	*		- Direction	= (P1 - P0) / d = normalized direction vector
	*		- A parameter t such as a point P on the line (P0,P1) is P = Origin + t * Direction
	*		- t = 0  -->  P = P0
	*		- t = d  -->  P = P1
	*
	*		Then we can define a general "ray" as:
	*
	*			struct Ray
	*			{
	*				Point	Origin;
	*				Point	Direction;
	*			};
	*
	*		But it actually maps three different things:
	*		- a segment,   when 0 <= t <= d
	*		- a half-line, when 0 <= t < +infinity, or -infinity < t <= d
	*		- a line,      when -infinity < t < +infinity
	*
	*		In Opcode, we support segment queries, which yield half-line queries by setting d = +infinity.
	*		We don't support line-queries. If you need them, shift the origin along the ray by an appropriate margin.
	*
	*		In short, the lower bound is always 0, and you can setup the higher bound "d" with RayCollider::SetMaxDist().
	*
	*		Query	|segment			|half-line		|line
	*		--------|-------------------|---------------|----------------
	*		Usages	|-shadow feelers	|-raytracing	|-
	*				|-sweep tests		|-in/out tests	|
	*
	*	FIRST CONTACT:
	*
	*		- You can setup "first contact" mode or "all contacts" mode with RayCollider::SetFirstContact().
	*		- In "first contact" mode we return as soon as the ray hits one face. If can be useful e.g. for shadow feelers, where
	*		you want to know whether the path to the light is free or not (a boolean answer is enough).
	*		- In "all contacts" mode we return all faces hit by the ray.
	*
	*	TEMPORAL COHERENCE:
	*
	*		- You can enable or disable temporal coherence with RayCollider::SetTemporalCoherence().
	*		- It currently only works in "first contact" mode.
	*		- If temporal coherence is enabled, the previously hit triangle is cached during the first query. Then, next queries
	*		start by colliding the ray against the cached triangle. If they still collide, we return immediately.
	*
	*	CLOSEST HIT:
	*
	*		- You can enable or disable "closest hit" with RayCollider::SetClosestHit().
	*		- It currently only works in "all contacts" mode.
	*		- If closest hit is enabled, faces are sorted by distance on-the-fly and the closest one only is reported.
	*
	*	BACKFACE CULLING:
	*
	*		- You can enable or disable backface culling with RayCollider::SetCulling().
	*		- If culling is enabled, ray will not hit back faces (only front faces).
	*/
	class CRayCollider : public Collider
	{
	public:
		CRayCollider(void);
		~CRayCollider(void);
	public:
		/**
		*	Generic stabbing query for generic OPCODE models. After the call, access the results:
		*	- with GetContactStatus()
		*	- in the user-provided destination array
		*
		*	\param		world_ray		[in] stabbing ray in world space
		*	\param		model [in] Opcode model to collide with
		*	\param		world [in] model's world matrix, or null
		*	\param		cache [in] a possibly cached face index, or null
		*	\return		true if success
		*	\warning	SCALE NOT SUPPORTED. The matrices must contain rotation & translation parts only.
		*/
		bool Collide(const CShapeRay& world_ray, const CShapeAABB& world_AABB, const Matrix4* world=NULL);
		bool Collide(const CShapeRay& world_ray, const CShapeOBB& world_oob);
		/**
		* @see Collide(). This function not only test if the ray and the box intersect, but also returns the intersection point
		* @param pDist: distance to the nearest face is returned. 
		*/
		bool Intersect(const CShapeRay& world_ray, const CShapeAABB& world_AABB, float * pDist, const Matrix4* world=NULL);
		bool Intersect(const CShapeRay& world_ray, const CShapeOBB& world_oob, float * pDist);
		/**
		*	Settings: enable or disable back face culling.
		*	\param		flag		[in] true to enable back face culling
		*	\see		SetClosestHit(bool flag)
		*	\see		SetMaxDist(float max_dist)
		*	\see		SetDestination(StabbedFaces* sf)
		*/
		void SetCulling(bool flag);

		/**
		*	Settings: sets the higher distance bound.
		*	\param		max_dist	[in] higher distance bound. Default = maximal value, for ray queries (else segment)
		*	\see		SetClosestHit(bool flag)
		*	\see		SetCulling(bool flag)
		*/
		void SetMaxDist(float max_dist);
	
	protected:
		// Ray in local space
		Vector3 mOrigin; //!< Ray origin
		Vector3 mDir; 	//!< Ray direction (normalized)
		Vector3 mFDir; 	//!< fabsf(mDir)
		Vector3 mData, mData2;

		// Dequantization coeffs
		Vector3 mCenterCoeff;
		Vector3 mExtentsCoeff;
		// Settings
		float mMaxDist; //!< Valid segment on the ray
		bool mCulling; //!< Stab culled faces or not

		// Overlap tests

		/**
		*	Computes a ray-AABB overlap test using the separating axis theorem. Ray is cached within the class.
		*	\param		center	[in] AABB center
		*	\param		extents	[in] AABB extents
		*	\return		true on overlap
		*/
		bool RayAABBOverlap(const Vector3& center, const Vector3& extents);

		/**
		*	Computes a segment-AABB overlap test using the separating axis theorem. Segment is cached within the class.
		*	\param		center	[in] AABB center
		*	\param		extents	[in] AABB extents
		*	\return		true on overlap
		*/
		bool SegmentAABBOverlap(const Vector3& center, const Vector3& extents);
		// TODO:
		bool RayTriOverlap(const Vector3& vert0, const Vector3& vert1, const Vector3& vert2);

		/**
		*	Initializes a stabbing query :
		*	- reset stats & contact status
		*	- compute ray in local space
		*	- check temporal coherence
		*
		*	\param		world_ray	[in] stabbing ray in world space
		*	\param		world		[in] object's world matrix, or null
		*	\return		TRUE if we can return immediately
		*	\warning	SCALE NOT SUPPORTED. The matrix must contain rotation & translation parts only.
		*/
		bool InitQuery(const CShapeRay& world_ray, const Matrix4* world=NULL);
	};

}

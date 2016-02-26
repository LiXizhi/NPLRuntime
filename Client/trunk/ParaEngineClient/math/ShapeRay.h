#pragma once

namespace ParaEngine
{
	class Plane;
	class CShapeBox;
	class CShapeAABB;
	/**
	*	A ray is a half-line P(t) = mOrig + mDir * t, with 0 <= t <= +infinity
	*/
	class CShapeRay
	{
	public:
		CShapeRay();
		~CShapeRay();
		CShapeRay(const Vector3& orig, const Vector3& dir) : mOrig(orig), mDir(dir)	{}
		/// Copy constructor
		CShapeRay(const CShapeRay& ray) : mOrig(ray.mOrig), mDir(ray.mDir)				{}
		
		float	SquareDistance(const Vector3& point, float* t=NULL)	const;
		float	Distance(const Vector3& point, float* t=NULL) const;

		/** Tests whether this ray intersects the given box. 
		* @param box: aabb box
		* @param world: a rotation transform applied to AABB. This can be NULL.
		* @returns A pair structure where the first element indicates whether
		an intersection occurs, and if true, the second element will
		indicate the distance along the ray at which it intersects. 
		*/
		std::pair<bool, float> intersects(const CShapeAABB& box, const Matrix4* world=NULL) const;
		
		/**
		@return: 
			1:intersected
			-1:The plane is parallel to the ray; 
			-2:The plane is facing away from the ray so no intersection occurs.
			-3:The intersection occurs behind the ray's origin.
		*/
		int IntersectPlane(const Plane * plane, Vector3 * point, float *distance) const;

		/**
		@param point: can be NULL
		@param distance: can be NULL
		@return :1 if intersected; 0 otherwise */
		int IntersectBox(const CShapeBox * box, Vector3* point, float *distance) const;

		/// Ray origin
		Vector3	mOrig;		
		/// Normalized direction
		Vector3	mDir;		
	};

}

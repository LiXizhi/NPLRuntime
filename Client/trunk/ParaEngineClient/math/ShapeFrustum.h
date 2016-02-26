#pragma once

namespace ParaEngine
{
	class CShapeAABB;
	class CShapeSphere;

	/** a general view frustum class.*/
	class CShapeFrustum
	{
	public:
		CShapeFrustum();
		/** build a frustum from a camera (projection, or viewProjection) matrix*/
		CShapeFrustum(const Matrix4* matrix);

		bool TestSphere     ( const CShapeSphere* sphere ) const;
		bool TestSweptSphere( const CShapeSphere* sphere, const Vector3* sweepDir ) const;
		/**  Tests if an AABB is inside/intersecting the view frustum
		* @return: 2 requires more testing(meaning either intersecting)
		* 1 is fast accept(meaning the box is fully inside the frustum)
		* 0 is fast reject, 
		*/
		int  TestBox        ( const CShapeAABB* box ) const;
		
		/** build a frustum from a camera (projection, or viewProjection) matrix
		* @param matViewProj the view projection matrix. 
		* @param bInversedMatrix: if this is true, matViewProj is the inverse of standard matViewProj
		* @param fNearPlane: the near plane in viewProjection coordinate, default to 0.f
		* @param fFarPlane: the far plane in viewProjection coordinate, default to 1.f
		*/
		virtual void UpdateFrustum(const Matrix4* matViewProj, bool bInversedMatrix=false, float fNearPlane = 0.f, float fFarPlane = 1.f);

		/** Get a given frustum plane.
		* @param nIndex: 0 is near plane, 1 is far plane, 2-5 is left, right, top, bottom.  
		*/
		inline Plane& GetPlane(int nIndex) {return planeFrustum[nIndex];}

		/**  whether the point is inside a given plane. 
		* @param iPlane: 0 is near plane, 1 is far plane, 2-5 is left, right, top, bottom.  
		*/
		bool CullPointWithPlane(int iPlane, const Vector3* vPos);

		/** return true if any of the point is inside the frustum */
		bool CullPointsWithFrustum(Vector3* pVecBounds, int nCount);

		/// corners of the view frustum
		Vector3 vecFrustum[8];    
		/// planes of the view frustum
		Plane planeFrustum[6];
		int nVertexLUT[6];
	public:
		/** this function tests if the projection of a bounding sphere along the light direction intersects the view frustum */
		static bool SweptSpherePlaneIntersect(float& t0, float& t1, const Plane* plane, const CShapeSphere* sphere, const Vector3* sweepDir);
	};
}

#pragma once
#include "ShapeSphere.h"
#include "ShapeFrustum.h"

namespace ParaEngine
{
	class CShapeAABB;

	/** the camera view culling frustum */
	class CCameraFrustum : public CShapeFrustum
	{
	public:
		virtual void UpdateFrustum(const Matrix4* matViewProj, bool bInversedMatrix = false, float fNearPlane = 0.f, float fFarPlane = 1.f);

		inline float GetViewDepth(){ return m_fViewDepth; };

		/** this is the same as the TestBox, except that the far plane is treated as a sphere plane whose radius is m_fViewDepth.
		this is currently used by terrain view frustum */
		bool TestBox_sphere_far_plane(const CShapeAABB* box) const;

		/** this is the same as TestBox, except that the box must be completely inside the far plane. 
		* @return: 2 requires more testing(meaning either intersecting)
		* 1 is fast accept(meaning the box is fully inside the frustum)
		* 0 is fast reject,
		*/
		int TestBoxInsideFarPlane(const CShapeAABB* box) const;


		/** the distance between the near plane and far plane in world coordinate. */
		float m_fViewDepth;
		/** the eye origin */
		Vector3	m_vEyeOrigin;
		/** 2D circle center.
		* It is the center of the smallest circle on the y=0 plane,
		* which contains the Frustum projection on to the 2D plane.
		* The information is used for rough object-level culling.
		*/
		Vector2	v2DCircleCenter;
		/** the radius of the 2D circle.
		* @See v2DCircleCenter */
		float v2DCircleRadius;
		/** bounding sphere*/
		CShapeSphere m_boundingSphere;
	};
}
#pragma once

namespace ParaEngine
{
	class CShapeAABB;
	#define PLANE_EPSILON		(1.0e-7f)

	/** plane */
	class CShapePlane : Plane
	{
	
		CShapePlane&	Zero();
		CShapePlane&	Set(float nx, float ny, float nz, float _d);
		CShapePlane&	Set(const Vector3& p, const Vector3& _n);
		CShapePlane&	Set(const Vector3& p0, const Vector3& p1, const Vector3& p2);

		/** The "positive side" of the plane is the half space to which the
		plane normal points. The "negative side" is the other half
		space. The flag "no side" indicates the plane itself.
		*/
		enum Side
		{
			NO_SIDE,
			POSITIVE_SIDE,
			NEGATIVE_SIDE,
			BOTH_SIDE
		};
		Side getSide (const Vector3& rkPoint) const;

		/**
		returns the side where the aligneBox is. the flag BOTH_SIDE indicates an intersecting box.
		one corner ON the plane is sufficient to consider the box and the plane intersecting.
		*/
		Side getSide (const CShapeAABB& rkBox) const;

		/** Returns which side of the plane that the given box lies on.
		The box is defined as centre/half-size pairs for effectively.
		@param centre The centre of the box.
		@param halfSize The half-size of the box.
		@returns
		POSITIVE_SIDE if the box complete lies on the "positive side" of the plane,
		NEGATIVE_SIDE if the box complete lies on the "negative side" of the plane,
		and BOTH_SIDE if the box intersects the plane.
		*/
		Side getSide (const Vector3& centre, const Vector3& halfSize) const;


		float	Distance(const Vector3& p)const;
		bool	Belongs(const Vector3& p)const;

		void	Normalize();

		// Cast operators
		operator Vector3()	const;
		operator Vector4() const;

		// Arithmetic operators
		CShapePlane	operator*(const Matrix4& m)		const;
		CShapePlane&	operator*=(const Matrix4& m);
	};
}

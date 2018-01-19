#pragma once
// Original free version by:
// Magic Software, Inc.
// http://www.geometrictools.com/
// Copyright (c) 2000, All Rights Reserved

#include "ParaVector3.h"

namespace ParaEngine 
{
	class CShapeAABB;

    /** Defines a plane in 3D space.
        @remarks
            A plane is defined in 3D space by the equation
            Ax + By + Cz + D = 0
        @par
            This equates to a vector (the normal of the plane, whose x, y
            and z components equate to the coefficients A, B and C
            respectively), and a constant (D) which is the distance along
            the normal you have to go to move the plane back to the origin.
     */
    class Plane
    {
	public:
		Vector3 normal;
		float d;
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

    public:
        /** Default constructor - sets everything to 0.
        */
        Plane ();
        Plane (const Plane& rhs);
        /** Construct a plane through a normal, and a distance to move the plane along the normal.*/
        Plane (const Vector3& rkNormal, float fConstant);
		/** Construct a plane using the 4 constants directly **/
		Plane (float a, float b, float c, float d);
        Plane (const Vector3& rkNormal, const Vector3& rkPoint);
        Plane (const Vector3& rkPoint0, const Vector3& rkPoint1,
            const Vector3& rkPoint2);

        Side getSide (const Vector3& rkPoint) const;

        /**
        returns the side where the aligneBox is. the flag BOTH_SIDE indicates an intersecting box.
        one corner ON the plane is sufficient to consider the box and the plane intersecting.
        */
        Side getSide (const AxisAlignedBox& rkBox) const;

		/**
		returns the side where the aligneBox is. the flag BOTH_SIDE indicates an intersecting box.
		one corner ON the plane is sufficient to consider the box and the plane intersecting.
		*/
		Side getSide(const CShapeAABB& rkBox) const;

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

        /** This is a pseudo distance. The sign of the return value is
            positive if the point is on the positive side of the plane,
            negative if the point is on the negative side, and zero if the
            point is on the plane.
            @par
            The absolute value of the return value is the true distance only
            when the plane normal is a unit length vector.
        */
        float getDistance (const Vector3& rkPoint) const;

        /** Redefine this plane based on 3 points. */
        void redefine(const Vector3& rkPoint0, const Vector3& rkPoint1,
            const Vector3& rkPoint2);

		/** Redefine this plane based on a normal and a point. */
		void redefine(const Vector3& rkNormal, const Vector3& rkPoint);

		inline Plane&	Set(float nx, float ny, float nz, float _d){ normal.x = nx; normal.y = ny; normal.z = nz; d = _d; return *this; };
		inline Plane&	Set(const Vector3& p, const Vector3& _n){
			redefine(_n, p);
			return *this;
		}
		inline Plane&	Set(const Vector3& p0, const Vector3& p1, const Vector3& p2){
			redefine(p0, p1, p2);
			return *this;
		}

		/** Project a vector onto the plane. 
		@remarks This gives you the element of the input vector that is perpendicular 
			to the normal of the plane. You can get the element which is parallel
			to the normal of the plane by subtracting the result of this method
			from the original vector, since parallel + perpendicular = original.
		@param v The input vector
		*/
		Vector3 projectVector(const Vector3& v) const;

        /** Normalizes the plane.
            @remarks
                This method normalizes the plane's normal and the length scale of d
                is as well.
            @note
                This function will not crash for zero-sized vectors, but there
                will be no changes made to their components.
            @returns The previous length of the plane's normal.
        */
        float normalise(void);


		inline float PlaneDotCoord(const Vector3& pV) const 
		{
			return normal.dotProduct(pV) + d;
		}

		inline float PlaneDotNormal(const Vector3& pV) const
		{
			return normal.dotProduct(pV);
		}
		
		Plane PlaneTransform(const Matrix4& M) const;

		inline float a() const { return normal.x; };
		inline float b() const { return normal.y; };
		inline float c() const { return normal.z; };

		// Cast operators
		operator Vector3()	const;
		operator Vector4() const;

		// Arithmetic operators
		Plane	operator*(const Matrix4& m)		const;
		Plane&	operator*=(const Matrix4& m);
		Plane operator-() const;

		operator const float*() const;
		operator float*();

        /// Comparison operator
        bool operator==(const Plane& rhs) const
        {
            return (rhs.d == d && rhs.normal == normal);
        }
        bool operator!=(const Plane& rhs) const
        {
            return (rhs.d != d && rhs.normal != normal);
        }

        friend std::ostream& operator<< (std::ostream& o, const Plane& p);
    };

    typedef std::vector<Plane> PlaneList;

} // namespace ParaEngine


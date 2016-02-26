#pragma once
#include "ParaMath.h"
#include "ParaQuaternion.h"
#include "ParaVector3.h"

namespace ParaEngine
{
	class Matrix4;

    /** 3-dimensional vector with double precision.
        @remarks
            A direction in 3D space represented as distances along the 3
            orthogonal axes (x, y, z). Note that positions, directions and
            scaling factors can be represented by a vector, depending on how
            you interpret the values.
    */
    class DVector3
    {
    public:
		double x, y, z;

    public:
		inline DVector3()
        {
        }

		inline DVector3(const double fX, const double fY, const double fZ)
            : x( fX ), y( fY ), z( fZ )
        {
        }

		inline explicit DVector3(const double afCoordinate[3])
            : x( afCoordinate[0] ), y( afCoordinate[1] ), z( afCoordinate[2] )
        {
        }

        inline explicit DVector3( const int afCoordinate[3] )
        {
            x = (double)afCoordinate[0];
            y = (double)afCoordinate[1];
            z = (double)afCoordinate[2];
        }

        inline explicit DVector3( double* const r )
            : x( r[0] ), y( r[1] ), z( r[2] )
        {
        }

        inline explicit DVector3( const double scaler )
            : x( scaler ), y( scaler ) , z( scaler )
        {
        }

		inline explicit DVector3(const Vector3& vFrom)
			: x(vFrom.x), y(vFrom.y), z(vFrom.z)
		{
		}

		inline double operator [] ( const size_t i ) const
        {
            assert( i < 3 );

            return *(&x+i);
        }

		inline double& operator [] ( const size_t i )
        {
            assert( i < 3 );

            return *(&x+i);
        }
		/// Pointer accessor for direct copying
		inline double* ptr()
		{
			return &x;
		}
		/// Pointer accessor for direct copying
		inline const double* ptr() const
		{
			return &x;
		}

		/** convert to a vector3 */
		operator Vector3 () const {
			return Vector3((float)x, (float)y, (float)z);
		};

		/*inline operator double* () { return &x; };
		inline operator const double* () const { return &x; };*/

        /** Assigns the value of the other vector.
            @param
                rkVector The other vector
        */
        inline DVector3& operator = ( const DVector3& rkVector )
        {
            x = rkVector.x;
            y = rkVector.y;
            z = rkVector.z;

            return *this;
        }

		inline DVector3& operator = (const Vector3& rkVector)
		{
			x = rkVector.x;
			y = rkVector.y;
			z = rkVector.z;
			return *this;
		}

        inline DVector3& operator = ( const double fScaler )
        {
            x = fScaler;
            y = fScaler;
            z = fScaler;

            return *this;
        }

        inline bool operator == ( const DVector3& rkVector ) const
        {
            return ( x == rkVector.x && y == rkVector.y && z == rkVector.z );
        }

        inline bool operator != ( const DVector3& rkVector ) const
        {
            return ( x != rkVector.x || y != rkVector.y || z != rkVector.z );
        }

        // arithmetic operations
        inline DVector3 operator + ( const DVector3& rkVector ) const
        {
            return DVector3(
                x + rkVector.x,
                y + rkVector.y,
                z + rkVector.z);
        }
		inline DVector3 operator + (const Vector3& rkVector) const
		{
			return DVector3(
				x + rkVector.x,
				y + rkVector.y,
				z + rkVector.z);
		}

        inline DVector3 operator - ( const DVector3& rkVector ) const
        {
            return DVector3(
                x - rkVector.x,
                y - rkVector.y,
                z - rkVector.z);
        }

		inline DVector3 operator - (const Vector3& rkVector) const
		{
			return DVector3(
				x - rkVector.x,
				y - rkVector.y,
				z - rkVector.z);
		}

        inline DVector3 operator * ( const double fScalar ) const
        {
            return DVector3(
                x * fScalar,
                y * fScalar,
                z * fScalar);
        }

		/** special cross product */
		inline DVector3 operator % (const DVector3& rhs) const
		{
			return crossProduct(rhs);
		}

        inline DVector3 operator * ( const DVector3& rhs) const
        {
            return DVector3(
                x * rhs.x,
                y * rhs.y,
                z * rhs.z);
        }

        inline DVector3 operator / ( const double fScalar ) const
        {
            assert( fScalar != 0.0f );

            double fInv = 1.0f / fScalar;

            return DVector3(
                x * fInv,
                y * fInv,
                z * fInv);
        }

        inline DVector3 operator / ( const DVector3& rhs) const
        {
            return DVector3(
                x / rhs.x,
                y / rhs.y,
                z / rhs.z);
        }

        inline const DVector3& operator + () const
        {
            return *this;
        }

        inline DVector3 operator - () const
        {
            return DVector3(-x, -y, -z);
        }

        // overloaded operators to help DVector3
        inline friend DVector3 operator * ( const double fScalar, const DVector3& rkVector )
        {
            return DVector3(
                fScalar * rkVector.x,
                fScalar * rkVector.y,
                fScalar * rkVector.z);
        }

        inline friend DVector3 operator / ( const double fScalar, const DVector3& rkVector )
        {
            return DVector3(
                fScalar / rkVector.x,
                fScalar / rkVector.y,
                fScalar / rkVector.z);
        }

        inline friend DVector3 operator + (const DVector3& lhs, const double rhs)
        {
            return DVector3(
                lhs.x + rhs,
                lhs.y + rhs,
                lhs.z + rhs);
        }

        inline friend DVector3 operator + (const double lhs, const DVector3& rhs)
        {
            return DVector3(
                lhs + rhs.x,
                lhs + rhs.y,
                lhs + rhs.z);
        }

        inline friend DVector3 operator - (const DVector3& lhs, const double rhs)
        {
            return DVector3(
                lhs.x - rhs,
                lhs.y - rhs,
                lhs.z - rhs);
        }

        inline friend DVector3 operator - (const double lhs, const DVector3& rhs)
        {
            return DVector3(
                lhs - rhs.x,
                lhs - rhs.y,
                lhs - rhs.z);
        }

        // arithmetic updates
        inline DVector3& operator += ( const DVector3& rkVector )
        {
            x += rkVector.x;
            y += rkVector.y;
            z += rkVector.z;

            return *this;
        }

		inline DVector3& operator += (const Vector3& rkVector)
		{
			x += rkVector.x;
			y += rkVector.y;
			z += rkVector.z;
			return *this;
		}

        inline DVector3& operator += ( const double fScalar )
        {
            x += fScalar;
            y += fScalar;
            z += fScalar;
            return *this;
        }

        inline DVector3& operator -= ( const DVector3& rkVector )
        {
            x -= rkVector.x;
            y -= rkVector.y;
            z -= rkVector.z;

            return *this;
        }

		inline DVector3& operator -= (const Vector3& rkVector)
		{
			x -= rkVector.x;
			y -= rkVector.y;
			z -= rkVector.z;

			return *this;
		}

        inline DVector3& operator -= ( const double fScalar )
        {
            x -= fScalar;
            y -= fScalar;
            z -= fScalar;
            return *this;
        }

        inline DVector3& operator *= ( const double fScalar )
        {
            x *= fScalar;
            y *= fScalar;
            z *= fScalar;
            return *this;
        }

        inline DVector3& operator *= ( const DVector3& rkVector )
        {
            x *= rkVector.x;
            y *= rkVector.y;
            z *= rkVector.z;

            return *this;
        }

        inline DVector3& operator /= ( const double fScalar )
        {
            assert( fScalar != 0.0f );

            double fInv = 1.0f / fScalar;

            x *= fInv;
            y *= fInv;
            z *= fInv;

            return *this;
        }

        inline DVector3& operator /= ( const DVector3& rkVector )
        {
            x /= rkVector.x;
            y /= rkVector.y;
            z /= rkVector.z;

            return *this;
        }


        /** Returns the length (magnitude) of the vector.
            @warning
                This operation requires a square root and is expensive in
                terms of CPU operations. If you don't need to know the exact
                length (e.g. for just comparing lengths) use squaredLength()
                instead.
        */
        inline double length () const
        {
            return Math::Sqrt( x * x + y * y + z * z );
        }

        /** Returns the square of the length(magnitude) of the vector.
            @remarks
                This  method is for efficiency - calculating the actual
                length of a vector requires a square root, which is expensive
                in terms of the operations required. This method returns the
                square of the length of the vector, i.e. the same as the
                length but before the square root is taken. Use this if you
                want to find the longest / shortest vector without incurring
                the square root.
        */
        inline double squaredLength () const
        {
            return x * x + y * y + z * z;
        }

        /** Returns the distance to another vector.
            @warning
                This operation requires a square root and is expensive in
                terms of CPU operations. If you don't need to know the exact
                distance (e.g. for just comparing distances) use squaredDistance()
                instead.
        */
        inline double distance(const DVector3& rhs) const
        {
            return (*this - rhs).length();
        }

        /** Returns the square of the distance to another vector.
            @remarks
                This method is for efficiency - calculating the actual
                distance to another vector requires a square root, which is
                expensive in terms of the operations required. This method
                returns the square of the distance to another vector, i.e.
                the same as the distance but before the square root is taken.
                Use this if you want to find the longest / shortest distance
                without incurring the square root.
        */
        inline double squaredDistance(const DVector3& rhs) const
        {
            return (*this - rhs).squaredLength();
        }

        /** Calculates the dot (scalar) product of this vector with another.
            @remarks
                The dot product can be used to calculate the angle between 2
                vectors. If both are unit vectors, the dot product is the
                cosine of the angle; otherwise the dot product must be
                divided by the product of the lengths of both vectors to get
                the cosine of the angle. This result can further be used to
                calculate the distance of a point from a plane.
            @param
                vec Vector with which to calculate the dot product (together
                with this one).
            @returns
                A double representing the dot product value.
        */
        inline double dotProduct(const DVector3& vec) const
        {
            return x * vec.x + y * vec.y + z * vec.z;
        }

        /** Calculates the absolute dot (scalar) product of this vector with another.
            @remarks
                This function work similar dotProduct, except it use absolute value
                of each component of the vector to computing.
            @param
                vec Vector with which to calculate the absolute dot product (together
                with this one).
            @returns
                A double representing the absolute dot product value.
        */
        inline double absDotProduct(const DVector3& vec) const
        {
            return Math::Abs(x * vec.x) + Math::Abs(y * vec.y) + Math::Abs(z * vec.z);
        }

        /** Normalises the vector.
            @remarks
                This method normalises the vector such that it's
                length / magnitude is 1. The result is called a unit vector.
            @note
                This function will not crash for zero-sized vectors, but there
                will be no changes made to their components.
            @returns The previous length of the vector.
        */
        inline double normalise()
        {
            double fLength = Math::Sqrt( x * x + y * y + z * z );

            // Will also work for zero-sized vectors, but will change nothing
            if ( fLength > 1e-08 )
            {
                double fInvLength = 1.0f / fLength;
                x *= fInvLength;
                y *= fInvLength;
                z *= fInvLength;
            }

            return fLength;
        }

        /** Calculates the cross-product of 2 vectors, i.e. the vector that
            lies perpendicular to them both.
            @remarks
                The cross-product is normally used to calculate the normal
                vector of a plane, by calculating the cross-product of 2
                non-equivalent vectors which lie on the plane (e.g. 2 edges
                of a triangle).
            @param
                vec Vector which, together with this one, will be used to
                calculate the cross-product.
            @returns
                A vector which is the result of the cross-product. This
                vector will <b>NOT</b> be normalised, to maximise efficiency
                - call DVector3::normalise on the result if you wish this to
                be done. As for which side the resultant vector will be on, the
                returned vector will be on the side from which the arc from 'this'
                to rkVector is anticlockwise, e.g. UNIT_Y.crossProduct(UNIT_Z)
                = UNIT_X, whilst UNIT_Z.crossProduct(UNIT_Y) = -UNIT_X.
				This is because OGRE uses a right-handed coordinate system.
            @par
                For a clearer explanation, look a the left and the bottom edges
                of your monitor's screen. Assume that the first vector is the
                left edge and the second vector is the bottom edge, both of
                them starting from the lower-left corner of the screen. The
                resulting vector is going to be perpendicular to both of them
                and will go <i>inside</i> the screen, towards the cathode tube
                (assuming you're using a CRT monitor, of course).
        */
        inline DVector3 crossProduct( const DVector3& rkVector ) const
        {
            return DVector3(
                y * rkVector.z - z * rkVector.y,
                z * rkVector.x - x * rkVector.z,
                x * rkVector.y - y * rkVector.x);
        }

        /** Returns a vector at a point half way between this and the passed
            in vector.
        */
        inline DVector3 midPoint( const DVector3& vec ) const
        {
            return DVector3(
                ( x + vec.x ) * 0.5f,
                ( y + vec.y ) * 0.5f,
                ( z + vec.z ) * 0.5f );
        }

        /** Returns true if the vector's scalar components are all greater
            that the ones of the vector it is compared against.
        */
        inline bool operator < ( const DVector3& rhs ) const
        {
            if( x < rhs.x && y < rhs.y && z < rhs.z )
                return true;
            return false;
        }

        /** Returns true if the vector's scalar components are all smaller
            that the ones of the vector it is compared against.
        */
        inline bool operator > ( const DVector3& rhs ) const
        {
            if( x > rhs.x && y > rhs.y && z > rhs.z )
                return true;
            return false;
        }

        /** Sets this vector's components to the minimum of its own and the
            ones of the passed in vector.
            @remarks
                'Minimum' in this case means the combination of the lowest
                value of x, y and z from both vectors. Lowest is taken just
                numerically, not magnitude, so -1 < 0.
        */
        inline void makeFloor( const DVector3& cmp )
        {
            if( cmp.x < x ) x = cmp.x;
            if( cmp.y < y ) y = cmp.y;
            if( cmp.z < z ) z = cmp.z;
        }

        /** Sets this vector's components to the maximum of its own and the
            ones of the passed in vector.
            @remarks
                'Maximum' in this case means the combination of the highest
                value of x, y and z from both vectors. Highest is taken just
                numerically, not magnitude, so 1 > -3.
        */
        inline void makeCeil( const DVector3& cmp )
        {
            if( cmp.x > x ) x = cmp.x;
            if( cmp.y > y ) y = cmp.y;
            if( cmp.z > z ) z = cmp.z;
        }

        /** Returns true if this vector is zero length. */
        inline bool isZeroLength(void) const
        {
            double sqlen = (x * x) + (y * y) + (z * z);
            return (sqlen < (1e-06 * 1e-06));

        }

        /** As normalise, except that this vector is unaffected and the
            normalized vector is returned as a copy. */
        inline DVector3 normalisedCopy(void) const
        {
            DVector3 ret = *this;
            ret.normalise();
            return ret;
        }

        /** Calculates a reflection vector to the plane with the given normal .
        @remarks NB assumes 'this' is pointing AWAY FROM the plane, invert if it is not.
        */
        inline DVector3 reflect(const DVector3& normal) const
        {
            return DVector3( *this - ( 2 * this->dotProduct(normal) * normal ) );
        }

		/** Returns whether this vector is within a positional tolerance
			of another floating precision vector.
		@param rhs The vector to compare with
		@param tolerance The amount that each element of the vector may vary by
			and still be considered equal
		*/
		inline bool positionEquals(const Vector3& rhs, float tolerance = 1e-03) const
		{
			return Math::RealEqual((float)x, rhs.x, tolerance) &&
				Math::RealEqual((float)y, rhs.y, tolerance) &&
				Math::RealEqual((float)z, rhs.z, tolerance);
		}

		/** Returns whether this vector is within a positional tolerance
			of another vector, also take scale of the vectors into account.
		@param rhs The vector to compare with
		@param tolerance The amount (related to the scale of vectors) that distance
            of the vector may vary by and still be considered close
		*/
		inline bool positionCloses(const DVector3& rhs, float tolerance = 1e-03f) const
		{
			return squaredDistance(rhs) <=
                (squaredLength() + rhs.squaredLength()) * tolerance;
		}

		/** Returns whether this vector is within a directional tolerance
			of another vector.
		@param rhs The vector to compare with
		@param tolerance The maximum angle by which the vectors may vary and
			still be considered equal
		@note Both vectors should be normalized.
		*/
		inline bool directionEquals(const DVector3& rhs,
			const Radian& tolerance) const
		{
			double dot = dotProduct(rhs);
			Radian angle = Math::ACos((float)dot);

			return Math::Abs(angle.valueRadians()) <= tolerance.valueRadians();
		}

        /** Function for writing to a stream.
        */
        inline friend std::ostream& operator <<
            ( std::ostream& o, const DVector3& v )
        {
            o << "DVector3(" << v.x << ", " << v.y << ", " << v.z << ")";
            return o;
        }
    };

}

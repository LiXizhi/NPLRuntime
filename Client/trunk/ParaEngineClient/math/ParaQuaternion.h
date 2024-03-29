﻿#pragma once
#include "ParaMath.h"

namespace ParaEngine {

    /** Implementation of a Quaternion, i.e. a rotation around an axis.
	* In ParaEngine, we use row matrix which is compatible with DirectX. 
	* so the FromRotationMatrix, and ToRotationMatrix will generate row matrix. 
    */
    class Quaternion
    {
	public:
		// special values
		static const Quaternion ZERO;
		static const Quaternion IDENTITY;

		float x, y, z, w;
    public:
		inline Quaternion(float fX = 0.0, float fY = 0.0, float fZ = 0.0, float fW = 1.0)
		{
			x = fX;
			y = fY;
			z = fZ;
			w = fW;
		}
        /// Construct a quaternion from a rotation matrix
        inline Quaternion(const Matrix3& rot)
        {
            this->FromRotationMatrix(rot);
        }

		inline Quaternion(const Matrix4& rot)
		{
			this->FromRotationMatrix(rot);
		}

		/** This constructor creates a new quaternion that will rotate vector
		* a into vector b about their mutually perpendicular axis. (if one exists)
		*/
		Quaternion(const Vector3& a, const Vector3& b);

        /// Construct a quaternion from an angle/axis
        inline Quaternion(const Radian& rfAngle, const Vector3& rkAxis)
        {
            this->FromAngleAxis(rfAngle, rkAxis);
        }

		/// Construct a quaternion from an angle/axis
		inline Quaternion(const Vector3& rkAxis, float fAngle)
		{
			this->FromAngleAxis(Radian(fAngle), rkAxis);
		}


        /// Construct a quaternion from 3 orthonormal local axes
        inline Quaternion(const Vector3& xaxis, const Vector3& yaxis, const Vector3& zaxis)
        {
            this->FromAxes(xaxis, yaxis, zaxis);
        }
        /// Construct a quaternion from 3 orthonormal local axes
        inline Quaternion(const Vector3* akAxis)
        {
            this->FromAxes(akAxis);
        }
		/// Construct a quaternion from 4 manual w/x/y/z values
		inline Quaternion(float* valptr)
		{
			memcpy(&x, valptr, sizeof(float)*4);
		}

		/// Array accessor operator
		inline float operator [] ( const size_t i ) const
		{
			assert( i < 4 );

			return *(&x+i);
		}

		/// Array accessor operator
		inline float& operator [] ( const size_t i )
		{
			assert( i < 4 );

			return *(&x+i);
		}

		/// Pointer accessor for direct copying
		inline float* ptr()
		{
			return &x;
		}

		/// Pointer accessor for direct copying
		inline const float* ptr() const
		{
			return &x;
		}

		void FromRotationMatrix (const Matrix3& kRot);
		void FromRotationMatrix(const Matrix4& kRot);

        void ToRotationMatrix (Matrix3& kRot) const;
		void ToRotationMatrix(Matrix4& kRot, const Vector3& Origin) const;
        void FromAngleAxis (const Radian& rfAngle, const Vector3& rkAxis);
        void ToAngleAxis (Radian& rfAngle, Vector3& rkAxis) const;
        inline void ToAngleAxis (Degree& dAngle, Vector3& rkAxis) const {
            Radian rAngle;
            ToAngleAxis ( rAngle, rkAxis );
            dAngle = rAngle;
        }
        void FromAxes (const Vector3* akAxis);
        void FromAxes (const Vector3& xAxis, const Vector3& yAxis, const Vector3& zAxis);
        void ToAxes (Vector3* akAxis) const;
        void ToAxes (Vector3& xAxis, Vector3& yAxis, Vector3& zAxis) const;
        /// Get the local x-axis
        Vector3 xAxis(void) const;
        /// Get the local y-axis
        Vector3 yAxis(void) const;
        /// Get the local z-axis
        Vector3 zAxis(void) const;

        inline Quaternion& operator= (const Quaternion& rkQ)
		{
			x = rkQ.x;
			y = rkQ.y;
			z = rkQ.z;
			w = rkQ.w;
			return *this;
		}
        Quaternion operator+ (const Quaternion& rkQ) const;
        Quaternion operator- (const Quaternion& rkQ) const;
        Quaternion operator* (const Quaternion& rkQ) const;
        Quaternion operator* (float fScalar) const;
        friend Quaternion operator* (float fScalar,
            const Quaternion& rkQ);
        Quaternion operator- () const;
        inline bool operator== (const Quaternion& rhs) const
		{
			return (rhs.x == x) && (rhs.y == y) && (rhs.z == z) && (rhs.w == w);
		}
        inline bool operator!= (const Quaternion& rhs) const
		{
			return !operator==(rhs);
		}
        // functions of a quaternion
        float Dot (const Quaternion& rkQ) const;  // dot product
        float Norm () const;  // squared-length
        /// Normalizes this quaternion, and returns the previous length
        float normalise(void); 
        Quaternion Inverse () const;  // apply to non-zero quaternion
        Quaternion UnitInverse () const;  // apply to unit-length quaternion
        Quaternion Exp () const;
        Quaternion Log () const;

		inline Quaternion& invertWinding() {
			x = -x; y = -y; z = -z;
			return *this;
		}

        // rotation of a vector by a quaternion
        Vector3 operator* (const Vector3& rkVector) const;

   		/** Calculate the local roll element of this quaternion.
		@param reprojectAxis By default the method returns the 'intuitive' result
			that is, if you projected the local Y of the quaternion onto the X and
			Y axes, the angle between them is returned. If set to false though, the
			result is the actual yaw that will be used to implement the quaternion,
			which is the shortest possible path to get to the same orientation and 
			may involve less axial rotation. 
		*/
		Radian getRoll(bool reprojectAxis = true) const;
   		/** Calculate the local pitch element of this quaternion
		@param reprojectAxis By default the method returns the 'intuitive' result
			that is, if you projected the local Z of the quaternion onto the X and
			Y axes, the angle between them is returned. If set to true though, the
			result is the actual yaw that will be used to implement the quaternion,
			which is the shortest possible path to get to the same orientation and 
			may involve less axial rotation. 
		*/
		Radian getPitch(bool reprojectAxis = true) const;
   		/** Calculate the local yaw element of this quaternion
		@param reprojectAxis By default the method returns the 'intuitive' result
			that is, if you projected the local Z of the quaternion onto the X and
			Z axes, the angle between them is returned. If set to true though, the
			result is the actual yaw that will be used to implement the quaternion,
			which is the shortest possible path to get to the same orientation and 
			may involve less axial rotation. 
		*/
		Radian getYaw(bool reprojectAxis = true) const;	
		
		void threeaxisrot(float r11, float r12, float r21, float r31, float r32, float& out1, float& out2, float& out3) const;
		
	    void ToEulerAnglesSequence(float& pitch, float& yaw, float& roll, const std::string& rotSeq = "xyz") const;

		void	ToRadians(float *outX, float *outY, float *outZ) const;

		/// Equality with tolerance (tolerance is max angle difference)
		bool equals(const Quaternion& rhs, const Radian& tolerance) const;
		
	    // spherical linear interpolation
        static Quaternion Slerp (float fT, const Quaternion& rkP, const Quaternion& rkQ, bool shortestPath = false);

        static Quaternion SlerpExtraSpins (float fT, const Quaternion& rkP, const Quaternion& rkQ,
            int iExtraSpins);

        // setup for spherical quadratic interpolation
        static void Intermediate (const Quaternion& rkQ0, const Quaternion& rkQ1, const Quaternion& rkQ2, Quaternion& rka, Quaternion& rkB);

        // spherical quadratic interpolation
        static Quaternion Squad (float fT, const Quaternion& rkP, const Quaternion& rkA, const Quaternion& rkB, const Quaternion& rkQ, bool shortestPath = false);

        // normalised linear interpolation - faster but less accurate (non-constant rotation velocity)
        static Quaternion nlerp(float fT, const Quaternion& rkP, const Quaternion& rkQ, bool shortestPath = false);

        // cutoff for sine near zero
        static const float ms_fEpsilon;

        /** Function for writing to a stream. Outputs "Quaternion(x, y, z, w)" with x,y,z,w
            being the member values of the quaternion.
        */
        inline friend std::ostream& operator <<
            ( std::ostream& o, const Quaternion& q )
        {
            o << "Quaternion(" << q.x << ", " << q.y << ", " << q.z << ", " << q.w << ")";
            return o;
        }

    };

}

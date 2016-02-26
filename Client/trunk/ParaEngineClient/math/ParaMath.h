#pragma once
#include <limits>
#include <stdint.h>
#include "ParaAngle.h"

#if !defined(WIN32) && !defined(INT64_C)
#  define INT64_C(c) static_cast<long long>(c ## LL)     /* signed 64 bit constant */
#  define UINT64_C(c) static_cast<unsigned long long>(c ## ULL) /* unsigned 64 bit constant */
#endif

namespace ParaEngine
{
	class CShapeBox;
	class CShapeSphere;
	class Matrix4;
	class DVector3;
	class Vector3;

	/** standard math lib */
    class Math 
    {
   public:
       /** The angular units used by the API. This functionality is now deprecated in favor
	       of discreet angular unit types ( see Degree and Radian above ). The only place
		   this functionality is actually still used is when parsing files. Search for
		   usage of the Angle class for those instances
       */
       enum AngleUnit
       {
           AU_DEGREE,
           AU_RADIAN
       };

	   static const float POS_INFINITY;
	   static const float NEG_INFINITY;
	   static const float PI;
	   static const float TWO_PI;
	   static const float HALF_PI;
	   static const float fDeg2Rad;
	   static const float fRad2Deg;

    protected:
		// angle units used by the api
		static AngleUnit msAngleUnit;

        /// Size of the trig tables as determined by constructor.
        static int mTrigTableSize;

        /// Radian -> index factor value ( mTrigTableSize / 2 * PI )
        static float mTrigTableFactor;
        static float* mSinTable;
        static float* mTanTable;

        /** Private function to build trig tables.
        */
        void buildTrigTables();

		static float SinTable (float fValue);
		static float TanTable (float fValue);
    public:
        /** Default constructor.
            @param
                trigTableSize Optional parameter to set the size of the
                tables used to implement Sin, Cos, Tan
        */
        Math(unsigned int trigTableSize = 4096);

        /** Default destructor.
        */
        ~Math();

		static inline int IAbs (int iValue) { return ( iValue >= 0 ? iValue : -iValue ); }
		static inline int ICeil (float fValue) { return int(ceil(fValue)); }
		static inline int IFloor (float fValue) { return int(floor(fValue)); }
        static int ISign (int iValue);

		static inline float Abs (float fValue) { return float(fabs(fValue)); }
		static inline double Abs(double fValue) { return fabs(fValue); }
		static inline int Abs(int nValue) { return nValue >= 0 ? nValue : -nValue; }
		static inline Degree Abs (const Degree& dValue) { return Degree(fabs(dValue.valueDegrees())); }
		static inline Radian Abs (const Radian& rValue) { return Radian(fabs(rValue.valueRadians())); }
		static Radian ACos (float fValue);
		static Radian ASin (float fValue);
		static inline Radian ATan (float fValue) { return Radian(atan(fValue)); }
		static inline Radian ATan2 (float fY, float fX) { return Radian(atan2(fY,fX)); }
		static inline float Ceil (float fValue) { return float(ceil(fValue)); }

        /** Cosine function.
            @param
                fValue Angle in radians
            @param
                useTables If true, uses lookup tables rather than
                calculation - faster but less accurate.
        */
        static inline float Cos (const Radian& fValue, bool useTables = false) {
			return (!useTables) ? float(cos(fValue.valueRadians())) : SinTable(fValue.valueRadians() + HALF_PI);
		}
        /** Cosine function.
            @param
                fValue Angle in radians
            @param
                useTables If true, uses lookup tables rather than
                calculation - faster but less accurate.
        */
        static inline float Cos (float fValue, bool useTables = false) {
			return (!useTables) ? float(cos(fValue)) : SinTable(fValue + HALF_PI);
		}

		static inline float Exp (float fValue) { return float(exp(fValue)); }

		static inline float Floor (float fValue) { return float(floor(fValue)); }

		static inline float Log (float fValue) { return float(log(fValue)); }

		static inline float Pow (float fBase, float fExponent) { return float(pow(fBase,fExponent)); }

        static float Sign (float fValue);
		static inline Radian Sign ( const Radian& rValue )
		{
			return Radian(Sign(rValue.valueRadians()));
		}
		static inline Degree Sign ( const Degree& dValue )
		{
			return Degree(Sign(dValue.valueDegrees()));
		}

        /** Sine function.
            @param
                fValue Angle in radians
            @param
                useTables If true, uses lookup tables rather than
                calculation - faster but less accurate.
        */
        static inline float Sin (const Radian& fValue, bool useTables = false) {
			return (!useTables) ? float(sin(fValue.valueRadians())) : SinTable(fValue.valueRadians());
		}
        /** Sine function.
            @param
                fValue Angle in radians
            @param
                useTables If true, uses lookup tables rather than
                calculation - faster but less accurate.
        */
        static inline float Sin (float fValue, bool useTables = false) {
			return (!useTables) ? float(sin(fValue)) : SinTable(fValue);
		}

		static inline float Sqr (float fValue) { return fValue*fValue; }
		static inline float Sqrt (float fValue) { return float(sqrt(fValue)); }

		static inline double Sqr(double fValue) { return fValue*fValue; }
		static inline double Sqrt(double fValue) { return sqrt(fValue); }

        static inline Radian Sqrt (const Radian& fValue) { return Radian(sqrt(fValue.valueRadians())); }

        static inline Degree Sqrt (const Degree& fValue) { return Degree(sqrt(fValue.valueDegrees())); }

        /** Inverse square root i.e. 1 / Sqrt(x), good for vector
            normalization.
        */
		static float InvSqrt(float fValue);

        static float UnitRandom ();  // in [0,1]

        static float RangeRandom (float fLow, float fHigh);  // in [fLow,fHigh]

        static float SymmetricRandom ();  // in [-1,1]

        /** Tangent function.
            @param
                fValue Angle in radians
            @param
                useTables If true, uses lookup tables rather than
                calculation - faster but less accurate.
        */
		static inline float Tan (const Radian& fValue, bool useTables = false) {
			return (!useTables) ? float(tan(fValue.valueRadians())) : TanTable(fValue.valueRadians());
		}
        /** Tangent function.
            @param
                fValue Angle in radians
            @param
                useTables If true, uses lookup tables rather than
                calculation - faster but less accurate.
        */
		static inline float Tan (float fValue, bool useTables = false) {
			return (!useTables) ? float(tan(fValue)) : TanTable(fValue);
		}

		static inline float DegreesToRadians(float degrees) { return degrees * fDeg2Rad; }
        static inline float RadiansToDegrees(float radians) { return radians * fRad2Deg; }

       /** These functions used to set the assumed angle units (radians or degrees) 
            expected when using the Angle type.
       @par
            You can set this directly after creating a new Root, and also before/after resource creation,
            depending on whether you want the change to affect resource files.
       */
       static void setAngleUnit(AngleUnit unit);
       /** Get the unit being used for angles. */
       static AngleUnit getAngleUnit(void);

       /** Convert from the current AngleUnit to radians. */
       static float AngleUnitsToRadians(float units);
       /** Convert from radians to the current AngleUnit . */
       static float RadiansToAngleUnits(float radians);
       /** Convert from the current AngleUnit to degrees. */
       static float AngleUnitsToDegrees(float units);
       /** Convert from degrees to the current AngleUnit. */
       static float DegreesToAngleUnits(float degrees);

       /** Checks whether a given point is inside a triangle, in a
            2-dimensional (Cartesian) space.
            @remarks
                The vertices of the triangle must be given in either
                trigonometrical (anticlockwise) or inverse trigonometrical
                (clockwise) order.
            @param
                p The point.
            @param
                a The triangle's first vertex.
            @param
                b The triangle's second vertex.
            @param
                c The triangle's third vertex.
            @returns
                If the point resides in the triangle, <b>true</b> is
                returned.
            @par
                If the point is outside the triangle, <b>false</b> is
                returned.
        */
        static bool pointInTri2D(const Vector2& p, const Vector2& a, 
			const Vector2& b, const Vector2& c);

       /** Checks whether a given 3D point is inside a triangle.
       @remarks
            The vertices of the triangle must be given in either
            trigonometrical (anticlockwise) or inverse trigonometrical
            (clockwise) order, and the point must be guaranteed to be in the
			same plane as the triangle
        @param
            p The point.
        @param
            a The triangle's first vertex.
        @param
            b The triangle's second vertex.
        @param
            c The triangle's third vertex.
		@param 
			normal The triangle plane's normal (passed in rather than calculated
				on demand since the caller may already have it)
        @returns
            If the point resides in the triangle, <b>true</b> is
            returned.
        @par
            If the point is outside the triangle, <b>false</b> is
            returned.
        */
        static bool pointInTri3D(const Vector3& p, const Vector3& a, 
			const Vector3& b, const Vector3& c, const Vector3& normal);
        /** Ray / plane intersection, returns boolean result and distance. */
        static std::pair<bool, float> intersects(const Ray& ray, const Plane& plane);

        /** Ray / sphere intersection, returns boolean result and distance. */
        static std::pair<bool, float> intersects(const Ray& ray, const Sphere& sphere, 
            bool discardInside = true);
        
        /** Ray / box intersection, returns boolean result and distance. */
        static std::pair<bool, float> intersects(const Ray& ray, const AxisAlignedBox& box);

        /** Ray / box intersection, returns boolean result and two intersection distance.
        @param
            ray The ray.
        @param
            box The box.
        @param
            d1 A real pointer to retrieve the near intersection distance
                from the ray origin, maybe <b>null</b> which means don't care
                about the near intersection distance.
        @param
            d2 A real pointer to retrieve the far intersection distance
                from the ray origin, maybe <b>null</b> which means don't care
                about the far intersection distance.
        @returns
            If the ray is intersects the box, <b>true</b> is returned, and
            the near intersection distance is return by <i>d1</i>, the
            far intersection distance is return by <i>d2</i>. Guarantee
            <b>0</b> <= <i>d1</i> <= <i>d2</i>.
        @par
            If the ray isn't intersects the box, <b>false</b> is returned, and
            <i>d1</i> and <i>d2</i> is unmodified.
        */
        static bool intersects(const Ray& ray, const AxisAlignedBox& box,
            float* d1, float* d2);

        /** Ray / triangle intersection, returns boolean result and distance.
        @param
            ray The ray.
        @param
            a The triangle's first vertex.
        @param
            b The triangle's second vertex.
        @param
            c The triangle's third vertex.
		@param 
			normal The triangle plane's normal (passed in rather than calculated
				on demand since the caller may already have it), doesn't need
                normalised since we don't care.
        @param
            positiveSide Intersect with "positive side" of the triangle
        @param
            negativeSide Intersect with "negative side" of the triangle
        @returns
            If the ray is intersects the triangle, a pair of <b>true</b> and the
            distance between intersection point and ray origin returned.
        @par
            If the ray isn't intersects the triangle, a pair of <b>false</b> and
            <b>0</b> returned.
        */
        static std::pair<bool, float> intersects(const Ray& ray, const Vector3& a,
            const Vector3& b, const Vector3& c, const Vector3& normal,
            bool positiveSide = true, bool negativeSide = true);

        /** Ray / triangle intersection, returns boolean result and distance.
        @param
            ray The ray.
        @param
            a The triangle's first vertex.
        @param
            b The triangle's second vertex.
        @param
            c The triangle's third vertex.
        @param
            positiveSide Intersect with "positive side" of the triangle
        @param
            negativeSide Intersect with "negative side" of the triangle
        @returns
            If the ray is intersects the triangle, a pair of <b>true</b> and the
            distance between intersection point and ray origin returned.
        @par
            If the ray isn't intersects the triangle, a pair of <b>false</b> and
            <b>0</b> returned.
        */
        static std::pair<bool, float> intersects(const Ray& ray, const Vector3& a,
            const Vector3& b, const Vector3& c,
            bool positiveSide = true, bool negativeSide = true);

        /** Sphere / box intersection test. */
        static bool intersects(const Sphere& sphere, const AxisAlignedBox& box);
		static bool intersects(const CShapeSphere& sphere, const CShapeBox& box);
        /** Plane / box intersection test. */
        static bool intersects(const Plane& plane, const AxisAlignedBox& box);

        /** Ray / convex plane list intersection test. 
        @param ray The ray to test with
        @param plaeList List of planes which form a convex volume
        @param normalIsOutside Does the normal point outside the volume
        */
        static std::pair<bool, float> intersects(
            const Ray& ray, const std::vector<Plane>& planeList, 
            bool normalIsOutside);
        /** Ray / convex plane list intersection test. 
        @param ray The ray to test with
        @param plaeList List of planes which form a convex volume
        @param normalIsOutside Does the normal point outside the volume
        */
        static std::pair<bool, float> intersects(
            const Ray& ray, const std::list<Plane>& planeList, 
            bool normalIsOutside);

        /** Sphere / plane intersection test. 
        @remarks NB just do a plane.getDistance(sphere.getCenter()) for more detail!
        */
        static bool intersects(const Sphere& sphere, const Plane& plane);

        /** Compare 2 reals, using tolerance for inaccuracies.
        */
        static bool RealEqual(float a, float b,
            float tolerance = std::numeric_limits<float>::epsilon());

        /** Calculates the tangent space vector for a given set of positions / texture coords. */
        static Vector3 calculateTangentSpaceVector(
            const Vector3& position1, const Vector3& position2, const Vector3& position3,
            float u1, float v1, float u2, float v2, float u3, float v3);

        /** Build a reflection matrix for the passed in plane. */
        static Matrix4 buildReflectionMatrix(const Plane& p);
        /** Calculate a face normal, including the w component which is the offset from the origin. */
        static Vector4 calculateFaceNormal(const Vector3& v1, const Vector3& v2, const Vector3& v3);
        /** Calculate a face normal, no w-information. */
        static Vector3 calculateBasicFaceNormal(const Vector3& v1, const Vector3& v2, const Vector3& v3);
        /** Calculate a face normal without normalize, including the w component which is the offset from the origin. */
        static Vector4 calculateFaceNormalWithoutNormalize(const Vector3& v1, const Vector3& v2, const Vector3& v3);
        /** Calculate a face normal without normalize, no w-information. */
        static Vector3 calculateBasicFaceNormalWithoutNormalize(const Vector3& v1, const Vector3& v2, const Vector3& v3);

		/** Generates a value based on the Gaussian (normal) distribution function
			with the given offset and scale parameters.
		*/
		static float gaussianDistribution(float x, float offset = 0.0f, float scale = 1.0f);

		/** Clamps X to be between Min and Max, inclusive */
		template< class T >
		static inline T Clamp(const T X, const T Min, const T Max)
		{
			return X < Min ? Min : X < Max ? X : Max;
		}

		static Matrix4 makeViewMatrix(const Vector3& position, const Quaternion& orientation, 
			const Matrix4* reflectMatrix = 0);

		/** Returns whether two floating-point values are equivalent within a given tolerance.
		@param f1 A first floating-point value.
		@param f2 A second floating-point value. */
		static inline bool IsEquivalent(float f1, float f2) { return f1 - f2 < FLT_TOLERANCE && f2 - f1 < FLT_TOLERANCE; }

		/** Returns whether two floating-point values are equivalent within a given tolerance.
		@param f1 A first floating-point value.
		@param f2 A second floating-point value.
		@param tolerance The tolerance in which to accept the two floating-point values as equivalent. */
		static inline bool IsEquivalent(float f1, float f2, float tolerance) { return f1 - f2 < tolerance && f2 - f1 < tolerance; }

		/** Returns whether two double-sized floating-point values are equivalent.
		@param f1 A first double-sized floating-point value.
		@param f2 A second double-sized floating-point value. */
		static inline bool IsEquivalent(double f1, double f2) { return f1 - f2 < DBL_TOLERANCE && f2 - f1 < DBL_TOLERANCE; }

		/** Returns whether two double-sized floating-point values are equivalent within a given tolerance.
		@param f1 A first double-sized floating-point value.
		@param f2 A second double-sized floating-point value.
		@param tolerance The tolerance in which to accept the two double-sized floating-point values as equivalent. */
		static inline bool IsEquivalent(double f1, double f2, double tolerance) { return f1 - f2 < tolerance && f2 - f1 < tolerance; }

		/** Returns whether two constant-sized arrays are equivalent.
		@param al A first constant-sized array.
		@param acount The number of elements in the first array.
		@param bl A second constant-sized array.
		@param bcount The number of elements in the second array.
		@return Whether the two arrays are equivalent. */
		template <class T>
		static inline bool IsEquivalent(const T* al, size_t acount, const T* bl, size_t bcount)
		{
			if (acount != bcount) return false;
			bool equivalent = true;
			for (size_t i = 0; i < acount && equivalent; ++i)
			{
				equivalent = IsEquivalent(al[i], bl[i]);
			}
			return equivalent;
		}

		/** this is a fast version of log2. return the smaller*/
		static int log2 (unsigned int x);

		/** this is a fast version of log2. return the bigger */
		static int log2_ceil(unsigned int x) {return (log2(x-1) + 1);}

		/** Returns higher value in a generic way */
		template< class T >
		static inline T Max(const T A, const T B)
		{
			return (A >= B) ? A : B;
		}
		/** Returns lower value in a generic way */
		template< class T >
		static inline T Min(const T A, const T B)
		{
			return (A <= B) ? A : B;
		}

		/**
		* Converts a float to an integer with truncation towards zero.
		* @param F		Floating point value to convert
		* @return		Truncated integer.
		*/
		static inline int32 Trunc(float F)
		{
			return (int32)F;
		}
		/**
		* Converts a float to an integer value with truncation towards zero.
		* @param F		Floating point value to convert
		* @return		Truncated integer value.
		*/
		static inline float TruncFloat(float F)
		{
			return (float)Trunc(F);
		}

		/**
		* Returns the fractional part of a float.
		* @param Value	Floating point value to convert
		* @return		A float between >=0 and < 1.
		*/
		static inline float Fractional(float Value)
		{
			return Value - TruncFloat(Value);
		}

		/** Snaps a value to the nearest grid multiple */
		static inline float GridSnap(float Location, float Grid)
		{
			if (Grid == 0.f)	return Location;
			else
			{
				return Floor((Location + 0.5f*Grid) / Grid)*Grid;
			}
		}
	

		/** Divides two integers and rounds up */
		template <class T>
		static inline T DivideAndRoundUp(T Dividend, T Divisor)
		{
			return (Dividend + Divisor - 1) / Divisor;
		}

		/** Returns highest of 3 values */
		template< class T >
		static inline T Max3(const T A, const T B, const T C)
		{
			return Max(Max(A, B), C);
		}

		/** Returns lowest of 3 values */
		template< class T >
		static inline T Min3(const T A, const T B, const T C)
		{
			return Min(Min(A, B), C);
		}

		/** Performs a linear interpolation between two values, Alpha ranges from 0-1 */
		template< class T, class U >
		static inline T Lerp(const T& A, const T& B, const U& Alpha)
		{
			return (T)(A + Alpha * (B - A));
		}

		/** Returns a random integer between 0 and RAND_MAX, inclusive */
		static inline int32 Rand() { return rand(); }

		/** Seeds global random number functions Rand() and FRand() */
		static inline void RandInit(int32 Seed) { srand(Seed); }

		/** Returns a random float between 0 and 1, inclusive. */
		static inline float FRand() { return rand() / (float)RAND_MAX; }

		/** Seeds future calls to SRand() */
		static void SRandInit(int32 Seed);

		/** Returns a seeded random float in the range [0,1), using the seed from SRandInit(). */
		static float SRand();

		/**
		* Returns value based on comparand. The main purpose of this function is to avoid
		* branching based on floating point comparison which can be avoided via compiler
		* intrinsics.
		*
		* Please note that we don't define what happens in the case of NaNs as there might
		* be platform specific differences.
		*
		* @param	Comparand		Comparand the results are based on
		* @param	ValueGEZero		Return value if Comparand >= 0
		* @param	ValueLTZero		Return value if Comparand < 0
		*
		* @return	ValueGEZero if Comparand >= 0, ValueLTZero otherwise
		*/
		static inline float FloatSelect(float Comparand, float ValueGEZero, float ValueLTZero)
		{
			return Comparand >= 0.f ? ValueGEZero : ValueLTZero;
		}

		/** change src from src to dest, by a maximum of fMaxStep. If dest has been reached, return true; otherwise return false.*/
		static bool SmoothMoveFloat1(float& src, const float dest, const float fMaxStep);

		/** change src from src to dest, by a maximum of fMaxStep. If dest has been reached, return true; otherwise return false.*/
		static bool SmoothMoveAngle1(float& src, const float dest, const float fMaxStep);

		/**
		* linearly (smoothly) move vPos to vPosTarget by the amount fIncrease return true if we have reached the target.
		* if the distance between target and the pos is within fTolerance, result is set to target
		*/
		static bool SmoothMoveVec3(Vector3* result, const Vector3& vPosTarget, const Vector3& vPos, FLOAT fIncrease, FLOAT fTolerance/*=0*/);

		/** make sure that the angle is in the range (-Pi,Pi]*/
		static float ToStandardAngle(float fAngle);
		static double ToStandardAngle(double fAngle);

		/** check if the matchStr matches the string pattern in matchPattern
		* @param matchPattern: the match pattern. It may end with "*"
		*	in which case "*" will match any character(s).
		*	e.g "Li*" will match both "LiXizhi" and "LiTC", whereas "Li" will only match "Li"
		* @param matchStr: the match string.
		* @return: return true if the two string matches. */
		static bool MatchString(const std::string& matchPattern, const std::string& matchStr);

		/** facing target in xz plane
		* @params fFacing: [out] it is in the range [0, 2PI]
		* return true if target!=source(a valid facing can be computed)
		*/
		static bool ComputeFacingTarget(const Vector3& target, const Vector3& source, FLOAT& fFacing);
		static bool ComputeFacingTarget(const DVector3& target, const DVector3& source, float& fFacing);
		/**
		* get the closest integer near the specified float number.
		* E.g. Round(1.49)==1, Round(1.51)==2, ToInt(
		*/
		static inline int Round(float fValue){
			return int(fValue > 0.0f ? fValue + 0.5f : fValue - 0.5f);
		}

		/// Returns MIN(x, y, z);
		static float MinVec3(const Vector3& v);
		/// Returns MAX(x, y, z);
		static float MaxVec3(const Vector3& v);

		/** return true if equal. T is a 3D vector, such as Vector3 */
		template <class T>
		static inline bool CompareXZ(const T& a, const T& b)
		{
			return (a.x == b.x && a.z == b.z);
		}

		/** return true if equal. T is a 3D vector, such as Vector3 */
		template <class T>
		static bool CompareXZ(const T& a, const T& b, float epsilon)
		{
			return (fabs(a.x - b.x) + fabs(a.z - b.z)) < epsilon;
		}

		static inline bool FuzzyCompare(double p1, double p2)
		{
			return (Math::Abs(p1 - p2) * 1000000000000. <= Math::Min(Math::Abs(p1), Math::Abs(p2)));
		}

		static bool FuzzyCompare(float p1, float p2)
		{
			return (Math::Abs(p1 - p2) * 100000.f <= Math::Min(Math::Abs(p1), Math::Abs(p2)));
		}

		static inline bool FuzzyIsNull(double d)
		{
			return Math::Abs(d) <= 0.000000000001;
		}

		static inline bool FuzzyIsNull(float f)
		{
			return Math::Abs(f) <= 0.00001f;
		}

		static inline bool IsNull(double d)
		{
			union U {
				double d;
				uint64 u;
			};
			U val;
			val.d = d;
			return (val.u & UINT64_C(0x7fffffffffffffff)) == 0;
		}

		/*
		This function tests a float for a null value. It doesn't
		check whether the actual value is 0 or close to 0, but whether
		it is binary 0, disregarding sign.
		*/
		static bool IsNull(float f)
		{
			union U {
				float f;
				uint32 u;
			};
			U val;
			val.f = f;
			return (val.u & 0x7fffffff) == 0;
		}

		/** whether value is not a number, T is usually float or double */
		template<typename T>
		static inline bool is_nan(T value)
		{
			return value != value;
		}

		/** whether value is infinity, T is usually float or double */
		template<typename T>
		static inline bool is_infinity(T value)
		{
			return std::numeric_limits<T>::has_infinity &&
				value == std::numeric_limits<T>::infinity();
		}

		/**
		* Get the scaling factor from globalMat.
		* since we need to create separate physics mesh with different scaling factors even for the same mesh model.
		* it is assumed that components of globalMat satisfies the following equation:
		* |(globalMat._11, globalMat._12, globalMat._13)| = 1;
		* |(globalMat._21, globalMat._22, globalMat._23)| = 1;
		* |(globalMat._31, globalMat._32, globalMat._33)| = 1;
		*
		* @param globalMat the input matrix
		* @param fScalingX [out] maybe NULL
		* @param fScalingY [out] maybe NULL
		* @param fScalingZ [out] maybe NULL
		*/
		static void GetMatrixScaling(const Matrix4& globalMat, float* fScalingX, float* fScalingY, float* fScalingZ);

		/** create a billboard matrix
		* @param pOut: [out], the output billboard matrix
		* @param matModelview: the current model view matrix. if NULL, the current model and view matrix on the global stack is used
		* @param vBillboardPos: the billboard position in model space. if NULL, (0,0,0) is used.
		* @param bAxisAligned: if true, it will create a billboard with Y (up) axis fixed. Default to false.
		* @return pOut is returned.
		*/
		static Matrix4* CreateBillboardMatrix(Matrix4* pOut, const Matrix4* matModelview, const Vector3* vBillboardPos, bool bAxisAligned = false);

		// Make a rotation matrix based on the camera's yaw & pitch
		static void CameraRotMatrixYawPitch(Matrix4& out, float fYaw, float fPitch);

		/** get nearest power of two of x*/
		static int NextPowerOf2(int x);
    };
}


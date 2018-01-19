#pragma once

typedef signed int			sdword;		//!< sizeof(sdword)	must be 4
typedef unsigned int		udword;		//!< sizeof(udword)	must be 4

#define IEEE_MAX_FLOAT			0x7f7fffff					//!< integer representation of MAX_FLOAT
#define IEEE_MIN_FLOAT			0xff7fffff					//!< integer representation of MIN_FLOAT
#define IEEE_1_0				0x3f800000					//!< integer representation of 1.0

#define	MAX_FLOAT				FLT_MAX						//!< max possible float value
#define	MIN_FLOAT				(-FLT_MAX)					//!< min possible loat value
#define FLT_MAX         3.402823466e+38F        /* max value */
#define	SQRT2				1.41421356237f											//!< sqrt(2)
#define	INVSQRT2			0.707106781188f											//!< 1 / sqrt(2)
#define	INVSQRT3			0.577350269189f											//!< 1 / sqrt(3)

/// Integer representation of a floating-point value.
#define IR(x)					((udword&)(x))

/// Signed integer representation of a floating-point value.
#define SIR(x)					((sdword&)(x))

/// Absolute integer representation of a floating-point value
#define AIR(x)					(IR(x)&0x7fffffff)

/// Floating-point representation of an integer value.
#define FR(x)					((float&)(x))

/// Integer-based comparison of a floating point value.
/// Don't use it blindly, it can be faster or slower than the FPU comparison, depends on the context.
#define IS_NEGATIVE_FLOAT(x)	(IR(x)&0x80000000)

#define GREATER(x, y)	AIR(x) > IR(y)

//! Fast fabs for floating-point values. It just clears the sign bit.
//! Don't use it blindly, it can be faster or slower than the FPU comparison, depends on the context.
inline float FastFabs(float x)
{
	udword FloatBits = IR(x)&0x7fffffff;
	return FR(FloatBits);
}

//! Fast square root for floating-point values.
inline float FastSqrt(float square)
{
#if defined (_MSC_VER) && ! defined(_WIN64)
	float retval;

	__asm {
		mov             eax, square
		sub             eax, 0x3F800000
		sar             eax, 1
		add             eax, 0x3F800000
		mov             [retval], eax
	}
	return retval;
#else
	return sqrt(square);
#endif
}

//! Saturates positive to zero.
inline float fsat(float f)
{
	udword y = (udword&)f & ~((sdword&)f >>31);
	return (float&)y;
}

//! Computes 1.0f / sqrtf(x).
inline float frsqrt(float f)
{
	float x = f * 0.5f;
	udword y = 0x5f3759df - ((udword&)f >> 1);
	// Iteration...
	(float&)y  = (float&)y * ( 1.5f - ( x * (float&)y * (float&)y ) );
	// Result
	return (float&)y;
}

//! Computes 1.0f / sqrtf(x). Comes from NVIDIA.
inline float InvSqrt(const float& x)
{
	udword tmp = (udword(IEEE_1_0 << 1) + IEEE_1_0 - *(udword*)&x) >> 1;   
	float y = *(float*)&tmp;                                             
	return y * (1.47f - 0.47f * x * y * y);
}

//! Computes 1.0f / sqrtf(x). Comes from Quake3. Looks like the first one I had above.
//! See http://www.magic-software.com/3DGEDInvSqrt.html
inline float RSqrt(float number)
{
	long i;
	float x2, y;
	const float threehalfs = 1.5f;

	x2 = number * 0.5f;
	y  = number;
	i  = * (long *) &y;
	i  = 0x5f3759df - (i >> 1);
	y  = * (float *) &i;
	y  = y * (threehalfs - (x2 * y * y));

	return y;
}

//! TO BE DOCUMENTED
inline float fsqrt(float f)
{
	udword y = ( ( (sdword&)f - 0x3f800000 ) >> 1 ) + 0x3f800000;
	// Iteration...?
	// (float&)y = (3.0f - ((float&)y * (float&)y) / f) * (float&)y * 0.5f;
	// Result
	return (float&)y;
}

//! Returns the float ranged epsilon value.
inline float fepsilon(float f)
{
	udword b = (udword&)f & 0xff800000;
	udword a = b | 0x00000001;
	(float&)a -= (float&)b;
	// Result
	return (float&)a;
}

//! Is the float valid ?
inline bool IsNAN(float value)				{ return (IR(value)&0x7f800000) == 0x7f800000;	}
inline bool IsIndeterminate(float value)	{ return IR(value) == 0xffc00000;				}
inline bool IsPlusInf(float value)			{ return IR(value) == 0x7f800000;				}
inline bool IsMinusInf(float value)		{ return IR(value) == 0xff800000;				}

inline	bool IsValidFloat(float value)
{
	if(IsNAN(value))			return false;
	if(IsIndeterminate(value))	return false;
	if(IsPlusInf(value))		return false;
	if(IsMinusInf(value))		return false;
	return true;
}
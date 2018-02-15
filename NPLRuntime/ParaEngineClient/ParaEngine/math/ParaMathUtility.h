#pragma once

/** The default tolerance for double-sized floating-point comparison functions. */
#define DBL_TOLERANCE 0.0001
/** The default tolerance for single-sized floating-point comparison functions. */
#define FLT_TOLERANCE 0.0001f

#ifndef MATH_PI
#define MATH_PI 3.141592654f
#define MATH_2PI 6.283185307f
#define MATH_1DIVPI 0.318309886f
#define MATH_1DIV2PI 0.159154943f
#define MATH_PIDIV2 1.570796327f
#define MATH_PIDIV4 0.785398163f
#endif
// Magic numbers for numerical precision.
#define DELTA			(0.00001f)
#define SMALL_NUMBER		(1.e-8f)
#define KINDA_SMALL_NUMBER	(1.e-4f)
#define BIG_NUMBER			(3.4e+38f)
#define EULERS_NUMBER       (2.71828182845904523536f)
// Copied from float.h
#define MAX_FLT 3.402823466e+38F

#ifndef FLOAT_POS_INFINITY
#define FLOAT_POS_INFINITY 999999999.0f
#endif

/** do not change the include order, there are dependencies*/
#include "ParaQuaternion.h"
#include "ParaVector2.h"
#include "ParaVector3.h"
#include "ParaVector4.h"
#include "ParaDVector3.h"
#include "ParaMathVector.h"

#include "ParaPlane.h"
#include "ParaColor.h"

#include "ParaMathMatrix.h"
#include "ParaMatrix3.h"
#include "ParaMatrix4.h"

// all *.inl(inline function body) goes in the end
#include "ParaAngle.inl"
#include "ParaVector3.inl"
#include "ParaDVector3.inl"
#include "ParaVector4.inl"


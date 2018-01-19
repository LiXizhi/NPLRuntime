#pragma once

namespace ParaEngine
{
#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

#ifndef MIN
#define MIN(a,b) ((a)<(b)?(a):(b))
#endif

#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif

#ifndef ABS
#define ABS(x) ((x)<0?-(x):(x))
#endif

#if !defined(SIGN)
#define SIGN(x) ((x) < 0 ? -1 : ((x) > 0 ? 1 : 0))
#endif

#ifndef PI
#define PI 3.1415926535897932385f
#endif
#ifndef HALF_PI
#define HALF_PI (PI / 2.0f)
#endif
#ifndef TWO_PI
#define TWO_PI (PI * 2.0f)
#endif

#undef SMALL_EPSILON
#define SMALL_EPSILON 0.000001f		/* Very small value */

#undef SMALL_EPSILON_D
#define SMALL_EPSILON_D 0.000000000001f	/* Very, very small value */

	// The smallest Z at which 3D clipping occurs
#define SMALL_Z 0.01f

}
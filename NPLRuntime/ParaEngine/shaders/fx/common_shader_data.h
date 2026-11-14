/*
#ifdef HALF_PRECISION
	#define SCALAR half
	#define SCALAR2 half2
	#define SCALAR3 half3
	#define SCALAR4 half4
	#define SCALAR2x2 half2x2
	#define SCALAR3x3 half3x3
	#define SCALAR4x4 half4x4
#else

	#define SCALAR float
	#define SCALAR2 float2
	#define SCALAR3 float3
	#define SCALAR4 float4
	#define SCALAR2x2 float2x2
	#define SCALAR3x3 float3x3
	#define SCALAR4x4 float4x4
#endif
*/

static const SCALAR4 ambient_light = {0.2f, 0.2f, 0.3f, 1.0f};
static const dword dw_ambient_light = 0x050507ff;

static const SCALAR4 xAxis = 
  {1.0f, 0.0f, 0.0f, 0.0f};
static const SCALAR4 zAxis = 
  {0.0f, 0.0f, -1.0f, 0.0f};
static const SCALAR4 vOne = 
  {1.0f, 1.0f, 1.0f, 0.0f};
static const SCALAR4 vHalf = 
  {0.5f, 0.5f, 0.5f, 0.0f};






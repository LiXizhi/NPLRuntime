// shared constant registers and functions
#include "common_shader_data.h"
#include "light_scattering_constants.h"
//
// There are three methods used to draw the ocean. Currently only the vertex color method is valid,
// which requires only vs1.0 and no pixel shader or textures. The other methods can be enabled by defining macros
// PSVS_NEW and PSVS_OLD. PSVS_NEW use in-scattering light; whereas PSVS_OLD uses bump mapping. 
// Both methods require at least PS 1.1. Please see gaia.1.6 for more information.
//
// #define PSVS_NEW
// #define PSVS_OLD

// transformations
const SCALAR4x4 mViewProj: VIEWPROJECTION: register(vs, c10);
const SCALAR4 vWorldPos: worldpos;
const SCALAR4 vCameraPos: worldcamerapos;
const SCALAR4 sun_vec: sunvector;
const SCALAR4 sun_color: suncolor;
const SCALAR4 posOffset : posScaleOffset;

static const SCALAR4 waterColor0 = 
  {0.15f, 0.4f, 0.5f, 0.7f}; // RGBA: Alpha used to be 1.0f

static const SCALAR4 waterColor1 = 
  {0.1f, 0.15f, 0.3f, 0.7f};// RGBA: Alpha used to be 1.0f

texture tex0 : TEXTURE; // ripple bump map
texture tex1 : TEXTURE; // water lookup table

struct VS_INPUT
{
	SCALAR2	Pos		: POSITION;
	SCALAR	ZPos0	: POSITION1;
	SCALAR2	Norm0	: NORMAL0;
	SCALAR	ZPos1	: POSITION2;
	SCALAR2	Norm1	: NORMAL1;
};

#ifdef PSVS_OLD
struct VS_OUTPUT
{
	SCALAR4 Pos		: POSITION;
	SCALAR4 Col		: COLOR0;
	SCALAR3 T0		: TEXCOORD0;
	SCALAR3 T1		: TEXCOORD1;
	SCALAR3 T2		: TEXCOORD2;
	SCALAR3 T3		: TEXCOORD3;
};
#endif

#ifdef PSVS_NEW
struct VS_OUTPUT_NEW
{
	SCALAR4 Pos		: POSITION;
	SCALAR4 vE		: COLOR0;
	SCALAR4 vI		: COLOR1;
	SCALAR2 T0		: TEXCOORD0;
};
#endif

struct VS_OUTPUT_P0
{
	SCALAR4 Pos		: POSITION;
	SCALAR4 Diffuse	: COLOR0;
	//SCALAR2 T0		: TEXCOORD0;
};

VS_OUTPUT_P0 VS_P0(VS_INPUT v)
{
	VS_OUTPUT_P0 Out;

	// offset xy and interpolate
	// z to get world position	
	SCALAR3 worldPos = 
	SCALAR3(
		v.Pos.x+posOffset.z,
		v.Pos.y+posOffset.w,
		lerp(v.ZPos0, v.ZPos1, posOffset.x)+vWorldPos.y).xzy;

	// transform and output
	Out.Pos = 
		mul(SCALAR4(worldPos, 1), mViewProj); 

	// interpolate normal
	SCALAR2 nXY = lerp(v.Norm0, v.Norm1, posOffset.x);
	// LXZ: why 8.0f, 24.0f are all OK values
	SCALAR3 normal = normalize(SCALAR3(nXY, 8.0f)).xzy;
	
	//compute the view vector
	SCALAR3 camera_vec = vCameraPos - worldPos;
	camera_vec = normalize(camera_vec);

	// color interpolator is the dot product 
	// of the view vector with the normal
	SCALAR cosTheta = saturate(dot(camera_vec, normal)); // original (-camera_vec) is not correct
	Out.Diffuse = lerp(waterColor0, waterColor1, cosTheta);
	return Out;
}

#ifdef PSVS_OLD
VS_OUTPUT VS(VS_INPUT v)
{
	VS_OUTPUT Out = (VS_OUTPUT)0;

	// offset xy and interpolate
	// z to get world position	
	SCALAR3 worldPos = 
	SCALAR3(
		v.Pos.x+posOffset.z,
		v.Pos.y+posOffset.w,
		lerp(v.ZPos0, v.ZPos1, posOffset.x)+vWorldPos.y).xzy;

	// transform and output
	Out.Pos = 
		mul(SCALAR4(worldPos, 1), mViewProj); 

	// interpolate normal
	SCALAR2 nXY = lerp(v.Norm0, v.Norm1, posOffset.x);
	SCALAR3 normal = normalize(SCALAR3(nXY, 24.0f)).xzy;

	// compute tex coords using world pos
	// and normal for animation
	SCALAR3 uvBase = 
		SCALAR3(
			worldPos.x*0.01f, 
			//worldPos.y*0.01f, 
			worldPos.z*0.01f, //  y is the up position.
			0.0f);

	SCALAR3 uvAnim = normal * 0.1f;
	Out.T0 = uvBase + uvAnim;
	Out.T1 = (uvBase * 0.5f) - uvAnim;

	// compute binormal and 
	// tangent using cross products
	SCALAR3 tangent = xAxis.yzx *  normal.zxy;
	tangent = (-normal.yzx * xAxis.zxy) + tangent;

	SCALAR3 binormal = normal.yzx *  tangent.zxy;
	binormal = (-tangent.yzx * normal.zxy) + binormal;

	// transform the sun vector to texture space
	SCALAR3 lightVec;
	lightVec.x = dot(sun_vec, binormal);
	lightVec.y = dot(sun_vec, tangent);
	lightVec.z = dot(sun_vec, normal);

	// normalize the light vector
	// and output
	Out.T2 = normalize(lightVec);

	//compute the view vector
	SCALAR3 camera_vec = vCameraPos - worldPos;
	SCALAR s = length(camera_vec);
	camera_vec = normalize(camera_vec);

	// transform the view vector to texture space
	SCALAR3 viewVec;
	viewVec.x = dot(camera_vec, binormal);
	viewVec.y = dot(camera_vec, tangent);
	viewVec.z = dot(camera_vec, normal);

	// normalize the view vector
	viewVec = normalize(viewVec);

	// compute the half-angle vector
	SCALAR3 half_angle_vec = (viewVec + lightVec)*0.5f;

	// normalize the half vector
	// and output
	Out.T3  = normalize(half_angle_vec);

	// color interpolator is the dot product 
	// of the view vector with the normal
	SCALAR cosTheta = saturate(dot(-camera_vec, normal));
	Out.Col = lerp(waterColor0, waterColor1, cosTheta);

	return Out;
}
#endif

#ifdef PSVS_NEW
VS_OUTPUT_NEW VS_new(VS_INPUT v)
{
	VS_OUTPUT_NEW Out;// = (VS_OUTPUT_NEW)0;

	// offset xy and interpolate
	// z to get world position	
	SCALAR3 worldPos = 
	SCALAR3(
		v.Pos.x+posOffset.z,
		v.Pos.y+posOffset.w,
		lerp(v.ZPos0, v.ZPos1, posOffset.x)+vWorldPos.y).xzy;

	// transform and output
	Out.Pos = 
		mul(SCALAR4(worldPos, 1), mViewProj); 

	// interpolate normal
	SCALAR2 nXY = lerp(v.Norm0, v.Norm1, posOffset.x);
	// LXZ: why 8.0f
	SCALAR3 normal = normalize(SCALAR3(nXY, 8.0f)).xzy;
	
	//compute the view vector
	SCALAR3 camera_vec = vCameraPos - worldPos;
	SCALAR s = length(camera_vec);
	camera_vec = normalize(camera_vec);

	// compute the half-angle vector
	SCALAR3 half_angle_vec = (camera_vec + sun_vec)*0.5f;
	half_angle_vec  = normalize(half_angle_vec);

	SCALAR NdH = (dot(normal, half_angle_vec)-0.5f)*2.0f;

	// color interpolator is the dot product 
	// of the view vector with the normal
	SCALAR cosTheta = saturate(dot(-camera_vec, normal));
		
	// output a texcoord which is really a 2D index into our 
	// lookup table texture. x= water color, y= specular scale
	Out.T0 = SCALAR2(cosTheta, -NdH);

	// compute the atmospheric lighting data
	atmosphericLighting(
		camera_vec,
		sun_vec,
		normal,
		sun_color,
		s,
		Out.vE,
		Out.vI);

	return Out;
}
#endif

#ifdef PSVS_OLD
sampler LinearSamp0 = sampler_state 
{
    texture = <tex0>;
    AddressU  = wrap;        
    AddressV  = wrap;
    AddressW  = wrap;
    MIPFILTER = LINEAR;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
};
#endif

#ifdef PSVS_NEW
sampler LinearSamp1 = sampler_state 
{
    texture = <tex1>;
    AddressU  = clamp;        
    AddressV  = clamp;
    AddressW  = clamp;
    MIPFILTER = POINT;
    MINFILTER = POINT;
    MAGFILTER = POINT;
};
#endif

#ifdef PSVS_OLD
SCALAR4 PS(VS_OUTPUT In) : COLOR
{
	// composite bump maps
	SCALAR3 bump0 = (tex2D(LinearSamp0, In.T0 )-0.5f);//*2.0f; 
	SCALAR3 bump1 = (tex2D(LinearSamp0, In.T1 )-0.5f);//*2.0f; 
	SCALAR3 bump = (bump0+bump1);//*0.5f;

	// compute base diffuse color
	SCALAR baseColor = dot(bump, In.T2);

	// compute specular component
	SCALAR specFactor = dot(bump, In.T3);
	SCALAR4 specColor = 
		specFactor * specFactor * 
		specFactor * specFactor * sun_color;

	// combine and output
	return (baseColor*In.Col)+specColor;
}
#endif

#ifdef PSVS_NEW
SCALAR4 PS_new(VS_OUTPUT_NEW In) : COLOR
{
	SCALAR4 color = tex2D(LinearSamp1, In.T0 );

	// compute specular component
	SCALAR specFactor = color.a;
	SCALAR3 specColor = specFactor * specFactor* sun_color * color;

	// apply the atmospheric lighting data to the base color
	color = ((color*In.vE) + (In.vI));

	// combine and output
	return SCALAR4(color+specColor, 1.0f);
	//return SCALAR4(0,1.f,0,1.f);
}
#endif

#ifdef PSVS_NEW
technique OceanWater_vs20_ps20
{
	pass P0
	{
		//	FILLMODE = WIREFRAME;
		// CULLMODE = CCW;
		ZENABLE = TRUE;
		ZWRITEENABLE = TRUE;
		ZFUNC = LESSEQUAL;

		AlphaBlendEnable = false;
		AlphaTestEnable = false;

		// shaders
		#define BUMP_MAPPING
		#ifdef BUMP_MAPPING
		VertexShader = compile vs_2_0 VS();
		PixelShader  = compile ps_2_0 PS();
		#else
		VertexShader = compile vs_2_0 VS_new();
		PixelShader  = compile ps_2_0 PS_new();
		#endif
	}  
}
#endif

#ifdef PSVS_OLD
technique OceanWater_vs11_ps11
{
	pass P0
	{
		//	FILLMODE = WIREFRAME;
		// CULLMODE = CCW;
		ZENABLE = TRUE;
		ZWRITEENABLE = TRUE;
		ZFUNC = LESSEQUAL;

		AlphaBlendEnable = false;
		AlphaTestEnable = false;

		// shaders
		VertexShader = compile vs_1_1 VS_new();
		PixelShader  = compile ps_1_1 PS_new();
	}  
}
#endif

technique OceanWater_vs11_ps0
{
	pass P0
	{
		//	FILLMODE = WIREFRAME;
		// CULLMODE = CCW;
		// disable fog
		FOGENABLE = false;
		
		AlphaBlendEnable = true;
		AlphaTestEnable = false;

		// shaders
		VertexShader = compile vs_1_1 VS_P0();
		PixelShader  = NULL;
		
#ifdef PSVS_NEW
		// Set up texture stage 0
		Texture[0] = <tex1>;
		AddressU[0]  = clamp;        
		AddressV[0]  = clamp;
		MinFilter[0] = Point;
		MagFilter[0] = Point;
		MipFilter[0] = Point;

		ColorArg1[0] = Texture;
		ColorArg2[0] = Diffuse;
		ColorOp[0] = Modulate;
		AlphaOp[0] = Disable;

		ColorArg1[1] = Current;
		ColorArg2[1] = Specular;
		ColorOp[1] = Modulate;
		AlphaOp[1] = Disable;

		ColorOp[2] = Disable;
		AlphaOp[2] = Disable;
#else
		Texture[0] = NULL;
		Texture[1] = NULL;
#endif
	}  
}

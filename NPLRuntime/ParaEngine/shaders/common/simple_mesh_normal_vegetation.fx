// Author: LiXizhi
// Desc: 2007/3
// we simulate vegatation animation, as follows
// Distort each mesh vertex according to wave function whose amplitude is a linear function of vertex height in model space.
// the wave function in X direction: WaveHeight = Amplitude*sin((2Pi*frequency)*time+2Pi/WaveLength*X), where Amplitude = MAGNITUDE*V(height).V(height) is vertex up(Y) component in model space.
// The new x component of a vertex is: Xnew = X+A*sin(2Pi*f*T + 2Pi/L*X)--> d(Xnew) = [1+A*2Pi/L*cos(2Pi*f*T + 2Pi/L*X)]*d(X)
// Hence we must ensure that |A*2Pi/L|<1 in order for d(X) and d(Xnew) having the same sign at any position along the X. In other words, |L|>|2Pi*A|
// Note: (1) X should be in world space whose origin is integer times of wave length. The actual world space origin may change in large game world, hence we need to use a world origin adjustment value to compensate for the change.
//       (2) View clipping on CPU must be slightly bigger.
// TODO: use instancing if we assume that A, L, f are constant for each model. For working parameters I tested are A=0.1meter, L=6 meters, anglevelocity=2Pi*f=1
// Generally wave length L should be three times larger than tree trunck. 
#include "simple_mesh_normal_vegetation.h"
/** whether to animate shadows in shadow map. This is usually not needed.*/
#define ANIMATE_SHADOW

#define ALPHA_TESTING_REF  0.5
#define MAX_LIGHTS_NUM	4
////////////////////////////////////////////////////////////////////////////////
//  Per frame parameters
float4x4 mWorldViewProj: worldviewprojection;
//float4x4 mViewProj: viewprojection;
float4x4 mWorldView: worldview;
float4x4 mWorld: world;

float3 sun_vec: sunvector;

float3	colorDiffuse:materialdiffuse;
float3	colorAmbient:ambientlight;

int		g_locallightnum:locallightnum;
float3	g_lightcolor[MAX_LIGHTS_NUM]	:	LightColors;
float3	g_lightparam[MAX_LIGHTS_NUM]	:	LightParams;
float4	g_lightpos[MAX_LIGHTS_NUM]		:	LightPositions;

////////////////////////////////////////////////////////////////////////////////
// per technique parameters
float4   g_fogParam : fogparameters; // (fogstart, fogrange, fogDensity, reserved)
float4    g_fogColor : fogColor;

// static branch boolean constants
bool g_bEnableSunLight	:sunlightenable;
bool g_bAlphaTesting	:alphatesting;
bool g_bEnableFog		:fogenable;

float4 g_parameter0		:ConstVector0; // x for time, y for max_height; z,w are for world origin adjustment offset.

// texture 0
texture tex0 : TEXTURE; 
sampler tex0Sampler : register(s0) = sampler_state 
{
    texture = <tex0>;
};

// texture 1 for reflection or normal map 
texture tex1 : TEXTURE; 
sampler tex1Sampler : register(s1) = sampler_state 
{
    texture = <tex1>;
};

// cubic environment map.
texture tex2 : TEXTURE; 
samplerCUBE texCubeSampler : register(s2) = sampler_state 
{
    texture = <tex2>;
};


struct Interpolants
{
  float4 positionSS			: POSITION;         // Screen space position
  float3 tex				: TEXCOORD0;        // texture coordinates
  float3 tex1				: TEXCOORD1;        // texture coordinates
  half3	 colorDiffuse		: COLOR0;			// diffuse color
};

////////////////////////////////////////////////////////////////////////////////
//
//                              Vertex Shader
//
////////////////////////////////////////////////////////////////////////////////
// Calculates fog factor based upon distance
half CalcFogFactor( half d )
{
    half fogCoeff = 0;
	fogCoeff = (d - g_fogParam.x)/g_fogParam.y;
    return saturate( fogCoeff);
}

Interpolants vertexShader(	float4	Pos			: POSITION,
							float3	Norm		: NORMAL,
							float2	Tex			: TEXCOORD0)
{
	Interpolants o = (Interpolants)0;
	// Vegetation distortation of Pos
	float3 worldPos = mul( Pos, mWorld ).xyz;

	float amplitude = VEGETATION_WAVE_MAGNITUDE*saturate(Pos.y/g_parameter0.y);
	worldPos.xz += g_parameter0.zw; // compensate for world origin, the new origin will always be at integer times of wave length. 
	float2 Wave = amplitude*float2(sin(g_parameter0.x+(6.283/VEGETATION_WAVE_LENGTH)*worldPos.x), cos(g_parameter0.x+(6.283/VEGETATION_WAVE_LENGTH)*worldPos.z));
	Pos.xz += Wave;
	
	// recalculate world space position with sway
	worldPos = mul( Pos, mWorld ).xyz;
	
	// screen space position
	o.positionSS = 	mul(Pos, mWorldViewProj);
	// camera space position
	float4 cameraPos = mul( Pos, mWorldView ); 
	// world space normal
	float3 worldNormal = normalize( mul( Norm, (float3x3)mWorld ) ); 
	
	// calculate light of the sun
	if(g_bEnableSunLight)
	{
		o.colorDiffuse = max(0,dot( sun_vec, worldNormal ))*colorDiffuse+colorAmbient;
	}
	else
	{
		half3 difCol = colorAmbient+colorDiffuse;
		o.colorDiffuse = difCol;
	}

	// compute local lights
	for( int LightIndex = 0; LightIndex < g_locallightnum; ++LightIndex )
	{
		float3 toLight = g_lightpos[LightIndex].xyz - worldPos;
		float lightDist = length( toLight );
		// this dynamic braching helps for bigge br models
		if(g_lightpos[LightIndex].w > lightDist)
		{
			float fAtten = 1 / dot(g_lightparam[LightIndex], float3(1,lightDist,lightDist*lightDist));
			float3 lightDir = normalize( toLight );
			o.colorDiffuse += max(0,dot( lightDir, worldNormal ) * g_lightcolor[LightIndex].xyz * fAtten);
		}
	}
	
	o.tex.xy = Tex;
	//save the fog distance
    o.tex.z = CalcFogFactor(cameraPos.z);
   
	return o;
}

////////////////////////////////////////////////////////////////////////////////
//
//                              Pixel Shader
//
////////////////////////////////////////////////////////////////////////////////

half4 pixelShader(Interpolants i) : COLOR
{
	half4 o;
	half4 normalColor = tex2D(tex0Sampler, i.tex.xy);
	normalColor.xyz = normalColor.xyz*i.colorDiffuse;
	
	if(g_bAlphaTesting)
	{
		// alpha testing and blending
		clip(normalColor.w-ALPHA_TESTING_REF);
	}
	
	if(g_bEnableFog)
	{
		//calculate the fog factor
		half fog = i.tex.z;
		o.xyz = lerp(normalColor.xyz, g_fogColor.xyz, fog);
		fog = saturate( (fog-0.8)*16 );
		o.w = lerp(normalColor.w, 0, fog);
	}
	else
	{
		o = normalColor;
	}
	
	return o;
}

////////////////////////////////////////////////////////////////////////////////
//
//                              shadow map : VS and PS
//
////////////////////////////////////////////////////////////////////////////////

void VertShadow( float4	Pos			: POSITION,
				 float3	Norm		: NORMAL,
				 float2	Tex			: TEXCOORD0,
                 out float4 oPos	: POSITION,
                 out float2	outTex	: TEXCOORD0,
                 out float2 Depth	: TEXCOORD1 )
{
#ifdef ANIMATE_SHADOW
	// Vegetation distortation of Pos
	float3 worldPos = mul( Pos, mWorld ).xyz;

	float amplitude = VEGETATION_WAVE_MAGNITUDE*saturate(Pos.y/g_parameter0.y);
	worldPos.xz += g_parameter0.zw; // compensate for world origin, the new origin will always be at integer times of wave length. 
	float2 Wave = amplitude*float2(sin(g_parameter0.x+(6.283/VEGETATION_WAVE_LENGTH)*worldPos.x), cos(g_parameter0.x+(6.283/VEGETATION_WAVE_LENGTH)*worldPos.z));
	Pos.xz += Wave;
#endif
	oPos = mul( Pos, mWorldViewProj );
    outTex = Tex;
    Depth.xy = oPos.zw;
}

float4 PixShadow( float2	inTex		: TEXCOORD0,
				 float2 Depth		: TEXCOORD1) : COLOR
{
	half alpha = tex2D(tex0Sampler, inTex.xy).w;
	
	if(g_bAlphaTesting)
	{
		// alpha testing
		alpha = lerp(1,0, alpha < ALPHA_TESTING_REF);
		clip(alpha-0.5);
	}
    float d = Depth.x / Depth.y;
    return float4(0, d.xx,alpha);
}

////////////////////////////////////////////////////////////////////////////////
//
//                              Technique
//
////////////////////////////////////////////////////////////////////////////////

technique SimpleMesh_vs20_ps20
{
	pass P0
	{
		// shaders
		VertexShader = compile vs_2_a vertexShader();
		PixelShader  = compile ps_2_a pixelShader();
		
		FogEnable = false;
	}
}

technique GenShadowMap
{
    pass p0
    {
        VertexShader = compile vs_2_a VertShadow();
        PixelShader = compile ps_2_a PixShadow();
        FogEnable = false;
    }
}
// Author: LiXizhi
// Desc: 2006/4
/** define this for the sky fog to be accurate*/
#define NODISTORTION_FOG
#define ALPHA_TESTING_REF  0.5
////////////////////////////////////////////////////////////////////////////////
//  Per frame parameters
float4x4 mWorldViewProj: worldviewprojection;
//float4x4 mViewProj: viewprojection;
float4x4 mWorldView: worldview;
float4x4 mWorld: world;

float3 sun_vec: sunvector;

float3	colorDiffuse:materialdiffuse;
float3	colorAmbient:ambientlight;

////////////////////////////////////////////////////////////////////////////////
// per technique parameters
float4  g_fogParam : fogparameters; // (fogstart, fogrange, fogDensity, reserved)
float4  g_fogColor : fogColor;
float4 g_skycolorfactor: ConstVector0;
// static branch boolean constants
bool g_bEnableFog		:fogenable;
bool g_bEnableSunLight : sunlightenable;
bool g_bAlphaTesting : alphatesting;

// texture 0
texture tex0 : TEXTURE; 
sampler tex0Sampler : register(s0) = sampler_state 
{
    texture = <tex0>;
	MinFilter = Linear;
	MipFilter = Linear;
	MagFilter = Linear;
	AddressU = clamp;
	AddressV = clamp;
};

struct Interpolants
{
  float4 positionSS			: POSITION;         // Screen space position
  float3 tex				: TEXCOORD0;        // texture coordinates
  float3 pos				: TEXCOORD1;        // model space coordinates
  float3 colorDiffuse		: TEXCOORD2;		// diffuse color
};

////////////////////////////////////////////////////////////////////////////////
//
//                              Vertex Shader
//
////////////////////////////////////////////////////////////////////////////////
// Calculates fog factor based upon distance
Interpolants vertexShader(	float4	Pos			: POSITION,
							float3	Norm		: NORMAL,
							float2	Tex			: TEXCOORD0)
{
	Interpolants o = (Interpolants)0;
	// during perspective divide, the z will be 1 (xyww/w = x/w, y/w, 1) in NDC space. 
	o.positionSS = mul(Pos, mWorldViewProj).xyww;
	// o.positionSS = mul(Pos, mWorldViewProj);
	// when transform z from [-1,1] to [0,1], some precision is lost, hence making z slightly smaller
	//o.positionSS.z = o.positionSS.w * 0.9999;
	//o.positionSS.z = o.positionSS.w - 0.00001; -- special fix for far-clipping plane to make final z bigger than 1. not tested.

	if (g_bEnableSunLight)
	{
		o.colorDiffuse = colorDiffuse*dot(sun_vec, half3(0, 1, 0)) + colorAmbient;
	}
	else
	{
		o.colorDiffuse = half3(1,1,1);
	}

	o.colorDiffuse *= g_skycolorfactor;
	o.colorDiffuse = min(1, o.colorDiffuse);
	
	o.tex.xy = Tex;
	o.tex.z = Pos.y;
#ifdef NODISTORTION_FOG
	o.pos = Pos.xyz/Pos.w;
#endif	
	return o;
}

////////////////////////////////////////////////////////////////////////////////
//
//                              Pixel Shader
//
////////////////////////////////////////////////////////////////////////////////
half CalcFogFactor( half d )
{
    half fogCoeff = 0;
	fogCoeff = (d - g_fogParam.x)/g_fogParam.y;
    return saturate( fogCoeff);
}

half4 pixelShader(Interpolants i) : COLOR
{
	half4 o;
	half4 normalColor = tex2D(tex0Sampler, i.tex.xy);
	normalColor.xyz = normalColor.xyz*i.colorDiffuse;
	
	if (g_bAlphaTesting)
	{
		// alpha testing and blending
		clip(normalColor.w - ALPHA_TESTING_REF);
	}

	if(g_bEnableFog)
	{
		//calculate the fog factor
		#ifdef NODISTORTION_FOG
			half fog = CalcFogFactor(i.pos.y/length(i.pos.xz));
		#else	
			half fog = CalcFogFactor(i.tex.z);
		#endif
		
		o.xyz = lerp(g_fogColor.xyz, normalColor.xyz, fog);
		o.w = normalColor.w;
	}
	else
	{
		o = normalColor;
	}
	return o;
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
		VertexShader = compile vs_2_0 vertexShader();
		PixelShader  = compile ps_2_0 pixelShader();
		
		FogEnable = false;
	}
}
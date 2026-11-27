// Author: LiXizhi
// Desc: 2006/4

#define ALPHA_TESTING_REF  0.5
////////////////////////////////////////////////////////////////////////////////
//  Per frame parameters
float4x4 mWorldViewProj: worldviewprojection;
//float4x4 mViewProj: viewprojection;
float4x4 mWorldView: worldview;
float4x4 mWorld: world;

float4 sun_vec: sunvector;

float3	colorDiffuse:materialdiffuse;
float3	colorAmbient:ambientlight;
float3	colorEmissive:materialemissive = float3(0,0,0);

#include "mesh_local_lights.h"

////////////////////////////////////////////////////////////////////////////////
// per technique parameters
float4   g_fogParam : fogparameters; // (fogstart, fogrange, fogDensity, reserved)
float4    g_fogColor : fogColor;

// static branch boolean constants
bool g_bEnableSunLight	:sunlightenable;
bool g_bAlphaTesting	:alphatesting;
bool g_bEnableFog		:fogenable;
bool g_bReflectionMap	:boolean5;
bool g_bEnvironmentMap	:boolean6;
float g_bReflectFactor	:reflectfactor;
float3 g_EyePositionW	:worldcamerapos;
float2 g_TexOffset		:ConstVector0; // TODO: for testing texture animation: x,y for translation
float3 g_TexRot			:ConstVector1;
float2 g_TexScale		:ConstVector2 = float2(1.0, 1.0);
//bool g_bNormalMap		:boolean6;
float g_opacity			:opacity = 1.0; 

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
  float3 colorDiffuse		: TEXCOORD2;		// diffuse color
};

////////////////////////////////////////////////////////////////////////////////
//
//                              Vertex Shader
//
////////////////////////////////////////////////////////////////////////////////
// Calculates fog factor based upon distance
float CalcFogFactor( float d )
{
    float fogCoeff = 0;
	fogCoeff = (d - g_fogParam.x)/g_fogParam.y;
    return saturate( fogCoeff);
}
Interpolants vertexShader(	float4	Pos			: POSITION,
							float3	Norm		: NORMAL,
							float2	Tex			: TEXCOORD0)
{
	Interpolants o = (Interpolants)0;
	// screen space position
	o.positionSS = 	mul(Pos, mWorldViewProj);
	// world space position: 
	// TODO: this can be removed if we can do N.8L in camera space. 
	float3 worldPos = mul( Pos, mWorld ).xyz;
	// camera space position
	float4 cameraPos = mul( Pos, mWorldView ); 
	// world space normal
	float3 worldNormal = normalize( mul( Norm, (float3x3)mWorld ) ); 
	
	// calculate light of the sun
	if(g_bEnableSunLight)
	{
		o.colorDiffuse = max(0,dot( sun_vec.xyz, worldNormal ))*colorDiffuse;
		o.colorDiffuse += colorAmbient;
	}
	else
	{
		o.colorDiffuse = min(1, colorDiffuse+colorAmbient);
	}
	

	o.colorDiffuse = ApplyLocalLights(o.colorDiffuse, worldPos, worldNormal);

	// uv offset
	float2 uv = Tex + g_TexOffset.xy;

	// uv scale
	uv.x *= g_TexScale.x;
	uv.y *= g_TexScale.y;

	// uv rotate
	float sinNum = sin(g_TexRot.x);
	float cosNum = cos(g_TexRot.x);
	uv -= float2(g_TexRot.y, g_TexRot.z);
	uv = mul(uv, float2x2(cosNum, -sinNum, sinNum, cosNum));
	uv += float2(g_TexRot.y, g_TexRot.z);

	o.tex.xy = uv;
	//save the fog distance
    o.tex.z = CalcFogFactor(cameraPos.z);
    if(g_bReflectionMap)
    {
		/* here is the prospective texture transform: 
		float4x4 mTex = {
			{0.5,0,  0,  0},
			{0,  0.5,0,  0},
			{0,  0,  0.5,0},
			{0.5,0.5,0.5,1},
			};
		Convert screen space position from [-1,-1]->[1,1] to [0,0]->[1,1]
		This is done to match the coordinate space of the reflection map*/
		o.tex1.xy = (o.positionSS.xy + o.positionSS.w)*0.5;
	  	o.tex1.z = o.positionSS.w;
    }
    else if(g_bEnvironmentMap)
    {
		// Obtain the eye vector in world (cube) space
		float3 eyeVector = normalize( worldPos-g_EyePositionW );

		// Compute the reflection vector and save to tex1
		o.tex1 = normalize(reflect(eyeVector, worldNormal));
    }
    
	return o;
}

////////////////////////////////////////////////////////////////////////////////
//
//                              Pixel Shader
//
////////////////////////////////////////////////////////////////////////////////


float4 pixelShader(Interpolants i) : COLOR0
{
	float4 o;
	float4 normalColor = tex2D(tex0Sampler, i.tex.xy);
	normalColor.xyz = normalColor.xyz*i.colorDiffuse;
	
	if(g_bAlphaTesting)
	{
		// alpha testing and blending
		clip(normalColor.w-ALPHA_TESTING_REF);
	}
	
	if(g_bReflectionMap)
	{
		float4 reflection = tex2D(tex1Sampler, float2(1,1)-i.tex1.xy/i.tex1.z);
		normalColor.xyz = lerp(normalColor.rgb, reflection.rgb, g_bReflectFactor);
	}
	else if(g_bEnvironmentMap)
    {
		float4 reflection = texCUBE( texCubeSampler, i.tex1 );
		normalColor.xyz = lerp(normalColor.rgb, reflection.rgb, g_bReflectFactor);
    }
    
	if(g_bEnableFog)
	{
		//calculate the fog factor
		float fog = i.tex.z;
		o.xyz = lerp(normalColor.xyz, g_fogColor.xyz, fog);
		fog = saturate( (fog-0.8)*16.0 );
		o.w = lerp(normalColor.w, 0.0, fog);
	}
	else
	{
		o = normalColor;
	}
	o.rgb += colorEmissive;
	o.w *= g_opacity;
	return o;
}

////////////////////////////////////////////////////////////////////////////////
//
//                              shadow map : VS and PS
//
////////////////////////////////////////////////////////////////////////////////

struct ShadowVSOut
{
	float4 oPos : POSITION;
	float2 outTex : TEXCOORD0;
	float2 Depth : TEXCOORD1;
};

ShadowVSOut VertShadow( float4	Pos			: POSITION,
				 float3	Norm		: NORMAL,
				 float2	Tex			: TEXCOORD0 )
{
    ShadowVSOut o;
    o.oPos = mul( Pos, mWorldViewProj );
    o.outTex = Tex;
    o.Depth.xy = o.oPos.zw;
    return o;
}

float4 PixShadow( ShadowVSOut s ) : COLOR0
{
	float alpha = tex2D(tex0Sampler, s.outTex.xy).w;
	
	if(g_bAlphaTesting)
	{
		if (alpha < ALPHA_TESTING_REF)
			clip(-1.0);
	}
    float d = s.Depth.x / s.Depth.y;
    return float4(0.0, d.xx, alpha);
}

////////////////////////////////////////////////////////////////////////////////
//
//                              Technique
//
////////////////////////////////////////////////////////////////////////////////
technique SimpleMesh_vs30_ps30
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
// Author: LiXizhi
// Desc: 2006/4

#define ALPHA_TESTING_REF  0.5
////////////////////////////////////////////////////////////////////////////////
//  Per frame parameters
float4x4 mWorldViewProj: worldviewprojection;

////////////////////////////////////////////////////////////////////////////////
// per technique parameters
// static branch boolean constants
bool g_bAlphaTesting	:alphatesting;

// texture 0
texture tex0 : TEXTURE; 
sampler tex0Sampler : register(s0) = sampler_state 
{
    texture = <tex0>;
};

struct Interpolants
{
  float4 positionSS			: POSITION;         // Screen space position
  float2 tex				: TEXCOORD0;        // texture coordinates
  half4	 colorDiffuse		: COLOR0;			// diffuse color
};

////////////////////////////////////////////////////////////////////////////////
//
//                              Vertex Shader
//
////////////////////////////////////////////////////////////////////////////////

Interpolants vertexShader(	float4	Pos			: POSITION,
							half4	Color		: COLOR0,
							float2	Tex			: TEXCOORD0)
{
	Interpolants o = (Interpolants)0;
	// screen space position
	o.positionSS = 	mul(Pos, mWorldViewProj);
	o.colorDiffuse = Color;
	o.tex.xy = Tex;
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
	normalColor *= i.colorDiffuse;
	
	if(g_bAlphaTesting)
	{
		// alpha testing and blending
		clip(normalColor.w-ALPHA_TESTING_REF);
	}

	o = normalColor;
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
		ZWriteEnable = false;
	}
}
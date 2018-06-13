// Author: LiXizhi
// Desc: 2006/9

////////////////////////////////////////////////////////////////////////////////
//  Per frame parameters
float4x4 mWorldViewProj: worldviewprojection;
//float4x4 mViewProj: viewprojection;
float4x4 mWorldView: worldview;
float4x4 mWorld: world;

float3	colorAmbient:ambientlight;

////////////////////////////////////////////////////////////////////////////////
// per technique parameters
float4  g_fogParam : fogparameters; // (fogstart, fogrange, fogDensity, reserved)
float4  g_fogColor : fogColor;
// static branch boolean constants
bool g_bEnableFog		:fogenable;

// texture 0
texture tex0 : TEXTURE; 
sampler tex0Sampler : register(s0) = sampler_state 
{
    texture = <tex0>;
};

struct Interpolants
{
  float4 positionSS			: POSITION;         // Screen space position
  float3 tex				: TEXCOORD0;        // texture coordinates
  half4	 colorDiffuse		: COLOR0;			// diffuse color
};

////////////////////////////////////////////////////////////////////////////////
//
//                              Vertex Shader
//
////////////////////////////////////////////////////////////////////////////////

Interpolants vertexShader(	float4	Pos			: POSITION,
							float2	Tex			: TEXCOORD0,
							half4	Color		: COLOR0)
{
	Interpolants o = (Interpolants)0;
	// screen space position
	o.positionSS = 	mul(Pos, mWorldViewProj);
	float4 cameraPos = mul( Pos, mWorldView ); //Save cameraPos for fog calculations
	o.colorDiffuse = Color;
	o.tex.xy = Tex;
	//o.tex.z = Color.z;
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
	normalColor.rgb *=colorAmbient.rgb;
	normalColor.a *= i.colorDiffuse.a;
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
		//ZEnable = false;
		ZWriteEnable = false;
	}
}
// Author: LiXizhi
// Data: 2006/7
// Desc: for displaying GUI object. 

////////////////////////////////////////////////////////////////////////////////
//  Per frame parameters
float4x4 mWorldViewProj: worldviewprojection;

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
};

////////////////////////////////////////////////////////////////////////////////
//
//                              Vertex Shader
//
////////////////////////////////////////////////////////////////////////////////
Interpolants vertexShader(	float4 Pos : POSITION,
							float2 Tex : TEXCOORD0 )
{
	Interpolants o = (Interpolants)0;
	// screen space position
	o.positionSS = 	mul(Pos, mWorldViewProj);
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
	half4 normalColor = tex2D(tex0Sampler, i.tex.xy);
	clip(normalColor.w-0.5);
	return normalColor;
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
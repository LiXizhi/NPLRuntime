// Author: LiXizhi
// Desc: 2006/4

// Uniforms

float4x4 worldviewprojection:worldviewprojection;

// texture 0
texture tex0 : TEXTURE; 
sampler tex0Sampler : register(s0) = sampler_state 
{
    texture = <tex0>;
};

struct Interpolants
{
  float4 pos			: POSITION;         // Screen space position
  float4 color			: COLOR;        // texture coordinates
};

////////////////////////////////////////////////////////////////////////////////
//
//                              Vertex Shader
//
////////////////////////////////////////////////////////////////////////////////

Interpolants vertexShader(	float4	Pos			: POSITION,
							float4  Color 		: COLOR)
{
	Interpolants o = (Interpolants)0;
	// screen space position
	o.pos = mul(Pos, worldviewprojection);
	o.color = Color;
	return o;
}

////////////////////////////////////////////////////////////////////////////////
//
//                              Pixel Shader
//
////////////////////////////////////////////////////////////////////////////////


half4 pixelShader(Interpolants i) : COLOR
{
	return i.color;
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
	}
}
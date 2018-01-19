// Author: LiXizhi
// Desc: 2006/4
// this file is not used: show map are always in technique of the coorresponding fx file.
////////////////////////////////////////////////////////////////////////////////
//  Per frame parameters
float4x4 mWorldViewProj: worldviewprojection;
//float4x4 mViewProj: viewprojection;
float4x4 mWorldView: worldview;
float4x4 mWorld: world;

struct Interpolants
{
  float4 positionSS			: POSITION;         // Screen space position
  float4 tex0				: TEXCOORD0;        // texture coordinates
};

////////////////////////////////////////////////////////////////////////////////
//
//                              Vertex Shader
//
////////////////////////////////////////////////////////////////////////////////

Interpolants vertexShader(	float4	Pos			: POSITION	)
{
	Interpolants o = (Interpolants)0;
	// screen space position
	o.positionSS = 	mul(Pos, mWorldViewProj);
	o.tex0 = float4(OUT.Position.zzz, OUT.Position.w);
	return o;
}

////////////////////////////////////////////////////////////////////////////////
//
//                              Pixel Shader
//
////////////////////////////////////////////////////////////////////////////////

float4 pixelShader(Interpolants i) : COLOR
{
	float depth = IN.TexCoord0.b / IN.TexCoord0.a;
    return float4(depth.xxx, 1.0);
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
		VertexShader = compile vs_1_1 vertexShader();
		PixelShader  = compile ps_2_0 pixelShader();
		
		FogEnable = false;
	}
}

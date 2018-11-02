// Author: LiXizhi
// Date: 2006/4
// Desc: just for hardware occlusion testing

// parameters
const float4x4 mViewProj: viewprojection;

struct Interpolants
{
  float4 positionSS                                 : POSITION;         // Screen space position
};

////////////////////////////////////////////////////////////////////////////////
//
//                              Vertex Shader
//
////////////////////////////////////////////////////////////////////////////////
Interpolants vertexShader(	float4	Pos			: POSITION,
							float2	Tex			: TEXCOORD0)
{
	Interpolants o = (Interpolants)0;
	// transform and output
	o.positionSS = 	mul(Pos, mViewProj);
	return o;
}

////////////////////////////////////////////////////////////////////////////////
//
//                              Pixel Shader
//
////////////////////////////////////////////////////////////////////////////////
half4 pixelShader(Interpolants i) : COLOR
{
	return half4(1,1,1,1);
}

////////////////////////////////////////////////////////////////////////////////
//
//                              Technique
//
////////////////////////////////////////////////////////////////////////////////
technique OcclusionTest_vs10_ps10
{
	pass P0
	{
		// shaders
		VertexShader = compile vs_2_0 vertexShader();
		PixelShader  = compile ps_2_0 pixelShader();
		
		// turn off color masks and zwrite
        ZWriteEnable = 0;
        // comment this line to debug. 
        ColorWriteEnable = 0; 
        FogEnable = False;
        cullmode = none;

        // other settings (play it safe):
        AlphaBlendEnable = False;
        Lighting = False;
        
		ColorOp[0] = SelectArg1;
		ColorArg1[0] = Diffuse;
		ColorOp[1] = Disable;

		AlphaOp[0] = SelectArg1;
		AlphaArg1[0] = Diffuse;
		AlphaOp[1] = Disable;
	}
}

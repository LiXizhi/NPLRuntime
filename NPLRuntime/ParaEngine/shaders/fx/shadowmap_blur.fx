// Author: LiXizhi
// date: 2007/2
// desc: Generic Guassian blur over shadow map color texture surface. two techniques are implemented. I use a 5*5 filter; 9*9 filter may be better.

// TexelIncrement should be set to 1/RRT_width and 1/RTT_height respectively for the two seperate convolution blur passes.
const float2 TexelIncrements : ConstVector0;

////////////////////////////////////////////////////////////////////////////////
//  Per frame parameters

////////////////////////////////////////////////////////////////////////////////
/// per technique parameters


// texture 0
texture GlowMap0 : TEXTURE; 
sampler GlowSamp1 : register(s0) = sampler_state 
{
    texture = <GlowMap0>;
    AddressU  = CLAMP;        
    AddressV  = CLAMP;
    AddressW  = CLAMP;
    MIPFILTER = NONE;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
};

////////////////////////////////////////////////////////////////////////////////
//
//                              data structures
//
////////////////////////////////////////////////////////////////////////////////

struct VS_OUTPUT_BLUR
{
    float4 Position   : POSITION;
    float2 TexCoord0   : TEXCOORD0;
    float2 TexCoord1   : TEXCOORD1;
    float2 TexCoord2   : TEXCOORD2;
    float2 TexCoord3   : TEXCOORD3;
    float2 TexCoord4   : TEXCOORD4;
    float4 Diffuse    : COLOR0;
};

////////////////////////////////////////////////////////////////////////////////
//
//                              Vertex Shader
//
////////////////////////////////////////////////////////////////////////////////

VS_OUTPUT_BLUR VS_Quad_Vertical_5tap(float3 Position : POSITION, 
			float2 TexCoord : TEXCOORD0)
{
    VS_OUTPUT_BLUR OUT = (VS_OUTPUT_BLUR)0;
    OUT.Position = float4(Position, 1);
    
    float3 Coord = float3(TexCoord.x , TexCoord.y , 1);
    float TexelIncrement = TexelIncrements.y;
    OUT.TexCoord0 = float2(Coord.x, Coord.y + TexelIncrement);
    OUT.TexCoord1 = float2(Coord.x, Coord.y + TexelIncrement * 2);
    OUT.TexCoord2 = float2(Coord.x, Coord.y);
    OUT.TexCoord3 = float2(Coord.x, Coord.y - TexelIncrement);
    OUT.TexCoord4 = float2(Coord.x, Coord.y - TexelIncrement * 2);
    return OUT;
}

VS_OUTPUT_BLUR VS_Quad_Horizontal_5tap(float3 Position : POSITION, 
			float2 TexCoord : TEXCOORD0)
{
    VS_OUTPUT_BLUR OUT = (VS_OUTPUT_BLUR)0;
    OUT.Position = float4(Position, 1);
    
    float3 Coord = float3(TexCoord.x, TexCoord.y, 1);
    float TexelIncrement = TexelIncrements.x;
    OUT.TexCoord0 = float2(Coord.x + TexelIncrement, Coord.y);
    OUT.TexCoord1 = float2(Coord.x + TexelIncrement * 2, Coord.y);
    OUT.TexCoord2 = float2(Coord.x, Coord.y);
    OUT.TexCoord3 = float2(Coord.x - TexelIncrement, Coord.y);
    OUT.TexCoord4 = float2(Coord.x - TexelIncrement * 2, Coord.y);
    return OUT;
}

////////////////////////////////////////////////////////////////////////////////
//
//                              Pixel Shader
//
////////////////////////////////////////////////////////////////////////////////

// Relative filter weights indexed by distance from "home" texel
//    This set for 5-texel sampling
#define WT5_0 1.0
#define WT5_1 0.8
#define WT5_2 0.4

#define WT5_NORMALIZE (WT5_0+2.0*(WT5_1+WT5_2))

float4 PS_Blur_Horizontal_5tap(VS_OUTPUT_BLUR IN) : COLOR
{   
    float4 OutCol = tex2D(GlowSamp1, IN.TexCoord0) * (WT5_1/WT5_NORMALIZE);
    OutCol += tex2D(GlowSamp1, IN.TexCoord1) * (WT5_2/WT5_NORMALIZE);
    OutCol += tex2D(GlowSamp1, IN.TexCoord2) * (WT5_0/WT5_NORMALIZE);
    OutCol += tex2D(GlowSamp1, IN.TexCoord3) * (WT5_1/WT5_NORMALIZE);
    OutCol += tex2D(GlowSamp1, IN.TexCoord4) * (WT5_2/WT5_NORMALIZE);
    return OutCol;
} 

float4 PS_Blur_Vertical_5tap(VS_OUTPUT_BLUR IN) : COLOR
{   
    float4 OutCol = tex2D(GlowSamp1, IN.TexCoord0) * (WT5_1/WT5_NORMALIZE);
    OutCol += tex2D(GlowSamp1, IN.TexCoord1) * (WT5_2/WT5_NORMALIZE);
    OutCol += tex2D(GlowSamp1, IN.TexCoord2) * (WT5_0/WT5_NORMALIZE);
    OutCol += tex2D(GlowSamp1, IN.TexCoord3) * (WT5_1/WT5_NORMALIZE);
    OutCol += tex2D(GlowSamp1, IN.TexCoord4) * (WT5_2/WT5_NORMALIZE);
    return OutCol;
} 


////////////////////////////////////////////////////////////////////////////////
//
//                              Technique
//
////////////////////////////////////////////////////////////////////////////////
technique Glow_5Tap 
{
	pass BlurGlowBuffer_Horz
    {
		cullmode = none;
		ZEnable = false;
		ZWriteEnable = false;
		AlphaBlendEnable = false;
		AlphaTestEnable = false;
		FogEnable = False;
		VertexShader = compile vs_2_0 VS_Quad_Horizontal_5tap();
		PixelShader  = compile ps_2_0 PS_Blur_Horizontal_5tap();
    }
    pass BlurGlowBuffer_Vert
    {
		cullmode = none;
		ZEnable = false;
		ZWriteEnable = false;
		AlphaBlendEnable = true;
		SrcBlend = one;
		DestBlend = zero;
		AlphaTestEnable = false;
		FogEnable = False;
		VertexShader = compile vs_2_0 VS_Quad_Vertical_5tap();
		PixelShader  = compile ps_2_0 PS_Blur_Vertical_5tap();
    }
}




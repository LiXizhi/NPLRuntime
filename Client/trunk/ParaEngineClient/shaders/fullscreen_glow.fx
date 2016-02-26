// Author: LiXizhi
// date: 2006/5
// desc: two techniques are implemented. One uses 5*5 filter, another uses 9*9 filter.
// TexelIncrement should be set to 1/RRT_width and 1/RTT_height respectively for the two seperate convolution blur passes.
const float2 TexelIncrements : ConstVector0;
// Glowness controls the intensity of the glow. 0 means no glow, 1 is normal glow, 3 is three times glow.
const float4 Glowness : ConstVector1;

////////////////////////////////////////////////////////////////////////////////
//  Per frame parameters

////////////////////////////////////////////////////////////////////////////////
/// per technique parameters
#define SampleCount 9

float2 sampleOffset[SampleCount];
float sampleWeight[SampleCount];
float glowThreshold = 0.8;
float4 glowParams = float4(1,1,1,1);


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

// texture 1
texture GlowMap1 : TEXTURE; 
sampler GlowSamp2 : register(s1) = sampler_state 
{
    texture = <GlowMap1>;
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
    float4 Diffuse    : COLOR0;
    float4 TexCoord0   : TEXCOORD0;
    float4 TexCoord1   : TEXCOORD1;
    float4 TexCoord2   : TEXCOORD2;
    float4 TexCoord3   : TEXCOORD3;
    float4 TexCoord4   : TEXCOORD4;
    float4 TexCoord5   : TEXCOORD5;
    float4 TexCoord6   : TEXCOORD6;
    float4 TexCoord7   : TEXCOORD7;
    float4 TexCoord8   : COLOR1;   
};

struct VS_OUTPUT
{
   	float4 Position   : POSITION;    
    float2 TexCoord   : TEXCOORD0;
};

////////////////////////////////////////////////////////////////////////////////
//
//                              Vertex Shader
//
////////////////////////////////////////////////////////////////////////////////

VS_OUTPUT VS_GlowSource(float3 Position : POSITION, 
			float2 TexCoord : TEXCOORD0)
{
    VS_OUTPUT OUT = (VS_OUTPUT)0;
    OUT.Position = float4(Position, 1);
    OUT.TexCoord = TexCoord;
    return OUT;
}

VS_OUTPUT VS_Quad(float3 Position : POSITION, 
			float3 TexCoord : TEXCOORD0)
{
    VS_OUTPUT OUT;
    OUT.Position = float4(Position, 1);
    OUT.TexCoord = TexCoord;
    return OUT;
}

VS_OUTPUT_BLUR VS_Quad_Vertical_9tap(float3 Position : POSITION, 
			float3 TexCoord : TEXCOORD0)
{
    VS_OUTPUT_BLUR OUT = (VS_OUTPUT_BLUR)0;
    OUT.Position = float4(Position, 1);
    
    float3 Coord = float3(TexCoord.x, TexCoord.y, 1);
    float TexelIncrement = TexelIncrements.y;
    OUT.TexCoord0 = float4(Coord.x, Coord.y + TexelIncrement, TexCoord.z, 1);
    OUT.TexCoord1 = float4(Coord.x, Coord.y + TexelIncrement * 2, TexCoord.z, 1);
    OUT.TexCoord2 = float4(Coord.x, Coord.y + TexelIncrement * 3, TexCoord.z, 1);
    OUT.TexCoord3 = float4(Coord.x, Coord.y + TexelIncrement * 4, TexCoord.z, 1);
    OUT.TexCoord4 = float4(Coord.x, Coord.y, TexCoord.z, 1);
    OUT.TexCoord5 = float4(Coord.x, Coord.y - TexelIncrement, TexCoord.z, 1);
    OUT.TexCoord6 = float4(Coord.x, Coord.y - TexelIncrement * 2, TexCoord.z, 1);
    OUT.TexCoord7 = float4(Coord.x, Coord.y - TexelIncrement * 3, TexCoord.z, 1);
    OUT.TexCoord8 = float4(Coord.x, Coord.y - TexelIncrement * 4, TexCoord.z, 1);
    return OUT;
}

VS_OUTPUT_BLUR VS_Quad_Horizontal_9tap(float3 Position : POSITION, 
			float3 TexCoord : TEXCOORD0)
{
    VS_OUTPUT_BLUR OUT = (VS_OUTPUT_BLUR)0;
    OUT.Position = float4(Position, 1);
    
    float3 Coord = float3(TexCoord.x, TexCoord.y, 1);
    float TexelIncrement = TexelIncrements.x;
    OUT.TexCoord0 = float4(Coord.x + TexelIncrement, Coord.y, TexCoord.z, 1);
    OUT.TexCoord1 = float4(Coord.x + TexelIncrement * 2, Coord.y, TexCoord.z, 1);
    OUT.TexCoord2 = float4(Coord.x + TexelIncrement * 3, Coord.y, TexCoord.z, 1);
    OUT.TexCoord3 = float4(Coord.x + TexelIncrement * 4, Coord.y, TexCoord.z, 1);
    OUT.TexCoord4 = float4(Coord.x, Coord.y, TexCoord.z, 1);
    OUT.TexCoord5 = float4(Coord.x - TexelIncrement, Coord.y, TexCoord.z, 1);
    OUT.TexCoord6 = float4(Coord.x - TexelIncrement * 2, Coord.y, TexCoord.z, 1);
    OUT.TexCoord7 = float4(Coord.x - TexelIncrement * 3, Coord.y, TexCoord.z, 1);
    OUT.TexCoord8 = float4(Coord.x - TexelIncrement * 4, Coord.y, TexCoord.z, 1);
    return OUT;
}

VS_OUTPUT_BLUR VS_Quad_Vertical_5tap(float3 Position : POSITION, 
			float3 TexCoord : TEXCOORD0)
{
    VS_OUTPUT_BLUR OUT = (VS_OUTPUT_BLUR)0;
    OUT.Position = float4(Position, 1);
    
    float3 Coord = float3(TexCoord.x , TexCoord.y , 1);
    float TexelIncrement = TexelIncrements.y;
    OUT.TexCoord0 = float4(Coord.x, Coord.y + TexelIncrement, TexCoord.z, 1);
    OUT.TexCoord1 = float4(Coord.x, Coord.y + TexelIncrement * 2, TexCoord.z, 1);
    OUT.TexCoord2 = float4(Coord.x, Coord.y, TexCoord.z, 1);
    OUT.TexCoord3 = float4(Coord.x, Coord.y - TexelIncrement, TexCoord.z, 1);
    OUT.TexCoord4 = float4(Coord.x, Coord.y - TexelIncrement * 2, TexCoord.z, 1);
    return OUT;
}

VS_OUTPUT_BLUR VS_Quad_Horizontal_5tap(float3 Position : POSITION, 
			float3 TexCoord : TEXCOORD0)
{
    VS_OUTPUT_BLUR OUT = (VS_OUTPUT_BLUR)0;
    OUT.Position = float4(Position, 1);
    
    float3 Coord = float3(TexCoord.x, TexCoord.y, 1);
    float TexelIncrement = TexelIncrements.x;
    OUT.TexCoord0 = float4(Coord.x + TexelIncrement, Coord.y, TexCoord.z, 1);
    OUT.TexCoord1 = float4(Coord.x + TexelIncrement * 2, Coord.y, TexCoord.z, 1);
    OUT.TexCoord2 = float4(Coord.x, Coord.y, TexCoord.z, 1);
    OUT.TexCoord3 = float4(Coord.x - TexelIncrement, Coord.y, TexCoord.z, 1);
    OUT.TexCoord4 = float4(Coord.x - TexelIncrement * 2, Coord.y, TexCoord.z, 1);
    return OUT;
}


VS_OUTPUT vs_main(float3 position :POSITION,
	float2 texcoord	:TEXCOORD0)
{
	VS_OUTPUT o;
	o.Position = float4(position,1);
	o.TexCoord = texcoord;
	return o;
}
////////////////////////////////////////////////////////////////////////////////
//
//                              Pixel Shader
//
////////////////////////////////////////////////////////////////////////////////

float4 PS_GlowSource(VS_OUTPUT IN) : COLOR
{   
	float4 tex = tex2D(GlowSamp1, IN.TexCoord);
	tex.xyz = tex.xyz * tex.w;
	return tex;
}  

float4 BloomExtract(VS_OUTPUT IN) : COLOR0
{
    // Look up the original image color.
    float4 tex = tex2D(GlowSamp1, IN.TexCoord);

    // Adjust it to keep only values brighter than the specified threshold.
    return saturate((tex - glowThreshold) / (1 - glowThreshold));
}


float4 ps_main(VS_OUTPUT IN):COLOR
{
	float4 finalColor = 0;
	for(int i=0;i<SampleCount;i++)
	{
		finalColor += tex2D(GlowSamp1,IN.TexCoord + sampleOffset[i]) * sampleWeight[i];
	}
	finalColor.w = 1;
	return finalColor;
}

// For two-pass blur, we have chosen to do  the horizontal blur FIRST. The
//	vertical pass includes a post-blur scale factor.

// Relative filter weights indexed by distance from "home" texel
//    This set for 9-texel sampling
#define WT9_0 1.0
#define WT9_1 0.8
#define WT9_2 0.6
#define WT9_3 0.4
#define WT9_4 0.2

// Alt pattern -- try your own!
// #define WT9_0 0.1
// #define WT9_1 0.2
// #define WT9_2 3.0
// #define WT9_3 1.0
// #define WT9_4 0.4

#define WT9_NORMALIZE (WT9_0+2.0*(WT9_1+WT9_2+WT9_3+WT9_4))

float4 PS_Blur_Horizontal_9tap(VS_OUTPUT_BLUR IN) : COLOR
{   
    float4 OutCol = tex2D(GlowSamp1, IN.TexCoord0) * (WT9_1/WT9_NORMALIZE);
    OutCol += tex2D(GlowSamp1, IN.TexCoord1) * (WT9_2/WT9_NORMALIZE);
    OutCol += tex2D(GlowSamp1, IN.TexCoord2) * (WT9_3/WT9_NORMALIZE);
    OutCol += tex2D(GlowSamp1, IN.TexCoord3) * (WT9_4/WT9_NORMALIZE);
    OutCol += tex2D(GlowSamp1, IN.TexCoord4) * (WT9_0/WT9_NORMALIZE);
    OutCol += tex2D(GlowSamp1, IN.TexCoord5) * (WT9_1/WT9_NORMALIZE);
    OutCol += tex2D(GlowSamp1, IN.TexCoord6) * (WT9_2/WT9_NORMALIZE);
    OutCol += tex2D(GlowSamp1, IN.TexCoord7) * (WT9_3/WT9_NORMALIZE);
    OutCol += tex2D(GlowSamp1, IN.TexCoord8) * (WT9_3/WT9_NORMALIZE);
    return OutCol;
} 

float4 PS_Blur_Vertical_9tap(VS_OUTPUT_BLUR IN) : COLOR
{   
    float4 OutCol = tex2D(GlowSamp2, IN.TexCoord0) * (WT9_1/WT9_NORMALIZE);
    OutCol += tex2D(GlowSamp2, IN.TexCoord1) * (WT9_2/WT9_NORMALIZE);
    OutCol += tex2D(GlowSamp2, IN.TexCoord2) * (WT9_3/WT9_NORMALIZE);
    OutCol += tex2D(GlowSamp2, IN.TexCoord3) * (WT9_4/WT9_NORMALIZE);
    OutCol += tex2D(GlowSamp2, IN.TexCoord4) * (WT9_0/WT9_NORMALIZE);
    OutCol += tex2D(GlowSamp2, IN.TexCoord5) * (WT9_1/WT9_NORMALIZE);
    OutCol += tex2D(GlowSamp2, IN.TexCoord6) * (WT9_2/WT9_NORMALIZE);
    OutCol += tex2D(GlowSamp2, IN.TexCoord7) * (WT9_3/WT9_NORMALIZE);
    OutCol += tex2D(GlowSamp2, IN.TexCoord8) * (WT9_3/WT9_NORMALIZE) * sampleWeight[0];
    //return float4(Glowness.xyz*OutCol.xyz, Glowness.w);
	return float4(Glowness.xyz,1);
} 

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
    OutCol += tex2D(GlowSamp1, IN.TexCoord4) * (WT5_2/WT5_NORMALIZE)* sampleWeight[0];
    return float4(OutCol.xyz,1);
} 

float4 PS_Blur_Vertical_5tap(VS_OUTPUT_BLUR IN) : COLOR
{   
    float4 OutCol = tex2D(GlowSamp2, IN.TexCoord0) * (WT5_1/WT5_NORMALIZE);
    OutCol += tex2D(GlowSamp2, IN.TexCoord1) * (WT5_2/WT5_NORMALIZE);
    OutCol += tex2D(GlowSamp2, IN.TexCoord2) * (WT5_0/WT5_NORMALIZE);
    OutCol += tex2D(GlowSamp2, IN.TexCoord3) * (WT5_1/WT5_NORMALIZE);
    OutCol += tex2D(GlowSamp2, IN.TexCoord4) * (WT5_2/WT5_NORMALIZE);
    //return float4(Glowness.xyz*OutCol.xyz, Glowness.w);
	return float4(OutCol.xyz,1);
} 

////////


// add glow on top of model

float4 PS_GlowPass(VS_OUTPUT IN) : COLOR
{   
	float3 baseColor = tex2D(GlowSamp1,IN.TexCoord.xy);
	float3 glow = tex2D(GlowSamp2,IN.TexCoord.xy);
	
	float baseGray = dot(baseColor,float3(0.3,0.59,0.11));
	float glowGray = dot(glow,float3(0.3,0.59,0.11));
	
	glow = lerp(glowGray, glow, glowParams.x) * glowParams.y;
	baseColor = lerp(baseGray,baseColor,glowParams.z) * glowParams.w;

	baseColor *= (1-saturate(glow));
	return float4(baseColor + glow,1);
}  


////////////////////////////////////////////////////////////////////////////////
//
//                              Technique
//
////////////////////////////////////////////////////////////////////////////////
technique Glow_5Tap 
{
	pass GlowSourcePass
    {
		cullmode = none;
		ZEnable = false;
		ZWriteEnable = false;
		AlphaBlendEnable = false;
		AlphaTestEnable = false;
		FogEnable = False;
		VertexShader = compile vs_2_0 VS_GlowSource();
		//PixelShader  = compile ps_2_0 PS_GlowSource();
		PixelShader  = compile ps_2_0 BloomExtract();
    }
    pass BlurGlowBuffer_Horz
    {
		cullmode = none;
		ZEnable = false;
		ZWriteEnable = false;
		AlphaBlendEnable = false;
		AlphaTestEnable = false;
		FogEnable = False;
		//VertexShader = compile vs_2_0 VS_Quad_Horizontal_5tap();
		//PixelShader  = compile ps_2_0 PS_Blur_Horizontal_5tap();
		VertexShader = compile vs_2_0 vs_main();
		PixelShader  = compile ps_2_0 ps_main();
    }
    pass BlurGlowBuffer_Vert
    {
		cullmode = none;
		ZEnable = false;
		ZWriteEnable = false;
		//AlphaBlendEnable = true;
		//SrcBlend = one;
		//DestBlend = zero;
		AlphaTestEnable = false;
		FogEnable = False;
		//VertexShader = compile vs_2_0 VS_Quad_Vertical_5tap();
		//PixelShader  = compile ps_2_0 PS_Blur_Vertical_5tap();
    }
    pass GlowPass
    {
		cullmode = none;
		ZEnable = false;
		ZWriteEnable = false;
		//AlphaBlendEnable = true;
		//SrcBlend = one;
		//DestBlend = SRCALPHA;
		AlphaTestEnable = false;
		FogEnable = False;
		VertexShader = compile vs_1_1 VS_Quad();
		PixelShader = compile ps_2_0 PS_GlowPass();	
    }
}

technique Glow_9Tap
{
	pass GlowSourcePass
    {
		cullmode = none;
		ZEnable = false;
		ZWriteEnable = false;
		AlphaBlendEnable = false;
		AlphaTestEnable = false;
		FogEnable = False;
		VertexShader = compile vs_2_0 VS_GlowSource();
		PixelShader  = compile ps_2_0 PS_GlowSource();
    }
	pass BlurGlowBuffer_Horz
    {
		cullmode = none;
		ZEnable = false;
		ZWriteEnable = false;
		AlphaBlendEnable = false;
		AlphaTestEnable = false;
		FogEnable = False;
		VertexShader = compile vs_2_0 VS_Quad_Horizontal_9tap();
		PixelShader  = compile ps_2_0 PS_Blur_Horizontal_9tap();
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
		VertexShader = compile vs_2_0 VS_Quad_Vertical_9tap();
		PixelShader  = compile ps_2_0 PS_Blur_Vertical_9tap();
    }
    pass GlowPass
   	{
		cullmode = none;
		ZEnable = false;
		ZWriteEnable = false;
		//AlphaBlendEnable = true;
		//SrcBlend = one;
		//DestBlend = SRCALPHA;
		AlphaTestEnable = false;
		FogEnable = False;
		VertexShader = compile vs_1_1 VS_Quad();
		PixelShader = compile ps_2_0 PS_GlowPass();	
    }
}


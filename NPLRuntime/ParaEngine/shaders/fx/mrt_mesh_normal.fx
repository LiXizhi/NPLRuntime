// Author: LiXizhi
// Desc: 2013/10

#define ALPHA_TESTING_REF  0.5
#define MAX_LIGHTS_NUM	4
////////////////////////////////////////////////////////////////////////////////
//  Per frame parameters
float4x4 mWorldViewProj: worldviewprojection;
//float4x4 mViewProj: viewprojection;
float4x4 mWorldView: worldview;
float4x4 mWorld: world;

float3 sun_vec: sunvector;

float3	colorDiffuse:materialdiffuse;
float3	colorAmbient:ambientlight;
float3	colorEmissive:materialemissive = float3(0,0,0);

////////////////////////////////////////////////////////////////////////////////
// per technique parameters
/** x is sun light strength, y is block light strength. */
float3	BlockLightStrength: LightStrength;

// static branch boolean constants
bool g_bEnableSunLight	:sunlightenable;
bool g_bAlphaTesting	:alphatesting;
bool g_bRGBOnlyTexturAnim  :boolean7;
float g_bReflectFactor	:reflectfactor;
float3 g_EyePositionW	:worldcamerapos;
float2 g_TexAnim		:ConstVector0; // TODO: for testing texture animation: x,y for translation
float2 g_CategoryID		:ConstVector1; 
//bool g_bNormalMap		:boolean6;
float g_opacity			:opacity = 1.0; 

// texture 0
texture tex0 : TEXTURE; 
sampler tex0Sampler : register(s0) = sampler_state 
{
    texture = <tex0>;
};


struct Interpolants
{
  float4 positionSS			: POSITION;         // Screen space position
  float4 tex				: TEXCOORD0;        // texture coordinates
  float4 normal				: TEXCOORD3;		// diffuse color
};

struct BlockPSOut
{
	// standard color
	float4 Color	: COLOR0;
	// r is 32bits depth
	float4 Depth	: COLOR1; 
	// r is category id, g is sun light, b is block light
	float4 BlockInfo : COLOR2; 
	float4 Normal : COLOR3; 
};

////////////////////////////////////////////////////////////////////////////////
//
//                              Vertex Shader
//
////////////////////////////////////////////////////////////////////////////////

Interpolants vertexShader(	float4	Pos			: POSITION,
							float3	Norm		: NORMAL,
							float2	Tex			: TEXCOORD0)
{
	Interpolants o = (Interpolants)0;
	// screen space position
	o.positionSS = 	mul(Pos, mWorldViewProj);
	// camera space position
	float4 cameraPos = mul( Pos, mWorldView ); 
	// world space normal
	float3 worldNormal = normalize( mul( Norm, (float3x3)mWorld ) ); 
	o.normal.xyz = worldNormal*0.5+0.5;
	
	// depth value
	o.normal.a = cameraPos.z;

	o.tex.xy = Tex+g_TexAnim.xy;
	o.tex.zw = Tex;
	return o;
}

////////////////////////////////////////////////////////////////////////////////
//
//                              Pixel Shader
//
////////////////////////////////////////////////////////////////////////////////


BlockPSOut pixelShader(Interpolants i)
{
	BlockPSOut output;
	float4 o;
	float4 normalColor = tex2D(tex0Sampler, i.tex.xy);

	if(g_bRGBOnlyTexturAnim)
		normalColor.w = tex2D(tex0Sampler, i.tex.zw).w;

	if(g_bAlphaTesting)
	{
		// alpha testing and blending
		clip(normalColor.w-ALPHA_TESTING_REF);
	}

	o = normalColor;
	o.rgb = o.rgb * colorDiffuse;

	o.rgb += colorEmissive;
	o.w *= g_opacity;
	
	output.Color = o;
	float category = 1.0;
	if (g_CategoryID.x > 0)
		category = g_CategoryID.x / 256.0;
	
	// 2020.7.18. Xizhi, fixed deferred rendering alpha blended effects
	float alpha = (normalColor.w < ALPHA_TESTING_REF) ? 0 : 1; 
	
	output.BlockInfo = float4(category, BlockLightStrength.x, BlockLightStrength.y, alpha);
	output.Depth = float4(i.normal.a, 0, 0, alpha);
	output.Normal = float4(i.normal.xyz, alpha);

	return output;
}

////////////////////////////////////////////////////////////////////////////////
//
//                              shadow map : VS and PS
//
////////////////////////////////////////////////////////////////////////////////

void VertShadow( float4	Pos			: POSITION,
				 float3	Norm		: NORMAL,
				 float2	Tex			: TEXCOORD0,
                 out float4 oPos	: POSITION,
                 out float4	outTex	: TEXCOORD0)
{
    oPos = mul( Pos, mWorldViewProj );
    outTex.xy = Tex;
	outTex.zw = oPos.zw;
}

float4 PixShadow( float4	inTex		: TEXCOORD0) : COLOR
{
	float alpha = tex2D(tex0Sampler, inTex.xy).w;
	
	if(g_bAlphaTesting)
	{
		// alpha testing
		clip(alpha - ALPHA_TESTING_REF);
	}
	// return float4(inTex.z/inTex.w, 0, 0, 1);
	return float4(inTex.z, 0, 0, 1); // inTex.w is 1 anyway
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
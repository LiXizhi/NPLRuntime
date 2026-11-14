// Author: LiXizhi
// Date: 2006/4
// Desc: this is not used, because the effect is really not good for global outdoor scene. However, the code can be enabled by defining SHADOW_ON_MESH macro

#define ALPHA_TESTING_REF  0.5
#define MAX_LIGHTS_NUM	4
////////////////////////////////////////////////////////////////////////////////
//  Per frame parameters
float4x4 mWorldViewProj: worldviewprojection;
//float4x4 mViewProj: viewprojection;
float4x4 mLightWorldViewProj: texworldviewproj;
float4x4 mWorldView: worldview;
float4x4 mWorld: world;

float3 sun_vec: sunvector;

float3	colorDiffuse:materialdiffuse;
float3	colorAmbient:ambientlight;

////////////////////////////////////////////////////////////////////////////////
// per technique parameters
float4   g_fogParam : fogparameters; // (fogstart, fogrange, fogDensity, reserved)
float4    g_fogColor : fogColor;

// static branch boolean constants
bool g_bEnableSunLight	:sunlightenable;
bool g_bAlphaTesting	:alphatesting;

bool g_bEnableFog		:fogenable;
bool	g_Useshadowmap	:	boolean8;
bool	g_UseShadowmapHW:	boolean9;
float	g_fShadowRadius :	shadowradius = 40;
float g_bReflectFactor	:reflectfactor;
float2	g_shadwoFactor :shadowfactor = float2(0.35,0.65);

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

// texture 2
texture ShadowMap2 : TEXTURE; 
sampler ShadowMapSampler: register(s2) = sampler_state 
{
    texture = <ShadowMap2>;
    MinFilter = Linear;  
    MagFilter = Linear;
    MipFilter = None;
    AddressU  = BORDER;
    AddressV  = BORDER;
    BorderColor = 0xffffffff;
};

struct Interpolants
{
  float4 positionSS			: POSITION;         // Screen space position
  float4 tex				: TEXCOORD0;        // texture coordinates
  float3 tex1				: TEXCOORD1;        // texture coordinates
  float4 tex2				: TEXCOORD2;        // texture coordinates
  float3	 colorDiffuse	: COLOR0;		// diffuse color
};

////////////////////////////////////////////////////////////////////////////////
//
//                              Vertex Shader
//
////////////////////////////////////////////////////////////////////////////////
// Calculates fog factor based upon distance
half CalcFogFactor( half d )
{
    half fogCoeff = 0;
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
	// TODO: this can be removed if we can do N.L in camera space. 
	float4 worldPos = mul( Pos, mWorld );
	// camera space position
	float4 cameraPos = mul( Pos, mWorldView ); 
	// world space normal
	float3 worldNormal = normalize( mul( Norm, (float3x3)mWorld ) ); 
	
	// calculate light of the sun
	if(g_bEnableSunLight)
	{
		//o.colorDiffuse = max(0,dot( sun_vec, worldNormal ))*colorDiffuse+colorAmbient;
		float lightFactor = dot(sun_vec,worldNormal);
		o.colorDiffuse.xyz = colorDiffuse * saturate(lightFactor);

		float3 groundColor = max( (colorAmbient - float3(0.15,0.10,0.05)),float3(0.05,0.05,0.05));
		float3 amb = lerp(groundColor,colorAmbient,lightFactor);
		o.colorDiffuse.xyz += amb;
	}
	else
	{
		o.colorDiffuse = min(1, colorDiffuse+colorAmbient);
	}
	
	o.tex.xy = Tex;
	//save the fog distance
    o.tex.z = CalcFogFactor(cameraPos.z);

    if(g_Useshadowmap)
    {
		// shadow blending factor, so that the near camera shadow is darker
		// (z-shadow_fade_start)/shadow_fade_range, so that the shadow end is shadow_fade_start+shadow_fade_range, this should match DEFAULT_SHADOW_RADIUS defined in sceneobject.cpp.
		// o.tex.w = saturate((cameraPos.z - g_fShadowRadius*0.6)/(g_fShadowRadius*0.4));  // TODO: per vertex distance is inaccurate when shadow range is comparable with triangle
		o.tex.w = saturate((cameraPos.z - 25)/15); 
		o.tex2 = mul(Pos, mLightWorldViewProj);
    }
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

	//gamma
	//normalColor.xyz =pow(normalColor.xyz,2.2);

	normalColor.xyz = normalColor.xyz*i.colorDiffuse;
	
	if(g_bAlphaTesting)
	{
		// alpha testing and blending
		clip(normalColor.w-ALPHA_TESTING_REF);
	}
	
	if(g_bEnableFog)
	{
		//calculate the fog factor
		half fog = i.tex.z;
		normalColor.xyz = lerp(normalColor.xyz, g_fogColor.xyz, fog);
		fog = saturate( (fog-0.8)*16 );
		normalColor.w = lerp(normalColor.w, 0, fog);
	}
	

	// diffuse color
	if(g_Useshadowmap && g_UseShadowmapHW)
    {
		half shadow = tex2Dproj(ShadowMapSampler, i.tex2).r;
		shadow = lerp(shadow, 1, i.tex.w);
		normalColor.xyz *= shadow * g_shadwoFactor.x + g_shadwoFactor.y;
	}
	return normalColor;
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
                 out float2	outTex	: TEXCOORD0,
                 out float2 Depth	: TEXCOORD1 )
{
    oPos = mul( Pos, mWorldViewProj );
    outTex = Tex;
    Depth.xy = oPos.zw;
}

float4 PixShadow( float2	inTex		: TEXCOORD0,
				 float2 Depth		: TEXCOORD1) : COLOR
{
	half alpha = tex2D(tex0Sampler, inTex.xy).w;
	
	if(g_bAlphaTesting)
	{
		// alpha testing
		alpha = lerp(1,0, alpha < ALPHA_TESTING_REF);
		clip(alpha-0.5);
	}
    float d = Depth.x / Depth.y;
    return float4(0, d.xx,alpha);
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
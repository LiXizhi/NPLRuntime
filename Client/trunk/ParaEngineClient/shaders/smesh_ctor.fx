// Author: LiXizhi
// Desc: 2006/4
// Desc: Per-pixel animation of fog effect. This is used for object construction animation in ParaEngine.
// Use: fogparameters and fogColor to control the animation. Other shader settings are basically the same as simple_mesh_normal.fx
// fogparameters.w is used as the minimum alpha of the object

#define ALPHA_TESTING_REF  0.5
////////////////////////////////////////////////////////////////////////////////
//  Per frame parameters
float4x4 mWorldViewProj: worldviewprojection;
//float4x4 mViewProj: viewprojection;
float4x4 mWorldView: worldview;
float4x4 mWorld: world;

float3 sun_vec: sunvector;

float3	colorDiffuse:materialdiffuse;
float3	colorAmbient:ambientlight;

////////////////////////////////////////////////////////////////////////////////
/// per technique parameters
float4   g_fogParam : fogparameters; // (fogstart, fogrange, fogDensity, reserved)
float4    g_fogColor : fogColor;

// static branch boolean constants
bool g_bAlphaTesting	:alphatesting;
bool g_bEnableSunLight: sunlightenable;

// texture 0
texture tex0 : TEXTURE; 
sampler tex0Sampler: register(s0) = sampler_state 
{
    texture = <tex0>;
};

struct Interpolants
{
  float4 positionSS			: POSITION;         // Screen space position
  float3 tex				: TEXCOORD0;        // texture coordinates
  half3	 colorDiffuse		: COLOR0;			// diffuse color
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
	// transform and output
	o.positionSS = 	mul(Pos, mWorldViewProj);
	
	// calculate light of the sun
	if(g_bEnableSunLight)
	{
		float3 worldNormal = normalize( mul( Norm, (float3x3)mWorld ) );
		o.colorDiffuse = max(0,dot( sun_vec, worldNormal ))*colorDiffuse+colorAmbient;
	}
	else
	{
		o.colorDiffuse = half3(1,1,1);
	}
	
	//save the fog distance for later
    o.tex.xy = Tex;
	o.tex.z = Pos.y;
	return o;
}

////////////////////////////////////////////////////////////////////////////////
//
//                              Pixel Shader
//
////////////////////////////////////////////////////////////////////////////////
// Calculates fog factor based upon distance
half CalcFogFactor( half d )
{
    half fogCoeff = 0;
	fogCoeff = (d - g_fogParam.x)/g_fogParam.y;
    return saturate( fogCoeff);
}

half4 pixelShader(Interpolants i) : COLOR
{
	half4 o;
	half4 normalColor = tex2D(tex0Sampler, i.tex.xy);
	normalColor.xyz = normalColor.xyz*i.colorDiffuse;
	
	//calculate the fog factor
	half fog = CalcFogFactor(i.tex.z);
	o.xyz = lerp(normalColor.xyz, g_fogColor.xyz, fog);
	fog = saturate( (fog-0.2)*1.25 );
	o.w = lerp(normalColor.w, 0, fog);
	
	clip(o.w-0.5);
	
	// this makes the unconstructed mesh a little more transparent
	o.w*=0.6;
	return o;
}

////////////////////////////////////////////////////////////////////////////////
//
//                              shadow map : VS and PS
//
////////////////////////////////////////////////////////////////////////////////

void VertShadow( float3	Pos			: POSITION,
				 float3	Norm		: NORMAL,
				 float2	Tex			: TEXCOORD0,
                 out float4 oPos	: POSITION,
                 out float3	outTex	: TEXCOORD0,
                 out float2 Depth	: TEXCOORD1 )
{
    oPos = mul( Pos, mWorldViewProj );
    outTex.xy = Tex;
    outTex.z = Pos.y; // construction height in model space
    Depth.xy = oPos.zw;
}

float4 PixShadow( float3	inTex		: TEXCOORD0,
				 float2 Depth		: TEXCOORD1) : COLOR
{
	half alpha = tex2D(tex0Sampler, inTex.xy).w;
	
	//calculate the fog factor
	alpha = CalcFogFactor(inTex.z);
	alpha = lerp(1,0, alpha > 0.5);
	clip(alpha-0.5);
	
    float d = Depth.x / Depth.y;
    return float4(0, d.xx,alpha);
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
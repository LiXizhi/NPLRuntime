// Author: LiXizhi
// Date: 2006/10
// Desc: instanced mesh with normal, This significantly reduces the CPU overhead of submitting many separate draw calls 
// and is a great technique for rendering trees, rocks, grass, RTS units and other groups of similar (but necessarily identical) objects. 
// please refer to Nvidia SDK sample "Instancing" for more information. 

#define ALPHA_TESTING_REF  0.5
////////////////////////////////////////////////////////////////////////////////
//  Per frame parameters
float4x4 mViewProj: viewprojection;
float4x4 mView: view;

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
float3 g_EyePositionW	:worldcamerapos;

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

struct Interpolants
{
  float4 positionSS			: POSITION;         // Screen space position
  float3 tex				: TEXCOORD0;        // texture coordinates
  float3 tex1				: TEXCOORD1;        // texture coordinates
  half3	 colorDiffuse		: COLOR0;			// diffuse color
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
							float2	Tex			: TEXCOORD0,
							float4 vInstanceMatrix1 : TEXCOORD1,
							float4 vInstanceMatrix2 : TEXCOORD2,
							float4 vInstanceMatrix3 : TEXCOORD3)
{
	Interpolants o = (Interpolants)0;
	
	// We've encoded the 4x3 world matrix in a 3x4, so do a quick transpose so we can use it in DX
	float4 row1 = float4(vInstanceMatrix1.x,vInstanceMatrix2.x,vInstanceMatrix3.x,0);
	float4 row2 = float4(vInstanceMatrix1.y,vInstanceMatrix2.y,vInstanceMatrix3.y,0);
	float4 row3 = float4(vInstanceMatrix1.z,vInstanceMatrix2.z,vInstanceMatrix3.z,0);
	float4 row4 = float4(vInstanceMatrix1.w,vInstanceMatrix2.w,vInstanceMatrix3.w,1);
	float4x4 mWorld = float4x4(row1,row2,row3,row4);
	
	// world space position: 
	float4 worldPos = mul( Pos, mWorld );
	// camera space position
	float4 cameraPos = mul( worldPos, mView ); 
	// screen space position
	o.positionSS = 	mul(worldPos, mViewProj);
	// world space normal
	float3 worldNormal = normalize( mul( Norm, (float3x3)mWorld ) ); 
	
	// calculate light of the sun
	if(g_bEnableSunLight)
	{
		o.colorDiffuse = max(0,dot( sun_vec, worldNormal ))*colorDiffuse+colorAmbient;
	}
	else
	{
		half3 difCol = colorAmbient+colorDiffuse;
		o.colorDiffuse = difCol;
	}
	
	o.tex.xy = Tex;
	//save the fog distance
    o.tex.z = CalcFogFactor(cameraPos.z);
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
		o.xyz = lerp(normalColor.xyz, g_fogColor.xyz, fog);
		fog = saturate( (fog-0.8)*16 );
		o.w = lerp(normalColor.w, 0, fog);
	}
	else
	{
		o = normalColor;
	}
	
	return o;
}

////////////////////////////////////////////////////////////////////////////////
//
//                              shadow map : VS and PS
//
////////////////////////////////////////////////////////////////////////////////

void VertShadow( float4	Pos			: POSITION,
				 float3	Norm		: NORMAL,
				 float2	Tex			: TEXCOORD0,
				 float4 vInstanceMatrix1 : TEXCOORD1,
				 float4 vInstanceMatrix2 : TEXCOORD2,
				 float4 vInstanceMatrix3 : TEXCOORD3,
                 out float4 oPos	: POSITION,
                 out float2	outTex	: TEXCOORD0,
                 out float2 Depth	: TEXCOORD1 )
{
	// We've encoded the 4x3 world matrix in a 3x4, so do a quick transpose so we can use it in DX
	float4 row1 = float4(vInstanceMatrix1.x,vInstanceMatrix2.x,vInstanceMatrix3.x,0);
	float4 row2 = float4(vInstanceMatrix1.y,vInstanceMatrix2.y,vInstanceMatrix3.y,0);
	float4 row3 = float4(vInstanceMatrix1.z,vInstanceMatrix2.z,vInstanceMatrix3.z,0);
	float4 row4 = float4(vInstanceMatrix1.w,vInstanceMatrix2.w,vInstanceMatrix3.w,1);
	float4x4 mWorld = float4x4(row1,row2,row3,row4);
	
	// world space position: 
	float4 worldPos = mul( Pos, mWorld );
	// screen space position
	oPos = 	mul(worldPos, mViewProj);
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
		VertexShader = compile vs_2_0 vertexShader();
		PixelShader  = compile ps_2_a pixelShader();
		
		FogEnable = false;
	}
}

technique GenShadowMap
{
    pass p0
    {
        VertexShader = compile vs_2_0 VertShadow();
        PixelShader = compile ps_2_a PixShadow();
        FogEnable = false;
    }
}
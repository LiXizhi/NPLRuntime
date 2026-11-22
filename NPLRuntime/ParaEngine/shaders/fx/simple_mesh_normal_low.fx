// Author: LiXizhi
// Desc: 2006/4

#define ALPHA_TESTING_REF  0.5
#define MAX_LIGHTS_NUM	4
////////////////////////////////////////////////////////////////////////////////
//  Per frame parameters
#ifdef STATIC_MESH_INSTANCED
float4x4 mViewProj: viewprojection;
float4x4 mView: view;
#else
float4x4 mWorldViewProj: worldviewprojection;
//float4x4 mViewProj: viewprojection;
float4x4 mWorldView: worldview;
float4x4 mWorld: world;
#endif

float3 sun_vec: sunvector;

float3	colorDiffuse:materialdiffuse;
float3	colorAmbient:ambientlight;
float3	colorEmissive:materialemissive = half3(0,0,0);

////////////////////////////////////////////////////////////////////////////////
// per technique parameters
float4   g_fogParam : fogparameters; // (fogstart, fogrange, fogDensity, reserved)
float4    g_fogColor : fogColor;

// static branch boolean constants
bool g_bEnableSunLight	:sunlightenable;
bool g_bAlphaTesting	:alphatesting;
bool g_bEnableFog		:fogenable;
bool g_bRGBOnlyTexturAnim  :boolean7;
float g_bReflectFactor	:reflectfactor;
float3 g_EyePositionW	:worldcamerapos;
float2 g_TexOffset		:ConstVector0; // TODO: for testing texture animation: x,y for translation
float3 g_TexRot			:ConstVector1;
float2 g_TexScale		:ConstVector2 = { 1.0f, 1.0f };
//bool g_bNormalMap		:boolean6;
float g_opacity			:opacity = 1.f; 

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
  float3 tex1				: TEXCOORD1;        // texture coordinates
  float4 colorDiffuse		: TEXCOORD2;		// diffuse color
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
#ifdef STATIC_MESH_INSTANCED
// The instance stream contains the three affine columns, including translation in w.
float4 InstancePosition(float4 pos, float4 x, float4 y, float4 z)
{
    return float4(dot(pos, x), dot(pos, y), dot(pos, z), 1.0);
}
#endif
#ifdef CHARACTER_SKINNED_LOW
float4 g_mWorldMatrixArray[174] : WORLDMATRIXARRAY;

void SkinVertex(inout float4 pos, inout float3 norm, float4 weights, float4 indices)
{
    float wSum = weights.x + weights.y + weights.z + weights.w;
    if (wSum < 0.001)
        return;
    int4 i = (int4)(indices * 255.0 + 0.5) * 3;
    float4 r0 = g_mWorldMatrixArray[i.x] * weights.x;
    float4 r1 = g_mWorldMatrixArray[i.x + 1] * weights.x;
    float4 r2 = g_mWorldMatrixArray[i.x + 2] * weights.x;
    if (weights.y > 0.0)
    {
        r0 += g_mWorldMatrixArray[i.y] * weights.y;
        r1 += g_mWorldMatrixArray[i.y + 1] * weights.y;
        r2 += g_mWorldMatrixArray[i.y + 2] * weights.y;
        if (weights.z > 0.0)
        {
            r0 += g_mWorldMatrixArray[i.z] * weights.z;
            r1 += g_mWorldMatrixArray[i.z + 1] * weights.z;
            r2 += g_mWorldMatrixArray[i.z + 2] * weights.z;
            if (weights.w > 0.0)
            {
                r0 += g_mWorldMatrixArray[i.w] * weights.w;
                r1 += g_mWorldMatrixArray[i.w + 1] * weights.w;
                r2 += g_mWorldMatrixArray[i.w + 2] * weights.w;
            }
        }
    }
    float4 p4 = float4(pos.xyz, 1.0);
    pos = float4(dot(p4, r0), dot(p4, r1), dot(p4, r2), 1.0);
    norm = float3(dot(norm, r0.xyz), dot(norm, r1.xyz), dot(norm, r2.xyz));
}
#endif
Interpolants vertexShader(	float4	Pos			: POSITION,
#ifdef STATIC_MESH_INSTANCED
    float4 instanceX : TEXCOORD1,
    float4 instanceY : TEXCOORD2,
    float4 instanceZ : TEXCOORD3,
#endif
#ifdef CHARACTER_SKINNED_LOW
    float4 weights : BLENDWEIGHT,
    float4 indices : BLENDINDICES,
#endif
    float3	Norm		: NORMAL,
							float2	Tex			: TEXCOORD0)
{
	Interpolants o = (Interpolants)0;
#ifdef CHARACTER_SKINNED_LOW
    SkinVertex(Pos, Norm, weights, indices);
#endif
#ifdef STATIC_MESH_INSTANCED
    float4 worldPos = InstancePosition(Pos, instanceX, instanceY, instanceZ);
    o.positionSS = mul(worldPos, mViewProj);
    float4 cameraPos = mul(worldPos, mView);
    float3 worldNormal = normalize(float3(dot(Norm, instanceX.xyz), dot(Norm, instanceY.xyz), dot(Norm, instanceZ.xyz)));
#else
    // screen space position
	o.positionSS = 	mul(Pos, mWorldViewProj);
	// world space position: 
	// TODO: this can be removed if we can do N.L in camera space. 
	float3 worldPos = mul( Pos, mWorld ).xyz;
	// camera space position
	float4 cameraPos = mul( Pos, mWorldView ); 
	// world space normal
	float3 worldNormal = normalize( mul( Norm, (float3x3)mWorld ) ); 
	
#endif
	// calculate light of the sun
	if(g_bEnableSunLight)
	{
		//o.colorDiffuse = max(0,dot( sun_vec, worldNormal ))*colorDiffuse;
		//o.colorDiffuse += colorAmbient;
		float lightFactor = dot(sun_vec,worldNormal);
		o.colorDiffuse.xyz = colorDiffuse * saturate(lightFactor);

		//float3 groundColor = max( (colorAmbient - float3(0.15,0.10,0.05)),float3(0.05,0.05,0.05));
		//float3 amb = lerp(groundColor,colorAmbient,lightFactor);
		o.colorDiffuse.xyz += colorAmbient;
	}
	else
	{
		o.colorDiffuse.xyz = min(1, colorDiffuse+colorAmbient).xyz;
	}

	// uv offset
	float2 uv = Tex + g_TexOffset.xy;

	// uv scale
	uv.x *= g_TexScale.x;
	uv.y *= g_TexScale.y;

	// uv rotate
	float sinNum = sin(g_TexRot.x);
	float cosNum = cos(g_TexRot.x);
	uv -= float2(g_TexRot.y, g_TexRot.z);
	uv = mul(uv, float2x2(cosNum, -sinNum, sinNum, cosNum));
	uv += float2(g_TexRot.y, g_TexRot.z);

	o.tex.xy = uv;

	o.tex.zw = Tex;
	//save the fog distance
    //o.tex.z = CalcFogFactor(cameraPos.z);
    o.colorDiffuse.w = CalcFogFactor(cameraPos.z);
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

	if(g_bRGBOnlyTexturAnim)
		normalColor.w = tex2D(tex0Sampler, i.tex.zw).w;

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
		half fog = i.colorDiffuse.w;
		o.xyz = lerp(normalColor.xyz, g_fogColor.xyz, fog);
		fog = saturate( (fog-0.8)*16 );
		o.w = lerp(normalColor.w, 0, fog);
	}
	else
	{
		o = normalColor;
	}
	o.rgb += colorEmissive;
	o.w *= g_opacity;
	
	return o;
}

////////////////////////////////////////////////////////////////////////////////
//
//                              shadow map : VS and PS
//
////////////////////////////////////////////////////////////////////////////////

void VertShadow( float4	Pos			: POSITION,
#ifdef STATIC_MESH_INSTANCED
    float4 instanceX : TEXCOORD1,
    float4 instanceY : TEXCOORD2,
    float4 instanceZ : TEXCOORD3,
#endif
#ifdef CHARACTER_SKINNED_LOW
    float4 weights : BLENDWEIGHT,
    float4 indices : BLENDINDICES,
#endif
    float3	Norm		: NORMAL,
				 float2	Tex			: TEXCOORD0,
                 out float4 oPos	: POSITION,
                 out float2	outTex	: TEXCOORD0,
                 out float2 Depth	: TEXCOORD1 )
{
#ifdef STATIC_MESH_INSTANCED
    oPos = mul(InstancePosition(Pos, instanceX, instanceY, instanceZ), mViewProj);
#else
#ifdef CHARACTER_SKINNED_LOW
    SkinVertex(Pos, Norm, weights, indices);
#endif
    oPos = mul( Pos, mWorldViewProj );
#endif
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
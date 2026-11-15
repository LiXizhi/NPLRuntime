// Author: ParaEngine Unified Runtime
// Desc: Hardware-instanced static mesh Deferred MRT G-Buffer & Shadow shader

#define ALPHA_TESTING_REF  0.5

float4x4 mViewProj : viewprojection;
float4x4 mView : view;

float3 sun_vec : sunvector;

float3 colorDiffuse : materialdiffuse;
float3 colorAmbient : ambientlight;
float3 colorEmissive : materialemissive = float3(0, 0, 0);

/** x is sun light strength, y is block light strength. */
float3 BlockLightStrength : LightStrength;

bool g_bEnableSunLight : sunlightenable;
bool g_bAlphaTesting : alphatesting;
bool g_bRGBOnlyTexturAnim : boolean7;
float3 g_EyePositionW : worldcamerapos;
float2 g_TexAnim : ConstVector0;
float2 g_CategoryID : ConstVector1;
float g_opacity : opacity = 1.0;

texture tex0 : TEXTURE;
sampler tex0Sampler : register(s0) = sampler_state
{
	texture = <tex0>;
};

struct Interpolants
{
	float4 positionSS : POSITION;
	float4 tex : TEXCOORD0;
	float4 normal : TEXCOORD3;
};

struct BlockPSOut
{
	float4 Color : COLOR0;
	float4 Depth : COLOR1;
	float4 BlockInfo : COLOR2;
	float4 Normal : COLOR3;
};

Interpolants vertexShader(
	float4 Pos : POSITION,
	float3 Norm : NORMAL,
	float2 Tex : TEXCOORD0,
	float4 vInstanceMatrix1 : TEXCOORD1,
	float4 vInstanceMatrix2 : TEXCOORD2,
	float4 vInstanceMatrix3 : TEXCOORD3)
{
	Interpolants o = (Interpolants)0;

	float4 row1 = float4(vInstanceMatrix1.x, vInstanceMatrix2.x, vInstanceMatrix3.x, 0);
	float4 row2 = float4(vInstanceMatrix1.y, vInstanceMatrix2.y, vInstanceMatrix3.y, 0);
	float4 row3 = float4(vInstanceMatrix1.z, vInstanceMatrix2.z, vInstanceMatrix3.z, 0);
	float4 row4 = float4(vInstanceMatrix1.w, vInstanceMatrix2.w, vInstanceMatrix3.w, 1);
	float4x4 mWorld = float4x4(row1, row2, row3, row4);

	float4 worldPos = mul(Pos, mWorld);
	float4 cameraPos = mul(worldPos, mView);
	o.positionSS = mul(worldPos, mViewProj);

	float3 worldNormal = normalize(mul(Norm, (float3x3)mWorld));
	o.normal.xyz = worldNormal * 0.5 + 0.5;
	o.normal.a = cameraPos.z;

	o.tex.xy = Tex + g_TexAnim.xy;
	o.tex.zw = Tex;
	return o;
}

BlockPSOut pixelShader(Interpolants i)
{
	BlockPSOut output;
	float4 o;
	float4 normalColor = tex2D(tex0Sampler, i.tex.xy);

	if (g_bRGBOnlyTexturAnim)
		normalColor.w = tex2D(tex0Sampler, i.tex.zw).w;

	if (g_bAlphaTesting)
	{
		clip(normalColor.w - ALPHA_TESTING_REF);
	}

	o = normalColor;
	o.rgb = o.rgb * colorDiffuse;
	o.rgb += colorEmissive;
	o.w *= g_opacity;

	output.Color = o;
	float category = 1.0;
	if (g_CategoryID.x > 0)
		category = g_CategoryID.x / 256.0;

	float alpha = (normalColor.w < ALPHA_TESTING_REF) ? 0 : 1;

	output.BlockInfo = float4(category, BlockLightStrength.x, BlockLightStrength.y, alpha);
	output.Depth = float4(i.normal.a, 0, 0, alpha);
	output.Normal = float4(i.normal.xyz, alpha);

	return output;
}

void VertShadow(
	float4 Pos : POSITION,
	float3 Norm : NORMAL,
	float2 Tex : TEXCOORD0,
	float4 vInstanceMatrix1 : TEXCOORD1,
	float4 vInstanceMatrix2 : TEXCOORD2,
	float4 vInstanceMatrix3 : TEXCOORD3,
	out float4 oPos : POSITION,
	out float4 outTex : TEXCOORD0)
{
	float4 row1 = float4(vInstanceMatrix1.x, vInstanceMatrix2.x, vInstanceMatrix3.x, 0);
	float4 row2 = float4(vInstanceMatrix1.y, vInstanceMatrix2.y, vInstanceMatrix3.y, 0);
	float4 row3 = float4(vInstanceMatrix1.z, vInstanceMatrix2.z, vInstanceMatrix3.z, 0);
	float4 row4 = float4(vInstanceMatrix1.w, vInstanceMatrix2.w, vInstanceMatrix3.w, 1);
	float4x4 mWorld = float4x4(row1, row2, row3, row4);

	float4 worldPos = mul(Pos, mWorld);
	oPos = mul(worldPos, mViewProj);
	outTex.xy = Tex;
	outTex.zw = oPos.zw;
}

float4 PixShadow(float4 inTex : TEXCOORD0) : COLOR0
{
	float alpha = tex2D(tex0Sampler, inTex.xy).w;
	if (g_bAlphaTesting)
	{
		clip(alpha - ALPHA_TESTING_REF);
	}
	return float4(inTex.z, 0, 0, 1);
}

technique SimpleMesh_vs30_ps30
{
	pass P0
	{
		VertexShader = compile vs_2_a vertexShader();
		PixelShader = compile ps_2_a pixelShader();
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

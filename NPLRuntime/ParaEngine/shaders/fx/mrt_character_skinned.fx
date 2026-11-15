// Author: ParaEngine Unified Runtime
// Desc: Hardware GPU-skinned character / ParaX mesh Deferred MRT G-Buffer & Shadow shader

#define ALPHA_TESTING_REF  0.5

float4x4 mWorldViewProj : worldviewprojection;
float4x4 mWorldView : worldview;
float4x4 mWorld : world;
float4 g_mWorldMatrixArray[174] : WORLDMATRIXARRAY;

float3 sun_vec : sunvector;

float3 colorDiffuse : materialdiffuse;
float3 colorAmbient : ambientlight;
float3 colorEmissive : materialemissive = float3(0, 0, 0);

/** x is sun light strength, y is block light strength. */
float3 BlockLightStrength : LightStrength;

bool g_bEnableSunLight : sunlightenable;
bool g_bAlphaTesting : alphatesting;
bool g_bRGBOnlyTexturAnim : boolean7;
float g_bReflectFactor : reflectfactor;
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

void SkinVertex(float3 pos, float3 norm, float4 blendWeights, float4 blendIndices,
	out float4 outPos, out float3 outNorm)
{
	float wSum = blendWeights.x + blendWeights.y + blendWeights.z + blendWeights.w;
	if (wSum < 0.001)
	{
		outPos = float4(pos, 1.0);
		outNorm = norm;
	}
	else
	{
		int4 idx = (int4)(blendIndices * 255.0 + 0.5) * 3;
		float4 p4 = float4(pos, 1.0);

		float4 r0 = g_mWorldMatrixArray[idx.x + 0] * blendWeights.x
			+ g_mWorldMatrixArray[idx.y + 0] * blendWeights.y
			+ g_mWorldMatrixArray[idx.z + 0] * blendWeights.z
			+ g_mWorldMatrixArray[idx.w + 0] * blendWeights.w;
		float4 r1 = g_mWorldMatrixArray[idx.x + 1] * blendWeights.x
			+ g_mWorldMatrixArray[idx.y + 1] * blendWeights.y
			+ g_mWorldMatrixArray[idx.z + 1] * blendWeights.z
			+ g_mWorldMatrixArray[idx.w + 1] * blendWeights.w;
		float4 r2 = g_mWorldMatrixArray[idx.x + 2] * blendWeights.x
			+ g_mWorldMatrixArray[idx.y + 2] * blendWeights.y
			+ g_mWorldMatrixArray[idx.z + 2] * blendWeights.z
			+ g_mWorldMatrixArray[idx.w + 2] * blendWeights.w;

		outPos = float4(dot(p4, r0), dot(p4, r1), dot(p4, r2), 1.0);
		outNorm = float3(dot(norm, r0.xyz), dot(norm, r1.xyz), dot(norm, r2.xyz));
	}
}

Interpolants vertexShader(
	float4 Pos : POSITION,
	float3 Norm : NORMAL,
	float2 Tex : TEXCOORD0,
	float4 blendWeights : BLENDWEIGHT,
	float4 blendIndices : BLENDINDICES)
{
	Interpolants o = (Interpolants)0;
	float4 skinnedPos;
	float3 skinnedNorm;
	SkinVertex(Pos.xyz, Norm, blendWeights, blendIndices, skinnedPos, skinnedNorm);

	o.positionSS = mul(skinnedPos, mWorldViewProj);
	float4 cameraPos = mul(skinnedPos, mWorldView);
	float3 worldNormal = normalize(mul(skinnedNorm, (float3x3)mWorld));
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
	float4 blendWeights : BLENDWEIGHT,
	float4 blendIndices : BLENDINDICES,
	out float4 oPos : POSITION,
	out float4 outTex : TEXCOORD0)
{
	float4 skinnedPos;
	float3 skinnedNorm;
	SkinVertex(Pos.xyz, Norm, blendWeights, blendIndices, skinnedPos, skinnedNorm);
	oPos = mul(skinnedPos, mWorldViewProj);
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

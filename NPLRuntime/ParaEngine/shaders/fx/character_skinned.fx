// Author: ParaEngine Unified Runtime
// Desc: Hardware GPU-skinned character / ParaX mesh forward & shadow shader (up to 58 bones, 4x3 affine palette)

#define ALPHA_TESTING_REF  0.5

float4x4 mWorldViewProj : worldviewprojection;
float4x4 mWorldView : worldview;
float4x4 mWorld : world;
float4 g_mWorldMatrixArray[174] : WORLDMATRIXARRAY;

float4 sun_vec : sunvector;

float3 colorDiffuse : materialdiffuse;
float3 colorAmbient : ambientlight;
float3 colorEmissive : materialemissive = float3(0, 0, 0);

#include "mesh_local_lights.h"

float4 g_fogParam : fogparameters;
float4 g_fogColor : fogColor;

bool g_bEnableSunLight : sunlightenable;
bool g_bAlphaTesting : alphatesting;
bool g_bEnableFog : fogenable;
bool g_bReflectionMap : boolean5;
bool g_bEnvironmentMap : boolean6;
float g_bReflectFactor : reflectfactor;
float3 g_EyePositionW : worldcamerapos;
float2 g_TexOffset : ConstVector0;
float3 g_TexRot : ConstVector1;
float2 g_TexScale : ConstVector2 = float2(1.0, 1.0);
float g_opacity : opacity = 1.0;

texture tex0 : TEXTURE;
sampler tex0Sampler : register(s0) = sampler_state
{
	texture = <tex0>;
};

texture tex1 : TEXTURE;
sampler tex1Sampler : register(s1) = sampler_state
{
	texture = <tex1>;
};

texture tex2 : TEXTURE;
samplerCUBE texCubeSampler : register(s2) = sampler_state
{
	texture = <tex2>;
};

struct Interpolants
{
	float4 positionSS : POSITION;
	float3 tex : TEXCOORD0;
	float3 tex1 : TEXCOORD1;
	float3 colorDiffuse : TEXCOORD2;
};

float CalcFogFactor(float d)
{
	float fogCoeff = (d - g_fogParam.x) / g_fogParam.y;
	return saturate(fogCoeff);
}

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

		float4 r0 = g_mWorldMatrixArray[idx.x + 0] * blendWeights.x;
		float4 r1 = g_mWorldMatrixArray[idx.x + 1] * blendWeights.x;
		float4 r2 = g_mWorldMatrixArray[idx.x + 2] * blendWeights.x;
		if (blendWeights.y + blendWeights.z + blendWeights.w > 0.001)
		{
			if (blendWeights.y > 0.001)
			{
				r0 += g_mWorldMatrixArray[idx.y + 0] * blendWeights.y;
				r1 += g_mWorldMatrixArray[idx.y + 1] * blendWeights.y;
				r2 += g_mWorldMatrixArray[idx.y + 2] * blendWeights.y;
			}
			if (blendWeights.z > 0.001)
			{
				r0 += g_mWorldMatrixArray[idx.z + 0] * blendWeights.z;
				r1 += g_mWorldMatrixArray[idx.z + 1] * blendWeights.z;
				r2 += g_mWorldMatrixArray[idx.z + 2] * blendWeights.z;
			}
			if (blendWeights.w > 0.001)
			{
				r0 += g_mWorldMatrixArray[idx.w + 0] * blendWeights.w;
				r1 += g_mWorldMatrixArray[idx.w + 1] * blendWeights.w;
				r2 += g_mWorldMatrixArray[idx.w + 2] * blendWeights.w;
			}
		}

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
	float3 worldPos = mul(skinnedPos, mWorld).xyz;
	float4 cameraPos = mul(skinnedPos, mWorldView);
	float3 worldNormal = normalize(mul(skinnedNorm, (float3x3)mWorld));

	if (g_bEnableSunLight)
	{
		o.colorDiffuse = max(0, dot(sun_vec.xyz, worldNormal)) * colorDiffuse;
		o.colorDiffuse += colorAmbient;
	}
	else
	{
		o.colorDiffuse = min(1, colorDiffuse + colorAmbient);
	}

	o.colorDiffuse = ApplyLocalLights(o.colorDiffuse, worldPos, worldNormal);

	float2 uv = Tex + g_TexOffset.xy;
	uv.x *= g_TexScale.x;
	uv.y *= g_TexScale.y;

	float sinNum = sin(g_TexRot.x);
	float cosNum = cos(g_TexRot.x);
	uv -= float2(g_TexRot.y, g_TexRot.z);
	uv = mul(uv, float2x2(cosNum, -sinNum, sinNum, cosNum));
	uv += float2(g_TexRot.y, g_TexRot.z);

	o.tex.xy = uv;
	o.tex.z = CalcFogFactor(cameraPos.z);
	if (g_bReflectionMap)
	{
		o.tex1.xy = (o.positionSS.xy + o.positionSS.w) * 0.5;
		o.tex1.z = o.positionSS.w;
	}
	else if (g_bEnvironmentMap)
	{
		float3 eyeVector = normalize(worldPos - g_EyePositionW);
		o.tex1 = normalize(reflect(eyeVector, worldNormal));
	}

	return o;
}

float4 pixelShader(Interpolants i) : COLOR0
{
	float4 o;
	float4 normalColor = tex2D(tex0Sampler, i.tex.xy);
	normalColor.xyz = normalColor.xyz * i.colorDiffuse;

	if (g_bAlphaTesting)
	{
		clip(normalColor.w - ALPHA_TESTING_REF);
	}

	if (g_bReflectionMap)
	{
		float4 reflection = tex2D(tex1Sampler, float2(1, 1) - i.tex1.xy / i.tex1.z);
		normalColor.xyz = lerp(normalColor.rgb, reflection.rgb, g_bReflectFactor);
	}
	else if (g_bEnvironmentMap)
	{
		float4 reflection = texCUBE(texCubeSampler, i.tex1);
		normalColor.xyz = lerp(normalColor.rgb, reflection.rgb, g_bReflectFactor);
	}

	if (g_bEnableFog)
	{
		float fog = i.tex.z;
		o.xyz = lerp(normalColor.xyz, g_fogColor.xyz, fog);
		fog = saturate((fog - 0.8) * 16.0);
		o.w = lerp(normalColor.w, 0.0, fog);
	}
	else
	{
		o = normalColor;
	}
	o.rgb += colorEmissive;
	o.w *= g_opacity;
	return o;
}

struct ShadowVSOut
{
	float4 oPos : POSITION;
	float2 outTex : TEXCOORD0;
	float2 Depth : TEXCOORD1;
};

ShadowVSOut VertShadow(
	float4 Pos : POSITION,
	float3 Norm : NORMAL,
	float2 Tex : TEXCOORD0,
	float4 blendWeights : BLENDWEIGHT,
	float4 blendIndices : BLENDINDICES)
{
	ShadowVSOut o;
	float4 skinnedPos;
	float3 skinnedNorm;
	SkinVertex(Pos.xyz, Norm, blendWeights, blendIndices, skinnedPos, skinnedNorm);
	o.oPos = mul(skinnedPos, mWorldViewProj);
	o.outTex = Tex;
	o.Depth.xy = o.oPos.zw;
	return o;
}

float4 PixShadow(ShadowVSOut s) : COLOR0
{
	float alpha = tex2D(tex0Sampler, s.outTex.xy).w;
	if (g_bAlphaTesting)
	{
		if (alpha < ALPHA_TESTING_REF)
			clip(-1.0);
	}
	float d = s.Depth.x / s.Depth.y;
	return float4(0.0, d.xx, alpha);
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

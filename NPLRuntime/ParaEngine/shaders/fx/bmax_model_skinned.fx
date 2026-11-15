// Author: ParaEngine Unified Runtime
// Desc: Hardware GPU-skinned BMaxModel forward & shadow shader (up to 58 bones, 4x3 affine palette)

float4x4 mWorldViewProj : worldviewprojection;
float4x4 mWorld : world;
float4 g_mWorldMatrixArray[174] : WORLDMATRIXARRAY;

float4 light_params : ConstVector0;
float4 sun_vec : sunvector;
float3 colorDiffuse : materialdiffuse;
float3 colorAmbient : ambientlight;

bool g_bEnableFog : fogenable;
float4 g_fogParam : fogparameters;
float4 g_fogColor : fogColor;
float g_opacity : opacity = 1.f;

float4 materialUV : materialUV;
float4 materialBaseColor : materialBaseColor;
float4 materialEmissiveColor : materialEmissiveColor;
float materialMetallic : materialMetallic;

struct VSOut
{
	float4 pos : POSITION;
	float4 color : COLOR0;
};

texture tex0 : TEXTURE;
sampler tex0Sampler : register(s0) = sampler_state
{
	Texture = <tex0>;
	MinFilter = POINT;
	MagFilter = POINT;
};

texture tex1 : TEXTURE;
sampler tex1Sampler : register(s1) = sampler_state
{
	Texture = <tex1>;
	AddressU = wrap;
	AddressV = wrap;
	MagFilter = Linear;
	MinFilter = Linear;
	MipFilter = Linear;
};

float CalcFogFactor(float d)
{
	float fogCoeff = (d - g_fogParam.x) / g_fogParam.y;
	return clamp(fogCoeff, 0.0, 1.0);
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

VSOut MainVS(
	float4 pos : POSITION,
	float3 Norm : NORMAL,
	float4 color : COLOR0,
	float4 blendWeights : BLENDWEIGHT,
	float4 blendIndices : BLENDINDICES)
{
	VSOut output;
	float4 skinnedPos;
	float3 skinnedNorm;
	SkinVertex(pos.xyz, Norm, blendWeights, blendIndices, skinnedPos, skinnedNorm);

	output.pos = mul(skinnedPos, mWorldViewProj);
	float3 worldNormal = normalize(mul(skinnedNorm, (float3x3)mWorld));
	float3 sun_lightmap = colorAmbient + colorDiffuse * saturate(dot(sun_vec.xyz, worldNormal));
	output.color.xyz = color.rgb * sun_lightmap;

	if (g_bEnableFog)
		output.color.w = CalcFogFactor(length(output.pos.xyz));
	else
		output.color.w = 0.0;

	return output;
}

float4 MainPS(VSOut input) : COLOR0
{
	float3 color = input.color.rgb;
	color = lerp(color, g_fogColor.xyz, input.color.w);
	return float4(color, g_opacity);
}

struct MaterialBlockVSOut
{
	float4 pos : POSITION;
	float2 texcoord : TEXCOORD0;
	half4 color : COLOR0;
};

MaterialBlockVSOut MaterialMainVS(
	float4 pos : POSITION,
	float3 Norm : NORMAL,
	half4 color : COLOR0,
	float4 blendWeights : BLENDWEIGHT,
	float4 blendIndices : BLENDINDICES)
{
	MaterialBlockVSOut output = (MaterialBlockVSOut)0;
	float4 skinnedPos;
	float3 skinnedNorm;
	SkinVertex(pos.xyz, Norm, blendWeights, blendIndices, skinnedPos, skinnedNorm);

	output.pos = mul(skinnedPos, mWorldViewProj);
	float3 normal = normalize(mul(skinnedNorm, (float3x3)mWorld));

	float3 sun_lightmap = colorAmbient + colorDiffuse * saturate(dot(sun_vec.xyz, normal));
	output.color.xyz = color.rgb * sun_lightmap;

	if (g_bEnableFog)
		output.color.w = CalcFogFactor(length(output.pos.xyz));
	else
		output.color.w = 0.0;

	float3 worldblockPos = mul(skinnedPos.xyz, (float3x3)mWorld).xyz;
	if (normal.x > 0.5) {
		output.texcoord = worldblockPos.zy;
	}
	else if (normal.x < -0.5) {
		output.texcoord = float2(32000.0 - worldblockPos.z, worldblockPos.y);
	}
	else if (normal.y > 0.5) {
		output.texcoord = worldblockPos.xz;
	}
	else if (normal.y < -0.5) {
		output.texcoord = float2(32000.0 - worldblockPos.x, worldblockPos.z);
	}
	else if (normal.z > 0.5) {
		output.texcoord = float2(32000.0 - worldblockPos.x, worldblockPos.y);
	}
	else if (normal.z < -0.5) {
		output.texcoord = worldblockPos.xy;
	}
	output.texcoord += materialUV.zw;
	return output;
}

half4 MaterialMainPS(MaterialBlockVSOut input) : COLOR0
{
	float2 uv = (input.texcoord - floor(input.texcoord / materialUV.xy) * materialUV.xy) / materialUV.xy;
	uv.y = 1.0 - uv.y;

	float4 albedoColor = tex2D(tex0Sampler, uv);
	albedoColor = albedoColor * materialBaseColor;

	if (materialEmissiveColor.a > 0)
	{
		float4 emissiveColor = tex2D(tex1Sampler, uv);
		emissiveColor.rgb *= materialEmissiveColor.rgb;
		emissiveColor.a *= materialEmissiveColor.a;
		albedoColor.xyz = lerp(albedoColor.xyz, emissiveColor.rgb, emissiveColor.a);
	}

	half4 oColor = half4(lerp(albedoColor.xyz, g_fogColor.xyz, input.color.w), albedoColor.a * g_opacity);
	return oColor;
}

void VertShadow(
	float4 Pos : POSITION,
	float3 Norm : NORMAL,
	float4 color : COLOR0,
	float4 blendWeights : BLENDWEIGHT,
	float4 blendIndices : BLENDINDICES,
	out float4 oPos : POSITION,
	out float2 Depth : TEXCOORD1)
{
	float4 skinnedPos;
	float3 skinnedNorm;
	SkinVertex(Pos.xyz, Norm, blendWeights, blendIndices, skinnedPos, skinnedNorm);
	oPos = mul(skinnedPos, mWorldViewProj);
	Depth.xy = oPos.zw;
}

float4 PixShadow(float2 Depth : TEXCOORD1) : COLOR0
{
	float d = Depth.x / Depth.y;
	return float4(0.0, d.xx, 1.0);
}

technique SimpleMesh_vs20_ps20
{
	pass P0
	{
		VertexShader = compile vs_2_a MainVS();
		PixelShader = compile ps_2_a MainPS();
		FogEnable = false;
	}
	pass P1
	{
		VertexShader = compile vs_2_a MaterialMainVS();
		PixelShader = compile ps_2_a MaterialMainPS();
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
	pass p1
	{
		VertexShader = compile vs_2_a VertShadow();
		PixelShader = compile ps_2_a PixShadow();
		FogEnable = false;
	}
}

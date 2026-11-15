// Author: ParaEngine Unified Runtime
// Desc: Hardware GPU-skinned BMaxModel Deferred MRT G-Buffer & Shadow shader

float4x4 mWorldViewProj : worldviewprojection;
float4x4 mWorldView : worldview;
float4x4 mWorld : world;
float4 g_mWorldMatrixArray[174] : WORLDMATRIXARRAY;

float3 colorDiffuse : materialdiffuse;
float3 colorAmbient : ambientlight;
/** x is sun light strength, y is block light strength. */
float3 BlockLightStrength : LightStrength;
float g_opacity : opacity = 1.0;

float4 materialUV : materialUV;
float4 materialBaseColor : materialBaseColor;
float4 materialEmissiveColor : materialEmissiveColor;
float materialMetallic : materialMetallic;
float materialSpecular : materialSpecular;

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

texture tex2 : TEXTURE;
sampler normalSampler : register(s2) = sampler_state
{
	texture = <tex2>;
	AddressU = wrap;
	AddressV = wrap;
	MagFilter = Linear;
	MinFilter = Linear;
	MipFilter = Linear;
};

struct VSOut
{
	float4 pos : POSITION;
	float4 color : TEXCOORD0;
	float3 normal : TEXCOORD1;
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

VSOut MainVS(
	float4 pos : POSITION,
	float3 Norm : NORMAL,
	half4 color : COLOR0,
	float4 blendWeights : BLENDWEIGHT,
	float4 blendIndices : BLENDINDICES)
{
	VSOut output;
	float4 skinnedPos;
	float3 skinnedNorm;
	SkinVertex(pos.xyz, Norm, blendWeights, blendIndices, skinnedPos, skinnedNorm);

	output.pos = mul(skinnedPos, mWorldViewProj);
	float4 cameraPos = mul(skinnedPos, mWorldView);
	float3 worldNormal = normalize(mul(skinnedNorm, (float3x3)mWorld));
	output.normal = worldNormal * 0.5 + 0.5;
	output.color.xyz = color.rgb * colorDiffuse;
	output.color.w = cameraPos.z;
	return output;
}

BlockPSOut MainPS(VSOut input)
{
	BlockPSOut output;
	output.Color = float4(input.color.rgb, g_opacity);
	output.BlockInfo = float4(1, BlockLightStrength.x, BlockLightStrength.y, 1);
	output.Depth = float4(input.color.w, 0, 0, 1);
	output.Normal = float4(input.normal.xyz, 1);
	return output;
}

struct MaterialVSOut
{
	float4 pos : POSITION;
	float4 color : COLOR0;
	float2 texcoord : TEXCOORD0;
	float3 normal : TEXCOORD1;
	float3 tangent : TEXCOORD2;
	float3 binormal : TEXCOORD3;
};

MaterialVSOut MaterialMainVS(
	float4 pos : POSITION,
	float3 Norm : NORMAL,
	half4 color : COLOR0,
	float4 blendWeights : BLENDWEIGHT,
	float4 blendIndices : BLENDINDICES)
{
	MaterialVSOut output = (MaterialVSOut)0;
	float4 skinnedPos;
	float3 skinnedNorm;
	SkinVertex(pos.xyz, Norm, blendWeights, blendIndices, skinnedPos, skinnedNorm);

	output.pos = mul(skinnedPos, mWorldViewProj);
	float4 cameraPos = mul(skinnedPos, mWorldView);
	float3 worldNormal = normalize(mul(skinnedNorm, (float3x3)mWorld));

	output.color.xyz = color.rgb * colorDiffuse;
	output.color.w = cameraPos.z;

	float3 worldblockPos = mul(skinnedPos.xyz, (float3x3)mWorld).xyz;
	float3 normal = worldNormal;
	float3 binormal = float3(0.0, 0.0, 0.0);
	float3 tangent = float3(0.0, 0.0, 0.0);
	if (normal.x > 0.5) {
		tangent = float3(0.0, 0.0, -1.0);
		binormal = float3(0.0, -1.0, 0.0);
		output.texcoord = worldblockPos.zy;
	}
	else if (normal.x < -0.5) {
		tangent = float3(0.0, 0.0, 1.0);
		binormal = float3(0.0, -1.0, 0.0);
		output.texcoord = float2(32000.0 - worldblockPos.z, worldblockPos.y);
	}
	else if (normal.y > 0.5) {
		tangent = float3(1.0, 0.0, 0.0);
		binormal = float3(0.0, 0.0, 1.0);
		output.texcoord = worldblockPos.xz;
	}
	else if (normal.y < -0.5) {
		tangent = float3(1.0, 0.0, 0.0);
		binormal = float3(0.0, 0.0, 1.0);
		output.texcoord = float2(32000.0 - worldblockPos.x, worldblockPos.z);
	}
	else if (normal.z > 0.5) {
		tangent = float3(1.0, 0.0, 0.0);
		binormal = float3(0.0, -1.0, 0.0);
		output.texcoord = float2(32000.0 - worldblockPos.x, worldblockPos.y);
	}
	else if (normal.z < -0.5) {
		tangent = float3(-1.0, 0.0, 0.0);
		binormal = float3(0.0, -1.0, 0.0);
		output.texcoord = worldblockPos.xy;
	}
	output.texcoord += materialUV.zw;
	output.tangent = tangent;
	output.binormal = binormal;

	if (materialMetallic > 0.1)
	{
		output.normal = normal;
	}
	else
	{
		output.normal = worldNormal * 0.5 + 0.5;
	}

	return output;
}

BlockPSOut MaterialMainPS(MaterialVSOut input)
{
	BlockPSOut o;

	float2 uv = (input.texcoord - floor(input.texcoord / materialUV.xy) * materialUV.xy) / materialUV.xy;
	uv.y = 1.0 - uv.y;

	float4 albedoColor = tex2D(tex0Sampler, uv);
	albedoColor = albedoColor * materialBaseColor;

	o.Color = float4(albedoColor.xyz, g_opacity);
	o.BlockInfo = float4(1, BlockLightStrength.x, BlockLightStrength.y, 1);
	o.Depth = float4(input.color.w, 0, 0, 1);

	if (materialEmissiveColor.a > 0)
	{
		float4 emissiveColor = tex2D(tex1Sampler, uv);
		emissiveColor.rgb *= materialEmissiveColor.rgb;
		emissiveColor.a *= materialEmissiveColor.a;
		o.Color.rgb = lerp(o.Color.rgb, emissiveColor.rgb, emissiveColor.a);
		o.BlockInfo.b = clamp(o.BlockInfo.b + emissiveColor.a, 0, 1.0);
	}

	if (materialMetallic > 0.1)
	{
		float3 normal = input.normal.xyz;
		float3x3 tbnMatrix = float3x3(input.tangent, input.binormal, normal);
		float4 normalColor = tex2D(normalSampler, uv);
		float3 bumpNormal = mul((normalColor.xyz * 2.0 - 1.0), tbnMatrix);
		normal = normalize(bumpNormal);
		normal = normal * 0.5 + 0.5;
		o.Normal = float4(normal.xyz, normalColor.w * (1 - materialSpecular));
		o.BlockInfo.r = 50.0 / 255.0;
	}
	else
	{
		o.Normal = float4(input.normal.xyz, (1 - materialSpecular));
	}
	return o;
}

void VertShadow(
	float4 Pos : POSITION,
	float3 Norm : NORMAL,
	half4 color : COLOR0,
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
	return float4(Depth.x, 0.0, 0.0, 1.0);
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

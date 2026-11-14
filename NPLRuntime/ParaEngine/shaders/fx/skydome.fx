// Author: LiXizhi
// Desc: Unified cross-platform SkyDome shader (D3D11/D3D12/Vulkan/Metal/OpenGL/WebGPU)
#define NODISTORTION_FOG

float4x4 mWorldViewProj : worldviewprojection;
float4 sunvector : sunvector; // normalized sun direction (.xyz)

float4 g_fogParam : fogparameters;
float4 g_fogColor : fogColor;
float4 g_skycolorfactor : ConstVector0;
bool g_bEnableFog : fogenable;

float skyDomeHeight : skyDomeHeight = 16.0;
float3 lightSkyColor : lightSkyColor = float3(0.627, 0.831, 1.0);
float3 darkSkyColor : darkSkyColor = float3(0.0, 0.25, 0.45);
float3 sunColor : sunColor = float3(1.0, 1.0, 0.6);
float3 cloudColor : cloudColor = float3(1.0, 1.0, 1.0);
float2 cloudOffset : cloudOffset = float2(0.0, 0.0);
float cloudBlendWeight : cloudBlendWeight = 0.0;
float2 sunIntensity : sunIntensity = float2(1.0, 0.3);
float2 sunSize : sunSize = float2(500.0, 12.0);
float2 moonSize : moonSize = float2(500.0, 100.0);
float cloudThickness : cloudThickness = 1.0;
float sunsetGlowWeight : sunsetGlowWeight = 0.0;

texture tex0 : TEXTURE;
sampler tex0Sampler : register(s0) = sampler_state
{
	texture = <tex0>;
	MinFilter = Linear;
	MipFilter = Linear;
	MagFilter = Linear;
	AddressU = clamp;
	AddressV = clamp;
};

texture tex1 : TEXTURE;
sampler tex1Sampler : register(s1) = sampler_state
{
	texture = <tex1>;
	MinFilter = Linear;
	MipFilter = Linear;
	MagFilter = Linear;
	AddressU = wrap;
	AddressV = wrap;
};

struct SkyDomeVSIn
{
	float4 Pos : POSITION;
	float2 texCoord : TEXCOORD0;
};

struct SkyDomeVSOut
{
	float4 Pos : POSITION;
	float4 uv : TEXCOORD0;
	float3 normal : TEXCOORD1;
	float4 cloud_uv : TEXCOORD2;
};

SkyDomeVSOut Transform(SkyDomeVSIn v)
{
	SkyDomeVSOut o = (SkyDomeVSOut)0;
	o.Pos = mul(v.Pos, mWorldViewProj).xyww;
	o.uv.xy = v.texCoord;
	o.uv.z = v.Pos.y / max(skyDomeHeight, 0.001);
	o.normal = v.Pos.xyz;
	o.cloud_uv.xy = v.texCoord + cloudOffset * 4.0;
	o.cloud_uv.zw = v.texCoord + cloudOffset;
	return o;
}

float CalcFogFactor(float d)
{
	return saturate(d * 2.0);
}

float4 DaytimeSkyPs(SkyDomeVSOut i) : COLOR0
{
	float3 normal = normalize(i.normal);
	float dotSunAngle = dot(sunvector.xyz, normal);
	float dotSunAnglePos = saturate(dotSunAngle);
	float sunHighlight = pow(max(dotSunAnglePos, 0.0001), sunSize.x) * sunIntensity.x;
	float sunHalo = pow(max(dotSunAnglePos, 0.0001), sunSize.y) * sunIntensity.y;

	float normalizedVertexHeight = i.uv.z;
	float3 skyColor = lerp(lightSkyColor, darkSkyColor, normalizedVertexHeight);

	float2 sunsetGlowTexCoord = float2((1.0 - dotSunAngle) * 0.5, normalizedVertexHeight);
	float3 sunsetGlowColor = tex2D(tex0Sampler, sunsetGlowTexCoord).rgb;

	float cloud0 = tex2D(tex1Sampler, i.cloud_uv.xy).x;
	float cloud1 = tex2D(tex1Sampler, i.cloud_uv.zw).x;
	float cloud = saturate(lerp(cloud0, cloud1, cloudBlendWeight)) * cloudThickness;

	skyColor = lerp(skyColor, cloudColor, cloud);
	skyColor = lerp(skyColor, skyColor * sunsetGlowColor, sunsetGlowWeight);

	float3 color = skyColor + sunColor * (sunHalo + sunHighlight);
	float fog = CalcFogFactor(normalizedVertexHeight);
	return float4(color, fog);
}

float4 NightSkyPS(SkyDomeVSOut i) : COLOR0
{
	float3 normal = normalize(i.normal);
	float3 mirrorSunVector = float3(sunvector.x, -sunvector.y, sunvector.z);
	float normalizedVertexHeight = i.uv.z;
	float3 skyColor = lerp(lightSkyColor, darkSkyColor, normalizedVertexHeight);

	float dotSunAngle = dot(mirrorSunVector, normal);
	float dotSunAnglePos = saturate(dotSunAngle);
	float2 sunsetGlowTexCoord = float2((1.0 - dotSunAngle) * 0.5, normalizedVertexHeight);
	float3 sunsetGlowColor = tex2D(tex0Sampler, sunsetGlowTexCoord).rgb;

	float cloud0 = tex2D(tex1Sampler, i.cloud_uv.xy).x;
	float cloud1 = tex2D(tex1Sampler, i.cloud_uv.zw).x;
	float cloud = saturate(lerp(cloud0, cloud1, cloudBlendWeight));

	float star = tex2D(tex1Sampler, i.uv.xy * 6.0).y * 2.0;
	skyColor += star;

	skyColor = lerp(skyColor, cloudColor, cloud);
	skyColor = lerp(skyColor, skyColor * sunsetGlowColor, sunsetGlowWeight);

	float sunHighlight = pow(max(dotSunAnglePos, 0.0001), sunSize.x) * sunIntensity.x;
	float sunHalo = pow(max(dotSunAnglePos, 0.0001), sunSize.y) * sunIntensity.y;
	skyColor += sunColor * (sunHalo + sunHighlight);

	float3 moonVector = float3(-sunvector.x, sunvector.y, -sunvector.z);
	float dotMoonAngle = dot(moonVector, normal);
	float dotMoonAnglePos = saturate(dotMoonAngle);

	float moonSizeDot = dotMoonAnglePos - (1.0 - 1.25 / max(moonSize.x, 0.001));
	float moonHighlight = 0.0;
	if (moonSizeDot > 0.0)
		moonHighlight = clamp(pow(moonSizeDot * 300.0, 0.5), 0.0, 1.0);

	float moonHalo = pow(max(dotMoonAnglePos, 0.0001), moonSize.y) * sunIntensity.y;
	skyColor += float3(1.0, 1.0, 0.6) * (moonHalo + moonHighlight);

	float fog = CalcFogFactor(normalizedVertexHeight);
	return float4(skyColor, fog);
}

technique SimpleMesh_vs20_ps20
{
	pass P0
	{
		VertexShader = compile vs_2_0 Transform();
		PixelShader = compile ps_2_0 DaytimeSkyPs();
		FogEnable = false;
	}
	pass P1
	{
		VertexShader = compile vs_2_0 Transform();
		PixelShader = compile ps_2_0 NightSkyPS();
		FogEnable = false;
	}
}

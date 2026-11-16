#include "CommonDefine.h"
#include "FXAA.h"

#define SHADOW_BIAS 0.0025f

float3 decodeNormal(float3 normal)
{
	return normal*2.0-1.0;
}

float calculateLightDiffuseFactor(float3 lightDirection,float3 normal)
{
	return saturate(dot(lightDirection,normal));
}

float InterleavedGradientNoise(float2 screenPixelPos)
{
	return frac(52.9829189f * frac(dot(screenPixelPos, float2(0.06711056f, 0.00583715f))));
}

//1:no shadow
//0:full shadow
float getShadowFactor(sampler2D s,float2 uv,float testDepth,float shadowMapSize,float invShadowMapSize)
{
#ifdef HARDWARE_SHADOW_ENABLE
	return tex2D(s,float3(uv,testDepth));
#else
#ifdef PCF_SHADOW_ENABLE
	// Bilinear 2x2 PCF at exact texel centers using explicit mip0 point fetches
	const float2 uv_in_texel_f = uv * shadowMapSize - 0.5f;
	const float2 uv_in_texel_i = floor(uv_in_texel_f);
	const float2 scalar = frac(uv_in_texel_f);
	const float2 baseUV = (uv_in_texel_i + 0.5f) * invShadowMapSize;
	float shadow0 = tex2Dlod(s, float4(baseUV, 0.0f, 0.0f)).r >= testDepth ? 1.0f : 0.0f;
	float shadow1 = tex2Dlod(s, float4(baseUV + float2(invShadowMapSize, 0.0f), 0.0f, 0.0f)).r >= testDepth ? 1.0f : 0.0f;
	float shadow_up = lerp(shadow0, shadow1, scalar.x);
	shadow0 = tex2Dlod(s, float4(baseUV + float2(0.0f, invShadowMapSize), 0.0f, 0.0f)).r >= testDepth ? 1.0f : 0.0f;
	shadow1 = tex2Dlod(s, float4(baseUV + float2(invShadowMapSize, invShadowMapSize), 0.0f, 0.0f)).r >= testDepth ? 1.0f : 0.0f;
	float shadow_down = lerp(shadow0, shadow1, scalar.x);
	return lerp(shadow_up, shadow_down, scalar.y);
#else
	float shadow_depth = tex2Dlod(s, float4(uv, 0.0f, 0.0f)).r;
	float shadow = shadow_depth >= testDepth ? 1.0f : 0.0f;
	return shadow;
#endif
#endif
}

// shadow blending factor, so that the near camera shadow is darker
float calculatefadeShadowFactor(float viewDepth,float shadowRadius)
{
	return saturate((viewDepth-shadowRadius*0.88f)/(shadowRadius*0.12f));
}

// Percentage-Closer Soft Shadows (PCSS) with IGN-rotated blocker search & Golden-Angle Vogel disk PCF
float calculatePCSSShadowFactor(
	sampler2D s,
	float4 worldPosition,
	float viewDepth,
	float4x4 shadowMatrix,
	float4x4 shadowViewProjMatrix,
	float shadowMapSize,
	float invShadowMapSize,
	float shadowRadius,
	float shadowBias,
	float2 screenPixelPos,
	float qualityLevel)
{
	const float4 vPosShadowSpace = mul(worldPosition, shadowViewProjMatrix);
	const float4 vShadowMapCoord = mul(worldPosition, shadowMatrix);
	const float2 uv = vShadowMapCoord.xy / max(vShadowMapCoord.w, 0.0001f);

	const float2 edgeDist = min(uv, 1.0f - uv);
	const float borderFade = saturate((min(edgeDist.x, edgeDist.y) - 0.01f) / 0.04f);

	if (viewDepth < shadowRadius && vPosShadowSpace.z > 0.0f && borderFade > 0.001f)
	{
		const float receiverDepth = vShadowMapCoord.z;
		const float shadowTestDepth = receiverDepth - shadowBias;

		const float ign = InterleavedGradientNoise(screenPixelPos);
		const float rotAngle = ign * 6.2831853f;
		const float cosA = cos(rotAngle);
		const float sinA = sin(rotAngle);

		// Step 1: 8-tap rotated multi-ring blocker search (center + inner ring + outer ring)
		static const float2 blockerTaps[8] = {
			float2( 0.0000f,  0.0000f),
			float2( 0.4330f,  0.2500f),
			float2(-0.4330f,  0.2500f),
			float2( 0.0000f, -0.5000f),
			float2( 0.9659f,  0.2588f),
			float2(-0.2588f,  0.9659f),
			float2(-0.9659f, -0.2588f),
			float2( 0.2588f, -0.9659f)
		};

		const float maxFilterTexels = (qualityLevel >= 5.0f) ? 5.2f : 4.2f;
		const float searchRadius = (maxFilterTexels + 0.75f) * invShadowMapSize;

		float blockerSum = 0.0f;
		float blockerCount = 0.0f;
		[unroll] for (int b = 0; b < 8; b++)
		{
			float2 rotated = float2(
				blockerTaps[b].x * cosA - blockerTaps[b].y * sinA,
				blockerTaps[b].x * sinA + blockerTaps[b].y * cosA);
			float sampleDepth = tex2Dlod(s, float4(uv + rotated * searchRadius, 0.0f, 0.0f)).r;
			if (sampleDepth < shadowTestDepth)
			{
				blockerSum += sampleDepth;
				blockerCount += 1.0f;
			}
		}

		// Lossless early-out when completely lit
		if (blockerCount < 0.5f)
		{
			return 1.0f;
		}

		// Step 2: Contact-hardening variable penumbra width (sharp at contact, soft far from caster)
		float avgBlockerDepth = blockerSum / blockerCount;
		float blockerDistance = max(0.0f, receiverDepth - avgBlockerDepth);
		float penumbraRatio = saturate(blockerDistance * 18.0f);
		float filterRadius = lerp(1.05f, maxFilterTexels, penumbraRatio) * invShadowMapSize;

		// Step 3: Golden-Angle Vogel disk PCF
		int pcfCount = (qualityLevel >= 5.0f) ? 16 : 12;
		float fCount = (float)pcfCount;
		float visibility = 0.0f;
		[loop] for (int k = 0; k < 16; k++)
		{
			if (k >= pcfCount) break;
			float r = sqrt(((float)k + 0.5f) / fCount);
			float theta = (float)k * 2.3999632f + rotAngle;
			float2 offset = float2(cos(theta), sin(theta)) * (r * filterRadius);
			float d = tex2Dlod(s, float4(uv + offset, 0.0f, 0.0f)).r;
			visibility += (d >= shadowTestDepth) ? 1.0f : 0.0f;
		}
		visibility /= fCount;

		visibility = lerp(visibility, 1.0f, calculatefadeShadowFactor(viewDepth, shadowRadius));
		return lerp(1.0f, visibility, borderFade);
	}
	return 1.0f;
}


float calculateShadowFactorWithBias(sampler2D s,float4 worldPosition,float viewDepth,float4x4 shadowMatrix, float4x4 shadowViewProjMatrix,float shadowMapSize,float invShadowMapSize,float shadowRadius,float shadowBias)
{
	const float4 vPosShadowSpace = mul(worldPosition, shadowViewProjMatrix);
	const float4 vShadowMapCoord = mul(worldPosition, shadowMatrix);

	if (viewDepth < shadowRadius && vPosShadowSpace.z > 0
		//Avoid computing shadows past the shadow map projection
		&& vShadowMapCoord.x < 1.0f && vShadowMapCoord.x > 0.0f && vShadowMapCoord.y < 1.0f && vShadowMapCoord.y > 0.0f 
		/*// avoid shadow if the center point is not completely in shadow
		&& (vPosShadowSpace.z - tex2D(s, vShadowMapCoord).r) >= SHADOW_BIAS
		*/)
	{
		const float shadow_test_depth = vShadowMapCoord.z - shadowBias;
		const float2 uv = vShadowMapCoord.xy / vShadowMapCoord.w;
#ifdef SOFT_SHADOW_ENABLE
		static const float2 poissonDisk[8] = {
			float2(-0.7071f,  0.7071f),
			float2( 0.0000f,  1.0000f),
			float2( 0.7071f,  0.7071f),
			float2(-1.0000f,  0.0000f),
			float2( 1.0000f,  0.0000f),
			float2(-0.7071f, -0.7071f),
			float2( 0.0000f, -1.0000f),
			float2( 0.7071f, -0.7071f)
		};
		const float filterRadius = 1.5f * invShadowMapSize;
		float ret = getShadowFactor(s, uv, shadow_test_depth, shadowMapSize, invShadowMapSize);
		for (int i = 0; i < 8; i++)
		{
			ret += getShadowFactor(s, uv + poissonDisk[i] * filterRadius, shadow_test_depth, shadowMapSize, invShadowMapSize);
		}
		ret /= 9.0f;
#else
		float ret = getShadowFactor(s, uv, shadow_test_depth, shadowMapSize, invShadowMapSize);
#endif
		ret = lerp(ret, 1.0f, calculatefadeShadowFactor(viewDepth, shadowRadius));
		return ret;
	}
	else
	{
		// not in shadow 
		return 1.f;
	}
}

float calculateShadowFactor(sampler2D s,float4 worldPosition,float viewDepth,float4x4 shadowMatrix, float4x4 shadowViewProjMatrix,float shadowMapSize,float invShadowMapSize,float shadowRadius)
{
	return calculateShadowFactorWithBias(s, worldPosition, viewDepth, shadowMatrix, shadowViewProjMatrix, shadowMapSize, invShadowMapSize, shadowRadius, SHADOW_BIAS);
}

float4 brightPassFilter(sampler2D s,float2 uv)
{
	float Luminance=0.08f;
	const float fMiddleGray=0.18f;
	const float fWhiteCutoff=0.8f;
	float3 ColorOut=tex2D(s,uv);

	ColorOut*=fMiddleGray/(Luminance+0.001f);
	ColorOut*=(1.0f+(ColorOut/(fWhiteCutoff * fWhiteCutoff)));
	ColorOut-=5.0f;

	ColorOut=max(ColorOut, 0.0f);

	ColorOut/=(10.0f+ColorOut);

	return float4(ColorOut, 1.0f);
}

float4 bloorH(sampler2D s,float2 uv,float2 invTexSize)
{
	const int g_cKernelSize=13;

	float2 PixelKernel[g_cKernelSize]=
	{
		{-6, 0},
		{-5, 0},
		{-4, 0},
		{-3, 0},
		{-2, 0},
		{-1, 0},
		{0, 0},
		{1, 0},
		{2, 0},
		{3, 0},
		{4, 0},
		{5, 0},
		{6, 0},
	};
	const float BlurWeights[g_cKernelSize]=
	{
		0.002216,
		0.008764,
		0.026995,
		0.064759,
		0.120985,
		0.176033,
		0.199471,
		0.176033,
		0.120985,
		0.064759,
		0.026995,
		0.008764,
		0.002216,
	};

	float4 Color=0;

	for(int i=0; i < g_cKernelSize; i++)
	{
		Color+=tex2D(s,uv+PixelKernel[i].xy*invTexSize) * BlurWeights[i];
	}

	return Color;
}

float4 bloorV(sampler2D s,float2 uv,float2 invTexSize)
{
	const int g_cKernelSize=13;

	float2 PixelKernel[g_cKernelSize]=
	{
		{0, -6},
		{0, -5},
		{0, -4},
		{0, -3},
		{0, -2},
		{0, -1},
		{0,  0},
		{0,  1},
		{0,  2},
		{0,  3},
		{0,  4},
		{0,  5},
		{0,  6},
	};
	const float BlurWeights[g_cKernelSize]=
	{
		0.002216,
		0.008764,
		0.026995,
		0.064759,
		0.120985,
		0.176033,
		0.199471,
		0.176033,
		0.120985,
		0.064759,
		0.026995,
		0.008764,
		0.002216,
	};

	float4 Color=0;

	for(int i=0; i < g_cKernelSize; i++)
	{
		Color+=tex2D(s,uv+PixelKernel[i].xy*invTexSize) * BlurWeights[i];
	}

	return Color;
}

float3 gammaCorrectRead(float3 rgb)
{
	return pow(rgb,2.2);
}

float3 gammaCorrectWrite(float3 rgb)
{
	return pow(rgb,1.0/2.2);
}
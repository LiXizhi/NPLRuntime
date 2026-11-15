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

//1:no shadow
//0：full shadow
float getShadowFactor(sampler2D s,float2 uv,float testDepth,float shadowMapSize,float invShadowMapSize)
{
#ifdef HARDWARE_SHADOW_ENABLE
	return tex2D(s,float3(uv,testDepth));
#else
#ifdef PCF_SHADOW_ENABLE
	// linear filtering the shadow map using 2*2 nearby texels to remove some aliasing
	const float2 uv_in_texel_f=uv*shadowMapSize;
	const float2 uv_in_texel_i=floor(uv_in_texel_f);
	const float2 scalar=frac(uv_in_texel_f);
	float2 uv0=uv_in_texel_i*invShadowMapSize;
	float2 uv1=(uv_in_texel_i+float2(1,0))*invShadowMapSize;
	float shadow0=tex2D(s,uv0).r>=testDepth?1:0;
	float shadow1=tex2D(s,uv1).r>=testDepth?1:0;
	float shadow_up=lerp(shadow0,shadow1,scalar.x);
	uv0=(uv_in_texel_i+float2(0,1))*invShadowMapSize;
	uv1=(uv_in_texel_i+1)*invShadowMapSize;
	shadow0=tex2D(s,uv0).r>=testDepth?1:0;
	shadow1=tex2D(s,uv1).r>=testDepth?1:0;
	float shadow_down=lerp(shadow0,shadow1,scalar.x);
	return lerp(shadow_up,shadow_down,scalar.y);
#else
	float shadow_depth=tex2D(s,uv).r;
	float shadow=shadow_depth>=testDepth?1:0;;
	return shadow;
#endif
#endif
}

// shadow blending factor, so that the near camera shadow is darker
float calculatefadeShadowFactor(float viewDepth,float shadowRadius)
{
	return saturate((viewDepth-shadowRadius*0.9)/(shadowRadius*0.1));
}


float calculateShadowFactor(sampler2D s,float4 worldPosition,float viewDepth,float4x4 shadowMatrix, float4x4 shadowViewProjMatrix,float shadowMapSize,float invShadowMapSize,float shadowRadius)
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
		const float shadow_test_depth = vShadowMapCoord.z - SHADOW_BIAS;
		const float2 uv = vShadowMapCoord.xy / vShadowMapCoord.w;
#ifdef SOFT_SHADOW_ENABLE
		float ret = 0;
		for (float i = -1.0f; i <= 1.0f; i += 1.0f)
		{
			for (float j = -1.0f; j <= 1.0f; j += 1.0f)
			{
				const float2 texelpos = uv + float2(i*invShadowMapSize, j*invShadowMapSize);
				ret += getShadowFactor(s, texelpos, shadow_test_depth, shadowMapSize, invShadowMapSize);
			}
		}
		ret /= 9;
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
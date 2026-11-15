/**
Author: LiXizhi
Company: ParaEngine 
Date: 2013.10.5
Desc: Deferred shading post processing 
References: 
http://www.gamedev.net/topic/506573-reconstructing-position-from-depth-data/
http://www.crytek.com/cryengine/presentations/real-time-atmospheric-effects-in-games-revisited
CustomBuild:"$(DXSDK_DIR)/Utilities/bin/x86/fxc.exe" /Tfx_2_0 /Gfp /nologo /Fo %(RelativeDir)/%(Filename).fxo %(FullPath) 
OUTPUT:%(RelativeDir)%(Filename).fxo
*/

/** only for debugging purposes*/
// #define DEBUG_MODE_SHOW_DEPTH
float nRowViews = 8;
float nColViews = 12;
float maxViewSeperationDist = 6.0f;
float eyeConvergenceDist = 6.0f; // eye focus
#include "CommonFunction.h"

float4x4 matView;
float4x4 matViewInverse;
float4x4 matProjection;
float4x4 mShadowMapTex;
float4x4 mShadowMapViewProj;
float2 viewportOffset;
float2 viewportScale;

float3 g_FogColor;
float2 g_shadowFactor = float2(0.35, 0.65);
// x is shadow map size(such as 2048), y = 1/x
float2 ShadowMapSize;
// usually 40 meters
float ShadowRadius;

#define SHADOW_BIAS 0.0001
/** define this to sample shadowmap 16 times to obtain soft shadow value. undefine to sample once*/
#define MULTI_SAMPLE_SHADOWMAP
/** whether to desaturate and apply blue tint at dark night. This is usually not necessary since the sun color is already blue at night. */
// #define NIGHT_EYE_EFFECT

/** Make the graphics a little brighter. 
TODO: Disable this macro, when HDR and block level color is implemented.  */
// #define HSV_CORRECTION_TODO_REMOVE_THIS

// x>0 use sun shadow map, y>0 use water reflection. 
float3 RenderOptions;

float2 screenParam;
float ViewAspect;
float TanHalfFOV;
float cameraFarPlane;
float FogStart;
float FogEnd;
float3 cameraPosition;
float3 sunDirection;
float3 SunColor;
float3 TorchLightColor;
float sunIntensity = 1.0; // 1 is noon. 0 is night
float timeMidnight = 0.0; // 1 is midnight.
texture sourceTexture0;
sampler colorSampler : register(s0) = sampler_state
{
    Texture = <sourceTexture0>;
    MinFilter = Linear;
    MagFilter = Linear;
    AddressU = clamp;
    AddressV = clamp;
};

texture sourceTexture1;
sampler matInfoSampler : register(s1) = sampler_state
{
    Texture = <sourceTexture1>;
    MinFilter = Linear;
    MagFilter = Linear;
    AddressU = clamp;
    AddressV = clamp;
};

texture sourceTexture2 : TEXTURE;
sampler ShadowMapSampler : register(s2) = sampler_state
{
    texture = <sourceTexture2>;
    MinFilter = Linear;
    MagFilter = Linear;
    MipFilter = None;
    AddressU = BORDER;
    AddressV = BORDER;
    BorderColor = 0x0;
};

texture sourceTexture3;
sampler depthSampler : register(s3) = sampler_state
{
    Texture = <sourceTexture3>;
    MinFilter = Linear;
    MagFilter = Linear;
    AddressU = clamp;
    AddressV = clamp;
};

texture sourceTexture4;
sampler normalSampler : register(s4) = sampler_state
{
    Texture = <sourceTexture4>;
    MinFilter = Linear;
    MagFilter = Linear;
    AddressU = clamp;
    AddressV = clamp;
};

struct VSOutput
{
    float4 pos : POSITION; // Screen space position
    float2 texCoord : TEXCOORD0; // texture coordinates
    float3 CameraEye : TEXCOORD2; // texture coordinates
};

//Function that retrieves the screen space surface normals. Used for lighting calculations
float4 GetNormal(float2 texCoord)
{
    float4 norm = tex2D(normalSampler, texCoord);
    return float4(norm.rgb * 2.0 - 1.0, norm.w);
}

#ifdef HSV_CORRECTION_TODO_REMOVE_THIS
float3 HUEtoRGB(in float H)
{
	float R = abs(H * 6 - 3) - 1;
	float G = 2 - abs(H * 6 - 2);
	float B = 2 - abs(H * 6 - 4);
	return saturate(float3(R,G,B));
}

float RGBCVtoHUE(in float3 RGB, in float C, in float V)
{
    float3 Delta = (V - RGB) / C;
    Delta.rgb -= Delta.brg;
    Delta.rgb += float3(2,4,6);
    // NOTE 1
    Delta.brg = step(V, RGB) * Delta.brg;
    float H;
    H = max(Delta.r, max(Delta.g, Delta.b));
    return frac(H / 6);
}

float3 HSVtoRGB(in float3 HSV)
{
	float3 RGB = HUEtoRGB(HSV.x);
	return ((RGB - 1) * HSV.y + 1) * HSV.z;
}

float3 RGBtoHSV(in float3 RGB)
{
	float3 HSV = 0;
	HSV.z = max(RGB.r, max(RGB.g, RGB.b));
	float M = min(RGB.r, min(RGB.g, RGB.b));
	float C = HSV.z - M;
	if (C != 0)
	{
		HSV.x = RGBCVtoHUE(RGB, C, HSV.z);
		HSV.y = C / HSV.z;
	}
	return HSV;
}
#endif
//Desaturates any color input at night, simulating the rods in the human eye
// @param amount: How much will the new desaturated and tinted image be mixed with the original image
float3 DoNightEye(float3 color, float amount)
{
    float3 rodColor = float3(0.2, 0.5, 1.0); //Cyan color that humans percieve when viewing extremely low light levels via rod cells in the eye
    float colorDesat = dot(color, float3(1, 1, 1)); //Desaturated color
    return lerp(color, rodColor * colorDesat, amount);
}

VSOutput CompositeQuadVS(float3 iPosition : POSITION,
					float2 texCoord : TEXCOORD0)
{
    VSOutput o;
    o.pos = float4(iPosition, 1);
    o.texCoord = texCoord + 0.5 / screenParam;

	// for reconstructing world position from depth value
    float3 outCameraEye = float3(iPosition.x * TanHalfFOV * ViewAspect, iPosition.y * TanHalfFOV, 1);
    o.CameraEye = outCameraEye;
    return o;
}


float2 convertCameraSpaceToScreenSpace(float3 cameraSpace)
{
    float4 clipSpace = mul(float4(cameraSpace, 1.0), matProjection);
    float2 NDCSpace = clipSpace.xy / clipSpace.w;
    float2 ScreenPos = 0.5 * NDCSpace + 0.5;
    return float2(ScreenPos.x, 1 - ScreenPos.y) * viewportScale + viewportOffset;
}


// compute water reflection by sampling along the reflected eye ray until a pixel is found. 
float4 ComputeRayTraceWaterReflection(float3 cameraSpacePosition, float3 cameraSpaceNormal)
{
    float initialStepAmount = 1;
	//float stepRefinementAmount = 0.1;
	//int maxRefinements = 0;
		 
    float3 cameraSpaceViewDir = normalize(cameraSpacePosition);
    float3 cameraSpaceVector = normalize(reflect(cameraSpaceViewDir, cameraSpaceNormal)) * initialStepAmount;
    float3 oldPosition = cameraSpacePosition;
    float3 cameraSpaceVectorPosition = oldPosition + cameraSpaceVector;
    float2 currentPosition = convertCameraSpaceToScreenSpace(cameraSpaceVectorPosition);
    float4 color = float4(0, 0, 0, 0);
    float2 finalSamplePos = float2(0, 0);
    float ray_length = initialStepAmount;
    int numSteps = 0;
    int max_step = 12; // cameraFarPlane/initialStepAmount; 4 * (1.5^10) = 230
    while (numSteps < max_step &&
		(currentPosition.x > 0 && currentPosition.x < 1 &&
         currentPosition.y > 0 && currentPosition.y < 1))
    {
        float2 samplePos = currentPosition.xy;
        float sampleDepth = tex2Dlod(depthSampler, float4(samplePos, 0, 0)).r;

        float currentDepth = cameraSpaceVectorPosition.z;
        float diff = currentDepth - sampleDepth;
		
        if (diff >= 0 && sampleDepth > 0 && diff <= ray_length)
        {
			// found it, exit the loop
            finalSamplePos.xy = samplePos;
            numSteps = max_step;
        }
        else
        {
            ray_length *= 1.5;
            cameraSpaceVector *= 1.5; //Each step gets bigger
            cameraSpaceVectorPosition += cameraSpaceVector;

            currentPosition = convertCameraSpaceToScreenSpace(cameraSpaceVectorPosition);
        }
        numSteps++;
    }
	
    if (finalSamplePos.x != 0 && finalSamplePos.y != 0)
    {
		// compute point color
        float2 texCoord = finalSamplePos.xy;
        color = tex2D(colorSampler, texCoord);
        color.a = 1;
		// r:category id,  g: sun light value, b: torch light value
        float4 block_info = tex2D(matInfoSampler, texCoord);
        int category_id = (int) (block_info.r * 255.0 + 0.4);
        float sun_light_strength = block_info.g * sunIntensity;
        float torch_light_strength = block_info.b;
		
		
		// use a simple way to render lighting for reflection to avoid another water rendering pass. 
        float shadow = 1;
        if (sun_light_strength > 0)
        {
			// get world space normal
            float3 normal = GetNormal(texCoord);
            float NdotL = dot(sunDirection, normal);

            float directSunLight = max(0, NdotL);
            if (directSunLight > 0)
                shadow = directSunLight;
            else
                shadow = 0;
        }

		//if(category_id == 255)
		//{
		//	// mesh object
		//	color.xyz *= shadow * g_shadowFactor.x + g_shadowFactor.y;
		//}
		//else
		{
			// other blocks
            float3 sun_light = color.xyz * SunColor.rgb * sun_light_strength;
            sun_light *= shadow * g_shadowFactor.x + g_shadowFactor.y;
            float3 torch_light = color.xyz * TorchLightColor.rgb * torch_light_strength;
			// compose and interpolate so that the strength of light is almost linear 
            color.xyz = lerp(torch_light.xyz + sun_light.xyz, sun_light.xyz, sun_light_strength / (torch_light_strength + sun_light_strength + 0.001));
        }
        color.a *= clamp(1 - pow(distance(float2(0.5, 0.5), finalSamplePos.xy) * 2.0, 2.0), 0.0, 1.0);
    }
    return color;
}

//Calculates direct sunlight without visibility check. mainly depends on surface normal. 
float CalculateDirectLighting(float3 normal, int category_id)
{
    float NdotL = dot(sunDirection, normal);
    float directSunLight;
    if (category_id == 31)
    {
		// grass 
        directSunLight = 0.9;
    }
    else if (category_id == 18)
    {
		// leaves
        if (NdotL > -0.01)
            directSunLight = max(0, NdotL);
        else
            directSunLight = abs(NdotL) * 0.25;
    }
    else
    {
		// default sun light 
        directSunLight = max(0, NdotL);
    }
    return directSunLight;
}

// compute sun shading
// @return shadow: 1 is no shadow, 0 is full shadow(dark)
float ComputeSunShading(float directSunLight, float sun_light_strength, float4 vWorldPosition, float depth)
{
	// 1 is no shadow, 0 is full shadow(dark)
    float shadow = 1;

	// only apply global sun shadows when there is enough sun light on the material
    if (RenderOptions.x > 0 && sun_light_strength > 0)
    {
        if (directSunLight > 0)
        {
            shadow = calculateShadowFactor(ShadowMapSampler, vWorldPosition, depth, mShadowMapTex, mShadowMapViewProj, ShadowMapSize.x, ShadowMapSize.y, ShadowRadius);
			// this final step is important to obtain the final sun's shading
            shadow = directSunLight * shadow;
        }
        else
        {
            shadow = 0;
        }
    }
    return shadow;
}

#ifdef 	DEBUG_MODE_SHOW_DEPTH
float4 DebugMode_ShowDepthPS(VSOutput input) :COLOR
{
	float2 texCoord = input.texCoord;
	float4 color = tex2D(colorSampler, texCoord);
	// get world space normal
	float3 normal = GetNormal(texCoord);

	// screen space depth value. 
	float depth = tex2D(depthSampler, texCoord).x / 40.f;
	color.xyzw = float4(depth, depth, depth, 1);
	return color;
}
#endif


float2 ComputeSuperMultiViewPos(float2 texCoord)
{
    float nCol = floor(texCoord.x * nColViews);
    float nRow = floor(texCoord.y * nColViews);
    texCoord.x = fmod(texCoord.x, 1.0f / nColViews) / (1.0f / nColViews);
    texCoord.y = fmod(texCoord.y, 1.0f / nRowViews) / (1.0f / nRowViews);
    float viewOffsetX = ((nRow * nColViews + nCol) / (nColViews * nRowViews) - 0.5) * maxViewSeperationDist;
    float3 eyePosition = float3(viewOffsetX, 0, 0);
    // TODO: need to multiple aspect ratio, here just assume 90 degrees for testing
    float3 cameraSpaceVector = normalize(float3((texCoord.x - 0.5) * eyeConvergenceDist, -(texCoord.y - 0.5) * eyeConvergenceDist, eyeConvergenceDist) - eyePosition);
    
    float initialStepAmount = 1;
    cameraSpaceVector *= initialStepAmount;
    float3 cameraSpaceVectorPosition = eyePosition + cameraSpaceVector;
    float2 currentPosition = convertCameraSpaceToScreenSpace(cameraSpaceVectorPosition);
    float4 color = float4(0, 0, 0, 0);
    float2 finalSamplePos = float2(0, 0);
    float ray_length = initialStepAmount;
    int numSteps = 0;
    int max_step = 12; // cameraFarPlane/initialStepAmount; 4 * (1.5^10) = 230
    while (numSteps < max_step &&
		(currentPosition.x > 0 && currentPosition.x < 1 &&
         currentPosition.y > 0 && currentPosition.y < 1))
    {
        float2 samplePos = currentPosition.xy;
        float sampleDepth = tex2Dlod(depthSampler, float4(samplePos, 0, 0)).r;

        float currentDepth = cameraSpaceVectorPosition.z;
        float diff = currentDepth - sampleDepth;
		
        if (diff >= 0 && sampleDepth > 0 && diff <= ray_length)
        {
			// found it, exit the loop
            finalSamplePos.xy = samplePos;
            numSteps = max_step;
        }
        else
        {
            ray_length *= 1.2;
            cameraSpaceVector *= 1.2; //Each step gets bigger
            cameraSpaceVectorPosition += cameraSpaceVector;

            currentPosition = convertCameraSpaceToScreenSpace(cameraSpaceVectorPosition);
        }
        numSteps++;
    }
    texCoord = finalSamplePos;
    return texCoord;
}

float4 Composite1PS(VSOutput input) : COLOR
{
    float2 texCoord = ComputeSuperMultiViewPos(input.texCoord);
    if (texCoord.x == 0 && texCoord.y == 0)
    {
        return float4(0, 0, 0, 1.0f);
    }
   
    float4 color = tex2D(colorSampler, texCoord);

#ifdef HSV_CORRECTION_TODO_REMOVE_THIS	
	float3 hsv = RGBtoHSV(color.rgb);
	hsv.y *= 1.3;
	color.rgb = HSVtoRGB(hsv);
#endif

	// r:category id,  g: sun light value, b: torch light value
    float4 block_info = tex2D(matInfoSampler, texCoord);
    int category_id = (int) (block_info.r * 255.0 + 0.4);
	
    float sun_light_strength = block_info.g * sunIntensity;
	
    float torch_light_strength = block_info.b;
	// get world space normal
    float4 normal_ = GetNormal(texCoord);
    float3 normal = normal_.xyz;
    float specular = 1.0 - normal_.w;
	// screen space depth value. 
    float depth = tex2D(depthSampler, texCoord).x;

	// Calculates direct sunlight without visibility check
    float directSunLight = CalculateDirectLighting(normal, category_id);
	// also give torch light some fake shading to make it look more dimentional at night. 
    if (category_id == 255)
        torch_light_strength *= (directSunLight * 0.3 + 0.7);
	
    if (depth > 0.01)
    {
		// reconstruct world space vector from depth
        float3 cameraSpacePosition = input.CameraEye * depth;
        float4 vWorldPosition = float4(cameraSpacePosition, 1);
        vWorldPosition = mul(vWorldPosition, matViewInverse);

		// 1 is no shadow, 0 is full shadow(dark)
        float shadow = ComputeSunShading(directSunLight, sun_light_strength, vWorldPosition, depth);
		
		
		// other blocks
        float3 sun_light = color.xyz * SunColor.rgb * sun_light_strength;

#ifdef NIGHT_EYE_EFFECT
		// map from [-1, 1] range to [0,0.8]. 
		sun_light = DoNightEye(sun_light, timeMidnight*0.8);
#endif
        sun_light *= shadow * g_shadowFactor.x + g_shadowFactor.y;
        float3 torch_light = color.xyz * TorchLightColor.rgb * torch_light_strength;

		// compose and interpolate so that the strength of light is almost linear 
        color.xyz = lerp(torch_light.xyz + sun_light.xyz, sun_light.xyz, sun_light_strength / (torch_light_strength + sun_light_strength + 0.001));
		
		// CalculateSpecularHighlight
        if (category_id == 50 || specular > 0.01)
        {
            float3 cameraSpaceViewDir = normalize(cameraSpacePosition);
            float3 cameraSpaceNormal = mul(normal, (float3x3) matView);
			// For fake specular light, we will assume light and eye are on the same point, so half vector is actually -viewDir.
            float viewVector = dot(-cameraSpaceViewDir, cameraSpaceNormal);
            if (viewVector > 0)
            {
                float spec = pow(viewVector, 60.0) * specular;
                float3 fakeSpecularColor = (TorchLightColor.rgb * torch_light_strength + SunColor * sun_light_strength);
                color.xyz = lerp(color.xyz, fakeSpecularColor, spec);
            }
        }

        if ((category_id == 8 || category_id == 9) && RenderOptions.y > 0)
        {
			// water blocks
            float3 cameraSpaceNormal = mul(normal, (float3x3) matView);
            float4 reflection = ComputeRayTraceWaterReflection(cameraSpacePosition, cameraSpaceNormal);

#define USE_WATER_REFLECTION_FRESNEL_EFFECT
#ifdef USE_WATER_REFLECTION_FRESNEL_EFFECT
            float3 cameraSpaceViewDir = -normalize(cameraSpacePosition);
            const float fresnelPower = 3.0;
            float fresnel = pow(saturate(1.0 - dot(cameraSpaceViewDir, cameraSpaceNormal)), fresnelPower) * 0.98 + 0.02;
            reflection.a *= fresnel;
#endif
            color.xyz = lerp(color.xyz, reflection.rgb, reflection.a);
        }

        float eyeDist = length(cameraSpacePosition);
        if (FogStart < FogEnd)
            color.xyz = lerp(color.xyz, g_FogColor.xyz, 1.0 - clamp((FogEnd - eyeDist) / (FogEnd - FogStart), 0.0, 1.0));
    }
		
	// Put color into gamma space for correct display
	// color.rgb = pow(color.rgb, (1.0f / 2.2f)); 
    return float4(color.rgb, 1.0);
}

// without water reflection
float4 CompositeLitePS(VSOutput input) : COLOR
{
    float2 texCoord = input.texCoord;
    float4 color = tex2D(colorSampler, texCoord);
	
	// r:category id,  g: sun light value, b: torch light value
    float4 block_info = tex2D(matInfoSampler, texCoord);
    int category_id = (int) (block_info.r * 255.0 + 0.4);
	
    float sun_light_strength = block_info.g * sunIntensity;
    float torch_light_strength = block_info.b;
	// get world space normal
    float3 normal = GetNormal(texCoord);
	// screen space depth value. 
    float depth = tex2D(depthSampler, texCoord).x;

	// Calculates direct sunlight without visibility check
    float directSunLight = CalculateDirectLighting(normal, category_id);
	// also give torch light some fake shading to make custom model look more dimentional at night. 
    if (category_id == 255)
        torch_light_strength *= (directSunLight * 0.3 + 0.7);
	
    if (depth > 0.01)
    {
		// reconstruct world space vector from depth
        float3 cameraSpacePosition = input.CameraEye * depth;
        float4 vWorldPosition = float4(cameraSpacePosition, 1);
        vWorldPosition = mul(vWorldPosition, matViewInverse);

		// 1 is no shadow, 0 is full shadow(dark)
        float shadow = ComputeSunShading(directSunLight, sun_light_strength, vWorldPosition, depth);

		// other blocks
        float3 sun_light = color.xyz * SunColor.rgb * sun_light_strength;

        sun_light *= shadow * g_shadowFactor.x + g_shadowFactor.y;
        float3 torch_light = color.xyz * TorchLightColor.rgb * torch_light_strength;
		
		// compose and interpolate so that the strength of light is almost linear 
        color.xyz = lerp(torch_light.xyz + sun_light.xyz, sun_light.xyz, sun_light_strength / (torch_light_strength + sun_light_strength + 0.001));

        float eyeDist = length(cameraSpacePosition);
        if (FogStart < FogEnd)
            color.xyz = lerp(color.xyz, g_FogColor.xyz, 1.0 - clamp((FogEnd - eyeDist) / (FogEnd - FogStart), 0.0, 1.0));
    }
    return color;
}

float4 CompositeFXAA(VSOutput input) : COLOR
{
    return FxaaPixelShader(
		input.texCoord, // FxaaFloat2 pos,
		FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f), // FxaaFloat4 fxaaConsolePosPos,
		colorSampler, // FxaaTex tex,
		colorSampler, // FxaaTex fxaaConsole360TexExpBiasNegOne,
		colorSampler, // FxaaTex fxaaConsole360TexExpBiasNegTwo,
		1.0 / screenParam, // FxaaFloat2 fxaaQualityRcpFrame,
		FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f), // FxaaFloat4 fxaaConsoleRcpFrameOpt,
		FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f), // FxaaFloat4 fxaaConsoleRcpFrameOpt2,
		FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f), // FxaaFloat4 fxaaConsole360RcpFrameOpt2,
		0.75f, // FxaaFloat fxaaQualitySubpix,
		0.166f, // FxaaFloat fxaaQualityEdgeThreshold,
		0.0833f, // FxaaFloat fxaaQualityEdgeThresholdMin,
		0.0f, // FxaaFloat fxaaConsoleEdgeSharpness,
		0.0f, // FxaaFloat fxaaConsoleEdgeThreshold,
		0.0f, // FxaaFloat fxaaConsoleEdgeThresholdMin,
		FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f) // FxaaFloat fxaaConsole360ConstDir,
		);
}

technique Default_Normal
{
    pass P0
    {
        cullmode = none;
        ZEnable = false;
        ZWriteEnable = false;
        FogEnable = False;
        VertexShader = compile vs_3_0 CompositeQuadVS();
#ifdef 	DEBUG_MODE_SHOW_DEPTH
		PixelShader = compile ps_3_0 DebugMode_ShowDepthPS();
#else
        PixelShader = compile ps_3_0 Composite1PS();
#endif
    }
    pass P1
    {
        cullmode = none;
        ZEnable = false;
        ZWriteEnable = false;
        FogEnable = False;
        AlphaBlendEnable = false;
        VertexShader = compile vs_3_0 CompositeQuadVS();
        PixelShader = compile ps_3_0 CompositeFXAA();
    }
}

technique Default_Lite
{
    pass P0
    {
        cullmode = none;
        ZEnable = false;
        ZWriteEnable = false;
        FogEnable = False;
        VertexShader = compile vs_3_0 CompositeQuadVS();
        PixelShader = compile ps_3_0 CompositeLitePS();
    }
}
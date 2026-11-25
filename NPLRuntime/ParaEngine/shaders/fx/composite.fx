/**
Author: LiXizhi
Company: ParaEngine
Date: 2015.5.10
Desc: Deferred shading post processing: 4 passes as below
pass0 (lightmap + shadow + HDR + nighteye + sunspot + torch glow) + (TODO:ssao, underwater, specularity)
pass1 (reflection) + (TODO:sunspot in water)
pass2 (bloom textures) + (TODO:average luminance)
final (DepthOfView + Bloom + RainFog + Vignette + ToneMap)  + (TODO: motionBlur + luminanceToneMap)

References:
http://www.gamedev.net/topic/506573-reconstructing-position-from-depth-data/
http://www.crytek.com/cryengine/presentations/real-time-atmospheric-effects-in-games-revisited
SUES v10.1: Sonic Ether's unbelievable shader
CustomBuild:"$(DXSDK_DIR)/Utilities/bin/x86/fxc.exe" /Tfx_2_0 /Gfp /nologo /Fo %(RelativeDir)/%(Filename).fxo %(FullPath)
OUTPUT:%(RelativeDir)%(Filename).fxo
*/
/** whether to enable debug view. lt:bloom textures, lr:bloom sum, lb:original, rb:final. */
// #define SHOW_DEBUG_VIEW

#include "CommonFunction.h"

float4x4 matView;
float4x4 matViewInverse;
float4x4 matProjection;
float4x4 mShadowMapTex;
float4x4 mShadowMapViewProj;
float2	viewportOffset;
float2	viewportScale;

float3   g_FogColor;

// x is shadow map size(such as 2048), y = 1/x
float2  ShadowMapSize;
// usually 40 meters
float	ShadowRadius; 

/** whether to desaturate and apply blue tint at dark night. */
#define NIGHT_EYE_EFFECT

// x>0 use sun shadow map, y>0 use water reflection, z>=1 bloom, z>=2 depth of view. 
float3 RenderOptions;


float2 screenParam;
float centerDepthSmooth = 15.f;
// between [0,1]
float rainStrength = 0.f;
// Display-referred photographic controls: warmth, saturation offset, lifted blacks and split tone.
float4 PhotoGrade = float4(0, 0, 0, 0);
float EyeBrightness = 0.2f;
float EyeContrast = 0.5f;
float BloomEnabled = 1.0f;
// Ambient occlusion, contact shadows, light shafts and bloom intensity.
float4 LightingStrength = float4(1, 1, 1, 1);
float4 BlockSkyVolume;
float BlockReflectionEnabled = 0;
float QualityLevel = 4.0f;
float SkyVisibilityView = 0.0f;
#ifdef BLOCK_SURFACE_EFFECTS
float BevelEnabled = 0.0f;
float ZigZagEnabled = 0.0f;
float4 BlockGridOrigin = float4(0.0f, 0.0f, 0.0f, 0.0f);
#endif
float BounceEnabled = 0.0f;
float MaxDOFBlur = 0.00045f;
float ViewAspect;
float TanHalfFOV;
float cameraFarPlane;
float3 cameraPosition;
float3 sunDirection;
float3 SunColor;
float3 TorchLightColor;
float timeNoon = 1.0; // 1 is noon. 0 is night
float timeMidnight = 0.0; // 1 is midnight.
float DepthOfViewFactor = 0.00045f;	//aperture - bigger values for shallower depth of field
float FogStart = 100.0;
float FogEnd = 140.0;
float CloudThickness = 0.0;

static const float bloom_threshold = 0.7;
// offset_x, offset_y, scale_factor
static const float3 offsets[16] = {
	float3(0.008, 0.0, 1.0), float3(0.006, 0.0, 1.2), float3(0.004, 0.0, 1.3), float3(0.002, 0.0, 1.5),
	float3(0.0, 0.008, 1.0), float3(0.0, 0.006, 1.2), float3(0.0, 0.004, 1.3), float3(0.0, 0.002, 1.5),
	-float3(0.008, 0.0, 1.0), -float3(0.006, 0.0, 1.2), -float3(0.004, 0.0, 1.3), -float3(0.002, 0.0, 1.5),
	-float3(0.0, 0.008, 1.0), -float3(0.0, 0.006, 1.2), -float3(0.0, 0.004, 1.3), -float3(0.0, 0.002, 1.5)
};

texture sourceTexture0;
sampler colorSampler:register(s0) = sampler_state
{
	Texture = <sourceTexture0>;
	MinFilter = Linear;
	MagFilter = Linear;
	AddressU = clamp;
	AddressV = clamp;
};

texture sourceTexture1;
sampler matInfoSampler:register(s1) = sampler_state
{
	Texture = <sourceTexture1>;
	MinFilter = Linear;
	MagFilter = Linear;
	AddressU = clamp;
	AddressV = clamp;
};

texture sourceTexture2 : TEXTURE;
sampler ShadowMapSampler: register(s2) = sampler_state
{
	texture = <sourceTexture2>;
	MinFilter = Point;
	MagFilter = Point;
	MipFilter = None;
	AddressU = clamp;
	AddressV = clamp;
};

texture sourceTexture3;
sampler depthSampler:register(s3) = sampler_state
{
	Texture = <sourceTexture3>;
	MinFilter = Linear;
	MagFilter = Linear;
	AddressU = clamp;
	AddressV = clamp;
};

texture sourceTexture4;
sampler normalSampler:register(s4) = sampler_state
{
	Texture = <sourceTexture4>;
	MinFilter = Linear;
	MagFilter = Linear;
	AddressU = clamp;
	AddressV = clamp;
};
texture sourceTexture7;
sampler blockReflectionSampler:register(s7) = sampler_state
{
	Texture = <sourceTexture7>;
	MinFilter = Point;
	MagFilter = Point;
	AddressU = clamp;
	AddressV = clamp;
};
texture sourceTexture6;
sampler blockSkySampler:register(s6) = sampler_state
{
	Texture = <sourceTexture6>;
	MinFilter = Linear;
	MagFilter = Linear;
	AddressU = clamp;
	AddressV = clamp;
};
texture sourceTexture5;
sampler compositeSampler:register(s5) = sampler_state
{
	Texture = <sourceTexture5>;
	MinFilter = Linear;
	MagFilter = Linear;
	AddressU = clamp;
	AddressV = clamp;
};
struct VSOutput
{
	float4 pos			: POSITION;         // Screen space position
	float2 texCoord		: TEXCOORD0;        // texture coordinates
	float3 CameraEye		: TEXCOORD2;      // texture coordinates
};

//All sky shading attributes
struct SkyStruct {
	float3 	albedo;				//Diffuse texture aka "color texture" of the sky
	float3 sunSpot;
	float sunProximity;
};

// Surface shading properties
struct SurfaceStruct
{
	int category_id;
	float3 	albedo;					// Diffuse texture "color texture" in linear color space (gamma decoded)
	float3 	normal;					// Screen-space surface normals
	float3	cameraSpacePos;
	float3	CameraEye;
	float2	texCoord;
	float 	depth;					// scene depth
	float 	NdotL; 					// dot(normal, lightVector). used for direct lighting calculation
	float 	shadow;
	SkyStruct sky;
	bool mask_sky;
	bool mask_water;
	bool mask_torch;
};

// Surface shading properties
struct SurfaceStruct2
{
	int category_id;
	float3 	color;					// Diffuse texture "color texture" in linear color space (gamma decoded)
	float	sunlightVisibility;
	float	torch_light_strength;
	float3 	normal;					// Screen-space surface normals
	float3	cameraSpacePos;
	float3	CameraEye;
	float2	texCoord;
	float 	depth;					// scene depth
	float 	NdotL; 					// dot(normal, lightVector). used for direct lighting calculation
	float 	shadow;
	float	specularity;
	bool mask_sky;
	bool mask_water;
	bool mask_metal;
};

// Lightmaps directly from C++ engine
struct RawLightmapStruct
{
	float torch;				//Light emitted from torches and other emissive blocks
	float sky;					//Light coming from the sky
};

//Lighting information to light the scene. These are untextured colored lightmaps to be multiplied with albedo to get the final lit and textured image.
struct LightmapStruct
{
	float3 sunlight;				//Direct light from the sun
	float3 skylight;				//Ambient light from the sky
	float3 bouncedSunlight;		//Fake bounced light, coming from opposite of sun direction and adding to ambient light
	float3 scatteredSunlight;		//Fake scattered sunlight, coming from same direction as sun and adding to ambient light
	float3 torchlight;			//Light emitted from torches and other emissive blocks
	float3 nolight;				//Base ambient light added to everything. For lighting caves so that the player can barely see even when no lights are present
	float3 sky;					//Color and brightness of the sky itself
};

// Result of shading calculation variables
struct ShadingStruct
{
	float   direct;
	float 	bounced; 			//Fake bounced sunlight
	float 	skylight; 			//Light coming from sky
	float 	scattered; 			//Fake scattered sunlight
	float 	sunlightVisibility; //Shadows
};

//Final textured and lit images sorted by what is illuminating them.
struct FinalStruct {
	float3 sunlight;				//Direct light from the sun
	float3 skylight;				//Ambient light from the sky
	float3 torchlight;			//Light emitted from torches and other emissive blocks
	float3 nolight;				//Base ambient light added to everything. For lighting caves so that the player can barely see even when no lights are present
	float3 sky;					//Color and brightness of the sky itself
	float3 glow_torch;
};

struct PSOut
{
	// HDR color
	float4 Color	: COLOR0;
	// 32bits additional information. 
	half4 Color2	: COLOR1;
};

VSOutput CompositeQuadVS(float3 iPosition:POSITION,
	float2 texCoord : TEXCOORD0)
{
	VSOutput o;
	o.pos = float4(iPosition, 1);
	o.texCoord = texCoord + 0.5 / screenParam;

	// for reconstructing world position from depth value
	float3 outCameraEye = float3(iPosition.x*TanHalfFOV*ViewAspect, iPosition.y*TanHalfFOV, 1);
	o.CameraEye = outCameraEye;

	return o;
}

//Function that retrieves the diffuse texture and convert it into linear space.
float3  GetAlbedoLinear(float2 texCoord)
{
	// decode gama correction, so that we work in original linear space
	return pow(tex2D(colorSampler, texCoord).rgb, 2.2f);
}

//Desaturates any color input at night, simulating the rods in the human eye
// @param amount: How much will the new desaturated and tinted image be mixed with the original image
void DoNightEye(inout float3 color, float amount)
{
	float3 rodColor = float3(0.2, 0.5, 1.0); 	//Cyan color that humans percieve when viewing extremely low light levels via rod cells in the eye
	float colorDesat = dot(color, float3(1, 1, 1)); 	//Desaturated color
	color = lerp(color, rodColor*colorDesat, amount);
}

float2 convertCameraSpaceToScreenSpace(float3 cameraSpace)
{
	float4 clipSpace = mul(float4(cameraSpace, 1.0), matProjection);
	float2 NDCSpace = clipSpace.xy / clipSpace.w;
	float2 ScreenPos = 0.5 * NDCSpace + 0.5;
	return float2(ScreenPos.x, 1 - ScreenPos.y) * viewportScale + viewportOffset;
}


// compute water reflection by sampling along the reflected eye ray until a pixel is found. 
float4 	ComputeRayTraceReflection(float3 cameraSpacePosition, float3 cameraSpaceNormal)
{
	float initialStepAmount = QualityLevel >= 9 ? 0.25 : 1.0;
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
	int max_step = QualityLevel >= 9 ? 64 : (QualityLevel >= 6 ? 48 : (QualityLevel >= 5 ? 24 : 12));
	float growth = QualityLevel >= 5 ? 1.15 : 1.5;
	while (numSteps < max_step &&
		cameraSpaceVectorPosition.z > 0.01 &&
		all(currentPosition > viewportOffset) && all(currentPosition < viewportOffset + viewportScale))
	{
		float2 samplePos = currentPosition.xy;
		float sampleDepth = tex2Dlod(depthSampler, float4(samplePos, 0, 0)).r;

		float currentDepth = cameraSpaceVectorPosition.z;
		float diff = currentDepth - sampleDepth;

		if (diff >= 0 && sampleDepth > 0 && diff <= ray_length)
		{
			// found it, exit the loop
			finalSamplePos.xy = samplePos;
			if (QualityLevel >= 5) {
				float3 lowerPosition = cameraSpaceVectorPosition - cameraSpaceVector;
				float3 upperPosition = cameraSpaceVectorPosition;
				int refinements = QualityLevel >= 9 ? 6 : (QualityLevel >= 6 ? 4 : 2);
				[loop] for (int refinement = 0; refinement < refinements; refinement++) {
					float3 midpoint = (lowerPosition + upperPosition) * 0.5;
					float2 coord = convertCameraSpaceToScreenSpace(midpoint);
					if (any(coord <= viewportOffset) || any(coord >= viewportOffset + viewportScale)) break;
					float depth = tex2Dlod(depthSampler, float4(coord, 0, 0)).r;
					if (depth > 0.01 && midpoint.z >= depth) upperPosition = midpoint;
					else lowerPosition = midpoint;
				}
				finalSamplePos = convertCameraSpaceToScreenSpace(upperPosition);
				float hitDepth = tex2Dlod(depthSampler, float4(finalSamplePos, 0, 0)).r;
				if (hitDepth <= 0.01 || upperPosition.z - hitDepth > max(0.15, hitDepth * 0.01)) finalSamplePos = 0;
			}
			numSteps = max_step;
		}
		else
		{
			ray_length *= growth;
			cameraSpaceVector *= growth;
			cameraSpaceVectorPosition += cameraSpaceVector;

			currentPosition = convertCameraSpaceToScreenSpace(cameraSpaceVectorPosition);
		}
		numSteps++;
	}

	if (finalSamplePos.x != 0 && finalSamplePos.y != 0)
	{
		// compute point color
		float2 texCoord = finalSamplePos.xy;
		color.rgb = GetAlbedoLinear(texCoord);
		float2 localUV = (finalSamplePos - viewportOffset) / viewportScale;
		color.a = clamp(1 - pow(distance(float2(0.5, 0.5), localUV)*2.0, 2.0), 0.0, 1.0);
	}
	return color;
}

float CalculateSkylight(in SurfaceStruct surface)
{
	if (surface.category_id == 31)
	{
		// grass 
		return 1.0f;
	}
	else
	{
		const float3 upVector = float3(0, 1.0, 0);
		float skylight = dot(surface.normal, upVector);
		skylight = skylight * 0.4f + 0.6f;
		return skylight;
	}
}

//Calculates direct sunlight without visibility check. mainly depends on surface normal. 
float 	CalculateDirectLighting(in SurfaceStruct surface)
{
	if (surface.category_id == 31)
	{
		// grass 
		return 1.0f;
	}
	else if (surface.category_id == 18)
	{
		// leaves
		// if(NdotL > -0.01)
		// 	return max(0, NdotL);
		// else
		// 	return abs(NdotL) * 0.25;
		return 1.0f;
	}
	else if (surface.category_id == 255)
	{
		// Gentle wrap diffuse for characters/models so faces and curved surfaces shade smoothly
		const float wrap = 0.25f;
		return saturate((surface.NdotL + wrap) / (1.0f + wrap));
	}
	else
	{
		// default sun light 
		return max(0, surface.NdotL*0.99 + 0.01);
	}
}

// @return [0,1]: 1 is no shadow(fully visible), 0 is completely in shadow(dark, unvisible)
float 	CalculateSunlightVisibility(inout SurfaceStruct surface, in ShadingStruct shading)
{
	if (RenderOptions.x < 0.99f || rainStrength >= 0.99f) {
		// no shadow when raining
		surface.shadow = 1.0; 
		return 1.0f;
	}
	
	// only apply global sun/moon shadows when there is direct celestial light on the material
	if (shading.direct > 0.0f)
	{
		// reconstruct world space vector
		float4 vWorldPosition = float4(surface.cameraSpacePos, 1);
		vWorldPosition = mul(vWorldPosition, matViewInverse);

		// 3D World-Space Normal-Offset Bias + tight slope depth bias (eliminates wall acne & peter-panning)
		float worldTexel = (ShadowRadius * 2.0f) * ShadowMapSize.y;
		float sinTheta = sqrt(saturate(1.0f - surface.NdotL * surface.NdotL));
		float normalOffsetScale = (surface.category_id == 255) ? (0.18f + 0.35f * sinTheta) : (0.35f + 0.75f * sinTheta);
		float4 biasedWorldPos = float4(vWorldPosition.xyz + surface.normal * (worldTexel * normalOffsetScale), 1.0f);
		float shadowBias = lerp(0.0012f, 0.00035f, saturate(surface.NdotL));
		float2 screenPixelPos = surface.texCoord * screenParam;

		surface.shadow = calculatePCSSShadowFactor(
			ShadowMapSampler, biasedWorldPos, surface.depth,
			mShadowMapTex, mShadowMapViewProj,
			ShadowMapSize.x, ShadowMapSize.y, ShadowRadius,
			shadowBias, screenPixelPos, QualityLevel);
		return surface.shadow;
	}
	else
	{
		surface.shadow = 0.0;
		return 0.0f;
	}
}

// Function that retrieves the lightmap of light emitted by emissive blocks like torches
// Apply inverse square law and normalize for natural light falloff
// return value is also [0, 1] but applied inverse square law. 
// Note: the precomputed lightmap is fine. if you want to change lightmap of torch yourself, call 
// ParaTerrain.GetBlockAttributeObject():SetField("UseLinearTorchBrightness", shader_method >=3);
float GetLightmapTorch(float lightmap)
{
	lightmap = 1.0f - lightmap;
	lightmap = pow(lightmap, 2.0f);
	lightmap = 1.0f / pow((lightmap * 6 + 0.2f), 2.0f);
	lightmap -= 0.0260; // = 1.0f / pow((1 * 6 + 0.2f), 2.0f);
	lightmap = max(0.0f, lightmap);
	lightmap *= 0.04f;
	return lightmap;
}

//Function that retrieves the lightmap of light emitted by the sky. This is a raw value from 0 (fully dark) to 1 (fully lit) regardless of time of day
float 	GetLightmapSky(float skylight) {
	return pow(skylight, 4.3f);
}

//Function that retrieves the screen space surface normals. Used for lighting calculations
float4  GetNormal(float2 texCoord) {
	float4 norm = tex2D(normalSampler, texCoord);
	return float4(norm.rgb * 2.0 - 1.0, norm.w);
}

/* linear depth in camera space. [0, cameraFarPlane] */
float  GetDepth(float2 texCoord) {
	return tex2D(depthSampler, texCoord).x;
}

float GetSunlightVisibility(float2 coord)
{
	return tex2D(ShadowMapSampler, coord).g;
}

// @param vec2Seed: normally this is texture coordinate.
float noise(float offset, float2 vec2Seed)
{
	float2 coord = vec2Seed + offset.xx;
	float noise = clamp(frac(sin(dot(coord, float2(12.9898f, 78.233f))) * 43758.5453f), 0.0f, 1.0f)*2.0f - 1.0f;
	return noise;
}

//Calculates direct sunlight without visibility check
void CalculateNdotL(inout SurfaceStruct surface)
{
	surface.NdotL = dot(sunDirection, surface.normal);
}

//Mask
void 	CalculateMasks(inout SurfaceStruct surface)
{
	surface.mask_sky = surface.depth < 0.01;
	surface.mask_water = (surface.category_id == 8 || surface.category_id == 9);
	surface.mask_torch = (surface.category_id == 5);
}

// Renders daytime solar disc + multi-layer Mie corona and nighttime textured lunar disc + silver halo
void ComputeCelestialSky(inout SurfaceStruct surface)
{
	if (!surface.mask_sky)
	{
		surface.sky.sunSpot = float3(0.0f, 0.0f, 0.0f);
		surface.sky.sunProximity = 0.0f;
		return;
	}

	float3 viewRayVS = normalize(surface.CameraEye);
	float3 sunDirWS = normalize(sunDirection);
	float3 sunDirVS = normalize(mul(sunDirWS, (float3x3)matView));
	float cosTheta = dot(viewRayVS, sunDirVS);
	surface.sky.sunProximity = saturate(cosTheta);

	float clearSky = (1.0f - rainStrength) * (1.0f - CloudThickness * 0.85f);
	float dayVis = saturate(1.0f - timeMidnight * 1.4f) * clearSky;
	float moonVis = smoothstep(0.05f, 0.55f, timeMidnight) * clearSky;

	float3 celestialGlow = float3(0.0f, 0.0f, 0.0f);

	// 1. Daytime Sun: crisp solar disc + golden-hour Rayleigh/Mie corona
	if (dayVis > 0.001f)
	{
		float horizonFactor = 1.0f - smoothstep(0.05f, 0.45f, saturate(sunDirWS.y));
		float3 solarTint = lerp(float3(1.0f, 0.96f, 0.90f), float3(1.32f, 0.72f, 0.36f), horizonFactor);

		float solarDisc = smoothstep(0.99915f, 0.99945f, cosTheta);
		float limb = lerp(0.75f, 1.25f, smoothstep(0.99915f, 0.99995f, cosTheta));
		celestialGlow += SunColor.rgb * solarTint * (solarDisc * limb * 48.0f * dayVis);

		float innerCorona = pow(saturate(cosTheta), 160.0f) * 3.6f;
		float midCorona   = pow(saturate(cosTheta), 28.0f) * (0.50f + 0.55f * horizonFactor);
		float wideGlare   = pow(saturate(cosTheta), 6.0f) * (0.10f + 0.30f * horizonFactor);
		celestialGlow += SunColor.rgb * solarTint * ((innerCorona + midCorona + wideGlare) * dayVis);

		// Subtle forward-scattering boost on existing sky gradient near sun
		surface.sky.albedo *= 1.0f + pow(saturate(cosTheta), 8.0f) * 1.8f * dayVis;
	}

	// 2. Nighttime Moon: crisp spherical lunar disc with procedural maria + silvery halo
	if (moonVis > 0.001f)
	{
		float3 viewRayWS = normalize(mul(viewRayVS, (float3x3)matViewInverse));
		float3 upRef = (abs(sunDirWS.y) > 0.98f) ? float3(0.0f, 0.0f, 1.0f) : float3(0.0f, 1.0f, 0.0f);
		float3 moonRight = normalize(cross(upRef, sunDirWS));
		float3 moonUp    = cross(sunDirWS, moonRight);

		const float moonSinRadius = 0.030f; // ~1.7 deg apparent lunar radius
		float2 moonUV = float2(dot(viewRayWS, moonRight), dot(viewRayWS, moonUp)) / moonSinRadius;
		float r2 = dot(moonUV, moonUV);

		if (cosTheta > 0.99f && r2 < 1.05f)
		{
			float r = sqrt(max(r2, 0.0f));
			float moonDisc = 1.0f - smoothstep(0.88f, 1.0f, r);
			// Procedural lunar maria & crater shading anchored in world celestial frame
			float maria = 0.84f
				+ 0.16f * sin(moonUV.x * 3.6f + 0.7f) * cos(moonUV.y * 3.2f - 0.4f)
				- 0.11f * saturate(sin(moonUV.x * 7.1f - moonUV.y * 5.7f + 1.1f));
			float sphereLimb = sqrt(saturate(1.0f - r2 * 0.60f));
			float3 moonSurface = float3(0.74f, 0.85f, 1.06f) * (maria * sphereLimb * 6.2f);
			celestialGlow += moonSurface * (moonDisc * moonVis);
		}

		float lunarCorona = pow(saturate(cosTheta), 180.0f) * 1.5f + pow(saturate(cosTheta), 24.0f) * 0.20f;
		celestialGlow += float3(0.20f, 0.34f, 0.62f) * (lunarCorona * moonVis);
	}

	surface.sky.sunSpot = celestialGlow;
}

// this is not fog, but a very low frequency scattering of fog color over the entire scene. 
void CalculateAtmosphericScattering(inout float3 color, in SurfaceStruct surface)
{
	float3 fogColor = g_FogColor.rgb;
	float fogFactor = pow(surface.depth / 1500.0f, 2.0f);
	// only paint on non-sky area
	fogFactor *= lerp(1.0f, 0.0f, float(surface.mask_sky));
	//add scattered low frequency light
	color += fogColor * fogFactor * 2.0f;
}

// apply fog effect from FogStart to cameraFarPlane
// NOT used: really bad effect. with depth of view, distance fog is not needed. 
void ApplyDistanceFog(inout float3 color, in SurfaceStruct surface)
{
	float3 fogColor = g_FogColor.rgb;
	float fogFactor = clamp((surface.depth - FogStart) / (cameraFarPlane - 16) * 6.0, 0.0, 1.0);
	// only paint on non-sky area
	fogFactor *= lerp(1.0f, 0.0f, float(surface.mask_sky));
	color = lerp(color, fogColor, fogFactor);
}

float2 ClampViewUV(float2 coord)
{
	float2 border = 0.5 / screenParam;
	return clamp(coord, viewportOffset + border, viewportOffset + viewportScale - border);
}

float3 ViewPosition(float2 coord, float depth)
{
	float2 ndc = (coord - viewportOffset) / viewportScale * 2.0f - 1.0f;
	float invP11 = (abs(matProjection._11) > 1e-5f) ? (1.0f / matProjection._11) : (TanHalfFOV * ViewAspect);
	float invP22 = (abs(matProjection._22) > 1e-5f) ? (1.0f / matProjection._22) : TanHalfFOV;
	float p32 = (abs(matProjection._22) > 1e-5f) ? matProjection._32 : 0.0f;
	return float3((ndc.x - matProjection._31) * invP11, (-ndc.y - p32) * invP22, 1.0f) * depth;
}

float3 DecodeSurfaceAlbedo(float3 rawLinear)
{
	float3 a = pow(max(rawLinear, 0.0f), 1.4f);
	return lerp(a, dot(a, float1(0.3333f).xxx), 0.035f);
}

#ifdef BLOCK_SURFACE_EFFECTS
float2 SnapTexelUV(float2 coord, float2 pixelPhase)
{
	float2 clamped = ClampViewUV(coord);
	return ClampViewUV((floor(clamped * screenParam - pixelPhase + 0.5f) + pixelPhase) / screenParam);
}

void GetVoxelFaceBasis(float3 face, out float3 axisU, out float3 axisV)
{
	if (abs(face.y) > 0.5f)
	{
		axisU = float3(1.0f, 0.0f, 0.0f);
		axisV = float3(0.0f, 0.0f, 1.0f);
	}
	else if (abs(face.x) > 0.5f)
	{
		axisU = float3(0.0f, 0.0f, 1.0f);
		axisV = float3(0.0f, 1.0f, 0.0f);
	}
	else
	{
		axisU = float3(1.0f, 0.0f, 0.0f);
		axisV = float3(0.0f, 1.0f, 0.0f);
	}
}

float3 GetCardinalFaceNormal(float3 n)
{
	float3 an = abs(n);
	if (an.y >= an.x && an.y >= an.z)
		return float3(0.0f, (n.y >= 0.0f) ? 1.0f : -1.0f, 0.0f);
	if (an.x >= an.z)
		return float3((n.x >= 0.0f) ? 1.0f : -1.0f, 0.0f, 0.0f);
	return float3(0.0f, 0.0f, (n.z >= 0.0f) ? 1.0f : -1.0f);
}

// 8-block periodic scalloped/tufted contour in [0, 1] matching 8-block snapped volume origins so walking never pops.
float ZigZagTuftProfile(float alongCoord, float seamLine, float axisSeed)
{
	float alongMod = alongCoord - floor(alongCoord / 8.0f) * 8.0f;
	float lineMod = seamLine - floor(seamLine / 8.0f) * 8.0f;
	float seed = frac(sin(lineMod * 37.17f + axisSeed * 91.73f) * 43758.5453f);
	float triPrimary = 1.0f - abs(frac(alongMod * 3.0f + seed) * 2.0f - 1.0f);
	float triDetail = 1.0f - abs(frac(alongMod * 5.0f + seed * 2.71f) * 2.0f - 1.0f);
	float sway = 0.5f + 0.5f * sin(alongMod * 6.2831853f + seed * 6.2831853f);
	float raw = saturate(triPrimary * 0.58f + triDetail * 0.27f + sway * 0.20f);
	return 0.16f + 0.84f * smoothstep(0.08f, 0.92f, raw);
}

float4 SampleZigZagVoxel(float3 voxIdx)
{
	float3 idx = floor(voxIdx + 0.5f);
	if (any(idx < 0.0f) || any(idx > 31.0f))
		return float4(0.0f, 0.0f, 0.0f, 0.0f);
	float tileX = fmod(idx.y, 8.0f) * 32.0f;
	float tileY = floor(idx.y / 8.0f + 0.001f) * 32.0f;
	float2 uv = float2(tileX + idx.x + 0.5f, tileY + idx.z + 0.5f) / 256.0f;
	return tex2Dlod(blockReflectionSampler, float4(uv, 0.0f, 0.0f));
}

float3 SampleZigZagAtlasSRGB(float slotIdx, float fu, float fv)
{
	float s = clamp(floor(slotIdx + 0.5f), 1.0f, 31.0f);
	float slotX = fmod(s, 8.0f) * 32.0f;
	float slotY = 128.0f + floor(s / 8.0f + 0.001f) * 32.0f;
	float2 tilePx = clamp(floor(float2(fu, fv) * 32.0f) + 0.5f, 0.5f, 31.5f);
	float2 uv = (float2(slotX, slotY) + tilePx) / 256.0f;
	return tex2Dlod(blockReflectionSampler, float4(uv, 0.0f, 0.0f)).rgb;
}

bool IsVoxelFaceExposed(float maskByte, float faceBitVal)
{
	return fmod(floor(maskByte / faceBitVal + 0.01f), 2.0f) >= 0.9f;
}

// World-space voxel + block-atlas zig-zag seam blending on upward-facing terrain surfaces (100% stable under camera rotation).
void ApplyTerrainZigZagBlend(inout SurfaceStruct surface, float3 rawColorSRGB, float selfCode)
{
	float3 rawFace = surface.normal;
	[branch] if (ZigZagEnabled < 0.5f || BlockGridOrigin.w <= 0.0f || surface.depth <= 0.01f || surface.depth >= 15.5f ||
		surface.mask_sky || surface.mask_water || surface.category_id != 0 || selfCode < 0.5f || selfCode > 254.5f || rawFace.y < 0.95f)
		return;

	// Snap 8-bit UNORM normal (where 0 decodes to +/-0.00392) to exact cardinal unit axis so floor(voxelPos) never shifts by 2mm
	float3 face = GetCardinalFaceNormal(rawFace);
	float3 axisU, axisV;
	GetVoxelFaceBasis(face, axisU, axisV);
	float3 axisW = abs(face);

	float3 exactPosVS = ViewPosition(surface.texCoord, surface.depth);
	float3 worldRel = mul(float4(exactPosVS, 1.0f), matViewInverse).xyz;
	float3 voxelPos = (worldRel - BlockGridOrigin.xyz) * BlockGridOrigin.w;

	float u = dot(voxelPos, axisU);
	float v = dot(voxelPos, axisV);
	float w = dot(voxelPos, axisW) - ((dot(face, axisW) > 0.0f) ? 0.5f : -0.5f);
	float iu = floor(u);
	float iv = floor(v);
	float iw = floor(w);
	float fu = u - iu;
	float fv = v - iv;
	float du = (fu < 0.5f) ? fu : (1.0f - fu);
	float dv = (fv < 0.5f) ? fv : (1.0f - fv);
	float su = (fu < 0.5f) ? -1.0f : 1.0f;
	float sv = (fv < 0.5f) ? -1.0f : 1.0f;
	float3 selfVoxel = axisU * iu + axisV * iv + axisW * iw;

	// Smoothly fade inside the [1..30]^3 interior of the 32^3 near-camera volume
	float3 edgeMargin = min(selfVoxel - 0.5f, 30.5f - selfVoxel);
	float volFade = saturate(min(edgeMargin.x, min(edgeMargin.y, edgeMargin.z)) * 0.5f);
	float fade = (1.0f - smoothstep(9.5f, 15.0f, surface.depth)) * volFade;
	if (fade <= 0.001f)
		return;

	float maxAmp = 0.16f * fade;
	if (min(du, dv) >= maxAmp + 0.06f)
		return;

	float isSideFace = (abs(face.y) < 0.5f) ? 1.0f : 0.0f;
	float faceBitVal = (face.y > 0.5f) ? 1.0f : ((face.y < -0.5f) ? 2.0f :
		((face.x > 0.5f) ? 4.0f : ((face.x < -0.5f) ? 8.0f : ((face.z > 0.5f) ? 16.0f : 32.0f))));

	float4 selfVox = SampleZigZagVoxel(selfVoxel);
	float selfVoxCode = floor(selfVox.r * 255.0f + 0.5f);

	// Resolve sub-millimeter depth-vs-rasterizer ties right on block seams so pixel ownership always matches _BlockInfoRT
	if (abs(selfVoxCode - selfCode) > 0.5f)
	{
		if (du < 0.03f)
		{
			float4 candU = SampleZigZagVoxel(selfVoxel + axisU * su);
			if (abs(floor(candU.r * 255.0f + 0.5f) - selfCode) < 0.5f)
			{
				iu += su;
				fu = (su > 0.0f) ? 0.0001f : 0.9999f;
				du = 0.0001f;
				su = -su;
				selfVoxel = axisU * iu + axisV * iv + axisW * iw;
				selfVox = candU;
				selfVoxCode = selfCode;
			}
		}
		if (abs(selfVoxCode - selfCode) > 0.5f && dv < 0.03f)
		{
			float4 candV = SampleZigZagVoxel(selfVoxel + axisV * sv);
			if (abs(floor(candV.r * 255.0f + 0.5f) - selfCode) < 0.5f)
			{
				iv += sv;
				fv = (sv > 0.0f) ? 0.0001f : 0.9999f;
				dv = 0.0001f;
				sv = -sv;
				selfVoxel = axisU * iu + axisV * iv + axisW * iw;
				selfVox = candV;
				selfVoxCode = selfCode;
			}
		}
	}

	float selfMask = floor(selfVox.b * 255.0f + 0.5f);
	if (!IsVoxelFaceExposed(selfMask, faceBitVal))
		return;

	// Preserve smooth vertex AO baked into _ColorRT when replacing albedo with un-shaded block atlas texels
	float selfSlot = floor((isSideFace > 0.5f ? selfVox.a : selfVox.g) * 255.0f + 0.5f);
	float vertexAO = 1.0f;
	if (selfSlot > 0.5f)
	{
		float3 selfTexSRGB = SampleZigZagAtlasSRGB(selfSlot, fu, fv);
		float selfLum = dot(selfTexSRGB, float3(0.299f, 0.587f, 0.114f));
		float rtLum = dot(rawColorSRGB, float3(0.299f, 0.587f, 0.114f));
		if (selfLum > 0.04f)
			vertexAO = clamp(rtLum / selfLum, 0.45f, 1.0f);
	}

	// Exact analytical perspective-foreshortened pixel footprint on the block plane (no ddx/ddy in branches)
	float3 eyeToHit = worldRel - matViewInverse[3].xyz;
	float rayDist = max(length(eyeToHit), 0.1f);
	float3 rayDirWS = eyeToHit / rayDist;
	float pixelAngle = (2.0f * TanHalfFOV) / max(screenParam.y, 1.0f);
	float denom = dot(rayDirWS, face);
	float safeDenom = (abs(denom) > 0.10f) ? denom : ((denom >= 0.0f) ? 0.10f : -0.10f);

	float selfTier = floor(selfCode / 16.0f + 0.001f);

	float blendU = 0.0f, codeU = 0.0f, tierU = 0.0f, slotU = 0.0f, lipU = 0.0f;
	float maskU = 0.0f;
	if (du < maxAmp + 0.06f)
	{
		float3 dirU = axisU * su;
		float4 voxU = SampleZigZagVoxel(selfVoxel + dirU);
		codeU = floor(voxU.r * 255.0f + 0.5f);
		maskU = floor(voxU.b * 255.0f + 0.5f);
		slotU = floor((isSideFace > 0.5f ? voxU.a : voxU.g) * 255.0f + 0.5f);
		if (codeU > 0.5f && abs(codeU - selfCode) > 0.5f && slotU > 0.5f && IsVoxelFaceExposed(maskU, faceBitVal))
		{
			tierU = floor(codeU / 16.0f + 0.001f);
			float toothU = ZigZagTuftProfile(v, floor(u + 0.5f), 1.0f);
			float reachU = (tierU > selfTier + 0.5f) ? (toothU * maxAmp)
				: ((abs(tierU - selfTier) < 0.5f) ? max(0.0f, ((codeU > selfCode) ? (toothU - 0.54f) : (0.62f - toothU)) * 1.35f * maxAmp) : 0.0f);
			if (reachU > 0.001f)
			{
				float3 gradU = dirU - face * (dot(rayDirWS, dirU) / safeDenom);
				gradU -= rayDirWS * dot(gradU, rayDirWS);
				float pxU = clamp(length(gradU) * rayDist * pixelAngle * BlockGridOrigin.w, 0.004f, 0.075f);
				blendU = saturate((reachU - du) / pxU + 0.5f);
				if (tierU > selfTier + 0.5f && du > reachU && du < reachU + 0.025f)
					lipU = saturate(1.0f - (du - reachU) / 0.025f) * (1.0f - blendU);
			}
		}
	}

	float blendV = 0.0f, codeV = 0.0f, tierV = 0.0f, slotV = 0.0f, lipV = 0.0f;
	float maskV = 0.0f;
	if (dv < maxAmp + 0.06f)
	{
		float3 dirV = axisV * sv;
		float4 voxV = SampleZigZagVoxel(selfVoxel + dirV);
		codeV = floor(voxV.r * 255.0f + 0.5f);
		maskV = floor(voxV.b * 255.0f + 0.5f);
		slotV = floor((isSideFace > 0.5f ? voxV.a : voxV.g) * 255.0f + 0.5f);
		if (codeV > 0.5f && abs(codeV - selfCode) > 0.5f && slotV > 0.5f && IsVoxelFaceExposed(maskV, faceBitVal))
		{
			tierV = floor(codeV / 16.0f + 0.001f);
			float toothV = ZigZagTuftProfile(u, floor(v + 0.5f), 2.0f);
			float reachV = (tierV > selfTier + 0.5f) ? (toothV * maxAmp)
				: ((abs(tierV - selfTier) < 0.5f) ? max(0.0f, ((codeV > selfCode) ? (toothV - 0.54f) : (0.62f - toothV)) * 1.35f * maxAmp) : 0.0f);
			if (reachV > 0.001f)
			{
				float3 gradV = dirV - face * (dot(rayDirWS, dirV) / safeDenom);
				gradV -= rayDirWS * dot(gradV, rayDirWS);
				float pxV = clamp(length(gradV) * rayDist * pixelAngle * BlockGridOrigin.w, 0.004f, 0.075f);
				blendV = saturate((reachV - dv) / pxV + 0.5f);
				if (tierV > selfTier + 0.5f && dv > reachV && dv < reachV + 0.025f)
					lipV = saturate(1.0f - (dv - reachV) / 0.025f) * (1.0f - blendV);
			}
		}
	}

	if (blendU > 0.005f || blendV > 0.005f)
	{
		if (blendU > 0.005f && blendV > 0.005f && abs(codeU - codeV) < 0.5f)
		{
			float unionBlend = 1.0f - (1.0f - blendU) * (1.0f - blendV);
			float3 donorSRGB = SampleZigZagAtlasSRGB(slotU, fu, fv) * vertexAO;
			surface.albedo = lerp(surface.albedo, DecodeSurfaceAlbedo(pow(max(donorSRGB, 0.0f), 2.2f)), unionBlend);
		}
		else if (tierU <= tierV)
		{
			if (blendU > 0.005f)
			{
				float3 srgbU = SampleZigZagAtlasSRGB(slotU, fu, fv) * vertexAO;
				surface.albedo = lerp(surface.albedo, DecodeSurfaceAlbedo(pow(max(srgbU, 0.0f), 2.2f)), blendU);
			}
			if (blendV > 0.005f)
			{
				float3 srgbV = SampleZigZagAtlasSRGB(slotV, fu, fv) * vertexAO;
				surface.albedo = lerp(surface.albedo, DecodeSurfaceAlbedo(pow(max(srgbV, 0.0f), 2.2f)), blendV);
			}
		}
		else
		{
			if (blendV > 0.005f)
			{
				float3 srgbV = SampleZigZagAtlasSRGB(slotV, fu, fv) * vertexAO;
				surface.albedo = lerp(surface.albedo, DecodeSurfaceAlbedo(pow(max(srgbV, 0.0f), 2.2f)), blendV);
			}
			if (blendU > 0.005f)
			{
				float3 srgbU = SampleZigZagAtlasSRGB(slotU, fu, fv) * vertexAO;
				surface.albedo = lerp(surface.albedo, DecodeSurfaceAlbedo(pow(max(srgbU, 0.0f), 2.2f)), blendU);
			}
		}
		return;
	}

	// Diagonal outer-corner tuft (only at true outer corners like T-junctions/crossroads where neither U nor V is already higher-tier)
	if (tierU <= selfTier + 0.5f && tierV <= selfTier + 0.5f &&
		du < maxAmp * 0.75f && dv < maxAmp * 0.75f && IsVoxelFaceExposed(maskU, faceBitVal) && IsVoxelFaceExposed(maskV, faceBitVal))
	{
		float3 dirDiag = axisU * su + axisV * sv;
		float4 voxD = SampleZigZagVoxel(selfVoxel + dirDiag);
		float codeD = floor(voxD.r * 255.0f + 0.5f);
		float maskD = floor(voxD.b * 255.0f + 0.5f);
		float slotD = floor((isSideFace > 0.5f ? voxD.a : voxD.g) * 255.0f + 0.5f);
		float tierD = floor(codeD / 16.0f + 0.001f);
		if (codeD > 0.5f && tierD > selfTier + 0.5f && slotD > 0.5f && IsVoxelFaceExposed(maskD, faceBitVal))
		{
			float cLineU = floor(u + 0.5f) - floor(floor(u + 0.5f) / 8.0f) * 8.0f;
			float cLineV = floor(v + 0.5f) - floor(floor(v + 0.5f) / 8.0f) * 8.0f;
			float cSeed = frac(sin(cLineU * 23.17f + cLineV * 59.41f) * 43758.5453f);
			float reachD = maxAmp * (0.44f + 0.22f * cSeed);
			float radialDist = length(float2(du, dv));
			float3 dirNorm = normalize(dirDiag);
			float3 gradD = dirNorm - face * (dot(rayDirWS, dirNorm) / safeDenom);
			gradD -= rayDirWS * dot(gradD, rayDirWS);
			float pxD = clamp(length(gradD) * rayDist * pixelAngle * BlockGridOrigin.w, 0.004f, 0.075f);
			float blendD = saturate((reachD - radialDist) / pxD + 0.5f);
			if (blendD > 0.005f)
			{
				float3 srgbD = SampleZigZagAtlasSRGB(slotD, fu, fv) * vertexAO;
				surface.albedo = lerp(surface.albedo, DecodeSurfaceAlbedo(pow(max(srgbD, 0.0f), 2.2f)), blendD);
				return;
			}
		}
	}

	float lip = max(lipU, lipV);
	if (lip > 0.001f)
		surface.albedo *= (1.0f - 0.075f * fade * lip);
}

float GetCardinalFaceBitVal(float3 dir)
{
	if (dir.y > 0.5f) return 1.0f;
	if (dir.y < -0.5f) return 2.0f;
	if (dir.x > 0.5f) return 4.0f;
	if (dir.x < -0.5f) return 8.0f;
	if (dir.z > 0.5f) return 16.0f;
	return 32.0f;
}

float SafeInvScalar(float v)
{
	return 1.0f / ((abs(v) > 1e-6f) ? v : ((v >= 0.0f) ? 1e-6f : -1e-6f));
}

bool GetCardinalExposure(float3 dir, bool expNX, bool expPX, bool expNY, bool expPY, bool expNZ, bool expPZ)
{
	if (dir.x < -0.5f) return expNX;
	if (dir.x > 0.5f)  return expPX;
	if (dir.y < -0.5f) return expNY;
	if (dir.y > 0.5f)  return expPY;
	if (dir.z < -0.5f) return expNZ;
	return expPZ;
}

void ClearCardinalExposure(float3 dir, inout bool expNX, inout bool expPX, inout bool expNY, inout bool expPY, inout bool expNZ, inout bool expPZ)
{
	if (dir.x < -0.5f) expNX = false;
	else if (dir.x > 0.5f) expPX = false;
	else if (dir.y < -0.5f) expNY = false;
	else if (dir.y > 0.5f) expPY = false;
	else if (dir.z < -0.5f) expNZ = false;
	else expPZ = false;
}

// 3D Round-Box ray-intersected bevels & silhouettes using true world-space voxel topology (view-consistent at all camera angles).
float ApplyRoundedBlockBorders(inout SurfaceStruct surface, inout float sun_light_strength, inout float torch_light_strength, float2 pixelPhase)
{
	float3 rawFace = surface.normal;
	float major = max(abs(rawFace.x), max(abs(rawFace.y), abs(rawFace.z)));
	[branch] if (BevelEnabled < 0.5f || surface.depth <= 0.01f || surface.depth >= 16.0f || surface.mask_sky || surface.mask_water ||
		surface.category_id == 255 || surface.category_id == 18 || surface.category_id == 31 || major < 0.99f)
		return 1.0f;

	float3 face = GetCardinalFaceNormal(rawFace);
	float fade = 1.0f - smoothstep(9.5f, 15.5f, surface.depth);
	float3 faceVS = mul(face, (float3x3)matView);
	float3 bend = float3(0.0f, 0.0f, 0.0f);
	float creaseAO = 1.0f;
	float convexRim = 0.0f;
	bool handledByVoxelGrid = false;

	[branch] if (BlockGridOrigin.w > 0.0f)
	{
		float blockSize = 1.0f / BlockGridOrigin.w;
		float2 origTexCoord = surface.texCoord;
		float3 rayVS1 = ViewPosition(origTexCoord, 1.0f);
		float3 exactPosVS = rayVS1 * surface.depth;
		float3 worldRel = mul(float4(exactPosVS, 1.0f), matViewInverse).xyz;
		float3 voxelPos = (worldRel - BlockGridOrigin.xyz) * BlockGridOrigin.w;

		float3 axisU, axisV;
		GetVoxelFaceBasis(face, axisU, axisV);
		float3 axisW = abs(face);

		float u = dot(voxelPos, axisU);
		float v = dot(voxelPos, axisV);
		float w = dot(voxelPos - face * 0.5f, axisW);
		float iu = floor(u);
		float iv = floor(v);
		float iw = floor(w);
		float fu = u - iu;
		float fv = v - iv;
		float du = (fu < 0.5f) ? fu : (1.0f - fu);
		float dv = (fv < 0.5f) ? fv : (1.0f - fv);
		float su = (fu < 0.5f) ? -1.0f : 1.0f;
		float sv = (fv < 0.5f) ? -1.0f : 1.0f;
		float3 selfVoxel = axisU * iu + axisV * iv + axisW * iw;

		float3 edgeMargin = min(selfVoxel - 0.5f, 30.5f - selfVoxel);
		float volFade = saturate(min(edgeMargin.x, min(edgeMargin.y, edgeMargin.z)) * 0.5f);
		float voxFade = fade * volFade;

		if (voxFade > 0.005f)
		{
			float faceBitVal = GetCardinalFaceBitVal(face);
			float selfMask = floor(SampleZigZagVoxel(selfVoxel).b * 255.0f + 0.5f);
			bool validSelf = (selfMask >= 63.5f) && IsVoxelFaceExposed(selfMask, faceBitVal);

			// Snap depth-reconstruction jitter on steep grazing seams & corner tips to the owning solid voxel
			if (!validSelf)
			{
				if (du < 0.16f)
				{
					float3 cand = selfVoxel + axisU * su;
					float candMask = floor(SampleZigZagVoxel(cand).b * 255.0f + 0.5f);
					if (candMask >= 63.5f && IsVoxelFaceExposed(candMask, faceBitVal))
					{
						selfVoxel = cand;
						selfMask = candMask;
						validSelf = true;
					}
				}
				if (!validSelf && dv < 0.16f)
				{
					float3 cand = selfVoxel + axisV * sv;
					float candMask = floor(SampleZigZagVoxel(cand).b * 255.0f + 0.5f);
					if (candMask >= 63.5f && IsVoxelFaceExposed(candMask, faceBitVal))
					{
						selfVoxel = cand;
						selfMask = candMask;
						validSelf = true;
					}
				}
				if (!validSelf && du < 0.16f && dv < 0.16f)
				{
					float3 cand = selfVoxel + axisU * su + axisV * sv;
					float candMask = floor(SampleZigZagVoxel(cand).b * 255.0f + 0.5f);
					if (candMask >= 63.5f && IsVoxelFaceExposed(candMask, faceBitVal))
					{
						selfVoxel = cand;
						selfMask = candMask;
						validSelf = true;
					}
				}
			}

			if (validSelf)
			{
				handledByVoxelGrid = true;

				float3 eyeWS = matViewInverse[3].xyz;
				float3 eyeLocal = (eyeWS - BlockGridOrigin.xyz) * BlockGridOrigin.w - selfVoxel;
				float3 rayStepWS = mul(rayVS1, (float3x3)matViewInverse);
				float3 rayDirWS = normalize(rayStepWS);
				float3 invDir = float3(SafeInvScalar(rayDirWS.x), SafeInvScalar(rayDirWS.y), SafeInvScalar(rayDirWS.z));

				float3 t0Box = (float3(0.0f, 0.0f, 0.0f) - eyeLocal) * invDir;
				float3 t1Box = (float3(1.0f, 1.0f, 1.0f) - eyeLocal) * invDir;
				float3 tMin3 = min(t0Box, t1Box);
				float3 tMax3 = max(t0Box, t1Box);
				float tmin = max(max(tMin3.x, max(tMin3.y, tMin3.z)), 0.0f);
				float tmax = min(tMax3.x, min(tMax3.y, tMax3.z));
				if (tmin > tmax)
				{
					float3 pMid = eyeLocal + rayDirWS * (0.5f * (tmin + tmax));
					float3 boxGap = max(max(-pMid, pMid - 1.0f), 0.0f);
					if (dot(boxGap, boxGap) > 0.05f * 0.05f)
						return 1.0f;
					tmax = tmin;
				}

				float3 p0 = clamp(eyeLocal + rayDirWS * tmin, 0.0f, 1.0f);
				float trel = tmax - tmin;
				float pu = dot(p0, axisU);
				float pv = dot(p0, axisV);
				du = (pu < 0.5f) ? pu : (1.0f - pu);
				dv = (pv < 0.5f) ? pv : (1.0f - pv);
				su = (pu < 0.5f) ? -1.0f : 1.0f;
				sv = (pv < 0.5f) ? -1.0f : 1.0f;

				float R = 0.115f * voxFade;
				float wAO = 0.16f * voxFade;
				if (min(du, dv) >= wAO)
					return 1.0f;

				bool expNX = IsVoxelFaceExposed(selfMask, 8.0f);
				bool expPX = IsVoxelFaceExposed(selfMask, 4.0f);
				bool expNY = IsVoxelFaceExposed(selfMask, 2.0f);
				bool expPY = IsVoxelFaceExposed(selfMask, 1.0f);
				bool expNZ = IsVoxelFaceExposed(selfMask, 32.0f);
				bool expPZ = IsVoxelFaceExposed(selfMask, 16.0f);

				float3 dirU = axisU * su;
				float3 dirV = axisV * sv;

				// 1) Concave inner crease AO & suppression of convex rounding into adjacent rising walls
				if (du < wAO)
				{
					float upMaskU = floor(SampleZigZagVoxel(selfVoxel + dirU + face).b * 255.0f + 0.5f);
					if (upMaskU >= 63.5f)
					{
						ClearCardinalExposure(dirU, expNX, expPX, expNY, expPY, expNZ, expPZ);
						float cw = saturate(1.0f - du / max(wAO, 0.001f));
						creaseAO = min(creaseAO, 1.0f - 0.11f * cw * cw);
					}
				}
				if (dv < wAO)
				{
					float upMaskV = floor(SampleZigZagVoxel(selfVoxel + dirV + face).b * 255.0f + 0.5f);
					if (upMaskV >= 63.5f)
					{
						ClearCardinalExposure(dirV, expNX, expPX, expNY, expPY, expNZ, expPZ);
						float cw = saturate(1.0f - dv / max(wAO, 0.001f));
						creaseAO = min(creaseAO, 1.0f - 0.11f * cw * cw);
					}
				}

				if (min(du, dv) >= R)
					return creaseAO;

				bool expU = GetCardinalExposure(dirU, expNX, expPX, expNY, expPY, expNZ, expPZ);
				bool expV = GetCardinalExposure(dirV, expNX, expPX, expNY, expPY, expNZ, expPZ);
				if (!expU && !expV)
					return creaseAO;

				// 2) Smoothly taper fillet radius to 0 where a convex lip meets a coplanar L-wall / staircase / window frame
				float rU = R;
				float rV = R;
				if (du < R && dv < R)
				{
					float bitU = GetCardinalFaceBitVal(dirU);
					float bitV = GetCardinalFaceBitVal(dirV);
					if (expU && !expV)
					{
						float nvMask = floor(SampleZigZagVoxel(selfVoxel + dirV).b * 255.0f + 0.5f);
						if (nvMask >= 63.5f && IsVoxelFaceExposed(nvMask, faceBitVal) && !IsVoxelFaceExposed(nvMask, bitU))
							rU = R * smoothstep(0.0f, R, dv);
					}
					if (expV && !expU)
					{
						float nuMask = floor(SampleZigZagVoxel(selfVoxel + dirU).b * 255.0f + 0.5f);
						if (nuMask >= 63.5f && IsVoxelFaceExposed(nuMask, faceBitVal) && !IsVoxelFaceExposed(nuMask, bitV))
							rV = R * smoothstep(0.0f, R, du);
					}
				}
				float rLocal = min(rU, rV);
				if (rLocal <= 0.001f)
					return creaseAO;

				// 3) Exact 3D Round-Box ray intersection inside [0, 1]^3
				float3 bmin = float3(
					expNX ? rLocal : -rLocal,
					expNY ? rLocal : -rLocal,
					expNZ ? rLocal : -rLocal);
				float3 bmax = float3(
					expPX ? (1.0f - rLocal) : (1.0f + rLocal),
					expPY ? (1.0f - rLocal) : (1.0f + rLocal),
					expPZ ? (1.0f - rLocal) : (1.0f + rLocal));

				float3 s0 = p0 - clamp(p0, bmin, bmax);
				float sdf0 = length(s0) - rLocal;
				float minSdf = sdf0;
				float tHit = 0.0f;

				if (sdf0 > 0.0f)
				{
					float3 tcA = (bmin - p0) * invDir;
					float3 tcB = (bmax - p0) * invDir;
					float3 tcMin3 = min(tcA, tcB);
					float3 tcMax3 = max(tcA, tcB);
					float tc0 = max(tcMin3.x, max(tcMin3.y, tcMin3.z));
					float tc1 = min(tcMax3.x, min(tcMax3.y, tcMax3.z));

					float tc = 0.0f;
					if (tc1 >= max(tc0, 0.0f) && tc0 <= trel)
					{
						tc = max(tc0, 0.0f);
						minSdf = -rLocal;
					}
					else if (dot(s0, rayDirWS) >= 0.0f)
					{
						tc = 0.0f;
						minSdf = sdf0;
					}
					else
					{
						float3 pEnd = p0 + rayDirWS * trel;
						float3 sEnd = pEnd - clamp(pEnd, bmin, bmax);
						if (dot(sEnd, rayDirWS) <= 0.0f)
						{
							tc = trel;
							minSdf = length(sEnd) - rLocal;
						}
						else
						{
							float lo = 0.0f;
							float hi = trel;
							float t = 0.5f * trel;
							[loop] for (int k = 0; k < 5; ++k)
							{
								float3 pt = p0 + rayDirWS * t;
								float3 st = pt - clamp(pt, bmin, bmax);
								float gp = dot(st, rayDirWS);
								if (gp < 0.0f) lo = t;
								else hi = t;
								float gpp = ((abs(st.x) > 1e-6f) ? (rayDirWS.x * rayDirWS.x) : 0.0f)
									+ ((abs(st.y) > 1e-6f) ? (rayDirWS.y * rayDirWS.y) : 0.0f)
									+ ((abs(st.z) > 1e-6f) ? (rayDirWS.z * rayDirWS.z) : 0.0f);
								float tn = (gpp > 1e-6f) ? (t - gp / gpp) : (0.5f * (lo + hi));
								t = (tn > lo && tn < hi) ? tn : (0.5f * (lo + hi));
							}
							tc = t;
							float3 sClose = (p0 + rayDirWS * tc) - clamp(p0 + rayDirWS * tc, bmin, bmax);
							minSdf = length(sClose) - rLocal;
						}
					}

					if (minSdf > 0.0f)
					{
						tHit = tc;
					}
					else
					{
						float rlo = 0.0f;
						float rhi = tc;
						float rt = 0.5f * tc;
						float r2 = rLocal * rLocal;
						[loop] for (int rStep = 0; rStep < 6; ++rStep)
						{
							float3 pt = p0 + rayDirWS * rt;
							float3 st = pt - clamp(pt, bmin, bmax);
							float val = dot(st, st) - r2;
							if (val > 0.0f) rlo = rt;
							else rhi = rt;
							float deriv = 2.0f * dot(st, rayDirWS);
							float rn = (deriv < -1e-6f) ? (rt - val / deriv) : (0.5f * (rlo + rhi));
							rt = (rn > rlo && rn < rhi) ? rn : (0.5f * (rlo + rhi));
						}
						tHit = rt;
					}
				}

				float3 pHit = p0 + rayDirWS * tHit;
				float3 sHit = pHit - clamp(pHit, bmin, bmax);
				float sLen = length(sHit);
				float3 nRB = (sLen > 1e-5f) ? (sHit / sLen) : face;

				// 4) Silhouette miss: coarse-to-fine 2D screen march from origTexCoord to the exact first texel outside selfVoxel
				if (minSdf > 0.0f)
				{
					float pxBlock = clamp(surface.depth * TanHalfFOV * 2.0f / max(screenParam.y, 1.0f), 0.0005f, 0.025f);
					float3 pExit = p0 + rayDirWS * trel;
					float3 nOut = nRB;
					if (!expNY && pExit.y < 0.002f && abs(nRB.y) < 0.05f)
					{
						// Protruding vertical cylinder base meeting a solid floor: step down-outward onto the floor
						nOut = normalize(nRB + float3(0.0f, -1.0f, 0.0f));
					}
					else if (!expPY && pExit.y > 0.998f && abs(nRB.y) < 0.05f)
					{
						nOut = normalize(nRB + float3(0.0f, 1.0f, 0.0f));
					}
					float3 nHoriz3D = nOut - rayDirWS * dot(nOut, rayDirWS);
					float nHorizLen = length(nHoriz3D);
					nHoriz3D = (nHorizLen > 1e-4f) ? (nHoriz3D / nHorizLen) : nOut;

					float2 originSS = convertCameraSpaceToScreenSpace(exactPosVS);
					float3 nudgedVS = mul(float4(worldRel + nHoriz3D * (0.05f * blockSize), 1.0f), matView).xyz;
					float2 deltaPx = (convertCameraSpaceToScreenSpace(nudgedVS) - originSS) * screenParam;
					float deltaLen = length(deltaPx);
					float2 stepDirUV = (deltaLen > 1e-4f) ? ((deltaPx / deltaLen) / screenParam) : float2(0.0f, -1.0f / screenParam.y);
					float fgTol = max(0.03f, pxBlock * 2.0f * blockSize);
					float maxMarchPx = clamp(deltaLen * 20.0f * (1.45f * rLocal) + 6.0f, 16.0f, 180.0f);

					float loPx = 0.0f;
					float hiPx = -1.0f;
					bool foundBg = false;
					float2 bestBgUV = origTexCoord;
					float bestBgDepth = 0.0f;
					float3 bestBgPosWS = float3(0.0f, 0.0f, 0.0f);

					[loop] for (int cIdx = 0; cIdx < 8; ++cIdx)
					{
						float curPx = (cIdx == 0) ? 1.0f :
							((cIdx == 1) ? 3.0f :
							((cIdx == 2) ? 6.0f :
							((cIdx == 3) ? 11.0f :
							((cIdx == 4) ? max(18.0f, floor(maxMarchPx * 0.28f + 0.5f)) :
							((cIdx == 5) ? max(28.0f, floor(maxMarchPx * 0.48f + 0.5f)) :
							((cIdx == 6) ? max(42.0f, floor(maxMarchPx * 0.72f + 0.5f)) :
								max(60.0f, floor(maxMarchPx + 0.5f))))))));
						float2 candUV = SnapTexelUV(origTexCoord + stepDirUV * curPx, pixelPhase);
						float candDepth = tex2Dlod(depthSampler, float4(candUV, 0, 0)).r;
						if (candDepth >= 0.01f && candDepth < surface.depth - fgTol)
						{
							// Hit a foreground occluder: bracket [loPx, curPx] in case a narrow gap precedes it
							hiPx = curPx;
							break;
						}
						bool isBg = (candDepth < 0.01f);
						float3 candPosWS = float3(0.0f, 0.0f, 0.0f);
						if (!isBg)
						{
							candPosWS = mul(float4(ViewPosition(candUV, candDepth), 1.0f), matViewInverse).xyz;
							float3 candLocal = (candPosWS - BlockGridOrigin.xyz) * BlockGridOrigin.w - selfVoxel;
							isBg = any(candLocal < -0.015f) || any(candLocal > 1.015f);
						}
						if (isBg)
						{
							hiPx = curPx;
							foundBg = true;
							bestBgUV = candUV;
							bestBgDepth = candDepth;
							bestBgPosWS = candPosWS;
							break;
						}
						loPx = curPx;
						if (curPx >= maxMarchPx)
							break;
					}

					if (hiPx > 0.0f)
					{
						[loop] for (int bIdx = 0; bIdx < 5; ++bIdx)
						{
							if (hiPx - loPx <= 1.1f)
								break;
							float midPx = floor(0.5f * (loPx + hiPx) + 0.5f);
							float2 candUV = SnapTexelUV(origTexCoord + stepDirUV * midPx, pixelPhase);
							float candDepth = tex2Dlod(depthSampler, float4(candUV, 0, 0)).r;
							if (candDepth >= 0.01f && candDepth < surface.depth - fgTol)
							{
								hiPx = midPx;
								continue;
							}
							bool isBg = (candDepth < 0.01f);
							float3 candPosWS = float3(0.0f, 0.0f, 0.0f);
							if (!isBg)
							{
								candPosWS = mul(float4(ViewPosition(candUV, candDepth), 1.0f), matViewInverse).xyz;
								float3 candLocal = (candPosWS - BlockGridOrigin.xyz) * BlockGridOrigin.w - selfVoxel;
								isBg = any(candLocal < -0.015f) || any(candLocal > 1.015f);
							}
							if (isBg)
							{
								hiPx = midPx;
								foundBg = true;
								bestBgUV = candUV;
								bestBgDepth = candDepth;
								bestBgPosWS = candPosWS;
							}
							else
							{
								loPx = midPx;
							}
						}
					}

					if (foundBg)
					{
						float3 bgColorSRGB = tex2Dlod(colorSampler, float4(bestBgUV, 0, 0)).rgb;
						float3 bgLinear = pow(max(bgColorSRGB, 0.0f), 2.2f);
						float4 bgInfo = tex2Dlod(matInfoSampler, float4(bestBgUV, 0, 0));
						float3 bgNorm = tex2Dlod(normalSampler, float4(bestBgUV, 0, 0)).rgb * 2.0f - 1.0f;

						surface.sky.albedo = bgLinear;
						surface.albedo = DecodeSurfaceAlbedo(bgLinear);
						surface.normal = bgNorm;
						surface.category_id = (int)(bgInfo.r * 255.0f + 0.4f);
						sun_light_strength = bgInfo.g;
						torch_light_strength = bgInfo.b;

						if (bestBgDepth < 0.01f)
						{
							surface.depth = bestBgDepth;
							surface.cameraSpacePos = rayVS1 * bestBgDepth;
						}
						else
						{
							float denomBg = dot(rayStepWS, bgNorm);
							float zPlane = (abs(denomBg) > 0.15f) ? (dot(bestBgPosWS - eyeWS, bgNorm) / denomBg) : bestBgDepth;
							if (abs(denomBg) > 0.15f && abs(zPlane - bestBgDepth) < 0.35f && zPlane >= surface.depth - 0.02f)
							{
								surface.depth = zPlane;
								surface.cameraSpacePos = rayVS1 * zPlane;
								surface.texCoord = origTexCoord;
							}
							else
							{
								surface.depth = bestBgDepth;
								surface.cameraSpacePos = ViewPosition(bestBgUV, bestBgDepth);
								surface.texCoord = bestBgUV;
							}
							CalculateMasks(surface);
							float bgBlendCode = floor(bgInfo.a * 255.0f + 0.5f);
							ApplyTerrainZigZagBlend(surface, bgColorSRGB, bgBlendCode);
						}
						return 1.0f;
					}
				}

				surface.normal = nRB;
				float rimAmount = saturate(1.0f - dot(nRB, face));
				float upBias = saturate(nRB.y * 0.5f + 0.5f);
				float bevelSheen = 1.0f + rimAmount * voxFade * (0.06f + 0.05f * upBias);
				return creaseAO * bevelSheen;
			}
		}
	}

	// Screen-space fallback for non-grid convex steps (e.g. stairs/slabs at half-block offsets)
	if (!handledByVoxelGrid)
	{
		float subWidth = 0.06f * fade;
		float2 radius = min(10.0f / screenParam, viewportScale * float2(1.0f / ViewAspect, 1.0f)
			* subWidth / max(2.0f * surface.depth * TanHalfFOV, 0.01f));
		[loop] for (int i = 0; i < 4; ++i)
		{
			float2 axis = i < 2 ? float2(1, 0) : float2(0, 1);
			float2 uv = ClampViewUV(surface.texCoord + axis * ((i == 0 || i == 2) ? radius : -radius));
			float depth = tex2Dlod(depthSampler, float4(uv, 0, 0)).r;
			float3 other = tex2Dlod(normalSampler, float4(uv, 0, 0)).rgb * 2.0f - 1.0f;
			float3 delta = ViewPosition(uv, depth) - surface.cameraSpacePos;
			float across = dot(mul(other, (float3x3)matView), delta);
			if (depth > 0.01f && dot(delta, delta) < 0.04f && dot(delta, faceVS) < 0.0f && across > 0.0f &&
				abs(dot(face, other)) < 0.12f)
			{
				float sw = saturate(1.0f - across / max(subWidth, 0.001f));
				bend += other * sw;
				convexRim = max(convexRim, sw * 0.5f);
			}
		}
		if (dot(bend, bend) > 0.0001f)
		{
			surface.normal = normalize(face + bend);
			float upBias = saturate(surface.normal.y * 0.5f + 0.5f);
			float bevelSheen = 1.0f + convexRim * fade * (0.05f + 0.04f * upBias);
			return bevelSheen;
		}
	}

	return 1.0f;
}

#endif

float AmbientVisibility(SurfaceStruct surface)
{
	if (QualityLevel < 5 || LightingStrength.x <= 0 || surface.mask_sky || surface.mask_water) return 1.0;
	float3 normal = normalize(mul(surface.normal, (float3x3)matView));
	float2 radius = min(32.0 / screenParam, float2(0.8 / ViewAspect, 0.8) / max(surface.depth * TanHalfFOV, 0.1)) * viewportScale;
	float total = 0;
	float count = QualityLevel >= 6 ? 16.0 : 8.0;
	[loop] for (int sampleIndex = 0; sampleIndex < 16; sampleIndex++)
	{
		if (sampleIndex >= count) break;
		float angle = sampleIndex * 2.399963;
		float2 coord = ClampViewUV(surface.texCoord + float2(cos(angle), sin(angle)) * radius * sqrt((sampleIndex + 0.5) / count));
		float depth = tex2Dlod(depthSampler, float4(coord, 0, 0)).r;
		float3 delta = ViewPosition(coord, depth) - surface.cameraSpacePos;
		float distanceToSample = length(delta);
		if (depth > 0.01 && distanceToSample > 0.02)
			total += saturate(dot(normal, delta / distanceToSample) - 0.12) * saturate(1.0 - distanceToSample / 1.6);
	}
	return saturate(1.0 - total * 2.0 * LightingStrength.x / count);
}

float ContactVisibility(SurfaceStruct surface)
{
	if (QualityLevel < 4 || LightingStrength.y <= 0 || RenderOptions.x < 0.5 || surface.mask_sky || surface.mask_water || surface.category_id == 255 || surface.NdotL <= 0 || rainStrength >= 0.99) return 1.0;
	float3 direction = normalize(mul(sunDirection, (float3x3)matView));
	float count = (QualityLevel >= 6) ? 12.0 : ((QualityLevel >= 5) ? 8.0 : 4.0);
	float ign = InterleavedGradientNoise(surface.texCoord * screenParam);
	float minThickness = max(0.02, surface.depth * 0.005);
	float maxThickness = 0.12 + surface.depth * 0.008;
	[loop] for (int stepIndex = 0; stepIndex < 12; stepIndex++)
	{
		if (stepIndex >= count) break;
		float t = ((float)stepIndex + ign) / count;
		float3 position = surface.cameraSpacePos + direction * (0.04 + t * 0.85);
		if (position.z <= 0.01) break;
		float2 coord = convertCameraSpaceToScreenSpace(position);
		if (any(coord <= viewportOffset) || any(coord >= viewportOffset + viewportScale)) break;
		float depth = tex2Dlod(depthSampler, float4(coord, 0, 0)).r;
		float difference = position.z - depth;
		if (depth > 0.01 && difference > minThickness && difference < maxThickness)
			return saturate(1.0 - 0.60 * LightingStrength.y * (1.0 - t));
	}
	return 1.0;
}

// do basic surface color calculation here (without reflection)
float BlockSkyOpenness(SurfaceStruct surface)
{
	if (QualityLevel < 9.5f || BlockSkyVolume.w <= 0.0f || surface.mask_sky)
		return 1.0f;
	float3 world = mul(float4(surface.cameraSpacePos, 1.0f), matViewInverse).xyz;
	float3 p = (world - BlockSkyVolume.xyz) * BlockSkyVolume.w + surface.normal * 0.51f;
	float3 eye = (matViewInverse[3].xyz - BlockSkyVolume.xyz) * BlockSkyVolume.w;
	float3 distance = abs(p - eye);
	float weight = 1.0f - smoothstep(8.0f, 14.0f, max(distance.x, max(distance.y, distance.z)));
	if (any(p < 0.0f) || any(p >= 32.0f))
		return 1.0f;
	if (weight <= 0.0f)
		return 1.0f;
	float2 inTile = clamp(p.xz, 0.5f, 31.5f);
	float y = clamp(p.y - 0.5f, 0.0f, 31.0f);
	float lo = floor(y);
	float hi = min(lo + 1.0f, 31.0f);
	float2 tile0 = float2(fmod(lo, 8.0f), floor(lo / 8.0f));
	float2 tile1 = float2(fmod(hi, 8.0f), floor(hi / 8.0f));
	float a = tex2Dlod(blockSkySampler, float4((tile0 * 32.0f + inTile) / float2(256,128), 0, 0)).r;
	float b = tex2Dlod(blockSkySampler, float4((tile1 * 32.0f + inTile) / float2(256,128), 0, 0)).r;
	return lerp(1.0f, lerp(a, b, frac(y)), weight);
}

PSOut CompositePS0(VSOutput input)
{
	//Initialize surface properties required for lighting calculation for any surface that is not part of the sky
	SurfaceStruct surface = (SurfaceStruct)0;
	float2 texCoord = input.texCoord;
	surface.texCoord = texCoord;
	float3 rawColorSRGB = tex2D(colorSampler, texCoord).rgb;
	surface.albedo = pow(max(rawColorSRGB, 0.0f), 2.2f);
	surface.sky.albedo = surface.albedo;
	surface.albedo = DecodeSurfaceAlbedo(surface.albedo);
	surface.normal = GetNormal(texCoord).xyz;
	surface.depth = GetDepth(texCoord);
	surface.CameraEye = input.CameraEye;
	surface.cameraSpacePos = input.CameraEye * surface.depth;
	// r:category id,  g: sun light value, b: torch light value, a: natural terrain zig-zag blend code
	float4 block_info = tex2D(matInfoSampler, texCoord);
	surface.category_id = (int)(block_info.r * 255.0 + 0.4);
	float sun_light_strength = block_info.g;
	float torch_light_strength = block_info.b;
	CalculateMasks(surface);
#ifdef BLOCK_SURFACE_EFFECTS
	float selfBlendCode = floor(block_info.a * 255.0 + 0.5);
	float2 pixelPhase = frac(texCoord * screenParam);

	ApplyTerrainZigZagBlend(surface, rawColorSRGB, selfBlendCode);
	float borderCreaseAO = ApplyRoundedBlockBorders(surface, sun_light_strength, torch_light_strength, pixelPhase);
	CalculateMasks(surface);
	surface.albedo *= borderCreaseAO;
#endif
	float skyOpenness = BlockSkyOpenness(surface);
	if (SkyVisibilityView > 0.5f)
	{
		// Preserve the opaque measurement in MRT1 across the subsequent alpha pass.
		// Use display gray directly so a given visibility always has the same value.
		float gray = surface.mask_sky ? 0.15f : skyOpenness;
		PSOut measured;
		measured.Color = float4(gray, gray, gray, 1.0f);
		measured.Color2 = measured.Color;
		return measured;
	}

	//Remove the sky from surface albedo, because sky will be handled separately
	surface.albedo *= 1.0f - float(surface.mask_sky);
	//Initialize sky surface properties (solar disc + corona by day, textured moon + halo by night)
	surface.sky.albedo = surface.sky.albedo * float(surface.mask_sky);
	ComputeCelestialSky(surface);

	//Initialize original Lightmap values
	RawLightmapStruct rawLightmap;
	rawLightmap.torch = GetLightmapTorch(torch_light_strength);
	rawLightmap.sky = GetLightmapSky(sun_light_strength);

	//Calculate surface shading
	ShadingStruct shading;
	CalculateNdotL(surface);
	float directSunShading = CalculateDirectLighting(surface); //Calculate direct sunlight without visibility check (shadows)
	shading.direct = lerp(CloudThickness, 1.0, directSunShading);
	shading.sunlightVisibility = CalculateSunlightVisibility(surface, shading);
	shading.sunlightVisibility *= ContactVisibility(surface);
	if (surface.category_id == 255) {
		// Soften cast shadows on character faces/bodies so shadowed skin stays clean and readable
		shading.sunlightVisibility = lerp(0.50f, 1.0f, shading.sunlightVisibility);
	}
	shading.direct *= lerp(CloudThickness, 1.0, shading.sunlightVisibility);
	shading.direct *= 1.0f - rainStrength;
	shading.direct *= 1.0f - CloudThickness;
	shading.direct *= pow(rawLightmap.sky, 0.1f);
	shading.skylight = CalculateSkylight(surface);

	// Golden-hour warm transmittance by day + silvery directional moonlight by night
	float horizonWarmth = (1.0f - smoothstep(0.04f, 0.40f, saturate(sunDirection.y))) * saturate(1.0f - timeMidnight * 2.0f);
	float3 warmSunColor = SunColor.rgb * lerp(float3(1.0f, 1.0f, 1.0f), float3(1.20f, 0.84f, 0.58f), horizonWarmth);
	float moonLightStrength = smoothstep(0.08f, 0.55f, timeMidnight) * (1.0f - rainStrength) * (1.0f - CloudThickness);
	float3 moonDirectColor = float3(0.14f, 0.22f, 0.38f) * moonLightStrength;
	float3 activeCelestialColor = warmSunColor + moonDirectColor;

	//Colorize surface shading and store in lightmaps
	LightmapStruct lightmap = (LightmapStruct)0;
	lightmap.sunlight = activeCelestialColor * shading.direct;
	lightmap.skylight = rawLightmap.sky;
	lightmap.skylight *= shading.skylight;
	lightmap.skylight *= AmbientVisibility(surface) * lerp(0.40f, 1.0f, skyOpenness);
	//give some ambient sunlight plus some base ambient at night. 
	lightmap.skylight *= (SunColor.rgb*(1.0f - CloudThickness)*2.0 + lerp(1.0, 0.14f, pow(timeMidnight, 0.6)));
	// Subtle cool sky-dome chromaticity in daytime shadows for natural warm/cool sun-shadow separation
	float shadowCoolness = (1.0f - shading.sunlightVisibility) * saturate(1.0f - timeMidnight * 1.5f) * (1.0f - rainStrength) * 0.32f;
	lightmap.skylight *= lerp(float3(1.0f, 1.0f, 1.0f), float3(0.84f, 0.94f, 1.15f), shadowCoolness);

	// Warm ground-bounce GI on undersides & opposite-sun faces
	float bounceGeom = saturate(-surface.NdotL * 0.5f + 0.5f) * 0.65f + saturate(-surface.normal.y) * 0.35f;
	lightmap.bouncedSunlight = rawLightmap.sky * bounceGeom * warmSunColor * (0.065f * (1.0f - rainStrength) * (1.0f - CloudThickness));

	// also give torch light some fake shading to make custom model look more dimentional at night. 
	float fakeTorchShading = 1.0;
	if (surface.category_id == 255) {
		fakeTorchShading = (directSunShading * 0.6 + 0.4);
		// Keep character ambient skylight soft and clean so shadowed faces don't turn muddy
		lightmap.skylight *= (directSunShading * 0.2 + 0.8);
	}
	lightmap.torchlight = (rawLightmap.torch * fakeTorchShading) * TorchLightColor;

	//Apply lightmaps to albedo and generate final shaded surface
	FinalStruct final = (FinalStruct)0;
	final.sunlight = surface.albedo * (lightmap.sunlight + lightmap.bouncedSunlight);
	final.skylight = surface.albedo * lightmap.skylight;
	final.torchlight = surface.albedo * lightmap.torchlight;
	final.glow_torch = surface.albedo * (float(surface.mask_torch) * TorchLightColor);

#ifdef NIGHT_EYE_EFFECT
	float nightEyeAmount = timeMidnight*0.8;
	DoNightEye(final.sunlight, nightEyeAmount);
	DoNightEye(final.skylight, nightEyeAmount);
	DoNightEye(surface.sky.albedo, nightEyeAmount);
#endif

	float3 finalComposite;
	finalComposite = final.sunlight	    * 1.0f					//Add direct sunlight/moonlight + bounce GI
		+ final.skylight     * 0.05f					//Add ambient skylight
		+ final.torchlight   * 2.0					//Add light coming from emissive blocks
		+ final.glow_torch   * 3.0f					// add torch, lamp, lava glow
		;
	//Apply sky to final composite
	surface.sky.albedo += surface.sky.sunSpot;
	finalComposite += surface.sky.albedo;

	CalculateAtmosphericScattering(finalComposite, surface);

	// Convert final image back into gamma 0.45 space
	finalComposite.rgb = pow(finalComposite.rgb, (1.0f / 2.2f));

	PSOut o;
	o.Color = float4(finalComposite.rgb, 1.0);
	// r: sky occlusion mask (for god rays), g: sunlight visibility (for specular), b: raw sky lightmap
	o.Color2 = float4(float(surface.mask_sky), surface.shadow * pow(rawLightmap.sky, 0.2f), rawLightmap.sky, 1.0);
	return o;
}

float4 ComputeFakeSkyReflection(inout SurfaceStruct2 surface)
{
	float3 fogLinear = pow(max(g_FogColor.rgb, 0.0f), 2.2f);
	float3 skyTint = fogLinear * lerp(0.85f, 0.25f, timeMidnight);
	return float4(skyTint, 1.0f);
}

void CalculateSpecularReflections(inout SurfaceStruct2 surface)
{
	int category_id = surface.category_id;

	surface.mask_sky = surface.depth < 0.01;
	surface.mask_water = (category_id == 8 || category_id == 9);
	surface.mask_metal = (category_id == 50);

	if (RenderOptions.y > 0.5 && (surface.mask_water || surface.mask_metal))
	{
		// water is 1 (full), metal has a weak reflective color. 
		float specularity = lerp(1.0, surface.specularity, surface.mask_metal);
		float3 cameraSpaceNormal = mul(surface.normal, (float3x3)matView);
		float4 hit = float4(0,0,0,0);
		float3 tracedColor = surface.color.rgb;
		if (QualityLevel >= 10 && BlockReflectionEnabled > 0.5f)
		{
			hit = tex2D(blockReflectionSampler, surface.texCoord);
			if (hit.a > 0.0f)
			{
				float facing = saturate(dot(-normalize(surface.cameraSpacePos), normalize(cameraSpaceNormal)));
				float fresnel = pow(1.0f - facing, 3.0f);
				float strength = surface.mask_water ? lerp(0.36f, 0.92f, fresnel)
					: lerp(0.12f + 0.10f * saturate(specularity), 0.44f, fresnel);
				tracedColor = lerp(surface.color.rgb, pow(hit.rgb, 2.2f), strength);
				if (hit.a >= 1.0f)
				{
					surface.color.rgb = tracedColor;
					return;
				}
			}
		}
		float4 reflection = ComputeRayTraceReflection(surface.cameraSpacePos, cameraSpaceNormal);
		float4 fakeSkyReflection = ComputeFakeSkyReflection(surface);

		// When SSR misses off-screen sky on water, blend subtle horizon sky reflection
		float3 reflectedColor = lerp(fakeSkyReflection.rgb, reflection.rgb, saturate(reflection.a * 1.5f));
		float reflectBlend = max(reflection.a, surface.mask_water ? 0.28f : 0.0f) * specularity;

#define USE_WATER_REFLECTION_FRESNEL_EFFECT
#ifdef USE_WATER_REFLECTION_FRESNEL_EFFECT
		float3 cameraSpaceViewDir = -normalize(surface.cameraSpacePos);
		const float fresnelPower = 3.0;
		float fresnel = pow(saturate(1.0 - dot(cameraSpaceViewDir, cameraSpaceNormal)), fresnelPower) * 0.98 + 0.02;
		reflectBlend *= fresnel;
#endif
		float3 fallback = lerp(surface.color.xyz, reflectedColor, reflectBlend);
		surface.color.xyz = lerp(fallback, tracedColor, hit.a);
	}
}


/** sunlight & moonlight specularity based on celestial and eye half vector, including glittering water path */
void CalculateSpecularHighlight(inout SurfaceStruct2 surface)
{
	if (!surface.mask_sky)
	{
		float3 cameraSpaceViewDir = -normalize(surface.cameraSpacePos);
		float3 cameraSpaceNormal = normalize(mul(surface.normal, (float3x3)matView));
		float3 celestialDirVS = normalize(mul(sunDirection, (float3x3)matView).xyz);
		float3 halfVector = normalize(celestialDirVS + cameraSpaceViewDir);
		float HdotN = saturate(dot(halfVector, cameraSpaceNormal));

		float moonSpecFactor = smoothstep(0.08f, 0.55f, timeMidnight) * (1.0f - CloudThickness);
		float3 celestialSpecColor = SunColor + float3(0.22f, 0.34f, 0.58f) * moonSpecFactor;

		if (surface.mask_water)
		{
			// Shimmering sun & moon path on water, masked by real-time PCSS shadows
			float waterFresnel = pow(saturate(1.0f - dot(cameraSpaceViewDir, cameraSpaceNormal)), 4.0f) * 0.85f + 0.15f;
			float tightGlitter = pow(HdotN, 220.0f) * 2.4f;
			float wideShimmer  = pow(HdotN, 28.0f) * 0.20f;
			float waterVis = surface.sunlightVisibility * (1.0f - rainStrength) * (1.0f - CloudThickness * 0.85f);
			surface.color.xyz += (tightGlitter + wideShimmer) * waterFresnel * waterVis * celestialSpecColor;
		}
		else
		{
			// everything has some specular light, surface.mask_metal has more
			float gloss = surface.specularity + 0.02; // 0.02 is base specular for all blocks.

			const float fresnelPower = 6.0;
			float fresnel = pow(saturate(1.0 - dot(cameraSpaceViewDir, cameraSpaceNormal)), fresnelPower) * 0.98 + 0.02;
			float spec = pow(HdotN, gloss * 30); 
			spec *= fresnel * gloss * surface.sunlightVisibility * (1.0 - rainStrength);
			float3 specularHighlight = spec * celestialSpecColor;

			// For fake torch specular light, we will assume torch light and eye are on the same point, so half vector is actually viewDir.
			if (surface.mask_metal || surface.specularity > 0.01)
			{
				float spec = pow(saturate(dot(cameraSpaceViewDir, cameraSpaceNormal)), 60.0);
				specularHighlight += (TorchLightColor.rgb*pow(surface.torch_light_strength*spec, 3.0));
			}
			surface.color.xyz += specularHighlight;
		}
	}
}

// Screen-space crepuscular sun & moon shafts (god rays) occluded by trees, terrain, and structures
void CalculateCrepuscularLightShafts(inout SurfaceStruct2 surface)
{
	float clearSky = (1.0f - rainStrength) * (1.0f - CloudThickness * 0.9f);
	if (clearSky <= 0.02f || LightingStrength.z <= 0) return;

	float3 sunDirVS = normalize(mul(sunDirection, (float3x3)matView).xyz);
	if (sunDirVS.z <= 0.10f) return;

	float3 viewRayVS = normalize(surface.CameraEye);
	float cosTheta = saturate(dot(viewRayVS, sunDirVS));
	float directionalPhase = pow(cosTheta, 5.0f);
	if (directionalPhase <= 0.002f) return;

	float2 sunScreenUV = convertCameraSpaceToScreenSpace(sunDirVS * 100.0f);
	float2 deltaUV = sunScreenUV - surface.texCoord;
	float distUV = length(deltaUV);
	deltaUV *= min(1.0f, 0.42f / max(distUV, 0.0001f));

	int shaftSteps = QualityLevel >= 9 ? 24 : (QualityLevel >= 5 ? 12 : 8);
	float fSteps = (float)shaftSteps;
	float ign = InterleavedGradientNoise(surface.texCoord * screenParam);

	float occlusionSum = 0.0f;
	[loop] for (int s = 0; s < shaftSteps; s++)
	{
		float t = ((float)s + ign) / fSteps;
		float2 sampleUV = surface.texCoord + deltaUV * t;
		if (all(sampleUV > viewportOffset) && all(sampleUV < viewportOffset + viewportScale))
		{
			// ShadowMapSampler.r holds mask_sky written in CompositePS0
			float isSky = tex2Dlod(ShadowMapSampler, float4(sampleUV, 0.0f, 0.0f)).r;
			occlusionSum += isSky * (1.0f - t * 0.35f);
		}
	}
	float shaftFactor = (occlusionSum / fSteps) * directionalPhase * clearSky;
	// Stronger shafts across foreground geometry, subtle atmospheric haze on open sky
	shaftFactor *= surface.mask_sky ? 0.20f : 1.0f;

	float horizonBoost = 1.0f - smoothstep(0.05f, 0.45f, saturate(sunDirection.y));
	float3 dayShaftColor = SunColor * lerp(float3(1.0f, 0.95f, 0.85f), float3(1.30f, 0.75f, 0.40f), horizonBoost) * (0.20f + 0.25f * horizonBoost) * saturate(1.0f - timeMidnight * 1.5f);
	float3 nightShaftColor = float3(0.16f, 0.28f, 0.50f) * (0.14f * smoothstep(0.08f, 0.55f, timeMidnight));

	surface.color.xyz += shaftFactor * LightingStrength.z * (dayShaftColor + nightShaftColor);
}

// Gather outgoing radiance from nearby, mutually facing surfaces in the lit opaque buffer.
float3 DiffuseBounce(SurfaceStruct2 surface)
{
	if (BounceEnabled < 0.5 || surface.mask_sky || surface.mask_water || surface.depth > 35.0) return 0;
	float3 normal = normalize(mul(surface.normal, (float3x3)matView));
	float2 radius = viewportScale * float2(1.0 / ViewAspect, 1.0) * (0.85 / max(surface.depth * TanHalfFOV, 0.05));
	int count = QualityLevel >= 9 ? 16 : 8;
	float3 irradiance = 0;
	[loop] for (int i = 0; i < count; ++i)
	{
		float angle = i * 2.399963;
		float2 uv = surface.texCoord + radius * float2(cos(angle), sin(angle)) * sqrt((i + 0.5) / count);
		if (any(uv != ClampViewUV(uv))) continue;
		float depth = tex2Dlod(depthSampler, float4(uv, 0, 0)).r;
		float3 delta = ViewPosition(uv, depth) - surface.cameraSpacePos;
		float distSq = dot(delta, delta);
		if (depth <= 0.01 || distSq < 0.0004 || distSq > 4.0) continue;
		float3 dir = delta * rsqrt(distSq);
		float3 emitter = tex2Dlod(normalSampler, float4(uv, 0, 0)).rgb * 2.0 - 1.0;
		float weight = saturate(dot(normal, dir)) * saturate(dot(mul(emitter, (float3x3)matView), -dir));
		float3 radiance = pow(max(tex2Dlod(colorSampler, float4(uv, 0, 0)).rgb, 0), 2.2);
		irradiance += min(radiance, 4.0) * weight * (1.0 - distSq * 0.25);
	}
	return irradiance * (1.6 / count) * (1.0 - smoothstep(24.0, 35.0, surface.depth));
}

// do water reflection, specular & crepuscular shafts here
float4 CompositePS1(VSOutput input) :COLOR
{
	SurfaceStruct2 surface = (SurfaceStruct2)0;
	float2 texCoord = input.texCoord;
	surface.texCoord = texCoord;
	surface.color = GetAlbedoLinear(texCoord);
	surface.sunlightVisibility = GetSunlightVisibility(texCoord);
	// get world space normal
	float4 normal_ = GetNormal(texCoord);
	surface.normal = normal_.xyz;
	surface.specularity = 1.0 - normal_.w;
	surface.depth = GetDepth(texCoord);
	surface.CameraEye = input.CameraEye;
	surface.cameraSpacePos = input.CameraEye * surface.depth;
	// r:category id,  g: sun light value, b: torch light value
	float4 block_info = tex2D(matInfoSampler, texCoord);
	surface.category_id = (int)(block_info.r * 255.0 + 0.4);
	// float sun_light_strength = block_info.g;
	surface.torch_light_strength = block_info.b;

	CalculateSpecularReflections(surface);
	surface.color += DiffuseBounce(surface) * sqrt(saturate(surface.color));
	CalculateSpecularHighlight(surface);
	CalculateCrepuscularLightShafts(surface);

	return float4(surface.color.rgb, 1.0);
}

// calculate bloom at given level of detail and write it to a given offset position
float3 CalculateBloom(float2 texcoord, int LOD, float2 offset)
{
	float scale = pow(2.0f, float(LOD));

	float padding = 0.02f;

	if (texcoord.x - offset.x + padding < 1.0f / scale + (padding * 2.0f)
		&& texcoord.y - offset.y + padding < 1.0f / scale + (padding * 2.0f)
		&& texcoord.x - offset.x + padding > 0.0f
		&&  texcoord.y - offset.y + padding > 0.0f) {

		float3 bloom = float1(0.0f).xxx;
		float allWeights = 0.0f;
		const float3 glowThreshold = float3(1.0, 1.0, 1.0);
		float kernelSize = QualityLevel == 3 ? 4.0 : 6.0;
		[loop] for (int i = 0; i < 6; i++) {
			if (i >= kernelSize) break;
			[loop] for (int j = 0; j < 6; j++) {
				if (j >= kernelSize) break;
				float2 kernelPosition = float2(i, j) * (5.0 / (kernelSize - 1.0));
				float weight = 1.0f - distance(kernelPosition, float2(2.5f, 2.5f)) / 3.5;
				weight = 1.0f - cos(weight * 3.1416f / 2.0f);
				weight = pow(weight, 2.0f);
				float2 coord = kernelPosition - 2.5;
				coord /= screenParam;

				float2 finalCoord = (texcoord.xy + coord.xy - offset.xy) * scale;
				// glow threshold is set to 
				float3 sampleColor = tex2Dlod(colorSampler, float4(finalCoord, 0, 0)).rgb;
				float3 bright = max(float3(0, 0, 0), sampleColor - glowThreshold);
				if (QualityLevel >= 5) {
					float brightness = max(sampleColor.r, max(sampleColor.g, sampleColor.b));
					float knee = clamp(brightness - 0.5, 0.0, 1.0);
					float contribution = max(brightness - 1.0, knee * knee * 0.5);
					bright = sampleColor * contribution / max(brightness, 0.0001);
				}
				bloom += bright * weight;
				allWeights += weight;
			}
		}
		bloom /= allWeights;
		return bloom;
	}
	else {
		return float1(0.0f).xxx;
	}

}

// calculate bloom texture (several bloom texture resolutions are calculated in one pass in different locations of the texture) 
float4 CompositePS2(VSOutput input) :COLOR
{
	float2 texCoord = input.texCoord;

	float3 bloom = CalculateBloom(texCoord, 2, float2(0.0f, 0.0f) + float2(0.000f, 0.000f));
	bloom += CalculateBloom(texCoord, 3, float2(0.0f, 0.25f) + float2(0.000f, 0.025f));
	bloom += CalculateBloom(texCoord, 4, float2(0.125f, 0.25f) + float2(0.025f, 0.025f));
	bloom += CalculateBloom(texCoord, 5, float2(0.1875f, 0.25f) + float2(0.050f, 0.025f));
	bloom += CalculateBloom(texCoord, 6, float2(0.21875f, 0.25f) + float2(0.075f, 0.025f));
	bloom += CalculateBloom(texCoord, 7, float2(0.25f, 0.25f) + float2(0.100f, 0.025f));
	bloom += CalculateBloom(texCoord, 8, float2(0.28f, 0.25f) + float2(0.125f, 0.025f));

	return float4(bloom.rgb, 1.0f);
}


/* the final composition step to tonemap to monitor resolution.
*/
struct VSOutputFinal
{
	float4 pos			: POSITION;         // Screen space position
	float2 texCoord		: TEXCOORD0;        // texture coordinates
};

// down size to 1/4 of original size. Calculate average color.
float4 GlowDownsizePS(VSOutputFinal input) :COLOR
{
	float2 texcoord = input.texCoord;
	float2 texStep = float2(1.0, 1.0) / screenParam;
	float3 bloom = float3(0.0, 0.0, 0.0);
	for (int i = 0; i < 4; i++) {
		for (int j = 0; j < 4; j++) {
			float2 coord = float2(i, j) * texStep;
			float2 finalCoord = (texcoord.xy + coord.xy);
			bloom += tex2D(colorSampler, finalCoord).rgb;
		}
	}
	return float4(bloom.rgb / 16.0, 1.0f);
}

VSOutputFinal FinalQuadVS(float3 iPosition:POSITION,
	float2 texCoord : TEXCOORD0)
{
	VSOutputFinal o;
	o.pos = float4(iPosition, 1);
	o.texCoord = texCoord + 0.5 / screenParam;
	return o;
}

// get and convert to linear color space (decode gamma)
float3 	GetColorTexture(float2 coord)
{
	return tex2D(compositeSampler, coord).rgb;
}

// value based on SEUS v10.0
float3 DepthOfField(float2 pos)
{
	float cursorDepth = centerDepthSmooth;
	if (cursorDepth == 0.0) {
		// just in case it is first person view. 
		cursorDepth = tex2D(depthSampler, viewportOffset + viewportScale * 0.5).x;
	}

	float blurclamp = MaxDOFBlur;

	float2 aspectcorrect = float2(1.0, ViewAspect) * 1.5;

	float depth = tex2D(depthSampler, pos).x;
	// depth += float(isHand) * 0.36f;

	float factor = (depth - cursorDepth) / cameraFarPlane;

	float2 dofblur = clamp(factor * DepthOfViewFactor, -blurclamp, blurclamp).xx;
	if (QualityLevel < 6 && abs(dofblur.x) * screenParam.y < 0.25) return GetColorTexture(pos);
	if (QualityLevel >= 6) {
		if (depth < 0.01) depth = cameraFarPlane;
		if (cursorDepth < 0.01) cursorDepth = cameraFarPlane;
		float blur = clamp((depth - cursorDepth) / max(depth, 0.1) * DepthOfViewFactor, -blurclamp, blurclamp);
		float3 accumulated = GetColorTexture(pos);
		float totalWeight = 1.0;
		[loop] for (int sampleIndex = 0; sampleIndex < 32; sampleIndex++) {
			float angle = sampleIndex * 2.399963;
			float2 offset = float2(cos(angle), sin(angle)) * sqrt((sampleIndex + 0.5) / 32.0);
			float2 coord = ClampViewUV(pos + offset * aspectcorrect * blur * viewportScale);
			float neighborDepth = tex2Dlod(depthSampler, float4(coord, 0, 0)).r;
			if (neighborDepth < 0.01) neighborDepth = cameraFarPlane;
			float weight = exp(-abs(neighborDepth - depth) / max(0.25, depth * 0.05));
			accumulated += tex2Dlod(compositeSampler, float4(coord, 0, 0)).rgb * weight;
			totalWeight += weight;
		}
		return accumulated / totalWeight;
	}

	float3 col = float3(0.0, 0.0, 0.0);
	col += GetColorTexture(pos);

	col += GetColorTexture(pos + (float2(0.0, 0.4)*aspectcorrect) * dofblur);
	col += GetColorTexture(pos + (float2(0.15, 0.37)*aspectcorrect) * dofblur);
	col += GetColorTexture(pos + (float2(0.29, 0.29)*aspectcorrect) * dofblur);
	col += GetColorTexture(pos + (float2(-0.37, 0.15)*aspectcorrect) * dofblur);
	col += GetColorTexture(pos + (float2(0.4, 0.0)*aspectcorrect) * dofblur);
	col += GetColorTexture(pos + (float2(0.37, -0.15)*aspectcorrect) * dofblur);
	col += GetColorTexture(pos + (float2(0.29, -0.29)*aspectcorrect) * dofblur);
	col += GetColorTexture(pos + (float2(-0.15, -0.37)*aspectcorrect) * dofblur);
	col += GetColorTexture(pos + (float2(0.0, -0.4)*aspectcorrect) * dofblur);
	col += GetColorTexture(pos + (float2(-0.15, 0.37)*aspectcorrect) * dofblur);
	col += GetColorTexture(pos + (float2(-0.29, 0.29)*aspectcorrect) * dofblur);
	col += GetColorTexture(pos + (float2(0.37, 0.15)*aspectcorrect) * dofblur);
	col += GetColorTexture(pos + (float2(-0.4, 0.0)*aspectcorrect) * dofblur);
	col += GetColorTexture(pos + (float2(-0.37, -0.15)*aspectcorrect) * dofblur);
	col += GetColorTexture(pos + (float2(-0.29, -0.29)*aspectcorrect) * dofblur);
	col += GetColorTexture(pos + (float2(0.15, -0.37)*aspectcorrect) * dofblur);

	col += GetColorTexture(pos + (float2(0.15, 0.37)*aspectcorrect) * dofblur*0.9);
	col += GetColorTexture(pos + (float2(-0.37, 0.15)*aspectcorrect) * dofblur*0.9);
	col += GetColorTexture(pos + (float2(0.37, -0.15)*aspectcorrect) * dofblur*0.9);
	col += GetColorTexture(pos + (float2(-0.15, -0.37)*aspectcorrect) * dofblur*0.9);
	col += GetColorTexture(pos + (float2(-0.15, 0.37)*aspectcorrect) * dofblur*0.9);
	col += GetColorTexture(pos + (float2(0.37, 0.15)*aspectcorrect) * dofblur*0.9);
	col += GetColorTexture(pos + (float2(-0.37, -0.15)*aspectcorrect) * dofblur*0.9);
	col += GetColorTexture(pos + (float2(0.15, -0.37)*aspectcorrect) * dofblur*0.9);

	col += GetColorTexture(pos + (float2(0.29, 0.29)*aspectcorrect) * dofblur*0.7);
	col += GetColorTexture(pos + (float2(0.4, 0.0)*aspectcorrect) * dofblur*0.7);
	col += GetColorTexture(pos + (float2(0.29, -0.29)*aspectcorrect) * dofblur*0.7);
	col += GetColorTexture(pos + (float2(0.0, -0.4)*aspectcorrect) * dofblur*0.7);
	col += GetColorTexture(pos + (float2(-0.29, 0.29)*aspectcorrect) * dofblur*0.7);
	col += GetColorTexture(pos + (float2(-0.4, 0.0)*aspectcorrect) * dofblur*0.7);
	col += GetColorTexture(pos + (float2(-0.29, -0.29)*aspectcorrect) * dofblur*0.7);
	col += GetColorTexture(pos + (float2(0.0, 0.4)*aspectcorrect) * dofblur*0.7);

	col += GetColorTexture(pos + (float2(0.29, 0.29)*aspectcorrect) * dofblur*0.4);
	col += GetColorTexture(pos + (float2(0.4, 0.0)*aspectcorrect) * dofblur*0.4);
	col += GetColorTexture(pos + (float2(0.29, -0.29)*aspectcorrect) * dofblur*0.4);
	col += GetColorTexture(pos + (float2(0.0, -0.4)*aspectcorrect) * dofblur*0.4);
	col += GetColorTexture(pos + (float2(-0.29, 0.29)*aspectcorrect) * dofblur*0.4);
	col += GetColorTexture(pos + (float2(-0.4, 0.0)*aspectcorrect) * dofblur*0.4);
	col += GetColorTexture(pos + (float2(-0.29, -0.29)*aspectcorrect) * dofblur*0.4);
	col += GetColorTexture(pos + (float2(0.0, 0.4)*aspectcorrect) * dofblur*0.4);

	float3 color = col / 41;
	return color;
}

struct BloomDataStruct
{
	float3 blur0;
	float3 blur1;
	float3 blur2;
	float3 blur3;
	float3 blur4;
	float3 blur5;
	float3 blur6;
	float3 bloom;
};

// Retrieve previously calculated bloom textures
void GetBloom(float2 texcoord, inout BloomDataStruct bloomData) {
	//constants for bloom bloomSlant
	const float    bloomSlant = 0.25f;
	const float bloomWeight[7] = { pow(7.0f, bloomSlant),
		pow(6.0f, bloomSlant),
		pow(5.0f, bloomSlant),
		pow(4.0f, bloomSlant),
		pow(3.0f, bloomSlant),
		pow(2.0f, bloomSlant),
		1.0f
	};

	float2 recipres = float2(1.0, 1.0) / screenParam;
	texcoord -= recipres;

	bloomData.blur0 = tex2D(colorSampler, (texcoord.xy) * (1.0f / pow(2.0f, 2.0f)) + float2(0.0f, 0.0f) + float2(0.000f, 0.000f)).rgb;
	bloomData.blur1 = tex2D(colorSampler, (texcoord.xy) * (1.0f / pow(2.0f, 3.0f)) + float2(0.0f, 0.25f) + float2(0.000f, 0.025f)).rgb;
	bloomData.blur2 = tex2D(colorSampler, (texcoord.xy) * (1.0f / pow(2.0f, 4.0f)) + float2(0.125f, 0.25f) + float2(0.025f, 0.025f)).rgb;
	bloomData.blur3 = tex2D(colorSampler, (texcoord.xy) * (1.0f / pow(2.0f, 5.0f)) + float2(0.1875f, 0.25f) + float2(0.050f, 0.025f)).rgb;
	bloomData.blur4 = tex2D(colorSampler, (texcoord.xy) * (1.0f / pow(2.0f, 6.0f)) + float2(0.21875f, 0.25f) + float2(0.075f, 0.025f)).rgb;
	bloomData.blur5 = tex2D(colorSampler, (texcoord.xy) * (1.0f / pow(2.0f, 7.0f)) + float2(0.25f, 0.25f) + float2(0.100f, 0.025f)).rgb;
	bloomData.blur6 = tex2D(colorSampler, (texcoord.xy) * (1.0f / pow(2.0f, 8.0f)) + float2(0.28f, 0.25f) + float2(0.125f, 0.025f)).rgb;

	bloomData.bloom = bloomData.blur0 * bloomWeight[0];
	bloomData.bloom += bloomData.blur1 * bloomWeight[1];
	bloomData.bloom += bloomData.blur2 * bloomWeight[2];
	bloomData.bloom += bloomData.blur3 * bloomWeight[3];
	bloomData.bloom += bloomData.blur4 * bloomWeight[4];
	bloomData.bloom += bloomData.blur5 * bloomWeight[5];
	bloomData.bloom += bloomData.blur6 * bloomWeight[6];
}

/** blur distance scene if raining. */
void AddRainFogScatter(float2 texcoord, inout float3 color, in BloomDataStruct bloomData)
{
	const float    bloomSlant = 0.0f;
	const float bloomWeight[7] = { pow(7.0f, bloomSlant),
		pow(6.0f, bloomSlant),
		pow(5.0f, bloomSlant),
		pow(4.0f, bloomSlant),
		pow(3.0f, bloomSlant),
		pow(2.0f, bloomSlant),
		1.0f
	};

	float3 fogBlur = bloomData.blur0 * bloomWeight[6] +
		bloomData.blur1 * bloomWeight[5] +
		bloomData.blur2 * bloomWeight[4] +
		bloomData.blur3 * bloomWeight[3] +
		bloomData.blur4 * bloomWeight[2] +
		bloomData.blur5 * bloomWeight[1] +
		bloomData.blur6 * bloomWeight[0];

	float fogTotalWeight = 1.0f * bloomWeight[0] +
		1.0f * bloomWeight[1] +
		1.0f * bloomWeight[2] +
		1.0f * bloomWeight[3] +
		1.0f * bloomWeight[4] +
		1.0f * bloomWeight[5] +
		1.0f * bloomWeight[6];

	fogBlur /= fogTotalWeight;

	float linearDepth = GetDepth(texcoord);

	float fogDensity = 0.023f * (rainStrength);
	float visibility = 1.0f / exp(linearDepth * fogDensity);
	float fogFactor = 1.0f - visibility;
	fogFactor = clamp(fogFactor, 0.0f, 1.0f);
	color = lerp(color, fogBlur, fogFactor);
}

float3 TonemapReinhard_Good(float3 color)
{
	if (QualityLevel >= 5) {
		float3 exposed = pow(max(color, 0.0), max(EyeContrast * 2.0, 0.01)) * EyeBrightness * 10.0;
		float3 mapped = saturate((exposed * (2.51 * exposed + 0.03)) / (exposed * (2.43 * exposed + 0.59) + 0.14));
		return pow(mapped, 1.0 / 2.2);
	}
	const float gamma = 2.2f;

	float3 value = pow(color.rgb, EyeContrast.x * 2);

	// exposure tone mapping
	// mapping brightness region 0-1 to exposure region 0-10
	float exposure = EyeBrightness.x * 10;
	value = 1.0f - exp(-value * exposure);

	// reinhard tone mapping
	//value = value / (value + 1.0f);

	// gamma correction
	color.rgb = pow(value, (1.0f / gamma));

	return color;
}

void Vignette(inout float3 color, float2 pos)
{
	float dist = distance(pos, float2(0.5, 0.5)) * 2.0f / 1.5142;
	dist = pow(dist, 1.1f);
	color.rgb *= 1.0 - dist;
}

float4 FinalPS(VSOutput input) :COLOR
{
	float2 texCoord = input.texCoord;
	float3 color;
	if (RenderOptions.z >= 2)
		color = DepthOfField(texCoord);
	else
		color = GetColorTexture(texCoord);

	// add bloom 
	BloomDataStruct bloomData = (BloomDataStruct)0;
	if (BloomEnabled > 0.5f || rainStrength > 0.01f)
		GetBloom(texCoord, bloomData);
	if (BloomEnabled > 0.5f)
		color += bloomData.bloom * (0.006f * LightingStrength.w);

	if (rainStrength > 0.01f)
		AddRainFogScatter(texCoord, color, bloomData);

	// apply user defined fog
	float eyeDist = length(input.CameraEye * GetDepth(texCoord));
	if (FogStart < FogEnd)
		color.xyz = lerp(color.xyz, pow(g_FogColor.xyz, 2.2f), 1.0 - clamp((FogEnd - eyeDist) / (FogEnd - FogStart), 0.0, 1.0));

	// vignette effect: darken edges
	//Vignette(color, texCoord);

#ifdef SHOW_DEBUG_VIEW
	color = lerp(lerp(bloomData.bloom.rgb, color, float(texCoord.y<0.5)), lerp(tex2D(compositeSampler, texCoord).rgb, tex2D(colorSampler, texCoord).rgb, float(texCoord.y<0.5)), float(texCoord.x<0.5)); // DEBUG: show bloom texture and result
#endif

	color = TonemapReinhard_Good(color);
	if (any(PhotoGrade != 0.0f))
	{
		float luma = dot(color, float3(0.2126f, 0.7152f, 0.0722f));
		color = lerp(luma.xxx, color, 1.0f + PhotoGrade.y);
		float midtones = 4.0f * saturate(luma) * (1.0f - saturate(luma));
		color += midtones * PhotoGrade.x * float3(0.14f, 0.025f, -0.16f);
		float3 tone = lerp(float3(-0.14f, 0.045f, 0.12f), float3(0.16f, 0.035f, -0.13f), smoothstep(0.2f, 0.8f, luma));
		color += midtones * PhotoGrade.w * tone;
		color = saturate(color * (1.0f - 0.18f * PhotoGrade.z) + 0.13f * PhotoGrade.z);
	}

	return float4(color, 1.0f);
}

float4 CompositeFXAA(VSOutput input) :COLOR
{
	float4 filtered = FxaaPixelShader(
	input.texCoord,							// FxaaFloat2 pos,
		FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f),		// FxaaFloat4 fxaaConsolePosPos,
		colorSampler,							// FxaaTex tex,
		colorSampler,							// FxaaTex fxaaConsole360TexExpBiasNegOne,
		colorSampler,							// FxaaTex fxaaConsole360TexExpBiasNegTwo,
		1.0 / screenParam,							// FxaaFloat2 fxaaQualityRcpFrame,
		FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f),		// FxaaFloat4 fxaaConsoleRcpFrameOpt,
		FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f),		// FxaaFloat4 fxaaConsoleRcpFrameOpt2,
		FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f),		// FxaaFloat4 fxaaConsole360RcpFrameOpt2,
		0.75f,									// FxaaFloat fxaaQualitySubpix,
		0.166f,									// FxaaFloat fxaaQualityEdgeThreshold,
		0.0833f,								// FxaaFloat fxaaQualityEdgeThresholdMin,
		0.0f,									// FxaaFloat fxaaConsoleEdgeSharpness,
		0.0f,									// FxaaFloat fxaaConsoleEdgeThreshold,
		0.0f,									// FxaaFloat fxaaConsoleEdgeThresholdMin,
		FxaaFloat4(0.0f, 0.0f, 0.0f, 0.0f)		// FxaaFloat fxaaConsole360ConstDir,
		);
	// FXAA returns edge luminance in alpha. The final scene is opaque.
	return float4(filtered.rgb, 1.0f);
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
		PixelShader = compile ps_3_0 CompositePS0();
	}
	pass P1
	{
		cullmode = none;
		ZEnable = false;
		ZWriteEnable = false;
		FogEnable = False;
		VertexShader = compile vs_3_0 CompositeQuadVS();
		PixelShader = compile ps_3_0 CompositePS1();
	}
	pass P2
	{
		cullmode = none;
		ZEnable = false;
		ZWriteEnable = false;
		FogEnable = False;
		VertexShader = compile vs_3_0 CompositeQuadVS();
		PixelShader = compile ps_3_0 CompositePS2();
	}
	pass P3
	{
		cullmode = none;
		ZEnable = false;
		ZWriteEnable = false;
		FogEnable = False;
		VertexShader = compile vs_3_0 CompositeQuadVS();
		PixelShader = compile ps_3_0 FinalPS();
	}
	pass P4
	{
		cullmode = none;
		ZEnable = false;
		ZWriteEnable = false;
		FogEnable = False;
		VertexShader = compile vs_3_0 FinalQuadVS();
		PixelShader = compile ps_3_0 GlowDownsizePS();
	}
	pass P5
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

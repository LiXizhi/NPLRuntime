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
float EyeBrightness = 0.2f;
float EyeContrast = 0.5f;
float BloomEnabled = 1.0f;
float QualityLevel = 4.0f;
float MaxDOFBlur = 0.014f;
float ViewAspect;
float TanHalfFOV;
float cameraFarPlane;
float3 cameraPosition;
float3 sunDirection;
float3 SunColor;
float3 TorchLightColor;
float timeNoon = 1.0; // 1 is noon. 0 is night
float timeMidnight = 0.0; // 1 is midnight.
float DepthOfViewFactor = 0.15;	//aperture - bigger values for shallower depth of field
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
	MinFilter = Linear;
	MagFilter = Linear;
	MipFilter = None;
	AddressU = BORDER;
	AddressV = BORDER;
	BorderColor = 0x0;
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
	int max_step = QualityLevel >= 6 ? 48 : (QualityLevel >= 5 ? 24 : 12);
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
				int refinements = QualityLevel >= 6 ? 4 : 2;
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
	
	// only apply global sun shadows when there is enough sun light on the material
	if (shading.direct > 0.0f)
	{
		// reconstruct world space vector
		float4 vWorldPosition = float4(surface.cameraSpacePos, 1);
		vWorldPosition = mul(vWorldPosition, matViewInverse);

		float shadowBias = lerp(0.0035f, 0.0012f, saturate(surface.NdotL));
		surface.shadow = calculateShadowFactorWithBias(ShadowMapSampler, vWorldPosition, surface.depth, mShadowMapTex, mShadowMapViewProj, ShadowMapSize.x, ShadowMapSize.y, ShadowRadius, shadowBias);
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

//circular sun
// return 1 is sun, 0 is not sun. 
bool CalculateSunspot(inout SurfaceStruct surface)
{
	float3 npos = normalize(surface.CameraEye);
	float3 halfVector2 = normalize(-mul(sunDirection, (float3x3)matView).xyz + npos);
	float sunProximity = 1.0f - dot(halfVector2, npos);
	surface.sky.sunProximity = sunProximity;
	return (sunProximity > 0.96f);
}

void AddSunglow(inout SurfaceStruct surface)
{
	float sunglowFactor = pow(surface.sky.sunProximity, 4.4);
	surface.sky.albedo *= 1.0f + sunglowFactor * (7.0f);
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
	float2 ndc = (coord - viewportOffset) / viewportScale * 2.0 - 1.0;
	return float3(ndc.x * TanHalfFOV * ViewAspect, -ndc.y * TanHalfFOV, 1.0) * depth;
}

float AmbientVisibility(SurfaceStruct surface)
{
	if (QualityLevel < 5 || surface.mask_sky || surface.mask_water) return 1.0;
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
	return saturate(1.0 - total * 2.0 / count);
}

float ContactVisibility(SurfaceStruct surface)
{
	if (QualityLevel < 5 || RenderOptions.x < 0.5 || surface.mask_sky || surface.mask_water || surface.category_id == 255 || surface.NdotL <= 0 || rainStrength >= 0.99) return 1.0;
	float3 direction = normalize(mul(sunDirection, (float3x3)matView));
	float count = QualityLevel >= 6 ? 12.0 : 6.0;
	[loop] for (int stepIndex = 1; stepIndex <= 12; stepIndex++)
	{
		if (stepIndex > count) break;
		float3 position = surface.cameraSpacePos + direction * (0.06 + stepIndex * 0.9 / count);
		if (position.z <= 0.01) break;
		float2 coord = convertCameraSpaceToScreenSpace(position);
		if (any(coord <= viewportOffset) || any(coord >= viewportOffset + viewportScale)) break;
		float depth = tex2Dlod(depthSampler, float4(coord, 0, 0)).r;
		float difference = position.z - depth;
		if (depth > 0.01 && difference > 0.03 && difference < 0.12)
			return lerp(0.35, 1.0, stepIndex / count);
	}
	return 1.0;
}

// do basic surface color calculation here (without reflection)
PSOut CompositePS0(VSOutput input)
{
	//Initialize surface properties required for lighting calculation for any surface that is not part of the sky
	SurfaceStruct surface = (SurfaceStruct)0;
	float2 texCoord = input.texCoord;
	surface.texCoord = texCoord;
	surface.albedo = GetAlbedoLinear(texCoord);
	surface.sky.albedo = surface.albedo;
	surface.albedo = pow(surface.albedo, 1.4f);
	surface.albedo = lerp(surface.albedo, dot(surface.albedo, float1(0.3333f).xxx), 0.035f);
	surface.normal = GetNormal(texCoord);
	surface.depth = GetDepth(texCoord);
	surface.CameraEye = input.CameraEye;
	surface.cameraSpacePos = input.CameraEye * surface.depth;
	// r:category id,  g: sun light value, b: torch light value
	float4 block_info = tex2D(matInfoSampler, texCoord);
	surface.category_id = (int)(block_info.r * 255.0 + 0.4);
	float sun_light_strength = block_info.g;
	float torch_light_strength = block_info.b;

	CalculateMasks(surface);

	//Remove the sky from surface albedo, because sky will be handled separately
	surface.albedo *= 1.0f - float(surface.mask_sky);
	//Initialize sky surface properties
	surface.sky.albedo = surface.sky.albedo * float(surface.mask_sky); //Gets the albedo texture for the sky
	surface.sky.sunSpot = SunColor * (float(CalculateSunspot(surface)) * float(surface.mask_sky));
	surface.sky.sunSpot *= 1.0f - rainStrength;
	surface.sky.sunSpot *= 1.0f - timeMidnight;
	surface.sky.sunSpot *= 100.0f;
	AddSunglow(surface);

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

	//Colorize surface shading and store in lightmaps
	LightmapStruct lightmap;
	lightmap.sunlight = SunColor.rgb * shading.direct;
	lightmap.skylight = rawLightmap.sky;
	lightmap.skylight *= shading.skylight;
	lightmap.skylight *= AmbientVisibility(surface);
	//give some ambient sunlight plus some base ambient at night. 
	lightmap.skylight *= (SunColor.rgb*(1.0f - CloudThickness)*2.0 + lerp(1.0, 0.1f, pow(timeMidnight, 0.6)));
	// lightmap.nolight = float3(0.05f, 0.05f, 0.05f);
	// also give torch light some fake shading to make custom model look more dimentional at night. 
	float fakeTorchShading = 1.0;
	if (surface.category_id == 255) {
		fakeTorchShading = (directSunShading * 0.6 + 0.4);
		// Keep character ambient skylight soft and clean so shadowed faces don't turn muddy
		lightmap.skylight *= (directSunShading * 0.2 + 0.8);
	}
	lightmap.torchlight = (rawLightmap.torch * fakeTorchShading) * TorchLightColor;

	//Apply lightmaps to albedo and generate final shaded surface
	FinalStruct final;
	//final.nolight = surface.albedo * lightmap.nolight;
	final.sunlight = surface.albedo * lightmap.sunlight;
	final.skylight = surface.albedo * lightmap.skylight;
	final.torchlight = surface.albedo * lightmap.torchlight;
	final.glow_torch = surface.albedo * (float(surface.mask_torch) * TorchLightColor);
	// final.glow_torch = pow(final.glow_torch, 2.0);

#ifdef NIGHT_EYE_EFFECT
	float nightEyeAmount = timeMidnight*0.8;
	DoNightEye(final.sunlight, nightEyeAmount);
	DoNightEye(final.skylight, nightEyeAmount);
	DoNightEye(surface.sky.albedo, nightEyeAmount);
	// DoNightEye(final.nolight, 0.8);
#endif

	float3 finalComposite;
	finalComposite = final.sunlight	    * 1.0f					//Add direct sunlight
		+ final.skylight     * 0.05f					//Add ambient skylight
														// + final.nolight	    * 0.03f					//Add base ambient light
		+ final.torchlight   * 2.0					//Add light coming from emissive blocks
		+ final.glow_torch   * 3.0f					// add torch, lamp, lava glow
		;
	//Apply sky to final composite
	// surface.sky.albedo *= lerp(1.0f, 3.0f, timeNoon);
	surface.sky.albedo += surface.sky.sunSpot;
	finalComposite += surface.sky.albedo;

	CalculateAtmosphericScattering(finalComposite, surface);
	// ApplyDistanceFog(finalComposite, surface);

	// Convert final image back into gamma 0.45 space
	finalComposite.rgb = pow(finalComposite.rgb, (1.0f / 2.2f));

	// Scale image down for HDR, only required for integer16, not floating16. 
	// finalComposite.rgb *= 0.01f;

	PSOut o;
	o.Color = float4(finalComposite.rgb, 1.0);
	o.Color2 = float4(0, surface.shadow * pow(rawLightmap.sky, 0.2f), rawLightmap.sky, 1.0);
	return o;
}

float4 ComputeFakeSkyReflection(inout SurfaceStruct2 surface)
{
	// TODO
	return float4(0, 0, 0, 1.0);
}

void CalculateSpecularReflections(inout SurfaceStruct2 surface)
{
	int category_id = surface.category_id;

	surface.mask_sky = surface.depth < 0.01;
	surface.mask_water = (category_id == 8 || category_id == 9);
	surface.mask_metal = (category_id == 50);

	if (RenderOptions.y > 0.5 && (surface.mask_water || surface.mask_metal))
	{
		// water is 1 (full), metal is has a week reflective color. 
		float specularity = lerp(1.0, surface.specularity, surface.mask_metal);
		float3 cameraSpaceNormal = mul(surface.normal, (float3x3)matView);
		float4 reflection = ComputeRayTraceReflection(surface.cameraSpacePos, cameraSpaceNormal);
		float4 fakeSkyReflection = ComputeFakeSkyReflection(surface);
		reflection.a = reflection.a * fakeSkyReflection.a * specularity;

#define USE_WATER_REFLECTION_FRESNEL_EFFECT
#ifdef USE_WATER_REFLECTION_FRESNEL_EFFECT
		float3 cameraSpaceViewDir = -normalize(surface.cameraSpacePos);
		const float fresnelPower = 3.0;
		float fresnel = pow(saturate(1.0 - dot(cameraSpaceViewDir, cameraSpaceNormal)), fresnelPower) * 0.98 + 0.02;
		reflection.a *= fresnel;
#endif
		surface.color.xyz = lerp(surface.color.xyz, reflection.rgb, reflection.a);
	}
}


/** sunlight specularity based on sun and eye half vector, and the normal */
void CalculateSpecularHighlight(inout SurfaceStruct2 surface)
{
	if (!surface.mask_sky && !surface.mask_water)
	{
		// everything has some specular light, surface.mask_metal has more
		float gloss = surface.specularity + 0.02; // 0.02 is base specular for all blocks.

		float3 cameraSpaceViewDir = -normalize(surface.cameraSpacePos);
		float3 cameraSpaceNormal = mul(surface.normal, (float3x3)matView);
		float3 halfVector = normalize(mul(sunDirection, (float3x3)matView).xyz + cameraSpaceViewDir);
		float HdotN = saturate(dot(halfVector, cameraSpaceNormal));

		const float fresnelPower = 6.0;
		float fresnel = pow(saturate(1.0 - dot(cameraSpaceViewDir, cameraSpaceNormal)), fresnelPower) * 0.98 + 0.02;
		// 30 is a magic number, that is making the sun specular lighting size proper. The bigger the value, the smaller the specular region. 
		float spec = pow(HdotN, gloss * 30); 
		spec *= fresnel * gloss * surface.sunlightVisibility * (1.0 - rainStrength);
		float3 specularHighlight = spec * SunColor;

		// For fake torch specular light, we will assume torch light and eye are on the same point, so half vector is actually viewDir.
		if (surface.mask_metal || surface.specularity > 0.01)
		{
			// 60 is a magic number, that is making the torch specular lighting at proper size. 
			float spec = pow(saturate(dot(cameraSpaceViewDir, cameraSpaceNormal)), 60.0);
			specularHighlight += (TorchLightColor.rgb*pow(surface.torch_light_strength*spec, 3.0));
		}
		surface.color.xyz += specularHighlight;
		
		// uncomment to test (show) just specular component
		// #define TEST_SHOW_SPECULAR_COMPNENT
#ifdef TEST_SHOW_SPECULAR_COMPNENT
		surface.color.xyz = specularHighlight;
#endif
	}
}

// do water reflection here
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
	CalculateSpecularHighlight(surface);

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
		color += bloomData.bloom*0.006f;

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

	return float4(color, 1.0f);
}

float4 CompositeFXAA(VSOutput input) :COLOR
{
	return FxaaPixelShader(
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

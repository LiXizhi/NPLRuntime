/**
Author: LiXizhi
Company: ParaEngine
Date: 2015.5.5
Desc: Deferred shading post processing after composite.fx
References:
- SEUS v10.0

CustomBuild:"$(DXSDK_DIR)/Utilities/bin/x86/fxc.exe" /Tfx_2_0 /Gfp /nologo /Fo %(RelativeDir)/%(Filename).fxo %(FullPath)
OUTPUT:%(RelativeDir)%(Filename).fxo
*/

float4x4 matView;
float4x4 matViewInverse;
float4x4 matProjection;

// x>0 use depth of view y>0 use boom. 
float3 RenderOptions;

float2 screenParam;

float centerDepthSmooth = 15.f;
float ViewAspect;

float TanHalfFOV;
float cameraFarPlane;
float3 cameraPosition;
float3 sunDirection;
float3 SunColor;
float3 TorchLightColor;
float TimeOfDaySTD; // 0 is noon. -1, 1 is night
static const float bloom_threshold = 0.7;
// offset_x, offset_y, scale_factor
static const float3 offsets[16] = { 
	 float3(0.008, 0.0, 1.0),  float3(0.006, 0.0, 1.2),  float3(0.004, 0.0, 1.3),  float3(0.002, 0.0, 1.5),
	 float3(0.0, 0.008, 1.0),  float3(0.0, 0.006, 1.2),  float3(0.0, 0.004, 1.3),  float3(0.0, 0.002, 1.5),
	-float3(0.008, 0.0, 1.0), -float3(0.006, 0.0, 1.2), -float3(0.004, 0.0, 1.3), -float3(0.002, 0.0, 1.5),
	-float3(0.0, 0.008, 1.0), -float3(0.0, 0.006, 1.2), -float3(0.0, 0.004, 1.3), -float3(0.0, 0.002, 1.5)
};


texture sourceTexture0;
sampler colorSampler:register(s0) = sampler_state
{
	Texture = < sourceTexture0 > ;
	MinFilter = Linear;
	MagFilter = Linear;
	AddressU = clamp;
	AddressV = clamp;
};

texture sourceTexture1;
sampler matInfoSampler:register(s1) = sampler_state
{
	Texture = < sourceTexture1 > ;
	MinFilter = Linear;
	MagFilter = Linear;
	AddressU = clamp;
	AddressV = clamp;
};

texture sourceTexture2 : TEXTURE;
sampler ShadowMapSampler: register(s2) = sampler_state
{
	texture = < sourceTexture2 > ;
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
	Texture = < sourceTexture3 > ;
	MinFilter = Linear;
	MagFilter = Linear;
	AddressU = clamp;
	AddressV = clamp;
};

texture sourceTexture4;
sampler normalSampler:register(s4) = sampler_state
{
	Texture = < sourceTexture4 > ;
	MinFilter = Linear;
	MagFilter = Linear;
	AddressU = clamp;
	AddressV = clamp;
};

struct VSOutput
{
	float4 pos			: POSITION;         // Screen space position
	float2 texCoord		: TEXCOORD0;        // texture coordinates
};

VSOutput FinalQuadVS(float3 iPosition:POSITION,
	float2 texCoord : TEXCOORD0)
{
	VSOutput o;
	o.pos = float4(iPosition, 1);
	o.texCoord = texCoord + 0.5 / screenParam;
	return o;
}

float3 	GetColorTexture(float2 coord)
{
	return tex2D(colorSampler, coord).rgb;
}

// value based on SEUS v10.0
float3 DepthOfField(float3 color, float2 pos)
{
	float cursorDepth = centerDepthSmooth;
	if (cursorDepth == 0.0){
		// just in case it is first person view. 
		cursorDepth = tex2D(depthSampler, float2(0.5, 0.5)).x;
	}

	const float blurclamp = 0.014;  // max blur amount
	const float bias = 0.15;	//aperture - bigger values for shallower depth of field

	float2 aspectcorrect = float2(1.0, ViewAspect) * 1.5;
	
	float depth = tex2D(depthSampler, pos).x;
	// depth += float(isHand) * 0.36f;

	float factor = (depth - cursorDepth) / cameraFarPlane;

	float2 dofblur = clamp(factor * bias * 0.6, -blurclamp, blurclamp).xx;

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

	color = col / 41;
	return color;
}

float3 Vignette(float3 color, float2 pos)
{
	float dist = distance(pos, float2(0.5, 0.5)) * 2.0f / 1.5142;
	dist = pow(dist, 1.1f);
	color.rgb *= 1.0 - dist;
	return color;
}


float3 Bloom(float3 color, float2 pos)
{
	float sweight = length(color) + 0.2; // + (1.0 - rainx)*0.2;
	//exclude bright pixels
	if (sweight < bloom_threshold)
	{
		float3 csample = float3(0.0, 0.0, 0.0);
		for (int i = 0; i < 16; i++) 
		{
			//float3 bloomColor = tex2D(colorSampler, pos + offsets[i].xy * 1.2).rgb;
			//float3 addOnColor = max((bloomColor.xyz - sweight) * offsets[i].z, 0.0);
			// only add color if color is bright enough
			//addOnColor = lerp(float3(0.0, 0.0, 0.0), addOnColor, length(bloomColor) > bloom_threshold);
			//csample += addOnColor;
			// csample += max((tex2D(colorSampler, pos + offsets[i].xy * 1.2).rgb - sweight) * offsets[i].z, 0.0);
			csample += max((tex2D(colorSampler, pos + offsets[i].xy * 1.2).rgb - sweight), 0.0);
		}		
		color += csample * 0.06;
	}
	return color;
}

float4 FinalPS(VSOutput input) :COLOR
{
	float2 pos = input.texCoord.xy;
	float3 color = GetColorTexture(pos);
	if (RenderOptions.x > 0){
		color = DepthOfField(color, pos);
	}
	if (RenderOptions.y > 0){
		color = Bloom(color, pos);
	}
	
	//color = Vignette(color, pos);
	//color = TonemapReinhardLinearHybrid(color, pos);
	return float4(color.rgb, 1.0f);
}

technique Default_Lite
{
	pass P0
	{
		cullmode = none;
		ZEnable = false;
		ZWriteEnable = false;
		FogEnable = False;
		VertexShader = compile vs_3_0 FinalQuadVS();
		PixelShader = compile ps_3_0 FinalPS();
	}
}
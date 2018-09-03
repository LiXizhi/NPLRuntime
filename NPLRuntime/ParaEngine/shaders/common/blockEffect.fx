// Author: LiXizhi,Clayman
// Desc: 2013/6

#define ALPHA_TESTING_REF  0.95
#define SHADOW_BIAS 0.00025f
/** undefine to use linear torch light, otherwise it is power */
// #define POWER_LIGHT_TORCH

#ifdef POWER_LIGHT_TORCH
	/** whether to torch lit small or bigger area */
	// #define LIGHT_TORCH_SMALL_RANGE
#endif

////////////////////////////////////////////////////////////////////////////////
//  Per frame parameters
float4x4 mWorldViewProj : worldviewprojection;
float4x4 mShadowVP : shadowvp;
float2   mShadowMapTexelSize : ShadowMapTexelSize;

bool g_bAlphaTesting : alphatesting;
// for selection effect: light_params.x: sun_lightIntensity, light_params.y: damageDegree
// for block effect: light_params.xyz: light color, light_params.w light intensity
float4 light_params: ConstVector0; 
float3 sun_vec: sunvector;

float4   g_fogParam : fogparameters; // (fogstart, fogrange, fogDensity, reserved)
float4   g_fogColor : fogColor;
float4x4 mWorld: world;
texture tex0: TEXTURE0;
sampler tex0Sampler = sampler_state 
{
	texture = <tex0>;
};
texture shadowMapTex : shadowmaptex;
sampler2D shadomMapTexSampler = sampler_state
{
	texture = <shadowMapTex>;
	MinFilter = Linear;  
	MagFilter = Linear;
	MipFilter = None;
	AddressU  = BORDER;
	AddressV  = BORDER;
	BorderColor = 0xffffffff;
};

struct SimpleVSOut
{
	float4 pos	:POSITION;
	float2 texcoord :	TEXCOORD0;
	float4 shadowPos :  TEXCOORD1; // pos in shadow space
	half4 color : COLOR0;
};

struct SelectBlockVertexLayout
{
	float4 pos	:POSITION;
	float2 texcoord	:TEXCOORD0;
};

struct SelectBlockVSOut
{
	float4 pos	:POSITION;
	float2 texcoord	:TEXCOORD0;
};

/////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////

// Calculates fog factor based upon distance
float CalcFogFactor(float d)
{
	float fogCoeff = 0.0;
	fogCoeff = (d - g_fogParam.x) / g_fogParam.y;
	// return clamp((fogCoeff - 0.8)*6.0, 0.0, 1.0);
	return clamp(fogCoeff, 0.0, 1.0);
}





// color.x: sun light  color.y:block light  color.z*255 block_id; 
SimpleVSOut SimpleMainVS(	float4 pos		: POSITION,
							float3	Norm	: NORMAL,
							half4 color		: COLOR0,
							half4 color2 : COLOR1,
							float2 texcoord	: TEXCOORD0)
{
	SimpleVSOut output;
	output.pos = mul(pos, mWorldViewProj);
	output.texcoord = texcoord;
	float4 worldPos = mul(pos, mWorld);
	output.shadowPos =  mul(worldPos, mShadowVP);

	// emissive block light received by this block. 
	float torch_light_strength = color.y;
	float3 torch_light = light_params.xyz * torch_light_strength;

	// sun light + sky(fog) light
	float sun_light_strength = clamp(color.x*light_params.w, 0, 1); // normalize to 0,1 range

	float lightFactor = 0.7 + 0.3*saturate(dot(sun_vec, Norm));
	float3 sun_light = (float3(1, 1, 1) + g_fogColor.xyz * 0.05) * (sun_light_strength * lightFactor);
	torch_light_strength *= lightFactor;

	// compose and interpolate so that the strength of light is almost linear 
	float3 final_light = lerp(torch_light.xyz+sun_light.xyz, sun_light.xyz, sun_light_strength / (torch_light_strength + sun_light_strength+0.001));
	
	// apply shadow either before or after color clamp
	//output.color.xyz = min(float3(1,1,1), final_light)*color.w;
	output.color.xyz = final_light*color.w;
	output.color.xyz *= color2.rgb;
	
	//calculate the fog factor
	output.color.w = CalcFogFactor(length(output.pos.xyz));
	return output;
}

float ShadowCalculation(float4 fragPosLightSpace)
{
	float3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	projCoords = projCoords * 0.5 + 0.5;
	if (projCoords.z > 1.0 || projCoords.z < 0)
	{
		return 0;
	}
	
	float closestDepth = tex2D(shadomMapTexSampler, float2(projCoords.x,1-projCoords.y)).r;
	float currentDepth = projCoords.z;
	float bias = 0.005;
	float shadow = currentDepth - bias > closestDepth ? 1.0 : 0.0;
	return shadow;
}

float4 SimpleMainPS(SimpleVSOut input) :COLOR0
{

	float shadow = ShadowCalculation(input.shadowPos);
	float4 albedoColor = tex2D(tex0Sampler,input.texcoord);
	float4 oColor = float4(lerp(float3(albedoColor.xyz * input.color.xyz), g_fogColor.xyz, input.color.w), albedoColor.a);

	// apply shadow
	oColor.rgb =oColor.rgb  * (1 - shadow*0.2) ;
	return oColor;
}



// color.x: sun light  color.y:block light  color.z*255 block_id; 
SimpleVSOut TransparentSimpleMainVS(	float4 pos		: POSITION,
							float3	Norm	: NORMAL,
							half4 color		: COLOR0,
							half4 color2	: COLOR1,
							float2 texcoord	: TEXCOORD0)
{
	SimpleVSOut output;
	output.pos = mul(pos, mWorldViewProj);
	output.texcoord = texcoord;

	float4 worldPos = mul(pos, mWorld);
	output.shadowPos =  mul(worldPos, mShadowVP);

	// emissive block light received by this block. 
	float torch_light_strength = color.y;
	float3 torch_light = light_params.xyz * torch_light_strength;

	// sun light + sky(fog) light
	float sun_light_strength = clamp(color.x*light_params.w, 0, 1); // normalize to 0,1 range
	float3 sun_light = (float3(1,1,1) + g_fogColor.xyz * 0.05) * sun_light_strength;
	
	// compose and interpolate so that the strength of light is almost linear 
	float3 final_light = lerp(torch_light.xyz+sun_light.xyz, sun_light.xyz, sun_light_strength / (torch_light_strength + sun_light_strength+0.001));
	
	// apply shadow either before or after color clamp
	//output.color.xyz = min(float3(1,1,1), final_light)*color.w;
	output.color.xyz = final_light*color.w;
	output.color.xyz *= color2.xyz;

	output.color.w = CalcFogFactor(length(output.pos.xyz));
	return output;
}


float4 TransparentMainPS(SimpleVSOut input) :COLOR0
{
	float4 albedoColor = tex2D(tex0Sampler,input.texcoord);
	clip(albedoColor.w-ALPHA_TESTING_REF);
	float4 oColor = float4(lerp(float3(albedoColor.xyz * input.color.xyz), g_fogColor.xyz, input.color.w), albedoColor.a);
	return oColor;
}



/////////////////////////////////////////////////////////////////
//
/////////////////////////////////////////////////////////////////

SelectBlockVSOut SelectBlockVS(	float4 pos	: POSITION,
								float2 texcoord	:TEXCOORD0)
{
	SelectBlockVSOut result;
	result.pos = mul(pos, mWorldViewProj);
	result.texcoord = texcoord;
	return result;
}

float4 SelectBlockPS(SelectBlockVSOut input) :COLOR0
{
	float4 color = tex2D(tex0Sampler,input.texcoord);
	
	color.xyz *= light_params.x;
	color.w = 1;
	return color;
}

float4 DamagedBlockPS(SelectBlockVSOut input) :COLOR0
{
	float4 color;
	color = tex2D(tex0Sampler,input.texcoord);
	// color.w = color.x * light_params.y;
	return color;
}


void VertShadow(float4	Pos			: POSITION,
	float2	Tex : TEXCOORD0,
	out float4 oPos : POSITION,
	out float2	outTex : TEXCOORD0,
	out float2 Depth : TEXCOORD1)
{
	oPos = mul(Pos, mWorldViewProj);
	outTex = Tex;
	Depth.xy = oPos.zw;
}

float4 PixShadow(float2	inTex: TEXCOORD0,
	float2 Depth : TEXCOORD1) : COLOR0
{
	half alpha = tex2D(tex0Sampler, inTex.xy).w;

	if (g_bAlphaTesting)
	{
		// alpha testing
		alpha = lerp(1,0, alpha < ALPHA_TESTING_REF);
		clip(alpha - 0.5);
	}
	float d = (Depth.x / Depth.y) * 0.5 + 0.5;
	return float4(d,0,0,0);
}






technique SimpleMesh_vs20_ps20
{
	pass P0
	{
		ZEnable = TRUE;
		ZWriteEnable = TRUE;
		VertexShader = compile vs_2_0 SimpleMainVS();
		PixelShader  = compile ps_2_0 SimpleMainPS();
	}
	pass P1
	{
		ZEnable = TRUE;
		ZWriteEnable = TRUE;
		VertexShader = compile vs_2_0 SelectBlockVS();
		PixelShader  = compile ps_2_0 SelectBlockPS();
	}
	pass P2
	{
		ZEnable = TRUE;
		ZWriteEnable = TRUE;
		VertexShader = compile vs_2_0 SelectBlockVS();
		PixelShader  = compile ps_2_0 DamagedBlockPS();
	}
	pass P3
	{
		ZEnable = TRUE;
		AlphaBlendEnable = TRUE;
		VertexShader = compile vs_2_0 TransparentSimpleMainVS();
		PixelShader  = compile ps_2_0 TransparentMainPS();
	}
}

technique GenShadowMap
{
	pass p0
	{
		VertexShader = compile vs_2_0 VertShadow();
		PixelShader = compile ps_2_0 PixShadow();
		FogEnable = false;
	}
}
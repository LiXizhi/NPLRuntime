// Author: LiXizhi,Clayman
// Desc: 2013/6

#define ALPHA_TESTING_REF  0.95
#define BLOCK_SIZE	1.0416666

/** undefine to use linear torch light, otherwise it is power */
// #define POWER_LIGHT_TORCH

#ifdef POWER_LIGHT_TORCH
	/** whether to torch lit small or bigger area */
	// #define LIGHT_TORCH_SMALL_RANGE
#endif

////////////////////////////////////////////////////////////////////////////////
//  Per frame parameters
float4x4 mWorldViewProj : worldviewprojection;

// for selection effect: light_params.x: sun_lightIntensity, light_params.y: damageDegree
// for block effect: light_params.xyz: light color, light_params.w light intensity
float4 light_params: ConstVector0; 
float3 sun_vec: sunvector;

bool g_bEnableFog		:fogenable;
float4   g_fogParam : fogparameters; // (fogstart, fogrange, fogDensity, reserved)
float4   g_fogColor : fogColor;
float4x4 mWorld: world;

// world position
float4 vWorldPos		: worldpos;

// block material params
float4 materialUV	: materialUV;
float4 materialBaseColor: materialBaseColor;
float4 materialEmissiveColor : materialEmissiveColor;
float materialMetallic: materialMetallic;


// texture 0
texture tex0 : TEXTURE; 
sampler tex0Sampler: register(s0) = sampler_state 
{
	Texture = <tex0>;

	MinFilter = POINT;
	MagFilter = POINT;
};

// texture 1
texture tex1 : TEXTURE;
sampler tex1Sampler: register(s1) = sampler_state
{
	Texture = <tex1>;
	AddressU = wrap;
	AddressV = wrap;
	MagFilter = Linear;
	MinFilter = Linear;
	MipFilter = Linear;
};

struct SimpleVSOut
{
	float4 pos	:POSITION;
	float2 texcoord :	TEXCOORD0;
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

float4 SimpleMainPS(SimpleVSOut input) :COLOR0
{
	float4 albedoColor = tex2D(tex0Sampler,input.texcoord);
	float4 oColor = float4(lerp(float3(albedoColor.xyz * input.color.xyz), g_fogColor.xyz, input.color.w), albedoColor.a);
	return oColor;
}

float4 TransparentMainPS(SimpleVSOut input) :COLOR0
{
	float4 albedoColor = tex2D(tex0Sampler,input.texcoord);
	clip(albedoColor.w-ALPHA_TESTING_REF);
	float4 oColor = float4(lerp(float3(albedoColor.xyz * input.color.xyz), g_fogColor.xyz, input.color.w), albedoColor.a);
	return oColor;
}

////////////////////////////////////Material//////////////////////////////////////////////////////

struct MaterialBlockVSOut
{
	float4 pos	:POSITION;
	float2 texcoord :	TEXCOORD0;
	half4 color : COLOR0;
};

MaterialBlockVSOut MaterialMainVS(	float4 pos		: POSITION,
							float3	Norm	: NORMAL,
							half4 color		: COLOR0,
							half4 color2 : COLOR1,
							float2 texcoord	: TEXCOORD0)
{
	MaterialBlockVSOut output;

	output.pos = mul(pos, mWorldViewProj);
	float3 normal = Norm;
	output.texcoord = texcoord;
	
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

	// calculate world position
	float3 worldblockPos = (vWorldPos.xyz / BLOCK_SIZE) + pos.xyz;
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

float4 MaterialMainPS(MaterialBlockVSOut input) :COLOR0
{
	float2 uv = (input.texcoord - floor(input.texcoord / materialUV.xy) * materialUV.xy) / materialUV.xy;
	uv.y = 1.0 - uv.y;

	float4 albedoColor = tex2D(tex0Sampler, uv);
	albedoColor = albedoColor * materialBaseColor;
	albedoColor.xyz *= input.color.xyz;

	if (materialEmissiveColor.a > 0)
	{
		float4 emissiveColor = tex2D(tex1Sampler, uv);
		emissiveColor.rgb *= materialEmissiveColor.rgb;
		emissiveColor.a *= materialEmissiveColor.a;
		albedoColor.xyz = lerp(albedoColor.xyz, emissiveColor.rgb, emissiveColor.a);
	}

	float4 oColor = float4(lerp(albedoColor.xyz, g_fogColor.xyz, input.color.w), albedoColor.a);
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

technique SimpleMesh_vs20_ps20
{
	pass P0
	{
		VertexShader = compile vs_2_0 SimpleMainVS();
		PixelShader  = compile ps_2_0 SimpleMainPS();
		FogEnable = false;
	}
	pass P1
	{
		VertexShader = compile vs_2_0 SelectBlockVS();
		PixelShader  = compile ps_2_0 SelectBlockPS();
		FogEnable = false;
	}
	pass P2
	{
		VertexShader = compile vs_2_0 SelectBlockVS();
		PixelShader  = compile ps_2_0 DamagedBlockPS();
		FogEnable = false;
	}
	pass P3
	{
		VertexShader = compile vs_2_0 TransparentSimpleMainVS();
		PixelShader  = compile ps_2_0 TransparentMainPS();
		FogEnable = false;
	}
	pass P4
	{
		VertexShader = compile vs_2_0 MaterialMainVS();
		PixelShader  = compile ps_2_0 MaterialMainPS();
		FogEnable = false;
	}
}


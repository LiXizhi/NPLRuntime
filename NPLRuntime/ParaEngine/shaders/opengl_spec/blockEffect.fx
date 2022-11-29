// Author: LiXizhi,Clayman
// Desc: 2013/6

#define ALPHA_TESTING_REF  0.95

#define BLOCK_SIZE	1.0416666
#define WAVING_GRASS
#define WAVING_LEAVES

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
// x for world time, y for rainStrength,
float4 g_parameter0		: ConstVector1;
// world position
float4 vWorldPos		: worldpos;
float4 sun_vec: sunvector;

bool g_bEnableFog		:fogenable;
float4   g_fogParam : fogparameters; // (fogstart, fogrange, fogDensity, reserved)
float4   g_fogColor : fogColor;
float4x4 mWorld: world;

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
	float4 color : COLOR0;
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
							float4 color		: COLOR0,
							float4 color2 : COLOR1,
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
							float4 color		: COLOR0,
							float4 color2	: COLOR1,
							float2 texcoord	: TEXCOORD0)
{
	SimpleVSOut output;
	output.texcoord = texcoord;
	
	// emissive block light received by this block. 
	float torch_light_strength = color.y;
	float3 torch_light = light_params.xyz * torch_light_strength;

	// category id for block types 
	int category_id = (int)(color.z * 255.0 + 0.4);
	
#ifdef WAVING_GRASS	
	// convert to 60FPS ticks
	float worldTime = g_parameter0.x / 60.0 * 3.14159265358979323846264;
	// usually 0
	float rainStrength = g_parameter0.y;
	float3 world_pos = vWorldPos.xyz + pos.xyz * BLOCK_SIZE;

	//Grass//
	if (category_id == 31 && texcoord.y < 0.15)
	{
		float speed = 8.0;

		float magnitude = sin((worldTime / (28.0)) + world_pos.x + world_pos.z) * 0.1 + 0.1;
		float d0 = sin(worldTime / (122.0 * speed)) * 3.0 - 1.5 + world_pos.z;
		float d1 = sin(worldTime / (152.0 * speed)) * 3.0 - 1.5 + world_pos.x;
		float d2 = sin(worldTime / (122.0 * speed)) * 3.0 - 1.5 + world_pos.x;
		float d3 = sin(worldTime / (152.0 * speed)) * 3.0 - 1.5 + world_pos.z;
		pos.x += sin((worldTime / (28.0 * speed)) + (world_pos.x + d0) * 0.1 + (world_pos.z + d1) * 0.1) * magnitude * (1.0f + rainStrength * 1.4f);
		pos.z += sin((worldTime / (28.0 * speed)) + (world_pos.z + d2) * 0.1 + (world_pos.x + d3) * 0.1) * magnitude * (1.0f + rainStrength * 1.4f);

		//small leaf movement//
		speed = 0.8;

		magnitude = (sin(((world_pos.y + world_pos.x) / 2.0 + worldTime / ((28.0)))) * 0.05 + 0.15) * 0.4;
		d0 = sin(worldTime / (112.0 * speed)) * 3.0 - 1.5;
		d1 = sin(worldTime / (142.0 * speed)) * 3.0 - 1.5;
		d2 = sin(worldTime / (112.0 * speed)) * 3.0 - 1.5;
		d3 = sin(worldTime / (142.0 * speed)) * 3.0 - 1.5;
		pos.x += sin((worldTime / (18.0 * speed)) + (-world_pos.x + d0)*1.6 + (world_pos.z + d1)*1.6) * magnitude * (1.0f + rainStrength * 1.7f);
		pos.z += sin((worldTime / (18.0 * speed)) + (world_pos.z + d2)*1.6 + (-world_pos.x + d3)*1.6) * magnitude * (1.0f + rainStrength * 1.7f);
		pos.y += sin((worldTime / (11.0 * speed)) + (world_pos.z + d2) + (world_pos.x + d3)) * (magnitude / 3.0) * (1.0f + rainStrength * 1.7f);
	}
#endif

#ifdef WAVING_LEAVES
	//Leaves//
	if (category_id == 18) {
		float speed = 1.0;
		float magnitude = (sin((world_pos.y + world_pos.x + worldTime / ((28.0) * speed))) * 0.15 + 0.15) * 0.20;
		float d0 = sin(worldTime / (112.0 * speed)) * 3.0 - 1.5;
		float d1 = sin(worldTime / (142.0 * speed)) * 3.0 - 1.5;
		float d2 = sin(worldTime / (132.0 * speed)) * 3.0 - 1.5;
		float d3 = sin(worldTime / (122.0 * speed)) * 3.0 - 1.5;
		pos.x += sin((worldTime / (18.0 * speed)) + (-world_pos.x + d0)*1.6 + (world_pos.z + d1)*1.6) * magnitude * (1.0f + rainStrength * 1.0f);
		pos.z += sin((worldTime / (17.0 * speed)) + (world_pos.z + d2)*1.6 + (-world_pos.x + d3)*1.6) * magnitude * (1.0f + rainStrength * 1.0f);
		pos.y += sin((worldTime / (11.0 * speed)) + (world_pos.z + d2) + (world_pos.x + d3)) * (magnitude / 2.0) * (1.0f + rainStrength * 1.0f);
	}
#endif	
	output.pos = mul(pos, mWorldViewProj);
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


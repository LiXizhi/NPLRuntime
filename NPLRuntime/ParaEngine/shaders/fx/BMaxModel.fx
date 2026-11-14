// Author: LiXizhi@yeah.net
// Date: 2015/5/22
// Desc: block max model shader.

////////////////////////////////////////////////////////////////////////////////
//  Per frame parameters
float4x4 mWorldViewProj : worldviewprojection;
float4x4 mWorld: world;

// for selection effect: light_params.x: sun_lightIntensity, light_params.y: damageDegree
// for block effect: light_params.xyz: light color, light_params.w light intensity
float4 light_params: ConstVector0; 
float4 sun_vec: sunvector;
float3	colorDiffuse:materialdiffuse;
float3	colorAmbient:ambientlight;

bool g_bEnableFog		:fogenable;
float4   g_fogParam : fogparameters; // (fogstart, fogrange, fogDensity, reserved)
float4   g_fogColor : fogColor;
float g_opacity : opacity = 1.f;

// block material params
float4 materialUV	: materialUV;
float4 materialBaseColor: materialBaseColor;
float4 materialEmissiveColor : materialEmissiveColor;
float materialMetallic : materialMetallic;

struct VSOut
{
	float4 pos	:POSITION;
	float4 color : COLOR0;
};

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

VSOut MainVS(	float4 pos		: POSITION,
				float3	Norm	: NORMAL,
				float4 color		: COLOR0
				)
{
	VSOut output;
	output.pos = mul(pos, mWorldViewProj);

	// world space normal
	float3 worldNormal = normalize(mul(Norm, (float3x3)mWorld));

	float3 sun_lightmap = colorAmbient + colorDiffuse*saturate(dot(sun_vec.xyz, worldNormal));
	// float sun_lightmap = 0.7 + 0.3*saturate(dot(sun_vec.xyz, worldNormal));
	output.color.xyz = color.rgb * sun_lightmap;

	// calculate the fog factor
	if (g_bEnableFog)
		output.color.w = CalcFogFactor(length(output.pos.xyz));
	else
		output.color.w = 0.0;

	return output;
}

float4 MainPS(VSOut input) :COLOR0
{
	float3 color = input.color.rgb;
	color = lerp(color, g_fogColor.xyz, input.color.w);
	return float4(color, g_opacity);
}



////////////////////////////////////Material//////////////////////////////////////////////////////

struct MaterialBlockVSOut
{
	float4 pos	:POSITION;
	float2 texcoord :	TEXCOORD0;
	half4 color : COLOR0;
};

MaterialBlockVSOut MaterialMainVS(float4 pos		: POSITION,
	float3	Norm : NORMAL,
	half4 color : COLOR0
)
{
	MaterialBlockVSOut output;
	output.pos = mul(pos, mWorldViewProj);

	// world space normal
	float3 normal = normalize(mul(Norm, (float3x3)mWorld));

	float3 sun_lightmap = colorAmbient + colorDiffuse * saturate(dot(sun_vec, normal));
	// float sun_lightmap = 0.7 + 0.3*saturate(dot(sun_vec, worldNormal));
	output.color.xyz = color.rgb * sun_lightmap;

	// calculate the fog factor
	if (g_bEnableFog)
		output.color.w = CalcFogFactor(length(output.pos.xyz));
	else
		output.color.w = 0.0;

	// calculate world position
	float3 worldblockPos = mul(pos.xyz, (float3x3)mWorld).xyz;
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

half4 MaterialMainPS(MaterialBlockVSOut input) :COLOR
{
	float2 uv = (input.texcoord - floor(input.texcoord / materialUV.xy) * materialUV.xy) / materialUV.xy;
	uv.y = 1.0 - uv.y;

	float4 albedoColor = tex2D(tex0Sampler, uv);
	albedoColor = albedoColor * materialBaseColor;
	// uncomment to multiply block color?
	// albedoColor.xyz *= input.color.xyz; 

	if (materialEmissiveColor.a > 0)
	{
		float4 emissiveColor = tex2D(tex1Sampler, uv);
		emissiveColor.rgb *= materialEmissiveColor.rgb;
		emissiveColor.a *= materialEmissiveColor.a;
		albedoColor.xyz = lerp(albedoColor.xyz, emissiveColor.rgb, emissiveColor.a);
	}

    half4 oColor = half4(lerp(albedoColor.xyz, g_fogColor.xyz, input.color.w), albedoColor.a * g_opacity);
	return half4(oColor);
}



////////////////////////////////////////////////////////////////////////////////
//
//                              shadow map : VS and PS
//
////////////////////////////////////////////////////////////////////////////////

void VertShadow(float4 Pos		: POSITION,
				float3	Norm : NORMAL,
				float4 color : COLOR0,
				out float4 oPos : POSITION,
				out float2 Depth : TEXCOORD1)
{
	oPos = mul(Pos, mWorldViewProj);
	Depth.xy = oPos.zw;
}

float4 PixShadow(float2 Depth : TEXCOORD1) : COLOR
{
	float d = Depth.x / Depth.y;
	return float4(0.0, d.xx, 1.0);
}

technique SimpleMesh_vs20_ps20
{
	pass P0
	{
		VertexShader = compile vs_2_0 MainVS();
		PixelShader = compile ps_2_0 MainPS();
		FogEnable = false;
	}
	pass P1
	{
		VertexShader = compile vs_2_0 MaterialMainVS();
		PixelShader = compile ps_2_0 MaterialMainPS();
		FogEnable = false;
	}
}

technique GenShadowMap
{
	pass p0
	{
		VertexShader = compile vs_2_a VertShadow();
		PixelShader = compile ps_2_a PixShadow();
		FogEnable = false;
	}
	pass p1
	{
		VertexShader = compile vs_2_a VertShadow();
		PixelShader = compile ps_2_a PixShadow();
		FogEnable = false;
	}
}

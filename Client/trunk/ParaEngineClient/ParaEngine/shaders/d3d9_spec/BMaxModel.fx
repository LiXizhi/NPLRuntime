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
float3 sun_vec: sunvector;
float3	colorDiffuse:materialdiffuse;
float3	colorAmbient:ambientlight;

bool g_bEnableFog		:fogenable;
float4   g_fogParam : fogparameters; // (fogstart, fogrange, fogDensity, reserved)
float4   g_fogColor : fogColor;
float g_opacity : opacity = 1.f;

struct VSOut
{
	float4 pos	:POSITION;
	half4 color : COLOR0;
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
				half4 color		: COLOR0
				)
{
	VSOut output;
	output.pos = mul(pos, mWorldViewProj);

	// world space normal
	float3 worldNormal = normalize(mul(Norm, (float3x3)mWorld));

	float3 sun_lightmap = colorAmbient + colorDiffuse*saturate(dot(sun_vec, worldNormal));
	// float sun_lightmap = 0.7 + 0.3*saturate(dot(sun_vec, worldNormal));
	output.color.xyz = color.rgb * sun_lightmap;

	// calculate the fog factor
	output.color.w = CalcFogFactor(length(output.pos.xyz));
	return output;
}

float4 MainPS(VSOut input) :COLOR0
{
	float3 color = input.color.rgb;
	color = lerp(color, g_fogColor.xyz, input.color.w);
	return float4(color, g_opacity);
}

////////////////////////////////////////////////////////////////////////////////
//
//                              shadow map : VS and PS
//
////////////////////////////////////////////////////////////////////////////////

void VertShadow(float4 Pos		: POSITION,
				float3	Norm : NORMAL,
				half4 color : COLOR0,
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
}

technique GenShadowMap
{
	pass p0
	{
		VertexShader = compile vs_2_a VertShadow();
		PixelShader = compile ps_2_a PixShadow();
		FogEnable = false;
	}
}

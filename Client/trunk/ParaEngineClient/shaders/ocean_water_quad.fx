// Author: LiXizhi, based on Clayman's map shader created for map app in ParaEngine.
// Date: 2008.12.21

float4x4 mWorldViewProj : worldviewprojection;
float4x4 mWorld :world;
float3 g_EyePositionW	:worldcamerapos;

float3 sun_vec	:sunvector;
float3 sun_color: suncolor;

float4   g_fogParam : fogparameters; // (fogstart, fogrange, fogDensity, reserved)
float4    g_fogColor : fogColor;

//custom parameter
const float4 vsReflectionTexcoordScaleOffset:uvScaleOffset;

const float3 shallowWaterColor: ConstVector1;
const float3 deepWaterColor: ConstVector2;
float2 waveDir: ConstVector3;

//half shininess	: specularPower = 3;
float time	: time;
float3 texCoordOffset : texCoordOffset;

//normal map
Texture tex0 : TEXTURE;
sampler2D tex0Sampler  :register(s0) = sampler_state{
	texture = <tex0>;
	mipfilter = linear;
	minfilter = linear;
	magfilter = linear;
	AddressU = mirror;
	AddressV = mirror;
};


void vs_main(
    inout float4 pos  : POSITION,
    inout float2 texCoord :TEXCOORD0,
    out float3 disturbTexCoord1 :TEXCOORD1,
    out float2 disturbTexCoord2 :TEXCOORD2,
	out float3 oWorldPos  :TEXCOORD3
	)
{
	float3 worldPos = mul(pos,mWorld);
	pos = mul(pos,mWorldViewProj);
	oWorldPos = pos;

	float3 viewDir = g_EyePositionW - worldPos;
	disturbTexCoord1.z = length(viewDir); //save the fog distance
	
	texCoord = texCoord;
	disturbTexCoord1.xy = (texCoord + time * 0.06*waveDir) * 4;
	disturbTexCoord2 = (texCoord + time * 0.34 * waveDir)*8;
	disturbTexCoord1.z = saturate( (disturbTexCoord1.z - g_fogParam.x)/g_fogParam.y);
}

void ps_main(in float2 texCoord: TEXCOORD0,
	in float3 waveCoord1 :TEXCOORD1,
	in float2 waveCoord2 :TEXCOORD2,
	in float3 worldPos	 :TEXCOORD3,
	out half4 color : COLOR)
{
	half3 bump1 = tex2D(tex0Sampler, waveCoord1).xyz;
	half3 bump2 = tex2D(tex0Sampler, waveCoord2).xyz;
	half3 finalWaveBump  = normalize(2* (bump1 + bump2) - 2);


	float3 viewVec = g_EyePositionW - worldPos;
	viewVec = normalize(viewVec);
	float3 halfVec = normalize(sun_vec + viewVec);
	half specular = pow( saturate( dot(finalWaveBump,halfVec)), 64);
	
	half diffuse = saturate(dot(finalWaveBump,float3(0,1,0)));
	half3 waterColor = lerp(shallowWaterColor,deepWaterColor,diffuse);

	color.xyz =  waterColor * diffuse + specular * 0.8;
	//calculate the fog factor
	color.xyz = lerp(color.xyz, g_fogColor.xyz, waveCoord1.z);
	color.a = 0.65;
}

technique OceanWater_vs30_ps30
{
    pass P0
    {
		ZENABLE = TRUE;
		ZWRITEENABLE = TRUE;
		ZFUNC = LESSEQUAL;
		
		AlphaBlendEnable = true;
		AlphaTestEnable = false;
		FogEnable = false;
		
        VertexShader = compile vs_2_0 vs_main();
        PixelShader = compile ps_2_0 ps_main();
    }
}

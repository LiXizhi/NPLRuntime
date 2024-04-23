// Author: LiXizhi
// Desc: 2008/11
/** define this for the sky fog to be accurate*/
#define NODISTORTION_FOG
////////////////////////////////////////////////////////////////////////////////
//  Per frame parameters
float4x4 mWorldViewProj: worldviewprojection;
float3 sunvector:sunvector;  //normalized sun direction

////////////////////////////////////////////////////////////////////////////////
// per technique parameters
float4  g_fogParam : fogparameters; // (fogstart, fogrange, fogDensity, reserved)
float4  g_fogColor : fogColor;
float4  g_skycolorfactor: ConstVector0;
// static branch boolean constants
bool g_bEnableFog		:fogenable;

///////////////////////////////////////////////////////////////////////
// parameters

float skyDomeHeight:skyDomeHeight = 16; //the real sky dome height, equals to max vertical radius;
float3 lightSkyColor:lightSkyColor = float3(0.627,0.831,1); 
float3 darkSkyColor:darkSkyColor = float3(0,0.25,0.45); //change these two value can create various sky appearance
float3 sunColor:sunColor = float3(1,1,0.6); //change the sun/moon color
float3 cloudColor:cloudColor = float3(1,1,1); //change cloud color
float2 cloudOffset:cloudOffset = float2(0,0); //current cloud offset value, this value should be update every frame to make the cloud move
float cloudBlendWeight:cloudBlendWeight; //the blend weight of two cloud layer, change it every frame to make cloud looks more dynamic
float2 sunIntensity:sunIntensity = float2(1,0.3); //a value to control sun exposal, x value control sun exposal, y control sun halo exposal
float2 sunSize:sunSize = float2(500,12); //control sun and halo size.
float2 moonSize:moonSize = float2(500, 100); //control moon and halo size.
float cloudThickness:cloudThickness= 1; //control cloud thickness, 0 means no cloud is visible at all.
float sunsetGlowWeight:sunsetGlowWeight; //to control how much sunset glow you want. This value should usually equals to normalized y position of the sun.

// texture 0
texture tex0 : TEXTURE; 
sampler sunsetGlowSpl : register(s0) = sampler_state 
{
    texture = <tex0>;
    MinFilter = Linear;
	MipFilter = Linear;
	MagFilter = Linear;
	AddressU = clamp; 
	AddressV = clamp;
};

// texture 1
texture tex1 : TEXTURE; 
sampler cloudSampler : register(s1) = sampler_state 
{
    texture = <tex1>;
    MinFilter = Linear;
	MipFilter = Linear;
	MagFilter = Linear;
	AddressU = wrap; 
	AddressV = wrap;
};


////////////////////////////////////////////////////////////////////////////////
//
//                              Vertex Shader
//
////////////////////////////////////////////////////////////////////////////////

void Transform(inout float4 Pos  : POSITION,
	inout float2 texCoord :TEXCOORD0,
	out float normalizedVertexHeight :TEXCOORD1,
	out float3 normal :TEXCOORD3,
	out float2 cloudTexCoord0 :TEXCOORD4,
	out float2 cloudTexCoord1 :TEXCOORD5)
{
    normalizedVertexHeight = Pos.y / skyDomeHeight;
    normal = Pos.xyz;
	// during perspective divide, the z will be 1 (xyww/w = x/w, y/w, 1) in NDC space. 
	Pos = mul(Pos, mWorldViewProj).xyww;
	// make the z to farthest point, depth value 1 == z/w. Some intel cards will flicker if z == w, so we will make it 0.999
	// Pos.z = Pos.w * 0.9999;

    cloudTexCoord0 = texCoord + cloudOffset*4;
    cloudTexCoord1 = texCoord + cloudOffset;
}


////////////////////////////////////////////////////////////////////////////////
//
//                              Pixel Shader
//
////////////////////////////////////////////////////////////////////////////////
float CalcFogFactor( float d )
{
    return saturate(d*2); // fog height is 0.5 of the height.
}

float4 DaytimeSkyPs(
	float2 texCoord :TEXCOORD0,
	float normalizedVertexHeight :TEXCOORD1,
	float3 normal:TEXCOORD3,
	float2 cloudTexCoord0 :TEXCOORD4,
	float2 cloudTexCoord1 :TEXCOORD5) : COLOR0
{	
	//compute sun/moon color
	normal = normalize(normal);
	float dotSunAngle = dot(sunvector.xyz, normal);
	float dotSunAnglePos = saturate(dotSunAngle);
	float sunHighlight = pow(dotSunAnglePos, sunSize.x)*sunIntensity.x;
	float sunHalo = pow(dotSunAnglePos, sunSize.y)*sunIntensity.y;

	float3 skyColor = lerp(lightSkyColor,darkSkyColor,normalizedVertexHeight);

	//compute sunset glow color
	float2 sunsetGlowTexCoord = float2 ((1.0 - dotSunAngle)*0.5, normalizedVertexHeight);
	float3 sunsetGlowColor = tex2D(sunsetGlowSpl,sunsetGlowTexCoord);
	
	float cloud0 = tex2D(cloudSampler,cloudTexCoord0).x;
	float cloud1 = tex2D(cloudSampler,cloudTexCoord1).x;
	float cloud = saturate(lerp(cloud0,cloud1,cloudBlendWeight))*cloudThickness;

	skyColor = lerp(skyColor, cloudColor, cloud);
	skyColor = lerp(skyColor, skyColor*sunsetGlowColor, sunsetGlowWeight);

	float3 color = skyColor+ sunColor*(sunHalo + sunHighlight);
	
	float fog = CalcFogFactor(normalizedVertexHeight);
	return float4(color,fog);
}

float4 NightSkyPS(
	float2 texCoord :TEXCOORD0,
	float normalizedVertexHeight :TEXCOORD1,
	float3 normal:TEXCOORD3,
	float2 cloudTexCoord0 :TEXCOORD4,
	float2 cloudTexCoord1 :TEXCOORD5) : COLOR0
{	
	//compute sun/moon color
	normal = normalize(normal);	
	float3 mirrorSunVector = float3(sunvector.x, -sunvector.y, sunvector.z);
	float3 skyColor = lerp(lightSkyColor,darkSkyColor,normalizedVertexHeight);
	
	//compute sunset glow color
	float dotSunAngle = dot(mirrorSunVector.xyz, normal);
	float dotSunAnglePos = saturate(dotSunAngle);
	float2 sunsetGlowTexCoord = float2 ((1.0 - dotSunAngle)*0.5, normalizedVertexHeight);
	float3 sunsetGlowColor = tex2D(sunsetGlowSpl, sunsetGlowTexCoord);
	
	float cloud0 = tex2D(cloudSampler,cloudTexCoord0).x;
	float cloud1 = tex2D(cloudSampler,cloudTexCoord1).x;
	float cloud = saturate(lerp(cloud0,cloud1,cloudBlendWeight));
	
	// add star from texture 
	float star = tex2D(cloudSampler,texCoord*6).y*2;
	skyColor += star;
	
	skyColor = lerp(skyColor, cloudColor, cloud);
	skyColor = lerp(skyColor, skyColor*sunsetGlowColor, sunsetGlowWeight);
	
	// render sun 
	float sunHighlight = pow(dotSunAnglePos, sunSize.x);
	float sunHalo = pow(dotSunAnglePos, sunSize.y)*sunIntensity.y;
	skyColor += sunColor * (sunHalo + sunHighlight);

	// render moon 
	float3 moonVector = float3(-sunvector.x, sunvector.y, -sunvector.z);
	float dotMoonAngle = dot(moonVector.xyz, normal);
	float dotMoonAnglePos = saturate(dotMoonAngle);
	
	float moonSizeDot = dotMoonAnglePos - (1.0 - 1.25/moonSize.x);
    float moonHighlight = 0.0;
	if (moonSizeDot > 0.0)
        moonHighlight = clamp(pow(moonSizeDot * 300, 0.5), 0.0, 1.0);
	
	float moonHalo = pow(dotMoonAnglePos, moonSize.y)*sunIntensity.y;
	skyColor += float3(1.0, 1.0, 0.6) * (moonHalo + moonHighlight);

	float fog = CalcFogFactor(normalizedVertexHeight);
	return float4(skyColor, fog);
}


////////////////////////////////////////////////////////////////////////////////
//
//                              Technique
//
////////////////////////////////////////////////////////////////////////////////

technique SimpleMesh_vs20_ps20
{
    pass P0
    {
        vertexShader = compile vs_2_0 Transform();
        pixelShader = compile ps_2_0 DaytimeSkyPs();
        
        FogEnable = false;
    }
    pass P1
    {
        vertexShader = compile vs_2_0 Transform();
        pixelShader = compile ps_2_0 NightSkyPS();
        
        FogEnable = false;
    }
}

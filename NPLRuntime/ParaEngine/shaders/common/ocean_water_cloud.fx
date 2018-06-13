// Author: LiXizhi
// Desc: Based on the Vertex Texture Fetch paper on Nvida.developer's website.
/** in meters, how much is the shorelone away from the edge*/
#define SHORELINE_DEPTH	2
/** define this to enable fog effect on the water surface. */
#define WATER_FOG_ENALBED

// parameters
const float4x4 mViewProj: VIEWPROJECTION;
const float4 vWorldPos: worldpos;
const float4 vCameraPos: worldcamerapos;
const float4 sun_vec: sunvector;
const float4 posOffset : posScaleOffset;
// xy is cloud texture offset direction, 
// z is the blend weight of two cloud layers, change it every frame to make cloud looks more dynamic, 
// z is to control cloud thickness, 0 means no cloud is visible at all. 1 is full.
const float4 g_oceanParams1 : ConstVector1; 
const float4 g_oceancolor : ConstVector2;
#ifdef WATER_FOG_ENALBED
float4   g_fogParam : fogparameters; // (fogstart, fogrange, fogDensity, reserved)
float4    g_fogColor : fogColor;
#endif
// direction and speed of the cloud.
const float2 g_bumpSpeed = {-1, 0.0}; 
// const float4 g_bumpSpeed : ConstVector3;// default is {-0.02, 0.0}; 
// since the ocean size is 100, texture scale has to be 0.01*n, where n is integer.
const float2 g_textureScale = {0.01,0.01};

texture tex0 : TEXTURE; 
sampler sunsetGlowSpl : register(s0) = sampler_state 
{
    texture = <tex0>;
    AddressU  = clamp;        
    AddressV  = clamp;
    MIPFILTER = NONE;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
};


texture tex1 : TEXTURE; 
// cloud map
sampler cloudSampler : register(s1) = sampler_state 
{
    texture = <tex1>;
    AddressU  = wrap;        
    AddressV  = wrap;
    MagFilter = Linear;	
	MinFilter = Linear;
	MipFilter = Linear;
};

struct Interpolants
{
  float4 positionSS                                 : POSITION;         // Screen space position
  float3 positionWS                                 : TEXCOORD0;        // normal in world space
  float3 eyeVectorWS                                : TEXCOORD2;        // eye vector in world space
  float4 bumpCoord0								: TEXCOORD3;        // normal texture cooridnates. 
  float2 bumpCoord1								: TEXCOORD4;        // normal texture cooridnates. 
};

////////////////////////////////////////////////////////////////////////////////
//
//
//                              Vertex Shader
//
//
////////////////////////////////////////////////////////////////////////////////

Interpolants vertexShader(	float2	Pos		: POSITION,
							float	terrainHeight : TEXCOORD0)
{
	Interpolants o = (Interpolants)0;
	float4	positionWS;
	float4	flatPositionWS;
	float3	reflectionVectorWS;
	float2	nXY;
	float1  waveHeight;
	// offset xy and interpolate z to get world position	
	flatPositionWS = float4(Pos.x+posOffset.z, vWorldPos.y, Pos.y+posOffset.w, 1);
	positionWS = flatPositionWS;
	
	// transform and output
	o.positionSS = 	mul(positionWS, mViewProj);
	o.positionWS = positionWS.xyz;
	
	// Output the eye vector in world space
	o.eyeVectorWS = vCameraPos - positionWS;
	o.bumpCoord0.z = length(o.eyeVectorWS); //save the fog distance
	o.eyeVectorWS = o.eyeVectorWS / o.bumpCoord0.z; // i.e. normalize(vCameraPos - positionWS);
	#ifdef WATER_FOG_ENALBED
	// o.bumpCoord0.z = saturate( (o.bumpCoord0.z - g_fogParam.x)/g_fogParam.y);
	o.bumpCoord0.z = saturate( (o.bumpCoord0.z - 100)/40);
	#endif

	o.bumpCoord0.w = terrainHeight-vWorldPos.x;
	o.bumpCoord0.xy = Pos.xy*g_textureScale + float2(1,0)*g_oceanParams1.y*4;
	o.bumpCoord1.xy = Pos.xy*g_textureScale + float2(1,0)*g_oceanParams1.y;
	return o;
}

// Pixel shader Function declarations
half computeFresnel(half3 light, half3 normal, half R0);

half4 pixelShader(Interpolants i) : COLOR
{
	half3 specular;
	half4 reflection;
	half4 refraction;
	half  fresnel;
	half4 o;
	
	//compute sunset glow color
	//float2 flatSunVector = normalize(sun_vec.xz);
	//float2 flatEyeVector = float2(0,1);
	//float angle = dot(flatSunVector,flatEyeVector);
	//float angle = flatSunVector.x;
	//float2 sunsetGlowTexCoord = float2( 1-(angle + 1)*0.5, 1-i.bumpCoord0.z);
	float2 sunsetGlowTexCoord = float2( 0.2, 1-i.bumpCoord0.z);
	float3 sunsetGlowColor = tex2D(sunsetGlowSpl,sunsetGlowTexCoord);

	// Normalize direction vectors
	float cloud0 = tex2D(cloudSampler, i.bumpCoord0.xy).x;
	float cloud1 = tex2D(cloudSampler, i.bumpCoord1.xy).x;
	float cloud = saturate(lerp(cloud0,cloud1,0.5));
	
	// Compute the specular term
	//float3 halfVectorWS = normalize(i.eyeVectorWS+ sun_vec);// the half vector in world space
	//specular.x = pow(max(dot(halfVectorWS, i.normalWS), 0), 3);
	//specular.xyz = specular.xxx; // specular term works well mostly at night.
	
	/// this is for shoreline test,shoreline depth is SHORELINE_DEPTH=0.5 meters.
	half alpha = (i.bumpCoord0.w+SHORELINE_DEPTH)/SHORELINE_DEPTH;
	alpha = clamp(alpha, 0,1);
	o.w = (1-alpha);
	o.xyz = lerp(sunsetGlowColor,g_oceancolor.rgb, cloud); 
	
	#ifdef WATER_FOG_ENALBED
	//calculate the fog factor
	o.xyz = lerp(o.xyz, g_fogColor.xyz, i.bumpCoord0.z);
	#endif
	return o;
}


////////////////////////////////////////////////////////////////////////////////
//
//
//                              Technique
//
//
////////////////////////////////////////////////////////////////////////////////

technique OceanWater_vs20_ps20
{
	pass P0
	{
		ZENABLE = TRUE;
		ZWRITEENABLE = TRUE;
		ZFUNC = LESSEQUAL;
		
		AlphaBlendEnable = true;
		AlphaTestEnable = false;
		FogEnable = false;

		// shaders
		VertexShader = compile vs_2_0 vertexShader();
		PixelShader  = compile ps_2_0 pixelShader();
	}  
}


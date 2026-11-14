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
//const float4 sun_color: suncolor;
const float4 posOffset : posScaleOffset;
const float4 vsReflectionTexcoordScaleOffset:uvScaleOffset;
const float4 psFresnelR0: FresnelR0;
const float3	 psTotalInternalReflectionSlopeBias : ConstVector0;
// each field is listed as normal sign, time at [0,100]
const float4 g_oceanParams1 : ConstVector1;
const float4 g_oceancolor : ConstVector2;
#ifdef WATER_FOG_ENALBED
float4   g_fogParam : fogparameters; // (fogstart, fogrange, fogDensity, reserved)
float4    g_fogColor : fogColor;
#endif
// direction and speed of the wind. how many U,V per second. Since, time is time % 100; x,y has to be 0.01*n, where n is integer.
//const float2 g_bumpSpeed = {-0.02, 0.0}; 
const float4 g_bumpSpeed : ConstVector3;// default is {-0.02, 0.0}; 
// since the ocean size is 100, texture scale has to be 0.01*n, where n is integer.
const float2 g_textureScale = {0.08,0.08};
//const float2 g_textureScale = {0.16,0.16};

texture tex0 : TEXTURE; 
// reflection map
sampler reflectionSampler = sampler_state 
{
    texture = <tex0>;
    AddressU  = clamp;        
    AddressV  = clamp;
    MIPFILTER = NONE;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
};


texture tex1 : TEXTURE; 
// normal map
sampler normalSampler = sampler_state 
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
  float3 normalWS                                   : TEXCOORD1;        // normal in world space
  float3 eyeVectorWS                                : TEXCOORD2;        // eye vector in world space
  float4 bumpCoord0									: TEXCOORD3;        // normal texture cooridnates. 
  float2 bumpCoord1									: TEXCOORD4;        // normal texture cooridnates. 
};

////////////////////////////////////////////////////////////////////////////////
//
//
//                              Vertex Shader
//
//
////////////////////////////////////////////////////////////////////////////////

// Vertex shader Function declarations
float2 computeTextureCoord(float3 positionWS, float3 directionWS, float distance, float2 scale, float2 offset);

Interpolants vertexShader(	float2	Pos		: POSITION,
							float	ZPos0	: POSITION1,
							float2	Norm0	: NORMAL0,
							float	ZPos1	: POSITION2,
							float2	Norm1	: NORMAL1,
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
	waveHeight = lerp(ZPos0, ZPos1, posOffset.x);
	positionWS = flatPositionWS;
	positionWS.y += waveHeight; // make smaller wave

	// transform and output
	o.positionSS = 	mul(positionWS, mViewProj);
	o.positionWS = positionWS.xyz;
	
	// Output the world space normal
	nXY = lerp(Norm0, Norm1, posOffset.x);
	o.normalWS = normalize(float3(nXY, 8.0f)).xzy;// LXZ: why 8.0f, 24.0f are all OK values
	
	// Output the eye vector in world space
	o.eyeVectorWS = vCameraPos - positionWS;
	o.bumpCoord0.z = length(o.eyeVectorWS); //save the fog distance
	o.eyeVectorWS = o.eyeVectorWS / o.bumpCoord0.z; // i.e. normalize(vCameraPos - positionWS);
	#ifdef WATER_FOG_ENALBED
	o.bumpCoord0.z = saturate( (o.bumpCoord0.z - g_fogParam.x)/g_fogParam.y);
	#endif
	
	//////////////////////////////////////////////////////////////////////////////////
	//
	//                   Calculate reflection map coordinates
	//
	//////////////////////////////////////////////////////////////////////////////////
	
	o.bumpCoord0.w = max(terrainHeight-vWorldPos.x, -SHORELINE_DEPTH);
	o.bumpCoord0.w = min(0, o.bumpCoord0.w);
	o.bumpCoord0.xy = Pos.xy*g_textureScale*1.5 + g_oceanParams1.y*g_bumpSpeed/1.5;
	o.bumpCoord1.xy = Pos.xy*g_textureScale + g_oceanParams1.y*g_bumpSpeed;
	return o;
}
// computeTextureCoord() takes a starting position, direction and distance in world space. 
// It computes a new position by moving the distance along the direction vector. This new
// world space position is projected into screen space. The screen space coordinates are
// massaged to work as texture coordinates.
float2 computeTextureCoord(float3 positionWS, float3 directionWS, float distance, float2 scale, float2 offset)
{
  float4 positionSS;

  // Compute the position after traveling a fixed distance
  positionWS.xyz = positionWS.xyz + directionWS * distance;
  
  // Compute the screen space position of the newly computed position
  positionSS = mul(float4(positionWS.xyz,1), mViewProj);

  // Do the perspective divide
  // Note by LXZ: since the surface is perturbed anyway, we do perspective divide in vertex shader, 
  // instead of in pixel shader (like projective mapping usually does)
  positionSS.xy /= positionSS.w;

  // Convert screen space position from [-1,-1]->[1,1] to [0,0]->[1,1]
  // This is done to match the coordinate space of the reflection/refraction map
  positionSS.xy = positionSS.xy * 0.5 + 0.5;

  // Account for the fact that we use a different field of view for the reflection/refraction maps.
  // This overdraw allows us to see stuff in the reflection/refraction maps that is not visible
  // from the normal viewpoint.
  positionSS.xy = positionSS.xy * scale + offset;

  // Flip the t texture coordinate upside down to be consistent with D3D
  positionSS.xy = half2(1,1) - positionSS.xy;

  // Return the screen space position as the result. This will be used as the texture coordinate
  // for the screenspace reflection/refraction maps.
  return(positionSS.xy);
}




////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//
//
//                              Pixel Shader
//
//
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////

// Pixel shader Function declarations
half computeFresnel(half3 light, half3 normal, half R0);

half4 pixelShader(Interpolants i) : COLOR
{
	half3 specular;
	half4 reflection;
	half4 refraction;
	half  fresnel;
	half4 o;
	
	// Normalize direction vectors
	half3 bump1 = tex2D(normalSampler, i.bumpCoord0).xyz;
	half3 bump2 = tex2D(normalSampler, i.bumpCoord1).xyz;
	i.normalWS += bump1 + bump2 - 1; // bump mapping the high frequency ripples.
	i.normalWS *= g_oceanParams1.xxx; // flip the normal if we're under water
	i.normalWS = normalize(i.normalWS);
	
	// Compute the specular term
	float3 halfVectorWS = normalize(i.eyeVectorWS+ sun_vec);// the half vector in world space
	specular.x = pow(max(dot(halfVectorWS, i.normalWS), 0), 3);

	specular.xyz = specular.xxx * half3(0.2, 0.2, 0.2); // specular term works well mostly at night.
	
	// Do the texture lookup for the reflection
	// Compute the reflection vector in world space
	float3 reflectionVectorWS = reflect(-i.eyeVectorWS, i.normalWS);
	
	// Compute the reflection map coordinates
	float2 reflectionTexCoord = computeTextureCoord(i.positionWS, reflectionVectorWS, 0.4f, 
		vsReflectionTexcoordScaleOffset.xy,vsReflectionTexcoordScaleOffset.zw);
	reflection = tex2D(reflectionSampler, reflectionTexCoord);
	
	// Handle total internal reflection
	half refractionVisibility = saturate(dot(i.eyeVectorWS, i.normalWS));
	refractionVisibility = saturate((refractionVisibility - psTotalInternalReflectionSlopeBias.y) * psTotalInternalReflectionSlopeBias.x +psTotalInternalReflectionSlopeBias.z);
	
	// Compute the fresnel to blend the refraction and reflection terms together
	fresnel = computeFresnel(i.eyeVectorWS, i.normalWS, psFresnelR0.x);
	

	// Combine the refraction,the blue ocean color(0,0,0.1), reflection and specular terms
	
	/// this is for shoreline test,shoreline depth is SHORELINE_DEPTH=0.5 meters.
	half alpha = (i.bumpCoord0.w+SHORELINE_DEPTH)/SHORELINE_DEPTH;
	alpha = clamp(alpha, 0,1);
	o.w = 1-alpha;
	o.xyz = reflection.xyz*fresnel+specular.xyz+((refractionVisibility*(1-fresnel))*g_oceancolor.rgb*o.w/*lerp(half3(0, 0.3, 0.3), 0, alpha)*/);
	
	#ifdef WATER_FOG_ENALBED
	//calculate the fog factor
	o.xyz = lerp(o.xyz, g_fogColor.xyz, i.bumpCoord0.z);
	#endif
	return o;
}

half computeFresnel(half3 eye, half3 normal, half R0)
{
  // R0 = pow(1.0 - refractionIndexRatio, 2.0) / pow(1.0 + refractionIndexRatio, 2.0);

  half eyeDotNormal;
 
  eyeDotNormal = dot(eye, normal);

  // Make sure eyeDotNormal is positive
  eyeDotNormal = max(eyeDotNormal, -eyeDotNormal);

  return(R0 + (1.0 - R0) * pow(1.0 - eyeDotNormal, 4.5));
}


////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
//
//
//                              Technique
//
//
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
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

		// shaders
		VertexShader = compile vs_3_0 vertexShader();
		PixelShader  = compile ps_3_0 pixelShader();
	}
}
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


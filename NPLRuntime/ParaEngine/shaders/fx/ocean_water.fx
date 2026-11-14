// Author: LiXizhi
// Desc: Based on the Vertex Texture Fetch paper on Nvida.developer's website.
//@def whether use backbuffer as refraction map.If so, pixel color alpha value is used as aprox. fresnel term.
#define ALPHA_BLENDING_REFRACTION_MAP
/** in meters, how much is the shorelone away from the edge*/
#define SHORELINE_DEPTH	2
/** define this to enable fog effect on the water surface. */
#define WATER_FOG_ENALBED
/** define this to use bump map. */
#define WATER_USEBUMPMAP

/**@def white shoreline */
//#define SHORELINE_WHITE
/**@def transparent shoreline */
#define SHORELINE_TRANSPARENT
/**@def using shoreline animated texture on texture 2*/
//#define SHORELINE_TEXTURE

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

bool g_bUseBumpMap	:boolean0;

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

#ifdef SHORELINE_TEXTURE
texture tex2 : TEXTURE; 
// normal map
sampler shorelineSampler = sampler_state 
{
    texture = <tex2>;
    AddressU  = wrap;        
    AddressV  = wrap;
    MagFilter = Linear;	
	MinFilter = Linear;
	MipFilter = Linear;
};
#endif

struct Interpolants
{
  float4 positionSS                                 : POSITION;         // Screen space position
  float3 reflectionTexCoord                         : TEXCOORD0;        // texture coordinates for reflection map
  float3 normalWS                                   : TEXCOORD1;        // normal in world space
  float3 eyeVectorWS                                : TEXCOORD2;        // eye vector in world space
  float3 halfVectorWS                               : TEXCOORD3;        // half vector in world space
  float3 bumpCoord0									: TEXCOORD4;        // normal texture cooridnates. 
};

////////////////////////////////////////////////////////////////////////////////
//
//
//                              Vertex Shader
//
//
////////////////////////////////////////////////////////////////////////////////

// Vertex shader Function declarations
float2 computeTextureCoord(float4 positionWS, float3 directionWS, float distance, float2 scale, float2 offset);

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
	float3	normalWS;
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
	
	// Output the world space normal
	nXY = lerp(Norm0, Norm1, posOffset.x);
	normalWS = normalize(float3(nXY, 8.0f)).xzy;// LXZ: why 8.0f, 24.0f are all OK values
	normalWS.xyz *= g_oceanParams1.x; // Flip the normal if we're under water
	o.normalWS = normalWS;
	
	// Output the eye vector in world space
	o.eyeVectorWS = vCameraPos - positionWS;
	o.bumpCoord0.z = length(o.eyeVectorWS); //save the fog distance
	o.eyeVectorWS = o.eyeVectorWS / o.bumpCoord0.z; // i.e. normalize(vCameraPos - positionWS);
	#ifdef WATER_FOG_ENALBED
	o.bumpCoord0.z = saturate( (o.bumpCoord0.z - g_fogParam.x)/g_fogParam.y);
	#endif


	// Output the half vector in world space
	// No need to normalize because it's normalized in the pixel shader
	o.halfVectorWS = o.eyeVectorWS + sun_vec;

	
	//////////////////////////////////////////////////////////////////////////////////
	//
	//                   Calculate reflection map coordinates
	//
	//////////////////////////////////////////////////////////////////////////////////
	
	// Compute the reflection vector in world space
	reflectionVectorWS = reflect(-o.eyeVectorWS, normalWS);
	
	// Compute the reflection map coordinates
	o.reflectionTexCoord.xy = computeTextureCoord(positionWS, reflectionVectorWS, 0.4f, vsReflectionTexcoordScaleOffset.xy,
												vsReflectionTexcoordScaleOffset.zw);
	o.reflectionTexCoord.z = terrainHeight-vWorldPos.x;
	#ifdef WATER_USEBUMPMAP
		o.bumpCoord0.xy = Pos.xy*g_textureScale + g_oceanParams1.y*g_bumpSpeed;
	#endif
	return o;
}
// computeTextureCoord() takes a starting position, direction and distance in world space. 
// It computes a new position by moving the distance along the direction vector. This new
// world space position is projected into screen space. The screen space coordinates are
// massaged to work as texture coordinates.
float2 computeTextureCoord(float4 positionWS, float3 directionWS, float distance, float2 scale, float2 offset)
{
  float4 positionSS;

  // Compute the position after traveling a fixed distance
  positionWS.xyz = positionWS.xyz + directionWS * distance;
  positionWS.w = 1.0;

  // Compute the screen space position of the newly computed position
  positionSS = mul(positionWS, mViewProj);

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
	half4 specular;
	half4 reflection;
	half4 refraction;
	half  fresnel;
	half4 o;
	
	// Normalize direction vectors
	#ifdef WATER_USEBUMPMAP
		i.normalWS += (tex2D(normalSampler, i.bumpCoord0.xy)*2.0-1.0).xzy*g_oceanParams1.x; // bump mapping the high frequency ripples.
	#endif
	i.normalWS = normalize(i.normalWS);
	i.halfVectorWS = normalize(i.halfVectorWS);
	
	// Compute the specular term
	specular.x = pow(max(dot(i.halfVectorWS, i.normalWS), 0), 3);

	// Put a cliff in the specular function
	if(specular.x < 0.5)
	{
		specular.x = 2.0 * specular.x * specular.x;
	}
	specular.xyz = specular.xxx * half3(0.2, 0.2, 0.2);
	specular.w = 0;
	
	// Do the texture lookup for the reflection
	reflection = tex2D(reflectionSampler, i.reflectionTexCoord.xy);
	
	// TODO:
	// Do the texture lookup for the refraction
	// Handle total internal reflection
	half refractionVisibility = saturate(dot(i.eyeVectorWS, i.normalWS));
	refractionVisibility = saturate((refractionVisibility - psTotalInternalReflectionSlopeBias.y) * psTotalInternalReflectionSlopeBias.x +
									psTotalInternalReflectionSlopeBias.z);
	// Give some blue tint to the refraction
	// refraction = lerp(refraction, half4(0, 0, 1, 0), .1);
	
	// Compute the fresnel to blend the refraction and reflection terms together
	fresnel = computeFresnel(i.eyeVectorWS, i.normalWS, psFresnelR0.x);
	
#ifdef ALPHA_BLENDING_REFRACTION_MAP
	// Combine the refraction,a blue tint(0,0,0.1), reflection and specular terms
	
	/// this is for shoreline test,shoreline depth is SHORELINE_DEPTH=0.5 meters.
	half alpha = (i.reflectionTexCoord.z+SHORELINE_DEPTH)/SHORELINE_DEPTH;
	alpha = clamp(alpha, 0,1);
	o.w = 1-alpha;
	o.xyz = reflection.xyz*fresnel+(specular.xyz+(refractionVisibility*0.9*(1-fresnel))*g_oceancolor.rgb*o.w/*lerp(half3(0, 0.3, 0.3), 0, alpha)*/);
	//o *=g_oceancolor;
	//o.xyz = o.www;
	//o.w = 1;
	
	
	#ifdef	SHORELINE_WHITE
		// white shoreline
		o.xyz = lerp(o.xyz, half3(1,1,1), alpha);
		// use sin(x) to create a moving waving shoreline of white color procedurally. 
		//alpha = sin((alpha-g_oceanParams1.y*0.5)*10)*alpha;
		//o.xyz = lerp(o.xyz, half3(1,1,1), alpha);
	#endif	
	#ifdef SHORELINE_TEXTURE
		// using a shoreline texture.
		o.xyzw = lerp(o.xyzw, tex2D(shorelineSampler, i.bumpCoord0.xy).rgba, alpha);
		//half4 color = tex2D(shorelineSampler, i.bumpCoord0.xy);
		//o.xyz = lerp(o.xyz, color.rgb*color.a, alpha);
		//o.w = lerp(o.w, 0, alpha);
	#endif
	
	#ifdef WATER_FOG_ENALBED
	//calculate the fog factor
	o.xyz = lerp(o.xyz, g_fogColor.xyz, i.bumpCoord0.z);
	#endif	
	return o;
#else
	// Combine the refraction, reflection and specular terms
	return(lerp(refraction, reflection, fresnel) + specular);
#endif
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


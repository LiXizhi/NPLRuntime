// Author: LiXizhi
// Desc: 2006/4
// terrain texture layers: There are no limit to the total number of layers.
// multiple passes can be used if there are more than 3 layers. All color operations are addictive and fog is applied to the first pass only.
// Pass1: (base+common)+shadowmap+(alpha+detail)+(alpha+detail)+(bump)
// Pass2: (alpha+detail)+shadowmap+(alpha+detail)+(alpha+detail)+(bump)
// PassN: same as Pass2
// Shadow map and bump map are shared textures for all passes.
// formula I used: <pass0>{(1-FogFactor)*[(base*common)+detail*alpha+detail*alpha] + FogFactor*FogColor} +<pass1>{(1-FogFactor)*[(base*common)+detail*alpha+detail*alpha]}+...+<passN>{...}

/**TODO: @def whether use local light */
// #define _USE_LOCAL_LIGHTS
#define SHADOW_EPSILON 0.00005f
#define MULTI_SAMPLE_SHADOWMAP
/**@def if this is defined. layers will be addictively blended, the order of layers are thus not important.
if this is not defined, layers are linearly blended with the previous layer, so the order of layer will play an important layer, since the 
last layer have priority over all previous layers.  */
#define ADDITIVE_LAYERS
/** if your GPU supports 12 simultanious textures, you can define the following macro. Also define SIMULTANEOUS_TEXTURE in terrain.cpp.*/
// #define SIMULTANEOUS_TEXTURE_12
/**@def use depth based shadow map with Percentage closer filter */
//#define USE_DEPTH_SHADOWMAP

////////////////////////////////////////////////////////////////////////////////
//  Per frame parameters
float4x4 mWorldViewProj: worldviewprojection;
//float4x4 mViewProj: viewprojection;
float4x4 mWorldView: worldview;
float4x4 mWorld: world;
float4x4 mLightWorldViewProj: texworldviewproj;
bool g_bEnableSunLight: sunlightenable;
float3 sun_vec: sunvector;

float3	colorDiffuse:materialdiffuse;
float3	colorAmbient:ambientlight;

float3 g_vertexOffset :posScaleOffset = float3(0,0,0);

//int		g_nLights		:	locallightnum;
//half4	g_lightcolor0	:	LightColor0;
//float4	g_lightpos0		:	LightPosition0;

bool	g_bIsBaseEnabled:	boolean10;
bool	g_bLayer1		:	boolean11;
bool	g_bLayer2		:	boolean12;
#ifdef SIMULTANEOUS_TEXTURE_12
	bool	g_bLayer3		:	boolean13;
	bool	g_bLayer4		:	boolean14;
#endif
////////////////////////////////////////////////////////////////////////////////
/// per technique parameters
bool	g_bEnableFog	:	fogenable;
float4   g_fogParam		:	fogparameters; // (fogstart, fogrange, fogDensity, reserved)
float4   g_fogColor		:	fogColor;
bool	g_Useshadowmap	:	boolean8;
bool	g_UseShadowmapHW:	boolean9;
int		g_nShadowMapSize:	shadowmapsize;

// texture 0
texture BaseTex0 : TEXTURE; 
sampler BaseTexSampler : register(s0) = sampler_state 
{
    texture = <BaseTex0>;
    MinFilter = Linear;  
    MagFilter = Linear;
    MipFilter = Linear;
    AddressU  = Clamp;
    AddressV  = Clamp;
};

// texture 1
texture DetailTex1 : TEXTURE; 
sampler DetailTex1Sampler: register(s1) = sampler_state 
{
    texture = <DetailTex1>;
    MinFilter = Linear;  
    MagFilter = Linear;
    MipFilter = Linear;
    AddressU  = wrap;
    AddressV  = wrap;
};

// texture 2
texture ShadowMap2 : TEXTURE; 
sampler ShadowMapSampler: register(s2) = sampler_state 
{
    texture = <ShadowMap2>;
    MinFilter = Linear;  
    MagFilter = Linear;
    MipFilter = None;
    AddressU  = BORDER;
    AddressV  = BORDER;
    BorderColor = 0xffffffff;
};

// texture 3
texture AlphaTex3 : TEXTURE; 
sampler AlphaTex3Sampler : register(s3)= sampler_state 
{
    texture = <AlphaTex3>;
    MinFilter = Linear;  
    MagFilter = Linear;
    MipFilter = Linear;
    AddressU  = Clamp;
    AddressV  = Clamp;
};

// texture 4
texture DetailTex4 : TEXTURE; 
sampler DetailTex4Sampler : register(s4)= sampler_state 
{
    texture = <DetailTex4>;
    MinFilter = Linear;  
    MagFilter = Linear;
    MipFilter = Linear;
    AddressU  = wrap;
    AddressV  = wrap;
};

// texture 5
texture AlphaTex5 : TEXTURE; 
sampler AlphaTex5Sampler : register(s5)= sampler_state 
{
    texture = <AlphaTex5>;
    MinFilter = Linear;  
    MagFilter = Linear;
    MipFilter = Linear;
    AddressU  = Clamp;
    AddressV  = Clamp;
};

// texture 6
texture DetailTex6 : TEXTURE; 
sampler DetailTex6Sampler : register(s6)= sampler_state 
{
    texture = <DetailTex6>;
    MinFilter = Linear;  
    MagFilter = Linear;
    MipFilter = Linear;
    AddressU  = wrap;
    AddressV  = wrap;
};
// texture 7
texture AlphaTex7 : TEXTURE; 
sampler AlphaTex7Sampler : register(s7)= sampler_state 
{
	texture = <AlphaTex7>;
	MinFilter = Linear;  
	MagFilter = Linear;
	MipFilter = Linear;
	AddressU  = Clamp;
	AddressV  = Clamp;
};
#ifdef SIMULTANEOUS_TEXTURE_12
	// texture 8
	texture DetailTex8 : TEXTURE; 
	sampler DetailTex8Sampler : register(s8)= sampler_state 
	{
		texture = <DetailTex8>;
		MinFilter = Linear;  
		MagFilter = Linear;
		MipFilter = Linear;
		AddressU  = wrap;
		AddressV  = wrap;
	};

	// texture 9
	texture AlphaTex9 : TEXTURE; 
	sampler AlphaTex9Sampler : register(s9)= sampler_state 
	{
		texture = <AlphaTex9>;
		MinFilter = Linear;  
		MagFilter = Linear;
		MipFilter = Linear;
		AddressU  = Clamp;
		AddressV  = Clamp;
	};

	// texture 10
	texture DetailTex10 : TEXTURE; 
	sampler DetailTex10Sampler : register(s10)= sampler_state 
	{
		texture = <DetailTex10>;
		MinFilter = Linear;  
		MagFilter = Linear;
		MipFilter = Linear;
		AddressU  = wrap;
		AddressV  = wrap;
	};
#endif
// texture 7
// TODO: reserved for bump map

struct Interpolants
{
  float4 positionSS			: POSITION;         // Screen space position
  float3 tex0				: TEXCOORD0;        // texture coordinates
  float2 tex1				: TEXCOORD1;        // texture coordinates
  float4 tex2				: TEXCOORD2;        // texture coordinates
  half3	 colorDiffuse		: COLOR0;			// diffuse color
};

////////////////////////////////////////////////////////////////////////////////
//
//                              Vertex Shader
//
////////////////////////////////////////////////////////////////////////////////
// Calculates fog factor based upon distance
half CalcFogFactor( half d )
{
    half fogCoeff = 0;
	fogCoeff = (d - g_fogParam.x)/g_fogParam.y;
    return saturate( fogCoeff);
}
Interpolants vertexShader(	float4	Pos			: POSITION,
							float3	Norm		: NORMAL,
							float2	Tex0		: TEXCOORD0,
							float2	Tex1		: TEXCOORD1)
{
	Interpolants o = (Interpolants)0;
	Pos.xyz += g_vertexOffset;
	// screen space position
	o.positionSS = 	mul(Pos, mWorldViewProj);
	// world space position: 
	// TODO: this can be removed if we can do N.L in camera space. 
	float4 worldPos = mul( Pos, mWorld );
	// camera space position
	float4 cameraPos = mul( Pos, mWorldView ); 
	// world space normal
	float3 worldNormal = normalize( mul( Norm, (float3x3)mWorld ) ); 
	
	// calculate light of the sun
	if(g_bEnableSunLight)
	{
		o.colorDiffuse = max(0,dot( sun_vec, worldNormal ))*colorDiffuse+colorAmbient;
	}
	else
	{
		half3 difCol = colorAmbient+colorDiffuse;
		o.colorDiffuse = difCol;
	}
	
	o.tex0.xy = Tex0;
	//save the fog distance
    o.tex0.z = CalcFogFactor(cameraPos.z);
    o.tex1.xy = Tex1;
    if(g_Useshadowmap)
    {
		o.tex2 = mul(Pos, mLightWorldViewProj);
    }
	return o;
}

////////////////////////////////////////////////////////////////////////////////
//
//                              Pixel Shader
//
////////////////////////////////////////////////////////////////////////////////

half4 pixelShader(Interpolants i) : COLOR
{
	half4 o = {0,0,0,1};
	half4 color0;
	half4 color1;
	half3 normalColor;
	half3 colorDif = i.colorDiffuse;
	half alpha;
	
	if(g_bIsBaseEnabled)
	{
		// base color
		color1 = tex2D(DetailTex1Sampler, i.tex1.xy)*tex2D(BaseTexSampler, i.tex0.xy);
		alpha = tex2D(AlphaTex7Sampler, i.tex0.xy).a*color1.a;
		normalColor.xyz = color1.xyz*alpha;
	}
	else
	{
		// alpha layer
		color1 = tex2D(DetailTex1Sampler, i.tex1.xy);
		alpha = tex2D(BaseTexSampler, i.tex0.xy).a*color1.a;
		normalColor.xyz = color1.xyz*alpha;
	}
	
	// alpha layers:
	if(g_bLayer1)
	{
		color1 = tex2D(DetailTex4Sampler, i.tex1.xy);
		alpha = tex2D(AlphaTex3Sampler, i.tex0.xy).a*color1.a;
		
		#ifdef ADDITIVE_LAYERS
			normalColor.xyz = normalColor.xyz+color1.xyz*alpha;
		#else
			normalColor.xyz = lerp(normalColor.xyz,color1.xyz,alpha);
		#endif
		
		if(g_bLayer2)
		{
			color1 = tex2D(DetailTex6Sampler, i.tex1.xy);
			alpha = tex2D(AlphaTex5Sampler, i.tex0.xy).a*color1.a;
			#ifdef ADDITIVE_LAYERS
				normalColor.xyz = normalColor.xyz+color1.xyz*alpha;
			#else
				normalColor.xyz = lerp(normalColor.xyz, color1.xyz,alpha);
			#endif
			
			#ifdef SIMULTANEOUS_TEXTURE_12
				if(g_bLayer3)
				{
					color1 = tex2D(DetailTex8Sampler, i.tex1.xy);
					alpha = tex2D(AlphaTex7Sampler, i.tex0.xy).a*color1.a;
					#ifdef ADDITIVE_LAYERS
						normalColor.xyz = normalColor.xyz+color1.xyz*alpha;
					#else
						normalColor.xyz = lerp(normalColor.xyz, color1.xyz,alpha);
					#endif
					if(g_bLayer4)
					{
						color1 = tex2D(DetailTex10Sampler, i.tex1.xy);
						alpha = tex2D(AlphaTex9Sampler, i.tex0.xy).a*color1.a;
						#ifdef ADDITIVE_LAYERS
							normalColor.xyz = normalColor.xyz+color1.xyz*alpha;
						#else
							normalColor.xyz = lerp(normalColor.xyz, color1.xyz,alpha);
						#endif
					}
				}
			#endif
		}
	}
	
	
	// diffuse color
	if(g_Useshadowmap)
    {
#ifdef USE_DEPTH_SHADOWMAP
		if(g_UseShadowmapHW)
		{
			// hardware shadow map
			half3 shadow    = tex2Dproj(ShadowMapSampler, i.tex2).rgb;
			//colorDif = ((shadow*0.2+0.8) * colorDif)*0.9 + 0.1;
			colorDif = (shadow*0.25+0.75) * colorDif;
		}
		else
		{
			// F32 shadow map
			float2 shadowTexCoord = i.tex2.xy / i.tex2.w;
			float  shadowTestDepth = i.tex2.z / i.tex2.w;
		
			#ifdef	MULTI_SAMPLE_SHADOWMAP
				// transform to texel space
				float2 texelpos = g_nShadowMapSize * shadowTexCoord;
			    
				// Determine the lerp amounts           
				float2 lerps = frac( texelpos );

				//read in bilerp stamp, doing the shadow checks
				//shadowTestDepth = shadowTestDepth- SHADOW_EPSILON;
				color0.x = (tex2D( ShadowMapSampler, shadowTexCoord ) >= shadowTestDepth);  
				texelpos = shadowTexCoord + float2(1.0/g_nShadowMapSize, 0);
				color0.y = (tex2D( ShadowMapSampler, texelpos) >= shadowTestDepth);  
				texelpos = shadowTexCoord + float2(0, 1.0/g_nShadowMapSize);
				color0.z = (tex2D( ShadowMapSampler, texelpos ) >= shadowTestDepth);  
				texelpos = shadowTexCoord + float2(1.0/g_nShadowMapSize, 1.0/g_nShadowMapSize);
				color0.w = (tex2D( ShadowMapSampler, texelpos) >= shadowTestDepth);
			    
				// lerp between the shadow values to calculate our light amount
				float shadow = lerp( lerp( color0.x, color0.y, lerps.x ),
										lerp( color0.z, color0.w, lerps.x ),
										lerps.y );
				lerps.x=(shadowTexCoord.x<0 || shadowTexCoord.y<0 || shadowTexCoord.x>1 || shadowTexCoord.y>1);
				shadow = lerp(shadow, 1, lerps.x);
			#else
				float  shadowDepth = tex2D(ShadowMapSampler, shadowTexCoord);
				float  shadow      = (shadowTestDepth <= shadowDepth);
			#endif
			colorDif = (shadow*0.25+0.75) * colorDif;
		}
#else
		half shadow = tex2Dproj(ShadowMapSampler, i.tex2).b;
		colorDif = (shadow*0.25+0.75) * colorDif;
#endif
    }
    
    normalColor.xyz = normalColor.xyz*colorDif;
	
	// fog
	if(g_bEnableFog)
	{
		half fog = i.tex0.z;
		// fog is only applied to the base pass, second and subsequent passes do not include the fog term
		color0.xyz = lerp(half3(0,0,0), g_fogColor.xyz, g_bIsBaseEnabled);
		o.xyz = lerp(normalColor.xyz, color0.xyz, fog);
	}
	else
		o.xyz = normalColor.xyz;
	return o;
}

half4 pixelShader2(Interpolants i) : COLOR
{
half4 o = {0,0,0,1};
	half4 color0;
	half4 color1;
	half3 normalColor;
	half3 colorDif = i.colorDiffuse;
	half alpha;
	
	if(g_bIsBaseEnabled)
	{
		// base color
		color1 = tex2D(DetailTex1Sampler, i.tex1.xy)*tex2D(BaseTexSampler, i.tex0.xy);
		alpha = tex2D(AlphaTex7Sampler, i.tex0.xy).a*color1.a;
		normalColor.xyz = color1.xyz*alpha;
	}
	else
	{
		// alpha layer
		color1 = tex2D(DetailTex1Sampler, i.tex1.xy);
		alpha = tex2D(BaseTexSampler, i.tex0.xy).a*color1.a;
		normalColor.xyz = color1.xyz*alpha;
	}
	
	// alpha layers:
	if(g_bLayer1)
	{
		color1 = tex2D(DetailTex4Sampler, i.tex1.xy);
		alpha = tex2D(AlphaTex3Sampler, i.tex0.xy).a*color1.a;
		
		#ifdef ADDITIVE_LAYERS
			normalColor.xyz = normalColor.xyz+color1.xyz*alpha;
		#else
			normalColor.xyz = lerp(normalColor.xyz,color1.xyz,alpha);
		#endif
		
		if(g_bLayer2)
		{
			color1 = tex2D(DetailTex6Sampler, i.tex1.xy);
			alpha = tex2D(AlphaTex5Sampler, i.tex0.xy).a*color1.a;
			#ifdef ADDITIVE_LAYERS
				normalColor.xyz = normalColor.xyz+color1.xyz*alpha;
			#else
				normalColor.xyz = lerp(normalColor.xyz, color1.xyz,alpha);
			#endif
			
			#ifdef SIMULTANEOUS_TEXTURE_12
				if(g_bLayer3)
				{
					color1 = tex2D(DetailTex8Sampler, i.tex1.xy);
					alpha = tex2D(AlphaTex7Sampler, i.tex0.xy).a*color1.a;
					#ifdef ADDITIVE_LAYERS
						normalColor.xyz = normalColor.xyz+color1.xyz*alpha;
					#else
						normalColor.xyz = lerp(normalColor.xyz, color1.xyz,alpha);
					#endif
					if(g_bLayer4)
					{
						color1 = tex2D(DetailTex10Sampler, i.tex1.xy);
						alpha = tex2D(AlphaTex9Sampler, i.tex0.xy).a*color1.a;
						#ifdef ADDITIVE_LAYERS
							normalColor.xyz = normalColor.xyz+color1.xyz*alpha;
						#else
							normalColor.xyz = lerp(normalColor.xyz, color1.xyz,alpha);
						#endif
					}
				}
			#endif
		}
	}
	
	
	// diffuse color
	if(g_Useshadowmap)
    {
#ifdef USE_DEPTH_SHADOWMAP 
		if(g_UseShadowmapHW)
		{
			// hardware shadow map
			half3 shadow    = tex2Dproj(ShadowMapSampler, i.tex2).rgb;
			//colorDif = ((shadow*0.2+0.8) * colorDif)*0.9 + 0.1;
			colorDif = (shadow*0.25+0.75) * colorDif;
		}
		else
		{
			// F32 shadow map
			float2 shadowTexCoord = i.tex2.xy / i.tex2.w;
			float  shadowTestDepth = i.tex2.z / i.tex2.w;
			
			float  shadowDepth = tex2D(ShadowMapSampler, shadowTexCoord);
			float  shadow      = (shadowTestDepth <= shadowDepth);
			colorDif = (shadow*0.25+0.75) * colorDif;
		}
#else
		half shadow = tex2Dproj(ShadowMapSampler, i.tex2).b;
		colorDif = (shadow*0.25+0.75) * colorDif;
#endif		
    }
    
    normalColor.xyz = normalColor.xyz*colorDif;
	
	// fog
	if(g_bEnableFog)
	{
		half fog = i.tex0.z;
		// fog is only applied to the base pass, second and subsequent passes do not include the fog term
		color0.xyz = lerp(half3(0,0,0), g_fogColor.xyz, g_bIsBaseEnabled);
		o.xyz = lerp(normalColor.xyz, color0.xyz, fog);
	}
	else
		o.xyz = normalColor.xyz;
	return o;
}


////////////////////////////////////////////////////////////////////////////////
//
//                              shadow map : VS and PS
//
////////////////////////////////////////////////////////////////////////////////

void VertShadow( float4	Pos			: POSITION,
				float3	Norm		: NORMAL,
				float2	Tex0		: TEXCOORD0,
				float2	Tex1		: TEXCOORD1,
                 out float4 oPos	: POSITION,
                 out float2	outTex	: TEXCOORD0,
                 out float2 Depth	: TEXCOORD1 )
{
	Pos.xyz += g_vertexOffset;
    oPos = mul( Pos, mWorldViewProj );
    outTex = Tex0;
    Depth.xy = oPos.zw;
}

float4 PixShadow( float2	inTex		: TEXCOORD0,
				 float2 Depth		: TEXCOORD1) : COLOR
{
	float d = Depth.x / Depth.y;
    return float4(d.xx,1,1);
}

////////////////////////////////////////////////////////////////////////////////
//
//                              Technique
//
////////////////////////////////////////////////////////////////////////////////
technique SimpleMesh_vs30_ps30
{
	pass P0
	{
		// shaders
		VertexShader = compile vs_3_0 vertexShader();
		PixelShader  = compile ps_3_0 pixelShader();
		
		FogEnable = false;
	}
}
technique SimpleMesh_vs20_ps20
{
	pass P0
	{
		// shaders
		VertexShader = compile vs_2_0 vertexShader();
		PixelShader  = compile ps_2_0 pixelShader2();
		
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
        cullmode = none;
    }
}

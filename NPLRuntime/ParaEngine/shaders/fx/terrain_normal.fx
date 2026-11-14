// Author: LiXizhi
// Desc: 2006/4
// terrain texture layers: There are no limit to the total number of layers.
// Fog and base(non-repeatable) textures are applied to all layers
// textures: 0,1,2(shadowmap), 3,4,5,6,7(base non-repeatable layer)
// formula I used: <pass0>{ ( 1 - FogFactor ) * BaseTex7 * [( alpha0 * detail1 ) + alpha3 * detail4 + alpha5 * detail6]} +
//					<pass1>{ ( 1 - FogFactor ) * BaseTex7 * [( alpha0 * detail1 ) + alpha3 * detail4 + alpha5 * detail6]} +
//					<passN>{ ( 1 - FogFactor ) * BaseTex7 * [( alpha0 * detail1 ) + alpha3 * detail4 + alpha5 * detail6]} + FogFactor * FogColor

/**TODO: @def whether use local light */
// #define _USE_LOCAL_LIGHTS

////////////////////////////////////////////////////////////////////////////////
//  Per frame parameters
float4x4 mWorldViewProj: worldviewprojection;
//float4x4 mViewProj: viewprojection;
float4x4 mWorldView: worldview;
float4x4 mWorld: world;
float4x4 mLightWorldViewProj: texworldviewproj;

float3 g_vertexOffset :posScaleOffset = float3(0,0,0);

bool g_bEnableSunLight: sunlightenable;
float4 sun_vec: sunvector;

float3	colorDiffuse:materialdiffuse;
float3	colorAmbient:ambientlight;

float3 g_EyePositionW	:worldcamerapos;

//int		g_nLights		:	locallightnum;
//half4	g_lightcolor0	:	LightColor0;
//float4	g_lightpos0		:	LightPosition0;

bool	g_bIsBaseEnabled:	boolean10;
bool	g_bLayer1		:	boolean11;
bool	g_bLayer2		:	boolean12;

////////////////////////////////////////////////////////////////////////////////
/// per technique parameters
bool	g_bEnableFog	:	fogenable;
float4   g_fogParam		:	fogparameters; // (fogstart, fogrange, fogDensity, reserved)
float4   g_fogColor		:	fogColor;
bool	g_Useshadowmap	:	boolean8;
int		g_nShadowMapSize:	shadowmapsize;
float	g_fShadowRadius :	shadowradius = 40;
float2	g_shadowFactor :shadowfactor = float2(0.35,0.65);


// texture 0
texture AlphaTex0 : TEXTURE; 
sampler AlphaTex0Sampler : register(s0) = sampler_state 
{
    texture = <AlphaTex0>;
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
texture BaseTex7 : TEXTURE; 
sampler BaseTex7Sampler : register(s7)= sampler_state 
{
	texture = <BaseTex7>;
	MinFilter = Linear;  
	MagFilter = Linear;
	MipFilter = Linear;
	AddressU  = Clamp;
	AddressV  = Clamp;
};

struct Interpolants
{
  float4 positionSS			: POSITION;         // Screen space position
  float4 tex0				: TEXCOORD0;        // texture coordinates
  float2 tex1				: TEXCOORD1;        // texture coordinates
  float4 tex2				: TEXCOORD2;        // texture coordinates
  float4 colorDiffuse		: TEXCOORD3;				// diffuse color
};

////////////////////////////////////////////////////////////////////////////////
//
//                              Vertex Shader
//
////////////////////////////////////////////////////////////////////////////////
// Calculates fog factor based upon distance
float CalcFogFactor( float d )
{
    return saturate( (d - g_fogParam.x) / g_fogParam.y );
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
	// camera space position
	float4 cameraPos = mul( Pos, mWorldView ); 
	// world space normal
	// float3 worldNormal = normalize( mul( Norm, (float3x3)mWorld ) ); 
	float3 worldNormal = normalize( Norm);

	// calculate light of the sun
	if(g_bEnableSunLight)
	{
		//o.colorDiffuse.xyz = max(0,dot( sun_vec.xyz, worldNormal ))*float3(1,1,0.7) + colorAmbient;
		o.colorDiffuse.xyz = max(0.0, dot( sun_vec.xyz, worldNormal ))*colorDiffuse + colorAmbient;
		
		float3 worldPos = mul(Pos,mWorld).xyz;
		float3 eyeVec = normalize(g_EyePositionW - worldPos);
		float3 reflectVec = reflect(-sun_vec.xyz,worldNormal);
		float specular = max(dot(eyeVec,reflectVec),0.0001);
		o.colorDiffuse.w = pow(specular,12.0) * 0.6;
	}
	else
	{
		o.colorDiffuse.xyz = max(float3(1,1,1), colorDiffuse+colorAmbient);
		o.colorDiffuse.w = 0.0;
	}
	
	o.tex0.xy = Tex0;
	//save the fog distance
    o.tex0.z = CalcFogFactor(cameraPos.z);
    o.tex1.xy = Tex1;
    if(g_Useshadowmap)
    {
		// shadow blending factor, so that the near camera shadow is darker
		// (z-shadow_fade_start)/shadow_fade_range, so that the shadow end is shadow_fade_start+shadow_fade_range, this should match DEFAULT_SHADOW_RADIUS defined in sceneobject.cpp.
		// o.tex0.w = saturate((cameraPos.z - g_fShadowRadius*0.6)/(g_fShadowRadius*0.4)); // TODO: per vertex distance is inaccurate when shadow range is comparable with triangle
		o.tex0.w = saturate((cameraPos.z - 25)/15);
		o.tex2 = mul(Pos, mLightWorldViewProj);
    }
	return o;
}

Interpolants vertexShader_NoNormal(	float4	Pos			: POSITION,
							float2	Tex0		: TEXCOORD0,
							float2	Tex1		: TEXCOORD1)
{
	Interpolants o = (Interpolants)0;
	Pos.xyz += g_vertexOffset;
	// screen space position
	o.positionSS = 	mul(Pos, mWorldViewProj);
	// camera space position
	float4 cameraPos = mul( Pos, mWorldView ); 
	
	// calculate light of the sun
	if(g_bEnableSunLight)
	{
		o.colorDiffuse.xyz = sun_vec.y * colorDiffuse;
		o.colorDiffuse.xyz += colorAmbient;
	}
	else
	{
		o.colorDiffuse.xyz = max(float3(1,1,1), colorDiffuse+colorAmbient);
	}
	o.colorDiffuse.w = 0.0;

	o.tex0.xy = Tex0;
	//save the fog distance
    o.tex0.z = CalcFogFactor(cameraPos.z);
    o.tex1.xy = Tex1;
    if(g_Useshadowmap)
    {
		// shadow blending factor, so that the near camera shadow is darker
		o.tex0.w = saturate((cameraPos.z - 25)/15); // (z-shadow_fade_start)/shadow_fade_range, // so that the shadow end is 30+20, this should match DEFAULT_SHADOW_RADIUS defined in sceneobject.cpp.
		o.tex2 = mul(Pos, mLightWorldViewProj);
    }
	return o;
}	
////////////////////////////////////////////////////////////////////////////////
//
//                              Pixel Shader
//
////////////////////////////////////////////////////////////////////////////////

float4 pixelShader(Interpolants i) : COLOR0
{
	float4 color1;
	float4 normalColor = float4(0,0,0,1);
	float3 colorDif = i.colorDiffuse.xyz;
	float alpha;
	float specularWeight;

	// layer alpha0 * detail1
	color1 = tex2D(DetailTex1Sampler, i.tex1.xy);

	//gamma
	//color1.xyz = pow(color1.xyz,2.2);

	alpha = tex2D(AlphaTex0Sampler, i.tex0.xy).a;

	normalColor.xyz = color1.xyz*alpha;
	specularWeight = (1.0 - color1.a) * alpha;
	
	// layers alpha3 * detail4 + alpha5 * detail6
	if(g_bLayer1)
	{
		color1 = tex2D(DetailTex4Sampler, i.tex1.xy);

		//gamma
		//color1.xyz = pow(color1.xyz,2.2);

		alpha = tex2D(AlphaTex3Sampler, i.tex0.xy).a;
		normalColor.xyz = normalColor.xyz+color1.xyz*alpha;
		specularWeight += (1.0 - color1.a) * alpha;

		if(g_bLayer2)
		{
			color1 = tex2D(DetailTex6Sampler, i.tex1.xy);
			
			//gamma
			//color1.xyz = pow(color1.xyz,2.2);

			alpha = tex2D(AlphaTex5Sampler, i.tex0.xy).a;
			normalColor.xyz = normalColor.xyz+color1.xyz*alpha;

			specularWeight += (1.0 - color1.a) * alpha;
		}
	}
	
	// multiple base layer
	normalColor.xyz *= tex2D(BaseTex7Sampler, i.tex0.xy).xyz;
	 
	
	// shadow color
	if(g_Useshadowmap)
    {
		float shadow = tex2Dproj(ShadowMapSampler, i.tex2).r;
		shadow = lerp(shadow, 1.0, i.tex0.w);

		// Note: if you modifed this, also modify simple_mesh_normal_shadow.fx
		// colorDif = (shadow*0.6+0.4) * colorDif;
		colorDif *= shadow * g_shadowFactor.x + g_shadowFactor.y;
		
    }
    normalColor.xyz *= colorDif;
	normalColor.xyz += specularWeight * i.colorDiffuse.www * colorDiffuse;


	// fog color
	if(g_bEnableFog)
	{
		normalColor.xyz *= (1.0 - i.tex0.z);
		if (g_bIsBaseEnabled)
			normalColor.xyz += g_fogColor.xyz * i.tex0.z;
	}

	return normalColor;
}

////////////////////////////////////////////////////////////////////////////////
//
//                              terrain in the distance fog: VS and PS
//
////////////////////////////////////////////////////////////////////////////////
struct InFogVSOut
{
	float4 oPos : POSITION;
};

InFogVSOut vertexShader_InFog(	float4	Pos			: POSITION,
							float2	Tex0		: TEXCOORD0,
							float2	Tex1		: TEXCOORD1
							)
{
	InFogVSOut o;
	Pos.xyz += g_vertexOffset;
	o.oPos = mul(Pos, mWorldViewProj);
	return o;
}

float4 pixelShader_InFog(InFogVSOut i) : COLOR0
{
    return g_fogColor;
}

////////////////////////////////////////////////////////////////////////////////
//
//                              shadow map : VS and PS
//
////////////////////////////////////////////////////////////////////////////////

struct TerrainShadowVSOut
{
	float4 oPos : POSITION;
	float2 outTex : TEXCOORD0;
	float2 Depth : TEXCOORD1;
};

TerrainShadowVSOut VertShadow( float4	Pos			: POSITION,
				float2	Tex0		: TEXCOORD0,
				float2	Tex1		: TEXCOORD1 )
{
	TerrainShadowVSOut o;
	Pos.xyz += g_vertexOffset;
    o.oPos = mul( Pos, mWorldViewProj );
    o.outTex = Tex0;
    o.Depth.xy = o.oPos.zw;
    return o;
}

float4 PixShadow( TerrainShadowVSOut s ) : COLOR0
{
	float d = s.Depth.x / s.Depth.y;
    return float4(1.0, d.xx, 1.0);
}


////////////////////////////////////////////////////////////////////////////////
//
//                              Editor mode vs & ps
//
////////////////////////////////////////////////////////////////////////////////
struct TerrainEditorVSOut
{
	float4 oPos : POSITION;
	float4 oColor : COLOR0;
};

TerrainEditorVSOut EditorVSMain(	float4	Pos			: POSITION,
							float4 Color		: COLOR0
							)
{
	TerrainEditorVSOut o;
	Pos.xyz += g_vertexOffset;
	o.oPos = mul(Pos, mWorldViewProj);
	o.oColor = Color;
	return o;
}

float4 EditorPSMain(TerrainEditorVSOut i) : COLOR0
{
    return float4(i.oColor.rgb, 0.25);
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
		VertexShader = compile vs_2_a vertexShader();
		PixelShader  = compile ps_2_a pixelShader();
		
		FogEnable = false;
	}
	pass P1
	{
		// shaders
		VertexShader = compile vs_2_0 vertexShader_InFog();
		PixelShader  = compile ps_2_0 pixelShader_InFog();
		
		FogEnable = false;
	}
	pass P2
	{
		// shaders
		VertexShader = compile vs_2_0 vertexShader_NoNormal();
		PixelShader  = compile ps_2_0 pixelShader();
		
		FogEnable = false;
	}

	pass P3
	{
		VertexShader = compile vs_2_0 EditorVSMain();
		PixelShader  = compile ps_2_0 EditorPSMain();
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

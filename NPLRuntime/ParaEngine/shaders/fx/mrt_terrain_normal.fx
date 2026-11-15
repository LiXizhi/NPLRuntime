// Author: LiXizhi
// Desc: 2013/10
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
float3 sun_vec: sunvector;

float3	colorDiffuse:materialdiffuse;
float3	colorAmbient:ambientlight;

float3 g_EyePositionW	:worldcamerapos;

//int		g_nLights		:	locallightnum;
//float4	g_lightcolor0	:	LightColor0;
//float4	g_lightpos0		:	LightPosition0;

bool	g_bIsBaseEnabled:	boolean10;
bool	g_bLayer1		:	boolean11;
bool	g_bLayer2		:	boolean12;

////////////////////////////////////////////////////////////////////////////////
/// per technique parameters
bool	g_Useshadowmap	:	boolean8;
int		g_nShadowMapSize:	shadowmapsize;
float	g_fShadowRadius :	shadowradius = 40;
float2	g_shadwoFactor :shadowfactor = float2(0.35,0.65);


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
  float2 tex0				: TEXCOORD0;        // texture coordinates
  float3 tex1				: TEXCOORD1;        // texture coordinates
  float4 colorDiffuse		: TEXCOORD2;				// diffuse color
  float3 normal   : TEXCOORD3;
};

struct BlockPSOut
{
	// standard color
	float4 Color	: COLOR0;
	// r is 32bits depth
	float4 Depth	: COLOR1; 
	// r is category id, g is sun light, b is block light
	float4 BlockInfo : COLOR2; 
	// xyz is normal. can be (0,0,0) if no normal is used. 
	float4 Normal : COLOR3; 
};


////////////////////////////////////////////////////////////////////////////////
//
//                              Vertex Shader
//
////////////////////////////////////////////////////////////////////////////////
Interpolants vertexShader(	float4	Pos			: POSITION,
							float3	Norm		: NORMAL,
							float2	Tex0		: TEXCOORD0,
							float2	Tex1		: TEXCOORD1)
{
	Interpolants o;
	Pos.xyz += g_vertexOffset;
	// screen space position
	o.positionSS = 	mul(Pos, mWorldViewProj);
	// camera space position
	float4 cameraPos = mul( Pos, mWorldView ); 
	// world space normal
	//float3 worldNormal = normalize( mul( Norm, (float3x3)mWorld ) ); 
	float3 worldNormal = normalize( Norm);
	o.normal = worldNormal*0.5+0.5;


	// calculate light of the sun
	if(g_bEnableSunLight)
	{
		//o.colorDiffuse.xyz = max(0,dot( sun_vec, worldNormal ))*float3(1,1,0.7) + colorAmbient;
		o.colorDiffuse.xyz = max(0,dot( sun_vec, worldNormal ))*colorDiffuse + colorAmbient;
		
		float3 worldPos = mul(Pos,mWorld);
		float3 eyeVec = normalize(g_EyePositionW - worldPos);
		float3 reflectVec = reflect(-sun_vec,worldNormal);
		float specular = max(dot(eyeVec,reflectVec),0);
		o.colorDiffuse.w = pow(specular,12) * 0.6;
	}
	else
	{
		o.colorDiffuse.xyz = max(1, colorDiffuse+colorAmbient);
		o.colorDiffuse.w = 0;
	}
	
	o.tex0.xy = Tex0;
    o.tex1.xy = Tex1;
	o.tex1.z = cameraPos.z;
	return o;
}

Interpolants vertexShader_NoNormal(	float4	Pos			: POSITION,
							float2	Tex0		: TEXCOORD0,
							float2	Tex1		: TEXCOORD1)
{
	Interpolants o;
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
		o.colorDiffuse.xyz = max(1, colorDiffuse+colorAmbient);
	}
	o.colorDiffuse.w = 0;

	o.tex0.xy = Tex0;
    o.tex1.xy = Tex1;
	o.tex1.z = cameraPos.z;
	o.normal = float3(0,0,0);
   
	return o;
}	
////////////////////////////////////////////////////////////////////////////////
//
//                              Pixel Shader
//
////////////////////////////////////////////////////////////////////////////////

BlockPSOut pixelShader(Interpolants i)
{
	BlockPSOut output;
	float4 color1;
	float4 normalColor = {0,0,0,1};
	float3 colorDif = i.colorDiffuse.xyz;
	float alpha;
	float specularWeight;

	// layer alpha0 * detail1
	color1 = tex2D(DetailTex1Sampler, i.tex1.xy);

	//gamma
	//color1.xyz = pow(color1.xyz,2.2);

	alpha = tex2D(AlphaTex0Sampler, i.tex0.xy).a;

	normalColor.xyz = color1.xyz*alpha;
	specularWeight = (1-color1.a) * alpha;
	
	// layers alpha3 * detail4 + alpha5 * detail6
	if(g_bLayer1)
	{
		color1 = tex2D(DetailTex4Sampler, i.tex1.xy);

		//gamma
		//color1.xyz = pow(color1.xyz,2.2);

		alpha = tex2D(AlphaTex3Sampler, i.tex0.xy).a;
		normalColor.xyz = normalColor.xyz+color1.xyz*alpha;
		specularWeight += (1 - color1.a) * alpha;

		if(g_bLayer2)
		{
			color1 = tex2D(DetailTex6Sampler, i.tex1.xy);
			
			//gamma
			//color1.xyz = pow(color1.xyz,2.2);

			alpha = tex2D(AlphaTex5Sampler, i.tex0.xy).a;
			normalColor.xyz = normalColor.xyz+color1.xyz*alpha;

			specularWeight += (1- color1.a) * alpha;
		}
	}
	
	// multiple base layer
	normalColor.xyz *= tex2D(BaseTex7Sampler, i.tex0.xy).xyz;
	 
	
    normalColor.xyz *= colorDif;
	normalColor.xyz += specularWeight * i.colorDiffuse.www * colorDiffuse;


	output.Color = normalColor;
	
	output.BlockInfo = float4(1, 1, 0, 1);
	output.Depth = float4(i.tex1.z, 0, 0, 1);
	output.Normal = float4(i.normal.xyz, 1);
	return output;
}

////////////////////////////////////////////////////////////////////////////////
//
//                              terrain in the distance fog: VS and PS
//
////////////////////////////////////////////////////////////////////////////////
void vertexShader_InFog(	float4	Pos			: POSITION,
							// float3	Norm		: NORMAL,
							float2	Tex0		: TEXCOORD0,
							float2	Tex1		: TEXCOORD1,
							out float4 oPos	: POSITION
							)
{
	Pos.xyz += g_vertexOffset;
	// screen space position
	oPos = 	mul(Pos, mWorldViewProj);
}

float4 pixelShader_InFog() : COLOR
{
	return float4(1.0, 1.0, 1.0, 1.);
}

////////////////////////////////////////////////////////////////////////////////
//
//                              shadow map : VS and PS
//
////////////////////////////////////////////////////////////////////////////////

void VertShadow( float4	Pos			: POSITION,
				// float3	Norm		: NORMAL,
				float2	Tex0		: TEXCOORD0,
				float2	Tex1		: TEXCOORD1,
                 out float4 oPos	: POSITION,
                 out float4	outTex	: TEXCOORD0)
{
	Pos.xyz += g_vertexOffset;
    oPos = mul( Pos, mWorldViewProj );
    outTex.xy = Tex0;
    outTex.zw = oPos.zw;
}

float4 PixShadow( float4	inTex	: TEXCOORD0) : COLOR
{
	return float4(inTex.z/inTex.w, 0, 0, 1);
	// forcing 1 to disable terrain to cast shadows
	// return float4(1, 0, 0, 1);
}

////////////////////////////////////////////////////////////////////////////////
//
//                              Editor mode vs & ps
//
////////////////////////////////////////////////////////////////////////////////
void EditorVSMain(	float4	Pos			: POSITION,
							float3 Color		: COLOR0,
							out float4 oPos	: POSITION,
							out float3 oColor :COLOR0
							)
{
	Pos.xyz += g_vertexOffset;
	// screen space position
	oPos = 	mul(Pos, mWorldViewProj);
	oColor = Color;
}

float4 EditorPSMain(float3 color:COLOR0) : COLOR
{
    return float4(color,0.25);
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
	pass P1
	{
		// shaders
		VertexShader = compile vs_3_0 vertexShader_InFog();
		PixelShader  = compile ps_3_0 pixelShader_InFog();
		
		FogEnable = false;
	}
	pass P2
	{
		// shaders
		VertexShader = compile vs_3_0 vertexShader_NoNormal();
		PixelShader  = compile ps_3_0 pixelShader();
		
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
        VertexShader = compile vs_3_0 VertShadow();
        PixelShader = compile ps_3_0 PixShadow();
        FogEnable = false;
        cullmode = none;
    }
}

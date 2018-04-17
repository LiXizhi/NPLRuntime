
float4x4 mWorldViewProj:worldviewprojection;
float4x4 mWorldView:worldview;
float4x4 mWorld:world;
float4 sunvector:sunvector;
float4 materialdiffuse:materialdiffuse;
float4 ambientlight:ambientlight;
float4 fogparameters:fogparameters; 
float4 fogColor:fogColor;
bool alphatesting:alphatesting;
float opacity:opacity;

// texture 0
texture tex0 : TEXTURE; 
sampler tex0Sampler : register(s0) = sampler_state 
{
    texture = <tex0>;
};


struct appdata
{
	float4 vertex : POSITION;
	float2 uv : TEXCOORD0;
	float3 normal : NORMAL;
};

struct v2f
{
  float4 vertex			: POSITION;
  float2 uv				: TEXCOORD0;
  float4 color          : COLOR;
};


float CalcFogFactor( float d )
{
    float fogCoeff = 0.0;
	fogCoeff = (d - fogparameters.x) / fogparameters.y;
    return clamp( fogCoeff, 0.0, 1.0);
}


v2f vert(appdata v)
{
	v2f o = (v2f)0;

	// screen space position
	o.vertex = mul(v.vertex, mWorldViewProj);
	o.uv = v.uv;
	float4 cameraPos = mul(v.vertex, mWorldView);
	float3 worldNormal = normalize( mul( v.normal, (float3x3)mWorld ) ); 
	o.color.xyz = max(0.0, dot( sunvector.xyz, worldNormal ))*materialdiffuse.xyz;
	o.color.xyz += ambientlight.xyz;
	//save the fog distance
	o.color.a = CalcFogFactor(cameraPos.z);

	return o;
}

////////////////////////////////////////////////////////////////////////////////
//
//                              Pixel Shader
//
////////////////////////////////////////////////////////////////////////////////


float4 frag(v2f i) : COLOR
{
	float4 albedoColor = tex2D(tex0Sampler, i.uv);
	albedoColor.rgb *= i.color.rgb;
	// this is for alpha testing. 
	if(alphatesting)
	{
		clip(albedoColor.w-0.5);
	}
	float4 ret = float4(0,0,0,1);
	float fog = i.color.a;
	fog = clamp( (fog-0.8)*16.0, 0.0, 1.0);
	
	ret.rgb = lerp(albedoColor.rgb, fogColor.rgb, fog);
	ret.a = lerp(albedoColor.a, 0.0, fog)*opacity;
	return ret;
}

technique default
{
	pass P0
	{
		// shaders
		VertexShader = compile vs_2_0 vert();
		PixelShader  = compile ps_2_0 frag();
	}
}
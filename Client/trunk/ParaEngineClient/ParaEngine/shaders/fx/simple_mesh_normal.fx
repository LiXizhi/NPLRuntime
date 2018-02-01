// Author: LiXizhi
// Desc: 2006/4

// Uniforms

float4x4 worldviewprojection:worldviewprojection;
float4x4 worldview:worldview;
float4x4 world:world;
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

struct Interpolants
{
  float4 pos			: POSITION;         // Screen space position
  float2 uv				: TEXCOORD0;        // texture coordinates
  float4 color          : COLOR;
};

////////////////////////////////////////////////////////////////////////////////
//
//                              Vertex Shader
//
////////////////////////////////////////////////////////////////////////////////


float CalcFogFactor( float d )
{
    float fogCoeff = 0.0;
	fogCoeff = (d - fogparameters.x) / fogparameters.y;
    return clamp( fogCoeff, 0.0, 1.0);
}


Interpolants vertexShader(	float4	Pos			: POSITION,
							float3	Norm		: NORMAL,
							float2  Tex      	: TEXCOORD0)
{
	Interpolants o = (Interpolants)0;
	// screen space position
	/*o.pos = mul(Pos, worldviewprojection);
	o.uv = Tex;

	float4 cameraPos = mul(pos,worldview);
	float3 worldNormal = normalize( mul( Norm, (float3x3)world ) ); 

	o.color.xyz = max(0.0, dot( sunvector.xyz, worldNormal ))*materialdiffuse.xyz;
	o.color.xyz += ambientlight.xyz;
	//save the fog distance
	o.color.a = CalcFogFactor(cameraPos.z);*/

	return o;
}

////////////////////////////////////////////////////////////////////////////////
//
//                              Pixel Shader
//
////////////////////////////////////////////////////////////////////////////////


half4 pixelShader(Interpolants i) : COLOR
{
	/*float4 albedoColor = tex2D(tex0Sampler, i.uv);
	albedoColor.rgb *= i.color.rgb;
	// this is for alpha testing. 
	if(alphatesting && albedoColor.a < 0.05)
		discard;
	
	float fog = i.color.a;
	half4 ret = half4(0,0,0,0);
	ret.rgb = mix(albedoColor.rgb, fogColor.rgb, fog);
	fog = clamp( (fog-0.8)*16.0, 0.0, 1.0);
	ret.a = mix(albedoColor.a, 0.0, fog)*opacity;
	return ret;*/
	return half4(1,0,0,1);
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
		// shaders
		VertexShader = compile vs_2_0 vertexShader();
		PixelShader  = compile ps_2_0 pixelShader();
	}
}
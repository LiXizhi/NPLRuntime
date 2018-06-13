
//a screen space wave effect for under water scene


//waveParam.x :wave offset speed
//waveParam.y :wave dencity
//waveParam.z :wave scalse
float3 waveParam  :register(vs,c0) = float3(1,0.4,0.045);

Texture2D albedoMap; 
sampler2D albedoSpl : register(s0) = sampler_state 
{
    texture = <albedoMap>;
    AddressU  = clamp;        
    AddressV  = clamp;
    AddressW  = clamp;
    MinFilter = linear;
    MagFilter = linear;
    MipFilter = linear;
};

texture noiseMap; 
sampler noiseSpl : register(s1) = sampler_state 
{
    texture = <noiseMap>;
    AddressU  =	wrap;        
    AddressV  =	wrap;
    AddressW  =	wrap;
    MinFilter = linear;
    MagFilter = linear;
    MipFilter = linear;
};

struct VS_OUTPUT
{
   	float4 Position   : POSITION;    
    float4 TexCoord   : TEXCOORD0;
	float3 waveParam  : TEXCOORD1;
};

VS_OUTPUT vs_main(
	float3 position :POSITION,
	float2 texcoord	:TEXCOORD0)
{
	VS_OUTPUT o;
	o.Position = float4(position,1);
	o.TexCoord.xy = texcoord;	
	o.TexCoord.zw = (texcoord + waveParam.x)* waveParam.y;

	float2 offsetDir = normalize(float2(0.5,0.5) - texcoord);
	//o.waveParam.xy = offsetDir * 0.05;
	o.waveParam.xy = offsetDir * waveParam.z;
	
	//wave weight, less offset in screen center
	o.waveParam.z = dot(offsetDir,offsetDir) * 0.8;
	return o;
}


float4 ps_main(VS_OUTPUT input):COLOR
{
	float2 offset = tex2D(noiseSpl,input.TexCoord.zw).xy;
	float2 texcoord = input.TexCoord.xy + input.waveParam.xy * offset;
	float3 color = tex2D(albedoSpl,texcoord).xyz;
	return float4(color,1);
}


////////////////////////////////////////////////////////////////////////////////
//
//                              Technique
//
////////////////////////////////////////////////////////////////////////////////
technique ScreenWave_vs30_ps30 
{
	pass Pass0
    {
		VertexShader = compile vs_1_1 vs_main();
		PixelShader = compile ps_2_0 ps_main();	
    }
}


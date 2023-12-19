float2 screenParam = float2(1280, 720);

texture tex0 : TEXTURE;
sampler tex0Sampler:register(s0) = sampler_state
{
    texture = <tex0>;
    MinFilter = Linear;
    MagFilter = Linear;
    AddressU = clamp;
    AddressV = clamp;
};

texture tex1 : TEXTURE;
sampler tex1Sampler:register(s1) = sampler_state
{
    texture = <tex1>;
	MinFilter = Linear;
	MagFilter = Linear;
	AddressU = clamp;
	AddressV = clamp;
};

void StereoVS(float3 iPosition:POSITION,
	out float4 oPosition:POSITION,
	inout float2 texCoord:TEXCOORD0)
{
	oPosition = float4(iPosition,1);
    //texCoord += 0.5 / screenParam;
}

float4 StereoPS(float2 texCoord:TEXCOORD0):COLOR
{
	float4 Color1 = tex2D(tex1Sampler, texCoord.xy);
	float4 Color2 = tex2D(tex0Sampler, texCoord.xy);

	Color1.r = Color2.r;
	Color1.g = Color1.g;
	Color1.b = Color1.b;
	Color1.a = 1.0f;
	return Color1;
}

float4 InterlacedPS(float2 texCoord : TEXCOORD0) : COLOR
{
    float4 color;
    // interlaced vertically 
    if (floor(fmod(floor(texCoord.x * screenParam.x), 2.0)) == 0.0)
    {
        color = tex2D(tex0Sampler, texCoord.xy);
    }
    else
    {
        color = tex2D(tex1Sampler, texCoord.xy);
    }
    color.a = 1.0f;
    return color;
}


technique Default
{
    pass P0
    {
		VertexShader = compile vs_2_0 StereoVS();
        PixelShader = compile ps_2_0 StereoPS();
    }
    pass P1
    {
        VertexShader = compile vs_2_0 StereoVS();
        PixelShader = compile ps_2_0 InterlacedPS();
    }
}
float2 screenParam = float2(512, 512);

// color buffer tex
texture sourceTexture0;
sampler originColorTexSampler:register(s0) = sampler_state
{
    Texture = <sourceTexture0>;
    MinFilter = Linear;
    MagFilter = Linear;
    AddressU = clamp;
    AddressV = clamp;
};

void VS(float3 iPosition:POSITION,
	out float4 oPosition:POSITION,
	inout float2 texCoord:TEXCOORD0)
{
	oPosition = float4(iPosition,1);
	texCoord += 0.5 / screenParam;
}

// refer to this CV algorithm: https://github.com/kajott/GIPS/blob/main/shaders/Edges/Contours.glsl
float4 FS(float2 texCoord:TEXCOORD0):COLOR
{

	float offset = 1.0 / 2000.0;

	float3 backgroundColor = float3(1.0f, 1.0f, 1.0f);
	float3 outlineColor = float3(0.2f, 0.2f, 0.2f);

	float threshold = 0.5;
	float range = 1.0; 

	// offset arr
	float2 offsets[9] = {
		float2(-offset, offset), // top-left p02 0
		float2( 0.0f, offset), // top-center p12 1
		float2( offset, offset), // top-right p22 2
		float2(-offset, 0.0f), // center-left p01 3
		float2( 0.0f, 0.0f), // center-center 
		float2( offset, 0.0f), // center-right p21 5
		float2(-offset, -offset), // bottom-left p00 6
		float2( 0.0f, -offset), // bottom-center p10 7
		float2( offset, -offset) // bottom-right  p20 8
	};
	float3 sampleTex[9];

	for(int i = 0; i < 9; i++) {
		sampleTex[i] = tex2D(originColorTexSampler, texCoord.xy + offsets[i]).rgb;
	}

	float3 p00 = sampleTex[6];
	float3 p01 = sampleTex[3];
	float3 p02 = sampleTex[0];
	float3 p10 = sampleTex[7];
	float3 p11 = sampleTex[4];
	float3 p12 = sampleTex[1];
	float3 p20 = sampleTex[8];
	float3 p21 = sampleTex[5];
	float3 p22 = sampleTex[2];

	float3 Gv = p00 - p02 + 2.0 * (p10 - p12) + p20 - p22;
	float3 Gh = p00 - p20 + 2.0 * (p01 - p21) + p02 - p22;
	float3 G = sqrt(Gv*Gv + Gh*Gh);

	float3 final = lerp(p11, outlineColor.rgb, min(1.0, max(0.0, max(G.r, max(G.g, G.b)) - threshold) / range));

	return float4(final, 1.0f);
}

technique Default
{
    pass P0
    {
		cullmode = none;
		ZEnable = false;
		ZWriteEnable = false;
		FogEnable = False;
		VertexShader = compile vs_1_1 VS();
        PixelShader = compile ps_2_0 FS();
    }
}
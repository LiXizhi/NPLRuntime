/**
Author: LiXizhi
Company: ParaEngine 
Date: 2014.7.7
Desc: HSV color correction post processing effect
*/
float2 screenParam = float2(512, 512);
float3 colorHSVAdd = float3(0,0,0);
//float3 colorHSVMultiply = float3(1, 1, 1);
float3 colorMultiply = float3(1, 1, 1);

texture sourceTexture0;
sampler sourceSpl:register(s0) = sampler_state
{
    Texture = <sourceTexture0>;
    MinFilter = Linear;
    MagFilter = Linear;
    AddressU = clamp;
    AddressV = clamp;
};


float3 HUEtoRGB(in float H)
{
	float R = abs(H * 6 - 3) - 1;
	float G = 2 - abs(H * 6 - 2);
	float B = 2 - abs(H * 6 - 4);
	return saturate(float3(R, G, B));
}

float RGBCVtoHUE(in float3 RGB, in float C, in float V)
{
	float3 Delta = (V - RGB) / C;
		Delta.rgb -= Delta.brg;
	Delta.rgb += float3(2, 4, 6);
	// NOTE 1
	Delta.brg = step(V, RGB) * Delta.brg;
	float H;
	H = max(Delta.r, max(Delta.g, Delta.b));
	return frac(H / 6);
}

float3 HSVtoRGB(in float3 HSV)
{
	float3 RGB = HUEtoRGB(HSV.x);
		return ((RGB - 1) * HSV.y + 1) * HSV.z;
}

float3 RGBtoHSV(in float3 RGB)
{
	float3 HSV = 0;
		HSV.z = max(RGB.r, max(RGB.g, RGB.b));
	float M = min(RGB.r, min(RGB.g, RGB.b));
	float C = HSV.z - M;
	if (C != 0)
	{
		HSV.x = RGBCVtoHUE(RGB, C, HSV.z);
		HSV.y = C / HSV.z;
	}
	return HSV;
}

void FullScreenQuadVS(float3 iPosition:POSITION,
	out float4 oPosition:POSITION,
	inout float2 texCoord:TEXCOORD0)
{
	oPosition = float4(iPosition,1);
	texCoord += 0.5 / screenParam;
}

float4 FilmScratchPS(float2 texCoord:TEXCOORD0):COLOR
{
	float4 color = tex2D(sourceSpl,texCoord);
	float3 hsv = RGBtoHSV(color.rgb);
	hsv.x += colorHSVAdd.x;
	hsv.y = max(0, hsv.y+colorHSVAdd.y);
	hsv.z += colorHSVAdd.z;
	color.rgb = HSVtoRGB(hsv);
	color.rgb *= colorMultiply.xyz;
	return color;
}

technique Default
{
    pass P0
    {
		cullmode = none;
		ZEnable = false;
		ZWriteEnable = false;
		FogEnable = False;
		VertexShader = compile vs_1_1 FullScreenQuadVS();
        PixelShader = compile ps_2_0 FilmScratchPS();
    }
}
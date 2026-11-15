/**
Author: DreamAndDead
Company: ParaEngine 
Date: 2020.3.31
Desc: HSV color correction post processing effect
*/
float2 screenParam = float2(512, 512);
float amountParam;
float midpointParam;
float roundnessParam;
float featherParam;

texture sourceTexture0;
sampler screen:register(s0) = sampler_state
{
    Texture = <sourceTexture0>;
    MinFilter = Linear;
    MagFilter = Linear;
    AddressU = clamp;
    AddressV = clamp;
};


void VignettingVS(float3 iPosition:POSITION,
					  out float4 oPosition:POSITION,
	                  inout float2 texCoord:TEXCOORD0)
{
	oPosition = float4(iPosition,1);
	texCoord += 0.5 / screenParam;
}

float4 blend(float4 src, float4 dest)
{
	float4 c;

	float a1 = src.a;
	float a2 = dest.a;

	float a = 1 - (1 - a1) * (1 - a2);

	c.r = (a1 * src.r + (1 - a1) * a2 * dest.r) / a;
	c.g = (a1 * src.g + (1 - a1) * a2 * dest.g) / a;
	c.b = (a1 * src.b + (1 - a1) * a2 * dest.b) / a;
	c.a = a;

	return c;
}

float4 VignettingPS(float2 texCoord:TEXCOORD0):COLOR
{
	float4 color = tex2D(screen, texCoord);
	float2 center = float2(0.5, 0.5);

	float4 vig_color = float4(0, 0, 0, 0);
	if (amountParam > 0)
	{
		vig_color = float4(1, 1, 1, clamp(amountParam, 0, 1));
	}
	else
	{
		vig_color = float4(0, 0, 0, clamp(abs(amountParam), 0, 1));
	}
	float midpoint = clamp(midpointParam, 0, 1);
	float roundness = clamp(roundnessParam, -1, 1);
	float feather = clamp(featherParam, 0, 1);


	/*
	 * 0   -> 0.5 - (sqrt(2) * 0.5 - 0.5) = 0.2929
	 * 0.5 -> 0.5
	 * 1   -> sqrt(2) * 0.5 = 0.7071
	 */
	float limit = 0.2929 + midpoint * (0.7071 - 0.2929);

	if (roundness < 0)
	{
		// when no feather
		// roundness = 0, radius = 0.5
		// roundness = -1, radius = 0
		float radius = limit + roundness * limit;

		float inner_limit = limit * (1 - feather);
		float outer_limit = limit * (1 + feather);

		float inner_radius = inner_limit * (1 + roundness);
		float outer_radius = outer_limit * (1 + roundness);

		float c = inner_limit - inner_radius;

		float2 p = texCoord - center;
		float x = abs(p.x);
		float y = abs(p.y);

		if (x > c && y > c)
		{
			float d = distance(float2(x, y), float2(c, c));
			float alpha = vig_color.a * clamp((d - inner_radius) / (outer_limit - inner_limit), 0, 1);
			color = blend(float4(vig_color.rgb, alpha), color);
		}
		else
		{
			float d = max(x, y);
			float alpha = vig_color.a * clamp((d - inner_limit) / (outer_limit - inner_limit), 0, 1);
			color = blend(float4(vig_color.rgb, alpha), color);
		}
	} else { // roundness >= 0
		// roundness = 0, a = limit, b = limit
		// roundness = 1, a = limit - 0.5 * limit, b = limit + 0.5 * limit
		float radius_delta = limit * 0.5 * roundness;
		float a = limit - radius_delta;
		float b = limit + radius_delta;

		float feather_a_delta = a * feather;
		float inner_a = a - feather_a_delta;
		float outer_a = a + feather_a_delta;

		float2 p = texCoord - center;

		float ratio = b / a;
		float eclipse_a = sqrt(pow(p.x, 2) + pow(p.y, 2) / pow(ratio, 2));

		float alpha = vig_color.a * clamp((eclipse_a - inner_a) / (outer_a - inner_a), 0, 1);

		color = blend(float4(vig_color.rgb, alpha), color);
	}

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
		VertexShader = compile vs_1_1 VignettingVS();
        PixelShader = compile ps_2_0 VignettingPS();
    }
}
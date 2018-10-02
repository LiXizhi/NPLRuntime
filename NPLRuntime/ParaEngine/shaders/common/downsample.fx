
// texture 0
texture tex0 : TEXTURE0;
sampler ColorTextureSampler = sampler_state 
{
    texture = <tex0>;
	MinFilter = POINT;
	MagFilter = POINT;
	AddressU = clamp;
	AddressV = clamp;
};

struct appdata
{
	float3 vertex 	: POSITION;
};

struct v2f
{
	float4 pos	: POSITION;
	float2 uv:TEXCOORD0;
};

v2f vert(appdata i)
{
	v2f o = (v2f)0;
    o.pos =  float4(i.vertex,1);
    o.uv  =  float2(i.vertex.xy * 0.5 + 0.5);
	// flip vertical
	o.uv = float2(o.uv.x,1 - o.uv.y);
	return o;
}

float4 frag(v2f i) : COLOR
{
   return tex2D(ColorTextureSampler,i.uv);
}

technique Default
{
	pass P0
	{
		// shaders
		ZEnable = false;
		ZWriteEnable = false;
		AlphaBlendEnable = false;
		VertexShader = compile vs_2_0 vert();
		PixelShader  = compile ps_2_0 frag();
	}
}
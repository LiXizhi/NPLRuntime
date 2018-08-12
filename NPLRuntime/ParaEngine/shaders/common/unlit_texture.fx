
float4x4 mWorldViewProj:worldviewprojection;

// texture 0
texture tex0 : TEXTURE; 
sampler tex0Sampler : TEXTURE0 = sampler_state 
{
    texture = <tex0>;
};

struct appdata
{
	float4 vertex : POSITION;
	float2 uv : TEXCOORD0;
};

struct v2f
{
  float4 vertex			: POSITION;
  float2 uv				: TEXCOORD0;
};


v2f vert(appdata v)
{
	v2f o = (v2f)0;
	// screen space position
	o.vertex = mul(v.vertex, mWorldViewProj);
	o.uv = v.uv;
	return o;
}

float4 frag(v2f i) : COLOR
{
	float4 color = tex2D(tex0Sampler, i.uv);
	return color;
}

technique SimpleMesh_vs20_ps20
{
	pass P0
	{
		// shaders
		VertexShader = compile vs_2_0 vert();
		PixelShader  = compile ps_2_0 frag();
	}
}
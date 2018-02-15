
float4x4 PARA_MATRIX_MVP:worldviewprojection;


struct appdata
{
	float4 vertex : POSITION;
	float4 color : COLOR;
};

struct v2f
{
  float4 vertex			: POSITION;
  float4 color          : COLOR;
};



v2f vert(appdata v)
{
	v2f o = (v2f)0;
	o.vertex = mul(v.vertex, PARA_MATRIX_MVP);
	o.color = v.color;
	return o;
}

float4 frag(v2f i) : COLOR
{
	return i.color;
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
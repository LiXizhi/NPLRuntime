// Author: LiXizhi
// Data: 2006/7
// Desc: for displaying GUI object. 

////////////////////////////////////////////////////////////////////////////////
//  Per frame parameters
float4x4 mWorldViewProj: worldviewprojection;
bool k_bBoolean0:boolean0;
// texture 0
texture tex0 : TEXTURE; 
sampler tex0Sampler : register(s0) = sampler_state 
{
    texture = <tex0>;
};

struct v2f
{
  float4 vertex			: POSITION;         // Screen space position
  float2 uv				: TEXCOORD0;        // texture coordinates
  float4 color          : COLOR0;
};


struct appdata
{
	float4 vertex 	: POSITION;
	float2 uv   	: TEXCOORD0;
	float4 color 	: COLOR0;	 
};


v2f vert(appdata v)
{
	v2f o = (v2f)0;
	o.vertex = 	mul(v.vertex, mWorldViewProj);
	o.uv = v.uv;
    o.color = v.color;
	return o;
}

float4 frag(v2f i) : COLOR
{
	float4 normalColor = tex2D(tex0Sampler, i.uv);
    if(k_bBoolean0)
    {
        return i.color * normalColor;
    }else
    {
        return float4(i.color.rgb,i.color.a * normalColor.a);
    }
	return normalColor;
}

technique SimpleMesh_vs20_ps20
{
	pass P0
	{
		// shaders
		VertexShader = compile vs_2_0 vert();
		PixelShader  = compile ps_2_0 frag();
		
		FogEnable = false;
	}
}
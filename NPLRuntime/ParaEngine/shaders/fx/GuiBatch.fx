// The third texture coordinate selects one of four bound sprite textures.
// Glyph slots are negative so their alpha mask can share a draw with images.
float4x4 mWorldViewProj : worldviewprojection;
sampler2D tex0Sampler : register(s0);
sampler2D tex1Sampler : register(s1);
sampler2D tex2Sampler : register(s2);
sampler2D tex3Sampler : register(s3);

struct appdata {
    float4 vertex : POSITION;
    float3 uv : TEXCOORD0;
    float4 color : COLOR0;
};

struct v2f {
    float4 vertex : POSITION;
    float3 uv : TEXCOORD0;
    float4 color : COLOR0;
};

v2f vert(appdata v) {
    v2f o = (v2f)0;
    o.vertex = mul(v.vertex, mWorldViewProj);
    o.uv = v.uv;
    o.color = v.color;
    return o;
}

float4 frag(v2f i) : COLOR {
    float2 dx = ddx(i.uv.xy);
    float2 dy = ddy(i.uv.xy);
    float slot = abs(i.uv.z);
    float4 sampled;
    if (slot < 1.5)
        sampled = tex2Dgrad(tex0Sampler, i.uv.xy, dx, dy);
    else if (slot < 2.5)
        sampled = tex2Dgrad(tex1Sampler, i.uv.xy, dx, dy);
    else if (slot < 3.5)
        sampled = tex2Dgrad(tex2Sampler, i.uv.xy, dx, dy);
    else
        sampled = tex2Dgrad(tex3Sampler, i.uv.xy, dx, dy);
    if (i.uv.z < 0)
        return float4(i.color.rgb, i.color.a * sampled.a);
    return i.color * sampled;
}

technique GuiBatch {
    pass P0 {
        VertexShader = compile vs_2_0 vert();
        PixelShader = compile ps_2_0 frag();
    }
}

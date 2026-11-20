// The third texture coordinate selects one of eight bound sprite textures.
// Glyph slots are negative so their alpha mask can share a draw with images.
float4x4 mWorldViewProj : worldviewprojection;
sampler2D tex0Sampler : register(s0);
sampler2D tex1Sampler : register(s1);
sampler2D tex2Sampler : register(s2);
sampler2D tex3Sampler : register(s3);

#ifdef GUI_BATCH_EIGHT_TEXTURES
sampler2D tex4Sampler : register(s4);
sampler2D tex5Sampler : register(s5);
sampler2D tex6Sampler : register(s6);
sampler2D tex7Sampler : register(s7);
#endif

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
#ifdef GUI_BATCH_EIGHT_TEXTURES
    else if (slot < 4.5)
        sampled = tex2Dgrad(tex3Sampler, i.uv.xy, dx, dy);
    else if (slot < 5.5)
        sampled = tex2Dgrad(tex4Sampler, i.uv.xy, dx, dy);
    else if (slot < 6.5)
        sampled = tex2Dgrad(tex5Sampler, i.uv.xy, dx, dy);
    else if (slot < 7.5)
        sampled = tex2Dgrad(tex6Sampler, i.uv.xy, dx, dy);
    else
        sampled = tex2Dgrad(tex7Sampler, i.uv.xy, dx, dy);
#else
    else
        sampled = tex2Dgrad(tex3Sampler, i.uv.xy, dx, dy);
#endif
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

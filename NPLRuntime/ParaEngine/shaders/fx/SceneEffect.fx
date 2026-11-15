float2 screenParam = float2(512, 512);
float4 effectParams = float4(1, 1, 1, 1);
texture sourceTexture0;
sampler sourceSampler : register(s0) = sampler_state
{
    Texture = <sourceTexture0>;
    MinFilter = Linear;
    MagFilter = Linear;
    AddressU = clamp;
    AddressV = clamp;
};

void VS(float3 position : POSITION, out float4 outputPosition : POSITION, inout float2 uv : TEXCOORD0)
{
    outputPosition = float4(position, 1);
    uv += 0.5 / screenParam;
}

float3 Sample(float2 uv) { return tex2D(sourceSampler, uv).rgb; }
float Luma(float3 color) { return dot(color, float3(0.299, 0.587, 0.114)); }
float4 Mix(float2 uv, float3 color) { return float4(lerp(Sample(uv), saturate(color), effectParams.x), 1); }
float Edge(float2 uv)
{
    float2 delta = effectParams.y / screenParam;
    float3 horizontal = Sample(uv + float2(delta.x, 0)) - Sample(uv - float2(delta.x, 0));
    float3 vertical = Sample(uv + float2(0, delta.y)) - Sample(uv - float2(0, delta.y));
    return saturate((length(horizontal) + length(vertical) - effectParams.z) * effectParams.w);
}
float3 Blur(float2 uv)
{
    float2 delta = effectParams.y / screenParam;
    return (Sample(uv) * 4 + Sample(uv + delta) + Sample(uv - delta) +
        Sample(uv + float2(delta.x, -delta.y)) + Sample(uv + float2(-delta.x, delta.y))) / 8;
}
float4 Grayscale(float2 uv:TEXCOORD0):COLOR { return Mix(uv, Luma(Sample(uv)).xxx); }
float4 Sepia(float2 uv:TEXCOORD0):COLOR
{
    float3 color = Sample(uv);
    return Mix(uv, float3(dot(color, float3(.393,.769,.189)), dot(color, float3(.349,.686,.168)), dot(color, float3(.272,.534,.131))));
}
float4 Invert(float2 uv:TEXCOORD0):COLOR { return Mix(uv, 1 - Sample(uv)); }
float4 Warm(float2 uv:TEXCOORD0):COLOR { return Mix(uv, Sample(uv) * float3(1.15,1.02,.85)); }
float4 Cool(float2 uv:TEXCOORD0):COLOR { return Mix(uv, Sample(uv) * float3(.85,1.02,1.18)); }
float4 Vintage(float2 uv:TEXCOORD0):COLOR { return Mix(uv, Sample(uv) * float3(.9,.78,.65) + float3(.12,.09,.08)); }
float4 Fade(float2 uv:TEXCOORD0):COLOR { return Mix(uv, Sample(uv) * .7 + .15); }
float4 Contrast(float2 uv:TEXCOORD0):COLOR { return Mix(uv, (Sample(uv) - .5) * effectParams.y + .5); }
float4 Exposure(float2 uv:TEXCOORD0):COLOR { return Mix(uv, Sample(uv) * exp2(effectParams.y)); }
float4 Saturation(float2 uv:TEXCOORD0):COLOR { float3 color = Sample(uv); return Mix(uv, lerp(Luma(color).xxx, color, effectParams.y)); }
float4 Posterize(float2 uv:TEXCOORD0):COLOR { return Mix(uv, floor(Sample(uv) * effectParams.y + .5) / effectParams.y); }
float4 Threshold(float2 uv:TEXCOORD0):COLOR { return Mix(uv, step(effectParams.y, Luma(Sample(uv))).xxx); }
float4 Solarize(float2 uv:TEXCOORD0):COLOR { float3 color = Sample(uv); return Mix(uv, lerp(color, 1-color, step(effectParams.y, color))); }
float4 Duotone(float2 uv:TEXCOORD0):COLOR { return Mix(uv, lerp(float3(.08,.02,.2), float3(1,.8,.25), Luma(Sample(uv)))); }
float4 Nightvision(float2 uv:TEXCOORD0):COLOR { return Mix(uv, Luma(Sample(uv)) * float3(.15,1.5,.25)); }
float4 Thermal(float2 uv:TEXCOORD0):COLOR { float heat = Luma(Sample(uv)); return Mix(uv, saturate(1.5 - abs(4 * heat - float3(3,2,1)))); }
float4 Cartoon(float2 uv:TEXCOORD0):COLOR { return Mix(uv, Sample(uv) * (1 - Edge(uv))); }
float4 Sketch(float2 uv:TEXCOORD0):COLOR { return Mix(uv, (1 - Edge(uv)).xxx); }
float4 Neon(float2 uv:TEXCOORD0):COLOR { return Mix(uv, saturate(Sample(uv) * 2 + .2) * Edge(uv)); }
float4 Emboss(float2 uv:TEXCOORD0):COLOR { return Mix(uv, .5 + (Sample(uv + effectParams.y / screenParam) - Sample(uv - effectParams.y / screenParam)) * 2); }
float4 Pixelate(float2 uv:TEXCOORD0):COLOR { float2 cells = screenParam / effectParams.y; return Mix(uv, Sample((floor(uv * cells) + .5) / cells)); }
float4 Softblur(float2 uv:TEXCOORD0):COLOR { return Mix(uv, Blur(uv)); }
float4 Sharpen(float2 uv:TEXCOORD0):COLOR { return Mix(uv, Sample(uv) + (Sample(uv) - Blur(uv)) * effectParams.z); }
float4 Chromatic(float2 uv:TEXCOORD0):COLOR { float2 delta = float2(effectParams.y / screenParam.x, 0); return Mix(uv, float3(Sample(uv + delta).r, Sample(uv).g, Sample(uv - delta).b)); }
float4 Vignette(float2 uv:TEXCOORD0):COLOR { float2 centered = (uv - .5) * 2; return Mix(uv, Sample(uv) * (1 - saturate(dot(centered,centered) * effectParams.y))); }
float4 Scanlines(float2 uv:TEXCOORD0):COLOR { return Mix(uv, Sample(uv) * (1 - effectParams.z * step(.5, frac(uv.y * screenParam.y / effectParams.y)))); }
float4 Halftone(float2 uv:TEXCOORD0):COLOR { float2 cell = frac(uv * screenParam / effectParams.y) - .5; return Mix(uv, step(dot(cell,cell), Luma(Sample(uv)) * .5).xxx); }
float4 Mirror(float2 uv:TEXCOORD0):COLOR { return Mix(uv, Sample(float2(abs(uv.x - .5) * 2, uv.y))); }
float4 Fisheye(float2 uv:TEXCOORD0):COLOR { float2 centered = uv - .5; return Mix(uv, Sample(.5 + centered * (1 + effectParams.y * dot(centered,centered)))); }
float4 Swirl(float2 uv:TEXCOORD0):COLOR
{
    float2 centered = uv - .5;
    float angle = effectParams.y * saturate(1 - length(centered) * 2);
    float sine = sin(angle), cosine = cos(angle);
    return Mix(uv, Sample(.5 + float2(centered.x*cosine-centered.y*sine, centered.x*sine+centered.y*cosine)));
}

#define EFFECT_PASS(Name) pass Name { CullMode = None; ZEnable = false; ZWriteEnable = false; FogEnable = false; AlphaBlendEnable = false; VertexShader = compile vs_1_1 VS(); PixelShader = compile ps_2_0 Name(); }
technique Default
{
    EFFECT_PASS(Grayscale)
    EFFECT_PASS(Sepia)
    EFFECT_PASS(Invert)
    EFFECT_PASS(Warm)
    EFFECT_PASS(Cool)
    EFFECT_PASS(Vintage)
    EFFECT_PASS(Fade)
    EFFECT_PASS(Contrast)
    EFFECT_PASS(Exposure)
    EFFECT_PASS(Saturation)
    EFFECT_PASS(Posterize)
    EFFECT_PASS(Threshold)
    EFFECT_PASS(Solarize)
    EFFECT_PASS(Duotone)
    EFFECT_PASS(Nightvision)
    EFFECT_PASS(Thermal)
    EFFECT_PASS(Cartoon)
    EFFECT_PASS(Sketch)
    EFFECT_PASS(Neon)
    EFFECT_PASS(Emboss)
    EFFECT_PASS(Pixelate)
    EFFECT_PASS(Softblur)
    EFFECT_PASS(Sharpen)
    EFFECT_PASS(Chromatic)
    EFFECT_PASS(Vignette)
    EFFECT_PASS(Scanlines)
    EFFECT_PASS(Halftone)
    EFFECT_PASS(Mirror)
    EFFECT_PASS(Fisheye)
    EFFECT_PASS(Swirl)
}
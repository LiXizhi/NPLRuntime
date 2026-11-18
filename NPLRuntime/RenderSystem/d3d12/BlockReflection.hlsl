RaytracingAccelerationStructure scene : register(t0);
Texture2D<float> depth : register(t1);
Texture2D<float4> normals : register(t2);
Texture2D<float4> materials : register(t3);
Texture2D<float4> sceneColor : register(t4);
RWByteAddressBuffer result : register(u0);

cbuffer View : register(b0)
{
    row_major float4x4 inverseView;
    float4 volume;
    float4 viewport;
    float2 lens;
    float daylight;
    float padding;
    float3 sun;
    float reserved;
};

static const float3 faceNormals[6] = {
    float3(0,0,-1), float3(0,0,1), float3(-1,0,0),
    float3(1,0,0), float3(0,-1,0), float3(0,1,0)
};

uint TraceReflection(uint2 pixel, float2 uv, float z, uint category, uint2 size)
{
    float2 ndc = ((uv - viewport.zw) / viewport.xy * 2 - 1) * float2(1,-1);
    float3 eyeRay = mul(float3(ndc * lens, 1), (float3x3)inverseView);
    float3 pos = (inverseView[3].xyz + eyeRay * z - volume.xyz) * volume.w;
    if (any(pos < 0) || any(pos >= 32)) return 0;
    float3 n = normalize(normals[pixel].xyz * 2 - 1);
    if (category == 50 && all(pixel + 1 < size))
    {
        // The metal normal map also drives small glints. Keep its full amplitude
        // for lighting, but constrain mirror distortion around the actual face.
        float zx = depth[pixel + uint2(1,0)], zy = depth[pixel + uint2(0,1)];
        float3 dx = eyeRay * (zx - z) + inverseView[0].xyz * (2 * lens.x * zx / (size.x * viewport.x));
        float3 dy = eyeRay * (zy - z) - inverseView[1].xyz * (2 * lens.y * zy / (size.y * viewport.y));
        float3 geometric = cross(dx, dy);
        if (dot(geometric, geometric) > 1e-12 && max(abs(zx-z), abs(zy-z)) * volume.w < 0.35)
        {
            geometric = normalize(geometric);
            if (dot(geometric, eyeRay) > 0) geometric = -geometric;
            n = normalize(lerp(geometric, n, 0.16));
        }
    }
    float3 direction = reflect(normalize(eyeRay), n);
    RayDesc ray;
    ray.Origin = pos + n * 0.045;
    ray.Direction = direction;
    ray.TMin = 0.025;
    ray.TMax = 50;
    RayQuery<RAY_FLAG_FORCE_OPAQUE> query;
    query.TraceRayInline(scene, RAY_FLAG_NONE, 255, ray);
    while (query.Proceed()) {}
    if (query.CommittedStatus() != COMMITTED_TRIANGLE_HIT) return 0;
    uint rgb = query.CommittedInstanceID();
    float3 color = float3((rgb >> 16) & 255, (rgb >> 8) & 255, rgb & 255) / 255.0;
    float3 face = faceNormals[query.CommittedPrimitiveIndex() / 2];
    float direct = saturate(dot(face, sun));
    if (direct > 0)
    {
        RayDesc shadow;
        shadow.Origin = ray.Origin + direction * query.CommittedRayT() + face * 0.015;
        shadow.Direction = sun;
        shadow.TMin = 0.01;
        shadow.TMax = 40;
        RayQuery<RAY_FLAG_FORCE_OPAQUE | RAY_FLAG_ACCEPT_FIRST_HIT_AND_END_SEARCH> blocker;
        blocker.TraceRayInline(scene, RAY_FLAG_NONE, 255, shadow);
        blocker.Proceed();
        if (blocker.CommittedStatus() == COMMITTED_TRIANGLE_HIT) direct = 0;
    }
    float lighting = lerp(0.18, 0.60, daylight) + direct * lerp(0.12, 0.55, daylight);
    color *= sqrt(lighting);
    // Reuse the shaded texel only if it depicts the DXR closest hit. Off-screen
    // or occluded hits retain the world material and ray-tested lighting.
    float3 hit = ray.Origin + direction * query.CommittedRayT();
    float3 delta = hit / volume.w + volume.xyz - inverseView[3].xyz;
    float3 hitView = mul((float3x3)inverseView, delta);
    if (hitView.z > 0.01 && all(lens > 0))
    {
        float2 localUV = hitView.xy / (lens * hitView.z) * float2(0.5, -0.5) + 0.5;
        if (all(localUV > 0) && all(localUV < 1))
        {
            uint2 samplePixel = min(uint2((localUV * viewport.xy + viewport.zw) * size), size - 1);
            if (abs(depth[samplePixel] - hitView.z) * volume.w < 0.15)
                color = sceneColor[samplePixel].rgb;
        }
    }
    uint3 lit = uint3(round(saturate(color) * 255));
    return lit.r | (lit.g << 8) | (lit.b << 16) | 0xff000000;
}

[RootSignature("SRV(t0),DescriptorTable(SRV(t1,numDescriptors=4)),UAV(u0),RootConstants(num32BitConstants=32,b0)")]
[numthreads(8,8,1)]
void main(uint3 id : SV_DispatchThreadID)
{
    uint w,h;
    depth.GetDimensions(w,h);
    if (id.x >= w || id.y >= h) return;
    uint2 p = id.xy;
    uint category = uint(materials[p].r * 255 + 0.4);
    float2 uv = (float2(p) + 0.5) / float2(w,h);
    uint packed = 0;
    if ((category == 8 || category == 9 || category == 50) && depth[p] > 0.01 &&
        all(uv >= viewport.zw) && all(uv < viewport.zw + viewport.xy))
        packed = TraceReflection(p, uv, depth[p], category, uint2(w,h));
    result.Store((p.y * ((w + 63) & ~63u) + p.x) * 4, packed);
}

RaytracingAccelerationStructure scene : register(t0);
Texture2D<float> depth : register(t1);
Texture2D<float4> normals : register(t2);
Texture2D<float4> materials : register(t3);
Texture2D<float4> sceneColor : register(t4);
RaytracingAccelerationStructure playerScene : register(t5);
ByteAddressBuffer playerVertices : register(t6);
ByteAddressBuffer playerIndices : register(t7);
Texture2D<float4> skins[16] : register(t8);
SamplerState skinSampler : register(s0);
RWByteAddressBuffer result : register(u0);

cbuffer View : register(b0)
{
    row_major float4x4 inverseView;
    float4 volume;
    float4 viewport;
    float2 lens;
    float daylight;
    uint materialByteOffset;
    float3 sun;
    float hasPlayer;
};

static const float3 faceNormals[6] = {
    float3(0,0,-1), float3(0,0,1), float3(-1,0,0),
    float3(1,0,0), float3(0,-1,0), float3(0,1,0)
};

float Coverage(float3 delta)
{
    float3 extent = abs(delta);
    return 1 - smoothstep(8.0, 14.0, max(extent.x, max(extent.y, extent.z)));
}

float3 UnpackRGB(uint rgb)
{
    return float3((rgb >> 16) & 255, (rgb >> 8) & 255, rgb & 255) / 255.0;
}

float4 PlayerTexel(uint primitive, float2 bary, uint slot)
{
    if (slot >= 16) return 1;
    uint3 offsets = playerIndices.Load3(primitive * 12) * 24 + 16;
    float2 a = asfloat(playerVertices.Load2(offsets.x));
    float2 b = asfloat(playerVertices.Load2(offsets.y));
    float2 c = asfloat(playerVertices.Load2(offsets.z));
    return skins[NonUniformResourceIndex(slot)].SampleLevel(skinSampler, a * (1-bary.x-bary.y) + b * bary.x + c * bary.y, 0);
}

uint TraceReflection(uint2 pixel, float2 uv, float z, uint category, uint2 size)
{
    float2 ndc = ((uv - viewport.zw) / viewport.xy * 2 - 1) * float2(1,-1);
    float3 eyeRay = mul(float3(ndc * lens, 1), (float3x3)inverseView);
    float3 pos = (inverseView[3].xyz + eyeRay * z - volume.xyz) * volume.w;
    float3 eye = (inverseView[3].xyz - volume.xyz) * volume.w;
    float weight = Coverage(pos - eye);
    if (weight <= 0 || any(pos < 0) || any(pos >= 32)) return 0;
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
    bool blockHit = query.CommittedStatus() == COMMITTED_TRIANGLE_HIT;
    float hitT = blockHit ? query.CommittedRayT() : ray.TMax;
    float3 color = 0, face = 0;
    if (blockHit)
    {
        color = UnpackRGB(query.CommittedInstanceID());
        face = faceNormals[query.CommittedPrimitiveIndex() / 2];
    }
    if (hasPlayer > 0)
    {
        RayQuery<RAY_FLAG_NONE> player;
        ray.TMax = hitT;
        player.TraceRayInline(playerScene, RAY_FLAG_NONE, 255, ray);
        while (player.Proceed())
        {
            uint primitive = player.CandidatePrimitiveIndex();
            uint material = playerIndices.Load(materialByteOffset + 4 * primitive);
            if ((material & 0x80000000) == 0 || PlayerTexel(primitive, player.CandidateTriangleBarycentrics(), material & 0x7fffffff).a >= 0.5)
                player.CommitNonOpaqueTriangleHit();
        }
        if (player.CommittedStatus() == COMMITTED_TRIANGLE_HIT)
        {
            hitT = player.CommittedRayT();
            uint primitive = player.CommittedPrimitiveIndex();
            uint3 offsets = playerIndices.Load3(primitive * 12) * 24;
            uint4 a = playerVertices.Load4(offsets.x), b = playerVertices.Load4(offsets.y), c = playerVertices.Load4(offsets.z);
            float2 uv = player.CommittedTriangleBarycentrics();
            color = UnpackRGB(a.w) * (1-uv.x-uv.y) + UnpackRGB(b.w) * uv.x + UnpackRGB(c.w) * uv.y;
            color *= PlayerTexel(primitive, uv, playerIndices.Load(materialByteOffset + 4 * primitive) & 0x7fffffff).rgb;
            face = normalize(cross(asfloat(b.xyz) - asfloat(a.xyz), asfloat(c.xyz) - asfloat(a.xyz)));
            if (dot(face, direction) > 0) face = -face;
        }
        else if (!blockHit) return 0;
    }
    else if (!blockHit) return 0;
    float3 hit = ray.Origin + direction * hitT;
    weight = min(weight, Coverage(hit - eye));
    if (weight <= 0) return 0;
    float direct = saturate(dot(face, sun));
    if (direct > 0)
    {
        RayDesc shadow;
        shadow.Origin = hit + face * 0.015;
        shadow.Direction = sun;
        shadow.TMin = 0.01;
        shadow.TMax = 40;
        RayQuery<RAY_FLAG_FORCE_OPAQUE> blocker;
        blocker.TraceRayInline(scene, RAY_FLAG_NONE, 255, shadow);
        while (blocker.Proceed()) {}
        if (blocker.CommittedStatus() == COMMITTED_TRIANGLE_HIT)
            direct *= 1 - Coverage(shadow.Origin + sun * blocker.CommittedRayT() - eye);
    }
    float lighting = lerp(0.18, 0.60, daylight) + direct * lerp(0.12, 0.55, daylight);
    color *= sqrt(lighting);
    // Reuse the shaded texel only if it depicts the DXR closest hit. Off-screen
    // or occluded hits retain the world material and ray-tested lighting.
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
    return lit.r | (lit.g << 8) | (lit.b << 16) | (uint(round(weight * 255)) << 24);
}

[RootSignature("SRV(t0),DescriptorTable(SRV(t1,numDescriptors=4),SRV(t8,numDescriptors=16)),UAV(u0),RootConstants(num32BitConstants=32,b0),SRV(t5),SRV(t6),SRV(t7),StaticSampler(s0, filter=FILTER_MIN_MAG_MIP_LINEAR, addressU=TEXTURE_ADDRESS_WRAP, addressV=TEXTURE_ADDRESS_WRAP)")]
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

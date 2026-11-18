// World-space probe atlas. Each thread traces a cosine-weighted upper hemisphere
// at the center of one air cell, including geometry outside the camera frustum.
RaytracingAccelerationStructure scene : register(t0);
RWByteAddressBuffer visibility : register(u0);

[RootSignature("SRV(t0), UAV(u0)")]
[numthreads(4, 4, 4)]
void main(uint3 cell : SV_DispatchThreadID)
{
    RayDesc ray;
    ray.Origin = float3(cell) + 0.5;
    ray.TMin = 0.01;
    ray.TMax = 8.0;
    float visible = 0.0;
    const uint samples = 16;
    for (uint i = 0; i < samples; ++i)
    {
        float radial = sqrt((i + 0.5) / samples);
        float angle = i * 2.39996323;
        ray.Direction = float3(radial * cos(angle), sqrt(1.0 - radial * radial), radial * sin(angle));
        RayQuery<RAY_FLAG_FORCE_OPAQUE> query;
        query.TraceRayInline(scene, RAY_FLAG_NONE, 255, ray);
        while (query.Proceed()) {}
        visible += query.CommittedStatus() == COMMITTED_NOTHING ? 1.0 : saturate(query.CommittedRayT() / ray.TMax);
    }
    uint2 pixel = cell.xz + 32 * uint2(cell.y % 8, cell.y / 8);
    uint index = pixel.x + 256 * pixel.y;
    visibility.Store(4 * index, asuint(visible / samples));
}

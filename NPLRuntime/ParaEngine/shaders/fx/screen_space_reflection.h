float2 convertCameraSpaceToScreenSpace(float3 cameraSpace)
{
	float4 clipSpace = mul(float4(cameraSpace, 1.0), matProjection);
	float2 NDCSpace = clipSpace.xy / clipSpace.w;
	float2 ScreenPos = 0.5 * NDCSpace + 0.5;
	return float2(ScreenPos.x, 1 - ScreenPos.y) * viewportScale + viewportOffset;
}


float ReflectionDepth(float2 uv)
{
	// Interpolating foreground and background depths creates false intersections at silhouettes.
	float2 center = (floor(uv * screenParam) + 0.5f) / screenParam;
	return tex2Dlod(depthSampler, float4(center, 0, 0)).r;
}

bool TraceScreenReflection(float3 viewPos, float3 normalVS, int steps, float growth, int refinements, out float2 hitUV)
{
	hitUV = 0;
	float3 direction = reflect(normalize(viewPos), normalize(normalVS));
	float stride = 0.10f;
	float travel = stride;
	float2 viewEnd = viewportOffset + viewportScale;

	[loop] for (int i = 0; i < steps; ++i)
	{
		float3 rayPos = viewPos + direction * travel;
		if (rayPos.z <= 0.01f) break;
		float2 uv = convertCameraSpaceToScreenSpace(rayPos);
		if (any(uv <= viewportOffset) || any(uv >= viewEnd)) break;
		float sceneDepth = ReflectionDepth(uv);
		float gap = rayPos.z - sceneDepth;
		const float thickness = 0.30f;
		if (sceneDepth > 0.01f && gap >= 0 && gap <= stride + thickness)
		{
			// Locate the first crossing within the stride before looking up the reflected color.
			float lo = travel - stride;
			float hi = travel;
			[loop] for (int j = 0; j < refinements; ++j)
			{
				float mid = (lo + hi) * 0.5f;
				float3 probe = viewPos + direction * mid;
				float depth = ReflectionDepth(convertCameraSpaceToScreenSpace(probe));
				if (depth > 0.01f && probe.z >= depth) hi = mid;
				else lo = mid;
			}
			float3 hit = viewPos + direction * hi;
			float2 candidateUV = convertCameraSpaceToScreenSpace(hit);
			float hitDepth = ReflectionDepth(candidateUV);
			// A discontinuity can bracket empty space. Continue tracing after a rejected hit.
			if (hitDepth > 0.01f && hit.z - hitDepth <= max(thickness, hitDepth * 0.01f))
			{
				hitUV = candidateUV;
				return true;
			}
		}
		stride *= growth;
		travel += stride;
	}
	return false;
}

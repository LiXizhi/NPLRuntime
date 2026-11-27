int g_locallightnum : locallightnum;
float3 g_lightcolor[4] : LightColors;
float3 g_lightparam[4] : LightParams;
float4 g_lightpos[4] : LightPositions;

float3 LocalLightDiffuse(float3 worldPos, float3 worldNormal, float4 lightPos, float3 lightColor, float3 lightParam)
{
	float3 toLight = lightPos.xyz - worldPos;
	float lightDist = length(toLight);
	if (lightPos.w > lightDist)
	{
		float attenuation = 1.0 / dot(lightParam, float3(1, lightDist, lightDist * lightDist));
		return max(0, dot(normalize(toLight), worldNormal) * lightColor * attenuation);
	}
	return float3(0, 0, 0);
}

float3 ApplyLocalLights(float3 diffuse, float3 worldPos, float3 worldNormal)
{
	// Fixed indices also work in WebGL 1's restricted GLSL ES 1.00 shaders.
	if (g_locallightnum > 0)
		diffuse += LocalLightDiffuse(worldPos, worldNormal, g_lightpos[0], g_lightcolor[0], g_lightparam[0]);
	if (g_locallightnum > 1)
		diffuse += LocalLightDiffuse(worldPos, worldNormal, g_lightpos[1], g_lightcolor[1], g_lightparam[1]);
	if (g_locallightnum > 2)
		diffuse += LocalLightDiffuse(worldPos, worldNormal, g_lightpos[2], g_lightcolor[2], g_lightparam[2]);
	if (g_locallightnum > 3)
		diffuse += LocalLightDiffuse(worldPos, worldNormal, g_lightpos[3], g_lightcolor[3], g_lightparam[3]);
	return diffuse;
}

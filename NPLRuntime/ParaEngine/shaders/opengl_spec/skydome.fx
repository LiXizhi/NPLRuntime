float4x4 mWorldViewProj:worldviewprojection;
float2 cloudOffset:cloudOffset;
float skyDomeHeight:skyDomeHeight;
float3 lightSkyColor:lightSkyColor;
float3 darkSkyColor:darkSkyColor;
float3 sunColor:sunColor;
float3 cloudColor:cloudColor;
float  cloudBlendWeight:cloudBlendWeight;
float2 sunIntensity:sunIntensity;
float2 sunSize:sunSize;
float2 moonSize:moonSize;
float cloudThickness:cloudThickness;
float sunsetGlowWeight:sunsetGlowWeight;
float4 sunvector:sunvector;

// texture 0
texture tex0 : TEXTURE; 
sampler tex0Sampler : register(s0) = sampler_state 
{
    texture = <tex0>;
};
// texture 1
texture tex1 : TEXTURE; 
sampler tex1Sampler : register(s0) = sampler_state 
{
    texture = <tex1>;
};

struct appdata
{
	float4 vertex : POSITION;
	float2 uv : TEXCOORD0;
};

struct v2f
{
  float4 vertex			: POSITION;
  float4 uv				: TEXCOORD0;
  float3 normal			: NORMAL;
  float4 cloud_uv		: TEXCOORD1;
};


v2f vert(appdata v)
{
	v2f o = (v2f)0;
	// screen space position
	o.vertex = mul(v.vertex, mWorldViewProj).xyww;
	o.uv.xy = v.uv;
	o.uv.z = v.vertex.y / skyDomeHeight;
	float2 flatVertexNormal = v.vertex.xz;
	float2 flatSunVector = float2(0.0,-1.0);
	o.uv.w = clamp(dot(flatVertexNormal,flatSunVector)*2.0, 0.0, 1.0);
	o.normal = v.vertex.xyz;
	o.cloud_uv.xy = v.uv + cloudOffset*4.0;
	o.cloud_uv.zw = v.uv + cloudOffset;

	return o;
}

float CalcFogFactor( float d )
{
	// fog height is 0.5 of the height.
    return clamp( d*2.0, 0.0, 1.0);
}

float4 frag_day(v2f i) : COLOR
{
	//compute sun/moon color
	float3 normal = normalize(i.normal);
	float dotSunAngle = dot(sunvector.xyz, normal);
	float dotSunAnglePos = clamp(dotSunAngle, 0.0, 1.0);
	float sunHighlight = pow( dotSunAnglePos, sunSize.x)*sunIntensity.x;
	float sunHalo = pow( dotSunAnglePos, sunSize.y)*sunIntensity.y;
	float normalizedVertexHeight = i.uv.z;
	float3 skyColor = lerp(lightSkyColor,darkSkyColor,normalizedVertexHeight);

	//compute sunset glow color
	float2 sunsetGlowTexCoord = float2((1.0-dotSunAngle)*0.5, normalizedVertexHeight);
	float2 sunsetGlowColor = tex2D(tex0Sampler,sunsetGlowTexCoord).xyz;

	float cloud0 = tex2D(tex1Sampler,i.cloud_uv.xy).x;
	float cloud1 = tex2D(tex1Sampler,i.cloud_uv.zw).x;
	float cloud = clamp(lerp(cloud0,cloud1,cloudBlendWeight),0.0,1.0)*cloudThickness;

	skyColor = lerp(skyColor,cloudColor,cloud);
	skyColor = lerp(skyColor,skyColor*sunsetGlowColor,sunsetGlowWeight);

	float3 color = skyColor + sunColor*(sunHalo + sunHighlight);
	
	float fog = CalcFogFactor(normalizedVertexHeight);

	return float4(color,fog);
}

float4 frag_night(v2f i) : COLOR
{
	//compute sun/moon color
	float3 normal = normalize(i.normal);
	float3 mirrorSunVector = float3(sunvector.x, -sunvector.y, sunvector.z);
	float dotSunAngle = dot(mirrorSunVector.xyz, normal);
	float dotSunAnglePos = clamp(dotSunAngle, 0.0, 1.0);
	float normalizedVertexHeight = i.uv.z;
	float3 skyColor = lerp(lightSkyColor,darkSkyColor,normalizedVertexHeight);

	//compute sunset glow color
	float2 sunsetGlowTexCoord = float2((1.0-dotSunAngle)*0.5, normalizedVertexHeight);
	float2 sunsetGlowColor = tex2D(tex0Sampler,sunsetGlowTexCoord).xyz;

	float cloud0 = tex2D(tex1Sampler,i.cloud_uv.xy).x;
	float cloud1 = tex2D(tex1Sampler,i.cloud_uv.zw).x;
	float cloud = clamp(lerp(cloud0,cloud1,cloudBlendWeight),0.0,1.0);

	// add star from texture 
	float star = tex2D(tex1Sampler,i.uv.xy*6.0).y*2.0;
	skyColor += star;

	skyColor = lerp(skyColor,cloudColor,cloud);
	skyColor = lerp(skyColor,skyColor*sunsetGlowColor,sunsetGlowWeight);

	// render sun
	float sunHighlight = pow( dotSunAnglePos, sunSize.x)*sunIntensity.x;
	float sunHalo = pow( dotSunAnglePos, sunSize.y)*sunIntensity.y;
	skyColor += sunColor*(sunHalo + sunHighlight);

	// render moon 
	float3 moonVector = float3(-sunvector.x, sunvector.y, -sunvector.z);
	float dotMoonAngle = dot(moonVector.xyz, normal);
	float dotMoonAnglePos = clamp(dotMoonAngle, 0.0, 1.0);
	
	float moonSizeDot = dotMoonAnglePos - (1.0 - 1.25 / moonSize.x);
    float moonHighlight = 0.0;
    if (moonSizeDot > 0.0)
        moonHighlight = clamp(pow(moonSizeDot * 300, 0.5), 0.0, 1.0);

	float moonHalo = pow(dotMoonAnglePos, moonSize.y)*sunIntensity.y;
	skyColor += float3(1.0, 1.0, 0.6) * (moonHalo + moonHighlight);
	
	float fog = CalcFogFactor(normalizedVertexHeight);

	return float4(skyColor,fog);
}



technique default
{
	pass P0
	{
		// shaders
		VertexShader = compile vs_2_0 vert();
		PixelShader  = compile ps_2_0 frag_day();
	}

	pass P1
	{
		// shaders
		VertexShader = compile vs_2_0 vert();
		PixelShader  = compile ps_2_0 frag_night();
	}
}
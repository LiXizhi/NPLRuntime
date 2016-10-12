//-----------------------------------------------------------------------------
// Class:	simulated sky effect (ported from skyDome.fx)
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2014.10.20
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class:	vertex shader file
//-----------------------------------------------------------------------------
const char* skyDomeEffect_vert = STRINGIFY(

attribute vec4 a_position;
attribute vec2 a_texCoord;

uniform mat4 worldviewprojection;
uniform vec2 cloudOffset;
uniform float skyDomeHeight;

varying vec4 v_texCoord; // xy is texCorrd, z is normalizedVertexHeight, w is att. 
varying vec3 v_normal;
varying vec4 v_cloudTexCoord; // xy is cloud tex0, zw is cloud tex1


void main()
{
	// during perspective divide, the z will be 1 (xyww/w = x/w, y/w, 1) in NDC space. (strangely z-fighting with the far clipping plane on some hardware)
	gl_Position = (worldviewprojection * a_position).xyww;
	// when transform z from [-1,1] to [0,1], some precision is lost, hence making z slightly smaller
	//gl_Position = (worldviewprojection * a_position);
	//gl_Position.z = gl_Position.w * 0.9999;

	v_texCoord.xy = a_texCoord.xy;
	v_texCoord.z = a_position.y / skyDomeHeight;
	vec2 flatVertexNormal = a_position.xz;
	vec2 flatSunVector = vec2(0.0,-1.0);
	v_texCoord.w = clamp(dot(flatVertexNormal,flatSunVector)*2.0, 0.0, 1.0); 
	// v_normal = normalize(a_position.xyz);
	v_normal = a_position.xyz;

    v_cloudTexCoord.xy = a_texCoord + cloudOffset*4.0;
    v_cloudTexCoord.zw = a_texCoord + cloudOffset;	
}
);

//-----------------------------------------------------------------------------
// Class:	daytime fragment shader file
//-----------------------------------------------------------------------------
const char* skyDomeDayEffect_frag = STRINGIFY(

uniform vec3 lightSkyColor;
uniform vec3 darkSkyColor;
uniform vec3 sunColor;
uniform vec3 cloudColor;
uniform vec2 cloudOffset;
uniform float cloudBlendWeight;
uniform vec2 sunIntensity;
uniform vec2 sunSize;
uniform vec2 moonSize;
uniform float cloudThickness;
uniform float sunsetGlowWeight;

uniform vec4 sunvector;

varying vec4 v_texCoord; // xy is texCorrd, z is normalizedVertexHeight, w is att. 
varying vec3 v_normal;
varying vec4 v_cloudTexCoord; // xy is cloud tex0, zw is cloud tex1

float CalcFogFactor( float d )
{
	// fog height is 0.5 of the height.
    return clamp( d*2.0, 0.0, 1.0);
}

void main()
{
	//compute sun/moon color
	vec3 normal = normalize(v_normal);
	float dotSunAngle = dot(sunvector.xyz, normal);
	float dotSunAnglePos = clamp(dotSunAngle, 0.0, 1.0);
	float sunHighlight = pow( dotSunAnglePos, sunSize.x)*sunIntensity.x;
	float sunHalo = pow( dotSunAnglePos, sunSize.y)*sunIntensity.y;
	float normalizedVertexHeight = v_texCoord.z;
	vec3 skyColor = mix(lightSkyColor,darkSkyColor,normalizedVertexHeight);

	//compute sunset glow color
	vec2 sunsetGlowTexCoord = vec2((1.0-dotSunAngle)*0.5, normalizedVertexHeight);
	vec3 sunsetGlowColor = texture2D(CC_Texture0,sunsetGlowTexCoord).xyz;
	
	float cloud0 = texture2D(CC_Texture1,v_cloudTexCoord.xy).x;
	float cloud1 = texture2D(CC_Texture1,v_cloudTexCoord.zw).x;
	float cloud = clamp(mix(cloud0,cloud1,cloudBlendWeight),0.0,1.0)*cloudThickness;

	skyColor = mix(skyColor,cloudColor,cloud);
	skyColor = mix(skyColor,skyColor*sunsetGlowColor,sunsetGlowWeight);
	
	vec3 color = skyColor + sunColor*(sunHalo + sunHighlight);
	
	float fog = CalcFogFactor(normalizedVertexHeight);
	gl_FragColor = vec4(color,fog);
	//gl_FragColor = vec4(abs(sunvector.x),abs(sunvector.y),abs(sunvector.z),1.0);
	// gl_FragColor = vec4(1.0,0.0,0.0,1.0);
	// gl_FragColor = vec4(sunColor, fog);
}
);

//-----------------------------------------------------------------------------
// Class:	night time fragment shader file
//-----------------------------------------------------------------------------
const char* skyDomeNightEffect_frag = STRINGIFY(

uniform vec3 lightSkyColor;
uniform vec3 darkSkyColor;
uniform vec3 sunColor;
uniform vec3 cloudColor;
uniform vec2 cloudOffset;
uniform float cloudBlendWeight;
uniform vec2 sunIntensity;
uniform vec2 sunSize;
uniform vec2 moonSize;
uniform float cloudThickness;
uniform float sunsetGlowWeight;

uniform vec4 sunvector;

varying vec4 v_texCoord; // xy is texCorrd, z is normalizedVertexHeight, w is att. 
varying vec3 v_normal;
varying vec4 v_cloudTexCoord; // xy is cloud tex0, zw is cloud tex1

float CalcFogFactor( float d )
{
	// fog height is 0.5 of the height.
    return clamp( d*2.0, 0.0, 1.0);
}

void main()
{
    //compute sun/moon color
	vec3 normal = normalize(v_normal);
	vec3 mirrorSunVector = vec3(sunvector.x, -sunvector.y, sunvector.z);
	float dotSunAngle = dot(mirrorSunVector.xyz, normal);
	float dotSunAnglePos = clamp(dotSunAngle, 0.0, 1.0);
	float normalizedVertexHeight = v_texCoord.z;
	vec3 skyColor = mix(lightSkyColor,darkSkyColor,normalizedVertexHeight);

	//compute sunset glow color
	vec2 sunsetGlowTexCoord = vec2((1.0-dotSunAngle)*0.5, normalizedVertexHeight);
	vec3 sunsetGlowColor = texture2D(CC_Texture0,sunsetGlowTexCoord).xyz;
	
	float cloud0 = texture2D(CC_Texture1,v_cloudTexCoord.xy).x;
	float cloud1 = texture2D(CC_Texture1,v_cloudTexCoord.zw).x;
	float cloud = clamp(mix(cloud0,cloud1,cloudBlendWeight),0.0,1.0);

	// add star from texture 
	float star = texture2D(CC_Texture1,v_texCoord.xy*6.0).y*2.0;
	skyColor += star;

	skyColor = mix(skyColor,cloudColor,cloud);
	skyColor = mix(skyColor,skyColor*sunsetGlowColor,sunsetGlowWeight);

	
	// render sun
	float sunHighlight = pow( dotSunAnglePos, sunSize.x)*sunIntensity.x;
	float sunHalo = pow( dotSunAnglePos, sunSize.y)*sunIntensity.y;
	skyColor += sunColor*(sunHalo + sunHighlight);
	
	
	// render moon 
	vec3 moonVector = vec3(-sunvector.x, sunvector.y, -sunvector.z);
	float dotMoonAngle = dot(moonVector.xyz, normal);
	float dotMoonAnglePos = clamp(dotMoonAngle, 0.0, 1.0);
	float moonHighlight = clamp(pow((dotMoonAnglePos - 0.9975)*300.0, 0.5)*clamp(sign(dotMoonAnglePos - 0.9975), 0.0, 1.0), 0.0, 1.0);
	float moonHalo = pow(dotMoonAnglePos, moonSize.y)*sunIntensity.y;
	skyColor += vec3(1.0, 1.0, 0.6) * (moonHalo + moonHighlight);
	
	float fog = CalcFogFactor(normalizedVertexHeight);
	gl_FragColor = vec4(skyColor,fog);
	// gl_FragColor = vec4(clamp(moonHalo, 0, 1),0.0,0.0,fog);

	// not supported on opengl es 2.0 and not recommended for early-z
	// gl_FragDepth = 1.0; 
}
);

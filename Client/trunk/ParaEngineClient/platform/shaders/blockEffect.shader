//-----------------------------------------------------------------------------
// Class:	basic block rendering
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2014.9.12
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class:	vertex shader file
//-----------------------------------------------------------------------------
const char* shaderBlockEffect_vert = STRINGIFY(

attribute vec4 a_position;
attribute vec2 a_texCoord;
attribute vec4 a_color; // x: blockid, y torch light, z sun light, w ambient occlusion
attribute vec4 a_color2; // second color
attribute vec3 a_normal;

uniform mat4 worldviewprojection;
uniform mat4 world;
uniform vec4 ConstVector0;  // light_params
uniform vec4 sunvector;
uniform vec4 fogparameters; // (fogstart, fogrange, fogDensity, reserved)

\n#ifdef GL_ES\n
uniform lowp vec4 fogColor;
varying lowp vec4 v_color;
varying mediump vec2 v_texCoord;
\n#else\n
uniform vec4 fogColor;
varying vec4 v_color;
varying vec2 v_texCoord;
\n#endif\n

// Calculates fog factor based upon distance
float CalcFogFactor( float d )
{
    float fogCoeff = 0.0;
	fogCoeff = (d - fogparameters.x) / fogparameters.y;
    return clamp( fogCoeff, 0.0, 1.0);
}

void main()
{
	gl_Position = worldviewprojection * a_position;
	v_texCoord = a_texCoord;

	// emissive block light received by this block. 
	float torch_light_strength = a_color.y;
	vec3 torch_light = ConstVector0.xyz * torch_light_strength;

	// sun light + sky(fog) light
	float sun_light_strength = clamp(a_color.z*ConstVector0.w, 0.0, 1.0); // normalize to 0,1 range

	float lightFactor = 0.7 + 0.3*clamp(dot(sunvector.xyz,a_normal), 0.0,1.0);
	vec3 sun_light = (vec3(1.0, 1.0, 1.0) + fogColor.xyz * 0.05) * (sun_light_strength * lightFactor);
	torch_light_strength *= lightFactor;

	// compose and interpolate so that the strength of light is almost linear 
	vec3 final_light = mix(torch_light.xyz+sun_light.xyz, sun_light.xyz, sun_light_strength / (torch_light_strength + sun_light_strength+0.001));
	
	// apply shadow either before or after color clamp
	v_color.xyz = final_light*a_color.w;
	v_color.rgb *= a_color2.bgr;

	//calculate the fog factor
	v_color.w = CalcFogFactor(length(gl_Position.xyz));
}
);

//-----------------------------------------------------------------------------
// Class:	fragment shader file: opache block
//-----------------------------------------------------------------------------
const char* shaderOpacheBlockEffect_frag = STRINGIFY(


\n#ifdef GL_ES\n
uniform lowp vec4 fogColor;
varying lowp vec4 v_color;
varying mediump vec2 v_texCoord;
\n#else\n
uniform vec4 fogColor;
varying vec4 v_color;
varying vec2 v_texCoord;
\n#endif\n

void main()
{
    vec4 albedoColor = texture2D(CC_Texture0, v_texCoord).xyzw;
	//gl_FragColor = vec4(albedoColor.xyz * v_color.xyz, albedoColor.a);
	gl_FragColor = vec4(mix(vec3(albedoColor.xyz * v_color.xyz), fogColor.xyz, v_color.w), albedoColor.a);
}
);

//-----------------------------------------------------------------------------
// Class:	vertex shader file: transparent
//-----------------------------------------------------------------------------
const char* shaderTransparentBlockEffect_vert = STRINGIFY(

attribute vec4 a_position;
attribute vec2 a_texCoord;
attribute vec4 a_color; // x: blockid, y torch light, z sun light, w ambient occlusion
attribute vec4 a_color2; // second color
attribute vec3 a_normal;

uniform mat4 worldviewprojection;
uniform mat4 world;
uniform vec4 ConstVector0;  // light_params
uniform vec4 sunvector;
uniform vec4 fogparameters; // (fogstart, fogrange, fogDensity, reserved)

\n#ifdef GL_ES\n
uniform lowp vec4 fogColor;
varying lowp vec4 v_color;
varying mediump vec2 v_texCoord;
\n#else\n
uniform vec4 fogColor;
varying vec4 v_color;
varying vec2 v_texCoord;
\n#endif\n

// Calculates fog factor based upon distance
float CalcFogFactor( float d )
{
    float fogCoeff = 0.0;
	fogCoeff = (d - fogparameters.x) / fogparameters.y;
    return clamp( fogCoeff, 0.0, 1.0);
}

void main()
{
	gl_Position = worldviewprojection * a_position;
	v_texCoord = a_texCoord;

	// emissive block light received by this block. 
	float torch_light_strength = a_color.y;
	vec3 torch_light = ConstVector0.xyz * torch_light_strength;

	// sun light + sky(fog) light
	float sun_light_strength = clamp(a_color.z*ConstVector0.w, 0.0, 1.0); // normalize to 0,1 range
	vec3 sun_light = (vec3(1.0, 1.0, 1.0) + fogColor.xyz * 0.05) * sun_light_strength;
	
	// compose and interpolate so that the strength of light is almost linear 
	vec3 final_light = mix(torch_light.xyz+sun_light.xyz, sun_light.xyz, sun_light_strength / (torch_light_strength + sun_light_strength+0.001));
	
	// apply shadow either before or after color clamp
	v_color.xyz = final_light*a_color.w;
	v_color.rgb *= a_color2.bgr;

	//calculate the fog factor
	v_color.w = CalcFogFactor(length(gl_Position.xyz));
}
);

//-----------------------------------------------------------------------------
// Class:	fragment shader file: transparent block
//-----------------------------------------------------------------------------
const char* shaderTransparentBlockEffect_frag = STRINGIFY(

\n#ifdef GL_ES\n
uniform lowp vec4 fogColor;
varying lowp vec4 v_color;
varying mediump vec2 v_texCoord;
\n#else\n
uniform vec4 fogColor;
varying vec4 v_color;
varying vec2 v_texCoord;
\n#endif\n

void main()
{
    vec4 albedoColor = texture2D(CC_Texture0, v_texCoord);
	// this is for alpha testing. 
	if(albedoColor.a < 0.95)
		discard;
	//gl_FragColor = vec4(albedoColor.xyz * v_color.xyz, albedoColor.a);
	gl_FragColor = vec4(mix(vec3(albedoColor.xyz * v_color.xyz), fogColor.xyz, v_color.w), albedoColor.a);
}
);

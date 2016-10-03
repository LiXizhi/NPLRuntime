//-----------------------------------------------------------------------------
// Class:	bmax model
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2015.5.23
// Desc: block max model
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class:	vertex shader file
//-----------------------------------------------------------------------------
const char* shaderBlockMaxEffect_vert = STRINGIFY(

attribute vec4 a_position;
attribute vec3 a_normal;
attribute vec4 a_color; 

uniform mat4 worldviewprojection;
uniform mat4 world;

uniform vec4 sunvector;
uniform vec4 materialdiffuse;
uniform vec4 ambientlight;

uniform vec4 fogparameters; // (fogstart, fogrange, fogDensity, reserved)
uniform vec4 fogColor;

varying vec4 v_color;

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

	// world space normal
	vec3 worldNormal = normalize( vec3(mat3(world) * a_normal) ); 

	vec3 sun_lightmap = ambientlight.rgb + materialdiffuse.rgb * clamp(dot(sunvector.xyz, worldNormal), 0.0, 1.0);

	// need to swizzle since directX and opengl packed color differently. 
	v_color.xyz = a_color.bgr*sun_lightmap;
	// calculate the fog factor
	v_color.w = CalcFogFactor(length(gl_Position.xyz));
}
);

//-----------------------------------------------------------------------------
// Class:	fragment shader file
//-----------------------------------------------------------------------------
const char* shaderBlockMaxEffect_frag = STRINGIFY(

varying vec4 v_color;

uniform vec4 fogColor;

void main()
{
	vec3 color = v_color.rgb;
	color = mix(color, fogColor.xyz, v_color.w);
	gl_FragColor = vec4(color, 1.0);
}
);

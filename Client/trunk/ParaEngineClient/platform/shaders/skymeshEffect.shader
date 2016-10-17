//-----------------------------------------------------------------------------
// Class:	sky mesh effect (ported from sky.fx)
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2014.10.19
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class:	vertex shader file
//-----------------------------------------------------------------------------
const char* skyMeshEffect_vert = STRINGIFY(

attribute vec4 a_position;
attribute vec2 a_texCoord;
attribute vec3 a_normal;

uniform mat4 worldviewprojection;
uniform mat4 worldview;
uniform mat4 world;

uniform vec4 sunvector;
uniform vec4 materialdiffuse;
uniform vec4 ambientlight;

uniform vec4 ConstVector0; // g_skycolorfactor

varying vec3 v_colorDiffuse;
varying vec3 v_tex;
varying vec3 v_pos;

void main()
{
	// during perspective divide, the z will be 1 (xyww/w = x/w, y/w, 1) in NDC space. (strangely z-fighting with the far clipping plane on some hardware)
	gl_Position = (worldviewprojection * a_position).xyww;
	// when transform z from [-1,1] to [0,1], some precision is lost, hence making z slightly smaller
	//gl_Position = (worldviewprojection * a_position);
	//gl_Position.z = gl_Position.w * 0.9999;

	v_colorDiffuse = materialdiffuse.xyz * dot( sunvector.xyz, vec3(0.0,1.0,0.0) ) + ambientlight.xyz;
	v_colorDiffuse *= ConstVector0.xyz;
	v_colorDiffuse = min(vec3(1.0, 1.0, 1.0), v_colorDiffuse);

	v_tex.xy = a_texCoord;
	v_tex.z = a_position.y;
	v_pos = a_position.xyz/a_position.w;
}
);

//-----------------------------------------------------------------------------
// Class:	fragment shader file
//-----------------------------------------------------------------------------
const char* skyMeshEffect_frag = STRINGIFY(

uniform vec4 fogparameters; // (fogstart, fogrange, fogDensity, reserved)
uniform vec4 fogColor;

varying vec3 v_colorDiffuse;
varying vec3 v_tex;
varying vec3 v_pos;

// Calculates fog factor based upon distance
float CalcFogFactor( float d )
{
    float fogCoeff = 0.0;
	fogCoeff = (d - fogparameters.x) / fogparameters.y;
    return clamp( fogCoeff, 0.0, 1.0);
}

void main()
{
    vec4 normalColor = texture2D(CC_Texture0, v_tex.xy);
	normalColor.xyz *= v_colorDiffuse.xyz;

	float fog = CalcFogFactor(v_pos.y/length(v_pos.xz));
	gl_FragColor.xyz = mix(fogColor.xyz, normalColor.xyz, fog);
	gl_FragColor.w = 1.0;
}
);

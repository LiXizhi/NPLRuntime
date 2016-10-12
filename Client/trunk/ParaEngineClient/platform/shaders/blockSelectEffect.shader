//-----------------------------------------------------------------------------
// Class:	selection block rendering
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2014.9.25
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class:	vertex shader file
//-----------------------------------------------------------------------------
const char* shaderBlockSelectEffect_vert = STRINGIFY(

attribute vec4 a_position;
attribute vec2 a_texCoord;

uniform mat4 worldviewprojection;
uniform vec4 ConstVector0;  // light_params

\n#ifdef GL_ES\n
varying mediump vec2 v_texCoord;
\n#else\n
varying vec2 v_texCoord;
\n#endif\n

void main()
{
	gl_Position = worldviewprojection * a_position;
	v_texCoord = a_texCoord;

}
);

//-----------------------------------------------------------------------------
// Class:	fragment shader file
//-----------------------------------------------------------------------------
const char* shaderBlockSelectEffect_frag = STRINGIFY(

uniform vec4 ConstVector0;  // light_params

\n#ifdef GL_ES\n
varying mediump vec2 v_texCoord;
\n#else\n
varying vec2 v_texCoord;
\n#endif\n

void main()
{
    vec4 albedoColor = texture2D(CC_Texture0, v_texCoord);
	albedoColor.xyz *= ConstVector0.x;
	albedoColor.w = 1.0;
	gl_FragColor = albedoColor;
}
);

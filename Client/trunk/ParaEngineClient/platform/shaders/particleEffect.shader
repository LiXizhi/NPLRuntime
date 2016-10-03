//-----------------------------------------------------------------------------
// Class:	particle effect (ported from simple_particle.fx)
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2014.10.8
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class:	vertex shader file
//-----------------------------------------------------------------------------
const char* particleEffect_vert = STRINGIFY(

attribute vec4 a_position;
attribute vec4 a_color;
attribute vec2 a_texCoord;

uniform mat4 worldviewprojection;

\n#ifdef GL_ES\n
varying lowp vec4 v_color;
varying mediump vec2 v_texCoord;
\n#else\n
varying vec4 v_color;
varying vec2 v_texCoord;
\n#endif\n

void main()
{
	gl_Position = worldviewprojection * a_position;
	v_texCoord = a_texCoord;
	// need to swizzle since directX and opengl packed color differently. 
	v_color = a_color.bgra;
}
);

//-----------------------------------------------------------------------------
// Class:	fragment shader file
//-----------------------------------------------------------------------------
const char* particleEffect_frag = STRINGIFY(

uniform bool alphatesting;

\n#ifdef GL_ES\n
varying lowp vec4 v_color;
varying mediump vec2 v_texCoord;
\n#else\n
varying vec4 v_color;
varying vec2 v_texCoord;
\n#endif\n

void main()
{
    vec4 albedoColor = texture2D(CC_Texture0, v_texCoord);
	albedoColor *= v_color;
	// this is for alpha testing. 
	if(alphatesting && albedoColor.a < 0.05)
		discard;
	
	gl_FragColor = albedoColor;
}
);

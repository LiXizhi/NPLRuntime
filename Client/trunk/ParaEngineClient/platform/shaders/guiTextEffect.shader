//-----------------------------------------------------------------------------
// Class:	GUI text rendering (OBSOLETED use guiEffect.shader instead)
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2014.10.9
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class:	vertex shader file
//-----------------------------------------------------------------------------
const char* guiTextEffect_vert = STRINGIFY(

attribute vec4 a_position;
attribute vec2 a_texCoord;
attribute vec4 a_color; 

uniform mat4 viewprojection;

\n#ifdef GL_ES\n
varying lowp vec4 v_color;
varying mediump vec2 v_texCoord;
\n#else\n
varying vec4 v_color;
varying vec2 v_texCoord;
\n#endif\n

void main()
{
	gl_Position = viewprojection * a_position;
	v_texCoord = a_texCoord;
	v_color = a_color;
}
);

//-----------------------------------------------------------------------------
// Class:	fragment shader file
//-----------------------------------------------------------------------------
const char* guiTextEffect_frag = STRINGIFY(

\n#ifdef GL_ES\n
varying lowp vec4 v_color;
varying mediump vec2 v_texCoord;
\n#else\n
varying vec4 v_color;
varying vec2 v_texCoord;
\n#endif\n

void main()
{
    gl_FragColor = vec4(v_color.zyx, 
		v_color.w * texture2D(CC_Texture0, v_texCoord).a);
}
);

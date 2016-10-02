//-----------------------------------------------------------------------------
// Class:	single color
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2014.9.23
// Desc: mostly used to render thick lines
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class:	vertex shader file
//-----------------------------------------------------------------------------
const char* singleColorEffect_vert = STRINGIFY(

attribute vec4 a_position;
attribute vec4 a_color; 

uniform mat4 worldviewprojection;

\n#ifdef GL_ES\n
varying lowp vec4 v_color;
\n#else\n
varying vec4 v_color;
\n#endif\n

void main()
{
	gl_Position = worldviewprojection * a_position;
	// need to swizzle since directX and opengl packed color differently. 
	v_color = a_color.bgra;
}
);

//-----------------------------------------------------------------------------
// Class:	fragment shader file
//-----------------------------------------------------------------------------
const char* singleColorEffect_frag = STRINGIFY(

\n#ifdef GL_ES\n
varying lowp vec4 v_color;
\n#else\n
varying vec4 v_color;
\n#endif\n

void main()
{
   gl_FragColor = v_color;
}
);

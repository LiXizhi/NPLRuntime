Unifroms
{
	mat4 worldviewprojection : worldviewprojection;
}

#CODEBLOCK_BEGIN

//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class:	vertex shader file
//-----------------------------------------------------------------------------

attribute vec4 a_position;
attribute vec4 a_color; 

uniform mat4 worldviewprojection;


varying  vec4 v_color;


void main()
{
	gl_Position = worldviewprojection * a_position;
	// need to swizzle since directX and opengl packed color differently. 
	v_color = a_color.bgra;
}
#CODEBLOCK_END vertexShader

#CODEBLOCK_BEGIN
//-----------------------------------------------------------------------------
// Class:	fragment shader file
//-----------------------------------------------------------------------------

varying  vec4 v_color;


void main()
{
   gl_FragColor = v_color;
}
#CODEBLOCK_END pixelShader

technique default
{
	pass P0
	{
		VertexShader = compile vs vertexShader();
		PixelShader = compile ps pixelShader();
	}
}

//-----------------------------------------------------------------------------
// Class:	GUI text rendering (OBSOLETED use guiEffect.shader instead)
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2014.10.9
//-----------------------------------------------------------------------------


Unifroms
{
	mat4 viewprojection:viewprojection;

}

#CODEBLOCK_BEGIN
//-----------------------------------------------------------------------------
// Class:	vertex shader file
//-----------------------------------------------------------------------------

attribute vec4 a_position;
attribute vec2 a_texCoord;
attribute vec4 a_color; 

uniform mat4 viewprojection;

varying lowp vec4 v_color;
varying mediump vec2 v_texCoord;


void main()
{
	gl_Position = viewprojection * a_position;
	v_texCoord = a_texCoord;
	v_color = a_color;
}
#CODEBLOCK_END vertexShader

//-----------------------------------------------------------------------------
// Class:	fragment shader file
//-----------------------------------------------------------------------------

#CODEBLOCK_BEGIN

varying lowp vec4 v_color;
varying mediump vec2 v_texCoord;


void main()
{
    gl_FragColor = vec4(v_color.zyx,v_color.w * texture2D(CC_Texture0, v_texCoord).a);
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

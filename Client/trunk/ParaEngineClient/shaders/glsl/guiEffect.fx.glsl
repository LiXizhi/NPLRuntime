Unifroms
{
	mat4 viewprojection : viewprojection;
	bool k_bBoolean0 : boolean0;
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
	v_color = a_color.zyxw;
}
#CODEBLOCK_END vertexShader


#CODEBLOCK_BEGIN
//-----------------------------------------------------------------------------
// Class:	fragment shader file
//-----------------------------------------------------------------------------

uniform bool k_bBoolean0; // weather it is text rendering or quad rendering


varying lowp vec4 v_color;
varying mediump vec2 v_texCoord;


void main()
{
	if(k_bBoolean0){
		gl_FragColor = v_color*texture2D(CC_Texture0, v_texCoord);
	}
	else{
		gl_FragColor = vec4(v_color.xyz, 
			v_color.w * texture2D(CC_Texture0, v_texCoord).a);
	}
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
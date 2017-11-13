Unifroms
{
	bool g_bAlphaTesting:alphatesting;
	mat4 mWorldViewProj:worldviewprojection;
	sampler2D tex0Sampler;

}
#CODEBLOCK_BEGIN

struct Interpolants {
    vec4 positionSS;
    vec2 tex;
    vec4 colorDiffuse;
};
uniform mat4 mWorldViewProj;
uniform bool g_bAlphaTesting;
uniform sampler2D tex0Sampler;
Interpolants vertexShader( in vec4 Pos, in vec4 Color, in vec2 Tex ) {
    Interpolants o = Interpolants(vec4(0.0, 0.0, 0.0, 0.0), vec2(0.0, 0.0), vec4(0.0, 0.0, 0.0, 0.0));
    o.positionSS = (mWorldViewProj * Pos);
    o.colorDiffuse = Color;
    o.tex.xy = Tex;
    return o;
}
attribute vec4 a_position;
attribute vec4 a_color;
attribute vec2 a_texCoord;
varying vec2 xlv_TEXCOORD0;
varying vec4 xlv_COLOR0;
void main() {
    Interpolants xl_retval;
    xl_retval = vertexShader( vec4(a_position), vec4(a_color.bgra), vec2(a_texCoord));
    gl_Position = vec4(xl_retval.positionSS);
    xlv_TEXCOORD0 = vec2(xl_retval.tex);
    xlv_COLOR0 = vec4(xl_retval.colorDiffuse);
}

#CODEBLOCK_END vertexShader


#CODEBLOCK_BEGIN
void xll_clip_f(float x) {
  if ( x<0.0 ) discard;
}
void xll_clip_vf2(vec2 x) {
  if (any(lessThan(x,vec2(0.0)))) discard;
}
void xll_clip_vf3(vec3 x) {
  if (any(lessThan(x,vec3(0.0)))) discard;
}
void xll_clip_vf4(vec4 x) {
  if (any(lessThan(x,vec4(0.0)))) discard;
}
struct Interpolants {
    vec4 positionSS;
    vec2 tex;
    vec4 colorDiffuse;
};
uniform mat4 mWorldViewProj;
uniform bool g_bAlphaTesting;
uniform sampler2D tex0Sampler;
vec4 pixelShader( in Interpolants i ) {
    vec4 o;
    vec4 normalColor = texture2D( tex0Sampler, i.tex.xy);
    normalColor *= i.colorDiffuse;
    if (g_bAlphaTesting){
        xll_clip_f((normalColor.w - 0.5));
    }
    o = normalColor;
    return o;
}
varying vec2 xlv_TEXCOORD0;
varying vec4 xlv_COLOR0;
void main() {
    vec4 xl_retval;
    Interpolants xlt_i;
    xlt_i.positionSS = vec4(0.0);
    xlt_i.tex = vec2(xlv_TEXCOORD0);
    xlt_i.colorDiffuse = vec4(xlv_COLOR0);
    xl_retval = pixelShader( xlt_i);
    gl_FragData[0] = vec4(xl_retval);
}

#CODEBLOCK_END pixelShader



technique SimpleMesh_vs20_ps20
{
	pass P0
	{
		VertexShader = compile vs vertexShader();
		PixelShader = compile ps pixelShader();
		FogEnable = FALSE;
		ZWriteEnable = FALSE;
	}
}

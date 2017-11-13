Unifroms
{
	vec3 colorAmbient:ambientlight;
	vec3 colorDiffuse:materialdiffuse;
	bool g_bAlphaTesting:alphatesting;
	bool g_bEnableFog:fogenable;
	bool g_bEnableSunLight:sunlightenable;
	vec4 g_fogColor:fogColor;
	vec4 g_fogParam:fogparameters;
	vec4 g_skycolorfactor:ConstVector0;
	mat4 mWorld:world;
	mat4 mWorldView:worldview;
	mat4 mWorldViewProj:worldviewprojection;
	vec3 sun_vec:sunvector;
	sampler2D tex0Sampler;

}
#CODEBLOCK_BEGIN

struct Interpolants {
    vec4 positionSS;
    vec3 tex;
    vec3 pos;
    vec3 colorDiffuse;
};
uniform mat4 mWorldViewProj;
uniform mat4 mWorldView;
uniform mat4 mWorld;
uniform vec3 sun_vec;
uniform vec3 colorDiffuse;
uniform vec3 colorAmbient;
uniform vec4 g_fogParam;
uniform vec4 g_fogColor;
uniform vec4 g_skycolorfactor;
uniform bool g_bEnableFog;
uniform bool g_bEnableSunLight;
uniform bool g_bAlphaTesting;
uniform sampler2D tex0Sampler;
Interpolants vertexShader( in vec4 Pos, in vec3 Norm, in vec2 Tex ) {
    Interpolants o = Interpolants(vec4(0.0, 0.0, 0.0, 0.0), vec3(0.0, 0.0, 0.0), vec3(0.0, 0.0, 0.0), vec3(0.0, 0.0, 0.0));
    o.positionSS = (mWorldViewProj * Pos).xyww;
    if (g_bEnableSunLight){
        o.colorDiffuse = ((colorDiffuse * dot( sun_vec, vec3( 0.0, 1.0, 0.0))) + colorAmbient);
    }
    else{
        o.colorDiffuse = vec3( 1.0, 1.0, 1.0);
    }
    o.colorDiffuse *= vec3( g_skycolorfactor);
    o.colorDiffuse = min( vec3( 1.0), o.colorDiffuse);
    o.tex.xy = Tex;
    o.tex.z = Pos.y;
    o.pos = (Pos.xyz / Pos.w);
    return o;
}
attribute vec4 a_position;
attribute vec3 a_normal;
attribute vec2 a_texCoord;
varying vec3 xlv_TEXCOORD0;
varying vec3 xlv_TEXCOORD1;
varying vec3 xlv_TEXCOORD2;
void main() {
    Interpolants xl_retval;
    xl_retval = vertexShader( vec4(a_position), vec3(a_normal), vec2(a_texCoord));
    gl_Position = vec4(xl_retval.positionSS);
    xlv_TEXCOORD0 = vec3(xl_retval.tex);
    xlv_TEXCOORD1 = vec3(xl_retval.pos);
    xlv_TEXCOORD2 = vec3(xl_retval.colorDiffuse);
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
float xll_saturate_f( float x) {
  return clamp( x, 0.0, 1.0);
}
vec2 xll_saturate_vf2( vec2 x) {
  return clamp( x, 0.0, 1.0);
}
vec3 xll_saturate_vf3( vec3 x) {
  return clamp( x, 0.0, 1.0);
}
vec4 xll_saturate_vf4( vec4 x) {
  return clamp( x, 0.0, 1.0);
}
mat2 xll_saturate_mf2x2(mat2 m) {
  return mat2( clamp(m[0], 0.0, 1.0), clamp(m[1], 0.0, 1.0));
}
mat3 xll_saturate_mf3x3(mat3 m) {
  return mat3( clamp(m[0], 0.0, 1.0), clamp(m[1], 0.0, 1.0), clamp(m[2], 0.0, 1.0));
}
mat4 xll_saturate_mf4x4(mat4 m) {
  return mat4( clamp(m[0], 0.0, 1.0), clamp(m[1], 0.0, 1.0), clamp(m[2], 0.0, 1.0), clamp(m[3], 0.0, 1.0));
}
struct Interpolants {
    vec4 positionSS;
    vec3 tex;
    vec3 pos;
    vec3 colorDiffuse;
};
uniform mat4 mWorldViewProj;
uniform mat4 mWorldView;
uniform mat4 mWorld;
uniform vec3 sun_vec;
uniform vec3 colorDiffuse;
uniform vec3 colorAmbient;
uniform vec4 g_fogParam;
uniform vec4 g_fogColor;
uniform vec4 g_skycolorfactor;
uniform bool g_bEnableFog;
uniform bool g_bEnableSunLight;
uniform bool g_bAlphaTesting;
uniform sampler2D tex0Sampler;
float CalcFogFactor( in float d ) {
    float fogCoeff = 0.0;
    fogCoeff = ((d - g_fogParam.x) / g_fogParam.y);
    return xll_saturate_f(fogCoeff);
}
vec4 pixelShader( in Interpolants i ) {
    vec4 o;
    vec4 normalColor = texture2D( tex0Sampler, i.tex.xy);
    normalColor.xyz = (normalColor.xyz * i.colorDiffuse);
    if (g_bAlphaTesting){
        xll_clip_f((normalColor.w - 0.5));
    }
    if (g_bEnableFog){
        float fog = CalcFogFactor( (i.pos.y / length(i.pos.xz)));
        o.xyz = mix( g_fogColor.xyz, normalColor.xyz, vec3( fog));
        o.w = normalColor.w;
    }
    else{
        o = normalColor;
    }
    return o;
}
varying vec3 xlv_TEXCOORD0;
varying vec3 xlv_TEXCOORD1;
varying vec3 xlv_TEXCOORD2;
void main() {
    vec4 xl_retval;
    Interpolants xlt_i;
    xlt_i.positionSS = vec4(0.0);
    xlt_i.tex = vec3(xlv_TEXCOORD0);
    xlt_i.pos = vec3(xlv_TEXCOORD1);
    xlt_i.colorDiffuse = vec3(xlv_TEXCOORD2);
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
	}
}

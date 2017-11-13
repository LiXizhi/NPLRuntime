Unifroms
{
	vec3 colorAmbient:ambientlight;
	vec3 colorDiffuse:materialdiffuse;
	bool g_bEnableFog:fogenable;
	vec4 g_fogColor:fogColor;
	vec4 g_fogParam:fogparameters;
	float g_opacity:opacity;
	vec4 light_params:ConstVector0;
	mat4 mWorld:world;
	mat4 mWorldViewProj:worldviewprojection;
	vec3 sun_vec:sunvector;

}
#CODEBLOCK_BEGIN
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
mat3 xll_constructMat3_mf4x4( mat4 m) {
  return mat3( vec3( m[0]), vec3( m[1]), vec3( m[2]));
}
struct VSOut {
    vec4 pos;
    vec4 color;
};
uniform mat4 mWorldViewProj;
uniform mat4 mWorld;
uniform vec4 light_params;
uniform vec3 sun_vec;
uniform vec3 colorDiffuse;
uniform vec3 colorAmbient;
uniform bool g_bEnableFog;
uniform vec4 g_fogParam;
uniform vec4 g_fogColor;
uniform float g_opacity;
float CalcFogFactor( in float d ) {
    float fogCoeff = 0.0;
    fogCoeff = ((d - g_fogParam.x) / g_fogParam.y);
    return clamp( fogCoeff, 0.0, 1.0);
}
VSOut MainVS( in vec4 pos, in vec3 Norm, in vec4 color ) {
    VSOut xlat_varoutput;
    xlat_varoutput.pos = (mWorldViewProj * pos);
    vec3 worldNormal = normalize((xll_constructMat3_mf4x4( mWorld) * Norm));
    vec3 sun_lightmap = (colorAmbient + (colorDiffuse * xll_saturate_f(dot( sun_vec, worldNormal))));
    xlat_varoutput.color.xyz = (color.xyz * sun_lightmap);
    xlat_varoutput.color.w = CalcFogFactor( length(xlat_varoutput.pos.xyz));
    return xlat_varoutput;
}
attribute vec4 a_position;
attribute vec3 a_normal;
attribute vec4 a_color;
varying vec4 xlv_COLOR0;
void main() {
    VSOut xl_retval;
    xl_retval = MainVS( vec4(a_position), vec3(a_normal), vec4(a_color.bgra));
    gl_Position = vec4(xl_retval.pos);
    xlv_COLOR0 = vec4(xl_retval.color);
}

#CODEBLOCK_END MainVS


#CODEBLOCK_BEGIN

struct VSOut {
    vec4 pos;
    vec4 color;
};
uniform mat4 mWorldViewProj;
uniform mat4 mWorld;
uniform vec4 light_params;
uniform vec3 sun_vec;
uniform vec3 colorDiffuse;
uniform vec3 colorAmbient;
uniform bool g_bEnableFog;
uniform vec4 g_fogParam;
uniform vec4 g_fogColor;
uniform float g_opacity;
vec4 MainPS( in VSOut xlat_varinput ) {
    vec3 color = xlat_varinput.color.xyz;
    color = mix( color, g_fogColor.xyz, vec3( xlat_varinput.color.w));
    return vec4( color, g_opacity);
}
varying vec4 xlv_COLOR0;
void main() {
    vec4 xl_retval;
    VSOut xlt_xlat_varinput;
    xlt_xlat_varinput.pos = vec4(0.0);
    xlt_xlat_varinput.color = vec4(xlv_COLOR0);
    xl_retval = MainPS( xlt_xlat_varinput);
    gl_FragData[0] = vec4(xl_retval);
}

#CODEBLOCK_END MainPS


#CODEBLOCK_BEGIN

struct VSOut {
    vec4 pos;
    vec4 color;
};
uniform mat4 mWorldViewProj;
uniform mat4 mWorld;
uniform vec4 light_params;
uniform vec3 sun_vec;
uniform vec3 colorDiffuse;
uniform vec3 colorAmbient;
uniform bool g_bEnableFog;
uniform vec4 g_fogParam;
uniform vec4 g_fogColor;
uniform float g_opacity;
void VertShadow( in vec4 Pos, in vec3 Norm, in vec4 color, out vec4 oPos, out vec2 Depth ) {
    oPos = (mWorldViewProj * Pos);
    Depth.xy = oPos.zw;
}
attribute vec4 a_position;
attribute vec3 a_normal;
attribute vec4 a_color;
varying vec2 xlv_TEXCOORD1;
void main() {
    vec4 xlt_oPos;
    vec2 xlt_Depth;
    VertShadow( vec4(a_position), vec3(a_normal), vec4(a_color.bgra), xlt_oPos, xlt_Depth);
    gl_Position = vec4(xlt_oPos);
    xlv_TEXCOORD1 = vec2(xlt_Depth);
}

#CODEBLOCK_END VertShadow


#CODEBLOCK_BEGIN

struct VSOut {
    vec4 pos;
    vec4 color;
};
uniform mat4 mWorldViewProj;
uniform mat4 mWorld;
uniform vec4 light_params;
uniform vec3 sun_vec;
uniform vec3 colorDiffuse;
uniform vec3 colorAmbient;
uniform bool g_bEnableFog;
uniform vec4 g_fogParam;
uniform vec4 g_fogColor;
uniform float g_opacity;
vec4 PixShadow( in vec2 Depth ) {
    float d = (Depth.x / Depth.y);
    return vec4( 0.0, vec2( d), 1.0);
}
varying vec2 xlv_TEXCOORD1;
void main() {
    vec4 xl_retval;
    xl_retval = PixShadow( vec2(xlv_TEXCOORD1));
    gl_FragData[0] = vec4(xl_retval);
}

#CODEBLOCK_END PixShadow



technique SimpleMesh_vs20_ps20
{
	pass P0
	{
		VertexShader = compile vs MainVS();
		PixelShader = compile ps MainPS();
		FogEnable = FALSE;
	}
}
technique GenShadowMap
{
	pass p0
	{
		VertexShader = compile vs VertShadow();
		PixelShader = compile ps PixShadow();
		FogEnable = FALSE;
	}
}

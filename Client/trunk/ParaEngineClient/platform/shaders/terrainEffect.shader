//-----------------------------------------------------------------------------
// Class:	terrain effect (ported from terrain_normal.fx)
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2015.1.27
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class:	vertex shader file
//-----------------------------------------------------------------------------
const char* terrainEffect_vert = STRINGIFY(

attribute vec4 a_position;
attribute vec3 a_normal;
attribute vec2 a_texCoord;
attribute vec2 a_texCoord1;

uniform mat4 worldviewprojection;
uniform mat4 world;

// uniform vec3 posScaleOffset; // g_vertexOffset;
uniform vec4 sunvector;
uniform vec4 ambientlight;

uniform vec4 texCoordOffset; // render_offset_x, render_offset_y, terrain_size
uniform vec3 worldcamerapos; // g_EyePositionW
uniform bool sunlightenable;
uniform bool boolean10; // g_bIsBaseEnabled

uniform bool fogenable; // g_bEnableFog
uniform vec4 fogparameters; // g_fogParam (fogstart, fogrange, fogDensity, reserved)


\n#ifdef GL_ES\n
uniform lowp vec4 materialdiffuse;
uniform lowp vec4 fogColor; // g_fogColor
varying lowp vec4 v_colorDiffuse;
varying mediump vec3 v_tex0;
varying mediump vec4 v_tex1;
\n#else\n
uniform vec4 materialdiffuse;
uniform vec4 fogColor; // g_fogColor
varying vec4 v_colorDiffuse;
varying vec3 v_tex0;
varying vec4 v_tex1;
\n#endif\n


// Calculates fog factor based upon distance
float CalcFogFactor( float d )
{
    float fogCoeff = 0.0;
	fogCoeff = (d - fogparameters.x) / fogparameters.y;
    return clamp( fogCoeff, 0.0, 1.0);
}

void main()
{
	gl_Position = worldviewprojection * a_position;
	
	vec3 worldNormal = normalize(a_normal);

	v_colorDiffuse.xyz = materialdiffuse.xyz * dot( sunvector.xyz, worldNormal ) + ambientlight.xyz;

	vec3 worldPos = (world * a_position).xyz;
	vec3 eyeVec = normalize(worldcamerapos - worldPos);
	vec3 reflectVec = reflect(-sunvector.xyz,worldNormal);
	float specular = max(dot(eyeVec,reflectVec),0.0);
	v_colorDiffuse.w = pow(specular,12.0) * 0.6;

	v_tex0.xy = a_texCoord.xy;
	v_tex0.z = CalcFogFactor(length(gl_Position.xyz));
	v_tex1.xy = a_texCoord1.xy;
	v_tex1.zw = vec2((a_position.x-texCoordOffset.x)/texCoordOffset.z, (a_position.z-texCoordOffset.y)/texCoordOffset.z);
}
);

//-----------------------------------------------------------------------------
// Class:	fragment shader file
//-----------------------------------------------------------------------------
const char* terrainEffect_frag = STRINGIFY(

uniform bool k_bBoolean10; // g_bIsBaseEnabled

\n#ifdef GL_ES\n
uniform lowp vec4 materialdiffuse;
uniform lowp vec4 fogColor; // g_fogColor
varying lowp vec4 v_colorDiffuse;
varying mediump vec3 v_tex0;
varying mediump vec4 v_tex1;
\n#else\n
uniform vec4 materialdiffuse;
uniform vec4 fogColor; // g_fogColor
varying vec4 v_colorDiffuse;
varying vec3 v_tex0;
varying vec4 v_tex1;
\n#endif\n


void main()
{
	vec4 normalColor = vec4(0.0, 0.0, 0.0, 1.0);
	vec3 colorDif = v_colorDiffuse.xyz;

	// layer alpha0 * detail1
	vec4 color1 = texture2D(CC_Texture1, v_tex1.xy);
	float alpha = texture2D(CC_Texture0, v_tex0.xy).a;

	normalColor.xyz = color1.xyz * alpha;
	float specularWeight = (1.0 - color1.a) * alpha;
	
    normalColor.xyz *= colorDif;
	normalColor.xyz += (specularWeight * v_colorDiffuse.www) * materialdiffuse.xyz;

	// multiple base layer
	normalColor.xyz *= texture2D(CC_Texture2, v_tex1.zw).xyz;

	// fog color
	normalColor.xyz *= (1.0-v_tex0.z);
	if(k_bBoolean10)
		normalColor.xyz += fogColor.xyz * v_tex0.z;
	
	gl_FragColor = normalColor;
}
);


Unifroms
{
	mat4 worldviewprojection:worldviewprojection;
	mat4 worldview:worldview;
	mat4 world:world;
	vec4 sunvector:sunvector;
	vec4 materialdiffuse:materialdiffuse;
	vec4 ambientlight:ambientlight;
	vec4 fogparameters:fogparameters; 
	vec4 fogColor:fogColor;
}

#CODEBLOCK_BEGIN
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class:	vertex shader file
//-----------------------------------------------------------------------------

attribute vec4 a_position;
attribute vec2 a_texCoord;
attribute vec3 a_normal;

uniform mat4 worldviewprojection;
uniform mat4 worldview;
uniform mat4 world;

uniform vec4 sunvector;
uniform vec4 materialdiffuse;
uniform vec4 ambientlight;

uniform vec4 fogparameters; // (fogstart, fogrange, fogDensity, reserved)
uniform vec4 fogColor;


varying vec4 v_color;
varying vec2 v_texCoord;


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
	v_texCoord = a_texCoord;

	// camera space position
	vec4 cameraPos = worldview * a_position; 
	// world space normal
	vec3 worldNormal = normalize( vec3(mat3(world) * a_normal) ); 

	// apply shadow either before or after color clamp
	v_color.xyz = max(0.0, dot( sunvector.xyz, worldNormal ))*materialdiffuse.xyz;
	v_color.xyz += ambientlight.xyz;
	//save the fog distance
	v_color.w = CalcFogFactor(cameraPos.z);
}
#CODEBLOCK_END meshNormalEffect_vert

#CODEBLOCK_BEGIN
//-----------------------------------------------------------------------------
// Class:	fragment shader file
//-----------------------------------------------------------------------------

uniform vec4 fogColor;
uniform bool alphatesting;
uniform float opacity; 

varying vec4 v_color;
varying vec2 v_texCoord;

void main()
{
    vec4 albedoColor = texture2D(CC_Texture0, v_texCoord);
	albedoColor.xyz *= v_color.xyz;
	// this is for alpha testing. 
	if(alphatesting && albedoColor.a < 0.05)
		discard;
	
	float fog = v_color.w;
	gl_FragColor.xyz = mix(albedoColor.xyz, fogColor.xyz, fog);
	fog = clamp( (fog-0.8)*16.0, 0.0, 1.0);
	gl_FragColor.w = mix(albedoColor.w, 0.0, fog)*opacity;
	// gl_FragColor = vec4(1.0,0.0,0.0, 1.0); // testing
}
#CODEBLOCK_END meshNormalEffect_frag

technique default
{
	pass P0
	{
		VertexShader = compile vs meshNormalEffect_vert();
		PixelShader = compile ps meshNormalEffect_frag();
	}
}
//-----------------------------------------------------------------------------
// Class:	basic block rendering
// Authors:	LiXizhi
// Emails:	LiXizhi@yeah.net
// Company: ParaEngine
// Date:	2014.9.12
//-----------------------------------------------------------------------------
//-----------------------------------------------------------------------------
// Class:	vertex shader file
//-----------------------------------------------------------------------------
const char* shaderBlockEffect_vert = STRINGIFY(

attribute vec4 a_position;
attribute vec2 a_texCoord;
attribute vec4 a_color; // x: blockid, y torch light, z sun light, w ambient occlusion
attribute vec4 a_color2; // second color
attribute vec3 a_normal;

uniform mat4 worldviewprojection;
uniform mat4 world;
uniform vec4 ConstVector0;  // light_params
uniform vec4 sunvector;
uniform vec4 fogparameters; // (fogstart, fogrange, fogDensity, reserved)

\n#ifdef GL_ES\n
uniform lowp vec4 fogColor;
varying lowp vec4 v_color;
varying mediump vec2 v_texCoord;
\n#else\n
uniform vec4 fogColor;
varying vec4 v_color;
varying vec2 v_texCoord;
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
	v_texCoord = a_texCoord;

	// emissive block light received by this block. 
	float torch_light_strength = a_color.y;
	vec3 torch_light = ConstVector0.xyz * torch_light_strength;

	// sun light + sky(fog) light
	float sun_light_strength = clamp(a_color.z*ConstVector0.w, 0.0, 1.0); // normalize to 0,1 range

	float lightFactor = 0.7 + 0.3*clamp(dot(sunvector.xyz,a_normal), 0.0,1.0);
	vec3 sun_light = (vec3(1.0, 1.0, 1.0) + fogColor.xyz * 0.05) * (sun_light_strength * lightFactor);
	torch_light_strength *= lightFactor;

	// compose and interpolate so that the strength of light is almost linear 
	vec3 final_light = mix(torch_light.xyz+sun_light.xyz, sun_light.xyz, sun_light_strength / (torch_light_strength + sun_light_strength+0.001));
	
	// apply shadow either before or after color clamp
	v_color.xyz = final_light*a_color.w;
	v_color.rgb *= a_color2.bgr;

	//calculate the fog factor
	v_color.w = CalcFogFactor(length(gl_Position.xyz));
}
);

//-----------------------------------------------------------------------------
// Class:	fragment shader file: opache block
//-----------------------------------------------------------------------------
const char* shaderOpacheBlockEffect_frag = STRINGIFY(


\n#ifdef GL_ES\n
uniform lowp vec4 fogColor;
varying lowp vec4 v_color;
varying mediump vec2 v_texCoord;
\n#else\n
uniform vec4 fogColor;
varying vec4 v_color;
varying vec2 v_texCoord;
\n#endif\n

void main()
{
    vec4 albedoColor = texture2D(CC_Texture0, v_texCoord).xyzw;
	//gl_FragColor = vec4(albedoColor.xyz * v_color.xyz, albedoColor.a);
	gl_FragColor = vec4(mix(vec3(albedoColor.xyz * v_color.xyz), fogColor.xyz, v_color.w), albedoColor.a);
}
);

//-----------------------------------------------------------------------------
// Class:	vertex shader file: transparent
//-----------------------------------------------------------------------------
const char* shaderTransparentBlockEffect_vert = STRINGIFY(

attribute vec4 a_position;
attribute vec2 a_texCoord;
attribute vec4 a_color; // x: blockid, y torch light, z sun light, w ambient occlusion
attribute vec4 a_color2; // second color
attribute vec3 a_normal;

uniform mat4 worldviewprojection;
uniform mat4 world;
uniform vec4 ConstVector0;  // light_params
uniform vec4 sunvector;
uniform vec4 fogparameters; // (fogstart, fogrange, fogDensity, reserved)

\n#ifdef GL_ES\n
uniform lowp vec4 fogColor;
varying lowp vec4 v_color;
varying mediump vec2 v_texCoord;
\n#else\n
uniform vec4 fogColor;
varying vec4 v_color;
varying vec2 v_texCoord;
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
	v_texCoord = a_texCoord;

	// emissive block light received by this block. 
	float torch_light_strength = a_color.y;
	vec3 torch_light = ConstVector0.xyz * torch_light_strength;

	// sun light + sky(fog) light
	float sun_light_strength = clamp(a_color.z*ConstVector0.w, 0.0, 1.0); // normalize to 0,1 range
	vec3 sun_light = (vec3(1.0, 1.0, 1.0) + fogColor.xyz * 0.05) * sun_light_strength;
	
	// compose and interpolate so that the strength of light is almost linear 
	vec3 final_light = mix(torch_light.xyz+sun_light.xyz, sun_light.xyz, sun_light_strength / (torch_light_strength + sun_light_strength+0.001));
	
	// apply shadow either before or after color clamp
	v_color.xyz = final_light*a_color.w;
	v_color.rgb *= a_color2.bgr;

	//calculate the fog factor
	v_color.w = CalcFogFactor(length(gl_Position.xyz));
}
);

//-----------------------------------------------------------------------------
// Class:	fragment shader file: transparent block
//-----------------------------------------------------------------------------
const char* shaderTransparentBlockEffect_frag = STRINGIFY(

\n#ifdef GL_ES\n
uniform lowp vec4 fogColor;
varying lowp vec4 v_color;
varying mediump vec2 v_texCoord;
\n#else\n
uniform vec4 fogColor;
varying vec4 v_color;
varying vec2 v_texCoord;
\n#endif\n

void main()
{
    vec4 albedoColor = texture2D(CC_Texture0, v_texCoord);
	// this is for alpha testing. 
	if(albedoColor.a < 0.95)
		discard;
	//gl_FragColor = vec4(albedoColor.xyz * v_color.xyz, albedoColor.a);
	gl_FragColor = vec4(mix(vec3(albedoColor.xyz * v_color.xyz), fogColor.xyz, v_color.w), albedoColor.a);
}
);

const char* shaderBlockWithShadowEffect_vert = STRINGIFY(

attribute vec4 a_position;
attribute vec2 a_texCoord;
attribute vec4 a_color; // x: blockid, y torch light, z sun light, w ambient occlusion
attribute vec4 a_color2; // second color
attribute vec3 a_normal;

uniform mat4 worldviewprojection;
uniform mat4 world;
uniform vec4 ConstVector0;  // light_params
uniform vec4 sunvector;
uniform vec4 fogparameters; // (fogstart, fogrange, fogDensity, reserved)
uniform mat4 texworldviewproj;

\n#ifdef GL_ES\n
uniform lowp vec4 fogColor;
varying lowp vec4 v_color;
varying mediump vec2 v_texCoord;
varying highp vec3 v_shadowCoord;
\n#else\n
uniform vec4 fogColor;
varying vec4 v_color;
varying vec2 v_texCoord;
varying vec3 v_shadowCoord;
\n#endif\n

// Calculates fog factor based upon distance
float CalcFogFactor(float d)
{
	float fogCoeff = 0.0;
	fogCoeff = (d - fogparameters.x) / fogparameters.y;
	return clamp(fogCoeff, 0.0, 1.0);
}

void main()
{
	gl_Position = worldviewprojection * a_position;
	v_texCoord = a_texCoord;

	// emissive block light received by this block. 
	float torch_light_strength = a_color.y;
	vec3 torch_light = ConstVector0.xyz * torch_light_strength;

	// sun light + sky(fog) light
	float sun_light_strength = clamp(a_color.z*ConstVector0.w, 0.0, 1.0); // normalize to 0,1 range

	float lightFactor = 0.7 + 0.3*clamp(dot(sunvector.xyz, a_normal), 0.0, 1.0);
	vec3 sun_light = (vec3(1.0, 1.0, 1.0) + fogColor.xyz * 0.05) * (sun_light_strength * lightFactor);
	torch_light_strength *= lightFactor;

	// compose and interpolate so that the strength of light is almost linear 
	vec3 final_light = mix(torch_light.xyz + sun_light.xyz, sun_light.xyz, sun_light_strength / (torch_light_strength + sun_light_strength + 0.001));

	// apply shadow either before or after color clamp
	v_color.xyz = final_light*a_color.w;
	v_color.rgb *= a_color2.bgr;

	//calculate the fog factor
	v_color.w = CalcFogFactor(length(gl_Position.xyz));

	vec4 shadow_coord = texworldviewproj * a_position;
	v_shadowCoord = shadow_coord.xyz / shadow_coord.w;
	v_shadowCoord.z = v_shadowCoord.z * 0.5 + 0.5;
}
);

const char* shaderOpacheBlockWithShadowEffect_frag = STRINGIFY(

uniform int shadowmapsize;
\n#ifdef GL_ES\n
precision highp float;
uniform lowp vec4 fogColor;
varying lowp vec4 v_color;
varying mediump vec2 v_texCoord;
varying highp vec3 v_shadowCoord;
\n#else\n
uniform vec4 fogColor;
varying vec4 v_color;
varying vec2 v_texCoord;
varying vec3 v_shadowCoord;
\n#endif\n

void main()
{
	vec4 albedoColor = texture2D(CC_Texture0, v_texCoord).xyzw;
	//gl_FragColor = vec4(albedoColor.xyz * v_color.xyz, albedoColor.a);
	gl_FragColor = vec4(mix(vec3(albedoColor.xyz * v_color.xyz), fogColor.xyz, v_color.w), albedoColor.a);
\n#define PCF\n
\n#ifdef PCF\n
	if ((v_shadowCoord.z >= 0.0) && (v_shadowCoord.z <= 1.0))
	{
		float shadow_map_size = 1024.0;
		float inv_shadow_map_size = 1.0 / shadow_map_size;
		vec2 base_shadow_coord = floor(v_shadowCoord.xy * shadow_map_size) * inv_shadow_map_size;
		vec2 mix_factor = (v_shadowCoord.xy - base_shadow_coord) * shadow_map_size;
		float shadow_factor_0 = 1.0;
		vec2 shadow_coord_0 = base_shadow_coord;
		if ((shadow_coord_0.x >= 0.0) && (shadow_coord_0.x <= 1.0) && (shadow_coord_0.y >= 0.0) && (shadow_coord_0.y <= 1.0))
		{
			vec4 shadow_depth_pack = texture2D(CC_Texture1, shadow_coord_0);
			float shadow_depth = shadow_depth_pack.r + shadow_depth_pack.g / 256.0 + shadow_depth_pack.b / (256.0*256.0) + shadow_depth_pack.a / (256.0*256.0*256.0);
			shadow_factor_0 = (v_shadowCoord.z > shadow_depth) ? 0.0 : 1.0;
		}
		float shadow_factor_1 = 1.0;
		vec2 shadow_coord_1 = base_shadow_coord + vec2(1.0, 0.0) * inv_shadow_map_size;
		if ((shadow_coord_1.x >= 0.0) && (shadow_coord_1.x <= 1.0) && (shadow_coord_1.y >= 0.0) && (shadow_coord_1.y <= 1.0))
		{
			vec4 shadow_depth_pack = texture2D(CC_Texture1, shadow_coord_1);
			float shadow_depth = shadow_depth_pack.r + shadow_depth_pack.g / 256.0 + shadow_depth_pack.b / (256.0*256.0) + shadow_depth_pack.a / (256.0*256.0*256.0);
			shadow_factor_1 = (v_shadowCoord.z > shadow_depth) ? 0.0 : 1.0;
		}
		float x_lerp_0 = mix(shadow_factor_0, shadow_factor_1, mix_factor.x);

		shadow_factor_0 = 1.0;
		shadow_coord_0 = base_shadow_coord + vec2(0.0, 1.0) * inv_shadow_map_size;
		if ((shadow_coord_0.x >= 0.0) && (shadow_coord_0.x <= 1.0) && (shadow_coord_0.y >= 0.0) && (shadow_coord_0.y <= 1.0))
		{
			vec4 shadow_depth_pack = texture2D(CC_Texture1, shadow_coord_0);
			float shadow_depth = shadow_depth_pack.r + shadow_depth_pack.g / 256.0 + shadow_depth_pack.b / (256.0*256.0) + shadow_depth_pack.a / (256.0*256.0*256.0);
			shadow_factor_0 = (v_shadowCoord.z > shadow_depth) ? 0.0 : 1.0;
		}
		shadow_factor_1 = 1.0;
		shadow_coord_1 = base_shadow_coord + vec2(1.0) * inv_shadow_map_size;
		if ((shadow_coord_1.x >= 0.0) && (shadow_coord_1.x <= 1.0) && (shadow_coord_1.y >= 0.0) && (shadow_coord_1.y <= 1.0))
		{
			vec4 shadow_depth_pack = texture2D(CC_Texture1, shadow_coord_1);
			float shadow_depth = shadow_depth_pack.r + shadow_depth_pack.g / 256.0 + shadow_depth_pack.b / (256.0*256.0) + shadow_depth_pack.a / (256.0*256.0*256.0);
			shadow_factor_1 = (v_shadowCoord.z > shadow_depth) ? 0.0 : 1.0;
		}
		float x_lerp_1 = mix(shadow_factor_0, shadow_factor_1, mix_factor.x);
		float factor = mix(x_lerp_0, x_lerp_1, mix_factor.y);
		factor = clamp(factor + 0.3, 0.0, 1.0);
		gl_FragColor.rgb *= factor;
	}
\n#else\n
	if ((v_shadowCoord.x >= 0.0) && (v_shadowCoord.x <= 1.0) && (v_shadowCoord.y >= 0.0) && (v_shadowCoord.y <= 1.0) && (v_shadowCoord.z >= 0.0) && (v_shadowCoord.z <= 1.0))
	{
		vec4 shadow_depth_pack = texture2D(CC_Texture1, v_shadowCoord.xy);
		float shadow_depth = shadow_depth_pack.r + shadow_depth_pack.g / 256.0 + shadow_depth_pack.b / (256.0*256.0) + shadow_depth_pack.a / (256.0*256.0*256.0);
		gl_FragColor.rgb *= (v_shadowCoord.z > shadow_depth) ? 0.3 : 1.0;
	}
\n#endif\n
}
);

const char* shaderTransparentBlockWithShadowEffect_frag = STRINGIFY(

\n#ifdef GL_ES\n
uniform lowp vec4 fogColor;
varying lowp vec4 v_color;
varying mediump vec2 v_texCoord;
varying highp vec3 v_shadowCoord;
\n#else\n
uniform vec4 fogColor;
varying vec4 v_color;
varying vec2 v_texCoord;
varying vec3 v_shadowCoord;
\n#endif\n

void main()
{
	vec4 albedoColor = texture2D(CC_Texture0, v_texCoord);
	// this is for alpha testing. 
	if (albedoColor.a < 0.95)
		discard;
	//gl_FragColor = vec4(albedoColor.xyz * v_color.xyz, albedoColor.a);
	gl_FragColor = vec4(mix(vec3(albedoColor.xyz * v_color.xyz), fogColor.xyz, v_color.w), albedoColor.a);

\n#define PCF\n
\n#ifdef PCF\n
	if ((v_shadowCoord.z >= 0.0) && (v_shadowCoord.z <= 1.0))
	{
		float shadow_map_size = 1024.0;
		float inv_shadow_map_size = 1.0 / shadow_map_size;
		vec2 base_shadow_coord = floor(v_shadowCoord.xy * shadow_map_size) * inv_shadow_map_size;
		vec2 mix_factor = (v_shadowCoord.xy - base_shadow_coord) * shadow_map_size;
		float shadow_factor_0 = 1.0;
		vec2 shadow_coord_0 = base_shadow_coord;
		if ((shadow_coord_0.x >= 0.0) && (shadow_coord_0.x <= 1.0) && (shadow_coord_0.y >= 0.0) && (shadow_coord_0.y <= 1.0))
		{
			vec4 shadow_depth_pack = texture2D(CC_Texture1, shadow_coord_0);
			float shadow_depth = shadow_depth_pack.r + shadow_depth_pack.g / 256.0 + shadow_depth_pack.b / (256.0*256.0) + shadow_depth_pack.a / (256.0*256.0*256.0);
			shadow_factor_0 = (v_shadowCoord.z > shadow_depth) ? 0.0 : 1.0;
		}
		float shadow_factor_1 = 1.0;
		vec2 shadow_coord_1 = base_shadow_coord + vec2(1.0, 0.0) * inv_shadow_map_size;
		if ((shadow_coord_1.x >= 0.0) && (shadow_coord_1.x <= 1.0) && (shadow_coord_1.y >= 0.0) && (shadow_coord_1.y <= 1.0))
		{
			vec4 shadow_depth_pack = texture2D(CC_Texture1, shadow_coord_1);
			float shadow_depth = shadow_depth_pack.r + shadow_depth_pack.g / 256.0 + shadow_depth_pack.b / (256.0*256.0) + shadow_depth_pack.a / (256.0*256.0*256.0);
			shadow_factor_1 = (v_shadowCoord.z > shadow_depth) ? 0.0 : 1.0;
		}
		float x_lerp_0 = mix(shadow_factor_0, shadow_factor_1, mix_factor.x);

		shadow_factor_0 = 1.0;
		shadow_coord_0 = base_shadow_coord + vec2(0.0, 1.0) * inv_shadow_map_size;
		if ((shadow_coord_0.x >= 0.0) && (shadow_coord_0.x <= 1.0) && (shadow_coord_0.y >= 0.0) && (shadow_coord_0.y <= 1.0))
		{
			vec4 shadow_depth_pack = texture2D(CC_Texture1, shadow_coord_0);
			float shadow_depth = shadow_depth_pack.r + shadow_depth_pack.g / 256.0 + shadow_depth_pack.b / (256.0*256.0) + shadow_depth_pack.a / (256.0*256.0*256.0);
			shadow_factor_0 = (v_shadowCoord.z > shadow_depth) ? 0.0 : 1.0;
		}
		shadow_factor_1 = 1.0;
		shadow_coord_1 = base_shadow_coord + vec2(1.0) * inv_shadow_map_size;
		if ((shadow_coord_1.x >= 0.0) && (shadow_coord_1.x <= 1.0) && (shadow_coord_1.y >= 0.0) && (shadow_coord_1.y <= 1.0))
		{
			vec4 shadow_depth_pack = texture2D(CC_Texture1, shadow_coord_1);
			float shadow_depth = shadow_depth_pack.r + shadow_depth_pack.g / 256.0 + shadow_depth_pack.b / (256.0*256.0) + shadow_depth_pack.a / (256.0*256.0*256.0);
			shadow_factor_1 = (v_shadowCoord.z > shadow_depth) ? 0.0 : 1.0;
		}
		float x_lerp_1 = mix(shadow_factor_0, shadow_factor_1, mix_factor.x);
		float factor = mix(x_lerp_0, x_lerp_1, mix_factor.y);
		factor = clamp(factor + 0.3, 0.0, 1.0);
		gl_FragColor.rgb *= factor;
	}
\n#else\n
	if ((v_shadowCoord.x >= 0.0) && (v_shadowCoord.x <= 1.0) && (v_shadowCoord.y >= 0.0) && (v_shadowCoord.y <= 1.0) && (v_shadowCoord.z >= 0.0) && (v_shadowCoord.z <= 1.0))
	{
		vec4 shadow_depth_pack = texture2D(CC_Texture1, v_shadowCoord.xy);
		float shadow_depth = shadow_depth_pack.r + shadow_depth_pack.g / 256.0 + shadow_depth_pack.b / (256.0*256.0) + shadow_depth_pack.a / (256.0*256.0*256.0);
		gl_FragColor.rgb *= (v_shadowCoord.z > shadow_depth) ? 0.3 : 1.0;
	}
\n#endif\n
}
);



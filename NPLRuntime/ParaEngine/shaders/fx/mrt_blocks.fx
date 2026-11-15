// Author: LiXizhi
// Date: 2013/10/4
// Desc: multiple render target block rendering shaders
// Some of its ideas are borrowed from sonic ether's unbelievable shaders. 

#define ALPHA_TESTING_REF  0.95

/** undefine to use linear torch light, otherwise it is power */
// #define POWER_LIGHT_TORCH

#ifdef POWER_LIGHT_TORCH
	/** whether to torch lit small or bigger area */
	// #define LIGHT_TORCH_SMALL_RANGE
#endif

#define BLOCK_SIZE	1.0416666

#define WAVING_GRASS
#define WAVING_WHEAT
#define WAVING_LEAVES
//#define WAVING_FIRE
/** whether to wave vertex position. */
#define WAVING_WATER
/** undefine this to debug reflection in composite shader. it will render mirror images. */
#define WATER_USEBUMPMAP

#define WATER_BUMP_SPEED  0.0004
#define WATER_BUMP_TEX_SCALE 0.04
#define WATER_BUMP_MAX_AMPLITUDE 0.1

/** how much to bump for blocks with normal map */
#define METAL_BUMP_MAX_AMPLITUDE 1.0

////////////////////////////////////////////////////////////////////////////////
//  Per frame parameters
float4x4 mWorldViewProj : worldviewprojection;
float4x4 mWorldView : worldview;

// for selection effect: light_params.x: sun_lightIntensity, light_params.y: damageDegree
// for block effect: light_params.xyz: light color, light_params.w light intensity
float4 light_params: ConstVector0; 
// world position
float4 vWorldPos		: worldpos;

// x for world time, y for rainStrength,
float4 g_parameter0		: ConstVector1; 

// block material params
float4 materialUV	: materialUV;
float4 materialBaseColor : materialBaseColor;
float4 materialEmissiveColor : materialEmissiveColor;
float materialMetallic : materialMetallic;
float materialSpecular : materialSpecular;


// texture 0
texture tex0 : TEXTURE; 
sampler tex0Sampler: register(s0) = sampler_state 
{
	Texture = <tex0>;

	MinFilter = POINT;
	MagFilter = POINT;
};

// texture 1
texture tex1 : TEXTURE; 
sampler tex1Sampler: register(s1) = sampler_state 
{
	Texture = <tex1>;
	AddressU = wrap;
	AddressV = wrap;
	MagFilter = Linear;
	MinFilter = Linear;
	MipFilter = Linear;
};

// normal map
texture tex2 : TEXTURE; 
sampler normalSampler = sampler_state 
{
    texture = <tex2>;
    AddressU  = wrap;        
    AddressV  = wrap;
    MagFilter = Linear;	
	MinFilter = Linear;
	MipFilter = Linear;
};

struct BasicBlockVSOut
{
	float4 pos		: POSITION;
	float2 texcoord : TEXCOORD0;
	float3 normal   : TEXCOORD1;
	float4 color : COLOR0;
	float4 depth : COLOR1;
};

struct SelectBlockVertexLayout
{
	float4 pos	:POSITION;
	float2 texcoord	:TEXCOORD0;
};

struct SelectBlockVSOut
{
	float4 pos	:POSITION;
	float2 texcoord	:TEXCOORD0;
};

struct BlockPSOut
{
	// standard color
	float4 Color	: COLOR0;
	// r is 32bits depth
	float4 Depth	: COLOR1; 
	// r is category id, g is sun light, b is block light
	float4 BlockInfo : COLOR2; 
	// xyz is normal. can be (0,0,0) if no normal is used. 
	float4 Normal : COLOR3; 
};


///////////////////////////////////////////////////////////////////////////////
// basic cube or solid blocks 
///////////////////////////////////////////////////////////////////////////////

BasicBlockVSOut BasicBlockVS(	float4 position	: POSITION,
							float3	Norm	: NORMAL,
							half4 color		: COLOR0,
							half4 color2 : COLOR1,
							float2 texcoord	: TEXCOORD0)
{
	BasicBlockVSOut output;

	output.pos = mul(position, mWorldViewProj);
	output.normal = Norm*0.5+0.5;
	output.texcoord = texcoord;
	
	// emissive block light received by this block. 
	float torch_light_strength = color.y;
	
	// sun light + sky(fog) light
	float sun_light_strength = color.x;
	
	// apply AO shadow
	output.color.xyz = color.www;
	output.color.xyz *= color2.rgb;

	output.color.w = 1;

	// block category id
	output.depth.r = color.z;
	// torch light value
	output.depth.g = sun_light_strength;
	// sun light value
	output.depth.b = torch_light_strength;
	// view space linear depth value
	float4 viewPos = mul(position, mWorldView);
	output.depth.a = viewPos.z;
	return output;
}

BlockPSOut BasicBlockPS(BasicBlockVSOut input)
{
	BlockPSOut o;
	float4 albedoColor = tex2D(tex0Sampler,input.texcoord);
	o.Color = albedoColor * input.color;
	o.BlockInfo = float4(input.depth.rgb, 1);
	o.Depth = float4(input.depth.a,0,0,1);
	o.Normal = float4(input.normal.xyz, 1.0);
	return o;
}

///////////////////////////////////////////////////////////////////////////////
// Transparent entities like grass and leaves
///////////////////////////////////////////////////////////////////////////////

// color.x: sun light  color.y:block light  color.z*255 category_id; 
BasicBlockVSOut TransparentEntityVS(	float4 position	: POSITION,
							float3	Norm		: NORMAL,
							half4 color		: COLOR0,
							half4 color2 : COLOR1,
							float2 texcoord	: TEXCOORD0)
{
	BasicBlockVSOut output;
	output.normal = Norm*0.5+0.5;
	// category id for block types 
	int category_id = (int)(color.z * 255.0 + 0.4);

	// convert to 60FPS ticks
	float worldTime = g_parameter0.x / 60.0 * 3.14159265358979323846264;
	// usually 0
	float rainStrength = g_parameter0.y;

	float3 world_pos = vWorldPos.xyz + position.xyz * BLOCK_SIZE;
#ifdef WAVING_GRASS	
	
	//Grass//
	if(	category_id == 31 && texcoord.y < 0.15)
	{
		float speed = 8.0;
		
		float magnitude = sin((worldTime / (28.0)) + world_pos.x + world_pos.z) * 0.1 + 0.1;
		float d0 = sin(worldTime / (122.0 * speed)) * 3.0 - 1.5 + world_pos.z;
		float d1 = sin(worldTime / (152.0 * speed)) * 3.0 - 1.5 + world_pos.x;
		float d2 = sin(worldTime / (122.0 * speed)) * 3.0 - 1.5 + world_pos.x;
		float d3 = sin(worldTime / (152.0 * speed)) * 3.0 - 1.5 + world_pos.z;
		position.x += sin((worldTime / (28.0 * speed)) + (world_pos.x + d0) * 0.1 + (world_pos.z + d1) * 0.1) * magnitude * (1.0f + rainStrength * 1.4f);
		position.z += sin((worldTime / (28.0 * speed)) + (world_pos.z + d2) * 0.1 + (world_pos.x + d3) * 0.1) * magnitude * (1.0f + rainStrength * 1.4f);

		//small leaf movement//
		speed = 0.8;
		
		magnitude = (sin(((world_pos.y + world_pos.x)/2.0 + worldTime / ((28.0)))) * 0.05 + 0.15) * 0.4;
		d0 = sin(worldTime / (112.0 * speed)) * 3.0 - 1.5;
		d1 = sin(worldTime / (142.0 * speed)) * 3.0 - 1.5;
		d2 = sin(worldTime / (112.0 * speed)) * 3.0 - 1.5;
		d3 = sin(worldTime / (142.0 * speed)) * 3.0 - 1.5;
		position.x += sin((worldTime / (18.0 * speed)) + (-world_pos.x + d0)*1.6 + (world_pos.z + d1)*1.6) * magnitude * (1.0f + rainStrength * 1.7f);
		position.z += sin((worldTime / (18.0 * speed)) + (world_pos.z + d2)*1.6 + (-world_pos.x + d3)*1.6) * magnitude * (1.0f + rainStrength * 1.7f);
		position.y += sin((worldTime / (11.0 * speed)) + (world_pos.z + d2) + (world_pos.x + d3)) * (magnitude/3.0) * (1.0f + rainStrength * 1.7f);
	}
#endif

#ifdef WAVING_LEAVES
	//Leaves//
	if (category_id == 18) {
		float speed = 1.0;
		float magnitude = (sin((world_pos.y + world_pos.x + worldTime / ((28.0) * speed))) * 0.15 + 0.15) * 0.20;
		float d0 = sin(worldTime / (112.0 * speed)) * 3.0 - 1.5;
		float d1 = sin(worldTime / (142.0 * speed)) * 3.0 - 1.5;
		float d2 = sin(worldTime / (132.0 * speed)) * 3.0 - 1.5;
		float d3 = sin(worldTime / (122.0 * speed)) * 3.0 - 1.5;
		position.x += sin((worldTime / (18.0 * speed)) + (-world_pos.x + d0)*1.6 + (world_pos.z + d1)*1.6) * magnitude * (1.0f + rainStrength * 1.0f);
		position.z += sin((worldTime / (17.0 * speed)) + (world_pos.z + d2)*1.6 + (-world_pos.x + d3)*1.6) * magnitude * (1.0f + rainStrength * 1.0f);
		position.y += sin((worldTime / (11.0 * speed)) + (world_pos.z + d2) + (world_pos.x + d3)) * (magnitude/2.0) * (1.0f + rainStrength * 1.0f);
	}
#endif	

	output.pos = mul(position, mWorldViewProj);
	output.texcoord = texcoord;
	
	// emissive block light received by this block. 
	float torch_light_strength = color.y;
	
	// sun light + sky(fog) light
	float sun_light_strength = color.x;
	
	// apply AO shadow
	output.color.xyz = color.www;
	output.color.xyz *= color2.rgb;
	output.color.w = 1;

	// block id
	output.depth.r = color.z; 
	// torch light value
	output.depth.g = sun_light_strength;
	// sun light value
	output.depth.b = torch_light_strength;
	// depth value
	// output.depth.a = output.pos.z / output.pos.w;
	float4 viewPos = mul(position, mWorldView);
	output.depth.a = viewPos.z;

	return output;
}

BlockPSOut TransparentEntityPS(BasicBlockVSOut input)
{
	BlockPSOut o;
	float4 albedoColor = tex2D(tex0Sampler,input.texcoord);
	clip(albedoColor.w-ALPHA_TESTING_REF);
	o.Color = albedoColor * input.color;
	o.BlockInfo = float4(input.depth.rgb, 1);
	o.Depth = float4(input.depth.a,0,0,1);
	o.Normal = float4(input.normal.xyz, 1);
	return o;
}



/////////////////////////////////////////////////////////////////
// selected block 
/////////////////////////////////////////////////////////////////

SelectBlockVSOut SelectBlockVS(	float4 pos	: POSITION,
								float2 texcoord	:TEXCOORD0)
{
	SelectBlockVSOut result;
	result.pos = mul(pos, mWorldViewProj);
	result.texcoord = texcoord;
	return result;
}

float4 SelectBlockPS(SelectBlockVSOut input) :COLOR0
{
	float4 color;
	float grayScale = tex2D(tex0Sampler,input.texcoord).x;
	grayScale = grayScale * grayScale * light_params.x;
	color.xyz = grayScale * float3(1,1,0.6);
	color.w = 1;
	return color;
}
/////////////////////////////////////////////////////////////////
// damaged block 
/////////////////////////////////////////////////////////////////

float4 DamagedBlockPS(SelectBlockVSOut input) :COLOR0
{
	float4 color;
	color = tex2D(tex0Sampler,input.texcoord);
	//color.w = color.x * light_params.y;
	return color;
}

///////////////////////////////////////////////////////////////////////////////
// water blocks 
///////////////////////////////////////////////////////////////////////////////

struct WaterBlockVSOut
{
	float4 pos		: POSITION;
	float4 texcoord : TEXCOORD0;
	float4 color : COLOR0;
	float4 depth : COLOR1;
	float4 position : COLOR2;
	float3 normal:COLOR3;
};

WaterBlockVSOut WaterVS(	float4 position	: POSITION,
							float3	Norm		: NORMAL,
							half4 color		: COLOR0,
							half4 color2 : COLOR1,
							float2 texcoord	: TEXCOORD0)
{
	WaterBlockVSOut output;

	output.position = position;
	output.normal = Norm*0.5+0.5;

	

	
	float3 world_pos = vWorldPos.xyz + position.xyz * BLOCK_SIZE;

	// convert to 60FPS ticks
	float worldTime = g_parameter0.x / 60;

#ifdef WATER_USEBUMPMAP
	// zw is normal map texture coordinates
	if(Norm.y!=0)
		output.texcoord.zw = position.xz*WATER_BUMP_TEX_SCALE + worldTime * WATER_BUMP_SPEED;
	else if(Norm.x!=0)
		output.texcoord.zw = position.yz*WATER_BUMP_TEX_SCALE + worldTime * WATER_BUMP_SPEED;
	else
		output.texcoord.zw = position.xy*WATER_BUMP_TEX_SCALE + worldTime * WATER_BUMP_SPEED;
#else
	output.texcoord.zw = float2(0, 0);
#endif

#ifdef WAVING_WATER
	float speed = 1.0;
    float magnitude = (sin((worldTime / ((28.0) * speed))) * 0.05 + 0.15) * 0.27;
    float d2 = sin(worldTime / (162.0 * speed)) * 3.0 - 1.5;
    float d3 = sin(worldTime / (112.0 * speed)) * 3.0 - 1.5;
    position.y += sin((worldTime / (15.0 * speed)) + (world_pos.z + d2) + (world_pos.x + d3)) * magnitude;
#endif

	output.pos = mul(position, mWorldViewProj);
	output.texcoord.xy = texcoord;
	
	
	// emissive block light received by this block. 
	float torch_light_strength = color.y;
	
	// sun light + sky(fog) light
	float sun_light_strength = color.x;
	
	// apply AO shadow
	output.color.xyz = color.www;

	output.color.w = 1;

	// block id
	output.depth.r = color.z; 
	// torch light value
	output.depth.g = sun_light_strength;
	// sun light value
	output.depth.b = torch_light_strength;
	// depth value
	// output.depth.a = output.pos.z / output.pos.w;
	float4 viewPos = mul(position, mWorldView);
	output.depth.a = viewPos.z;

	return output;
}

BlockPSOut WaterPS(WaterBlockVSOut input)
{
	BlockPSOut o;
	float4 albedoColor = tex2D(tex0Sampler,input.texcoord.xy);
	o.Color = albedoColor * input.color;
	o.BlockInfo = float4(input.depth.rgb, 1);
	o.Depth = float4(input.depth.a,0,0,1);

#ifdef WATER_USEBUMPMAP
	float3 normal = input.normal.xyz*2-1;
	normal += (tex2D(normalSampler, input.texcoord.zw)*2.0 - 1.0).xyz*WATER_BUMP_MAX_AMPLITUDE;
	normal = normalize(normal);
	o.Normal = float4(normal*0.5 + 0.5, 1); // since water is alpha blended, the alpha channel must be 1, instead of (1-specular)
#else
	o.Normal = float4(input.normal.xyz, 1);
#endif

	return o;
}


///////////////////////////////////////////////////////////////////////////////
// basic cube or solid blocks 
///////////////////////////////////////////////////////////////////////////////

struct BumpBlockVSOut
{
	float4 pos		: POSITION;
	float2 texcoord : TEXCOORD0;
	float3 normal   : TEXCOORD1;
	float3 tangent  : TEXCOORD2;
	float3 binormal : TEXCOORD3;
	float4 color : COLOR0;
	float4 depth : COLOR1;
};

BumpBlockVSOut BumpBlockVS(float4 position	: POSITION,
	float3	Norm : NORMAL,
	half4 color : COLOR0,
	half4 color2 : COLOR1,
	float2 texcoord : TEXCOORD0)
{
	BumpBlockVSOut output;

	output.pos = mul(position, mWorldViewProj);
	float3 normal = Norm;
	output.normal = normal;
	output.texcoord = texcoord;

	// emissive block light received by this block. 
	float torch_light_strength = color.y;

	// sun light + sky(fog) light
	float sun_light_strength = color.x;

	// apply AO shadow
	output.color.xyz = color.www;
	output.color.xyz *= color2.rgb;

	output.color.w = 1;

	// block category id
	output.depth.r = color.z;
	// torch light value
	output.depth.g = sun_light_strength;
	// sun light value
	output.depth.b = torch_light_strength;
	// view space linear depth value
	float4 viewPos = mul(position, mWorldView);
	output.depth.a = viewPos.z;

	float3 binormal;
	float3 tangent;
	if (normal.x > 0.5) {
		//  1.0,  0.0,  0.0
		tangent = (float3(0.0, 0.0, -1.0));
		binormal = (float3(0.0, -1.0, 0.0));
	}
	else if (normal.x < -0.5) {
		// -1.0,  0.0,  0.0
		tangent = (float3(0.0, 0.0, 1.0));
		binormal = (float3(0.0, -1.0, 0.0));
	}
	else if (normal.y > 0.5) {
		//  0.0,  1.0,  0.0
		tangent = (float3(1.0, 0.0, 0.0));
		binormal = (float3(0.0, 0.0, 1.0));
	}
	else if (normal.y < -0.5) {
		//  0.0, -1.0,  0.0
		tangent = (float3(1.0, 0.0, 0.0));
		binormal = (float3(0.0, 0.0, 1.0));
	}
	else if (normal.z > 0.5) {
		//  0.0,  0.0,  1.0
		tangent = (float3(1.0, 0.0, 0.0));
		binormal = (float3(0.0, -1.0, 0.0));
	}
	else if (normal.z < -0.5) {
		//  0.0,  0.0, -1.0
		tangent = (float3(-1.0, 0.0, 0.0));
		binormal = (float3(0.0, -1.0, 0.0));
	}
	output.tangent = tangent;
	output.binormal = binormal;
	
	return output;
}

BlockPSOut BumpBlockPS(BumpBlockVSOut input)
{
	BlockPSOut o;
	float4 albedoColor = tex2D(tex0Sampler, input.texcoord);
	o.Color = albedoColor * input.color;
	o.BlockInfo = float4(input.depth.rgb, 1);
	o.Depth = float4(input.depth.a, 0, 0, 1);

	float3 normal = input.normal.xyz;
	// tagent to world space row major matrix
	float3x3 tbnMatrix = float3x3(input.tangent, input.binormal, normal);

	// normal map 
	float4 normalColor = tex2D(normalSampler, input.texcoord.xy);
	float3 bumpNormal = mul((normalColor.xyz * 2.0 - 1.0), tbnMatrix);
	normal = normalize(bumpNormal);
	normal = normal*0.5 + 0.5;
	o.Normal = float4(normal.xyz, normalColor.w);
	return o;
}

///////////////////////////////////////////////////////////////////////////////
// basic cube or solid blocks 
///////////////////////////////////////////////////////////////////////////////

BumpBlockVSOut MaterialMainVS(float4 position	: POSITION,
	float3	Norm : NORMAL,
	half4 color : COLOR0,
	half4 color2 : COLOR1,
	float2 texcoord : TEXCOORD0)
{
	BumpBlockVSOut output;

	output.pos = mul(position, mWorldViewProj);
	float3 normal = Norm;
	output.texcoord = texcoord;

	// emissive block light received by this block. 
	float torch_light_strength = color.y;

	// sun light + sky(fog) light
	float sun_light_strength = color.x;

	// apply AO shadow
	output.color.xyz = color.www;
	output.color.xyz *= color2.rgb;

	output.color.w = 1;

	// block category id
	output.depth.r = color.z;
	// torch light value
	output.depth.g = sun_light_strength;
	// sun light value
	output.depth.b = torch_light_strength;
	// view space linear depth value
	float4 viewPos = mul(position, mWorldView);
	output.depth.a = viewPos.z;

	// calculate world position
	float3 worldblockPos = (vWorldPos.xyz / BLOCK_SIZE) + position.xyz;

	float3 binormal;
	float3 tangent;
	if (normal.x > 0.5) {
		//  1.0,  0.0,  0.0
		tangent = (float3(0.0, 0.0, -1.0));
		binormal = (float3(0.0, -1.0, 0.0));
		output.texcoord = worldblockPos.zy;
	}
	else if (normal.x < -0.5) {
		// -1.0,  0.0,  0.0
		tangent = (float3(0.0, 0.0, 1.0));
		binormal = (float3(0.0, -1.0, 0.0));
		output.texcoord = float2(32000.0-worldblockPos.z, worldblockPos.y);
	}
	else if (normal.y > 0.5) {
		//  0.0,  1.0,  0.0
		tangent = (float3(1.0, 0.0, 0.0));
		binormal = (float3(0.0, 0.0, 1.0));
		output.texcoord = worldblockPos.xz;
	}
	else if (normal.y < -0.5) {
		//  0.0, -1.0,  0.0
		tangent = (float3(1.0, 0.0, 0.0));
		binormal = (float3(0.0, 0.0, 1.0));
		output.texcoord = float2(32000.0 - worldblockPos.x, worldblockPos.z);
	}
	else if (normal.z > 0.5) {
		//  0.0,  0.0,  1.0
		tangent = (float3(1.0, 0.0, 0.0));
		binormal = (float3(0.0, -1.0, 0.0));
		output.texcoord = float2(32000.0-worldblockPos.x, worldblockPos.y);
	}
	else if (normal.z < -0.5) {
		//  0.0,  0.0, -1.0
		tangent = (float3(-1.0, 0.0, 0.0));
		binormal = (float3(0.0, -1.0, 0.0));
		output.texcoord = worldblockPos.xy;
	}
	output.texcoord += materialUV.zw;
	output.tangent = tangent;
	output.binormal = binormal;

	if (materialMetallic > 0.1)
	{
		// normal map

		// block category id
		output.depth.r = 50.0/255.0; // metal's category id is 50
		output.normal = normal;
	}
	else
	{
		output.normal = Norm * 0.5 + 0.5;
	}

	return output;
}

BlockPSOut MaterialMainPS(BumpBlockVSOut input)
{
	BlockPSOut o;

	float2 uv = (input.texcoord - floor(input.texcoord / materialUV.xy)*materialUV.xy) / materialUV.xy;
	uv.y = 1.0 - uv.y;

	float4 albedoColor = tex2D(tex0Sampler, uv);
	albedoColor = albedoColor * materialBaseColor;
	
	o.Color = albedoColor * input.color;
	o.BlockInfo = float4(input.depth.rgb, 1);
	o.Depth = float4(input.depth.a, 0, 0, 1);

	if (materialEmissiveColor.a > 0)
	{
		float4 emissiveColor = tex2D(tex1Sampler, uv);
		emissiveColor.rgb *= materialEmissiveColor.rgb;
		emissiveColor.a *= materialEmissiveColor.a;
		o.Color.rgb = lerp(o.Color.rgb, emissiveColor.rgb, emissiveColor.a);
		o.BlockInfo.b = clamp(o.BlockInfo.b + emissiveColor.a, 0, 1.0);
	}

	if (materialMetallic > 0.1)
	{
		float3 normal = input.normal.xyz;
		// tagent to world space row major matrix
		float3x3 tbnMatrix = float3x3(input.tangent, input.binormal, normal);

		// normal map 
		float4 normalColor = tex2D(normalSampler, uv);
		float3 bumpNormal = mul((normalColor.xyz * 2.0 - 1.0), tbnMatrix);
		normal = normalize(bumpNormal);
		normal = normal * 0.5 + 0.5;
		o.Normal = float4(normal.xyz, normalColor.w * (1 - materialSpecular));
	}
	else
	{
		o.Normal = float4(input.normal.xyz, (1 - materialSpecular));
	}

	return o;
}


////////////////////////////////////////////////////////////////////////////////
//
//                              shadow map : VS and PS
//
////////////////////////////////////////////////////////////////////////////////

void VertShadow(	float4 position	: POSITION,
					float3	Norm		: NORMAL,
					half4 color		: COLOR0,
					half4 color2 : COLOR1,
					float2 texcoord	: TEXCOORD0,
                 out float4 oPos	: POSITION,
                 out float4	outTex	: TEXCOORD0)
{
    oPos = mul( position, mWorldViewProj );
	outTex.xy = texcoord;
    outTex.zw = oPos.zw;
}

float4 PixShadow( float4	inTex		: TEXCOORD0) : COLOR
{
	half alpha = tex2D(tex0Sampler, inTex.xy).w;
	// alpha testing
	clip(alpha-ALPHA_TESTING_REF);
	
    //return float4(inTex.z/inTex.w, 0, 0, 1);
	return float4(inTex.z, 0, 0, 1); // inTex.w is 1 anyway
	// return float4(0.5, 0, 0, 1);
}

technique SimpleMesh_vs30_ps30
{
	pass P0
	{
		VertexShader = compile vs_3_0 BasicBlockVS();
		PixelShader  = compile ps_3_0 BasicBlockPS();
		FogEnable = false;
	}
	pass P1
	{
		VertexShader = compile vs_3_0 SelectBlockVS();
		PixelShader  = compile ps_3_0 SelectBlockPS();
		FogEnable = false;
	}
	pass P2
	{
		VertexShader = compile vs_3_0 SelectBlockVS();
		PixelShader  = compile ps_3_0 DamagedBlockPS();
		FogEnable = false;
	}
	pass P3
	{
		VertexShader = compile vs_3_0 TransparentEntityVS();
		PixelShader  = compile ps_3_0 TransparentEntityPS();
		FogEnable = false;
	}
	pass P4
	{
		VertexShader = compile vs_3_0 WaterVS();
		PixelShader  = compile ps_3_0 WaterPS();
		FogEnable = false;
	}
	pass P5
	{
		VertexShader = compile vs_3_0 BumpBlockVS();
		PixelShader = compile ps_3_0 BumpBlockPS();
		FogEnable = false;
	}
	pass P5
	{
		VertexShader = compile vs_3_0 MaterialMainVS();
		PixelShader = compile ps_3_0 MaterialMainPS();
		FogEnable = false;
	}
}

technique GenShadowMap
{
    pass p0
    {
        VertexShader = compile vs_2_a VertShadow();
        PixelShader = compile ps_2_a PixShadow();
        FogEnable = false;
    }
}
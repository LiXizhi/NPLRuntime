// Author: LiXizhi@yeah.net
// Date: 2015/5/22
// Desc: block max model shader.

////////////////////////////////////////////////////////////////////////////////
//  Per frame parameters
float4x4 mWorldViewProj : worldviewprojection;
float4x4 mWorldView: worldview;
float4x4 mWorld: world;

float3	colorDiffuse:materialdiffuse;
float3	colorAmbient:ambientlight;
/** x is sun light strength, y is block light strength. */
float3	BlockLightStrength: LightStrength;
float g_opacity : opacity = 1.0;

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
	AddressU = wrap;
	AddressV = wrap;
	MagFilter = Linear;
	MinFilter = Linear;
	MipFilter = Linear;
};

struct VSOut
{
	float4 pos	:POSITION;
	float4 color			: TEXCOORD0;		// diffuse color
	float3 normal				: TEXCOORD1;		
};

VSOut MainVS(float4 pos		: POSITION,
	float3	Norm : NORMAL,
	half4 color : COLOR0
	)
{
	VSOut output;
	output.pos = mul(pos, mWorldViewProj);
	// camera space position
	float4 cameraPos = mul(pos, mWorldView);

	// world space normal
	float3 worldNormal = normalize(mul(Norm, (float3x3)mWorld));
	output.normal = worldNormal*0.5 + 0.5;

	output.color.xyz = color.rgb * colorDiffuse;

	// calculate the fog factor
	output.color.w = cameraPos.z;
	return output;
}

struct BlockPSOut
{
	// standard color
	float4 Color	: COLOR0;
	// r is 32bits depth
	float4 Depth	: COLOR1;
	// r is category id, g is sun light, b is block light
	float4 BlockInfo : COLOR2;
	float4 Normal : COLOR3;
};

BlockPSOut MainPS(VSOut input)
{
	BlockPSOut output;
	output.Color = float4(input.color.rgb, g_opacity);
	output.BlockInfo = float4(1, BlockLightStrength.x, BlockLightStrength.y, 1);
	output.Depth = float4(input.color.w, 0, 0, 1);
	output.Normal = float4(input.normal.xyz, 1);
	return output;
}


struct MaterialVSOut
{
	float4 pos		: POSITION;
	float4 color	: COLOR0;
	float2 texcoord : TEXCOORD0;
	float3 normal   : TEXCOORD1;
	float3 tangent  : TEXCOORD2;
	float3 binormal : TEXCOORD3;
};

MaterialVSOut MaterialMainVS(float4 pos		: POSITION,
	float3	Norm : NORMAL,
	half4 color : COLOR0
)
{
	MaterialVSOut output;
	output.pos = mul(pos, mWorldViewProj);
	// camera space position
	float4 cameraPos = mul(pos, mWorldView);

	// world space normal
	float3 worldNormal = normalize(mul(Norm, (float3x3)mWorld));
	
	output.color.xyz = color.rgb * colorDiffuse;

	// calculate the fog factor
	output.color.w = cameraPos.z;

	float3 worldblockPos = mul(pos.xyz, (float3x3)mWorld).xyz;
	float3 normal = worldNormal;
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
		output.texcoord = float2(32000.0 - worldblockPos.z, worldblockPos.y);
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
		output.texcoord = float2(32000.0 - worldblockPos.x, worldblockPos.y);
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
		output.normal = normal;
	}
	else
	{
		output.normal = Norm * 0.5 + 0.5;
	}

	return output;
}

BlockPSOut MaterialMainPS(MaterialVSOut input)
{
	BlockPSOut o;
	
	float2 uv = (input.texcoord - floor(input.texcoord / materialUV.xy)*materialUV.xy) / materialUV.xy;
	uv.y = 1.0 - uv.y;

	float4 albedoColor = tex2D(tex0Sampler, uv);
	albedoColor = albedoColor * materialBaseColor;

	//o.Color = float4(albedoColor.xyz * input.color.xyz, g_opacity);
	o.Color = float4(albedoColor.xyz, g_opacity);

	o.BlockInfo = float4(1, BlockLightStrength.x, BlockLightStrength.y, 1);
	o.Depth = float4(input.color.w, 0, 0, 1);

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

		o.BlockInfo.r = 50.0 / 255.0; // metal's category id is 50
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

void VertShadow(float4 Pos		: POSITION,
float3	Norm : NORMAL,
half4 color : COLOR0,
out float4 oPos : POSITION,
out float2 Depth : TEXCOORD1)
{
	oPos = mul(Pos, mWorldViewProj);
	Depth.xy = oPos.zw;
}

float4 PixShadow(float2 Depth : TEXCOORD1) : COLOR
{
	return float4(Depth.x, 0.0, 0.0, 1.0);
}

technique SimpleMesh_vs20_ps20
{
	pass P0
	{
		VertexShader = compile vs_2_0 MainVS();
		PixelShader = compile ps_2_0 MainPS();
		FogEnable = false;
	}
	pass P1
	{
		VertexShader = compile vs_2_0 MaterialMainVS();
		PixelShader = compile ps_2_0 MaterialMainPS();
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
	pass p1
	{
		VertexShader = compile vs_2_a VertShadow();
		PixelShader = compile ps_2_a PixShadow();
		FogEnable = false;
	}
}

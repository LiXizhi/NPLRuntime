// Directional light

float ViewAspect;
float TanHalfFOV;
float2 screenParam;
float2 viewportOffset;
float2 viewportScale;

float4x4 matWorld;
float4x4 matView;
float4x4 matProj;

float4 light_diffuse;
float4 light_specular;
float4 light_ambient;

float3 light_position;
float3 light_direction;

float light_range;
float light_falloff;

float light_attenuation0;
float light_attenuation1;
float light_attenuation2;

float light_theta;
float light_phi;


texture tex0 : TEXTURE;
sampler diffuseSampler : register(s0) = sampler_state
{
	Texture = <tex0>;
	MinFilter = Linear;
	MagFilter = Linear;
	MipFilter = None;
	AddressU = clamp;
	AddressV = clamp;
};

texture tex1 : TEXTURE;
sampler depthSampler : register(s1) = sampler_state
{
	Texture = <tex1>;
	MinFilter = Linear;
	MagFilter = Linear;
	MipFilter = None;
	AddressU = clamp;
	AddressV = clamp;
};

texture tex2 : TEXTURE;
sampler normalSampler : register(s2) = sampler_state
{
	Texture = <tex2>;
	MinFilter = Linear;
	MagFilter = Linear;
	MipFilter = None;
	AddressU = clamp;
	AddressV = clamp;
};


struct VSInput
{
    float3 pos : POSITION;
    float2 texCoord : TEXCOORD0;
};

struct VSOut
{
    float4 pos : POSITION;
    float2 texCoord : TEXCOORD0;
    float3 cameraEye : TEXCOORD1;
};

struct PSOut
{
    float4 color : COLOR0;
	// r is category id, g is sun light, b is block light
	float4 BlockInfo : COLOR1;
};

VSOut MainVS(VSInput input)
{
    VSOut output;

    output.pos = float4(input.pos, 1);
    output.texCoord = input.texCoord;
    output.cameraEye = float3(input.pos.x * TanHalfFOV * ViewAspect, input.pos.y * TanHalfFOV, 1);

    return output;
}

float dist_factor(float3 object_pos)
{
	float4 light_pos = mul(float4(light_position, 1), matView);
	float dist = distance(object_pos, light_pos.xyz / light_pos.w);

	float dist_att = 1 / (light_attenuation0 + light_attenuation1 * dist + light_attenuation2 * dist * dist);
	float fastFalloffRange = min(1.0, light_range*0.2);
	float fastFalloffDist = dist - (light_range - fastFalloffRange);
	if (fastFalloffDist > 0)
	{
		// we will make the light disappear quickly in fall off range
		dist_att = dist_att * (1.0 - saturate(fastFalloffDist / fastFalloffRange));
	}
    return dist_att;
}

/*
 * diffuse: object material diffuse color
 * normal: object normal vector in camera space
 * position: object position in camera space
 */
float lighting(float3 normal, float3 position)
{
    float I_diff, I_spec, I_total;
    float3 l, v, n, h;
    float att;

    n = normalize(normal);
    v = normalize(-position);

    // FIXME: two test value
    float m_shi = 1;
    float4 m_spec = float4(1, 1, 1, 1);

    att = 1;

    // tranform light direction from wolrd space to camera space
    float4 light_dir = mul(float4(light_direction, 0), matView);
    l = normalize(-light_dir.xyz);

    I_diff = saturate(dot(l, n));

    h = normalize(l + v);

    I_spec = saturate(dot(l, n)) * pow(saturate(dot(h, n)), m_shi);

    I_total = att * (I_diff + I_spec);
    return I_total;
}

PSOut MainPS(VSOut input)
{
    PSOut output;

    float2 texCoord = input.texCoord;
    float4 color = tex2D(diffuseSampler, texCoord);
    float alpha = color.a;
	
    // if the normal is world space normal value
    float4 norm = tex2D(normalSampler, texCoord);
    float4 normal_in_camera = mul(float4(norm.rgb * 2.0 - 1.0, 0), matView);
    float3 normal = normalize(normal_in_camera.xyz);

	// screen space depth value. 
    float depth = tex2D(depthSampler, texCoord).x;

    // position in camera space
    float4 position = float4(input.cameraEye * depth, 1);

	float lightValue = lighting(normal.xyz, position.xyz);
	// only add to block light value
	output.color = float4(light_diffuse.rgb*lightValue, 1.0);
	output.BlockInfo = float4(0, 0, lightValue, 1.0);
    return output;
}


technique DirectionalLight
{
    pass P0
    {
        VertexShader = compile vs_3_0 MainVS();
        PixelShader = compile ps_3_0 MainPS();

        CullMode = None;
        ZEnable = false;
        ZWriteEnable = false;
    }
}

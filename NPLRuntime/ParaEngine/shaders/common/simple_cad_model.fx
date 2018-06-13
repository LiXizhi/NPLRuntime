// Author: LiXizhi
// Desc: 2006/4

#define ALPHA_TESTING_REF  0.5
#define MAX_LIGHTS_NUM	4
////////////////////////////////////////////////////////////////////////////////
//  Per frame parameters
float4x4 mWorldViewProj: worldviewprojection;
float4x4 mWorldView: worldview;
float4x4 mWorld: world;
float3 g_EyePositionW	:worldcamerapos;

float3 sun_vec: sunvector;

float3	colorDiffuse:materialdiffuse = float3(1,1,1);
float3	colorAmbient:ambientlight;
float3	colorEmissive:materialemissive = half3(0,0,0);

// static branch boolean constants
float g_opacity			:opacity = 1.f; 


////////////////////////////////////////////////////////////////////////////////
//
//  render cad model
//
////////////////////////////////////////////////////////////////////////////////
struct CadModel_VSMainIn
{
	float4 pos:POSITION;
	float3 normal:NORMAL;
};

struct CadModel_VSMainOut
{
	float4 pos :POSITION;
	float3 normal :TEXCOORD0;
	float3 eyeVec :TEXCOORD1;
};

CadModel_VSMainOut CadModel_VSMain(CadModel_VSMainIn vsIn)
{
	CadModel_VSMainOut vsOut;
	//vsIn.pos.xyz *= 100;
	vsOut.pos = mul(vsIn.pos,mWorldViewProj);
	vsOut.normal = mul(vsIn.normal,mWorld);
	
	float3 worldPos = mul(vsIn.pos,mWorld);
	vsOut.eyeVec = g_EyePositionW - worldPos;

	/*
	float3 eyeVec = normalize(g_EyePositionW - worldPos);
	o.texcoord.z = 1- saturate(dot(eveVec,worldNormal));
	*/
	return vsOut;
}

float4 CadModel_PSMain(CadModel_VSMainOut psIn):COLOR0
{
	float3 lightVec = normalize(float3(1,1,1));
	float3 normal = normalize(psIn.normal);
	float diffuseFactor = saturate(dot(lightVec,normal));

	float3 backLightVec = normalize(float3(-1,-1,-1));
	diffuseFactor += saturate(dot(backLightVec,normal))*0.3;

	float3 eyeVec = normalize(psIn.eyeVec);
	float3 reflectVec = reflect(-lightVec,normal);
	float specularFactor = saturate(dot(eyeVec,reflectVec));
	specularFactor = pow(specularFactor,8);
	
	float3 finalColor = (diffuseFactor + specularFactor + 0.1) * colorDiffuse;
	return float4(finalColor,1);
}


float4 CadModelWire_VSMain(float4 pos:POSITION):POSITION
{
	return mul(pos,mWorldViewProj);
}

float4 CadModelWire_PSMain():COLOR0
{
	return float4(colorDiffuse,1);
}


////////////////////////////////////////////////////////////////////////////////
//
//                              Technique
//
////////////////////////////////////////////////////////////////////////////////
technique SimpleMesh_vs30_ps30
{
	pass P0
	{
		VertexShader = compile vs_2_0 CadModel_VSMain();
		PixelShader  = compile ps_2_0 CadModel_PSMain();
	}
	pass P1
	{
		VertexShader = compile vs_2_0 CadModelWire_VSMain();
		PixelShader  = compile ps_2_0 CadModelWire_PSMain();
	}
}

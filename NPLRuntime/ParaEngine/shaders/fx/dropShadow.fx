

//render a batch of drop shadow
float4x4 mViewProj:viewprojection :register(vs,c0);

//40 instance,each instance use 4 solt for matrix 1 solt for other data
float4 instanceData[210]  :register(vs,c20);


struct VSInput
{
	float4 pos:POSITION;
	float instanceIndex:TEXCOORD0;
};

struct VSOutput
{
	float4 pos :POSITION;
};

VSOutput DropShadow_vsMain(VSInput input)
{
	VSOutput output;
	
	float idx = input.instanceIndex * 3;
	float4 worldPos;
	worldPos.x = dot(input.pos,instanceData[idx]);
	worldPos.y = dot(input.pos,instanceData[idx+1]);
	worldPos.z = dot(input.pos,instanceData[idx+2]);
	worldPos.w = 1;
	output.pos = mul(worldPos,mViewProj);
	return output;
}

float4 DropShadow_psMain():COLOR0
{
	return float4(0.05,0.15,0.2,0.6);
}

////////////////////////////////////////////////////////////////////////////////
//                              Technique
////////////////////////////////////////////////////////////////////////////////
technique SimpleMesh_vs30_ps30
{
	pass P0
	{
		VertexShader = compile vs_2_0 DropShadow_vsMain();
		PixelShader  = compile ps_2_0 DropShadow_psMain();
		FogEnable = false;
	}
}

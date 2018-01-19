// author: LiXizhi
// date: 2005.8
// the flare texture grid
texture tex0 : TEXTURE;

const SCALAR4 posOffset : posScaleOffset;
const SCALAR4 texOffset : uvScaleOffset;
const SCALAR4 flare_color: flareColor;

struct VS_INPUT
{
  SCALAR2	Pos	: POSITION;
  SCALAR2	Tex	: TEXCOORD0;
};

struct VS_OUTPUT
{
    SCALAR4 Pos  : POSITION;
    SCALAR4 vTex0: TEXCOORD0;
    SCALAR4 Diff: COLOR0;
};

VS_OUTPUT VS(const VS_INPUT v)
{
	VS_OUTPUT Out = (VS_OUTPUT)0;

	Out.Pos.xy = (v.Pos.xy*posOffset.xy)+ posOffset.zw;
	Out.Pos.z = 0.5f;
	Out.Pos.w = 1.0f;

	Out.vTex0.xy = v.Tex.xy + texOffset.zw;
	Out.vTex0.z = 0.0f;
	Out.vTex0.w = 1.0f;

	Out.Diff = flare_color;
	return Out;
}

sampler LinearSamp0 = 
sampler_state 
{
    texture = <tex0>;
    AddressU  = wrap;        
    AddressV  = wrap;
    AddressW  = wrap;
    MIPFILTER = LINEAR;
    MINFILTER = LINEAR;
    MAGFILTER = LINEAR;
};

SCALAR4 PS(VS_OUTPUT In) : COLOR
{   
	SCALAR4 color= tex2D(LinearSamp0, In.vTex0 ); 
	return (color * In.Diff);
}  

technique LensFlare_vs20_ps20
{
	pass P0
	{
		// no culling
		CULLMODE = NONE;

		AlphaBlendEnable = true;
		SrcBlend = SrcAlpha;
		DestBlend = one;
		BlendOp = add;
		AlphaTestEnable = false;

		// do not test z,
		ZENABLE = FALSE;
		ZWRITEENABLE = FALSE;

		// shaders
		VertexShader = compile vs_2_0 VS();
		PixelShader  = compile ps_2_0 PS();
	}  
}

technique LensFlare_vs11_ps11
{
	pass P0
	{
		// no culling
		CULLMODE = NONE;

		AlphaBlendEnable = true;
		SrcBlend = SrcAlpha;
		DestBlend = one;
		BlendOp = add;
		AlphaTestEnable = false;

		// do not test z,
		ZENABLE = FALSE;
		ZWRITEENABLE = FALSE;

		// shaders
		VertexShader = compile vs_1_1 VS();
		PixelShader  = compile ps_1_1 PS();
	}  
}

technique LensFlare_vs11_ps0
{
	pass P0
	{
		// no culling
		CULLMODE = NONE;

		AlphaBlendEnable = true;
		SrcBlend = SrcAlpha;
		DestBlend = one;
		BlendOp = add;
		AlphaTestEnable = false;

		// do not test z,
		ZENABLE = FALSE;
		ZWRITEENABLE = FALSE;

		// shaders
		VertexShader = compile vs_1_1 VS();
		PixelShader  = null;
		
		Texture[0] = <tex0>;
		AddressU[0]  = clamp;        
		AddressV[0]  = clamp;
		MinFilter[0] = Linear;
		MagFilter[0] = Linear;
		MipFilter[0] = Linear;

		ColorArg1[0] = Texture;
		ColorArg2[0] = Diffuse;
		ColorOp[0] = Modulate;
		AlphaArg1[0] = Texture;
		AlphaArg2[0] = Diffuse;
		AlphaOp[0] = Modulate;

		ColorOp[1]= disable;
		AlphaOp[1]= disable;
	}  
}


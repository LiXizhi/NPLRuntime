/**
Author: hyz, lxz
Email: 
Date: 2022.10.18
Desc: 
*/

/** define this to show original all 6 original ods textures in the upper region of the screen. */
// #define DEBUG_SHOW_ODS_ORIGINAL_SIX_FACES

float2 screenParam = float2(512, 512);
float num_h = 4;
float num_v = 2;
float4 cubeTextureRect = float4(0,0,1,1);
bool needCompositeUI = false;
float4 dftTextureRect = float4(0,0,0,0);
float ui_fov_v = 1.04;
float ui_fov_h = 1.57;

float _pi = 3.141592653;

texture sourceTexture0;
sampler sourceSp0:register(s0) = sampler_state
{
    Texture = <sourceTexture0>;
    MinFilter = None;
    MagFilter = None;
    AddressU = clamp;
    AddressV = clamp;
};

texture sourceTexture1;
sampler sourceSp1:register(s1) = sampler_state
{
	Texture = <sourceTexture1>;
	MinFilter = None;
	MagFilter = None;
	AddressU = clamp;
	AddressV = clamp;
};

void StereoVS(float3 iPosition:POSITION,
	out float4 oPosition:POSITION,
	inout float2 texCoord:TEXCOORD0)
{
	oPosition = float4(iPosition,1);
	//texCoord += 0.5 / screenParam;
}

float4 StereoPS(float2 texCoord:TEXCOORD0) :COLOR
{
#ifdef DEBUG_SHOW_ODS_ORIGINAL_SIX_FACES
	if (texCoord.y <= 0.5)
	{
		float4 color0 = tex2D(sourceSp0, float2(texCoord.x, texCoord.y));
		color0.a = 1.0;
		return color0;
	}
	clip(0);
#endif
	
	float u = texCoord.x/cubeTextureRect.z;
	float v = texCoord.y/cubeTextureRect.w;
	// float u = texCoord.x;
	// float v = texCoord.y;
	float theta = (u-0.5)*2*_pi; //转成极坐标，水平方向角度
	float phi = (v-0.5)*_pi; //转成极坐标，垂直方向角度

	//转成球面坐标
	float x = cos(phi) * sin(theta); //x正方向向右
	float y = sin(phi); //y正方向向上
	float z = cos(phi) * cos(theta);//z正方向向前

	float scale;
	float2 px;//在每个面的坐标

	int2 portOffset;//用的哪个面

	if (abs(x) >= abs(y) && abs(x) >= abs(z)) {
		if (x < 0.0) {//左面
			scale = -1.0 / x;
			px.x = ( z*scale + 1.0) / 2.0;
			px.y = ( y*scale + 1.0) / 2.0;
			portOffset.x = 3;
			portOffset.y = 0;
		}
		else {//右面
			scale = 1.0 / x;
			px.x = (-z*scale + 1.0) / 2.0;
			px.y = ( y*scale + 1.0) / 2.0;
			portOffset.x = 1;
			portOffset.y = 0;
		}
	}
	else if (abs(y) >= abs(z)) {
		if (y < 0.0) { //上面
			scale = -1.0 / y;
			px.x = ( x*scale + 1.0) / 2.0;
			px.y = ( z*scale + 1.0) / 2.0;
			portOffset.x = 0;
			portOffset.y = 1;
		}
		else { //下面
			scale = 1.0 / y;
			px.x = ( x*scale + 1.0) / 2.0;
			px.y = (-z*scale + 1.0) / 2.0;
			portOffset.x = 1;
			portOffset.y = 1;
		}
	}
	else {
		if (z < 0.0) { //后面
			scale = -1.0 / z;
			px.x = (-x*scale + 1.0) / 2.0;
			px.y = ( y*scale + 1.0) / 2.0;
			portOffset.x = 2;
			portOffset.y = 0;
		}
		else { //前面
			scale = 1.0 / z;
			px.x = ( x*scale + 1.0) / 2.0;
			px.y = ( y*scale + 1.0) / 2.0;
			portOffset.x = 0;
			portOffset.y = 0;
		}
	}

	u = (portOffset.x+px.x)/num_h;
	v = (portOffset.y+px.y)/num_v;

	float4 color0 = tex2D(sourceSp0, float2(u,v*cubeTextureRect.w));

	 if(needCompositeUI){
	 	if(portOffset.x==0&&portOffset.y==0){//正前方
	 		float scale_v = tan(ui_fov_v/2)/tan(1.57/2);
	 		float scale_h = tan(ui_fov_h/2)/tan(1.57/2);
			
	 		float v = (px.y + scale_v*0.5 - 0.5)/(scale_v);
	 		float u = (px.x + scale_h*0.5 - 0.5)/(scale_h);
	 		if(v>=0&&v<=1&&u>=0&&u<=1){
				
	 			u = u*dftTextureRect.z + dftTextureRect.x;
	 			v = v*dftTextureRect.w + dftTextureRect.y;
	 			float4 color1 = tex2D(sourceSp0,float2(u,v));
	 			color0 = color0*(1-color1.a)+color1*color1.a;
	 		}
	 	}else if(portOffset.x==3&&portOffset.y==0){//左边

	 	}else if(portOffset.x==1&&portOffset.y==0){//右边

	 	}
	 }
		
	color0.a = 1.0;
	return color0;
}

float4 StereoPS_debug_ods(float2 texCoord:TEXCOORD0) :COLOR
{
	if (texCoord.y <= 0.75)
	{
		float4 color0 = tex2D(sourceSp0, float2(texCoord.x, texCoord.y));
		color0.a = 1.0;
		return color0;
	}
	clip(0);
	return float4(0,0,0,0);
}

technique Default
{
    pass P0
    {
		cullmode = none;
		ZEnable = false;
		ZWriteEnable = false;
		FogEnable = False;
		VertexShader = compile vs_3_0 StereoVS();
        PixelShader = compile ps_3_0 StereoPS();
    }

	pass P1
	{
		cullmode = none;
		ZEnable = false;
		ZWriteEnable = false;
		FogEnable = False;
		VertexShader = compile vs_3_0 StereoVS();
		PixelShader = compile ps_3_0 StereoPS_debug_ods();
	}

}

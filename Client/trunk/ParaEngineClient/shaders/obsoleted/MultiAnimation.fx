//
// Skinned Mesh Effect file 
// Copyright (c) 2000-2002 Microsoft Corporation. All rights reserved.
//


#include "skin.vsh"


float4 lhtDir       = { 0.0f, 0.0f, -1.0f, 1.0f };                      // light Direction 
float4 lightDiffuse = { 0.6f, 0.6f, 0.6f, 1.0f };                       // Light Diffuse
float4 MaterialAmbient : MATERIALAMBIENT = { 0.1f, 0.1f, 0.1f, 1.0f };
float4 MaterialDiffuse : MATERIALDIFFUSE = { 0.8f, 0.8f, 0.8f, 1.0f };

float4x4    mViewProj : VIEWPROJECTION;

///////////////////////////////////////////////////////
struct VS_INPUT
{
    float4  Pos             : POSITION;
    float3  BlendWeights    : BLENDWEIGHT;
    float4  BlendIndices    : BLENDINDICES;
    float3  Normal          : NORMAL;
    float3  Tex0            : TEXCOORD0;
};

struct VS_OUTPUT
{
    float4  Pos     : POSITION;
    float4  Diffuse : COLOR;
    float2  Tex0    : TEXCOORD0;
};


float3 Diffuse( float3 Normal )
{
    float CosTheta;
    
    // N.L Clamped
    CosTheta = max( 0.0f, dot( Normal, lhtDir.xyz ) );
       
    // propogate scalar result to vector
    return ( CosTheta );
}


VS_OUTPUT VShade( VS_INPUT i, uniform int iNumBones )
{
    VS_OUTPUT   o;
    float3      Pos = 0.0f;
    float3      Normal = 0.0f;    
    float       LastWeight = 0.0f;
    
    // skin VB inputs
    VS_SKIN_INPUT vsi = { i.Pos, i.BlendWeights, i.BlendIndices, i.Normal };
    VS_SKIN_OUTPUT vso = VS_Skin( vsi, iNumBones );
     
    // transform position from world space into view and then projection space
    o.Pos = mul( float4( vso.vPos.xyz, 1.0f ), mViewProj );

    // normalize normals
    Normal = normalize( vso.vNor );

    // Shade (Ambient + etc.)
    o.Diffuse = float4( MaterialAmbient.xyz + Diffuse( Normal ) * MaterialDiffuse.xyz, 1.0 );

    // copy the input texture coordinate through
    o.Tex0  = i.Tex0.xy;

    return o;
}

int CurNumBones = 2;

VertexShader vsArray20[ 4 ] = { compile vs_2_0 VShade( 1 ), 
                                compile vs_2_0 VShade( 2 ),
                                compile vs_2_0 VShade( 3 ),
                                compile vs_2_0 VShade( 4 ) };

VertexShader vsArray11[ 4 ] = { compile vs_1_1 VShade( 1 ), 
                                compile vs_1_1 VShade( 2 ),
                                compile vs_1_1 VShade( 3 ),
                                compile vs_1_1 VShade( 4 ) };

VertexShader vsArray2s[ 4 ] = { compile vs_2_sw VShade( 1 ), 
                                compile vs_2_sw VShade( 2 ),
                                compile vs_2_sw VShade( 3 ),
                                compile vs_2_sw VShade( 4 ) };

//////////////////////////////////////
// Techniques specs follow
//////////////////////////////////////

technique t0
{
    pass p0
    {
        VertexShader = ( vsArray20[ CurNumBones ] );
    }
}

technique t1
{
    pass p0
    {
        VertexShader = ( vsArray11[ CurNumBones ] );
    }
}

technique t2
{
    pass p0
    {
        VertexShader = ( vsArray2s[ CurNumBones ] );
    }
}

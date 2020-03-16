//*********************************************************
//
// Copyright (c) Microsoft. All rights reserved.
// This code is licensed under the MIT License (MIT).
// THIS CODE IS PROVIDED *AS IS* WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING ANY
// IMPLIED WARRANTIES OF FITNESS FOR A PARTICULAR
// PURPOSE, MERCHANTABILITY, OR NON-INFRINGEMENT.
//
//*********************************************************

#include "Common.hlsl"

struct VSInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    float3 tangent : TANGENT;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    float3 tangent : TANGENT;
};

PSInput VSMain(VSInput input)
{
    PSInput result;
        
    float4 worldpos = float4(input.position, 1.0f);
    worldpos = mul(worldpos, gWorld);
    worldpos = mul(worldpos, gViewProj);
        
    result.position = worldpos;
    result.normal = mul(input.normal, (float3x3) gWorld);
    result.uv = input.uv;
    result.tangent = input.tangent;

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{     
    Material mat = gMaterials[gMatIndex];
    uint diffuseIndex = mat.diffuseMapIndex;
    float4 color = float4(mat.diffuseColor, 1.0f);
    
    color *= gTextureMaps[diffuseIndex].Sample(gAnisotropicWrap, input.uv);
   
    input.normal = normalize(input.normal);
    
    int i = 0;
    float3 diffuse = float3(0.0f, 0.0f, 0.0f);
    
    for (i = 0; i < 3; ++i)
    {
        float3 l = gLights[i].direction; // directional light
        float3 d = max(dot(l, input.normal), 0.0f) * gLights[i].strength;
        diffuse += d;
    }
    
    diffuse *= color.xyz;
    
    float4 ambient = gAmbient * color;
    
    return ambient + float4(diffuse, 0.0f);
  
}

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
    float4 color = float4(1.0f, 1.0f, 1.0f, 1.0f);
    input.normal = normalize(input.normal);
    float3 light = normalize(-gLight.direction);    // directional light
    float3 diffuse = saturate(dot(light, input.normal)) * gLight.strength * color.xyz;
    
    float4 ambient = gAmbient * color;
    
    return ambient + float4(diffuse, 0.0f);
  
}

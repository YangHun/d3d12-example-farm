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

    
    /*
    float4 worldPosition = (float4(input.position, 1.0f));
    worldPosition.y += 0.2f;
    
    //result.position = float4(input.position, 1.0f);
    result.position = worldPosition;
    */
    
    float4x4 i = float4x4(
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );
    //result.position = mul(float4(input.position, 1.0f), i);
    
    //float4 worldpos = mul(float4(input.position, 1.0f), gWorld);
    float4 worldpos = float4(input.position, 1.0f);
    //result.position = mul(worldpos, gViewProj);
    
    worldpos = mul(worldpos, gWorld);
    worldpos = mul(worldpos, gViewProj);
    
    
    result.position = worldpos;
    result.normal = input.normal;
    result.uv = input.uv;
    result.tangent = input.tangent;

    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return float4(input.normal, 1.0f);
}

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
};

struct PSInput
{
    float4 position : SV_POSITION;
    float3 localPos : POSITION;
};

PSInput VSMain(VSInput input)
{
    PSInput result;
    
    result.localPos = input.position;
    
    float4 world = float4(input.position, 1.0f);
    world = mul(world, gWorld);
    world.xyz += gEyePos;
    
    // Set z = w so that z/w = 1 (i.e., skydome always on far plane).
    result.position = mul(world, gViewProj).xyww;
   
    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return gCubeMap.Sample(gLinearWrap, input.localPos);
}

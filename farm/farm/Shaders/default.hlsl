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
    float4 shadowPos : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    float3 tangent : TANGENT;
};

PSInput VSMain(VSInput input)
{
    PSInput result;
        
    float4 worldpos = float4(input.position, 1.0f);
    worldpos = mul(worldpos, gWorld);
    
    float4 shadowpos = mul(worldpos, gLightViewProj);
    shadowpos = mul(shadowpos, gNormalizedDevice);
    
    worldpos = mul(worldpos, gViewProj);
        
    result.position = worldpos;
    result.shadowPos = shadowpos;
    result.normal = mul(input.normal, (float3x3) gWorld);
    result.uv = input.uv;
    result.tangent = input.tangent;
    
    return result;
}

float GetShadowFactor(float4 shadowPos)
{
    
    shadowPos.xyz /= shadowPos.w;
    
    // depth in NDC space.
    float depth = shadowPos.w;
    
    uint w, h, m;
    gShadowMap.GetDimensions(0, w, h, m);
    
    // get texel size.
    float dx = 1.0f / (float) w;

    float percentLit = 0.0f;
    const float2 offsets[9] =
    {
        float2(-dx, -dx), float2(0.0f, -dx), float2(dx, -dx),
        float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
        float2(-dx, +dx), float2(0.0f, +dx), float2(dx, +dx)
    };
    
    [unroll]
    for (int i = 0; i < 9; ++i)
    {
        percentLit += gShadowMap.SampleCmpLevelZero(gsamShadow, shadowPos.xy + offsets[i], depth).r;
    }
    
    return percentLit / 9.0f;
}


float4 PSMain(PSInput input) : SV_TARGET
{        
    
    //return float4(0.0f, 0.0f, 0.0f, 1.0f);
    //return float4((gLights[0].direction.xyz), 1.0f);
    
    
    Material mat = gMaterials[gMatIndex];
    uint diffuseIndex = mat.diffuseMapIndex;
    float4 color = float4(mat.diffuseColor, 1.0f);
    
    color *= gTextureMaps[diffuseIndex].Sample(gAnisotropicWrap, input.uv);
   
    input.normal = normalize(input.normal);
    
    int i = 0;
    float3 diffuse = float3(0.0f, 0.0f, 0.0f);
    
    
    // todo: light dir, shadow Map value
    
    float shadowFactor = 1.0f;
    //float shadowFactor = GetShadowFactor(input.shadowPos);
    
    
    for (i = 0; i < 1; ++i)
    {
        //float3 l = -float3(-0.57735f, -0.57735f, 0.57735f);
        float3 l = -gLights[i].direction; // directional light
        float3 d = max(dot(l, input.normal), 0.0f) * gLights[i].strength;
        if (i == 0)
            d *= shadowFactor;
        diffuse += d;
    }
    
    diffuse *= color.xyz;
    
    float4 ambient = gAmbient * color;
    
    return ambient + float4(diffuse, 0.0f);
  
}

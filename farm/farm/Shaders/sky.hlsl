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

PSInput VSMain(VSInput input, uint instanceID : SV_InstanceID)
{
    PSInput result;
    
    
    result.localPos = input.position;
    
    Instance inst = gInstances[instanceID];
    float4x4 world = inst.World;
    
    float4 pos = float4(input.position, 1.0f);
    pos = mul(pos, world);
    pos.xyz += gEyePos;
    
    // Set z = w so that z/w = 1 (i.e., skydome always on far plane).
    result.position = mul(pos, gViewProj).xyww;
   
    return result;
}

float4 PSMain(PSInput input) : SV_TARGET
{
    return gCubeMap.Sample(gLinearWrap, input.localPos);
}

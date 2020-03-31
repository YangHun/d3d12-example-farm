// Include common HLSL code.
#include "Common.hlsl"

struct VSInput
{
    float3 position : POSITION;
    float2 uv : TEXCOORD;
};

struct PSInput
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD;
};

PSInput VSMain(VSInput input, uint instanceID : SV_InstanceID)
{
    PSInput result;
    
    Instance inst = gInstances[instanceID];
    float4x4 world = inst.World;
    
    // generate shadow map.
    float4 pos = mul(float4(input.position, 1.0f), world);
    pos = mul(pos, gLightViewProj);
    result.position = pos;

    result.uv = input.uv;
    
    return result;
}

void PSMain(PSInput input)
{
   // first shadow pass does not determine the fragment's color.  
}

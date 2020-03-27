// Include common HLSL code.
#include "Common.hlsl"

struct VSInput
{
    float3 position : POSITION;
};

struct PSInput
{
    float4 position : SV_POSITION;  // homogeneous clip space position
};

PSInput VSMain(VSInput input, uint instanceID : SV_InstanceID)
{
    PSInput result;
    
    Instance inst = gInstances[instanceID];
    float4x4 world = inst.World;
         
    result.position = mul(mul(float4(input.position, 1.0f), world), gViewProj);
    
    return result;
}

float4 PSMain(PSInput input) : SV_Target
{
    return float4(0.0f, 1.0f, 0.0f, 1.0f);   // always set green color

}

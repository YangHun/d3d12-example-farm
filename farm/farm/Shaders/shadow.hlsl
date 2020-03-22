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

PSInput VSMain(VSInput input)
{
    PSInput result;
    
    // generate shadow map.
    float pos = float4(input.position, 1.0f);
    pos = mul(pos, gWorld);
    pos = mul(pos, gLightViewProj);
    result.position = pos;

    return result;
}

void PSMain()
{
   // first shadow pass does not determine the fragment's color.  
}

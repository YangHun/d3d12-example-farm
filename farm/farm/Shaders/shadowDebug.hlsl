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
    
    result.position = float4(input.position, 1.0f);
    result.uv = input.uv;
    
    return result;
}

float4 PSMain(PSInput input) : SV_Target
{
    //return float4(1.0f, 1.0f, 0.0f, 1.0f);
    //return gTextureMaps[7].Sample(gAnisotropicWrap, input.uv);
    return gShadowMap.Sample(gAnisotropicWrap, input.uv);

}

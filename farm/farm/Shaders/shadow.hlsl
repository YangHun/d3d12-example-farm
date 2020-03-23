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
    float4 pos = float4(input.position, 1.0f);
    pos = mul(pos, gWorld);
    pos = mul(pos, gLightViewProj);
    result.position = pos;

    float4 uv = float4(input.uv, 0.0f, 1.0f);
    uv = mul(uv, gLightViewProj);
    uv = mul(uv, gNormalizedDevice);
    
    result.uv = uv.xy;
    
    return result;
}

void PSMain(PSInput input)
{
    Material mat = gMaterials[gMatIndex];
    uint diffuseIndex = mat.diffuseMapIndex;
    float4 diffuseAlbedo = float4(mat.diffuseColor, 1.0f);
	
	// Dynamically look up the texture in the array.
    diffuseAlbedo *= gTextureMaps[diffuseIndex].Sample(gAnisotropicWrap, input.uv);
       
   // first shadow pass does not determine the fragment's color.  
    
}

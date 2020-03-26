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
    //pos = mul(pos, gNormalizedDevice);
    result.position = pos;

    result.uv = input.uv;
    
    return result;
}

void PSMain(PSInput input)
{
 //   Material mat = gMaterials[gMatIndex];
 //   uint diffuseIndex = mat.diffuseMapIndex;
 //   float4 diffuseAlbedo = float4(mat.diffuseColor, 1.0f);
	
	//// Dynamically look up the texture in the array.
 //   diffuseAlbedo *= gTextureMaps[diffuseIndex].Sample(gAnisotropicWrap, input.uv);
       
   // first shadow pass does not determine the fragment's color.  
    
}

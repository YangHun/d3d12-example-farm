// Constant data that varies per frame.

struct Light
{
    float4 strength;
    float4 direction; // directional light
};

struct Material
{
    float4x4 textureTile;
    float3 diffuseColor;
    float pad0;
    uint diffuseMapIndex;
};

struct Instance
{
    float4x4 World;
    uint MatIndex;
    uint pad0;
    uint pad1;
    uint pad2;
};

cbuffer cbScene : register(b0)
{
    float4x4 gViewProj;
    float4 gAmbient;
    float4 gEyePos;
    float4 gFogColor;
    
    float gFogStartDistance;
    float gFogRange;
    float2 pad0;
    
    Light gLights[3];
}

cbuffer cbShadow : register(b1)
{
    float4x4 gLightViewProj;   
    float4x4 gNormalizedDevice;
}

TextureCube gCubeMap : register(t0);
Texture2D gShadowMap : register(t1);
Texture2D gTextureMaps[48] : register(t2);  // space 0

StructuredBuffer<Instance> gInstances : register(t0, space1);
StructuredBuffer<Material> gMaterials : register(t1, space1);

SamplerState gAnisotropicWrap : register(s0);
SamplerState gLinearWrap : register(s1);
SamplerComparisonState gsamShadow : register(s2);
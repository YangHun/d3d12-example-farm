// Constant data that varies per frame.

struct Light
{
    float3 strength;
    float3 direction; // directional light
};

struct Material
{
    uint diffuseMapIndex;
    float3 diffuseColor;
};

cbuffer cbPerObject : register(b0)
{
    float4x4 gWorld;
    uint gMatIndex;
};

cbuffer cbScene : register(b1)
{
    float4x4 gViewProj;
    float4 gAmbient;
    float3 gEyePos;
    Light gLights[16];
}

TextureCube gCubeMap : register(t0);

Texture2D gTextureMaps[48] : register(t1);  // space 0
StructuredBuffer<Material> gMaterials : register(t0, space1);

SamplerState gAnisotropicWrap : register(s0);
SamplerState gLinearWrap : register(s1);
// Constant data that varies per frame.

struct Light
{
    float3 strength;
    float3 direction; // directional light
};

struct Material
{
    uint diffuseMapIndex;
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
    Light gLight;
}

Texture2D gTextureMaps[48] : register(t0);  // space 0
StructuredBuffer<Material> gMaterials : register(t0, space1);

SamplerState gAnisotropicWrap : register(s0);
//SamplerState gsamLinearWrap : register(s0);
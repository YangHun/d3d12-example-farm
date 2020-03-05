// Constant data that varies per frame.

struct Light
{
    float3 strength;
    float3 direction; // directional light
};


cbuffer cbPerObject : register(b0)
{
    float4x4 gWorld;
};

cbuffer cbScene : register(b1)
{
    float4x4 gViewProj;
    float4 gAmbient;
    float3 gEyePos;
    Light gLight;
}

TextureCube gCubeMap : register(t0);
//Texture2D gTextureMaps[48] : register(t1);

//StructuredBuffer<MaterialData> gMaterialData : register(t0, space1);

SamplerState gsamLinearWrap : register(s2);
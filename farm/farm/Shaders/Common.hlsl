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
    Light gLight;
}
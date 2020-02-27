// Constant data that varies per frame.
cbuffer cbPerObject : register(b0)
{
    float4x4 gWorld;
};

cbuffer cbScene : register(b1)
{
    float4x4 gViewProj;
}
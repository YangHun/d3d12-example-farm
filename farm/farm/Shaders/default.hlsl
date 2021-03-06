#include "Common.hlsl"

struct VSInput
{
    float3 position : POSITION;
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    float3 tangent : TANGENT;
};

struct PSInput
{
    float4 position : SV_POSITION;  // homogeneous clip space position
    float4 shadowPos : POSITION0;   // light-projected NDC space position
    float3 worldPos : POSITION1;    // world space position
    float3 normal : NORMAL;
    float2 uv : TEXCOORD;
    float3 tangent : TANGENT;
    
    nointerpolation uint matIndex : MATINDEX;
};

PSInput VSMain(VSInput input, uint instanceID : SV_InstanceID)
{
    PSInput result;
     
    Instance inst = gInstances[instanceID];
    float4x4 world = inst.World;
    result.matIndex = inst.MatIndex;
    
    float4 worldpos = mul(float4(input.position, 1.0f), world);
    result.worldPos = worldpos;
    
    float4 shadowpos = mul(worldpos, gLightViewProj);
    shadowpos = mul(shadowpos, gNormalizedDevice);
    result.shadowPos = shadowpos;
     
    float4x4 tile = gMaterials[result.matIndex].textureTile;
    
    result.position = mul(worldpos, gViewProj);
    result.normal = mul(float4(input.normal, 0.0f), world).xyz;
    result.uv = mul(float4(input.uv, 0.0f, 1.0f), tile).xy;
    result.tangent = input.tangent;
    
    return result;
}

float GetShadowFactor(float4 shadowPos)
{
    // PCF kernel   
    shadowPos.xyz /= shadowPos.w;
    
    // depth in NDC space.
    float depth = shadowPos.z;
    
    uint w, h, m;
    gShadowMap.GetDimensions(0, w, h, m);
    
    // get texel size.
    float dx = 1.0f / (float) w;

    float percentLit = 0.0f;
    const float2 offsets[9] =
    {
        float2(-dx, -dx), float2(0.0f, -dx), float2(dx, -dx),
        float2(-dx, 0.0f), float2(0.0f, 0.0f), float2(dx, 0.0f),
        float2(-dx, +dx), float2(0.0f, +dx), float2(dx, +dx)
    };
    
    [unroll]
    for (int i = 0; i < 9; ++i)
    {
        percentLit += gShadowMap.SampleCmpLevelZero(gsamShadow, shadowPos.xy + offsets[i], depth).r;
    }
    
    return percentLit / 9.0f;
}


float4 PSMain(PSInput input) : SV_TARGET
{       
    
    Material mat = gMaterials[input.matIndex];
    uint diffuseIndex = mat.diffuseMapIndex;
    float4 color = float4(mat.diffuseColor, 1.0f);
    
    color *= gTextureMaps[diffuseIndex].Sample(gAnisotropicWrap, input.uv);
   
    input.normal = normalize(input.normal);
    
    int i = 0;
    float3 diffuse = float3(0.0f, 0.0f, 0.0f);
  
    // 0번째 light에 대해서만 shadow map을 그렸으므로, 나머지 light는 shadow factor 적용 X
    float3 shadowFactor = float3(GetShadowFactor(input.shadowPos), 1.0f, 1.0f);

    for (i = 0; i < 3; ++i)
    {
        float3 l = -gLights[i].direction.xyz; // directional light
        float3 d = max(dot(l, input.normal), 0.0f) * gLights[i].strength.xyz;
        d *= shadowFactor[i];
        diffuse += d;
    }
    
    diffuse *= color.xyz;
    
    
    float4 light = gAmbient * color + float4(diffuse, color.a);
       
    float distToEye = length(gEyePos.xyz - input.worldPos.xyz);
    float fogAmount = saturate((distToEye - gFogStartDistance) / gFogRange);
    
    light = lerp(light, gFogColor, fogAmount);
    light.a = color.a;
    
    return light;
  
}

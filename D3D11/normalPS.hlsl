#include "ShaderIncludes.hlsli"
#define MAX_LIGHTS 5

cbuffer ExternalData : register(b0)
{
    float4 colorTint;
    float3 roughness;
    float3 cameraPosition;
    float3 ambientColor;
    Light lights[MAX_LIGHTS];
    float2 uvScale;
    float2 uvOffset;
    int isSpecular;
    
    float farClip;
    int fogType;
    float3 fogColor;
    float fogStartDist;
    float fogEndDist;
    float fogDensity;
    int heightBasedFog;
    float fogHeight;
}

//define textures
Texture2D Albedo : register(t0);
Texture2D NormalMap : register(t1);
Texture2D RoughnessMap : register(t2);
Texture2D MetalnessMap : register(t3);
Texture2D ShadowMap : register(t4);

SamplerState BasicSampler : register(s0);
SamplerComparisonState ShadowSampler : register(s1);

// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
float4 main(VertexToPixel input) : SV_TARGET
{
    input.normal = normalize(input.normal);
    input.tangent = normalize(input.tangent);
    input.uv = input.uv * uvScale + uvOffset;
    
    //normal map
    float3 unpackedNormal = NormalMap.Sample(BasicSampler, input.uv).rgb * 2 - 1;
    unpackedNormal = normalize(unpackedNormal);
    
    float3 N = normalize(input.normal);
    float3 T = normalize(input.tangent);
    T = normalize(T - N * dot(T, N));
    float3 B = cross(T, N);
    float3x3 TBN = float3x3(T, B, N);
    input.normal = normalize(mul(unpackedNormal, TBN));
    
    //roughness map
    float roughness = RoughnessMap.Sample(BasicSampler, input.uv).r;
    
    //specular
    float metalness = MetalnessMap.Sample(BasicSampler, input.uv).r;
    
    //color map
    float3 surfaceColor = pow(Albedo.Sample(BasicSampler, input.uv).rgb, 2.2f);
    surfaceColor = surfaceColor * (float3) colorTint;
    

    // Specular color determination -----------------
// Assume albedo texture is actually holding specular color where metalness == 1
// Note the use of lerp here - metal is generally 0 or 1, but might be in between
// because of linear texture sampling, so we lerp the specular color to match
    float3 specularColor = lerp(F0_NON_METAL, surfaceColor.rgb, metalness);
  
    //shadow map
    // Perform the perspective divide (divide by W) ourselves
    input.shadowMapPos /= input.shadowMapPos.w;
    // Convert the normalized device coordinates to UVs for sampling
    float2 shadowUV = input.shadowMapPos.xy * 0.5f + 0.5f;
    shadowUV.y = 1 - shadowUV.y; // Flip the Y
    // Grab the distances we need: light-to-pixel and closest-surface
    float depthFromLight = input.shadowMapPos.z; // / input.shadowMapPos.w;
    float shadowAmount = ShadowMap.SampleCmpLevelZero(ShadowSampler, shadowUV, depthFromLight); // For testing, just return black where there are shadows.

    float3 finalColor = 0;
    
    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        switch (lights[i].Type)
        {
            case LIGHT_TYPE_DIRECTIONAL:
                float3 lightResult = DirLightPBR(lights[i], input.normal, input.worldPosition, cameraPosition, roughness, metalness, surfaceColor, specularColor);

                if (i == 0)
                {
                    lightResult *= shadowAmount;
                }
                finalColor += lightResult;
                break;
            case LIGHT_TYPE_POINT:
                finalColor += PointLightPBR(lights[i], input.normal, input.worldPosition, cameraPosition, roughness, metalness, surfaceColor, specularColor);
                break;
            case LIGHT_TYPE_SPOT:
                break;
        }
    }
    float fog = 0;
    float surfaceToCamera = distance(cameraPosition, input.worldPosition);
    switch (fogType)
    {
        case 0: //linear fog
            fog = surfaceToCamera / farClip;
            break;
        case 1: //smoothstep fog
            fog = smoothstep(fogStartDist, fogEndDist, surfaceToCamera);
            break;
        case 2: //exponential fog
            fog = 1 - exp(-surfaceToCamera * fogDensity);
            break;
        default:
            break;
    }
    if (heightBasedFog)
    {
        fog *= smoothstep(fogHeight, 0, input.worldPosition.y);
    }
    fog = saturate(fog);
    finalColor = lerp(finalColor, fogColor, fog);
        
    return float4(pow(finalColor, 1.0f / 2.2f), 1);
}
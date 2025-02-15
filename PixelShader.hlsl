#include "ShaderIncludes.hlsli"
cbuffer ExternalData : register(b0)
{
    float2 uvScale;
    float2 uvOffset;
    float3 cameraPosition;
    int lightCount;
    Light lights[MAX_LIGHTS];
}
// Struct representing the data we expect to receive from earlier pipeline stages
// - Should match the output of our corresponding vertex shader
// - The name of the struct itself is unimportant
// - The variable names don't have to match other shaders (just the semantics)
// - Each variable must have a semantic, which defines its usage
struct VertexToPixel
{
	// Data type
	//  |
	//  |   Name          Semantic
	//  |    |                |
	//  v    v                v
    float4 screenPosition : SV_POSITION;
    float2 uv : TEXCOORD;
    float3 normal : NORMAL;
    float3 tangent : TANGENT;
    float3 worldPosition : POSITION;
};

Texture2D AlbedoTexture : register(t0);
Texture2D NormalMap : register(t1);
Texture2D RoughnessMap : register(t2);
Texture2D MetalMap : register(t3);
SamplerState BasicSampler : register(s0);

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
    
    //color map
    float3 surfaceColor = pow(AlbedoTexture.Sample(BasicSampler, input.uv).rgb, 2.2f);
    
    //roughness map
    float roughness = RoughnessMap.Sample(BasicSampler, input.uv).r;
    
    //specular
    float metalness = MetalMap.Sample(BasicSampler, input.uv).r;
    
    // Specular color determination -----------------
// Assume albedo texture is actually holding specular color where metalness == 1
// Note the use of lerp here - metal is generally 0 or 1, but might be in between
// because of linear texture sampling, so we lerp the specular color to match
    float3 specularColor = lerp(F0_NON_METAL, surfaceColor.rgb, metalness);
  
    float3 finalColor = 0;
    
    for (int i = 0; i < lightCount; i++)
    {
        //Light light = lights[i];
        //light.Direction = normalize(light.Direction);
        
        switch (lights[i].Type)
        {
            case LIGHT_TYPE_DIRECTIONAL:
                finalColor += DirLightPBR(lights[i], input.normal, input.worldPosition, cameraPosition, roughness, metalness, surfaceColor, specularColor);
                break;
            case LIGHT_TYPE_POINT:
                finalColor += PointLightPBR(lights[i], input.normal, input.worldPosition, cameraPosition, roughness, metalness, surfaceColor, specularColor);
                break;
            case LIGHT_TYPE_SPOT:
                break;
            default:
                finalColor = float3(0, 1, 0);
                break;
        }
    }
    //return float4(surfaceColor.xyz, 1);
    return float4(pow(finalColor, 1.0f / 2.2f), 1);

}
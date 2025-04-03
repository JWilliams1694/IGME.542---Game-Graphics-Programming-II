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
}

//define textures
Texture2D SurfaceTexture : register(t0);
Texture2D SpecularTexture : register(t1);

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
    input.uv=input.uv * uvScale + uvOffset;
    float3 surfaceColor = pow(SurfaceTexture.Sample(BasicSampler, input.uv).rgb, 2.2f);
    surfaceColor = surfaceColor * (float3) colorTint;
    float specularScale = SpecularTexture.Sample(BasicSampler, input.uv).r;
    float3 finalColor = ambientColor * surfaceColor;
    
    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        float3 lightDir = normalize(-lights[i].Direction);
        float3 viewDir = normalize(cameraPosition - input.worldPosition);
        float3 diffuse = Diffuse(input.normal, lightDir, lights[i].Color, lights[i].Intensity);
        float3 specular = Specular(input.normal, lightDir, viewDir, lights[i].Color, roughness.x);
        
        switch (lights[i].Type)
        {
            case LIGHT_TYPE_DIRECTIONAL:
                lightDir = normalize(-lights[i].Direction);
                diffuse = Diffuse(input.normal, lightDir, lights[i].Color, lights[i].Intensity);
                specular = Specular(input.normal, lightDir, viewDir, lights[i].Color, roughness.x) * specularScale;
                break;
            case LIGHT_TYPE_POINT:
                lightDir = normalize(lights[i].Position - input.worldPosition);
                float attenuation = Attenuate(lights[i], input.worldPosition);

                diffuse = Diffuse(input.normal, lightDir, lights[i].Color, lights[i].Intensity) * attenuation;
                specular = Specular(input.normal, lightDir, viewDir, lights[i].Color, roughness.x) * attenuation * specularScale;
                break;
            case LIGHT_TYPE_SPOT:
                break;
        }
        finalColor += (diffuse * surfaceColor + specular);
    }
    return float4(pow(finalColor, 1.0f / 2.2f), 1);
}
#include "ShaderStructs.hlsli"
cbuffer externalData : register(b0)
{
    float3 colorTint;
};

Texture2D Particle : register(t0);
SamplerState BasicSampler : register(s0);

float4 main(VertexToPixel_Particle input) : SV_TARGET
{
    float4 color = Particle.Sample(BasicSampler, input.uv) * input.color;
    color.rgb *= colorTint;

    return lerp(color, float4(1, 1, 1, 0.5f), 1);
}
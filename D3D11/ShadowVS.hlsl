#include "ShaderIncludes.hlsli"
cbuffer DataFromCPU : register(b0)
{
    matrix worldMatrix;
    matrix viewMatrix;
    matrix projectionMatrix;
}

// --------------------------------------------------------
// A simplified vertex shader for rendering to a shadow map
// --------------------------------------------------------
float4 main(VertexShaderInput input) : SV_POSITION
{
    matrix wvp = mul(projectionMatrix, mul(viewMatrix, worldMatrix));
    return mul(wvp, float4(input.localPosition, 1.0f));
}


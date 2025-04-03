#include "ShaderIncludes.hlsli"
cbuffer ExternalData : register(b0)
{
    float totalTime; // Time passed, sent from the C++ code
} 

// --------------------------------------------------------
// The entry point (main method) for our pixel shader
// 
// - Input is the data coming down the pipeline (defined by the struct)
// - Output is a single color (float4)
// - Has a special semantic (SV_TARGET), which means 
//    "put the output of this into the current render target"
// - Named "main" because that's the default the shader compiler looks for
// --------------------------------------------------------
float4 main(float4 position : SV_POSITION, float2 uv : TEXCOORD) : SV_TARGET
{

    float wave = sin(uv.x * 10.0 + totalTime * 2.0); 


    float r = 0.5 + 0.5 * wave; 
    float g = 0.5 + 0.5 * wave;
    float b = 0.8; 

    return float4(r, g, b, 1); 
}
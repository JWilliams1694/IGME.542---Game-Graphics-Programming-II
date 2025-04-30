#include "ShaderStructs.hlsli"
#include "Lighting.hlsli"
cbuffer externalData : register(b0)
{
    float screenWidth;
    float screenHeight;
    int isRefractive;
    float refractionScale;
    float3 cameraPosition;
};

Texture2D NormalMap : register(t0);
Texture2D ScreenPixels : register(t1);
TextureCube EnvironmentMap : register(t2);
SamplerState BasicSampler : register(s0);
SamplerState ClampSampler : register(s1);

// Fresnel term - Schlick approx.
float SimpleFresnel(float3 n, float3 v, float f0)
{
// Pre-calculation
    float NdotV = saturate(dot(n, v));
// Final value
    return f0 + (1 - f0) * pow(1 - NdotV, 5);
}

//float4 main(VertexToPixel input) : SV_TARGET
//{
//// The actual screen UV of this pixel
//// “screenSize” is float2 from C++ holding (windowWidth, windowHeight)
//    float2 screenUV = input.screenPosition.xy / float2(screenWidth, screenHeight);
//// Use object’s normal map as an offset
//// Not physically accurate, but plausibly cool enough!
//    float2 offsetUV = NormalMap.Sample(BasicSampler, input.uv).xy * 2 - 1;
//    offsetUV.y *= -1; // UV's are upside down compared to world space
//// Distort the screen UV by the offset, scaling as necessary
//    float2 refractedUV = screenUV + offsetUV * refractionScale;
//    return ScreenPixels.Sample(ClampSampler, refractedUV);
//    float3 sceneColor = pow(ScreenPixels.Sample(ClampSampler, refractedUV).rgb, 2.2f);
//    float3 viewToCam = normalize(cameraPosition - input.worldPos);
//    float3 viewRefl = normalize(reflect(-viewToCam, input.normal));
//    float3 envSample = EnvironmentMap.Sample(BasicSampler, viewRefl).rgb;
//    
//    // Determine the reflectivity based on viewing angle
//// using the Schlick approximation of the Fresnel term
//    float fresnel = SimpleFresnel(
//input.normal,
//viewToCam,
//F0_NON_METAL);
//
//// May need to un-gamma correct texture sample, and
//// re-gamma correct result here since this is a linear
//// interpolation (should be done in linear color space)
//    return float4(lerp(sceneColor, envSample, fresnel), 1);
//}
float4 main(VertexToPixel input) : SV_TARGET
{
	// Clean up un-normalized normals
    input.normal = normalize(input.normal);
    input.tangent = normalize(input.tangent);

	// Adjust uv scaling


	// Use normal mapping
    input.normal = NormalMapping(NormalMap, BasicSampler, input.uv, input.normal, input.tangent);

	// Using normal map as "displacement"
    float2 offsetUV = NormalMap.Sample(BasicSampler, input.uv).xy * 2 - 1;
    offsetUV.y *= -1;
	
	// Calculate screen UV
    float2 screenUV = input.screenPosition.xy / float2(screenWidth, screenHeight);
	
	// Final refracted UV (where to pull the pixel color)
	// If not using refraction at all, just use the screen UV itself
    float2 refractedUV = screenUV + offsetUV * refractionScale;


	// Get the color at the (now verified) offset UV
    float3 sceneColor = pow(ScreenPixels.Sample(ClampSampler, refractedUV).rgb, 2.2f); // Un-gamma correct

	// Get reflections
    float3 viewToCam = normalize(cameraPosition - input.worldPos);
    float3 viewRefl = normalize(reflect(-viewToCam, input.normal));
    float3 envSample = EnvironmentMap.Sample(BasicSampler, viewRefl).rgb;

	// Determine the reflectivity based on viewing angle
	// using the Schlick approximation of the Fresnel term
    float fresnel = SimpleFresnel(input.normal, viewToCam, F0_NON_METAL);
    return float4(pow(lerp(sceneColor, envSample, fresnel), 1.0f / 2.2f), 1); // Re-gamma correct after linear interpolation
}
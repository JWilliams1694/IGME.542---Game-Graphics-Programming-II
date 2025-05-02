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
TextureCube EnvironmentMap : register(t1);
Texture2D ScreenPixels : register(t2);
Texture2D Silhouette : register(t3);
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

float4 main(VertexToPixel input) : SV_TARGET
{
	// The actual screen UV of this pixel
	// “screenSize” is float2 from C++ holding (windowWidth, windowHeight)
		float2 screenUV = input.screenPosition.xy / float2(screenWidth, screenHeight);

// Use object’s normal map as an offset
// Not physically accurate, but plausibly cool enough!
	float2 offsetUV = NormalMap.Sample(BasicSampler, input.uv).xy * 2 - 1;
	offsetUV.y *= -1; // UV's are upside down compared to world space
	// Distort the screen UV by the offset, scaling as necessary
		float2 refractedUV = screenUV + offsetUV * refractionScale;

		//deal with refractiions in front of the object
		float silhouette = Silhouette.Sample(ClampSampler, refractedUV).r;
		if (silhouette < 1.0f)
		{
			refractedUV = screenUV;
		}
		float3 sceneColor = pow(ScreenPixels.Sample(ClampSampler, refractedUV).rgb, 2.2f);
		float3 viewToCam = normalize(cameraPosition - input.worldPos);
		float3 viewRefl = normalize(reflect(-viewToCam, input.normal));
		float3 envSample = EnvironmentMap.Sample(BasicSampler, viewRefl).rgb;

		// Determine the reflectivity based on viewing angle
	// using the Schlick approximation of the Fresnel term
		float fresnel = SimpleFresnel(
	input.normal,
	viewToCam,
	F0_NON_METAL);
		return float4(1, 1, 1, 1);
		// May need to un-gamma correct texture sample, and
		// re-gamma correct result here since this is a linear
		// interpolation (should be done in linear color space)
			return float4(lerp(sceneColor, envSample, fresnel), 1);
}
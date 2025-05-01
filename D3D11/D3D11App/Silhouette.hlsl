struct VertexToPixel
{
    float4 position : SV_POSITION;
    float2 uv : TEXCOORD0;
};
Texture2D PixelColors : register(t0);

float4 main(VertexToPixel input) : SV_TARGET
{
    //returns solid color from silhouette (black or white)
    return PixelColors.Load(float3(input.position.xy, 0));
}
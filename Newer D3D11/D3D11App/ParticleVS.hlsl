struct Particle
{
	float EmitTime;
	float3 StartPos;
	float3 StartVelocity;
	float StartRotation;
	float EndRotation;
	float3 padding;
};

cbuffer externalData : register(b0)
{
	matrix view;
	matrix projection;

	float4 startColor;
	float4 endColor;

	float currentTime;
	float3 accel;

	int spriteSheetWidth;
	int spriteSheetHeight;
	float spriteSheetFrameWidth;
	float spriteSheetFrameHeight;
	float spriteSheetSpeedScale;

	float startSize;
	float endSize;
	float lifetime;
};

struct VertexToPixel
{
	float4 position : SV_POSITION;
	float2 uv : TEXCOORD0;
	float4 colorTint : COLOR;
};

// Buffer of particle data
StructuredBuffer<Particle> ParticleData : register(t0);

VertexToPixel main(uint id : SV_VertexID)
{
	VertexToPixel output;

	uint particleID = id / 4; // Every group of 4 verts are ONE particle! (int division)
	uint cornerID = id % 4; // 0,1,2,3 = which corner of the particle’s "quad"
	Particle p = ParticleData.Load(particleID); // Each vertex gets associated particle!

	float age = currentTime - p.EmitTime; // currentTime is from C++
	float agePercent = age / lifetime;

	//Offset from current position based on age
	float3 pos = accel * age * age / 2.0f + p.StartVelocity * age + p.StartPos;

	//calculate size
	float size = lerp(startSize, endSize, agePercent);

	// Offsets for the 4 corners of a quad - we'll only use one for each
// vertex, but which one depends on the cornerID
	float2 offsets[4];
	offsets[0] = float2(-1.0f, +1.0f); // TL
	offsets[1] = float2(+1.0f, +1.0f); // TR
	offsets[2] = float2(+1.0f, -1.0f); // BR
	offsets[3] = float2(-1.0f, -1.0f); // BL

	//rotation stuff
	float s, c, rotation = lerp(p.StartRotation, p.EndRotation, agePercent);
	sincos(rotation, s, c);
	float2x2 rot =
	{
		c, s,
		-s, c
	};
	float2 rotatedOffset = mul(offsets[cornerID], rot) * size;

	// Billboarding!
// Offset the position based on the camera's right and up vectors
	pos += float3(view._11, view._12, view._13) * offsets[cornerID].x; // RIGHT
	pos += float3(view._21, view._22, view._23) * offsets[cornerID].y; // UP

	matrix viewProj = mul(projection, view);
	output.position = mul(viewProj, float4(pos, 1.0f));


	//animation stuff from chris demo
	float animPercent = fmod(agePercent * spriteSheetSpeedScale, 1.0f);
	uint ssIndex = (uint)floor(animPercent * (spriteSheetWidth * spriteSheetHeight));

	// Get the U/V indices (basically column & row index across the sprite sheet)
	uint uIndex = ssIndex % spriteSheetWidth;
	uint vIndex = ssIndex / spriteSheetWidth; // Integer division is important here!

	// Convert to a top-left corner in uv space (0-1)
	float u = uIndex / (float)spriteSheetWidth;
	float v = vIndex / (float)spriteSheetHeight;

	float2 uvs[4];
	uvs[0] = float2(u, v); // TL
	uvs[1] = float2(u + spriteSheetFrameWidth, v); // TR
	uvs[2] = float2(u + spriteSheetFrameWidth, v + spriteSheetFrameHeight); // BR
	uvs[3] = float2(u, v + spriteSheetFrameHeight); // BL

	output.uv = uvs[cornerID];
	output.colorTint = lerp(startColor, endColor, agePercent);

	return output;
}
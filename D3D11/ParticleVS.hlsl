cbuffer externalData : register(b0)
{
    matrix view;
    matrix projection;
    float currentTime;
    Particle particles[MAX_PARTICLES];
};

struct Particle
{
    float EmitTime;
    float3 StartPosition;
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
    
    return pos;
}
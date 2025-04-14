#include "Emitter.h"
#include "Graphics.h"

Emitter::Emitter(int maxParticles,
	int particlesPerSec,
	int maxLifetime,
	float startSize,
	float endSize,
	DirectX::XMFLOAT4 startColor,
	DirectX::XMFLOAT4 endColor,
	DirectX::XMFLOAT3 startVelocity,
	DirectX::XMFLOAT3 emitterPosition,
	std::shared_ptr<Material> material) :
	maxParticles(maxParticles),
	particlesPerSec(particlesPerSec),
	maxLifetime(maxLifetime),
	startSize(startSize),
	endSize(endSize),
	startColor(startColor),
	endColor(endColor),
	startVelocity(startVelocity),
	material(material)
{
	transform = std::make_shared<Transform>();
	transform->SetPosition(emitterPosition);

	CreateParticleBuffer();
}

Emitter::~Emitter()
{
	delete[] particles;
}

void Emitter::Update(float dt, float currentTime)
{
	lastEmit += dt;
	while (lastEmit > emitTimer)
	{
		EmitParticle(currentTime);
		lastEmit -= emitTimer;
	}

}

void Emitter::Draw(std::shared_ptr<Camera> camera, float currentTime)
{
}

void Emitter::CreateParticleBuffer()
{
	unsigned int* indices = new unsigned int[maxParticles * 6];
	int indexCount = 0;
	for (int i = 0; i < maxParticles * 4; i += 4)
	{
		indices[indexCount++] = i;
		indices[indexCount++] = i + 1;
		indices[indexCount++] = i + 2;
		indices[indexCount++] = i;
		indices[indexCount++] = i + 2;
		indices[indexCount++] = i + 3;
	}

	// Make a dynamic buffer to hold all particle data on GPU
		// Note: We'll be overwriting this every frame with new lifetime data
	D3D11_BUFFER_DESC desc = {};
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.MiscFlags = D3D11_RESOURCE_MISC_BUFFER_STRUCTURED;
	desc.StructureByteStride = sizeof(Particle);
	desc.ByteWidth = sizeof(Particle) * maxParticles;
	Graphics::Device->CreateBuffer(&desc, 0, particleDataBuffer.GetAddressOf());
	// Create an SRV that points to a structured buffer of particles
	// so we can grab this data in a vertex shader
	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_BUFFER;
	srvDesc.Format = DXGI_FORMAT_UNKNOWN;
	srvDesc.Buffer.FirstElement = 0;
	srvDesc.Buffer.NumElements = maxParticles;
	Graphics::Device->CreateShaderResourceView(particleDataBuffer.Get(), &srvDesc, particleDataSRV.GetAddressOf());

}

void Emitter::CopyToGPU()
{
	// Map the buffer, locking it on the GPU so we can write to it
	D3D11_MAPPED_SUBRESOURCE mapped = {};
	Graphics::Context->Map(particleDataBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mapped);
	// How are living particles arranged in the buffer?
	if (indexFirstAlive < indexFirstDead)
	{
		// Only copy from FirstAlive -> FirstDead
		memcpy(
			mapped.pData, // Destination = start of particle buffer
			particles + indexFirstAlive, // Source = particle array, offset to first living particle
			sizeof(Particle) * livingParticles); // Amount = number of particles (measured in BYTES!)
	}
	else
	{
		// Copy from 0 -> FirstDead
		memcpy(
			mapped.pData, // Destination = start of particle buffer
			particles, // Source = start of particle array
			sizeof(Particle) * indexFirstDead); // Amount = particles up to first dead (measured in BYTES!)
		// ALSO copy from FirstAlive -> End
		memcpy(
			(void*)((Particle*)mapped.pData + indexFirstDead), // Destination = particle buffer, AFTER the data we copied in previous memcpy()
			particles + indexFirstAlive, // Source = particle array, offset to first living particle
			sizeof(Particle) * (maxParticles - indexFirstAlive)); // Amount = number of living particles at end of array (measured in BYTES!)
	}
	// Unmap (unlock) now that we're done with it
	Graphics::Context->Unmap(particleDataBuffer.Get(), 0);
}

void Emitter::EmitParticle(float currentTime)
{
	if (livingParticles >= maxParticles)
	{
		return;
	}

	int index = indexFirstDead;

	particles[index].EmitTime = currentTime;


}

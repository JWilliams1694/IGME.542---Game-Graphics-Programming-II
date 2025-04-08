#include "Emitter.h"
#include "Graphics.h"

void Emitter::Update(float dt, float currentTime)
{
	lastEmit += dt;
	while (lastEmit > secondsPerParticle)
	{
		EmitParticle(currentTime);
		lastEmit -= secondsPerParticle;
	}

}

void Emitter::CreateParticleBuffer()
{
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

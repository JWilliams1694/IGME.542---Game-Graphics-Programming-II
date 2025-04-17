#include "Emitter.h"
#include "Graphics.h"

#define RandomRange(min, max) ((float)rand() / RAND_MAX * (max - min) + min)

Emitter::Emitter(int maxParticles,
				 int particlesPerSec,
				 float maxLifetime,
				 float startSize,
				 float endSize,
				 DirectX::XMFLOAT4 startColor,
				 DirectX::XMFLOAT4 endColor,
				 DirectX::XMFLOAT3 startVelocity,
				 DirectX::XMFLOAT3 velocityRandomRange,
				 DirectX::XMFLOAT3 emitterPosition,
				 DirectX::XMFLOAT3 positionRandomRange,
				 DirectX::XMFLOAT2 rotationStartMinMax,
				 DirectX::XMFLOAT2 rotationEndMinMax,
				 DirectX::XMFLOAT3 emitterAcceleration,
				 std::shared_ptr<Material> material,
				 unsigned int spriteSheetWidth,
				 unsigned int spriteSheetHeight,
				 float spriteSheetSpeedScale) :
	maxParticles(maxParticles),
	particlesPerSec(particlesPerSec),
	secondsPerParticle(1.0f / particlesPerSec),
	maxLifetime(maxLifetime),
	startSize(startSize),
	endSize(endSize),
	startColor(startColor),
	endColor(endColor),
	positionRandomRange(positionRandomRange),
	startVelocity(startVelocity),
	velocityRandomRange(velocityRandomRange),
	emitterAcceleration(emitterAcceleration),
	rotationStartMinMax(rotationStartMinMax),
	rotationEndMinMax(rotationEndMinMax),
	spriteSheetWidth(max(spriteSheetWidth, 1)),
	spriteSheetHeight(max(spriteSheetHeight, 1)),
	spriteSheetFrameWidth(1.0f / spriteSheetWidth),
	spriteSheetFrameHeight(1.0f / spriteSheetHeight),
	spriteSheetSpeedScale(spriteSheetSpeedScale),
	material(material),
	particles(nullptr)
{
	transform = std::make_shared<Transform>();
	transform->SetPosition(emitterPosition);
	indexFirstAlive = 0;
	indexFirstDead = 0;
	livingParticles = 0;
	lastEmit = 0;
	totalEmitTime = 0;

	CreateParticleBuffer();
}

Emitter::~Emitter()
{
	delete[] particles;
}

void Emitter::Update(float dt, float currentTime)
{
	lastEmit += dt;
	totalEmitTime += dt;

	// Check if any particles are dead using circular buffer 
	if (livingParticles > 0)
	{
		if (indexFirstAlive < indexFirstDead)
		{
			for (int i = indexFirstAlive; i < indexFirstDead; i++)
			{
				CheckSingleParticle(totalEmitTime, i);
			}
		}
		else if (indexFirstDead < indexFirstAlive)
		{
			for (int i = indexFirstAlive; i < maxParticles; i++)
			{
				CheckSingleParticle(totalEmitTime, i);
			}
			for (int i = 0; i < indexFirstDead; i++)
			{
				CheckSingleParticle(totalEmitTime, i);
			}
		}
		else
		{
			for (int i = 0; i < maxParticles; i++)
			{
				CheckSingleParticle(totalEmitTime, i);
			}
		}
		
	}
	// Update the particle data
	while (lastEmit > secondsPerParticle)
	{
		EmitParticle(currentTime);
		lastEmit -= secondsPerParticle;
	}
}

void Emitter::Draw(std::shared_ptr<Camera> camera, float currentTime, bool debug)
{
	CopyToGPU();

	UINT stride = 0;
	UINT offset = 0;
	ID3D11Buffer* nullBuffer = 0;
	Graphics::Context->IASetVertexBuffers(0, 1, &nullBuffer, &stride, &offset);
	Graphics::Context->IASetIndexBuffer(indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);

	material->PrepareMaterial(transform, camera);

	//vertex data
	std::shared_ptr<SimpleVertexShader> vs = material->GetVertexShader();
	vs->SetMatrix4x4("view", camera->GetView());
	vs->SetMatrix4x4("projection", camera->GetProjection());
	vs->SetFloat("currentTime", totalEmitTime);
	vs->SetFloat("lifetime", maxLifetime);
	vs->SetFloat3("accel", emitterAcceleration);
	vs->SetFloat("startSize", startSize);
	vs->SetFloat("endSize", endSize);
	vs->SetFloat4("startColor", startColor);
	vs->SetFloat4("endColor", endColor);
	vs->SetInt("spriteSheetWidth", spriteSheetWidth);
	vs->SetInt("spriteSheetHeight", spriteSheetHeight);
	vs->SetFloat("spriteSheetFrameWidth", spriteSheetFrameWidth);
	vs->SetFloat("spriteSheetFrameHeight", spriteSheetFrameHeight);
	vs->SetFloat("spriteSheetSpeedScale", spriteSheetSpeedScale);
	vs->CopyAllBufferData();

	vs->SetShaderResourceView("ParticleData", particleDataSRV);

	//pixel data
	std::shared_ptr<SimplePixelShader> ps = material->GetPixelShader();
	ps->SetInt("debugWireframe", debug);
	ps->CopyAllBufferData();


	// Now that all of our data is in the beginning of the particle buffer,
	// we can simply draw the correct amount of living particle indices.
	// Each particle = 4 vertices = 6 indices for a quad
	Graphics::Context->DrawIndexed(livingParticles * 6, 0, 0);
}

std::shared_ptr<Transform> Emitter::GetTransform()
{
	return transform;
}

std::shared_ptr<Material> Emitter::GetMaterial()
{
	return material;
}

void Emitter::SetMaterial(std::shared_ptr<Material> material)
{
	this->material = material;
}

int Emitter::GetParticlesPerSec()
{
	return particlesPerSec;
}

void Emitter::SetParticlesPerSec(int particlesPerSec)
{
	this->particlesPerSec = max(1, particlesPerSec);
	this->secondsPerParticle = 1.0f / particlesPerSec;
}

int Emitter::GetMaxParticles()
{
	return maxParticles;
}

void Emitter::SetMaxParticles(int maxParticles)
{
	this->maxParticles = max(1, maxParticles);
	CreateParticleBuffer();
}

bool Emitter::IsSpriteSheet()
{
	return spriteSheetHeight > 1 || spriteSheetFrameWidth > 1;
}

void Emitter::CreateParticleBuffer()
{
	//resetting if already exists
	if (particles) delete[] particles;
	indexBuffer.Reset();
	particleDataBuffer.Reset();
	particleDataSRV.Reset();

	// Create the particle array
	particles = new Particle[maxParticles];
	ZeroMemory(particles, sizeof(Particle) * maxParticles);


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
	D3D11_SUBRESOURCE_DATA indexData = {};
	indexData.pSysMem = indices;

	// Create the index buffer
	D3D11_BUFFER_DESC ibDesc = {};
	ibDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
	ibDesc.CPUAccessFlags = 0;
	ibDesc.Usage = D3D11_USAGE_DEFAULT;
	ibDesc.ByteWidth = sizeof(unsigned int) * maxParticles * 6;
	Graphics::Device->CreateBuffer(&ibDesc, &indexData, indexBuffer.GetAddressOf());
	delete[] indices;

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

//creates a single particle, random parts taken from chris demo
void Emitter::EmitParticle(float currentTime)
{
	if (livingParticles >= maxParticles)
	{
		return;
	}

	int index = indexFirstDead;

	particles[index].EmitTime = currentTime;

	// Adjust the particle start position based on the random range (box shape)
	particles[index].StartPos = transform->GetPosition();
	particles[index].StartPos.x += positionRandomRange.x * RandomRange(-1.0f, 1.0f);
	particles[index].StartPos.y += positionRandomRange.y * RandomRange(-1.0f, 1.0f);
	particles[index].StartPos.z += positionRandomRange.z * RandomRange(-1.0f, 1.0f);

	// Adjust particle start velocity based on random range
	particles[index].StartVelocity = startVelocity;
	particles[index].StartVelocity.x += velocityRandomRange.x * RandomRange(-1.0f, 1.0f);
	particles[index].StartVelocity.y += velocityRandomRange.y * RandomRange(-1.0f, 1.0f);
	particles[index].StartVelocity.z += velocityRandomRange.z * RandomRange(-1.0f, 1.0f);

	// Adjust start and end rotation values based on range
	particles[index].StartRotation = RandomRange(rotationStartMinMax.x, rotationStartMinMax.y);
	particles[index].EndRotation = RandomRange(rotationEndMinMax.x, rotationEndMinMax.y);

	// Increment the first dead particle (since it's now alive)
	indexFirstDead++;
	indexFirstDead %= maxParticles; // Wrap

	livingParticles++;


}

// checks a single particle
void Emitter::CheckSingleParticle(float currentTime, int i)
{
	float age = currentTime - particles[i].EmitTime;
	if (age > maxLifetime)
	{
		indexFirstAlive++;
		indexFirstAlive %= maxParticles;
		livingParticles--;
	}
}

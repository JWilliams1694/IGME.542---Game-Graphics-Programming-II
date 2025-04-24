#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <DirectXMath.h>
#include <memory>
#include "SimpleShader.h"
#include "Camera.h"
#include "Transform.h"
#include "Material.h"
struct Particle
{
	float EmitTime;
	DirectX::XMFLOAT3 StartPos;
	DirectX::XMFLOAT3 StartVelocity;
	float padding;
};

class Emitter
{
public:
	Emitter(int maxParticles,
			int particlesPerSec,
			float maxLifetime,
			float startSize,
			float endSize,
			DirectX::XMFLOAT4 startColor,
			DirectX::XMFLOAT4 endColor,
			DirectX::XMFLOAT3 startVel,
			DirectX::XMFLOAT3 velocityRandRange,
			DirectX::XMFLOAT3 emitterPos,
			DirectX::XMFLOAT3 positionRandRange,
			DirectX::XMFLOAT3 emitterAccel,
			std::shared_ptr<Material> material,
			bool enabled);
	~Emitter();

	//methods
	void Update(float dt, float currentTime);
	void Draw(std::shared_ptr<Camera> camera, float currentTime);

	//getters & setters
	std::shared_ptr<Transform> GetTransform();

	std::shared_ptr<Material> GetMaterial();
	void SetMaterial(std::shared_ptr<Material> material);

	int GetParticlesPerSec();
	void SetParticlesPerSec(int particlesPerSec);

	int GetMaxParticles();
	void SetMaxParticles(int maxParticles);

	//variables
	DirectX::XMFLOAT4 startColor;
	DirectX::XMFLOAT4 endColor;
	DirectX::XMFLOAT3 startVel;
	DirectX::XMFLOAT3 emitterAccel;
	float startSize;
	float endSize;
	float maxLifetime; // The max lifetime of particles
	bool enabled;
	//random vars
	DirectX::XMFLOAT3 positionRandRange;
	DirectX::XMFLOAT3 velocityRandRange;

private:
	//particle properties
	int maxParticles; // Maximum number of particles
	Particle* particles; // All possible particles
	int livingParticles; // The amount of currently living particles
	int indexFirstDead;
	int indexFirstAlive;

	//emission properties
	int particlesPerSec; // How many particles to emit each second
	float secondsPerParticle; // How many seconds between each particle emission
	float totalEmitTime; // How many (fractional) seconds between each particle emission
	float lastEmit; // How long has it been since the last emit

	//rendering properties
	Microsoft::WRL::ComPtr<ID3D11Buffer> particleDataBuffer;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> particleDataSRV;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;

	//methods
	void CreateParticleBuffer();
	void CopyToGPU();
	void EmitParticle(float currentTime);
	void CheckSingleParticle(float currentTime, int i);

	std::shared_ptr<Transform> transform;
	std::shared_ptr<Material> material;
};


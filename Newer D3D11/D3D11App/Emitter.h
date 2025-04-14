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
	float StartRotation;
	float EndRotation;
	DirectX::XMFLOAT3 padding;
};

class Emitter
{
public:
	Emitter(int maxParticles,
		int particlesPerSec,
		int maxLifetime,
		float startSize,
		float endSize,
		DirectX::XMFLOAT4 startColor,
		DirectX::XMFLOAT4 endColor,
		DirectX::XMFLOAT3 startVelocity,
		DirectX::XMFLOAT3 emitterPosition,
		std::shared_ptr<Material> material);
	~Emitter();

	//methods
	void Update(float dt, float currentTime);
	void Draw(std::shared_ptr<Camera> camera, float currentTime);

	//variables
	DirectX::XMFLOAT4 startColor;
	DirectX::XMFLOAT4 endColor;
	DirectX::XMFLOAT3 startVelocity;
	DirectX::XMFLOAT3 emitterAcceleration;
	float startSize;
	float endSize;

private:
	//particle properties
	int maxParticles; // Maximum number of particles
	Particle* particles; // All possible particles
	int livingParticles; // The amount of currently living particles
	int indexFirstDead;
	int indexFirstAlive;


	//emission properties
	int maxLifetime; // The max lifetime of particles
	int particlesPerSec; // How many particles to emit each second
	float emitTimer; // How many (fractional) seconds between each particle emission
	float lastEmit; // How long has it been since the last emit

	//rendering properties
	Microsoft::WRL::ComPtr<ID3D11Buffer> particleDataBuffer;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> particleDataSRV;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;

	//methods
	void CreateParticleBuffer();
	void CopyToGPU();
	void EmitParticle(float currentTime);

	std::shared_ptr<Transform> transform;
	std::shared_ptr<Material> material;
};


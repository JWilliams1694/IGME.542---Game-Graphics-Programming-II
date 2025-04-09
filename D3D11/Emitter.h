#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <DirectXMath.h>
#include <memory>
#include "SimpleShader.h"
#include "Camera.h"
#include "Transform.h"
struct Particle
{
	float EmitTime;
	DirectX::XMFLOAT3 StartPos;
}; 

class Emitter
{
public:
	Emitter();

	void Update(float dt, float currentTime);
	void Draw();
	

private:
	//particle properties
	int maxParticles; // Maximum number of particles
	Particle* particles; // All possible particles
	int livingParticles; // The amount of currently living particles
	int indexFirstDead;
	int indexFirstAlive;

	//emission properties
	int maxLifetime; // The max lifetime of particles
	int emitRate; // How many particles to emit each second
	float emitTimer; // How many (fractional) seconds between each particle emission
	float lastEmit; // How long has it been since the last emit

	//rendering properties
	Microsoft::WRL::ComPtr<ID3D11Buffer> particleDataBuffer;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> particleDataSRV;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;


	void CreateParticleBuffer();
	void CopyToGPU();
	void EmitParticle(float currentTime);
};


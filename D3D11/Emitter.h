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


private:
	//particle properties
	int maxParticles; // Maximum number of particles
	Particle* particles; // All possible particles
	int livingParticles; //amount of current particles

	//
};


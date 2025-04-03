#pragma once
#include "Transform.h"
#include <DirectXMath.h>
#include <memory>

class Camera
{
public:
	Camera(DirectX::XMFLOAT3 position, float aspectRatio, float FOV = DirectX::XM_PIDIV4, float nearClip = 0.01f, float farClip = 100.0f);
	~Camera();
	//getters
	std::shared_ptr<Transform> GetTransform();
	DirectX::XMFLOAT4X4 GetViewMatrix();
	DirectX::XMFLOAT4X4 GetProjMatrix();
	float GetAspectRatio();
	float GetFOV();
	float GetNearClip();
	float GetFarClip();

	//updaters
	void Update(float deltaTime);
	void UpdateViewMatrix();
	void UpdateProjMatrix(float aspectRatio);
	void SetFOV(float fov);

private:
	std::shared_ptr<Transform> transform;
	DirectX::XMFLOAT4X4 viewMatrix;
	DirectX::XMFLOAT4X4 projMatrix;
	float aspectRatio, FOV, nearClip, farClip;
};


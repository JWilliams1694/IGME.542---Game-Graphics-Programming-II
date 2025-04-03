#include "Camera.h"
#include "Window.h"
#include "Input.h"

using namespace DirectX;

Camera::Camera(DirectX::XMFLOAT3 position, float aspectRatio, float FOV, float nearClip, float farClip)
	:aspectRatio(aspectRatio), FOV(FOV), nearClip(nearClip), farClip(farClip)
{
	transform = std::make_shared<Transform>();
	transform->SetPosition(position);

	UpdateViewMatrix();
	UpdateProjMatrix(aspectRatio);
}

Camera::~Camera()
{
}

std::shared_ptr<Transform> Camera::GetTransform()
{
	return transform;
}

DirectX::XMFLOAT4X4 Camera::GetViewMatrix()
{
	return viewMatrix;
}

DirectX::XMFLOAT4X4 Camera::GetProjMatrix()
{
	return projMatrix;
}

float Camera::GetAspectRatio()
{
	return aspectRatio;
}

float Camera::GetFOV()
{
	return FOV;
}

float Camera::GetNearClip()
{
	return nearClip;
}

float Camera::GetFarClip()
{
	return farClip;
}

void Camera::Update(float deltaTime)
{
	float moveSpeed = 15 * deltaTime;
	if (Input::KeyDown('W'))
	{
		transform->MoveRelative(0, 0, moveSpeed);
	}
	if (Input::KeyDown('S'))
	{
		transform->MoveRelative(0, 0, -moveSpeed);
	}
	if (Input::KeyDown('A'))
	{
		transform->MoveRelative(-moveSpeed, 0, 0);
	}
	if (Input::KeyDown('D'))
	{
		transform->MoveRelative(moveSpeed, 0, 0);
	}
	if (Input::KeyDown('Q'))
	{
		transform->MoveAbsolute(0, -moveSpeed, 0);
	}
	if (Input::KeyDown('E'))
	{
		transform->MoveAbsolute(0, moveSpeed, 0);
	}
	if (Input::MouseLeftDown())
	{
		float lookSpeed = 0.01f;
		float xDiff = lookSpeed * Input::GetMouseXDelta();
		float yDiff = lookSpeed * Input::GetMouseYDelta();

		//lock rotation range
		XMFLOAT3 rot = transform->GetRotation();
		if (rot.x > XM_PIDIV2) rot.x = XM_PIDIV2;
		if (rot.x < -XM_PIDIV2) rot.x = -XM_PIDIV2;
		transform->Rotate(yDiff, xDiff, 0);
	}

	UpdateViewMatrix();
}

void Camera::UpdateViewMatrix()
{
	XMFLOAT3 pos = transform->GetPosition();
	XMFLOAT3 forward = transform->GetForward();
	XMFLOAT3 up = transform->GetUp();

	XMMATRIX view = XMMatrixLookToLH(
		XMLoadFloat3(&pos),
		XMLoadFloat3(&forward),
		XMLoadFloat3(&up));
	XMStoreFloat4x4(&viewMatrix, view);
}

void Camera::UpdateProjMatrix(float aspectRatio)
{
	XMMATRIX proj = XMMatrixPerspectiveFovLH(
		FOV,
		aspectRatio,
		nearClip, //near
		farClip); //far
	XMStoreFloat4x4(&projMatrix, proj);
}

void Camera::SetFOV(float fov)
{
	FOV = fov;
	UpdateProjMatrix(aspectRatio);
}

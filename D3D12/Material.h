#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include <DirectXMath.h>
#include <memory>
#include "Camera.h"
#include "Transform.h"
#include <unordered_map>
class Material
{
public:
	Material(Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState, DirectX::XMFLOAT3 tint,float roughness=1.0f,
			 DirectX::XMFLOAT2 uvScale = DirectX::XMFLOAT2(1, 1),
			 DirectX::XMFLOAT2 uvOffset = DirectX::XMFLOAT2(0, 0));

	//setters
	void SetTint(DirectX::XMFLOAT3 tint);
	void SetRoughness(float roughness);
	void SetUVScale(DirectX::XMFLOAT2 uvScale);
	void SetUVOffset(DirectX::XMFLOAT2 uvOffset);

	//getters
	DirectX::XMFLOAT3 GetTint();
	float GetRoughness();
	DirectX::XMFLOAT2 GetUVScale();
	DirectX::XMFLOAT2 GetUVOffset();
	D3D12_GPU_DESCRIPTOR_HANDLE GetFinalGPUHandleForSRVs();
	Microsoft::WRL::ComPtr<ID3D12PipelineState> GetPipelineState();

	//methods
	void AddTexture(D3D12_CPU_DESCRIPTOR_HANDLE srv, int slot);
	void FinalizeMaterial();

private:
	DirectX::XMFLOAT3 tint;
	float roughness;
	DirectX::XMFLOAT2 uvScale;
	DirectX::XMFLOAT2 uvOffset;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState;
	D3D12_CPU_DESCRIPTOR_HANDLE textureSRVsBySlot[128]; //128 suggested
	int highestSRVSlot = 0;
	D3D12_GPU_DESCRIPTOR_HANDLE finalGPUHandleForSRVs;
	bool finalized = false;
};
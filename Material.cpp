#include "Material.h"

Material::Material(DirectX::XMFLOAT3 tint, DirectX::XMFLOAT2 uvScale, DirectX::XMFLOAT2 uvOffset, Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState)
{
}

void Material::SetTint(DirectX::XMFLOAT3 tint)
{
}

void Material::SetUVScale(DirectX::XMFLOAT2 uvScale)
{
}

void Material::SetUVOffset(DirectX::XMFLOAT2 uvOffset)
{
}

DirectX::XMFLOAT3 Material::GetTint()
{
	return DirectX::XMFLOAT3();
}

DirectX::XMFLOAT2 Material::GetUVScale()
{
	return DirectX::XMFLOAT2();
}

DirectX::XMFLOAT2 Material::GetUVOffset()
{
	return DirectX::XMFLOAT2();
}

D3D12_GPU_DESCRIPTOR_HANDLE Material::GetFinalGPUHandleForSRVs()
{
	return D3D12_GPU_DESCRIPTOR_HANDLE();
}

Microsoft::WRL::ComPtr<ID3D12PipelineState> Material::GetPipelineState()
{
	return Microsoft::WRL::ComPtr<ID3D12PipelineState>();
}

void Material::AddTexture(D3D12_CPU_DESCRIPTOR_HANDLE srv, int slot)
{
}

void Material::FinalizeMaterial()
{
}

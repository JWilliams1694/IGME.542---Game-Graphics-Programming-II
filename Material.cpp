#include "Material.h"
#include "Graphics.h"

Material::Material(Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState,DirectX::XMFLOAT3 tint, DirectX::XMFLOAT2 uvScale, DirectX::XMFLOAT2 uvOffset) :
	tint(tint),
	uvScale(uvScale),
	uvOffset(uvOffset),
	pipelineState(pipelineState)
{
	//initialize data
	finalGPUHandleForSRVs = {};
}

void Material::SetTint(DirectX::XMFLOAT3 tint)
{
	this->tint = tint;
}

void Material::SetUVScale(DirectX::XMFLOAT2 uvScale)
{
	this->uvScale = uvScale;
}

void Material::SetUVOffset(DirectX::XMFLOAT2 uvOffset)
{
	this->uvOffset = uvOffset;
}

DirectX::XMFLOAT3 Material::GetTint()
{
	return tint;
}

DirectX::XMFLOAT2 Material::GetUVScale()
{
	return uvScale;
}

DirectX::XMFLOAT2 Material::GetUVOffset()
{
	return uvOffset;
}

D3D12_GPU_DESCRIPTOR_HANDLE Material::GetFinalGPUHandleForSRVs()
{
	return finalGPUHandleForSRVs;
}

Microsoft::WRL::ComPtr<ID3D12PipelineState> Material::GetPipelineState()
{
	return pipelineState;
}

void Material::AddTexture(D3D12_CPU_DESCRIPTOR_HANDLE srv, int slot)
{
	if (!finalized)
	{
		textureSRVsBySlot[slot] = srv;
		highestSRVSlot = slot;
	}
}

void Material::FinalizeMaterial()
{
	if (finalized)
		return;

	for (int i = 0; i <= highestSRVSlot; i++)
	{
		//copy srv induvidually to descriptor heap
		D3D12_GPU_DESCRIPTOR_HANDLE gpuHandle = Graphics::CopySRVsToDescriptorHeapAndGetGPUDescriptorHandle(textureSRVsBySlot[i], 1);

		if (i == 0)
		{
			finalGPUHandleForSRVs = gpuHandle;
		}
		finalized = true;
	}
}

#include "Material.h"

Material::Material(const char* name,
				   std::shared_ptr<SimplePixelShader> pixelShader,
				   std::shared_ptr<SimpleVertexShader> vertexShader,
				   DirectX::XMFLOAT4 tint,
				   float roughness,
				   DirectX::XMFLOAT2 uvScale,
				   DirectX::XMFLOAT2 uvOffset)
	:name(name),
	pixelShader(pixelShader),
	vertexShader(vertexShader),
	tint(tint),
	roughness(roughness),
	uvScale(uvScale),
	uvOffset(uvOffset)
{
}

std::shared_ptr<SimplePixelShader> Material::GetPixelShader()
{
	return pixelShader;
}

std::shared_ptr<SimpleVertexShader> Material::GetVertexShader()
{
	return vertexShader;
}

DirectX::XMFLOAT4 Material::GetTint()
{
	return tint;
}

const char* Material::GetName()
{
	return name;
}

float Material::GetRoughness()
{
	return roughness;
}

DirectX::XMFLOAT2 Material::GetUVScale()
{
	return uvScale;
}

DirectX::XMFLOAT2 Material::GetUVOffset()
{
	return uvOffset;
}

void Material::SetPixelShader(std::shared_ptr<SimplePixelShader> pixelShader)
{
	this->pixelShader = pixelShader;
}

void Material::SetVertexShader(std::shared_ptr<SimpleVertexShader> vertexShader)
{
	this->vertexShader = vertexShader;
}

void Material::SetTint(DirectX::XMFLOAT4 tint)
{
	this->tint = tint;
}

void Material::SetName(const char* name)
{
	this->name = name;
}

void Material::SetRoughness(float roughness)
{
	this->roughness = roughness;
}

void Material::SetUVScale(DirectX::XMFLOAT2 uvScale)
{
	this->uvScale = uvScale;
}

void Material::SetUVOffset(DirectX::XMFLOAT2 uvOffset)
{
	this->uvOffset = uvOffset;
}

void Material::AddTextureSRV(std::string shaderVariableName, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv)
{
	textureSRVs.insert({ shaderVariableName, srv });
}

void Material::AddSampler(std::string shaderVariableName, Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler)
{
	samplers.insert({ shaderVariableName, sampler });
}

void Material::MakeMaterials(std::shared_ptr<Transform> transform, std::shared_ptr<Camera> camera)
{


	vertexShader->SetMatrix4x4("worldMatrix", transform->GetWorldMatrix());
	vertexShader->SetMatrix4x4("viewMatrix", camera->GetViewMatrix());
	vertexShader->SetMatrix4x4("projectionMatrix", camera->GetProjMatrix());
	vertexShader->SetMatrix4x4("worldInvTranspose", transform->GetWorldInverseTransposeMatrix());
	vertexShader->CopyAllBufferData();

	pixelShader->SetFloat4("colorTint", tint);
	pixelShader->SetFloat("roughness", roughness);
	pixelShader->SetFloat3("cameraPosition", camera->GetTransform()->GetPosition());
	pixelShader->SetFloat2("uvScale", uvScale);
	pixelShader->SetFloat2("uvOffset", uvOffset);
	pixelShader->CopyAllBufferData();


	// Set the vertex and pixel shaders
	vertexShader->SetShader();
	pixelShader->SetShader();

	for (auto& t : textureSRVs) { pixelShader->SetShaderResourceView(t.first.c_str(), t.second.Get()); }
	for (auto& s : samplers) { pixelShader->SetSamplerState(s.first.c_str(), s.second.Get()); }
}

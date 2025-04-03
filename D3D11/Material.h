#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include <DirectXMath.h>
#include <memory>
#include "SimpleShader.h"
#include "Camera.h"
#include "Transform.h"
#include <unordered_map>
class Material
{
public:
	Material(const char* name,
			 std::shared_ptr<SimplePixelShader>pixelShader,
			 std::shared_ptr<SimpleVertexShader>vertexShader,
			 DirectX::XMFLOAT4 tint=DirectX::XMFLOAT4(1,1,1,1), float roughness=0.0f,
			 DirectX::XMFLOAT2 uvScale = DirectX::XMFLOAT2(1, 1),
			 DirectX::XMFLOAT2 uvOffset = DirectX::XMFLOAT2(0, 0));
	std::shared_ptr<SimplePixelShader> GetPixelShader();
	std::shared_ptr<SimpleVertexShader> GetVertexShader();
	DirectX::XMFLOAT4 GetTint();
	const char* GetName();
	float GetRoughness();
	DirectX::XMFLOAT2 GetUVScale();
	DirectX::XMFLOAT2 GetUVOffset();


	void SetPixelShader(std::shared_ptr<SimplePixelShader> pixelShader);
	void SetVertexShader(std::shared_ptr<SimpleVertexShader> vertexShader);
	void SetTint(DirectX::XMFLOAT4 tint);
	void SetName(const char* name);
	void SetRoughness(float roughness);
	void SetUVScale(DirectX::XMFLOAT2 uvScale);
	void SetUVOffset(DirectX::XMFLOAT2 uvOffset);

	void AddTextureSRV(std::string shaderVariableName, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> srv);
	void AddSampler(std::string shaderVariableName, Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler);

	void MakeMaterials(std::shared_ptr<Transform> transform, std::shared_ptr<Camera> camera);
private:
	std::shared_ptr<SimplePixelShader> pixelShader;
	std::shared_ptr<SimpleVertexShader> vertexShader;
	DirectX::XMFLOAT4 tint;
	const char* name;
	float roughness;
	DirectX::XMFLOAT2 uvScale;
	DirectX::XMFLOAT2 uvOffset;
	bool isSpecular;

	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11ShaderResourceView>> textureSRVs;
	std::unordered_map<std::string, Microsoft::WRL::ComPtr<ID3D11SamplerState>> samplers;
};


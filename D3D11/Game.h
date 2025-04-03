#pragma once

#include <d3d11.h>
#include <wrl/client.h>
#include <memory>
#include <vector>
#include <DirectXMath.h>
#include "GameEntity.h"
#include "Camera.h"
#include "SimpleShader.h"
#include "Mesh.h"
#include "Material.h"
#include "Lights.h"
#include "Sky.h"


class Game
{
public:
	// Basic OOP setup
	Game() = default;
	~Game();
	Game(const Game&) = delete; // Remove copy constructor
	Game& operator=(const Game&) = delete; // Remove copy-assignment operator

	// Primary functions
	void Initialize();
	void Update(float deltaTime, float totalTime);
	void Draw(float deltaTime, float totalTime);
	void OnResize();
	void ShadowDraw();
	void CreateShadowMap();
	void DrawLightSources();
	void UpdateShView();
	void PostProcess();

private:

	// Initialization helper methods - feel free to customize, combine, remove, etc.
	void LoadShaders();
	void CreateMaterialsTexturesMeshes();

	std::vector<std::shared_ptr<Mesh>> meshes;
	std::vector<std::shared_ptr<GameEntity>> entities;
	std::vector<std::shared_ptr<Camera>> cameras;
	float color[4] = { 0.4f, 0.6f, 0.75f, 0.0f };

	//shape color and offset
	DirectX::XMFLOAT4 colorTint = DirectX::XMFLOAT4(0.5f, 1.0f, 0.5f, 1.0f);
	DirectX::XMFLOAT3 offset = DirectX::XMFLOAT3(-0.3f, 0.0f, 0.0f);

	// Note the usage of ComPtr below
	//  - This is a smart pointer for objects that abide by the
	//     Component Object Model, which DirectX objects do
	//  - More info here: https://github.com/Microsoft/DirectXTK/wiki/ComPtr

	// Buffers to hold actual geometry data
	Microsoft::WRL::ComPtr<ID3D11Buffer> vertexBuffer;
	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;

	// Shaders and shader-related constructs
	std::shared_ptr<SimplePixelShader> pixelShader;
	std::shared_ptr<SimplePixelShader> normalPSShader;
	std::shared_ptr<SimplePixelShader> uvPSShader;
	std::shared_ptr<SimplePixelShader> customPSShader;
	std::shared_ptr<SimpleVertexShader> vertexShader;
	std::shared_ptr<SimpleVertexShader> skyVS;
	std::shared_ptr<SimplePixelShader> skyPS;
	std::shared_ptr<SimpleVertexShader> shadowVS;

	int activeCameraIndex;

	//materials
	std::vector<std::shared_ptr<Material>> materials;


	//lights
	std::vector<Light> lights;
	DirectX::XMFLOAT3 ambientColor = DirectX::XMFLOAT3(0.1f, 0.1f, 0.3f);
	std::shared_ptr<Mesh> pointLightMesh;


	//sky
	std::shared_ptr<Sky> sky;

	//shadows
	ShadowOptions shadowOptions;
	Microsoft::WRL::ComPtr<ID3D11SamplerState> shadowSampler;
	Microsoft::WRL::ComPtr<ID3D11RasterizerState> shadowRasterizer;
	std::shared_ptr<SimpleVertexShader> shadowVertexShader;
	DirectX::XMMATRIX shView;

	// Resources that are shared among all post processes
	Microsoft::WRL::ComPtr<ID3D11SamplerState> ppSampler;
	Microsoft::WRL::ComPtr<ID3D11RenderTargetView> ppRTV; // For rendering
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> ppSRV; // For sampling

	//post process 
	std::shared_ptr<SimplePixelShader> blurPS;
	std::shared_ptr<SimpleVertexShader> fullscreenTriVS;
	int blurDistance;
	bool isBlurred;

	PostProcessOptions postProcessOptions;


};


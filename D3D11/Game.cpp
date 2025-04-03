#include "Game.h"
#include "Graphics.h"
#include "Vertex.h"
#include "Input.h"
#include "PathHelpers.h"
#include "Window.h"
#include "GUI.h"
#include "SimpleShader.h"
#include <random>


#include <DirectXMath.h>

// Needed for a helper function to load pre-compiled shader files
#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>

#include "WICTextureLoader.h"

// This code assumes files are in "ImGui" subfolder!
// Adjust as necessary for your own folder structure and project setup
#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"

// For the DirectX Math library
using namespace DirectX;

// --------------------------------------------------------
// Called once per program, after the window and graphics API
// are initialized but before the game loop begins
// --------------------------------------------------------
void Game::Initialize()
{
	// Helper methods for loading shaders, creating some basic
	// geometry to draw and some simple camera matrices.
	//  - You'll be expanding and/or replacing these later
	LoadShaders();
	CreateMaterialsTexturesMeshes();
	PostProcess();



	// Set initial graphics API state
	//  - These settings persist until we change them
	//  - Some of these, like the primitive topology & input layout, probably won't change
	//  - Others, like setting shaders, will need to be moved elsewhere later
	{
		// Tell the input assembler (IA) stage of the pipeline what kind of
		// geometric primitives (points, lines or triangles) we want to draw.  
		// Essentially: "What kind of shape should the GPU draw with our vertices?"
		Graphics::Context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	}


	// Initialize ImGui itself & platform/renderer backends
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui_ImplWin32_Init(Window::Handle());
	ImGui_ImplDX11_Init(Graphics::Device.Get(), Graphics::Context.Get());
	// Pick a style (uncomment one of these 3)
	ImGui::StyleColorsDark();
	//ImGui::StyleColorsLight();
	//ImGui::StyleColorsClassic();


	//camera = std::make_shared<Camera>(XMFLOAT3(0, 0, -5), Window::AspectRatio(), XM_PIDIV4);
	cameras.push_back(std::make_shared<Camera>(XMFLOAT3(8, 5, -20), Window::AspectRatio(), XM_PIDIV4));
	cameras.push_back(std::make_shared<Camera>(XMFLOAT3(0, -1, -10), Window::AspectRatio(), XM_PIDIV4));
	activeCameraIndex = 0;

	//all of the lights
	Light dirLight1 = {};
	dirLight1.Color = XMFLOAT3(1, 1, 1);
	dirLight1.Type = LIGHT_TYPE_DIRECTIONAL;
	dirLight1.Intensity = 2.5f;
	dirLight1.Direction = XMFLOAT3(0, -3, -3);

	Light dirLight2 = {};
	dirLight2.Color = XMFLOAT3(1, 1, 1);
	dirLight2.Type = LIGHT_TYPE_DIRECTIONAL;
	dirLight2.Intensity = 1.0f;
	dirLight2.Direction = XMFLOAT3(-1, 0, 0);

	Light dirLight3 = {};
	dirLight3.Color = XMFLOAT3(1, 1, 1);
	dirLight3.Type = LIGHT_TYPE_DIRECTIONAL;
	dirLight3.Intensity = 1.0f;
	dirLight3.Direction = XMFLOAT3(0, -5, 1);

	Light pointLight1 = {};
	pointLight1.Color = XMFLOAT3(1, 1, 1);
	pointLight1.Type = LIGHT_TYPE_POINT;
	pointLight1.Intensity = 2.0f;
	pointLight1.Position = XMFLOAT3(-1.5f, 0, 0);
	pointLight1.Range = 15.0f;

	Light pointLight2 = {};
	pointLight2.Color = XMFLOAT3(1, 1, 1);
	pointLight2.Type = LIGHT_TYPE_POINT;
	pointLight2.Intensity = 2.0f;
	pointLight2.Position = XMFLOAT3(20, 0, 0);
	pointLight2.Range = 15.0f;

	// Add all lights to the list
	lights.push_back(dirLight1);
	//lights.push_back(dirLight2);
	//lights.push_back(dirLight3);
	lights.push_back(pointLight1);
	lights.push_back(pointLight2);

	//normalize all of the lights
	for (int i = 0; i < lights.size(); i++)
		if (lights[i].Type != LIGHT_TYPE_POINT)
			XMStoreFloat3(
				&lights[i].Direction,
				XMVector3Normalize(XMLoadFloat3(&lights[i].Direction))
			);
	shadowOptions.ShadowMapResolution = 2048;
	shadowOptions.ShadowProjectionSize = 50.0f;
	CreateShadowMap();

	postProcessOptions = {
		.BlurAmount = 3,
		.IsBlurred = false,
		.FogType = 1,
		.FogColor = XMFLOAT3(0.1f, 0.1f, 0.1f),
		.FogStartDistance = 0.0f,
		.FogEndDistance = 40.0f,
		.FogDensity = 0.08f,
		.HeightBasedFog = false,
		.FogHeight = 5.0f,
	};
}


// --------------------------------------------------------
// Clean up memory or objects created by this class
// 
// Note: Using smart pointers means there probably won't
//       be much to manually clean up here!
// --------------------------------------------------------
Game::~Game()
{
	// ImGui clean up
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
}


// --------------------------------------------------------
// Loads shaders from compiled shader object (.cso) files
// and also created the Input Layout that describes our 
// vertex data to the rendering pipeline. 
// - Input Layout creation is done here because it must 
//    be verified against vertex shader byte code
// - We'll have that byte code already loaded below
// --------------------------------------------------------
void Game::LoadShaders()
{
	vertexShader = std::make_shared<SimpleVertexShader>(Graphics::Device, Graphics::Context, FixPath(L"VertexShader.cso").c_str());
	pixelShader = std::make_shared<SimplePixelShader>(Graphics::Device, Graphics::Context, FixPath(L"PixelShader.cso").c_str());
	normalPSShader = std::make_shared<SimplePixelShader>(Graphics::Device, Graphics::Context, FixPath(L"normalPS.cso").c_str());
	uvPSShader = std::make_shared<SimplePixelShader>(Graphics::Device, Graphics::Context, FixPath(L"uvPS.cso").c_str());
	customPSShader = std::make_shared<SimplePixelShader>(Graphics::Device, Graphics::Context, FixPath(L"customPS.cso").c_str());
	skyVS = std::make_shared<SimpleVertexShader>(Graphics::Device, Graphics::Context, FixPath(L"SkyVS.cso").c_str());
	skyPS = std::make_shared<SimplePixelShader>(Graphics::Device, Graphics::Context, FixPath(L"SkyPS.cso").c_str());
	shadowVS = std::make_shared<SimpleVertexShader>(Graphics::Device, Graphics::Context, FixPath(L"ShadowVS.cso").c_str());
	blurPS = std::make_shared<SimplePixelShader>(Graphics::Device, Graphics::Context, FixPath(L"BoxBlurPS.cso").c_str());
	fullscreenTriVS = std::make_shared<SimpleVertexShader>(Graphics::Device, Graphics::Context, FixPath(L"fullscreenTriVS.cso").c_str());
}

void Game::CreateMaterialsTexturesMeshes()
{
	Microsoft::WRL::ComPtr<ID3D11SamplerState> sampler;
	D3D11_SAMPLER_DESC sampleDesc = {};
	sampleDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	sampleDesc.Filter = D3D11_FILTER_ANISOTROPIC;
	sampleDesc.MaxAnisotropy = 16;
	sampleDesc.MaxLOD = D3D11_FLOAT32_MAX;
	Graphics::Device->CreateSamplerState(&sampleDesc, sampler.GetAddressOf());



	//textures
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> cobbleA, cobbleN, cobbleR, cobbleM;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> floorA, floorN, floorR, floorM;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> paintA, paintN, paintR, paintM;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> scratchedA, scratchedN, scratchedR, scratchedM;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> bronzeA, bronzeN, bronzeR, bronzeM;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> roughA, roughN, roughR, roughM;
	Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> woodA, woodN, woodR, woodM;

	//shortcut to get all textures from class demo
#define LoadTexture(path, srv) CreateWICTextureFromFile(Graphics::Device.Get(), Graphics::Context.Get(), FixPath(path).c_str(), 0, srv.GetAddressOf());
	LoadTexture(L"../../Assets/Textures/cobblestone_albedo.png", cobbleA);
	LoadTexture(L"../../Assets/Textures/cobblestone_normals.png", cobbleN);
	LoadTexture(L"../../Assets/Textures/cobblestone_roughness.png", cobbleR);
	LoadTexture(L"../../Assets/Textures/cobblestone_metal.png", cobbleM);

	LoadTexture(L"../../Assets/Textures/floor_albedo.png", floorA);
	LoadTexture(L"../../Assets/Textures/floor_normals.png", floorN);
	LoadTexture(L"../../Assets/Textures/floor_roughness.png", floorR);
	LoadTexture(L"../../Assets/Textures/floor_metal.png", floorM);

	LoadTexture(L"../../Assets/Textures/paint_albedo.png", paintA);
	LoadTexture(L"../../Assets/Textures/paint_normals.png", paintN);
	LoadTexture(L"../../Assets/Textures/paint_roughness.png", paintR);
	LoadTexture(L"../../Assets/Textures/paint_metal.png", paintM);

	LoadTexture(L"../../Assets/Textures/scratched_albedo.png", scratchedA);
	LoadTexture(L"../../Assets/Textures/scratched_normals.png", scratchedN);
	LoadTexture(L"../../Assets/Textures/scratched_roughness.png", scratchedR);
	LoadTexture(L"../../Assets/Textures/scratched_metal.png", scratchedM);

	LoadTexture(L"../../Assets/Textures/bronze_albedo.png", bronzeA);
	LoadTexture(L"../../Assets/Textures/bronze_normals.png", bronzeN);
	LoadTexture(L"../../Assets/Textures/bronze_roughness.png", bronzeR);
	LoadTexture(L"../../Assets/Textures/bronze_metal.png", bronzeM);

	LoadTexture(L"../../Assets/Textures/rough_albedo.png", roughA);
	LoadTexture(L"../../Assets/Textures/rough_normals.png", roughN);
	LoadTexture(L"../../Assets/Textures/rough_roughness.png", roughR);
	LoadTexture(L"../../Assets/Textures/rough_metal.png", roughM);

	LoadTexture(L"../../Assets/Textures/wood_albedo.png", woodA);
	LoadTexture(L"../../Assets/Textures/wood_normals.png", woodN);
	LoadTexture(L"../../Assets/Textures/wood_roughness.png", woodR);
	LoadTexture(L"../../Assets/Textures/wood_metal.png", woodM);
#undef LoadTexture

	//meshes
	std::shared_ptr<Mesh> cubeMesh = std::make_shared<Mesh>("Cube", FixPath(L"../../Assets/Meshes/cube.obj").c_str());
	std::shared_ptr<Mesh> cylinderMesh = std::make_shared<Mesh>("Cylinder", FixPath(L"../../Assets/Meshes/cylinder.obj").c_str());
	std::shared_ptr<Mesh> helixMesh = std::make_shared<Mesh>("Helix", FixPath(L"../../Assets/Meshes/helix.obj").c_str());
	std::shared_ptr<Mesh> quadMesh = std::make_shared<Mesh>("Quad", FixPath(L"../../Assets/Meshes/quad.obj").c_str());
	std::shared_ptr<Mesh> quadDoubleSidedMesh = std::make_shared<Mesh>("Double Sided Quad", FixPath(L"../../Assets/Meshes/quad_double_sided.obj").c_str());
	std::shared_ptr<Mesh> sphereMesh = std::make_shared<Mesh>("Sphere", FixPath(L"../../Assets/Meshes/sphere.obj").c_str());
	std::shared_ptr<Mesh> torusMesh = std::make_shared<Mesh>("Torus", FixPath(L"../../Assets/Meshes/torus.obj").c_str());

	meshes.push_back(cubeMesh);
	meshes.push_back(cylinderMesh);
	meshes.push_back(helixMesh);
	meshes.push_back(quadMesh);
	meshes.push_back(quadDoubleSidedMesh);
	meshes.push_back(sphereMesh);
	meshes.push_back(torusMesh);
	pointLightMesh = sphereMesh;

	//sky
	sky = std::make_shared<Sky>(
		FixPath(L"../../Assets/Skies/Planet/right.png").c_str(),
		FixPath(L"../../Assets/Skies/Planet/left.png").c_str(),
		FixPath(L"../../Assets/Skies/Planet/up.png").c_str(),
		FixPath(L"../../Assets/Skies/Planet/down.png").c_str(),
		FixPath(L"../../Assets/Skies/Planet/front.png").c_str(),
		FixPath(L"../../Assets/Skies/Planet/back.png").c_str(),
		meshes[5],
		skyVS,
		skyPS,
		sampler);


	//materials
	std::shared_ptr<Material> cobbleMat = std::make_shared<Material>("Cobblestone (2x Scale)", normalPSShader, vertexShader);
	cobbleMat->AddSampler("BasicSampler", sampler);
	cobbleMat->AddTextureSRV("Albedo", cobbleA);
	cobbleMat->AddTextureSRV("NormalMap", cobbleN);
	cobbleMat->AddTextureSRV("RoughnessMap", cobbleR);
	cobbleMat->AddTextureSRV("MetalnessMap", cobbleM);

	std::shared_ptr<Material> floorMat = std::make_shared<Material>("Metal Floor", normalPSShader, vertexShader);
	floorMat->AddSampler("BasicSampler", sampler);
	floorMat->AddTextureSRV("Albedo", floorA);
	floorMat->AddTextureSRV("NormalMap", floorN);
	floorMat->AddTextureSRV("RoughnessMap", floorR);
	floorMat->AddTextureSRV("MetalnessMap", floorM);

	std::shared_ptr<Material> paintMat = std::make_shared<Material>("Blue Paint", normalPSShader, vertexShader);
	paintMat->AddSampler("BasicSampler", sampler);
	paintMat->AddTextureSRV("Albedo", paintA);
	paintMat->AddTextureSRV("NormalMap", paintN);
	paintMat->AddTextureSRV("RoughnessMap", paintR);
	paintMat->AddTextureSRV("MetalnessMap", paintM);

	std::shared_ptr<Material> scratchedMat = std::make_shared<Material>("Scratched Paint", normalPSShader, vertexShader);
	scratchedMat->AddSampler("BasicSampler", sampler);
	scratchedMat->AddTextureSRV("Albedo", scratchedA);
	scratchedMat->AddTextureSRV("NormalMap", scratchedN);
	scratchedMat->AddTextureSRV("RoughnessMap", scratchedR);
	scratchedMat->AddTextureSRV("MetalnessMap", scratchedM);

	std::shared_ptr<Material> bronzeMat = std::make_shared<Material>("Bronze", normalPSShader, vertexShader);
	bronzeMat->AddSampler("BasicSampler", sampler);
	bronzeMat->AddTextureSRV("Albedo", bronzeA);
	bronzeMat->AddTextureSRV("NormalMap", bronzeN);
	bronzeMat->AddTextureSRV("RoughnessMap", bronzeR);
	bronzeMat->AddTextureSRV("MetalnessMap", bronzeM);

	std::shared_ptr<Material> roughMat = std::make_shared<Material>("Rough Metal", normalPSShader, vertexShader);
	roughMat->AddSampler("BasicSampler", sampler);
	roughMat->AddTextureSRV("Albedo", roughA);
	roughMat->AddTextureSRV("NormalMap", roughN);
	roughMat->AddTextureSRV("RoughnessMap", roughR);
	roughMat->AddTextureSRV("MetalnessMap", roughM);

	std::shared_ptr<Material> woodMat = std::make_shared<Material>("Wood", normalPSShader, vertexShader);
	woodMat->AddSampler("BasicSampler", sampler);
	woodMat->AddTextureSRV("Albedo", woodA);
	woodMat->AddTextureSRV("NormalMap", woodN);
	woodMat->AddTextureSRV("RoughnessMap", woodR);
	woodMat->AddTextureSRV("MetalnessMap", woodM);


	materials.push_back(cobbleMat);
	materials.push_back(floorMat);
	materials.push_back(paintMat);
	materials.push_back(scratchedMat);
	materials.push_back(bronzeMat);
	materials.push_back(roughMat);
	materials.push_back(woodMat);

	//entities
	int rows = 5;// (int)materials.size();
	for (int j = 0; j < rows; j++)
	{
		for (int i = 0; i < meshes.size(); i++)
		{
			entities.push_back(std::make_shared<GameEntity>(meshes[i], materials[i]));
		}
	}


	for (int j = 0; j < rows; j++)
	{
		for (int i = 0; i < meshes.size(); i++)
		{
			int index = j * (int)meshes.size() + i;
			entities[index]->GetTransform()->SetPosition((float)i * 3, (float)j * 2, (float)j * 4);
		}
	}
	std::shared_ptr<GameEntity> floor = std::make_shared<GameEntity>(cubeMesh, woodMat);
	floor->GetTransform()->SetScale(20, 1, 20);
	floor->GetTransform()->SetPosition(7, -3, 0);
	entities.push_back(floor);

	PostProcess();

	// Sampler state for post processing
	D3D11_SAMPLER_DESC ppSampDesc = {};
	ppSampDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
	ppSampDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
	ppSampDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
	ppSampDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	ppSampDesc.MaxLOD = D3D11_FLOAT32_MAX;
	Graphics::Device->CreateSamplerState(&ppSampDesc, ppSampler.GetAddressOf());
}

// --------------------------------------------------------
// Handle resizing to match the new window size
//  - Eventually, we'll want to update our 3D camera
// --------------------------------------------------------
void Game::OnResize()
{
	if (cameras.size() > 0)
		for (auto& cam : cameras)
			cam->UpdateProjMatrix(Window::AspectRatio());

	if (Graphics::Device)
	{
		PostProcess();
	}
}

// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	CreateUI(deltaTime);
	BuildUI(meshes, entities, activeCameraIndex, cameras, lights, shadowOptions, postProcessOptions);
	// Example input checking: Quit if the escape key is pressed
	if (Input::KeyDown(VK_ESCAPE))
		Window::Quit();

	for (unsigned int i = 0; i < entities.size() - 1; i++)
	{
		entities[i]->GetTransform()->Rotate(0, 0.5f * deltaTime, 0);
	}
	cameras[activeCameraIndex]->Update(deltaTime);
	UpdateShView();
}
// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// Frame START
	// - These things should happen ONCE PER FRAME
	// - At the beginning of Game::Draw() before drawing *anything*
	{
		// Clear the back buffer (erase what's on screen) and depth buffer

		Graphics::Context->ClearRenderTargetView(Graphics::BackBufferRTV.Get(), GetBackgroundColor());
		Graphics::Context->ClearDepthStencilView(Graphics::DepthBufferDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	}

	//shadow stuff
	ShadowDraw();

	//post processing pre draw
	if (postProcessOptions.IsBlurred)
	{
		const float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
		Graphics::Context->ClearRenderTargetView(ppRTV.Get(), clearColor);
		Graphics::Context->OMSetRenderTargets(1, ppRTV.GetAddressOf(), Graphics::DepthBufferDSV.Get());
	}


	for (auto& ent : entities)
	{
		std::shared_ptr<SimpleVertexShader> vs = ent->GetMaterial()->GetVertexShader();
		vs->SetMatrix4x4("shadowView", shadowOptions.ShadowViewMatrix);
		vs->SetMatrix4x4("shadowProjection", shadowOptions.ShadowProjectionMatrix);
		std::shared_ptr<SimplePixelShader> ps = ent->GetMaterial()->GetPixelShader();
		ps->SetFloat3("ambientColor", ambientColor);
		ps->SetData("lights", &lights[0], sizeof(Light) * (int)lights.size());
		ps->SetFloat("farClip", cameras[activeCameraIndex]->GetFarClip());
		ps->SetInt("fogType", postProcessOptions.FogType);
		ps->SetFloat3("fogColor", postProcessOptions.FogColor);
		ps->SetFloat("fogStartDist", postProcessOptions.FogStartDistance);
		ps->SetFloat("fogEndDist", postProcessOptions.FogEndDistance);
		ps->SetFloat("fogDensity", postProcessOptions.FogDensity);
		ps->SetInt("heightBasedFog", postProcessOptions.HeightBasedFog);
		ps->SetFloat("fogHeight", postProcessOptions.FogHeight);

		ps->SetInt("lightCount", (int)lights.size());

		ps->SetShaderResourceView("ShadowMap", shadowOptions.ShadowSRV);
		ps->SetSamplerState("ShadowSampler", shadowSampler);

		ent->Draw(cameras[activeCameraIndex]);
	}
	sky->Draw(cameras[activeCameraIndex]);

	//visualizes lights
	//DrawLightSources();

	//post processing post draw
	Graphics::Context->OMSetRenderTargets(1, Graphics::BackBufferRTV.GetAddressOf(), 0);

	if (postProcessOptions.IsBlurred)
	{

		UINT stride = sizeof(Vertex);
		UINT offset = 0;
		ID3D11Buffer* nothing = 0;
		Graphics::Context->IASetIndexBuffer(0, DXGI_FORMAT_R32_UINT, 0);
		Graphics::Context->IASetVertexBuffers(0, 1, &nothing, &stride, &offset);


		fullscreenTriVS->SetShader();
		blurPS->SetShader();
		blurPS->SetShaderResourceView("Pixels", ppSRV.Get());
		blurPS->SetSamplerState("ClampSampler", ppSampler.Get());
		Graphics::Context->Draw(3, 0); // Draw exactly 3 vertices (one triangle)
		blurPS->SetFloat("pixelWidth", 1.0f / Window::Width());
		blurPS->SetFloat("pixelHeight", 1.0f / Window::Height());
		blurPS->SetInt("blurRadius", postProcessOptions.BlurAmount);
		blurPS->CopyAllBufferData();
		Graphics::Context->Draw(3, 0);
		// Activate shaders and bind resources

	}
	ID3D11ShaderResourceView* nullSRVs[16] = {};
	Graphics::Context->PSSetShaderResources(0, 16, nullSRVs);
	// Frame END
	// - These should happen exactly ONCE PER FRAME
	// - At the very end of the frame (after drawing *everything*)
	{
		ImGui::Render(); // Turns this frame’s UI into renderable triangles
		ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData()); // Draws it to the screen

		// Present at the end of the frame
		bool vsync = Graphics::VsyncState();
		Graphics::SwapChain->Present(
			vsync ? 1 : 0,
			vsync ? 0 : DXGI_PRESENT_ALLOW_TEARING);

		// Re-bind back buffer and depth buffer after presenting
		Graphics::Context->OMSetRenderTargets(
			1,
			Graphics::BackBufferRTV.GetAddressOf(),
			Graphics::DepthBufferDSV.Get());
	}
}

void Game::ShadowDraw()
{
	Graphics::Context->OMSetRenderTargets(0, 0, shadowOptions.ShadowDSV.Get());
	Graphics::Context->ClearDepthStencilView(shadowOptions.ShadowDSV.Get(), D3D11_CLEAR_DEPTH, 1.0f, 0);
	Graphics::Context->RSSetState(shadowRasterizer.Get());

	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = 0.0f;
	viewport.TopLeftY = 0.0f;
	viewport.Width = (float)shadowOptions.ShadowMapResolution;
	viewport.Height = (float)shadowOptions.ShadowMapResolution;
	viewport.MinDepth = 0.0f;
	viewport.MaxDepth = 1.0f;
	Graphics::Context->RSSetViewports(1, &viewport);

	shadowVS->SetShader();
	shadowVS->SetMatrix4x4("viewMatrix", shadowOptions.ShadowViewMatrix);
	shadowVS->SetMatrix4x4("projectionMatrix", shadowOptions.ShadowProjectionMatrix);
	Graphics::Context->PSSetShader(0, 0, 0);
	// Loop and draw all entities
	for (auto& e : entities)
	{
		shadowVS->SetMatrix4x4("worldMatrix", e->GetTransform()->GetWorldMatrix());
		shadowVS->CopyAllBufferData();
		e->GetMesh()->Draw();
	}

	//draw shadows for lights
	//from chris
/*for (int i = 0; i <lights.size(); i++)
	{
		// Only drawing point lights here
		Light light = lights[i];
		if (light.Type != LIGHT_TYPE_POINT)
			continue;

		// Calc quick scale based on range
		float scale = light.Range * light.Range / 200.0f;
		XMMATRIX scaleMat = XMMatrixScaling(scale, scale, scale);
		XMMATRIX transMat = XMMatrixTranslation(light.Position.x, light.Position.y, light.Position.z);

		// Make the transform for this light
		XMFLOAT4X4 world;
		XMStoreFloat4x4(&world, scaleMat * transMat);
		shadowVS->SetMatrix4x4("worldMatrix", world);
		shadowVS->CopyAllBufferData();

		pointLightMesh->Draw();
	}*/

	Graphics::Context->OMSetRenderTargets(1, Graphics::BackBufferRTV.GetAddressOf(), Graphics::DepthBufferDSV.Get());
	viewport.Width = (float)Window::Width();
	viewport.Height = (float)Window::Height();
	Graphics::Context->RSSetViewports(1, &viewport);
	Graphics::Context->RSSetState(0);
}

void Game::CreateShadowMap()
{
	shadowOptions.ShadowDSV.Reset();
	shadowOptions.ShadowSRV.Reset();
	shadowSampler.Reset();
	shadowRasterizer.Reset();

	// Create the actual texture that will be the shadow map
	D3D11_TEXTURE2D_DESC shadowDesc = {};
	shadowDesc.Width = shadowOptions.ShadowMapResolution; // Ideally a power of 2 (like 1024)
	shadowDesc.Height = shadowOptions.ShadowMapResolution; // Ideally a power of 2 (like 1024)
	shadowDesc.ArraySize = 1;
	shadowDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL | D3D11_BIND_SHADER_RESOURCE;
	shadowDesc.CPUAccessFlags = 0;
	shadowDesc.Format = DXGI_FORMAT_R32_TYPELESS;
	shadowDesc.MipLevels = 1;
	shadowDesc.MiscFlags = 0;
	shadowDesc.SampleDesc.Count = 1;
	shadowDesc.SampleDesc.Quality = 0;
	shadowDesc.Usage = D3D11_USAGE_DEFAULT;
	Microsoft::WRL::ComPtr<ID3D11Texture2D> shadowTexture;
	Graphics::Device->CreateTexture2D(&shadowDesc, 0, shadowTexture.GetAddressOf());

	D3D11_DEPTH_STENCIL_VIEW_DESC shadowDSDesc = {};
	shadowDSDesc.Format = DXGI_FORMAT_D32_FLOAT;
	shadowDSDesc.ViewDimension = D3D11_DSV_DIMENSION_TEXTURE2D;
	shadowDSDesc.Texture2D.MipSlice = 0;
	Graphics::Device->CreateDepthStencilView(shadowTexture.Get(), &shadowDSDesc, shadowOptions.ShadowDSV.GetAddressOf());

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
	srvDesc.Format = DXGI_FORMAT_R32_FLOAT;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MipLevels = 1;
	srvDesc.Texture2D.MostDetailedMip = 0;
	Graphics::Device->CreateShaderResourceView(shadowTexture.Get(), &srvDesc, shadowOptions.ShadowSRV.GetAddressOf());

	D3D11_RASTERIZER_DESC shadowRastDesc = {};
	shadowRastDesc.FillMode = D3D11_FILL_SOLID;
	shadowRastDesc.CullMode = D3D11_CULL_BACK;
	shadowRastDesc.DepthClipEnable = true;
	shadowRastDesc.DepthBias = 1000; // Min. precision units, not world units!
	shadowRastDesc.SlopeScaledDepthBias = 1.0f; // Bias more based on slope
	Graphics::Device->CreateRasterizerState(&shadowRastDesc, &shadowRasterizer);

	D3D11_SAMPLER_DESC shadowSampDesc = {};
	shadowSampDesc.Filter = D3D11_FILTER_COMPARISON_MIN_MAG_MIP_LINEAR;
	shadowSampDesc.ComparisonFunc = D3D11_COMPARISON_LESS;
	shadowSampDesc.AddressU = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.AddressV = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.AddressW = D3D11_TEXTURE_ADDRESS_BORDER;
	shadowSampDesc.BorderColor[0] = 1.0f; // Only need the first component
	Graphics::Device->CreateSamplerState(&shadowSampDesc, &shadowSampler);

	float sceneCenterOffset = 20.0f;
	XMVECTOR lightDir = (XMLoadFloat3(&lights[0].Direction));
	XMVECTOR lightPosition = XMVectorScale(lightDir, -sceneCenterOffset);


	shView = XMMatrixLookAtLH(
		lightPosition,
		lightDir,
		XMVectorSet(0, 1, 0, 0));
	XMStoreFloat4x4(&shadowOptions.ShadowViewMatrix, shView);

	XMMATRIX shProj = XMMatrixOrthographicLH(shadowOptions.ShadowProjectionSize, shadowOptions.ShadowProjectionSize, 0.1f, 100.0f);
	XMStoreFloat4x4(&shadowOptions.ShadowProjectionMatrix, shProj);


}

//demo from Chris to show how to draw light sources
void Game::DrawLightSources()
{
	// Turn on these shaders
	vertexShader->SetShader();
	normalPSShader->SetShader();

	// Set up vertex shader
	vertexShader->SetMatrix4x4("viewMatrix", cameras[activeCameraIndex]->GetViewMatrix());
	vertexShader->SetMatrix4x4("projectionMatrix", cameras[activeCameraIndex]->GetProjMatrix());

	for (int i = 0; i < lights.size(); i++)
	{
		Light light = lights[i];

		// Only drawing point lights here
		if (light.Type != LIGHT_TYPE_POINT)
			continue;

		// Calc quick scale based on range
		float scale = light.Range * light.Range / 200.0f;

		XMMATRIX scaleMat = XMMatrixScaling(scale, scale, scale);
		XMMATRIX transMat = XMMatrixTranslation(light.Position.x, light.Position.y, light.Position.z);

		// Make the transform for this light
		XMFLOAT4X4 world;
		XMStoreFloat4x4(&world, scaleMat * transMat);

		// Set up the world matrix for this light
		vertexShader->SetMatrix4x4("worldMatrix", world);

		// Set up the pixel shader data
		XMFLOAT3 finalColor = light.Color;
		finalColor.x *= light.Intensity;
		finalColor.y *= light.Intensity;
		finalColor.z *= light.Intensity;
		normalPSShader->SetFloat3("colorTint", finalColor);

		// Copy data
		vertexShader->CopyAllBufferData();
		normalPSShader->CopyAllBufferData();

		// Draw
		pointLightMesh->Draw();
	}

}

void Game::UpdateShView()
{
	float sceneCenterOffset = 25.0f;
	XMVECTOR lightDir = (XMLoadFloat3(&lights[0].Direction));
	XMVECTOR lightPosition = XMVectorScale(lightDir, -sceneCenterOffset);

	shView = XMMatrixLookAtLH(
		lightPosition,
		lightDir,
		XMVectorSet(0, 1, 0, 0));
	XMStoreFloat4x4(&shadowOptions.ShadowViewMatrix, shView);
}

void Game::PostProcess()
{
	// Describe the texture we're creating
	D3D11_TEXTURE2D_DESC textureDesc = {};
	textureDesc.Width = Window::Width();
	textureDesc.Height = Window::Height();
	textureDesc.ArraySize = 1;
	textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
	textureDesc.CPUAccessFlags = 0;
	textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	textureDesc.MipLevels = 1;
	textureDesc.MiscFlags = 0;
	textureDesc.SampleDesc.Count = 1;
	textureDesc.SampleDesc.Quality = 0;
	textureDesc.Usage = D3D11_USAGE_DEFAULT;
	// Create the resource (no need to track it after the views are created below)
	Microsoft::WRL::ComPtr<ID3D11Texture2D> ppTexture;
	Graphics::Device->CreateTexture2D(&textureDesc, 0, ppTexture.GetAddressOf());

	// Create the Render Target View
	D3D11_RENDER_TARGET_VIEW_DESC rtvDesc = {};
	rtvDesc.Format = textureDesc.Format;
	rtvDesc.Texture2D.MipSlice = 0;
	rtvDesc.ViewDimension = D3D11_RTV_DIMENSION_TEXTURE2D;
	Graphics::Device->CreateRenderTargetView(
		ppTexture.Get(),
		&rtvDesc,
		ppRTV.ReleaseAndGetAddressOf());
	// Create the Shader Resource View
	// By passing it a null description for the SRV, we
	// get a "default" SRV that has access to the entire resource
	Graphics::Device->CreateShaderResourceView(
		ppTexture.Get(),
		0,
		ppSRV.ReleaseAndGetAddressOf());
}


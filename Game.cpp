#include "Game.h"
#include "Graphics.h"
#include "Vertex.h"
#include "Input.h"
#include "PathHelpers.h"
#include "Window.h"
#include "BufferStructs.h"
#include <stdlib.h>
#include "Raytracing.h"

#include <DirectXMath.h>
#include <time.h> 

// Needed for a helper function to load pre-compiled shader files
#pragma comment(lib, "d3dcompiler.lib")
#include <d3dcompiler.h>

// For the DirectX Math library
using namespace DirectX;

//random method from demo for convenience
#define Random(min, max) (float)rand() / RAND_MAX * (max - min) + min


// --------------------------------------------------------
// Called once per program, after the window and graphics API
// are initialized but before the game loop begins
// --------------------------------------------------------
void Game::Initialize()
{
	srand((unsigned int)time(0));

	// Initialize raytracing
	RayTracing::Initialize(
		Window::Width(),
		Window::Height(),
		FixPath(L"RayTracing.cso"));

	maxShapes = 50;
	lightCount = 32;
	CreateLights();
	camera = std::make_shared<Camera>(XMFLOAT3(0, 0, -20), Window::AspectRatio(), XM_PIDIV4);
	
	materials.push_back( std::make_shared<Material>(pipelineState, XMFLOAT3(0.5f, 0.5f, 0.5f)));
	materials.push_back(std::make_shared<Material>(pipelineState, XMFLOAT3(1,0,0)));

	cubeMesh = std::make_shared<Mesh>("Cube", FixPath(L"../../Assets/Meshes/cube.obj").c_str());
	sphereMesh = std::make_shared<Mesh>("Sphere", FixPath(L"../../Assets/Meshes/sphere.obj").c_str());
	helixMesh = std::make_shared<Mesh>("Helix", FixPath(L"../../Assets/Meshes/helix.obj").c_str());

	
	CreateGeometry();



	// Create a BLAS for a single mesh, then the TLAS for our “scene”
	RayTracing::CreateTopLevelAccelerationStructureForScene(entities);
	// Finalize any initialization and wait for the GPU
	// before proceeding to the game loop
	Graphics::CloseAndExecuteCommandList();
	Graphics::WaitForGPU();
	Graphics::ResetAllocatorAndCommandList();
}

// --------------------------------------------------------
// Clean up memory or objects created by this class
// 
// Note: Using smart pointers means there probably won't
//       be much to manually clean up here!
// --------------------------------------------------------
Game::~Game()
{
	// Wait for the GPU before we shut down
	Graphics::WaitForGPU();
}

// --------------------------------------------------------
// Creates the geometry we're going to draw
// --------------------------------------------------------
void Game::CreateGeometry()
{
	std::shared_ptr<GameEntity> floor = std::make_shared<GameEntity>(cubeMesh, materials[0]);
	floor->GetTransform()->SetScale(15, 15, 15);
	floor->GetTransform()->SetPosition(0, -25, 0);
	entities.push_back(floor);

	std::shared_ptr<GameEntity> helix = std::make_shared<GameEntity>(helixMesh, materials[1]);
	helix->GetTransform()->SetScale(1, 2, 1);
	helix->GetTransform()->SetPosition(0, 3, 0);
	entities.push_back(helix);

	for (int i = 0; i < maxShapes; i++)
	{
		std::shared_ptr<Material> mat = std::make_shared<Material>(pipelineState, XMFLOAT3(
			Random(0.0f, 1.0f),
			Random(0.0f, 1.0f),
			Random(0.0f, 1.0f)));

		float scale = Random(0.25f, 1.0f);

		std::shared_ptr<GameEntity> sphere = std::make_shared<GameEntity>(sphereMesh, mat);
		sphere->GetTransform()->SetScale(scale, scale, scale);
		sphere->GetTransform()->SetPosition(
			Random(-10, 10), Random(-10, 10), Random(-10, 10));
		entities.push_back(sphere);

		std::shared_ptr<GameEntity> cube = std::make_shared<GameEntity>(cubeMesh, mat);
		cube->GetTransform()->SetScale(scale, scale, scale);
		cube->GetTransform()->SetPosition(
			Random(-10, 10), Random(-10, 10), Random(-10, 10));

		entities.push_back(cube);
	}
}

void Game::CreateLights()
{
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

	// Add all of the lights to the list
	lights.push_back(dirLight1);
	lights.push_back(dirLight2);
	lights.push_back(dirLight3);
	//lights.push_back(pointLight1);
	//lights.push_back(pointLight2);

	while (lights.size() < MAX_LIGHTS)
	{
		Light point = {};
		point.Color = XMFLOAT3(Random(0, 1), Random(0, 1), Random(0, 1));
		point.Type = LIGHT_TYPE_POINT;
		point.Intensity = Random(0.1f, 5.0f);
		point.Position = XMFLOAT3(Random(-15.0f, 15.0f), Random(-2.0f, 5.0f), Random(-15.0f, 15.0f));
		point.Range = Random(3.0f, 15.0f);

		lights.push_back(point);
	}
	lights.resize(MAX_LIGHTS);
}


// --------------------------------------------------------
// Handle resizing to match the new window size
//  - Eventually, we'll want to update our 3D camera
// --------------------------------------------------------
void Game::OnResize()
{
	if (camera)
	{
		camera->UpdateProjMatrix(Window::AspectRatio());
	}
	// Resize raytracing output texture
	RayTracing::ResizeOutputUAV(Window::Width(), Window::Height());
}


// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{

	for (int i = 0; i < entities.size(); i++)
	{
		if (i != 0)
		{
			entities[i]->GetTransform()->Rotate(0.5f * deltaTime, deltaTime, 0);


			// Calculate the new scale using a sine wave function with a time offset
			float timeOffset = i * 0.1f; // Adjust the offset as needed
			float scale = 0.5f + 0.5f * sin(totalTime * 2.0f + timeOffset); // Adjust the frequency as needed
			scale = max(scale, 0.1f); // Ensure the scale does not go below 0.1
			entities[i]->GetTransform()->SetScale(scale, scale, scale);
		}
	}
	camera->Update(deltaTime);
	// Example input checking: Quit if the escape key is pressed
	if (Input::KeyDown(VK_ESCAPE))
		Window::Quit();
}


// --------------------------------------------------------
// Clear the screen, redraw everything, present to the user
// --------------------------------------------------------
void Game::Draw(float deltaTime, float totalTime)
{
	// Grab the current back buffer for this frame
	Microsoft::WRL::ComPtr<ID3D12Resource> currentBackBuffer =
		Graphics::BackBuffers[Graphics::SwapChainIndex()];

	RayTracing::CreateTopLevelAccelerationStructureForScene(entities);
	// Perform ray trace (which also copies the results to the back buffer)
	RayTracing::Raytrace(camera, currentBackBuffer);

	{
		Graphics::CloseAndExecuteCommandList();
		// Present the current back buffer and move to the next one
		bool vsync = Graphics::VsyncState();
		Graphics::SwapChain->Present(
			vsync ? 1 : 0,
			vsync ? 0 : DXGI_PRESENT_ALLOW_TEARING);
		Graphics::AdvanceSwapChainIndex();

		// Wait for the GPU to be done and then reset the command list & allocator
		Graphics::WaitForGPU();
		Graphics::ResetAllocatorAndCommandList();
	}
}

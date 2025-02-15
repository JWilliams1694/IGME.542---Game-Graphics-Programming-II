#include "Game.h"
#include "Graphics.h"
#include "Vertex.h"
#include "Input.h"
#include "PathHelpers.h"
#include "Window.h"
#include "BufferStructs.h"
#include <stdlib.h>

#include <DirectXMath.h>

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
	lightCount = 32;
	CreateLights();
	camera = std::make_shared<Camera>(XMFLOAT3(0, 0, -20), Window::AspectRatio(), XM_PIDIV4);
	CreateRootSigAndPipelineState();
	CreateGeometry();
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
	//copy pasted from demo to save time 
	// Load textures
	D3D12_CPU_DESCRIPTOR_HANDLE cobblestoneAlbedo = Graphics::LoadTexture(FixPath(L"../../Assets/Textures/PBR/cobblestone_albedo.png").c_str());
	D3D12_CPU_DESCRIPTOR_HANDLE cobblestoneNormals = Graphics::LoadTexture(FixPath(L"../../Assets/Textures/PBR/cobblestone_normals.png").c_str());
	D3D12_CPU_DESCRIPTOR_HANDLE cobblestoneRoughness = Graphics::LoadTexture(FixPath(L"../../Assets/Textures/PBR/cobblestone_roughness.png").c_str());
	D3D12_CPU_DESCRIPTOR_HANDLE cobblestoneMetal = Graphics::LoadTexture(FixPath(L"../../Assets/Textures/PBR/cobblestone_metal.png").c_str());

	D3D12_CPU_DESCRIPTOR_HANDLE bronzeAlbedo = Graphics::LoadTexture(FixPath(L"../../Assets/Textures/PBR/bronze_albedo.png").c_str());
	D3D12_CPU_DESCRIPTOR_HANDLE bronzeNormals = Graphics::LoadTexture(FixPath(L"../../Assets/Textures/PBR/bronze_normals.png").c_str());
	D3D12_CPU_DESCRIPTOR_HANDLE bronzeRoughness = Graphics::LoadTexture(FixPath(L"../../Assets/Textures/PBR/bronze_roughness.png").c_str());
	D3D12_CPU_DESCRIPTOR_HANDLE bronzeMetal = Graphics::LoadTexture(FixPath(L"../../Assets/Textures/PBR/bronze_metal.png").c_str());

	D3D12_CPU_DESCRIPTOR_HANDLE scratchedAlbedo = Graphics::LoadTexture(FixPath(L"../../Assets/Textures/PBR/scratched_albedo.png").c_str());
	D3D12_CPU_DESCRIPTOR_HANDLE scratchedNormals = Graphics::LoadTexture(FixPath(L"../../Assets/Textures/PBR/scratched_normals.png").c_str());
	D3D12_CPU_DESCRIPTOR_HANDLE scratchedRoughness = Graphics::LoadTexture(FixPath(L"../../Assets/Textures/PBR/scratched_roughness.png").c_str());
	D3D12_CPU_DESCRIPTOR_HANDLE scratchedMetal = Graphics::LoadTexture(FixPath(L"../../Assets/Textures/PBR/scratched_metal.png").c_str());


	std::shared_ptr<Mesh> cubeMesh = std::make_shared<Mesh>("Cube", FixPath(L"../../Assets/Meshes/cube.obj").c_str());
	std::shared_ptr<Mesh> sphereMesh = std::make_shared<Mesh>("Sphere", FixPath(L"../../Assets/Meshes/sphere.obj").c_str());
	std::shared_ptr<Mesh> helixMesh = std::make_shared<Mesh>("Helix", FixPath(L"../../Assets/Meshes/helix.obj").c_str());
	std::shared_ptr<Mesh> torusMesh = std::make_shared<Mesh>("Torus", FixPath(L"../../Assets/Meshes/torus.obj").c_str());
	std::shared_ptr<Mesh> cylinderMesh = std::make_shared<Mesh>("Cylinder", FixPath(L"../../Assets/Meshes/cylinder.obj").c_str());

	// Create materials
	//taken from demo to save time
	std::shared_ptr<Material> cobbleMat = std::make_shared<Material>(pipelineState, XMFLOAT3(1, 1, 1));
	cobbleMat->AddTexture(cobblestoneAlbedo, 0);
	cobbleMat->AddTexture(cobblestoneNormals, 1);
	cobbleMat->AddTexture(cobblestoneRoughness, 2);
	cobbleMat->AddTexture(cobblestoneMetal, 3);
	cobbleMat->FinalizeMaterial();

	std::shared_ptr<Material> bronzeMat = std::make_shared<Material>(pipelineState, XMFLOAT3(1, 1, 1));
	bronzeMat->AddTexture(bronzeAlbedo, 0);
	bronzeMat->AddTexture(bronzeNormals, 1);
	bronzeMat->AddTexture(bronzeRoughness, 2);
	bronzeMat->AddTexture(bronzeMetal, 3);
	bronzeMat->FinalizeMaterial();

	std::shared_ptr<Material> scratchedMat = std::make_shared<Material>(pipelineState, XMFLOAT3(1, 1, 1));
	scratchedMat->AddTexture(scratchedAlbedo, 0);
	scratchedMat->AddTexture(scratchedNormals, 1);
	scratchedMat->AddTexture(scratchedRoughness, 2);
	scratchedMat->AddTexture(scratchedMetal, 3);
	scratchedMat->FinalizeMaterial();


	meshes.push_back(cubeMesh);
	meshes.push_back(sphereMesh);
	meshes.push_back(helixMesh);
	meshes.push_back(torusMesh);
	meshes.push_back(cylinderMesh);

	entities.push_back(std::make_shared<GameEntity>(meshes[0], cobbleMat));
	entities.push_back(std::make_shared<GameEntity>(meshes[1], bronzeMat));
	entities.push_back(std::make_shared<GameEntity>(meshes[2], scratchedMat));
	entities.push_back(std::make_shared<GameEntity>(meshes[3], cobbleMat));
	entities.push_back(std::make_shared<GameEntity>(meshes[4], bronzeMat));

	entities[0]->GetTransform()->SetPosition(-6, 0, 0);
	entities[1]->GetTransform()->SetPosition(-3, 0, 0);
	entities[2]->GetTransform()->SetPosition(0, 0, 0);
	entities[3]->GetTransform()->SetPosition(3, 0, 0);
	entities[4]->GetTransform()->SetPosition(6, 0, 0);
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

	// Resize the viewport and scissor rectangle
	{
		// Set up the viewport so we render into the correct
		// portion of the render target
		viewport = {};
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = (float)Window::Width();
		viewport.Height = (float)Window::Height();
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		// Define a scissor rectangle that defines a portion of
		// the render target for clipping. This is different from
		// a viewport in that it is applied after the pixel shader.
		// We need at least one of these, but we're rendering to
		// the entire window, so it'll be the same size.
		scissorRect = {};
		scissorRect.left = 0;
		scissorRect.top = 0;
		scissorRect.right = Window::Width();
		scissorRect.bottom = Window::Height();
	}
}


// --------------------------------------------------------
// Update your game here - user input, move objects, AI, etc.
// --------------------------------------------------------
void Game::Update(float deltaTime, float totalTime)
{
	for (auto& entity : entities)
	{
		entity->GetTransform()->Rotate(0.5f * deltaTime, deltaTime, 0);
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
	// Clearing the render target
	{
		// Transition the back buffer from present to render target
		D3D12_RESOURCE_BARRIER rb = {};
		rb.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		rb.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		rb.Transition.pResource = currentBackBuffer.Get();
		rb.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
		rb.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
		rb.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		Graphics::CommandList->ResourceBarrier(1, &rb);
		// Background color (Cornflower Blue in this case) for clearing
		float color[] = { 0.4f, 0.6f, 0.75f, 1.0f };
		// Clear the RTV
		Graphics::CommandList->ClearRenderTargetView(
			Graphics::RTVHandles[Graphics::SwapChainIndex()],
			color,
			0, 0); // No scissor rectangles
		// Clear the depth buffer, too
		Graphics::CommandList->ClearDepthStencilView(
			Graphics::DSVHandle,
			D3D12_CLEAR_FLAG_DEPTH,
			1.0f, // Max depth = 1.0f
			0, // Not clearing stencil, but need a value
			0, 0); // No scissor rects
	}
	// Rendering here!
	{
		// Set overall pipeline state
		Graphics::CommandList->SetPipelineState(pipelineState.Get());
		// Root sig (must happen before root descriptor table)
		Graphics::CommandList->SetGraphicsRootSignature(rootSignature.Get());
		Graphics::CommandList->SetDescriptorHeaps(1, Graphics::CBVSRVDescriptorHeap.GetAddressOf());

		// Set up other commands for rendering
		Graphics::CommandList->OMSetRenderTargets(
			1, &Graphics::RTVHandles[Graphics::SwapChainIndex()], true, &Graphics::DSVHandle);
		Graphics::CommandList->RSSetViewports(1, &viewport);
		Graphics::CommandList->RSSetScissorRects(1, &scissorRect);
		Graphics::CommandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		for (auto& entity : entities)
		{
			// grab the current entity’s material
			std::shared_ptr<Material> mat = entity->GetMaterial();
			//set the pipeline state this material intends to use
			Graphics::CommandList->SetPipelineState(mat->GetPipelineState().Get());
			// Set the SRV descriptor handle for this material's textures
			// Note: This assumes that descriptor table 2 is for textures (as per our root sig)
			Graphics::CommandList->SetGraphicsRootDescriptorTable(2, mat->GetFinalGPUHandleForSRVs());

			//Fill out a VertexShaderExternalData struct with the entity’s world matrix and the camera’s matrices
			VertexShaderExternalData vsData = {};
			vsData.world = entity->GetTransform()->GetWorldMatrix();
			vsData.worldInvTranspose = entity->GetTransform()->GetWorldInverseTransposeMatrix();
			vsData.view = camera->GetViewMatrix();
			vsData.projection = camera->GetProjMatrix();

			//Use FillNextConstantBufferAndGetGPUDescriptorHandle() to copy the above struct to the GPU
			// and get back the corresponding handle to the constant buffer view.
			D3D12_GPU_DESCRIPTOR_HANDLE cbHandle = Graphics::FillNextConstantBufferAndGetGPUDescriptorHandle((void*)(&vsData), sizeof(VertexShaderExternalData));

			//Use commandList->SetGraphicsRootDescriptorTable(0, handle) to set the handle from the previous line.
			Graphics::CommandList->SetGraphicsRootDescriptorTable(0, cbHandle);

			// Pixel shader data and cbuffer setup
			{
				PixelShaderExternalData psData = {};
				psData.uvScale = mat->GetUVScale();
				psData.uvOffset = mat->GetUVOffset();
				psData.cameraPosition = camera->GetTransform()->GetPosition();
				psData.lightCount = lightCount;
				memcpy(psData.lights, &lights[0], sizeof(Light) * MAX_LIGHTS);
				// Send this to a chunk of the constant buffer heap
				// and grab the GPU handle for it so we can set it for this draw
				D3D12_GPU_DESCRIPTOR_HANDLE cbHandlePS =
					Graphics::FillNextConstantBufferAndGetGPUDescriptorHandle(
						(void*)(&psData), sizeof(PixelShaderExternalData));
				// Set this constant buffer handle
				// Note: This assumes that descriptor table 1 is the
				// place to put this particular descriptor. This
				// is based on how we set up our root signature.
				Graphics::CommandList->SetGraphicsRootDescriptorTable(
					1, cbHandlePS);
			}

			//Grab the vertex buffer view and index buffer view from this entity’s mesh
			std::shared_ptr<Mesh> mesh = entity->GetMesh();
			D3D12_VERTEX_BUFFER_VIEW vbv = mesh->GetVertexBuffer();
			D3D12_INDEX_BUFFER_VIEW  ibv = mesh->GetIndexBuffer();

			//Set them using IASetVertexBuffers() and IASetIndexBuffer()
			Graphics::CommandList->IASetVertexBuffers(0, 1, &vbv);
			Graphics::CommandList->IASetIndexBuffer(&ibv);

			//Call DrawIndexedInstanced() using the index count of this entity’s mesh
			Graphics::CommandList->DrawIndexedInstanced((UINT)mesh->GetIndexCount(), 1, 0, 0, 0);
		}
	}
	// Present
	{
		// Transition back to present
		D3D12_RESOURCE_BARRIER rb = {};
		rb.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		rb.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		rb.Transition.pResource = currentBackBuffer.Get();
		rb.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
		rb.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
		rb.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		Graphics::CommandList->ResourceBarrier(1, &rb);
		// Must occur BEFORE present
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

// --------------------------------------------------------
// Loads the two basic shaders, then creates the root signature
// and pipeline state object for our very basic demo.
// --------------------------------------------------------
void Game::CreateRootSigAndPipelineState()
{
	// Blobs to hold raw shader byte code used in several steps below
	Microsoft::WRL::ComPtr<ID3DBlob> vertexShaderByteCode;
	Microsoft::WRL::ComPtr<ID3DBlob> pixelShaderByteCode;
	// Load shaders
	{
		// Read our compiled vertex shader code into a blob
		// - Essentially just "open the file and plop its contents here"
		D3DReadFileToBlob(
			FixPath(L"VertexShader.cso").c_str(), vertexShaderByteCode.GetAddressOf());
		D3DReadFileToBlob(
			FixPath(L"PixelShader.cso").c_str(), pixelShaderByteCode.GetAddressOf());
	}
	// Input layout
	const unsigned int inputElementCount = 4;
	D3D12_INPUT_ELEMENT_DESC inputElements[inputElementCount] = {};
	{
		inputElements[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		inputElements[0].Format = DXGI_FORMAT_R32G32B32_FLOAT; // R32 G32 B32 = float3
		inputElements[0].SemanticName = "POSITION"; // Name must match semantic in shader
		inputElements[0].SemanticIndex = 0; // This is the first POSITION semantic
		inputElements[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		inputElements[1].Format = DXGI_FORMAT_R32G32_FLOAT; // R32 G32 = float2
		inputElements[1].SemanticName = "TEXCOORD";
		inputElements[1].SemanticIndex = 0; // This is the first TEXCOORD semantic
		inputElements[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		inputElements[2].Format = DXGI_FORMAT_R32G32B32_FLOAT; // R32 G32 B32 = float3
		inputElements[2].SemanticName = "NORMAL";
		inputElements[2].SemanticIndex = 0; // This is the first NORMAL semantic
		inputElements[3].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
		inputElements[3].Format = DXGI_FORMAT_R32G32B32_FLOAT; // R32 G32 B32 = float3
		inputElements[3].SemanticName = "TANGENT";
		inputElements[3].SemanticIndex = 0; // This is the first TANGENT semantic
	}
	// Root Signature
	{
		// Describe the range of CBVs needed for the vertex shader
		D3D12_DESCRIPTOR_RANGE cbvRangeVS = {};
		cbvRangeVS.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		cbvRangeVS.NumDescriptors = 1;
		cbvRangeVS.BaseShaderRegister = 0;
		cbvRangeVS.RegisterSpace = 0;
		cbvRangeVS.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
		// Describe the range of CBVs needed for the pixel shader
		D3D12_DESCRIPTOR_RANGE cbvRangePS = {};
		cbvRangePS.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
		cbvRangePS.NumDescriptors = 1;
		cbvRangePS.BaseShaderRegister = 0;
		cbvRangePS.RegisterSpace = 0;
		cbvRangePS.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
		// Create a range of SRV's for textures
		D3D12_DESCRIPTOR_RANGE srvRange = {};
		srvRange.RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
		srvRange.NumDescriptors = 4; // Set to max number of textures at once (match pixel shader!)
		srvRange.BaseShaderRegister = 0; // Starts at s0 (match pixel shader!)
		srvRange.RegisterSpace = 0;
		srvRange.OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;
		// Create the root parameters
		D3D12_ROOT_PARAMETER rootParams[3] = {};
		// CBV table param for vertex shader
		rootParams[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParams[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
		rootParams[0].DescriptorTable.NumDescriptorRanges = 1;
		rootParams[0].DescriptorTable.pDescriptorRanges = &cbvRangeVS;
		// CBV table param for pixel shader
		rootParams[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParams[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		rootParams[1].DescriptorTable.NumDescriptorRanges = 1;
		rootParams[1].DescriptorTable.pDescriptorRanges = &cbvRangePS;
		// SRV table param
		rootParams[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
		rootParams[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		rootParams[2].DescriptorTable.NumDescriptorRanges = 1;
		rootParams[2].DescriptorTable.pDescriptorRanges = &srvRange;
		// Create a single static sampler (available to all pixel shaders at the same slot)
		D3D12_STATIC_SAMPLER_DESC anisoWrap = {};
		anisoWrap.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		anisoWrap.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		anisoWrap.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		anisoWrap.Filter = D3D12_FILTER_ANISOTROPIC;
		anisoWrap.MaxAnisotropy = 16;
		anisoWrap.MaxLOD = D3D12_FLOAT32_MAX;
		anisoWrap.ShaderRegister = 0; // register(s0)
		anisoWrap.ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
		D3D12_STATIC_SAMPLER_DESC samplers[] = { anisoWrap };
		// Describe the full root signature
		D3D12_ROOT_SIGNATURE_DESC rootSig = {};
		rootSig.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;
		rootSig.NumParameters = ARRAYSIZE(rootParams);
		rootSig.pParameters = rootParams;
		rootSig.NumStaticSamplers = ARRAYSIZE(samplers);
		rootSig.pStaticSamplers = samplers;

		ID3DBlob* serializedRootSig = 0;
		ID3DBlob* errors = 0;
		D3D12SerializeRootSignature(
			&rootSig,
			D3D_ROOT_SIGNATURE_VERSION_1,
			&serializedRootSig,
			&errors);
		// Check for errors during serialization
		if (errors != 0)
		{
			OutputDebugString((wchar_t*)errors->GetBufferPointer());
		}
		// Actually create the root sig
		Graphics::Device->CreateRootSignature(
			0,
			serializedRootSig->GetBufferPointer(),
			serializedRootSig->GetBufferSize(),
			IID_PPV_ARGS(rootSignature.GetAddressOf()));
	}
	// Pipeline state
	{
		// Describe the pipeline state
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		// -- Input assembler related ---
		psoDesc.InputLayout.NumElements = inputElementCount;
		psoDesc.InputLayout.pInputElementDescs = inputElements;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		// Overall primitive topology type (triangle, line, etc.) is set here
		// IASetPrimTop() is still used to set list/strip/adj options
		// Root sig
		psoDesc.pRootSignature = rootSignature.Get();
		// -- Shaders (VS/PS) ---
		psoDesc.VS.pShaderBytecode = vertexShaderByteCode->GetBufferPointer();
		psoDesc.VS.BytecodeLength = vertexShaderByteCode->GetBufferSize();
		psoDesc.PS.pShaderBytecode = pixelShaderByteCode->GetBufferPointer();
		psoDesc.PS.BytecodeLength = pixelShaderByteCode->GetBufferSize();
		// -- Render targets ---
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
		psoDesc.SampleDesc.Count = 1;
		psoDesc.SampleDesc.Quality = 0;
		// -- States ---
		psoDesc.RasterizerState.FillMode = D3D12_FILL_MODE_SOLID;
		psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_BACK;
		psoDesc.RasterizerState.DepthClipEnable = true;
		psoDesc.DepthStencilState.DepthEnable = true;
		psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
		psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
		psoDesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
		psoDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
		psoDesc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
		psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask =
			D3D12_COLOR_WRITE_ENABLE_ALL;
		// -- Misc ---
		psoDesc.SampleMask = 0xffffffff;
		// Create the pipe state object
		Graphics::Device->CreateGraphicsPipelineState(
			&psoDesc,
			IID_PPV_ARGS(pipelineState.GetAddressOf()));
	}
	// Set up the viewport and scissor rectangle
	{
		// Set up the viewport so we render into the correct
		// portion of the render target
		viewport = {};
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = (float)Window::Width();
		viewport.Height = (float)Window::Height();
		viewport.MinDepth = 0.0f;
		viewport.MaxDepth = 1.0f;
		// Define a scissor rectangle that defines a portion of
		// the render target for clipping. This is different from
		// a viewport in that it is applied after the pixel shader.
		// We need at least one of these, but we're rendering to
		// the entire window, so it'll be the same size.
		scissorRect = {};
		scissorRect.left = 0;
		scissorRect.top = 0;
		scissorRect.right = Window::Width();
		scissorRect.bottom = Window::Height();
	}
}
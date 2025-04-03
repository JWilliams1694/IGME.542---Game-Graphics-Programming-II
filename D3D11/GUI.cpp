#include "GUI.h"
#include <DirectXMath.h>
#include "Window.h"
#include "Input.h"
#include "GameEntity.h"
#include "Game.h"

#include "ImGui/imgui.h"
#include "ImGui/imgui_impl_dx11.h"
#include "ImGui/imgui_impl_win32.h"

using namespace DirectX;
bool showUIDemo = false;
float color[4] = { 0.4f, 0.6f, 0.75f, 0.0f };

void CreateUI(float deltaTime)
{// Put this all in a helper method that is called from Game::Update()
// Feed fresh data to ImGui
	ImGuiIO& io = ImGui::GetIO();
	io.DeltaTime = deltaTime;
	io.DisplaySize.x = (float)Window::Width();
	io.DisplaySize.y = (float)Window::Height();
	// Reset the frame
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
	// Determine new input capture
	Input::SetKeyboardCapture(io.WantCaptureKeyboard);
	Input::SetMouseCapture(io.WantCaptureMouse);
	// Show the demo window
	//ImGui::ShowDemoWindow();
}

void BuildUI(std::vector<std::shared_ptr<Mesh>> meshes,
			 std::vector<std::shared_ptr<GameEntity>>& entities,
			 int& activeCameraIndex,
			 std::vector<std::shared_ptr<Camera>> cameras,
			 std::vector<Light>& lights, ShadowOptions shadowOptions,
			 PostProcessOptions& postProcessOptions)
{
	ImGuiIO& io = ImGui::GetIO();
	struct funcs { static bool IsLegacyNativeDupe(ImGuiKey) { return false; } };
	ImGuiKey start_key = ImGuiKey_NamedKey_BEGIN;

	if (showUIDemo)
	{
		ImGui::ShowDemoWindow();
	}
	ImGui::Begin("Inspector");

	//window details
	if (ImGui::TreeNode("Details"))
	{
		// Replace the %f with the next parameter, and format as a float
		ImGui::Text("Framerate: %f fps", ImGui::GetIO().Framerate);
		// Replace each %d with the next parameter, and format as decimal integers
		// The "x" will be printed as-is between the numbers, like so: 800x600
		ImGui::Text("Window Resolution: %dx%d", Window::Width(), Window::Height());
		if (ImGui::Button("Show ImGui Demo Window"))
		{
			showUIDemo = !showUIDemo;
		}
		ImGui::TreePop();
	}

	//color stuff
	if (ImGui::TreeNode("Color"))
	{
		//float color[4] = { 0.4f, 0.6f, 0.75f, 0.0f };
		ImGui::ColorEdit4("Background Color", color);
		ImGui::TreePop();
	}

	//meshes
	if (ImGui::TreeNode("Meshes"))
	{
		for (int i = 0; i < meshes.size(); i++)
		{
			ImGui::PushID(meshes[i].get());
			if (ImGui::TreeNode("Mesh Node", "Mesh: %s", meshes[i]->GetName()))
			{
				ImGui::Text("Vertices: %d", meshes[i]->GetVertexCount());
				ImGui::Text("Indices: %d", meshes[i]->GetIndexCount());
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
		ImGui::TreePop();
	}
	// entities
	if (ImGui::TreeNode("Entities"))
	{
		for (int i = 0; i < entities.size(); i++)
		{
			ImGui::PushID(entities[i].get());
			if (ImGui::TreeNode("Entity Node", "Entity %d", i))
			{
				// Build UI for one entity at a time
				EntityUI(entities[i]);

				ImGui::TreePop();
			}
			ImGui::PopID();
		}

		// Finalize the tree node
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Camera"))
	{
		CameraUI(activeCameraIndex, cameras);
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Lights"))
	{
		for (int i = 0; i < lights.size(); i++)  // Adjust for the number of lights you have
		{
			ImGui::PushID(i); // Ensure unique ID for each light's UI section
			if (ImGui::TreeNode("Light Node", "Light %d", i))
			{
				// Change light color using ColorEdit3 for RGB values
				ImGui::ColorEdit3("Color", (float*)&lights[i].Color); // Edit light color

				// Optionally modify the intensity of the light as well
				ImGui::SliderFloat("Intensity", &lights[i].Intensity, 0.0f, 20.0f);
				if (lights[i].Type == LIGHT_TYPE_DIRECTIONAL)
				{
					ImGui::SliderFloat3(
						"Direction",
						&lights[i].Direction.x,
						-20.0f, 20.0f);
				}
				if (lights[i].Type == LIGHT_TYPE_POINT)
				{
					ImGui::SliderFloat3(
						"Position",
						&lights[i].Position.x,
						-30.0f, 30.0f);

					ImGui::SliderFloat("Range", &lights[i].Range, 0.0f, 30.0f);
				}
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Textures"))
	{
		for (int i = 0; i < entities.size(); i++)
		{
			ImGui::PushID(entities[i].get());
			if (ImGui::TreeNode("Textures Node", "Texture %d : %s", i, entities[i]->GetMaterial()->GetName()))
			{
				std::shared_ptr<Material> mat = entities[i]->GetMaterial();
				ImGui::Text("Material: %s", mat->GetName());
				ImGui::Text("Roughness: %f", mat->GetRoughness());
				ImGui::Text("Tint: %f, %f, %f, %f", mat->GetTint().x, mat->GetTint().y, mat->GetTint().z, mat->GetTint().w);
				DirectX::XMFLOAT2 uvScale = mat->GetUVScale();
				if (ImGui::SliderFloat2("UV Scale", &uvScale.x, -20.0f, 20.0f))
				{
					mat->SetUVScale(uvScale);
				}
				DirectX::XMFLOAT2 uvOffset = mat->GetUVOffset();
				if (ImGui::SliderFloat2("UV Offset", &uvOffset.x, -20.0f, 20.0f))
				{
					mat->SetUVOffset(uvOffset);
				}
				ImGui::TreePop();
			}
			ImGui::PopID();
		}
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Shadow"))
	{
		ImGui::Image(shadowOptions.ShadowSRV.Get(), ImVec2(512, 512));
		ImGui::TreePop();
	}
	if (ImGui::TreeNode("Post-Processing"))
	{
		if (ImGui::TreeNode("Blur Settings"))
		{
			ImGui::Checkbox("Enable Blur", &postProcessOptions.IsBlurred);
			if (postProcessOptions.IsBlurred)
			{
				ImGui::SliderInt("Blur Amount", &postProcessOptions.BlurAmount, 0, 10);
			}
			ImGui::TreePop();
		}
		if (ImGui::TreeNode("Fog Settings"))
		{
			// Fog Type Selector (Assuming FogType is an integer for different types)
			const char* fogTypes[] = { "Linear", "Smoothstep", "Exponential","None" };
			ImGui::Combo("Fog Type", &postProcessOptions.FogType, fogTypes, IM_ARRAYSIZE(fogTypes));

			// Fog Color
			ImGui::ColorEdit3("Fog Color", reinterpret_cast<float*>(&postProcessOptions.FogColor));

			// Fog Start and End Distances (for linear fog)
			if (postProcessOptions.FogType == 1)
			{
				ImGui::SliderFloat("Fog Start Distance", &postProcessOptions.FogStartDistance, 0.0f, 100.0f, "%.1f");
				ImGui::SliderFloat("Fog End Distance", &postProcessOptions.FogEndDistance, 0.0f, 200.0f, "%.1f");
			}
			// Fog Density (for exponential fog)
			if (postProcessOptions.FogType == 2)
			{
				ImGui::SliderFloat("Fog Density", &postProcessOptions.FogDensity, 0.04f, 1.0f, "%.02f");

			}
			// Height-Based Fog
			ImGui::Checkbox("Enable Height-Based Fog", &postProcessOptions.HeightBasedFog);
			if (postProcessOptions.HeightBasedFog)
			{
				ImGui::SliderFloat("Fog Height", &postProcessOptions.FogHeight, 1.0f, 100.0f, "%.1f");
			}
			ImGui::TreePop();
		}
		ImGui::TreePop();
	}
	ImGui::End();
}

void EntityUI(std::shared_ptr<GameEntity> entity)
{
	ImGui::Text("Mesh: %s", entity->GetMesh()->GetName());
	std::shared_ptr<Transform> trans = entity->GetTransform();
	XMFLOAT3 pos = trans->GetPosition();
	XMFLOAT3 rot = trans->GetRotation();
	XMFLOAT3 sca = trans->GetScale();


	if (ImGui::DragFloat3("Position", &pos.x, 0.01f)) trans->SetPosition(pos);
	if (ImGui::DragFloat3("Rotation", &rot.x, 0.01f)) trans->SetRotation(rot);
	if (ImGui::DragFloat3("Scale", &sca.x, 0.01f)) trans->SetScale(sca);
}

float* GetBackgroundColor()
{
	return color;
}

void CameraUI(int& activeCameraIndex, std::vector<std::shared_ptr<Camera>> cameras)
{
	static int i = 0;
	int j = 0;
	ImGui::Spacing();
	for (j = 0; j < cameras.size(); j++)
	{
		ImGui::PushID(j);
		ImGui::Text("Camera %d", j + 1); ImGui::SameLine();
		ImGui::RadioButton(" ", &i, j);
		ImGui::PopID();
	}
	activeCameraIndex = i;

	ImGui::Spacing();

	XMFLOAT3 position = cameras[activeCameraIndex]->GetTransform()->GetPosition();
	XMFLOAT3 rotation = cameras[activeCameraIndex]->GetTransform()->GetRotation();
	float aspectRatio = cameras[activeCameraIndex]->GetAspectRatio();
	float FOV = cameras[activeCameraIndex]->GetFOV() * 180.0f / XM_PI;

	if (ImGui::DragFloat3("Position", &position.x, 0.01f))
		cameras[activeCameraIndex]->GetTransform()->SetPosition(position);
	if (ImGui::DragFloat3("Rotation", &rotation.x, 0.01f))
		cameras[activeCameraIndex]->GetTransform()->SetRotation(rotation);
	if (ImGui::SliderFloat("FOV", &FOV, 0.1f, 180))
		cameras[activeCameraIndex]->SetFOV(FOV * XM_PI / 180.0f);
	cameras[activeCameraIndex]->UpdateProjMatrix(aspectRatio);

}
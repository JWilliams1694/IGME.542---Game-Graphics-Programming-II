#pragma once

#include <vector>
#include <memory>
#include "Camera.h"

#include "Mesh.h"
#include "GameEntity.h"
#include "Lights.h"

void CreateUI(float deltaTime);
void BuildUI(std::vector<std::shared_ptr<Mesh>> meshes,
			 std::vector<std::shared_ptr<GameEntity>>& entities,
			 int& activeCameraIndex,
			 std::vector<std::shared_ptr<Camera>> cameras,
			 std::vector<Light>& lights, ShadowOptions shadowOptions, PostProcessOptions& postProcessOptions);
void EntityUI(std::shared_ptr<GameEntity> entity);
void CameraUI(int& activeCameraIndex, std::vector<std::shared_ptr<Camera>> cameras);
float* GetBackgroundColor();

extern bool showUIDemo;
extern float color[4];
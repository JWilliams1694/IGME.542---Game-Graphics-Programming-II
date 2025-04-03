#pragma once
#include <memory>
#include "Transform.h"
#include "Mesh.h"
#include "Camera.h"
#include "Material.h"

class GameEntity
{
public:
	//constructor
	GameEntity(std::shared_ptr<Mesh> mesh,std::shared_ptr<Material> material);

	//getters
	std::shared_ptr<Transform> GetTransform();
	std::shared_ptr<Mesh> GetMesh();
	std::shared_ptr<Material> GetMaterial();

	//methods
	void Draw(std::shared_ptr<Camera> camera);

private:
	std::shared_ptr<Transform> transform;
	std::shared_ptr<Mesh> mesh;
	std::shared_ptr<Material> material;


	//shape color and offset
	DirectX::XMFLOAT4 colorTint = DirectX::XMFLOAT4(0.5f, 1.0f, 0.5f, 1.0f);
};


#include "GameEntity.h"

GameEntity::GameEntity(std::shared_ptr<Mesh> mesh)
{
}

std::shared_ptr<Mesh> GameEntity::GetMesh()
{
    return mesh;
}

std::shared_ptr<Transform> GameEntity::GetTransform()
{
    return transform;
}

void GameEntity::SetMesh(std::shared_ptr<Mesh> mesh)
{
    this->mesh = mesh;
}

void GameEntity::SetTransform(std::shared_ptr<Transform> transform)
{
    this->transform = transform;
}

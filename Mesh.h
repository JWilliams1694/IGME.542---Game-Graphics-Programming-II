#pragma once
#include <d3d11.h>
#include <wrl/client.h>
#include "Graphics.h"
#include "Vertex.h"
using Microsoft::WRL::ComPtr;

class Mesh
{
public:
	Mesh(const char* name, Vertex* vertices, unsigned int* indices, size_t  vertexNum, size_t  indexNum);
	Mesh(const char* name, const std::wstring& fileName);
	~Mesh();

	//methods
	ComPtr<ID3D11Buffer> GetVertexBuffer();
	ComPtr<ID3D11Buffer> GetIndexBuffer();
	const char* GetName();
	size_t GetIndexCount();
	size_t GetVertexCount();
	void Draw();
private:
	const char* name;
	//buffers
	ComPtr<ID3D11Buffer> vertexBuffer;
	ComPtr<ID3D11Buffer> indexBuffer;
	//counts
	size_t indexNum;
	size_t vertexNum;

	//method to create buffer for constructor
	void CreateBuffer(Vertex* vertices, unsigned int* indices, size_t  vertexNum, size_t  indexNum);
	void CalculateTangents(Vertex* verts, size_t numVerts, unsigned int* indices, size_t numIndices);
};


#pragma once
#include <d3d12.h>
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
	D3D12_VERTEX_BUFFER_VIEW GetVertexBuffer();
	D3D12_INDEX_BUFFER_VIEW GetIndexBuffer();
	const char* GetName();
	size_t GetIndexCount();
	size_t GetVertexCount();
private:
	const char* name;
	//buffers
	D3D12_VERTEX_BUFFER_VIEW vbView;
	D3D12_INDEX_BUFFER_VIEW ibView;
	ComPtr<ID3D12Resource> vertexBuffer;
	ComPtr<ID3D12Resource> indexBuffer;
	//counts
	size_t indexNum;
	size_t vertexNum;

	//method to create buffer for constructor
	void CreateBuffer(Vertex* vertices, unsigned int* indices, size_t  vertexNum, size_t  indexNum);
	void CalculateTangents(Vertex* verts, size_t numVerts, unsigned int* indices, size_t numIndices);
};


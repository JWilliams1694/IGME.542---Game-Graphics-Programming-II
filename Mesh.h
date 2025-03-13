#pragma once
#include <d3d12.h>
#include <wrl/client.h>
#include "Graphics.h"
#include "Vertex.h"
using Microsoft::WRL::ComPtr;

struct MeshRaytracingData
{
	D3D12_GPU_DESCRIPTOR_HANDLE IndexBufferSRV{ };
	D3D12_GPU_DESCRIPTOR_HANDLE VertexBufferSRV{ };
	Microsoft::WRL::ComPtr<ID3D12Resource> BLAS;
	unsigned int HitGroupIndex = 0;
};

class Mesh
{
public:
	Mesh(const char* name, Vertex* vertices, unsigned int* indices, size_t  vertexNum, size_t  indexNum);
	Mesh(const char* name, const std::wstring& fileName);
	~Mesh();

	//methods
	Microsoft::WRL::ComPtr<ID3D12Resource> GetVertexBuffer();
	Microsoft::WRL::ComPtr<ID3D12Resource> GetIndexBuffer();
	D3D12_VERTEX_BUFFER_VIEW GetVertexBufferView();
	D3D12_INDEX_BUFFER_VIEW GetIndexBufferView();
	const char* GetName();
	size_t GetIndexCount();
	size_t GetVertexCount();
	MeshRaytracingData GetRaytracingData();
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

	MeshRaytracingData raytracingData;

	//method to create buffer for constructor
	void CreateBuffer(Vertex* vertices, unsigned int* indices, size_t  vertexNum, size_t  indexNum);
	void CalculateTangents(Vertex* verts, size_t numVerts, unsigned int* indices, size_t numIndices);
};


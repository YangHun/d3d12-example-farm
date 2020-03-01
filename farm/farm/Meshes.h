#pragma once
#ifndef _MESHES_H
#define _MESHES_H

using namespace DirectX;
using Microsoft::WRL::ComPtr;

struct FVector3 {
public:
	float x;
	float y;
	float z;

public:
	FVector3() : x(0.0f), y(0.0f), z(0.0f) {}
	FVector3(float x, float y, float z) : x(x), y(y), z(z) { }
	FVector3(DirectX::XMFLOAT3 df) : x(df.x), y(df.y), z(df.z) { }

	bool operator==(const FVector3& rhs) const
	{
		return x == rhs.x && y == rhs.y && z == rhs.z;
	}
};

struct FVector2 {
public:
	float x;
	float y;

public:
	FVector2() : x(0.0f), y(0.0f) {}
	FVector2(float x, float y) : x(x), y(y) { }
	FVector2(DirectX::XMFLOAT2 df) : x(df.x), y(df.y) { }
	bool operator==(const FVector2& rhs) const
	{
		return x == rhs.x && y == rhs.y;
	}
};

struct Vertex
{
public:
	XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };
	XMFLOAT3 normal = { 0.0f, 0.0f, 0.0f };
	XMFLOAT2 uv = { 0.0f, 0.0f };
	XMFLOAT3 tangent = { 0.0f, 0.0f, 0.0f };
	//	XMFLOAT4 color = { 1.0f, 1.0f, 1.0f, 1.0f };

public:
	bool operator==(const Vertex& rhs) const
	{
		return FVector3(position) == FVector3(rhs.position)
			&& FVector3(normal) == FVector3(rhs.normal)
			&& FVector2(uv) == FVector2(rhs.uv)
			&& FVector3(tangent) == FVector3(rhs.tangent);
	}

};

struct Mesh {
	std::vector<Vertex> vertices;
	std::vector<uint16_t> indices;
};

struct Transform
{
	XMFLOAT3 position = { 0.0f, 0.0f, 0.0f };
	XMFLOAT3 rotation = { 0.0f, 0.0f, 0.0f };
	XMFLOAT3 scale = { 1.0f, 1.0f, 1.0f };
};

struct MeshDesc {
	D3D_PRIMITIVE_TOPOLOGY primitiveType = D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	Mesh* mesh = nullptr;
	UINT indexCount;
	UINT startIndexLocation = 0;
	int baseVertexLocation = 0;

	UINT constantBufferId = -1;

	//Transform transform;
	//XMFLOAT4X4 world;

	ComPtr<ID3D12Resource> vertexBuffer = nullptr;
	ComPtr<ID3D12Resource> indexBuffer = nullptr;
	ComPtr<ID3D12Resource> vertexUploadBuffer = nullptr;
	ComPtr<ID3D12Resource> indexUploadBuffer = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	D3D12_INDEX_BUFFER_VIEW indexBufferView;
	bool dirty = true;
};




// hash function

namespace std
{
	template<>
	struct hash<XMFLOAT2>
	{
	public:
		size_t operator()(const XMFLOAT2& t) const
		{
			return (std::hash<float>()(t.x)
				^ (std::hash<float>()(t.y) << 1));
		}
	};
	template<>
	struct hash<XMFLOAT3>
	{
	public:
		size_t operator()(const XMFLOAT3& t) const
		{
			return ((std::hash<float>()(t.x)
				^ (std::hash<float>()(t.y) << 1)) >> 1)
				^ (std::hash<float>()(t.z) << 1);
		}
	};
	template<>
	struct hash<Vertex>
	{
	public:
		size_t operator()(const Vertex& t) const
		{
			return ((std::hash<XMFLOAT3>()(t.position)
				^ (std::hash<XMFLOAT3>()(t.normal) << 1)) >> 1)
				^ (std::hash<XMFLOAT2>()(t.uv) << 1);;
		}
	};
}

#endif
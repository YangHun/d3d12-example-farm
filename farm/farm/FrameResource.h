#pragma once

using namespace DirectX;

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
//	XMFLOAT3 normal;
//	XMFLOAT2 uv;
//	XMFLOAT3 tangent;
	XMFLOAT4 color = { 1.0f, 1.0f, 1.0f, 1.0f };

public:
	bool operator==(const Vertex& rhs) const
	{
		return FVector3(position) == FVector3(rhs.position);
	}

};

struct Mesh {
	std::vector<Vertex> vertices;
	std::vector<uint16_t> indices;
};


// hash function

namespace std
{
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
	struct hash<XMFLOAT4>
	{
	public:
		size_t operator()(const XMFLOAT4& t) const
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
			return (std::hash<XMFLOAT3>()(t.position)
				^ (std::hash<XMFLOAT4>()(t.color) << 1));
		}
	};
}
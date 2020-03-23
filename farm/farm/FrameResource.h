#pragma once

#include "d3dHelper.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

enum class E_RenderLayer
{
	Opaque,
	Transparent,
	Sky,
	Count
};

struct Light
{
	XMFLOAT4 strength = { 0.5f, 0.5f, 0.5f, 0.0f};
	XMFLOAT4 direction = { 0.0f, 0.0f, 0.0f, 0.0f };
};

// Constant buffer size는 256의 배수가 되도록 padding 해준다.
// minimum hardware allocation size가 256이기 때문.
struct SceneConstantBuffer
{
	XMFLOAT4X4 viewproj;	// View Proj matrix
	XMFLOAT4 ambientLight;
	XMFLOAT4 eye;
	Light Lights[3];
};


struct ObjectConstantBuffer
{
	XMFLOAT4X4 model;	// Model matrix
	UINT matIndex;		// material index
};

struct ShadowPassConstantBuffer
{
	XMFLOAT4X4 lightViewProj;	// light-space View Proj materix.
	XMFLOAT4X4 NDC;				// normalized device transform matrix
};

struct MaterialBuffer
{
	XMFLOAT3 diffuseColor;
	UINT diffuseMapIndex;
};

class FrameResource
{
public:
	FrameResource(ID3D12Device* pDevice);
	~FrameResource();

	void PopulateCommandList();

	//void XM_CALLCONV UpdateConstantBuffers(FXMMATRIX view, CXMMATRIX proj);

	
public:
	ComPtr<ID3D12CommandAllocator> m_commandAllocator;
	ComPtr<ID3D12Resource> m_cbvUploadHeap;
	SceneConstantBuffer* m_pConstantBuffers;
	UINT64 m_fenceValue;
	   
};
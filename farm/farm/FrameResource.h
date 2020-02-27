#pragma once

#include "d3dHelper.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

// Constant buffer size는 256의 배수가 되도록 padding 해준다.
// minimum hardware allocation size가 256이기 때문.
struct SceneConstantBuffer
{
	XMFLOAT4X4 viewproj;	// View Proj matrix
};


struct ObjectConstantBuffer
{
	XMFLOAT4X4 model;	// Model matrix
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
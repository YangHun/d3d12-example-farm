#pragma once

#include "d3dHelper.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

// Constant buffer size�� 256�� ����� �ǵ��� padding ���ش�.
// minimum hardware allocation size�� 256�̱� ����.
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
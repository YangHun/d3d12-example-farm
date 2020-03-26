#include "stdafx.h"
#include "d3dHelper.h"
#include "FrameResource.h"

FrameResource::FrameResource(ID3D12Device* pDevice) :
	m_fenceValue (0)
{
	// ���� frame�� �۾��� �����°� ��ٸ��� �ʰ� ���� frame�� command list�� �ۼ��ϹǷ�, frame resource �� ���� command allocator�� ������.
	ThrowIfFailed(pDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)));

	 // Create an upload heap for the constant buffers.
	ThrowIfFailed(pDevice->CreateCommittedResource(
	&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
	D3D12_HEAP_FLAG_NONE,
	&CD3DX12_RESOURCE_DESC::Buffer(sizeof(SceneConstantBuffer)),
	D3D12_RESOURCE_STATE_GENERIC_READ,
	nullptr,
	IID_PPV_ARGS(&m_cbvUploadHeap)));

	ThrowIfFailed(m_cbvUploadHeap->Map(0, nullptr, reinterpret_cast<void**>(&m_pConstantBuffers)));
}

FrameResource::~FrameResource()
{
	m_cbvUploadHeap->Unmap(0, nullptr);
	m_pConstantBuffers = nullptr;
}

void FrameResource::PopulateCommandList()
{

}

void XM_CALLCONV UpdateConstantBuffers(FXMMATRIX view, CXMMATRIX proj)
{
	XMFLOAT4 mvp;


}
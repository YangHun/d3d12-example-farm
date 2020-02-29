#pragma once
#include "d3dHelper.h"

using Microsoft::WRL::ComPtr;

template<typename T>
class UploadBuffer
{
public:
	UploadBuffer(ID3D12Device* device, UINT elementCount, bool isConstantBuffer) :
		m_isConstantBuffer (isConstantBuffer)
	{
		m_byteSize = (!isConstantBuffer) ? sizeof(T) : ((sizeof(T) + 255) & ~255);
		
		ThrowIfFailed(device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Buffer(m_byteSize * elementCount),
			D3D12_RESOURCE_STATE_GENERIC_READ,
			nullptr,
			IID_PPV_ARGS(&m_uploadBuffer)
		));
	
		ThrowIfFailed(m_uploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_pData)));
	}

	UploadBuffer(const UploadBuffer& rhs) = delete;
	UploadBuffer& operator=(const UploadBuffer& rhs) = delete;

	~UploadBuffer()
	{
		if (m_uploadBuffer != nullptr)
			m_uploadBuffer->Unmap(0, nullptr);

		m_pData = nullptr;
	}

	ID3D12Resource* Resource()const
	{
		return m_uploadBuffer.Get();
	}

	void CopyData(int elementIndex, const T& data)
	{
		memcpy(&m_pData[elementIndex * m_byteSize], &data, sizeof(T));
	}

private:
	ComPtr<ID3D12Resource> m_uploadBuffer;
	BYTE* m_pData = nullptr;

	UINT m_byteSize = 0;
	bool m_isConstantBuffer = false;
};
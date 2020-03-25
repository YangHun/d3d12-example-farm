#pragma once
using namespace DirectX;
using Microsoft::WRL::ComPtr;

class ShadowMap
{
public:
	ShadowMap(ID3D12Device* pDevice, UINT width, UINT height);

	UINT GetWidth() const { return m_width; }
	UINT GetHeight() const { return m_height; }

	ID3D12Resource* Resource() { return m_shadowMap.Get(); }
	
	void BuildDescriptors(CD3DX12_CPU_DESCRIPTOR_HANDLE cpuSrv, CD3DX12_GPU_DESCRIPTOR_HANDLE gpuSrv, CD3DX12_CPU_DESCRIPTOR_HANDLE cpuDsv);


	void SetHeapIDs(UINT srvID, UINT dsvID) {
		m_SrvID = srvID;
		m_DsvID = dsvID;
	}

	UINT GetSrvHeapID() { return m_SrvID; }
	UINT GetDsvHeapID() { return m_DsvID; }
	D3D12_VIEWPORT GetViewport() const { return m_viewport; }
	D3D12_RECT GetRect() const { return m_sissorRect; }

	CD3DX12_CPU_DESCRIPTOR_HANDLE GetDsv() const { return m_cpuDsvHandle; }
	CD3DX12_GPU_DESCRIPTOR_HANDLE GetSrv() const { return m_gpuSrvHandle; }

private:
	ID3D12Device* m_pDevice;
	UINT m_width;
	UINT m_height;

	D3D12_VIEWPORT m_viewport;
	D3D12_RECT m_sissorRect;

	CD3DX12_CPU_DESCRIPTOR_HANDLE m_cpuSrvHandle;
	CD3DX12_GPU_DESCRIPTOR_HANDLE m_gpuSrvHandle;
	CD3DX12_CPU_DESCRIPTOR_HANDLE m_cpuDsvHandle;

	ComPtr<ID3D12Resource> m_shadowMap = nullptr;

	UINT m_SrvID;
	UINT m_DsvID;
};
#include "stdafx.h"
#include "DirectXApp.h"

DirectXApp::DirectXApp(UINT width, UINT height, std::wstring name) :
	m_width(width),
	m_height(height),
	m_title(name),
	m_useWarpDevice(false)
{
	m_aspectRatio = static_cast<float>(width) / static_cast<float>(height);
	
	WCHAR assetsPath[512];
	GetAssetsPath(assetsPath, _countof(assetsPath));
	m_assetsPath = assetsPath;
}

DirectXApp::~DirectXApp() 
{
}

void DirectXApp::GetHardwareAdapter(IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter, D3D_FEATURE_LEVEL* pFeature)
{
	ComPtr<IDXGIAdapter1> adapter;
	*ppAdapter = nullptr;

	// Factory 를 이용해 system 상의 모든 adpater를 순회한다
	for (UINT adapterIndex = 0;
		pFactory->EnumAdapters1(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND;
		++adapterIndex)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		// 해당 adapter가 s/w adapter 일 경우, 패스
		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			continue;
		}

		// 이 adapter가 d3d12를 지원할 경우, return.
		if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
		{
			*ppAdapter = adapter.Detach();
			*pFeature = D3D_FEATURE_LEVEL_11_0;
			return;
		}
	}
}

void DirectXApp::CreateConstantBuffer(ID3D12Device* pDevice, UINT64 byteSize, UINT64 elementCount, ComPtr<ID3D12Resource>& uploadBuffer)
{

	UINT bufferSize = (byteSize + 255) & ~255;

	ThrowIfFailed(pDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(bufferSize * elementCount),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&uploadBuffer)
	));
}

// default buffer 및 upload buffer 관련 helper function.
void DirectXApp::CreateDefaultBuffer(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList, const void* pInitData, UINT64 byteSize, ComPtr<ID3D12Resource>& defaultBuffer, ComPtr<ID3D12Resource>& uploadBuffer)
{

	// default buffer resource 생성
	ThrowIfFailed(pDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(byteSize),
		D3D12_RESOURCE_STATE_COMMON,
		nullptr,
		IID_PPV_ARGS(&defaultBuffer)
	));

	// cpu 메모리 데이터를 복사해서 default buffer에 넣기 위해, upload heap 생성 
	ThrowIfFailed(pDevice->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(byteSize),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&uploadBuffer)
	));

	D3D12_SUBRESOURCE_DATA subResourceData = {
		pInitData,
		byteSize,
		byteSize
	};

	// data를 복사하기 위한 resource state 관리
	pCommandList->ResourceBarrier(
		1,
		&CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
			D3D12_RESOURCE_STATE_COMMON,
			D3D12_RESOURCE_STATE_COPY_DEST));
	// default buffer data를 upload buffer에 복사
	UpdateSubresources<1>(pCommandList, defaultBuffer.Get(), uploadBuffer.Get(), 0, 0, 1, &subResourceData);
	// 복사 후 resource state를 generic read로 바꿈
	pCommandList->ResourceBarrier(
		1,
		&CD3DX12_RESOURCE_BARRIER::Transition(defaultBuffer.Get(),
			D3D12_RESOURCE_STATE_COPY_DEST,
			D3D12_RESOURCE_STATE_GENERIC_READ));

}

// Helper function for parsing any supplied command line args.
_Use_decl_annotations_
void DirectXApp::ParseCommandLineArgs(WCHAR* argv[], int argc)
{
	for (int i = 1; i < argc; ++i)
	{
		if (_wcsnicmp(argv[i], L"-warp", wcslen(argv[i])) == 0 ||
			_wcsnicmp(argv[i], L"/warp", wcslen(argv[i])) == 0)
		{
			m_useWarpDevice = true;
			m_title = m_title + L" (WARP)";
		}
	}
}

// Helper function for resolving the full path of assets.
std::wstring DirectXApp::GetAssetFullPath(LPCWSTR assetName)
{
	return m_assetsPath + assetName;
}

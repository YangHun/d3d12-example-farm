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

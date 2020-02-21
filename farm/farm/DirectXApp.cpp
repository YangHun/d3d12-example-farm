#include "DirectXApp.h"

DirectXApp::DirectXApp(UINT width, UINT height, std::wstring name) :
	mWidth(width),
	mHeight(height),
	mWinTitle(name)
{
	mAspectRatio = static_cast<float>(width) / static_cast<float>(height);
}

DirectXApp::~DirectXApp() 
{
}

void DirectXApp::GetHardwareAdapter(IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter)
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
			return;
		}
	}
}


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

	// Factory �� �̿��� system ���� ��� adpater�� ��ȸ�Ѵ�
	for (UINT adapterIndex = 0;
		pFactory->EnumAdapters1(adapterIndex, &adapter) != DXGI_ERROR_NOT_FOUND;
		++adapterIndex)
	{
		DXGI_ADAPTER_DESC1 desc;
		adapter->GetDesc1(&desc);

		// �ش� adapter�� s/w adapter �� ���, �н�
		if (desc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE)
		{
			continue;
		}

		// �� adapter�� d3d12�� ������ ���, return.
		if (SUCCEEDED(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, _uuidof(ID3D12Device), nullptr)))
		{
			*ppAdapter = adapter.Detach();
			return;
		}
	}
}


#pragma once

#include "d3dHelper.h"
#include "Win32Application.h"

using Microsoft::WRL::ComPtr;

class DirectXApp
{
public:
	DirectXApp(UINT width, UINT height, std::wstring name);
	virtual ~DirectXApp();

	virtual void Initialize() = 0;
	virtual void Update() = 0;
	virtual void Render() = 0;
	virtual void Destroy() = 0;

	virtual void OnKeyDown(UINT8 key) {}
	virtual void OnKeyUp(UINT8 key) {}

	virtual void OnMouseDown(UINT8 btnState, int x, int y) {}
	virtual void OnMouseUp(UINT8 btnState, int x, int y) {}
	virtual void OnMouseMove(UINT8 btnState, int x, int y) {}
	virtual void OnMouseLeave(UINT8 btnState, int x, int y) {}

	UINT GetWidth() const { return m_width; }
	UINT GetHeight() const { return m_height; }
	const WCHAR* GetTitle() const { return m_title.c_str(); }
	void ParseCommandLineArgs(WCHAR* argv[], int argc);

protected:
	void GetHardwareAdapter(IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter, D3D_FEATURE_LEVEL* pFeature);
	std::wstring GetAssetFullPath(LPCWSTR assetName);
	void CreateDefaultBuffer(ID3D12Device* pDevice, ID3D12GraphicsCommandList* pCommandList, const void* pInitData, UINT64 byteSize, ComPtr<ID3D12Resource>& defaultBuffer, ComPtr<ID3D12Resource>& uploadBuffer);
	void CreateConstantBuffer(ID3D12Device* pDevice, UINT64 byteSize, UINT64 elementCount, ComPtr<ID3D12Resource>& uploadBuffer);

protected:
	UINT m_width;
	UINT m_height;

	bool m_useWarpDevice;

private:
	std::wstring m_assetsPath;
	std::wstring m_title;
};
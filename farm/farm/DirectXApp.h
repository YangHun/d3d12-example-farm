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


	UINT GetWidth() const { return m_width; }
	UINT GetHeight() const { return m_height; }
	const WCHAR* GetTitle() const { return m_title.c_str(); }
	void ParseCommandLineArgs(WCHAR* argv[], int argc);

protected:
	void GetHardwareAdapter(IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter, D3D_FEATURE_LEVEL* pFeature);
	std::wstring GetAssetFullPath(LPCWSTR assetName);

protected:
	UINT m_width;
	UINT m_height;
	float m_aspectRatio;

	bool m_useWarpDevice;

private:
	std::wstring m_assetsPath;
	std::wstring m_title;
};
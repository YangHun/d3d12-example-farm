#pragma once

#include <d3d12.h>
#include <dxgi1_4.h>
#include <wrl.h>
#include <string>

using Microsoft::WRL::ComPtr;

class DirectXApp
{
public:
	DirectXApp(UINT width, UINT height, std::wstring name);
	virtual ~DirectXApp();

	virtual void Initialize();
	virtual void Update();
	virtual void Render();
	virtual void Destroy();


	UINT GetWidth() const { return mWidth; }
	UINT GetHeight() const { return mHeight; }
	const WCHAR* GetTitle() const { return mWinTitle.c_str(); }

protected:
	void GetHardwareAdapter(IDXGIFactory2* pFactory, IDXGIAdapter1** ppAdapter);

protected:
	UINT mWidth;
	UINT mHeight;
	float mAspectRatio;

private:
	std::wstring mWinTitle;
};
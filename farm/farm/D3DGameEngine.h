#pragma once

#include "DirectXApp.h"
#include "DirectXGame.h"
#include "GameTimer.h"
#include "FrameResource.h"
#include "UploadBuffer.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;


class D3DGameEngine : public DirectXApp
{
public:
	D3DGameEngine(UINT width, UINT height, std::wstring name);

	virtual void Initialize();
	virtual void Update();
	virtual void Render();
	virtual void Destroy();

private:
	void LoadPipeline();
	void LoadAssets();
	void WaitForPreviousFrame();
	void PopulateCommandList();
	void RenderUI();
	void DrawCurrentScene();
	void DrawCurrentUI();

	static const UINT FrameCount = 2;

	// game framework
	DirectXGame m_game;


	// pipeline objects
	CD3DX12_VIEWPORT m_viewport;
	CD3DX12_RECT m_scissorRect;
	ComPtr<ID3D12Device> m_device;
	ComPtr<ID3D12CommandQueue> m_commandQueue;
	ComPtr<IDXGISwapChain3> m_swapChain;
	ComPtr<ID3D12RootSignature> m_rootSignature;
	ComPtr<ID3D12PipelineState> m_pipelineState;		// PSO
	ComPtr<ID3D12CommandAllocator> m_commandAllocator;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;
	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
	UINT m_rtvDescriptorSize;
	UINT m_dsvDescriptorSize;
	ComPtr<ID3D12Resource> m_renderTargets[FrameCount];

	// App resources.
	ComPtr<ID3D12Resource> m_vertexBuffer;
	ComPtr<ID3D12Resource> m_vertexUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	ComPtr<ID3D12Resource> m_indexBuffer;
	ComPtr<ID3D12Resource> m_indexUploadBuffer;
	D3D12_INDEX_BUFFER_VIEW m_indexBufferView;
	ComPtr<ID3D12Resource> m_depthStencil;	// depth stencil buffer


	std::unique_ptr<UploadBuffer<SceneConstantBuffer>> m_constantBuffer;

	//ComPtr<ID3D12Resource> m_cbUploadBuffer; // constant buffer uploader
	//SceneConstantBuffer* m_pConstantBuffer;


	 // 2d pipeline objects
	ComPtr<ID3D11On12Device> m_d3d11On12Device;
	ComPtr<ID3D11DeviceContext> m_d3d11DeviceContext;
	ComPtr<ID2D1Factory3> m_d2dFactory;
	ComPtr<ID2D1Device2> m_d2dDevice;
	ComPtr<ID2D1DeviceContext2> m_d2dDeviceContext;
	ComPtr<ID2D1Bitmap1> m_d2dRenderTargets[FrameCount];
	ComPtr<IDWriteFactory> m_dWriteFactory;
	ComPtr<ID3D11Resource> m_wrappedBackBuffers[FrameCount];

	// 2d resouces.
	ComPtr<ID2D1SolidColorBrush> m_textBrush;
	ComPtr<IDWriteTextFormat> m_textFormat;

	GameTimer m_timer;

	// synchronization objects
	UINT m_frameIndex;
	HANDLE m_fenceEvent;
	ComPtr<ID3D12Fence> m_fence;
	UINT64 m_fenceValue;

	UINT m_4xMsaaQuality = 0;
};
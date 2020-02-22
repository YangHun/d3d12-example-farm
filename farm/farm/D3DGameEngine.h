#pragma once

#include "DirectXApp.h"

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

	static const UINT FrameCount = 2;

	// pipeline objects

	ComPtr<ID3D12Device> m_device;
	ComPtr<ID3D12CommandQueue> m_commandQueue;
	ComPtr<IDXGISwapChain3> m_swapChain;
	ComPtr<ID3D12RootSignature> m_rootSignature;
	ComPtr<ID3D12PipelineState> m_pipelineState;		// PSO
	ComPtr<ID3D12CommandAllocator> m_commandAllocator;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;
	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	UINT m_rtvDescriptorSize;
	ComPtr<ID3D12Resource> m_renderTargets[FrameCount];


	// synchronization objects
	UINT m_frameIndex;
	HANDLE m_fenceEvent;
	ComPtr<ID3D12Fence> m_fence;
	UINT64 m_fenceValue;

	UINT m_4xMsaaQuality = 0;
};
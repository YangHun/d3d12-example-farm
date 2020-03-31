#pragma once

#include "DirectXApp.h"
#include "DirectXGame.h"
#include "GameTimer.h"
#include "FrameResource.h"
#include "UploadBuffer.h"
#include "DDSTextureLoader.h"
#include "ShadowMap.h"

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

	virtual void OnKeyDown(UINT8 key);
	virtual void OnKeyUp(UINT8 key);
	virtual void OnMouseDown(UINT8 btnState, int x, int y);
	virtual void OnMouseUp(UINT8 btnState, int x, int y);
	virtual void OnMouseMove(UINT8 btnState, int x, int y);
	virtual void OnMouseLeave(UINT8 btnState, int x, int y);

private:
	enum class E_RootParam 
	{
		InstanceMap,
		MaterialMap,
		Scene,
		ShadowPass,
		BufferTexture,
		Texture2DHeap,
		Count
	};

	enum class E_PipelineState
	{
		Opaque,
		Sky,
		ShadowOpaque,
		ShadowDebug,
#ifdef COLLIDER_DEBUG
		ColliderDebug,
#endif
		Count
	};

	void LoadPipeline();
	void LoadAssets();
	void WaitForPreviousFrame();
	void PopulateCommandList();
	void RenderUI();
	void DrawCurrentScene(E_PipelineState state, E_RenderLayer layer);
	void DrawCurrentUI();

	std::wstring PrintInfo();

	POINT GetWindowCenter();

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
	std::unordered_map<int, ComPtr<ID3D12PipelineState>> m_pipelineStates;		// PSO
	ComPtr<ID3D12CommandAllocator> m_commandAllocator;
	ComPtr<ID3D12GraphicsCommandList> m_commandList;
	ComPtr<ID3D12DescriptorHeap> m_rtvHeap;
	ComPtr<ID3D12DescriptorHeap> m_dsvHeap;
	ComPtr<ID3D12DescriptorHeap> m_srvHeap;
	UINT m_rtvDescriptorSize;
	UINT m_dsvDescriptorSize;
	UINT m_CbvSrvUavDescriptorSize;
	ComPtr<ID3D12Resource> m_renderTargets[FrameCount];

	CD3DX12_GPU_DESCRIPTOR_HANDLE m_nullSrv;

	// App resources.
	ComPtr<ID3D12Resource> m_vertexBuffer;
	ComPtr<ID3D12Resource> m_vertexUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
	ComPtr<ID3D12Resource> m_indexBuffer;
	ComPtr<ID3D12Resource> m_indexUploadBuffer;
	D3D12_INDEX_BUFFER_VIEW m_indexBufferView;
	ComPtr<ID3D12Resource> m_depthStencil;	// depth stencil buffer

	std::unique_ptr<ShadowMap> m_shadowMap;
	UINT m_skyID;

	std::unique_ptr<UploadBuffer<SceneConstantBuffer>> m_constantBuffer;
	std::unique_ptr<UploadBuffer<MaterialBuffer>> m_materialBuffer;
	
	// key = meshDescID
	// value = UploadBuffer<InstanceBuffer> [E_RenderLayer]
	std::unordered_map<int, std::vector<std::unique_ptr<UploadBuffer<InstanceBuffer>>>> m_instanceBuffers;


	std::unique_ptr<UploadBuffer<ShadowPassConstantBuffer>> m_shadowBuffer;

	XMFLOAT3 m_BaseLightDirections[3] = {
		XMFLOAT3(-0.57735f, -0.57735f, 0.57735f),
		XMFLOAT3(0.57735f, -0.57735f, 0.57735f),
		XMFLOAT3(0.0f, -0.707f, -0.707f)
	};

	 // 2d pipeline objects
	ComPtr<ID3D11On12Device> m_d3d11On12Device;
	ComPtr<ID3D11DeviceContext> m_d3d11DeviceContext;
	ComPtr<ID2D1Factory3> m_d2dFactory;
	ComPtr<ID2D1Device2> m_d2dDevice;
	ComPtr<ID2D1DeviceContext2> m_d2dDeviceContext;
	ComPtr<ID2D1Bitmap1> m_d2dRenderTargets[FrameCount];
	ComPtr<IDWriteFactory> m_dWriteFactory;
	ComPtr<ID3D11Resource> m_wrappedBackBuffers[FrameCount];

	// 2d image.
	ComPtr<IWICImagingFactory2> m_wicFactory;

	GameTimer m_timer;
	float m_angle = 0;

	// synchronization objects
	UINT m_frameIndex;
	HANDLE m_fenceEvent;
	ComPtr<ID3D12Fence> m_fence;
	UINT64 m_fenceValue;

	UINT m_4xMsaaQuality = 0;


	// for debug.
	bool m_debugShadowMap = false;
	bool m_debugLog = false;

	std::vector<size_t> m_layerInstanceCount;
	std::vector<size_t> m_psoDrawCalls;

#ifdef COLLIDER_DEBUG
	bool m_debugCollider = false;
#endif
};
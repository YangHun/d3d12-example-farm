#pragma once

#include "DirectXApp.h"

using namespace DirectX;
using Microsoft::WRL::ComPtr;

struct Vertex
{
	XMFLOAT3 position;
	XMFLOAT4 color;
};

class DirectXGame
{

public:
	class Scene
	{
	public:
		Scene(UINT id);

		void Initialize();

	private:
		void LoadAssets();

	public:
		UINT m_id;

		std::vector<Vertex> m_data;

		// current scene resources.
		ComPtr<ID3D12Resource> m_vertexBuffer;
		ComPtr<ID3D12Resource> m_indexBuffer;
		ComPtr<ID3D12Resource> m_vertexUploadBuffer;
		ComPtr<ID3D12Resource> m_indexUploadBuffer;
		D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;
		D3D12_INDEX_BUFFER_VIEW m_indexBufferView;
	};

	DirectXGame();

	void Initialize();
	void BuildScenes();

	UINT SceneCount() const { return m_allScenes.size(); }
	Scene* GetScene(UINT index) const { return m_allScenes[index].get();  }
	bool IsSceneChanged() const { return m_dirtyScene; }
	void SetUpdated() { m_dirtyScene = true; }

	void GetCurrentSceneBufferView(D3D12_VERTEX_BUFFER_VIEW& vertexBufferView, D3D12_INDEX_BUFFER_VIEW& indexBufferView);

private:	
	Scene* m_currentScene = nullptr;
	std::vector<std::unique_ptr<Scene>> m_allScenes;

	bool m_dirtyScene = true;
};

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

	// synchronization objects
	UINT m_frameIndex;
	HANDLE m_fenceEvent;
	ComPtr<ID3D12Fence> m_fence;
	UINT64 m_fenceValue;

	UINT m_4xMsaaQuality = 0;
};
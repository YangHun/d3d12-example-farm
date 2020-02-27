#include "stdafx.h"
#include "D3DGameEngine.h"
#include "scenes.h"

D3DGameEngine::D3DGameEngine(UINT width, UINT height, std::wstring name) :
	DirectXApp(width, height, name),
	m_frameIndex(0),
	m_viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)),
	m_scissorRect(0, 0, static_cast<LONG>(width), static_cast<LONG>(height)),
	m_fenceValue(0),
	m_rtvDescriptorSize(0),
	m_dsvDescriptorSize(0)
{
	m_game = DirectXGame();
	m_camera.SetPosition(XMFLOAT3(0.0f, 5.0f, -10.0f));
	m_camera.SetFrustum(XM_PI * 0.25f);
	m_timer.Reset();
}

void D3DGameEngine::Initialize()
{
	m_timer.Start();
	LoadPipeline();
	LoadAssets();
}

void D3DGameEngine::LoadPipeline()
{
	UINT dxgiFactoryFlags = 0;

	// Create the device.
	ComPtr<IDXGIFactory4> factory;
	ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&factory)));

	if (m_useWarpDevice)
	{
		ComPtr<IDXGIAdapter> warpAdapter;
		ThrowIfFailed(factory->EnumWarpAdapter(IID_PPV_ARGS(&warpAdapter)));

		ThrowIfFailed(D3D12CreateDevice(
			warpAdapter.Get(),
			D3D_FEATURE_LEVEL_11_0,
			IID_PPV_ARGS(&m_device)));
	}
	else {
		ComPtr<IDXGIAdapter1> hardwareAdapter;
		D3D_FEATURE_LEVEL featureLevel;
		GetHardwareAdapter(factory.Get(), &hardwareAdapter, &featureLevel);

		ThrowIfFailed(D3D12CreateDevice(
			hardwareAdapter.Get(),
			featureLevel,
			IID_PPV_ARGS(&m_device)
		));
	}

	// Check 4X Multi Sampling Anti-Aliasing (MSAA) quality level support.
	// d3d11은 모든 render target format에 대해 4x MSAA 를 지원하므로 quality check만 수행 
	{
		D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevel;
		msQualityLevel.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		msQualityLevel.SampleCount = 4;
		msQualityLevel.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
		msQualityLevel.NumQualityLevels = 0;
		ThrowIfFailed(m_device->CheckFeatureSupport(
			D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS,
			&msQualityLevel,
			sizeof(msQualityLevel)));

		m_4xMsaaQuality = msQualityLevel.NumQualityLevels;
		assert(m_4xMsaaQuality > 0 && "Unexpected MSAA quality level.");
	}

	// Describe and create the command queue.
	D3D12_COMMAND_QUEUE_DESC queueDesc = {};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

	ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));

	// Describe and create the swap chain.
	// IDXGISwapChain1 은 MSAA 를 지원하지 않는다. MSAA를 지원하는 다른 buffer를 만들어야 함.
	// https://stackoverflow.com/questions/40110699/creating-a-swap-chain-with-msaa-fails
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc = {};
	swapChainDesc.BufferCount = FrameCount;
	swapChainDesc.Width = m_width;
	swapChainDesc.Height = m_height;
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	swapChainDesc.SampleDesc.Count = 1;

	ComPtr<IDXGISwapChain1> swapChain;
	ThrowIfFailed(factory->CreateSwapChainForHwnd(
		m_commandQueue.Get(),
		Win32Application::GetHwnd(),
		&swapChainDesc,
		nullptr,
		nullptr,
		&swapChain));

	ThrowIfFailed(swapChain.As(&m_swapChain));
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();

	
	// Create descriptor heaps.
	{
		// Describe and create a render target view (RTV) descriptor heap.
		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
		rtvHeapDesc.NumDescriptors = FrameCount;
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));


		// Describe and create a depth / stencil buffer view (DSV) descriptor heap.
		D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc = {};
		dsvHeapDesc.NumDescriptors = 1;
		dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
		dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		ThrowIfFailed(m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));
		
		/*
		D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
		cbvHeapDesc.NumDescriptors = TextureCount;
		cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		ThrowIfFailed(m_device->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&m_srvHeap)));
		NAME_D3D12_OBJECT(m_srvHeap);
		*/

		m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		m_dsvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
	}

	// Create frame resources.
	// Constant buffer는 GPU가 해당 buffer를 참조하는 작업이 끝나기 전엔 update 될 수 없다.
	// GPU가 n번째 프레임을 다 그릴 때까지 CPU가 idle 상태가 되지 않게 하기 위하여,
	// 매 프레임마다 update 해야 하는 resource들의 circular array 를 만들어
	// CPU가 다음 몇 개 프레임의 Resource 값을 미리 계산해 둘 수 있게 한다.
	{

		// 정의할 constant buffer가 없으므로 frame resource 대신 rtv를 만든다.

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

		for (UINT n = 0; n < FrameCount; ++n)
		{
			ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])));
			m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);
			rtvHandle.Offset(1, m_rtvDescriptorSize);
		}

	}

	// Create the command list allocator and main command list.
	ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)));

}

void D3DGameEngine::LoadAssets()
{

	// Create the main constant buffer.
	CreateConstantBuffer(m_device.Get(), sizeof(SceneConstantBuffer), 1, m_cbUploadBuffer);
	ThrowIfFailed(m_cbUploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&m_pConstantBuffer)));


	// Create the root signature.
	{
		CD3DX12_ROOT_PARAMETER rootParameters[2];

		
		rootParameters[0].InitAsConstantBufferView(0);
		rootParameters[1].InitAsConstantBufferView(1);

		CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init(2, rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;
		ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
		ThrowIfFailed(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
	}

	// Create the pipeline state, which includes compiling and loading shaders.
	{
		ComPtr<ID3DBlob> vertexShader;
		ComPtr<ID3DBlob> pixelShader;

#if defined(_DEBUG)
		// Enable better shader debugging with the graphics debugging tools.
		UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
		UINT compileFlags = 0;
#endif
		
		ThrowIfFailed(D3DCompileFromFile(L"Shaders/default.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VSMain", "vs_5_1", compileFlags, 0, &vertexShader, nullptr));
		ThrowIfFailed(D3DCompileFromFile(L"Shaders/default.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PSMain", "ps_5_1", compileFlags, 0, &pixelShader, nullptr));

		// Define the vertex input layout.
		D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
		{
			{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
			{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 32, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		};

		CD3DX12_RASTERIZER_DESC rasterizerStateDesc(D3D12_DEFAULT);
		rasterizerStateDesc.CullMode = D3D12_CULL_MODE_BACK;

		// Describe and create the graphics pipeline state object (PSO).
		D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
		psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
		psoDesc.pRootSignature = m_rootSignature.Get();
		psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
		psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
		psoDesc.RasterizerState = rasterizerStateDesc;
		psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
		psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
		psoDesc.SampleMask = UINT_MAX;
		psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
		psoDesc.NumRenderTargets = 1;
		psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
		psoDesc.DSVFormat = DXGI_FORMAT_D32_FLOAT;
		psoDesc.SampleDesc.Count = 1;
		ThrowIfFailed(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));
	}

	// Create the main command list.
	ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), m_pipelineState.Get(), IID_PPV_ARGS(&m_commandList)));
	
	
	// Initialize the game.
	m_game.Initialize();

	// Create the vertex buffer and index buffer for all meshes in game.
	// Scenes would reference mesh descriptions to draw specific meshes.
	{
		for (auto& i : m_game.m_meshes) {

			auto desc = i.second.get();
			UINT verticeSize = sizeof(Vertex) * (UINT)(desc->mesh->vertices.size());
			UINT indiceSize = sizeof(uint16_t) * (UINT)(desc->mesh->indices.size());

			CreateDefaultBuffer(m_device.Get(), m_commandList.Get(), desc->mesh->vertices.data(), verticeSize, desc->vertexBuffer, desc->vertexUploadBuffer);
			// create the index buffer using triangle data and copy to upload buffer.
			CreateDefaultBuffer(m_device.Get(), m_commandList.Get(), desc->mesh->indices.data(), indiceSize, desc->indexBuffer, desc->indexUploadBuffer);

			desc->vertexBufferView.BufferLocation = desc->vertexBuffer->GetGPUVirtualAddress();
			desc->vertexBufferView.StrideInBytes = sizeof(Vertex);
			desc->vertexBufferView.SizeInBytes = verticeSize;

			desc->indexBufferView.BufferLocation = desc->indexBuffer->GetGPUVirtualAddress();
			desc->indexBufferView.Format = DXGI_FORMAT_R16_UINT;
			desc->indexBufferView.SizeInBytes = indiceSize;

		}
	}

	// Create object constant buffers.
	{
		for (UINT i = 0; i < m_game.SceneCount(); ++i)
		{
			auto s = m_game.GetScene(i);
			CreateConstantBuffer(m_device.Get(),sizeof(ObjectConstantBuffer), (UINT)s->m_objects.size(), s->m_cbUploadBuffer);
			ThrowIfFailed(s->m_cbUploadBuffer->Map(0, nullptr, reinterpret_cast<void**>(&s->m_objConstantBuffer)));
		}
	}

	// 초기화 단계에서 더이상 Command를 추가하지 않으므로 Close하고, 초기화 관련 command 실행
	ThrowIfFailed(m_commandList->Close());
	ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// Create synchronization objects and wait until assets have been uploaded to the GPU.
	{
		ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
		m_fenceValue = 1;

		// Create an event handle to use for frame synchronization.
		m_fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
		if (m_fenceEvent == nullptr)
		{
			ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
		}

		// Wait for the command list to execute; we are reusing the same command 
		// list in our main loop but for now, we just want to wait for setup to 
		// complete before continuing.
		WaitForPreviousFrame();
	}
}

void D3DGameEngine::Update()
{
	m_timer.Tick();
	if (m_game.IsSceneChanged())
	{
		// do sth
		m_game.SetUpdated();
	}

	WaitForPreviousFrame();

	// update constant buffer data
	{
		XMMATRIX view = m_camera.LookAt(XMFLOAT3(0.0f, 0.0f, 0.0f));
		XMMATRIX proj = m_camera.GetProjectionMatrix(m_aspectRatio);
				
		XMFLOAT4X4 mvp;
		XMStoreFloat4x4(&mvp, XMMatrixTranspose(view * proj));
		memcpy(m_pConstantBuffer, &mvp, sizeof(mvp));
	}

	// update object buffers of active scene
	m_game.GetCurrentScene()->UpdateObjectConstantBuffers();
	
}

void D3DGameEngine::Render()
{
	// 다음 frame에 실행할 command를 command list에 등록
	PopulateCommandList();

	// command list가 갖고 있는 command를 GPU의 command queue에 등록
	ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// 현재 frame의 back buffer와 frame buffer를 swap.
	ThrowIfFailed(m_swapChain->Present(1, 0));

	WaitForPreviousFrame();
}

void D3DGameEngine::Destroy()
{
	// GPU 작업이 모두 끝나서 더이상 참조하는 resource가 없을 때까지 기다린다
	WaitForPreviousFrame();
	CloseHandle(m_fenceEvent);
}

void D3DGameEngine::WaitForPreviousFrame()
{
	// Command queue에 fence value increasement command를 추가
	const UINT64 fence = m_fenceValue;
	ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), fence));
	m_fenceValue++;

	// 이전 command 작업이 모두 끝나고 fence value가 update될 때까지 기다린다
	if (m_fence->GetCompletedValue() < fence)
	{
		ThrowIfFailed(m_fence->SetEventOnCompletion(fence, m_fenceEvent));
		WaitForSingleObject(m_fenceEvent, INFINITE);
	}

	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
}

void D3DGameEngine::PopulateCommandList()
{
	ThrowIfFailed(m_commandAllocator->Reset());
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), m_pipelineState.Get()));

	m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
	m_commandList->RSSetViewports(1, &m_viewport);
	m_commandList->RSSetScissorRects(1, &m_scissorRect);

	// back buffer texture의 state를 render target 상태로 변경.
	// GPU가 해당 resource 에 write 작업을 할 것임을 명시한다.
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);
	m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

	const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	
	m_commandList->SetGraphicsRootConstantBufferView(1, m_cbUploadBuffer.Get()->GetGPUVirtualAddress());

	// 현재 scene의 index instance를 그린다.
	DrawCurrentScene();

	// back buffer texture의 state를 present 상태로 변경.
	// 위 command가 실행된 후 back buffer에 더이상 write 작업이 없음을 명시.
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));


	// command 추가가 완료되었으므로 queue에 넘기기 전 list를 close.
	ThrowIfFailed(m_commandList->Close());
}

void D3DGameEngine::DrawCurrentScene()
{
	auto scene = m_game.GetCurrentScene();

	for (auto obj : scene->m_objects)
	{
		m_commandList->IASetVertexBuffers(0, 1, &obj->vertexBufferView);
		m_commandList->IASetIndexBuffer(&obj->indexBufferView);
		m_commandList->IASetPrimitiveTopology(obj->primitiveType);

		D3D12_GPU_VIRTUAL_ADDRESS address = scene->m_cbUploadBuffer.Get()->GetGPUVirtualAddress() + obj->constantBufferId * sizeof(ObjectConstantBuffer);

		//D3D12_GPU_VIRTUAL_ADDRESS objCBAddress = objectCB->GetGPUVirtualAddress() + ri->ObjCBIndex * objCBByteSize;
		m_commandList->SetGraphicsRootConstantBufferView(0, address);

		m_commandList->DrawIndexedInstanced(obj->indexCount, 1, obj->startIndexLocation, obj->baseVertexLocation, obj->startIndexLocation);
	}
}
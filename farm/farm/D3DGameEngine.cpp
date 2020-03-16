#include "stdafx.h"
#include "D3DGameEngine.h"

D3DGameEngine::D3DGameEngine(UINT width, UINT height, std::wstring name) :
	DirectXApp(width, height, name),
	m_frameIndex(0),
	m_viewport(0.0f, 0.0f, static_cast<float>(width), static_cast<float>(height)),
	m_scissorRect(0, 0, static_cast<LONG>(width), static_cast<LONG>(height)),
	m_fenceValue(0),
	m_rtvDescriptorSize(0),
	m_dsvDescriptorSize(0),
	m_game(&m_viewport)
{
	CoInitialize(NULL);	//initialize COM to use Windows Imaging Component
	m_timer.Reset();	

}

void D3DGameEngine::Initialize()
{
	
	m_timer.Start();
	LoadPipeline();
	LoadAssets();

	//m_game.GetCurrentScene()->m_camera.SetInitialMousePos(GetWindowCenter());

	RECT window;
	GetWindowRect(Win32Application::GetHwnd(), &window);
	SetCursorPos((window.left + window.right) / 2, (window.top + window.bottom) / 2);
	ShowCursor(false);
}

POINT D3DGameEngine::GetWindowCenter()
{
	RECT window;
	GetWindowRect(Win32Application::GetHwnd(), &window);
	//SetCursorPos(m_width / 2 + window.left, m_height / 2 + window.top);

	POINT result;
	result.x = (window.left + window.right) / 2;
	result.y = (window.top + window.bottom) / 2;

	return result;
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
	{
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
	}

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
		
		
		// Describe and create the shader resource view (SRV) descriptor heap.
		D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
		srvHeapDesc.NumDescriptors = 64;
		srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
		srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
		ThrowIfFailed(m_device->CreateDescriptorHeap(&srvHeapDesc, IID_PPV_ARGS(&m_srvHeap)));
		

		m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		m_dsvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_DSV);
		m_CbvSrvUavDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	}

	// Create an 11 device wrapped aroud the 12 device and share
	// 12's command queue to use D2D11.
	{
		ComPtr<ID3D11Device> d3d11Device;
		ThrowIfFailed(D3D11On12CreateDevice(
			m_device.Get(),
			D3D11_CREATE_DEVICE_BGRA_SUPPORT,
			nullptr,
			0,
			reinterpret_cast<IUnknown**>(m_commandQueue.GetAddressOf()),
			1,
			0,
			&d3d11Device,
			&m_d3d11DeviceContext,
			nullptr
		));
		ThrowIfFailed(d3d11Device.As(&m_d3d11On12Device));
	}

	// Create D2D/DWrite components.
	{
		D2D1_FACTORY_OPTIONS d2dFactoryOptions = {};

		D2D1_DEVICE_CONTEXT_OPTIONS deviceOptions = D2D1_DEVICE_CONTEXT_OPTIONS_NONE;
		ThrowIfFailed(D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory3), &d2dFactoryOptions, &m_d2dFactory));
		ComPtr<IDXGIDevice> dxgiDevice;
		ThrowIfFailed(m_d3d11On12Device.As(&dxgiDevice));
		ThrowIfFailed(m_d2dFactory->CreateDevice(dxgiDevice.Get(), &m_d2dDevice));
		ThrowIfFailed(m_d2dDevice->CreateDeviceContext(deviceOptions, &m_d2dDeviceContext));
		ThrowIfFailed(DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), &m_dWriteFactory));
	}

	// Query the desktop's dpi settings, which will be used to create
	// D2D's render targets.
	UINT dpi = GetDpiForWindow(Win32Application::GetHwnd());	//compatible with Windows 10
	D2D1_BITMAP_PROPERTIES1 bitmapProperties = D2D1::BitmapProperties1(
		D2D1_BITMAP_OPTIONS_TARGET | D2D1_BITMAP_OPTIONS_CANNOT_DRAW,
		D2D1::PixelFormat(DXGI_FORMAT_UNKNOWN, D2D1_ALPHA_MODE_PREMULTIPLIED),
		(float)dpi,
		(float)dpi
	);

	// create WIC Factory.

	ThrowIfFailed(CoCreateInstance(CLSID_WICImagingFactory2, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&m_wicFactory)));


	// Create frame resources.
	// Constant buffer는 GPU가 해당 buffer를 참조하는 작업이 끝나기 전엔 update 될 수 없다.
	// GPU가 n번째 프레임을 다 그릴 때까지 CPU가 idle 상태가 되지 않게 하기 위하여,
	// 매 프레임마다 update 해야 하는 resource들의 circular array 를 만들어
	// CPU가 다음 몇 개 프레임의 Resource 값을 미리 계산해 둘 수 있게 한다.
	{

		CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart());

		for (UINT n = 0; n < FrameCount; ++n)
		{
			ThrowIfFailed(m_swapChain->GetBuffer(n, IID_PPV_ARGS(&m_renderTargets[n])));
			m_device->CreateRenderTargetView(m_renderTargets[n].Get(), nullptr, rtvHandle);

			// Create a wrapped 11On12 resource of this back buffer. Since we are 
			// rendering all D3D12 content first and then all D2D content, we specify 
			// the In resource state as RENDER_TARGET - because D3D12 will have last 
			// used it in this state - and the Out resource state as PRESENT. When 
			// ReleaseWrappedResources() is called on the 11On12 device, the resource 
			// will be transitioned to the PRESENT state.
			D3D11_RESOURCE_FLAGS d3d11Flags = { D3D11_BIND_RENDER_TARGET };
			ThrowIfFailed(m_d3d11On12Device->CreateWrappedResource(
				m_renderTargets[n].Get(),
				&d3d11Flags,
				D3D12_RESOURCE_STATE_RENDER_TARGET,
				D3D12_RESOURCE_STATE_PRESENT,
				IID_PPV_ARGS(&m_wrappedBackBuffers[n])
			));
			// Create a render target for D2D to draw directly to this back buffer.
			ComPtr<IDXGISurface> surface;
			ThrowIfFailed(m_wrappedBackBuffers[n].As(&surface));
			ThrowIfFailed(m_d2dDeviceContext->CreateBitmapFromDxgiSurface(
				surface.Get(),
				&bitmapProperties,
				&m_d2dRenderTargets[n]
			));


			rtvHandle.Offset(1, m_rtvDescriptorSize);
		}

	}

	// Create the depth stencil view.
	{
		D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilDesc = {};
		depthStencilDesc.Format = DXGI_FORMAT_D32_FLOAT;
		depthStencilDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
		depthStencilDesc.Flags = D3D12_DSV_FLAG_NONE;

		D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
		depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
		depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
		depthOptimizedClearValue.DepthStencil.Stencil = 0;

		ThrowIfFailed(m_device->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_D32_FLOAT, m_width, m_height, 1, 0, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL),
			D3D12_RESOURCE_STATE_DEPTH_WRITE,
			&depthOptimizedClearValue,
			IID_PPV_ARGS(&m_depthStencil)
		));

		NAME_D3D12_OBJECT(m_depthStencil);

		m_device->CreateDepthStencilView(m_depthStencil.Get(), &depthStencilDesc, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
	}

	// Create the command list allocator and main command list.
	ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)));

}

void D3DGameEngine::LoadAssets()
{

	// Create the main constant buffer.
	m_constantBuffer = std::make_unique<UploadBuffer<SceneConstantBuffer>>(m_device.Get(), 1, true);

	// Create the root signature.
	{
		CD3DX12_DESCRIPTOR_RANGE textureTable;
		textureTable.Init(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 48, 0, 0);

		CD3DX12_ROOT_PARAMETER rootParameters[4];

		rootParameters[0].InitAsConstantBufferView(0);	// register b0 (object constant buffer)
		rootParameters[1].InitAsConstantBufferView(1);	// register b1 (scene constant buffer)
		rootParameters[2].InitAsShaderResourceView(0, 1); // register t0, space 1 (material map)
		rootParameters[3].InitAsDescriptorTable(1, &textureTable, D3D12_SHADER_VISIBILITY_PIXEL); // register t0 (texture table)


		const CD3DX12_STATIC_SAMPLER_DESC anisotropicWrap(
			0,
			D3D12_FILTER_ANISOTROPIC,
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			D3D12_TEXTURE_ADDRESS_MODE_WRAP,
			0.0f, 8);


		const std::vector<CD3DX12_STATIC_SAMPLER_DESC> samplers =
		{
			anisotropicWrap	// register s0 (gAnisotropicWrap)
		};

		CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
		rootSignatureDesc.Init(4, rootParameters, samplers.size(), samplers.data(), D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);
		/*rootSignatureDesc.Init(3, rootParameters, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);*/

		ComPtr<ID3DBlob> signature;
		ComPtr<ID3DBlob> error;
		ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
		ThrowIfFailed(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
	}

	// Create the pipeline states, which includes compiling and loading shaders.
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
		psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
		psoDesc.SampleDesc.Count = 1;
		ThrowIfFailed(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineStates["opaque"])));


		//// Shaders for sky.

		//ComPtr<ID3DBlob> skyVertexShader;
		//ComPtr<ID3DBlob> skyPixelShader;
		//ThrowIfFailed(D3DCompileFromFile(L"Shaders/sky.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VSMain", "vs_5_1", compileFlags, 0, &skyVertexShader, nullptr));
		//ThrowIfFailed(D3DCompileFromFile(L"Shaders/sky.hlsl", nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PSMain", "ps_5_1", compileFlags, 0, &skyPixelShader, nullptr));

		//// Pipeline state for Skybox.
		//psoDesc.RasterizerState.CullMode = D3D12_CULL_MODE_NONE;
		//psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
		//psoDesc.pRootSignature = m_rootSignature.Get();
		//psoDesc.VS = CD3DX12_SHADER_BYTECODE(skyVertexShader.Get());
		//psoDesc.PS = CD3DX12_SHADER_BYTECODE(skyPixelShader.Get());

		//ThrowIfFailed(m_device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineStates["sky"])));
	}

	// Create D2D/DWrite objects for rendering text.
	{
		ThrowIfFailed(m_d2dDeviceContext->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &m_textBrush));
		ThrowIfFailed(m_dWriteFactory->CreateTextFormat(
			L"Verdana",
			NULL,
			DWRITE_FONT_WEIGHT_NORMAL,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			25,
			L"en-us",
			&m_textFormat
		));

		ThrowIfFailed(m_textFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_LEADING));
		ThrowIfFailed(m_textFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER));
	}

	// load 2d sprite image.
	{

		auto sprites = Assets::GetSprites();

		for (auto& s : sprites)
		{
			Microsoft::WRL::ComPtr<IWICBitmapDecoder> bitmapDecoder;
			ThrowIfFailed(m_wicFactory->CreateDecoderFromFilename(
				s->filePath.c_str(),
				NULL,
				GENERIC_READ,
				WICDecodeMetadataCacheOnLoad,
				bitmapDecoder.GetAddressOf()));

			Microsoft::WRL::ComPtr<IWICBitmapFrameDecode> frame;
			ThrowIfFailed(bitmapDecoder->GetFrame(0, frame.GetAddressOf()));

			Microsoft::WRL::ComPtr<IWICFormatConverter> image;
			ThrowIfFailed(m_wicFactory->CreateFormatConverter(image.GetAddressOf()));

			ThrowIfFailed(image->Initialize(
				frame.Get(),
				GUID_WICPixelFormat32bppPBGRA,
				WICBitmapDitherTypeNone,
				NULL,
				0,
				WICBitmapPaletteTypeCustom));
		
			ThrowIfFailed(m_d2dDeviceContext->CreateBitmapFromWicBitmap(image.Get(), &s->bitmap));
		}
	}

	// Create the main command list.
	// Set initial pipeline state is null.
	ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_commandList)));
	


	// Initialize the game.
	m_game.Initialize();

	// Create the vertex buffer and index buffer for all meshes in game.
	// Scenes would reference mesh descriptions to draw specific meshes.
	{
		auto meshes = Assets::GetMeshDesc();
		for (auto desc : meshes) {

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

	// Create the material buffer for assets.
	{
		m_materialBuffer = std::make_unique<UploadBuffer<MaterialBuffer>>(m_device.Get(), (UINT)Assets::m_materials.size(), false);
	}

	// Load textures and fill out the heap with actual descriptors.
	{
		CD3DX12_CPU_DESCRIPTOR_HANDLE heapDescriptor(m_srvHeap->GetCPUDescriptorHandleForHeapStart());

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;

		// heapDescriptor에 등록하는 순서와 Texture의 buffer id가 같도록, 정렬한 벡터를 사용.
		auto textures = Assets::GetOrderedTextures();
		for (auto t : textures)
		{
			ThrowIfFailed(DirectX::CreateDDSTextureFromFile12(m_device.Get(),
				m_commandList.Get(), t->filePath.c_str(),
				t->resource, t->uploadHeap));

			srvDesc.Format = t->resource->GetDesc().Format;
			srvDesc.Texture2D.MipLevels = t->resource->GetDesc().MipLevels;
			
			m_device->CreateShaderResourceView(t->resource.Get(), &srvDesc, heapDescriptor);
			heapDescriptor.Offset(1, m_CbvSrvUavDescriptorSize);
		}
	}

	// Create object constant buffers.
	{
		for (UINT i = 0; i < m_game.SceneCount(); ++i)
		{
			auto s = m_game.GetScene(i);
			//s->m_objConstantBuffers = std::make_unique<UploadBuffer<ObjectConstantBuffer>>(m_device.Get(), (UINT)(s->m_renderObjects.size()), true);
			
			// 동적으로 object가 추가될 수 있으므로, Scene에 존재할 수 있는 최대 오브젝트 개수로 초기화한다.
			// 만약 이보다 많은 수의 object가 buffer에 담겨야 한다면, UploadBuffer를 release 후 다시 초기화한다.
			s->m_objConstantBuffers = std::make_unique<UploadBuffer<ObjectConstantBuffer>>(m_device.Get(), (UINT)(200), true);
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

	WaitForPreviousFrame();

	// update object constant buffer
	auto scene = m_game.GetCurrentScene();
	scene->Update(m_timer.DeltaTime());

	// update material buffer

	auto matbuffer = m_materialBuffer.get();

	for (auto& e : Assets::m_materials)
	{
		Material* mat = e.second.get();
		if (mat->dirty)
		{
			MaterialBuffer material;
			material.diffuseMapIndex = mat->diffuseMapIndex;
			material.diffuseColor = mat->diffuseColor;

			matbuffer->CopyData(mat->bufferId, material);
			
			mat->dirty = false;
		}
	}


	// update scene constant buffer
	auto mainbuffer = m_constantBuffer.get();
	SceneConstantBuffer cBuffer;

	XMMATRIX view = scene->GetCamera()->GetViewMatrix();
	XMMATRIX proj = scene->GetCamera()->GetProjectionMatrix();
	XMStoreFloat4x4(&cBuffer.viewproj, XMMatrixTranspose(view * proj));
	cBuffer.ambientLight = { 0.25f, 0.25f, 0.35f, 1.0f };


	//XMMATRIX r = XMMatrixRotationY(m_timer.Time() * 0.1f);
	XMMATRIX r = XMMatrixIdentity();
	XMVECTOR lightDir = XMVector3TransformNormal(XMVectorSet(0.57735f, -0.57735f, 0.57735f, 0.0f), r);

	for (int i = 0; i < 3; ++i)
	{
		XMStoreFloat3(&cBuffer.Lights[i].direction, XMLoadFloat3(&mBaseLightDirections[i]));
	}

	cBuffer.Lights[0].strength = { 0.9f, 0.9f, 0.7f };
	cBuffer.Lights[1].strength = { 0.4f, 0.4f, 0.4f };
	cBuffer.Lights[2].strength = { 0.2f, 0.2f, 0.2f };
	
	cBuffer.eye = scene->GetCamera()->GetEyePosition();

	mainbuffer->CopyData(0, cBuffer);
	
}

void D3DGameEngine::Render()
{
	// 다음 frame에 실행할 command를 command list에 등록
	PopulateCommandList();

	// command list가 갖고 있는 command를 GPU의 command queue에 등록
	ID3D12CommandList* ppCommandLists[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);

	// d3d12 buffer를 먼저 그린 다음, ui를 그린다.
	RenderUI();

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

void D3DGameEngine::OnKeyDown(UINT8 key) 
{
	m_game.OnKeyDown(key);
}

void D3DGameEngine::OnKeyUp(UINT8 key)
{
	m_game.OnKeyUp(key);
}

void D3DGameEngine::OnMouseDown(UINT8 btnState, int x, int y)
{
	m_game.OnMouseDown(btnState, x, y);
}

void D3DGameEngine::OnMouseUp(UINT8 btnState, int x, int y)
{
	m_game.OnMouseUp(btnState, x, y);
}

void D3DGameEngine::OnMouseMove(UINT8 btnState, int x, int y)
{
	TRACKMOUSEEVENT mouseEvent;
	mouseEvent.cbSize = sizeof(mouseEvent);
	mouseEvent.dwFlags = TME_LEAVE;
	mouseEvent.hwndTrack = Win32Application::GetHwnd();

	TrackMouseEvent(&mouseEvent);

	m_game.OnMouseMove(btnState, x, y);
}


void D3DGameEngine::OnMouseLeave(UINT8 btnState, int x, int y)
{
	// param x, y는 client 창 좌표 기준
	// SetCursorPos의 param은 window (모니터) 좌표 기준
	m_game.OnMouseLeave(btnState, x, y);

	RECT window;
	GetWindowRect(Win32Application::GetHwnd(), &window);
	SetCursorPos((window.left + window.right) / 2, (window.top + window.bottom) / 2);
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
	ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), m_pipelineStates["opaque"].Get()));

	ID3D12DescriptorHeap* descriptorHeaps[] = { m_srvHeap.Get() };
	m_commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
	m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());

	// bind material buffer to command queue
	m_commandList->SetGraphicsRootShaderResourceView(2, m_materialBuffer->Resource()->GetGPUVirtualAddress());
	// bind texture heap to command queue
	m_commandList->SetGraphicsRootDescriptorTable(3, m_srvHeap->GetGPUDescriptorHandleForHeapStart());

	m_commandList->RSSetViewports(1, &m_viewport);
	m_commandList->RSSetScissorRects(1, &m_scissorRect);

	// back buffer texture의 state를 render target 상태로 변경.
	// GPU가 해당 resource 에 write 작업을 할 것임을 명시한다.
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), m_frameIndex, m_rtvDescriptorSize);

	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle(m_dsvHeap->GetCPUDescriptorHandleForHeapStart());

	m_commandList->OMSetRenderTargets(1, &rtvHandle, FALSE, &dsvHandle);

	const float clearColor[] = { 0.7f, 0.7f, 0.7f, 1.0f };
	m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
	m_commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, nullptr);


	auto address = m_constantBuffer.get()->Resource()->GetGPUVirtualAddress();
	
	m_commandList->SetGraphicsRootConstantBufferView(1, address);

	// 현재 scene의 indexed instance를 그린다.
	m_commandList->SetPipelineState(m_pipelineStates["opaque"].Get());
	DrawCurrentScene();

	// back buffer state 를 여기서 present로 transition하지 않는다.
	// 이후, wrapped 11on12 target resource가 released될 때 일어남.
		
	// back buffer texture의 state를 present 상태로 변경.
	// 위 command가 실행된 후 back buffer에 더이상 write 작업이 없음을 명시.
	// m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));


	// command 추가가 완료되었으므로 queue에 넘기기 전 list를 close.
	ThrowIfFailed(m_commandList->Close());
}


void D3DGameEngine::DrawCurrentScene()
{
	UINT bufferSize = (sizeof(ObjectConstantBuffer) + 255) & ~255;

	auto scene = m_game.GetCurrentScene();
	auto buffer = scene->m_objConstantBuffers.get();

	for (auto obj : scene->m_renderObjects)
	{
		if (!obj->IsActive()) continue;
		auto mesh = obj->GetRenderer()->meshDesc();

		m_commandList->IASetVertexBuffers(0, 1, &mesh->vertexBufferView);
		m_commandList->IASetIndexBuffer(&mesh->indexBufferView);
		m_commandList->IASetPrimitiveTopology(mesh->primitiveType);

		// get object constant buffer address.
		D3D12_GPU_VIRTUAL_ADDRESS address = buffer->Resource()->GetGPUVirtualAddress() + obj->GetBufferID() * bufferSize;

		m_commandList->SetGraphicsRootConstantBufferView(0, address);

		m_commandList->DrawIndexedInstanced(mesh->indexCount, 1, mesh->startIndexLocation, mesh->baseVertexLocation, mesh->startIndexLocation);
	}
}


void D3DGameEngine::RenderUI()
{
	// Acquire our wrapped render target resource for the current back buffer.
	m_d3d11On12Device->AcquireWrappedResources(m_wrappedBackBuffers[m_frameIndex].GetAddressOf(), 1);

	// Render text directly to the back buffer.
	m_d2dDeviceContext->SetTarget(m_d2dRenderTargets[m_frameIndex].Get());
	m_d2dDeviceContext->BeginDraw();
	m_d2dDeviceContext->SetTransform(D2D1::Matrix3x2F::Identity());
	
	DrawCurrentUI();
	
	ThrowIfFailed(m_d2dDeviceContext->EndDraw());

	// Release our wrapped render target resource. Releasing 
	// transitions the back buffer resource to the state specified
	// as the OutState when the wrapped resource was created.
	m_d3d11On12Device->ReleaseWrappedResources(m_wrappedBackBuffers[m_frameIndex].GetAddressOf(), 1);

	// Flush to submit the 11 command list to the shared command queue.
	m_d3d11DeviceContext->Flush();
}

void D3DGameEngine::DrawCurrentUI()
{
	
	auto objs = m_game.GetCurrentScene()->GetAllUIObjects();
	
	for (auto obj : objs)
	{
		if (!obj->IsActive()) continue;
		obj->Draw(m_d2dDeviceContext.Get());
	}

	// Render text over D3D12 using D2D via the 11On12 device.
	D2D1_SIZE_F rtSize = m_d2dRenderTargets[m_frameIndex]->GetSize();
	
	
	std::wstring info = m_game.GetCurrentScene()->GetCamera()->PrintInformation()
		+ m_game.GetPlayer()->PrintPlayerInfo();
	
	//static const WCHAR text[] = info.c_str();
	D2D1_RECT_F textRect = D2D1::RectF(0, 0, rtSize.width, rtSize.height);

	m_d2dDeviceContext->DrawTextW(
		info.c_str(),
		//_countof(text) - 1,
		info.size(),
		m_textFormat.Get(),
		&textRect,
		m_textBrush.Get()
	);

	// Render text over D3D12 using D2D via the 11On12 device.
	static const WCHAR text2[] = L"Hello ~ :)";
	D2D1_RECT_F textRect2 = D2D1::RectF(0, 0, rtSize.width, rtSize.height/ 2.0f);

	m_d2dDeviceContext->DrawTextW(
		text2,
		_countof(text2) - 1,
		m_textFormat.Get(),
		&textRect2,
		m_textBrush.Get()
	);


	//m_d2dDeviceContext->DrawBitmap(
	//	Assets::m_sprites["example"]->bitmap.Get(),
	//	&textRect,
	//	1.0f,
	//	D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR,
	//	NULL);



}

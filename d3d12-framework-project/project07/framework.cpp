#include "framework.h"

GameFramework::GameFramework(UINT width, UINT height) : 
	m_width(width), 
	m_height(height), 
	m_frameIndex{0},
	m_viewport{0.0f, 0.0f, (FLOAT)width, (FLOAT)height},
	m_scissorRect{0, 0, (LONG)width, (LONG)height}, 
	m_rtvDescriptorSize {0}
{
	m_aspectRatio = (FLOAT)width / (FLOAT)height;
}

GameFramework::~GameFramework()
{

}

void GameFramework::OnCreate(HINSTANCE hInstance, HWND hWnd)
{
	m_hInstance = hInstance;
	m_hWnd = hWnd;

	StartPipeline();

	BuildObjects(); 
}

void GameFramework::OnDestroy()
{
	WaitForGpuComplete();

	::CloseHandle(m_fenceEvent);
}

void GameFramework::StartPipeline()
{
	UINT dxgiFactoryFlags = 0;
#if defined(_DEBUG)
	ComPtr<ID3D12Debug> DebugController;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&DebugController))))
	{
		DebugController->EnableDebugLayer();
		dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
	}
#endif
	ComPtr<IDXGIFactory4> m_factory;
	DX::ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&m_factory)));

	// 1. ����̽� ����
	CreateDevice();

	// 2. �潺 ��ü ����
	CreateFence();

	// 3. 4X MSAA ǰ�� ���� ���� ���� ����
	Check4xMSAAMultiSampleQuality();

	// 4. ��� ť, ��� �Ҵ���, ��� ����Ʈ ����
	CreateCommandQueueAndList();

	// 5. ���� ü�� ����
	CreateSwapChain();

	// 6. ������ �� ����
	CreateRtvDsvDescriptorHeap();

	// 7. �ĸ� ���ۿ� ���� ���� Ÿ�� �� ����
	CreateRenderTargetView();

	// 8. ���� ���ٽ� ����, ���� ���ٽ� �� ����
	CreateDepthStencilView();

	// 9. ��Ʈ �ñ״�ó ����
	CreateRootSignature();
}

void GameFramework::CreateDevice()
{
	DX::ThrowIfFailed(CreateDXGIFactory1(IID_PPV_ARGS(&m_factory)));

	// �ϵ���� ����͸� ��Ÿ���� ��ġ�� ������ ����.
	// �ּ� ��� ������ D3D_FEATURE_LEVEL_12_0 �̴�.
	ComPtr<IDXGIAdapter1> Adapter;
	for (UINT i = 0; DXGI_ERROR_NOT_FOUND != m_factory->EnumAdapters1(i, &Adapter); ++i)
	{
		DXGI_ADAPTER_DESC1 adapterDesc;
		Adapter->GetDesc1(&adapterDesc);
		if (adapterDesc.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) continue;
		if (SUCCEEDED(D3D12CreateDevice(Adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_device)))) break;
	}

	// �����ߴٸ� WARP ����͸� ��Ÿ���� ��ġ�� �����Ѵ�.
	if (!m_device)
	{
		m_factory->EnumWarpAdapter(IID_PPV_ARGS(&Adapter));
		DX::ThrowIfFailed(D3D12CreateDevice(Adapter.Get(), D3D_FEATURE_LEVEL_12_0, IID_PPV_ARGS(&m_device)));
	}
}

void GameFramework::CreateFence()
{
	// CPU�� GPU�� ����ȭ�� ���� Fence ��ü�� �����Ѵ�.
	DX::ThrowIfFailed(m_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)));
	m_fenceEvent = CreateEvent(nullptr, false, false, nullptr);
	m_fenceValue = 1;
}

void GameFramework::Check4xMSAAMultiSampleQuality()
{
	D3D12_FEATURE_DATA_MULTISAMPLE_QUALITY_LEVELS msQualityLevels;
	msQualityLevels.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	msQualityLevels.SampleCount = 4;
	msQualityLevels.Flags = D3D12_MULTISAMPLE_QUALITY_LEVELS_FLAG_NONE;
	msQualityLevels.NumQualityLevels = 0;
	DX::ThrowIfFailed(m_device->CheckFeatureSupport(D3D12_FEATURE_MULTISAMPLE_QUALITY_LEVELS, &msQualityLevels, sizeof(msQualityLevels)));
	m_MSAA4xQualityLevel = msQualityLevels.NumQualityLevels;

	assert(m_MSAA4xQualityLevel > 0 && "Unexpected MSAA Quality Level..");
}

void GameFramework::CreateCommandQueueAndList()
{
	D3D12_COMMAND_QUEUE_DESC queueDesc{};
	queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	// ��� ť ����
	DX::ThrowIfFailed(m_device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&m_commandQueue)));
	// ��� �Ҵ��� ����
	DX::ThrowIfFailed(m_device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&m_commandAllocator)));
	// ��� ����Ʈ ����
	DX::ThrowIfFailed(m_device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, m_commandAllocator.Get(), nullptr, IID_PPV_ARGS(&m_commandList)));
	// Reset�� ȣ���ϱ� ������ Close ���·� ����
	DX::ThrowIfFailed(m_commandList->Close());
}

void GameFramework::CreateSwapChain()
{
	DXGI_SWAP_CHAIN_DESC sd{};
	sd.BufferDesc.Width = m_width;
	sd.BufferDesc.Height = m_height;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.SampleDesc.Count = m_MSAA4xQualityLevel > 1 ? 4 : 1;
	sd.SampleDesc.Quality = m_MSAA4xQualityLevel > 1 ? m_MSAA4xQualityLevel - 1 : 0;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.BufferCount = SwapChainBufferCount;
	sd.OutputWindow = m_hWnd;
	sd.Windowed = true;
	sd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;

	ComPtr<IDXGISwapChain> swapChain;
	DX::ThrowIfFailed(m_factory->CreateSwapChain(m_commandQueue.Get(), &sd, &swapChain));
	DX::ThrowIfFailed(swapChain.As(&m_swapChain));
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
}

void GameFramework::CreateRtvDsvDescriptorHeap()
{
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc{};
	rtvHeapDesc.NumDescriptors = SwapChainBufferCount;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	rtvHeapDesc.NodeMask = 0;
	DX::ThrowIfFailed(m_device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&m_rtvHeap)));
	m_rtvDescriptorSize = m_device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	D3D12_DESCRIPTOR_HEAP_DESC dsvHeapDesc{};
	dsvHeapDesc.NumDescriptors = 1;
	dsvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	dsvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
	dsvHeapDesc.NodeMask = 0;
	DX::ThrowIfFailed(m_device->CreateDescriptorHeap(&dsvHeapDesc, IID_PPV_ARGS(&m_dsvHeap)));
}

void GameFramework::CreateRenderTargetView()
{
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHeapHandle{ m_rtvHeap->GetCPUDescriptorHandleForHeapStart() };
	for (UINT i = 0; i < SwapChainBufferCount; ++i)
	{
		// ���� ü���� i��° ���۸� ��´�.
		m_swapChain->GetBuffer(i, IID_PPV_ARGS(&m_renderTargets[i]));

		// �� ���ۿ� ���� ���� Ÿ�� �並 �����Ѵ�.
		m_device->CreateRenderTargetView(m_renderTargets[i].Get(), NULL, rtvHeapHandle);

		// ���� ���� �׸����� �Ѿ��.
		rtvHeapHandle.Offset(1, m_rtvDescriptorSize);
	}
}

void GameFramework::CreateDepthStencilView()
{
	D3D12_RESOURCE_DESC depthStencilDesc{};
	depthStencilDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	depthStencilDesc.Alignment = 0;
	depthStencilDesc.Width = m_width;
	depthStencilDesc.Height = m_height;
	depthStencilDesc.DepthOrArraySize = 1;
	depthStencilDesc.MipLevels = 1;
	depthStencilDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilDesc.SampleDesc.Count = m_MSAA4xQualityLevel > 1 ? 4 : 1;
	depthStencilDesc.SampleDesc.Quality = m_MSAA4xQualityLevel > 1 ? m_MSAA4xQualityLevel - 1 : 0;
	depthStencilDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	depthStencilDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE optClear{};
	optClear.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	optClear.DepthStencil.Depth = 1.0f;
	optClear.DepthStencil.Stencil = 0;

	DX::ThrowIfFailed(m_device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&depthStencilDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE,
		&optClear,
		IID_PPV_ARGS(m_depthStencil.GetAddressOf())));

	// D3D12_DEPTH_STENCIL_VIEW_DESC ����ü�� ���� ���ٽ� �並 �����ϴµ�, 
	// �ڿ��� ��� ���ҵ��� �ڷ� ���Ŀ� ���� ����� ������ �ִ�.
	D3D12_DEPTH_STENCIL_VIEW_DESC depthStencilViewDesc{};
	depthStencilViewDesc.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	depthStencilViewDesc.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	depthStencilViewDesc.Flags = D3D12_DSV_FLAG_NONE;
	m_device->CreateDepthStencilView(m_depthStencil.Get(), &depthStencilViewDesc, m_dsvHeap->GetCPUDescriptorHandleForHeapStart());
}

void GameFramework::CreateRootSignature()
{
	CD3DX12_ROOT_PARAMETER rootParameter[2];

	// cbGameObject : ���� ��ȯ ���(16)
	rootParameter[0].InitAsConstants(16, 0, 0, D3D12_SHADER_VISIBILITY_ALL);

	// cbCamera : �� ��ȯ ���(16) + ���� ��ȯ ���(16)
	rootParameter[1].InitAsConstants(32, 1, 0, D3D12_SHADER_VISIBILITY_ALL);

	CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
	rootSignatureDesc.Init(_countof(rootParameter), rootParameter, 0, NULL, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

	ComPtr<ID3DBlob> signature, error;
	DX::ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
	DX::ThrowIfFailed(m_device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&m_rootSignature)));
}


void GameFramework::BuildObjects()
{
	m_commandList->Reset(m_commandAllocator.Get(), nullptr);

	m_scene = make_unique<Scene>();

	// Render only vertice buffer
	//vector<Vertex> vertices;
	//vertices.emplace_back(XMFLOAT3{ -0.5f, +0.5f, -0.5f }, XMFLOAT4{ 1.0f, 0.0f, 0.0f, 1.0f });
	//vertices.emplace_back(XMFLOAT3{ +0.5f, +0.5f, -0.5f }, XMFLOAT4{ 0.0f, 1.0f, 0.0f, 1.0f });
	//vertices.emplace_back(XMFLOAT3{ +0.5f, -0.5f, -0.5f }, XMFLOAT4{ 0.0f, 0.0f, 1.0f, 1.0f });

	//vertices.emplace_back(XMFLOAT3{ -0.5f, +0.5f, -0.5f }, XMFLOAT4{ 1.0f, 0.0f, 0.0f, 1.0f });
	//vertices.emplace_back(XMFLOAT3{ +0.5f, -0.5f, -0.5f }, XMFLOAT4{ 0.0f, 1.0f, 0.0f, 1.0f });
	//vertices.emplace_back(XMFLOAT3{ -0.5f, -0.5f, -0.5f }, XMFLOAT4{ 0.0f, 0.0f, 1.0f, 1.0f });

	//vertices.emplace_back(XMFLOAT3{ -0.5f, +0.5f, +0.5f }, XMFLOAT4{ 1.0f, 0.0f, 0.0f, 1.0f });
	//vertices.emplace_back(XMFLOAT3{ +0.5f, +0.5f, +0.5f }, XMFLOAT4{ 0.0f, 1.0f, 0.0f, 1.0f });
	//vertices.emplace_back(XMFLOAT3{ +0.5f, +0.5f, -0.5f }, XMFLOAT4{ 0.0f, 0.0f, 1.0f, 1.0f });

	//vertices.emplace_back(XMFLOAT3{ -0.5f, +0.5f, +0.5f }, XMFLOAT4{ 1.0f, 0.0f, 0.0f, 1.0f });
	//vertices.emplace_back(XMFLOAT3{ +0.5f, +0.5f, -0.5f }, XMFLOAT4{ 0.0f, 1.0f, 0.0f, 1.0f });
	//vertices.emplace_back(XMFLOAT3{ -0.5f, +0.5f, -0.5f }, XMFLOAT4{ 0.0f, 0.0f, 1.0f, 1.0f });

	//vertices.emplace_back(XMFLOAT3{ -0.5f, -0.5f, +0.5f }, XMFLOAT4{ 1.0f, 0.0f, 0.0f, 1.0f });
	//vertices.emplace_back(XMFLOAT3{ +0.5f, -0.5f, +0.5f }, XMFLOAT4{ 0.0f, 1.0f, 0.0f, 1.0f });
	//vertices.emplace_back(XMFLOAT3{ +0.5f, +0.5f, +0.5f }, XMFLOAT4{ 0.0f, 0.0f, 1.0f, 1.0f });

	//vertices.emplace_back(XMFLOAT3{ -0.5f, -0.5f, +0.5f }, XMFLOAT4{ 1.0f, 0.0f, 0.0f, 1.0f });
	//vertices.emplace_back(XMFLOAT3{ +0.5f, +0.5f, +0.5f }, XMFLOAT4{ 0.0f, 1.0f, 0.0f, 1.0f });
	//vertices.emplace_back(XMFLOAT3{ -0.5f, +0.5f, +0.5f }, XMFLOAT4{ 0.0f, 0.0f, 1.0f, 1.0f });

	//vertices.emplace_back(XMFLOAT3{ -0.5f, -0.5f, -0.5f }, XMFLOAT4{ 1.0f, 0.0f, 0.0f, 1.0f });
	//vertices.emplace_back(XMFLOAT3{ +0.5f, -0.5f, -0.5f }, XMFLOAT4{ 0.0f, 1.0f, 0.0f, 1.0f });
	//vertices.emplace_back(XMFLOAT3{ +0.5f, -0.5f, +0.5f }, XMFLOAT4{ 0.0f, 0.0f, 1.0f, 1.0f });

	//vertices.emplace_back(XMFLOAT3{ -0.5f, -0.5f, -0.5f }, XMFLOAT4{ 1.0f, 0.0f, 0.0f, 1.0f });
	//vertices.emplace_back(XMFLOAT3{ +0.5f, -0.5f, +0.5f }, XMFLOAT4{ 0.0f, 1.0f, 0.0f, 1.0f });
	//vertices.emplace_back(XMFLOAT3{ -0.5f, -0.5f, +0.5f }, XMFLOAT4{ 0.0f, 0.0f, 1.0f, 1.0f });

	//vertices.emplace_back(XMFLOAT3{ -0.5f, +0.5f, +0.5f }, XMFLOAT4{ 1.0f, 0.0f, 0.0f, 1.0f });
	//vertices.emplace_back(XMFLOAT3{ -0.5f, +0.5f, -0.5f }, XMFLOAT4{ 0.0f, 1.0f, 0.0f, 1.0f });
	//vertices.emplace_back(XMFLOAT3{ -0.5f, -0.5f, -0.5f }, XMFLOAT4{ 0.0f, 0.0f, 1.0f, 1.0f });

	//vertices.emplace_back(XMFLOAT3{ -0.5f, +0.5f, +0.5f }, XMFLOAT4{ 1.0f, 0.0f, 0.0f, 1.0f });
	//vertices.emplace_back(XMFLOAT3{ -0.5f, -0.5f, -0.5f }, XMFLOAT4{ 0.0f, 1.0f, 0.0f, 1.0f });
	//vertices.emplace_back(XMFLOAT3{ -0.5f, -0.5f, +0.5f }, XMFLOAT4{ 0.0f, 0.0f, 1.0f, 1.0f });

	//vertices.emplace_back(XMFLOAT3{ +0.5f, +0.5f, -0.5f }, XMFLOAT4{ 1.0f, 0.0f, 0.0f, 1.0f });
	//vertices.emplace_back(XMFLOAT3{ +0.5f, +0.5f, +0.5f }, XMFLOAT4{ 0.0f, 1.0f, 0.0f, 1.0f });
	//vertices.emplace_back(XMFLOAT3{ +0.5f, -0.5f, +0.5f }, XMFLOAT4{ 0.0f, 0.0f, 1.0f, 1.0f });

	//vertices.emplace_back(XMFLOAT3{ +0.5f, +0.5f, -0.5f }, XMFLOAT4{ 1.0f, 0.0f, 0.0f, 1.0f });
	//vertices.emplace_back(XMFLOAT3{ +0.5f, -0.5f, +0.5f }, XMFLOAT4{ 0.0f, 1.0f, 0.0f, 1.0f });
	//vertices.emplace_back(XMFLOAT3{ +0.5f, -0.5f, -0.5f }, XMFLOAT4{ 0.0f, 0.0f, 1.0f, 1.0f });

	//vector<UINT> indices;


	// Render by index buffer
	vector<Vertex> vertices;
	vertices.emplace_back(XMFLOAT3{ -0.5f, +0.5f, +0.5f }, XMFLOAT4{ 1.0f, 0.0f, 0.0f, 1.0f });
	vertices.emplace_back(XMFLOAT3{ +0.5f, +0.5f, +0.5f }, XMFLOAT4{ 0.0f, 1.0f, 0.0f, 1.0f });
	vertices.emplace_back(XMFLOAT3{ +0.5f, +0.5f, -0.5f }, XMFLOAT4{ 0.0f, 0.0f, 1.0f, 1.0f });
	vertices.emplace_back(XMFLOAT3{ -0.5f, +0.5f, -0.5f }, XMFLOAT4{ 1.0f, 1.0f, 0.0f, 1.0f });

	vertices.emplace_back(XMFLOAT3{ -0.5f, -0.5f, +0.5f }, XMFLOAT4{ 1.0f, 0.0f, 1.0f, 1.0f });
	vertices.emplace_back(XMFLOAT3{ +0.5f, -0.5f, +0.5f }, XMFLOAT4{ 0.0f, 1.0f, 1.0f, 1.0f });
	vertices.emplace_back(XMFLOAT3{ +0.5f, -0.5f, -0.5f }, XMFLOAT4{ 0.5f, 0.5f, 0.0f, 1.0f });
	vertices.emplace_back(XMFLOAT3{ -0.5f, -0.5f, -0.5f }, XMFLOAT4{ 0.0f, 0.5f, 0.5f, 1.0f });

	vector<UINT> indices;
	indices.push_back(0); indices.push_back(1); indices.push_back(2);
	indices.push_back(0); indices.push_back(2); indices.push_back(3);

	indices.push_back(3); indices.push_back(2); indices.push_back(6);
	indices.push_back(3); indices.push_back(6); indices.push_back(7);

	indices.push_back(7); indices.push_back(6); indices.push_back(5);
	indices.push_back(7); indices.push_back(5); indices.push_back(4);

	indices.push_back(1); indices.push_back(0); indices.push_back(4);
	indices.push_back(1); indices.push_back(4); indices.push_back(5);

	indices.push_back(0); indices.push_back(3); indices.push_back(7);
	indices.push_back(0); indices.push_back(7); indices.push_back(4);

	indices.push_back(2); indices.push_back(1); indices.push_back(5);
	indices.push_back(2); indices.push_back(5); indices.push_back(6);
	
	Mesh cube{ m_device, m_commandList, vertices, indices };
	shared_ptr<Mesh> mesh{ make_shared<Mesh>(cube) };

	shared_ptr<Shader> shader{ make_shared<Shader>(m_device, m_rootSignature) };

	// ���ӿ�����Ʈ ����
	for (int i = 0; i < 1000; ++i) {
		unique_ptr<RotatingObject> obj{ make_unique<RotatingObject>() };
		obj->SetPosition(XMFLOAT3(i % 10 * 5, (i / 10) % 10 * 5, (i / 100) % 10 * 5));
		obj->SetMesh(mesh);
		obj->SetShader(shader);
		m_scene->GetGameObjects().push_back(move(obj));
	}

	// �÷��̾� ����
	shared_ptr<Player> player{ make_shared<Player>() };
	player->SetMesh(make_unique <Mesh>(m_device, m_commandList, vertices, indices));
	player->SetShader(shader);
	m_scene->SetPlayer(player);

	// ī�޶� ����
	shared_ptr<ThirdPersonCamera> camera{ make_shared<ThirdPersonCamera>() };
	camera->SetEye(XMFLOAT3{ 0.0f, 0.0f, 0.0f });
	camera->SetAt(XMFLOAT3{ 0.0f, 0.0f, 1.0f });
	camera->SetUp(XMFLOAT3{ 0.0f, 1.0f, 0.0f });
	camera->SetPlayer(m_scene->GetPlayer());

	XMFLOAT4X4 projMatrix;
	XMStoreFloat4x4(&projMatrix, XMMatrixPerspectiveFovLH(0.25f * XM_PI, m_aspectRatio, 0.1f, 1000.0f));
	camera->SetProjMatrix(projMatrix);
	m_scene->SetCamera(camera);

	// �÷��̾� ī�޶� ����
	m_scene->GetPlayer()->SetCamera(m_scene->GetCamera());


	// ��� ����
	m_commandList->Close();
	ID3D12CommandList* ppCommandList[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandList), ppCommandList);

	// ��ɵ��� �Ϸ�� ������ ���
	WaitForGpuComplete();

	// ����Ʈ ���۷��� ���簡 �Ϸ�����Ƿ� ���ε� ���۸� �����Ѵ�.
	for (const auto& obj : m_scene->GetGameObjects())
		obj->ReleaseUploadBuffer();

	m_timer.Tick();
}

void GameFramework::FrameAdvance()
{
	m_timer.Tick();

	Update(m_timer.GetDeltaTime());
	Render();
}

void GameFramework::Update(FLOAT timeElapsed)
{
	if (m_scene) m_scene->Update(timeElapsed);
}

void GameFramework::WaitForGpuComplete()
{
	//GPU�� �潺�� ���� �����ϴ� ����� ��� ť�� �߰��Ѵ�. 
	const UINT64 fence = m_fenceValue;
	DX::ThrowIfFailed(m_commandQueue->Signal(m_fence.Get(), fence));
	//CPU �潺�� ���� ������Ų��. 
	++m_fenceValue;

	//�潺�� ���� ���� ������ ������ ������ �潺�� ���� ���� ������ ���� �� ������ ��ٸ���. 
	if (m_fence->GetCompletedValue() < fence)
	{
		DX::ThrowIfFailed(m_fence->SetEventOnCompletion(fence, m_fenceEvent));
		::WaitForSingleObject(m_fenceEvent, INFINITE);
	}
	m_frameIndex = m_swapChain->GetCurrentBackBufferIndex();
}

void GameFramework::Render()
{
	// ��� �Ҵ��ڿ� ��� ����Ʈ�� �����Ѵ�. 
	DX::ThrowIfFailed(m_commandAllocator->Reset());
	DX::ThrowIfFailed(m_commandList->Reset(m_commandAllocator.Get(), nullptr));

	// �ڿ� �뵵�� ���õ� ���� ���̸� Direct3D�� �����Ѵ�.
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

	//����Ʈ�� ���� �簢���� �����Ѵ�. 
	m_commandList->SetGraphicsRootSignature(m_rootSignature.Get());
	m_commandList->RSSetViewports(1, &m_viewport);
	m_commandList->RSSetScissorRects(1, &m_scissorRect);

	//������ ���� Ÿ�ٿ� �ش��ϴ� �����ڿ� ���� ���ٽ� �������� CPU �ּ�(�ڵ�)�� ����Ѵ�. 
	CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle{ m_rtvHeap->GetCPUDescriptorHandleForHeapStart(), static_cast<INT>(m_frameIndex), m_rtvDescriptorSize };
	CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle{ m_dsvHeap->GetCPUDescriptorHandleForHeapStart() };
	m_commandList->OMSetRenderTargets(1, &rtvHandle, TRUE, &dsvHandle);

	// ���ϴ� �������� ���� Ÿ���� �����, ���ϴ� ������ ���� ���ٽ� �並 �����.
	const FLOAT clearColor[]{ 0.0f, 0.125f, 0.3f, 1.0f };
	m_commandList->ClearRenderTargetView(rtvHandle, clearColor, 0, NULL);
	m_commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_DEPTH | D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);

	// Scene�� Render�Ѵ�.
	m_scene->Render(m_commandList);

	// �ڿ� �뵵�� ���õ� ���� ���̸� Direct3D�� �����Ѵ�.
	m_commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(m_renderTargets[m_frameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

	// ��ɵ��� ����� ��ģ��.
	DX::ThrowIfFailed(m_commandList->Close());

	
	// ��� ������ ���� Ŀ�ǵ� ����Ʈ�� Ŀ�ǵ� ť�� �߰��Ѵ�.
	ID3D12CommandList* ppCommandList[] = { m_commandList.Get() };
	m_commandQueue->ExecuteCommandLists(_countof(ppCommandList), ppCommandList);

	DX::ThrowIfFailed(m_swapChain->Present(1, 0));

	WaitForGpuComplete();
}
#include "shader.h"

Shader::Shader(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12RootSignature>& rootSignature) : Shader{}
{
	ComPtr<ID3DBlob> mvsByteCode;
	ComPtr<ID3DBlob> mpsByteCode;

#if defined(_DEBUG)
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif

	DX::ThrowIfFailed(D3DCompileFromFile(TEXT("Resource/Shader/Shaders.hlsl"), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VS_STANDARD_MAIN", "vs_5_1", compileFlags, 0, &mvsByteCode, nullptr));
	DX::ThrowIfFailed(D3DCompileFromFile(TEXT("Resource/Shader/Shaders.hlsl"), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PS_STANDARD_MAIN", "ps_5_1", compileFlags, 0, &mpsByteCode, nullptr));

	m_inputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	psoDesc.InputLayout = { m_inputLayout.data(), (UINT)m_inputLayout.size() };
	psoDesc.pRootSignature = rootSignature.Get();
	psoDesc.VS = CD3DX12_SHADER_BYTECODE(mvsByteCode.Get());
	psoDesc.PS = CD3DX12_SHADER_BYTECODE(mpsByteCode.Get());
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	DX::ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));

}

Shader::~Shader()
{
	ReleaseShaderVariable();
}

void Shader::Update(FLOAT timeElapsed)
{
	if (m_player) {
		m_player->Update(timeElapsed);
	}

	for (const auto& elm : m_gameObjects)
		if (elm) elm->Update(timeElapsed);
}

void Shader::Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) const
{
	Shader::UpdateShaderVariable(commandList);

	if (m_player) m_player->Render(commandList);

	for (const auto& elm : m_gameObjects)
		if (elm) elm->Render(commandList);
}

void Shader::UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& commandList) const
{
	commandList->SetPipelineState(m_pipelineState.Get());
}

void Shader::ReleaseShaderVariable()
{

}

void Shader::ReleaseUploadBuffer() const
{
	if (m_player) m_player->ReleaseUploadBuffer();

	for (const auto& elm : m_gameObjects)
		if (elm) elm->ReleaseUploadBuffer();
}

void Shader::SetPlayer(const shared_ptr<Player>& player)
{
	if (m_player) m_player.reset();
	m_player = player;
}

void Shader::SetCamera(const shared_ptr<Camera>& camera)
{
	if (m_camera) m_camera.reset();
	m_camera = camera;
}

TerrainShader::TerrainShader(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12RootSignature>& rootSignature)
{
	CreatePipelineState(device, rootSignature);
}

void TerrainShader::CreatePipelineState(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12RootSignature>& rootSignature)
{
	ComPtr<ID3DBlob> vertexShader, pixelShader;

#if defined(_DEBUG)
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif

	DX::ThrowIfFailed(D3DCompileFromFile(TEXT("Resource/Shader/Shaders.hlsl"), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VS_TERRAIN_MAIN", "vs_5_1", compileFlags, 0, &vertexShader, nullptr));
	DX::ThrowIfFailed(D3DCompileFromFile(TEXT("Resource/Shader/Shaders.hlsl"), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PS_TERRAIN_MAIN", "ps_5_1", compileFlags, 0, &pixelShader, nullptr));

	// ���� ���̴� ���̾ƿ� ����
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[]
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 1, DXGI_FORMAT_R32G32_FLOAT, 0, 20, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	// PSO ����
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
	psoDesc.pRootSignature = rootSignature.Get();
	psoDesc.VS = CD3DX12_SHADER_BYTECODE(vertexShader.Get());
	psoDesc.PS = CD3DX12_SHADER_BYTECODE(pixelShader.Get());
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	psoDesc.SampleDesc.Count = 1;
	DX::ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));
}

void TerrainShader::ReleaseUploadBuffer() const
{
	if (m_heightMap) m_heightMap->ReleaseUploadBuffer();
}

void TerrainShader::Update(FLOAT timeElapsed)
{
	if (m_heightMap) m_heightMap->Update(timeElapsed);
}

void TerrainShader::Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) const
{
	UpdateShaderVariable(commandList);

	if (m_heightMap) { m_heightMap->Render(commandList); }
}

InstancingShader::InstancingShader(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12RootSignature>& rootSignature, const Mesh& mesh, UINT count) : 
	m_instancingCount(count)
{
	ComPtr<ID3DBlob> mvsByteCode;
	ComPtr<ID3DBlob> mpsByteCode;

#if defined(_DEBUG)
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif

	DX::ThrowIfFailed(D3DCompileFromFile(TEXT("Resource/Shader/Shaders.hlsl"), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VS_INSTANCE_MAIN", "vs_5_1", compileFlags, 0, &mvsByteCode, nullptr));
	DX::ThrowIfFailed(D3DCompileFromFile(TEXT("Resource/Shader/Shaders.hlsl"), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PS_INSTANCE_MAIN", "ps_5_1", compileFlags, 0, &mpsByteCode, nullptr));
	
	m_inputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "INSTANCE", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1}, 
		{ "INSTANCE", 1, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 16, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1},
		{ "INSTANCE", 2, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 32, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1},
		{ "INSTANCE", 3, DXGI_FORMAT_R32G32B32A32_FLOAT, 1, 48, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1},
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	psoDesc.InputLayout = { m_inputLayout.data(), (UINT)m_inputLayout.size() };
	psoDesc.pRootSignature = rootSignature.Get();
	psoDesc.VS = CD3DX12_SHADER_BYTECODE(mvsByteCode.Get());
	psoDesc.PS = CD3DX12_SHADER_BYTECODE(mpsByteCode.Get());
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	DX::ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));

	CreateShaderVariable(device);
	m_mesh = make_unique<Mesh>(mesh);
}

void InstancingShader::Update(FLOAT timeElapsed)
{
	if (m_player) m_player->Update(timeElapsed);
	for (const auto& elm : m_gameObjects)
		if (elm) elm->Update(timeElapsed);
}

void InstancingShader::Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) const
{
	InstancingShader::UpdateShaderVariable(commandList);

	int i = 0;
	for (const auto& elm : m_gameObjects) {
		m_instancingBufferPointer[i++].worldMatrix = Matrix::Transpose(elm->GetWorldMatrix());
	}

	m_mesh->Render(commandList, m_instancingBufferView);
}

void InstancingShader::CreateShaderVariable(const ComPtr<ID3D12Device>& device)
{
	DX::ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(InstancingData) * m_instancingCount),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		NULL,
		IID_PPV_ARGS(&m_instancingBuffer)));

	// �ν��Ͻ� ���� ������
	m_instancingBuffer->Map(0, NULL, reinterpret_cast<void**>(&m_instancingBufferPointer));

	// �ν��Ͻ� ���� �� ����
	m_instancingBufferView.BufferLocation = m_instancingBuffer->GetGPUVirtualAddress();
	m_instancingBufferView.StrideInBytes = sizeof(InstancingData);
	m_instancingBufferView.SizeInBytes = sizeof(InstancingData) * m_instancingCount;
}

void InstancingShader::UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& commandList) const
{
	commandList->SetPipelineState(m_pipelineState.Get());
}

void InstancingShader::ReleaseShaderVariable()
{
	if (m_instancingBuffer) m_instancingBuffer->Unmap(0, nullptr);
}

TextureHierarchyShader::TextureHierarchyShader(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12RootSignature>& rootSignature)
{
	ComPtr<ID3DBlob> mvsByteCode;
	ComPtr<ID3DBlob> mpsByteCode;

#if defined(_DEBUG)
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif
	DX::ThrowIfFailed(D3DCompileFromFile(TEXT("Resource/Shader/Shaders.hlsl"), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VS_TEXTUREHIERARCHY_MAIN", "vs_5_1", compileFlags, 0, &mvsByteCode, nullptr));
	DX::ThrowIfFailed(D3DCompileFromFile(TEXT("Resource/Shader/Shaders.hlsl"), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PS_TEXTUREHIERARCHY_MAIN", "ps_5_1", compileFlags, 0, &mpsByteCode, nullptr));

	m_inputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 48, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	psoDesc.InputLayout = { m_inputLayout.data(), (UINT)m_inputLayout.size() };
	psoDesc.pRootSignature = rootSignature.Get();
	psoDesc.VS = CD3DX12_SHADER_BYTECODE(mvsByteCode.Get());
	psoDesc.PS = CD3DX12_SHADER_BYTECODE(mpsByteCode.Get());
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	DX::ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));
}


SkyboxShader::SkyboxShader(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12RootSignature>& rootSignature)
{
	ComPtr<ID3DBlob> mvsByteCode;
	ComPtr<ID3DBlob> mpsByteCode;

#if defined(_DEBUG)
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif

	DX::ThrowIfFailed(D3DCompileFromFile(TEXT("Resource/Shader/Shaders.hlsl"), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VS_SKYBOX_MAIN", "vs_5_1", compileFlags, 0, &mvsByteCode, nullptr));
	DX::ThrowIfFailed(D3DCompileFromFile(TEXT("Resource/Shader/Shaders.hlsl"), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PS_SKYBOX_MAIN", "ps_5_1", compileFlags, 0, &mpsByteCode, nullptr));

	m_inputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	psoDesc.InputLayout = { m_inputLayout.data(), (UINT)m_inputLayout.size() };
	psoDesc.pRootSignature = rootSignature.Get();
	psoDesc.VS = CD3DX12_SHADER_BYTECODE(mvsByteCode.Get());
	psoDesc.PS = CD3DX12_SHADER_BYTECODE(mpsByteCode.Get());
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);

	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	DX::ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));
}

BlendingShader::BlendingShader(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12RootSignature>& rootSignature)
{
	ComPtr<ID3DBlob> mvsByteCode;
	ComPtr<ID3DBlob> mpsByteCode;

#if defined(_DEBUG)
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif

	DX::ThrowIfFailed(D3DCompileFromFile(TEXT("Resource/Shader/Shaders.hlsl"), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VS_BLENDING_MAIN", "vs_5_1", compileFlags, 0, &mvsByteCode, nullptr));
	DX::ThrowIfFailed(D3DCompileFromFile(TEXT("Resource/Shader/Shaders.hlsl"), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PS_BLENDING_MAIN", "ps_5_1", compileFlags, 0, &mpsByteCode, nullptr));

	m_inputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	psoDesc.InputLayout = { m_inputLayout.data(), (UINT)m_inputLayout.size() };
	psoDesc.pRootSignature = rootSignature.Get();
	psoDesc.VS = CD3DX12_SHADER_BYTECODE(mvsByteCode.Get());
	psoDesc.PS = CD3DX12_SHADER_BYTECODE(mpsByteCode.Get());
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);

	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;

	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.BlendState.AlphaToCoverageEnable = TRUE;
	psoDesc.BlendState.IndependentBlendEnable = FALSE;
	psoDesc.BlendState.RenderTarget[0].BlendEnable = TRUE;
	psoDesc.BlendState.RenderTarget[0].LogicOpEnable = FALSE;
	psoDesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	psoDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	psoDesc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	psoDesc.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	psoDesc.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	psoDesc.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	psoDesc.BlendState.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	DX::ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));
}

BillBoardShader::BillBoardShader(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12RootSignature>& rootSignature, UINT count) :
	m_instancingCount(count)
{
	ComPtr<ID3DBlob> mvsByteCode;
	ComPtr<ID3DBlob> mgsByteCode;
	ComPtr<ID3DBlob> mpsByteCode;

#if defined(_DEBUG)
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif

	DX::ThrowIfFailed(D3DCompileFromFile(TEXT("Resource/Shader/Shaders.hlsl"), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VS_BILLBOARD_MAIN", "vs_5_1", compileFlags, 0, &mvsByteCode, nullptr));
	DX::ThrowIfFailed(D3DCompileFromFile(TEXT("Resource/Shader/Shaders.hlsl"), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "GS_BILLBOARD_MAIN", "gs_5_1", compileFlags, 0, &mgsByteCode, nullptr));
	DX::ThrowIfFailed(D3DCompileFromFile(TEXT("Resource/Shader/Shaders.hlsl"), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PS_BILLBOARD_MAIN", "ps_5_1", compileFlags, 0, &mpsByteCode, nullptr));

	m_inputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "SIZE", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "WPOSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 1, 0, D3D12_INPUT_CLASSIFICATION_PER_INSTANCE_DATA, 1 }
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	psoDesc.InputLayout = { m_inputLayout.data(), (UINT)m_inputLayout.size() };
	psoDesc.pRootSignature = rootSignature.Get();
	psoDesc.VS = CD3DX12_SHADER_BYTECODE(mvsByteCode.Get());
	psoDesc.GS = CD3DX12_SHADER_BYTECODE(mgsByteCode.Get());
	psoDesc.PS = CD3DX12_SHADER_BYTECODE(mpsByteCode.Get());
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);

	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;

	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.BlendState.AlphaToCoverageEnable = TRUE;
	psoDesc.BlendState.IndependentBlendEnable = FALSE;
	psoDesc.BlendState.RenderTarget[0].BlendEnable = TRUE;
	psoDesc.BlendState.RenderTarget[0].LogicOpEnable = FALSE;
	psoDesc.BlendState.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	psoDesc.BlendState.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	psoDesc.BlendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	psoDesc.BlendState.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	psoDesc.BlendState.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	psoDesc.BlendState.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	psoDesc.BlendState.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	DX::ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));

	CreateShaderVariable(device);
}

void BillBoardShader::Update(FLOAT timeElapsed)
{
	for (const auto& elm : m_gameObjects)
		if (elm) elm->Update(timeElapsed);
}

void BillBoardShader::Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) const
{
	BillBoardShader::UpdateShaderVariable(commandList);

	if (m_texture) { m_texture->UpdateShaderVariable(commandList); }
	if (m_material) { m_material->UpdateShaderVariable(commandList); }

	int i = 0;
	for (const auto& elm : m_gameObjects) {
		m_instancingBufferPointer[i++].position = elm->GetPosition();
	}

	m_mesh->Render(commandList, m_instancingBufferView);
}

void BillBoardShader::CreateShaderVariable(const ComPtr<ID3D12Device>& device)
{
	DX::ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(sizeof(BillBoardInstancingData) * m_instancingCount),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		NULL,
		IID_PPV_ARGS(&m_instancingBuffer)));

	// �ν��Ͻ� ���� ������
	m_instancingBuffer->Map(0, NULL, reinterpret_cast<void**>(&m_instancingBufferPointer));

	// �ν��Ͻ� ���� �� ����
	m_instancingBufferView.BufferLocation = m_instancingBuffer->GetGPUVirtualAddress();
	m_instancingBufferView.StrideInBytes = sizeof(BillBoardInstancingData);
	m_instancingBufferView.SizeInBytes = sizeof(BillBoardInstancingData) * m_instancingCount;
}

void BillBoardShader::UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& commandList) const
{
	commandList->SetPipelineState(m_pipelineState.Get());
}

void BillBoardShader::ReleaseShaderVariable()
{
	if (m_instancingBuffer) m_instancingBuffer->Unmap(0, nullptr);
}

ParticleShader::ParticleShader(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12RootSignature>& rootSignature) : Shader{}
{
	ComPtr<ID3DBlob> mvsByteCode;
	ComPtr<ID3DBlob> mgsdByteCode;
	ComPtr<ID3DBlob> mgssoByteCode;
	ComPtr<ID3DBlob> mpsByteCode;

#if defined(_DEBUG)
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif

	DX::ThrowIfFailed(D3DCompileFromFile(TEXT("Resource/Shader/Shaders.hlsl"), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VS_PARTICLE_MAIN", "vs_5_1", compileFlags, 0, &mvsByteCode, nullptr));
	DX::ThrowIfFailed(D3DCompileFromFile(TEXT("Resource/Shader/Shaders.hlsl"), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "GS_PARTICLE_DRAW", "gs_5_1", compileFlags, 0, &mgsdByteCode, nullptr));
	DX::ThrowIfFailed(D3DCompileFromFile(TEXT("Resource/Shader/Shaders.hlsl"), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "GS_PARTICLE_STREAMOUTPUT", "gs_5_1", compileFlags, 0, &mgssoByteCode, nullptr));
	DX::ThrowIfFailed(D3DCompileFromFile(TEXT("Resource/Shader/Shaders.hlsl"), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PS_PARTICLE_MAIN", "ps_5_1", compileFlags, 0, &mpsByteCode, nullptr));

	m_inputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "VELOCITY", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "AGE", 0, DXGI_FORMAT_R32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "LIFETIME", 0, DXGI_FORMAT_R32_FLOAT, 0, 28, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};


	D3D12_SO_DECLARATION_ENTRY* streamOutputDecls{ new D3D12_SO_DECLARATION_ENTRY[4] };
	streamOutputDecls[0] = { 0, "POSITION", 0, 0, 3, 0 };
	streamOutputDecls[1] = { 0, "VELOCITY", 0, 0, 3, 0 };
	streamOutputDecls[2] = { 0, "AGE", 0, 0, 1, 0 };
	streamOutputDecls[3] = { 0, "LIFETIME", 0, 0, 1, 0 };

	UINT* bufferStrides{ new UINT[1] };
	bufferStrides[0] = sizeof(ParticleVertex);

	D3D12_STREAM_OUTPUT_DESC streamOutput{};
	streamOutput.NumEntries = 4;
	streamOutput.pSODeclaration = streamOutputDecls;
	streamOutput.NumStrides = 1;
	streamOutput.pBufferStrides = bufferStrides;
	streamOutput.RasterizedStream = D3D12_SO_NO_RASTERIZED_STREAM;

	CD3DX12_DEPTH_STENCIL_DESC depthStencilState{ D3D12_DEFAULT };
	depthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;

	CD3DX12_BLEND_DESC blendState{ D3D12_DEFAULT };
	blendState.RenderTarget[0].BlendEnable = TRUE;
	blendState.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendState.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	blendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC streamPsoDesc{};
	streamPsoDesc.InputLayout = { m_inputLayout.data(), (UINT)m_inputLayout.size() };
	streamPsoDesc.pRootSignature = rootSignature.Get();
	streamPsoDesc.VS = CD3DX12_SHADER_BYTECODE(mvsByteCode.Get());
	streamPsoDesc.GS = CD3DX12_SHADER_BYTECODE(mgssoByteCode.Get());
	streamPsoDesc.StreamOutput = streamOutput;
	streamPsoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	streamPsoDesc.DepthStencilState = depthStencilState;
	streamPsoDesc.BlendState = blendState;
	streamPsoDesc.SampleMask = UINT_MAX;
	streamPsoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	streamPsoDesc.NumRenderTargets = 0;
	streamPsoDesc.RTVFormats[0] = DXGI_FORMAT_UNKNOWN;
	streamPsoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	streamPsoDesc.SampleDesc.Count = 1;
	DX::ThrowIfFailed(device->CreateGraphicsPipelineState(&streamPsoDesc, IID_PPV_ARGS(&m_streamPipelineState)));

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	psoDesc.InputLayout = { m_inputLayout.data(), (UINT)m_inputLayout.size() };
	psoDesc.pRootSignature = rootSignature.Get();
	psoDesc.VS = CD3DX12_SHADER_BYTECODE(mvsByteCode.Get());
	psoDesc.GS = CD3DX12_SHADER_BYTECODE(mgsdByteCode.Get());
	psoDesc.PS = CD3DX12_SHADER_BYTECODE(mpsByteCode.Get());
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = depthStencilState;
	psoDesc.BlendState = blendState;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_POINT;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	psoDesc.SampleDesc.Count = 1;
	DX::ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));

}

void ParticleShader::Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) const
{
	for (auto& particle : m_particles) {
		commandList->SetPipelineState(m_streamPipelineState.Get());
		particle->RenderStreamOutput(commandList);

		commandList->SetPipelineState(m_pipelineState.Get());
		particle->Render(commandList);
	}
}

StencilRenderShader::StencilRenderShader(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12RootSignature>& rootSignature)
{
	ComPtr<ID3DBlob> mvsByteCode;
	ComPtr<ID3DBlob> mpsByteCode;

#if defined(_DEBUG)
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif
	DX::ThrowIfFailed(D3DCompileFromFile(TEXT("Resource/Shader/Shaders.hlsl"), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VS_TEXTUREHIERARCHY_MAIN", "vs_5_1", compileFlags, 0, &mvsByteCode, nullptr));
	DX::ThrowIfFailed(D3DCompileFromFile(TEXT("Resource/Shader/Shaders.hlsl"), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PS_TEXTUREHIERARCHY_MAIN", "ps_5_1", compileFlags, 0, &mpsByteCode, nullptr));

	m_inputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 24, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "BITANGENT", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 36, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 48, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	psoDesc.InputLayout = { m_inputLayout.data(), (UINT)m_inputLayout.size() };
	psoDesc.pRootSignature = rootSignature.Get();
	psoDesc.VS = CD3DX12_SHADER_BYTECODE(mvsByteCode.Get());
	psoDesc.PS = CD3DX12_SHADER_BYTECODE(mpsByteCode.Get());
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);

	psoDesc.DepthStencilState = CD3DX12_DEPTH_STENCIL_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState.DepthEnable = true;
	psoDesc.DepthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	psoDesc.DepthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_LESS;
	psoDesc.DepthStencilState.StencilEnable = true;
	psoDesc.DepthStencilState.StencilReadMask = 0xff;
	psoDesc.DepthStencilState.StencilWriteMask = 0xff;
	psoDesc.DepthStencilState.FrontFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;
	psoDesc.DepthStencilState.FrontFace.StencilFailOp = D3D12_STENCIL_OP_REPLACE;	
	psoDesc.DepthStencilState.BackFace.StencilPassOp = D3D12_STENCIL_OP_REPLACE;

	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
	psoDesc.BlendState.RenderTarget[0].RenderTargetWriteMask = 0;

	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	DX::ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));
}

void StencilRenderShader::Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) const
{
	commandList->OMSetStencilRef(1);
	Shader::Render(commandList);
	commandList->OMSetStencilRef(0);
}

UIRenderShader::UIRenderShader(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12RootSignature>& rootSignature)
{
	ComPtr<ID3DBlob> mvsByteCode;
	ComPtr<ID3DBlob> mpsByteCode;

#if defined(_DEBUG)
	UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
	UINT compileFlags = 0;
#endif

	DX::ThrowIfFailed(D3DCompileFromFile(TEXT("Resource/Shader/Shaders.hlsl"), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "VS_UI_MAIN", "vs_5_1", compileFlags, 0, &mvsByteCode, nullptr));
	DX::ThrowIfFailed(D3DCompileFromFile(TEXT("Resource/Shader/Shaders.hlsl"), nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "PS_UI_MAIN", "ps_5_1", compileFlags, 0, &mpsByteCode, nullptr));

	m_inputLayout =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	// ���� ���� OFF
	CD3DX12_DEPTH_STENCIL_DESC depthStencilState{ D3D12_DEFAULT };
	depthStencilState.DepthEnable = FALSE;
	depthStencilState.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ZERO;
	depthStencilState.DepthFunc = D3D12_COMPARISON_FUNC_NEVER;

	// ���� ����
	CD3DX12_BLEND_DESC blendState{ D3D12_DEFAULT };
	blendState.AlphaToCoverageEnable = false;
	blendState.RenderTarget[0].BlendEnable = TRUE;
	blendState.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
	blendState.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
	blendState.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc{};
	psoDesc.InputLayout = { m_inputLayout.data(), (UINT)m_inputLayout.size() };
	psoDesc.pRootSignature = rootSignature.Get();
	psoDesc.VS = CD3DX12_SHADER_BYTECODE(mvsByteCode.Get());
	psoDesc.PS = CD3DX12_SHADER_BYTECODE(mpsByteCode.Get());
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.DepthStencilState = depthStencilState;
	psoDesc.BlendState = blendState;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.Flags = D3D12_PIPELINE_STATE_FLAG_NONE;
	DX::ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&m_pipelineState)));
}

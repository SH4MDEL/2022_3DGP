#include "scene.h"


Scene::~Scene()
{
}

void Scene::OnProcessingMouseMessage(HWND hWnd, UINT width, UINT height, FLOAT deltaTime) const
{
	SetCursor(NULL);
	RECT rect; GetWindowRect(hWnd, &rect);
	POINT prevPosition{ rect.left + width / 2, rect.top + height / 2 };

	POINT nextPosition; GetCursorPos(&nextPosition);

	int dx = nextPosition.x - prevPosition.x;
	int dy = nextPosition.y - prevPosition.y;
	m_player->Rotate(dy * 5.0f * deltaTime, dx * 5.0f * deltaTime, 0.0f);
	SetCursorPos(prevPosition.x, prevPosition.y);
}

void Scene::OnProcessingKeyboardMessage(FLOAT timeElapsed) const
{
	if (GetAsyncKeyState('W') & 0x8000)
	{
		m_player->AddVelocity(Vector3::Mul(m_player->GetFront(), timeElapsed * 10.0f));
	}
	if (GetAsyncKeyState('A') & 0x8000)
	{
		m_player->AddVelocity(Vector3::Mul(m_player->GetRight(), timeElapsed * -10.0f));
	}
	if (GetAsyncKeyState('S') & 0x8000)
	{
		m_player->AddVelocity(Vector3::Mul(m_player->GetFront(), timeElapsed * -10.0f));
	}
	if (GetAsyncKeyState('D') & 0x8000)
	{
		m_player->AddVelocity(Vector3::Mul(m_player->GetRight(), timeElapsed * 10.0f));
	}
	if (GetAsyncKeyState(VK_SPACE) & 0x8000)
	{
		m_player->AddVelocity(Vector3::Mul(m_player->GetUp(), timeElapsed * 10.0f));
	}
	if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
	{
		m_player->AddVelocity(Vector3::Mul(m_player->GetUp(), timeElapsed * -10.0f));
	}
}

void Scene::BuildObjects(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandlist, const ComPtr<ID3D12RootSignature>& rootsignature, FLOAT aspectRatio)
{
	unique_ptr<TextureHierarchyShader> playerShader{ make_unique<TextureHierarchyShader>(device, rootsignature) };

	// �÷��̾� ����
	m_player = make_shared<Player>();
	m_player->LoadGeometry(device, commandlist, TEXT("Model/Gunship.bin"));
	m_player->SetRotorFrame();
	m_player->SetPosition(XMFLOAT3(100.0f, 100.0f, 65.0f));
	m_player->SetScale(0.2f, 0.2f, 0.2f);
	playerShader->SetPlayer(m_player);

	// ī�޶� ����
	m_camera = make_shared<ThirdPersonCamera>();
	m_camera->CreateShaderVariable(device, commandlist);
	m_camera->SetEye(XMFLOAT3{ 0.0f, 0.0f, 0.0f });
	m_camera->SetAt(XMFLOAT3{ 0.0f, 0.0f, 1.0f });
	m_camera->SetUp(XMFLOAT3{ 0.0f, 1.0f, 0.0f });
	m_camera->SetPlayer(m_player);
	m_player->SetCamera(m_camera);

	XMFLOAT4X4 projMatrix;
	XMStoreFloat4x4(&projMatrix, XMMatrixPerspectiveFovLH(0.25f * XM_PI, aspectRatio, 0.1f, 1000.0f));
	m_camera->SetProjMatrix(projMatrix);

	// ���� ����
	unique_ptr<TerrainShader> terrainShader{ make_unique<TerrainShader>(device, rootsignature) };
	shared_ptr<HeightMapTerrain> terrain{
		make_shared<HeightMapTerrain>(device, commandlist, TEXT("Resource/HeightMap/HeightMap.raw"), 257, 257, 257, 257, XMFLOAT3{ 1.0f, 0.1f, 1.0f })
	};
	shared_ptr<Texture> terrainTexture{
		make_shared<Texture>()
	};
	terrainTexture->LoadTextureFile(device, commandlist, TEXT("Resource/Texture/Base_Texture.dds"), 2); // BaseTexture
	terrainTexture->LoadTextureFile(device, commandlist, TEXT("Resource/Texture/Detail_Texture.dds"), 3); // DetailTexture
	terrainTexture->CreateSrvDescriptorHeap(device);
	terrainTexture->CreateShaderResourceView(device, D3D12_SRV_DIMENSION_TEXTURE2D);
	terrain->SetPosition(XMFLOAT3{ 0.0f, 0.0f, 0.0f });
	terrain->SetTexture(terrainTexture);
	terrainShader->SetTerrain(terrain);
	m_player->SetTerrain(terrain);

	// ��ī�̹ڽ� ����
	unique_ptr<SkyboxShader> skyboxShader = make_unique<SkyboxShader>(device, rootsignature);
	shared_ptr<Skybox> skybox{ make_shared<Skybox>(device, commandlist, 20.0f, 20.0f, 20.0f) };
	shared_ptr<Texture> skyboxTexture{
		make_shared<Texture>()
	};
	skyboxTexture->LoadTextureFile(device, commandlist, TEXT("Resource/Texture/SkyBox.dds"), 4);
	skyboxTexture->CreateSrvDescriptorHeap(device);
	skyboxTexture->CreateShaderResourceView(device, D3D12_SRV_DIMENSION_TEXTURECUBE);
	skybox->SetTexture(skyboxTexture);
	skyboxShader->GetGameObjects().push_back(skybox);

	// �� ���� ����
	unique_ptr<BlendingShader> blendingShader = make_unique<BlendingShader>(device, rootsignature);
	shared_ptr<GameObject> river = make_shared<GameObject>();
	TextureRectMesh riverMesh{ device, commandlist, XMFLOAT3{0.f, 0.f, 0.f}, 20.f, 0.f, 20.f };
	river->SetMesh(riverMesh);
	shared_ptr<Texture> riverTexture{
		make_shared<Texture>()
	};
	riverTexture->LoadTextureFile(device, commandlist, TEXT("Resource/Texture/Water.dds"), 5);
	riverTexture->CreateSrvDescriptorHeap(device);
	riverTexture->CreateShaderResourceView(device, D3D12_SRV_DIMENSION_TEXTURE2D);
	auto riverMaterial = make_shared<Material>();
	riverMaterial->m_albedoColor.w = 0.51f;
	river->SetScale(13.f, 0.f, 13.f);
	river->SetMaterial(riverMaterial);
	river->SetPosition(XMFLOAT3{ 125.0f, 70.0f, 125.0f });
	river->SetTexture(riverTexture);
	blendingShader->GetGameObjects().push_back(river);

	// ���̴� ����
	m_shader.insert(make_pair("PLYAER", move(playerShader)));
	m_shader.insert(make_pair("TERRAIN", move(terrainShader)));
	m_shader.insert(make_pair("SKYBOX", move(skyboxShader)));
	m_shader.insert(make_pair("BLENDING", move(blendingShader)));
}

void Scene::Update(FLOAT timeElapsed)
{
	m_camera->Update(timeElapsed);
	if (m_shader["SKYBOX"]) for (auto& skybox : m_shader["SKYBOX"]->GetGameObjects()) skybox->SetPosition(m_camera->GetEye());
	for (const auto& shader : m_shader)
		shader.second->Update(timeElapsed);
}

void Scene::Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) const
{
	if (m_camera) m_camera->UpdateShaderVariable(commandList);
	for (const auto& shader : m_shader) {
		if (shader.first != "BLENDING") {
			shader.second->Render(commandList);
		}
	}
	m_shader.at("BLENDING")->Render(commandList);
}

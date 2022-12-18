#include "scene.h"


Scene::Scene(UINT width, UINT height) : m_width(width), m_height(height)
{
}

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

void Scene::OnProcessingMouseMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) const
{
	if (m_player) m_player->LaunchMissile();
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
	if (GetAsyncKeyState('E') & 0x8000)
	{
		g_toggle = true;
	}
	else {
		g_toggle = false;
	}
	if (GetAsyncKeyState('R') & 0x8000)
	{
		g_firstPerson = true;
	}
	else {
		g_firstPerson = false;
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
	auto hierarchyShader{ make_shared<TextureHierarchyShader>(device, rootsignature) };
	auto stencilRenderShader{ make_shared<StencilRenderShader>(device, rootsignature) };
	auto blendHierarchyShader{ make_shared<BlendHierarchyShader>(device, rootsignature) };

	// 플레이어 생성
	m_player = make_shared<Player>();
	m_player->LoadGeometry(device, commandlist, TEXT("Model/SuperCobra.bin"));
	m_player->SetRotorFrame();
	m_player->SetPosition(XMFLOAT3(100.0f, 100.0f, 65.0f));
	m_player->SetScale(0.3f, 0.3f, 0.3f);
	hierarchyShader->SetPlayer(m_player);

	// 카메라 생성
	m_camera = make_shared<ThirdPersonCamera>();
	m_camera->CreateShaderVariable(device, commandlist);
	m_camera->SetEye(XMFLOAT3{ 0.0f, 0.0f, 0.0f });
	m_camera->SetAt(XMFLOAT3{ 0.0f, 0.0f, 1.0f });
	m_camera->SetUp(XMFLOAT3{ 0.0f, 1.0f, 0.0f });
	m_camera->SetPlayer(m_player);
	m_player->SetCamera(m_camera);

	m_firstCamera = make_shared<FirstPersonCamera>();
	m_firstCamera->CreateShaderVariable(device, commandlist);
	m_firstCamera->SetEye(XMFLOAT3{ 0.0f, 0.0f, 0.0f });
	m_firstCamera->SetAt(XMFLOAT3{ 0.0f, 0.0f, 1.0f });
	m_firstCamera->SetUp(XMFLOAT3{ 0.0f, 1.0f, 0.0f });
	m_firstCamera->SetPlayer(m_player);
	m_player->SetFirstCamera(m_firstCamera);

	XMFLOAT4X4 projMatrix;
	XMStoreFloat4x4(&projMatrix, XMMatrixPerspectiveFovLH(0.25f * XM_PI, aspectRatio, 0.1f, 1000.0f));
	m_camera->SetProjMatrix(projMatrix);
	m_firstCamera->SetProjMatrix(projMatrix);

	auto particleShader{ make_shared<ParticleShader>(device, rootsignature) };
	// 적 생성
	auto enemyPoint1 = make_shared<EnemyManager>();
	enemyPoint1->SetPosition(XMFLOAT3{ 50.0f, 100.0f, 170.0f });
	enemyPoint1->SetParticleShader(particleShader);
	enemyPoint1->SetStencilShader(stencilRenderShader);
	enemyPoint1->SetShader(hierarchyShader);
	enemyPoint1->InitEnemy(device, commandlist);
	enemyPoint1->SetTarget(m_player);
	hierarchyShader->GetGameObjects().push_back(enemyPoint1);
	stencilRenderShader->GetGameObjects().push_back(enemyPoint1);
	blendHierarchyShader->GetGameObjects().push_back(enemyPoint1);

	auto enemyPoint2 = make_shared<EnemyManager>();
	enemyPoint2->SetPosition(XMFLOAT3{ 220.0f, 110.0f, 150.0f });
	enemyPoint2->SetParticleShader(particleShader);
	enemyPoint2->SetStencilShader(stencilRenderShader);
	enemyPoint2->SetShader(hierarchyShader);
	enemyPoint2->InitEnemy(device, commandlist);
	enemyPoint2->SetTarget(m_player);
	hierarchyShader->GetGameObjects().push_back(enemyPoint2);
	stencilRenderShader->GetGameObjects().push_back(enemyPoint2);
	blendHierarchyShader->GetGameObjects().push_back(enemyPoint2);

	auto enemyPoint3 = make_shared<EnemyManager>();
	enemyPoint3->SetPosition(XMFLOAT3{ 180.0f, 110.0f, 230.0f });
	enemyPoint3->SetParticleShader(particleShader);
	enemyPoint3->SetStencilShader(stencilRenderShader);
	enemyPoint3->SetShader(hierarchyShader);
	enemyPoint3->InitEnemy(device, commandlist);
	enemyPoint3->SetTarget(m_player);
	hierarchyShader->GetGameObjects().push_back(enemyPoint3);
	stencilRenderShader->GetGameObjects().push_back(enemyPoint3);
	blendHierarchyShader->GetGameObjects().push_back(enemyPoint3);

	// 지형 생성
	auto terrainShader{ make_shared<TerrainShader>(device, rootsignature) };
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
	enemyPoint1->SetTerrain(terrain);
	enemyPoint2->SetTerrain(terrain);
	enemyPoint3->SetTerrain(terrain);

	// 스카이박스 생성
	auto skyboxShader = make_shared<SkyboxShader>(device, rootsignature);
	shared_ptr<Skybox> skybox{ make_shared<Skybox>(device, commandlist, 20.0f, 20.0f, 20.0f) };
	shared_ptr<Texture> skyboxTexture{
		make_shared<Texture>()
	};
	skyboxTexture->LoadTextureFile(device, commandlist, TEXT("Resource/Texture/SkyBox.dds"), 4);
	skyboxTexture->CreateSrvDescriptorHeap(device);
	skyboxTexture->CreateShaderResourceView(device, D3D12_SRV_DIMENSION_TEXTURECUBE);
	skybox->SetTexture(skyboxTexture);
	skyboxShader->GetGameObjects().push_back(skybox);

	// 물 지형 생성
	auto blendingShader = make_shared<BlendingShader>(device, rootsignature);
	shared_ptr<GameObject> river = make_shared<GameObject>();
	shared_ptr<TextureRectMesh> riverMesh = make_shared<TextureRectMesh>(device, commandlist, XMFLOAT3{0.f, 0.f, 0.f}, 20.f, 0.f, 20.f);
	river->SetMesh(riverMesh);
	shared_ptr<Texture> riverTexture{
		make_shared<Texture>()
	};
	riverTexture->LoadTextureFile(device, commandlist, TEXT("Resource/Texture/Water.dds"), 2);
	riverTexture->CreateSrvDescriptorHeap(device);
	riverTexture->CreateShaderResourceView(device, D3D12_SRV_DIMENSION_TEXTURE2D);
	auto riverMaterial = make_shared<Material>();
	riverMaterial->m_albedoColor.w = 0.6f;
	river->SetScale(13.f, 0.f, 13.f);
	river->SetMaterial(riverMaterial);
	river->SetPosition(XMFLOAT3{ 125.0f, 70.0f, 125.0f });
	river->SetTexture(riverTexture);
	blendingShader->GetGameObjects().push_back(river);

	// 빌보드 생성
	auto flower1Mesh = make_shared<BillBoardMesh>(device, commandlist, XMFLOAT3{ 0.f, 0.f, 0.f }, XMFLOAT2{ 1.f, 1.f });
	auto flower2Mesh = make_shared<BillBoardMesh>(device, commandlist, XMFLOAT3{ 0.f, 0.f, 0.f }, XMFLOAT2{ 1.f, 1.f });
	auto grass1Mesh = make_shared<BillBoardMesh>(device, commandlist, XMFLOAT3{ 0.f, 0.f, 0.f }, XMFLOAT2{ 1.f, 1.f });
	auto grass2Mesh = make_shared<BillBoardMesh>(device, commandlist, XMFLOAT3{ 0.f, 0.f, 0.f }, XMFLOAT2{ 1.f, 1.f });
	auto flower1Texture { make_shared<Texture>() };
	flower1Texture->LoadTextureFile(device, commandlist, TEXT("Resource/Texture/Flower01.dds"), 2);
	flower1Texture->CreateSrvDescriptorHeap(device);
	flower1Texture->CreateShaderResourceView(device, D3D12_SRV_DIMENSION_TEXTURE2D);
	auto flower2Texture{ make_shared<Texture>() };
	flower2Texture->LoadTextureFile(device, commandlist, TEXT("Resource/Texture/Flower02.dds"), 2);
	flower2Texture->CreateSrvDescriptorHeap(device);
	flower2Texture->CreateShaderResourceView(device, D3D12_SRV_DIMENSION_TEXTURE2D);
	auto grass1Texture{ make_shared<Texture>() };
	grass1Texture->LoadTextureFile(device, commandlist, TEXT("Resource/Texture/Grass01.dds"), 2);
	grass1Texture->CreateSrvDescriptorHeap(device);
	grass1Texture->CreateShaderResourceView(device, D3D12_SRV_DIMENSION_TEXTURE2D);
	auto grass2Texture{ make_shared<Texture>() };
	grass2Texture->LoadTextureFile(device, commandlist, TEXT("Resource/Texture/Grass02.dds"), 2);
	grass2Texture->CreateSrvDescriptorHeap(device);
	grass2Texture->CreateShaderResourceView(device, D3D12_SRV_DIMENSION_TEXTURE2D);
	vector<XMFLOAT3> flower1Pos;
	vector<XMFLOAT3> flower2Pos;
	vector<XMFLOAT3> grass1Pos;
	vector<XMFLOAT3> grass2Pos;
	XMFLOAT3 terrainPosition = terrain->GetPosition();
	XMFLOAT3 terrainScale = terrain->GetScale();
	FLOAT terrainHeight = terrain->GetHeight(terrainPosition.x, terrainPosition.z);
	for (int z = 2; z <= 64; ++z) {
		FLOAT nz = ((terrainPosition.z + (terrain->GetLength() / 64 * z)));
		for (int x = 2; x <= 64; ++x) {
			FLOAT nx = ((terrainPosition.x + (terrain->GetWidth() / 64 * x)));
			FLOAT ny = terrain->GetHeight(nx, nz) / terrainScale.y + 0.4f;
			if (ny > 70.f) {
				int value = GetRandomInt(1, 4);
				if (value == 1) {
					flower1Pos.emplace_back(XMFLOAT3{ nx, ny, nz });
				}
				if (value == 2) {
					flower2Pos.emplace_back(XMFLOAT3{ nx, ny, nz });
				}
				if (value == 3) {
					grass1Pos.emplace_back(XMFLOAT3{ nx, ny, nz });
				}
				if (value == 4) {
					grass2Pos.emplace_back(XMFLOAT3{ nx, ny, nz });
				}
			}
		}
	}
	auto flower1Shader = make_shared<BillBoardShader>(device, rootsignature, (UINT)flower1Pos.size());
	auto flower2Shader = make_shared<BillBoardShader>(device, rootsignature, (UINT)flower2Pos.size());
	auto grass1Shader = make_shared<BillBoardShader>(device, rootsignature, (UINT)grass1Pos.size());
	auto grass2Shader = make_shared<BillBoardShader>(device, rootsignature, (UINT)grass2Pos.size());
	flower1Shader->SetInstancePositions(move(flower1Pos));
	flower2Shader->SetInstancePositions(move(flower2Pos));
	grass1Shader->SetInstancePositions(move(grass1Pos));
	grass2Shader->SetInstancePositions(move(grass2Pos));
	flower1Shader->SetMesh(flower1Mesh);
	flower2Shader->SetMesh(flower2Mesh);
	grass1Shader->SetMesh(grass1Mesh);
	grass2Shader->SetMesh(grass2Mesh);
	flower1Shader->SetTexture(flower1Texture);
	flower2Shader->SetTexture(flower2Texture);
	grass1Shader->SetTexture(grass1Texture);
	grass2Shader->SetTexture(grass2Texture);

	//스텐실 버퍼 생성
	auto outlineShader{ make_shared<UIRenderShader>(device, rootsignature) };
	auto uiMesh{ make_shared<UIMesh>(device, commandlist) };
	auto stencilTexture{ make_shared<Texture>() };
	stencilTexture->CreateTexture(device, DXGI_FORMAT_R24G8_TYPELESS, m_width, m_height, 5);
	stencilTexture->CreateSrvDescriptorHeap(device);
	stencilTexture->CreateShaderResourceView(device, D3D12_SRV_DIMENSION_BUFFER);
	auto ui{ make_shared<UIObject>() };
	ui->SetMesh(uiMesh);
	ui->SetTexture(stencilTexture);
	outlineShader->GetGameObjects().push_back(ui);

	auto windowShader{ make_shared<WindowShader>(device, rootsignature) };
	auto windowMesh{ make_shared<UIMesh>(device, commandlist) };
	auto window{ make_shared<UIObject>() };
	window->SetMesh(windowMesh);
	windowShader->GetGameObjects().push_back(ui);

	// 셰이더 설정
	m_shader.insert(make_pair("TERRAIN", move(terrainShader)));
	m_shader.insert(make_pair("SKYBOX", move(skyboxShader)));
	m_shader.insert(make_pair("FLOWER1", move(flower1Shader)));
	m_shader.insert(make_pair("FLOWER2", move(flower2Shader)));
	m_shader.insert(make_pair("GRASS1", move(grass1Shader)));
	m_shader.insert(make_pair("GRASS2", move(grass2Shader)));
	m_shader.insert(make_pair("BLENDING", move(blendingShader)));
	m_shader.insert(make_pair("HIERARCHY", move(hierarchyShader)));
	m_shader.insert(make_pair("STENCIL", move(stencilRenderShader)));
	m_shader.insert(make_pair("OUTLINE", move(outlineShader)));
	m_shader.insert(make_pair("WINDOW", move(windowShader)));
	m_shader.insert(make_pair("BLENDHIERARCHY", move(blendHierarchyShader)));
}

void Scene::Update(FLOAT timeElapsed)
{
	m_camera->Update(timeElapsed);
	m_firstCamera->Update(timeElapsed);
	if (m_shader["SKYBOX"]) for (auto& skybox : m_shader["SKYBOX"]->GetGameObjects()) skybox->SetPosition(m_camera->GetEye());
	for (const auto& shader : m_shader) shader.second->Update(timeElapsed);

	CheckPlayerByObjectCollisions();
	CheckMissileByObjectCollisions();
	CheckTerrainBorderLimit();
}

void Scene::Render(const ComPtr<ID3D12GraphicsCommandList>& commandList, CD3DX12_CPU_DESCRIPTOR_HANDLE dsvHandle) const
{
	if (g_firstPerson) {
		if (m_firstCamera) m_firstCamera->UpdateShaderVariable(commandList);
	}
	else {
		if (m_camera) m_camera->UpdateShaderVariable(commandList);
	}
	m_shader.at("TERRAIN")->Render(commandList);
	m_shader.at("SKYBOX")->Render(commandList);
	m_shader.at("BLENDING")->Render(commandList);
	m_shader.at("FLOWER1")->Render(commandList);
	m_shader.at("FLOWER2")->Render(commandList);
	m_shader.at("GRASS1")->Render(commandList);
	m_shader.at("GRASS2")->Render(commandList);

	//commandList->ClearDepthStencilView(dsvHandle, D3D12_CLEAR_FLAG_STENCIL, 1.0f, 0, 0, NULL);
	if (g_toggle) m_shader.at("STENCIL")->Render(commandList);
	m_shader.at("OUTLINE")->Render(commandList);
	g_postProcess = true;
	if (g_toggle) m_shader.at("BLENDHIERARCHY")->Render(commandList);
	g_postProcess = false;

	if (g_firstPerson) m_shader.at("WINDOW")->Render(commandList);

	m_shader.at("HIERARCHY")->Render(commandList);
}

void Scene::CheckPlayerByObjectCollisions()
{
	for (auto enemyManager : m_shader["HIERARCHY"]->GetGameObjects()) {
		for (auto enemy : static_pointer_cast<EnemyManager>(enemyManager)->GetEnemys()) {
			if (enemy->GetStatus() == Enemy::LIVE && enemy->GetBoundingBox().Intersects(m_player->GetBoundingBox())) {
				static_pointer_cast<Enemy>(enemy)->SetStatus(Enemy::BLOWING);
			}
		}
	}
}

void Scene::CheckMissileByObjectCollisions()
{
	if (m_player->GetMissileState() == Player::SHOTTING) {
		for (auto enemyManager : m_shader["HIERARCHY"]->GetGameObjects()) {
			for (auto enemy : static_pointer_cast<EnemyManager>(enemyManager)->GetEnemys()) {
				if (enemy->GetStatus() == Enemy::LIVE && enemy->GetBoundingBox().Intersects(m_player->GetMissile()->GetBoundingBox())) {
					static_pointer_cast<Enemy>(enemy)->SetStatus(Enemy::BLOWING);
					m_player->SetMissileState(Player::READY);
				}
			}
		}
	}
}

void Scene::CheckTerrainBorderLimit()
{
	XMFLOAT3 pos = m_player->GetPosition();
	if (pos.x >= 255.f) m_player->SetPosition(XMFLOAT3{ 255.f, pos.y, pos.z });
	if (pos.x <= 0.f) m_player->SetPosition(XMFLOAT3{ 0.f, pos.y, pos.z });
	if (pos.z >= 255.f) m_player->SetPosition(XMFLOAT3{ pos.x, pos.y, 255.f });
	if (pos.z <= 0.f) m_player->SetPosition(XMFLOAT3{ pos.x, pos.y, 0.f });
	for (auto enemyManager : m_shader["HIERARCHY"]->GetGameObjects()) {
		for (auto enemy : static_pointer_cast<EnemyManager>(enemyManager)->GetEnemys()) {
			XMFLOAT3 pos = enemy->GetPosition();
			if (enemy->GetStatus() == Enemy::LIVE && 
				(pos.x >= 255.f || pos.x <= 0.f || pos.z >= 255.f || pos.z <= 0.f)) {
				static_pointer_cast<Enemy>(enemy)->SetStatus(Enemy::BLOWING);
			}
		}
	}
}



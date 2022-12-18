#include "object.h"
#include "shader.h"
#include "framework.h"

GameObject::GameObject() : m_right{ 1.0f, 0.0f, 0.0f }, m_up{ 0.0f, 1.0f, 0.0f }, m_front{ 0.0f, 0.0f, 1.0f }, m_roll{ 0.0f }, m_pitch{ 0.0f }, m_yaw{ 0.0f }
{
	XMStoreFloat4x4(&m_worldMatrix, XMMatrixIdentity());
	XMStoreFloat4x4(&m_transformMatrix, XMMatrixIdentity());
}

GameObject::~GameObject()
{
	if (m_mesh) m_mesh->ReleaseUploadBuffer();
}

void GameObject::Update(FLOAT timeElapsed)
{
	if (m_sibling) m_sibling->Update(timeElapsed);
	if (m_child) m_child->Update(timeElapsed);

	UpdateBoundingBox();
}

void GameObject::Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) const
{
	XMFLOAT4X4 worldMatrix;
	XMStoreFloat4x4(&worldMatrix, XMMatrixTranspose(XMLoadFloat4x4(&m_worldMatrix)));
	commandList->SetGraphicsRoot32BitConstants(0, 16, &worldMatrix, 0);

	if (m_texture) { m_texture->UpdateShaderVariable(commandList); }
	if (m_material) { m_material->UpdateShaderVariable(commandList); }
	if (m_frameName != "GunnerDoor" && m_frameName != "Cockpit") {
		if (m_mesh) m_mesh->Render(commandList);
	}

	if (m_sibling) m_sibling->Render(commandList);
	if (m_child) m_child->Render(commandList);
}

void GameObject::Move(const XMFLOAT3& shift)
{
	SetPosition(Vector3::Add(GetPosition(), shift));
}

void GameObject::Rotate(FLOAT roll, FLOAT pitch, FLOAT yaw)
{
	// 회전
	XMMATRIX rotate{ XMMatrixRotationRollPitchYaw(XMConvertToRadians(roll), XMConvertToRadians(pitch), XMConvertToRadians(yaw)) };
	XMMATRIX transformMatrix{ rotate * XMLoadFloat4x4(&m_transformMatrix) };
	XMStoreFloat4x4(&m_transformMatrix, transformMatrix);

	// 로컬 x,y,z축 최신화
	XMStoreFloat3(&m_right, XMVector3TransformNormal(XMLoadFloat3(&m_right), rotate));
	XMStoreFloat3(&m_up, XMVector3TransformNormal(XMLoadFloat3(&m_up), rotate));
	XMStoreFloat3(&m_front, XMVector3TransformNormal(XMLoadFloat3(&m_front), rotate));

	UpdateTransform(nullptr);
}

void GameObject::SetMesh(const shared_ptr<Mesh>& mesh)
{
	if (m_mesh) m_mesh.reset();
	m_mesh = mesh;
}

void GameObject::SetTexture(const shared_ptr<Texture>& texture)
{
	if (m_texture) m_texture.reset();
	m_texture = texture;
}

void GameObject::SetMaterial(const shared_ptr<Material>& material)
{
	if (m_material) m_material.reset();
	m_material = material;
}

XMFLOAT3 GameObject::GetPosition() const
{
	return XMFLOAT3{ m_worldMatrix._41, m_worldMatrix._42, m_worldMatrix._43 };
}

void GameObject::SetWorldMatrix(const XMFLOAT4X4& worldMatrix)
{
	m_worldMatrix = worldMatrix;
}

void GameObject::SetPosition(const XMFLOAT3& position)
{
	m_transformMatrix._41 = position.x;
	m_transformMatrix._42 = position.y;
	m_transformMatrix._43 = position.z;

	UpdateTransform(nullptr);
}

void GameObject::SetScale(FLOAT x, FLOAT y, FLOAT z)
{
	XMMATRIX scale = XMMatrixScaling(x, y, z);
	m_transformMatrix = Matrix::Mul(scale, m_transformMatrix);

	UpdateTransform(nullptr);
}

void GameObject::UpdateTransform(XMFLOAT4X4* parentMatrix)
{
	m_worldMatrix = (parentMatrix) ? Matrix::Mul(m_transformMatrix, *parentMatrix) : m_transformMatrix;

	if (m_sibling) m_sibling->UpdateTransform(parentMatrix);
	if (m_child) m_child->UpdateTransform(&m_worldMatrix);
}

void GameObject::ReleaseUploadBuffer() const
{
	if (m_mesh) m_mesh->ReleaseUploadBuffer();
	if (m_texture) m_texture->ReleaseUploadBuffer();

	if (m_sibling) m_sibling->ReleaseUploadBuffer();
	if (m_child) m_child->ReleaseUploadBuffer();
}

void GameObject::SetChild(const shared_ptr<GameObject>& child)
{
	if (child) {
		// https://welikecse.tistory.com/13
		child->m_parent = shared_from_this();
	}
	if (m_child) {
		if (child) child->m_sibling = m_child->m_sibling;
		m_child->m_sibling = child;
	}
	else {
		m_child = child;
	}
}

shared_ptr<GameObject> GameObject::FindFrame(string frameName)
{
	shared_ptr<GameObject> frame;
	if (m_frameName == frameName) return shared_from_this();

	if (m_sibling) if (frame = m_sibling->FindFrame(frameName)) return frame;
	if (m_child) if (frame = m_child->FindFrame(frameName)) return frame;

	return nullptr;
}

void GameObject::LoadGeometry(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList, const wstring& fileName)
{
	ifstream in{ fileName, std::ios::binary };
	if (!in) return;

	LoadFrameHierarchy(device, commandList, in);
}

void GameObject::LoadFrameHierarchy(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList, ifstream& in)
{
	BYTE strLength;
	INT frame, texture;

	unique_ptr<MeshFromFile> mesh = make_unique<MeshFromFile>();
	while (1) {
		in.read((char*)(&strLength), sizeof(BYTE));
		string strToken(strLength, '\0');
		in.read((&strToken[0]), sizeof(char) * strLength);

		if (strToken == "<Frame>:") {
			in.read((char*)(&frame), sizeof(INT));
			in.read((char*)(&texture), sizeof(INT));

			in.read((char*)(&strLength), sizeof(BYTE));
			m_frameName.resize(strLength);
			in.read((&m_frameName[0]), sizeof(char) * strLength);
		}
		else if (strToken == "<Transform>:") {
			XMFLOAT3 position, rotation, scale;
			XMFLOAT4 qrotation;

			in.read((char*)(&position), sizeof(FLOAT) * 3);
			in.read((char*)(&rotation), sizeof(FLOAT) * 3);
			in.read((char*)(&scale), sizeof(FLOAT) * 3);
			in.read((char*)(&qrotation), sizeof(FLOAT) * 4);
		}
		else if (strToken == "<TransformMatrix>:") {
			in.read((char*)(&m_transformMatrix), sizeof(FLOAT) * 16);
		}
		else if (strToken == "<Mesh>:") {
			mesh->LoadMesh(device, commandList, in);
			SetBoundingBox(mesh->GetBoundingBox());
		}
		else if (strToken == "<Materials>:") {
			mesh->LoadMaterial(device, commandList, in);
		}
		else if (strToken == "<Children>:") {
			INT childNum = 0;
			in.read((char*)(&childNum), sizeof(INT));
			if (childNum) {
				for (int i = 0; i < childNum; ++i) {
					shared_ptr<GameObject> child = make_shared<GameObject>();
					child->LoadFrameHierarchy(device, commandList, in);
					SetChild(child);
				}
			}
		}
		else if (strToken == "</Frame>") {
			break;
		}
	}
	m_mesh = move(mesh);
}

void GameObject::UpdateBoundingBox()
{
	m_boundingBox.Center = GetPosition();
}

void GameObject::SetBoundingBox(const BoundingOrientedBox& boundingBox)
{
	m_boundingBox = boundingBox;
}

Helicoptor::Helicoptor() : GameObject() {}

void Helicoptor::Update(FLOAT timeElapsed)
{
	if (m_mainRotorFrame) m_mainRotorFrame->Rotate(0.f, 1080.f * timeElapsed, 0.f);
	if (m_tailRotorFrame) m_tailRotorFrame->Rotate(720.f * timeElapsed, 0.f, 0.f);

	GameObject::Update(timeElapsed);
}

void Helicoptor::SetRotorFrame()
{
	m_mainRotorFrame = FindFrame("MainRotor");
	m_tailRotorFrame = FindFrame("TailRotor");
	m_missileFrame = FindFrame("Radar");
}

Enemy::Enemy() : m_status(DEATH), m_blowingAge(0.f)
{
}

void Enemy::InitParticle(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList)
{
	if (m_particle) m_particle.reset();
	m_particle = make_unique<ParticleMesh>(device, commandList);
}

void Enemy::ReleaseUploadBuffer() const
{
	m_particle->ReleaseUploadBuffer();
	GameObject::ReleaseUploadBuffer();
}

void Enemy::Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) const
{
	if (!g_postProcess) {
		if (g_toggle) {
			if (m_stencilShader) commandList->SetPipelineState(m_stencilShader->GetPipelineState().Get());
		}
		else {
			if (m_shader) commandList->SetPipelineState(m_shader->GetPipelineState().Get());
		}
	}

	if (m_status == LIVE) { GameObject::Render(commandList); }

	if (!g_postProcess && m_status == BLOWING) {
		XMFLOAT4X4 worldMatrix;
		XMStoreFloat4x4(&worldMatrix, XMMatrixTranspose(XMLoadFloat4x4(&m_worldMatrix)));
		commandList->SetGraphicsRoot32BitConstants(0, 16, &worldMatrix, 0);
		commandList->SetGraphicsRoot32BitConstants(0, 1, &(m_blowingAge), 33);
		
		if (m_particleShader) {
			commandList->SetPipelineState(m_particleShader->GetStreamPipelineState().Get());
			if (m_particle) m_particle->RenderStreamOutput(commandList);

			commandList->SetPipelineState(m_particleShader->GetPipelineState().Get());
			if (m_particle) m_particle->Render(commandList);
		}
	}
}

void Enemy::Update(FLOAT timeElapsed)
{
	m_timeElapsed = timeElapsed;

	Helicoptor::Update(timeElapsed);
	if (m_status == LIVE) {
		XMFLOAT3 movePosition = Vector3::Sub(m_targetPosition, GetPosition());
		movePosition = Vector3::Normalize(movePosition);
		XMFLOAT3 look = GetLook();

		XMFLOAT3 crossProduct = Vector3::Cross(look, movePosition);
		if (look.z < 0) { crossProduct.x *= -1.0f; }

		Rotate(crossProduct.x * 10.f, crossProduct.y * 10.f, 0.0f);

		Move(Vector3::Mul(look, 10.0f * timeElapsed));

		// 바닥 못 뚫게 보정
		XMFLOAT3 pos = GetPosition();
		if ((m_terrainHeight + 0.05f) / 0.1f > pos.y) {
			SetPosition(XMFLOAT3{ pos.x, (m_terrainHeight + 0.05f) / 0.1f, pos.z });
		}
	}
	if (m_status == BLOWING) {
		m_blowingAge += timeElapsed;
		if (m_blowingAge >= m_blowingLifeTime) {
			m_status = DEATH;
			m_blowingAge = 0.f;
		}
	}
}

void Enemy::SetRotorFrame()
{
	m_mainRotorFrame = FindFrame("Top_Rotor");
	m_tailRotorFrame = FindFrame("Tail_Rotor");
}

EnemyManager::EnemyManager() : m_regenTimer(0.f)
{
}

void EnemyManager::InitEnemy(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandlist)
{
	for (int i = 0; i < 5; ++i) {
		m_enemys.push_back(make_shared<Enemy>());
		m_enemys.back()->LoadGeometry(device, commandlist, TEXT("Model/Mi24.bin"));
		m_enemys.back()->InitParticle(device, commandlist);
		m_enemys.back()->SetRotorFrame();
		m_enemys.back()->SetPosition(GetPosition());
		m_enemys.back()->GetMesh()->SetBoundingBox(GetPosition(), XMFLOAT3(1.f, 1.f, 1.f));
		m_enemys.back()->SetBoundingBox(m_enemys.back()->GetMesh()->GetBoundingBox());
		m_enemys.back()->SetScale(0.2f, 0.2f, 0.2f);
		m_enemys.back()->SetParticleShader(m_particleShader);
		m_enemys.back()->SetStencilShader(m_stencilShader);
		m_enemys.back()->SetShader(m_shader);
	}
}

void EnemyManager::Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) const
{
	for (const auto enemy : m_enemys) enemy->Render(commandList);
}

void EnemyManager::Update(FLOAT timeElapsed)
{
	m_regenTimer += timeElapsed;
	if (m_regenTimer >= m_regenTime) {
		for (const auto enemy : m_enemys) {
			if (enemy->GetStatus() == Enemy::DEATH) {
				enemy->SetStatus(Enemy::LIVE);
				enemy->SetPosition(GetPosition());
				m_regenTimer = 0.f;
				break;
			}
		}
	}
	XMFLOAT3 targetPosition = m_target->GetPosition();
	for (const auto enemy : m_enemys) {
		XMFLOAT3 position = enemy->GetPosition();
		enemy->SetTargetPosition(targetPosition);
		enemy->SetTerrainHeight(m_terrain->GetHeight(position.x, position.z));
		enemy->Update(timeElapsed);
	}
}

void EnemyManager::ReleaseUploadBuffer() const
{
	for (const auto enemy : m_enemys) {
		enemy->ReleaseUploadBuffer();
	}
	GameObject::ReleaseUploadBuffer();
}

HeightMapTerrain::HeightMapTerrain(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList,
	const wstring& fileName, INT width, INT length, INT blockWidth, INT blockLength, XMFLOAT3 scale)
	: m_width{ width }, m_length{ length }, m_scale{ scale }
{
	// 높이맵이미지 로딩
	m_heightMapImage = make_unique<HeightMapImage>(fileName, m_width, m_length, m_scale);

	// 가로, 세로 블록의 개수
	int widthBlockCount{ m_width / blockWidth };
	int lengthBlockCount{ m_length / blockLength };

	// 블록 생성
	for (int z = 0; z < lengthBlockCount; ++z)
		for (int x = 0; x < widthBlockCount; ++x)
		{
			int xStart{ x * (blockWidth - 1) };
			int zStart{ z * (blockLength - 1) };
			unique_ptr<GameObject> block{ make_unique<GameObject>() };
			block->SetMesh(make_shared<HeightMapGridMesh>(device, commandList, xStart, zStart, blockWidth, blockLength, m_scale,m_heightMapImage.get()));
			m_blocks.push_back(move(block));
		}
}

void HeightMapTerrain::Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) const
{
	if (m_texture) m_texture->UpdateShaderVariable(commandList);
	for (const auto& block : m_blocks)
		block->Render(commandList);
}

void HeightMapTerrain::Move(const XMFLOAT3& shift)
{
	for (auto& block : m_blocks)
		block->Move(shift);
}

void HeightMapTerrain::Rotate(FLOAT roll, FLOAT pitch, FLOAT yaw)
{
	for (auto& block : m_blocks)
		block->Rotate(roll, pitch, yaw);
}

void HeightMapTerrain::SetPosition(const XMFLOAT3& position)
{
	// 지형의 위치 설정은 모든 블록들의 위치를 조정한다는 것임
	for (auto& block : m_blocks)
		block->SetPosition(position);
}

FLOAT HeightMapTerrain::GetHeight(FLOAT x, FLOAT z) const
{
	// 파라미터로 들어온 (x, z)는 플레이어의 위치이다.
	// (x, z)를 대응하는 이미지 좌표로 바꿔줘야한다.

	// 지형의 시작점 반영
	XMFLOAT3 pos{ GetPosition() };
	x -= pos.x;
	z -= pos.z;

	// 지형의 스케일 반영
	x /= m_scale.x;
	z /= m_scale.z;

	return pos.y + m_heightMapImage->GetHeight(x, z) * m_scale.y;
}

XMFLOAT3 HeightMapTerrain::GetNormal(FLOAT x, FLOAT z) const
{
	// 파라미터로 들어온 (x, z)는 플레이어의 위치이다.

	XMFLOAT3 pos{ GetPosition() };
	x -= pos.x; x /= m_scale.x;
	z -= pos.z; z /= m_scale.z;

	// (x, z) 주변의 노멀 4개를 보간해서 계산
	int ix{ static_cast<int>(x) };
	int iz{ static_cast<int>(z) };
	float fx{ x - ix };
	float fz{ z - iz };

	XMFLOAT3 LT{ m_heightMapImage->GetNormal(ix, iz + 1) };
	XMFLOAT3 LB{ m_heightMapImage->GetNormal(ix, iz) };
	XMFLOAT3 RT{ m_heightMapImage->GetNormal(ix + 1, iz + 1) };
	XMFLOAT3 RB{ m_heightMapImage->GetNormal(ix + 1, iz) };

	XMFLOAT3 bot{ Vector3::Add(Vector3::Mul(LB, 1.0f - fx), Vector3::Mul(RB, fx)) };
	XMFLOAT3 top{ Vector3::Add(Vector3::Mul(LT, 1.0f - fx), Vector3::Mul(RT, fx)) };
	return Vector3::Normalize(Vector3::Add(Vector3::Mul(bot, 1.0f - fz), Vector3::Mul(top, fz)));
}

void HeightMapTerrain::ReleaseUploadBuffer() const
{
	if (m_texture) m_texture->ReleaseUploadBuffer();
	for (auto& block : m_blocks) {
		block->ReleaseUploadBuffer();
	}
}


Skybox::Skybox(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList,
	FLOAT width, FLOAT length, FLOAT depth) : GameObject()
{
	m_mesh = make_unique<SkyboxMesh>(device, commandList, width, length, depth);
}

void Skybox::Update(FLOAT timeElapsed)
{
}

UIObject::UIObject() : GameObject()
{
}

void UIObject::Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) const
{
	if (m_texture) {
		m_texture->Copy(commandList, g_GameFramework.GetDepthStencil(), D3D12_RESOURCE_STATE_DEPTH_WRITE);
		m_texture->UpdateShaderVariable(commandList);
	}
	m_mesh->Render(commandList);
}

#include "object.h"

GameObject::GameObject() : m_right{ 1.0f, 0.0f, 0.0f }, m_up{ 0.0f, 1.0f, 0.0f }, m_front{ 0.0f, 0.0f, 1.0f }, m_roll{ 0.0f }, m_pitch{ 0.0f }, m_yaw{ 0.0f }
{
	XMStoreFloat4x4(&m_worldMatrix, XMMatrixIdentity());
}

GameObject::~GameObject()
{
	if (m_mesh) m_mesh->ReleaseUploadBuffer();
}

void GameObject::Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) const
{
	XMFLOAT4X4 worldMatrix;
	XMStoreFloat4x4(&worldMatrix, XMMatrixTranspose(XMLoadFloat4x4(&m_worldMatrix)));
	commandList->SetGraphicsRoot32BitConstants(0, 16, &worldMatrix, 0);

	if (m_texture) { m_texture->UpdateShaderVariable(commandList); }

	if (m_mesh) m_mesh->Render(commandList);
}

void GameObject::Move(const XMFLOAT3& shift)
{
	SetPosition(Vector3::Add(GetPosition(), shift));
}

void GameObject::Rotate(FLOAT roll, FLOAT pitch, FLOAT yaw)
{
	// ȸ��
	XMMATRIX rotate{ XMMatrixRotationRollPitchYaw(XMConvertToRadians(roll), XMConvertToRadians(pitch), XMConvertToRadians(yaw)) };
	XMMATRIX worldMatrix{ rotate * XMLoadFloat4x4(&m_worldMatrix) };
	XMStoreFloat4x4(&m_worldMatrix, worldMatrix);

	// ���� x,y,z�� �ֽ�ȭ
	XMStoreFloat3(&m_right, XMVector3TransformNormal(XMLoadFloat3(&m_right), rotate));
	XMStoreFloat3(&m_up, XMVector3TransformNormal(XMLoadFloat3(&m_up), rotate));
	XMStoreFloat3(&m_front, XMVector3TransformNormal(XMLoadFloat3(&m_front), rotate));
}

void GameObject::SetMesh(const Mesh& mesh)
{
	if (m_mesh) m_mesh.reset();
	m_mesh = make_unique<Mesh>(mesh);
}

void GameObject::SetTexture(const shared_ptr<Texture>& texture)
{
	if (m_texture) m_texture.reset();
	m_texture = texture;
}

XMFLOAT3 GameObject::GetPosition() const
{
	return XMFLOAT3{ m_worldMatrix._41, m_worldMatrix._42, m_worldMatrix._43 };
}

void GameObject::SetPosition(const XMFLOAT3& position)
{
	m_worldMatrix._41 = position.x;
	m_worldMatrix._42 = position.y;
	m_worldMatrix._43 = position.z;
}

void GameObject::ReleaseUploadBuffer() const
{
	if (m_mesh) m_mesh->ReleaseUploadBuffer();
	if (m_texture) m_texture->ReleaseUploadBuffer();

}

RotatingObject::RotatingObject() : m_rotationSpeed(100.0f)
{

}

void RotatingObject::Update(FLOAT timeElapsed)
{
	GameObject::Rotate(0.0f, m_rotationSpeed * timeElapsed, 0.0f);
}

void RotatingObject::SetRotationSpeed(FLOAT rotationSpeed)
{
	m_rotationSpeed = rotationSpeed;
}

HeightMapTerrain::HeightMapTerrain(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList,
	const wstring& fileName, INT width, INT length, INT blockWidth, INT blockLength, XMFLOAT3 scale)
	: m_width{ width }, m_length{ length }, m_scale{ scale }
{
	// ���̸��̹��� �ε�
	m_heightMapImage = make_unique<HeightMapImage>(fileName, m_width, m_length, m_scale);

	// ����, ���� ����� ����
	int widthBlockCount{ m_width / blockWidth };
	int lengthBlockCount{ m_length / blockLength };

	// ��� ����
	for (int z = 0; z < lengthBlockCount; ++z)
		for (int x = 0; x < widthBlockCount; ++x)
		{
			int xStart{ x * (blockWidth - 1) };
			int zStart{ z * (blockLength - 1) };
			unique_ptr<GameObject> block{ make_unique<GameObject>() };
			block->SetMesh(HeightMapGridMesh(device, commandList, xStart, zStart, blockWidth, blockLength, m_scale,m_heightMapImage.get()));
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
	// ������ ��ġ ������ ��� ��ϵ��� ��ġ�� �����Ѵٴ� ����
	for (auto& block : m_blocks)
		block->SetPosition(position);
}

FLOAT HeightMapTerrain::GetHeight(FLOAT x, FLOAT z) const
{
	// �Ķ���ͷ� ���� (x, z)�� �÷��̾��� ��ġ�̴�.
	// (x, z)�� �����ϴ� �̹��� ��ǥ�� �ٲ�����Ѵ�.

	// ������ ������ �ݿ�
	XMFLOAT3 pos{ GetPosition() };
	x -= pos.x;
	z -= pos.z;

	// ������ ������ �ݿ�
	x /= m_scale.x;
	z /= m_scale.z;

	return pos.y + m_heightMapImage->GetHeight(x, z) * m_scale.y;
}

XMFLOAT3 HeightMapTerrain::GetNormal(FLOAT x, FLOAT z) const
{
	// �Ķ���ͷ� ���� (x, z)�� �÷��̾��� ��ġ�̴�.

	XMFLOAT3 pos{ GetPosition() };
	x -= pos.x; x /= m_scale.x;
	z -= pos.z; z /= m_scale.z;

	// (x, z) �ֺ��� ��� 4���� �����ؼ� ���
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

Skybox::Skybox(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList)
{
}

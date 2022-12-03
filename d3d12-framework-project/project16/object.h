#pragma once
#include "stdafx.h"
#include "mesh.h"
#include "texture.h"
#include "material.h"

class GameObject : public enable_shared_from_this<GameObject>
{
public:
	GameObject();
	~GameObject();

	virtual void Update(FLOAT timeElapsed);
	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) const;
	virtual void Move(const XMFLOAT3& shift);
	virtual void Rotate(FLOAT roll, FLOAT pitch, FLOAT yaw);
	virtual void UpdateTransform(XMFLOAT4X4* parentMatrix = nullptr);

	void SetMesh(const shared_ptr<Mesh>& mesh);
	void SetTexture(const shared_ptr<Texture>& texture);
	void SetMaterial(const shared_ptr<Material>& material);

	void SetWorldMatrix(const XMFLOAT4X4& worldMatrix);
	void SetPosition(const XMFLOAT3& position);
	void SetScale(FLOAT x, FLOAT y, FLOAT z);

	XMFLOAT4X4 GetWorldMatrix() const { return m_worldMatrix; }
	XMFLOAT3 GetPosition() const;
	XMFLOAT3 GetRight() const { return m_right; }
	XMFLOAT3 GetUp() const { return m_up; }
	XMFLOAT3 GetFront() const { return m_front; }
	XMFLOAT3 GetLook() const { return Vector3::Normalize(XMFLOAT3(m_worldMatrix._31, m_worldMatrix._32, m_worldMatrix._33)); }

	virtual void ReleaseUploadBuffer() const;

	void SetChild(const shared_ptr<GameObject>& child);
	shared_ptr<GameObject> FindFrame(string frameName);

	void LoadGeometry(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList, const wstring& fileName);
	void LoadFrameHierarchy(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList, ifstream& in);

	void UpdateBoundingBox();
	void SetBoundingBox(const BoundingOrientedBox& boundingBox);
	BoundingOrientedBox& GetBoundingBox() { return m_boundingBox; }

protected:
	XMFLOAT4X4					m_transformMatrix;
	XMFLOAT4X4					m_worldMatrix;

	XMFLOAT3					m_right;		// ���� x��
	XMFLOAT3					m_up;			// ���� y��
	XMFLOAT3					m_front;		// ���� z��

	FLOAT						m_roll;			// x�� ȸ����
	FLOAT						m_pitch;		// y�� ȸ����
	FLOAT						m_yaw;			// z�� ȸ����

	shared_ptr<Mesh>			m_mesh;			// �޽�
	shared_ptr<Texture>			m_texture;		// �ؽ�ó
	shared_ptr<Material>		m_material;		// ����

	string						m_frameName;	// ���� �������� �̸�
	shared_ptr<GameObject>		m_parent;
	shared_ptr<GameObject>		m_sibling;
	shared_ptr<GameObject>		m_child;

	BoundingOrientedBox			m_boundingBox = BoundingOrientedBox();
};

class Helicoptor : public GameObject
{
public:
	Helicoptor();
	~Helicoptor() = default;

	virtual void Update(FLOAT timeElapsed) override;

	virtual void SetRotorFrame();
protected:
	shared_ptr<GameObject>	m_mainRotorFrame;
	shared_ptr<GameObject>	m_tailRotorFrame;
	shared_ptr<GameObject>	m_missileFrame;
};

class Shader;
class ParticleShader;
class Enemy : public Helicoptor
{
public:
	Enemy();
	~Enemy() = default;

	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) const override;
	virtual void Update(FLOAT timeElapsed) override;

	virtual void SetRotorFrame() override;
	shared_ptr<Mesh> GetMesh() { return m_mesh; }

	void InitParticle(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList);

	enum Status {
		LIVE,
		BLOWING,
		DEATH
	};

	INT GetStatus() { return m_status; }
	void SetStatus(INT status) { m_status = status; }
	void SetTargetPosition(XMFLOAT3 position) { m_targetPosition = position; }
	void SetTerrainHeight(FLOAT height) { m_terrainHeight = height; }
	void SetParticleShader(const shared_ptr<ParticleShader>& particleShader) { m_particleShader = particleShader; }
	void SetShader(const shared_ptr<Shader>& shader) { m_shader = shader; }

private:
	INT			m_status;
	XMFLOAT3	m_targetPosition;
	FLOAT		m_terrainHeight;

	unique_ptr<ParticleMesh>		m_particle;
	shared_ptr<ParticleShader>		m_particleShader;
	shared_ptr<Shader>				m_shader;
	FLOAT							m_blowingAge;
	const FLOAT						m_blowingLifeTime = 0.5f;
	FLOAT							m_timeElapsed;
};

class HeightMapTerrain;


class EnemyManager : public GameObject
{
public:
	EnemyManager();
	~EnemyManager() = default;

	void InitEnemy(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandlist);
	void SetTarget(const shared_ptr<GameObject>& target) { m_target = target; }
	void SetTerrain(const shared_ptr< HeightMapTerrain>& terrain) { m_terrain = terrain; }
	void SetParticleShader(const shared_ptr<ParticleShader>& particleShader) { m_particleShader = particleShader; }
	void SetShader(const shared_ptr<Shader>& shader) { m_shader = shader; }
	vector<shared_ptr<Enemy>> GetEnemys() { return m_enemys; }

	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) const override;
	virtual void Update(FLOAT timeElapsed) override;

private:
	const FLOAT						m_regenTime = 5.f;
	const INT						m_maxRegen = 20;
	vector<shared_ptr<Enemy>>		m_enemys;
	FLOAT							m_regenTimer;

	shared_ptr<GameObject>			m_target;
	shared_ptr<HeightMapTerrain>	m_terrain;
	shared_ptr<ParticleShader>		m_particleShader;
	shared_ptr<Shader>				m_shader;
};

class HeightMapTerrain : public GameObject
{
public:
	HeightMapTerrain(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList, 
		const wstring& fileName, INT width, INT length, INT blockWidth, INT blockLength, XMFLOAT3 scale);
	~HeightMapTerrain() = default;

	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>&commandList) const;
	virtual void Move(const XMFLOAT3& shift);
	virtual void Rotate(FLOAT roll, FLOAT pitch, FLOAT yaw);

	void SetPosition(const XMFLOAT3& position);

	XMFLOAT3 GetPosition() const { return m_blocks.front()->GetPosition(); }
	FLOAT GetHeight(FLOAT x, FLOAT z) const;
	XMFLOAT3 GetNormal(FLOAT x, FLOAT z) const;
	INT GetWidth() const { return m_width; }
	INT GetLength() const { return m_length; }
	XMFLOAT3 GetScale() const { return m_scale; }

	void ReleaseUploadBuffer() const override;

private:
	unique_ptr<HeightMapImage>		m_heightMapImage;	// ���̸� �̹���
	vector<unique_ptr<GameObject>>	m_blocks;			// ��ϵ�
	INT								m_width;			// �̹����� ���� ����
	INT								m_length;			// �̹����� ���� ����
	XMFLOAT3						m_scale;			// Ȯ�� ����
};

class Skybox : public GameObject
{
public:
	Skybox(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList,
		FLOAT width, FLOAT length, FLOAT depth);
	~Skybox() = default;

	virtual void Update(FLOAT timeElapsed);
};
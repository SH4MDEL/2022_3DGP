#pragma once
#include "stdafx.h"
#include "mesh.h"
#include "shader.h"

class GameObject
{
public:
	GameObject();
	~GameObject();

	virtual void Update(FLOAT timeElapsed) { };
	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) const;
	virtual void Move(const XMFLOAT3& shift);
	virtual void Rotate(FLOAT roll, FLOAT pitch, FLOAT yaw);
	virtual void UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& commandList) const;

	void SetMesh(const shared_ptr<Mesh>& Mesh);
	void SetShader(const shared_ptr<Shader>& shader);

	void SetPosition(const XMFLOAT3& position);

	XMFLOAT3 GetPosition() const;
	XMFLOAT3 GetRight() const { return m_right; }
	XMFLOAT3 GetUp() const { return m_up; }
	XMFLOAT3 GetFront() const { return m_front; }

	void ReleaseUploadBuffer() const;

protected:
	XMFLOAT4X4			m_worldMatrix;	// ���� ��ȯ

	XMFLOAT3			m_right;		// ���� x��
	XMFLOAT3			m_up;			// ���� y��
	XMFLOAT3			m_front;		// ���� z��

	FLOAT				m_roll;			// x�� ȸ����
	FLOAT				m_pitch;		// y�� ȸ����
	FLOAT				m_yaw;			// z�� ȸ����

	shared_ptr<Mesh>	m_mesh;			// �޽�
	shared_ptr<Shader>	m_shader;		// ���̴�
};

class RotatingObject : public GameObject
{
public:
	RotatingObject();
	~RotatingObject() = default;

	virtual void Update(FLOAT timeElapsed);

private:
	FLOAT				m_rotationSpeed;
};
#pragma once
#include "stdafx.h"
#include "object.h"

#define ROLL_MAX +20
#define ROLL_MIN -10

class Camera;

class Player : public GameObject
{
public:
	Player();
	~Player() = default;

	virtual void Rotate(FLOAT roll, FLOAT pitch, FLOAT yaw);

	void ApplyFriction(FLOAT deltaTime);

	XMFLOAT3 GetVelocity() const { return m_velocity; }
	void SetVelocity(const XMFLOAT3& velocity) { m_velocity = velocity; }
	void AddVelocity(const XMFLOAT3& increase);
	void SetCamera(const shared_ptr<Camera>& camera) { m_camera = camera; }

private:
	XMFLOAT3			m_velocity;		// �ӵ�
	FLOAT				m_maxVelocity;	// �ִ�ӵ�
	FLOAT				m_friction;		// ������
	shared_ptr<Camera>	m_camera;		// ī�޶�
};

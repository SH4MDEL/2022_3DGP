#pragma once
#include "stdafx.h"

class Psysics
{
public:
	Psysics();
	~Psysics();
private:
	const FLOAT						m_gravity = -10.0f;		// �߷�
	XMFLOAT3						m_velocity;		// �ӵ�
	XMFLOAT3						m_acceleration;	// ���ӵ�
	FLOAT							m_maxVelocity;	// �ִ�ӵ�
	FLOAT							m_friction;		// ������
};


#pragma once
#include "stdafx.h"
#include "object.h"

class Camera;

class Player : public Helicoptor
{
public:
	Player();
	~Player() = default;

	virtual void Update(FLOAT timeElapsed);
	virtual void Rotate(FLOAT roll, FLOAT pitch, FLOAT yaw);

	void ApplyFriction(FLOAT deltaTime);
	void ApplyGravity(FLOAT deltaTime);

	XMFLOAT3 GetVelocity() const { return m_velocity; }
	shared_ptr<HeightMapTerrain> GetTerrain() const { return m_terrain; }
	shared_ptr<GameObject> GetMissile() const { return m_missileFrame; }

	void SetVelocity(const XMFLOAT3& velocity) { m_velocity = velocity; }
	void AddVelocity(const XMFLOAT3& increase);
	void SetCamera(const shared_ptr<Camera>& camera) { m_camera = camera; }
	void SetFirstCamera(const shared_ptr<Camera>& firstCamera) { m_firstCamera = firstCamera; }
	void SetTerrain(const shared_ptr<HeightMapTerrain>& terrain) { m_terrain = terrain; }

	enum MissileStatus {
		READY,
		SHOTTING,
		BLOWING
	};

	void LaunchMissile();
	INT GetMissileState() { return m_missileState; }
	void SetMissileState(INT missileState) { m_missileState = missileState; }

private:
	XMFLOAT3						m_velocity;		// 속도
	FLOAT							m_maxVelocity;	// 최대속도
	FLOAT							m_friction;		// 마찰력

	shared_ptr<Camera>				m_camera;		// 카메라
	shared_ptr<Camera>				m_firstCamera;
	shared_ptr<HeightMapTerrain>	m_terrain;

	const FLOAT						m_missileLifeTime = 3.f;
	INT								m_missileState;
	XMFLOAT3						m_missileVector;
	XMFLOAT4X4						m_missileWorld;
	FLOAT							m_missileTimer;
};

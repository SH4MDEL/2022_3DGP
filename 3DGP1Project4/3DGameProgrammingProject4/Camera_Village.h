#pragma once
#include "Camera.h"

class CPlayer_Village;

class CCamera_Village : public CCamera
{
public:
	CCamera_Village();
	CCamera_Village(CCamera_Village* pCamera);
	virtual ~CCamera_Village();

	CPlayer_Village* m_pPlayer;

	virtual void SetOffset(XMFLOAT3 xmf3Offset) { m_xmf3Offset = xmf3Offset; m_xmf3Position.x += xmf3Offset.x; m_xmf3Position.y += xmf3Offset.y; m_xmf3Position.z += xmf3Offset.z; }

	void SetPlayer(CPlayer_Village* pPlayer) { m_pPlayer = pPlayer; }
	CPlayer_Village* GetPlayer() { return(m_pPlayer); }
};

class CSpaceShipCamera_Village : public CCamera_Village
{
public:
	CSpaceShipCamera_Village(CCamera_Village* pCamera);
	virtual ~CSpaceShipCamera_Village();

	virtual void Rotate(float fPitch = 0.0f, float fYaw = 0.0f, float fRoll = 0.0f);
};

class CFirstPersonCamera_Village : public CCamera_Village
{
public:
	CFirstPersonCamera_Village(CCamera_Village* pCamera);
	virtual ~CFirstPersonCamera_Village();

	virtual void Rotate(float fPitch = 0.0f, float fYaw = 0.0f, float fRoll = 0.0f);
};

class CThirdPersonCamera_Village : public CCamera_Village
{
public:
	CThirdPersonCamera_Village(CCamera_Village* pCamera);
	virtual ~CThirdPersonCamera_Village();

	virtual void Update(XMFLOAT3& xmf3LookAt, float fTimeElapsed);
	virtual void SetLookAt(XMFLOAT3& vLookAt);
};


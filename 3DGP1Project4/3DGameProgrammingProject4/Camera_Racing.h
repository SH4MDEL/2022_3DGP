#pragma once
#include "Camera.h"

class CPlayer_Racing;

class CCamera_Racing : public CCamera
{
public:
	CCamera_Racing();
	CCamera_Racing(CCamera_Racing* pCamera);
	virtual ~CCamera_Racing();

	CPlayer_Racing* m_pPlayer;

	virtual void SetOffset(XMFLOAT3 xmf3Offset) { m_xmf3Offset = xmf3Offset; }

	void SetPlayer(CPlayer_Racing* pPlayer) { m_pPlayer = pPlayer; }
	CPlayer_Racing* GetPlayer() { return(m_pPlayer); }
};

class CThirdPersonCamera_Racing : public CCamera_Racing
{
public:
	CThirdPersonCamera_Racing(CCamera_Racing* pCamera);
	virtual ~CThirdPersonCamera_Racing();

	virtual void Update(XMFLOAT3& xmf3LookAt, float fTimeElapsed);
	virtual void SetLookAt(XMFLOAT3& vLookAt);
};




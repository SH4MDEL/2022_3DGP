#pragma once

#include "Mesh.h"
#include "Camera.h"

class CGameObject
{
public:
	CGameObject() { }
	virtual ~CGameObject();

public:
	bool						m_bActive = true;

	CMesh*						m_pMesh = NULL;
	XMFLOAT4X4					m_xmf4x4World = Matrix4x4::Identity();

	BoundingOrientedBox			m_xmOOBB = BoundingOrientedBox();

	CGameObject*				m_pObjectCollided = NULL;
	DWORD						m_dwColor = RGB(255, 0, 0);

	XMFLOAT3					m_xmf3MovingDirection = XMFLOAT3(0.0f, 0.0f, 1.0f);
	float						m_fMovingSpeed = 0.0f;
	float						m_fMovingRange = 0.0f;

	XMFLOAT3					m_xmf3RotationAxis = XMFLOAT3(0.0f, 1.0f, 0.0f);
	float						m_fRotationSpeed = 0.0f;

public:
	void SetActive(bool bActive) { m_bActive = bActive; }
	void SetMesh(CMesh *pMesh) { m_pMesh = pMesh; if (pMesh) pMesh->AddRef(); }

	void SetColor(DWORD dwColor) { m_dwColor = dwColor; }

	void SetRotationTransform(XMFLOAT4X4 *pmxf4x4Transform);

	void SetPosition(float x, float y, float z);
	void SetPosition(XMFLOAT3& xmf3Position);

	void SetMovingDirection(const XMFLOAT3& xmf3MovingDirection) { m_xmf3MovingDirection = Vector3::Normalize(xmf3MovingDirection); }
	void SetMovingSpeed(float fSpeed) { m_fMovingSpeed = fSpeed; }
	void SetMovingRange(float fRange) { m_fMovingRange = fRange; }

	void SetRotationAxis(const XMFLOAT3& xmf3RotationAxis) { m_xmf3RotationAxis = Vector3::Normalize(xmf3RotationAxis); }
	void SetRotationSpeed(float fSpeed) { m_fRotationSpeed = fSpeed; }

	void MoveStrafe(float fDistance = 1.0f);
	void MoveUp(float fDistance = 1.0f);
	void MoveForward(float fDistance = 1.0f);
	void Move(XMFLOAT3& vDirection, float fSpeed);

	void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);
	void Rotate(XMFLOAT3& xmf3Axis, float fAngle);

	void Scale(float x, float y, float z);
	void Scale(XMFLOAT3& xmfFloat);

	XMFLOAT3 GetPosition();
	XMFLOAT3 GetLook();
	XMFLOAT3 GetUp();
	XMFLOAT3 GetRight();

	void LookTo(XMFLOAT3& xmf3LookTo, XMFLOAT3& xmf3Up);
	void LookAt(XMFLOAT3& xmf3LookAt, XMFLOAT3& xmf3Up);

	void UpdateBoundingBox();

	void Render(HDC hDCFrameBuffer, XMFLOAT4X4* pxmf4x4World, CMesh* pMesh);

	virtual void OnUpdateTransform() { }
	virtual void Animate(float fElapsedTime);
	virtual void Render(HDC hDCFrameBuffer, CCamera* pCamera);

	void GenerateRayForPicking(XMVECTOR& xmvPickPosition, XMMATRIX& xmmtxView, XMVECTOR& xmvPickRayOrigin, XMVECTOR& xmvPickRayDirection);
	int PickObjectByRayIntersection(XMVECTOR& xmPickPosition, XMMATRIX& xmmtxView, float* pfHitDistance);
};

class CEnemyObject : public CGameObject
{
public:
	CEnemyObject();
	virtual ~CEnemyObject();

	bool						m_bBlowingUp = false;
	bool						m_bIsDeleted = false;

	XMFLOAT4X4					m_pxmf4x4Transforms[EXPLOSION_DEBRISES];

	float						m_fElapsedTimes = 0.0f;
	float						m_fDuration = 2.0f;
	float						m_fExplosionSpeed = 10.0f;
	float						m_fExplosionRotation = 720.0f;

	float						m_fEnemySpeedElapse = 0.0f;

	virtual void Animate(float fElapsedTime);
	virtual void Render(HDC hDCFrameBuffer, CCamera* pCamera);

public:
	static CMesh*				m_pEnemyMesh;
	static XMFLOAT3				m_pxmf3SphereVectors[EXPLOSION_DEBRISES];

	static void PrepareExplosion();
};

class CBulletObject : public CGameObject
{
public:
	CBulletObject(float fEffectiveRange);
	virtual ~CBulletObject();

public:
	virtual void Animate(float fElapsedTime);

	float						m_fBulletEffectiveRange = 50.0f;
	float						m_fMovingDistance = 0.0f;
	float						m_fRotationAngle = 0.0f;
	XMFLOAT3					m_xmf3FirePosition = XMFLOAT3(0.0f, 0.0f, 1.0f);

	float						m_fElapsedTimeAfterFire = 0.0f;
	float						m_fLockingDelayTime = 0.3f;
	float						m_fLockingTime = 4.0f;
	CGameObject*				m_pLockedObject = NULL;

	void SetFirePosition(XMFLOAT3 xmf3FirePosition);
	void Reset();
};

class CAxisObject : public CGameObject
{
public:
	CAxisObject() { }
	virtual ~CAxisObject() { }

	virtual void Render(HDC hDCFrameBuffer, CCamera* pCamera);
};

class CRailObject : public CGameObject
{
public:
	CRailObject(XMFLOAT3 p1, XMFLOAT3 p2, XMFLOAT3 p3, XMFLOAT3 p4);
	virtual ~CRailObject();

	virtual void Render(HDC hDCFrameBuffer, CCamera* pCamera);

	void SetPlayerEntryPoint();
	void EnableRailObject();
	bool isEnableRailObject() { return isinPlayer; }
	XMFLOAT3 UpdatePlayerPosition(float fElapsedTime);
	XMFLOAT3 GetPlayerDirection() { return m_xmf3NormalizedDirection; };
public:
	bool isinPlayer	= false;
	float m_fPlayerSpeed = 20.0f;
	XMFLOAT3 m_xmf3PlayerPosition;
	XMFLOAT3 m_xmf3NormalizedDirection;

	std::vector<XMFLOAT4X4> m_xmf4x4Bezier;
	std::vector<XMFLOAT4X4>::iterator m_xmf4x4BezierIter;
};

class CTrainObject : public CGameObject
{
public:
	CTrainObject() {};
	virtual ~CTrainObject() {};

	virtual void Render(HDC hDCFrameBuffer, CCamera* pCamera);
};
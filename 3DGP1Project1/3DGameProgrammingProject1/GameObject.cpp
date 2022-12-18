#include "stdafx.h"
#include "GameObject.h"
#include "GraphicsPipeline.h"

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
inline float RandF(float fMin, float fMax)
{
	return(fMin + ((float)rand() / (float)RAND_MAX) * (fMax - fMin));
}

XMVECTOR RandomUnitVectorOnSphere()
{
	XMVECTOR xmvOne = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
	XMVECTOR xmvZero = XMVectorZero();

	while (true)
	{
		XMVECTOR v = XMVectorSet(RandF(-1.0f, 1.0f), RandF(-1.0f, 1.0f), RandF(-1.0f, 1.0f), 0.0f);
		if (!XMVector3Greater(XMVector3LengthSq(v), xmvOne)) return(XMVector3Normalize(v));
	}
}


CGameObject::~CGameObject(void)
{
	if (m_pMesh) m_pMesh->Release();
}

void CGameObject::SetPosition(float x, float y, float z)
{
	m_xmf4x4World._41 = x;
	m_xmf4x4World._42 = y;
	m_xmf4x4World._43 = z;
}

void CGameObject::SetPosition(XMFLOAT3& xmf3Position)
{
	m_xmf4x4World._41 = xmf3Position.x;
	m_xmf4x4World._42 = xmf3Position.y;
	m_xmf4x4World._43 = xmf3Position.z;
}

XMFLOAT3 CGameObject::GetPosition()
{
	return(XMFLOAT3(m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43));
}

XMFLOAT3 CGameObject::GetLook()
{
	XMFLOAT3 xmf3LookAt(m_xmf4x4World._31, m_xmf4x4World._32, m_xmf4x4World._33);
	xmf3LookAt = Vector3::Normalize(xmf3LookAt);
	return(xmf3LookAt);
}

XMFLOAT3 CGameObject::GetUp()
{
	XMFLOAT3 xmf3Up(m_xmf4x4World._21, m_xmf4x4World._22, m_xmf4x4World._23);
	xmf3Up = Vector3::Normalize(xmf3Up);
	return(xmf3Up);
}

XMFLOAT3 CGameObject::GetRight()
{
	XMFLOAT3 xmf3Right(m_xmf4x4World._11, m_xmf4x4World._12, m_xmf4x4World._13);
	xmf3Right = Vector3::Normalize(xmf3Right);
	return(xmf3Right);
}

void CGameObject::SetRotationTransform(XMFLOAT4X4* pmxf4x4Transform)
{
	m_xmf4x4World._11 = pmxf4x4Transform->_11; m_xmf4x4World._12 = pmxf4x4Transform->_12; m_xmf4x4World._13 = pmxf4x4Transform->_13;
	m_xmf4x4World._21 = pmxf4x4Transform->_21; m_xmf4x4World._22 = pmxf4x4Transform->_22; m_xmf4x4World._23 = pmxf4x4Transform->_23;
	m_xmf4x4World._31 = pmxf4x4Transform->_31; m_xmf4x4World._32 = pmxf4x4Transform->_32; m_xmf4x4World._33 = pmxf4x4Transform->_33;
}

void CGameObject::MoveStrafe(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Right = GetRight();
	xmf3Position = Vector3::Add(xmf3Position, Vector3::ScalarProduct(xmf3Right, fDistance));

	CGameObject::SetPosition(xmf3Position);
}

void CGameObject::MoveUp(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Up = GetUp();
	xmf3Position = Vector3::Add(xmf3Position, Vector3::ScalarProduct(xmf3Up, fDistance));

	CGameObject::SetPosition(xmf3Position);
}

void CGameObject::MoveForward(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3LookAt = GetLook();
	xmf3Position = Vector3::Add(xmf3Position, Vector3::ScalarProduct(xmf3LookAt, fDistance));

	CGameObject::SetPosition(xmf3Position);
}

void CGameObject::Rotate(float fPitch, float fYaw, float fRoll)
{
	XMFLOAT4X4 mtxRotate = Matrix4x4::RotationYawPitchRoll(fPitch, fYaw, fRoll);
	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);
}

void CGameObject::Rotate(XMFLOAT3& xmf3RotationAxis, float fAngle)
{
	XMFLOAT4X4 mtxRotate = Matrix4x4::RotationAxis(xmf3RotationAxis, fAngle);
	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);
}

void CGameObject::Scale(float x, float y, float z)
{
	XMFLOAT4X4 mtxScale;
	mtxScale._11 = x; mtxScale._12 = 0.0f; mtxScale._13 = 0.0f, mtxScale._14 = 0.0f;
	mtxScale._21 = 0.0f; mtxScale._22 = y; mtxScale._23 = 0.0f, mtxScale._24 = 0.0f;
	mtxScale._31 = 0.0f; mtxScale._32 = 0.0f; mtxScale._33 = z, mtxScale._34 = 0.0f;
	mtxScale._41 = 0.0f; mtxScale._42 = 0.0f; mtxScale._43 = 0.0f, mtxScale._44 = 1.0f;
	m_xmf4x4World = Matrix4x4::Multiply(mtxScale, m_xmf4x4World);
}

void CGameObject::Scale(XMFLOAT3& xmfFloat)
{
	Scale(xmfFloat.x, xmfFloat.y, xmfFloat.z);
}

void CGameObject::Move(XMFLOAT3& vDirection, float fSpeed)
{
	SetPosition(m_xmf4x4World._41 + vDirection.x * fSpeed, m_xmf4x4World._42 + vDirection.y * fSpeed, m_xmf4x4World._43 + vDirection.z * fSpeed);
}

void CGameObject::LookTo(XMFLOAT3& xmf3LookTo, XMFLOAT3& xmf3Up)
{
	XMFLOAT4X4 xmf4x4View = Matrix4x4::LookToLH(GetPosition(), xmf3LookTo, xmf3Up);
	m_xmf4x4World._11 = xmf4x4View._11; m_xmf4x4World._12 = xmf4x4View._21; m_xmf4x4World._13 = xmf4x4View._31;
	m_xmf4x4World._21 = xmf4x4View._12; m_xmf4x4World._22 = xmf4x4View._22; m_xmf4x4World._23 = xmf4x4View._32;
	m_xmf4x4World._31 = xmf4x4View._13; m_xmf4x4World._32 = xmf4x4View._23; m_xmf4x4World._33 = xmf4x4View._33;
}

void CGameObject::LookAt(XMFLOAT3& xmf3LookAt, XMFLOAT3& xmf3Up)
{
	XMFLOAT4X4 xmf4x4View = Matrix4x4::LookAtLH(GetPosition(), xmf3LookAt, xmf3Up);
	m_xmf4x4World._11 = xmf4x4View._11; m_xmf4x4World._12 = xmf4x4View._21; m_xmf4x4World._13 = xmf4x4View._31;
	m_xmf4x4World._21 = xmf4x4View._12; m_xmf4x4World._22 = xmf4x4View._22; m_xmf4x4World._23 = xmf4x4View._32;
	m_xmf4x4World._31 = xmf4x4View._13; m_xmf4x4World._32 = xmf4x4View._23; m_xmf4x4World._33 = xmf4x4View._33;
}

void CGameObject::UpdateBoundingBox()
{
	if (m_pMesh)
	{
		m_pMesh->m_xmOOBB.Transform(m_xmOOBB, XMLoadFloat4x4(&m_xmf4x4World));
		XMStoreFloat4(&m_xmOOBB.Orientation, XMQuaternionNormalize(XMLoadFloat4(&m_xmOOBB.Orientation)));
	}
}

void CGameObject::Animate(float fElapsedTime)
{
	if (m_fRotationSpeed != 0.0f) Rotate(m_xmf3RotationAxis, m_fRotationSpeed * fElapsedTime);
	if (m_fMovingSpeed != 0.0f) Move(m_xmf3MovingDirection, m_fMovingSpeed * fElapsedTime);

	UpdateBoundingBox();
}

void CGameObject::Render(HDC hDCFrameBuffer, XMFLOAT4X4* pxmf4x4World, CMesh* pMesh)
{
	if (pMesh)
	{
		CGraphicsPipeline::SetWorldTransform(pxmf4x4World);

		HPEN hPen = ::CreatePen(PS_SOLID, 0, m_dwColor);
		HPEN hOldPen = (HPEN)::SelectObject(hDCFrameBuffer, hPen);
		pMesh->Render(hDCFrameBuffer);
		::SelectObject(hDCFrameBuffer, hOldPen);
		::DeleteObject(hPen);
	}
}

void CGameObject::Render(HDC hDCFrameBuffer, CCamera* pCamera)
{
	if (pCamera->IsInFrustum(m_xmOOBB)) CGameObject::Render(hDCFrameBuffer, &m_xmf4x4World, m_pMesh);
}

void CGameObject::GenerateRayForPicking(XMVECTOR& xmvPickPosition, XMMATRIX& xmmtxView, XMVECTOR& xmvPickRayOrigin, XMVECTOR& xmvPickRayDirection)
{
	XMMATRIX xmmtxToModel = XMMatrixInverse(NULL, XMLoadFloat4x4(&m_xmf4x4World) * xmmtxView);

	XMFLOAT3 xmf3CameraOrigin(0.0f, 0.0f, 0.0f);
	xmvPickRayOrigin = XMVector3TransformCoord(XMLoadFloat3(&xmf3CameraOrigin), xmmtxToModel);
	xmvPickRayDirection = XMVector3TransformCoord(xmvPickPosition, xmmtxToModel);
	xmvPickRayDirection = XMVector3Normalize(xmvPickRayDirection - xmvPickRayOrigin);
}

int CGameObject::PickObjectByRayIntersection(XMVECTOR& xmvPickPosition, XMMATRIX& xmmtxView, float* pfHitDistance)
{
	int nIntersected = 0;
	if (m_pMesh)
	{
		XMVECTOR xmvPickRayOrigin, xmvPickRayDirection;
		GenerateRayForPicking(xmvPickPosition, xmmtxView, xmvPickRayOrigin, xmvPickRayDirection);
		nIntersected = m_pMesh->CheckRayIntersection(xmvPickRayOrigin, xmvPickRayDirection, pfHitDistance);
	}
	return(nIntersected);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
XMFLOAT3 CEnemyObject::m_pxmf3SphereVectors[EXPLOSION_DEBRISES];
CMesh* CEnemyObject::m_pEnemyMesh = NULL;

CEnemyObject::CEnemyObject()
{
}

CEnemyObject::~CEnemyObject()
{
}

void CEnemyObject::PrepareExplosion()
{
	for (int i = 0; i < EXPLOSION_DEBRISES; i++) XMStoreFloat3(&m_pxmf3SphereVectors[i], ::RandomUnitVectorOnSphere());

	m_pEnemyMesh = new CCubeMesh(0.5f, 0.5f, 0.5f);
}

void CEnemyObject::Animate(float fElapsedTime)
{
	if (m_bBlowingUp)
	{
		m_fElapsedTimes += fElapsedTime;
		if (m_fElapsedTimes <= m_fDuration)
		{
			XMFLOAT3 xmf3Position = GetPosition();
			for (int i = 0; i < EXPLOSION_DEBRISES; i++)
			{
				m_pxmf4x4Transforms[i] = Matrix4x4::Identity();
				m_pxmf4x4Transforms[i]._41 = xmf3Position.x + m_pxmf3SphereVectors[i].x * m_fExplosionSpeed * m_fElapsedTimes;
				m_pxmf4x4Transforms[i]._42 = xmf3Position.y + m_pxmf3SphereVectors[i].y * m_fExplosionSpeed * m_fElapsedTimes;
				m_pxmf4x4Transforms[i]._43 = xmf3Position.z + m_pxmf3SphereVectors[i].z * m_fExplosionSpeed * m_fElapsedTimes;
				m_pxmf4x4Transforms[i] = Matrix4x4::Multiply(Matrix4x4::RotationAxis(m_pxmf3SphereVectors[i], m_fExplosionRotation * m_fElapsedTimes), m_pxmf4x4Transforms[i]);
			}
		}
		else
		{
			m_bBlowingUp = false;
			m_bIsDeleted = true;
			m_fElapsedTimes = 0.0f;
		}
	}
	else
	{
		CGameObject::Animate(fElapsedTime);
	}
	m_fEnemySpeedElapse += fElapsedTime;
	if (m_fEnemySpeedElapse >= 2.0 && m_fMovingSpeed < 200.0f) {
		m_fMovingSpeed += 5.0f;
		m_fEnemySpeedElapse = 0.0f;
	}
}

void CEnemyObject::Render(HDC hDCFrameBuffer, CCamera* pCamera)
{
	if (m_bBlowingUp)
	{
		for (int i = 0; i < EXPLOSION_DEBRISES; i++)
		{
			CGameObject::Render(hDCFrameBuffer , &m_pxmf4x4Transforms[i], m_pEnemyMesh);
		}
	}
	else
	{
		CGameObject::Render(hDCFrameBuffer, pCamera);
	}
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
CBulletObject::CBulletObject(float fEffectiveRange)
{
	m_fBulletEffectiveRange = fEffectiveRange;
}

CBulletObject::~CBulletObject()
{
}

void CBulletObject::SetFirePosition(XMFLOAT3 xmf3FirePosition)
{
	m_xmf3FirePosition = xmf3FirePosition;
	SetPosition(xmf3FirePosition);
}

void CBulletObject::Reset()
{
	m_pLockedObject = NULL;
	m_fElapsedTimeAfterFire = 0;
	m_fMovingDistance = 0;
	m_fRotationAngle = 0.0f;

	m_bActive = false;
}

void CBulletObject::Animate(float fElapsedTime)
{
	m_fElapsedTimeAfterFire += fElapsedTime;

	float fDistance = m_fMovingSpeed * fElapsedTime;

	if ((m_fElapsedTimeAfterFire > m_fLockingDelayTime) && m_pLockedObject)
	{
		XMFLOAT3 xmf3Position = GetPosition();
		XMVECTOR xmvPosition = XMLoadFloat3(&xmf3Position);

		XMFLOAT3 xmf3LockedObjectPosition = m_pLockedObject->GetPosition();
		XMVECTOR xmvLockedObjectPosition = XMLoadFloat3(&xmf3LockedObjectPosition);
		XMVECTOR xmvToLockedObject = xmvLockedObjectPosition - xmvPosition;
		xmvToLockedObject = XMVector3Normalize(xmvToLockedObject);

		XMVECTOR xmvMovingDirection = XMLoadFloat3(&m_xmf3MovingDirection);
		xmvMovingDirection = XMVector3Normalize(XMVectorLerp(xmvMovingDirection, xmvToLockedObject, 0.25f));
		XMStoreFloat3(&m_xmf3MovingDirection, xmvMovingDirection);
	}
#ifdef _WITH_VECTOR_OPERATION
	XMFLOAT3 xmf3Position = GetPosition();

	m_fRotationAngle += m_fRotationSpeed * fElapsedTime;
	if (m_fRotationAngle > 360.0f) m_fRotationAngle = m_fRotationAngle - 360.0f;

	XMFLOAT4X4 mtxRotate1 = Matrix4x4::RotationYawPitchRoll(0.0f, m_fRotationAngle, 0.0f);

	XMFLOAT3 xmf3RotationAxis = Vector3::CrossProduct(m_xmf3RotationAxis, m_xmf3MovingDirection, true);
	float fDotProduct = Vector3::DotProduct(m_xmf3RotationAxis, m_xmf3MovingDirection);
	float fRotationAngle = ::IsEqual(fDotProduct, 1.0f) ? 0.0f : (float)XMConvertToDegrees(acos(fDotProduct));
	XMFLOAT4X4 mtxRotate2 = Matrix4x4::RotationAxis(xmf3RotationAxis, fRotationAngle);

	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate1, mtxRotate2);

	XMFLOAT3 xmf3Movement = Vector3::ScalarProduct(m_xmf3MovingDirection, fDistance, false);
	xmf3Position = Vector3::Add(xmf3Position, xmf3Movement);
	SetPosition(xmf3Position);
#else
	XMFLOAT4X4 mtxRotate = Matrix4x4::RotationYawPitchRoll(0.0f, m_fRotationSpeed * fElapsedTime, 0.0f);
	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);
	XMFLOAT3 xmf3Movement = Vector3::ScalarProduct(m_xmf3MovingDirection, fDistance, false);
	XMFLOAT3 xmf3Position = GetPosition();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Movement);
	SetPosition(xmf3Position);
	m_fMovingDistance += fDistance;
#endif

	UpdateBoundingBox();

	if ((m_fMovingDistance > m_fBulletEffectiveRange) || (m_fElapsedTimeAfterFire > m_fLockingTime)) Reset();
}

/////////////////////////////////////////////////////////////////////////////////////////////////////
//
void CAxisObject::Render(HDC hDCFrameBuffer, CCamera* pCamera)
{
	CGraphicsPipeline::SetWorldTransform(&m_xmf4x4World);

	m_pMesh->Render(hDCFrameBuffer);
}

/////////////////////////////////////////////////////////////////////////////////////////////////////

CRailObject::CRailObject(XMFLOAT3 p1, XMFLOAT3 p2, XMFLOAT3 p3, XMFLOAT3 p4)
{
	int iDistance = (int)((sqrt(pow(p3.x - p2.x, 2) + pow(p3.y - p2.y, 2) + (pow(p3.z - p2.z, 2))) + 1) / 2.0f);
	for (int i = 0; i < iDistance; ++i) {
		float t = (float)i / (float)iDistance;
		m_xmf4x4World._41 = (float)(((pow(-t, 3) + 2 * pow(t, 2) - t) * p1.x + (3 * pow(t, 3) - 5 * pow(t, 2) + 2) * p2.x +
			(-3 * pow(t, 3) + 4 * pow(t, 2) + t) * p3.x + (pow(t, 3) - pow(t, 2)) * p4.x) / 2.0f);
		m_xmf4x4World._42 = (float)(((pow(-t, 3) + 2 * pow(t, 2) - t) * p1.y + (3 * pow(t, 3) - 5 * pow(t, 2) + 2) * p2.y +
			(-3 * pow(t, 3) + 4 * pow(t, 2) + t) * p3.y + (pow(t, 3) - pow(t, 2)) * p4.y) / 2.0f);
		m_xmf4x4World._43 = (float)(((pow(-t, 3) + 2 * pow(t, 2) - t) * p1.z + (3 * pow(t, 3) - 5 * pow(t, 2) + 2) * p2.z +
			(-3 * pow(t, 3) + 4 * pow(t, 2) + t) * p3.z + (pow(t, 3) - pow(t, 2)) * p4.z) / 2.0f);

		m_xmf4x4Bezier.push_back(m_xmf4x4World);
	}
	m_xmf4x4BezierIter = m_xmf4x4Bezier.begin();
	m_xmf3PlayerPosition.x = m_xmf4x4Bezier[0]._41;
	m_xmf3PlayerPosition.y = m_xmf4x4Bezier[0]._42;
	m_xmf3PlayerPosition.z = m_xmf4x4Bezier[0]._43;
}

CRailObject::~CRailObject()
{
	while (!m_xmf4x4Bezier.empty()) {
		m_xmf4x4Bezier.pop_back();
	}
}

void CRailObject::Render(HDC hDCFrameBuffer, CCamera* pCamera)
{
	for (auto elem : m_xmf4x4Bezier) {
		CGameObject::Render(hDCFrameBuffer, &elem, m_pMesh);
	}
}

void CRailObject::EnableRailObject()
{
	isinPlayer = true;
	SetPlayerEntryPoint();
}

void CRailObject::SetPlayerEntryPoint()
{
	if ((m_xmf4x4BezierIter + 1) != m_xmf4x4Bezier.end()) {
		m_xmf3NormalizedDirection.x = (m_xmf4x4BezierIter + 1)->_41 - (*m_xmf4x4BezierIter)._41;
		m_xmf3NormalizedDirection.y = (m_xmf4x4BezierIter + 1)->_42 - (*m_xmf4x4BezierIter)._42;
		m_xmf3NormalizedDirection.z = (m_xmf4x4BezierIter + 1)->_43 - (*m_xmf4x4BezierIter)._43;
	}

	float sum = (float)sqrt(pow(m_xmf3NormalizedDirection.x, 2) + pow(m_xmf3NormalizedDirection.y, 2) + pow(m_xmf3NormalizedDirection.z, 2));

	m_xmf3NormalizedDirection.x = m_xmf3NormalizedDirection.x / sum;
	m_xmf3NormalizedDirection.y = m_xmf3NormalizedDirection.y / sum;
	m_xmf3NormalizedDirection.z = m_xmf3NormalizedDirection.z / sum;
}

XMFLOAT3 CRailObject::UpdatePlayerPosition(float fElapsedTime)
{
	if (m_xmf3PlayerPosition.z + m_xmf3NormalizedDirection.z * m_fPlayerSpeed * fElapsedTime >= (m_xmf4x4BezierIter + 1)->_43) {
		++m_xmf4x4BezierIter;
		if (m_xmf4x4BezierIter + 1 == m_xmf4x4Bezier.end()) {
			isinPlayer = false;

			m_xmf3PlayerPosition.x = (*m_xmf4x4BezierIter)._41;
			m_xmf3PlayerPosition.y = (*m_xmf4x4BezierIter)._42;
			m_xmf3PlayerPosition.z = (*m_xmf4x4BezierIter)._43;
			return m_xmf3PlayerPosition;
		}
		SetPlayerEntryPoint();
	}
	m_xmf3PlayerPosition.x += m_xmf3NormalizedDirection.x * m_fPlayerSpeed * fElapsedTime;
	m_xmf3PlayerPosition.y += m_xmf3NormalizedDirection.y * m_fPlayerSpeed * fElapsedTime;
	m_xmf3PlayerPosition.z += m_xmf3NormalizedDirection.z * m_fPlayerSpeed * fElapsedTime;

	return m_xmf3PlayerPosition;
}

void CTrainObject::Render(HDC hDCFrameBuffer, CCamera* pCamera)
{
	CGameObject::Render(hDCFrameBuffer, &m_xmf4x4World, m_pMesh);
}
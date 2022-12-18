#include "stdafx.h"
#include "Scene.h"
#include "GraphicsPipeline.h"

CScene::CScene(CPlayer* pPlayer)
{
	m_pPlayer = pPlayer;
}

CScene::~CScene()
{
}

void CScene::BuildObjects()
{
	CEnemyObject::PrepareExplosion();

	for (m_RailObjects = 0; m_RailObjects < 23; ++m_RailObjects) {
		XMFLOAT3 Coord(RD::GetRandomfloat(-10.0f, 10.0f), RD::GetRandomfloat(-10.0f, 10.0f), (float)m_RailObjects * 10.0f - 200.0f);
		m_dRailCoordinate.push_back(Coord);
	}

	pRailMesh = new CRailMesh(8.0);

	XMINT3 Color;
	for (int i = 0; i < 20; ++i) {
		Color = RD::GetRandomXMINT3(150, 200);
		m_dRailManager.push_back(new CRailObject(m_dRailCoordinate[i], m_dRailCoordinate[i + 1], 
			m_dRailCoordinate[i + 2], m_dRailCoordinate[i + 3]));
		m_dRailManager.back()->SetMesh(pRailMesh);
		m_dRailManager.back()->SetColor(RGB(Color.x, Color.y, Color.z));
	}

	m_dRailManagerIter = m_dRailManager.begin();
	for (int i = 0; i < 10; i++) {
		++m_dRailManagerIter;
	}
	(*m_dRailManagerIter)->EnableRailObject();

	pCubeMesh = new CCubeMesh(8.0f, 8.0f, 8.0f);
	for (int i = 0; i < 3; ++i) {
		XMFLOAT3 Coord = RD::GetRandomXMFLOAT3(-100, 100);
		XMINT3 Color = RD::GetRandomXMINT3(150, 200);
		m_lEnemyObjects.push_back(new CEnemyObject());
		m_lEnemyObjects.back()->SetMesh(pCubeMesh);
		m_lEnemyObjects.back()->SetPosition(Coord);
		m_lEnemyObjects.back()->SetColor(RGB(Color.x, Color.y, Color.z));
		m_lEnemyObjects.back()->SetRotationAxis(RD::GetRandomXMFLOAT3(0, 1));
		m_lEnemyObjects.back()->SetRotationSpeed(RD::GetRandomfloat(50, 100));
		m_lEnemyObjects.back()->SetMovingSpeed(RD::GetRandomfloat(5, 10));
	}

	CTrainMesh* pTrainMesh = new CTrainMesh(3.0f, 3.0f, 6.0f);
	m_pTrainObject = new CTrainObject();
	m_pTrainObject->SetMesh(pTrainMesh);
	m_pTrainObject->SetColor(RGB(0, 0, 0));
	m_pTrainObject->SetPosition(0.0f, 0.0f, 0.0f);

#ifdef _WITH_DRAW_AXIS
	m_pWorldAxis = new CGameObject();
	CAxisMesh* pAxisMesh = new CAxisMesh(0.5f, 0.5f, 0.5f);
	m_pWorldAxis->SetMesh(pAxisMesh);
#endif
}

void CScene::ReleaseObjects()
{
	if (CEnemyObject::m_pEnemyMesh) CEnemyObject::m_pEnemyMesh->Release();


#ifdef _WITH_DRAW_AXIS
	if (m_pWorldAxis) delete m_pWorldAxis;
#endif
}

void CScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
}

void CScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	switch (nMessageID)
	{
	case WM_KEYDOWN:
		switch (wParam)
		{
		case '1':
		case '2':
		case '3':
		case '4':
		case '5':
		case '6':
		case '7':
		case '8':
		case '9':
			break;
		case 'Z':
		case 'z':
			
			break;
		default:
			break;
		}
	default:
		break;
	}
}

CGameObject* CScene::PickObjectPointedByCursor(int xClient, int yClient, CCamera* pCamera)
{
	XMFLOAT3 xmf3PickPosition;
	xmf3PickPosition.x = (((2.0f * xClient) / (float)pCamera->m_Viewport.m_nWidth) - 1) / pCamera->m_xmf4x4PerspectiveProject._11;
	xmf3PickPosition.y = -(((2.0f * yClient) / (float)pCamera->m_Viewport.m_nHeight) - 1) / pCamera->m_xmf4x4PerspectiveProject._22;
	xmf3PickPosition.z = 1.0f;

	XMVECTOR xmvPickPosition = XMLoadFloat3(&xmf3PickPosition);
	XMMATRIX xmmtxView = XMLoadFloat4x4(&pCamera->m_xmf4x4View);

	int nIntersected = 0;
	float fNearestHitDistance = FLT_MAX;
	CGameObject* pNearestObject = NULL;

	return(pNearestObject);
}

void CScene::CheckObjectByBulletCollisions()
{
	CBulletObject** ppBullets = ((CTrainPlayer*)m_pPlayer)->m_ppBullets;
	for (std::list<CGameObject*>::iterator iter = m_lEnemyObjects.begin(); iter != m_lEnemyObjects.end(); ++iter) {
		for (int i = 0; i < BULLETS; ++i) {
			if (ppBullets[i]->m_bActive && (*iter)->m_xmOOBB.Intersects(ppBullets[i]->m_xmOOBB)) {
				CEnemyObject* pEnemyObject = (CEnemyObject*)(*iter);
				pEnemyObject->m_bBlowingUp = true;
				ppBullets[i]->Reset();
			}
		}
	}

}

void CScene::Animate(float fElapsedTime)
{
	XMFLOAT3 m_xmf3PlayerPosition;
	if ((*m_dRailManagerIter)->isEnableRailObject()) {
		m_xmf3PlayerPosition = (*m_dRailManagerIter)->UpdatePlayerPosition(fElapsedTime);
		m_pPlayer->SetPosition(m_xmf3PlayerPosition);
		m_pTrainObject->SetPosition(m_xmf3PlayerPosition);

		m_pTrainObject->Rotate(
			(*m_dRailManagerIter)->GetPlayerDirection().y,
			(*m_dRailManagerIter)->GetPlayerDirection().x, 0.0f);

		for (std::list<CGameObject*>::iterator iter = m_lEnemyObjects.begin(); iter != m_lEnemyObjects.end(); ++iter) {
			XMFLOAT3 m_xmf3PlayerNormal;
			m_xmf3PlayerNormal.x = m_xmf3PlayerPosition.x - (*iter)->GetPosition().x;
			m_xmf3PlayerNormal.y = m_xmf3PlayerPosition.y - (*iter)->GetPosition().y;
			m_xmf3PlayerNormal.z = m_xmf3PlayerPosition.z - (*iter)->GetPosition().z;
			float sum = (float)sqrt(pow(m_xmf3PlayerNormal.x, 2) + pow(m_xmf3PlayerNormal.y, 2) + pow(m_xmf3PlayerNormal.z, 2));
			m_xmf3PlayerNormal.x /= sum;
			m_xmf3PlayerNormal.y /= sum;
			m_xmf3PlayerNormal.z /= sum;
			(*iter)->SetMovingDirection(m_xmf3PlayerNormal);
			(*iter)->Animate(fElapsedTime);
		}
	}
	else {
		XMINT3 Color = RD::GetRandomXMINT3(150, 200);
		XMFLOAT3 Coord(RD::GetRandomfloat(-10.0f, 10.0f), RD::GetRandomfloat(-10.0f, 10.0f), (float)m_RailObjects++ * 10.0f - 200.0f);
		m_dRailCoordinate.push_back(Coord);
		m_dRailCoordinate.pop_front();

		auto i = m_dRailCoordinate.rbegin();
		m_dRailManager.push_back(new CRailObject(*(i + 3), *(i + 2), *(i + 1), *i));
		m_dRailManager.back()->SetMesh(pRailMesh);
		m_dRailManager.back()->SetColor(RGB(Color.x, Color.y, Color.z));
		m_dRailManager.pop_front();
		m_dRailManagerIter = m_dRailManager.begin();
		for (int i = 0; i < 10; i++) {
			++m_dRailManagerIter;
		}

		(*m_dRailManagerIter)->EnableRailObject();

		m_xmf3PlayerPosition = (*m_dRailManagerIter)->UpdatePlayerPosition(fElapsedTime);
		m_pPlayer->SetPosition(m_xmf3PlayerPosition);
		m_pTrainObject->SetPosition(m_xmf3PlayerPosition);
	}
	for (std::list<CGameObject*>::iterator iter = m_lEnemyObjects.begin(); iter != m_lEnemyObjects.end(); ++iter) {
		CEnemyObject* pEnemyObject = (CEnemyObject*)(*iter);
		if (pEnemyObject->m_bIsDeleted) {
			m_lEnemyObjects.erase(iter);
			break;
		}
	}
	m_fEnemyCreateElapse += fElapsedTime;
	if (m_fEnemyCreateElapse >= 3.0 && m_lEnemyObjects.size() < m_iMaxEnemy) {
		int count = RD::GetRandomint(1, 2);
		for (int i = 0; i < count; ++i) {
			XMFLOAT3 Coord = RD::GetRandomXMFLOAT3(-100, 100);
			XMINT3 Color = RD::GetRandomXMINT3(150, 200);
			m_lEnemyObjects.push_back(new CEnemyObject());
			m_lEnemyObjects.back()->SetMesh(pCubeMesh);
			m_lEnemyObjects.back()->SetPosition(m_xmf3PlayerPosition.x + Coord.x,
				m_xmf3PlayerPosition.y + Coord.y, m_xmf3PlayerPosition.z + Coord.z);
			m_lEnemyObjects.back()->SetColor(RGB(Color.x, Color.y, Color.z));
			m_lEnemyObjects.back()->SetRotationAxis(RD::GetRandomXMFLOAT3(0, 1));
			m_lEnemyObjects.back()->SetRotationSpeed(RD::GetRandomfloat(50, 100));
			m_lEnemyObjects.back()->SetMovingSpeed(RD::GetRandomfloat(5, 10));

			m_fEnemyCreateElapse = 0.0f;
		}
	}
	CheckObjectByBulletCollisions();
}

void CScene::Render(HDC hDCFrameBuffer, CCamera* pCamera)
{
	CGraphicsPipeline::SetViewport(&pCamera->m_Viewport);

	CGraphicsPipeline::SetViewPerspectiveProjectTransform(&pCamera->m_xmf4x4ViewPerspectiveProject);

	for (std::list<CGameObject*>::iterator iter = m_lEnemyObjects.begin(); iter != m_lEnemyObjects.end(); ++iter) {
		(*iter)->Render(hDCFrameBuffer, pCamera);
	}

	for (const auto& elem : m_dRailManager) elem->Render(hDCFrameBuffer, pCamera);

	if (m_pPlayer) {
		m_pPlayer->Render(hDCFrameBuffer, pCamera);
		m_pTrainObject->Render(hDCFrameBuffer, pCamera);
	}

//UI
#ifdef _WITH_DRAW_AXIS
	CGraphicsPipeline::SetViewOrthographicProjectTransform(&pCamera->m_xmf4x4ViewOrthographicProject);
	m_pWorldAxis->SetRotationTransform(&m_pPlayer->m_xmf4x4World);
	m_pWorldAxis->Render(hDCFrameBuffer, pCamera);
#endif
}

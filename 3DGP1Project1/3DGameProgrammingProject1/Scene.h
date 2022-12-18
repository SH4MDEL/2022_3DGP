#pragma once

#include "GameObject.h"
#include "Camera.h"
#include "Player.h"

class CScene
{
public:
	CScene(CPlayer *pPlayer);
	virtual ~CScene();

private:
	CPlayer*					m_pPlayer = NULL;

	CTrainObject*					m_pTrainObject = NULL;

	CCubeMesh*					pCubeMesh;
	CRailMesh*					pRailMesh;
	int							m_RailObjects = 0;

	std::deque<CRailObject*>		m_dRailManager;
	std::deque<CRailObject*>::iterator		m_dRailManagerIter;

	std::deque<XMFLOAT3>		m_dRailCoordinate;

	std::list<CGameObject*>		m_lEnemyObjects;
	float						m_fEnemyCreateElapse = 0.0f;
	int							m_iMaxEnemy = 20;

#ifdef _WITH_DRAW_AXIS
	CGameObject*				m_pWorldAxis = NULL;
#endif

public:
	virtual void BuildObjects();
	virtual void ReleaseObjects();

	void CheckObjectByBulletCollisions();

	virtual void Animate(float fElapsedTime);
	virtual void Render(HDC hDCFrameBuffer, CCamera* pCamera);

	virtual void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	virtual void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	CGameObject* PickObjectPointedByCursor(int xClient, int yClient, CCamera* pCamera);
};


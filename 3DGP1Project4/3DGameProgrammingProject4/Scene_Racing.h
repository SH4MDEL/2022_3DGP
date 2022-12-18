#pragma once
#include "Scene.h"
#include "Player_Racing.h"

class CScene_Racing : public CScene
{
public:
	CScene_Racing();
	~CScene_Racing();

	virtual bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) override;
	virtual bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) override;

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) override;
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList) override;
	virtual void ReleaseShaderVariables() override;

	void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) override;
	void ReleaseObjects() override;

	bool CheckPlayerByObjectCollisions();

	ID3D12RootSignature* CreateGraphicsRootSignature(ID3D12Device* pd3dDevice) override;

	virtual bool ProcessInput(UCHAR* pKeysBuffer) override;
	virtual void AnimateObjects(float fTimeElapsed) override;
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera_Racing* pCamera = NULL);

	virtual void ReleaseUploadBuffers() override;

	CPlayer_Racing* m_pPlayer = NULL;

public:

	std::vector<CGameObject_Racing*> m_vGameObjects;
};



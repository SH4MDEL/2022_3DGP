#pragma once
#include "Scene.h"
#include "Player_Village.h"

class CScene_Village : public CScene
{
public:
	CScene_Village();
	virtual ~CScene_Village();

	virtual bool OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) override;
	virtual bool OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam) override;

	virtual void BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) override;
	virtual void ReleaseObjects() override;

	void BuildLightsAndMaterials();

	virtual ID3D12RootSignature* CreateGraphicsRootSignature(ID3D12Device* pd3dDevice) override;
	ID3D12RootSignature* GetGraphicsRootSignature() { return(m_pd3dGraphicsRootSignature); }
	void SetGraphicsRootSignature(ID3D12GraphicsCommandList* pd3dCommandList) { pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature); }

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) override;
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList) override;
	virtual void ReleaseShaderVariables();

	virtual bool ProcessInput(UCHAR* pKeysBuffer) override;
	virtual void AnimateObjects(float fTimeElapsed) override;
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera_Village* pCamera = NULL);

	virtual void ReleaseUploadBuffers();

	void CheckPlayerByObjectCollisions();

	void DayShift();
	void NightShift();

	CPlayer_Village* m_pPlayer = NULL;

	CShader_Village** m_ppShaders = NULL;
	int	m_nShaders = 0;

	bool m_bInputProcess = false;
	bool m_bIsDay = true;

	int m_iLightCount;
};


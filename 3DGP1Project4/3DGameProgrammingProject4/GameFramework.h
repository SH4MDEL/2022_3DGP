#pragma once

#include "Timer.h"
#include "Player_Village.h"
#include "Scene.h"
#include "Scene_Racing.h"
#include "Scene_Village.h"

class CGameFramework
{
public:
	CGameFramework();
	~CGameFramework();

	bool OnCreate(HINSTANCE hInstance, HWND hMainWnd);
	void OnDestroy();

	void CreateSwapChain();
	void CreateDirect3DDevice();
	void CreateCommandQueueAndList();

	void CreateRtvAndDsvDescriptorHeaps();

	void CreateRenderTargetViews();
	void CreateDepthStencilView();
	void CreateRenderTargetViewsAndDepthStencilView();

	void ChangeSwapChainState();

	void BuildObjects();
	void ReleaseObjects();

	void ProcessInput();
	void AnimateObjects();
	void FrameAdvance();

	void WaitForGpuComplete();
	void MoveToNextFrame();

	void OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	void OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);
	LRESULT CALLBACK OnProcessingWindowMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam);

	void NightShift() { pfClearColor[0] = 0.113f, pfClearColor[1] = 0.172f, pfClearColor[2] = 0.419f, pfClearColor[3] = 1.0f; m_pVScene->NightShift(); }
	void DayShift() { pfClearColor[0] = 0.9f, pfClearColor[1] = 0.9f, pfClearColor[2] = 0.9f, pfClearColor[3] = 1.0f; m_pVScene->DayShift();}
	void RacingShift() { pfClearColor[0] = 0.125f, pfClearColor[1] = 0.125f, pfClearColor[2] = 0.3f, pfClearColor[3] = 1.0f; }

private:
	HINSTANCE					m_hInstance;
	HWND						m_hWnd;

	int							m_nWndClientWidth;
	int							m_nWndClientHeight;

	IDXGIFactory4				*m_pdxgiFactory;
	IDXGISwapChain3				*m_pdxgiSwapChain;
	ID3D12Device				*m_pd3dDevice;

	bool						m_bMsaa4xEnable = false;
	UINT						m_nMsaa4xQualityLevels = 0;

	static const UINT			m_nSwapChainBuffers = 2;
	UINT						m_nSwapChainBufferIndex;

	ID3D12Resource				*m_ppd3dSwapChainBackBuffers[m_nSwapChainBuffers];
	ID3D12DescriptorHeap		*m_pd3dRtvDescriptorHeap;
	UINT						m_nRtvDescriptorIncrementSize;

	ID3D12Resource				*m_pd3dDepthStencilBuffer;
	ID3D12DescriptorHeap		*m_pd3dDsvDescriptorHeap;
	UINT						m_nDsvDescriptorIncrementSize;

	ID3D12CommandAllocator		*m_pd3dCommandAllocator;
	ID3D12CommandQueue			*m_pd3dCommandQueue;
	ID3D12GraphicsCommandList	*m_pd3dCommandList;

	ID3D12Fence					*m_pd3dFence;
	UINT64						m_nFenceValues[m_nSwapChainBuffers];
	HANDLE						m_hFenceEvent;

#if defined(_DEBUG)
	ID3D12Debug					*m_pd3dDebugController;
#endif
	CScene_Village* m_pVScene = NULL;
	CPlayer_Village* m_pVPlayer = NULL;
	CCamera_Village* m_pVCamera = NULL;

	CScene_Racing* m_pRScene = NULL;
	CPlayer_Racing* m_pRPlayer = NULL;
	CCamera_Racing* m_pRCamera = NULL;

	int m_iSelectedScene;

	POINT						m_ptOldCursorPos;

	CGameTimer					m_GameTimer;
	_TCHAR						m_pszFrameRate[50];

	float pfClearColor[4];
};


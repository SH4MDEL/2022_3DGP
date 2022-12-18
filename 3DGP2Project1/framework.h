#pragma once
#include "stdafx.h"
#include "timer.h"
#include "camera.h"
#include "scene.h"
#include "object.h"

class GameFramework
{
public:
	GameFramework(UINT width, UINT height);
	~GameFramework();

	void OnCreate(HINSTANCE hInstance, HWND hWnd);
	void OnDestroy();
	void OnProcessingMouseMessage() const;
	void OnProcessingMouseMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) const;
	void OnProcessingKeyboardMessage() const;
	void StartPipeline();

	// 1. 디바이스 생성
	void CreateDevice();

	// 2. 펜스 객체 생성
	void CreateFence();

	// 3. 4X MSAA 품질 수준 지원 여부 점검
	void Check4xMSAAMultiSampleQuality();

	// 4. 명령 큐, 명령 할당자, 명령 리스트 생성
	void CreateCommandQueueAndList();

	// 5. 스왑 체인 생성
	void CreateSwapChain();

	// 6. 서술자 힙 생성
	void CreateRtvDsvDescriptorHeap();

	// 7. 후면 버퍼에 대한 렌더 타겟 뷰 생성
	void CreateRenderTargetView();

	// 8. 깊이 스텐실 버퍼, 깊이 스텐실 뷰 생성
	void CreateDepthStencilView();

	// 9. 루트 시그니처 생성
	void CreateRootSignature();


	void BuildObjects();

	void FrameAdvance();
	void Update(FLOAT timeElapsed);
	void Render();
	//void AnimateObjects();

	void WaitForGpuComplete();

	UINT GetWindowWidth() const { return m_width; }
	UINT GetWindowHeight() const { return m_height; }
	void SetIsActive(BOOL isActive) { m_isActive = isActive; }

private:
	static const INT					SwapChainBufferCount = 2;

	// Window
	HINSTANCE							m_hInstance;
	HWND								m_hWnd;
	UINT								m_width;
	UINT								m_height;
	FLOAT								m_aspectRatio;
	BOOL								m_isActive;

	D3D12_VIEWPORT						m_viewport;
	D3D12_RECT							m_scissorRect;
	ComPtr<IDXGIFactory4>				m_factory;
	ComPtr<IDXGISwapChain3>				m_swapChain;
	ComPtr<ID3D12Device>				m_device;
	INT									m_MSAA4xQualityLevel;
	ComPtr<ID3D12CommandAllocator>		m_commandAllocator;
	ComPtr<ID3D12CommandQueue>			m_commandQueue;
	ComPtr<ID3D12GraphicsCommandList>	m_commandList;
	ComPtr<ID3D12Resource>				m_renderTargets[SwapChainBufferCount];
	ComPtr<ID3D12DescriptorHeap>		m_rtvHeap;
	UINT								m_rtvDescriptorSize;
	ComPtr<ID3D12Resource>				m_depthStencil;
	ComPtr<ID3D12DescriptorHeap>		m_dsvHeap;
	ComPtr<ID3D12RootSignature>			m_rootSignature;


	ComPtr<ID3D12Fence>					m_fence;
	UINT								m_frameIndex;
	UINT64								m_fenceValue;
	HANDLE								m_fenceEvent;

	Timer								m_timer;

	unique_ptr<Scene>					m_scene;

};


#pragma once
#include "stdafx.h"
#include "timer.h"

class GameFramework
{
public:
	GameFramework(UINT width, UINT height);
	~GameFramework();

	void OnCreate(HINSTANCE hInstance, HWND hWnd);
	void OnDestroy();
	void StartPipeline();

	// 1. ����̽� ����
	void CreateDevice();

	// 2. �潺 ��ü ����
	void CreateFence();

	// 3. 4X MSAA ǰ�� ���� ���� ���� ����
	void Check4xMSAAMultiSampleQuality();

	// 4. ��� ť, ��� �Ҵ���, ��� ����Ʈ ����
	void CreateCommandQueueAndList();

	// 5. ���� ü�� ����
	void CreateSwapChain();

	// 6. ������ �� ����
	void CreateRtvDsvDescriptorHeap();

	// 7. �ĸ� ���ۿ� ���� ���� Ÿ�� �� ����
	void CreateRenderTargetView();

	// 8. ���� ���ٽ� ����, ���� ���ٽ� �� ����
	void CreateDepthStencilView();

	// 9. ��Ʈ �ñ״�ó ����
	void CreateRootSignature();

	//void BuildObjects();

	void FrameAdvance();
	void Update();
	void Render();
	//void AnimateObjects();

	void WaitForGpuComplete();

	UINT GetWindowWidth() const { return m_width; }
	UINT GetWindowHeight() const { return m_height; }

private:
	static const INT					SwapChainBufferCount = 2;

	// Window
	HINSTANCE							m_hInstance;
	HWND								m_hWnd;
	UINT								m_width;
	UINT								m_height;

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
	ComPtr<ID3D12PipelineState>			m_pipelineState;

	ComPtr<ID3D12Fence>					m_fence;
	UINT								m_frameIndex;
	UINT64								m_fenceValue;
	HANDLE								m_fenceEvent;

	Timer								m_timer;

};


#pragma once
#include "stdafx.h"
#include "DDSTextureLoader12.h"

class Texture
{
public:
	Texture() = default;
	~Texture() = default;

	void LoadTextureFile(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList, const wstring& fileName, UINT rootParameterIndex);
	bool LoadTextureFileHierarchy(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList, ifstream& in, UINT rootParameterIndex);
	void CreateTexture(const ComPtr<ID3D12Device>& device, DXGI_FORMAT format, UINT width, UINT height, UINT rootParameterIndex);

	void Copy(const ComPtr<ID3D12GraphicsCommandList>& commandList, const ComPtr<ID3D12Resource>& src, D3D12_RESOURCE_STATES srcState);

	void CreateSrvDescriptorHeap(const ComPtr<ID3D12Device>& device);
	void CreateShaderResourceView(const ComPtr<ID3D12Device>& device, INT viewDemention);
	void UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& commandList);
	void ReleaseUploadBuffer();

	ComPtr<ID3D12DescriptorHeap> GetSrvDescriptorHeap() const { return m_srvDescriptorHeap; }


private:
	UINT m_cbvsrvDescriptorSize;

	ComPtr<ID3D12DescriptorHeap>				m_srvDescriptorHeap;
	ComPtr<ID3D12DescriptorHeap>				m_dsvDescriptorHeap;
	vector<pair<ComPtr<ID3D12Resource>, UINT>>	m_textures;
	vector<ComPtr<ID3D12Resource>>				m_textureUploadHeap;

};
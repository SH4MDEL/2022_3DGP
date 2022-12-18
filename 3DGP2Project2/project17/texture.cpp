#include "texture.h"

void Texture::LoadTextureFile(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList, const wstring& fileName, UINT rootParameterIndex)
{
	m_cbvsrvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV);
	ComPtr<ID3D12Resource> textureBuffer;
	ComPtr<ID3D12Resource> textureUploadHeap;

	unique_ptr<uint8_t[]> ddsData;
	vector<D3D12_SUBRESOURCE_DATA> subresources;
	DDS_ALPHA_MODE ddsAlphaMode{ DDS_ALPHA_MODE_UNKNOWN };
	DX::ThrowIfFailed(DirectX::LoadDDSTextureFromFileEx(device.Get(), fileName.c_str(), 0,
		D3D12_RESOURCE_FLAG_NONE, DDS_LOADER_DEFAULT, &textureBuffer, ddsData, subresources, &ddsAlphaMode));

	UINT nSubresources{ (UINT)subresources.size() };
	const UINT64 nBytes{ GetRequiredIntermediateSize(textureBuffer.Get(), 0, nSubresources) };

	// Create the GPU upload buffer.
	DX::ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Buffer(nBytes),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		NULL,
		IID_PPV_ARGS(&textureUploadHeap)
	));

	// subresources에 있는 데이터를 textureBuffer로 복사
	UpdateSubresources(commandList.Get(), textureBuffer.Get(), textureUploadHeap.Get(), 0, 0, nSubresources, subresources.data());

	// 리소스 베리어 설정
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(textureBuffer.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));

	// 저장
	m_textures.push_back(make_pair(textureBuffer, rootParameterIndex));
	m_textureUploadHeap.push_back(textureUploadHeap);
}

bool Texture::LoadTextureFileHierarchy(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList, ifstream& in, UINT rootParameterIndex)
{
	BYTE strLength;
	in.read((char*)(&strLength), sizeof(BYTE));
	string strToken(strLength, '\0');
	in.read((&strToken[0]), sizeof(char) * strLength);

	if (strToken != "null") {
		wstring wstrToken = L"";
		wstrToken.assign(strToken.begin(), strToken.end());
		wstring strPath = L"Model/Textures/" + wstrToken + L".dds";
		if (strToken[0] != '@') {
			LoadTextureFile(device, commandList, strPath, rootParameterIndex);
		}
		else {
			strPath.erase(find(strPath.begin(), strPath.end(), '@'));
			LoadTextureFile(device, commandList, strPath, rootParameterIndex);
		}
		return true;
	}
	return false;
}

void Texture::CreateTexture(const ComPtr<ID3D12Device>& device, DXGI_FORMAT format, UINT width, UINT height, UINT rootParameterIndex)
{
	ComPtr<ID3D12Resource> textureBuffer;
	// Create the GPU upload buffer.
	DX::ThrowIfFailed(device->CreateCommittedResource(
		&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
		D3D12_HEAP_FLAG_NONE,
		&CD3DX12_RESOURCE_DESC::Tex2D(format, width, height, 1, 1, 1, 0, D3D12_RESOURCE_FLAG_NONE),
		D3D12_RESOURCE_STATE_GENERIC_READ,
		NULL,
		IID_PPV_ARGS(&textureBuffer)
	));
	m_textures.push_back(make_pair(textureBuffer, rootParameterIndex));
}

void Texture::Copy(const ComPtr<ID3D12GraphicsCommandList>& commandList, const ComPtr<ID3D12Resource>& src, D3D12_RESOURCE_STATES srcState)
{
	for (const auto& texture : m_textures) {
		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(texture.first.Get(), D3D12_RESOURCE_STATE_GENERIC_READ, D3D12_RESOURCE_STATE_COPY_DEST));
		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(src.Get(), srcState, D3D12_RESOURCE_STATE_COPY_SOURCE));
		commandList->CopyResource(texture.first.Get(), src.Get());
		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(texture.first.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_GENERIC_READ));
		commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(src.Get(), D3D12_RESOURCE_STATE_COPY_SOURCE, srcState));
	}
}

void Texture::CreateSrvDescriptorHeap(const ComPtr<ID3D12Device>& device)
{
	// SRV를 생성하기 위한 SRV 서술자 힙을 생성한다.
	D3D12_DESCRIPTOR_HEAP_DESC srvHeapDesc = {};
	srvHeapDesc.NumDescriptors = m_textures.size();
	srvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	srvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
	DX::ThrowIfFailed(device->CreateDescriptorHeap(
		&srvHeapDesc, IID_PPV_ARGS(&m_srvDescriptorHeap)));
}

void Texture::CreateShaderResourceView(const ComPtr<ID3D12Device>& device, INT viewDemention)
{
	// SRV를 생성한다.

	// 힙의 시작을 가리키는 포인터를 얻는다.
	CD3DX12_CPU_DESCRIPTOR_HANDLE hDesciptor(
		m_srvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());

	for (const auto& texture : m_textures) {
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		switch (viewDemention)
		{
		case D3D12_SRV_DIMENSION_TEXTURE2D:
			srvDesc.Format = texture.first->GetDesc().Format;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.MipLevels = texture.first->GetDesc().MipLevels;
			srvDesc.Texture2D.ResourceMinLODClamp = 0.0f;
			break;
		case D3D12_SRV_DIMENSION_TEXTURECUBE:
			srvDesc.Format = texture.first->GetDesc().Format;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURECUBE;
			srvDesc.TextureCube.MipLevels = 1;
			srvDesc.TextureCube.MostDetailedMip = 0;
			srvDesc.TextureCube.ResourceMinLODClamp = 0.0f;
			break;
		case D3D12_SRV_DIMENSION_BUFFER:
			srvDesc.Format = DXGI_FORMAT_X24_TYPELESS_G8_UINT;
			srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
			srvDesc.Texture2D.MostDetailedMip = 0;
			srvDesc.Texture2D.MipLevels = 1;
			srvDesc.Texture2D.PlaneSlice = 1;
			break;
		}
		device->CreateShaderResourceView(texture.first.Get(), &srvDesc, hDesciptor);
		
		hDesciptor.Offset(1, m_cbvsrvDescriptorSize);
	}
}

void Texture::UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& commandList)
{
	ID3D12DescriptorHeap* ppHeaps[] = { m_srvDescriptorHeap.Get() };
	commandList->SetDescriptorHeaps(_countof(ppHeaps), ppHeaps);

	D3D12_GPU_DESCRIPTOR_HANDLE hDesciptor{ m_srvDescriptorHeap->GetGPUDescriptorHandleForHeapStart() };
	for (const auto& index : m_textures)
	{
		commandList->SetGraphicsRootDescriptorTable(index.second, hDesciptor);
		hDesciptor.ptr += m_cbvsrvDescriptorSize;
	}
}

void Texture::ReleaseUploadBuffer()
{
	for (auto& textureUploadBuffer : m_textureUploadHeap)
		textureUploadBuffer.Reset();
}
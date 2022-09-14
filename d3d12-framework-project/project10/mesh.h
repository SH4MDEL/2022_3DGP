#pragma once
#include "stdafx.h"

class Mesh
{
public:
	Mesh(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList, const vector<Vertex>& vertices, const vector<UINT>& indices);
	~Mesh() = default;

	void Render(const ComPtr<ID3D12GraphicsCommandList>& m_commandList) const;
	void Render(const ComPtr<ID3D12GraphicsCommandList>& m_commandList, const D3D12_VERTEX_BUFFER_VIEW& instanceBufferView) const;
	void ReleaseUploadBuffer();

private:
	UINT						m_nVertices;
	ComPtr<ID3D12Resource>		m_vertexBuffer;
	ComPtr<ID3D12Resource>		m_vertexUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW	m_vertexBufferView;

	UINT						m_nIndices;
	ComPtr<ID3D12Resource>		m_indexBuffer;
	ComPtr<ID3D12Resource>		m_indexUploadBuffer;
	D3D12_INDEX_BUFFER_VIEW		m_indexBufferView;
};

class HeightMapImage
{
public:
	HeightMapImage();
	~HeightMapImage() = default;

	FLOAT GetHeight(FLOAT x, FLOAT z) const;	// (x, z) ��ġ�� �ȼ� ���� ����� ���� ���� ��ȯ
	XMFLOAT3 GetNormal(INT x, INT z) const;		// (x, z) ��ġ�� ���� ���� ��ȯ

	XMFLOAT3 GetScale() { return m_scale; }
	BYTE* GetPixels() { return m_pixels.get(); }
	INT GetWidth() { return m_width; }
	INT GetLength() { return m_length; }
private:
	unique_ptr<BYTE[]>			m_pixels;	// ���� �� �̹��� �ȼ�(8-��Ʈ)���� ������ �迭�̴�. �� �ȼ��� 0~255�� ���� ���´�.
	INT							m_width;	// ���� �� �̹����� ���ο� ���� ũ���̴�
	INT							m_length;
	XMFLOAT3					m_scale;	// ���� �� �̹����� ������ �� �� Ȯ���Ͽ� ����� ���ΰ��� ��Ÿ���� ������ �����̴�
};

class HeightMapGridMesh : public Mesh
{
public:
	HeightMapGridMesh(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList, const wstring& fileName,
		INT width, INT length, INT blockWidth, INT blockLength, XMFLOAT3 scale);
};
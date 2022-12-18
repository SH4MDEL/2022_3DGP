#pragma once
#include "Mesh.h"

class CMesh_Village : public CMesh
{
public:
	CMesh_Village(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, char* pstrFileName = NULL);
	virtual ~CMesh_Village();

public:	
	virtual void ReleaseUploadBuffers() override;

protected:
	UINT							m_nVertices = 0;

	XMFLOAT3* m_pxmf3Positions = NULL;
	ID3D12Resource* m_pd3dPositionBuffer = NULL;
	ID3D12Resource* m_pd3dPositionUploadBuffer = NULL;

	XMFLOAT3* m_pxmf3Normals = NULL;
	ID3D12Resource* m_pd3dNormalBuffer = NULL;
	ID3D12Resource* m_pd3dNormalUploadBuffer = NULL;

	UINT							m_nVertexBufferViews = 0;
	D3D12_VERTEX_BUFFER_VIEW* m_pd3dVertexBufferViews = NULL;

	UINT							m_nIndices = 0;
	UINT* m_pnIndices = NULL;

	UINT* m_pnSubSetIndices = NULL;
	UINT* m_pnSubSetStartIndices = NULL;
	UINT** m_ppnSubSetIndices = NULL;

	ID3D12Resource** m_ppd3dIndexBuffers = NULL;
	ID3D12Resource** m_ppd3dIndexUploadBuffers = NULL;

	D3D12_INDEX_BUFFER_VIEW* m_pd3dIndexBufferViews = NULL;

	D3D12_PRIMITIVE_TOPOLOGY		m_d3dPrimitiveTopology = D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST;
	UINT							m_nSlot = 0;
	UINT							m_nStride = 0;
	UINT							m_nOffset = 0;

	UINT							m_nStartIndex = 0;
	int								m_nBaseVertex = 0;

public:
	BoundingBox						m_xmBoundingBox;

	UINT							m_nSubsets = 0;

public:
	void OnPreRender(ID3D12GraphicsCommandList* pd3dCommandList);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, UINT nSubset);

	void LoadMeshFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, char* pstrFileName);
};



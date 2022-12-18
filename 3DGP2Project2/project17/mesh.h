#pragma once
#include "stdafx.h"
#include "texture.h"
#include "material.h"

struct Vertex
{
	Vertex(const XMFLOAT3& p, const XMFLOAT4& c) : position{ p }, color{ c } { }
	~Vertex() = default;

	XMFLOAT3 position;
	XMFLOAT4 color;
};

struct NormalVertex
{
	NormalVertex() : position{ XMFLOAT3{0.f, 0.f, 0.f} }, normal{ XMFLOAT3{0.f, 0.f, 0.f} } {}
	NormalVertex(const XMFLOAT3& p, const XMFLOAT3& n) : position{ p }, normal{ n } { }
	~NormalVertex() = default;

	XMFLOAT3 position;
	XMFLOAT3 normal;
};

struct TerrainVertex
{
	TerrainVertex(const XMFLOAT3& p, const XMFLOAT2& uv0, const XMFLOAT2& uv1) : position{ p }, uv0{ uv0 }, uv1{ uv1 } { }
	~TerrainVertex() = default;

	XMFLOAT3 position;
	XMFLOAT2 uv0;
	XMFLOAT2 uv1;
};

struct TextureVertex
{
	TextureVertex(const XMFLOAT3& position, const XMFLOAT2& uv) : position{ position }, uv{ uv } { }
	~TextureVertex() = default;

	XMFLOAT3 position;
	XMFLOAT2 uv;
};

struct SkyboxVertex
{
	SkyboxVertex(const XMFLOAT3& position) : position{ position } { }
	~SkyboxVertex() = default;

	XMFLOAT3 position;
};

struct TextureHierarchyVertex
{
	TextureHierarchyVertex() : position{ XMFLOAT3{0.f, 0.f, 0.f} }, normal{ XMFLOAT3{0.f, 0.f, 0.f} },
		tangent{ XMFLOAT3{0.f, 0.f, 0.f} }, biTangent{ XMFLOAT3{0.f, 0.f, 0.f} }, uv0{ XMFLOAT2{0.f, 0.f} }{}
	TextureHierarchyVertex(const XMFLOAT3& p, const XMFLOAT3& n, const XMFLOAT3& t, const XMFLOAT3& bt, const XMFLOAT2& uv0) : 
		position{ p }, normal{ n }, tangent{ t }, biTangent{ bt }, uv0{ uv0 } { }
	~TextureHierarchyVertex() = default;

	XMFLOAT3 position;
	XMFLOAT3 normal;
	XMFLOAT3 tangent;
	XMFLOAT3 biTangent;
	XMFLOAT2 uv0;
};

struct ParticleVertex
{
	ParticleVertex() : position{ XMFLOAT3{0.f, 0.f, 0.f} }, velocity{ XMFLOAT3{0.f, 0.f, 0.f} },
		age{ 0.f }, lifeTime{ 0.f } {}
	ParticleVertex(const XMFLOAT3& p, const XMFLOAT3& v, const FLOAT& a, const FLOAT& l) : 
		position{ p }, velocity{ v }, age{ a }, lifeTime{ l } { }
	~ParticleVertex() = default;

	XMFLOAT3 position;
	XMFLOAT3 velocity;
	FLOAT age;
	FLOAT lifeTime;

};

class Mesh
{
public:
	Mesh() = default;
	Mesh(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList, const vector<Vertex>& vertices, const vector<UINT>& indices);
	~Mesh() = default;

	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& m_commandList) const;
	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& m_commandList, const D3D12_VERTEX_BUFFER_VIEW& instanceBufferView) const;
	virtual void ReleaseUploadBuffer();

	void SetBoundingBox(XMFLOAT3 center, XMFLOAT3 extents) { m_boundingBox.Center = center; m_boundingBox.Extents = extents; }
	BoundingOrientedBox GetBoundingBox() { return m_boundingBox; }

protected:
	UINT						m_nVertices;
	ComPtr<ID3D12Resource>		m_vertexBuffer;
	ComPtr<ID3D12Resource>		m_vertexUploadBuffer;
	D3D12_VERTEX_BUFFER_VIEW	m_vertexBufferView;

	UINT						m_nIndices;
	ComPtr<ID3D12Resource>		m_indexBuffer;
	ComPtr<ID3D12Resource>		m_indexUploadBuffer;
	D3D12_INDEX_BUFFER_VIEW		m_indexBufferView;

	D3D12_PRIMITIVE_TOPOLOGY	m_primitiveTopology;
	BoundingOrientedBox			m_boundingBox;
};

class MeshFromFile : public Mesh
{
public:
	MeshFromFile();
	~MeshFromFile() = default;

	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& m_commandList) const;

	void ReleaseUploadBuffer() override;

	void LoadMesh(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList, ifstream& in);
	void LoadMaterial(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList, ifstream& in);
	void SetMaterials(const shared_ptr<vector<Material>> materials) { m_materials = materials; }

private:
	string								m_meshName;

	UINT								m_nSubMeshes;
	vector<UINT>						m_vSubsetIndices;
	vector<vector<UINT>>				m_vvSubsetIndices;

	vector<ComPtr<ID3D12Resource>>		m_subsetIndexBuffers;
	vector<ComPtr<ID3D12Resource>>		m_subsetIndexUploadBuffers;
	vector<D3D12_INDEX_BUFFER_VIEW>		m_subsetIndexBufferViews;

	shared_ptr<vector<Material>>		m_materials;
};

class HeightMapImage
{
public:
	HeightMapImage(const wstring& fileName, INT width, INT length, XMFLOAT3 scale);
	~HeightMapImage() = default;

	FLOAT GetHeight(FLOAT x, FLOAT z) const;	// (x, z) 위치의 픽셀 값에 기반한 지형 높이 반환
	XMFLOAT3 GetNormal(INT x, INT z) const;		// (x, z) 위치의 법선 벡터 반환

	XMFLOAT3 GetScale() { return m_scale; }
	BYTE* GetPixels() { return m_pixels.get(); }
	INT GetWidth() { return m_width; }
	INT GetLength() { return m_length; }
private:
	unique_ptr<BYTE[]>			m_pixels;	// 높이 맵 이미지 픽셀(8-비트)들의 이차원 배열이다. 각 픽셀은 0~255의 값을 갖는다.
	INT							m_width;	// 높이 맵 이미지의 가로와 세로 크기이다
	INT							m_length;
	XMFLOAT3					m_scale;	// 높이 맵 이미지를 실제로 몇 배 확대하여 사용할 것인가를 나타내는 스케일 벡터이다
};

class HeightMapGridMesh : public Mesh
{
public:
	HeightMapGridMesh(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList,
		INT xStart, INT zStart, INT width, INT length, XMFLOAT3 scale, HeightMapImage* heightMapImage);
	virtual ~HeightMapGridMesh() = default;

	//virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, int nSubSet);

	XMFLOAT3 GetScale() { return(m_scale); }
	INT GetWidth() { return(m_width); }
	INT GetLength() { return(m_length); }

protected:
	//격자의 크기(가로: x-방향, 세로: z-방향)이다. 
	INT m_width;
	INT m_length;

	//격자의 스케일(가로: x-방향, 세로: z-방향, 높이: y-방향) 벡터이다. 
	//실제 격자 메쉬의 각 정점의 x-좌표, y-좌표, z-좌표는 스케일 벡터의 x-좌표, y-좌표, z-좌표로 곱한 값을 갖는다. 
	//즉, 실제 격자의 x-축 방향의 간격은 1이 아니라 스케일 벡터의 x-좌표가 된다. 
	//이렇게 하면 작은 격자(적은 정점)를 사용하더라도 큰 크기의 격자(지형)를 생성할 수 있다.
	XMFLOAT3 m_scale;
};

class TextureRectMesh : public Mesh
{
public:
	TextureRectMesh(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList,
		XMFLOAT3 position, FLOAT width=20.0f, FLOAT height=20.0f, FLOAT depth=20.0f);
	~TextureRectMesh() = default;
};

class SkyboxMesh : public Mesh
{
public:
	SkyboxMesh(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList, 
		FLOAT width = 20.0f, FLOAT height = 20.0f, FLOAT depth = 20.0f);
	~SkyboxMesh() = default;
};

class BillBoardMesh : public Mesh
{
public:
	BillBoardMesh(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList,
		XMFLOAT3 position, XMFLOAT2 size);
	~BillBoardMesh() = default;
};


#define MAX_PARTICLE_COUNT 100
class ParticleMesh : public Mesh
{
public:
	ParticleMesh(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList);
	~ParticleMesh() = default;

	void CreateStreamOutputBuffer(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList);

	void RenderStreamOutput(const ComPtr<ID3D12GraphicsCommandList>& commandList);
	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& commandList);

	virtual void ReleaseUploadBuffer() override;

private:
	ComPtr<ID3D12Resource>				m_streamOutputBuffer;
	D3D12_STREAM_OUTPUT_BUFFER_VIEW		m_streamOutputBufferView;

	ComPtr<ID3D12Resource>				m_drawBuffer;

	ComPtr<ID3D12Resource>				m_filledSizeBuffer;
	ComPtr<ID3D12Resource>				m_filledSizeUploadBuffer;
	ComPtr<ID3D12Resource>				m_filledSizeReadbackBuffer;

	UINT*								m_filledSizeUploadBufferSize;
	UINT*								m_filledSizeReadbackBufferSize;

	BOOL								m_isBinding;
};

class UIMesh : public Mesh
{
public:
	UIMesh(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12GraphicsCommandList>& commandList);
	~UIMesh() = default;
};
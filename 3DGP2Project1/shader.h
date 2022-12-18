#pragma once
#include "stdafx.h"
#include "object.h"
#include "player.h"
#include "camera.h"

class Shader
{
public:
	Shader() {};
	Shader(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12RootSignature>& rootSignature);
	~Shader();

	virtual void UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& commandList) const;
	virtual void ReleaseShaderVariable();

	virtual void ReleaseUploadBuffer() const;

	virtual void Update(FLOAT timeElapsed);
	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) const;

	shared_ptr<Player> GetPlayer() const { return m_player; }
	shared_ptr<Camera> GetCamera() const { return m_camera; }
	vector<shared_ptr<GameObject>>& GetGameObjects() { return m_gameObjects; }
	ComPtr<ID3D12PipelineState> GetPipelineState() const { return m_pipelineState; }

	void SetPlayer(const shared_ptr<Player>& player);
	void SetCamera(const shared_ptr<Camera>& camera);

protected:
	ComPtr<ID3D12PipelineState>			m_pipelineState;
	vector<D3D12_INPUT_ELEMENT_DESC>	m_inputLayout;

	vector<shared_ptr<GameObject>>		m_gameObjects;

	shared_ptr<Player>					m_player;
	shared_ptr<Camera>					m_camera;
};

class TerrainShader : public Shader
{
public:
	TerrainShader(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12RootSignature>& rootSignature);
	~TerrainShader() = default;

	void ReleaseUploadBuffer() const override;

	virtual void CreatePipelineState(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12RootSignature>& rootSignature);

	virtual void Update(FLOAT timeElapsed);
	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) const;

	void SetTerrain(const shared_ptr<HeightMapTerrain>& heightMap) { m_heightMap = heightMap; }

private:
	shared_ptr<HeightMapTerrain>		m_heightMap;
};


struct InstancingData
{
	XMFLOAT4X4 worldMatrix;
};

class InstancingShader : public Shader 
{
public:
	InstancingShader() {};
	InstancingShader(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12RootSignature>& rootSignature, const Mesh& mesh, UINT count);
	~InstancingShader() = default;

	virtual void Update(FLOAT timeElapsed);
	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) const;

	virtual void CreateShaderVariable(const ComPtr<ID3D12Device>& device);
	virtual void UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& commandList) const override;
	virtual void ReleaseShaderVariable();

protected:
	unique_ptr<Mesh>					m_mesh;

	ComPtr<ID3D12Resource>				m_instancingBuffer;
	InstancingData*						m_instancingBufferPointer;
	D3D12_VERTEX_BUFFER_VIEW			m_instancingBufferView;

	UINT								m_instancingCount;
};

class TextureHierarchyShader : public Shader
{
public:
	TextureHierarchyShader(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12RootSignature>& rootSignature);
	~TextureHierarchyShader() = default;
};

class SkyboxShader : public Shader
{
public:
	SkyboxShader(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12RootSignature>& rootSignature);
	~SkyboxShader() = default;
};

class BlendingShader : public Shader
{
public:
	BlendingShader(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12RootSignature>& rootSignature);
	~BlendingShader() = default;
};

struct BillBoardInstancingData
{
	XMFLOAT3 position;
};

class BillBoardShader : public Shader
{
public:
	BillBoardShader(const ComPtr<ID3D12Device>& device, const ComPtr<ID3D12RootSignature>& rootSignature, UINT count);
	~BillBoardShader() = default;

	virtual void Update(FLOAT timeElapsed);
	virtual void Render(const ComPtr<ID3D12GraphicsCommandList>& commandList) const;

	virtual void CreateShaderVariable(const ComPtr<ID3D12Device>& device);
	virtual void UpdateShaderVariable(const ComPtr<ID3D12GraphicsCommandList>& commandList) const override;
	virtual void ReleaseShaderVariable();

	void SetMesh(const shared_ptr<Mesh>& mesh) { m_mesh = mesh; }
	void SetTexture(const shared_ptr<Texture>& texture) { m_texture = texture; }
	void SetMaterial(const shared_ptr<Material>& material) { m_material = material; }

protected:
	shared_ptr<Mesh>					m_mesh;
	shared_ptr<Texture>					m_texture;		// 텍스처
	shared_ptr<Material>				m_material;		// 재질

	ComPtr<ID3D12Resource>				m_instancingBuffer;
	BillBoardInstancingData*			m_instancingBufferPointer;
	D3D12_VERTEX_BUFFER_VIEW			m_instancingBufferView;

	UINT								m_instancingCount;
};
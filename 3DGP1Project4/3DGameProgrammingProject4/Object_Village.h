#pragma once
#include "Object.h"
#include "Mesh_Village.h"

#define RESOURCE_TEXTURE2D			0x01
#define RESOURCE_TEXTURE2D_ARRAY	0x02	//[]
#define RESOURCE_TEXTURE2DARRAY		0x03
#define RESOURCE_TEXTURE_CUBE		0x04
#define RESOURCE_BUFFER				0x05

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
class CShader_Village;

struct SRVROOTARGUMENTINFO
{
	int								m_nRootParameterIndex = 0;
	D3D12_GPU_DESCRIPTOR_HANDLE		m_d3dSrvGpuDescriptorHandle;
};

class CTexture
{
public:
	CTexture(int nTextureResources = 1, UINT nResourceType = RESOURCE_TEXTURE2D, int nSamplers = 0);
	virtual ~CTexture();

private:
	int								m_nReferences = 0;

	UINT							m_nTextureType = RESOURCE_TEXTURE2D;

	int								m_nTextures = 0;
	ID3D12Resource** m_ppd3dTextures = NULL;
	ID3D12Resource** m_ppd3dTextureUploadBuffers;

	int								m_nSamplers = 0;
	D3D12_GPU_DESCRIPTOR_HANDLE* m_pd3dSamplerGpuDescriptorHandles = NULL;

public:
	SRVROOTARGUMENTINFO* m_pRootArgumentInfos = NULL;

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

	void SetRootArgument(int nIndex, UINT nRootParameterIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dsrvGpuDescriptorHandle);
	void SetSampler(int nIndex, D3D12_GPU_DESCRIPTOR_HANDLE d3dSamplerGpuDescriptorHandle);

	void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	void UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, int nIndex);
	void ReleaseShaderVariables();

	void LoadTextureFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, wchar_t* pszFileName, UINT nIndex);

	int GetTextures() { return(m_nTextures); }
	ID3D12Resource* GetTexture(int nIndex) { return(m_ppd3dTextures[nIndex]); }
	UINT GetTextureType() { return(m_nTextureType); }

	void ReleaseUploadBuffers();
};

class CMaterial_Village
{
public:
	CMaterial_Village();
	virtual ~CMaterial_Village();

private:
	int								m_nReferences = 0;

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

	XMFLOAT4						m_xmf4AlbedoColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	CTexture* m_pAlbedoTexture = NULL;

	XMFLOAT4						m_xmf4EmissionColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	CTexture* m_pEmissionTexture = NULL;

	int								m_nMaterial = 1; //Material Index, CScene::m_pReflections[]

	void SetAlbedoColor(XMFLOAT4 xmf4Color) { m_xmf4AlbedoColor = xmf4Color; }
	void SetEmissionColor(XMFLOAT4 xmf4Color) { m_xmf4EmissionColor = xmf4Color; }
	void SetMaterial(int nMaterial) { m_nMaterial = nMaterial; }
	void SetAlbedoTexture(CTexture* pTexture);
	void SetEmissionTexture(CTexture* pTexture);

	void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList);
	void ReleaseShaderVariables();

	void ReleaseUploadBuffers();
};


class CGameObject_Village : public CGameObject
{
public:
	CGameObject_Village(UINT nMaterials = 1);
	virtual ~CGameObject_Village();

public:
	char							m_pstrName[64] = { '\0' };


	CMesh_Village*					m_pMesh = NULL;
	CShader_Village*				m_pShader = NULL;

	UINT							m_nMaterials = 0;
	CMaterial_Village**				m_ppMaterials = NULL;

	BoundingOrientedBox				m_xmOOBB = BoundingOrientedBox();

protected:
	ID3D12Resource* m_pd3dcbGameObject = NULL;

public:
	CB_GAMEOBJECT_INFO* m_pcbMappedGameObject = NULL;

public:
	void SetBoundingBox();

	void SetMesh(CMesh_Village* pMesh);
	void SetShader(CShader_Village* pShader);
	void SetAlbedoColor(UINT nIndex, XMFLOAT4 xmf4Color);
	void SetEmissionColor(UINT nIndex, XMFLOAT4 xmf4Color);
	void SetMaterial(UINT nIndex, CMaterial_Village* pMaterial);
	void SetMaterial(UINT nIndex, UINT nMaterial);

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) override;
	virtual void ReleaseShaderVariables() override;
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList) override;

	virtual void Animate(float fTimeElapsed);
	virtual void OnPrepareRender() override {}
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL) override;

	void UpdateBoundingBox();

	virtual void ReleaseUploadBuffers() override;

	XMFLOAT3 GetPosition() override;
	XMFLOAT3 GetLook() override;
	XMFLOAT3 GetUp() override;
	XMFLOAT3 GetRight() override;

	void SetPosition(float x, float y, float z) override;
	void SetPosition(XMFLOAT3 xmf3Position) override;
	void SetScale(float x, float y, float z);

	virtual void MoveStrafe(float fDistance = 1.0f);
	virtual void MoveUp(float fDistance = 1.0f);
	virtual void MoveForward(float fDistance = 1.0f);

	virtual void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f);
	virtual void Rotate(XMFLOAT3* pxmf3Axis, float fAngle);

	void LoadGameObjectFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, const char* pstrFileName);
};



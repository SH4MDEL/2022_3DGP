#pragma once
#include "Object.h"
#include "Mesh_Racing.h"

#define MATERIAL_ALBEDO_MAP			0x01
#define MATERIAL_SPECULAR_MAP		0x02
#define MATERIAL_NORMAL_MAP			0x04
#define MATERIAL_METALLIC_MAP		0x08
#define MATERIAL_EMISSION_MAP		0x10
#define MATERIAL_DETAIL_ALBEDO_MAP	0x20
#define MATERIAL_DETAIL_NORMAL_MAP	0x40

class CShader_Racing;

struct MATERIALLOADINFO
{
	XMFLOAT4						m_xmf4AlbedoColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	XMFLOAT4						m_xmf4EmissiveColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);
	XMFLOAT4						m_xmf4SpecularColor = XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f);

	float							m_fGlossiness = 0.0f;
	float							m_fSmoothness = 0.0f;
	float							m_fSpecularHighlight = 0.0f;
	float							m_fMetallic = 0.0f;
	float							m_fGlossyReflection = 0.0f;

	UINT							m_nType = 0x00;

	TCHAR							m_pstrAlbedoMapName[128] = { '\0' };
	TCHAR							m_pstrSpecularMapName[128] = { '\0' };
	TCHAR							m_pstrMetallicMapName[128] = { '\0' };
	TCHAR							m_pstrNormalMapName[128] = { '\0' };
	TCHAR							m_pstrEmissionMapName[128] = { '\0' };
	TCHAR							m_pstrDetailAlbedoMapName[128] = { '\0' };
	TCHAR							m_pstrDetailNormalMapName[128] = { '\0' };
};

struct MATERIALSLOADINFO
{
	int								m_nMaterials = 0;
	MATERIALLOADINFO* m_pMaterials = NULL;
};

class CMaterial_Racing
{
public:
	CMaterial_Racing();
	virtual ~CMaterial_Racing();

private:
	int								m_nReferences = 0;

public:
	void AddRef() { m_nReferences++; }
	void Release() { if (--m_nReferences <= 0) delete this; }

	CShader_Racing* m_pShader = NULL;

	void SetShader(CShader_Racing* pShader);
	void SetPseudoLightingShader() { SetShader(m_pPseudoLightingShader); }

	void UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList);

protected:
	static CShader_Racing* m_pPseudoLightingShader;

public:
	static void PrepareShaders(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature);
};


class CGameObject_Racing : CGameObject
{
public:
	CGameObject_Racing();
	virtual ~CGameObject_Racing();

private:
	int								m_nReferences = 0;

public:
	void AddRef();
	void Release();

public:
	TCHAR							m_pstrFrameName[256];

	CMesh_Racing* m_pMesh = NULL;

	int								m_nMaterials = 0;
	CMaterial_Racing** m_ppMaterials = NULL;

	XMFLOAT4X4						m_xmf4x4Transform;

	BoundingOrientedBox				m_xmMovedOOBB = BoundingOrientedBox();
	BoundingOrientedBox				m_xmOOBB = BoundingOrientedBox();

	CGameObject_Racing* m_pParent = NULL;
	CGameObject_Racing* m_pChild = NULL;
	CGameObject_Racing* m_pSibling = NULL;

	bool							m_bBreakable = false;
	bool							m_bBlowingUp = false;

	void SetMesh(CMesh_Racing* pMesh);
	void SetShader(CShader_Racing* pShader);
	void SetShader(int nMaterial, CShader_Racing* pShader);
	void SetMaterial(int nMaterial, CMaterial_Racing* pMaterial);

	void SetChild(CGameObject_Racing* pChild, bool bReferenceUpdate = false);

	virtual void BuildMaterials(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) { }

	virtual void OnInitialize() { }
	virtual void Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent = NULL);
	virtual void OnPrepareRender() { }
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);
	void Render(ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT4X4* pxmf4x4World, CDiffusedMesh* pMesh);

	virtual void CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList) override;
	virtual void UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList) override;
	virtual void ReleaseShaderVariables() override;

	virtual void UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT4X4* pxmf4x4World);
	virtual void UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, CMaterial_Racing* pMaterial);

	virtual void ReleaseUploadBuffers() override;

	virtual XMFLOAT3 GetPosition() override;
	virtual XMFLOAT3 GetLook() override;
	virtual XMFLOAT3 GetUp() override;
	virtual XMFLOAT3 GetRight() override;

	void SetPosition(float x, float y, float z) override;
	void SetPosition(XMFLOAT3 xmf3Position) override;
	void SetScale(float x, float y, float z);

	void MoveStrafe(float fDistance = 1.0f) override;
	void MoveUp(float fDistance = 1.0f) override;
	void MoveForward(float fDistance = 1.0f) override;

	void Rotate(float fPitch = 10.0f, float fYaw = 10.0f, float fRoll = 10.0f) override;
	void Rotate(XMFLOAT3* pxmf3Axis, float fAngle) override;
	void Rotate(XMFLOAT4* pxmf4Quaternion);

	CGameObject_Racing* GetParent() { return(m_pParent); }
	void UpdateTransform(XMFLOAT4X4* pxmf4x4Parent = NULL);
	CGameObject_Racing* FindFrame(const _TCHAR* pstrFrameName);

	UINT GetMeshType() { return((m_pMesh) ? m_pMesh->GetType() : 0); }

	void UpdateBoundingBox();

public:
	static MATERIALSLOADINFO* LoadMaterialsInfoFromFile(wifstream& InFile);
	static CMeshLoadInfo* LoadMeshInfoFromFile(wifstream& InFile);

	static CGameObject_Racing* LoadFrameHierarchyFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, wifstream& InFile);
	static CGameObject_Racing* LoadGeometryFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, const TCHAR* pstrFileName);

	static void PrintFrameInfo(CGameObject_Racing* pGameObject, CGameObject_Racing* pParent);
};

class CRotatingObject : public CGameObject_Racing
{
public:
	CRotatingObject();
	virtual ~CRotatingObject();

private:
	XMFLOAT3					m_xmf3RotationAxis;
	float						m_fRotationSpeed;

public:
	void SetRotationSpeed(float fRotationSpeed) { m_fRotationSpeed = fRotationSpeed; }
	void SetRotationAxis(XMFLOAT3 xmf3RotationAxis) { m_xmf3RotationAxis = xmf3RotationAxis; }

	virtual void Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent = NULL);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);
};

class CRevolvingObject : public CGameObject_Racing
{
public:
	CRevolvingObject();
	virtual ~CRevolvingObject();

private:
	XMFLOAT3					m_xmf3RevolutionAxis;
	float						m_fRevolutionSpeed;

public:
	void SetRevolutionSpeed(float fRevolutionSpeed) { m_fRevolutionSpeed = fRevolutionSpeed; }
	void SetRevolutionAxis(XMFLOAT3 xmf3RevolutionAxis) { m_xmf3RevolutionAxis = xmf3RevolutionAxis; }

	virtual void Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent = NULL);
};

class CCactusObject : public CGameObject_Racing
{
public:
	CCactusObject();
	virtual ~CCactusObject();

	XMFLOAT4X4					m_pxmf4x4Transforms[EXPLOSION_DEBRISES];

	float							m_fElapsedTimes = 0.0f;
	float							m_fDuration = 2.0f;
	float						m_fExplosionSpeed = 10.0f;
	float						m_fExplosionRotation = 360.0f;

public:
	virtual void OnInitialize();
	virtual void Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent = NULL);
	virtual void Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera = NULL);

	static CDiffusedMesh* m_pExplosionMesh;
	static XMFLOAT3				m_pxmf3SphereVectors[EXPLOSION_DEBRISES];

	static void PrepareExplosion(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList);
};

class CTreeObject : public CGameObject_Racing
{
public:
	CTreeObject();
	virtual ~CTreeObject();


public:
	virtual void OnInitialize();
	virtual void Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent = NULL);
};

class CRock1Object : public CGameObject_Racing
{
public:
	CRock1Object();
	virtual ~CRock1Object();


public:
	virtual void OnInitialize();
	virtual void Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent = NULL);
};

class CRock2Object : public CGameObject_Racing
{
public:
	CRock2Object();
	virtual ~CRock2Object();


public:
	virtual void OnInitialize();
	virtual void Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent = NULL);
};



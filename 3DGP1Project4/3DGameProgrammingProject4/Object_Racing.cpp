#include "stdafx.h"
#include "Object_Racing.h"
#include "Shader_Racing.h"

inline float RandF(float fMin, float fMax)
{
	return(fMin + ((float)rand() / (float)RAND_MAX) * (fMax - fMin));
}

XMVECTOR RandomUnitVectorOnSphere()
{
	XMVECTOR xmvOne = XMVectorSet(1.0f, 1.0f, 1.0f, 1.0f);
	XMVECTOR xmvZero = XMVectorZero();

	while (true)
	{
		XMVECTOR v = XMVectorSet(RandF(-1.0f, 1.0f), RandF(-1.0f, 1.0f), RandF(-1.0f, 1.0f), 0.0f);
		if (!XMVector3Greater(XMVector3LengthSq(v), xmvOne)) return(XMVector3Normalize(v));
	}
}


////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CShader_Racing* CMaterial_Racing::m_pPseudoLightingShader = NULL;

CMaterial_Racing::CMaterial_Racing()
{
}

CMaterial_Racing::~CMaterial_Racing()
{
	if (m_pShader) m_pShader->Release();
}

void CMaterial_Racing::SetShader(CShader_Racing* pShader)
{
	if (m_pShader) m_pShader->Release();
	m_pShader = pShader;
	if (m_pShader) m_pShader->AddRef();
}

void CMaterial_Racing::UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList)
{
}

void CMaterial_Racing::PrepareShaders(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature)
{
	m_pPseudoLightingShader = new CPseudoLightingShader();
	m_pPseudoLightingShader->CreateShader(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature);
	m_pPseudoLightingShader->CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CGameObject_Racing::CGameObject_Racing()
{
	m_xmf4x4Transform = Matrix4x4::Identity();
	m_xmf4x4World = Matrix4x4::Identity();
}

CGameObject_Racing::~CGameObject_Racing()
{
	if (m_pMesh) m_pMesh->Release();

	if (m_nMaterials > 0)
	{
		for (int i = 0; i < m_nMaterials; i++)
		{
			if (m_ppMaterials[i]) m_ppMaterials[i]->Release();
		}
	}
	if (m_ppMaterials) delete[] m_ppMaterials;
}

void CGameObject_Racing::AddRef()
{
	m_nReferences++;

	if (m_pSibling) m_pSibling->AddRef();
	if (m_pChild) m_pChild->AddRef();
}

void CGameObject_Racing::Release()
{
	if (m_pChild) m_pChild->Release();
	if (m_pSibling) m_pSibling->Release();

	if (--m_nReferences <= 0) delete this;
}

void CGameObject_Racing::SetChild(CGameObject_Racing* pChild, bool bReferenceUpdate)
{
	if (pChild)
	{
		pChild->m_pParent = this;
		if (bReferenceUpdate) pChild->AddRef();
	}
	if (m_pChild)
	{
		if (pChild) pChild->m_pSibling = m_pChild->m_pSibling;
		m_pChild->m_pSibling = pChild;
	}
	else
	{
		m_pChild = pChild;
	}
}

void CGameObject_Racing::SetMesh(CMesh_Racing* pMesh)
{
	if (m_pMesh) m_pMesh->Release();
	m_pMesh = pMesh;
	if (m_pMesh) m_pMesh->AddRef();
}

void CGameObject_Racing::SetShader(CShader_Racing* pShader)
{
	m_nMaterials = 1;
	m_ppMaterials = new CMaterial_Racing * [m_nMaterials];
	m_ppMaterials[0] = new CMaterial_Racing();
	m_ppMaterials[0]->SetShader(pShader);
}

void CGameObject_Racing::SetShader(int nMaterial, CShader_Racing* pShader)
{
	if (m_ppMaterials[nMaterial]) m_ppMaterials[nMaterial]->SetShader(pShader);
}

void CGameObject_Racing::SetMaterial(int nMaterial, CMaterial_Racing* pMaterial)
{
	if (m_ppMaterials[nMaterial]) m_ppMaterials[nMaterial]->Release();
	m_ppMaterials[nMaterial] = pMaterial;
	if (m_ppMaterials[nMaterial]) m_ppMaterials[nMaterial]->AddRef();
}

void CGameObject_Racing::UpdateBoundingBox()
{
	m_xmOOBB.Transform(m_xmMovedOOBB, XMLoadFloat4x4(&m_xmf4x4World));
	XMStoreFloat4(&m_xmMovedOOBB.Orientation, XMQuaternionNormalize(XMLoadFloat4(&m_xmMovedOOBB.Orientation)));
}

void CGameObject_Racing::Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent)
{
	if (m_pSibling) m_pSibling->Animate(fTimeElapsed, pxmf4x4Parent);
	if (m_pChild) m_pChild->Animate(fTimeElapsed, &m_xmf4x4World);

}

CGameObject_Racing* CGameObject_Racing::FindFrame(const _TCHAR* pstrFrameName)
{
	CGameObject_Racing* pFrameObject = NULL;
	if (!_tcsncmp(m_pstrFrameName, pstrFrameName, _tcslen(pstrFrameName))) return(this);

	if (m_pSibling) if (pFrameObject = m_pSibling->FindFrame(pstrFrameName)) return(pFrameObject);
	if (m_pChild) if (pFrameObject = m_pChild->FindFrame(pstrFrameName)) return(pFrameObject);

	return(NULL);
}

void CGameObject_Racing::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	OnPrepareRender();

	UpdateShaderVariable(pd3dCommandList, &m_xmf4x4World);

	if (m_nMaterials > 0)
	{
		for (int i = 0; i < m_nMaterials; i++)
		{
			if (m_ppMaterials[i])
			{
				if (m_ppMaterials[i]->m_pShader) m_ppMaterials[i]->m_pShader->Render(pd3dCommandList, pCamera);
				m_ppMaterials[i]->UpdateShaderVariable(pd3dCommandList);
			}

			if (m_pMesh) m_pMesh->Render(pd3dCommandList, i);
		}
	}
	if (m_pSibling) m_pSibling->Render(pd3dCommandList, pCamera);
	if (m_pChild) m_pChild->Render(pd3dCommandList, pCamera);
}

void CGameObject_Racing::Render(ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT4X4* pxmf4x4World, CDiffusedMesh* pMesh)
{
	if (pMesh)
	{
		UpdateShaderVariable(pd3dCommandList, pxmf4x4World);
		pMesh->Render(pd3dCommandList);
	}
}

void CGameObject_Racing::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
}

void CGameObject_Racing::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
}

void CGameObject_Racing::UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, XMFLOAT4X4* pxmf4x4World)
{
	XMFLOAT4X4 xmf4x4World;
	XMStoreFloat4x4(&xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(pxmf4x4World)));
	pd3dCommandList->SetGraphicsRoot32BitConstants(1, 16, &xmf4x4World, 0);
}

void CGameObject_Racing::UpdateShaderVariable(ID3D12GraphicsCommandList* pd3dCommandList, CMaterial_Racing* pMaterial)
{
}

void CGameObject_Racing::ReleaseShaderVariables()
{
}

void CGameObject_Racing::ReleaseUploadBuffers()
{
	if (m_pMesh) m_pMesh->ReleaseUploadBuffers();

	if (m_pSibling) m_pSibling->ReleaseUploadBuffers();
	if (m_pChild) m_pChild->ReleaseUploadBuffers();
}

void CGameObject_Racing::UpdateTransform(XMFLOAT4X4* pxmf4x4Parent)
{
	m_xmf4x4World = (pxmf4x4Parent) ? Matrix4x4::Multiply(m_xmf4x4Transform, *pxmf4x4Parent) : m_xmf4x4Transform;

	if (m_pSibling) m_pSibling->UpdateTransform(pxmf4x4Parent);
	if (m_pChild) m_pChild->UpdateTransform(&m_xmf4x4World);
}

void CGameObject_Racing::SetPosition(float x, float y, float z)
{
	m_xmf4x4Transform._41 = x;
	m_xmf4x4Transform._42 = y;
	m_xmf4x4Transform._43 = z;

	UpdateTransform(NULL);
}

void CGameObject_Racing::SetPosition(XMFLOAT3 xmf3Position)
{
	SetPosition(xmf3Position.x, xmf3Position.y, xmf3Position.z);
}

void CGameObject_Racing::SetScale(float x, float y, float z)
{
	XMMATRIX mtxScale = XMMatrixScaling(x, y, z);
	m_xmf4x4Transform = Matrix4x4::Multiply(mtxScale, m_xmf4x4Transform);

	UpdateTransform(NULL);
}

XMFLOAT3 CGameObject_Racing::GetPosition()
{
	return(XMFLOAT3(m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43));
}

XMFLOAT3 CGameObject_Racing::GetLook()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._31, m_xmf4x4World._32, m_xmf4x4World._33)));
}

XMFLOAT3 CGameObject_Racing::GetUp()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._21, m_xmf4x4World._22, m_xmf4x4World._23)));
}

XMFLOAT3 CGameObject_Racing::GetRight()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._11, m_xmf4x4World._12, m_xmf4x4World._13)));
}

void CGameObject_Racing::MoveStrafe(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Right = GetRight();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Right, fDistance);
	CGameObject_Racing::SetPosition(xmf3Position);
}

void CGameObject_Racing::MoveUp(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Up = GetUp();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Up, fDistance);
	CGameObject_Racing::SetPosition(xmf3Position);
}

void CGameObject_Racing::MoveForward(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Look = GetLook();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Look, fDistance);
	CGameObject_Racing::SetPosition(xmf3Position);
}

void CGameObject_Racing::Rotate(float fPitch, float fYaw, float fRoll)
{
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(fPitch), XMConvertToRadians(fYaw), XMConvertToRadians(fRoll));
	m_xmf4x4Transform = Matrix4x4::Multiply(mtxRotate, m_xmf4x4Transform);

	UpdateTransform(NULL);
}

void CGameObject_Racing::Rotate(XMFLOAT3* pxmf3Axis, float fAngle)
{
	XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(pxmf3Axis), XMConvertToRadians(fAngle));
	m_xmf4x4Transform = Matrix4x4::Multiply(mtxRotate, m_xmf4x4Transform);

	UpdateTransform(NULL);
}

void CGameObject_Racing::Rotate(XMFLOAT4* pxmf4Quaternion)
{
	XMMATRIX mtxRotate = XMMatrixRotationQuaternion(XMLoadFloat4(pxmf4Quaternion));
	m_xmf4x4Transform = Matrix4x4::Multiply(mtxRotate, m_xmf4x4Transform);

	UpdateTransform(NULL);
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
#define _WITH_DEBUG_FRAME_HIERARCHY

CMeshLoadInfo* CGameObject_Racing::LoadMeshInfoFromFile(wifstream& InFile)
{
	TCHAR pstrToken[256] = { 0 };
	int nPositions = 0, nColors = 0, nNormals = 0, nIndices = 0, nSubMeshes = 0, nSubIndices = 0;

	CMeshLoadInfo* pMeshInfo = new CMeshLoadInfo;

	InFile >> pMeshInfo->m_nVertices;
	InFile >> pMeshInfo->m_pstrMeshName;

	for (; ; )
	{
		InFile >> pstrToken;
		if (!InFile) break;

		if (!_tcscmp(pstrToken, _T("<Bounds>:")))
		{
			InFile >> pMeshInfo->m_xmf3AABBCenter.x >> pMeshInfo->m_xmf3AABBCenter.y >> pMeshInfo->m_xmf3AABBCenter.z >> pMeshInfo->m_xmf3AABBExtents.x >> pMeshInfo->m_xmf3AABBExtents.y >> pMeshInfo->m_xmf3AABBExtents.z;
		}
		else if (!_tcscmp(pstrToken, _T("<Positions>:")))
		{
			InFile >> nPositions;
			if (nPositions > 0)
			{
				pMeshInfo->m_nType |= VERTEXT_POSITION;
				pMeshInfo->m_pxmf3Positions = new XMFLOAT3[nPositions];
				for (int i = 0; i < nPositions; i++)
				{
					InFile >> pMeshInfo->m_pxmf3Positions[i].x >> pMeshInfo->m_pxmf3Positions[i].y >> pMeshInfo->m_pxmf3Positions[i].z;
				}
			}
		}
		else if (!_tcscmp(pstrToken, _T("<Colors>:")))
		{
			InFile >> nColors;
			if (nColors > 0)
			{
				pMeshInfo->m_nType |= VERTEXT_COLOR;
				pMeshInfo->m_pxmf4Colors = new XMFLOAT4[nColors];
				for (int i = 0; i < nColors; i++)
				{
					InFile >> pMeshInfo->m_pxmf4Colors[i].x >> pMeshInfo->m_pxmf4Colors[i].y >> pMeshInfo->m_pxmf4Colors[i].z >> pMeshInfo->m_pxmf4Colors[i].w;
				}
			}
		}
		else if (!_tcscmp(pstrToken, _T("<Normals>:")))
		{
			InFile >> nNormals;
			if (nNormals > 0)
			{
				pMeshInfo->m_nType |= VERTEXT_NORMAL;
				pMeshInfo->m_pxmf3Normals = new XMFLOAT3[nNormals];
				for (int i = 0; i < nNormals; i++)
				{
					InFile >> pMeshInfo->m_pxmf3Normals[i].x >> pMeshInfo->m_pxmf3Normals[i].y >> pMeshInfo->m_pxmf3Normals[i].z;
				}
			}
		}
		else if (!_tcscmp(pstrToken, _T("<Indices>:")))
		{
			InFile >> nIndices;
			if (nIndices > 0)
			{
				pMeshInfo->m_pnIndices = new UINT[nIndices];
				for (int i = 0; i < nIndices; i++) InFile >> pMeshInfo->m_pnIndices[i];
			}
		}
		else if (!_tcscmp(pstrToken, _T("<SubMeshes>:")))
		{
			InFile >> pMeshInfo->m_nSubMeshes;
			if (pMeshInfo->m_nSubMeshes > 0)
			{
				pMeshInfo->m_pnSubSetIndices = new int[pMeshInfo->m_nSubMeshes];
				pMeshInfo->m_ppnSubSetIndices = new UINT * [pMeshInfo->m_nSubMeshes];
				for (int i = 0; i < pMeshInfo->m_nSubMeshes; i++)
				{
					InFile >> pstrToken;
					if (!_tcscmp(pstrToken, _T("<SubMesh>:")))
					{
						InFile >> pstrToken; //i
						InFile >> pMeshInfo->m_pnSubSetIndices[i];
						if (pMeshInfo->m_pnSubSetIndices[i] > 0)
						{
							pMeshInfo->m_ppnSubSetIndices[i] = new UINT[pMeshInfo->m_pnSubSetIndices[i]];
							for (int j = 0; j < pMeshInfo->m_pnSubSetIndices[i]; j++) InFile >> pMeshInfo->m_ppnSubSetIndices[i][j];
						}

					}
				}
			}
		}
		else if (!_tcscmp(pstrToken, _T("</Mesh>")))
		{
			break;
		}
	}
	return(pMeshInfo);
}

MATERIALSLOADINFO* CGameObject_Racing::LoadMaterialsInfoFromFile(wifstream& InFile)
{
	TCHAR pstrToken[256] = { 0 };
	int nMaterial = 0;

	MATERIALSLOADINFO* pMaterialsInfo = new MATERIALSLOADINFO;

	InFile >> pMaterialsInfo->m_nMaterials;
	pMaterialsInfo->m_pMaterials = new MATERIALLOADINFO[pMaterialsInfo->m_nMaterials];

	for (; ; )
	{
		InFile >> pstrToken;
		if (!InFile) break;

		if (!_tcscmp(pstrToken, _T("<Material>:")))
		{
			InFile >> nMaterial;
		}
		else if (!_tcscmp(pstrToken, _T("<AlbedoColor>:")))
		{
			InFile >> pMaterialsInfo->m_pMaterials[nMaterial].m_xmf4AlbedoColor.x >> pMaterialsInfo->m_pMaterials[nMaterial].m_xmf4AlbedoColor.y >> pMaterialsInfo->m_pMaterials[nMaterial].m_xmf4AlbedoColor.z >> pMaterialsInfo->m_pMaterials[nMaterial].m_xmf4AlbedoColor.w;
		}
		else if (!_tcscmp(pstrToken, _T("<EmissiveColor>:")))
		{
			InFile >> pMaterialsInfo->m_pMaterials[nMaterial].m_xmf4EmissiveColor.x >> pMaterialsInfo->m_pMaterials[nMaterial].m_xmf4EmissiveColor.y >> pMaterialsInfo->m_pMaterials[nMaterial].m_xmf4EmissiveColor.z >> pMaterialsInfo->m_pMaterials[nMaterial].m_xmf4EmissiveColor.w;
		}
		else if (!_tcscmp(pstrToken, _T("<SpecularColor>:")))
		{
			InFile >> pMaterialsInfo->m_pMaterials[nMaterial].m_xmf4SpecularColor.x >> pMaterialsInfo->m_pMaterials[nMaterial].m_xmf4SpecularColor.y >> pMaterialsInfo->m_pMaterials[nMaterial].m_xmf4SpecularColor.z >> pMaterialsInfo->m_pMaterials[nMaterial].m_xmf4SpecularColor.w;
		}
		else if (!_tcscmp(pstrToken, _T("<Glossiness>:")))
		{
			InFile >> pMaterialsInfo->m_pMaterials[nMaterial].m_fGlossiness;
		}
		else if (!_tcscmp(pstrToken, _T("<Smoothness>:")))
		{
			InFile >> pMaterialsInfo->m_pMaterials[nMaterial].m_fSmoothness;
		}
		else if (!_tcscmp(pstrToken, _T("<Metallic>:")))
		{
			InFile >> pMaterialsInfo->m_pMaterials[nMaterial].m_fSpecularHighlight;
		}
		else if (!_tcscmp(pstrToken, _T("<SpecularHighlight>:")))
		{
			InFile >> pMaterialsInfo->m_pMaterials[nMaterial].m_fMetallic;
		}
		else if (!_tcscmp(pstrToken, _T("<GlossyReflection>:")))
		{
			InFile >> pMaterialsInfo->m_pMaterials[nMaterial].m_fGlossyReflection;
		}
		else if (!_tcscmp(pstrToken, _T("</Materials>")))
		{
			break;
		}
	}
	return(pMaterialsInfo);
}

CGameObject_Racing* CGameObject_Racing::LoadFrameHierarchyFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, wifstream& InFile)
{
	TCHAR pstrToken[256] = { 0 };

	int nFrame = 0;

	CGameObject_Racing* pGameObject = NULL;

	for (; ; )
	{
		InFile >> pstrToken;
		if (!InFile) break;

		if (!_tcscmp(pstrToken, _T("<Frame>:")))
		{
			pGameObject = new CGameObject_Racing();

			InFile >> nFrame;
			InFile >> pGameObject->m_pstrFrameName;
		}
		else if (!_tcscmp(pstrToken, _T("<Transform>:")))
		{
			XMFLOAT3 xmf3Position, xmf3Rotation, xmf3Scale;
			XMFLOAT4 xmf4Rotation;
			InFile >> xmf3Position.x >> xmf3Position.y >> xmf3Position.z;
			InFile >> xmf3Rotation.x >> xmf3Rotation.y >> xmf3Rotation.z; //Euler Angle
			InFile >> xmf3Scale.x >> xmf3Scale.y >> xmf3Scale.z;
			InFile >> xmf4Rotation.x >> xmf4Rotation.y >> xmf4Rotation.z >> xmf4Rotation.w; //Quaternion
		}
		else if (!_tcscmp(pstrToken, _T("<TransformMatrix>:")))
		{
			InFile >> pGameObject->m_xmf4x4Transform._11 >> pGameObject->m_xmf4x4Transform._12 >> pGameObject->m_xmf4x4Transform._13 >> pGameObject->m_xmf4x4Transform._14; //ToParentTransform
			InFile >> pGameObject->m_xmf4x4Transform._21 >> pGameObject->m_xmf4x4Transform._22 >> pGameObject->m_xmf4x4Transform._23 >> pGameObject->m_xmf4x4Transform._24;
			InFile >> pGameObject->m_xmf4x4Transform._31 >> pGameObject->m_xmf4x4Transform._32 >> pGameObject->m_xmf4x4Transform._33 >> pGameObject->m_xmf4x4Transform._34;
			InFile >> pGameObject->m_xmf4x4Transform._41 >> pGameObject->m_xmf4x4Transform._42 >> pGameObject->m_xmf4x4Transform._43 >> pGameObject->m_xmf4x4Transform._44;
		}
		else if (!_tcscmp(pstrToken, _T("<Mesh>:")))
		{
			CMeshLoadInfo* pMeshInfo = pGameObject->LoadMeshInfoFromFile(InFile);
			if (pMeshInfo)
			{
				CMesh_Racing* pMesh = NULL;
				if (pMeshInfo->m_nType & VERTEXT_NORMAL)
				{
					pMesh = new CMeshIlluminatedFromFile(pd3dDevice, pd3dCommandList, pMeshInfo);
				}
				if (pMesh) pGameObject->SetMesh(pMesh);
				delete pMeshInfo;
			}
		}
		else if (!_tcscmp(pstrToken, _T("<Materials>:")))
		{
			MATERIALSLOADINFO* pMaterialsInfo = pGameObject->LoadMaterialsInfoFromFile(InFile);
			if (pMaterialsInfo && (pMaterialsInfo->m_nMaterials > 0))
			{
				pGameObject->m_nMaterials = pMaterialsInfo->m_nMaterials;
				pGameObject->m_ppMaterials = new CMaterial_Racing * [pMaterialsInfo->m_nMaterials];

				for (int i = 0; i < pMaterialsInfo->m_nMaterials; i++)
				{
					pGameObject->m_ppMaterials[i] = NULL;

					CMaterial_Racing* pMaterial = new CMaterial_Racing();
					if (pGameObject->GetMeshType() & VERTEXT_NORMAL) pMaterial->SetPseudoLightingShader();

					pGameObject->SetMaterial(i, pMaterial);
				}
			}
		}
		else if (!_tcscmp(pstrToken, _T("<Children>:")))
		{
			int nChilds = 0;
			InFile >> nChilds;

			if (nChilds > 0)
			{
				for (int i = 0; i < nChilds; i++)
				{
					CGameObject_Racing* pChild = CGameObject_Racing::LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, InFile);
					if (pChild) pGameObject->SetChild(pChild);
#ifdef _WITH_DEBUG_RUNTIME_FRAME_HIERARCHY
					TCHAR pstrDebug[256] = { 0 };
					_stprintf_s(pstrDebug, 256, _T("(Child Frame: %p) (Parent Frame: %p)\n"), pChild, pGameObject);
					OutputDebugString(pstrDebug);
#endif
				}
			}
		}
		else if (!_tcscmp(pstrToken, _T("</Frame>")))
		{
			break;
		}
	}
	return(pGameObject);
}

void CGameObject_Racing::PrintFrameInfo(CGameObject_Racing* pGameObject, CGameObject_Racing* pParent)
{
	TCHAR pstrDebug[256] = { 0 };

	_stprintf_s(pstrDebug, 256, _T("(Frame: %p) (Parent: %p)\n"), pGameObject, pParent);
	OutputDebugString(pstrDebug);

	if (pGameObject->m_pSibling) CGameObject_Racing::PrintFrameInfo(pGameObject->m_pSibling, pParent);
	if (pGameObject->m_pChild) CGameObject_Racing::PrintFrameInfo(pGameObject->m_pChild, pGameObject);
}

CGameObject_Racing* CGameObject_Racing::LoadGeometryFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, ID3D12RootSignature* pd3dGraphicsRootSignature, const TCHAR* pstrFileName)
{
	CGameObject_Racing* pGameObject = NULL;

	wifstream InFile(pstrFileName);

	TCHAR pstrToken[256] = { 0 };

	for (; ; )
	{
		InFile >> pstrToken;
		if (!InFile) break;

		if (!_tcscmp(pstrToken, _T("<Hierarchy>:")))
		{
			pGameObject = CGameObject_Racing::LoadFrameHierarchyFromFile(pd3dDevice, pd3dCommandList, pd3dGraphicsRootSignature, InFile);
		}
		else if (!_tcscmp(pstrToken, _T("</Hierarchy>")))
		{
			break;
		}
	}

#ifdef _WITH_DEBUG_FRAME_HIERARCHY
	TCHAR pstrDebug[256] = { 0 };
	_stprintf_s(pstrDebug, 256, _T("\nFrame Hierarchy(%s)\n"), pstrFileName);
	OutputDebugString(pstrDebug);

	CGameObject_Racing::PrintFrameInfo(pGameObject, NULL);
#endif

	return(pGameObject);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
CRotatingObject::CRotatingObject()
{
	m_xmf3RotationAxis = XMFLOAT3(0.0f, 1.0f, 0.0f);
	m_fRotationSpeed = 15.0f;
}

CRotatingObject::~CRotatingObject()
{
}

void CRotatingObject::Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent)
{
	CGameObject_Racing::Rotate(&m_xmf3RotationAxis, m_fRotationSpeed * fTimeElapsed);

	CGameObject_Racing::Animate(fTimeElapsed, pxmf4x4Parent);
}

void CRotatingObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	CGameObject_Racing::Render(pd3dCommandList, pCamera);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
CRevolvingObject::CRevolvingObject()
{
	m_xmf3RevolutionAxis = XMFLOAT3(1.0f, 0.0f, 0.0f);
	m_fRevolutionSpeed = 0.0f;
}

CRevolvingObject::~CRevolvingObject()
{
}

void CRevolvingObject::Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent)
{
	XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(&m_xmf3RevolutionAxis), XMConvertToRadians(m_fRevolutionSpeed * fTimeElapsed));
	m_xmf4x4Transform = Matrix4x4::Multiply(m_xmf4x4Transform, mtxRotate);

	CGameObject_Racing::Animate(fTimeElapsed, pxmf4x4Parent);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//

XMFLOAT3 CCactusObject::m_pxmf3SphereVectors[EXPLOSION_DEBRISES];
CDiffusedMesh* CCactusObject::m_pExplosionMesh = NULL;

CCactusObject::CCactusObject()
{
	m_xmOOBB = BoundingOrientedBox(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
	m_bBreakable = true;
}

CCactusObject::~CCactusObject()
{
}

void CCactusObject::OnInitialize()
{
}

void CCactusObject::PrepareExplosion(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	for (int i = 0; i < EXPLOSION_DEBRISES; i++) XMStoreFloat3(&m_pxmf3SphereVectors[i], ::RandomUnitVectorOnSphere());

	m_pExplosionMesh = new CCubeMeshDiffused(pd3dDevice, pd3dCommandList, 0.2f, 0.2f, 0.2f);
}

void CCactusObject::Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent)
{
	if (m_bBlowingUp)
	{
		m_fElapsedTimes += fTimeElapsed;
		if (m_fElapsedTimes <= m_fDuration)
		{
			XMFLOAT3 xmf3Position = GetPosition();
			for (int i = 0; i < EXPLOSION_DEBRISES; i++)
			{
				m_pxmf4x4Transforms[i] = Matrix4x4::Identity();
				m_pxmf4x4Transforms[i]._41 = xmf3Position.x + m_pxmf3SphereVectors[i].x * m_fExplosionSpeed * m_fElapsedTimes;
				m_pxmf4x4Transforms[i]._42 = xmf3Position.y + m_pxmf3SphereVectors[i].y * m_fExplosionSpeed * m_fElapsedTimes;
				m_pxmf4x4Transforms[i]._43 = xmf3Position.z + m_pxmf3SphereVectors[i].z * m_fExplosionSpeed * m_fElapsedTimes;
				m_pxmf4x4Transforms[i] = Matrix4x4::Multiply(Matrix4x4::RotationAxis(m_pxmf3SphereVectors[i], m_fExplosionRotation * m_fElapsedTimes), m_pxmf4x4Transforms[i]);
			}
		}
		else
		{

		}
	}
	else
	{
		CGameObject_Racing::Animate(fTimeElapsed, pxmf4x4Parent);
	}
}

void CCactusObject::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	if (m_bBlowingUp && m_fElapsedTimes > m_fDuration)
	{
	}
	else if (m_bBlowingUp) {
		for (int i = 0; i < EXPLOSION_DEBRISES; i++)
		{
			CGameObject_Racing::Render(pd3dCommandList, &m_pxmf4x4Transforms[i], m_pExplosionMesh);
		}
	}
	else
	{
		CGameObject_Racing::Render(pd3dCommandList, pCamera);
	}
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
CTreeObject::CTreeObject()
{
	m_xmOOBB = BoundingOrientedBox(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.5f, 1.5f, 1.5f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
	m_bBreakable = false;
}

CTreeObject::~CTreeObject()
{
}

void CTreeObject::OnInitialize()
{
}

void CTreeObject::Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent)
{
	CGameObject_Racing::Animate(fTimeElapsed, pxmf4x4Parent);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
CRock1Object::CRock1Object()
{
	m_xmOOBB = BoundingOrientedBox(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
	m_bBreakable = false;
}

CRock1Object::~CRock1Object()
{
}

void CRock1Object::OnInitialize()
{
}

void CRock1Object::Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent)
{
	CGameObject_Racing::Animate(fTimeElapsed, pxmf4x4Parent);
}

////////////////////////////////////////////////////////////////////////////////////////////////////
//
CRock2Object::CRock2Object()
{
	m_xmOOBB = BoundingOrientedBox(XMFLOAT3(0.0f, 0.0f, 0.0f), XMFLOAT3(1.0f, 1.0f, 1.0f), XMFLOAT4(0.0f, 0.0f, 0.0f, 1.0f));
	m_bBreakable = false;
}

CRock2Object::~CRock2Object()
{
}

void CRock2Object::OnInitialize()
{
}

void CRock2Object::Animate(float fTimeElapsed, XMFLOAT4X4* pxmf4x4Parent)
{
	CGameObject_Racing::Animate(fTimeElapsed, pxmf4x4Parent);
}
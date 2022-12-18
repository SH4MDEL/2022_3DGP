#include "stdafx.h"
#include "Object_Village.h"
#include "Shader_Village.h"

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
//
CMaterial_Village::CMaterial_Village()
{
}

CMaterial_Village::~CMaterial_Village()
{
}

void CMaterial_Village::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
}

void CMaterial_Village::ReleaseShaderVariables()
{
}

void CMaterial_Village::ReleaseUploadBuffers()
{
}


CGameObject_Village::CGameObject_Village(UINT nMaterials)
{
	m_nMaterials = nMaterials;
	m_ppMaterials = new CMaterial_Village * [m_nMaterials];
	for (UINT i = 0; i < m_nMaterials; i++) m_ppMaterials[i] = NULL;

	m_xmf4x4World = Matrix4x4::Identity();
}

CGameObject_Village::~CGameObject_Village()
{
	ReleaseShaderVariables();

	if (m_pMesh) m_pMesh->Release();
	m_pMesh = NULL;

	if (m_ppMaterials)
	{
		for (UINT i = 0; i < m_nMaterials; i++) if (m_ppMaterials[i]) m_ppMaterials[i]->Release();
		delete[] m_ppMaterials;
	}

	if (m_pShader) m_pShader->Release();
}

void CGameObject_Village::SetBoundingBox()
{
	m_xmOOBB.Center = m_pMesh->m_xmBoundingBox.Center;
	m_xmOOBB.Extents = m_pMesh->m_xmBoundingBox.Extents;
}

void CGameObject_Village::SetMesh(CMesh_Village* pMesh)
{
	if (m_pMesh) m_pMesh->Release();
	m_pMesh = pMesh;
	if (pMesh) pMesh->AddRef();
}

void CGameObject_Village::SetShader(CShader_Village* pShader)
{
	if (m_pShader) m_pShader->Release();
	m_pShader = pShader;
	if (m_pShader) m_pShader->AddRef();
}

void CGameObject_Village::SetAlbedoColor(UINT nIndex, XMFLOAT4 xmf4Color)
{
	if ((nIndex >= 0) && (nIndex < m_nMaterials))
	{
		if (!m_ppMaterials[nIndex])
		{
			m_ppMaterials[nIndex] = new CMaterial_Village();
			m_ppMaterials[nIndex]->AddRef();
		}
		m_ppMaterials[nIndex]->SetAlbedoColor(xmf4Color);
	}
}

void CGameObject_Village::SetEmissionColor(UINT nIndex, XMFLOAT4 xmf4Color)
{
	if ((nIndex >= 0) && (nIndex < m_nMaterials))
	{
		if (!m_ppMaterials[nIndex])
		{
			m_ppMaterials[nIndex] = new CMaterial_Village();
			m_ppMaterials[nIndex]->AddRef();
		}
		m_ppMaterials[nIndex]->SetEmissionColor(xmf4Color);
	}
}

void CGameObject_Village::SetMaterial(UINT nIndex, CMaterial_Village* pMaterial)
{
	if ((nIndex >= 0) && (nIndex < m_nMaterials))
	{
		if (m_ppMaterials[nIndex]) m_ppMaterials[nIndex]->Release();
		m_ppMaterials[nIndex] = pMaterial;
		if (pMaterial) pMaterial->AddRef();
	}
}

void CGameObject_Village::SetMaterial(UINT nIndex, UINT nReflection)
{
	if ((nIndex >= 0) && (nIndex < m_nMaterials))
	{
		if (!m_ppMaterials[nIndex])
		{
			m_ppMaterials[nIndex] = new CMaterial_Village();
			m_ppMaterials[nIndex]->AddRef();
		}
		m_ppMaterials[nIndex]->m_nMaterial = nReflection;
	}
}

void CGameObject_Village::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	UINT ncbElementBytes = ((sizeof(CB_GAMEOBJECT_INFO) + 255) & ~255); //256의 배수
	m_pd3dcbGameObject = ::CreateBufferResource(pd3dDevice, pd3dCommandList, NULL, ncbElementBytes, D3D12_HEAP_TYPE_UPLOAD, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, NULL);

	m_pd3dcbGameObject->Map(0, NULL, (void**)&m_pcbMappedGameObject);
}

void CGameObject_Village::ReleaseShaderVariables()
{
	if (m_pd3dcbGameObject)
	{
		m_pd3dcbGameObject->Unmap(0, NULL);
		m_pd3dcbGameObject->Release();
	}
	for (UINT i = 0; i < m_nMaterials; i++) if (m_ppMaterials[i]) m_ppMaterials[i]->ReleaseShaderVariables();
	if (m_pShader) m_pShader->ReleaseShaderVariables();
}

void CGameObject_Village::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
	XMStoreFloat4x4(&m_pcbMappedGameObject->m_xmf4x4World, XMMatrixTranspose(XMLoadFloat4x4(&m_xmf4x4World)));

	D3D12_GPU_VIRTUAL_ADDRESS d3dGpuVirtualAddress = m_pd3dcbGameObject->GetGPUVirtualAddress();
	pd3dCommandList->SetGraphicsRootConstantBufferView(1, d3dGpuVirtualAddress);
}

void CGameObject_Village::Animate(float fTimeElapsed)
{
	UpdateBoundingBox();
}

void CGameObject_Village::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	OnPrepareRender();

	if (m_pMesh && m_ppMaterials)
	{
		if (m_pShader)
		{
			m_pShader->Render(pd3dCommandList, pCamera);
			m_pShader->UpdateShaderVariables(pd3dCommandList);
		}
		if (m_pd3dcbGameObject && m_pcbMappedGameObject) UpdateShaderVariables(pd3dCommandList);

		m_pMesh->OnPreRender(pd3dCommandList);
		for (UINT i = 0; i < m_nMaterials; i++)
		{
			if (m_ppMaterials[i])
			{
				pd3dCommandList->SetGraphicsRoot32BitConstant(4, m_ppMaterials[i]->m_nMaterial, 0);
				pd3dCommandList->SetGraphicsRoot32BitConstants(4, 4, &m_ppMaterials[i]->m_xmf4AlbedoColor, 4);
				pd3dCommandList->SetGraphicsRoot32BitConstants(4, 4, &m_ppMaterials[i]->m_xmf4EmissionColor, 8);
			}
			m_pMesh->Render(pd3dCommandList, i);
		}
	}
}

void CGameObject_Village::UpdateBoundingBox()
{
	// OOBB의 중심을 월드좌표로 이동
	XMFLOAT3 Pos = GetPosition();
	m_xmOOBB.Center = Pos;
}


void CGameObject_Village::ReleaseUploadBuffers()
{
	if (m_pMesh) m_pMesh->ReleaseUploadBuffers();
	for (UINT i = 0; i < m_nMaterials; i++) if (m_ppMaterials[i]) m_ppMaterials[i]->ReleaseUploadBuffers();
	if (m_pShader) m_pShader->ReleaseUploadBuffers();
}

void CGameObject_Village::SetPosition(float x, float y, float z)
{
	m_xmf4x4World._41 = x;
	m_xmf4x4World._42 = y;
	m_xmf4x4World._43 = z;
}

void CGameObject_Village::SetPosition(XMFLOAT3 xmf3Position)
{
	SetPosition(xmf3Position.x, xmf3Position.y, xmf3Position.z);
}

void CGameObject_Village::SetScale(float x, float y, float z)
{
	XMMATRIX mtxScale = XMMatrixScaling(x, y, z);
	m_xmf4x4World = Matrix4x4::Multiply(mtxScale, m_xmf4x4World);
}

XMFLOAT3 CGameObject_Village::GetPosition()
{
	return(XMFLOAT3(m_xmf4x4World._41, m_xmf4x4World._42, m_xmf4x4World._43));
}

XMFLOAT3 CGameObject_Village::GetLook()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._31, m_xmf4x4World._32, m_xmf4x4World._33)));
}

XMFLOAT3 CGameObject_Village::GetUp()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._21, m_xmf4x4World._22, m_xmf4x4World._23)));
}

XMFLOAT3 CGameObject_Village::GetRight()
{
	return(Vector3::Normalize(XMFLOAT3(m_xmf4x4World._11, m_xmf4x4World._12, m_xmf4x4World._13)));
}

void CGameObject_Village::MoveStrafe(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Right = GetRight();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Right, fDistance);
	CGameObject_Village::SetPosition(xmf3Position);
}

void CGameObject_Village::MoveUp(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Up = GetUp();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Up, fDistance);
	CGameObject_Village::SetPosition(xmf3Position);
}

void CGameObject_Village::MoveForward(float fDistance)
{
	XMFLOAT3 xmf3Position = GetPosition();
	XMFLOAT3 xmf3Look = GetLook();
	xmf3Position = Vector3::Add(xmf3Position, xmf3Look, fDistance);
	CGameObject_Village::SetPosition(xmf3Position);
}

void CGameObject_Village::Rotate(float fPitch, float fYaw, float fRoll)
{
	XMMATRIX mtxRotate = XMMatrixRotationRollPitchYaw(XMConvertToRadians(fPitch), XMConvertToRadians(fYaw), XMConvertToRadians(fRoll));
	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);
}

void CGameObject_Village::Rotate(XMFLOAT3* pxmf3Axis, float fAngle)
{
	XMMATRIX mtxRotate = XMMatrixRotationAxis(XMLoadFloat3(pxmf3Axis), XMConvertToRadians(fAngle));
	m_xmf4x4World = Matrix4x4::Multiply(mtxRotate, m_xmf4x4World);
}

void CGameObject_Village::LoadGameObjectFromFile(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList, const char* pstrFileName)
{
	FILE* pFile = NULL;
	::fopen_s(&pFile, pstrFileName, "rb");
	::rewind(pFile);

	char pstrToken[64] = { '\0' };
	char pstrGameObjectName[64] = { '\0' };
	char pstrFilePath[64] = { '\0' };

	BYTE nStrLength = 0, nObjectNameLength = 0;
	UINT nReads = 0, nMaterials = 0;
	size_t nConverted = 0;

	nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pFile);
	nReads = (UINT)::fread(pstrToken, sizeof(char), nStrLength, pFile); //"<GameObject>:"
	nReads = (UINT)::fread(&nObjectNameLength, sizeof(BYTE), 1, pFile);
	nReads = (UINT)::fread(pstrGameObjectName, sizeof(char), nObjectNameLength, pFile);
	pstrGameObjectName[nObjectNameLength] = '\0';

	nReads = (UINT)::fread(&nStrLength, sizeof(BYTE), 1, pFile);
	nReads = (UINT)::fread(pstrToken, sizeof(char), nStrLength, pFile); //"<Materials>:"
	nReads = (UINT)::fread(&nMaterials, sizeof(int), 1, pFile);

	strcpy_s(m_pstrName, 64, pstrGameObjectName);

	XMFLOAT4 xmf4AlbedoColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f), xmf4EmissionColor = XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	for (UINT k = 0; k < nMaterials; k++)
	{
		SetMaterial(k, 1);

		nReads = (UINT)::fread(&xmf4AlbedoColor, sizeof(float), 4, pFile);
		SetAlbedoColor(k, xmf4AlbedoColor);

		nReads = (UINT)::fread(&xmf4EmissionColor, sizeof(float), 4, pFile);
		SetEmissionColor(k, xmf4EmissionColor);
	}

	nReads = (UINT)::fread(&m_xmf4x4World, sizeof(float), 16, pFile);

	strcpy_s(pstrFilePath, 64, "Models/");
	strcpy_s(pstrFilePath + 7, 64 - 7, pstrGameObjectName);
	strcpy_s(pstrFilePath + 7 + nObjectNameLength, 64 - 7 - nObjectNameLength, ".bin");
	CMesh_Village* pMesh = new CMesh_Village(pd3dDevice, pd3dCommandList, pstrFilePath);
	SetMesh(pMesh);

	::fclose(pFile);
}
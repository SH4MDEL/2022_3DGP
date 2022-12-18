//-----------------------------------------------------------------------------
// File: CScene.cpp
//-----------------------------------------------------------------------------

#include "stdafx.h"
#include "Scene.h"

CScene::CScene()
{
}

CScene::~CScene()
{
}

void CScene::BuildObjects(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
	CCactusObject::PrepareExplosion(pd3dDevice, pd3dCommandList);

	m_pd3dGraphicsRootSignature = CreateGraphicsRootSignature(pd3dDevice);

	CMaterial::PrepareShaders(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature);

	std::ifstream in("ObjectData.txt");
	char data;
	float x = 0, z = 0;

	CGameObject* TreeModel = CGameObject::LoadGeometryFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Models/Tree.txt");
	CGameObject* CactusModel = CGameObject::LoadGeometryFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Models/Cactus.txt");
	CGameObject* Rock1Model = CGameObject::LoadGeometryFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Models/Rock.txt");
	CGameObject* Rock2Model = CGameObject::LoadGeometryFromFile(pd3dDevice, pd3dCommandList, m_pd3dGraphicsRootSignature, L"Models/Rock2.txt");

	CTreeObject* TreeObject = nullptr;
	CCactusObject* CactusObject = nullptr;
	CRock1Object* Rock1Object = nullptr;
	CRock2Object* Rock2Object = nullptr;

	while (in >> data) {
		if (data == '1') {
			TreeObject = new CTreeObject();
			TreeObject->SetChild(TreeModel, true);
			TreeObject->OnInitialize();
			TreeObject->SetPosition(x * 5.0f, 0.0f, z * 5.0f);
			TreeObject->UpdateBoundingBox();
			m_vGameObjects.push_back(TreeObject);
		}
		else if (data == '2') {
			CactusObject = new CCactusObject();
			CactusObject->SetChild(CactusModel, true);
			CactusObject->OnInitialize();
			CactusObject->SetPosition(x * 5.0f, 0.0f, z * 5.0f);
			CactusObject->SetScale(2.0f, 2.0f, 2.0f);
			CactusObject->UpdateBoundingBox();
			m_vGameObjects.push_back(CactusObject);
		}
		else if (data == '3') {
			Rock1Object = new CRock1Object();
			Rock1Object->SetChild(Rock1Model, true);
			Rock1Object->OnInitialize();
			Rock1Object->SetPosition(x * 5.0f, 0.0f, z * 5.0f);
			Rock1Object->SetScale(1.5f, 1.5f, 1.5f);
			Rock1Object->UpdateBoundingBox();
			m_vGameObjects.push_back(Rock1Object);
		}
		else if (data == '4') {
			Rock2Object = new CRock2Object();
			Rock2Object->SetChild(Rock2Model, true);
			Rock2Object->OnInitialize();
			Rock2Object->SetPosition(x * 5.0f, 0.0f, z * 5.0f);
			Rock2Object->SetScale(1.5f, 1.5f, 1.5f);
			Rock2Object->UpdateBoundingBox();
			m_vGameObjects.push_back(Rock2Object);
		}
		if (x == 99.0f) {
			x = 0.0f;
			z += 1.0f;
		}
		else {
			x += 1.0f;
		}

	}

	CreateShaderVariables(pd3dDevice, pd3dCommandList);
}

void CScene::ReleaseObjects()
{
	if (m_pd3dGraphicsRootSignature) m_pd3dGraphicsRootSignature->Release();

	while (m_vGameObjects.size())
	{
		m_vGameObjects.pop_back();
	}

	ReleaseShaderVariables();
}

bool CScene::CheckPlayerByObjectCollisions()
{
	for (const auto& elem : m_vGameObjects) {
		if (m_pPlayer->m_xmMovedOOBB.Intersects(elem->m_xmMovedOOBB)) {
			if (elem->m_bBlowingUp) {
				return false;
			}
			if (elem->m_bBreakable && m_pPlayer->GetisBoost()) {
				elem->m_bBlowingUp = true;
				return false;
			}
			return true;
		}
	}
	return false;
}

ID3D12RootSignature* CScene::CreateGraphicsRootSignature(ID3D12Device* pd3dDevice)
{
	ID3D12RootSignature* pd3dGraphicsRootSignature = NULL;

	D3D12_ROOT_PARAMETER pd3dRootParameters[2];

	pd3dRootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	pd3dRootParameters[0].Descriptor.ShaderRegister = 1; //Camera
	pd3dRootParameters[0].Descriptor.RegisterSpace = 0;
	pd3dRootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	pd3dRootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	pd3dRootParameters[1].Constants.Num32BitValues = 32;
	pd3dRootParameters[1].Constants.ShaderRegister = 2; //GameObject
	pd3dRootParameters[1].Constants.RegisterSpace = 0;
	pd3dRootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	D3D12_ROOT_SIGNATURE_FLAGS d3dRootSignatureFlags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS | D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS;
	D3D12_ROOT_SIGNATURE_DESC d3dRootSignatureDesc;
	::ZeroMemory(&d3dRootSignatureDesc, sizeof(D3D12_ROOT_SIGNATURE_DESC));
	d3dRootSignatureDesc.NumParameters = _countof(pd3dRootParameters);
	d3dRootSignatureDesc.pParameters = pd3dRootParameters;
	d3dRootSignatureDesc.NumStaticSamplers = 0;
	d3dRootSignatureDesc.pStaticSamplers = NULL;
	d3dRootSignatureDesc.Flags = d3dRootSignatureFlags;

	ID3DBlob* pd3dSignatureBlob = NULL;
	ID3DBlob* pd3dErrorBlob = NULL;
	D3D12SerializeRootSignature(&d3dRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &pd3dSignatureBlob, &pd3dErrorBlob);
	pd3dDevice->CreateRootSignature(0, pd3dSignatureBlob->GetBufferPointer(), pd3dSignatureBlob->GetBufferSize(), __uuidof(ID3D12RootSignature), (void**)& pd3dGraphicsRootSignature);
	if (pd3dSignatureBlob) pd3dSignatureBlob->Release();
	if (pd3dErrorBlob) pd3dErrorBlob->Release();

	return(pd3dGraphicsRootSignature);
}

void CScene::CreateShaderVariables(ID3D12Device* pd3dDevice, ID3D12GraphicsCommandList* pd3dCommandList)
{
}

void CScene::UpdateShaderVariables(ID3D12GraphicsCommandList* pd3dCommandList)
{
}

void CScene::ReleaseShaderVariables()
{
}

void CScene::ReleaseUploadBuffers()
{
	for (const auto& elem : m_vGameObjects) elem->ReleaseUploadBuffers();
}

bool CScene::OnProcessingMouseMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return(false);
}

bool CScene::OnProcessingKeyboardMessage(HWND hWnd, UINT nMessageID, WPARAM wParam, LPARAM lParam)
{
	return(false);
}

bool CScene::ProcessInput(UCHAR* pKeysBuffer)
{
	return(false);
}

void CScene::AnimateObjects(float fTimeElapsed)
{
	for (const auto& elem : m_vGameObjects) elem->Animate(fTimeElapsed, NULL);

	for (const auto& elem : m_vGameObjects) elem->UpdateTransform(NULL);

	//CheckPlayerByObjectCollisions();
}

void CScene::Render(ID3D12GraphicsCommandList* pd3dCommandList, CCamera* pCamera)
{
	pd3dCommandList->SetGraphicsRootSignature(m_pd3dGraphicsRootSignature);

	pCamera->SetViewportsAndScissorRects(pd3dCommandList);
	pCamera->UpdateShaderVariables(pd3dCommandList);

	UpdateShaderVariables(pd3dCommandList);

	for (const auto& elem : m_vGameObjects) {
		elem->UpdateTransform(NULL);
		elem->Render(pd3dCommandList, pCamera);
	}
}
